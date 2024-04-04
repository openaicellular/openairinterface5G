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

/*! \file phy_procedures_nr_ue.c
 * \brief Implementation of UE procedures from 36.213 LTE specifications
 * \author R. Knopp, F. Kaltenberger, N. Nikaein, A. Mico Pereperez, G. Casati
 * \date 2018
 * \version 0.1
 * \company Eurecom
 * \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr, navid.nikaein@eurecom.fr, guido.casati@iis.fraunhofer.de
 * \note
 * \warning
 */

#define _GNU_SOURCE

#include "nr/nr_common.h"
#include "assertions.h"
#include "defs.h"
#include "PHY/defs_nr_UE.h"
#include "PHY/NR_REFSIG/dmrs_nr.h"
#include "PHY/phy_extern_nr_ue.h"
#include "PHY/MODULATION/modulation_UE.h"
#include "PHY/INIT/nr_phy_init.h"
#include "PHY/NR_REFSIG/ptrs_nr.h"
#include "PHY/NR_UE_TRANSPORT/nr_transport_ue.h"
#include "PHY/NR_TRANSPORT/nr_transport_proto.h"
#include "PHY/NR_UE_TRANSPORT/nr_transport_proto_ue.h"
#include "PHY/NR_UE_TRANSPORT/srs_modulation_nr.h"
#include "SCHED_NR_UE/phy_sch_processing_time.h"
#include "PHY/NR_UE_ESTIMATION/nr_estimation.h"
#ifdef EMOS
#include "SCHED/phy_procedures_emos.h"
#endif
#include "executables/softmodem-common.h"
#include "executables/nr-uesoftmodem.h"
#include "SCHED_NR_UE/pucch_uci_ue_nr.h"
#include <openair1/PHY/TOOLS/phy_scope_interface.h>
#include "nfapi/open-nFAPI/nfapi/public_inc/nfapi_nr_interface.h"
#include "PHY/NR_REFSIG/ptrs_nr.h"
#include "openair1/PHY/NR_REFSIG/refsig_defs_ue.h"

//#define DEBUG_PHY_PROC
//#define NR_PDCCH_SCHED_DEBUG
//#define NR_PUCCH_SCHED
//#define NR_PUCCH_SCHED_DEBUG
//#define NR_PDSCH_DEBUG

#ifndef PUCCH
#define PUCCH
#endif

#include "common/utils/LOG/log.h"

#ifdef EMOS
fifo_dump_emos_UE emos_dump_UE;
#endif

#include "common/utils/LOG/vcd_signal_dumper.h"
#include "UTIL/OPT/opt.h"
#include "intertask_interface.h"
#include "T.h"

static const unsigned int gain_table[31] = {100,  112,  126,  141,  158,  178,  200,  224,  251, 282,  316,
                                            359,  398,  447,  501,  562,  631,  708,  794,  891, 1000, 1122,
                                            1258, 1412, 1585, 1778, 1995, 2239, 2512, 2818, 3162};

void nr_fill_dl_indication(nr_downlink_indication_t *dl_ind,
                           fapi_nr_dci_indication_t *dci_ind,
                           fapi_nr_rx_indication_t *rx_ind,
                           const UE_nr_rxtx_proc_t *proc,
                           const PHY_VARS_NR_UE *ue,
                           void *phy_data)
{
  memset((void*)dl_ind, 0, sizeof(nr_downlink_indication_t));

  dl_ind->gNB_index = proc->gNB_id;
  dl_ind->module_id = ue->Mod_id;
  dl_ind->cc_id     = ue->CC_id;
  dl_ind->frame     = proc->frame_rx;
  dl_ind->slot      = proc->nr_slot_rx;
  dl_ind->phy_data  = phy_data;

  if (dci_ind) {

    dl_ind->rx_ind = NULL; //no data, only dci for now
    dl_ind->dci_ind = dci_ind;

  } else if (rx_ind) {

    dl_ind->rx_ind = rx_ind; //  hang on rx_ind instance
    dl_ind->dci_ind = NULL;

  }
}

static uint32_t get_ssb_arfcn(const NR_DL_FRAME_PARMS *frame_parms)
{
  uint32_t band_size_hz = frame_parms->N_RB_DL * 12 * frame_parms->subcarrier_spacing;
  int ssb_center_sc = frame_parms->ssb_start_subcarrier + 120; // ssb is 20 PRBs -> 240 sub-carriers
  uint64_t ssb_freq = frame_parms->dl_CarrierFreq - (band_size_hz / 2) + frame_parms->subcarrier_spacing * ssb_center_sc;
  return to_nrarfcn(frame_parms->nr_band, ssb_freq, frame_parms->numerology_index, band_size_hz);
}

void nr_fill_rx_indication(fapi_nr_rx_indication_t *rx_ind,
                           const uint8_t pdu_type,
                           const PHY_VARS_NR_UE *ue,
                           const NR_UE_DLSCH_t *dlsch0,
                           const NR_UE_DLSCH_t *dlsch1,
                           const uint16_t n_pdus,
                           const UE_nr_rxtx_proc_t *proc,
                           const void *typeSpecific,
                           uint8_t *b)
{
  if (n_pdus > 1){
    LOG_E(PHY, "In %s: multiple number of DL PDUs not supported yet...\n", __FUNCTION__);
  }

  if ((pdu_type !=  FAPI_NR_RX_PDU_TYPE_SSB) && dlsch0) {
    int t=WS_C_RNTI;
    if (pdu_type == FAPI_NR_RX_PDU_TYPE_RAR)
      t=WS_RA_RNTI;
    if  (pdu_type == FAPI_NR_RX_PDU_TYPE_SIB)
      t=WS_SI_RNTI;
    trace_NRpdu(DIRECTION_DOWNLINK,
		b,
		dlsch0->dlsch_config.TBS / 8,
		t,
		dlsch0->rnti,
		proc->frame_rx,
		proc->nr_slot_rx,
		0,0);
  }
  switch (pdu_type){
    case FAPI_NR_RX_PDU_TYPE_SIB:
    case FAPI_NR_RX_PDU_TYPE_RAR:
    case FAPI_NR_RX_PDU_TYPE_DLSCH:
      if(dlsch0) {
        const NR_DL_UE_HARQ_t *dl_harq0 = &ue->dl_harq_processes[0][dlsch0->dlsch_config.harq_process_nbr];
        rx_ind->rx_indication_body[n_pdus - 1].pdsch_pdu.harq_pid = dlsch0->dlsch_config.harq_process_nbr;
        rx_ind->rx_indication_body[n_pdus - 1].pdsch_pdu.ack_nack = dl_harq0->ack;
        rx_ind->rx_indication_body[n_pdus - 1].pdsch_pdu.pdu = b;
        rx_ind->rx_indication_body[n_pdus - 1].pdsch_pdu.pdu_length = dlsch0->dlsch_config.TBS / 8;
      }
      if(dlsch1) {
        AssertFatal(1==0,"Second codeword currently not supported\n");
      }
      break;
    case FAPI_NR_RX_PDU_TYPE_SSB: {
        fapi_nr_ssb_pdu_t *ssb_pdu = &rx_ind->rx_indication_body[n_pdus - 1].ssb_pdu;
        if(typeSpecific) {
          const NR_DL_FRAME_PARMS *frame_parms = &ue->frame_parms;
          const fapiPbch_t *pbch = (fapiPbch_t *)typeSpecific;
          memcpy(ssb_pdu->pdu, pbch->decoded_output, sizeof(pbch->decoded_output));
          ssb_pdu->additional_bits = pbch->xtra_byte;
          ssb_pdu->ssb_index = (frame_parms->ssb_index) & 0x7;
          ssb_pdu->ssb_length = frame_parms->Lmax;
          ssb_pdu->cell_id = frame_parms->Nid_cell;
          ssb_pdu->ssb_start_subcarrier = frame_parms->ssb_start_subcarrier;
          ssb_pdu->rsrp_dBm = ue->measurements.ssb_rsrp_dBm[frame_parms->ssb_index];
          ssb_pdu->arfcn = get_ssb_arfcn(frame_parms);
          ssb_pdu->radiolink_monitoring = RLM_in_sync; // TODO to be removed from here
          ssb_pdu->decoded_pdu = true;
        }
        else {
          ssb_pdu->radiolink_monitoring = RLM_out_of_sync; // TODO to be removed from here
          ssb_pdu->decoded_pdu = false;
        }
      }
    break;
    case FAPI_NR_CSIRS_IND:
      memcpy(&rx_ind->rx_indication_body[n_pdus - 1].csirs_measurements,
             (fapi_nr_csirs_measurements_t*)typeSpecific,
             sizeof(*(fapi_nr_csirs_measurements_t*)typeSpecific));
      break;
    default:
    break;
  }

  rx_ind->rx_indication_body[n_pdus -1].pdu_type = pdu_type;
  rx_ind->number_pdus = n_pdus;

}

