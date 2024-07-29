/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.0  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file PHY/NR_TRANSPORT/nr_ulsch_decoding.c
* \brief Top-level routines for decoding  LDPC (ULSCH) transport channels from 38.212, V15.4.0 2018-12
* \author Ahmed Hussein
* \date 2019
* \version 0.1
* \company Fraunhofer IIS
* \email: ahmed.hussein@iis.fraunhofer.de
* \note
* \warning
*/


// [from gNB coding]
#include "PHY/defs_gNB.h"
#include "PHY/CODING/coding_extern.h"
#include "PHY/CODING/coding_defs.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "PHY/CODING/nrLDPC_coding_interface.h"
#include "PHY/CODING/nrLDPC_extern.h"
#include "PHY/NR_TRANSPORT/nr_transport_common_proto.h"
#include "PHY/NR_TRANSPORT/nr_transport_proto.h"
#include "PHY/NR_TRANSPORT/nr_ulsch.h"
#include "PHY/NR_TRANSPORT/nr_dlsch.h"
#include "SCHED_NR/sched_nr.h"
#include "SCHED_NR/fapi_nr_l1.h"
#include "defs.h"
#include "common/utils/LOG/vcd_signal_dumper.h"
#include "common/utils/LOG/log.h"
#include <syscall.h>
//#define DEBUG_ULSCH_DECODING
//#define gNB_DEBUG_TRACE

#define OAI_UL_LDPC_MAX_NUM_LLR 27000//26112 // NR_LDPC_NCOL_BG1*NR_LDPC_ZMAX = 68*384
//#define DEBUG_CRC
#ifdef DEBUG_CRC
#define PRINT_CRC_CHECK(a) a
#else
#define PRINT_CRC_CHECK(a)
#endif

//extern double cpuf;

