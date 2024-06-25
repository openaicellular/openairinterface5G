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

/*! \file nrppa_gNB_position_information_transfer_procedures.c
 * \brief NRPPA gNB tasks related to position information transfer
 * \author Adeel Malik
 * \email adeel.malik@eurecom.fr
 *\date 2023
 * \version 1.0
 * @ingroup _nrppa
 */

#include "intertask_interface.h"
#include "nrppa_common.h"
#include "nrppa_gNB_position_information_transfer_procedures.h"
#include "nrppa_gNB_itti_messaging.h"
#include "nrppa_gNB_encoder.h"

// DOWNLINK
// PositioningInformationExchange (Parent) procedure for  PositioningInformationRequest/Response/Failure
int nrppa_gNB_handle_PositioningInformationExchange(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *rx_pdu)
{
  LOG_I(NRPPA, "Processing Received PositioningInformationRequest \n");
  DevAssert(rx_pdu != NULL);
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, rx_pdu);

  // Preparing f1ap request for RRC
  MessageDef *msg = itti_alloc_new_message(TASK_RRC_GNB, 0, F1AP_POSITIONING_INFORMATION_REQ);
  f1ap_positioning_information_req_t *f1ap_req = &F1AP_POSITIONING_INFORMATION_REQ(msg);
  f1ap_req->nrppa_msg_info.instance = nrppa_msg_info->instance;
  f1ap_req->nrppa_msg_info.gNB_ue_ngap_id = nrppa_msg_info->gNB_ue_ngap_id;
  f1ap_req->nrppa_msg_info.amf_ue_ngap_id = nrppa_msg_info->amf_ue_ngap_id;
  f1ap_req->nrppa_msg_info.routing_id_buffer = nrppa_msg_info->routing_id_buffer;
  f1ap_req->nrppa_msg_info.routing_id_length = nrppa_msg_info->routing_id_length;

  // Processing Received PositioningInformationRequest
  NRPPA_PositioningInformationRequest_t *container = NULL;
  NRPPA_PositioningInformationRequest_IEs_t *ie = NULL;

  // IE 9.2.3 Message type (M)
  container = &rx_pdu->choice.initiatingMessage->value.choice.PositioningInformationRequest;

  // IE 9.2.4 nrppatransactionID (M)
  f1ap_req->nrppa_msg_info.nrppa_transaction_id = rx_pdu->choice.initiatingMessage->nrppatransactionID;

  // IE 9.2.27 RequestedSRSTransmissionCharacteristics (Optional)
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_PositioningInformationRequest_IEs_t,
                              ie,
                              container,
                              NRPPA_ProtocolIE_ID_id_RequestedSRSTransmissionCharacteristics,
                              false);
  if (ie != NULL) {
    NRPPA_RequestedSRSTransmissionCharacteristics_t req_SRS_info = ie->value.choice.RequestedSRSTransmissionCharacteristics;
    // IE Resource Type
    f1ap_req->req_SRS_info.resourceType = req_SRS_info.resourceType;
    // IE number Of periodic Transmissions
    // long	*numberOfTransmissions = req_SRS_info.numberOfTransmissions;
    // f1ap_req->req_SRS_info.numberOfTransmissions =*numberOfTransmissions; //req_SRS_info.numberOfTransmissions; // OPTIONAL
    // IE bandwidth_srs
    switch (req_SRS_info.bandwidth.present) {
      case NRPPA_BandwidthSRS_PR_NOTHING:
        f1ap_req->req_SRS_info.bandwidth_srs.present = f1ap_bandwidth_srs_pr_nothing;
        break;
      case NRPPA_BandwidthSRS_PR_fR1:
        f1ap_req->req_SRS_info.bandwidth_srs.present = f1ap_bandwidth_srs_pr_fR1;
        f1ap_req->req_SRS_info.bandwidth_srs.choice.fR1 = req_SRS_info.bandwidth.choice.fR1;
        break;
      case NRPPA_BandwidthSRS_PR_fR2:
        f1ap_req->req_SRS_info.bandwidth_srs.present = f1ap_bandwidth_srs_pr_fR2;
        f1ap_req->req_SRS_info.bandwidth_srs.choice.fR2 = req_SRS_info.bandwidth.choice.fR1;
        break;
      default:
        NRPPA_ERROR("PositioningInformationRequest Unknown BandwidthSRS Choice\n");
        break;
    }
  }

  LOG_I(NRPPA,
        "Forwarding to RRC PositioningInformationRequest gNB_CU_ue_id=%d, gNB_DU_ue_id=%d \n",
        f1ap_req->gNB_CU_ue_id,
        f1ap_req->gNB_DU_ue_id);
  itti_send_msg_to_task(TASK_RRC_GNB, 0, msg);
  return 0;
}

// PositioningActivation (Parent) procedure for  PositioningActivationRequest/Response/Failure
int nrppa_gNB_handle_PositioningActivation(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu)
{
  LOG_I(NRPPA, "Processing Received PositioningActivation \n");
  DevAssert(pdu != NULL);
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, pdu);

  // Preparing f1ap request for RRC
  MessageDef *msg = itti_alloc_new_message(TASK_RRC_GNB, 0, F1AP_POSITIONING_ACTIVATION_REQ);
  f1ap_positioning_activation_req_t *f1ap_req = &F1AP_POSITIONING_ACTIVATION_REQ(msg);
  f1ap_req->nrppa_msg_info.instance = nrppa_msg_info->instance;
  f1ap_req->nrppa_msg_info.gNB_ue_ngap_id = nrppa_msg_info->gNB_ue_ngap_id;
  f1ap_req->nrppa_msg_info.amf_ue_ngap_id = nrppa_msg_info->amf_ue_ngap_id;
  f1ap_req->nrppa_msg_info.routing_id_buffer = nrppa_msg_info->routing_id_buffer;
  f1ap_req->nrppa_msg_info.routing_id_length = nrppa_msg_info->routing_id_length;

  // Processing Received PositioningActivation
  NRPPA_PositioningActivationRequest_t *container = NULL;
  NRPPA_PositioningActivationRequestIEs_t *ie = NULL;

  // IE 9.2.3 Message type (M)
  container = &pdu->choice.initiatingMessage->value.choice.PositioningActivationRequest;

  // IE 9.2.4 nrppatransactionID (M)
  f1ap_req->nrppa_msg_info.nrppa_transaction_id = pdu->choice.initiatingMessage->nrppatransactionID;

  // IE  SRSType (M)
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_PositioningActivationRequestIEs_t, ie, container, NRPPA_ProtocolIE_ID_id_SRSType, false);
  if (ie != NULL) {
    NRPPA_SRSType_t srs_type = ie->value.choice.SRSType;
    // IE SRS type filling in f1ap message
    switch (srs_type.present) {
      case NRPPA_SRSType_PR_NOTHING:
        f1ap_req->srs_type.present = f1ap_srs_type_pr_NOTHING;
        break;
      case NRPPA_SRSType_PR_semipersistentSRS:
        f1ap_req->srs_type.present = f1ap_srs_type_pr_semipersistentSRS;
        f1ap_req->srs_type.choice.semipersistentSRS.sRSResourceSetID = srs_type.choice.semipersistentSRS->sRSResourceSetID;
        // f1ap_req->srs_type.choice.semipersistentSRS.sRSSpatialRelation // optional
        break;
      case NRPPA_SRSType_PR_aperiodicSRS:
        f1ap_req->srs_type.present = f1ap_srs_type_pr_aperiodicSRS;
        f1ap_req->srs_type.choice.aperiodicSRS.aperiodic = srs_type.choice.aperiodicSRS->aperiodic;
        // f1ap_req->srs_type.choice.aperiodicSRS.sRSResourceTrigger // optional
        break;
      default:
        NRPPA_ERROR("PositioningActivationRequest Unknown SRS type\n");
        break;
    }
  }

  // IE  Activation Time 9.2.36 (Optional) type sfn_initialisation_time
  // NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_PositioningActivationRequestIEs_t, ie, container, NRPPA_ProtocolIE_ID_id_ActivationTime,
  // true); f1ap_req->activation_time.size f1ap_req->activation_time.buf

  LOG_I(NRPPA,
        "Forwarding to RRC PositioningActivationRequest gNB_CU_ue_id=%d, gNB_DU_ue_id=%d \n",
        f1ap_req->gNB_CU_ue_id,
        f1ap_req->gNB_DU_ue_id);
  itti_send_msg_to_task(TASK_RRC_GNB, 0, msg);
  return 0;
}

