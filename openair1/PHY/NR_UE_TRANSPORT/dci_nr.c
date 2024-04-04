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

/*! \file dci_nr.c
 * \brief Implements PDCCH physical channel TX/RX procedures (36.211) and DCI encoding/decoding (36.212/36.213). Current LTE
 * compliance V8.6 2009-03. \author R. Knopp, A. Mico Pereperez \date 2018 \version 0.1 \company Eurecom \email: knopp@eurecom.fr
 * \note
 * \warning
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "executables/softmodem-common.h"
#include "nr_transport_proto_ue.h"
#include "PHY/CODING/nrPolar_tools/nr_polar_dci_defs.h"
#include "PHY/phy_extern.h"
#include "PHY/CODING/coding_extern.h"
#include "PHY/sse_intrin.h"
#include "common/utils/nr/nr_common.h"
#include <openair1/PHY/TOOLS/phy_scope_interface.h>
#include "PHY/NR_UE_ESTIMATION/nr_estimation.h"

#include "assertions.h"
#include "T.h"

static const char nr_dci_format_string[8][30] = {"NR_DL_DCI_FORMAT_1_0",
                                                 "NR_DL_DCI_FORMAT_1_1",
                                                 "NR_DL_DCI_FORMAT_2_0",
                                                 "NR_DL_DCI_FORMAT_2_1",
                                                 "NR_DL_DCI_FORMAT_2_2",
                                                 "NR_DL_DCI_FORMAT_2_3",
                                                 "NR_UL_DCI_FORMAT_0_0",
                                                 "NR_UL_DCI_FORMAT_0_1"};

//#define DEBUG_DCI_DECODING 1

//#define NR_PDCCH_DCI_DEBUG            // activates NR_PDCCH_DCI_DEBUG logs
#ifdef NR_PDCCH_DCI_DEBUG
#define LOG_DDD(a, ...) printf("<-NR_PDCCH_DCI_DEBUG (%s)-> " a, __func__, ##__VA_ARGS__ )
#define LOG_DSYMB(b)                                                               \
  LOG_DDD("RB[c_rb %d] \t RE[re %d] => rxF_ext[%d]=(%d,%d)\t rxF[%d]=(%d,%d)\n" b, \
          c_rb,                                                                    \
          i,                                                                       \
          j,                                                                       \
          rxF_ext[j].r,                                                            \
          rxF_ext[j].i,                                                            \
          i,                                                                       \
          rxF[i].r,                                                                \
          rxF[i].i)
#else
#define LOG_DDD(a...)
#define LOG_DSYMB(a...)
#endif
#define NR_NBR_CORESET_ACT_BWP 3 // The number of CoreSets per BWP is limited to 3 (including initial CORESET: ControlResourceId 0)
#define NR_NBR_SEARCHSPACE_ACT_BWP \
  10 // The number of SearSpaces per BWP is limited to 10 (including initial SEARCHSPACE: SearchSpaceId 0)

#ifdef LOG_I
#undef LOG_I
#define LOG_I(A, B...) printf(B)
#endif



static void nr_pdcch_demapping_deinterleaving(const c16_t *llr,
                                              c16_t *e_rx,
                                              const uint8_t coreset_time_dur,
                                              const uint8_t start_symbol,
                                              const uint32_t coreset_nbr_rb,
                                              const uint8_t reg_bundle_size_L_in,
                                              const uint8_t coreset_interleaver_size_R,
                                              const uint8_t n_shift,
                                              const uint8_t number_of_candidates,
                                              const uint16_t *CCE,
                                              const uint8_t *L)
{
  /*
   * This function will do demapping and deinterleaving from llr containing demodulated symbols
   * Demapping will regroup in REG and bundles
   * Deinterleaving will order the bundles
   *
   * In the following example we can see the process. The llr contains the demodulated IQs, but they are not ordered from
   REG 0,1,2,..
   * In e_rx (z) we will order the REG ids and group them into bundles.
   * Then we will put the bundles in the correct order as indicated in subclause 7.3.2.2
   *
   llr --------------------------> e_rx (z) ----> e_rx (z)
   |   ...
   |   ...
   |   REG 26
   symbol 2    |   ...
   |   ...
   |   REG 5
   |   REG 2

   |   ...
   |   ...
   |   REG 25
   symbol 1    |   ...
   |   ...
   |   REG 4
   |   REG 1

   |   ...
   |   ...                           ...              ...
   |   REG 24 (bundle 7)             ...              ...
   symbol 0    |   ...                           bundle 3         bundle 6
   |   ...                           bundle 2         bundle 1
   |   REG 3                         bundle 1         bundle 7
   |   REG 0  (bundle 0)             bundle 0         bundle 0

  */
  const int N_regs = coreset_nbr_rb * coreset_time_dur;
  /* interleaving will be done only if reg_bundle_size_L != 0 */
  const int coreset_C = (reg_bundle_size_L_in != 0) ? (uint32_t)(N_regs / (coreset_interleaver_size_R * reg_bundle_size_L_in)) : 0;
  const int coreset_interleaved = (reg_bundle_size_L_in != 0) ? 1 : 0;
  const int reg_bundle_size_L = (reg_bundle_size_L_in != 0) ? reg_bundle_size_L_in : 6;

  const int B_rb = reg_bundle_size_L / coreset_time_dur; // nb of RBs occupied by each REG bundle
  const int num_bundles_per_cce = 6 / reg_bundle_size_L;
  const int n_cce = N_regs / 6;
  const int max_bundles = n_cce * num_bundles_per_cce;
  int f_bundle_j_list[max_bundles];
  {
    int c = 0;
    int r = 0;
    // for each bundle
    for (int nb = 0; nb < max_bundles; nb++) {
      uint16_t f_bundle_j = 0;
      if (coreset_interleaved == 0)
        f_bundle_j = nb;
      else {
        if (r == coreset_interleaver_size_R) {
          r = 0;
          c++;
        }
        f_bundle_j = ((r * coreset_C) + c + n_shift) % (N_regs / reg_bundle_size_L);
        r++;
      }
      f_bundle_j_list[nb] = f_bundle_j;
    }
  }

  // Get cce_list indices by bundle index in ascending order
  int f_bundle_j_list_ord[number_of_candidates][max_bundles];
  for (int c_id = 0; c_id < number_of_candidates; c_id++ ) {
    const int start_bund_cand = CCE[c_id] * num_bundles_per_cce;
    const int max_bund_per_cand = L[c_id] * num_bundles_per_cce;
    int f_bundle_j_list_id = 0;
    for (int nb = 0; nb < max_bundles; nb++) {
      for (int bund_cand = start_bund_cand; bund_cand < start_bund_cand + max_bund_per_cand; bund_cand++) {
        if (f_bundle_j_list[bund_cand] == nb) {
          f_bundle_j_list_ord[c_id][f_bundle_j_list_id] = nb;
          f_bundle_j_list_id++;
        }
      }
    }
  }

  const int data_sc = 9; // 9 sub-carriers with data per PRB
  int rb_count = 0;
  for (int c_id = 0; c_id < number_of_candidates; c_id++ ) {
    for (int symbol_idx = start_symbol; symbol_idx < start_symbol+coreset_time_dur; symbol_idx++) {
      for (int cce_count = 0; cce_count < L[c_id]; cce_count ++) {
        for (int k = 0; k < NR_NB_REG_PER_CCE / reg_bundle_size_L; k++) { // loop over REG bundles
          int f = f_bundle_j_list_ord[c_id][k + NR_NB_REG_PER_CCE * cce_count / reg_bundle_size_L];
          for (int rb = 0; rb < B_rb; rb++) { // loop over the RBs of the bundle
            c16_t *out = e_rx + data_sc * rb_count;
            const c16_t *in = llr + (uint16_t)(f * B_rb + rb + symbol_idx * coreset_nbr_rb) * data_sc;
            for (int i = 0; i < data_sc; i++) {
              out[i] = in[i];
#ifdef NR_PDCCH_DCI_DEBUG
              LOG_I(NR_PHY_DCI,
                    "[candidate=%d,symbol_idx=%d,cce=%d,REG bundle=%d,PRB=%d] z[%d]=(%d,%d) <-> \t llr[%d]=(%d,%d) \n",
                    c_id,
                    symbol_idx,
                    cce_count,
                    k,
                    f * B_rb + rb,
                    (index_z + i),
                    out->r,
                    out->i,
                    index_llr + i,
                    in.r,
                    in.i);
#endif
            }
            rb_count++;
          }
        }
      }
    }
  }
}

