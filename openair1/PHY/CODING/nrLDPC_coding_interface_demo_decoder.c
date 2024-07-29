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

/*! \file PHY/CODING/nrLDPC_coding_interface_demo_decoder.c
* \brief Top-level routines for decoding  LDPC transport channels
* \author Ahmed Hussein
* \date 2019
* \version 0.1
* \company Fraunhofer IIS
* \email: ahmed.hussein@iis.fraunhofer.de
* \note
* \warning
*/


// [from gNB coding]
#include "PHY/defs_gNB.h"
#include "PHY/CODING/coding_extern.h"
#include "PHY/CODING/coding_defs.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "PHY/CODING/nrLDPC_coding_interface.h"
#include "PHY/CODING/nrLDPC_extern.h"
#include "PHY/NR_TRANSPORT/nr_transport_common_proto.h"
#include "PHY/NR_TRANSPORT/nr_transport_proto.h"
#include "PHY/NR_TRANSPORT/nr_ulsch.h"
#include "PHY/NR_TRANSPORT/nr_dlsch.h"
#include "SCHED_NR/sched_nr.h"
#include "SCHED_NR/fapi_nr_l1.h"
#include "defs.h"
#include "common/utils/LOG/vcd_signal_dumper.h"
#include "common/utils/LOG/log.h"
#include <syscall.h>
//#define gNB_DEBUG_TRACE

#define OAI_LDPC_DECODER_MAX_NUM_LLR 27000 //26112 // NR_LDPC_NCOL_BG1*NR_LDPC_ZMAX = 68*384
//#define DEBUG_CRC
#ifdef DEBUG_CRC
#define PRINT_CRC_CHECK(a) a
#else
#define PRINT_CRC_CHECK(a)
#endif

#include "nfapi/open-nFAPI/nfapi/public_inc/nfapi_interface.h"
#include "nfapi/open-nFAPI/nfapi/public_inc/nfapi_nr_interface.h"

#define DEMO_LDPCLIB_SUFFIX ""

/**
 * \typedef nrLDPC_decoding_parameters_t
 * \struct nrLDPC_decoding_parameters_s
 * \brief decoding parameter of transport blocks
 * \var decoderParams decoder parameters
 * \var Qm modulation order
 * \var Kc size of base graph input
 * \var rv_index
 * \var max_number_iterations maximum number of LDPC iterations
 * \var abort_decode pointer to decode abort flag
 * \var tbslbrm transport block size LBRM in bytes
 * \var A Transport block size (This is A from 38.212 V15.4.0 section 5.1)
 * \var K
 * \var Z lifting size
 * \var F filler bits size
 * \var r segment index in TB
 * \var E input llr segment size
 * \var C number of segments 
 * \var llr input llr segment array
 * \var d Pointers to code blocks before LDPC decoding (38.212 V15.4.0 section 5.3.2)
 * \var d_to_be_cleared
 * pointer to the flag used to clear d properly
 * when true, clear d after rate dematching
 * \var c Pointers to code blocks after LDPC decoding (38.212 V15.4.0 section 5.2.2)
 * \var decodeSuccess pointer to the flag indicating that the decoding of the segment was successful
 */
typedef struct nrLDPC_decoding_parameters_s{

  t_nrLDPC_dec_params decoderParms;

  uint8_t Qm;

  uint8_t Kc;
  uint8_t rv_index;
  decode_abort_t *abort_decode;

  uint32_t tbslbrm;
  uint32_t A;
  uint32_t K;
  uint32_t Z;
  uint32_t F;

  uint32_t C;

  int E;
  short *llr;
  int16_t *d;
  bool *d_to_be_cleared;
  uint8_t *c;
  bool *decodeSuccess;
} nrLDPC_decoding_parameters_t;

// Global var to limit the rework of the dirty legacy code
ldpc_interface_t ldpc_interface_demo;