int nrppa_gNB_handle_PositioningDeactivation(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu)
{
  LOG_I(NRPPA, "Processing Received PositioningDeActivation \n");
  DevAssert(pdu != NULL);
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, pdu);

  // Preparing f1ap request for RRC
  MessageDef *msg = itti_alloc_new_message(TASK_RRC_GNB, 0, F1AP_POSITIONING_DEACTIVATION);
  f1ap_positioning_deactivation_t *f1ap_req = &F1AP_POSITIONING_DEACTIVATION(msg);
  f1ap_req->nrppa_msg_info.instance = nrppa_msg_info->instance;
  f1ap_req->nrppa_msg_info.gNB_ue_ngap_id = nrppa_msg_info->gNB_ue_ngap_id;
  f1ap_req->nrppa_msg_info.amf_ue_ngap_id = nrppa_msg_info->amf_ue_ngap_id;
  f1ap_req->nrppa_msg_info.routing_id_buffer = nrppa_msg_info->routing_id_buffer;
  f1ap_req->nrppa_msg_info.routing_id_length = nrppa_msg_info->routing_id_length;

  // Processing Received PositioningDeActivation
  NRPPA_PositioningDeactivation_t *container = NULL;
  NRPPA_PositioningDeactivationIEs_t *ie = NULL;

  // IE 9.2.3 Message type (M)
  container = &pdu->choice.initiatingMessage->value.choice.PositioningDeactivation;

  // IE 9.2.4 nrppatransactionID (M)
  f1ap_req->nrppa_msg_info.nrppa_transaction_id = pdu->choice.initiatingMessage->nrppatransactionID;

  // IE  Abort Transmission(M)
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_PositioningDeactivationIEs_t, ie, container, NRPPA_ProtocolIE_ID_id_AbortTransmission, false);
  if (ie != NULL) {
    NRPPA_AbortTransmission_t abort_transmission = ie->value.choice.AbortTransmission;
    // IE abort_transmission filling in f1ap message
    switch (abort_transmission.present) {
      case NRPPA_AbortTransmission_PR_NOTHING:
        f1ap_req->abort_transmission.present = f1ap_abort_transmission_pr_NOTHING;
        break;
      case NRPPA_AbortTransmission_PR_sRSResourceSetID:
        f1ap_req->abort_transmission.present = f1ap_abort_transmission_pr_sRSResourceSetID;
        f1ap_req->abort_transmission.choice.sRSResourceSetID = abort_transmission.choice.sRSResourceSetID;
        break;
      case NRPPA_AbortTransmission_PR_releaseALL:
        f1ap_req->abort_transmission.present = f1ap_abort_transmission_pr_releaseALL;
        f1ap_req->abort_transmission.choice.releaseALL = abort_transmission.choice.releaseALL;
        break;
      default:
        NRPPA_ERROR("PositioningDeActivation Unknown Abort Transmission\n");
        break;
    }
  }
  LOG_I(NRPPA,
        "Forwarding to RRC PositioningDeactivation gNB_CU_ue_id=%d, gNB_DU_ue_id=%d \n",
        f1ap_req->gNB_CU_ue_id,
        f1ap_req->gNB_DU_ue_id);
  itti_send_msg_to_task(TASK_RRC_GNB, 0, msg);
  return 0;
}