int get_tx_amp_prach(int power_dBm, int power_max_dBm, int N_RB_UL){

  int gain_dB = power_dBm - power_max_dBm, amp_x_100 = -1;

  switch (N_RB_UL) {
  case 6:
  amp_x_100 = AMP;      // PRACH is 6 PRBS so no scale
  break;
  case 15:
  amp_x_100 = 158*AMP;  // 158 = 100*sqrt(15/6)
  break;
  case 25:
  amp_x_100 = 204*AMP;  // 204 = 100*sqrt(25/6)
  break;
  case 50:
  amp_x_100 = 286*AMP;  // 286 = 100*sqrt(50/6)
  break;
  case 75:
  amp_x_100 = 354*AMP;  // 354 = 100*sqrt(75/6)
  break;
  case 100:
  amp_x_100 = 408*AMP;  // 408 = 100*sqrt(100/6)
  break;
  default:
  LOG_E(PHY, "Unknown PRB size %d\n", N_RB_UL);
  return (amp_x_100);
  break;
  }
  if (gain_dB < -30) {
    return (amp_x_100/3162);
  } else if (gain_dB > 0)
    return (amp_x_100);
  else
    return (amp_x_100/gain_table[-gain_dB]);  // 245 corresponds to the factor sqrt(25/6)

  return (amp_x_100);
}

// UL time alignment procedures:
// - If the current tx frame and slot match the TA configuration
//   then timing advance is processed and set to be applied in the next UL transmission
// - Application of timing adjustment according to TS 38.213 p4.2
// - handle RAR TA application as per ch 4.2 TS 38.213
void ue_ta_procedures(PHY_VARS_NR_UE *ue, int slot_tx, int frame_tx)
{
  if (frame_tx == ue->ta_frame && slot_tx == ue->ta_slot) {
    uint16_t ofdm_symbol_size = ue->frame_parms.ofdm_symbol_size;

    // convert time factor "16 * 64 * T_c / (2^mu)" in N_TA calculation in TS38.213 section 4.2 to samples by multiplying with
    // samples per second
    //   16 * 64 * T_c            / (2^mu) * samples_per_second
    // = 16 * T_s                 / (2^mu) * samples_per_second
    // = 16 * 1 / (15 kHz * 2048) / (2^mu) * (15 kHz * 2^mu * ofdm_symbol_size)
    // = 16 * 1 /           2048           *                  ofdm_symbol_size
    // = 16 * ofdm_symbol_size / 2048
    uint16_t bw_scaling = 16 * ofdm_symbol_size / 2048;

    ue->timing_advance += (ue->ta_command - 31) * bw_scaling;

    LOG_D(PHY,
          "[UE %d] [%d.%d] Got timing advance command %u from MAC, new value is %d\n",
          ue->Mod_id,
          frame_tx,
          slot_tx,
          ue->ta_command,
          ue->timing_advance);

    ue->ta_frame = -1;
    ue->ta_slot = -1;
  }
}

void phy_procedures_nrUE_TX(PHY_VARS_NR_UE *ue, const UE_nr_rxtx_proc_t *proc, nr_phy_data_tx_t *phy_data)
{
  const int slot_tx = proc->nr_slot_tx;
  const int frame_tx = proc->frame_tx;
  const int gNB_id = proc->gNB_id;

  AssertFatal(ue->CC_id == 0, "Transmission on secondary CCs is not supported yet\n");

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX,VCD_FUNCTION_IN);

  const int samplesF_per_slot = NR_SYMBOLS_PER_SLOT * ue->frame_parms.ofdm_symbol_size;
  c16_t txdataF_buf[ue->frame_parms.nb_antennas_tx * samplesF_per_slot] __attribute__((aligned(32)));
  memset(txdataF_buf, 0, sizeof(txdataF_buf));
  c16_t *txdataF[ue->frame_parms.nb_antennas_tx]; /* workaround to be compatible with current txdataF usage in all tx procedures. */
  for(int i=0; i< ue->frame_parms.nb_antennas_tx; ++i)
    txdataF[i] = &txdataF_buf[i * samplesF_per_slot];

  LOG_D(PHY,"****** start TX-Chain for AbsSubframe %d.%d ******\n", frame_tx, slot_tx);

  start_meas(&ue->phy_proc_tx);

  for (uint8_t harq_pid = 0; harq_pid < NR_MAX_ULSCH_HARQ_PROCESSES; harq_pid++) {
    if (ue->ul_harq_processes[harq_pid].status == ACTIVE) {
      nr_ue_ulsch_procedures(ue, harq_pid, frame_tx, slot_tx, gNB_id, phy_data, (c16_t **)&txdataF);
    }
  }

  ue_srs_procedures_nr(ue, proc, (c16_t **)&txdataF);

  pucch_procedures_ue_nr(ue, proc, phy_data, (c16_t **)&txdataF);

  LOG_D(PHY, "Sending Uplink data \n");
  nr_ue_pusch_common_procedures(ue, proc->nr_slot_tx, &ue->frame_parms, ue->frame_parms.nb_antennas_tx, (c16_t **)txdataF);

  nr_ue_prach_procedures(ue, proc);

  LOG_D(PHY, "****** end TX-Chain for AbsSubframe %d.%d ******\n", proc->frame_tx, proc->nr_slot_tx);

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
  stop_meas(&ue->phy_proc_tx);
}

