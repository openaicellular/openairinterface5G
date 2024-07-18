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

/*! \file nrppa_gNB_TRP_information_transfer_procedures.c
 * \brief NRPPA gNB tasks related to TRP information transfer
 * \author Adeel Malik
 * \email adeel.malik@eurecom.fr
 * \date 2023
 * \version 1.0
 * @ingroup _nrppa
 */

#include "intertask_interface.h"

#include "nrppa_common.h"
#include "nrppa_gNB_TRP_information_transfer_procedures.h"
#include "nrppa_gNB_itti_messaging.h"
#include "nrppa_gNB_encoder.h"

/* TRPInformationExchange (Parent) procedure for  TRPInformationRequest, TRPInformationResponse, and TRPInformationFailure*/
int nrppa_gNB_handle_TRPInformationExchange(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu)
{
  LOG_I(NRPPA, "Processing Received TRPInformationRequest \n");
  DevAssert(pdu != NULL);
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, pdu);

  // Forward request to RRC
  MessageDef *msg = itti_alloc_new_message(TASK_RRC_GNB, 0, F1AP_TRP_INFORMATION_REQ);
  f1ap_trp_information_req_t *f1ap_req = &F1AP_TRP_INFORMATION_REQ(msg);
  f1ap_req->nrppa_msg_info.instance = nrppa_msg_info->instance;
  f1ap_req->nrppa_msg_info.gNB_ue_ngap_id = nrppa_msg_info->gNB_ue_ngap_id;
  f1ap_req->nrppa_msg_info.amf_ue_ngap_id = nrppa_msg_info->amf_ue_ngap_id;
  f1ap_req->nrppa_msg_info.routing_id_buffer = nrppa_msg_info->routing_id_buffer;
  f1ap_req->nrppa_msg_info.routing_id_length = nrppa_msg_info->routing_id_length;

  // Processing Received TRPInformationRequest
  NRPPA_TRPInformationRequest_t *container= NULL;
  NRPPA_TRPInformationRequest_IEs_t *ie= NULL;

  // IE 9.2.3 Message type (M)
  container = &pdu->choice.initiatingMessage->value.choice.TRPInformationRequest;
  // IE 9.2.4 nrppatransactionID (M)
  f1ap_req->transaction_id = pdu->choice.initiatingMessage->nrppatransactionID;
  f1ap_req->nrppa_msg_info.nrppa_transaction_id = pdu->choice.initiatingMessage->nrppatransactionID;

  // IE TRP List
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_TRPInformationRequest_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_TRPList, false);
  if (ie != NULL) {
    LOG_I(NRPPA, "Process TRPInformationRequest IE  TRP List");
    //    NRPPA_TRPList_t TRP_List = ie->value.choice.TRPList;  // TODO process this and fill f1ap message
  }

  // IE TRP Information Type List
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_TRPInformationRequest_IEs_t,
                              ie,
                              container,
                              NRPPA_ProtocolIE_ID_id_TRPInformationTypeList,
                              false);
  if (ie != NULL) {
    LOG_I(NRPPA, "Process TRPInformationRequest IE TRPInformationTypeList");
    // NRPPA_TRPInformationTypeList_t TRP_Info_Type_List= ie->value.choice.TRPInformationTypeList; // TODO process this and fill
    // f1ap message
  }

  LOG_I(NRPPA, "Forwarding to RRC TRPInformationRequest transaction_id=%d\n", f1ap_req->transaction_id);
  itti_send_msg_to_task(TASK_RRC_GNB, 0, msg);
  return 0;
}

