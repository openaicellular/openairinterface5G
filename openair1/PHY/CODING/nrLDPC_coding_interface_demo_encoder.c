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

/*! \file PHY/CODING/nrLDPC_coding_interface_demo_encoder.c
* \brief Top-level routines for implementing LDPC encoding of transport channels
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

//#define DEBUG_LDPC_ENCODING
//#define DEBUG_LDPC_ENCODING_FREE 1

extern ldpc_interface_t ldpc_interface_demo;

typedef struct ldpc8blocks_args_s {
  nrLDPC_TB_encoding_parameters_t *nrLDPC_TB_encoding_parameters;
  encoder_implemparams_t impp;
} ldpc8blocks_args_t;

static void ldpc8blocks_demo(void *p)
{
  ldpc8blocks_args_t *args = (ldpc8blocks_args_t *)p;
  nrLDPC_TB_encoding_parameters_t *nrLDPC_TB_encoding_parameters = args->nrLDPC_TB_encoding_parameters;
  encoder_implemparams_t *impp = &args->impp;

  uint8_t mod_order = nrLDPC_TB_encoding_parameters->Qm;
  uint16_t nb_rb = nrLDPC_TB_encoding_parameters->nb_rb;
  uint32_t A = nrLDPC_TB_encoding_parameters->A;

  unsigned int G = nrLDPC_TB_encoding_parameters->G;
  LOG_D(PHY,"dlsch coding A %d  Kr %d G %d (nb_rb %d, mod_order %d)\n",
        A,impp->K,G, nb_rb,(int)mod_order);
  // nrLDPC_encoder output is in "d"
  // let's make this interface happy!
  uint8_t tmp[8][68 * 384]__attribute__((aligned(32)));
  uint8_t *d[impp->n_segments];
  for (int rr=impp->macro_num*8, i=0; rr < impp->n_segments && rr < (impp->macro_num+1)*8; rr++,i++ )
    d[rr] = tmp[i];
  uint8_t *c[nrLDPC_TB_encoding_parameters->C];
  for (int r = 0; r < nrLDPC_TB_encoding_parameters->C; r++)
    c[r]=nrLDPC_TB_encoding_parameters->segments[r].c;
  ldpc_interface_demo.LDPCencoder(c, d, impp);
  // Compute where to place in output buffer that is concatenation of all segments
  uint32_t r_offset=0;
  for (int i=0; i < impp->macro_num*8; i++ )
     r_offset+=nrLDPC_TB_encoding_parameters->segments[i].E;
  for (int rr=impp->macro_num*8; rr < impp->n_segments && rr < (impp->macro_num+1)*8; rr++ ) {
    if (impp->F>0) {
      // writing into positions d[r][k-2Zc] as in clause 5.3.2 step 2) in 38.212
      memset(&d[rr][impp->K - impp->F - 2 * impp->Zc], NR_NULL, impp->F);
    }

#ifdef DEBUG_LDPC_ENCODING
    LOG_D(PHY,"rvidx in encoding = %d\n", rel15->rvIndex[0]);
#endif
    uint32_t E = nrLDPC_TB_encoding_parameters->segments[rr].E;
    LOG_D(NR_PHY,
          "Rate Matching, Code segment %d/%d (coded bits (G) %u, E %d, Filler bits %d, Filler offset %d mod_order %d, nb_rb "
          "%d,nrOfLayer %d)...\n",
          rr,
          impp->n_segments,
          G,
          E,
          impp->F,
          impp->K - impp->F - 2 * impp->Zc,
          mod_order,
          nb_rb,
          nrLDPC_TB_encoding_parameters->nb_layers);

    uint32_t Tbslbrm = nrLDPC_TB_encoding_parameters->tbslbrm;

    uint8_t e[E];
    bzero (e, E);
    nr_rate_matching_ldpc(Tbslbrm,
                          impp->BG,
                          impp->Zc,
                          d[rr],
                          e,
                          impp->n_segments,
                          impp->F,
                          impp->K - impp->F - 2 * impp->Zc,
                          nrLDPC_TB_encoding_parameters->rv_index,
                          E);
    if (impp->K - impp->F - 2 * impp->Zc > E) {
      LOG_E(PHY,
            "dlsch coding A %d  Kr %d G %d (nb_rb %d, mod_order %d)\n",
            A,
            impp->K,
            G,
            nb_rb,
            (int)mod_order);

      LOG_E(NR_PHY,
            "Rate Matching, Code segment %d/%d (coded bits (G) %u, E %d, Kr %d, Filler bits %d, Filler offset %d mod_order %d, "
            "nb_rb %d)...\n",
            rr,
            impp->n_segments,
            G,
            E,
            impp->K,
            impp->F,
            impp->K - impp->F - 2 * impp->Zc,
            mod_order,
            nb_rb);
    }
#ifdef DEBUG_LDPC_ENCODING

    for (int i =0; i<16; i++)
      printf("output ratematching e[%d]= %d r_offset %u\n", i,e[i], r_offset);

#endif
    nr_interleaving_ldpc(E,
                         mod_order,
                         e,
                         impp->output+r_offset);
#ifdef DEBUG_LDPC_ENCODING

    for (int i =0; i<16; i++)
      printf("output interleaving f[%d]= %d r_offset %u\n", i,impp->output[i+r_offset], r_offset);

    if (r==impp->n_segments-1)
      write_output("enc_output.m","enc",impp->output,G,1,4);

#endif
    r_offset += E;
  }
}

static int nrLDPC_prepare_TB_encoding(nrLDPC_slot_encoding_parameters_t *nrLDPC_slot_encoding_parameters, int dlsch_id)
{
  nrLDPC_TB_encoding_parameters_t *nrLDPC_TB_encoding_parameters = &nrLDPC_slot_encoding_parameters->TBs[dlsch_id];

  encoder_implemparams_t impp;

  impp.n_segments = nrLDPC_TB_encoding_parameters->C;
  impp.tinput = nrLDPC_slot_encoding_parameters->tinput;
  impp.tprep = nrLDPC_slot_encoding_parameters->tprep;
  impp.tparity = nrLDPC_slot_encoding_parameters->tparity;
  impp.toutput = nrLDPC_slot_encoding_parameters->toutput;
  impp.Kb = nrLDPC_TB_encoding_parameters->Kb;
  impp.Zc = nrLDPC_TB_encoding_parameters->Z;
  NR_DL_gNB_HARQ_t harq;
  impp.harq = &harq;
  impp.BG = nrLDPC_TB_encoding_parameters->BG;
  impp.output = nrLDPC_TB_encoding_parameters->segments->output;
  impp.K = nrLDPC_TB_encoding_parameters->K;
  impp.F = nrLDPC_TB_encoding_parameters->F;
  impp.Qm = nrLDPC_TB_encoding_parameters->Qm;
  impp.Tbslbrm = nrLDPC_TB_encoding_parameters->tbslbrm;
  impp.G = nrLDPC_TB_encoding_parameters->G;
  for (int r = 0; r < nrLDPC_TB_encoding_parameters->C; r++) {
    impp.perCB[r].E_cb = nrLDPC_TB_encoding_parameters->segments[r].E;
  }
  impp.rv = nrLDPC_TB_encoding_parameters->rv_index;

  int nbJobs = 0;
  for (int j = 0; j < (impp.n_segments / 8 + ((impp.n_segments & 7) == 0 ? 0 : 1)); j++) {
    notifiedFIFO_elt_t *req = newNotifiedFIFO_elt(sizeof(ldpc8blocks_args_t), j, nrLDPC_slot_encoding_parameters->respEncode, ldpc8blocks_demo);
    ldpc8blocks_args_t *perJobImpp = (ldpc8blocks_args_t *)NotifiedFifoData(req);
    impp.macro_num = j;
    perJobImpp->impp = impp;
    perJobImpp->nrLDPC_TB_encoding_parameters = nrLDPC_TB_encoding_parameters;
    pushTpool(nrLDPC_slot_encoding_parameters->threadPool, req);
    nbJobs++;
  }
  return nbJobs;
}

int nrLDPC_coding_encoder(nrLDPC_slot_encoding_parameters_t *nrLDPC_slot_encoding_parameters)
{

  int nbEncode = 0;
  for (int dlsch_id = 0; dlsch_id < nrLDPC_slot_encoding_parameters->nb_TBs; dlsch_id++) {
    nbEncode += nrLDPC_prepare_TB_encoding(nrLDPC_slot_encoding_parameters, dlsch_id);
  }

  return nbEncode;

}
