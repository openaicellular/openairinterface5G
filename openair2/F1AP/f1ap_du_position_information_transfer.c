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

/*! \file f1ap_du_position_information_transfer.c
 * \brief F1AP tasks related to position information transfer , DU side
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
#include "f1ap_du_position_information_transfer.h"
//#include "openair2/LAYER2/NR_MAC_gNB/mac_rrc_dl_handler.h"

#include "openair2/LAYER2/NR_MAC_gNB/nr_mac_gNB.h"
#include <openair3/ocp-gtpu/gtp_itf.h>

int DU_handle_POSITIONING_INFORMATION_REQUEST(instance_t instance, uint32_t assoc_id, uint32_t stream, F1AP_F1AP_PDU_t *pdu)
{
  // TODO not complete yet
  MessageDef *msg_p;
  F1AP_PositioningInformationRequest_t *container;
  F1AP_PositioningInformationRequestIEs_t *ie;
  DevAssert(pdu);
  msg_p = itti_alloc_new_message(TASK_CU_F1, 0, F1AP_POSITIONING_INFORMATION_REQ);
  f1ap_positioning_information_req_t *f1ap_positioning_information_req = &F1AP_POSITIONING_INFORMATION_REQ(msg_p);
  container = &pdu->choice.initiatingMessage->value.choice.PositioningInformationRequest;

  // int i;
  /* GNB_CU_UE_F1AP_ID */
  F1AP_FIND_PROTOCOLIE_BY_ID(F1AP_PositioningInformationRequestIEs_t, ie, container, F1AP_ProtocolIE_ID_id_gNB_CU_UE_F1AP_ID, true);
  f1ap_positioning_information_req->gNB_CU_ue_id = ie->value.choice.GNB_CU_UE_F1AP_ID;
  LOG_D(F1AP, "f1ap_positioning_information_req->gNB_CU_ue_id is: %d \n", f1ap_positioning_information_req->gNB_CU_ue_id);

  /* GNB_DU_UE_F1AP_ID */
  F1AP_FIND_PROTOCOLIE_BY_ID(F1AP_PositioningInformationRequestIEs_t, ie, container, F1AP_ProtocolIE_ID_id_gNB_DU_UE_F1AP_ID, true);
  f1ap_positioning_information_req->gNB_DU_ue_id = ie->value.choice.GNB_DU_UE_F1AP_ID;
  LOG_D(F1AP, "f1ap_positioning_information_req->gNB_DU_ue_id is: %d \n", f1ap_positioning_information_req->gNB_DU_ue_id);

  // f1ap_ue_context_setup_resp->rnti =f1ap_get_rnti_by_du_id(CUtype, instance, f1ap_ue_context_setup_resp->gNB_DU_ue_id);

  /* RequestedSRSTransmissionCharacteristics (O)*/
  F1AP_FIND_PROTOCOLIE_BY_ID(F1AP_PositioningInformationRequestIEs_t,
                             ie,
                             container,
                             F1AP_ProtocolIE_ID_id_RequestedSRSTransmissionCharacteristics,
                             true);

  // Preparing Response for the Received PositioningInformationRequest

  bool response_type_indicator =
      1; // 1 = send Position Information transfer Response, 0 = send Position Information transfer Failure

  if (response_type_indicator) {
    printf("[F1AP] PIR Test Adeel:  Processing Received PositioningInformationRequest \n");
    LOG_D(F1AP, "DU Preparing PositioningInformationResponse message \n");
    // nrppa_gNB_PositioningInformationResponse(nrppa_transaction_id, nrppa_msg_info);
    // DU_send_POSITIONING_INFORMATION_RESPONSE()
    return 0;
  } else {
    printf("[F1AP] PIF Test Adeel:  Processing Received PositioningInformationRequest \n");
    LOG_D(F1AP, "DU Preparing PositioningInformationFailure  message \n");
    // nrppa_pdu_length= nrppa_gNB_PositioningInformationFailure(nrppa_transaction_id, &tx_nrppa_pdu);
    // DU_send_POSITIONING_INFORMATION_FAILURE()
    return 0;
  }

  //  itti_send_msg_to_task(TASK_NRPPA, instance, msg_p);
  return 0;
}