static void nr_pdcch_llr(const NR_DL_FRAME_PARMS *frame_parms,
                         const int32_t rx_size,
                         const int rel_symb_monOcc,
                         const c16_t rxdataF_comp[][rx_size],
                         const int llrSize,
                         c16_t pdcch_llr[llrSize],
                         const uint32_t coreset_nbr_rb)
{
  const c16_t *rxF = &rxdataF_comp[0][0];
  c16_t *pdcch_llrp = &pdcch_llr[rel_symb_monOcc * coreset_nbr_rb * 9];

  if (!pdcch_llrp) {
    LOG_E(NR_PHY_DCI, "pdcch_qpsk_llr: llr is null\n");
    return;
  }

  LOG_DDD("llr logs: pdcch qpsk llr for symbol %d (pos %d), llr offset %ld\n",
          symbol,
          (symbol * frame_parms->N_RB_DL * 12),
          pdcch_llrp - pdcch_llr);

  for (int i = 0; i < coreset_nbr_rb * 9; i++) {
    // We clip the signal
    c16_t res;
    res.r = min(rxF->r, 31);
    res.r = max(-32, res.r);
    res.i = min(rxF->i, 31);
    res.i = max(-32, res.i);
    *pdcch_llrp = res;

    LOG_DDD("llr logs: rb=%d i=%d *rxF:%d => *pdcch_llrp:%d\n", i / 18, i, *rxF, *pdcch_llrp);
    rxF++;
    pdcch_llrp++;
  }
}

// compute average channel_level on each (TX,RX) antenna pair
static void nr_pdcch_channel_level(const int32_t rx_size,
                                   const c16_t dl_ch_estimates_ext[][rx_size],
                                   const NR_DL_FRAME_PARMS *frame_parms,
                                   int32_t *avg,
                                   const uint8_t nb_rb)
{
  for (int aarx = 0; aarx < frame_parms->nb_antennas_rx; aarx++) {
    // clear average level
    simde__m128i avg128P = simde_mm_setzero_si128();
    simde__m128i *dl_ch128 = (simde__m128i *)&dl_ch_estimates_ext[aarx][0];

    for (int rb = 0; rb < (nb_rb * 3) >> 2; rb++) {
      avg128P = simde_mm_add_epi32(avg128P, simde_mm_madd_epi16(dl_ch128[0], dl_ch128[0]));
      avg128P = simde_mm_add_epi32(avg128P, simde_mm_madd_epi16(dl_ch128[1], dl_ch128[1]));
      avg128P = simde_mm_add_epi32(avg128P, simde_mm_madd_epi16(dl_ch128[2], dl_ch128[2]));
      //      for (int i=0;i<24;i+=2) printf("pdcch channel re %d
      //      (%d,%d)\n",(rb*12)+(i>>1),((int16_t*)dl_ch128)[i],((int16_t*)dl_ch128)[i+1]);
      dl_ch128 += 3;
      /*
      if (rb==0) {
      print_shorts("dl_ch128",&dl_ch128[0]);
      print_shorts("dl_ch128",&dl_ch128[1]);
      print_shorts("dl_ch128",&dl_ch128[2]);
      }
      */
    }

    DevAssert(nb_rb);
    avg[aarx] = 0;
    for (int i = 0; i < 4; i++)
      avg[aarx] += ((int32_t *)&avg128P)[i] / (nb_rb * 9);
    LOG_DDD("Channel level : %d\n", avg[aarx]);
  }
}

