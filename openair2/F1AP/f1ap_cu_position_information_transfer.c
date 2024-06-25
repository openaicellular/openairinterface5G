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

/*! \file f1ap_cu_position_information_transfer.c
 * \brief F1AP tasks related to position information transfer , CU side
 * \author EURECOM
 * \date 2023
 * \version 0.1
 * \company Eurecom
 * \email: adeel.malik@eurecom.fr
 * \note
 * \warning
 */

#include "f1ap_common.h"
#include "f1ap_encoder.h"
#include "f1ap_itti_messaging.h"
#include "f1ap_cu_position_information_transfer.h"
#include <string.h>

#include "rrc_extern.h"
#include "openair2/RRC/NR/rrc_gNB_NGAP.h"
#include <openair3/ocp-gtpu/gtp_itf.h>
#include "LAYER2/nr_pdcp/nr_pdcp_oai_api.h"

int CU_send_POSITIONING_INFORMATION_REQUEST(instance_t instance,
                                            f1ap_positioning_information_req_t *f1ap_positioning_information_req)
{
  printf("[F1AP] Test1 CU_send_POSITIONING_INFORMATION_REQUEST");
  F1AP_F1AP_PDU_t pdu = {0};
  /* Create */
  /* mandatory */
  /* 0. Message Type */
  pdu.present = F1AP_F1AP_PDU_PR_initiatingMessage;
  asn1cCalloc(pdu.choice.initiatingMessage, tmp);
  tmp->procedureCode = F1AP_ProcedureCode_id_PositioningInformationExchange;
  tmp->criticality = F1AP_Criticality_reject;
  tmp->value.present = F1AP_InitiatingMessage__value_PR_PositioningInformationRequest;
  F1AP_PositioningInformationRequest_t *out = &pdu.choice.initiatingMessage->value.choice.PositioningInformationRequest;

  /* mandatory */
  /* c1. GNB_CU_UE_F1AP_ID */
  asn1cSequenceAdd(out->protocolIEs.list, F1AP_PositioningInformationRequestIEs_t, ie1);
  ie1->id = F1AP_ProtocolIE_ID_id_gNB_CU_UE_F1AP_ID;
  ie1->criticality = F1AP_Criticality_reject;
  ie1->value.present = F1AP_PositioningInformationRequestIEs__value_PR_GNB_CU_UE_F1AP_ID;
  // TODO check CPtype /
  ie1->value.choice.GNB_CU_UE_F1AP_ID =
      0; // f1ap_get_cu_ue_f1ap_id(CPtype, instance, f1ap_positioning_information_req->gNB_CU_ue_id);
         // //f1ap_ue_context_setup_req->gNB_CU_ue_id;
  //  ie1->value.choice.GNB_CU_UE_F1AP_ID = f1ap_get_cu_ue_f1ap_id(CUtype, instance,
  //  f1ap_positioning_information_req->gNB_CU_ue_id); //f1ap_ue_context_setup_req->gNB_CU_ue_id;
  //  ie1->value.choice.GNB_CU_UE_F1AP_ID = f1ap_get_cu_ue_f1ap_id(CUtype, instance, f1ap_ue_context_setup_req->rnti);
  //  //f1ap_ue_context_setup_req->gNB_CU_ue_id;

  /* mandatory  TODO*/
  /* c2. GNB_DU_UE_F1AP_ID */
  //  if (f1ap_ue_context_setup_req->gNB_DU_ue_id) {
  {
    asn1cSequenceAdd(out->protocolIEs.list, F1AP_PositioningInformationRequestIEs_t, ie2);
    ie2->id = F1AP_ProtocolIE_ID_id_gNB_DU_UE_F1AP_ID;
    ie2->criticality = F1AP_Criticality_ignore;
    ie2->value.present = F1AP_PositioningInformationRequestIEs__value_PR_GNB_DU_UE_F1AP_ID;
    ie2->value.choice.GNB_DU_UE_F1AP_ID =
        0; // f1ap_get_du_ue_f1ap_id(CUtype, instance, f1ap_positioning_information_req->gNB_DU_ue_id);
           // //*f1ap_ue_context_setup_req->gNB_DU_ue_id;
  }

  /* OPTIONAL */
  /* Requested_SRS_Transmission_Characteristics // IE 9.3.1.175 (O) */
  {
    asn1cSequenceAdd(out->protocolIEs.list, F1AP_PositioningInformationRequestIEs_t, ie3);
    ie3->id = F1AP_ProtocolIE_ID_id_RequestedSRSTransmissionCharacteristics;
    ie3->criticality = F1AP_Criticality_reject;
    ie3->value.present = F1AP_PositioningInformationRequestIEs__value_PR_RequestedSRSTransmissionCharacteristics;
    // MaskedIMEISV_TO_BIT_STRING(12340000l, &ie15->value.choice.MaskedIMEISV); // size (64)
  }
  /* encode */
  uint8_t *buffer = NULL;
  uint32_t len = 0;

  if (f1ap_encode_pdu(&pdu, &buffer, &len) < 0) {
    LOG_E(F1AP, "Failed to encode F1 POSITION INFORMATION REQUEST\n");
    return -1;
  }

  LOG_D(F1AP, "F1AP PositioningInformationRequest Encoded %u bits\n", len);
  //  f1ap_itti_send_sctp_data_req(true, instance, buffer, len, 0 /* BK: fix me*/);
  f1ap_itti_send_sctp_data_req(instance, buffer, len);
  return 0;
}

