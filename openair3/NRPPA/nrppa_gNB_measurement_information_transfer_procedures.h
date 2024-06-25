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

/*! \file nrppa_gNB_measurement_information_transfer_procedures.h
 * \brief NRPPA gNB tasks related to measurement information transfer
 * \author Adeel Malik
 * \email adeel.malik@eurecom.fr
 *\date 2023
 * \version 1.0
 * @ingroup _nrppa
 */

#ifndef NRPPA_GNB_MEASUREMENT_INFORMATION_TRANSFER_PROCEDURES_H_
#define NRPPA_GNB_MEASUREMENT_INFORMATION_TRANSFER_PROCEDURES_H_

// DOWNLINK
// Measurement (Parent) procedure for  MeasurementRequest, MeasurementResponse, and MeasurementFailure
int nrppa_gNB_handle_Measurement(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu);
int nrppa_gNB_handle_MeasurementUpdate(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu);
int nrppa_gNB_handle_MeasurementAbort(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu);

// UPLINK
int nrppa_gNB_MeasurementFailure(instance_t instance, MessageDef *msg_p);
int nrppa_gNB_MeasurementResponse(instance_t instance, MessageDef *msg_p);
int nrppa_gNB_MeasurementReport(instance_t instance, MessageDef *msg_p);
int nrppa_gNB_MeasurementFailureIndication(instance_t instance, MessageDef *msg_p);
#endif /* NGAP_GNB_MEASUREMENT_INFORMATION_TRANSFER_PROCEDURES_H_ */