// This function will extract the mapped DM-RS PDCCH REs as per 38.211 Section 7.4.1.3.2 (Mapping to physical resources)
static void nr_pdcch_extract_rbs_single(const uint32_t rxdataF_sz,
                                        const c16_t rxdataF[][rxdataF_sz],
                                        const int32_t est_size,
                                        const c16_t dl_ch_estimates[][est_size],
                                        const int32_t rx_size,
                                        c16_t rxdataF_ext[][rx_size],
                                        c16_t dl_ch_estimates_ext[][rx_size],
                                        const NR_DL_FRAME_PARMS *frame_parms,
                                        const uint8_t *coreset_freq_dom,
                                        const uint32_t coreset_nbr_rb,
                                        const uint32_t n_BWP_start)
{
  /*
   * This function is demapping DM-RS PDCCH RE
   * Implementing 38.211 Section 7.4.1.3.2 Mapping to physical resources
   * PDCCH DM-RS signals are mapped on RE a_k_l where:
   * k = 12*n + 4*kprime + 1
   * n=0,1,..
   * kprime=0,1,2
   * According to this equations, DM-RS PDCCH are mapped on k where k%12==1 || k%12==5 || k%12==9
   *
   */

#define NBR_RE_PER_RB_WITH_DMRS 12
  // after removing the 3 DMRS RE, the RB contains 9 RE with PDCCH
#define NBR_RE_PER_RB_WITHOUT_DMRS 9
  // uint8_t rb_count_bit;

  for (int aarx = 0; aarx < frame_parms->nb_antennas_rx; aarx++) {
    const c16_t *dl_ch0 = &dl_ch_estimates[aarx][0];
    const c16_t *rxFbase = rxdataF[aarx];
    LOG_DDD("dl_ch0 = &dl_ch_estimates[aarx = (%d)][0]\n", aarx);

    c16_t *dl_ch0_ext = &dl_ch_estimates_ext[aarx][0];
    c16_t *rxF_ext = &rxdataF_ext[aarx][0];

    /*
     * The following for loop handles treatment of PDCCH contained in table rxdataF (in frequency domain)
     * In NR the PDCCH IQ symbols are contained within RBs in the CORESET defined by higher layers which is located within the BWP
     * Lets consider that the first RB to be considered as part of the CORESET and part of the PDCCH is n_BWP_start
     * Several cases have to be handled differently as IQ symbols are situated in different parts of rxdataF:
     * 1. Number of RBs in the system bandwidth is even
     *    1.1 The RB is <  than the N_RB_DL/2 -> IQ symbols are in the second half of the rxdataF (from first_carrier_offset)
     *    1.2 The RB is >= than the N_RB_DL/2 -> IQ symbols are in the first half of the rxdataF (from element 0)
     * 2. Number of RBs in the system bandwidth is odd
     * (particular case when the RB with DC as it is treated differently: it is situated in symbol borders of rxdataF)
     *    2.1 The RB is <  than the N_RB_DL/2 -> IQ symbols are in the second half of the rxdataF (from first_carrier_offset)
     *    2.2 The RB is >  than the N_RB_DL/2 -> IQ symbols are in the first half of the rxdataF (from element 0 + 2nd half RB
     * containing DC) 2.3 The RB is == N_RB_DL/2          -> IQ symbols are in the upper border of the rxdataF for first 6 IQ
     * element and the lower border of the rxdataF for the last 6 IQ elements If the first RB containing PDCCH within the UE BWP
     * and within the CORESET is higher than half of the system bandwidth (N_RB_DL), then the IQ symbol is going to be found at
     * the position 0+c_rb-N_RB_DL/2 in rxdataF and we have to point the pointer at (1+c_rb-N_RB_DL/2) in rxdataF
     */

    int c_rb = 0;
    for (int rb = 0; rb < coreset_nbr_rb; rb++, c_rb++) {
      int c_rb_by6 = c_rb / 6;

      // skip zeros in frequency domain bitmap
      while ((coreset_freq_dom[c_rb_by6 >> 3] & (1 << (7 - (c_rb_by6 & 7)))) == 0) {
        c_rb += 6;
        c_rb_by6 = c_rb / 6;
      }

      // first we set initial conditions for pointer to rxdataF depending on the situation of the first RB within the CORESET
      // (c_rb = n_BWP_start)
      if ((frame_parms->N_RB_DL & 1) == 1 && (c_rb + n_BWP_start) == (frame_parms->N_RB_DL >> 1)) {
        // treatment of RB containing the DC
        // if odd number RBs in system bandwidth and first RB to be treated is higher than middle system bandwidth (around DC)
        // we have to treat the RB in two parts: first part from i=0 to 5, the data is at the end of rxdataF (pointing at the
        // end of the table)
        const c16_t *rxF = rxFbase + frame_parms->first_carrier_offset + 12 * (c_rb + n_BWP_start);

        int i = 0, j = 0;
        for (; i < 6; i++) { // treating first part of the RB note that i=5 would correspond to DC. We treat it in NR
          if ((i != 1) && (i != 5)) {
            dl_ch0_ext[j] = dl_ch0[i];
            rxF_ext[j] = rxF[i];
            LOG_DSYMB("");
            j++;
          } else {
            LOG_DSYMB("\t\t <==> DM-RS PDCCH, this is a pilot symbol\n");
          }
        }

        // then we point at the begining of the symbol part of rxdataF do process second part of RB
        for (; i < 12; i++) {
          if ((i != 9)) {
            dl_ch0_ext[j] = dl_ch0[i];
            rxF_ext[j] = rxFbase[i - 6];
            LOG_DSYMB("");
            j++;
          } else {
            LOG_DSYMB("\t\t <==> DM-RS PDCCH, this is a pilot symbol\n");
          }
        }

      } else { // treatment of any RB that does not contain the DC
        const c16_t *rxF = NULL;
        if ((frame_parms->N_RB_DL & 1) == 0) {
          if ((c_rb + n_BWP_start) < (frame_parms->N_RB_DL >> 1))
            // if RB to be treated is lower than middle system bandwidth then rxdataF pointed
            // at (offset + c_br + symbol * ofdm_symbol_size): even case
            rxF = rxFbase + (frame_parms->first_carrier_offset + 12 * c_rb) + n_BWP_start * 12;
          else
            // number of RBs is even  and c_rb is higher than half system bandwidth (we don't skip DC)
            // if these conditions are true the pointer has to be situated at the 1st part of the rxdataF
            // we point at the 1st part of the rxdataF in symbol
            rxF = rxFbase + 12 * (c_rb + n_BWP_start - (frame_parms->N_RB_DL >> 1));
        } else {
          if ((c_rb + n_BWP_start) < (frame_parms->N_RB_DL >> 1))
            // if RB to be treated is lower than middle system bandwidth then rxdataF pointed
            //  at (offset + c_br + symbol * ofdm_symbol_size): odd case
            rxF = rxFbase + frame_parms->first_carrier_offset + 12 * (c_rb + n_BWP_start);
          else if ((c_rb + n_BWP_start) > (frame_parms->N_RB_DL >> 1))
            // number of RBs is odd  and   c_rb is higher than half system bandwidth + 1
            // if these conditions are true the pointer has to be situated at the 1st part of
            // the rxdataF just after the first IQ symbols of the RB containing DC
            // we point at the 1st part of the rxdataF in symbol
            rxF = rxFbase + 12 * (c_rb + n_BWP_start - (frame_parms->N_RB_DL >> 1)) - 6;
        }
        AssertFatal(rxF, "bug");
        int j = 0;

        for (int i = 0; i < 12; i++) {
          if ((i != 1) && (i != 5) && (i != 9)) {
            rxF_ext[j] = rxF[i];
            dl_ch0_ext[j] = dl_ch0[i];
            LOG_DSYMB("");
            j++;
          } else {
            LOG_DSYMB("\t\t <==> DM-RS PDCCH, this is a pilot symbol\n");
          }
        }
      }
      dl_ch0_ext += NBR_RE_PER_RB_WITHOUT_DMRS;
      rxF_ext += NBR_RE_PER_RB_WITHOUT_DMRS;
      dl_ch0 += 12;
    }
  }
}

