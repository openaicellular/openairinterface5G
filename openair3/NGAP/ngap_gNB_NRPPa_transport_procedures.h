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

/*! \file ngap_gNB_NRPPa_transport_procedures.h
 * \brief NGAP gNB NRPPa transport procedures
 * \author  Adeel Malik
 * \date 2023
 * \email: adeel.malik@eurecom.fr
 * \version 1.0
 * @ingroup _ngap
 */

#ifndef NGAP_GNB_NRPPA_PROCEDURES_H_
#define NGAP_GNB_NRPPA_PROCEDURES_H_

// UPLINK UE ASSOCIATED NRPPA TRANSPORT (9.2.9.2 of TS 38.413 Version 16.0.0.0 Release 16)
int ngap_gNB_UplinkUEAssociatedNRPPaTransport(instance_t instance, ngap_UplinkUEAssociatedNRPPa_t *ngap_UplinkUEAssociatedNRPPa_p);

// UPLINK NON UE ASSOCIATED NRPPA TRANSPORT (9.2.9.4 of TS 38.413 Version 16.0.0.0 Release 16)
int ngap_gNB_UplinkNonUEAssociatedNRPPaTransport(instance_t instance,
                                                 ngap_UplinkNonUEAssociatedNRPPa_t *ngap_UplinkNonUEAssociatedNRPPa_p);

// DOWNLINK UE ASSOCIATED NRPPA TRANSPORT (9.2.9.1 of TS 38.413 Version 16.0.0.0 Release 16)
int ngap_gNB_handle_DownlinkUEAssociatedNRPPaTransport(sctp_assoc_t assoc_id, uint32_t stream, NGAP_NGAP_PDU_t *pdu);
// sctp_assoc_t  instead of uint32_t check

// DOWNLINK NON UE ASSOCIATED NRPPA TRANSPORT (9.2.9.3 of TS 38.413 Version 16.0.0.0 Release 16)
int ngap_gNB_handle_DownlinkNonUEAssociatedNRPPaTransport(sctp_assoc_t assoc_id, uint32_t stream, NGAP_NGAP_PDU_t *pdu);

#endif /* NGAP_GNB_NRPPA_PROCEDURES_H_ */
