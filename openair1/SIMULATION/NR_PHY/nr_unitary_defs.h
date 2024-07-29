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

/*!\file openair1/SIMULATION/NR_PHY/nr_unitary_defs.h
 * \brief
 * \author Turker Yilmaz
 * \date 2019
 * \version 0.1
 * \company EURECOM
 * \email turker.yilmaz@eurecom.fr
 * \note
 * \warning
*/

#ifndef __NR_UNITARY_DEFS__H__
#define __NR_UNITARY_DEFS__H__

#include "NR_ServingCellConfigCommon.h"
#include "NR_ServingCellConfig.h"

int oai_exit=0;

void exit_function(const char* file, const char* function, const int line, const char *s, const int assert) {
  const char * msg= s==NULL ? "no comment": s;
  printf("Exiting at: %s:%d %s(), %s\n", file, line, function, msg);
  exit(-1);
}

signed char quantize(double D, double x, unsigned char B) {
  double qxd;
  short maxlev;
  qxd = floor(x / D);
  maxlev = 1 << (B - 1); //(char)(pow(2,B-1));

  if (qxd <= -maxlev)
    qxd = -maxlev;
  else if (qxd >= maxlev)
    qxd = maxlev - 1;

  return ((char) qxd);
}

void slot_fep_unitary_helper(const UE_nr_rxtx_proc_t *proc,
                             const NR_DL_FRAME_PARMS *fp,
                             int symbol,
                             c16_t **common_vars_rxdata,
                             c16_t rxdata[fp->nb_antennas_rx][ALNARS_32_8(fp->ofdm_symbol_size + fp->nb_prefix_samples0)],
                             c16_t rxdataF[fp->nb_antennas_rx][ALNARS_32_8(fp->ofdm_symbol_size)])
{
  const int abs_symbol = proc->nr_slot_rx * fp->symbols_per_slot + symbol;
  int offsetSamples = fp->get_samples_slot_timestamp(proc->nr_slot_rx, fp, 0);
  for (int s = proc->nr_slot_rx * fp->symbols_per_slot; s <= abs_symbol; s++) {
    const int prefixSamples = (s % (0x7 << fp->numerology_index)) ? fp->nb_prefix_samples : fp->nb_prefix_samples0;
    offsetSamples += prefixSamples;
  }
  offsetSamples += symbol * fp->ofdm_symbol_size;
  for (int aa = 0; aa < fp->nb_antennas_rx; aa++) {
    memcpy(rxdata[aa], &common_vars_rxdata[aa][offsetSamples], sizeof(c16_t) * fp->ofdm_symbol_size);
  }
  nr_symbol_fep(fp, proc->nr_slot_rx, symbol, link_type_dl, rxdata, rxdataF);
}

int oai_nfapi_rach_ind(nfapi_rach_indication_t *rach_ind) {return(0);}
//NR_IF_Module_t *NR_IF_Module_init(int Mod_id){return(NULL);}
int oai_nfapi_ul_config_req(nfapi_ul_config_request_t *ul_config_req) { return(0); }

void fill_scc_sim(NR_ServingCellConfigCommon_t *scc,uint64_t *ssb_bitmap,int N_RB_DL,int N_RB_UL,int mu_dl,int mu_ul);
void fix_scc(NR_ServingCellConfigCommon_t *scc,uint64_t ssbmap);
void prepare_scc(NR_ServingCellConfigCommon_t *scc);
void prepare_scd(NR_ServingCellConfig_t *scd);
uint32_t ngap_generate_gNB_id(void) {return 0;}

#endif