void nr_ue_measurement_procedures(uint16_t l,
                                  PHY_VARS_NR_UE *ue,
                                  const UE_nr_rxtx_proc_t *proc,
                                  NR_UE_DLSCH_t *dlsch,
                                  uint32_t pdsch_est_size,
                                  int32_t dl_ch_estimates[][pdsch_est_size])
{
  NR_DL_FRAME_PARMS *frame_parms=&ue->frame_parms;
  int nr_slot_rx = proc->nr_slot_rx;
  int gNB_id = proc->gNB_id;
  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_MEASUREMENT_PROCEDURES, VCD_FUNCTION_IN);

  if (l==2) {

    LOG_D(PHY,"Doing UE measurement procedures in symbol l %u Ncp %d nr_slot_rx %d, rxdata %p\n",
      l,
      ue->frame_parms.Ncp,
      nr_slot_rx,
      ue->common_vars.rxdata);

    nr_ue_measurements(ue, proc, dlsch, pdsch_est_size, dl_ch_estimates);

#if T_TRACER
    if(nr_slot_rx == 0)
      T(T_UE_PHY_MEAS,
        T_INT(gNB_id),
        T_INT(ue->Mod_id),
        T_INT(proc->frame_rx % 1024),
        T_INT(nr_slot_rx),
        T_INT((int)(10 * log10(ue->measurements.rsrp[0]) - ue->rx_total_gain_dB)),
        T_INT((int)ue->measurements.rx_rssi_dBm[0]),
        T_INT((int)(ue->measurements.rx_power_avg_dB[0] - ue->measurements.n0_power_avg_dB)),
        T_INT((int)ue->measurements.rx_power_avg_dB[0]),
        T_INT((int)ue->measurements.n0_power_avg_dB),
        T_INT((int)ue->measurements.wideband_cqi_avg[0]));
#endif
  }

  // accumulate and filter timing offset estimation every subframe (instead of every frame)
  if (( nr_slot_rx == 2) && (l==(2-frame_parms->Ncp))) {

    // AGC

    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GAIN_CONTROL, VCD_FUNCTION_IN);


    //printf("start adjust gain power avg db %d\n", ue->measurements.rx_power_avg_dB[gNB_id]);
    phy_adjust_gain_nr (ue,ue->measurements.rx_power_avg_dB[gNB_id],gNB_id);
    
    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GAIN_CONTROL, VCD_FUNCTION_OUT);

}

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_MEASUREMENT_PROCEDURES, VCD_FUNCTION_OUT);
}

unsigned int nr_get_tx_amp(int power_dBm, int power_max_dBm, int N_RB_UL, int nb_rb)
{

  int gain_dB = power_dBm - power_max_dBm;
  double gain_lin;

  gain_lin = pow(10,.1*gain_dB);
  if ((nb_rb >0) && (nb_rb <= N_RB_UL)) {
    return((int)(AMP*sqrt(gain_lin*N_RB_UL/(double)nb_rb)));
  }
  else {
    LOG_E(PHY,"Illegal nb_rb/N_RB_UL combination (%d/%d)\n",nb_rb,N_RB_UL);
    //mac_xface->macphy_exit("");
  }
  return(0);
}

/* To be called after PDCCH scheduled */
void nr_pdcch_slot_init(nr_phy_data_t *phyData, PHY_VARS_NR_UE *ue)
{
  const unsigned short scrambling_id = phyData->phy_pdcch_config.pdcch_config[0].coreset.pdcch_dmrs_scrambling_id;
  // checking if re-initialization of scrambling IDs is needed (should be done here but scrambling ID for PDCCH is not taken from
  // RRC)
  if (scrambling_id != ue->scramblingID_pdcch) {
    ue->scramblingID_pdcch = scrambling_id;
    nr_gold_pdcch(ue, ue->scramblingID_pdcch);
  }
  /* Initialize DLSCH struct. Used by MAC to copy PDSCH phy config */
  nr_ue_dlsch_init(phyData->dlsch, NR_MAX_NB_LAYERS > 4 ? 2 : 1, ue->max_ldpc_iterations);
}

/* To be called every slot after PDSCH scheduled */
void nr_pdsch_slot_init(nr_phy_data_t *phyData, PHY_VARS_NR_UE *ue)
{
  /* checking if re-initialization of scrambling IDs is needed */
  const NR_UE_DLSCH_t *dlsch0 = &phyData->dlsch[0];
  const int scramblingId = dlsch0->dlsch_config.dlDmrsScramblingId;
  const int nscid = dlsch0->dlsch_config.nscid;
  if (scramblingId != ue->scramblingID_dlsch[nscid]) {
    ue->scramblingID_dlsch[nscid] = scramblingId;
    nr_gold_pdsch(nscid, scramblingId, ue);
  }
}

/* Generate channel estimates for received OFDM symbol */
int nr_pdsch_generate_channel_estimates(
    const PHY_VARS_NR_UE *ue,
    const UE_nr_rxtx_proc_t *proc,
    const int symbol,
    const NR_UE_DLSCH_t *dlsch,
    const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
    c16_t channel_estimates[dlsch->Nl][ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size])
{
  if (!get_isPilot_symbol(symbol, dlsch))
    return -1;
  const int BWPStart = dlsch->dlsch_config.BWPStart;
  const int pdschStartRb = dlsch->dlsch_config.start_rb;
  const int pdschNbRb = dlsch->dlsch_config.number_rbs;
  const int nbAntRx = ue->frame_parms.nb_antennas_rx;
  int retVal = 0;

  /* TODO: Could be launched in Tpool for MIMO */
  for (int l = 0; l < dlsch->Nl; l++) { // for MIMO Config: it shall loop over no_layers
    for (int aarx = 0; aarx < nbAntRx; aarx++) {
      const int port = get_dmrs_port(l, dlsch->dlsch_config.dmrs_ports);
      retVal += nr_pdsch_channel_estimation(ue,
                                            proc,
                                            dlsch->dlsch_config.rb_offset,
                                            port,
                                            aarx,
                                            symbol,
                                            BWPStart,
                                            dlsch->dlsch_config.dmrsConfigType,
                                            ue->frame_parms.first_carrier_offset + (BWPStart + pdschStartRb) * 12,
                                            pdschNbRb,
                                            dlsch->dlsch_config.nscid,
                                            rxdataF[aarx],
                                            channel_estimates[l][aarx]);
    }
  }
  return retVal;
}

/* Extract data resourse elements from received OFDM symbol for all antennas */
void nr_generate_pdsch_extracted_rxdataF(
    const PHY_VARS_NR_UE *ue,
    const UE_nr_rxtx_proc_t *proc,
    const int symbol,
    const NR_UE_DLSCH_t *dlsch,
    const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
    c16_t rxdataF_ext[ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB])
{
  /* TODO: Could be launched in Tpool for MIMO */
  for (int aarx = 0; aarx < ue->frame_parms.nb_antennas_rx; aarx++) {
    nr_extract_data_res(&ue->frame_parms,
                        &dlsch->dlsch_config,
                        get_isPilot_symbol(symbol, dlsch),
                        rxdataF[aarx],
                        rxdataF_ext[aarx]);
  }
}

/* Do time domain averaging of received channel estimates for all antennas */
void nr_pdsch_estimates_time_avg(
    const NR_UE_DLSCH_t *dlsch,
    const NR_DL_FRAME_PARMS *frame_parms,
    c16_t dl_ch_est[NR_SYMBOLS_PER_SLOT][frame_parms->nb_antennas_rx][dlsch->Nl][frame_parms->ofdm_symbol_size])
{
  const int nbAntRx = frame_parms->nb_antennas_rx;
  const int dlDmrsSymbPos = dlsch->dlsch_config.dlDmrsSymbPos;
  /* TODO: Could be launched in Tpool for MIMO */
  for (int aarx = 0; aarx < nbAntRx; aarx++) {
    for (int l = 0; l < dlsch->Nl; l++) {
      /* Average estimates in time if configured */
      nr_chest_time_domain_avg(frame_parms,
                               dlsch->dlsch_config.number_symbols,
                               dlsch->dlsch_config.start_symbol,
                               dlDmrsSymbPos,
                               dlsch->dlsch_config.number_rbs,
                               aarx,
                               l,
                               dlsch->Nl,
                               true,
                               (c16_t *)dl_ch_est);
    }
  }
}

