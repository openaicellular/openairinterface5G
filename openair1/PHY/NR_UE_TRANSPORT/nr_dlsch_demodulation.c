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

/*! \file nr_dlsch_demodulation.c
 * \brief Top-level routines for demodulating the PDSCH physical channel from 38-211, V15.2 2018-06
 * \author H.Wang
 * \date 2018
 * \version 0.1
 * \company Eurecom
 * \note
 * \warning
 */
#include "PHY/defs_nr_UE.h"
#include "PHY/phy_extern.h"
#include "PHY/NR_TRANSPORT/nr_transport_proto.h"
#include "nr_transport_proto_ue.h"
#include "PHY/sse_intrin.h"
#include "T.h"
#include "openair1/PHY/NR_UE_ESTIMATION/nr_estimation.h"
#include "openair1/PHY/NR_TRANSPORT/nr_dlsch.h"
#include "PHY/NR_REFSIG/nr_refsig.h"
#include "PHY/NR_REFSIG/dmrs_nr.h"
#include "common/utils/nr/nr_common.h"
#include <complex.h>
#include "openair1/PHY/TOOLS/phy_scope_interface.h"
#include "nfapi/open-nFAPI/nfapi/public_inc/nfapi_nr_interface.h"

/* dynamic shift for LLR computation for TM3/4
 * set as command line argument, see lte-softmodem.c
 * default value: 0
 */
int32_t nr_dlsch_demod_shift = 0;
//int16_t interf_unaw_shift = 13;

// #define DEBUG_HARQ(a...) printf(a)
#define DEBUG_HARQ(...)
//#define DEBUG_DLSCH_DEMOD
//#define DEBUG_PDSCH_RX

// [MCS][i_mod (0,1,2) = (2,4,6)]
//unsigned char offset_mumimo_llr_drange_fix=0;
//inferference-free case
/*unsigned char interf_unaw_shift_tm4_mcs[29]={5, 3, 4, 3, 3, 2, 1, 1, 2, 0, 1, 1, 1, 1, 0, 0,
                                             1, 1, 1, 1, 0, 2, 1, 0, 1, 0, 1, 0, 0} ;*/

//unsigned char interf_unaw_shift_tm1_mcs[29]={5, 5, 4, 3, 3, 3, 2, 2, 4, 4, 2, 3, 3, 3, 1, 1,
//                                          0, 1, 1, 2, 5, 4, 4, 6, 5, 1, 0, 5, 6} ; // mcs 21, 26, 28 seem to be errorneous

/*
unsigned char offset_mumimo_llr_drange[29][3]={{8,8,8},{7,7,7},{7,7,7},{7,7,7},{6,6,6},{6,6,6},{6,6,6},{5,5,5},{4,4,4},{1,2,4}, // QPSK
{5,5,4},{5,5,5},{5,5,5},{3,3,3},{2,2,2},{2,2,2},{2,2,2}, // 16-QAM
{2,2,1},{3,3,3},{3,3,3},{3,3,1},{2,2,2},{2,2,2},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}; //64-QAM
*/
 /*
 //first optimization try
 unsigned char offset_mumimo_llr_drange[29][3]={{7, 8, 7},{6, 6, 7},{6, 6, 7},{6, 6, 6},{5, 6, 6},{5, 5, 6},{5, 5, 6},{4, 5, 4},{4, 3, 4},{3, 2, 2},{6, 5, 5},{5, 4, 4},{5, 5, 4},{3, 3, 2},{2, 2, 1},{2, 1, 1},{2, 2, 2},{3, 3, 3},{3, 3, 2},{3, 3, 2},{3, 2, 1},{2, 2, 2},{2, 2, 2},{0, 0, 0},{0, 0, 0},{0, 0, 0},{0, 0, 0},{0, 0, 0}};
 */
 //second optimization try
 /*
   unsigned char offset_mumimo_llr_drange[29][3]={{5, 8, 7},{4, 6, 8},{3, 6, 7},{7, 7, 6},{4, 7, 8},{4, 7, 4},{6, 6, 6},{3, 6, 6},{3, 6, 6},{1, 3, 4},{1, 1, 0},{3, 3, 2},{3, 4, 1},{4, 0, 1},{4, 2, 2},{3, 1, 2},{2, 1, 0},{2, 1, 1},{1, 0, 1},{1, 0, 1},{0, 0, 0},{1, 0, 0},{0, 0, 0},{0, 1, 0},{1, 0, 0},{0, 0, 0},{0, 0, 0},{0, 0, 0},{0, 0, 0}};  w
 */
//unsigned char offset_mumimo_llr_drange[29][3]= {{0, 6, 5},{0, 4, 5},{0, 4, 5},{0, 5, 4},{0, 5, 6},{0, 5, 3},{0, 4, 4},{0, 4, 4},{0, 3, 3},{0, 1, 2},{1, 1, 0},{1, 3, 2},{3, 4, 1},{2, 0, 0},{2, 2, 2},{1, 1, 1},{2, 1, 0},{2, 1, 1},{1, 0, 1},{1, 0, 1},{0, 0, 0},{1, 0, 0},{0, 0, 0},{0, 1, 0},{1, 0, 0},{0, 0, 0},{0, 0, 0},{0, 0, 0},{0, 0, 0}};

#define print_ints(s,x) printf("%s = %d %d %d %d\n",s,(x)[0],(x)[1],(x)[2],(x)[3])
#define print_shorts(s,x) printf("%s = [%d+j*%d, %d+j*%d, %d+j*%d, %d+j*%d]\n",s,(x)[0],(x)[1],(x)[2],(x)[3],(x)[4],(x)[5],(x)[6],(x)[7])

bool get_isPilot_symbol(const int symbol, const NR_UE_DLSCH_t *dlsch)
{
  bool pilot = (dlsch->dlsch_config.dlDmrsSymbPos >> symbol) & 1;
  return pilot;
}

int get_nb_re_pdsch_symbol(const int symbol, const NR_UE_DLSCH_t *dlsch)
{
  bool isPilot = get_isPilot_symbol(symbol, dlsch);
  int config_type = dlsch->dlsch_config.dmrsConfigType;
  int nb_rb_pdsch = dlsch->dlsch_config.number_rbs;
  int nb_re_pdsch = (isPilot)
                        ? ((config_type == NFAPI_NR_DMRS_TYPE1) ? nb_rb_pdsch * (12 - 6 * dlsch->dlsch_config.n_dmrs_cdm_groups)
                                                                : nb_rb_pdsch * (12 - 4 * dlsch->dlsch_config.n_dmrs_cdm_groups))
                        : (nb_rb_pdsch * 12);
  return nb_re_pdsch;
}

void compute_dl_valid_re(const NR_UE_DLSCH_t *dlsch, const int32_t ptrs_re[][NR_SYMBOLS_PER_SLOT], int ret[NR_SYMBOLS_PER_SLOT])
{
  const int start_symb = dlsch->dlsch_config.start_symbol;
  const int numb_symb = dlsch->dlsch_config.number_symbols;

  for (int i = start_symb; i < start_symb + numb_symb; i++) {
    ret[i] = get_nb_re_pdsch_symbol(i, dlsch) - ptrs_re[0][i];
  }
}

int get_max_llr_per_symbol(const NR_UE_DLSCH_t *dlsch)
{
  const int start_symb = dlsch->dlsch_config.start_symbol;
  const int numb_symb = dlsch->dlsch_config.number_symbols;
  int tmp = -1;
  int nb_llr = -1;

  for (int i = start_symb; i < start_symb + numb_symb; i++) {
    tmp = get_nb_re_pdsch_symbol(i, dlsch) * dlsch->dlsch_config.qamModOrder;
    if (tmp > nb_llr)
      nb_llr = tmp;
  }

  return nb_llr;
}

int get_pdcch_max_rbs(const NR_UE_PDCCH_CONFIG *phy_pdcch_config)
{
  int nb_rb = 0;
  int rb_offset = 0;
  for (int i = 0; i < phy_pdcch_config->nb_search_space; i++) {
    int tmp = 0;
    get_coreset_rballoc(phy_pdcch_config->pdcch_config[i].coreset.frequency_domain_resource, &tmp, &rb_offset);
    if (tmp > nb_rb)
      nb_rb = tmp;
  }
  return nb_rb;
}

int get_max_pdcch_symb(const NR_UE_PDCCH_CONFIG *phy_pdcch_config)
{
  int max_pdcch_symb = 0;
  for (int i = 0; i < phy_pdcch_config->nb_search_space; i++)
    if (phy_pdcch_config->pdcch_config[i].coreset.duration > max_pdcch_symb)
      max_pdcch_symb = phy_pdcch_config->pdcch_config[i].coreset.duration;

  return max_pdcch_symb;
}

void nr_dlsch_deinterleaving(uint8_t symbol,
                             uint8_t start_symbol,
                             uint16_t L,
                             uint16_t *llr,
                             uint16_t *llr_deint,
                             uint16_t nb_rb_pdsch)
{

  uint32_t bundle_idx, N_bundle, R, C, r,c;
  int32_t m,k;
  uint8_t nb_re;

  R=2;
  N_bundle = nb_rb_pdsch/L;
  C=N_bundle/R;

  uint32_t bundle_deint[N_bundle];
  memset(bundle_deint, 0 , sizeof(bundle_deint));

  printf("N_bundle %u L %d nb_rb_pdsch %d\n",N_bundle, L,nb_rb_pdsch);

  if (symbol==start_symbol)
	  nb_re = 6;
  else
	  nb_re = 12;


  AssertFatal(llr!=NULL,"nr_dlsch_deinterleaving: FATAL llr is Null\n");


  for (c =0; c< C; c++){
	  for (r=0; r<R;r++){
		  bundle_idx = r*C+c;
		  bundle_deint[bundle_idx] = c*R+r;
		  //printf("c %u r %u bundle_idx %u bundle_deinter %u\n", c, r, bundle_idx, bundle_deint[bundle_idx]);
	  }
  }

  for (k=0; k<N_bundle;k++)
  {
	  for (m=0; m<nb_re*L;m++){
		  llr_deint[bundle_deint[k]*nb_re*L+m]= llr[k*nb_re*L+m];
		  //printf("k %d m %d bundle_deint %d llr_deint %d\n", k, m, bundle_deint[k], llr_deint[bundle_deint[k]*nb_re*L+m]);
	  }
  }
}

static void nr_channel_compensation(const int length,
                                    const int nb_rb,
                                    const int output_shift,
                                    const int mod_order,
                                    const c16_t dl_ch_estimates_ext[nb_rb * NR_NB_SC_PER_RB],
                                    const c16_t rxdataF_ext[nb_rb * NR_NB_SC_PER_RB],
                                    c16_t dl_ch_mag[nb_rb * NR_NB_SC_PER_RB],
                                    c16_t dl_ch_magb[nb_rb * NR_NB_SC_PER_RB],
                                    c16_t dl_ch_magr[nb_rb * NR_NB_SC_PER_RB],
                                    c16_t rxdataF_comp[nb_rb * NR_NB_SC_PER_RB]);

