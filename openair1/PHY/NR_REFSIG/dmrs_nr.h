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

/**********************************************************************
*
* FILENAME    :  dmrs.h
*
* MODULE      :  demodulation reference signals
*
* DESCRIPTION :  generation of dmrs sequences for NR 5G
*                3GPP TS 38.211
*
************************************************************************/

#ifndef DMRS_NR_H
#define DMRS_NR_H

#include "PHY/defs_nr_UE.h"
#include "PHY/types.h"
#include "PHY/NR_REFSIG/ss_pbch_nr.h"
#include "PHY/NR_REFSIG/pss_nr.h"
#include "PHY/NR_REFSIG/sss_nr.h"

/************** CODE GENERATION ***********************************/

/************** DEFINE ********************************************/


/************* STRUCTURES *****************************************/


/************** VARIABLES *****************************************/

/************** FUNCTION ******************************************/

int pseudo_random_sequence(int M_PN, uint32_t *c, uint32_t cinit);
void lte_gold_new(LTE_DL_FRAME_PARMS *frame_parms, uint32_t lte_gold_table[20][2][14], uint16_t Nid_cell);
uint16_t get_dmrs_freq_idx_ul(uint16_t n, uint8_t k_prime, uint8_t delta, uint8_t dmrs_type);

uint8_t allowed_xlsch_re_in_dmrs_symbol(uint16_t k,
                                        uint16_t start_sc,
                                        uint16_t ofdm_symbol_size,
                                        uint8_t numDmrsCdmGrpsNoData,
                                        uint8_t dmrs_type);

void nr_gen_ref_conj_symbols(const uint32_t *in, uint32_t length, int16_t *output, uint16_t offset, int mod_order);
int8_t get_next_dmrs_symbol_in_slot(uint16_t  ul_dmrs_symb_pos, uint8_t counter, uint8_t end_symbol);
uint8_t get_dmrs_symbols_in_slot(uint16_t l_prime_mask,  uint16_t nb_symb, uint8_t start);
int8_t get_valid_dmrs_idx_for_channel_est(uint16_t  dmrs_symb_pos, uint8_t counter);
void nr_chest_time_domain_avg(const NR_DL_FRAME_PARMS *frame_parms,
                              const int num_symbols,
                              const int start_symbol,
                              const int dmrs_bitmap,
                              const int num_rbs,
                              const int antIdx,
                              const int layerIdx,
                              const int Nl,
                              const bool isUE,
                              c16_t *ch_estimates);
int get_first_symb_idx_with_data(const int dmrsBitMap,
                                 const int dmrsType,
                                 const int numDmrsCmdGrp,
                                 const int startSymbol,
                                 const int numSymbs);
static inline uint8_t is_dmrs_symbol(const uint8_t l, const uint16_t dmrsSymbMask)
{
  DevAssert(l < 32);
  return ((dmrsSymbMask >> l) & 0x1);
}
int get_last_dmrs_symbol_in_slot(const int dmrs_symbol_pos);
int get_num_data_symb_before_last_dmrs(const int dmrs_symb_pos);
#undef EXTERN

#endif /* DMRS_NR_H */