/* Do several channel measurements */
void nr_pdsch_measurements(
    const NR_DL_FRAME_PARMS *frame_parms,
    const NR_UE_DLSCH_t *dlsch,
    const int symbol,
    const int output_shift,
    const c16_t dl_ch_est_ext[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
    PHY_NR_MEASUREMENTS *measurements)
{
  for (int aarx = 0; aarx < frame_parms->nb_antennas_rx; aarx++) {
    /* Channel correlation matrix */
    int rho[dlsch->Nl][dlsch->Nl][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB];
    nr_compute_channel_correlation(dlsch->Nl,
                                   get_nb_re_pdsch_symbol(symbol, dlsch),
                                   dlsch->dlsch_config.number_rbs,
                                   frame_parms->nb_antennas_rx,
                                   aarx,
                                   output_shift,
                                   dl_ch_est_ext,
                                   rho);
    if (symbol
        == get_first_symb_idx_with_data(dlsch->dlsch_config.dlDmrsSymbPos,
                                        dlsch->dlsch_config.dmrsConfigType,
                                        dlsch->dlsch_config.n_dmrs_cdm_groups,
                                        dlsch->dlsch_config.start_symbol,
                                        dlsch->dlsch_config.number_symbols)) {
      for (int l = 0; l < dlsch->Nl; l++) {
        for (int atx = 0; atx < dlsch->Nl; atx++) {
          measurements->rx_correlation[0][aarx][l * dlsch->Nl + atx] =
              signal_energy(&rho[aarx][l][atx], get_nb_re_pdsch_symbol(symbol, dlsch));
        }
      }
    } // First symbol with data
  } // Antennas
}

/* Performs for a OFDM symbol:
  1) PTRS phase compensation
  2) LLR generation for all layers
*/
void pdsch_llr_generation(
    const NR_DL_FRAME_PARMS *frame_parms,
    const int symbol,
    const NR_UE_DLSCH_t *dlsch,
    const c16_t ptrs_phase[frame_parms->nb_antennas_rx][NR_SYMBOLS_PER_SLOT],
    const int32_t ptrs_re[frame_parms->nb_antennas_rx][NR_SYMBOLS_PER_SLOT],
    const c16_t dl_ch_mag[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
    const c16_t dl_ch_magb[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
    const c16_t dl_ch_magr[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
    c16_t rxdataF_comp[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
    const int llrSize,
    int16_t layerLlr[dlsch->Nl][llrSize])
{
  const int nb_re_pdsch = get_nb_re_pdsch_symbol(symbol, dlsch);
  int dl_valid_re = nb_re_pdsch;
  const int pduBitmap = dlsch->dlsch_config.pduBitmap;
  for (int aarx = 0; aarx < frame_parms->nb_antennas_rx; aarx++) {
    /* PTRS phase compensation */
    if ((pduBitmap & 0x1) && (dlsch->rnti_type == TYPE_C_RNTI_)) {
      nr_pdsch_ptrs_compensate(ptrs_phase[aarx][symbol], symbol, dlsch, rxdataF_comp[0][aarx]);
      /* Adjust the valid DL RE's */
      if (aarx == 0)
        dl_valid_re -= ptrs_re[0][symbol];
    }
  }

  nr_dlsch_llr(frame_parms,
               dlsch,
               dl_valid_re,
               dl_ch_mag[0][0],
               dl_ch_magb[0][0],
               dl_ch_magr[0][0],
               rxdataF_comp,
               llrSize,
               layerLlr);
}

void pdsch_llr_generation_Tpool(void *parms)
{
  nr_ue_symb_data_t *msg = (nr_ue_symb_data_t *)parms;
  const PHY_VARS_NR_UE *ue = msg->UE;
  const NR_UE_DLSCH_t *dlsch = msg->p_dlsch;
  const c16_t(*ptrs_phase)[ue->frame_parms.nb_antennas_rx][NR_SYMBOLS_PER_SLOT] =
      (const c16_t(*)[ue->frame_parms.nb_antennas_rx][NR_SYMBOLS_PER_SLOT])msg->ptrs_phase_per_slot;
  const int32_t(*ptrs_re)[ue->frame_parms.nb_antennas_rx][NR_SYMBOLS_PER_SLOT] =
      (const int32_t(*)[ue->frame_parms.nb_antennas_rx][NR_SYMBOLS_PER_SLOT])msg->ptrs_re_per_slot;
  const c16_t(*dl_ch_mag)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB] =
      (const c16_t(*)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB]) msg->dl_ch_mag;
  const c16_t(*dl_ch_magb)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB] =
      (const c16_t(*)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB]) msg->dl_ch_magb;
  const c16_t(*dl_ch_magr)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB] =
      (const c16_t(*)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB]) msg->dl_ch_magr;
  c16_t(*rxdataF_comp)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB] =
      (c16_t(*)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB]) msg->rxdataF_comp;
  int16_t(*layerLlr)[dlsch->Nl][msg->llrSize] = (int16_t(*)[dlsch->Nl][msg->llrSize])msg->layer_llr;

  pdsch_llr_generation(&ue->frame_parms,
                       msg->symbol,
                       dlsch,
                       *ptrs_phase,
                       *ptrs_re,
                       *dl_ch_mag,
                       *dl_ch_magb,
                       *dl_ch_magr,
                       *rxdataF_comp,
                       msg->llrSize,
                       *layerLlr);
}