#define print_shorts(s, x) printf("%s %d,%d,%d,%d,%d,%d,%d,%d\n", s, (x)[0], (x)[1], (x)[2], (x)[3], (x)[4], (x)[5], (x)[6], (x)[7])

static void nr_pdcch_channel_compensation(const int32_t rx_size,
                                          const c16_t rxdataF_ext[][rx_size],
                                          const c16_t dl_ch_estimates_ext[][rx_size],
                                          c16_t rxdataF_comp[][rx_size],
                                          const NR_DL_FRAME_PARMS *frame_parms,
                                          const uint8_t output_shift,
                                          const uint32_t coreset_nbr_rb)
{
  for (int aarx = 0; aarx < frame_parms->nb_antennas_rx; aarx++) {
    const simde__m128i *dl_ch128 = (simde__m128i *)&dl_ch_estimates_ext[aarx][0];
    const simde__m128i *rxdataF128 = (simde__m128i *)&rxdataF_ext[aarx][0];
    simde__m128i *rxdataF_comp128 = (simde__m128i *)&rxdataF_comp[aarx][0];
    // printf("ch compensation dl_ch ext addr %p \n", &dl_ch_estimates_ext[(aatx<<1)+aarx][symbol*20*12]);
    // printf("rxdataf ext addr %p symbol %d\n", &rxdataF_ext[aarx][symbol*20*12], symbol);
    // printf("rxdataf_comp addr %p\n",&rxdataF_comp[(aatx<<1)+aarx][symbol*20*12]);

    for (int rb = 0; rb < (coreset_nbr_rb * 3) >> 2; rb++) {
      // multiply by conjugated channel
      simde__m128i mmtmpP0 = simde_mm_madd_epi16(dl_ch128[0], rxdataF128[0]);
      // print_ints("re",&mmtmpP0);
      //  mmtmpP0 contains real part of 4 consecutive outputs (32-bit)
      simde__m128i mmtmpP1 = simde_mm_shufflelo_epi16(dl_ch128[0], SIMDE_MM_SHUFFLE(2, 3, 0, 1));
      mmtmpP1 = simde_mm_shufflehi_epi16(mmtmpP1, SIMDE_MM_SHUFFLE(2, 3, 0, 1));
      mmtmpP1 = simde_mm_sign_epi16(mmtmpP1, *(simde__m128i *)&conjugate[0]);
      // print_ints("im",&mmtmpP1);
      mmtmpP1 = simde_mm_madd_epi16(mmtmpP1, rxdataF128[0]);
      // mmtmpP1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpP0 = simde_mm_srai_epi32(mmtmpP0, output_shift);
      //  print_ints("re(shift)",&mmtmpP0);
      mmtmpP1 = simde_mm_srai_epi32(mmtmpP1, output_shift);
      //  print_ints("im(shift)",&mmtmpP1);
      simde__m128i mmtmpP2 = simde_mm_unpacklo_epi32(mmtmpP0, mmtmpP1);
      simde__m128i mmtmpP3 = simde_mm_unpackhi_epi32(mmtmpP0, mmtmpP1);
      // print_ints("c0",&mmtmpP2);
      // print_ints("c1",&mmtmpP3);
      rxdataF_comp128[0] = simde_mm_packs_epi32(mmtmpP2, mmtmpP3);
      //      print_shorts("rx:",(int16_t*)rxdataF128);
      //      print_shorts("ch:",(int16_t*)dl_ch128);
      //      print_shorts("pack:",(int16_t*)rxdataF_comp128);
      // multiply by conjugated channel
      mmtmpP0 = simde_mm_madd_epi16(dl_ch128[1], rxdataF128[1]);
      // mmtmpP0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpP1 = simde_mm_shufflelo_epi16(dl_ch128[1], SIMDE_MM_SHUFFLE(2, 3, 0, 1));
      mmtmpP1 = simde_mm_shufflehi_epi16(mmtmpP1, SIMDE_MM_SHUFFLE(2, 3, 0, 1));
      mmtmpP1 = simde_mm_sign_epi16(mmtmpP1, *(simde__m128i *)&conjugate[0]);
      mmtmpP1 = simde_mm_madd_epi16(mmtmpP1, rxdataF128[1]);
      // mmtmpP1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpP0 = simde_mm_srai_epi32(mmtmpP0, output_shift);
      mmtmpP1 = simde_mm_srai_epi32(mmtmpP1, output_shift);
      mmtmpP2 = simde_mm_unpacklo_epi32(mmtmpP0, mmtmpP1);
      mmtmpP3 = simde_mm_unpackhi_epi32(mmtmpP0, mmtmpP1);
      rxdataF_comp128[1] = simde_mm_packs_epi32(mmtmpP2, mmtmpP3);
      // print_shorts("rx:",rxdataF128+1);
      // print_shorts("ch:",dl_ch128+1);
      // print_shorts("pack:",rxdataF_comp128+1);
      //  multiply by conjugated channel
      mmtmpP0 = simde_mm_madd_epi16(dl_ch128[2], rxdataF128[2]);
      // mmtmpP0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpP1 = simde_mm_shufflelo_epi16(dl_ch128[2], SIMDE_MM_SHUFFLE(2, 3, 0, 1));
      mmtmpP1 = simde_mm_shufflehi_epi16(mmtmpP1, SIMDE_MM_SHUFFLE(2, 3, 0, 1));
      mmtmpP1 = simde_mm_sign_epi16(mmtmpP1, *(simde__m128i *)&conjugate[0]);
      mmtmpP1 = simde_mm_madd_epi16(mmtmpP1, rxdataF128[2]);
      // mmtmpP1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpP0 = simde_mm_srai_epi32(mmtmpP0, output_shift);
      mmtmpP1 = simde_mm_srai_epi32(mmtmpP1, output_shift);
      mmtmpP2 = simde_mm_unpacklo_epi32(mmtmpP0, mmtmpP1);
      mmtmpP3 = simde_mm_unpackhi_epi32(mmtmpP0, mmtmpP1);
      rxdataF_comp128[2] = simde_mm_packs_epi32(mmtmpP2, mmtmpP3);
      ///////////////////////////////////////////////////////////////////////////////////////////////
      // print_shorts("rx:",rxdataF128+2);
      // print_shorts("ch:",dl_ch128+2);
      // print_shorts("pack:",rxdataF_comp128+2);

      for (int i = 0; i < 12; i++)
        LOG_DDD("rxdataF128[%d]=(%d,%d) X dlch[%d]=(%d,%d) rxdataF_comp128[%d]=(%d,%d)\n",
                (rb * 12) + i,
                ((c16_t *)rxdataF128)[i].r,
                ((c16_t *)rxdataF128)[i].i,
                (rb * 12) + i,
                ((c16_t *)dl_ch128)[i].r,
                ((c16_t *)dl_ch128)[i].i,
                (rb * 12) + i,
                ((c16_t *)rxdataF_comp128)[i].r,
                ((c16_t *)rxdataF_comp128)[i].i);

      dl_ch128 += 3;
      rxdataF128 += 3;
      rxdataF_comp128 += 3;
    }
  }
}