static void nr_process_decode_segment(void *arg)
{
  nrLDPC_decoding_parameters_t *rdata = (nrLDPC_decoding_parameters_t *)arg;
  t_nrLDPC_dec_params *p_decoderParms = &rdata->decoderParms;
  const int Kr = rdata->K;
  const int Kr_bytes = Kr >> 3;
  const int K_bits_F = Kr - rdata->F;
  const int A = rdata->A;
  const int E = rdata->E;
  const int Qm = rdata->Qm;
  const int rv_index = rdata->rv_index;
  const uint8_t kc = rdata->Kc;
  short *ulsch_llr = rdata->llr;
  int8_t llrProcBuf[OAI_LDPC_DECODER_MAX_NUM_LLR] __attribute__((aligned(32)));

  t_nrLDPC_time_stats procTime = {0};
  t_nrLDPC_time_stats *p_procTime = &procTime;

  ////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////// nr_deinterleaving_ldpc ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////// ulsch_llr =====> ulsch_harq->e //////////////////////////////

  /// code blocks after bit selection in rate matching for LDPC code (38.212 V15.4.0 section 5.4.2.1)
  int16_t harq_e[E];

  nr_deinterleaving_ldpc(E, Qm, harq_e, ulsch_llr);

  //////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// nr_rate_matching_ldpc_rx ////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////

  ///////////////////////// ulsch_harq->e =====> ulsch_harq->d /////////////////////////


  if (nr_rate_matching_ldpc_rx(rdata->tbslbrm,
                               p_decoderParms->BG,
                               p_decoderParms->Z,
                               rdata->d,
                               harq_e,
                               rdata->C,
                               rv_index,
                               *rdata->d_to_be_cleared,
                               E,
                               rdata->F,
                               Kr - rdata->F - 2 * (p_decoderParms->Z))
      == -1) {

    LOG_E(PHY, "nrLDPC_coding_interface_demo_decoder.c: Problem in rate_matching\n");
    return;
  }

  *rdata->d_to_be_cleared = false;

  memset(rdata->c, 0, Kr_bytes);
  p_decoderParms->crc_type = crcType(rdata->C, A);
  p_decoderParms->E = lenWithCrc(rdata->C, A);
  // start_meas(&phy_vars_gNB->ulsch_ldpc_decoding_stats);

  // set first 2*Z_c bits to zeros

  int16_t z[68 * 384 + 16] __attribute__((aligned(16)));

  memset(z, 0, 2 * rdata->Z * sizeof(*z));
  // set Filler bits
  memset(z + K_bits_F, 127, rdata->F * sizeof(*z));
  // Move coded bits before filler bits
  memcpy(z + 2 * rdata->Z, rdata->d, (K_bits_F - 2 * rdata->Z) * sizeof(*z));
  // skip filler bits
  memcpy(z + Kr, rdata->d + (Kr - 2 * rdata->Z), (kc * rdata->Z - Kr) * sizeof(*z));
  // Saturate coded bits before decoding into 8 bits values
  simde__m128i *pv = (simde__m128i *)&z;
  int8_t l[68 * 384 + 16] __attribute__((aligned(16)));
  simde__m128i *pl = (simde__m128i *)&l;
  for (int i = 0, j = 0; j < ((kc * rdata->Z) >> 4) + 1; i += 2, j++) {
    pl[j] = simde_mm_packs_epi16(pv[i], pv[i + 1]);
  }
  //////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////// nrLDPC_decoder /////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////// pl =====> llrProcBuf //////////////////////////////////
  int decodeIterations =
      ldpc_interface_demo.LDPCdecoder(p_decoderParms, 0, 0, 0, l, llrProcBuf, p_procTime, rdata->abort_decode);

  if (decodeIterations <= p_decoderParms->numMaxIter) {
    memcpy(rdata->c,llrProcBuf,  Kr>>3);
    *rdata->decodeSuccess = true;
  } else {
    *rdata->decodeSuccess = false;
  }
}

