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

#include "PHY/defs_nr_UE.h"
#include "PHY/defs_gNB.h"
#include "modulation_UE.h"
#include "nr_modulation.h"
#include "PHY/LTE_ESTIMATION/lte_estimation.h"
#include "PHY/NR_UE_ESTIMATION/nr_estimation.h"
#include <common/utils/LOG/log.h>

//#define DEBUG_FEP

/*#ifdef LOG_I
#undef LOG_I
#define LOG_I(A,B...) printf(A)
#endif*/

/* rxdata & rxdataF should be 16 bytes aligned */
void nr_symbol_fep(
    const PHY_VARS_NR_UE *ue,
    const int slot,
    const unsigned char symbol,
    const int link_type,
    const c16_t rxdata[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size + ue->frame_parms.nb_prefix_samples0],
    c16_t rxdataF[ue->frame_parms.nb_antennas_rx][ue->frame_parms.ofdm_symbol_size])
{
  const NR_DL_FRAME_PARMS *frame_parms = &ue->frame_parms;

  AssertFatal(symbol < frame_parms->symbols_per_slot, "slot_fep: symbol must be between 0 and %d\n", frame_parms->symbols_per_slot-1);
  AssertFatal(slot < frame_parms->slots_per_frame, "slot_fep: Ns must be between 0 and %d\n", frame_parms->slots_per_frame - 1);

  dft_size_idx_t dftsize = get_dft(frame_parms->ofdm_symbol_size);
  for (unsigned char aa = 0; aa < frame_parms->nb_antennas_rx; aa++) {
    dft(dftsize, (int16_t *)&rxdata[aa][0], (int16_t *)&rxdataF[aa][0], 1);

    apply_nr_rotation_RX(frame_parms, rxdataF[aa], frame_parms->symbol_rotation[link_type], slot, frame_parms->N_RB_DL, 0, symbol, 1, true);
  }
}

int nr_slot_fep_ul(NR_DL_FRAME_PARMS *frame_parms,
                   int32_t *rxdata,
                   int32_t *rxdataF,
                   unsigned char symbol,
                   unsigned char Ns,
                   int sample_offset)
{
  unsigned int nb_prefix_samples  = frame_parms->nb_prefix_samples;
  unsigned int nb_prefix_samples0 = frame_parms->nb_prefix_samples0;

  dft_size_idx_t dftsize = get_dft(frame_parms->ofdm_symbol_size);
  // This is for misalignment issues
  int32_t tmp_dft_in[8192] __attribute__ ((aligned (32)));

  // offset of first OFDM symbol
  unsigned int rxdata_offset = frame_parms->get_samples_slot_timestamp(Ns,frame_parms,0);
  unsigned int abs_symbol = Ns * frame_parms->symbols_per_slot + symbol;
  for (int idx_symb = Ns*frame_parms->symbols_per_slot; idx_symb <= abs_symbol; idx_symb++)
    rxdata_offset += (idx_symb%(0x7<<frame_parms->numerology_index)) ? nb_prefix_samples : nb_prefix_samples0;
  rxdata_offset += frame_parms->ofdm_symbol_size * symbol;

  // use OFDM symbol from within 1/8th of the CP to avoid ISI
  rxdata_offset -= (nb_prefix_samples / frame_parms->ofdm_offset_divisor);

  int16_t *rxdata_ptr;

  if(sample_offset > rxdata_offset) {

    memcpy((void *)&tmp_dft_in[0],
           (void *)&rxdata[frame_parms->samples_per_frame - sample_offset + rxdata_offset],
           (sample_offset - rxdata_offset) * sizeof(int32_t));
    memcpy((void *)&tmp_dft_in[sample_offset - rxdata_offset],
           (void *)&rxdata[0],
           (frame_parms->ofdm_symbol_size - sample_offset + rxdata_offset) * sizeof(int32_t));
    rxdata_ptr = (int16_t *)tmp_dft_in;

  } else if (((rxdata_offset - sample_offset) & 7) != 0) {

    // if input to dft is not 256-bit aligned
    memcpy((void *)&tmp_dft_in[0],
           (void *)&rxdata[rxdata_offset - sample_offset],
           (frame_parms->ofdm_symbol_size) * sizeof(int32_t));
    rxdata_ptr = (int16_t *)tmp_dft_in;

  } else {

    // use dft input from RX buffer directly
    rxdata_ptr = (int16_t *)&rxdata[rxdata_offset - sample_offset];

  }

  dft(dftsize,
      rxdata_ptr,
      (int16_t *)&rxdataF[symbol * frame_parms->ofdm_symbol_size],
      1);

  return 0;
}