/* Decode DLSCH from LLRs and send TB to MAC */
bool pdsch_post_processing(PHY_VARS_NR_UE *ue,
                           UE_nr_rxtx_proc_t *proc,
                           NR_UE_DLSCH_t *dlsch,
                           c16_t ptrs_phase[ue->frame_parms.nb_antennas_rx][NR_SYMBOLS_PER_SLOT],
                           const int32_t ptrs_re[ue->frame_parms.nb_antennas_rx][NR_SYMBOLS_PER_SLOT],
                           const c16_t dl_ch_mag[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx]
                                                [dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
                           const c16_t dl_ch_magb[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx]
                                                 [dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
                           const c16_t dl_ch_magr[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx]
                                                 [dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
                           c16_t rxdataF_comp[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx]
                                             [dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB])
{
  fapi_nr_dl_config_dlsch_pdu_rel15_t *dlsch_config = &dlsch->dlsch_config;
  const int pduBitmap = dlsch_config->pduBitmap;
  for (int aarx = 0; aarx < ue->frame_parms.nb_antennas_rx; aarx++) {
    /* Interpolate PTRS estimated in time domain */
    if ((pduBitmap & 0x1) && (dlsch->rnti_type == TYPE_C_RNTI_)) {
      nr_pdsch_ptrs_tdinterpol(dlsch, ptrs_phase[aarx]);
    }
  }

  /* FIFO to store results */
  notifiedFIFO_t resFifo;
  initNotifiedFIFO(&resFifo);

  /* create LLR layer buffer */
  const int llr_per_symbol = get_max_llr_per_symbol(dlsch);
  int16_t layer_llr[NR_SYMBOLS_PER_SLOT][NR_MAX_NB_LAYERS][llr_per_symbol];

  start_meas(&ue->pdsch_llr_gen);
  const int s0 = dlsch_config->start_symbol;
  const int s1 = dlsch_config->number_symbols;
  for (int j = s0; j < (s0 + s1); j++) {
    /* launch worker threads */
    notifiedFIFO_elt_t *newElt =
        newNotifiedFIFO_elt(sizeof(nr_ue_symb_data_t), proc->nr_slot_rx, &resFifo, pdsch_llr_generation_Tpool);
    nr_ue_symb_data_t *msg = (nr_ue_symb_data_t *)NotifiedFifoData(newElt);
    msg->symbol = j;
    msg->UE = ue;
    msg->proc = proc;
    msg->p_dlsch = dlsch;
    msg->llrSize = llr_per_symbol;
    msg->layer_llr = (int16_t *)layer_llr[j];
    msg->ptrs_phase_per_slot = (c16_t *)ptrs_phase;
    msg->ptrs_re_per_slot = (int32_t *)ptrs_re;
    msg->dl_ch_mag = (c16_t *)dl_ch_mag[j];
    msg->dl_ch_magb = (c16_t *)dl_ch_magb[j];
    msg->dl_ch_magr = (c16_t *)dl_ch_magr[j];
    msg->rxdataF_comp = (c16_t *)rxdataF_comp[j];
    pushTpool(&(get_nrUE_params()->Tpool), newElt);
  }

  /* Collect processed info from finished threads */
  for (int i = 0; i < s1; i++) {
    notifiedFIFO_elt_t *res = pullTpool(&resFifo, &(get_nrUE_params()->Tpool));
    LOG_D(PHY, "Got LLRs from symbol %d\n", ((nr_ue_symb_data_t *)res->msgData)->symbol);
    if (res == NULL)
      LOG_E(PHY, "Tpool has been aborted\n");
    else
      delNotifiedFIFO_elt(res);
  }
  stop_meas(&ue->pdsch_llr_gen);

  /* LLR buffer creation */
  const uint8_t nb_re_dmrs = (dlsch_config->dmrsConfigType == NFAPI_NR_DMRS_TYPE1)
                                 ? 6 * dlsch_config->n_dmrs_cdm_groups
                                 : 4 * dlsch_config->n_dmrs_cdm_groups;

  const int dmrs_len = get_num_dmrs(dlsch->dlsch_config.dlDmrsSymbPos);

  uint32_t unav_res = 0;
  if(pduBitmap & 0x1) {
    uint16_t ptrsSymbPos = get_ptrs_symb_idx(dlsch_config->number_symbols,
                                              dlsch_config->start_symbol,
                                              1 << dlsch_config->PTRSTimeDensity,
                                              dlsch_config->dlDmrsSymbPos);
    int n_ptrs = (dlsch_config->number_rbs + dlsch_config->PTRSFreqDensity - 1) / dlsch_config->PTRSFreqDensity;
    int ptrsSymbPerSlot = get_ptrs_symbols_in_slot(ptrsSymbPos, dlsch_config->start_symbol, dlsch_config->number_symbols);
    unav_res = n_ptrs * ptrsSymbPerSlot;
  }
  const int harq_pid = dlsch[0].dlsch_config.harq_process_nbr;
  NR_DL_UE_HARQ_t *dl_harq0 = &ue->dl_harq_processes[0][harq_pid];
  dl_harq0->G = nr_get_G(dlsch_config->number_rbs,
                         dlsch_config->number_symbols,
                         nb_re_dmrs,
                         dmrs_len,
                         unav_res,
                         dlsch_config->qamModOrder,
                         dlsch->Nl);
  const int rx_llr_size = dl_harq0->G;
  const int rx_llr_buf_sz = ((rx_llr_size + 31) / 32) * 32;
  const int nb_codewords = NR_MAX_NB_LAYERS > 4 ? 2 : 1;
  start_meas(&ue->pdsch_llr_demapping);
  __attribute__((aligned(16))) int16_t llr[nb_codewords][rx_llr_buf_sz];
  memset(llr, 0, sizeof(llr));

  /* LLR Layer Demapping */
  int dl_valid_re[NR_SYMBOLS_PER_SLOT];
  compute_dl_valid_re(dlsch, ptrs_re, dl_valid_re);
  nr_dlsch_layer_demapping(dlsch->Nl,
                           dlsch->dlsch_config.qamModOrder,
                           llr_per_symbol,
                           layer_llr,
                           dlsch,
                           dl_valid_re,
                           rx_llr_size,
                           llr[0]);
  stop_meas(&ue->pdsch_llr_demapping);

  UEscopeCopy(ue, pdschLlr, llr[0], sizeof(int16_t), 1, rx_llr_size, 0);

  LOG_D(PHY, "DLSCH data reception at nr_slot_rx: %d\n", proc->nr_slot_rx);

  start_meas(&ue->dlsch_procedures_stat);
  bool dec_res = nr_ue_dlsch_procedures(ue, proc, dlsch, rx_llr_size, llr);
  stop_meas(&ue->dlsch_procedures_stat);

  if (ue->phy_sim_pdsch_llr)
    memcpy(ue->phy_sim_pdsch_llr, llr, sizeof(int16_t) * rx_llr_size);

  return dec_res;
}

bool nr_ue_pdsch_procedures(void *parms)
{
  nr_ue_symb_data_t *msg = (nr_ue_symb_data_t *)parms;
  PHY_VARS_NR_UE *ue = msg->UE;
  UE_nr_rxtx_proc_t *proc = &msg->valProc;
  NR_UE_DLSCH_t *dlsch = &msg->dlsch[0];

  c16_t(*dl_ch_est)[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size] =
      (c16_t(*)[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size])
          msg->pdsch_dl_ch_estimates;

  if (ue->chest_time == 1)
    nr_pdsch_estimates_time_avg(dlsch, &ue->frame_parms, *dl_ch_est);
  /* set PTRS bitmap */
  if ((dlsch->dlsch_config.pduBitmap & 0x1) && (dlsch->rnti_type == TYPE_C_RNTI_)) {
    msg->dlsch[0].ptrs_symbols = get_ptrs_symb_idx(dlsch->dlsch_config.number_symbols,
                                                   dlsch->dlsch_config.start_symbol,
                                                   1 << dlsch->dlsch_config.PTRSTimeDensity,
                                                   dlsch->dlsch_config.dlDmrsSymbPos);
  }

  start_meas(&ue->pdsch_comp_out);

  __attribute__((aligned(32))) c16_t rxdataF_comp[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx]
                                                 [dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB];
  memset(rxdataF_comp, 0, sizeof(rxdataF_comp));

  __attribute__((aligned(32))) c16_t pdsch_dl_ch_est_ext[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx]
                                                        [dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB];
  memset(pdsch_dl_ch_est_ext, 0, sizeof(pdsch_dl_ch_est_ext));

  __attribute__((aligned(32)))
  c16_t dl_ch_mag[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB];
  memset(dl_ch_mag, 0, sizeof(dl_ch_mag));

  __attribute__((aligned(32))) c16_t dl_ch_magb[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx]
                                               [dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB];
  memset(dl_ch_magb, 0, sizeof(dl_ch_magb));

  __attribute__((aligned(32))) c16_t dl_ch_magr[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx]
                                               [dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB];
  memset(dl_ch_magr, 0, sizeof(dl_ch_magr));

  __attribute__((aligned(32))) c16_t ptrs_phase[NR_SYMBOLS_PER_SLOT][ue->frame_parms.nb_antennas_rx];
  memset(ptrs_phase, 0, sizeof(ptrs_phase));

  __attribute__((aligned(32))) int32_t ptrs_re[NR_SYMBOLS_PER_SLOT][ue->frame_parms.nb_antennas_rx];
  memset(ptrs_re, 0, sizeof(ptrs_re));

  notifiedFIFO_t nf;
  initNotifiedFIFO(&nf);
  for (int symbol = dlsch->dlsch_config.start_symbol;
       symbol < dlsch->dlsch_config.start_symbol + dlsch->dlsch_config.number_symbols;
       symbol++) {
    notifiedFIFO_elt_t *newElt = newNotifiedFIFO_elt(sizeof(nr_ue_symb_data_t), proc->nr_slot_tx, &nf, nr_pdsch_comp_out);
    nr_ue_symb_data_t *symbMsg = (nr_ue_symb_data_t *)NotifiedFifoData(newElt);
    const int symbBlockSizeExt = ue->frame_parms.nb_antennas_rx * dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB;
    symbMsg->UE = ue;
    symbMsg->proc = proc;
    symbMsg->symbol = symbol;
    symbMsg->p_dlsch = dlsch;
    symbMsg->pdsch_dl_ch_estimates = msg->pdsch_dl_ch_estimates;
    symbMsg->rxdataF_ext = msg->rxdataF_ext + (symbol * symbBlockSizeExt);
    symbMsg->pdsch_dl_ch_est_ext = (c16_t *)pdsch_dl_ch_est_ext[symbol];
    symbMsg->rxdataF_comp = (c16_t *)rxdataF_comp[symbol];
    symbMsg->dl_ch_mag = (c16_t *)dl_ch_mag[symbol];
    symbMsg->dl_ch_magb = (c16_t *)dl_ch_magb[symbol];
    symbMsg->dl_ch_magr = (c16_t *)dl_ch_magr[symbol];
    symbMsg->ptrs_phase_per_slot = (c16_t *)ptrs_phase;
    symbMsg->ptrs_re_per_slot = (int32_t *)ptrs_re;
    pushTpool(&(get_nrUE_params()->Tpool), newElt);
  }

  for (int resIdx = 0; resIdx < dlsch->dlsch_config.number_symbols; resIdx++) {
    notifiedFIFO_elt_t *res = pullTpool(&nf, &(get_nrUE_params()->Tpool));
    if (res == NULL)
      LOG_E(PHY, "Tpool has been aborted\n");
    else
      delNotifiedFIFO_elt(res);
  }
  stop_meas(&ue->pdsch_comp_out);

  /* Copy physim test buffer and scope data */
  int estEltOffsetExt = 0;
  const int elementSz = dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB;
  for (int symbol = dlsch->dlsch_config.start_symbol;
       symbol < dlsch->dlsch_config.start_symbol + dlsch->dlsch_config.number_symbols;
       symbol++) {
    if (ue->phy_sim_pdsch_rxdataF_comp)
      memcpy(ue->phy_sim_pdsch_rxdataF_comp + estEltOffsetExt * sizeof(c16_t),
             rxdataF_comp[symbol][0][0],
             sizeof(c16_t) * elementSz);
    UEscopeCopy(ue,
                pdschRxdataF_comp,
                rxdataF_comp[symbol][0],
                sizeof(c16_t),
                ue->frame_parms.nb_antennas_rx,
                elementSz,
                estEltOffsetExt);
    estEltOffsetExt += elementSz;
  }

  return pdsch_post_processing(ue, proc, dlsch, ptrs_phase, ptrs_re, dl_ch_mag, dl_ch_magb, dl_ch_magr, rxdataF_comp);
}

void nr_ue_csi_rs_symbol_procedures(
    const PHY_VARS_NR_UE *ue,
    const UE_nr_rxtx_proc_t *proc,
    const nr_csi_phy_parms_t *csi_phy_parms,
    const int symbol,
    const fapi_nr_dl_config_csirs_pdu_rel15_t *csirs_config_pdu,
    const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
    c16_t csi_rs_ls_estimates[ue->frame_parms.nb_antennas_rx][csi_phy_parms->N_ports][ue->frame_parms.ofdm_symbol_size],
    nr_csi_symbol_res_t *csi_symb_res)
{
  nr_csi_rs_channel_estimation(ue,
                               proc,
                               csirs_config_pdu,
                               ue->nr_csi_info,
                               (const c16_t **)ue->nr_csi_info->csi_rs_generated_signal,
                               csi_phy_parms->N_cdm_groups,
                               csi_phy_parms->CDM_group_size,
                               csi_phy_parms->k_prime,
                               csi_phy_parms->l_prime,
                               csi_phy_parms->N_ports,
                               csi_phy_parms->j_cdm,
                               csi_phy_parms->k_overline,
                               csi_phy_parms->l_overline,
                               rxdataF,
                               symbol,
                               csi_rs_ls_estimates,
                               csi_symb_res);
}

bool nr_ue_dlsch_procedures(PHY_VARS_NR_UE *ue,
                            UE_nr_rxtx_proc_t *proc,
                            NR_UE_DLSCH_t dlsch[2],
                            const int llrSize,
                            int16_t llr[NR_MAX_NB_LAYERS > 4 ? 2 : 1][llrSize])
{
  DevAssert(dlsch[0].active);

  const int harq_pid = dlsch[0].dlsch_config.harq_process_nbr;
  const int frame_rx = proc->frame_rx;
  const int nr_slot_rx = proc->nr_slot_rx;
  NR_DL_UE_HARQ_t *dl_harq0 = &ue->dl_harq_processes[0][harq_pid];
  NR_DL_UE_HARQ_t *dl_harq1 = &ue->dl_harq_processes[1][harq_pid];

  const uint8_t is_cw0_active = dl_harq0->status;
  const uint8_t is_cw1_active = dl_harq1->status;
  const uint16_t nb_symb_sch = dlsch[0].dlsch_config.number_symbols;

  LOG_D(PHY, "Frame.slot %d.%d Start LDPC Decoder for CW0 [harq_pid %d] ? %d \n", frame_rx, nr_slot_rx, harq_pid, is_cw0_active);
  LOG_D(PHY, "Frame.slot %d.%d Start LDPC Decoder for CW1 [harq_pid %d] ? %d \n", frame_rx, nr_slot_rx, harq_pid, is_cw1_active);

  DevAssert(is_cw0_active == ACTIVE);

  start_meas(&ue->dlsch_unscrambling_stats);
  nr_dlsch_unscrambling(llr[0],
                        dl_harq0->G,
                        0,
                        dlsch[0].dlsch_config.dlDataScramblingId,
                        dlsch[0].rnti);
    

  stop_meas(&ue->dlsch_unscrambling_stats);

  start_meas(&ue->dlsch_decoding_stats);

  // create memory to store decoder output
  const int num_rb = dlsch[0].dlsch_config.number_rbs;
  const int segx = MAX_NUM_NR_DLSCH_SEGMENTS_PER_LAYER * NR_MAX_NB_LAYERS; // number of segments to be allocated
  const int segy = MAX_NUM_NR_DLSCH_SEGMENTS_PER_LAYER * NR_MAX_NB_LAYERS * num_rb;
  const int a_segments = (num_rb != 273) ? ((segy / 273) + 1) : segx;

  const uint32_t dlsch_bytes = a_segments * 1056; // allocated bytes per segment
  __attribute__ ((aligned(32))) uint8_t p_b[dlsch_bytes];

  const int ret = nr_dlsch_decoding(ue,
                                    proc,
                                    proc->gNB_id,
                                    llr[0],
                                    &ue->frame_parms,
                                    &dlsch[0],
                                    dl_harq0,
                                    frame_rx,
                                    nb_symb_sch,
                                    nr_slot_rx,
                                    harq_pid,
                                    dlsch_bytes,
                                    p_b);

  LOG_T(PHY,"dlsch decoding, ret = %d\n", ret);

  int ind_type = -1;
  switch(dlsch[0].rnti_type) {
    case TYPE_RA_RNTI_:
      ind_type = FAPI_NR_RX_PDU_TYPE_RAR;
      break;

    case TYPE_SI_RNTI_:
      ind_type = FAPI_NR_RX_PDU_TYPE_SIB;
      break;

    case TYPE_C_RNTI_:
      ind_type = FAPI_NR_RX_PDU_TYPE_DLSCH;
      break;

    default:
      AssertFatal(true, "Invalid DLSCH type %d\n", dlsch[0].rnti_type);
      break;
  }

  nr_downlink_indication_t dl_indication;
  fapi_nr_rx_indication_t rx_ind = {0};
  nr_fill_dl_indication(&dl_indication, NULL, &rx_ind, proc, ue, NULL);
  const uint16_t number_pdus = 1;
  nr_fill_rx_indication(&rx_ind, ind_type, ue, &dlsch[0], NULL, number_pdus, proc, NULL, p_b);

  LOG_D(PHY, "DL PDU length in bits: %d, in bytes: %d \n", dlsch[0].dlsch_config.TBS, dlsch[0].dlsch_config.TBS / 8);

  stop_meas(&ue->dlsch_decoding_stats);

#if 0
  if(is_cw1_active) {
    // start ldpc decode for CW 1
    fapi_nr_dl_config_dlsch_pdu_rel15_t *dlsch_config = &dlsch[1].dlsch_config;
    uint32_t unav_res = 0;
    if(dlsch_config->pduBitmap & 0x1) {
      uint16_t ptrsSymbPos = 0;
      set_ptrs_symb_idx(&ptrsSymbPos,
                        dlsch_config->number_symbols,
                        dlsch_config->start_symbol,
                        1 << dlsch_config->PTRSTimeDensity,
                        dlsch_config->dlDmrsSymbPos);
      int n_ptrs = (dlsch_config->number_rbs + dlsch_config->PTRSFreqDensity - 1) / dlsch_config->PTRSFreqDensity;
      int ptrsSymbPerSlot = get_ptrs_symbols_in_slot(ptrsSymbPos, dlsch_config->start_symbol, dlsch_config->number_symbols);
      unav_res = n_ptrs * ptrsSymbPerSlot;
    }
    dl_harq1->G = nr_get_G(dlsch_config->number_rbs,
                           nb_symb_sch,
                           nb_re_dmrs,
                           dmrs_len,
                           unav_res,
                           dlsch_config->qamModOrder,
                           dlsch[1].Nl);
    start_meas(&ue->dlsch_unscrambling_stats);
    nr_dlsch_unscrambling(llr[1],
                          dl_harq1->G,
                          0,
                          dlsch[1].dlsch_config.dlDataScramblingId,
                          dlsch[1].rnti);
    stop_meas(&ue->dlsch_unscrambling_stats);

    start_meas(&ue->dlsch_decoding_stats);

    const int ret1 = nr_dlsch_decoding(ue,
                                       proc,
                                       proc->gNB_id,
                                       llr[1],
                                       &ue->frame_parms,
                                       &dlsch[1],
                                       dl_harq1,
                                       frame_rx,
                                       nb_symb_sch,
                                       nr_slot_rx,
                                       harq_pid,
                                       dlsch_bytes,
                                       p_b);
    LOG_T(PHY,"CW dlsch decoding, ret1 = %d\n", ret1);

    stop_meas(&ue->dlsch_decoding_stats);
    if (cpumeas(CPUMEAS_GETSTATE)) {
      LOG_D(PHY, " --> Unscrambling for CW1 %5.3f\n",
            (ue->dlsch_unscrambling_stats.p_time)/(cpuf*1000.0));
      LOG_D(PHY, "AbsSubframe %d.%d --> ldpc Decoding for CW1 %5.3f\n",
            frame_rx%1024, nr_slot_rx,(ue->dlsch_decoding_stats.p_time)/(cpuf*1000.0));
    }
    LOG_D(PHY, "harq_pid: %d, TBS expected dlsch1: %d \n", harq_pid, dl_harq1->TBS);
  }
#endif

  //  send to mac
  if (ue->if_inst && ue->if_inst->dl_indication) {
    ue->if_inst->dl_indication(&dl_indication);
  }

  if (ue->phy_sim_dlsch_b)
    memcpy(ue->phy_sim_dlsch_b, p_b, dlsch_bytes);

  return (ret < ue->max_ldpc_iterations + 1);
}

void prs_processing(const PHY_VARS_NR_UE *ue,
                    const UE_nr_rxtx_proc_t *proc,
                    const int symbol,
                    const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size])
{
  int nr_slot_rx = proc->nr_slot_rx;
  int frame_rx = proc->frame_rx;
  const NR_DL_FRAME_PARMS *fp = &ue->frame_parms;
  // Check for PRS slot - section 7.4.1.7.4 in 3GPP rel16 38.211
  for (int gNB_id = 0; gNB_id < ue->prs_active_gNBs; gNB_id++) {
    for (int rsc_id = 0; rsc_id < ue->prs_vars[gNB_id]->NumPRSResources; rsc_id++) {
      const prs_config_t *prs_config = &ue->prs_vars[gNB_id]->prs_resource[rsc_id].prs_cfg;
      prs_meas_t **prs_meas = ue->prs_vars[gNB_id]->prs_resource[rsc_id].prs_meas;
      c16_t *ch_est = ue->prs_vars[gNB_id]->prs_resource[rsc_id].ch_est;
      for (int i = 0; i < prs_config->PRSResourceRepetition; i++)
      {
        if( (((frame_rx*fp->slots_per_frame + nr_slot_rx) - (prs_config->PRSResourceSetPeriod[1] + prs_config->PRSResourceOffset) + prs_config->PRSResourceSetPeriod[0])%prs_config->PRSResourceSetPeriod[0]) == i*prs_config->PRSResourceTimeGap)
        {
          int last_prs_symbol = prs_config->SymbolStart + prs_config->NumPRSSymbols - 1;
          if ((symbol >= prs_config->SymbolStart) && (symbol < last_prs_symbol)) {
            nr_prs_channel_estimation(gNB_id, rsc_id, i, symbol, ue, proc, prs_config, rxdataF, prs_meas, ch_est);
          } else if (symbol == last_prs_symbol) {
            nr_prs_channel_estimation(gNB_id, rsc_id, i, symbol, ue, proc, prs_config, rxdataF, prs_meas, ch_est);
            nr_prs_doa_estimation(gNB_id, rsc_id, ue, proc, prs_config, ch_est, prs_meas);
          }
        }
      } // for i
    } // for rsc_id
  } // for gNB_id
}

int is_ssb_in_symbol(const fapi_nr_config_request_t *cfg,
                     const int symbIdxInFrame,
                     const int slot,
                     const NR_DL_FRAME_PARMS *fp,
                     const int ssbMask,
                     const int ssbIndex)
{
  /* Skip if current SSB index is not transmitted */
  if (!((ssbMask >> (31 - (ssbIndex % 32))) & 0x1)) {
    return false;
  }

  const int startPbchSymb = nr_get_ssb_start_symbol(fp, ssbIndex) + 1;
  const int startPbchSymbHf = (cfg->ssb_table.ssb_period == 0) ? (startPbchSymb + (fp->slots_per_frame * NR_SYMBOLS_PER_SLOT / 2))
                                                               : (fp->slots_per_frame * NR_SYMBOLS_PER_SLOT);

  /* Skip if no SSB in current symbol */
  if ((symbIdxInFrame >= startPbchSymb && symbIdxInFrame < (startPbchSymb + NB_SYMBOLS_PBCH))
      || (symbIdxInFrame >= startPbchSymbHf && symbIdxInFrame < (startPbchSymbHf + NB_SYMBOLS_PBCH))) {
    return true;
  }

  return false;
}

int get_ssb_index_in_symbol(const fapi_nr_config_request_t *cfg,
                            const NR_DL_FRAME_PARMS *fp,
                            const int symbIdxInFrame,
                            const int slot,
                            const int frame)
{
  /* checking if current frame is compatible with SSB periodicity */
  if (cfg->ssb_table.ssb_period != 0 && (frame % (1 << (cfg->ssb_table.ssb_period - 1)))) {
    return -1;
  }

  /* Find the SSB index corresponding to current symbol */
  for (int ssbIndex = 0; ssbIndex < fp->Lmax; ssbIndex++) {
    const int ssbMask = cfg->ssb_table.ssb_mask_list[ssbIndex / 32].ssb_mask;
    if (is_ssb_in_symbol(cfg, symbIdxInFrame, slot, fp, ssbMask, ssbIndex))
      return ssbIndex;
  }

  return -1;
}

/* Description: Generates PBCH LLRs from frequency domain signal for a OFDM symbol.
                Generates PBCH time domain channel response.
   Returns    : SSB index if symbol contains SSB. Else returns -1. */
int nr_process_pbch_symbol(PHY_VARS_NR_UE *ue,
                           const UE_nr_rxtx_proc_t *proc,
                           const int symbol,
                           const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
                           const int ssbIndexIn,
                           c16_t dl_ch_estimates_time[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
                           int16_t pbch_e_rx[NR_POLAR_PBCH_E])
{
  const fapi_nr_config_request_t *cfg = &ue->nrUE_config;
  const int symbIdxInFrame = symbol + NR_SYMBOLS_PER_SLOT * proc->nr_slot_rx;

  /* Search for SSB index if given SSB index is invalid */
  const int ssbIndex = (ssbIndexIn < 0)
                           ? get_ssb_index_in_symbol(cfg, &ue->frame_parms, symbIdxInFrame, proc->nr_slot_rx, proc->frame_rx)
                           : ssbIndexIn;

  if (ssbIndex < 0)
    return -1;

  LOG_D(PHY, "Frame %d, Slot %d, Symbol %d, SSB Index %d\n", proc->frame_rx, proc->nr_slot_rx, symbol, ssbIndex);
  const int startPbchSymb = nr_get_ssb_start_symbol(&ue->frame_parms, ssbIndex) + 1;
  const int startPbchSymbHf = startPbchSymb + (ue->frame_parms.slots_per_frame * NR_SYMBOLS_PER_SLOT / 2);

  /* Found PBCH. Process it */
  c16_t dl_ch_estimates[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size];

  const int relPbchSymb = (symbIdxInFrame > (ue->frame_parms.slots_per_frame * NR_SYMBOLS_PER_SLOT / 2))
                              ? (symbIdxInFrame - startPbchSymbHf)
                              : (symbIdxInFrame - startPbchSymb);

  for (int aarx = 0; aarx < ue->frame_parms.nb_antennas_rx; aarx++) {
    nr_pbch_channel_estimation(ue,
                               relPbchSymb,
                               ssbIndex & 7,
                               symbIdxInFrame > (ue->frame_parms.slots_per_frame * NR_SYMBOLS_PER_SLOT / 2),
                               rxdataF[aarx],
                               dl_ch_estimates[aarx]);
  }

  const int symbIdxInSSB = relPbchSymb + 1;
  nr_generate_pbch_llr(ue, symbIdxInSSB, ssbIndex, rxdataF, dl_ch_estimates, pbch_e_rx);
  /* Do measurements on middle symbol of PBCH block */
  if (relPbchSymb == 1) {
    nr_ue_ssb_rsrp_measurements(ue, ssbIndex, proc, rxdataF);
    nr_ue_rrc_measurements(ue, proc, rxdataF);
    /* resetting ssb index for PBCH detection if there is a stronger SSB index */
    if (ue->measurements.ssb_rsrp_dBm[ssbIndex] > ue->measurements.ssb_rsrp_dBm[ue->frame_parms.ssb_index]) {
      ue->frame_parms.ssb_index = ssbIndex;
    }
  }

  /* Get channel response to measure timing error */
  if ((ue->frame_parms.ssb_index == ssbIndex) && (relPbchSymb == NB_SYMBOLS_PBCH - 1)) {
    // do ifft of channel estimate
    const idft_size_idx_t idftsizeidx = get_idft(ue->frame_parms.ofdm_symbol_size);
    idft(idftsizeidx, (int16_t *)&dl_ch_estimates, (int16_t *)dl_ch_estimates_time, 1);

    UEscopeCopy(ue,
                pbchDlChEstimateTime,
                (void *)dl_ch_estimates_time,
                sizeof(c16_t),
                ue->frame_parms.nb_antennas_rx,
                ue->frame_parms.ofdm_symbol_size,
                0);
  }

  return ssbIndex;
}

// todo:
// - power control as per 38.213 ch 7.4
void nr_ue_prach_procedures(PHY_VARS_NR_UE *ue, const UE_nr_rxtx_proc_t *proc)
{
  int gNB_id = proc->gNB_id;
  int frame_tx = proc->frame_tx, nr_slot_tx = proc->nr_slot_tx, prach_power; // tx_amp
  uint8_t mod_id = ue->Mod_id;

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX_PRACH, VCD_FUNCTION_IN);

  if (ue->prach_vars[gNB_id]->active) {
    fapi_nr_ul_config_prach_pdu *prach_pdu = &ue->prach_vars[gNB_id]->prach_pdu;
    ue->tx_power_dBm[nr_slot_tx] = prach_pdu->prach_tx_power;

    LOG_D(PHY, "In %s: [UE %d][RAPROC][%d.%d]: Generating PRACH Msg1 (preamble %d, P0_PRACH %d)\n",
          __FUNCTION__,
          mod_id,
          frame_tx,
          nr_slot_tx,
          prach_pdu->ra_PreambleIndex,
          ue->tx_power_dBm[nr_slot_tx]);

    ue->prach_vars[gNB_id]->amp = AMP;

    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GENERATE_PRACH, VCD_FUNCTION_IN);

    prach_power = generate_nr_prach(ue, gNB_id, frame_tx, nr_slot_tx);

    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GENERATE_PRACH, VCD_FUNCTION_OUT);

    LOG_D(PHY, "In %s: [UE %d][RAPROC][%d.%d]: Generated PRACH Msg1 (TX power PRACH %d dBm, digital power %d dBW (amp %d)\n",
      __FUNCTION__,
      mod_id,
      frame_tx,
      nr_slot_tx,
      ue->tx_power_dBm[nr_slot_tx],
      dB_fixed(prach_power),
      ue->prach_vars[gNB_id]->amp);

    ue->prach_vars[gNB_id]->active = false;
  }

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX_PRACH, VCD_FUNCTION_OUT);

}