static void nr_pdcch_detection_mrc(const NR_DL_FRAME_PARMS *frame_parms, const int32_t rx_size, c16_t rxdataF_comp[][rx_size])
{
  if (frame_parms->nb_antennas_rx > 1) {
    simde__m128i *rxdataF_comp128_0 = (simde__m128i *)&rxdataF_comp[0][0];
    simde__m128i *rxdataF_comp128_1 = (simde__m128i *)&rxdataF_comp[1][0];
    const int sz = frame_parms->N_RB_DL * 3;
    // MRC on each re of rb
    for (int i = 0; i < sz; i++) {
      rxdataF_comp128_0[i] =
          simde_mm_adds_epi16(simde_mm_srai_epi16(rxdataF_comp128_0[i], 1), simde_mm_srai_epi16(rxdataF_comp128_1[i], 1));
    }
  }
}

/* Produce LLRs from received PDCCH signal */
static void nr_rx_pdcch_symbol(const PHY_VARS_NR_UE *ue,
                               const UE_nr_rxtx_proc_t *proc,
                               const int symbol,
                               const int rel_symb_monOcc,
                               const int ss_idx,
                               const nr_phy_data_t *phy_data,
                               const int llrSize,
                               const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
                               c16_t llr[llrSize])
{
  const NR_DL_FRAME_PARMS *fp = &ue->frame_parms;
  const NR_UE_PDCCH_CONFIG *phy_pdcch_config = &phy_data->phy_pdcch_config;
  const fapi_nr_coreset_t *coreset = &phy_pdcch_config->pdcch_config[ss_idx].coreset;
  const int32_t pdcch_est_size = ((((fp->ofdm_symbol_size + LTE_CE_FILTER_LENGTH) + 15) / 16) * 16);
  __attribute__((aligned(16))) c16_t pdcch_dl_ch_estimates[fp->nb_antennas_rx][pdcch_est_size];

  nr_pdcch_channel_estimation(ue,
                              proc,
                              symbol,
                              coreset,
                              fp->first_carrier_offset,
                              phy_pdcch_config->pdcch_config[ss_idx].BWPStart,
                              pdcch_est_size,
                              pdcch_dl_ch_estimates,
                              rxdataF);

  // for (int i=0; i<pdcch_est_size; i++) printf("%d: %d.%d\n", i, ((int16_t *)&pdcch_dl_ch_estimates[0])[i*2], ((int16_t
  // *)&pdcch_dl_ch_estimates[0])[i*2+1]);
  const int32_t rx_size = ((4 * fp->N_RB_DL * 12 + 31) >> 5) << 5;
  __attribute__((aligned(32))) c16_t rxdataF_ext[fp->nb_antennas_rx][rx_size];
  __attribute__((aligned(32))) c16_t rxdataF_comp[fp->nb_antennas_rx][rx_size];
  __attribute__((aligned(32))) c16_t pdcch_dl_ch_estimates_ext[fp->nb_antennas_rx][rx_size];
  memset(rxdataF_comp, 0, sizeof(rxdataF_comp));

  int n_rb;
  int rb_offset;
  get_coreset_rballoc(coreset->frequency_domain_resource, &n_rb, &rb_offset);

  nr_pdcch_extract_rbs_single(ue->frame_parms.ofdm_symbol_size,
                              rxdataF,
                              pdcch_est_size,
                              pdcch_dl_ch_estimates,
                              rx_size,
                              rxdataF_ext,
                              pdcch_dl_ch_estimates_ext,
                              fp,
                              coreset->frequency_domain_resource,
                              n_rb,
                              phy_pdcch_config->pdcch_config[ss_idx].BWPStart);

  int avgP[4];
  nr_pdcch_channel_level(rx_size, pdcch_dl_ch_estimates_ext, fp, avgP, n_rb);

  int avgs = 0;
  for (int aarx = 0; aarx < fp->nb_antennas_rx; aarx++) {
    avgs = cmax(avgs, avgP[aarx]);
  }

  const int log2_maxh = (log2_approx(avgs) / 2) + 5; //+frame_parms->nb_antennas_rx;

  nr_pdcch_channel_compensation(rx_size,
                                rxdataF_ext,
                                pdcch_dl_ch_estimates_ext,
                                rxdataF_comp,
                                fp,
                                log2_maxh,
                                n_rb); // log2_maxh+I0_shift

  UEscopeCopy(ue, pdcchRxdataF_comp, rxdataF_comp, sizeof(struct complex16), fp->nb_antennas_rx, rx_size, 0);

  if (fp->nb_antennas_rx > 1) {
    nr_pdcch_detection_mrc(fp, rx_size, rxdataF_comp);
  }

  nr_pdcch_llr(fp, rx_size, rel_symb_monOcc, rxdataF_comp, llrSize, llr, n_rb);
}

