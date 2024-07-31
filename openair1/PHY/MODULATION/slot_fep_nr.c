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

int sl_nr_slot_fep(PHY_VARS_NR_UE *ue,
                   UE_nr_rxtx_proc_t *proc,
                   unsigned char symbol,
                   unsigned char Ns,
                   uint32_t sample_offset,
                   c16_t rxdataF[][ue->SL_UE_PHY_PARAMS.sl_frame_params.samples_per_slot_wCP])
{
  NR_DL_FRAME_PARMS *frame_params = &ue->SL_UE_PHY_PARAMS.sl_frame_params;
  NR_UE_COMMON *common_vars = &ue->common_vars;

  AssertFatal(symbol < frame_params->symbols_per_slot,
              "slot_fep: symbol must be between 0 and %d\n",
              frame_params->symbols_per_slot - 1);
  AssertFatal(Ns < frame_params->slots_per_frame, "slot_fep: Ns must be between 0 and %d\n", frame_params->slots_per_frame - 1);

  unsigned int nb_prefix_samples = frame_params->nb_prefix_samples;
  unsigned int nb_prefix_samples0 = frame_params->nb_prefix_samples0;

  dft_size_idx_t dftsize = get_dft(frame_params->ofdm_symbol_size);
  // This is for misalignment issues
  int32_t tmp_dft_in[8192] __attribute__((aligned(32)));

  unsigned int rx_offset = frame_params->get_samples_slot_timestamp(Ns, frame_params, 0);
  unsigned int abs_symbol = Ns * frame_params->symbols_per_slot + symbol;

  rx_offset += sample_offset;

  for (int idx_symb = Ns * frame_params->symbols_per_slot; idx_symb <= abs_symbol; idx_symb++)
    rx_offset += (idx_symb % (0x7 << frame_params->numerology_index)) ? nb_prefix_samples : nb_prefix_samples0;
  rx_offset += frame_params->ofdm_symbol_size * symbol;

  // use OFDM symbol from within 1/8th of the CP to avoid ISI
  rx_offset -= (nb_prefix_samples / frame_params->ofdm_offset_divisor);

#ifdef SL_DEBUG_SLOT_FEP
  //  if (ue->frame <100)
  LOG_I(PHY,
        "slot_fep: slot %d, symbol %d, nb_prefix_samples %u, nb_prefix_samples0 %u, rx_offset %u\n",
        Ns,
        symbol,
        nb_prefix_samples,
        nb_prefix_samples0,
        rx_offset);
#endif

  for (unsigned char aa = 0; aa < frame_params->nb_antennas_rx; aa++) {
    memset(&rxdataF[aa][frame_params->ofdm_symbol_size * symbol], 0, frame_params->ofdm_symbol_size * sizeof(int32_t));

    int16_t *rxdata_ptr = (int16_t *)&common_vars->rxdata[aa][rx_offset];

    // if input to dft is not 256-bit aligned
    if ((rx_offset & 7) != 0) {
      memcpy((void *)&tmp_dft_in[0], (void *)&common_vars->rxdata[aa][rx_offset], frame_params->ofdm_symbol_size * sizeof(int32_t));

      rxdata_ptr = (int16_t *)tmp_dft_in;
    }

    dft(dftsize, rxdata_ptr, (int16_t *)&rxdataF[aa][frame_params->ofdm_symbol_size * symbol], 1);

    int symb_offset = (Ns % frame_params->slots_per_subframe) * frame_params->symbols_per_slot;
    int32_t rot2 = ((uint32_t *)frame_params->symbol_rotation[2])[symbol + symb_offset];
    ((int16_t *)&rot2)[1] = -((int16_t *)&rot2)[1];

#ifdef SL_DEBUG_SLOT_FEP
    //  if (ue->frame <100)
    LOG_I(PHY,
          "slot_fep: slot %d, symbol %d rx_offset %u, rotation symbol %d %d.%d\n",
          Ns,
          symbol,
          rx_offset,
          symbol + symb_offset,
          ((int16_t *)&rot2)[0],
          ((int16_t *)&rot2)[1]);
#endif

    rotate_cpx_vector((c16_t *)&rxdataF[aa][frame_params->ofdm_symbol_size * symbol],
                      (c16_t *)&rot2,
                      (c16_t *)&rxdataF[aa][frame_params->ofdm_symbol_size * symbol],
                      frame_params->ofdm_symbol_size,
                      15);

    int16_t *shift_rot = (int16_t *)frame_params->timeshift_symbol_rotation;

    mult_cpx_vector((const c16_t *)&rxdataF[aa][frame_params->ofdm_symbol_size * symbol],
                    (const c16_t *)shift_rot,
                    &rxdataF[aa][frame_params->ofdm_symbol_size * symbol],
                    frame_params->ofdm_symbol_size,
                    15);
  }

  LOG_D(PHY, "SIDELINK RX: Slot FEP: done for symbol:%d\n", symbol);

  return 0;
}