/* Do channel compenstion on all antennas */
static void nr_pdsch_channel_compensation(const NR_DL_FRAME_PARMS *frame_parms,
                                          const NR_UE_DLSCH_t *dlsch,
                                          const int symbol,
                                          const c16_t rxdataF_ext[frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
                                          const c16_t dl_ch_est_ext[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
                                          c16_t dl_ch_mag[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
                                          c16_t dl_ch_magb[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
                                          c16_t dl_ch_magr[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
                                          c16_t rxdataF_comp[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB])
{
  const int log2_maxh = get_maxh_extimates(frame_parms, dlsch, symbol, dl_ch_est_ext);
  for (int l = 0; l < dlsch->Nl; l++) {
    for (int aarx = 0; aarx < frame_parms->nb_antennas_rx; aarx++) {
      nr_channel_compensation(get_nb_re_pdsch_symbol(symbol, dlsch),
                              dlsch->dlsch_config.number_rbs,
                              log2_maxh,
                              dlsch->dlsch_config.qamModOrder,
                              dl_ch_est_ext[l][aarx],
                              rxdataF_ext[aarx],
                              dl_ch_mag[l][aarx],
                              dl_ch_magb[l][aarx],
                              dl_ch_magr[l][aarx],
                              rxdataF_comp[l][aarx]);
    }
  }
}

/* Extract channel estimates for corresponding data REs in a OFDM symbol for all antennas */
void nr_generate_pdsch_extracted_chestimates(
    const PHY_VARS_NR_UE *ue,
    const NR_UE_DLSCH_t *dlsch,
    const int symbol,
    const c16_t dl_ch_est[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
    c16_t dl_ch_est_ext[dlsch->Nl][ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB])
{
  const NR_DL_FRAME_PARMS *frame_parms = &ue->frame_parms;
  const int nbAntRx = frame_parms->nb_antennas_rx;
  const int dlDmrsSymbPos = dlsch->dlsch_config.dlDmrsSymbPos;
  /* TODO: Could be launched in Tpool for MIMO */
  const int validDmrsEst = (ue->chest_time == 0) ? get_valid_dmrs_idx_for_channel_est(dlDmrsSymbPos, symbol)
                                                 : get_next_dmrs_symbol_in_slot(dlDmrsSymbPos, 0, 14);
  for (int aarx = 0; aarx < nbAntRx; aarx++) {
    for (int l = 0; l < dlsch->Nl; l++) {
      /* Extract estimates for all symbols */
      nr_extract_pdsch_chest_res(frame_parms,
                                 &dlsch->dlsch_config,
                                 get_isPilot_symbol(symbol, dlsch),
                                 dl_ch_est[validDmrsEst][l][aarx],
                                 dl_ch_est_ext[l][aarx]);
    }
  }
}

/* Do channel scaling on all antennas */
void nr_pdsch_channel_level_scaling(
    const NR_DL_FRAME_PARMS *frame_parms,
    const NR_UE_DLSCH_t *dlsch,
    const int symbol,
    c16_t dl_ch_est_ext[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB])
{
  const int nl = dlsch->Nl;
  const int nbAntRx = frame_parms->nb_antennas_rx;
  for (int l = 0; l < nl; l++) {
    for (int aarx = 0; aarx < nbAntRx; aarx++) {
      nr_scale_channel(get_nb_re_pdsch_symbol(symbol, dlsch),
                       dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB,
                       dl_ch_est_ext[l][aarx]);
    }
  }
}

/* Do PTRS estimation on all antennas */
void nr_pdsch_ptrs_processing(
    const PHY_VARS_NR_UE *ue,
    const int gNB_id,
    const int rnti,
    const int slot,
    const int symbol,
    const NR_UE_DLSCH_t *dlsch,
    c16_t rxdataF_comp[dlsch->Nl][ue->frame_parms.nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
    c16_t ptrs_phase[ue->frame_parms.nb_antennas_rx][NR_SYMBOLS_PER_SLOT],
    int32_t ptrs_re[ue->frame_parms.nb_antennas_rx][NR_SYMBOLS_PER_SLOT])
{
  for (int aarx = 0; aarx < ue->frame_parms.nb_antennas_rx; aarx++) {
    nr_pdsch_ptrs_processing_core(ue,
                                  gNB_id,
                                  slot,
                                  symbol,
                                  get_nb_re_pdsch_symbol(symbol, dlsch),
                                  rnti,
                                  dlsch,
                                  rxdataF_comp[0][aarx],
                                  ptrs_phase[aarx] + symbol,
                                  ptrs_re[aarx] + symbol);
  }
}

void nr_pdsch_comp_out(void *parms)
{
  nr_ue_symb_data_t *msg = (nr_ue_symb_data_t *)parms;
  const PHY_VARS_NR_UE *ue = msg->UE;
  const UE_nr_rxtx_proc_t *proc = msg->proc;
  const NR_UE_DLSCH_t *dlsch = msg->p_dlsch;
  const int symbolVecSize = dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB;
  const c16_t(*dl_ch_est)[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size] =
      (const c16_t(*)[NR_SYMBOLS_PER_SLOT][dlsch->Nl][ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size])
          msg->pdsch_dl_ch_estimates;

  c16_t(*dl_ch_est_ext)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][symbolVecSize] =
      (c16_t(*)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][symbolVecSize])msg->pdsch_dl_ch_est_ext;

  nr_generate_pdsch_extracted_chestimates(ue, dlsch, msg->symbol, *dl_ch_est, *dl_ch_est_ext);
  nr_pdsch_channel_level_scaling(&ue->frame_parms, dlsch, msg->symbol, *dl_ch_est_ext);
  const int maxh = get_maxh_extimates(&ue->frame_parms, dlsch, msg->symbol, *dl_ch_est_ext);

  const c16_t(*rxdataF_ext)[ue->frame_parms.nb_antennas_rx][symbolVecSize] =
      (const c16_t(*)[ue->frame_parms.nb_antennas_rx][symbolVecSize])msg->rxdataF_ext;

  c16_t(*dl_ch_mag)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][symbolVecSize] =
      (c16_t(*)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][symbolVecSize])msg->dl_ch_mag;

  c16_t(*dl_ch_magb)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][symbolVecSize] =
      (c16_t(*)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][symbolVecSize])msg->dl_ch_magb;

  c16_t(*dl_ch_magr)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][symbolVecSize] =
      (c16_t(*)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][symbolVecSize])msg->dl_ch_magr;

  c16_t(*rxdataF_comp)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][symbolVecSize] =
      (c16_t(*)[dlsch->Nl][ue->frame_parms.nb_antennas_rx][symbolVecSize])msg->rxdataF_comp;

  nr_pdsch_channel_compensation(&ue->frame_parms,
                                dlsch,
                                msg->symbol,
                                *rxdataF_ext,
                                *dl_ch_est_ext,
                                *dl_ch_mag,
                                *dl_ch_magb,
                                *dl_ch_magr,
                                *rxdataF_comp);

  if (ue->frame_parms.nb_antennas_rx > 1) {
    const int nb_re_pdsch = get_nb_re_pdsch_symbol(msg->symbol, dlsch);
    nr_dlsch_detection_mrc(dlsch->Nl,
                           ue->frame_parms.nb_antennas_rx,
                           dlsch->dlsch_config.number_rbs,
                           nb_re_pdsch,
                           *rxdataF_comp,
                           *dl_ch_mag,
                           *dl_ch_magb,
                           *dl_ch_magr);
    if (dlsch->Nl >= 2) {
      nr_zero_forcing_rx(dlsch->Nl,
                         ue->frame_parms.nb_antennas_rx,
                         dlsch->dlsch_config.number_rbs,
                         nb_re_pdsch,
                         dlsch->dlsch_config.qamModOrder,
                         maxh,
                         *dl_ch_est_ext,
                         *rxdataF_comp,
                         *dl_ch_mag,
                         *dl_ch_magb,
                         *dl_ch_magr);
    }
  }

  c16_t(*ptrs_phase)[ue->frame_parms.nb_antennas_rx][NR_SYMBOLS_PER_SLOT] =
      (c16_t(*)[ue->frame_parms.nb_antennas_rx][NR_SYMBOLS_PER_SLOT])msg->ptrs_phase_per_slot;

  int32_t(*ptrs_re)[ue->frame_parms.nb_antennas_rx][NR_SYMBOLS_PER_SLOT] =
      (int32_t(*)[ue->frame_parms.nb_antennas_rx][NR_SYMBOLS_PER_SLOT])msg->ptrs_re_per_slot;

  if ((dlsch->dlsch_config.pduBitmap & 0x1) && (dlsch->rnti_type == TYPE_C_RNTI_)) {
    nr_pdsch_ptrs_processing(ue,
                             proc->gNB_id,
                             dlsch->rnti,
                             proc->nr_slot_rx,
                             msg->symbol,
                             dlsch,
                             *rxdataF_comp,
                             *ptrs_phase,
                             *ptrs_re);
  }
}