// UPLINK
int nrppa_gNB_PositioningInformationResponse(instance_t instance, MessageDef *msg_p)
{
  f1ap_positioning_information_resp_t *resp = &F1AP_POSITIONING_INFORMATION_RESP(msg_p);
  LOG_I(NRPPA,
        "Received PositioningInformationResponse info from RRC  gNB_CU_ue_id=%d, gNB_DU_ue_id=%d  rnti= %04x\n",
        resp->gNB_CU_ue_id,
        resp->gNB_DU_ue_id,
        resp->nrppa_msg_info.ue_rnti);

  // Prepare NRPPA Position Information transfer Response
  NRPPA_NRPPA_PDU_t tx_pdu ={0};

  /* Prepare the NRPPA message to encode for successfulOutcome PositioningInformationResponse */

  // IE: 9.2.3 Message Type successfulOutcome (M)
  memset(&tx_pdu, 0, sizeof(tx_pdu));
  tx_pdu.present = NRPPA_NRPPA_PDU_PR_successfulOutcome;
  asn1cCalloc(tx_pdu.choice.successfulOutcome, head);
  head->procedureCode = NRPPA_ProcedureCode_id_positioningInformationExchange;
  head->criticality = NRPPA_Criticality_reject;
  head->value.present = NRPPA_SuccessfulOutcome__value_PR_PositioningInformationResponse;

  // IE 9.2.4 nrppatransactionID  (M)
  head->nrppatransactionID = resp->nrppa_msg_info.nrppa_transaction_id;
  NRPPA_PositioningInformationResponse_t *out = &head->value.choice.PositioningInformationResponse;

  // IE 9.2.28 SRS Configuration (O)
  //  changing SRS configuration on the fly is not possible, therefore we add the predefined SRS config in NRPPa pdu
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_PositioningInformationResponse_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_SRSConfiguration;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_PositioningInformationResponse_IEs__value_PR_SRSConfiguration;

    // Preparing SRS Carrier List an IE  of SRS Configuration
    int nb_of_srscarrier = resp->srs_configuration.srs_carrier_list.srs_carrier_list_length;
    f1ap_srs_carrier_list_item_t *carrier_list_item = resp->srs_configuration.srs_carrier_list.srs_carrier_list_item;

    LOG_D(NRPPA, "Positioning_information_response(); nb_of_srscarrier= %d \n", nb_of_srscarrier);
    for (int ci = 0; ci < nb_of_srscarrier; ci++) {
      asn1cSequenceAdd(ie->value.choice.SRSConfiguration.sRSCarrier_List.list, NRPPA_SRSCarrier_List_Item_t, item);

      item->pointA = carrier_list_item->pointA; // IE of SRSCarrier_List_Item
      // asn1cCalloc(item->pCI, pci);
      // pci = carrier_list_item->pci; //Optional IE Physical cell ID of the cell that contians the SRS carrier

      // Preparing Active UL BWP information IE of SRSCarrier_List
      item->activeULBWP.locationAndBandwidth = carrier_list_item->active_ul_bwp.locationAndBandwidth;
      item->activeULBWP.subcarrierSpacing = carrier_list_item->active_ul_bwp.subcarrierSpacing;
      item->activeULBWP.cyclicPrefix = carrier_list_item->active_ul_bwp.cyclicPrefix;
      item->activeULBWP.txDirectCurrentLocation = carrier_list_item->active_ul_bwp.txDirectCurrentLocation;
      // asn1cCalloc(item->activeULBWP.shift7dot5kHz, shift7dot5kHz);
      // shift7dot5kHz = carrier_list_item->active_ul_bwp.shift7dot5kHz; //  Optional

      // Preparing sRSResource_List  IE of SRSConfig (IE of activeULBWP)
      int nb_srsresource = carrier_list_item->active_ul_bwp.sRSConfig.sRSResource_List.srs_resource_list_length;
      asn1cCalloc(item->activeULBWP.sRSConfig.sRSResource_List, srsresource_list);
      f1ap_srs_resource_t *res_item = carrier_list_item->active_ul_bwp.sRSConfig.sRSResource_List.srs_resource;
      for (int k = 0; k < nb_srsresource; k++){ // Preparing SRS Resource List
        asn1cSequenceAdd(srsresource_list->list, NRPPA_SRSResource_t, resource_item);
        resource_item->sRSResourceID = res_item->sRSResourceID; //(M)
        resource_item->nrofSRS_Ports = res_item->nrofSRS_Ports; //(M) port1	= 0, ports2	= 1, ports4	= 2
        resource_item->startPosition = res_item->startPosition; //(M)
        resource_item->nrofSymbols = res_item->nrofSymbols; //(M)  n1	= 0, n2	= 1, n4	= 2
        resource_item->repetitionFactor = res_item->repetitionFactor; //(M)  n1	= 0, n2	= 1, n4	= 2
        resource_item->freqDomainPosition = res_item->freqDomainPosition; //(M)
        resource_item->freqDomainShift = res_item->freqDomainShift; //(M)
        resource_item->c_SRS = res_item->c_SRS; //(M)
        resource_item->b_SRS = res_item->b_SRS; //(M)
        resource_item->b_hop = res_item->b_hop; //(M)
        resource_item->groupOrSequenceHopping = res_item->groupOrSequenceHopping; //(M)
        resource_item->slotOffset = res_item->slotOffset; //(M)
        resource_item->sequenceId = res_item->sequenceId; //(M)

        // IE transmissionComb
        switch (res_item->transmissionComb.present) {
          case f1ap_transmission_comb_pr_n2:
            resource_item->transmissionComb.present = NRPPA_TransmissionComb_PR_n2;
            asn1cCalloc(resource_item->transmissionComb.choice.n2, comb_n2);
            comb_n2->combOffset_n2 = res_item->transmissionComb.choice.n2.combOffset_n2;
            comb_n2->cyclicShift_n2 = res_item->transmissionComb.choice.n2.cyclicShift_n2;
            break;

          case f1ap_transmission_comb_pr_n4:
            resource_item->transmissionComb.present = NRPPA_TransmissionComb_PR_n4;
            asn1cCalloc(resource_item->transmissionComb.choice.n4, comb_n4);
            comb_n4->combOffset_n4 = res_item->transmissionComb.choice.n4.combOffset_n4;
            comb_n4->cyclicShift_n4 = res_item->transmissionComb.choice.n4.cyclicShift_n4;
            break;

          case f1ap_transmission_comb_pr_nothing:
            resource_item->transmissionComb.present = NRPPA_TransmissionComb_PR_NOTHING;
            break;

          default:
            NRPPA_ERROR("Unknown  Resource Item TransmissionComb\n");
            break;
        }

        // IE resourceType
        switch (res_item->resourceType.present) {
          case f1ap_resource_type_pr_periodic:
            resource_item->resourceType.present = NRPPA_ResourceType_PR_periodic;
            asn1cCalloc(resource_item->resourceType.choice.periodic, res_type_periodic);
            res_type_periodic->periodicity = res_item->resourceType.choice.periodic.periodicity;
            res_type_periodic->offset = res_item->resourceType.choice.periodic.offset;
            break;
          case f1ap_resource_type_pr_aperiodic:
            resource_item->resourceType.present = NRPPA_ResourceType_PR_aperiodic;
            asn1cCalloc(resource_item->resourceType.choice.aperiodic, res_type_aperiodic);
            res_type_aperiodic->aperiodicResourceType = res_item->resourceType.choice.aperiodic.aperiodicResourceType;
            break;
          case f1ap_resource_type_pr_semi_persistent:
            resource_item->resourceType.present = NRPPA_ResourceType_PR_semi_persistent;
            asn1cCalloc(resource_item->resourceType.choice.periodic, res_type_semi_persistent);
            res_type_semi_persistent->periodicity = res_item->resourceType.choice.semi_persistent.periodicity;
            res_type_semi_persistent->offset = res_item->resourceType.choice.semi_persistent.offset;
            break;
          case f1ap_resource_type_pr_nothing:
            resource_item->resourceType.present = NRPPA_ResourceType_PR_NOTHING;
            break;
          default:
            NRPPA_ERROR("Unknown Resource Item resourceType\n");
            break;
        }
        if (k < nb_srsresource - 1) {
          res_item++;
        }
      } // for(int k=0; k < nb_srsresource; k++)

      // Preparing posSRSResource_List IE of SRSConfig (IE of activeULBWP)
      int nb_possrsresource = carrier_list_item->active_ul_bwp.sRSConfig.posSRSResource_List.pos_srs_resource_list_length;
      f1ap_pos_srs_resource_item_t *pos_res_item =
          carrier_list_item->active_ul_bwp.sRSConfig.posSRSResource_List.pos_srs_resource_item;
      asn1cCalloc(item->activeULBWP.sRSConfig.posSRSResource_List, possrsresource_list);
      LOG_D(NRPPA, " PositioningInformationResponse nb_possrsresource=%d \n", nb_possrsresource);
      for (int p = 0; p < nb_possrsresource; p++) // Preparing posSRSResource_List
      {
        asn1cSequenceAdd(possrsresource_list->list, NRPPA_PosSRSResource_Item_t, pos_resource_item);
        pos_resource_item->srs_PosResourceId = pos_res_item->srs_PosResourceId; // (M)
        pos_resource_item->startPosition = pos_res_item->startPosition; // (M)  range (0,1,...13)
        pos_resource_item->nrofSymbols = pos_res_item->nrofSymbols; // (M)  n1	= 0, n2	= 1, n4	= 2, n8	= 3, n12 = 4
        pos_resource_item->freqDomainShift = pos_res_item->freqDomainShift; // (M)
        pos_resource_item->c_SRS = pos_res_item->c_SRS; // (M)
        pos_resource_item->groupOrSequenceHopping = pos_res_item->groupOrSequenceHopping; // (M)
        pos_resource_item->sequenceId = pos_res_item->sequenceId; //(M)

        // IE transmissionCombPos
        switch (pos_res_item->transmissionCombPos.present) {
          case f1ap_transmission_comb_pos_pr_NOTHING:
            pos_resource_item->transmissionCombPos.present = NRPPA_TransmissionCombPos_PR_NOTHING;
            break;
          case f1ap_transmission_comb_pos_pr_n2:
            pos_resource_item->transmissionCombPos.present = NRPPA_TransmissionCombPos_PR_n2;
            asn1cCalloc(pos_resource_item->transmissionCombPos.choice.n2, combPos_n2);
            combPos_n2->combOffset_n2 = pos_res_item->transmissionCombPos.choice.n2.combOffset_n2;
            combPos_n2->cyclicShift_n2 = pos_res_item->transmissionCombPos.choice.n2.cyclicShift_n2;
            break;
          case f1ap_transmission_comb_pos_pr_n4:
            pos_resource_item->transmissionCombPos.present = NRPPA_TransmissionCombPos_PR_n4;
            asn1cCalloc(pos_resource_item->transmissionCombPos.choice.n4, combPos_n4);
            combPos_n4->combOffset_n4 = pos_res_item->transmissionCombPos.choice.n4.combOffset_n4;
            combPos_n4->cyclicShift_n4 = pos_res_item->transmissionCombPos.choice.n4.cyclicShift_n4;
            break;
          case f1ap_transmission_comb_pos_pr_n8:
            pos_resource_item->transmissionCombPos.present = NRPPA_TransmissionCombPos_PR_n8;
            asn1cCalloc(pos_resource_item->transmissionCombPos.choice.n8, combPos_n8);
            combPos_n8->combOffset_n8 = pos_res_item->transmissionCombPos.choice.n8.combOffset_n8;
            combPos_n8->cyclicShift_n8 = pos_res_item->transmissionCombPos.choice.n8.cyclicShift_n8;
            break;
          default:
            NRPPA_ERROR(" Pos Resource Item Unknown transmissionCombPos \n");
            break;
        }

        // IE resourceTypePos
        switch (pos_res_item->resourceTypePos.present) {
          case f1ap_resource_type_pos_pr_NOTHING:
            pos_resource_item->resourceTypePos.present = NRPPA_ResourceTypePos_PR_NOTHING;
            break;
          case f1ap_resource_type_pos_pr_periodic:
            pos_resource_item->resourceTypePos.present = NRPPA_ResourceTypePos_PR_periodic;
            asn1cCalloc(pos_resource_item->resourceTypePos.choice.periodic, pos_res_type_periodic);
            pos_res_type_periodic->periodicity = pos_res_item->resourceTypePos.choice.periodic.periodicity;
            pos_res_type_periodic->offset = pos_res_item->resourceTypePos.choice.periodic.offset;
            break;
          case f1ap_resource_type_pos_pr_semi_persistent:
            pos_resource_item->resourceTypePos.present = NRPPA_ResourceTypePos_PR_semi_persistent;
            asn1cCalloc(pos_resource_item->resourceTypePos.choice.semi_persistent, pos_res_type_semi_persistent);
            pos_res_type_semi_persistent->periodicity = pos_res_item->resourceTypePos.choice.semi_persistent.periodicity;
            pos_res_type_semi_persistent->offset = pos_res_item->resourceTypePos.choice.semi_persistent.offset;
            break;
          case f1ap_resource_type_pos_pr_aperiodic:
            pos_resource_item->resourceTypePos.present = NRPPA_ResourceTypePos_PR_aperiodic;
            asn1cCalloc(pos_resource_item->resourceTypePos.choice.aperiodic, pos_res_type_aperiodic);
            pos_res_type_aperiodic->slotOffset = pos_res_item->resourceTypePos.choice.aperiodic.slotOffset;
            break;
          default:
            NRPPA_ERROR("Pos Resource Item Unknown resourceTypePos \n");
            break;
        }
        // pos_resource_item->spatialRelationPos                     =pos_res_item->spatialRelationPos;	// OPTIONAl
        if (p < nb_possrsresource - 1) {
          pos_res_item++;
        }
      } // for(int p=0; p < nb_possrsresource; p++)

      // Preparing sRSResourceSet_List IE of SRSConfig (IE of activeULBWP)
      int nb_srsresourceset = carrier_list_item->active_ul_bwp.sRSConfig.sRSResourceSet_List
                                  .srs_resource_set_list_length; // srs_config->srs_ResourceSetToAddModList->list.count;
      f1ap_srs_resource_set_t *resSet_item = carrier_list_item->active_ul_bwp.sRSConfig.sRSResourceSet_List.srs_resource_set;
      LOG_D(NRPPA, "PositioningInformationResponse nb_srsresourceset=%d \n", nb_srsresourceset);
      asn1cCalloc(item->activeULBWP.sRSConfig.sRSResourceSet_List, srsresourceset_list);
      for (int y = 0; y < nb_srsresourceset; y++) // Preparing SRS Resource Set List
      {
        asn1cSequenceAdd(srsresourceset_list->list, NRPPA_SRSResourceSet_t, srsresourceset_item);
        // IE sRSResourceSetID
        srsresourceset_item->sRSResourceSetID = resSet_item->sRSResourceSetID; // (M)
        // IE sRSResourceID_List
        int nb_srsresourceperset = resSet_item->sRSResourceID_List.srs_resource_id_list_length;
        long *srs_res_id = resSet_item->sRSResourceID_List.srs_resource_id;
        for (int y = 0; y < nb_srsresourceperset; y++) {
          asn1cSequenceAdd(srsresourceset_item->sRSResourceID_List.list, NRPPA_SRSResourceID_t, srsresourceID);
          srsresourceID = srs_res_id;
          srs_res_id++;
        }

        // IE resourceSetType
        switch (resSet_item->resourceSetType.present) {
          case f1ap_resource_set_type_pr_periodic:
            srsresourceset_item->resourceSetType.present = NRPPA_ResourceSetType_PR_periodic;
            asn1cCalloc(srsresourceset_item->resourceSetType.choice.periodic, res_set_type_periodic);
            res_set_type_periodic->periodicSet = resSet_item->resourceSetType.choice.periodic.periodicSet;
            break;
          case f1ap_resource_set_type_pr_aperiodic:
            srsresourceset_item->resourceSetType.present = NRPPA_ResourceSetType_PR_aperiodic;
            asn1cCalloc(srsresourceset_item->resourceSetType.choice.aperiodic, res_set_type_aperiodic);
            res_set_type_aperiodic->sRSResourceTrigger = resSet_item->resourceSetType.choice.aperiodic.sRSResourceTrigger;
            res_set_type_aperiodic->slotoffset = resSet_item->resourceSetType.choice.aperiodic.slotoffset;
            break;
          case f1ap_resource_set_type_pr_semi_persistent:
            srsresourceset_item->resourceSetType.present = NRPPA_ResourceSetType_PR_semi_persistent;
            asn1cCalloc(srsresourceset_item->resourceSetType.choice.semi_persistent, res_set_type_semi_persistent);
            res_set_type_semi_persistent->semi_persistentSet =
                resSet_item->resourceSetType.choice.semi_persistent.semi_persistentSet;
            break;
          case f1ap_resource_set_type_pr_nothing:
            srsresourceset_item->resourceSetType.present = NRPPA_ResourceSetType_PR_NOTHING;
            break;
          default:
            NRPPA_ERROR("srsresourceset_item Unknown resourceSetType \n");
            break;
        }

        if (y < nb_srsresourceset - 1) {
          resSet_item++;
        }
      } // for(int y=0; y < nb_srsresourceset; y++)

      // Preparing posSRSResourceSet_List IE of SRSConfig (IE of activeULBWP)
      int nb_possrsresourceset = carrier_list_item->active_ul_bwp.sRSConfig.posSRSResourceSet_List
                                     .pos_srs_resource_set_list_length; // srs_config->srs_ResourceToAddModList->list.count;
      asn1cCalloc(item->activeULBWP.sRSConfig.posSRSResourceSet_List, possrsresourceset_list);
      f1ap_pos_srs_resource_set_item_t *pos_res_set_item =
          carrier_list_item->active_ul_bwp.sRSConfig.posSRSResourceSet_List.pos_srs_resource_set_item;
      LOG_D(NRPPA, "PositioningInformationResponse nb_possrsresourceset=%d \n", nb_possrsresourceset);
      for (int j = 0; j < nb_possrsresourceset; j++) // Preparing posSRSResourceSet_List
      {
        asn1cSequenceAdd(possrsresourceset_list->list, NRPPA_PosSRSResourceSet_Item_t, pos_resource_set_item);

        // IE possrsResourceSetID
        pos_resource_set_item->possrsResourceSetID = pos_res_set_item->possrsResourceSetID; // (M)
        // IE possRSResourceID_List
        int nb_srsposresourceperset = pos_res_set_item->possRSResourceID_List.pos_srs_resource_id_list_length;
        long *pos_srs_res_id = pos_res_set_item->possRSResourceID_List.srs_pos_resource_id;
        for (int y = 0; y < nb_srsposresourceperset; y++) {
          asn1cSequenceAdd(pos_resource_set_item->possRSResourceID_List.list, NRPPA_SRSPosResourceID_t, srsposresourceID);
          srsposresourceID = pos_srs_res_id; // TODO add increment to pointer
          pos_srs_res_id++;
        }

        // IE posresourceSetType
        switch (pos_res_set_item->posresourceSetType.present) {
          case f1ap_pos_resource_set_type_pr_nothing:
            pos_resource_set_item->posresourceSetType.present = NRPPA_PosResourceSetType_PR_NOTHING;
            break;
          case f1ap_pos_resource_set_type_pr_periodic:
            pos_resource_set_item->posresourceSetType.present = NRPPA_PosResourceSetType_PR_periodic;
            asn1cCalloc(pos_resource_set_item->posresourceSetType.choice.periodic, pos_res_set_type_periodic);
            pos_res_set_type_periodic->posperiodicSet = pos_res_set_item->posresourceSetType.choice.periodic.posperiodicSet;
            break;
          case f1ap_pos_resource_set_type_pr_aperiodic:
            pos_resource_set_item->posresourceSetType.present = NRPPA_PosResourceSetType_PR_aperiodic;
            asn1cCalloc(pos_resource_set_item->posresourceSetType.choice.aperiodic, pos_res_set_type_aperiodic);
            pos_res_set_type_aperiodic->sRSResourceTrigger =
                pos_res_set_item->posresourceSetType.choice.aperiodic.sRSResourceTrigger_List;
            break;
          case f1ap_pos_resource_set_type_pr_semi_persistent:
            pos_resource_set_item->posresourceSetType.present = NRPPA_PosResourceSetType_PR_semi_persistent;
            asn1cCalloc(pos_resource_set_item->posresourceSetType.choice.semi_persistent, pos_res_set_type_semi_persistent);
            pos_res_set_type_semi_persistent->possemi_persistentSet =
                pos_res_set_item->posresourceSetType.choice.semi_persistent.possemi_persistentSet;
            break;
          default:
            NRPPA_ERROR("Unknown posresourceSetType \n");
            break;
        }

        if (j < nb_possrsresourceset - 1) {
          pos_res_set_item++;
        }

      } // for(int j=0; j < nb_possrsresourceset; j++)*/
      //printf("\n AM TEST NRPPA PIR \n \n SRS configuration in PIR \n");
      //xer_fprint(stdout, &asn_DEF_NRPPA_SRSConfig, &item->activeULBWP.sRSConfig);
      //printf("\n AM TEST NRPPA PIR \n \n ActiveULBWP as in PIR \n");
      //xer_fprint(stdout, &asn_DEF_NRPPA_ActiveULBWP, &item->activeULBWP);

      //  Preparing Uplink Channel BW Per SCS List information IE of SRSCarrier_List
      int size_SpecificCarrier_list = carrier_list_item->uplink_channel_bw_per_scs_list.scs_specific_carrier_list_length;
      f1ap_scs_specific_carrier_t *scs_spe_carrier_item = carrier_list_item->uplink_channel_bw_per_scs_list.scs_specific_carrier;
      for (int b = 0; b < size_SpecificCarrier_list; b++) {
        asn1cSequenceAdd(item->uplinkChannelBW_PerSCS_List.list, NRPPA_SCS_SpecificCarrier_t, SpecificCarrier_item);
        SpecificCarrier_item->offsetToCarrier = scs_spe_carrier_item->offsetToCarrier;
        SpecificCarrier_item->subcarrierSpacing = scs_spe_carrier_item->subcarrierSpacing;
        SpecificCarrier_item->carrierBandwidth = scs_spe_carrier_item->carrierBandwidth;

        if (b < size_SpecificCarrier_list - 1) {
          scs_spe_carrier_item++;
        }

      } // for(int b=0; b < size_SpecificCarrier_list; b++)
      if (ci < nb_of_srscarrier - 1) {
          carrier_list_item++;
        }
    } // for (int ci = 0; ci < nb_of_srscarrier; ci++)
  }

  /*// IE 9.2.36 SFN Initialisation Time (Optional)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_PositioningInformationResponse_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_SFNInitialisationTime;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_PositioningInformationResponse_IEs__value_PR_SFNInitialisationTime;
    ie->value.choice.SFNInitialisationTime.buf = resp->sfn_initialisation_time.buf ;
    ie->value.choice.SFNInitialisationTime.size= resp->sfn_initialisation_time.size;
    ie->value.choice.SFNInitialisationTime.bits_unused=resp->sfn_initialisation_time.bits_unused;
  }*/

  /*//  IE 9.2.2 CriticalityDiagnostics (Optional)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_PositioningInformationResponse_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_CriticalityDiagnostics;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_PositioningInformationResponse_IEs__value_PR_CriticalityDiagnostics;
  }*/

  LOG_I(NRPPA, "Calling encoder for PositioningInformationResponse \n");
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, &tx_pdu);

  /* Encode NRPPA message */
  uint8_t *buffer = NULL;
  uint32_t length = 0;
  if (nrppa_gNB_encode_pdu(&tx_pdu, &buffer, &length) < 0) {
    NRPPA_ERROR("Failed to encode Uplink NRPPa PositioningInformationResponse\n");
    return -1;
  }

  /* Forward the NRPPA PDU to NGAP */
  nrppa_f1ap_info_t *info=&resp->nrppa_msg_info;
  if (info->gNB_ue_ngap_id > 0 && info->amf_ue_ngap_id > 0) //( 1) // TODO
  {
    LOG_D(NRPPA,
          "Sending UplinkUEAssociatedNRPPa (PositioningInformationResponse) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkUEAssociatedNRPPa(info->instance,
                                                info->gNB_ue_ngap_id,
                                                info->amf_ue_ngap_id,
                                                info->routing_id_buffer,
                                                info->routing_id_length,
                                                buffer,
                                                length);
    return length;
  } else if (info->gNB_ue_ngap_id == -1 && info->amf_ue_ngap_id == -1) //
  {
    LOG_D(
        NRPPA,
        "Sending UplinkNonUEAssociatedNRPPa (PositioningInformationResponse) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
        info->gNB_ue_ngap_id,
        info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa(info->instance,
                                                   info->routing_id_buffer,
                                                   info->routing_id_length,
                                                   buffer,
                                                   length);
    return length;
  } else {
    NRPPA_ERROR("Failed to find context for Uplink NonUE/UE Associated NRPPa PositioningInformationResponse\n");
    return -1;
  }
}

