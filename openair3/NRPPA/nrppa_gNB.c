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

/*! \file nrppa_gNB.c
 * \brief NRPPA gNB task
 * \author  Adeel Maik
 * \date 2023
 * \email: adeel.malik@eurecom.fr
 * \version 1.0
 * @ingroup _nrppa
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <crypt.h>

#include "tree.h"
#include "queue.h"

#include "intertask_interface.h"

#include "assertions.h"
#include "conversions.h"

#include "nrppa_gNB.h"
#include "nrppa_common.h"
#include "nrppa_gNB_handlers.h"
#include "nrppa_gNB_position_information_transfer_procedures.h"
#include "nrppa_gNB_TRP_information_transfer_procedures.h"
#include "nrppa_gNB_measurement_information_transfer_procedures.h"

void nrppa_gNB_init(void)
{
  NRPPA_DEBUG("Starting NRPPA layer\n");
  itti_mark_task_ready(TASK_NRPPA);
}

void *nrppa_gNB_process_itti_msg(void *notUsed)
{
  MessageDef *received_msg = NULL;
  int result;
  itti_receive_msg(TASK_NRPPA, &received_msg);
  if (received_msg) {
    instance_t instance = ITTI_MSG_DESTINATION_INSTANCE(received_msg);
    LOG_I(NRPPA, "Received message %s\n", ITTI_MSG_NAME(received_msg));
    switch (ITTI_MSG_ID(received_msg)) {
      case TERMINATE_MESSAGE:
        NRPPA_WARN(" *** Exiting NRPPA thread\n"); // to be implemented // ad**l
        itti_exit_task();
        break;
      case NGAP_DOWNLINKUEASSOCIATEDNRPPA:
        nrppa_handle_DownlinkUEAssociatedNRPPaTransport(instance, &NGAP_DOWNLINKUEASSOCIATEDNRPPA(received_msg));
        break;
      case NGAP_DOWNLINKNONUEASSOCIATEDNRPPA:
        nrppa_handle_DownlinkNonUEAssociatedNRPPaTransport(instance, &NGAP_DOWNLINKNONUEASSOCIATEDNRPPA(received_msg));
        break;
      case F1AP_POSITIONING_INFORMATION_RESP:
        nrppa_gNB_PositioningInformationResponse(instance, received_msg);
        break;
      case F1AP_POSITIONING_INFORMATION_FAILURE:
        nrppa_gNB_PositioningInformationFailure(instance, received_msg);
        break;
      case F1AP_POSITIONING_INFORMATION_UPDATE:
        nrppa_gNB_PositioningInformationUpdate(instance, received_msg);
        break;
      case F1AP_POSITIONING_ACTIVATION_RESP:
        nrppa_gNB_PositioningActivationResponse(instance, received_msg);
        break;
      case F1AP_POSITIONING_ACTIVATION_FAILURE:
        nrppa_gNB_PositioningActivationFailure(instance, received_msg);
        break;
      case F1AP_MEASUREMENT_RESP:
        nrppa_gNB_MeasurementResponse(instance, received_msg);
        break;
      case F1AP_MEASUREMENT_FAILURE:
        nrppa_gNB_MeasurementFailure(instance, received_msg);
        break;
      case F1AP_MEASUREMENT_REPORT:
        nrppa_gNB_MeasurementReport(instance, received_msg);
        break;
      case F1AP_MEASUREMENT_FAILURE_IND:
        nrppa_gNB_MeasurementFailureIndication(instance, received_msg);
        break;
      case F1AP_TRP_INFORMATION_RESP:
        nrppa_gNB_TRPInformationResponse(instance, received_msg);
        break;
      case F1AP_TRP_INFORMATION_FAILURE:
        nrppa_gNB_TRPInformationFailure(instance, received_msg);
        break;
      default:
        NRPPA_ERROR("Received unhandled message: %d:%s\n", ITTI_MSG_ID(received_msg), ITTI_MSG_NAME(received_msg));
        break;
    }
    result = itti_free(ITTI_MSG_ORIGIN_ID(received_msg), received_msg);
    AssertFatal(result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);
  }
  return NULL;
}

void *nrppa_gNB_task(void *arg)
{
  nrppa_gNB_init();

  while (1) {
    (void)nrppa_gNB_process_itti_msg(NULL);
  }

  return NULL;
}