int nrppa_gNB_TRPInformationResponse(instance_t instance, MessageDef *msg_p)
{
  f1ap_trp_information_resp_t *resp = &F1AP_TRP_INFORMATION_RESP(msg_p);
  LOG_I(NRPPA,
        "Received TRPInformationResponse info from RRC  transaction_id=%d,  rnti= %04x\n",
        resp->transaction_id,
        resp->nrppa_msg_info.ue_rnti);

  // Prepare NRPPA TRP Information transfer Response
  NRPPA_NRPPA_PDU_t pdu= {0};
  /* Prepare the NRPPA message to encode for successfulOutcome TRPInformationResponse */

  // IE: 9.2.3 Message Type successfulOutcome TRPInformationResponse (M)
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = NRPPA_NRPPA_PDU_PR_successfulOutcome;
  asn1cCalloc(pdu.choice.successfulOutcome, head);
  head->procedureCode = NRPPA_ProcedureCode_id_tRPInformationExchange;
  head->criticality = NRPPA_Criticality_reject;
  head->value.present = NRPPA_SuccessfulOutcome__value_PR_TRPInformationResponse;

  // IE 9.2.4 nrppatransactionID  (M)
  head->nrppatransactionID = resp->nrppa_msg_info.nrppa_transaction_id;
  NRPPA_TRPInformationResponse_t *out = &head->value.choice.TRPInformationResponse;

  // IE TRP Information List (M)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_TRPInformationResponse_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_TRPInformationList;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_TRPInformationResponse_IEs__value_PR_TRPInformationList;

    int nb_of_TRP = resp->trp_information_list.trp_information_list_length;
    f1ap_trp_information_item_t *trpInfItem= resp->trp_information_list.trp_information_item;
    for (int i = 0; i < nb_of_TRP; i++) {
      asn1cSequenceAdd(ie->value.choice.TRPInformationList.list, TRPInformationList__Member, item);
      item->tRP_ID = trpInfItem->tRPInformation.tRPID;

      // Preparing tRPInformation IE of NRPPA_TRPInformationList__Member
      int nb_tRPInfoTypes = trpInfItem->tRPInformation.tRPInformationTypeResponseList.trp_information_type_response_list_length; 
      f1ap_trp_information_type_response_item_t *resItem=trpInfItem->tRPInformation.tRPInformationTypeResponseList.trp_information_type_response_item;
      for (int k = 0; k < nb_tRPInfoTypes; k++) // Preparing NRPPA_TRPInformation_t a list of  TRPInformation_item
      {
        asn1cSequenceAdd(item->tRPInformation.list, NRPPA_TRPInformationItem_t, trpinfo_item);
        switch (resItem->present){
        case f1ap_trp_information_type_response_item_pr_NOTHING:
          trpinfo_item->present= NRPPA_TRPInformationItem_PR_NOTHING;
          break;
        case f1ap_trp_information_type_response_item_pr_nG_RAN_CGI:
          trpinfo_item->present= NRPPA_TRPInformationItem_PR_nG_RAN_CGI;
          // nR_CellID
          asn1cCalloc(trpinfo_item->choice.nG_RAN_CGI, nG_RAN_CGI);
          nG_RAN_CGI->nG_RANcell.present = NRPPA_NG_RANCell_PR_nR_CellID;
          nG_RAN_CGI->nG_RANcell.choice.nR_CellID.bits_unused=resItem->choice.nG_RAN_CGI.nRCellIdentity.bits_unused;
          //nG_RAN_CGI->nG_RANcell.choice.nR_CellID.buf=resItem->choice.nG_RAN_CGI.nRCellIdentity.buf;
          nG_RAN_CGI->nG_RANcell.choice.nR_CellID.buf=malloc(resItem->choice.nG_RAN_CGI.nRCellIdentity.size);
          memcpy(nG_RAN_CGI->nG_RANcell.choice.nR_CellID.buf,resItem->choice.nG_RAN_CGI.nRCellIdentity.buf,resItem->choice.nG_RAN_CGI.nRCellIdentity.size);
          nG_RAN_CGI->nG_RANcell.choice.nR_CellID.size=resItem->choice.nG_RAN_CGI.nRCellIdentity.size;
          // pLMN_Identity
          //nG_RAN_CGI->pLMN_Identity.buf = resItem->choice.nG_RAN_CGI.pLMN_Identity.buf;
          nG_RAN_CGI->pLMN_Identity.buf = malloc(resItem->choice.nG_RAN_CGI.pLMN_Identity.size);
          memcpy(nG_RAN_CGI->pLMN_Identity.buf,resItem->choice.nG_RAN_CGI.pLMN_Identity.buf,resItem->choice.nG_RAN_CGI.pLMN_Identity.size);
          nG_RAN_CGI->pLMN_Identity.size = resItem->choice.nG_RAN_CGI.pLMN_Identity.size;          
          break;
        case f1ap_trp_information_type_response_item_pr_geographicalCoordinates:
          trpinfo_item->present= NRPPA_TRPInformationItem_PR_geographicalCoordinates;
          asn1cCalloc(trpinfo_item->choice.geographicalCoordinates, geoCord);
          f1ap_trp_position_definition_type_t *f1_trpPosDef= &resItem->choice.geographicalCoordinates.tRPPositionDefinitionType;
          NRPPA_TRPPositionDefinitionType_t *nrppa_trpPosDef= &geoCord->tRPPositionDefinitionType;
          switch (f1_trpPosDef->present){
          case f1ap_trp_position_definition_type_pr_NOTHING:
            nrppa_trpPosDef->present = NRPPA_TRPPositionDefinitionType_PR_NOTHING;
            break;
          case f1ap_trp_position_definition_type_pr_direct:
            nrppa_trpPosDef->present = NRPPA_TRPPositionDefinitionType_PR_direct;
            NRPPA_ERROR(" TODO at RRC/F1AP TRP Position Definition Type Direct\n");
            break;
          case f1ap_trp_position_definition_type_pr_referenced:
            nrppa_trpPosDef->present = NRPPA_TRPPositionDefinitionType_PR_referenced;
            asn1cCalloc(nrppa_trpPosDef->choice.referenced, referenced);
            // IE referencePoint
            switch (f1_trpPosDef->choice.referenced.referencePoint.present){
            case f1ap_reference_point_pr_NOTHING:
              referenced->referencePoint.present=NRPPA_ReferencePoint_PR_NOTHING;
              break;
            case f1ap_reference_point_pr_coordinateID:
              referenced->referencePoint.present=NRPPA_ReferencePoint_PR_relativeCoordinateID;
              referenced->referencePoint.choice.relativeCoordinateID=f1_trpPosDef->choice.referenced.referencePoint.choice.coordinateID;
              break;
            case f1ap_reference_point_pr_referencePointCoordinate:
              NRPPA_ERROR(" TODO at RRC/F1AP TRP referencePointCoordinate\n");
              break;
            case f1ap_reference_point_pr_referencePointCoordinateHA:
              NRPPA_ERROR(" TODO at RRC/F1AP TRP referencePointCoordinateHA\n");
              break;
            default:
              NRPPA_ERROR("Unknown TRP referencePoint\n");
              break;
            }

            // IE referencePointType
            switch (f1_trpPosDef->choice.referenced.referencePointType.present){
            case f1ap_trp_reference_point_type_pr_NOTHING:
              referenced->referencePointType.present=NRPPA_TRPReferencePointType_PR_NOTHING;
              break;
            case f1ap_trp_reference_point_type_pr_tRPPositionRelativeGeodetic:
              referenced->referencePointType.present=NRPPA_TRPReferencePointType_PR_tRPPositionRelativeGeodetic;
              NRPPA_ERROR(" TODO F1AP not done TRP Reference Point Type\n");
              break;
            case f1ap_trp_reference_point_type_pr_tRPPositionRelativeCartesian:
              referenced->referencePointType.present=NRPPA_TRPReferencePointType_PR_tRPPositionRelativeCartesian; 
              asn1cCalloc(referenced->referencePointType.choice.tRPPositionRelativeCartesian, RelCart);
              f1ap_relative_cartesian_location_t *f1_RelCart=&f1_trpPosDef->choice.referenced.referencePointType.choice.tRPPositionRelativeCartesian;
              RelCart->xvalue=f1_RelCart->xvalue;
              RelCart->xYZunit=f1_RelCart->xYZunit;
              RelCart->yvalue=f1_RelCart->yvalue;
              RelCart->zvalue=f1_RelCart->zvalue;
              RelCart->locationUncertainty.horizontalConfidence=f1_RelCart->locationUncertainty.horizontalConfidence;
              RelCart->locationUncertainty.horizontalUncertainty=f1_RelCart->locationUncertainty.horizontalUncertainty;
              RelCart->locationUncertainty.verticalConfidence=f1_RelCart->locationUncertainty.verticalConfidence;
              RelCart->locationUncertainty.verticalUncertainty=f1_RelCart->locationUncertainty.verticalUncertainty;
              break;
            default:
              NRPPA_ERROR(" Unknown TRP Reference Point Type\n");
              break;
            }
            break;
          default:
            NRPPA_ERROR("Unknown TRP Position Definition Type\n");
            break;
          }
          break;

        /*TODO following options are not filled at RRC level  
        case f1ap_trp_information_type_response_item_pr_pCI_NR:
          trpinfo_item->present= NRPPA_TRPInformationItem_PR_pCI_NR;
          trpinfo_item->choice.pCI_NR = resItem->choice.pCI_NR; 
          break;
        case f1ap_trp_information_type_response_item_pr_nRARFCN:
          // Not present in NRPPa
          //trpinfo_item->present= NRPPA_TRPInformationItem_PR_nRARFCN;
          //trpinfo_item->choice.nRARFCN = resItem->choice.nRARFCN;
          break;
        case f1ap_trp_information_type_response_item_pr_pRSConfiguration:
          trpinfo_item->present= NRPPA_TRPInformationItem_PR_pRSConfiguration;
          trpinfo_item->choice.pRSConfiguration = resItem->choice.pRSConfiguration;
          break;
        case f1ap_trp_information_type_response_item_pr_sSBinformation:
          trpinfo_item->present= NRPPA_TRPInformationItem_PR_sSBinformation;
          trpinfo_item->choice.sSBinformation = resItem->choice.sSBinformation;
          break;
        case f1ap_trp_information_type_response_item_pr_sFNInitialisationTime:
          trpinfo_item->present= NRPPA_TRPInformationItem_PR_sFNInitialisationTime;
          trpinfo_item->choice.sFNInitialisationTime = resItem->choice.sFNInitialisationTime;
          break;
        case f1ap_trp_information_type_response_item_pr_spatialDirectionInformation:
          trpinfo_item->present= NRPPA_TRPInformationItem_PR_spatialDirectionInformation;
          trpinfo_item->choice.spatialDirectionInformation = resItem->choice.spatialDirectionInformation;
          break;
          */
        default:
          NRPPA_ERROR("Unknown TRP Information Item\n");
          break;
        }
        if (k < nb_tRPInfoTypes - 1) {
          resItem++;
        }
      } // for(int k=0; k < nb_tRPInfoTypes; k++)
      if (i < nb_of_TRP - 1) {
          trpInfItem++;
        }
    } // for (int i = 0; i < nb_of_TRP; i++)

  } // IE Information List

  /*//  TODO IE 9.2.2 CriticalityDiagnostics (O)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_TRPInformationResponse_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_CriticalityDiagnostics;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_TRPInformationResponse_IEs__value_PR_CriticalityDiagnostics;
  }*/

  LOG_I(NRPPA, "Calling encoder for TRPInformationResponse \n");
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, &pdu);

  /* Encode NRPPA message */
  uint8_t *buffer = NULL;
  uint32_t length = 0;
  if (nrppa_gNB_encode_pdu(&pdu, &buffer, &length) < 0) {
    NRPPA_ERROR("Failed to encode Uplink NRPPa TRPInformationResponse\n");
    /* Encode procedure has failed... */
    return -1;
  }

  /* Forward the NRPPA PDU to NGAP */
  nrppa_f1ap_info_t *info=&resp->nrppa_msg_info;
  if (info->gNB_ue_ngap_id > 0 && info->amf_ue_ngap_id > 0) //( 1) // TODO
  {
    LOG_D(NRPPA,
          "Sending UplinkUEAssociatedNRPPa (TRPInformationResponse) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
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
    LOG_D(NRPPA,
          "Sending UplinkNonUEAssociatedNRPPa (TRPInformationResponse) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa(info->instance,
                                                   info->routing_id_buffer,
                                                   info->routing_id_length,
                                                   buffer,
                                                   length);
    return length;
  } else {
    NRPPA_ERROR("Failed to find context for Uplink NonUE/UE Associated NRPPa TRPInformationResponse\n");
    return -1;
  }
}

