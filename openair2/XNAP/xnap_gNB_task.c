/* Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
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

/*! \file XNAP/xnap_gNB_task.c
 * \brief XNAP tasks and functions definitions
 * \author Sreeshma Shiv
 * \date Aug 2023
 * \version 1.0
 * \email: sreeshmau@iisc.ac.in
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "intertask_interface.h"
#include <openair3/ocp-gtpu/gtp_itf.h>
#include "xnap_gNB_task.h"
#include "xnap_gNB_defs.h"
#include "xnap_gNB_management_procedures.h"
#include "xnap_gNB_handler.h"
#include "queue.h"
#include "assertions.h"
#include "conversions.h"
#include "xnap_gNB_generate_messages.h"
#include "gnb_config.h"
#include "xnap_ids.h"

RB_PROTOTYPE(xnap_gnb_tree, xnap_gNB_data_t, entry, xnap_gNB_compare_assoc_id);

static void xnap_gNB_handle_sctp_data_ind(instance_t instance, sctp_data_ind_t *sctp_data_ind);

static void xnap_gNB_handle_sctp_association_ind(instance_t instance, sctp_new_association_ind_t *sctp_new_association_ind);

// static void xnap_gNB_handle_register_gNB(instance_t instance, xnap_register_gnb_req_t *xnap_register_gNB);

static void xnap_gNB_send_sctp_assoc_req(instance_t instance_p, xnap_net_config_t *nc, int index);
static void xnap_gNB_handle_sctp_association_resp(instance_t instance, sctp_new_association_resp_t *sctp_new_association_resp);

static void xnap_gNB_handle_sctp_data_ind(instance_t instance, sctp_data_ind_t *sctp_data_ind)
{
  int result;
  DevAssert(sctp_data_ind != NULL);
  xnap_gNB_handle_message(instance,
                          sctp_data_ind->assoc_id,
                          sctp_data_ind->stream,
                          sctp_data_ind->buffer,
                          sctp_data_ind->buffer_length);
  result = itti_free(TASK_UNKNOWN, sctp_data_ind->buffer);
  AssertFatal(result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);
}

static void xnap_gNB_handle_sctp_association_resp(instance_t instance, sctp_new_association_resp_t *sctp_new_association_resp)
{
  xnap_gNB_instance_t *instance_xn = xnap_gNB_get_instance(instance); // managementproc;
  DevAssert(sctp_new_association_resp != NULL);
  DevAssert(instance_xn != NULL);
  /*Return if connection to gNB failed- to be modified if needed. (Exit on error in X2AP)*/
  if (sctp_new_association_resp->sctp_state == SCTP_STATE_UNREACHABLE) {
    LOG_E(XNAP,
          "association with gNB failed, is it running? If no, run it first. If yes, check IP addresses in your configuration "
          "file.\n");
    return;
  }
  if (sctp_new_association_resp->sctp_state != SCTP_STATE_ESTABLISHED) {
    LOG_W(XNAP,
          "Received unsuccessful result for SCTP association state (%u), assoc_id (%d), instance %ld, cnx_id %u \n",
          sctp_new_association_resp->sctp_state,
          sctp_new_association_resp->assoc_id,
          instance,
          sctp_new_association_resp->ulp_cnx_id);
    xnap_handle_xn_setup_message(instance,
                                 sctp_new_association_resp->assoc_id,
                                 sctp_new_association_resp->sctp_state == SCTP_STATE_SHUTDOWN);
    // sleep(3);
    // xnap_gNB_send_sctp_assoc_req(instance, &instance_xn->net_config, sctp_new_association_resp->ulp_cnx_id);
    return; // exit -1 for debugging
  }

  xnap_gNB_data_t *xnap_gnb_data_p = calloc(1, sizeof(*xnap_gnb_data_p));
  AssertFatal(xnap_gnb_data_p != NULL, "out of memory\n");
  xnap_gnb_data_p->cnx_id = sctp_new_association_resp->ulp_cnx_id;
  xnap_gnb_data_p->assoc_id = sctp_new_association_resp->assoc_id;
  xnap_gnb_data_p->state = XNAP_GNB_STATE_WAITING;
  xnap_gnb_data_p->in_streams = sctp_new_association_resp->in_streams;
  xnap_gnb_data_p->out_streams = sctp_new_association_resp->out_streams;
  xnap_gnb_data_p->nci = 12345678;
  xnap_insert_gnb(instance, xnap_gnb_data_p);
  xnap_dump_trees(instance);
  xnap_gNB_generate_xn_setup_request(sctp_new_association_resp->assoc_id, &instance_xn->setup_req);
}

