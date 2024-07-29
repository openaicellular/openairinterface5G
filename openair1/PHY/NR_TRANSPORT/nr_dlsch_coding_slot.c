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

/*! \file PHY/LTE_TRANSPORT/dlsch_coding.c
* \brief Top-level routines for implementing LDPC-coded (DLSCH) transport channels from 38-212, 15.2
* \author H.Wang
* \date 2018
* \version 0.1
* \company Eurecom
* \email:
* \note
* \warning
*/

#include "PHY/defs_gNB.h"
#include "PHY/CODING/coding_extern.h"
#include "PHY/CODING/coding_defs.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "PHY/CODING/nrLDPC_coding_interface.h"
#include "PHY/CODING/nrLDPC_extern.h"
#include "PHY/NR_TRANSPORT/nr_transport_proto.h"
#include "PHY/NR_TRANSPORT/nr_transport_common_proto.h"
#include "PHY/NR_TRANSPORT/nr_dlsch.h"
#include "SCHED_NR/sched_nr.h"
#include "common/utils/LOG/vcd_signal_dumper.h"
#include "common/utils/LOG/log.h"
#include "common/utils/nr/nr_common.h"
#include <syscall.h>
#include <openair2/UTIL/OPT/opt.h>

//#define DEBUG_DLSCH_CODING
//#define DEBUG_DLSCH_FREE 1