void nr_compute_channel_correlation(const int n_layers,
                                    const int length,
                                    const int nb_rb,
                                    const int nb_antennas_rx,
                                    const int antIdx,
                                    const int output_shift,
                                    const c16_t dl_ch_estimates_ext[n_layers][nb_antennas_rx][nb_rb * NR_NB_SC_PER_RB],
                                    int32_t rho[n_layers][n_layers][nb_rb * NR_NB_SC_PER_RB])
{
  // we compute the Tx correlation matrix for each Rx antenna
  // As an example the 2x2 MIMO case requires
  // rho[aarx][nl*nl] = [cov(H_aarx_0,H_aarx_0) cov(H_aarx_0,H_aarx_1)
  //                               cov(H_aarx_1,H_aarx_0) cov(H_aarx_1,H_aarx_1)], aarx=0,...,nb_antennas_rx-1

  // int avg_rho_re[frame_parms->nb_antennas_rx][nl*nl];
  // int avg_rho_im[frame_parms->nb_antennas_rx][nl*nl];

  const int nb_rb_0 = length / 12 + ((length % 12) ? 1 : 0);
  for (int l = 0; l < n_layers; l++) {
    for (int atx = 0; atx < n_layers; atx++) {
      // avg_rho_re[aarx][l*n_layers+atx] = 0;
      // avg_rho_im[aarx][l*n_layers+atx] = 0;
      simde__m128i *rho128 = (simde__m128i *)rho[l][atx];
      simde__m128i *dl_ch128 = (simde__m128i *)dl_ch_estimates_ext[l][antIdx];
      simde__m128i *dl_ch128_2 = (simde__m128i *)dl_ch_estimates_ext[atx][antIdx];

      for (int rb = 0; rb < nb_rb_0; rb++) {
        // multiply by conjugated channel
        simde__m128i mmtmpD0 = simde_mm_madd_epi16(dl_ch128[0], dl_ch128_2[0]);
        //  print_ints("re",&mmtmpD0);
        // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
        simde__m128i mmtmpD1 = simde_mm_shufflelo_epi16(dl_ch128[0], SIMDE_MM_SHUFFLE(2, 3, 0, 1));
        mmtmpD1 = simde_mm_shufflehi_epi16(mmtmpD1, SIMDE_MM_SHUFFLE(2, 3, 0, 1));
        mmtmpD1 = simde_mm_sign_epi16(mmtmpD1, *(simde__m128i *)&conjugate[0]);
        //  print_ints("im",&mmtmpD1);
        mmtmpD1 = simde_mm_madd_epi16(mmtmpD1, dl_ch128_2[0]);
        // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
        mmtmpD0 = simde_mm_srai_epi32(mmtmpD0, output_shift);
        //  print_ints("re(shift)",&mmtmpD0);
        mmtmpD1 = simde_mm_srai_epi32(mmtmpD1, output_shift);
        //  print_ints("im(shift)",&mmtmpD1);
        simde__m128i mmtmpD2 = simde_mm_unpacklo_epi32(mmtmpD0, mmtmpD1);
        simde__m128i mmtmpD3 = simde_mm_unpackhi_epi32(mmtmpD0, mmtmpD1);
        //        print_ints("c0",&mmtmpD2);
        //  print_ints("c1",&mmtmpD3);
        rho128[0] = simde_mm_packs_epi32(mmtmpD2, mmtmpD3);
        // print_shorts("rx:",dl_ch128_2);
        // print_shorts("ch:",dl_ch128);
        // print_shorts("pack:",rho128);

        /*avg_rho_re[aarx][l*n_layers+atx] +=(((int16_t*)&rho128[0])[0]+
          ((int16_t*)&rho128[0])[2] +
          ((int16_t*)&rho128[0])[4] +
          ((int16_t*)&rho128[0])[6])/16;*/
        /*avg_rho_im[aarx][l*n_layers+atx] +=(((int16_t*)&rho128[0])[1]+
          ((int16_t*)&rho128[0])[3] +
          ((int16_t*)&rho128[0])[5] +
          ((int16_t*)&rho128[0])[7])/16;*/

        // multiply by conjugated channel
        mmtmpD0 = simde_mm_madd_epi16(dl_ch128[1], dl_ch128_2[1]);
        // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
        mmtmpD1 = simde_mm_shufflelo_epi16(dl_ch128[1], SIMDE_MM_SHUFFLE(2, 3, 0, 1));
        mmtmpD1 = simde_mm_shufflehi_epi16(mmtmpD1, SIMDE_MM_SHUFFLE(2, 3, 0, 1));
        mmtmpD1 = simde_mm_sign_epi16(mmtmpD1, *(simde__m128i *)conjugate);
        mmtmpD1 = simde_mm_madd_epi16(mmtmpD1, dl_ch128_2[1]);
        // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
        mmtmpD0 = simde_mm_srai_epi32(mmtmpD0, output_shift);
        mmtmpD1 = simde_mm_srai_epi32(mmtmpD1, output_shift);
        mmtmpD2 = simde_mm_unpacklo_epi32(mmtmpD0, mmtmpD1);
        mmtmpD3 = simde_mm_unpackhi_epi32(mmtmpD0, mmtmpD1);
        rho128[1] = simde_mm_packs_epi32(mmtmpD2, mmtmpD3);
        // print_shorts("rx:",dl_ch128_2+1);
        // print_shorts("ch:",dl_ch128+1);
        // print_shorts("pack:",rho128+1);

        // multiply by conjugated channel
        /*avg_rho_re[aarx][l*n_layers+atx] +=(((int16_t*)&rho128[1])[0]+
          ((int16_t*)&rho128[1])[2] +
          ((int16_t*)&rho128[1])[4] +
          ((int16_t*)&rho128[1])[6])/16;*/
        /*avg_rho_im[aarx][l*n_layers+atx] +=(((int16_t*)&rho128[1])[1]+
          ((int16_t*)&rho128[1])[3] +
          ((int16_t*)&rho128[1])[5] +
          ((int16_t*)&rho128[1])[7])/16;*/

        mmtmpD0 = simde_mm_madd_epi16(dl_ch128[2], dl_ch128_2[2]);
        // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
        mmtmpD1 = simde_mm_shufflelo_epi16(dl_ch128[2], SIMDE_MM_SHUFFLE(2, 3, 0, 1));
        mmtmpD1 = simde_mm_shufflehi_epi16(mmtmpD1, SIMDE_MM_SHUFFLE(2, 3, 0, 1));
        mmtmpD1 = simde_mm_sign_epi16(mmtmpD1, *(simde__m128i *)conjugate);
        mmtmpD1 = simde_mm_madd_epi16(mmtmpD1, dl_ch128_2[2]);
        // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
        mmtmpD0 = simde_mm_srai_epi32(mmtmpD0, output_shift);
        mmtmpD1 = simde_mm_srai_epi32(mmtmpD1, output_shift);
        mmtmpD2 = simde_mm_unpacklo_epi32(mmtmpD0, mmtmpD1);
        mmtmpD3 = simde_mm_unpackhi_epi32(mmtmpD0, mmtmpD1);

        rho128[2] = simde_mm_packs_epi32(mmtmpD2, mmtmpD3);
        // print_shorts("rx:",dl_ch128_2+2);
        // print_shorts("ch:",dl_ch128+2);
        // print_shorts("pack:",rho128+2);

        /*avg_rho_re[aarx][l*n_layers+atx] +=(((int16_t*)&rho128[2])[0]+
          ((int16_t*)&rho128[2])[2] +
          ((int16_t*)&rho128[2])[4] +
          ((int16_t*)&rho128[2])[6])/16;*/
        /*avg_rho_im[aarx][l*n_layers+atx] +=(((int16_t*)&rho128[2])[1]+
          ((int16_t*)&rho128[2])[3] +
          ((int16_t*)&rho128[2])[5] +
          ((int16_t*)&rho128[2])[7])/16;*/

        dl_ch128 += 3;
        dl_ch128_2 += 3;
        rho128 += 3;
      }
    }
  }
}

//==============================================================================================
// Pre-processing for LLR computation
//==============================================================================================

