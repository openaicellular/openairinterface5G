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

/*! \file PHY/NR_UE_TRANSPORT/nr_dlsch_decoding_slot.c
 */

#include "common/utils/LOG/vcd_signal_dumper.h"
#include "PHY/defs_nr_UE.h"
#include "SCHED_NR_UE/harq_nr.h"
#include "PHY/phy_extern_nr_ue.h"
#include "PHY/CODING/coding_extern.h"
#include "PHY/CODING/coding_defs.h"
#include "PHY/CODING/nrLDPC_coding_interface.h"
#include "PHY/NR_UE_TRANSPORT/nr_transport_proto_ue.h"
#include "SCHED_NR_UE/defs.h"
#include "SIMULATION/TOOLS/sim.h"
#include "executables/nr-uesoftmodem.h"
#include "PHY/CODING/nrLDPC_extern.h"
#include "common/utils/nr/nr_common.h"
#include "openair1/PHY/TOOLS/phy_scope_interface.h"
#include "nfapi/open-nFAPI/nfapi/public_inc/nfapi_nr_interface.h"

static extended_kpi_ue kpiStructure = {0};

/*! \brief Prepare necessary parameters for nrLDPC_coding_interface
 */
uint32_t nr_dlsch_decoding_slot(PHY_VARS_NR_UE *phy_vars_ue,
                                const UE_nr_rxtx_proc_t *proc,
                                NR_UE_DLSCH_t *dlsch,
                                int16_t **dlsch_llr,
                                uint8_t **b,
                                int *G,
                                int nb_dlsch,
                                uint8_t *DLSCH_ids)
{
  notifiedFIFO_t nf;
  initNotifiedFIFO(&nf);

  nrLDPC_slot_decoding_parameters_t slot_decoding_params;
  slot_decoding_params.frame = proc->frame_rx;
  slot_decoding_params.slot = proc->nr_slot_rx;
  slot_decoding_params.nb_TBs = nb_dlsch;
  slot_decoding_params.threadPool = &get_nrUE_params()->Tpool;
  slot_decoding_params.respDecode = &nf;
  nrLDPC_TB_decoding_parameters_t TBs[nb_dlsch];
  slot_decoding_params.TBs = TBs;

  int max_num_segments = 0;

  for (uint8_t pdsch_id = 0; pdsch_id < nb_dlsch; pdsch_id++) {
    uint8_t DLSCH_id = DLSCH_ids[pdsch_id];
    fapi_nr_dl_config_dlsch_pdu_rel15_t *dlsch_config = &dlsch[DLSCH_id].dlsch_config;
    int harq_pid = dlsch_config->harq_process_nbr;
    NR_DL_UE_HARQ_t *harq_process = &phy_vars_ue->dl_harq_processes[DLSCH_id][harq_pid];
    uint8_t dmrs_Type = dlsch_config->dmrsConfigType;
    AssertFatal(dmrs_Type == 0 || dmrs_Type == 1, "Illegal dmrs_type %d\n", dmrs_Type);
    uint8_t nb_re_dmrs;

    phy_vars_ue->dl_stats[harq_process->DLround]++;
    LOG_D(PHY, "Round %d RV idx %d\n", harq_process->DLround, dlsch->dlsch_config.rv);

    if (dmrs_Type == NFAPI_NR_DMRS_TYPE1)
      nb_re_dmrs = 6 * dlsch_config->n_dmrs_cdm_groups;
    else
      nb_re_dmrs = 4 * dlsch_config->n_dmrs_cdm_groups;
    uint16_t dmrs_length = get_num_dmrs(dlsch_config->dlDmrsSymbPos);

    if (!harq_process) {
      LOG_E(PHY, "dlsch_decoding_slot.c: NULL harq_process pointer\n");
      return dlsch[DLSCH_id].max_ldpc_iterations + 1;
    }

    nrLDPC_TB_decoding_parameters_t *TB_decoding_params = &TBs[pdsch_id];

    // ------------------------------------------------------------------
    TB_decoding_params->G = G[DLSCH_id];
    TB_decoding_params->nb_rb = dlsch_config->number_rbs;
    TB_decoding_params->Qm = dlsch_config->qamModOrder;
    TB_decoding_params->mcs = dlsch_config->mcs;
    TB_decoding_params->nb_layers = dlsch[DLSCH_id].Nl;
    TB_decoding_params->BG = dlsch_config->ldpcBaseGraph;
    TB_decoding_params->A = dlsch_config->TBS;
    // ------------------------------------------------------------------

    float Coderate = (float)dlsch->dlsch_config.targetCodeRate / 10240.0f;

    LOG_D(
        PHY,
        "%d.%d DLSCH %d Decoding, harq_pid %d TBS %d G %d nb_re_dmrs %d length dmrs %d mcs %d Nl %d nb_symb_sch %d nb_rb %d Qm %d "
        "Coderate %f\n",
        slot_decoding_params.frame,
        slot_decoding_params.slot,
        DLSCH_id,
        harq_pid,
        dlsch_config->TBS,
        TB_decoding_params->G,
        nb_re_dmrs,
        dmrs_length,
        TB_decoding_params->mcs,
        TB_decoding_params->nb_layers,
        dlsch_config->number_symbols,
        TB_decoding_params->nb_rb,
        TB_decoding_params->Qm,
        Coderate);

    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_SEGMENTATION, VCD_FUNCTION_IN);

    if (harq_process->first_rx == 1) {
      // This is a new packet, so compute quantities regarding segmentation
      nr_segmentation(NULL,
                      NULL,
                      lenWithCrc(1, TB_decoding_params->A), // We give a max size in case of 1 segment
                      &TB_decoding_params->C,
                      &TB_decoding_params->K,
                      &TB_decoding_params->Z, // [hna] Z is Zc
                      &TB_decoding_params->F,
                      TB_decoding_params->BG);
      harq_process->C = TB_decoding_params->C;
      harq_process->K = TB_decoding_params->K;
      harq_process->Z = TB_decoding_params->Z;
      harq_process->F = TB_decoding_params->F;

      if (harq_process->C > MAX_NUM_NR_DLSCH_SEGMENTS_PER_LAYER * TB_decoding_params->nb_layers) {
        LOG_E(PHY, "nr_segmentation.c: too many segments %d, A %d\n", harq_process->C, TB_decoding_params->A);
        return dlsch[DLSCH_id].max_ldpc_iterations + 1;
      }

      if (LOG_DEBUGFLAG(DEBUG_DLSCH_DECOD) && (!slot_decoding_params.frame % 100))
        LOG_I(PHY, "K %d C %d Z %d nl %d \n", harq_process->K, harq_process->C, harq_process->Z, TB_decoding_params->nb_layers);
      // clear HARQ buffer
      for (int i = 0; i < harq_process->C; i++)
        memset(harq_process->d[i], 0, 5 * 8448 * sizeof(int16_t));
    } else {
      // This is not a new packet, so retrieve previously computed quantities regarding segmentation
      TB_decoding_params->C = harq_process->C;
      TB_decoding_params->K = harq_process->K;
      TB_decoding_params->Z = harq_process->Z;
      TB_decoding_params->F = harq_process->F;
    }
    max_num_segments = max(max_num_segments, TB_decoding_params->C);

    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_SEGMENTATION, VCD_FUNCTION_OUT);

    if (LOG_DEBUGFLAG(DEBUG_DLSCH_DECOD))
      LOG_I(PHY, "Segmentation: C %d, K %d\n", harq_process->C, harq_process->K);

    TB_decoding_params->rnti = dlsch[DLSCH_id].rnti;
    TB_decoding_params->max_ldpc_iterations = dlsch[DLSCH_id].max_ldpc_iterations;
    TB_decoding_params->rv_index = dlsch_config->rv;
    TB_decoding_params->tbslbrm = dlsch_config->tbslbrm;
    TB_decoding_params->abort_decode = &harq_process->abort_decode;
    set_abort(&harq_process->abort_decode, false);
  }

  nrLDPC_segment_decoding_parameters_t segments[nb_dlsch][max_num_segments];
  bool d_to_be_cleared[nb_dlsch][max_num_segments];

  for (uint8_t pdsch_id = 0; pdsch_id < nb_dlsch; pdsch_id++) {
    uint8_t DLSCH_id = DLSCH_ids[pdsch_id];
    fapi_nr_dl_config_dlsch_pdu_rel15_t *dlsch_config = &dlsch[DLSCH_id].dlsch_config;
    int harq_pid = dlsch_config->harq_process_nbr;
    NR_DL_UE_HARQ_t *harq_process = &phy_vars_ue->dl_harq_processes[DLSCH_id][harq_pid];
    nrLDPC_TB_decoding_parameters_t *TB_decoding_params = &TBs[pdsch_id];
    TB_decoding_params->segments = segments[pdsch_id];

    uint32_t r_offset = 0;
    for (int r = 0; r < TB_decoding_params->C; r++) {
      if (harq_process->first_rx == 1)
        d_to_be_cleared[pdsch_id][r] = true;
      else
        d_to_be_cleared[pdsch_id][r] = false;
      nrLDPC_segment_decoding_parameters_t *segment_decoding_params = &TB_decoding_params->segments[r];
      segment_decoding_params->E =
          nr_get_E(TB_decoding_params->G, TB_decoding_params->C, TB_decoding_params->Qm, TB_decoding_params->nb_layers, r);
      segment_decoding_params->R = nr_get_R_ldpc_decoder(TB_decoding_params->rv_index,
                                                         segment_decoding_params->E,
                                                         TB_decoding_params->BG,
                                                         TB_decoding_params->Z,
                                                         &harq_process->llrLen,
                                                         harq_process->DLround);
      segment_decoding_params->llr = dlsch_llr[DLSCH_id] + r_offset;
      segment_decoding_params->d = harq_process->d[r];
      segment_decoding_params->d_to_be_cleared = &d_to_be_cleared[pdsch_id][r];
      segment_decoding_params->c = harq_process->c[r];
      segment_decoding_params->decodeSuccess = false;

      r_offset += segment_decoding_params->E;
    }
  }

  int number_tasks_decode = nrLDPC_coding_interface.nrLDPC_coding_decoder(&slot_decoding_params);

  // Execute thread pool tasks if any
  while (number_tasks_decode > 0) {
    notifiedFIFO_elt_t *req = pullTpool(&nf, &get_nrUE_params()->Tpool);
    if (req == NULL)
      return dlsch[0].last_iteration_cnt; // Tpool has been stopped
    delNotifiedFIFO_elt(req);
    number_tasks_decode--;
  }

  // post decode
  for (uint8_t pdsch_id = 0; pdsch_id < nb_dlsch; pdsch_id++) {
    int num_seg_ok = 0;
    uint8_t DLSCH_id = DLSCH_ids[pdsch_id];
    fapi_nr_dl_config_dlsch_pdu_rel15_t *dlsch_config = &dlsch[DLSCH_id].dlsch_config;
    int harq_pid = dlsch_config->harq_process_nbr;
    NR_DL_UE_HARQ_t *harq_process = &phy_vars_ue->dl_harq_processes[DLSCH_id][harq_pid];

    nrLDPC_TB_decoding_parameters_t *TB_decoding_params = &TBs[pdsch_id];

    uint32_t offset = 0;
    for (int r = 0; r < TB_decoding_params->C; r++) {
      nrLDPC_segment_decoding_parameters_t *segment_decoding_params = &TB_decoding_params->segments[r];
      if (segment_decoding_params->decodeSuccess) {
        memcpy(b[DLSCH_id] + offset,
               harq_process->c[r],
               (harq_process->K >> 3) - (harq_process->F >> 3) - ((harq_process->C > 1) ? 3 : 0));
        num_seg_ok++;
      } else {
        LOG_D(PHY, "downlink segment error %d/%d\n", r, harq_process->C);
        LOG_D(PHY, "DLSCH %d in error\n", DLSCH_id);
      }
      offset += (harq_process->K >> 3) - (harq_process->F >> 3) - ((harq_process->C > 1) ? 3 : 0);
    }

    kpiStructure.nb_total++;
    kpiStructure.blockSize = dlsch_config->TBS;
    kpiStructure.dl_mcs = dlsch_config->mcs;
    kpiStructure.nofRBs = dlsch_config->number_rbs;

    if (num_seg_ok == harq_process->C) {
      if (harq_process->C > 1) {
        int A = dlsch_config->TBS;
        /* check global CRC */
        // we have regrouped the transport block, so it is "1" segment
        if (!check_crc(b[DLSCH_id], lenWithCrc(1, A), crcType(1, A))) {
          harq_process->ack = 0;
          dlsch[DLSCH_id].last_iteration_cnt = dlsch[DLSCH_id].max_ldpc_iterations + 1;
          LOG_E(PHY,
                "Frame %d.%d LDPC global CRC fails, but individual LDPC CRC succeeded. %d segs\n",
                proc->frame_rx,
                proc->nr_slot_rx,
                harq_process->C);
          LOG_D(PHY, "DLSCH %d received nok\n", DLSCH_id);
        }
        // We search only a reccuring OAI error that propagates all 0 packets with a 0 CRC, so we do the check only if the 2 first
        // bytes of the CRC are 0 (it can be CRC16 or CRC24)
        const int sz = A / 8;
        if (b[DLSCH_id][sz] == 0 && b[DLSCH_id][sz + 1] == 0) {
          int i = 0;
          while (b[DLSCH_id][i] == 0 && i < sz)
            i++;
          if (i == sz) {
            LOG_E(PHY,
                  "received all 0 pdu, consider it false reception, even if the TS 38.212 7.2.1 says only we should attach the "
                  "corresponding CRC, and nothing prevents to have a all 0 packet\n");
            dlsch[DLSCH_id].last_iteration_cnt = dlsch[DLSCH_id].max_ldpc_iterations + 1;
          }
        }
      }

      harq_process->status = SCH_IDLE;
      harq_process->ack = 1;

      // Same as gNB, set to max_ldpc_iterations is sufficient given that this variable is only used for checking for failure
      dlsch[DLSCH_id].last_iteration_cnt = dlsch[DLSCH_id].max_ldpc_iterations;
      LOG_D(PHY, "DLSCH %d received ok\n", DLSCH_id);
    } else {
      kpiStructure.nb_nack++;

      harq_process->ack = 0;

      // Same as gNB, set to max_ldpc_iterations + 1 is sufficient given that this variable is only used for checking for failure
      dlsch[DLSCH_id].last_iteration_cnt = dlsch[DLSCH_id].max_ldpc_iterations + 1;
      LOG_D(PHY, "DLSCH %d received nok\n", DLSCH_id);
    }
  }

  return dlsch[0].last_iteration_cnt;
}