int nrppa_gNB_PositioningInformationFailure(instance_t instance, MessageDef *msg_p)
{
  f1ap_positioning_information_failure_t *failure_msg = &F1AP_POSITIONING_INFORMATION_FAILURE(msg_p);
  LOG_I(NRPPA,
        "Received PositioningInformationFailure info from RRC  gNB_CU_ue_id=%d, gNB_DU_ue_id=%d  rnti= %04x\n",
        failure_msg->gNB_CU_ue_id,
        failure_msg->gNB_DU_ue_id,
        failure_msg->nrppa_msg_info.ue_rnti);

  // Prepare NRPPA Position Information failure
  NRPPA_NRPPA_PDU_t tx_pdu ={0};

  // IE: 9.2.3 Message Type unsuccessfulOutcome PositioningInformationFaliure (M)
  memset(&tx_pdu, 0, sizeof(tx_pdu));
  tx_pdu.present = NRPPA_NRPPA_PDU_PR_unsuccessfulOutcome;
  asn1cCalloc(tx_pdu.choice.unsuccessfulOutcome, head);
  head->procedureCode = NRPPA_ProcedureCode_id_positioningInformationExchange;
  head->criticality = NRPPA_Criticality_reject;
  head->value.present = NRPPA_UnsuccessfulOutcome__value_PR_PositioningInformationFailure;

  // IE 9.2.4 nrppatransactionID  (M)
  head->nrppatransactionID = failure_msg->nrppa_msg_info.nrppa_transaction_id;
  NRPPA_PositioningInformationFailure_t *out = &head->value.choice.PositioningInformationFailure;

  // TODO IE 9.2.1 Cause (M)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_PositioningInformationFailure_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_Cause;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_PositioningInformationFailure_IEs__value_PR_Cause;
    switch (failure_msg->cause.present) {
      case f1ap_cause_nothing:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_NOTHING;
        break;
      case f1ap_cause_radio_network:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_radioNetwork;
        ie->value.choice.Cause.choice.radioNetwork = failure_msg->cause.choice.radioNetwork;
        break;
      // case f1ap_cause_transport:
      // ie->value.choice.Cause.present = NRPPA_Cause_PR_transport;
      // ie->value.choice.Cause.choice.transport = 0;
      // break; // IE not in nrppa specification
      case f1ap_cause_protocol:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_protocol;
        ie->value.choice.Cause.choice.protocol = failure_msg->cause.choice.protocol;
        break;
      case f1ap_cause_misc:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_misc;
        ie->value.choice.Cause.choice.misc = failure_msg->cause.choice.misc;
        break;
      default:
        NRPPA_ERROR("Unknown PositioningInformationFailure Cause\n");
        break;
    }
  }

  /*//  TODO IE 9.2.2 CriticalityDiagnostics (O)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_PositioningInformationFailure_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_CriticalityDiagnostics;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_PositioningInformationFailure_IEs__value_PR_CriticalityDiagnostics;
  }*/

  LOG_I(NRPPA, "Calling encoder for PositioningInformationFailure \n");
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, &tx_pdu);

  /* Encode NRPPA message */
  uint8_t *buffer = NULL;
  uint32_t length = 0;
  if (nrppa_gNB_encode_pdu(&tx_pdu, &buffer, &length) < 0) {
    NRPPA_ERROR("Failed to encode Uplink NRPPa PositioningInformationFailure \n");
    return -1;
  }

  /* Forward the NRPPA PDU to NGAP */
  nrppa_f1ap_info_t *info=&failure_msg->nrppa_msg_info;
  if (info->gNB_ue_ngap_id > 0 && info->amf_ue_ngap_id > 0) {
    LOG_D(NRPPA,
          "Sending UplinkUEAssociatedNRPPa (PositioningInformationFailure) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkUEAssociatedNRPPa(info->instance,
                                                info->gNB_ue_ngap_id,
                                                info->amf_ue_ngap_id,
                                                info->routing_id_buffer,
                                                info->routing_id_length,
                                                buffer,
                                                length);
    return length;
  } else if (info->gNB_ue_ngap_id == -1 && info->amf_ue_ngap_id == -1) {
    LOG_D(
        NRPPA,
        "Sending UplinkNonUEAssociatedNRPPa (PositioningInformationFailure) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
        info->gNB_ue_ngap_id,
        info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa(info->instance,
                                                   info->routing_id_buffer,
                                                   info->routing_id_length,
                                                   buffer,
                                                   length);
    return length;
  } else {
    NRPPA_ERROR("Failed to find context for Uplink NonUE/UE Associated NRPPa PositioningInformationFailure\n");
    return -1;
  }
}

