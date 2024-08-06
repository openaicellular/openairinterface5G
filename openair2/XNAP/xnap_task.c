/* Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
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

/*! \file XNAP/xnap_gNB_task.c
 * \brief XNAP tasks and functions definitions
 * \author Sreeshma Shiv
 * \date Aug 2023
 * \version 1.0
 * \email: sreeshmau@iisc.ac.in
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "intertask_interface.h"
#include <openair3/ocp-gtpu/gtp_itf.h>
#include "xnap_task.h"
#include "gnb_paramdef.h"
#include "common/config/config_userapi.h"

void *xnap_task(void *arg)
{
  LOG_D(XNAP, "Starting XNAP layer\n");
  itti_mark_task_ready(TASK_XNAP);

  while (1) {
    MessageDef *received_msg = NULL;
    itti_receive_msg(TASK_XNAP, &received_msg);
    LOG_D(XNAP, "Received message %d:%s\n", ITTI_MSG_ID(received_msg), ITTI_MSG_NAME(received_msg));
    switch (ITTI_MSG_ID(received_msg)) {
      case TERMINATE_MESSAGE:
        LOG_W(XNAP, "Exiting XNAP thread\n");
        itti_exit_task();
        break;

      default:
        LOG_E(XNAP, "Received unhandled message: %d:%s\n", ITTI_MSG_ID(received_msg), ITTI_MSG_NAME(received_msg));
        break;
    }

    int result = itti_free(ITTI_MSG_ORIGIN_ID(received_msg), received_msg);
    AssertFatal(result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);
    received_msg = NULL;
  }
  return NULL;
}


int is_xnap_enabled(void)
{
  int xn_enabled = 0;
  char xn_path[MAX_OPTNAME_SIZE*2 + 8];
  sprintf(xn_path, "%s.[%i]", GNB_CONFIG_STRING_GNB_LIST, 0);
  paramdef_t Xn_Params[] = XnPARAMS_DESC;
  config_get(config_get_if(), Xn_Params, sizeofArray(Xn_Params), xn_path);
  if (*(Xn_Params[XNAP_ENABLE_IDX].i64ptr))
  {
    xn_enabled = 1;
    LOG_I(XNAP, "XNAP enabled\n");
  }
  return xn_enabled;
}