/* rxdata & rxdataF should be 16 bytes aligned */
void nr_symbol_fep(const NR_DL_FRAME_PARMS *frame_parms,
                   const int slot,
                   const unsigned char symbol,
                   const int link_type,
                   const c16_t *rxdata[frame_parms->nb_antennas_rx],
                   c16_t *rxdataF[frame_parms->nb_antennas_rx])
{
  AssertFatal(symbol < frame_parms->symbols_per_slot,
              "slot_fep: symbol must be between 0 and %d\n",
              frame_parms->symbols_per_slot - 1);
  AssertFatal(slot < frame_parms->slots_per_frame, "slot_fep: Ns must be between 0 and %d\n", frame_parms->slots_per_frame - 1);

  dft_size_idx_t dftsize = get_dft(frame_parms->ofdm_symbol_size);
  for (unsigned char aa = 0; aa < frame_parms->nb_antennas_rx; aa++) {
    dft(dftsize, (int16_t *)rxdata[aa], (int16_t *)rxdataF[aa], 1);

    apply_nr_rotation_symbol_RX(frame_parms,
                                rxdataF[aa],
                                frame_parms->symbol_rotation[link_type],
                                frame_parms->N_RB_DL,
                                slot,
                                symbol);
  }
}

int nr_slot_fep(PHY_VARS_NR_UE *ue,
                NR_DL_FRAME_PARMS *frame_parms,
                const UE_nr_rxtx_proc_t *proc,
                unsigned char symbol,
                c16_t rxdataF[][frame_parms->samples_per_slot_wCP],
                uint32_t linktype)
{
  const NR_UE_COMMON *common_vars = &ue->common_vars;
  int Ns = proc->nr_slot_rx;

  AssertFatal(symbol < frame_parms->symbols_per_slot,
              "slot_fep: symbol must be between 0 and %d\n",
              frame_parms->symbols_per_slot - 1);
  AssertFatal(Ns < frame_parms->slots_per_frame, "slot_fep: Ns must be between 0 and %d\n", frame_parms->slots_per_frame - 1);

  unsigned int nb_prefix_samples = frame_parms->nb_prefix_samples;
  unsigned int nb_prefix_samples0 = (ue->is_synchronized) ? frame_parms->nb_prefix_samples0 : nb_prefix_samples;

  unsigned int rx_offset = frame_parms->get_samples_slot_timestamp(Ns,frame_parms,0);
  unsigned int abs_symbol = Ns * frame_parms->symbols_per_slot + symbol;
  for (int idx_symb = Ns * frame_parms->symbols_per_slot; idx_symb <= abs_symbol; idx_symb++)
    rx_offset += (idx_symb % (0x7 << frame_parms->numerology_index)) ? nb_prefix_samples : nb_prefix_samples0;
  rx_offset += frame_parms->ofdm_symbol_size * symbol;

  // use OFDM symbol from within 1/8th of the CP to avoid ISI
  rx_offset -= (nb_prefix_samples / frame_parms->ofdm_offset_divisor);

  LOG_D(PHY,
        "slot_fep: slot %d, symbol %d, nb_prefix_samples %u, nb_prefix_samples0 %u, rx_offset %u energy %d\n",
        Ns,
        symbol,
        nb_prefix_samples,
        nb_prefix_samples0,
        rx_offset,
        dB_fixed(signal_energy((int32_t *)&common_vars->rxdata[0][rx_offset], frame_parms->ofdm_symbol_size)));

  const c16_t *rxdata_symb_ptr[frame_parms->nb_antennas_rx];
  c16_t *rxdataF_symb_ptr[frame_parms->nb_antennas_rx];
  for (unsigned char aa = 0; aa < frame_parms->nb_antennas_rx; aa++) {
    rxdata_symb_ptr[aa] = &common_vars->rxdata[aa][rx_offset];
    rxdataF_symb_ptr[aa] = &rxdataF[aa][frame_parms->ofdm_symbol_size * symbol];
  }
  start_meas(&ue->rx_dft_stats);
  nr_symbol_fep(&ue->frame_parms, Ns, symbol, linktype, rxdata_symb_ptr, rxdataF_symb_ptr);
  stop_meas(&ue->rx_dft_stats);
  return 0;
}