static void nr_channel_compensation(const int length,
                                    const int nb_rb,
                                    const int output_shift,
                                    const int mod_order,
                                    const c16_t dl_ch_estimates_ext[nb_rb * NR_NB_SC_PER_RB],
                                    const c16_t rxdataF_ext[nb_rb * NR_NB_SC_PER_RB],
                                    c16_t dl_ch_mag[nb_rb * NR_NB_SC_PER_RB],
                                    c16_t dl_ch_magb[nb_rb * NR_NB_SC_PER_RB],
                                    c16_t dl_ch_magr[nb_rb * NR_NB_SC_PER_RB],
                                    c16_t rxdataF_comp[nb_rb * NR_NB_SC_PER_RB])
{
  simde__m128i QAM_amp128 = {0};
  simde__m128i QAM_amp128b = {0};
  simde__m128i QAM_amp128r = {0};

  if (mod_order == 4) {
    QAM_amp128 = simde_mm_set1_epi16(QAM16_n1); // 2/sqrt(10)
    QAM_amp128b = simde_mm_setzero_si128();
    QAM_amp128r = simde_mm_setzero_si128();
  } else if (mod_order == 6) {
    QAM_amp128 = simde_mm_set1_epi16(QAM64_n1); //
    QAM_amp128b = simde_mm_set1_epi16(QAM64_n2);
    QAM_amp128r = simde_mm_setzero_si128();
  } else if (mod_order == 8) {
    QAM_amp128 = simde_mm_set1_epi16(QAM256_n1);
    QAM_amp128b = simde_mm_set1_epi16(QAM256_n2);
    QAM_amp128r = simde_mm_set1_epi16(QAM256_n3);
  }

  const int nb_rb_0 = length / 12 + ((length % 12) ? 1 : 0);
  const simde__m128i *dl_ch128 = (simde__m128i *)dl_ch_estimates_ext;
  const simde__m128i *rxdataF128 = (simde__m128i *)rxdataF_ext;
  simde__m128i *dl_ch_mag128 = (simde__m128i *)dl_ch_mag;
  simde__m128i *dl_ch_mag128b = (simde__m128i *)dl_ch_magb;
  simde__m128i *dl_ch_mag128r = (simde__m128i *)dl_ch_magr;
  simde__m128i *rxdataF_comp128 = (simde__m128i *)rxdataF_comp;
  for (int rb = 0; rb < nb_rb_0; rb++) {
    simde__m128i mmtmpD0;
    simde__m128i mmtmpD1;

    if (mod_order > 2) {
      // get channel amplitude if not QPSK

      mmtmpD0 = simde_mm_madd_epi16(dl_ch128[0], dl_ch128[0]);
      mmtmpD0 = simde_mm_srai_epi32(mmtmpD0, output_shift);

      mmtmpD1 = simde_mm_madd_epi16(dl_ch128[1], dl_ch128[1]);
      mmtmpD1 = simde_mm_srai_epi32(mmtmpD1, output_shift);

      mmtmpD0 = simde_mm_packs_epi32(mmtmpD0, mmtmpD1); //|H[0]|^2 |H[1]|^2 |H[2]|^2 |H[3]|^2 |H[4]|^2 |H[5]|^2 |H[6]|^2 |H[7]|^2

      // store channel magnitude here in a new field of dlsch

      dl_ch_mag128[0] = simde_mm_unpacklo_epi16(mmtmpD0, mmtmpD0);
      dl_ch_mag128b[0] = dl_ch_mag128[0];
      dl_ch_mag128r[0] = dl_ch_mag128[0];
      dl_ch_mag128[0] = simde_mm_mulhi_epi16(dl_ch_mag128[0], QAM_amp128);
      dl_ch_mag128[0] = simde_mm_slli_epi16(dl_ch_mag128[0], 1);

      dl_ch_mag128b[0] = simde_mm_mulhi_epi16(dl_ch_mag128b[0], QAM_amp128b);
      dl_ch_mag128b[0] = simde_mm_slli_epi16(dl_ch_mag128b[0], 1);

      dl_ch_mag128r[0] = simde_mm_mulhi_epi16(dl_ch_mag128r[0], QAM_amp128r);
      dl_ch_mag128r[0] = simde_mm_slli_epi16(dl_ch_mag128r[0], 1);

      // print_ints("Re(ch):",(int16_t*)&mmtmpD0);
      // print_shorts("QAM_amp:",(int16_t*)&QAM_amp128);
      // print_shorts("mag:",(int16_t*)&dl_ch_mag128[0]);
      dl_ch_mag128[1] = simde_mm_unpackhi_epi16(mmtmpD0, mmtmpD0);
      dl_ch_mag128b[1] = dl_ch_mag128[1];
      dl_ch_mag128r[1] = dl_ch_mag128[1];
      dl_ch_mag128[1] = simde_mm_mulhi_epi16(dl_ch_mag128[1], QAM_amp128);
      dl_ch_mag128[1] = simde_mm_slli_epi16(dl_ch_mag128[1], 1);

      dl_ch_mag128b[1] = simde_mm_mulhi_epi16(dl_ch_mag128b[1], QAM_amp128b);
      dl_ch_mag128b[1] = simde_mm_slli_epi16(dl_ch_mag128b[1], 1);

      dl_ch_mag128r[1] = simde_mm_mulhi_epi16(dl_ch_mag128r[1], QAM_amp128r);
      dl_ch_mag128r[1] = simde_mm_slli_epi16(dl_ch_mag128r[1], 1);

      mmtmpD0 =
          simde_mm_madd_epi16(dl_ch128[2], dl_ch128[2]); //[H_I(0)^2+H_Q(0)^2 H_I(1)^2+H_Q(1)^2 H_I(2)^2+H_Q(2)^2 H_I(3)^2+H_Q(3)^2]
      mmtmpD0 = simde_mm_srai_epi32(mmtmpD0, output_shift);
      mmtmpD1 = simde_mm_packs_epi32(mmtmpD0, mmtmpD0); //[|H(0)|^2 |H(1)|^2 |H(2)|^2 |H(3)|^2 |H(0)|^2 |H(1)|^2 |H(2)|^2 |H(3)|^2]

      dl_ch_mag128[2] =
          simde_mm_unpacklo_epi16(mmtmpD1, mmtmpD1); //[|H(0)|^2 |H(0)|^2 |H(1)|^2 |H(1)|^2 |H(2)|^2 |H(2)|^2 |H(3)|^2 |H(3)|^2]
      dl_ch_mag128b[2] = dl_ch_mag128[2];
      dl_ch_mag128r[2] = dl_ch_mag128[2];

      dl_ch_mag128[2] = simde_mm_mulhi_epi16(dl_ch_mag128[2], QAM_amp128);
      dl_ch_mag128[2] = simde_mm_slli_epi16(dl_ch_mag128[2], 1);

      dl_ch_mag128b[2] = simde_mm_mulhi_epi16(dl_ch_mag128b[2], QAM_amp128b);
      dl_ch_mag128b[2] = simde_mm_slli_epi16(dl_ch_mag128b[2], 1);

      dl_ch_mag128r[2] = simde_mm_mulhi_epi16(dl_ch_mag128r[2], QAM_amp128r);
      dl_ch_mag128r[2] = simde_mm_slli_epi16(dl_ch_mag128r[2], 1);
    }

    // multiply by conjugated channel
    mmtmpD0 = simde_mm_madd_epi16(dl_ch128[0], rxdataF128[0]);
    //  print_ints("re",&mmtmpD0);

    // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
    mmtmpD1 = simde_mm_shufflelo_epi16(dl_ch128[0], SIMDE_MM_SHUFFLE(2, 3, 0, 1));
    mmtmpD1 = simde_mm_shufflehi_epi16(mmtmpD1, SIMDE_MM_SHUFFLE(2, 3, 0, 1));
    mmtmpD1 = simde_mm_sign_epi16(mmtmpD1, *(simde__m128i *)&conjugate[0]);
    //  print_ints("im",&mmtmpD1);
    mmtmpD1 = simde_mm_madd_epi16(mmtmpD1, rxdataF128[0]);
    // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
    mmtmpD0 = simde_mm_srai_epi32(mmtmpD0, output_shift);
    //  print_ints("re(shift)",&mmtmpD0);
    mmtmpD1 = simde_mm_srai_epi32(mmtmpD1, output_shift);
    //  print_ints("im(shift)",&mmtmpD1);
    simde__m128i mmtmpD2 = simde_mm_unpacklo_epi32(mmtmpD0, mmtmpD1);
    simde__m128i mmtmpD3 = simde_mm_unpackhi_epi32(mmtmpD0, mmtmpD1);
    //        print_ints("c0",&mmtmpD2);
    //  print_ints("c1",&mmtmpD3);
    rxdataF_comp128[0] = simde_mm_packs_epi32(mmtmpD2, mmtmpD3);

#ifdef DEBUG_DLSCH_DEMOD
    printf("rb_index %d shift %d\n", rb, output_shift);
    printf("rxF");
    print_shorts("  ", (int16_t *)&rxdataF128[0]);
    printf("ch");
    print_shorts("  ", (int16_t *)&dl_ch128[0]);
    printf("rx_comp");
    print_shorts("  ", (int16_t *)&rxdataF_comp128[0]);
#endif

    // multiply by conjugated channel
    mmtmpD0 = simde_mm_madd_epi16(dl_ch128[1], rxdataF128[1]);
    // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
    mmtmpD1 = simde_mm_shufflelo_epi16(dl_ch128[1], SIMDE_MM_SHUFFLE(2, 3, 0, 1));
    mmtmpD1 = simde_mm_shufflehi_epi16(mmtmpD1, SIMDE_MM_SHUFFLE(2, 3, 0, 1));
    mmtmpD1 = simde_mm_sign_epi16(mmtmpD1, *(simde__m128i *)conjugate);
    mmtmpD1 = simde_mm_madd_epi16(mmtmpD1, rxdataF128[1]);
    // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
    mmtmpD0 = simde_mm_srai_epi32(mmtmpD0, output_shift);
    mmtmpD1 = simde_mm_srai_epi32(mmtmpD1, output_shift);
    mmtmpD2 = simde_mm_unpacklo_epi32(mmtmpD0, mmtmpD1);
    mmtmpD3 = simde_mm_unpackhi_epi32(mmtmpD0, mmtmpD1);

    rxdataF_comp128[1] = simde_mm_packs_epi32(mmtmpD2, mmtmpD3);
#ifdef DEBUG_DLSCH_DEMOD
    print_shorts("rx:", (int16_t *)&rxdataF128[1]);
    print_shorts("ch:", (int16_t *)&dl_ch128[1]);
    print_shorts("pack:", (int16_t *)&rxdataF_comp128[1]);
#endif

    // multiply by conjugated channel
    mmtmpD0 = simde_mm_madd_epi16(dl_ch128[2], rxdataF128[2]);
    // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
    mmtmpD1 = simde_mm_shufflelo_epi16(dl_ch128[2], SIMDE_MM_SHUFFLE(2, 3, 0, 1));
    mmtmpD1 = simde_mm_shufflehi_epi16(mmtmpD1, SIMDE_MM_SHUFFLE(2, 3, 0, 1));
    mmtmpD1 = simde_mm_sign_epi16(mmtmpD1, *(simde__m128i *)conjugate);
    mmtmpD1 = simde_mm_madd_epi16(mmtmpD1, rxdataF128[2]);
    // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
    mmtmpD0 = simde_mm_srai_epi32(mmtmpD0, output_shift);
    mmtmpD1 = simde_mm_srai_epi32(mmtmpD1, output_shift);
    mmtmpD2 = simde_mm_unpacklo_epi32(mmtmpD0, mmtmpD1);
    mmtmpD3 = simde_mm_unpackhi_epi32(mmtmpD0, mmtmpD1);

    rxdataF_comp128[2] = simde_mm_packs_epi32(mmtmpD2, mmtmpD3);
#ifdef DEBUG_DLSCH_DEMOD
    print_shorts("rx:", (int16_t *)&rxdataF128[2]);
    print_shorts("ch:", (int16_t *)&dl_ch128[2]);
    print_shorts("pack:", (int16_t *)&rxdataF_comp128[2]);
#endif

    dl_ch128 += 3;
    dl_ch_mag128 += 3;
    dl_ch_mag128b += 3;
    dl_ch_mag128r += 3;
    rxdataF128 += 3;
    rxdataF_comp128 += 3;
  }
}

void nr_scale_channel(const int len, const int extSize, c16_t dl_ch_estimates_ext[extSize])
{
  const int nb_rb_0 = len / 12 + ((len % 12) ? 1 : 0);

  // Determine scaling amplitude based the symbol

  const int ch_amp = 1024 * 8; //((pilots) ? (dlsch_ue[0]->sqrt_rho_b) : (dlsch_ue[0]->sqrt_rho_a));

  const simde__m128i ch_amp128 = simde_mm_set1_epi16(ch_amp); // Q3.13

  simde__m128i *dl_ch128 = (simde__m128i *)dl_ch_estimates_ext;

  for (int rb = 0; rb < nb_rb_0; rb++) {
    dl_ch128[0] = simde_mm_mulhi_epi16(dl_ch128[0], ch_amp128);
    dl_ch128[0] = simde_mm_slli_epi16(dl_ch128[0], 3);

    dl_ch128[1] = simde_mm_mulhi_epi16(dl_ch128[1], ch_amp128);
    dl_ch128[1] = simde_mm_slli_epi16(dl_ch128[1], 3);

    dl_ch128[2] = simde_mm_mulhi_epi16(dl_ch128[2], ch_amp128);
    dl_ch128[2] = simde_mm_slli_epi16(dl_ch128[2], 3);
    dl_ch128 += 3;
  }

  simde_mm_empty();
  simde_m_empty();
}


//compute average channel_level on each (TX,RX) antenna pair
int32_t get_nr_channel_level(const int len, const int extSize, const c16_t dl_ch_estimates_ext[extSize])
{
  const int16_t x = factor2(len);
  const int16_t y = (len) >> x;
  const int nb_rb_0 = len / 12 + ((len % 12) ? 1 : 0);

  AssertFatal(y!=0,"Cannot divide by zero: in function %s of file %s\n", __func__, __FILE__);

  const simde__m128i *dl_ch128 = (simde__m128i *)dl_ch_estimates_ext;
  simde__m128i avg128D = simde_mm_setzero_si128();

  for (int rb = 0; rb < nb_rb_0; rb++) {
    avg128D = simde_mm_add_epi32(avg128D, simde_mm_srai_epi32(simde_mm_madd_epi16(dl_ch128[0], dl_ch128[0]), x));
    avg128D = simde_mm_add_epi32(avg128D, simde_mm_srai_epi32(simde_mm_madd_epi16(dl_ch128[1], dl_ch128[1]), x));
    avg128D = simde_mm_add_epi32(avg128D, simde_mm_srai_epi32(simde_mm_madd_epi16(dl_ch128[2], dl_ch128[2]), x));
    dl_ch128 += 3;
  }

  int32_t *tmp = (int32_t *)&avg128D;
  const int32_t avg =((int64_t)tmp[0] + tmp[1] + tmp[2] + tmp[3]) / y;

  return avg;
}

int get_nr_channel_level_median(const int avg, const int length, const int extSize, const c16_t dl_ch_estimates_ext[extSize])
{
  int max = avg; // initialize the med point for max
  int min = avg; // initialize the med point for min
  const int length2 = length >> 2; // length = number of REs, hence length2=nb_REs*(32/128) in SIMD loop

  const simde__m128i *dl_ch128 = (simde__m128i *)dl_ch_estimates_ext;

  for (int i = 0; i < length2; i++) {
    const simde__m128i norm128D =
        simde_mm_srai_epi32(simde_mm_madd_epi16(dl_ch128[0], dl_ch128[0]), 2); //[|H_0|²/4 |H_1|²/4 |H_2|²/4 |H_3|²/4]

    int32_t *tmp = (int32_t *)&norm128D;
    const int64_t norm_pack = (int64_t)tmp[0] + tmp[1] + tmp[2] + tmp[3];

    if (norm_pack > max)
      max = norm_pack; // store values more than max
    if (norm_pack < min)
      min = norm_pack; // store values less than min
    dl_ch128 += 1;
  }

  const int median = (max + min) >> 1;
  return median;
}

/* Get average channel level from all antennas */
int32_t get_maxh_extimates(const NR_DL_FRAME_PARMS *frame_parms,
                           const NR_UE_DLSCH_t *dlsch,
                           const int symbol,
                           const c16_t dl_ch_est_ext[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB])
{
  const int nl = dlsch->Nl;
  const int nbAntRx = frame_parms->nb_antennas_rx;
  int avg = 0;
  int median;
  int avgs = 0;
  for (int l = 0; l < nl; l++) {
    for (int aarx = 0; aarx < nbAntRx; aarx++) {
      const int nb_re_pdsch = get_nb_re_pdsch_symbol(symbol, dlsch);
      if (nb_re_pdsch > 0) {
        avg = get_nr_channel_level(nb_re_pdsch, dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB, dl_ch_est_ext[l][aarx]);
      }
      avgs = cmax(avgs, avg);
      if (nl > 1) {
        median = get_nr_channel_level_median((const int)avg,
                                             get_nb_re_pdsch_symbol(symbol, dlsch),
                                             dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB,
                                             dl_ch_est_ext[l][aarx]);
        avgs = cmax(avgs, median);
      }
    }
  }
  return ((log2_approx(avgs) / 2) + 1);
}