int nrppa_gNB_PositioningInformationUpdate(instance_t instance, MessageDef *msg_p)
{
  f1ap_positioning_information_update_t *update_msg = &F1AP_POSITIONING_INFORMATION_UPDATE(msg_p);
  LOG_I(NRPPA,
        "Received PositioningInformationUpdate from RRC gNB_CU_ue_id=%d, gNB_DU_ue_id=%d  rnti= %04x\n",
        update_msg->gNB_CU_ue_id,
        update_msg->gNB_DU_ue_id,
        update_msg->nrppa_msg_info.ue_rnti);

  // Prepare NRPPA Position Information Update
  NRPPA_NRPPA_PDU_t pdu ={0};

  // Prepare the NRPPA message to encode for initiating message PositioningInformationUpdate

  // IE: 9.2.3 Message Type (M)
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = NRPPA_NRPPA_PDU_PR_initiatingMessage;
  asn1cCalloc(pdu.choice.initiatingMessage, head);
  head->procedureCode = NRPPA_ProcedureCode_id_positioningInformationUpdate;
  head->criticality = NRPPA_Criticality_ignore;
  head->value.present = NRPPA_InitiatingMessage__value_PR_PositioningInformationUpdate;

  // IE 9.2.4 nrppatransactionID  (M)
  head->nrppatransactionID = update_msg->nrppa_msg_info.nrppa_transaction_id;

  // NRPPA_PositioningInformationUpdate_t *out = &head->value.choice.PositioningInformationUpdate;

  /*// IE 9.2.28 SRS Configuration (Optional)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_PositioningInformationUpdate_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_SRSConfiguration;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_PositioningInformationUpdate_IEs__value_PR_SRSConfiguration;
    // TO add refer to PositioningInformationResponse
  } // IE 9.2.28 SRS Configuration*/

  /*// IE 9.2.36 SFN Initialisation Time (Optional)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_PositioningInformationUpdate_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_SFNInitialisationTime;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_PositioningInformationUpdate_IEs__value_PR_SFNInitialisationTime;
    ie->value.choice.SFNInitialisationTime.buf = update_msg->sfn_initialisation_time.buf ;
    ie->value.choice.SFNInitialisationTime.size= update_msg->sfn_initialisation_time.size;
    ie->value.choice.SFNInitialisationTime.bits_unused=update_msg->sfn_initialisation_time.bits_unused;
  }*/

  LOG_I(NRPPA, "Calling encoder for PositioningInformationUpdate \n");
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, &pdu);

  /* Encode NRPPA message */
  uint8_t *buffer = NULL;
  uint32_t length = 0;
  if (nrppa_gNB_encode_pdu(&pdu, &buffer, &length) < 0) {
    NRPPA_ERROR("Failed to encode Uplink NRPPa PositioningInformationUpdate\n");
    /* Encode procedure has failed... */
    return -1;
  }

    /* Forward the NRPPA PDU to NGAP */
  nrppa_f1ap_info_t *info=&update_msg->nrppa_msg_info;
  if (info->gNB_ue_ngap_id > 0 && info->amf_ue_ngap_id > 0) {
    LOG_D(NRPPA,
          "Sending UplinkUEAssociatedNRPPa (PositioningInformationUpdate) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkUEAssociatedNRPPa(info->instance,
                                                info->gNB_ue_ngap_id,
                                                info->amf_ue_ngap_id,
                                                info->routing_id_buffer,
                                                info->routing_id_length,
                                                buffer,
                                                length); // tx_nrppa_pdu=buffer, nrppa_pdu_length=length
    return length;
  } else if (info->gNB_ue_ngap_id == -1 && info->amf_ue_ngap_id == -1) //
  {
    LOG_D(NRPPA,
          "Sending UplinkNonUEAssociatedNRPPa (PositioningInformationUpdate) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa(info->instance,
                                                   info->routing_id_buffer,
                                                   info->routing_id_length,
                                                   buffer,
                                                   length);
    return length;
  } else {
    NRPPA_ERROR("Failed to find context for Uplink NonUE/UE Associated NRPPa PositioningInformationUpdate\n");
    return -1;
  }
}

