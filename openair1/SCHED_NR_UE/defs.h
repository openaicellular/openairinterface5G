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

/*
  \brief NR UE PHY functions prototypes
  \author R. Knopp, F. Kaltenberger
  \company EURECOM
  \email knopp@eurecom.fr
*/

#ifndef __openair_SCHED_H__
#define __openair_SCHED_H__

#include "PHY/defs_nr_UE.h"


/*enum THREAD_INDEX { OPENAIR_THREAD_INDEX = 0,
                    TOP_LEVEL_SCHEDULER_THREAD_INDEX,
                    DLC_SCHED_THREAD_INDEX,
                    openair_SCHED_NB_THREADS
                  };*/ // do not modify this line


#define OPENAIR_THREAD_PRIORITY        255


#define OPENAIR_THREAD_STACK_SIZE     PTHREAD_STACK_MIN //4096 //RTL_PTHREAD_STACK_MIN*6
//#define DLC_THREAD_STACK_SIZE        4096 //DLC stack size
//#define UE_SLOT_PARALLELISATION
//#define UE_DLSCH_PARALLELISATION

/*enum openair_SCHED_STATUS {
  openair_SCHED_STOPPED=1,
  openair_SCHED_STARTING,
  openair_SCHED_STARTED,
  openair_SCHED_STOPPING
};*/

/*enum openair_ERROR {
  // HARDWARE CAUSES
  openair_ERROR_HARDWARE_CLOCK_STOPPED= 1,

  // SCHEDULER CAUSE
  openair_ERROR_OPENAIR_RUNNING_LATE,
  openair_ERROR_OPENAIR_SCHEDULING_FAILED,

  // OTHERS
  openair_ERROR_OPENAIR_TIMING_OFFSET_OUT_OF_BOUNDS,
};*/

/*enum openair_SYNCH_STATUS {
  openair_NOT_SYNCHED=1,
  openair_SYNCHED,
  openair_SCHED_EXIT
};*/

/*enum openair_HARQ_TYPE {
  openair_harq_DL = 0,
  openair_harq_UL,
  openair_harq_RA
};*/

#define DAQ_AGC_ON 1
#define DAQ_AGC_OFF 0


typedef struct {
  uint8_t decoded_output[3]; // PBCH paylod not larger than 3B
  uint8_t xtra_byte;
} fapiPbch_t;

/** @addtogroup _PHY_PROCEDURES_
 * @{
 */

/*! \brief Scheduling for UE TX procedures in normal subframes.
  @param ue Pointer to UE variables on which to act
  @param proc Pointer to RXn-TXnp4 proc information
@param phy_data
*/
void phy_procedures_nrUE_TX(PHY_VARS_NR_UE *ue, const UE_nr_rxtx_proc_t *proc, nr_phy_data_tx_t *phy_data);

void nr_ue_pdsch_procedures_symbol(void *params);