//==============================================================================================
// Extraction functions
//==============================================================================================

/* Extract data resourse elements from received OFDM symbol */
void nr_extract_data_res(const NR_DL_FRAME_PARMS *frame_parms,
                         const fapi_nr_dl_config_dlsch_pdu_rel15_t *dlsch_config,
                         const bool isPilot,
                         const c16_t rxdataF[frame_parms->ofdm_symbol_size],
                         c16_t rxdataF_ext[dlsch_config->number_rbs * NR_NB_SC_PER_RB])
{
  const int pdschStartRb = dlsch_config->start_rb + dlsch_config->BWPStart;
  const int pdschNbRb = dlsch_config->number_rbs;
  const int startRe = (frame_parms->first_carrier_offset + pdschStartRb * NR_NB_SC_PER_RB) % frame_parms->ofdm_symbol_size;
  if (isPilot) {
    const int configType = dlsch_config->dmrsConfigType;
    const int nDmrsCdmGroups = dlsch_config->n_dmrs_cdm_groups;
    if (configType == NFAPI_NR_DMRS_TYPE1) {
      if (nDmrsCdmGroups == 1) {
        int k = startRe;
        for (int j = 0; j < 6 * pdschNbRb; j += 3) {
          rxdataF_ext[j] = rxdataF[k + 1];
          rxdataF_ext[j + 1] = rxdataF[k + 3];
          rxdataF_ext[j + 2] = rxdataF[k + 5];
          k += 6;
          if (k >= frame_parms->ofdm_symbol_size) {
            k -= frame_parms->ofdm_symbol_size;
          }
        }
      }
    } else { /* configType == NFAPI_NR_DMRS_TYPE2 */
      if (nDmrsCdmGroups == 1) {
        int k = startRe;
        for (int j = 0; j < 8 * pdschNbRb; j += 4) {
          rxdataF_ext[j] = rxdataF[k + 2];
          rxdataF_ext[j + 1] = rxdataF[k + 3];
          rxdataF_ext[j + 2] = rxdataF[k + 4];
          rxdataF_ext[j + 3] = rxdataF[k + 5];
          k += 6;
          if (k >= frame_parms->ofdm_symbol_size) {
            k -= frame_parms->ofdm_symbol_size;
          }
        }
      } else if (nDmrsCdmGroups == 2) {
        int k = startRe;
        for (int j = 0; j < 4 * pdschNbRb; j += 2) {
          rxdataF_ext[j] = rxdataF[k + 4];
          rxdataF_ext[j + 1] = rxdataF[k + 5];
          k += 6;
          if (k >= frame_parms->ofdm_symbol_size) {
            k -= frame_parms->ofdm_symbol_size;
          }
        }
      }
    }
  } else {
    if (startRe + pdschNbRb * NR_NB_SC_PER_RB <= frame_parms->ofdm_symbol_size) {
      memcpy(rxdataF_ext, &rxdataF[startRe], pdschNbRb * NR_NB_SC_PER_RB * sizeof(c16_t));
    } else {
      const int negLength = frame_parms->ofdm_symbol_size - startRe;
      const int posLength = pdschNbRb * NR_NB_SC_PER_RB - negLength;
      memcpy(rxdataF_ext, &rxdataF[startRe], negLength * sizeof(c16_t));
      memcpy(&rxdataF_ext[negLength], rxdataF, posLength * sizeof(c16_t));
    }
  }
}

/* Extract channel estimates for corresponding data REs in a OFDM symbol */
void nr_extract_pdsch_chest_res(const NR_DL_FRAME_PARMS *frame_parms,
                                const fapi_nr_dl_config_dlsch_pdu_rel15_t *dlsch_config,
                                const bool isPilot,
                                const c16_t dl_ch_est[frame_parms->ofdm_symbol_size],
                                c16_t dl_ch_est_ext[dlsch_config->number_rbs * NR_NB_SC_PER_RB])
{
  const int pdschNbRb = dlsch_config->number_rbs;
  if (isPilot) {
    const int configType = dlsch_config->dmrsConfigType;
    const int nDmrsCdmGroups = dlsch_config->n_dmrs_cdm_groups;
    if (configType == NFAPI_NR_DMRS_TYPE1) {
      if (nDmrsCdmGroups == 1) {
        for (int j = 0; j < 6 * pdschNbRb; j += 3) {
          dl_ch_est_ext[j] = dl_ch_est[1];
          dl_ch_est_ext[j + 1] = dl_ch_est[3];
          dl_ch_est_ext[j + 2] = dl_ch_est[5];
          dl_ch_est += 6;
        }
      }
    } else { /* configType == NFAPI_NR_DMRS_TYPE2 */
      if (nDmrsCdmGroups == 1) {
        for (int j = 0; j < 8 * pdschNbRb; j += 4) {
          dl_ch_est_ext[j] = dl_ch_est[1];
          dl_ch_est_ext[j + 1] = dl_ch_est[3];
          dl_ch_est_ext[j + 2] = dl_ch_est[4];
          dl_ch_est_ext[j + 3] = dl_ch_est[5];
          dl_ch_est += 6;
        }
      } else if (nDmrsCdmGroups == 2) {
        for (int j = 0; j < 4 * pdschNbRb; j += 2) {
          dl_ch_est_ext[j] = dl_ch_est[4];
          dl_ch_est_ext[j + 1] = dl_ch_est[5];
          dl_ch_est += 6;
        }
      }
    }
  } else {
    memcpy(dl_ch_est_ext, dl_ch_est, pdschNbRb * NR_NB_SC_PER_RB * sizeof(c16_t));
  }
}

void nr_dlsch_extract_rbs(c16_t **rxdataF,
                          c16_t **dl_ch_estimates,
                          uint32_t rx_size_symbol,
                          c16_t rxdataF_ext[][rx_size_symbol],
                          c16_t dl_ch_estimates_ext[][rx_size_symbol],
                          unsigned char symbol,
                          uint8_t pilots,
                          uint8_t config_type,
                          unsigned short start_rb,
                          unsigned short nb_rb_pdsch,
                          uint8_t n_dmrs_cdm_groups,
                          uint8_t Nl,
                          NR_DL_FRAME_PARMS *frame_parms,
                          uint16_t dlDmrsSymbPos)
{
  if (config_type == NFAPI_NR_DMRS_TYPE1) {
    AssertFatal(n_dmrs_cdm_groups == 1 || n_dmrs_cdm_groups == 2,
                "n_dmrs_cdm_groups %d is illegal\n",n_dmrs_cdm_groups);
  } else {
    AssertFatal(n_dmrs_cdm_groups == 1 || n_dmrs_cdm_groups == 2 || n_dmrs_cdm_groups == 3,
                "n_dmrs_cdm_groups %d is illegal\n",n_dmrs_cdm_groups);
  }

  const unsigned short start_re = (frame_parms->first_carrier_offset + start_rb * NR_NB_SC_PER_RB) % frame_parms->ofdm_symbol_size;

  for (unsigned char aarx = 0; aarx < frame_parms->nb_antennas_rx; aarx++) {
    c16_t *rxF_ext = rxdataF_ext[aarx];
    c16_t *rxF = rxdataF[aarx];

    for (unsigned char l = 0; l < Nl; l++) {
      c16_t *dl_ch0 = dl_ch_estimates[(l * frame_parms->nb_antennas_rx) + aarx];
      c16_t *dl_ch0_ext = dl_ch_estimates_ext[(l * frame_parms->nb_antennas_rx) + aarx];

      if (pilots == 0) { //data symbol only
        if (l == 0) {
          if (start_re + nb_rb_pdsch * NR_NB_SC_PER_RB <= frame_parms->ofdm_symbol_size) {
            memcpy(rxF_ext, &rxF[start_re], nb_rb_pdsch * NR_NB_SC_PER_RB * sizeof(int32_t));
          } else {
            int neg_length = frame_parms->ofdm_symbol_size - start_re;
            int pos_length = nb_rb_pdsch * NR_NB_SC_PER_RB - neg_length;
            memcpy(rxF_ext, &rxF[start_re], neg_length * sizeof(int32_t));
            memcpy(&rxF_ext[neg_length], rxF, pos_length * sizeof(int32_t));
          }
        }
        memcpy(dl_ch0_ext, dl_ch0, nb_rb_pdsch * NR_NB_SC_PER_RB * sizeof(int32_t));
      }
      else if (config_type == NFAPI_NR_DMRS_TYPE1){
        if (n_dmrs_cdm_groups == 1) { //data is multiplexed
          if (l == 0) {
            unsigned short k = start_re;
            for (unsigned short j = 0; j < 6*nb_rb_pdsch; j += 3) {
              rxF_ext[j]   = rxF[k+1];
              rxF_ext[j+1] = rxF[k+3];
              rxF_ext[j+2] = rxF[k+5];
              k += 6;
              if (k >= frame_parms->ofdm_symbol_size)
                k -= frame_parms->ofdm_symbol_size;
            }
          }
          for (unsigned short j = 0; j < 6*nb_rb_pdsch; j += 3) {
            dl_ch0_ext[j]   = dl_ch0[1];
            dl_ch0_ext[j+1] = dl_ch0[3];
            dl_ch0_ext[j+2] = dl_ch0[5];
            dl_ch0 += 6;
          }
        }
      }
      else {//NFAPI_NR_DMRS_TYPE2
        if (n_dmrs_cdm_groups == 1) { //data is multiplexed
          if (l == 0) {
            unsigned short k = start_re;
            for (unsigned short j = 0; j < 8*nb_rb_pdsch; j += 4) {
              rxF_ext[j]   = rxF[k+2];
              rxF_ext[j+1] = rxF[k+3];
              rxF_ext[j+2] = rxF[k+4];
              rxF_ext[j+3] = rxF[k+5];
              k += 6;
              if (k >= frame_parms->ofdm_symbol_size)
                k -= frame_parms->ofdm_symbol_size;
            }
          }
          for (unsigned short j = 0; j < 8*nb_rb_pdsch; j += 4) {
            dl_ch0_ext[j]   = dl_ch0[2];
            dl_ch0_ext[j+1] = dl_ch0[3];
            dl_ch0_ext[j+2] = dl_ch0[4];
            dl_ch0_ext[j+3] = dl_ch0[5];
            dl_ch0 += 6;
          }
        }
        else if (n_dmrs_cdm_groups == 2) { //data is multiplexed
          if (l == 0) {
            unsigned short k = start_re;
            for (unsigned short j = 0; j < 4*nb_rb_pdsch; j += 2) {
              rxF_ext[j]   = rxF[k+4];
              rxF_ext[j+1] = rxF[k+5];
              k += 6;
              if (k >= frame_parms->ofdm_symbol_size)
                k -= frame_parms->ofdm_symbol_size;
            }
          }
          for (unsigned short j = 0; j < 4*nb_rb_pdsch; j += 2) {
            dl_ch0_ext[j]   = dl_ch0[4];
            dl_ch0_ext[j+1] = dl_ch0[5];
            dl_ch0 += 6;
          }
        }
      }
    }
  }
}