int xnap_gNB_init_sctp(instance_t instance_p, xnap_net_config_t *nc, char *my_addr)
{

  DevAssert(nc != NULL);
  size_t addr_len = strlen(my_addr) + 1;
  LOG_I(E1AP, "**************XNAP_CUCP_SCTP_REQ(create socket) for %s len %ld\n", my_addr, addr_len);
  MessageDef *message = itti_alloc_new_message_sized(TASK_XNAP, 0, SCTP_INIT_MSG_MULTI_REQ, sizeof(sctp_init_t) + addr_len);
  sctp_init_t *sctp_init = &message->ittiMsg.sctp_init_multi;
  sctp_init->port = nc->gnb_port_for_XNC;
  sctp_init->ppid = XNAP_SCTP_PPID;
  char *addr_buf = (char *) (sctp_init + 1);
  sctp_init->bind_address = addr_buf;
  memcpy(addr_buf, my_addr, addr_len);
  return itti_send_msg_to_task(TASK_SCTP, instance_p, message);


}
static void xnap_gNB_send_sctp_assoc_req(instance_t instance, xnap_net_config_t *nc, int index)
{
  MessageDef *message = NULL;
  sctp_new_association_req_t *sctp_new_association_req = NULL;
  DevAssert(nc != NULL);
  message = itti_alloc_new_message(TASK_XNAP, 0, SCTP_NEW_ASSOCIATION_REQ);
  sctp_new_association_req = &message->ittiMsg.sctp_new_association_req;
  sctp_new_association_req->port = nc->gnb_port_for_XNC;
  sctp_new_association_req->ppid = XNAP_SCTP_PPID;
  sctp_new_association_req->in_streams = nc->sctp_streams.sctp_in_streams;
  sctp_new_association_req->out_streams = nc->sctp_streams.sctp_out_streams;

  memcpy(&sctp_new_association_req->remote_address,
         &nc->target_gnb_xn_ip_address[index],
         sizeof(nc->target_gnb_xn_ip_address[index]));
  memcpy(&sctp_new_association_req->local_address, &nc->gnb_xn_ip_address, sizeof(nc->gnb_xn_ip_address));
  sctp_new_association_req->ulp_cnx_id = index;
  itti_send_msg_to_task(TASK_SCTP, instance, message);
}

static void xnap_gNB_handle_sctp_init_msg_multi_cnf(instance_t instance, sctp_init_msg_multi_cnf_t *m)
{
  xnap_gNB_instance_t *instance_xn = xnap_gNB_get_instance(instance);
  DevAssert(m != NULL);
  DevAssert(instance_xn != NULL);
  // instance->multi_sd = m->multi_sd;

  /* Exit if CNF message reports failure.
   * Failure means multi_sd < 0.
   */
  if (m->multi_sd < 0) {
    LOG_E(XNAP, "Error: be sure to properly configure XN in your configuration file.\n");
    DevAssert(m->multi_sd >= 0);
  }

  /* Trying to connect to the provided list of gNB ip address */

  for (int index = 0; index < instance_xn->net_config.nb_xn; index++) {
    LOG_I(XNAP, "gNB[%ld] gNB id %lx index %d acting as an initiator (client)\n", instance, instance_xn->setup_req.gNB_id, index);
    instance_xn->xn_target_gnb_pending_nb++;
    xnap_gNB_send_sctp_assoc_req(instance, &instance_xn->net_config, index);
  }
}