static void apply_nr_rotation_symbol_RX(const NR_DL_FRAME_PARMS *frame_parms,
                                        c16_t *this_symbol,
                                        const c16_t *shift_rot,
                                        const c16_t rot,
                                        const int nb_rb)
{
  if (nb_rb & 1) {
    rotate_cpx_vector(this_symbol, &rot, this_symbol, (nb_rb + 1) * 6, 15);
    rotate_cpx_vector(this_symbol + frame_parms->first_carrier_offset - 6,
                      &rot,
                      this_symbol + frame_parms->first_carrier_offset - 6,
                      (nb_rb + 1) * 6,
                      15);
    multadd_cpx_vector((int16_t *)this_symbol, (int16_t *)shift_rot, (int16_t *)this_symbol, 1, (nb_rb + 1) * 6, 15);
    multadd_cpx_vector((int16_t *)(this_symbol + frame_parms->first_carrier_offset - 6),
                       (int16_t *)(shift_rot + frame_parms->first_carrier_offset - 6),
                       (int16_t *)(this_symbol + frame_parms->first_carrier_offset - 6),
                       1,
                       (nb_rb + 1) * 6,
                       15);
  } else {
    rotate_cpx_vector(this_symbol, &rot, this_symbol, nb_rb * 6, 15);
    rotate_cpx_vector(this_symbol + frame_parms->first_carrier_offset,
                      &rot,
                      this_symbol + frame_parms->first_carrier_offset,
                      nb_rb * 6,
                      15);
    multadd_cpx_vector((int16_t *)this_symbol, (int16_t *)shift_rot, (int16_t *)this_symbol, 1, nb_rb * 6, 15);
    multadd_cpx_vector((int16_t *)(this_symbol + frame_parms->first_carrier_offset),
                       (int16_t *)(shift_rot + frame_parms->first_carrier_offset),
                       (int16_t *)(this_symbol + frame_parms->first_carrier_offset),
                       1,
                       nb_rb * 6,
                       15);
  }
}

void apply_nr_rotation_RX(const NR_DL_FRAME_PARMS *frame_parms,
                          c16_t *rxdataF,
                          const c16_t *rot,
                          const int slot,
                          const int nb_rb,
                          const int soffset,
                          const int first_symbol,
                          const int nsymb,
                          const bool isUE)
{
  AssertFatal(first_symbol + nsymb <= NR_NUMBER_OF_SYMBOLS_PER_SLOT,
              "First symbol %d and number of symbol %d not compatible with number of symbols in a slot %d\n",
              first_symbol, nsymb, NR_NUMBER_OF_SYMBOLS_PER_SLOT);
  const int symb_offset = (slot % frame_parms->slots_per_subframe) * frame_parms->symbols_per_slot;

  for (int symbol = first_symbol; symbol < first_symbol + nsymb; symbol++) {
    c16_t rot2 = rot[symbol + symb_offset];
    rot2.i = -rot2.i;
    LOG_D(PHY,"slot %d, symb_offset %d rotating by %d.%d\n", slot, symb_offset, rot2.r, rot2.i);
    c16_t *shift_rot = (c16_t *)frame_parms->timeshift_symbol_rotation;
    c16_t *this_symbol = isUE ? (&rxdataF[soffset]) : (&rxdataF[soffset + (frame_parms->ofdm_symbol_size * symbol)]);

    apply_nr_rotation_symbol_RX(frame_parms, this_symbol, shift_rot, rot2, nb_rb);
  }
}