void nr_dlsch_detection_mrc(const int n_tx,
                            const int n_rx,
                            const int nb_rb,
                            const int length,
                            c16_t rxdataF_comp[n_tx][n_rx][nb_rb * NR_NB_SC_PER_RB],
                            c16_t dl_ch_mag[n_tx][n_rx][nb_rb * NR_NB_SC_PER_RB],
                            c16_t dl_ch_magb[n_tx][n_rx][nb_rb * NR_NB_SC_PER_RB],
                            c16_t dl_ch_magr[n_tx][n_rx][nb_rb * NR_NB_SC_PER_RB])
{
  unsigned char aatx, aarx;
  int i;
  simde__m128i *rxdataF_comp128_0,*rxdataF_comp128_1,*dl_ch_mag128_0,*dl_ch_mag128_1,*dl_ch_mag128_0b,*dl_ch_mag128_1b,*dl_ch_mag128_0r,*dl_ch_mag128_1r;

  uint32_t nb_rb_0 = length/12 + ((length%12)?1:0);

  if (n_rx>1) {
    for (aatx=0; aatx<n_tx; aatx++) {
      rxdataF_comp128_0 = (simde__m128i *)(rxdataF_comp[aatx][0]);
      dl_ch_mag128_0 = (simde__m128i *)dl_ch_mag[aatx][0];
      dl_ch_mag128_0b = (simde__m128i *)dl_ch_magb[aatx][0];
      dl_ch_mag128_0r = (simde__m128i *)dl_ch_magr[aatx][0];
      for (aarx=1; aarx<n_rx; aarx++) {
        rxdataF_comp128_1 = (simde__m128i *)(rxdataF_comp[aatx][aarx]);
        dl_ch_mag128_1 = (simde__m128i *)dl_ch_mag[aatx][aarx];
        dl_ch_mag128_1b = (simde__m128i *)dl_ch_magb[aatx][aarx];
        dl_ch_mag128_1r = (simde__m128i *)dl_ch_magr[aatx][aarx];

        // MRC on each re of rb, both on MF output and magnitude (for 16QAM/64QAM/256 llr computation)
        for (i=0; i<nb_rb_0*3; i++) {
          rxdataF_comp128_0[i] = simde_mm_adds_epi16(rxdataF_comp128_0[i],rxdataF_comp128_1[i]);
          dl_ch_mag128_0[i]    = simde_mm_adds_epi16(dl_ch_mag128_0[i],dl_ch_mag128_1[i]);
          dl_ch_mag128_0b[i]   = simde_mm_adds_epi16(dl_ch_mag128_0b[i],dl_ch_mag128_1b[i]);
          dl_ch_mag128_0r[i]   = simde_mm_adds_epi16(dl_ch_mag128_0r[i],dl_ch_mag128_1r[i]);
        }
      }
    }
#ifdef DEBUG_DLSCH_DEMOD
    for (i=0; i<nb_rb_0*3; i++) {
      printf("RB %d\n", i / 3);
      rxdataF_comp128_0 = (simde__m128i *)(rxdataF_comp[0][0]);
      rxdataF_comp128_1 = (simde__m128i *)(rxdataF_comp[0][n_rx]);
      print_shorts("tx 1 mrc_re/mrc_Im:", (int16_t *)&rxdataF_comp128_0[i]);
      print_shorts("tx 2 mrc_re/mrc_Im:", (int16_t *)&rxdataF_comp128_1[i]);
      // printf("mrc mag0 = %d = %d \n",((int16_t*)&dl_ch_mag128_0[0])[0],((int16_t*)&dl_ch_mag128_0[0])[1]);
      // printf("mrc mag0b = %d = %d \n",((int16_t*)&dl_ch_mag128_0b[0])[0],((int16_t*)&dl_ch_mag128_0b[0])[1]);
    }
#endif
    simde_mm_empty();
    simde_m_empty();
  }
}

/* Zero Forcing Rx function: nr_a_sum_b()
 * Compute the complex addition x=x+y
 *
 * */
void nr_a_sum_b(c16_t *input_x, c16_t *input_y, unsigned short nb_rb)
{
  unsigned short rb;
  simde__m128i *x = (simde__m128i *)input_x;
  simde__m128i *y = (simde__m128i *)input_y;

  for (rb=0; rb<nb_rb; rb++) {
    x[0] = simde_mm_adds_epi16(x[0], y[0]);
    x[1] = simde_mm_adds_epi16(x[1], y[1]);
    x[2] = simde_mm_adds_epi16(x[2], y[2]);
    x += 3;
    y += 3;
  }
}

/* Zero Forcing Rx function: nr_a_mult_b()
 * Compute the complex Multiplication c=a*b
 *
 * */
void nr_a_mult_b(c16_t *a, c16_t *b, c16_t *c, unsigned short nb_rb, unsigned char output_shift0)
{
  //This function is used to compute complex multiplications
  short nr_conjugate[8]__attribute__((aligned(16))) = {1,-1,1,-1,1,-1,1,-1};
  unsigned short rb;
  simde__m128i *a_128,*b_128, *c_128, mmtmpD0,mmtmpD1,mmtmpD2,mmtmpD3;

  a_128 = (simde__m128i *)a;
  b_128 = (simde__m128i *)b;

  c_128 = (simde__m128i *)c;

  for (rb=0; rb<3*nb_rb; rb++) {
    // the real part
    mmtmpD0 = simde_mm_sign_epi16(a_128[0],*(simde__m128i*)&nr_conjugate[0]);
    mmtmpD0 = simde_mm_madd_epi16(mmtmpD0,b_128[0]); //Re: (a_re*b_re - a_im*b_im)

    // the imag part
    mmtmpD1 = simde_mm_shufflelo_epi16(a_128[0],SIMDE_MM_SHUFFLE(2,3,0,1));
    mmtmpD1 = simde_mm_shufflehi_epi16(mmtmpD1,SIMDE_MM_SHUFFLE(2,3,0,1));
    mmtmpD1 = simde_mm_madd_epi16(mmtmpD1,b_128[0]);//Im: (x_im*y_re + x_re*y_im)

    mmtmpD0 = simde_mm_srai_epi32(mmtmpD0,output_shift0);
    mmtmpD1 = simde_mm_srai_epi32(mmtmpD1,output_shift0);
    mmtmpD2 = simde_mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
    mmtmpD3 = simde_mm_unpackhi_epi32(mmtmpD0,mmtmpD1);

    c_128[0] = simde_mm_packs_epi32(mmtmpD2,mmtmpD3);

    /*printf("\n Computing mult \n");
    print_shorts("a:",(int16_t*)&a_128[0]);
    print_shorts("b:",(int16_t*)&b_128[0]);
    print_shorts("pack:",(int16_t*)&c_128[0]);*/

    a_128+=1;
    b_128+=1;
    c_128+=1;
  }
}

/* Zero Forcing Rx function: nr_element_sign()
 * Compute b=sign*a
 *
 * */
static inline void nr_element_sign(c16_t *a, // a
                                   c16_t *b, // b
                                   unsigned short nb_rb,
                                   int32_t sign)
{
  const int16_t nr_sign[8] __attribute__((aligned(16))) = {-1, -1, -1, -1, -1, -1, -1, -1};
  simde__m128i *a_128,*b_128;

  a_128 = (simde__m128i *)a;
  b_128 = (simde__m128i *)b;

  for (int rb = 0; rb < 3 * nb_rb; rb++) {
    if (sign < 0)
      b_128[rb] = simde_mm_sign_epi16(a_128[rb], ((simde__m128i *)nr_sign)[0]);
    else
      b_128[rb] = a_128[rb];

#ifdef DEBUG_DLSCH_DEMOD
    print_shorts("b:", (int16_t *)b_128);
#endif
  }
}

/* Zero Forcing Rx function: nr_det_4x4()
 * Compute the matrix determinant for 4x4 Matrix
 *
 * */
static void nr_determin(int size,
                        c16_t *a44[][size], //
                        c16_t *ad_bc, // ad-bc
                        unsigned short nb_rb,
                        int32_t sign,
                        int32_t shift0)
{
  AssertFatal(size > 0, "");

  if(size==1) {
    nr_element_sign(a44[0][0], // a
                    ad_bc, // b
                    nb_rb,
                    sign);
  } else {
    int16_t k, rr[size - 1], cc[size - 1];
    c16_t outtemp[12 * nb_rb] __attribute__((aligned(32)));
    c16_t outtemp1[12 * nb_rb] __attribute__((aligned(32)));
    c16_t *sub_matrix[size - 1][size - 1];
    for (int rtx=0;rtx<size;rtx++) {//row calculation for determin
      int ctx=0;
      //find the submatrix row and column indices
      k=0;
      for(int rrtx=0;rrtx<size;rrtx++)
        if(rrtx != rtx) rr[k++] = rrtx;
      k=0;
      for(int cctx=0;cctx<size;cctx++)
        if(cctx != ctx) cc[k++] = cctx;
      // fill out the sub matrix corresponds to this element

      for (int ridx = 0; ridx < (size - 1); ridx++)
        for (int cidx = 0; cidx < (size - 1); cidx++)
          sub_matrix[cidx][ridx] = a44[cc[cidx]][rr[ridx]];

      nr_determin(size - 1,
                  sub_matrix, // a33
                  outtemp,
                  nb_rb,
                  ((rtx & 1) == 1 ? -1 : 1) * ((ctx & 1) == 1 ? -1 : 1) * sign,
                  shift0);
      nr_a_mult_b(a44[ctx][rtx], outtemp, rtx == 0 ? ad_bc : outtemp1, nb_rb, shift0);

      if (rtx != 0)
        nr_a_sum_b(ad_bc, outtemp1, nb_rb);
    }
  }
}

static double complex nr_determin_cpx(int32_t size, // size
                                      double complex a44_cpx[][size], //
                                      int32_t sign)
{
  double complex outtemp, outtemp1;
  //Allocate the submatrix elements
  DevAssert(size > 0);
  if(size==1) {
    return (a44_cpx[0][0] * sign);
  }else {
    double complex sub_matrix[size - 1][size - 1];
    int16_t k, rr[size - 1], cc[size - 1];
    outtemp1 = 0;
    for (int rtx=0;rtx<size;rtx++) {//row calculation for determin
      int ctx=0;
      //find the submatrix row and column indices
      k=0;
      for(int rrtx=0;rrtx<size;rrtx++)
        if(rrtx != rtx) rr[k++] = rrtx;
      k=0;
      for(int cctx=0;cctx<size;cctx++)
        if(cctx != ctx) cc[k++] = cctx;
      //fill out the sub matrix corresponds to this element
       for (int ridx=0;ridx<(size-1);ridx++)
         for (int cidx=0;cidx<(size-1);cidx++)
           sub_matrix[cidx][ridx] = a44_cpx[cc[cidx]][rr[ridx]];

       outtemp = nr_determin_cpx(size - 1,
                                 sub_matrix, // a33
                                 ((rtx & 1) == 1 ? -1 : 1) * ((ctx & 1) == 1 ? -1 : 1) * sign);
       outtemp1 += a44_cpx[ctx][rtx] * outtemp;
    }

    return((double complex)outtemp1);
  }
}

/* Zero Forcing Rx function: nr_matrix_inverse()
 * Compute the matrix inverse and determinant up to 4x4 Matrix
 *
 * */