int nr_ulsch_decoding_slot(PHY_VARS_gNB *phy_vars_gNB,
                           NR_DL_FRAME_PARMS *frame_parms,
                           uint32_t frame,
                           uint8_t nr_tti_rx,
                           uint32_t *G,
                           uint8_t *ULSCH_ids,
                           int nb_pusch)
{

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_gNB_ULSCH_DECODING, 1);

  nrLDPC_slot_decoding_parameters_t nrLDPC_slot_decoding_parameters;
  nrLDPC_slot_decoding_parameters.frame = frame;
  nrLDPC_slot_decoding_parameters.slot = nr_tti_rx;
  nrLDPC_slot_decoding_parameters.nb_TBs = nb_pusch;
  nrLDPC_slot_decoding_parameters.threadPool = &phy_vars_gNB->threadPool;
  nrLDPC_slot_decoding_parameters.respDecode = &phy_vars_gNB->respDecode;
  nrLDPC_TB_decoding_parameters_t TBs[nb_pusch];
  nrLDPC_slot_decoding_parameters.TBs = TBs;

  int max_num_segments = 0;

  for (uint8_t pusch_id = 0; pusch_id < nb_pusch; pusch_id++) {
    uint8_t ULSCH_id = ULSCH_ids[pusch_id];
    NR_gNB_ULSCH_t *ulsch = &phy_vars_gNB->ulsch[ULSCH_id];
    NR_gNB_PUSCH *pusch = &phy_vars_gNB->pusch_vars[ULSCH_id];
    NR_UL_gNB_HARQ_t *harq_process = ulsch->harq_process;
    nfapi_nr_pusch_pdu_t *pusch_pdu = &phy_vars_gNB->ulsch[ULSCH_id].harq_process->ulsch_pdu;

    nrLDPC_TB_decoding_parameters_t nrLDPC_TB_decoding_parameters;

    nrLDPC_TB_decoding_parameters.G = G[pusch_id];

    if (!harq_process) {
      LOG_E(PHY, "ulsch_decoding.c: NULL harq_process pointer\n");
      return -1;
    }
  
    // ------------------------------------------------------------------
    nrLDPC_TB_decoding_parameters.nb_rb = pusch_pdu->rb_size;
    nrLDPC_TB_decoding_parameters.Qm = pusch_pdu->qam_mod_order;
    nrLDPC_TB_decoding_parameters.mcs = pusch_pdu->mcs_index;
    nrLDPC_TB_decoding_parameters.nb_layers = pusch_pdu->nrOfLayers;
    // ------------------------------------------------------------------
  
    harq_process->processedSegments = 0;
    harq_process->TBS = pusch_pdu->pusch_data.tb_size;
  
    nrLDPC_TB_decoding_parameters.BG = pusch_pdu->maintenance_parms_v3.ldpcBaseGraph;
    nrLDPC_TB_decoding_parameters.A = (harq_process->TBS) << 3;
    NR_gNB_PHY_STATS_t *stats = get_phy_stats(phy_vars_gNB, ulsch->rnti);
    if (stats) {
      stats->frame = frame;
      stats->ulsch_stats.round_trials[harq_process->round]++;
      for (int aarx = 0; aarx < frame_parms->nb_antennas_rx; aarx++) {
        stats->ulsch_stats.power[aarx] = dB_fixed_x10(pusch->ulsch_power[aarx]);
        stats->ulsch_stats.noise_power[aarx] = dB_fixed_x10(pusch->ulsch_noise_power[aarx]);
      }
      if (!harq_process->harq_to_be_cleared) {
        stats->ulsch_stats.current_Qm = nrLDPC_TB_decoding_parameters.Qm;
        stats->ulsch_stats.current_RI = nrLDPC_TB_decoding_parameters.nb_layers;
        stats->ulsch_stats.total_bytes_tx += harq_process->TBS;
      }
    }
  
    uint8_t harq_pid = ulsch->harq_pid;
    LOG_D(PHY,
          "ULSCH Decoding, harq_pid %d rnti %x TBS %d G %d mcs %d Nl %d nb_rb %d, Qm %d, Coderate %f RV %d round %d new RX %d\n",
          harq_pid,
          ulsch->rnti,
          nrLDPC_TB_decoding_parameters.A,
          nrLDPC_TB_decoding_parameters.G,
          nrLDPC_TB_decoding_parameters.mcs,
          nrLDPC_TB_decoding_parameters.nb_layers,
          nrLDPC_TB_decoding_parameters.nb_rb,
          nrLDPC_TB_decoding_parameters.Qm,
          pusch_pdu->target_code_rate / 10240.0f,
          pusch_pdu->pusch_data.rv_index,
          harq_process->round,
          harq_process->harq_to_be_cleared);
  
    // [hna] Perform nr_segmenation with input and output set to NULL to calculate only (C, K, Z, F)
    nr_segmentation(NULL,
                    NULL,
                    lenWithCrc(1, nrLDPC_TB_decoding_parameters.A), // size in case of 1 segment
                    &nrLDPC_TB_decoding_parameters.C,
                    &nrLDPC_TB_decoding_parameters.K,
                    &nrLDPC_TB_decoding_parameters.Z, // [hna] Z is Zc
                    &nrLDPC_TB_decoding_parameters.F,
                    nrLDPC_TB_decoding_parameters.BG);
    harq_process->C = nrLDPC_TB_decoding_parameters.C;
    harq_process->K = nrLDPC_TB_decoding_parameters.K;
    harq_process->Z = nrLDPC_TB_decoding_parameters.Z;
    harq_process->F = nrLDPC_TB_decoding_parameters.F;
  
    uint16_t a_segments = MAX_NUM_NR_ULSCH_SEGMENTS_PER_LAYER * nrLDPC_TB_decoding_parameters.nb_layers; // number of segments to be allocated
    if (nrLDPC_TB_decoding_parameters.C > a_segments) {
      LOG_E(PHY, "nr_segmentation.c: too many segments %d, A %d\n", harq_process->C, nrLDPC_TB_decoding_parameters.A);
      return(-1);
    }
    if (nrLDPC_TB_decoding_parameters.nb_rb != 273) {
      a_segments = a_segments*nrLDPC_TB_decoding_parameters.nb_rb;
      a_segments = a_segments/273 +1;
    }
    if (nrLDPC_TB_decoding_parameters.C > a_segments) {
      LOG_E(PHY,"Illegal harq_process->C %d > %d\n",harq_process->C,a_segments);
      return -1;
    }
    max_num_segments = max(max_num_segments, nrLDPC_TB_decoding_parameters.C);
  
#ifdef DEBUG_ULSCH_DECODING
    printf("ulsch decoding nr segmentation Z %d\n", nrLDPC_TB_decoding_parameters.Z);
    if (!frame % 100)
      printf("K %d C %d Z %d \n",
             nrLDPC_TB_decoding_parameters.K,
             nrLDPC_TB_decoding_parameters.C,
             nrLDPC_TB_decoding_parameters.Z);
    printf("Segmentation: C %d, K %d\n",
           nrLDPC_TB_decoding_parameters.C,
           nrLDPC_TB_decoding_parameters.K);
#endif
  
    nrLDPC_TB_decoding_parameters.rnti = ulsch->rnti;
    nrLDPC_TB_decoding_parameters.max_ldpc_iterations = ulsch->max_ldpc_iterations;
    nrLDPC_TB_decoding_parameters.rv_index = pusch_pdu->pusch_data.rv_index;
    nrLDPC_TB_decoding_parameters.tbslbrm = pusch_pdu->maintenance_parms_v3.tbSizeLbrmBytes;
    nrLDPC_TB_decoding_parameters.abort_decode = &harq_process->abort_decode;
    set_abort(&harq_process->abort_decode, false);

    TBs[pusch_id] = nrLDPC_TB_decoding_parameters;
  }

  nrLDPC_segment_decoding_parameters_t segments[nb_pusch][max_num_segments];

  for (uint8_t pusch_id = 0; pusch_id < nb_pusch; pusch_id++) {
    uint8_t ULSCH_id = ULSCH_ids[pusch_id];
    NR_gNB_ULSCH_t *ulsch = &phy_vars_gNB->ulsch[ULSCH_id];
    NR_UL_gNB_HARQ_t *harq_process = ulsch->harq_process;
    short *ulsch_llr = phy_vars_gNB->pusch_vars[ULSCH_id].llr;

    nrLDPC_TB_decoding_parameters_t nrLDPC_TB_decoding_parameters = TBs[pusch_id];
    nrLDPC_TB_decoding_parameters.segments = segments[pusch_id];

    uint32_t r_offset = 0;
    for (int r = 0; r < nrLDPC_TB_decoding_parameters.C; r++) {
      nrLDPC_segment_decoding_parameters_t nrLDPC_segment_decoding_parameters;
      nrLDPC_segment_decoding_parameters.E = nr_get_E(nrLDPC_TB_decoding_parameters.G,
                                                      nrLDPC_TB_decoding_parameters.C,
                                                      nrLDPC_TB_decoding_parameters.Qm,
                                                      nrLDPC_TB_decoding_parameters.nb_layers,
                                                      r);
      nrLDPC_segment_decoding_parameters.R = nr_get_R_ldpc_decoder(nrLDPC_TB_decoding_parameters.rv_index,
                                                                   nrLDPC_segment_decoding_parameters.E,
                                                                   nrLDPC_TB_decoding_parameters.BG,
                                                                   nrLDPC_TB_decoding_parameters.Z,
                                                                   &harq_process->llrLen,
                                                                   harq_process->round);
      nrLDPC_segment_decoding_parameters.llr = ulsch_llr + r_offset;
      nrLDPC_segment_decoding_parameters.d = harq_process->d[r];
      nrLDPC_segment_decoding_parameters.d_to_be_cleared = &harq_process->d_to_be_cleared[r];
      nrLDPC_segment_decoding_parameters.c = harq_process->c[r];
      nrLDPC_segment_decoding_parameters.decodeSuccess = false;

      nrLDPC_TB_decoding_parameters.segments[r] = nrLDPC_segment_decoding_parameters;
      r_offset += nrLDPC_segment_decoding_parameters.E;
    }
    if (harq_process->harq_to_be_cleared) {
      for (int r = 0; r < nrLDPC_TB_decoding_parameters.C; r++) {
        harq_process->d_to_be_cleared[r] = true;
      }
      harq_process->harq_to_be_cleared = false;
    }

    TBs[pusch_id] = nrLDPC_TB_decoding_parameters;
  }

  int number_tasks_decode = nrLDPC_coding_interface.nrLDPC_coding_decoder(&nrLDPC_slot_decoding_parameters);

  // Execute thread pool tasks if any
  while (number_tasks_decode > 0) {
    notifiedFIFO_elt_t *req = pullTpool(&phy_vars_gNB->respDecode, &phy_vars_gNB->threadPool);
    if (req == NULL)
      return -1; // Tpool has been stopped
    delNotifiedFIFO_elt(req);
    number_tasks_decode--;
  }

  // post decode
  for (uint8_t pusch_id = 0; pusch_id < nb_pusch; pusch_id++) {
    uint8_t ULSCH_id = ULSCH_ids[pusch_id];
    NR_gNB_ULSCH_t *ulsch = &phy_vars_gNB->ulsch[ULSCH_id];
    NR_UL_gNB_HARQ_t *harq_process = ulsch->harq_process;

    nrLDPC_TB_decoding_parameters_t nrLDPC_TB_decoding_parameters = TBs[pusch_id];

    uint32_t offset = 0;
    for (int r = 0; r < nrLDPC_TB_decoding_parameters.C; r++) {
      nrLDPC_segment_decoding_parameters_t nrLDPC_segment_decoding_parameters = nrLDPC_TB_decoding_parameters.segments[r];
      // Copy c to b in case of decoding success
      if (nrLDPC_segment_decoding_parameters.decodeSuccess) {
        memcpy(harq_process->b + offset, harq_process->c[r], (harq_process->K >> 3) - (harq_process->F >> 3) - ((harq_process->C > 1) ? 3 : 0));
      } else {
        LOG_D(PHY, "uplink segment error %d/%d\n", r, harq_process->C);
        LOG_D(PHY, "ULSCH %d in error\n", ULSCH_id);
      }
      offset += ((harq_process->K >> 3) - (harq_process->F >> 3) - ((harq_process->C > 1) ? 3 : 0));
    }
  }

  return 0;
}