int DU_send_POSITIONING_INFORMATION_RESPONSE(instance_t instance,
                                             f1ap_positioning_information_resp_t *f1ap_positioning_information_resp)
{
  F1AP_F1AP_PDU_t pdu = {0};
  F1AP_PositioningInformationResponse_t *out;
  uint8_t *buffer = NULL;
  uint32_t len = 0;

  /* Create */
  /* 0. Message Type */
  pdu.present = F1AP_F1AP_PDU_PR_successfulOutcome;
  asn1cCalloc(pdu.choice.successfulOutcome, tmp);
  tmp->procedureCode = F1AP_ProcedureCode_id_PositioningInformationExchange;
  tmp->criticality = F1AP_Criticality_reject;
  tmp->value.present = F1AP_SuccessfulOutcome__value_PR_PositioningInformationResponse;
  out = &tmp->value.choice.PositioningInformationResponse;

  /* mandatory */
  /* c1. GNB_CU_UE_F1AP_ID */
  asn1cSequenceAdd(out->protocolIEs.list, F1AP_PositioningInformationResponseIEs_t, ie1);
  ie1->id = F1AP_ProtocolIE_ID_id_gNB_CU_UE_F1AP_ID;
  ie1->criticality = F1AP_Criticality_reject;
  ie1->value.present = F1AP_PositioningInformationResponseIEs__value_PR_GNB_CU_UE_F1AP_ID;
  ie1->value.choice.GNB_CU_UE_F1AP_ID = f1ap_positioning_information_resp->gNB_CU_ue_id;

  /* mandatory */
  /* c2. GNB_DU_UE_F1AP_ID */
  asn1cSequenceAdd(out->protocolIEs.list, F1AP_PositioningInformationResponseIEs_t, ie2);
  ie2->id = F1AP_ProtocolIE_ID_id_gNB_DU_UE_F1AP_ID;
  ie2->criticality = F1AP_Criticality_reject;
  ie2->value.present = F1AP_PositioningInformationResponseIEs__value_PR_GNB_DU_UE_F1AP_ID;
  ie2->value.choice.GNB_DU_UE_F1AP_ID =
      f1ap_positioning_information_resp->gNB_DU_ue_id; // f1ap_get_du_ue_f1ap_id(DUtype, instance, resp->rnti);

  /* Optional */
  /* SRSConfiguration */
  if (0) {
    asn1cSequenceAdd(out->protocolIEs.list, F1AP_PositioningInformationResponseIEs_t, ie3);
    ie3->id = F1AP_ProtocolIE_ID_id_SRSConfiguration;
    ie3->criticality = F1AP_Criticality_ignore;
    ie3->value.present = F1AP_PositioningInformationResponseIEs__value_PR_SRSConfiguration;
  }

  /* Optional */
  /* SFN Initialisation Time*/
  if (0) {
    asn1cSequenceAdd(out->protocolIEs.list, F1AP_PositioningInformationResponseIEs_t, ie4);
    ie4->id = F1AP_ProtocolIE_ID_id_SFNInitialisationTime;
    ie4->criticality = F1AP_Criticality_ignore;
    ie4->value.present = F1AP_PositioningInformationResponseIEs__value_PR_SFNInitialisationTime;
    // TODO Retreive SFN Initialisation Time and assign
    // ie4->value.choice.SFNInitialisationTime.buf = NULL ; //TODO adeel retrieve and add TYPE typedef struct BIT_STRING_s {uint8_t
    // *buf;	size_t size;	int bits_unused;} BIT_STRING_t; ie4->value.choice.SFNInitialisationTime.size =4;
    // ie4->value.choice.SFNInitialisationTime.bits_unused =0;
  }

  /* Optional */
  /*  CriticalityDiagnostics */
  if (0) {
    asn1cSequenceAdd(out->protocolIEs.list, F1AP_PositioningInformationResponseIEs_t, ie5);
    ie5->id = F1AP_ProtocolIE_ID_id_CriticalityDiagnostics;
    ie5->criticality = F1AP_Criticality_ignore;
    ie5->value.present = F1AP_PositioningInformationResponseIEs__value_PR_CriticalityDiagnostics;
    asn1cCallocOne(ie5->value.choice.CriticalityDiagnostics.procedureCode, F1AP_ProcedureCode_id_PositioningInformationExchange);
    asn1cCallocOne(ie5->value.choice.CriticalityDiagnostics.triggeringMessage, F1AP_TriggeringMessage_initiating_message);
    asn1cCallocOne(ie5->value.choice.CriticalityDiagnostics.procedureCriticality, F1AP_Criticality_reject);
    asn1cCallocOne(ie5->value.choice.CriticalityDiagnostics.transactionID, 0);
  }

  /* encode */
  if (f1ap_encode_pdu(&pdu, &buffer, &len) < 0) {
    LOG_E(F1AP, "Failed to encode F1 UE CONTEXT SETUP RESPONSE\n");
    return -1;
  }

  //  f1ap_itti_send_sctp_data_req(false, instance, buffer, len, getCxt(false, instance)->default_sctp_stream_id);
  f1ap_itti_send_sctp_data_req(instance, buffer, len);
  return 0;
}