uint8_t nr_matrix_inverse(int32_t size,
                          c16_t *a44[][size], // Input matrix//conjH_H_elements[0]
                          c16_t *inv_H_h_H[][size], // Inverse
                          c16_t *ad_bc, // determin
                          unsigned short nb_rb,
                          int32_t flag, // fixed point or floating flag
                          int32_t shift0)
{
  DevAssert(size > 1);
  int16_t k,rr[size-1],cc[size-1];

  if(flag) {//fixed point SIMD calc.
    //Allocate the submatrix elements
    c16_t *sub_matrix[size - 1][size - 1];

    //Compute Matrix determinant
    nr_determin(size,
                a44, //
                ad_bc, // determinant
                nb_rb,
                +1,
                shift0);
    //print_shorts("nr_det_",(int16_t*)&ad_bc[0]);

    //Compute Inversion of the H^*H matrix
    /* For 2x2 MIMO matrix, we compute
     * *        |(conj_H_00xH_00+conj_H_10xH_10)   (conj_H_00xH_01+conj_H_10xH_11)|
     * * H_h_H= |                                                                 |
     * *        |(conj_H_01xH_00+conj_H_11xH_10)   (conj_H_01xH_01+conj_H_11xH_11)|
     * *
     * *inv(H_h_H) =(1/det)*[d  -b
     * *                     -c  a]
     * **************************************************************************/
    for (int rtx=0;rtx<size;rtx++) {//row
      k=0;
      for(int rrtx=0;rrtx<size;rrtx++)
        if(rrtx != rtx) rr[k++] = rrtx;
      for (int ctx=0;ctx<size;ctx++) {//column
        k=0;
        for(int cctx=0;cctx<size;cctx++)
          if(cctx != ctx) cc[k++] = cctx;

        //fill out the sub matrix corresponds to this element
        for (int ridx=0;ridx<(size-1);ridx++)
          for (int cidx=0;cidx<(size-1);cidx++)
            // To verify
            sub_matrix[cidx][ridx]=a44[cc[cidx]][rr[ridx]];

        nr_determin(size - 1, // size
                    sub_matrix,
                    inv_H_h_H[rtx][ctx], // out transpose
                    nb_rb,
                    ((rtx & 1) == 1 ? -1 : 1) * ((ctx & 1) == 1 ? -1 : 1),
                    shift0);
      }
    }
  }
  else {//floating point calc.
    //Allocate the submatrix elements
    double complex sub_matrix_cpx[size - 1][size - 1];
    //Convert the IQ samples (in Q15 format) to float complex
    double complex a44_cpx[size][size];
    double complex inv_H_h_H_cpx[size][size];
    double complex determin_cpx;
    for (int i=0; i<12*nb_rb; i++) {

      //Convert Q15 to floating point
      for (int rtx=0;rtx<size;rtx++) {//row
        for (int ctx=0;ctx<size;ctx++) {//column
          a44_cpx[ctx][rtx] = ((double)(a44[ctx][rtx])[i].r) / (1 << (shift0 - 1)) + I * ((double)(a44[ctx][rtx])[i].i) / (1 << (shift0 - 1));
          //if (i<4) printf("a44_cpx(%d,%d)= ((FP %d))%lf+(FP %d)j%lf \n",ctx,rtx,((short *)a44[ctx*size+rtx])[(i<<1)],creal(a44_cpx[ctx*size+rtx]),((short *)a44[ctx*size+rtx])[(i<<1)+1],cimag(a44_cpx[ctx*size+rtx]));
        }
      }
      //Compute Matrix determinant (copy real value only)
      determin_cpx = nr_determin_cpx(size,
                                     a44_cpx, //
                                     +1);
      //if (i<4) printf("order %d nr_det_cpx = %lf+j%lf \n",log2_approx(creal(determin_cpx)),creal(determin_cpx),cimag(determin_cpx));

      //Round and convert to Q15 (Out in the same format as Fixed point).
      if (creal(determin_cpx)>0) {//determin of the symmetric matrix is real part only
        ((short*) ad_bc)[i<<1] = (short) ((creal(determin_cpx)*(1<<(shift0)))+0.5);//
        //((short*) ad_bc)[(i<<1)+1] = (short) ((cimag(determin_cpx)*(1<<(shift0)))+0.5);//
      } else {
        ((short*) ad_bc)[i<<1] = (short) ((creal(determin_cpx)*(1<<(shift0)))-0.5);//
        //((short*) ad_bc)[(i<<1)+1] = (short) ((cimag(determin_cpx)*(1<<(shift0)))-0.5);//
      }
      //if (i<4) printf("nr_det_FP= %d+j%d \n",((short*) ad_bc)[i<<1],((short*) ad_bc)[(i<<1)+1]);
      //Compute Inversion of the H^*H matrix (normalized output divide by determinant)
      for (int rtx=0;rtx<size;rtx++) {//row
        k=0;
        for(int rrtx=0;rrtx<size;rrtx++)
          if(rrtx != rtx) rr[k++] = rrtx;
        for (int ctx=0;ctx<size;ctx++) {//column
          k=0;
          for(int cctx=0;cctx<size;cctx++)
            if(cctx != ctx) cc[k++] = cctx;

          //fill out the sub matrix corresponds to this element
          for (int ridx=0;ridx<(size-1);ridx++)
            for (int cidx=0;cidx<(size-1);cidx++)
              sub_matrix_cpx[cidx][ridx] = a44_cpx[cc[cidx]][rr[ridx]];

          inv_H_h_H_cpx[rtx][ctx] = nr_determin_cpx(size - 1, // size,
                                                    sub_matrix_cpx, //
                                                    ((rtx & 1) == 1 ? -1 : 1) * ((ctx & 1) == 1 ? -1 : 1));
          //if (i==0) printf("H_h_H(r%d,c%d)=%lf+j%lf --> inv_H_h_H(%d,%d) = %lf+j%lf \n",rtx,ctx,creal(a44_cpx[ctx*size+rtx]),cimag(a44_cpx[ctx*size+rtx]),ctx,rtx,creal(inv_H_h_H_cpx[rtx*size+ctx]),cimag(inv_H_h_H_cpx[rtx*size+ctx]));

          if (creal(inv_H_h_H_cpx[rtx][ctx]) > 0)
            inv_H_h_H[rtx][ctx][i].r = (short)((creal(inv_H_h_H_cpx[rtx][ctx]) * (1 << (shift0 - 1))) + 0.5); // Convert to Q 18
          else
            inv_H_h_H[rtx][ctx][i].r = (short)((creal(inv_H_h_H_cpx[rtx][ctx]) * (1 << (shift0 - 1))) - 0.5); //

          if (cimag(inv_H_h_H_cpx[rtx][ctx]) > 0)
            inv_H_h_H[rtx][ctx][i].i = (short)((cimag(inv_H_h_H_cpx[rtx][ctx]) * (1 << (shift0 - 1))) + 0.5); //
          else
            inv_H_h_H[rtx][ctx][i].i = (short)((cimag(inv_H_h_H_cpx[rtx][ctx]) * (1 << (shift0 - 1))) - 0.5); //

          //if (i<4) printf("inv_H_h_H_FP(%d,%d)= %d+j%d \n",ctx,rtx, ((short *) inv_H_h_H[rtx*size+ctx])[i<<1],((short *) inv_H_h_H[rtx*size+ctx])[(i<<1)+1]);
        }
      }
    }
  }
  return(0);
}

/* Zero Forcing Rx function: nr_conjch0_mult_ch1()
 *
 *
 * */
void nr_conjch0_mult_ch1(const int *ch0,
                         const int *ch1,
                         int32_t *ch0conj_ch1,
                         const unsigned short nb_rb,
                         const unsigned char output_shift0)
{
  //This function is used to compute multiplications in H_hermitian * H matrix
  short nr_conjugate[8]__attribute__((aligned(16))) = {-1,1,-1,1,-1,1,-1,1};
  unsigned short rb;
  simde__m128i *ch0conj_ch1_128, mmtmpD0, mmtmpD1, mmtmpD2, mmtmpD3;

  const simde__m128i *dl_ch0_128 = (simde__m128i *)ch0;
  const simde__m128i *dl_ch1_128 = (simde__m128i *)ch1;

  ch0conj_ch1_128 = (simde__m128i *)ch0conj_ch1;

  for (rb=0; rb<3*nb_rb; rb++) {

    mmtmpD0 = simde_mm_madd_epi16(dl_ch0_128[0],dl_ch1_128[0]);
    mmtmpD1 = simde_mm_shufflelo_epi16(dl_ch0_128[0],SIMDE_MM_SHUFFLE(2,3,0,1));
    mmtmpD1 = simde_mm_shufflehi_epi16(mmtmpD1,SIMDE_MM_SHUFFLE(2,3,0,1));
    mmtmpD1 = simde_mm_sign_epi16(mmtmpD1,*(simde__m128i*)&nr_conjugate[0]);
    mmtmpD1 = simde_mm_madd_epi16(mmtmpD1,dl_ch1_128[0]);
    mmtmpD0 = simde_mm_srai_epi32(mmtmpD0,output_shift0);
    mmtmpD1 = simde_mm_srai_epi32(mmtmpD1,output_shift0);
    mmtmpD2 = simde_mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
    mmtmpD3 = simde_mm_unpackhi_epi32(mmtmpD0,mmtmpD1);

    ch0conj_ch1_128[0] = simde_mm_packs_epi32(mmtmpD2,mmtmpD3);

    /*printf("\n Computing conjugates \n");
    print_shorts("ch0:",(int16_t*)&dl_ch0_128[0]);
    print_shorts("ch1:",(int16_t*)&dl_ch1_128[0]);
    print_shorts("pack:",(int16_t*)&ch0conj_ch1_128[0]);*/

    dl_ch0_128+=1;
    dl_ch1_128+=1;
    ch0conj_ch1_128+=1;
  }
  simde_mm_empty();
  simde_m_empty();
}

/* Zero Forcing Rx function: up to 4 layers
 *
 *
 * */