// adeel TODO fill F1AP msg for rrc
int nrppa_gNB_PositioningActivationResponse(instance_t instance, MessageDef *msg_p)
{
  f1ap_positioning_activation_resp_t *resp = &F1AP_POSITIONING_ACTIVATION_RESP(msg_p);
  LOG_I(NRPPA,
        "Received PositioningActivationResponse info from RRC  gNB_CU_ue_id=%d, gNB_DU_ue_id=%d  rnti= %04x\n",
        resp->gNB_CU_ue_id,
        resp->gNB_DU_ue_id,
        resp->nrppa_msg_info.ue_rnti);

  // Prepare NRPPA Positioning  Activation Response
  NRPPA_NRPPA_PDU_t pdu ={0};

  /* Prepare the NRPPA message to encode for successfulOutcome PositioningActivationResponse */

  // IE: 9.2.3 Message Type successfulOutcome (M)
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = NRPPA_NRPPA_PDU_PR_successfulOutcome;
  asn1cCalloc(pdu.choice.successfulOutcome, head);
  head->procedureCode = NRPPA_ProcedureCode_id_positioningActivation;
  head->criticality = NRPPA_Criticality_reject;
  head->value.present = NRPPA_SuccessfulOutcome__value_PR_PositioningActivationResponse;

  // IE 9.2.4 nrppatransactionID  /* mandatory */
  head->nrppatransactionID = resp->nrppa_msg_info.nrppa_transaction_id;

  NRPPA_PositioningActivationResponse_t *out = &head->value.choice.PositioningActivationResponse;

  /*//  TODO IE 9.2.2 CriticalityDiagnostics (O)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_PositioningActivationResponseIEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_CriticalityDiagnostics;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_PositioningActivationResponseIEs__value_PR_CriticalityDiagnostics;
  }*/
  // IE  SystemFrameNumber (O)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_PositioningActivationResponseIEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_SystemFrameNumber;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_PositioningActivationResponseIEs__value_PR_SystemFrameNumber;
    ie->value.choice.SystemFrameNumber = resp->system_frame_number;
  }
  //  IE  SlotNumber (O)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_PositioningActivationResponseIEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_SlotNumber;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_PositioningActivationResponseIEs__value_PR_SlotNumber;
    ie->value.choice.SlotNumber = resp->slot_number;
  }

  LOG_I(NRPPA, "Calling encoder for PositioningActivationResponse \n");
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, &pdu);
  /* Encode NRPPA message */
  uint8_t *buffer = NULL;
  uint32_t length = 0;
  if (nrppa_gNB_encode_pdu(&pdu, &buffer, &length) < 0) {
    NRPPA_ERROR("Failed to encode Uplink NRPPa PositioningActivationResponse\n");
    return -1;
  }

  /* Forward the NRPPA PDU to NGAP */
  nrppa_f1ap_info_t *info=&resp->nrppa_msg_info;
  if (info->gNB_ue_ngap_id > 0 && info->amf_ue_ngap_id > 0) {
    LOG_D(NRPPA,
          "Sending UplinkUEAssociatedNRPPa (PositioningActivationResponse) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkUEAssociatedNRPPa(info->instance,
                                                info->gNB_ue_ngap_id,
                                                info->amf_ue_ngap_id,
                                                info->routing_id_buffer,
                                                info->routing_id_length,
                                                buffer,
                                                length);
    return length;
  } else if (info->gNB_ue_ngap_id == -1 && info->amf_ue_ngap_id == -1) {
    LOG_D(
        NRPPA,
        "Sending UplinkNonUEAssociatedNRPPa (PositioningActivationResponse) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
        info->gNB_ue_ngap_id,
        info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa(info->instance,
                                                   info->routing_id_buffer,
                                                   info->routing_id_length,
                                                   buffer,
                                                   length);
    return length;
  } else {
    NRPPA_ERROR("Failed to find context for Uplink NonUE/UE Associated NRPPa PositioningActivationResponse\n");
    return -1;
  }
}