bool is_start_symbol_in_ss(const fapi_nr_dl_config_dci_dl_pdu_rel15_t *ss, const int symbol)
{
  return ((ss->coreset.StartSymbolBitmap >> (NR_SYMBOLS_PER_SLOT - 1 - symbol)) & 1);
}

int get_pdcch_mon_occasions_slot(const fapi_nr_dl_config_dci_dl_pdu_rel15_t *ss, uint8_t start_symb[NR_SYMBOLS_PER_SLOT])
{
  int sum = 0;
  for (int s = 0; s < NR_SYMBOLS_PER_SLOT; s++) {
    if (is_start_symbol_in_ss(ss, s)) {
      if (start_symb != NULL)
        start_symb[sum] = s;
      sum++;
    }
  }

  return sum;
}

int get_max_pdcch_monOcc(const NR_UE_PDCCH_CONFIG *phy_pdcch_config)
{
  int monOcc = 0;
  for (int ss = 0; ss < phy_pdcch_config->nb_search_space; ss++) {
    monOcc = max(monOcc, get_pdcch_mon_occasions_slot(&phy_pdcch_config->pdcch_config[ss], NULL));
  }
  return monOcc;
}

static void nr_dci_decoding_procedure(const PHY_VARS_NR_UE *ue,
                                      const UE_nr_rxtx_proc_t *proc,
                                      c16_t *pdcch_e_rx,
                                      const fapi_nr_dl_config_dci_dl_pdu_rel15_t *rel15,
                                      int *dci_thres,
                                      fapi_nr_dci_indication_t *dci_ind);

/* Decode DCI from LLRs */
void nr_pdcch_decode(const UE_nr_rxtx_proc_t *proc,
                    const int ss_idx,
                    const nr_phy_data_t *phy_data,
                    const int llrSize,
                    const int max_monOcc,
                    const c16_t llr[llrSize * max_monOcc],
                    PHY_VARS_NR_UE *ue,
                    fapi_nr_dci_indication_t *dci_ind)
{
  UEscopeCopy(ue, pdcchLlr, llr, sizeof(c16_t), 1, llrSize, 0);

  const NR_UE_PDCCH_CONFIG *phy_pdcch_config = &phy_data->phy_pdcch_config;

  const fapi_nr_dl_config_dci_dl_pdu_rel15_t *rel15 = &phy_pdcch_config->pdcch_config[ss_idx];

  start_meas(&ue->dlsch_rx_pdcch_stats);

  int n_rb;
  int rb_offset;
  get_coreset_rballoc((const uint8_t *)rel15->coreset.frequency_domain_resource, &n_rb, &rb_offset);

  uint8_t start_symb[NR_SYMBOLS_PER_SLOT] = {0};
  const int num_monitoring_occ = get_pdcch_mon_occasions_slot(rel15, start_symb);

  for (int m = 0; m < num_monitoring_occ; m++) {
    /// PDCCH/DCI e-sequence (input to rate matching).
    c16_t pdcch_e_rx[NR_MAX_PDCCH_SIZE];

    nr_pdcch_demapping_deinterleaving(&llr[m * llrSize],
                                      pdcch_e_rx,
                                      rel15->coreset.duration,
                                      0, // always 0 because llr buffer is only for current monitoring occasion
                                      n_rb,
                                      rel15->coreset.RegBundleSize,
                                      rel15->coreset.InterleaverSize,
                                      rel15->coreset.ShiftIndex,
                                      rel15->number_of_candidates,
                                      rel15->CCE,
                                      rel15->L);

    nr_dci_decoding_procedure(ue, proc, pdcch_e_rx, rel15, &ue->dci_thres, dci_ind);
  }
}