int nr_process_pbch_symbol(PHY_VARS_NR_UE *ue,
                           const UE_nr_rxtx_proc_t *proc,
                           const int symbol,
                           const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
                           const int ssbIndexIn,
                           c16_t dl_ch_estimates_time[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
                           int16_t pbch_e_rx[NR_POLAR_PBCH_E]);

int nr_pbch_decode(PHY_VARS_NR_UE *ue,
                   UE_nr_rxtx_proc_t *proc,
                   const int i_ssb,
                   int16_t pbch_e_rx[NR_POLAR_PBCH_E],
                   fapiPbch_t *result);

void processSlotTX(void *arg);

/*! \brief UE PRACH procedures.
 */
void nr_ue_prach_procedures(PHY_VARS_NR_UE *ue, const UE_nr_rxtx_proc_t *proc);

int8_t nr_find_ue(uint16_t rnti, PHY_VARS_eNB *phy_vars_eNB);

/*! \brief UL time alignment procedures for TA application
  @param ue
  @param slot_tx
  @param frame_tx
*/
void ue_ta_procedures(PHY_VARS_NR_UE *ue, int slot_tx, int frame_tx);

unsigned int nr_get_tx_amp(int power_dBm, int power_max_dBm, int N_RB_UL, int nb_rb);

void set_tx_harq_id(NR_UE_ULSCH_t *ulsch, int harq_pid, int slot_tx);
int get_tx_harq_id(NR_UE_ULSCH_t *ulsch, int slot_tx);

int is_pbch_in_slot(fapi_nr_config_request_t *config, int frame, int slot, NR_DL_FRAME_PARMS *fp);
int is_ssb_in_slot(fapi_nr_config_request_t *config, int frame, int slot, NR_DL_FRAME_PARMS *fp);
bool is_csi_rs_in_symbol(fapi_nr_dl_config_csirs_pdu_rel15_t csirs_config_pdu, int symbol);
void nr_csi_slot_init(const PHY_VARS_NR_UE *ue,
                      const UE_nr_rxtx_proc_t *proc,
                      const fapi_nr_dl_config_csirs_pdu_rel15_t *csirs_config_pdu,
                      nr_csi_info_t *nr_csi_info,
                      nr_csi_phy_parms_t *csi_phy_parms);

/*! \brief This function prepares the dl indication to pass to the MAC
 */
void nr_fill_dl_indication(nr_downlink_indication_t *dl_ind,
                           fapi_nr_dci_indication_t *dci_ind,
                           fapi_nr_rx_indication_t *rx_ind,
                           const UE_nr_rxtx_proc_t *proc,
                           const PHY_VARS_NR_UE *ue,
                           void *phy_data);

/*@}*/

/*! \brief This function prepares the dl rx indication
 */
void nr_fill_rx_indication(fapi_nr_rx_indication_t *rx_ind,
                           const uint8_t pdu_type,
                           const PHY_VARS_NR_UE *ue,
                           const NR_UE_DLSCH_t *dlsch0,
                           const NR_UE_DLSCH_t *dlsch1,
                           const uint16_t n_pdus,
                           const UE_nr_rxtx_proc_t *proc,
                           const void *typeSpecific,
                           uint8_t *b);

bool nr_ue_dlsch_procedures(PHY_VARS_NR_UE *ue,
                            UE_nr_rxtx_proc_t *proc,
                            NR_UE_DLSCH_t dlsch[2],
                            const int llrSize,
                            int16_t llr[NR_MAX_NB_LAYERS > 4 ? 2 : 1][llrSize]);

bool nr_ue_pdsch_procedures(void *parms);

void nr_pdcch_slot_init(nr_phy_data_t *phyData, PHY_VARS_NR_UE *ue);

void nr_pdsch_slot_init(nr_phy_data_t *phyData, PHY_VARS_NR_UE *ue);

void nr_ue_csi_rs_symbol_procedures(
    const PHY_VARS_NR_UE *ue,
    const UE_nr_rxtx_proc_t *proc,
    const nr_csi_phy_parms_t *csi_phy_parms,
    const int symbol,
    const fapi_nr_dl_config_csirs_pdu_rel15_t *csirs_config_pdu,
    const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
    c16_t csi_rs_ls_estimates[ue->frame_parms.nb_antennas_rx][csi_phy_parms->N_ports][ue->frame_parms.ofdm_symbol_size],
    nr_csi_symbol_res_t *csi_symb_res);

void nr_ue_csi_rs_procedures(
    const PHY_VARS_NR_UE *ue,
    const UE_nr_rxtx_proc_t *proc,
    const NR_UE_CSI_RS *csirs_vars,
    const nr_csi_phy_parms_t *csi_phy_parms,
    nr_csi_symbol_res_t *res,
    int32_t csi_rs_ls_estimated_channel[ue->frame_parms.nb_antennas_rx][csi_phy_parms->N_ports][ue->frame_parms.ofdm_symbol_size]);

void nr_csi_rs_channel_estimation(const PHY_VARS_NR_UE *ue,
                                  const UE_nr_rxtx_proc_t *proc,
                                  const fapi_nr_dl_config_csirs_pdu_rel15_t *csirs_config_pdu,
                                  const nr_csi_info_t *nr_csi_info,
                                  const c16_t **csi_rs_generated_signal,
                                  const uint8_t N_cdm_groups,
                                  const uint8_t CDM_group_size,
                                  const uint8_t k_prime,
                                  const uint8_t l_prime,
                                  const uint8_t N_ports,
                                  const uint8_t j_cdm[16],
                                  const uint8_t k_overline[16],
                                  const uint8_t l_overline[16],
                                  const c16_t rxdataF[][ue->frame_parms.ofdm_symbol_size],
                                  const int symbol,
                                  c16_t csi_rs_ls_estimated_channel[][N_ports][ue->frame_parms.ofdm_symbol_size],
                                  nr_csi_symbol_res_t *res);

void nr_csi_im_symbol_power_estimation(const PHY_VARS_NR_UE *ue,
                                       const UE_nr_rxtx_proc_t *proc,
                                       const fapi_nr_dl_config_csiim_pdu_rel15_t *csiim_config_pdu,
                                       const int symbol,
                                       const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
                                       nr_csi_symbol_res_t *csi_im_res);

void nr_ue_csi_im_procedures(const fapi_nr_dl_config_csiim_pdu_rel15_t *csiim_config_pdu,
                             const nr_csi_symbol_res_t *res,
                             nr_csi_phy_parms_t *csi_phy_parms);

void nr_pdcch_generate_llr(const PHY_VARS_NR_UE *ue,
                           const UE_nr_rxtx_proc_t *proc,
                           const int symbol,
                           const nr_phy_data_t *phy_data,
                           const int llrSize,
                           const int max_monOcc,
                           const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
                           c16_t llr[phy_data->phy_pdcch_config.nb_search_space * max_monOcc * llrSize]);

void nr_pdcch_dci_indication(const UE_nr_rxtx_proc_t *proc,
                             const int llrSize,
                             const int max_monOcc,
                             PHY_VARS_NR_UE *ue,
                             nr_phy_data_t *phy_data,
                             const c16_t llr[phy_data->phy_pdcch_config.nb_search_space * max_monOcc * llrSize]);

int nr_pdsch_generate_channel_estimates(
    const PHY_VARS_NR_UE *ue,
    const UE_nr_rxtx_proc_t *proc,
    const int symbol,
    const NR_UE_DLSCH_t *dlsch,
    const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
    c16_t channel_estimates[dlsch->Nl][ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size]);

void nr_generate_pdsch_extracted_rxdataF(
    const PHY_VARS_NR_UE *ue,
    const UE_nr_rxtx_proc_t *proc,
    const int symbol,
    const NR_UE_DLSCH_t *dlsch,
    const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
    c16_t rxdataF_ext[ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB]);

void prs_processing(const PHY_VARS_NR_UE *ue,
                    const UE_nr_rxtx_proc_t *proc,
                    const int symbol,
                    const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size]);

void free_pdsch_slot_proc_buffers(nr_ue_symb_data_t *symb_data);
#endif
/** @}*/
