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

/*! \file nrppa_gNB_itti_messaging.c
 * \brief nrppa itti messages handlers for gNB
 * \author Adeel Malik
 * \email adeel.malik@eurecom.fr
 *\date 2023
 * \version 1.0
 * @ingroup _nrppa
 */

/* TODO */
#include "intertask_interface.h"
#include "nrppa_gNB_itti_messaging.h"
/* TODO */

void nrppa_gNB_itti_send_UplinkUEAssociatedNRPPa(instance_t instance,
                                                 uint32_t gNB_ue_ngap_id,
                                                 uint32_t amf_ue_ngap_id,
                                                 uint8_t *routingId_buffer,
                                                 uint32_t routingId_buffer_length,
                                                 uint8_t *nrppa_pdu,
                                                 uint32_t nrppa_pdu_length)
{
  LOG_I(NRPPA, "initiating nrppa_gNB_itti_send_UplinkUEAssociatedNRPPa \n");

  MessageDef *msg = itti_alloc_new_message(TASK_NRPPA, 0, NGAP_UPLINKUEASSOCIATEDNRPPA);
  ngap_UplinkUEAssociatedNRPPa_t *ULNRPPA = &NGAP_UPLINKUEASSOCIATEDNRPPA(msg);
  ULNRPPA->gNB_ue_ngap_id = gNB_ue_ngap_id;

  // Routing ID
  ULNRPPA->routing_id.buffer = malloc(sizeof(uint8_t) * routingId_buffer_length);
  memcpy(ULNRPPA->routing_id.buffer, routingId_buffer, routingId_buffer_length);
  ULNRPPA->routing_id.length = routingId_buffer_length;

  // NRPPA PDU
  ULNRPPA->nrppa_pdu.buffer = malloc(sizeof(uint8_t) * nrppa_pdu_length);
  memcpy(ULNRPPA->nrppa_pdu.buffer, nrppa_pdu, nrppa_pdu_length);
  ULNRPPA->nrppa_pdu.length = nrppa_pdu_length;
  itti_send_msg_to_task(TASK_NGAP, instance, msg);


  /*MessageDef *msg=itti_alloc_new_message_sized(TASK_NRPPA, 0, NGAP_UPLINKUEASSOCIATEDNRPPA,
       sizeof(ngap_UplinkUEAssociatedNRPPa_t)+ routingId_buffer_length + nrppa_pdu_length);

  ngap_UplinkUEAssociatedNRPPa_t *msgData = &NGAP_UPLINKUEASSOCIATEDNRPPA(msg);
  msgData->gNB_ue_ngap_id = gNB_ue_ngap_id;

  // Routing ID
  msgData->routing_id.buffer=(uint8_t *)(msgData+1);
  memcpy(msgData->routing_id.buffer, routingId_buffer, routingId_buffer_length);
  msgData->routing_id.length = routingId_buffer_length;

  // NRPPa PDU
  msgData->nrppa_pdu.buffer = msgData->routing_id.buffer+routingId_buffer_length;
  memcpy(msgData->nrppa_pdu.buffer, nrppa_pdu, nrppa_pdu_length);
  msgData->nrppa_pdu.length = nrppa_pdu_length;

  itti_send_msg_to_task(TASK_NGAP, instance, msg);*/
}

void nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa(instance_t instance,
                                                    uint8_t *routingId_buffer,
                                                    uint32_t routingId_buffer_length,
                                                    uint8_t *nrppa_pdu,
                                                    uint32_t nrppa_pdu_length)
{
  LOG_I(NRPPA, "initiating nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa \n");

  MessageDef *msg = itti_alloc_new_message(TASK_NRPPA, 0, NGAP_UPLINKNONUEASSOCIATEDNRPPA);
  ngap_UplinkNonUEAssociatedNRPPa_t *ULNRPPA= &NGAP_UPLINKNONUEASSOCIATEDNRPPA(msg);

  // Routing ID
  ULNRPPA->routing_id.buffer = malloc(sizeof(uint8_t) * routingId_buffer_length);
  memcpy(ULNRPPA->routing_id.buffer, routingId_buffer, routingId_buffer_length);
  ULNRPPA->routing_id.length = routingId_buffer_length;

  // NRPPa PDU
  ULNRPPA->nrppa_pdu.buffer = malloc(sizeof(uint8_t) * nrppa_pdu_length);
  memcpy(ULNRPPA->nrppa_pdu.buffer, nrppa_pdu, nrppa_pdu_length);
  ULNRPPA->nrppa_pdu.length = nrppa_pdu_length;

  itti_send_msg_to_task(TASK_NGAP, instance, msg);

// TODO try the below approach
/* MessageDef *msg=itti_alloc_new_message_sized(TASK_NRPPA, 0, NGAP_UPLINKNONUEASSOCIATEDNRPPA,
       sizeof(ngap_UplinkNonUEAssociatedNRPPa_t)+ routingId_buffer_length + nrppa_pdu_length);

  ngap_UplinkNonUEAssociatedNRPPa_t *msgData = &NGAP_UPLINKNONUEASSOCIATEDNRPPA(msg);

  // Routing ID
  msgData->routing_id.buffer=(uint8_t *)(msgData+1);
  memcpy(msgData->routing_id.buffer, routingId_buffer, routingId_buffer_length);
  msgData->routing_id.length = routingId_buffer_length;

  // NRPPa PDU
  msgData->nrppa_pdu.buffer = msgData->routing_id.buffer+routingId_buffer_length;
  memcpy(msgData->nrppa_pdu.buffer, nrppa_pdu, nrppa_pdu_length);
  msgData->nrppa_pdu.length = nrppa_pdu_length;

  itti_send_msg_to_task(TASK_NGAP, instance, msg);*/
}