static void xnap_gNB_handle_sctp_association_ind(instance_t instance, sctp_new_association_ind_t *sctp_new_association_ind)
{
  xnap_gNB_instance_t *instance_p = xnap_gNB_get_instance(instance);
  DevAssert(instance_p != NULL);
  xnap_gNB_data_t *xnap_gnb_data_p;
  DevAssert(sctp_new_association_ind != NULL);
  LOG_W(XNAP, "SCTP Association IND Received.\n");
  //xnap_dump_trees(instance);
  xnap_gnb_data_p = xnap_get_gNB(instance, sctp_new_association_ind->assoc_id);
  if (xnap_gnb_data_p == NULL) {
    LOG_W(XNAP, "xnap_gnb_data_p does not exist, creating new descriptor\n");
    /* TODO: Create new gNB descriptor-not yet associated? */
    xnap_gNB_data_t *xnap_gnb_data_p = calloc(1, sizeof(*xnap_gnb_data_p));
    AssertFatal(xnap_gnb_data_p != NULL, "out of memory\n");
    xnap_gnb_data_p->assoc_id = sctp_new_association_ind->assoc_id;
    xnap_gnb_data_p->nci = 12345678;
    xnap_gnb_data_p->state = XNAP_GNB_STATE_WAITING;
    xnap_gnb_data_p->in_streams = sctp_new_association_ind->in_streams;
    xnap_gnb_data_p->out_streams = sctp_new_association_ind->out_streams;
    //xnap_dump_trees(instance);
    xnap_insert_gnb(instance, xnap_gnb_data_p);
    xnap_dump_trees(instance);
  } else {
    xnap_gnb_data_p->in_streams = sctp_new_association_ind->in_streams;
    xnap_gnb_data_p->out_streams = sctp_new_association_ind->out_streams;
    LOG_W(XNAP, "Updated streams for assoc id: %d \n", sctp_new_association_ind->assoc_id);
  }
  xnap_dump_trees(instance);
}

static
void xnap_gNB_handle_handover_prep(instance_t instance,
                                  xnap_handover_req_t *xnap_handover_req)
{
  xnap_gNB_instance_t *instance_p;
  xnap_id_manager     *id_manager;
  //int                 ue_id;
  sctp_assoc_t assoc_id;
  
  instance_p = xnap_gNB_get_instance(instance);
  DevAssert(instance_p != NULL);
  int target_nci = xnap_handover_req->target_cgi.cgi;
  assoc_id = xnap_gNB_get_assoc_id(instance_p,target_nci);

  /* allocate xnap ID */
  id_manager = &instance_p->id_manager;
  xnap_id_manager_init(id_manager);
  int xn_id = xnap_allocate_new_id(id_manager);
  
  LOG_I(XNAP, "NR Cell ID: %d \n",target_nci);
  LOG_I(XNAP, "Association ID: %d \n",assoc_id);
  //xnap_handover_req_t *req = &XNAP_HANDOVER_REQ(message_p);
  if (xn_id == -1) {
    LOG_E(XNAP,"could not allocate a new XNAP UE ID\n");
    /* TODO: cancel handover: send (to be defined) message to RRC */
    exit(1);
  }
  xnap_handover_req->s_ng_node_ue_xnap_id = xn_id;
  xnap_set_ids(id_manager, xnap_handover_req->s_ng_node_ue_xnap_id, xnap_handover_req->ue_id, xnap_handover_req->s_ng_node_ue_xnap_id, 0);
  //memcpy(&xnap_handover_req->ue_context.tnl_ip_source.buffer, &xnap_handover_req->ue_context.tnl_ip_source, sizeof(uint8_t) * 4);
  /* id_source is ue_id, id_target is unknown yet */
  //xnap_set_ids(id_manager, ue_id, xnap_handover_req->rnti, ue_id, -1); //use?ho req structure wont have rnti- take from where it belongs to.
  //xnap_id_set_state(id_manager, ue_id, XNID_STATE_SOURCE_PREPARE);// use?
  //xnap_set_reloc_prep_timer(id_manager, ue_id,
                            //xnap_timer_get_tti(&instance_p->timers));
  //xnap_id_set_target(id_manager, ue_id, target);

  //xnap_gNB_generate_xn_handover_request(instance_p, target, xnap_handover_req, ue_id); //Review removed target here but needs it
  xnap_gNB_generate_xn_handover_request(assoc_id, xnap_handover_req);
}