void nr_zero_forcing_rx(const int n_tx,
                        const int n_rx,
                        const int nb_rb,
                        const int length,
                        const int mod_order,
                        const int shift,
                        const c16_t dl_ch_estimates_ext[n_tx][n_rx][nb_rb * NR_NB_SC_PER_RB],
                        c16_t rxdataF_comp[n_tx][n_rx][nb_rb * NR_NB_SC_PER_RB],
                        c16_t dl_ch_mag[n_tx][n_rx][nb_rb * NR_NB_SC_PER_RB],
                        c16_t dl_ch_magb[n_tx][n_rx][nb_rb * NR_NB_SC_PER_RB],
                        c16_t dl_ch_magr[n_tx][n_rx][nb_rb * NR_NB_SC_PER_RB])
{
  int *ch0r, *ch0c;
  uint32_t nb_rb_0 = length/12 + ((length%12)?1:0);
  c16_t determ_fin[12 * nb_rb_0] __attribute__((aligned(32)));

  ///Allocate H^*H matrix elements and sub elements
  c16_t conjH_H_elements_data[n_rx][n_tx][n_tx][12 * nb_rb_0];
  memset(conjH_H_elements_data, 0, sizeof(conjH_H_elements_data));
  c16_t *conjH_H_elements[n_rx][n_tx][n_tx];
  for (int aarx = 0; aarx < n_rx; aarx++)
    for (int rtx = 0; rtx < n_tx; rtx++)
      for (int ctx = 0; ctx < n_tx; ctx++)
        conjH_H_elements[aarx][rtx][ctx] = conjH_H_elements_data[aarx][rtx][ctx];

  //Compute H^*H matrix elements and sub elements:(1/2^log2_maxh)*conjH_H_elements
  for (int rtx=0;rtx<n_tx;rtx++) {//row
    for (int ctx=0;ctx<n_tx;ctx++) {//column
      for (int aarx=0;aarx<n_rx;aarx++)  {
        ch0r = (int *)dl_ch_estimates_ext[rtx][aarx];
        ch0c = (int *)dl_ch_estimates_ext[ctx][aarx];
        nr_conjch0_mult_ch1(ch0r,
                            ch0c,
                            (int32_t *)conjH_H_elements[aarx][ctx][rtx], // sic
                            nb_rb_0,
                            shift);
        if (aarx != 0)
          nr_a_sum_b(conjH_H_elements[0][ctx][rtx], conjH_H_elements[aarx][ctx][rtx], nb_rb_0);
      }
    }
  }

  //Compute the inverse and determinant of the H^*H matrix
  //Allocate the inverse matrix
  c16_t *inv_H_h_H[n_tx][n_tx];
  c16_t inv_H_h_H_data[n_tx][n_tx][12 * nb_rb_0];
  memset(inv_H_h_H_data, 0, sizeof(inv_H_h_H_data));
  for (int rtx = 0; rtx < n_tx; rtx++)
    for (int ctx = 0; ctx < n_tx; ctx++)
      inv_H_h_H[ctx][rtx] = inv_H_h_H_data[ctx][rtx];

  int fp_flag = 1;//0: float point calc 1: Fixed point calc
  nr_matrix_inverse(n_tx,
                    conjH_H_elements[0], // Input matrix
                    inv_H_h_H, // Inverse
                    determ_fin, // determin
                    nb_rb_0,
                    fp_flag, // fixed point flag
                    shift - (fp_flag == 1 ? 2 : 0)); // the out put is Q15

  // multiply Matrix inversion pf H_h_H by the rx signal vector
  c16_t outtemp[12 * nb_rb_0] __attribute__((aligned(32)));
  //Allocate rxdataF for zforcing out
  c16_t rxdataF_zforcing[n_tx][12 * nb_rb_0];
  memset(rxdataF_zforcing, 0, sizeof(rxdataF_zforcing));

  for (int rtx=0;rtx<n_tx;rtx++) {//Output Layers row
    // loop over Layers rtx=0,...,N_Layers-1
    for (int ctx = 0; ctx < n_tx; ctx++) { // column multi
      // printf("Computing r_%d c_%d\n",rtx,ctx);
      // print_shorts(" H_h_H=",(int16_t*)&conjH_H_elements[ctx*n_tx+rtx][0][0]);
      // print_shorts(" Inv_H_h_H=",(int16_t*)&inv_H_h_H[ctx*n_tx+rtx][0]);
      nr_a_mult_b(inv_H_h_H[ctx][rtx], (c16_t *)(rxdataF_comp[ctx][0]), outtemp, nb_rb_0, shift - (fp_flag == 1 ? 2 : 0));
      nr_a_sum_b(rxdataF_zforcing[rtx], outtemp,
                 nb_rb_0); // a =a + b
    }
#ifdef DEBUG_DLSCH_DEMOD
    printf("Computing layer_%d \n",rtx);;
    print_shorts(" Rx signal:=",(int16_t*)&rxdataF_zforcing[rtx][0]);
    print_shorts(" Rx signal:=",(int16_t*)&rxdataF_zforcing[rtx][4]);
    print_shorts(" Rx signal:=",(int16_t*)&rxdataF_zforcing[rtx][8]);
#endif
  }

  //Copy zero_forcing out to output array
  for (int rtx=0;rtx<n_tx;rtx++)
    nr_element_sign(rxdataF_zforcing[rtx], (c16_t *)(rxdataF_comp[rtx][0]), nb_rb_0, +1);

  //Update LLR thresholds with the Matrix determinant
  simde__m128i *dl_ch_mag128_0=NULL,*dl_ch_mag128b_0=NULL,*dl_ch_mag128r_0=NULL,*determ_fin_128;
  simde__m128i mmtmpD2,mmtmpD3;
  simde__m128i QAM_amp128={0},QAM_amp128b={0},QAM_amp128r={0};
  short nr_realpart[8]__attribute__((aligned(16))) = {1,0,1,0,1,0,1,0};
  determ_fin_128      = (simde__m128i *)&determ_fin[0];

  if (mod_order>2) {
    if (mod_order == 4) {
      QAM_amp128 = simde_mm_set1_epi16(QAM16_n1);  //2/sqrt(10)
      QAM_amp128b = simde_mm_setzero_si128();
      QAM_amp128r = simde_mm_setzero_si128();
    } else if (mod_order == 6) {
      QAM_amp128  = simde_mm_set1_epi16(QAM64_n1); //4/sqrt{42}
      QAM_amp128b = simde_mm_set1_epi16(QAM64_n2); //2/sqrt{42}
      QAM_amp128r = simde_mm_setzero_si128();
    } else if (mod_order == 8) {
      QAM_amp128 = simde_mm_set1_epi16(QAM256_n1); //8/sqrt{170}
      QAM_amp128b = simde_mm_set1_epi16(QAM256_n2);//4/sqrt{170}
      QAM_amp128r = simde_mm_set1_epi16(QAM256_n3);//2/sqrt{170}
    }
    dl_ch_mag128_0 = (simde__m128i *)dl_ch_mag[0][0];
    dl_ch_mag128b_0 = (simde__m128i *)dl_ch_magb[0][0];
    dl_ch_mag128r_0 = (simde__m128i *)dl_ch_magr[0][0];

    for (int rb=0; rb<3*nb_rb_0; rb++) {
      //for symmetric H_h_H matrix, the determinant is only real values
        mmtmpD2 = simde_mm_sign_epi16(determ_fin_128[0],*(simde__m128i*)&nr_realpart[0]);//set imag part to 0
        mmtmpD3 = simde_mm_shufflelo_epi16(mmtmpD2,SIMDE_MM_SHUFFLE(2,3,0,1));
        mmtmpD3 = simde_mm_shufflehi_epi16(mmtmpD3,SIMDE_MM_SHUFFLE(2,3,0,1));
        mmtmpD2 = simde_mm_add_epi16(mmtmpD2,mmtmpD3);

        dl_ch_mag128_0[0] = mmtmpD2;
        dl_ch_mag128b_0[0] = mmtmpD2;
        dl_ch_mag128r_0[0] = mmtmpD2;

        dl_ch_mag128_0[0] = simde_mm_mulhi_epi16(dl_ch_mag128_0[0],QAM_amp128);
        dl_ch_mag128_0[0] = simde_mm_slli_epi16(dl_ch_mag128_0[0],1);

        dl_ch_mag128b_0[0] = simde_mm_mulhi_epi16(dl_ch_mag128b_0[0],QAM_amp128b);
        dl_ch_mag128b_0[0] = simde_mm_slli_epi16(dl_ch_mag128b_0[0],1);
        dl_ch_mag128r_0[0] = simde_mm_mulhi_epi16(dl_ch_mag128r_0[0],QAM_amp128r);
        dl_ch_mag128r_0[0] = simde_mm_slli_epi16(dl_ch_mag128r_0[0],1);


      determ_fin_128 += 1;
      dl_ch_mag128_0 += 1;
      dl_ch_mag128b_0 += 1;
      dl_ch_mag128r_0 += 1;
    }
  }
}

void nr_dlsch_layer_demapping(const uint8_t Nl,
                              const uint8_t mod_order,
                              const int llrLayerSize,
                              const int16_t llr_layers[NR_SYMBOLS_PER_SLOT][NR_MAX_NB_LAYERS][llrLayerSize],
                              const NR_UE_DLSCH_t *dlsch,
                              const int32_t re_len[NR_SYMBOLS_PER_SLOT],
                              const int llrSize,
                              int16_t llr[llrSize])
{
  const int s0 = dlsch->dlsch_config.start_symbol;
  const int s1 = dlsch->dlsch_config.number_symbols;

  int k = 0;
  switch (Nl) {
    case 1:
      for (int i = s0; i < (s0 + s1); i++) {
        memcpy(llr + k, llr_layers[i][0], re_len[i] * mod_order * sizeof(int16_t));
        k += re_len[i] * mod_order;
      }
      break;

    case 2:
    case 3:
    case 4:
      for (int i = s0; i < (s0 + s1); i++) {
        int m = 0;
        for (int j = 0; j < re_len[i]; j++) {
          for (int l = 0; l < Nl; l++) {
            memcpy(llr + k, llr_layers[i][l] + m * mod_order, sizeof(int16_t) * mod_order);
            k += mod_order;
            // if (i<4) printf("length%d: llr_layers[l%d][m%d]=%d: \n",length,l,m,llr_layers[l][i*mod_order+m]);
          }
          m++;
        }
      }
      break;

    default:
      AssertFatal(0, "Not supported number of layers %d\n", Nl);
  }
}

/* Computes LLRs from compensated PDSCH signal per OFDM symbol for all layers */
int nr_dlsch_llr(const NR_DL_FRAME_PARMS *frame_parms,
                 const NR_UE_DLSCH_t *dlsch,
                 const int len,
                 const c16_t dl_ch_mag[dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
                 const c16_t dl_ch_magb[dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
                 const c16_t dl_ch_magr[dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
                 const c16_t rxdataF_comp[dlsch->Nl][frame_parms->nb_antennas_rx][dlsch->dlsch_config.number_rbs * NR_NB_SC_PER_RB],
                 const int llrSize,
                 int16_t layer_llr[dlsch->Nl][llrSize])
{
  switch (dlsch->dlsch_config.qamModOrder) {
    case 2 :
      for(int l=0; l < dlsch[0].Nl; l++)
        nr_dlsch_qpsk_llr(frame_parms, rxdataF_comp[l][0], layer_llr[l], len, dlsch->dlsch_config.number_rbs);
      break;

    case 4 :
      for(int l=0; l < dlsch[0].Nl; l++)
        nr_dlsch_16qam_llr(frame_parms, rxdataF_comp[l][0], layer_llr[l], dl_ch_mag, len, dlsch->dlsch_config.number_rbs);
      break;

    case 6 :
      for(int l=0; l < dlsch[0].Nl; l++)
        nr_dlsch_64qam_llr(frame_parms,
                           rxdataF_comp[l][0],
                           layer_llr[l],
                           dl_ch_mag,
                           dl_ch_magb,
                           len,
                           dlsch->dlsch_config.number_rbs);
      break;

    case 8:
      for(int l=0; l < dlsch[0].Nl; l++)
        nr_dlsch_256qam_llr(frame_parms,
                            rxdataF_comp[l][0],
                            layer_llr[l],
                            dl_ch_mag,
                            dl_ch_magb,
                            dl_ch_magr,
                            len,
                            dlsch->dlsch_config.number_rbs);
      break;

    default:
      LOG_W(PHY,"rx_dlsch.c : Unknown mod_order!!!!\n");
      return(-1);
      break;
  }

  return 0;
}
//==============================================================================================
