/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
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

/*! \file PHY/NR_UE_TRANSPORT/nr_ulsch_coding_slot.c
 */

#include "PHY/defs_UE.h"
#include "PHY/NR_UE_TRANSPORT/nr_transport_proto_ue.h"
#include "PHY/CODING/coding_defs.h"
#include "PHY/CODING/coding_extern.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "PHY/CODING/nrLDPC_extern.h"
#include "PHY/CODING/nrLDPC_coding_interface.h"
#include "PHY/NR_UE_TRANSPORT/nr_transport_ue.h"
#include "executables/nr-uesoftmodem.h"
#include "common/utils/LOG/vcd_signal_dumper.h"

int nr_ulsch_encoding_slot(PHY_VARS_NR_UE *ue,
                           NR_UE_ULSCH_t *ulsch,
                           const uint32_t frame,
                           const uint8_t slot,
                           int nb_harq,
                           uint8_t *harq_pids,
                           unsigned int *G)
{
  start_meas(&ue->ulsch_encoding_stats);
  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_NR_UE_ULSCH_ENCODING, VCD_FUNCTION_IN);

  notifiedFIFO_t nf;
  initNotifiedFIFO(&nf);

  nrLDPC_slot_encoding_parameters_t slot_encoding_params;
  slot_encoding_params.frame = frame;
  slot_encoding_params.slot = slot;
  slot_encoding_params.nb_TBs = nb_harq;
  slot_encoding_params.respEncode = &nf;
  slot_encoding_params.threadPool = &get_nrUE_params()->Tpool;
  slot_encoding_params.tinput = NULL;
  slot_encoding_params.tprep = NULL;
  slot_encoding_params.tparity = NULL;
  slot_encoding_params.toutput = NULL;
  nrLDPC_TB_encoding_parameters_t TBs[nb_harq];
  slot_encoding_params.TBs = TBs;

  int max_num_segments = 0;

  for (uint8_t idx = 0; idx < nb_harq; idx++) {
    uint8_t harq_pid = harq_pids[idx];
    nrLDPC_TB_encoding_parameters_t *TB_encoding_params = &TBs[idx];

    /////////////////////////parameters and variables initialization/////////////////////////

    unsigned int crc = 1;
    NR_UL_UE_HARQ_t *harq_process = &ue->ul_harq_processes[harq_pid];
    const nfapi_nr_ue_pusch_pdu_t *pusch_pdu = &ulsch->pusch_pdu;
    uint16_t nb_rb = pusch_pdu->rb_size;
    uint32_t A = pusch_pdu->pusch_data.tb_size << 3;
    uint8_t Qm = pusch_pdu->qam_mod_order;
    // target_code_rate is in 0.1 units
    float Coderate = (float)pusch_pdu->target_code_rate / 10240.0f;

    LOG_D(NR_PHY, "ulsch coding nb_rb %d, Nl = %d\n", nb_rb, pusch_pdu->nrOfLayers);
    LOG_D(NR_PHY, "ulsch coding A %d G %d mod_order %d Coderate %f\n", A, G[idx], Qm, Coderate);
    LOG_D(NR_PHY, "harq_pid %d, pusch_data.new_data_indicator %d\n", harq_pid, pusch_pdu->pusch_data.new_data_indicator);

    ///////////////////////// a---->| add CRC |---->b /////////////////////////

    int max_payload_bytes = MAX_NUM_NR_ULSCH_SEGMENTS_PER_LAYER * pusch_pdu->nrOfLayers * 1056;
    int B;
    if (A > NR_MAX_PDSCH_TBS) {
      // Add 24-bit crc (polynomial A) to payload
      crc = crc24a(harq_process->payload_AB, A) >> 8;
      harq_process->payload_AB[A >> 3] = ((uint8_t *)&crc)[2];
      harq_process->payload_AB[1 + (A >> 3)] = ((uint8_t *)&crc)[1];
      harq_process->payload_AB[2 + (A >> 3)] = ((uint8_t *)&crc)[0];
      B = A + 24;
      AssertFatal((A / 8) + 4 <= max_payload_bytes, "A %d is too big (A/8+4 = %d > %d)\n", A, (A / 8) + 4, max_payload_bytes);
    } else {
      // Add 16-bit crc (polynomial A) to payload
      crc = crc16(harq_process->payload_AB, A) >> 16;
      harq_process->payload_AB[A >> 3] = ((uint8_t *)&crc)[1];
      harq_process->payload_AB[1 + (A >> 3)] = ((uint8_t *)&crc)[0];
      B = A + 16;
      AssertFatal((A / 8) + 3 <= max_payload_bytes, "A %d is too big (A/8+3 = %d > %d)\n", A, (A / 8) + 3, max_payload_bytes);
    }

    ///////////////////////// b---->| block segmentation |---->c /////////////////////////

    harq_process->BG = pusch_pdu->ldpcBaseGraph;

    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_NR_SEGMENTATION, VCD_FUNCTION_IN);
    start_meas(&ue->ulsch_segmentation_stats);
    TB_encoding_params->Kb = nr_segmentation(harq_process->payload_AB,
                                             harq_process->c,
                                             B,
                                             &harq_process->C,
                                             &harq_process->K,
                                             &harq_process->Z,
                                             &harq_process->F,
                                             harq_process->BG);
    TB_encoding_params->C = harq_process->C;
    TB_encoding_params->K = harq_process->K;
    TB_encoding_params->Z = harq_process->Z;
    TB_encoding_params->F = harq_process->F;
    TB_encoding_params->BG = harq_process->BG;
    if (TB_encoding_params->C > MAX_NUM_NR_DLSCH_SEGMENTS_PER_LAYER * pusch_pdu->nrOfLayers) {
      LOG_E(PHY, "nr_segmentation.c: too many segments %d, B %d\n", TB_encoding_params->C, B);
      return (-1);
    }
    stop_meas(&ue->ulsch_segmentation_stats);
    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_NR_SEGMENTATION, VCD_FUNCTION_OUT);
    max_num_segments = max(max_num_segments, TB_encoding_params->C);

    TB_encoding_params->rnti = pusch_pdu->rnti;
    TB_encoding_params->nb_rb = nb_rb;
    TB_encoding_params->Qm = Qm;
    TB_encoding_params->mcs = pusch_pdu->mcs_index;
    TB_encoding_params->nb_layers = pusch_pdu->nrOfLayers;
    TB_encoding_params->rv_index = pusch_pdu->pusch_data.rv_index;
    TB_encoding_params->G = G[idx];
    TB_encoding_params->tbslbrm = pusch_pdu->tbslbrm;
    TB_encoding_params->A = A;
  } // idx (harq_pid)

  nrLDPC_segment_encoding_parameters_t segments[nb_harq][max_num_segments];

  for (uint8_t idx = 0; idx < nb_harq; idx++) {
    uint8_t harq_pid = harq_pids[idx];
    nrLDPC_TB_encoding_parameters_t *TB_encoding_params = &TBs[idx];
    NR_UL_UE_HARQ_t *harq_process = &ue->ul_harq_processes[harq_pid];
    TB_encoding_params->segments = segments[idx];

    int r_offset = 0;
    for (int r = 0; r < TB_encoding_params->C; r++) {
      nrLDPC_segment_encoding_parameters_t *segment_encoding_params = &TB_encoding_params->segments[r];
      segment_encoding_params->c = harq_process->c[r];
      segment_encoding_params->E =
          nr_get_E(TB_encoding_params->G, TB_encoding_params->C, TB_encoding_params->Qm, TB_encoding_params->nb_layers, r);
      segment_encoding_params->output = harq_process->f + r_offset;
      r_offset += segment_encoding_params->E;
    } // TB_encoding_params->C
  } // idx (harq_pid)

  ///////////////////////// | LDCP coding | ////////////////////////////////////

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_LDPC_ENCODER_OPTIM, VCD_FUNCTION_IN);

  int nbJobs = 0;
  nbJobs = nrLDPC_coding_interface.nrLDPC_coding_encoder(&slot_encoding_params);

  if (nbJobs < 0)
    return -1;
  while (nbJobs) {
    notifiedFIFO_elt_t *req = pullTpool(&nf, &get_nrUE_params()->Tpool);
    if (req == NULL)
      break; // Tpool has been stopped
    delNotifiedFIFO_elt(req);
    nbJobs--;
  }

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_LDPC_ENCODER_OPTIM, VCD_FUNCTION_OUT);

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_NR_UE_ULSCH_ENCODING, VCD_FUNCTION_OUT);
  stop_meas(&ue->ulsch_encoding_stats);
  return 0;
}