void set_first_last_pdcch_symb(const NR_UE_PDCCH_CONFIG *phy_pdcch_config, int *first_symb, int *last_symb)
{
  *first_symb = NR_SYMBOLS_PER_SLOT; // max first pdcch symbol
  *last_symb = 0; // min last pdcch symbol
  for (int ss = 0; ss < phy_pdcch_config->nb_search_space; ss++) {
    for (int symb = 0; symb < NR_SYMBOLS_PER_SLOT; symb++) {
      if (is_start_symbol_in_ss(&phy_pdcch_config->pdcch_config[ss], symb)) {
        const int duration = phy_pdcch_config->pdcch_config[ss].coreset.duration;
        *first_symb = min(*first_symb, symb);
        *last_symb = max(*last_symb, symb + duration - 1);
      }
    }
  }
}

/* Generates PDCCH LLRs from received symbol for each Search-Space */
void nr_pdcch_generate_llr(const PHY_VARS_NR_UE *ue,
                           const UE_nr_rxtx_proc_t *proc,
                           const int symbol,
                           const nr_phy_data_t *phy_data,
                           const int llrSize,
                           const int max_monOcc,
                           const c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size],
                           c16_t llr[phy_data->phy_pdcch_config.nb_search_space * max_monOcc * llrSize])
{
  const NR_UE_PDCCH_CONFIG *phy_pdcch_config = &phy_data->phy_pdcch_config;

  // Loop over search spaces
  for (int ss_idx = 0; ss_idx < phy_pdcch_config->nb_search_space; ss_idx++) {
    uint8_t start_symb[NR_SYMBOLS_PER_SLOT] = {0};
    const int num_monOcc = get_pdcch_mon_occasions_slot(&phy_pdcch_config->pdcch_config[ss_idx], start_symb);
    // Loop over monitoring occations within the slot in this ss
    for (int m = 0; m < num_monOcc; m++) {
      const int first_symb = start_symb[m];
      const int last_symb = first_symb + phy_pdcch_config->pdcch_config[ss_idx].coreset.duration;
      // Decode PDCCH and generate LLR for each ss in this OFDM symbol
      if ((symbol >= first_symb) && (symbol < last_symb)) {
        const int rel_symb_monOcc = symbol - first_symb;
        nr_rx_pdcch_symbol(ue,
                           proc,
                           symbol,
                           rel_symb_monOcc,
                           ss_idx,
                           phy_data,
                           llrSize,
                           rxdataF,
                           &llr[ss_idx * max_monOcc * llrSize + m * llrSize]);
      }
    }
  }
}

/* Decode DCI from LLRs for each Search-Space and send to MAC */
void nr_pdcch_dci_indication(const UE_nr_rxtx_proc_t *proc,
                            const int llrSize,
                            const int max_monOcc,
                            PHY_VARS_NR_UE *ue,
                            nr_phy_data_t *phy_data,
                            const c16_t llr[phy_data->phy_pdcch_config.nb_search_space * max_monOcc * llrSize])
{
  const NR_UE_PDCCH_CONFIG *phy_pdcch_config = (const NR_UE_PDCCH_CONFIG *)&phy_data->phy_pdcch_config;

  nr_downlink_indication_t dl_indication;
  fapi_nr_dci_indication_t dci_ind = {0};

  for (int ss_idx = 0; ss_idx < phy_pdcch_config->nb_search_space; ss_idx++) {
    nr_pdcch_decode(proc,
                    ss_idx,
                    (const nr_phy_data_t *)phy_data,
                    llrSize,
                    max_monOcc,
                    &llr[ss_idx * max_monOcc * llrSize],
                    ue,
                    &dci_ind);
  }

  for (int i = 0; i < dci_ind.number_of_dcis; i++) {
    LOG_D(PHY,
          "Frame.slot: %d.%d: DCI %i of %d total DCIs found --> rnti %x : format %d\n",
          proc->frame_rx,
          proc->nr_slot_rx,
          i + 1,
          dci_ind.number_of_dcis,
          dci_ind.dci_list[i].rnti,
          dci_ind.dci_list[i].dci_format);
  }

  /* Send to MAC */
  nr_fill_dl_indication(&dl_indication, &dci_ind, NULL, proc, ue, phy_data);
  ue->if_inst->dl_indication(&dl_indication);
}

static void nr_pdcch_unscrambling(c16_t *e_rx,
                                  uint16_t scrambling_RNTI,
                                  uint32_t length,
                                  uint16_t pdcch_DMRS_scrambling_id,
                                  int16_t *z2)
{
  int i;
  uint8_t reset;
  uint32_t x1 = 0, x2 = 0, s = 0;
  uint16_t n_id; //{0,1,...,65535}
  uint32_t rnti = (uint32_t) scrambling_RNTI;
  reset = 1;
  // x1 is set in first call to lte_gold_generic
  n_id = pdcch_DMRS_scrambling_id;
  x2 = ((rnti << 16) + n_id) % (1U << 31); // this is c_init in 38.211 v15.1.0 Section 7.3.2.3

  LOG_D(NR_PHY_DCI, "PDCCH Unscrambling x2 %x : scrambling_RNTI %x\n", x2, rnti);
  int16_t *ptr = &e_rx[0].r;
  for (i = 0; i < length; i++) {
    if ((i & 0x1f) == 0) {
      s = lte_gold_generic(&x1, &x2, reset);
      reset = 0;
    }

    if (((s >> (i % 32)) & 1) == 1)
      z2[i] = -ptr[i];
    else
      z2[i] = ptr[i];
  }
}


/* This function compares the received DCI bits with
 * re-encoded DCI bits and returns the number of mismatched bits
 */