int nr_dlsch_encoding_slot(PHY_VARS_gNB *gNB,
                           processingData_L1tx_t *msgTx,
                           int frame,
                           uint8_t slot,
                           NR_DL_FRAME_PARMS *frame_parms,
                           unsigned char **output,
                           time_stats_t *tinput,
                           time_stats_t *tprep,
                           time_stats_t *tparity,
                           time_stats_t *toutput,
                           time_stats_t *dlsch_rate_matching_stats,
                           time_stats_t *dlsch_interleaving_stats,
                           time_stats_t *dlsch_segmentation_stats)
{

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_gNB_DLSCH_ENCODING, VCD_FUNCTION_IN);

  nrLDPC_slot_encoding_parameters_t nrLDPC_slot_encoding_parameters;
  nrLDPC_slot_encoding_parameters.frame = frame;
  nrLDPC_slot_encoding_parameters.slot = slot;
  nrLDPC_slot_encoding_parameters.nb_TBs = msgTx->num_pdsch_slot;
  //nrLDPC_slot_encoding_parameters.respEncode = &gNB->respEncode;
  nrLDPC_slot_encoding_parameters.threadPool = &gNB->threadPool;
  nrLDPC_slot_encoding_parameters.tinput = tinput;
  nrLDPC_slot_encoding_parameters.tprep = tprep;
  nrLDPC_slot_encoding_parameters.tparity = tparity;
  nrLDPC_slot_encoding_parameters.toutput = toutput;
  nrLDPC_TB_encoding_parameters_t TBs[msgTx->num_pdsch_slot];
  nrLDPC_slot_encoding_parameters.TBs = TBs;

  int max_num_segments = 0;

  for (int dlsch_id=0; dlsch_id<msgTx->num_pdsch_slot; dlsch_id++) {

    NR_gNB_DLSCH_t *dlsch = msgTx->dlsch[dlsch_id];

    NR_DL_gNB_HARQ_t *harq = &dlsch->harq_process;
    unsigned int crc=1;
    nfapi_nr_dl_tti_pdsch_pdu_rel15_t *rel15 = &harq->pdsch_pdu.pdsch_pdu_rel15;
    uint32_t A = rel15->TBSize[0]<<3;
    unsigned char *a=harq->pdu;
    if (rel15->rnti != SI_RNTI)
      trace_NRpdu(DIRECTION_DOWNLINK, a, rel15->TBSize[0], WS_C_RNTI, rel15->rnti, frame, slot,0, 0);

    NR_gNB_PHY_STATS_t *phy_stats = NULL;
    if (rel15->rnti != 0xFFFF)
      phy_stats = get_phy_stats(gNB, rel15->rnti);

    if (phy_stats) {
      phy_stats->frame = frame;
      phy_stats->dlsch_stats.total_bytes_tx += rel15->TBSize[0];
      phy_stats->dlsch_stats.current_RI = rel15->nrOfLayers;
      phy_stats->dlsch_stats.current_Qm = rel15->qamModOrder[0];
    }

    int max_bytes = MAX_NUM_NR_DLSCH_SEGMENTS_PER_LAYER*rel15->nrOfLayers*1056;
    int B;
    if (A > NR_MAX_PDSCH_TBS) {
      // Add 24-bit crc (polynomial A) to payload
      crc = crc24a(a,A)>>8;
      a[A>>3] = ((uint8_t *)&crc)[2];
      a[1+(A>>3)] = ((uint8_t *)&crc)[1];
      a[2+(A>>3)] = ((uint8_t *)&crc)[0];
      //printf("CRC %x (A %d)\n",crc,A);
      //printf("a0 %d a1 %d a2 %d\n", a[A>>3], a[1+(A>>3)], a[2+(A>>3)]);
      B = A + 24;
      //    harq->b = a;
      AssertFatal((A / 8) + 4 <= max_bytes,
                  "A %d is too big (A/8+4 = %d > %d)\n",
                  A,
                  (A / 8) + 4,
                  max_bytes);
      memcpy(harq->b, a, (A / 8) + 4); // why is this +4 if the CRC is only 3 bytes?
    } else {
      // Add 16-bit crc (polynomial A) to payload
      crc = crc16(a,A)>>16;
      a[A>>3] = ((uint8_t *)&crc)[1];
      a[1+(A>>3)] = ((uint8_t *)&crc)[0];
      //printf("CRC %x (A %d)\n",crc,A);
      //printf("a0 %d a1 %d \n", a[A>>3], a[1+(A>>3)]);
      B = A + 16;
      //    harq->b = a;
      AssertFatal((A / 8) + 3 <= max_bytes,
                  "A %d is too big (A/8+3 = %d > %d)\n",
                  A,
                  (A / 8) + 3,
                  max_bytes);
      memcpy(harq->b, a, (A / 8) + 3); // using 3 bytes to mimic the case of 24 bit crc
    }

    nrLDPC_TB_encoding_parameters_t nrLDPC_TB_encoding_parameters;

    nrLDPC_TB_encoding_parameters.BG = rel15->maintenance_parms_v3.ldpcBaseGraph;
    nrLDPC_TB_encoding_parameters.Z = harq->Z;
    nrLDPC_TB_encoding_parameters.A = A;
    start_meas(dlsch_segmentation_stats);
    nrLDPC_TB_encoding_parameters.Kb = nr_segmentation(harq->b,
                                                       harq->c,
                                                       B,
                                                       &nrLDPC_TB_encoding_parameters.C,
                                                       &nrLDPC_TB_encoding_parameters.K,
                                                       &nrLDPC_TB_encoding_parameters.Z,
                                                       &nrLDPC_TB_encoding_parameters.F,
                                                       nrLDPC_TB_encoding_parameters.BG);
    stop_meas(dlsch_segmentation_stats);

    if (nrLDPC_TB_encoding_parameters.C>MAX_NUM_NR_DLSCH_SEGMENTS_PER_LAYER*rel15->nrOfLayers) {
      LOG_E(PHY, "nr_segmentation.c: too many segments %d, B %d\n", nrLDPC_TB_encoding_parameters.C, B);
      return(-1);
    }
    max_num_segments = max(max_num_segments, nrLDPC_TB_encoding_parameters.C);

    TBs[dlsch_id] = nrLDPC_TB_encoding_parameters;
  }

  nrLDPC_segment_encoding_parameters_t segments[msgTx->num_pdsch_slot][max_num_segments];

  for (int dlsch_id = 0; dlsch_id < msgTx->num_pdsch_slot; dlsch_id++) {
    NR_gNB_DLSCH_t *dlsch = msgTx->dlsch[dlsch_id];
    NR_DL_gNB_HARQ_t *harq = &dlsch->harq_process;
    nfapi_nr_dl_tti_pdsch_pdu_rel15_t *rel15 = &harq->pdsch_pdu.pdsch_pdu_rel15;

    nrLDPC_TB_encoding_parameters_t nrLDPC_TB_encoding_parameters = TBs[dlsch_id];
    nrLDPC_TB_encoding_parameters.segments = segments[dlsch_id];

    for (int r = 0; r < nrLDPC_TB_encoding_parameters.C; r++) {
      nrLDPC_TB_encoding_parameters.segments[r].c = harq->c[r];
#ifdef DEBUG_DLSCH_CODING
      LOG_D(PHY, "Encoder: B %d F %d \n", harq->B, nrLDPC_TB_encoding_parameters.F);
      LOG_D(PHY, "start ldpc encoder segment %d/%d\n", r, nrLDPC_TB_encoding_parameters.C);
      LOG_D(PHY, "input %d %d %d %d %d \n", harq->c[r][0], harq->c[r][1], harq->c[r][2], harq->c[r][3], harq->c[r][4]);
      for (int cnt = 0; cnt < 22 * (nrLDPC_TB_encoding_parameters.Z) / 8; cnt++) {
        LOG_D(PHY, "%d ", harq->c[r][cnt]);
      }
      LOG_D(PHY, "\n");
#endif
    }

    nrLDPC_TB_encoding_parameters.rnti = rel15->rnti;
    nrLDPC_TB_encoding_parameters.nb_rb = rel15->rbSize;
    nrLDPC_TB_encoding_parameters.Qm = rel15->qamModOrder[0];
    nrLDPC_TB_encoding_parameters.mcs = rel15->mcsIndex[0];
    nrLDPC_TB_encoding_parameters.nb_layers = rel15->nrOfLayers;
    nrLDPC_TB_encoding_parameters.rv_index = rel15->rvIndex[0];

    int nb_re_dmrs =
        (rel15->dmrsConfigType == NFAPI_NR_DMRS_TYPE1) ? (6 * rel15->numDmrsCdmGrpsNoData) : (4 * rel15->numDmrsCdmGrpsNoData);
    nrLDPC_TB_encoding_parameters.G = nr_get_G(rel15->rbSize,
                                               rel15->NrOfSymbols,
                                               nb_re_dmrs,
                                               get_num_dmrs(rel15->dlDmrsSymbPos),
                                               harq->unav_res,
                                               rel15->qamModOrder[0],
                                               rel15->nrOfLayers);

    nrLDPC_TB_encoding_parameters.tbslbrm = rel15->maintenance_parms_v3.tbSizeLbrmBytes;

    int r_offset = 0;
    for (int r = 0; r < nrLDPC_TB_encoding_parameters.C; r++) {
      nrLDPC_TB_encoding_parameters.segments[r].E = nr_get_E(nrLDPC_TB_encoding_parameters.G,
                                                             nrLDPC_TB_encoding_parameters.C,
                                                             nrLDPC_TB_encoding_parameters.Qm,
                                                             rel15->nrOfLayers,
                                                             r);
      nrLDPC_TB_encoding_parameters.segments[r].output = output[dlsch_id] + r_offset;
      r_offset += nrLDPC_TB_encoding_parameters.segments[r].E;
    }

    TBs[dlsch_id] = nrLDPC_TB_encoding_parameters;
  }
  notifiedFIFO_t nf;
  initNotifiedFIFO(&nf);
  nrLDPC_slot_encoding_parameters.respEncode = &nf;

  int nbJobs = nrLDPC_coding_interface.nrLDPC_coding_encoder(&nrLDPC_slot_encoding_parameters);

  if (nbJobs < 0)
    return -1;
  while (nbJobs) {
    notifiedFIFO_elt_t *req = pullTpool(&nf, &gNB->threadPool);
    if (req == NULL)
      break; // Tpool has been stopped
    delNotifiedFIFO_elt(req);
    nbJobs--;
  }

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_gNB_DLSCH_ENCODING, VCD_FUNCTION_OUT);
  return 0;
}