// TODO fill F1AP msg for rrc
int nrppa_gNB_TRPInformationFailure(instance_t instance, MessageDef *msg_p)
{
  f1ap_trp_information_failure_t *failure_msg = &F1AP_TRP_INFORMATION_FAILURE(msg_p);
  LOG_I(NRPPA,
        "Received TrpInformationFailure info from RRC  transaction_id=%d,  rnti= %04x\n",
        failure_msg->transaction_id,
        failure_msg->nrppa_msg_info.ue_rnti);

  // Prepare NRPPA Position Information failure
  NRPPA_NRPPA_PDU_t pdu ={0};
  /* Prepare the NRPPA message to encode for unsuccessfulOutcome TRPInformationFailure */

  // IE: 9.2.3 Message Type unsuccessfulOutcome TRPInformationFaliure (M)
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = NRPPA_NRPPA_PDU_PR_unsuccessfulOutcome;
  asn1cCalloc(pdu.choice.unsuccessfulOutcome, head);
  head->procedureCode = NRPPA_ProcedureCode_id_tRPInformationExchange;
  head->criticality = NRPPA_Criticality_reject;
  head->value.present = NRPPA_UnsuccessfulOutcome__value_PR_TRPInformationFailure;

  // IE 9.2.4 nrppatransactionID  (M)
  head->nrppatransactionID = failure_msg->nrppa_msg_info.nrppa_transaction_id;

  NRPPA_TRPInformationFailure_t *out = &head->value.choice.TRPInformationFailure;
  // TODO IE 9.2.1 Cause (M)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_TRPInformationFailure_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_Cause;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_TRPInformationFailure_IEs__value_PR_Cause;
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
        NRPPA_ERROR("Unknown TrpInformationFailure Cause\n");
        break;
    }
  }

  /*//  TODO IE 9.2.2 CriticalityDiagnostics (O)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_TRPInformationFailure_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_CriticalityDiagnostics;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_TRPInformationFailure_IEs__value_PR_CriticalityDiagnostics;
  }*/

  /* Encode NRPPA message */
  uint8_t *buffer = NULL;
  uint32_t length = 0;
  if (nrppa_gNB_encode_pdu(&pdu, &buffer, &length) < 0) {
    NRPPA_ERROR("Failed to encode Uplink NRPPa TRPInformationFailure \n");
    return -1;
  }

  /* Forward the NRPPA PDU to NGAP */
  nrppa_f1ap_info_t *info=&failure_msg->nrppa_msg_info;
  if (info->gNB_ue_ngap_id > 0 && info->amf_ue_ngap_id > 0) {
    LOG_D(NRPPA,
          "Sending UplinkUEAssociatedNRPPa (TRPInformationFailure) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
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
          "Sending UplinkNonUEAssociatedNRPPa (TRPInformationFailure) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
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