// adeel TODO fill F1AP msg for rrc
int nrppa_gNB_PositioningActivationFailure(instance_t instance, MessageDef *msg_p)
{
  f1ap_positioning_activation_failure_t *failure_msg = &F1AP_POSITIONING_ACTIVATION_FAILURE(msg_p);
  LOG_I(NRPPA,
        "Received PositioningActivationFailure info from RRC  gNB_CU_ue_id=%d, gNB_DU_ue_id=%d  rnti= %04x\n",
        failure_msg->gNB_CU_ue_id,
        failure_msg->gNB_DU_ue_id,
        failure_msg->nrppa_msg_info.ue_rnti);

  // Prepare NRPPA Positioning Activation Failure

  NRPPA_NRPPA_PDU_t pdu ={0};

  /* Prepare the NRPPA message to encode for unsuccessfulOutcome PositioningActivationFailure */
  // IE: 9.2.3 Message Type unsuccessfulOutcome PositioningActivationFailure /* mandatory */
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = NRPPA_NRPPA_PDU_PR_unsuccessfulOutcome;
  asn1cCalloc(pdu.choice.unsuccessfulOutcome, head);
  head->procedureCode = NRPPA_ProcedureCode_id_positioningActivation;
  head->criticality = NRPPA_Criticality_reject;
  head->value.present = NRPPA_UnsuccessfulOutcome__value_PR_PositioningActivationFailure;

  // IE 9.2.4 nrppatransactionID  /* mandatory */
  head->nrppatransactionID = failure_msg->nrppa_msg_info.nrppa_transaction_id;

  NRPPA_PositioningActivationFailure_t *out = &head->value.choice.PositioningActivationFailure;

  // TODO IE 9.2.1 Cause (M)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_PositioningActivationFailureIEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_Cause;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_PositioningActivationFailureIEs__value_PR_Cause;
    switch (failure_msg->cause.present) {
      case f1ap_cause_nothing:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_NOTHING;
        break;
      case f1ap_cause_radio_network:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_radioNetwork;
        ie->value.choice.Cause.choice.radioNetwork = failure_msg->cause.choice.radioNetwork;
        break;
      // case f1ap_cause_transport:
      // ie->value.choice.Cause.present = NRPPA_Cause_PR_transport;
      // ie->value.choice.Cause.choice.transport = 0;
      // break; // IE not in nrppa specification
      case f1ap_cause_protocol:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_protocol;
        ie->value.choice.Cause.choice.protocol = failure_msg->cause.choice.protocol;
        break;
      case f1ap_cause_misc:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_misc;
        ie->value.choice.Cause.choice.misc = failure_msg->cause.choice.misc;
        break;
      default:
        NRPPA_ERROR("Unknown PositioningActivationFailure Cause\n");
        break;
    }
  }

  //  TODO IE 9.2.2 CriticalityDiagnostics (O)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_PositioningActivationFailureIEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_CriticalityDiagnostics;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_PositioningActivationFailureIEs__value_PR_CriticalityDiagnostics;
    // TODO Retreive CriticalityDiagnostics information and assign
    // ie->value.choice.CriticalityDiagnostics. = ;
    // ie->value.choice.CriticalityDiagnostics. = ;
  }

  /* Encode NRPPA message */
  uint8_t *buffer = NULL;
  uint32_t length = 0;
  if (nrppa_gNB_encode_pdu(&pdu, &buffer, &length) < 0) {
    NRPPA_ERROR("Failed to encode Uplink NRPPa PositioningActivationFailure \n");
    return -1;
  }

  /* Forward the NRPPA PDU to NGAP */
  nrppa_f1ap_info_t *info=&failure_msg->nrppa_msg_info;
  if (info->gNB_ue_ngap_id > 0 && info->amf_ue_ngap_id > 0) {
    LOG_D(NRPPA,
          "Sending UplinkUEAssociatedNRPPa (PositioningActivationFailure) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkUEAssociatedNRPPa(info->instance,
                                                info->gNB_ue_ngap_id,
                                                info->amf_ue_ngap_id,
                                                info->routing_id_buffer,
                                                info->routing_id_length,
                                                buffer,
                                                length); // tx_nrppa_pdu=buffer, nrppa_pdu_length=length
    return length;
  } else if (info->gNB_ue_ngap_id == -1 && info->amf_ue_ngap_id == -1) //
  {
    LOG_D(NRPPA,
          "Sending UplinkNonUEAssociatedNRPPa (PositioningActivationFailure) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa(info->instance,
                                                   info->routing_id_buffer,
                                                   info->routing_id_length,
                                                   buffer,
                                                   length);
    return length;
  } else {
    NRPPA_ERROR("Failed to find context for Uplink NonUE/UE Associated NRPPa PositioningActivationFailure\n");
    return -1;
  }
}