int nrLDPC_prepare_TB_decoding(nrLDPC_slot_decoding_parameters_t *nrLDPC_slot_decoding_parameters, int pusch_id)
{

  nrLDPC_TB_decoding_parameters_t *nrLDPC_TB_decoding_parameters = &nrLDPC_slot_decoding_parameters->TBs[pusch_id];

  t_nrLDPC_dec_params decParams = {.check_crc = check_crc};
  decParams.BG = nrLDPC_TB_decoding_parameters->BG;
  decParams.Z = nrLDPC_TB_decoding_parameters->Z;
  decParams.numMaxIter = nrLDPC_TB_decoding_parameters->max_ldpc_iterations;
  decParams.outMode = 0;

  for (int r = 0; r < nrLDPC_TB_decoding_parameters->C; r++) {
    union ldpcReqUnion id = {.s = {nrLDPC_TB_decoding_parameters->rnti, nrLDPC_slot_decoding_parameters->frame, nrLDPC_slot_decoding_parameters->slot, 0, 0}};
    notifiedFIFO_elt_t *req = newNotifiedFIFO_elt(sizeof(nrLDPC_decoding_parameters_t), id.p, nrLDPC_slot_decoding_parameters->respDecode, &nr_process_decode_segment);
    nrLDPC_decoding_parameters_t *rdata = (nrLDPC_decoding_parameters_t *)NotifiedFifoData(req);
    decParams.R = nrLDPC_TB_decoding_parameters->segments[r].R;
    decParams.setCombIn = !nrLDPC_TB_decoding_parameters->segments[r].d_to_be_cleared;
    rdata->decoderParms = decParams;
    rdata->llr = nrLDPC_TB_decoding_parameters->segments[r].llr;
    rdata->Kc = decParams.BG == 2 ? 52 : 68;
    rdata->C = nrLDPC_TB_decoding_parameters->C;
    rdata->E = nrLDPC_TB_decoding_parameters->segments[r].E;
    rdata->A = nrLDPC_TB_decoding_parameters->A;
    rdata->Qm = nrLDPC_TB_decoding_parameters->Qm;
    rdata->K = nrLDPC_TB_decoding_parameters->K;
    rdata->Z = nrLDPC_TB_decoding_parameters->Z;
    rdata->F = nrLDPC_TB_decoding_parameters->F;
    rdata->rv_index = nrLDPC_TB_decoding_parameters->rv_index;
    rdata->tbslbrm = nrLDPC_TB_decoding_parameters->tbslbrm;
    rdata->abort_decode = nrLDPC_TB_decoding_parameters->abort_decode;
    rdata->d = nrLDPC_TB_decoding_parameters->segments[r].d;
    rdata->d_to_be_cleared = nrLDPC_TB_decoding_parameters->segments[r].d_to_be_cleared;
    rdata->c = nrLDPC_TB_decoding_parameters->segments[r].c;
    rdata->decodeSuccess = &nrLDPC_TB_decoding_parameters->segments[r].decodeSuccess;
    pushTpool(nrLDPC_slot_decoding_parameters->threadPool, req);
    LOG_D(PHY, "Added a block to decode, in pipe: %d\n", r);
  }
  return nrLDPC_TB_decoding_parameters->C;
}

int32_t nrLDPC_coding_init(void){

  load_LDPClib(DEMO_LDPCLIB_SUFFIX, &ldpc_interface_demo);

  return 0;

}

int32_t nrLDPC_coding_shutdown(void){

  free_LDPClib(&ldpc_interface_demo);

  return 0;

}

int32_t nrLDPC_coding_decoder(nrLDPC_slot_decoding_parameters_t *nrLDPC_slot_decoding_parameters){

  int nbDecode = 0;
  for (int pusch_id = 0; pusch_id < nrLDPC_slot_decoding_parameters->nb_TBs; pusch_id++) {
    nbDecode += nrLDPC_prepare_TB_decoding(nrLDPC_slot_decoding_parameters, pusch_id);
  }

  return nbDecode;

}