int DU_send_POSITIONING_INFORMATION_FAILURE(instance_t instance,
                                            f1ap_positioning_information_failure_t *f1ap_positioning_information_failure)
{
  F1AP_F1AP_PDU_t pdu = {0};
  F1AP_PositioningInformationFailure_t *out;
  uint8_t *buffer = NULL;
  uint32_t len = 0;

  /* Create */
  /* 0. Message Type */
  pdu.present = F1AP_F1AP_PDU_PR_unsuccessfulOutcome;
  asn1cCalloc(pdu.choice.unsuccessfulOutcome, tmp);
  tmp->procedureCode = F1AP_ProcedureCode_id_PositioningInformationExchange;
  tmp->criticality = F1AP_Criticality_reject;
  tmp->value.present = F1AP_UnsuccessfulOutcome__value_PR_PositioningInformationFailure;
  out = &tmp->value.choice.PositioningInformationFailure;

  /* mandatory */
  /* c1. GNB_CU_UE_F1AP_ID */
  asn1cSequenceAdd(out->protocolIEs.list, F1AP_PositioningInformationFailureIEs_t, ie1);
  ie1->id = F1AP_ProtocolIE_ID_id_gNB_CU_UE_F1AP_ID;
  ie1->criticality = F1AP_Criticality_reject;
  ie1->value.present = F1AP_PositioningInformationFailureIEs__value_PR_GNB_CU_UE_F1AP_ID;
  ie1->value.choice.GNB_CU_UE_F1AP_ID = f1ap_positioning_information_failure->gNB_CU_ue_id;

  /* mandatory */
  /* c2. GNB_DU_UE_F1AP_ID */
  asn1cSequenceAdd(out->protocolIEs.list, F1AP_PositioningInformationFailureIEs_t, ie2);
  ie2->id = F1AP_ProtocolIE_ID_id_gNB_DU_UE_F1AP_ID;
  ie2->criticality = F1AP_Criticality_reject;
  ie2->value.present = F1AP_PositioningInformationFailureIEs__value_PR_GNB_DU_UE_F1AP_ID;
  ie2->value.choice.GNB_DU_UE_F1AP_ID =
      f1ap_positioning_information_failure->gNB_DU_ue_id; // f1ap_get_du_ue_f1ap_id(DUtype, instance, resp->rnti);

  /* Optional */
  /*  CriticalityDiagnostics */
  if (0) {
    asn1cSequenceAdd(out->protocolIEs.list, F1AP_PositioningInformationFailureIEs_t, ie3);
    ie3->id = F1AP_ProtocolIE_ID_id_Cause;
    ie3->criticality = F1AP_Criticality_ignore;
    ie3->value.present = F1AP_PositioningInformationFailureIEs__value_PR_Cause;
    // TODO Reteive Cause and assign
    ie3->value.choice.Cause.present = F1AP_Cause_PR_misc; // IE 1
    // ie->value.choice.Cause.present = F1AP_Cause_PR_NOTHING ; //IE 1
    ie3->value.choice.Cause.choice.misc = 0; // TODO dummay response
                                             // ie->value.choice.Cause. =;  // IE 2 and so on
  }

  /* Optional */
  /*  CriticalityDiagnostics */
  if (0) {
    asn1cSequenceAdd(out->protocolIEs.list, F1AP_PositioningInformationFailureIEs_t, ie4);
    ie4->id = F1AP_ProtocolIE_ID_id_CriticalityDiagnostics;
    ie4->criticality = F1AP_Criticality_ignore;
    ie4->value.present = F1AP_PositioningInformationFailureIEs__value_PR_CriticalityDiagnostics;
    asn1cCallocOne(ie4->value.choice.CriticalityDiagnostics.procedureCode, F1AP_ProcedureCode_id_PositioningInformationExchange);
    asn1cCallocOne(ie4->value.choice.CriticalityDiagnostics.triggeringMessage, F1AP_TriggeringMessage_initiating_message);
    asn1cCallocOne(ie4->value.choice.CriticalityDiagnostics.procedureCriticality, F1AP_Criticality_reject);
    asn1cCallocOne(ie4->value.choice.CriticalityDiagnostics.transactionID, 0);
  }

  /* encode */
  if (f1ap_encode_pdu(&pdu, &buffer, &len) < 0) {
    LOG_E(F1AP, "Failed to encode F1 UE CONTEXT SETUP RESPONSE\n");
    return -1;
  }

  //  f1ap_itti_send_sctp_data_req(false, instance, buffer, len, getCxt(false, instance)->default_sctp_stream_id);
  f1ap_itti_send_sctp_data_req(instance, buffer, len);
  return 0;
}