void *xnap_task(void *arg)
{
  MessageDef *received_msg = NULL;
  int result;
  LOG_D(XNAP, "Starting XNAP layer\n");
  itti_mark_task_ready(TASK_XNAP);

  while (1) {
    itti_receive_msg(TASK_XNAP, &received_msg);
    LOG_D(XNAP, "Received message %d:%s\n", ITTI_MSG_ID(received_msg), ITTI_MSG_NAME(received_msg));
    switch (ITTI_MSG_ID(received_msg)) {
      case TERMINATE_MESSAGE:
        LOG_W(XNAP, "Exiting XNAP thread\n");
        itti_exit_task();
        break;

      case XNAP_REGISTER_GNB_REQ: {
        xnap_net_config_t *xn_nc = &XNAP_REGISTER_GNB_REQ(received_msg).net_config;
        xnap_gNB_init_sctp(ITTI_MSG_DESTINATION_INSTANCE(received_msg), xn_nc, xn_nc->gnb_xn_ip_address.ipv4_address);
      } break;

      case XNAP_SETUP_FAILURE: // from rrc/xnap
        xnap_gNB_generate_xn_setup_failure(ITTI_MSG_ORIGIN_INSTANCE(received_msg), &XNAP_SETUP_FAILURE(received_msg));
        break;

      case XNAP_SETUP_RESP: // from rrc
        xnap_gNB_generate_xn_setup_response(ITTI_MSG_ORIGIN_INSTANCE(received_msg), &XNAP_SETUP_RESP(received_msg));
        break;

      case XNAP_HANDOVER_REQ: // from rrc
        xnap_gNB_handle_handover_prep(ITTI_MSG_ORIGIN_INSTANCE(received_msg),
                                            &XNAP_HANDOVER_REQ(received_msg));
	      break;
       case XNAP_HANDOVER_REQ_ACK:
        LOG_I(XNAP,"HANDOVER_ACK going into the function \n");
        xnap_gNB_generate_xn_handover_request_ack(ITTI_MSG_ORIGIN_INSTANCE(received_msg), &XNAP_HANDOVER_REQ_ACK(received_msg),ITTI_MSG_DESTINATION_INSTANCE(received_msg));
        break;


      case SCTP_INIT_MSG_MULTI_CNF:
        xnap_gNB_handle_sctp_init_msg_multi_cnf(ITTI_MSG_DESTINATION_INSTANCE(received_msg),
                                                &received_msg->ittiMsg.sctp_init_msg_multi_cnf);
        break;

      case SCTP_NEW_ASSOCIATION_RESP:
        xnap_gNB_handle_sctp_association_resp(ITTI_MSG_DESTINATION_INSTANCE(received_msg),
                                              &received_msg->ittiMsg.sctp_new_association_resp);
        break;

      case SCTP_NEW_ASSOCIATION_IND:
        xnap_gNB_handle_sctp_association_ind(ITTI_MSG_DESTINATION_INSTANCE(received_msg),
                                             &received_msg->ittiMsg.sctp_new_association_ind);
        break;

      case SCTP_DATA_IND:
        xnap_gNB_handle_sctp_data_ind(ITTI_MSG_DESTINATION_INSTANCE(received_msg), &received_msg->ittiMsg.sctp_data_ind);
        break;

      default:
        LOG_E(XNAP, "Received unhandled message: %d:%s\n", ITTI_MSG_ID(received_msg), ITTI_MSG_NAME(received_msg));
        break;
    }

    result = itti_free(ITTI_MSG_ORIGIN_ID(received_msg), received_msg);
    AssertFatal(result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);
    received_msg = NULL;
  }

  return NULL;
}

#include "common/config/config_userapi.h"

int is_xnap_enabled(void)
{
  static volatile int config_loaded = 0;
  static volatile int enabled = 0;
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  if (pthread_mutex_lock(&mutex))
    goto mutex_error;
  if (config_loaded) {
    if (pthread_mutex_unlock(&mutex))
      goto mutex_error;
    return enabled;
  }

  char *enable_xn = NULL;
  paramdef_t p[] = {{"enable_xn", "yes/no", 0, .strptr = &enable_xn, .defstrval = "", TYPE_STRING, 0}};

  /* TODO: do it per module - we check only first gNB */
  config_get(config_get_if(), p, sizeofArray(p), "gNBs.[0]");
  if (enable_xn != NULL && strcmp(enable_xn, "yes") == 0) {
    enabled = 1;
  }

  /*Consider also the case of enabling XnAP for a gNB by parsing a gNB configuration file*/

  config_get(config_get_if(), p, sizeofArray(p), "gNBs.[0]");
  if (enable_xn != NULL && strcmp(enable_xn, "yes") == 0) {
    enabled = 1;
  }

  config_loaded = 1;

  if (pthread_mutex_unlock(&mutex))
    goto mutex_error;
  return enabled;

mutex_error:
  LOG_E(XNAP, "mutex error\n");
  exit(1);
}