int nr_slot_fep_init_sync(const NR_DL_FRAME_PARMS *frame_parms,
                          const unsigned int symbol,
                          const unsigned int sample_offset,
                          const c16_t **rxdata,
                          const unsigned int link_type,
                          c16_t rxdataF[][frame_parms->samples_per_slot_wCP])
{
  const int slot = 0;

  AssertFatal(symbol < frame_parms->symbols_per_slot,
              "slot_fep: symbol must be between 0 and %d\n",
              frame_parms->symbols_per_slot - 1);

  unsigned int nb_prefix_samples = frame_parms->nb_prefix_samples;
  unsigned int frame_length_samples = frame_parms->samples_per_frame;

  // This is for misalignment issues
  c16_t tmp_dft_in[frame_parms->nb_antennas_rx][8192] __attribute__((aligned(32)));

  unsigned int rx_offset = sample_offset + nb_prefix_samples + (frame_parms->ofdm_symbol_size + nb_prefix_samples) * symbol;

  const c16_t *rxdata_symb_ptr[frame_parms->nb_antennas_rx];
  c16_t *rxdataF_symb_ptr[frame_parms->nb_antennas_rx];
  for (unsigned char aa = 0; aa < frame_parms->nb_antennas_rx; aa++) {
    rx_offset %= frame_length_samples * 2;

    if (rx_offset + frame_parms->ofdm_symbol_size > frame_length_samples * 2) {
      // rxdata is 2 frames len
      // we have to wrap on the end
      memcpy(&tmp_dft_in[aa][0], &rxdata[aa][rx_offset], (frame_length_samples * 2 - rx_offset) * sizeof(int32_t));
      memcpy(&tmp_dft_in[aa][frame_length_samples * 2 - rx_offset],
             &rxdata[aa][0],
             (frame_parms->ofdm_symbol_size - (frame_length_samples * 2 - rx_offset)) * sizeof(int32_t));
      rxdata_symb_ptr[aa] = &tmp_dft_in[aa][0];
    } else {
      rxdata_symb_ptr[aa] = &rxdata[aa][rx_offset];
    }
    rxdataF_symb_ptr[aa] = &rxdataF[aa][frame_parms->ofdm_symbol_size * symbol];
  }

  nr_symbol_fep(frame_parms, slot, symbol, link_type, rxdata_symb_ptr, rxdataF_symb_ptr);

  return 0;
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

void apply_nr_rotation_symbol_RX(const NR_DL_FRAME_PARMS *frame_parms,
                                 c16_t *rxdataF,
                                 const c16_t *rot,
                                 int nb_rb,
                                 int slot,
                                 int symbol)
{
  const int symb_offset = (slot % frame_parms->slots_per_subframe) * frame_parms->symbols_per_slot;

    c16_t rot2 = rot[symbol + symb_offset];
    rot2.i = -rot2.i;
    LOG_D(PHY,"slot %d, symb_offset %d rotating by %d.%d\n", slot, symb_offset, rot2.r, rot2.i);
    const c16_t *shift_rot = (c16_t *)frame_parms->timeshift_symbol_rotation;
    unsigned int offset = frame_parms->first_carrier_offset;
    if (nb_rb & 1) {
      offset -= 6;
      nb_rb++;
    }
    rotate_cpx_vector(rxdataF, &rot2, rxdataF, nb_rb * 6, 15);
    rotate_cpx_vector(rxdataF + offset, &rot2, rxdataF + offset, nb_rb * 6, 15);
    mult_cpx_vector(rxdataF, shift_rot, rxdataF, nb_rb * 6, 15);
    mult_cpx_vector(rxdataF + offset, shift_rot + offset, rxdataF + offset, nb_rb * 6, 15);
}
