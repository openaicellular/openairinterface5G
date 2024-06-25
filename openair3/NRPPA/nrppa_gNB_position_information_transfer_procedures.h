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

/*! \file nrppa_gNB_position_information_transfer_procedures.h
 * \brief NRPPA gNB tasks related to position information transfer
 * \author Adeel Malik
 * \email adeel.malik@eurecom.fr
 *\date 2023
 * \version 1.0
 * @ingroup _nrppa
 */

#ifndef NRPPA_GNB_POSITION_INFORMATION_TRANSFER_PROCEDURES_H_
#define NRPPA_GNB_POSITION_INFORMATION_TRANSFER_PROCEDURES_H_

// DOWNLINK
// PositioningInformationExchange (Parent) procedure for  PositioningInformationRequest,
// PositioningInformationResponse, and PositioningInformationFailure
int nrppa_gNB_handle_PositioningInformationExchange(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu);
// PositioningActivation (Parent) procedure for  PositioningActivationRequest,
// PositioningActivationResponse, and PositioningActivationFailure
int nrppa_gNB_handle_PositioningActivation(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu);
int nrppa_gNB_handle_PositioningDeactivation(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu);

// UPLINK
int nrppa_gNB_PositioningInformationFailure(instance_t instance, MessageDef *msg_p);
int nrppa_gNB_PositioningInformationResponse(instance_t instance, MessageDef *msg_p);
int nrppa_gNB_PositioningActivationFailure(instance_t instance, MessageDef *msg_p);
int nrppa_gNB_PositioningActivationResponse(instance_t instance, MessageDef *msg_p);
int nrppa_gNB_PositioningInformationUpdate(instance_t instance, MessageDef *msg_p);

#endif /* NRPPA_GNB_POSITION_INFORMATION_TRANSFER_PROCEDURES_H_ */