static uint16_t nr_dci_false_detection(const int16_t *soft_in,
                                       const int encoded_length,
                                       const int rnti,
                                       const int8_t messageType,
                                       const uint16_t messageLength,
                                       const uint8_t aggregation_level,
                                       uint64_t *dci)
{
  uint32_t encoder_output[NR_MAX_DCI_SIZE_DWORD];
  polar_encoder_fast(dci, (void*)encoder_output, rnti, 1,
                    messageType, messageLength, aggregation_level);
  uint8_t *enout_p = (uint8_t*)encoder_output;
  uint16_t x = 0;

  for (int i=0; i<encoded_length/8; i++) {
    x += ( enout_p[i] & 1 ) ^ ( ( soft_in[i*8] >> 15 ) & 1);
    x += ( ( enout_p[i] >> 1 ) & 1 ) ^ ( ( soft_in[i*8+1] >> 15 ) & 1 );
    x += ( ( enout_p[i] >> 2 ) & 1 ) ^ ( ( soft_in[i*8+2] >> 15 ) & 1 );
    x += ( ( enout_p[i] >> 3 ) & 1 ) ^ ( ( soft_in[i*8+3] >> 15 ) & 1 );
    x += ( ( enout_p[i] >> 4 ) & 1 ) ^ ( ( soft_in[i*8+4] >> 15 ) & 1 );
    x += ( ( enout_p[i] >> 5 ) & 1 ) ^ ( ( soft_in[i*8+5] >> 15 ) & 1 );
    x += ( ( enout_p[i] >> 6 ) & 1 ) ^ ( ( soft_in[i*8+6] >> 15 ) & 1 );
    x += ( ( enout_p[i] >> 7 ) & 1 ) ^ ( ( soft_in[i*8+7] >> 15 ) & 1 );
  }
  return x;
}

void nr_dci_decoding_procedure(const PHY_VARS_NR_UE *ue,
                               const UE_nr_rxtx_proc_t *proc,
                               c16_t *pdcch_e_rx,
                               const fapi_nr_dl_config_dci_dl_pdu_rel15_t *rel15,
                               int *dci_thres,
                               fapi_nr_dci_indication_t *dci_ind)
{
  int e_rx_cand_idx = 0;
  *dci_ind = (fapi_nr_dci_indication_t){.SFN = proc->frame_rx, .slot = proc->nr_slot_rx};

  for (int j=0;j<rel15->number_of_candidates;j++) {
    const int CCEind = rel15->CCE[j];
    const int L = rel15->L[j];

    // Loop over possible DCI lengths
    
    for (int k = 0; k < rel15->num_dci_options; k++) {
      // skip this candidate if we've already found one with the
      // same rnti and format at a different aggregation level
      int ind;
      for (ind = 0; ind < dci_ind->number_of_dcis; ind++) {
        if (rel15->rnti == dci_ind->dci_list[ind].rnti && rel15->dci_format_options[k] == dci_ind->dci_list[ind].dci_format) {
          break;
        }
      }
      if (ind < dci_ind->number_of_dcis)
        continue;
      int dci_length = rel15->dci_length_options[k];
      uint64_t dci_estimation[2]= {0};

      LOG_D(NR_PHY_DCI,
            "(%i.%i) Trying DCI candidate %d of %d number of candidates, CCE %d (%d), L %d, length %d, format %s\n",
            proc->frame_rx,
            proc->nr_slot_rx,
            j,
            rel15->number_of_candidates,
            CCEind,
            e_rx_cand_idx,
            L,
            dci_length,
            nr_dci_format_string[rel15->dci_format_options[k]]);

      int16_t tmp_e[16 * 108];
      nr_pdcch_unscrambling(&pdcch_e_rx[e_rx_cand_idx], rel15->coreset.scrambling_rnti, L*108, rel15->coreset.pdcch_dmrs_scrambling_id, tmp_e);
      // this polar version decodes 64 bits max, dci_estimation[1] will never be filled
      uint16_t crc = polar_decoder_int16(tmp_e,
                                         dci_estimation,
                                         1,
                                         NR_POLAR_DCI_MESSAGE_TYPE, dci_length, L);

      rnti_t n_rnti = rel15->rnti;
      LOG_D(NR_PHY_DCI,
            "(%i.%i) dci indication (rnti %x,dci format %s,n_CCE %d,payloadSize %d,payload %lx, is rnti: %d )\n",
            proc->frame_rx,
            proc->nr_slot_rx,
            n_rnti,
            nr_dci_format_string[rel15->dci_format_options[k]],
            CCEind,
            dci_length,
            dci_estimation[0],
            crc == n_rnti);
      if (crc == n_rnti) {
        uint16_t mb = nr_dci_false_detection(tmp_e, L * 108, n_rnti, NR_POLAR_DCI_MESSAGE_TYPE, dci_length, L, dci_estimation);
        *dci_thres = (*dci_thres + mb) / 2;
        if (mb > (*dci_thres + 30)) {
          LOG_W(NR_PHY_DCI,
                "DCI false positive. Dropping DCI index %d. Mismatched bits: %d/%d. Current DCI threshold: %d\n",
                j,
                mb,
                L * 108,
                ue->dci_thres);
          continue;
        } else {
          AssertFatal(dci_ind->number_of_dcis < sizeofArray(dci_ind->dci_list), "Fix allocation\n");
          fapi_nr_dci_indication_pdu_t *dci = dci_ind->dci_list + dci_ind->number_of_dcis;
          *dci = (fapi_nr_dci_indication_pdu_t){
              .rnti = n_rnti,
              .n_CCE = CCEind,
              .N_CCE = L,
              .dci_format = rel15->dci_format_options[k],
              .ss_type = rel15->ss_type_options[k],
              .coreset_type = rel15->coreset.CoreSetType,
          };
          int n_rb, rb_offset;
          get_coreset_rballoc(rel15->coreset.frequency_domain_resource, &n_rb, &rb_offset);
          dci->cset_start = rel15->BWPStart + rb_offset;
          dci->payloadSize = dci_length;
          memcpy(dci->payloadBits, dci_estimation, (dci_length + 7) / 8);
          dci_ind->number_of_dcis++;
          break;    // If DCI is found, no need to check for remaining DCI lengths
        }
      } else {
        LOG_D(NR_PHY_DCI,
              "(%i.%i) Decoded crc %x does not match rnti %x for DCI format %d\n",
              proc->frame_rx,
              proc->nr_slot_rx,
              crc,
              n_rnti,
              rel15->dci_format_options[k]);
      }
    }
    e_rx_cand_idx += 9 * L * 6; // e_rx index for next candidate (L CCEs, 6 REGs per CCE and 9 REs per REG )
  }
}
