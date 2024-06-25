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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "intertask_interface.h"
#include "assertions.h"
#include "conversions.h"
#include "xnap_defs.h"

static xnap_gNB_instance_t *xn_inst[NUMBER_OF_gNB_MAX] = {0};

static int xnap_gNB_compare_assoc_id(const xnap_gNB_data_t *p1, const xnap_gNB_data_t *p2)
{
  if (p1->assoc_id > p2->assoc_id)
    return 1;
  if (p1->assoc_id == p2->assoc_id)
    return 0;
  return -1; /* p1->assoc_id < p1->assoc_id */
}

RB_GENERATE(xnap_gnb_tree, xnap_gNB_data_t, entry, xnap_gNB_compare_assoc_id);

static pthread_mutex_t xn_inst_mtx = PTHREAD_MUTEX_INITIALIZER;
void createXninst(instance_t instanceP, xnap_setup_req_t *req, xnap_net_config_t *nc)
{
  DevAssert(instanceP == 0);
  pthread_mutex_lock(&xn_inst_mtx);
  AssertFatal(xn_inst[0] == NULL, "Attempted to initialize multiple Xn instances\n");
  xn_inst[0] = calloc(1, sizeof(xnap_gNB_instance_t));
  AssertFatal(xn_inst[0] != NULL, "out of memory\n");
  if (req)
    xn_inst[0]->setup_req = *req;
  if (nc)
    xn_inst[0]->net_config = *nc;
  pthread_mutex_unlock(&xn_inst_mtx);
}