int CU_handle_POSITIONING_INFORMATION_RESPONSE(instance_t instance, sctp_assoc_t assoc_id, uint32_t stream, F1AP_F1AP_PDU_t *pdu)
{
  MessageDef *msg_p;
  F1AP_PositioningInformationResponse_t *container;
  F1AP_PositioningInformationResponseIEs_t *ie;
  DevAssert(pdu);
  msg_p = itti_alloc_new_message(TASK_DU_F1, 0, F1AP_POSITIONING_INFORMATION_RESP);
  f1ap_positioning_information_resp_t *f1ap_positioning_information_resp = &F1AP_POSITIONING_INFORMATION_RESP(msg_p);
  container = &pdu->choice.successfulOutcome->value.choice.PositioningInformationResponse;

  // int i;
  /* GNB_CU_UE_F1AP_ID */
  F1AP_FIND_PROTOCOLIE_BY_ID(F1AP_PositioningInformationResponseIEs_t,
                             ie,
                             container,
                             F1AP_ProtocolIE_ID_id_gNB_CU_UE_F1AP_ID,
                             true);
  f1ap_positioning_information_resp->gNB_CU_ue_id = ie->value.choice.GNB_CU_UE_F1AP_ID;
  LOG_D(F1AP, "f1ap_positioning_information_resp->gNB_CU_ue_id is: %d \n", f1ap_positioning_information_resp->gNB_CU_ue_id);

  /* GNB_DU_UE_F1AP_ID */
  F1AP_FIND_PROTOCOLIE_BY_ID(F1AP_PositioningInformationResponseIEs_t,
                             ie,
                             container,
                             F1AP_ProtocolIE_ID_id_gNB_DU_UE_F1AP_ID,
                             true);
  f1ap_positioning_information_resp->gNB_DU_ue_id = ie->value.choice.GNB_DU_UE_F1AP_ID;
  LOG_D(F1AP, "f1ap_positioning_information_resp->gNB_DU_ue_id is: %d \n", f1ap_positioning_information_resp->gNB_DU_ue_id);

  // f1ap_ue_context_setup_resp->rnti =f1ap_get_rnti_by_du_id(CUtype, instance, f1ap_ue_context_setup_resp->gNB_DU_ue_id);

  // SRSConfiguration
  F1AP_FIND_PROTOCOLIE_BY_ID(F1AP_PositioningInformationResponseIEs_t,
                             ie,
                             container,
                             F1AP_ProtocolIE_ID_id_SRSConfiguration,
                             false);

  // SFNInitialisationTime
  F1AP_FIND_PROTOCOLIE_BY_ID(F1AP_PositioningInformationResponseIEs_t,
                             ie,
                             container,
                             F1AP_ProtocolIE_ID_id_SFNInitialisationTime,
                             false);

  // CriticalityDiagnostics
  F1AP_FIND_PROTOCOLIE_BY_ID(F1AP_PositioningInformationResponseIEs_t,
                             ie,
                             container,
                             F1AP_ProtocolIE_ID_id_CriticalityDiagnostics,
                             false);

  //  itti_send_msg_to_task(TASK_NRPPA, instance, msg_p);
  return 0;
}

int CU_handle_POSITIONING_INFORMATION_FAILURE(instance_t instance, sctp_assoc_t assoc_id, uint32_t stream, F1AP_F1AP_PDU_t *pdu)
{
  MessageDef *msg_p;
  F1AP_PositioningInformationFailure_t *container;
  F1AP_PositioningInformationFailureIEs_t *ie;
  DevAssert(pdu);
  msg_p = itti_alloc_new_message(TASK_DU_F1, 0, F1AP_POSITIONING_INFORMATION_FAILURE);
  f1ap_positioning_information_failure_t *f1ap_positioning_information_failure = &F1AP_POSITIONING_INFORMATION_FAILURE(msg_p);
  container = &pdu->choice.unsuccessfulOutcome->value.choice.PositioningInformationFailure;

  // int i;
  /* GNB_CU_UE_F1AP_ID */ /* mandatory */
  F1AP_FIND_PROTOCOLIE_BY_ID(F1AP_PositioningInformationFailureIEs_t, ie, container, F1AP_ProtocolIE_ID_id_gNB_CU_UE_F1AP_ID, true);
  f1ap_positioning_information_failure->gNB_CU_ue_id = ie->value.choice.GNB_CU_UE_F1AP_ID;
  LOG_D(F1AP, "f1ap_positioning_information_failure->gNB_CU_ue_id is: %d \n", f1ap_positioning_information_failure->gNB_CU_ue_id);

  /* GNB_DU_UE_F1AP_ID */ /* mandatory */
  F1AP_FIND_PROTOCOLIE_BY_ID(F1AP_PositioningInformationFailureIEs_t, ie, container, F1AP_ProtocolIE_ID_id_gNB_DU_UE_F1AP_ID, true);
  f1ap_positioning_information_failure->gNB_DU_ue_id = ie->value.choice.GNB_DU_UE_F1AP_ID;
  LOG_D(F1AP, "f1ap_positioning_information_failure->gNB_DU_ue_id is: %d \n", f1ap_positioning_information_failure->gNB_DU_ue_id);

  // f1ap_ue_context_setup_resp->rnti =f1ap_get_rnti_by_du_id(CUtype, instance, f1ap_ue_context_setup_resp->gNB_DU_ue_id);

  // Cause (M) /* mandatory */
  F1AP_FIND_PROTOCOLIE_BY_ID(F1AP_PositioningInformationFailureIEs_t, ie, container, F1AP_ProtocolIE_ID_id_SRSConfiguration, false);

  // CriticalityDiagnostics /* optional */
  F1AP_FIND_PROTOCOLIE_BY_ID(F1AP_PositioningInformationFailureIEs_t,
                             ie,
                             container,
                             F1AP_ProtocolIE_ID_id_CriticalityDiagnostics,
                             false);

  //  itti_send_msg_to_task(TASK_NRPPA, instance, msg_p);
  return 0;
}
