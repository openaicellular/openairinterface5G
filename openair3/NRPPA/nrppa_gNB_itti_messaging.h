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

/*! \file nrppa_gNB_itti_messaging.h
 * \brief nrppa itti messages handlers for gNB
 * \author Adeel Malik
 * \email adeel.malik@eurecom.fr
 *\date 2023
 * \version 1.0
 * @ingroup _nrppa
 */

#ifndef NRPPA_GNB_ITTI_MESSAGING_H_
#define NRPPA_GNB_ITTI_MESSAGING_H_

void nrppa_gNB_itti_send_UplinkUEAssociatedNRPPa(instance_t instance,
                                                 uint32_t gNB_ue_ngap_id,
                                                 uint32_t amf_ue_ngap_id,
                                                 uint8_t *routingId_buffer,
                                                 uint32_t routingId_buffer_length,
                                                 uint8_t *nrppa_pdu,
                                                 uint32_t nrppa_pdu_length);

void nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa(instance_t instance,
                                                    uint8_t *routingId_buffer,
                                                    uint32_t routingId_buffer_length,
                                                    uint8_t *nrppa_pdu,
                                                    uint32_t nrppa_pdu_length);

#endif /* NRPPA_GNB_ITTI_MESSAGING_H_ */
