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
 * Author and copyright: Laurent Thomas, open-cells.com
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

#define _GNU_SOURCE
#include <time.h>
#include <stdlib.h>
#include "e1ap_common.h"
#include "e1ap_default_values.h"
#include "e1ap_asnc.h"
#include "common/openairinterface5g_limits.h"
#include "common/utils/ocp_itti/intertask_interface.h"

static e1ap_upcp_inst_t *e1ap_inst[NUMBER_OF_gNB_MAX] = {0};

e1ap_upcp_inst_t *getCxtE1(instance_t instance)
{
  AssertFatal(instance < sizeofArray(e1ap_inst), "instance exceeds limit\n");
  return e1ap_inst[instance];
}

void createE1inst(E1_t type, instance_t instance, uint64_t gnb_id, e1ap_net_config_t *nc, e1ap_setup_req_t *req)
{
  AssertFatal(e1ap_inst[instance] == NULL, "Double call to E1 instance %d\n", (int)instance);
  e1ap_inst[instance] = calloc(1, sizeof(e1ap_upcp_inst_t));
  e1ap_inst[instance]->type = type;
  e1ap_inst[instance]->instance = instance;
  e1ap_inst[instance]->gnb_id = gnb_id;
  e1ap_inst[instance]->cuup.assoc_id = -1;
  if (nc)
    memcpy(&e1ap_inst[instance]->net_config, nc, sizeof(*nc));
  if (req) {
    AssertFatal(type == UPtype, "E1 setup request only to be stored for CU-UP\n");
    memcpy(&e1ap_inst[instance]->cuup.setupReq, req, sizeof(*req));
  }
  e1ap_inst[instance]->gtpInstN3 = -1;
  e1ap_inst[instance]->gtpInstF1U = -1;
}

E1AP_TransactionID_t transacID[E1AP_MAX_NUM_TRANSAC_IDS] = {0}; 

void e1ap_common_init() {
  srand(time(NULL));
}

static bool check_transac_id(E1AP_TransactionID_t id, int *freeIdx)
{
  bool isFreeIdxSet = false;
  for (int i=0; i < E1AP_MAX_NUM_TRANSAC_IDS; i++) {
    if (id == transacID[i])
      return false;
    else if (!isFreeIdxSet && (transacID[i] == 0)) {
      *freeIdx = i;
      isFreeIdxSet = true;
    }
  }

  return true;
}

long E1AP_get_next_transaction_identifier() {
  E1AP_TransactionID_t genTransacId;
  bool isTransacIdValid = false;
  int freeIdx = 0;

  while (!isTransacIdValid) {
    genTransacId = rand() & 255;
    isTransacIdValid = check_transac_id(genTransacId, &freeIdx);
  }

  AssertFatal(freeIdx < E1AP_MAX_NUM_TRANSAC_IDS, "Free Index exceeds array length\n");
  transacID[freeIdx] = genTransacId;
  return genTransacId;
}

void E1AP_free_transaction_identifier(long id) {

  for (int i=0; i < E1AP_MAX_NUM_TRANSAC_IDS; i++) {
    if (id == transacID[i]) {
      transacID[i] = 0;
      return;
    }
  }
  LOG_E(E1AP, "Couldn't find transaction ID %ld in list\n", id);
}

int e1ap_decode_initiating_message(E1AP_E1AP_PDU_t *pdu) {
  DevAssert(pdu != NULL);

  switch(pdu->choice.initiatingMessage->procedureCode) {
    case E1AP_ProcedureCode_id_gNB_CU_UP_E1Setup:
      break;

    case E1AP_ProcedureCode_id_gNB_CU_UP_ConfigurationUpdate:
      break;

    case E1AP_ProcedureCode_id_bearerContextSetup:
      break;

    case E1AP_ProcedureCode_id_bearerContextModification:
      break;

    case E1AP_ProcedureCode_id_bearerContextRelease:
      break;

    default:
      LOG_E(E1AP, "Unsupported procedure code (%d) for initiating message\n",
            (int)pdu->choice.initiatingMessage->procedureCode);
      return -1;
  }
  return 0;
}

int e1ap_decode_successful_outcome(E1AP_E1AP_PDU_t *pdu) {
  DevAssert(pdu != NULL);
  switch(pdu->choice.successfulOutcome->procedureCode) {
    case E1AP_ProcedureCode_id_gNB_CU_UP_E1Setup:
      break;

    case E1AP_ProcedureCode_id_bearerContextSetup:
      break;

    case E1AP_ProcedureCode_id_bearerContextModification:
      break;

    case E1AP_ProcedureCode_id_bearerContextRelease:
      break;

    default:
      LOG_E(E1AP, "Unsupported procedure code (%d) for successful message\n",
            (int)pdu->choice.successfulOutcome->procedureCode);
      return -1;
  }
  return 0;
}

int e1ap_decode_unsuccessful_outcome(E1AP_E1AP_PDU_t *pdu) {
  DevAssert(pdu != NULL);
  switch(pdu->choice.unsuccessfulOutcome->procedureCode) {
    case E1AP_ProcedureCode_id_gNB_CU_UP_E1Setup:
      break;

    default:
      LOG_E(E1AP, "Unsupported procedure code (%d) for unsuccessful message\n",
            (int)pdu->choice.unsuccessfulOutcome->procedureCode);
      return -1;
  }
  return 0;
}

int e1ap_decode_pdu(E1AP_E1AP_PDU_t *pdu, const uint8_t *const buffer, uint32_t length) {
  asn_dec_rval_t dec_ret;
  DevAssert(buffer != NULL);
  dec_ret = aper_decode(NULL,
                        &asn_DEF_E1AP_E1AP_PDU,
                        (void **)&pdu,
                        buffer,
                        length,
                        0,
                        0);

  if (LOG_DEBUGFLAG(DEBUG_ASN1)) {
    LOG_E(E1AP, "----------------- ASN1 DECODER PRINT START----------------- \n");
    xer_fprint(stdout, &asn_DEF_E1AP_E1AP_PDU, pdu);
    LOG_E(E1AP, "----------------- ASN1 DECODER PRINT END ----------------- \n");
  }

  if (dec_ret.code != RC_OK) {
    AssertFatal(1==0,"Failed to decode pdu\n");
    return -1;
  }

  switch(pdu->present) {
    case E1AP_E1AP_PDU_PR_initiatingMessage:
      return e1ap_decode_initiating_message(pdu);

    case E1AP_E1AP_PDU_PR_successfulOutcome:
      return e1ap_decode_successful_outcome(pdu);

    case E1AP_E1AP_PDU_PR_unsuccessfulOutcome:
      return e1ap_decode_unsuccessful_outcome(pdu);

    default:
      LOG_E(E1AP, "Unknown presence (%d) or not implemented\n", (int)pdu->present);
      break;
  }

  return -1;
}

int e1ap_encode_send(E1_t type, sctp_assoc_t assoc_id, E1AP_E1AP_PDU_t *pdu, uint16_t stream, const char *func)
{
  DevAssert(pdu != NULL);

  if (LOG_DEBUGFLAG(DEBUG_ASN1)) {
    LOG_E(E1AP, "----------------- ASN1 ENCODER PRINT START ----------------- \n");
    xer_fprint(stdout, &asn_DEF_E1AP_E1AP_PDU, pdu);
    LOG_E(E1AP, "----------------- ASN1 ENCODER PRINT END----------------- \n");
  }

  char errbuf[2048]; /* Buffer for error message */
  size_t errlen = sizeof(errbuf); /* Size of the buffer */
  int ret = asn_check_constraints(&asn_DEF_E1AP_E1AP_PDU, pdu, errbuf, &errlen);

  if(ret) {
    LOG_E(E1AP, "%s: Constraint validation failed: %s\n", func, errbuf);
  }

  void *buffer = NULL;
  ssize_t encoded = aper_encode_to_new_buffer(&asn_DEF_E1AP_E1AP_PDU, 0, pdu, &buffer);
  ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E1AP_E1AP_PDU, pdu);

  if (encoded < 0) {
    LOG_E(E1AP, "%s: Failed to encode E1AP message\n", func);
    return -1;
  }
  MessageDef *message = itti_alloc_new_message((type == CPtype) ? TASK_CUCP_E1 : TASK_CUUP_E1, 0, SCTP_DATA_REQ);
  sctp_data_req_t *s = &message->ittiMsg.sctp_data_req;
  s->assoc_id = assoc_id;
  s->buffer = buffer;
  s->buffer_length = encoded;
  s->stream = stream;
  LOG_D(E1AP, "%s: Sending ITTI message to SCTP Task\n", func);
  itti_send_msg_to_task(TASK_SCTP, 0 /*unused by callee*/, message);

  return encoded;
}

uint64_t get_5QI_id(uint64_t fiveqi)
{
  for (int id = 0; id < STANDARIZED_5QI_NUM; id++) {
    if (params_5QI[id].five_QI == fiveqi)
      return id;
  }
  AssertFatal(1 == 0, "Invalid 5QI value\n");
}

static int qos_cmp(const void *a, const void *b, void *qostype)
{
  fiveQI_type_t qos_type = *((fiveQI_type_t *)qostype);
  long priority1 = 0, priority2 = 0;
  if (qos_type == non_dynamic) { // non-dynamic
    priority1 =
        params_5QI[get_5QI_id(((qos_flow_to_setup_t *)a)->qos_params.qos_characteristics.non_dynamic.fiveqi)].priority_level;
    priority2 =
        params_5QI[get_5QI_id(((qos_flow_to_setup_t *)b)->qos_params.qos_characteristics.non_dynamic.fiveqi)].priority_level;
  } else { // dynamic
    priority1 = ((qos_flow_to_setup_t *)a)->qos_params.qos_characteristics.dynamic.qos_priority_level;
    priority2 = ((qos_flow_to_setup_t *)b)->qos_params.qos_characteristics.dynamic.qos_priority_level;
  }
  return priority1 - priority2;
}

void get_drb_characteristics(qos_flow_to_setup_t *qos_flows_in,
                             int num_qos_flows,
                             fiveQI_type_t qos_type,
                             qos_flow_level_qos_parameters_t *dRB_QoS)
{
  qos_characteristics_t *qos_char_drb = &dRB_QoS->qos_characteristics;
  qos_flow_to_setup_t *qos_flow = qos_flows_in;

  if (qos_type == non_dynamic) { // non-dynamic
    qsort_r(qos_flow, num_qos_flows, sizeof(qos_flow_to_setup_t), qos_cmp, (void *)&qos_type);
    qos_char_drb->qos_type = non_dynamic;
    qos_char_drb->non_dynamic.fiveqi = qos_flow->qos_params.qos_characteristics.non_dynamic.fiveqi;
  } else { // dynamic
    qsort_r(qos_flow, num_qos_flows, sizeof(qos_flow_to_setup_t), qos_cmp, (void *)&qos_type);
    qos_char_drb->qos_type = dynamic;
    qos_char_drb->dynamic.qos_priority_level = qos_flow->qos_params.qos_characteristics.dynamic.qos_priority_level;
    qos_char_drb->dynamic.packet_delay_budget = qos_flow->qos_params.qos_characteristics.dynamic.packet_delay_budget;
    qos_char_drb->dynamic.packet_error_rate.per_exponent =
        qos_flow->qos_params.qos_characteristics.dynamic.packet_error_rate.per_exponent;
    qos_char_drb->dynamic.packet_error_rate.per_scalar =
        qos_flow->qos_params.qos_characteristics.dynamic.packet_error_rate.per_scalar;
  }

  /* GBR Information for a DRB */
  gbr_qos_flow_information_t *gbr_in = qos_flow->qos_params.gbr_qos_flow_info;
  if (gbr_in) {
    dRB_QoS->gbr_qos_flow_info = calloc(1, sizeof(gbr_qos_flow_information_t));
    dRB_QoS->gbr_qos_flow_info->gbr_dl = gbr_in->gbr_dl;
    dRB_QoS->gbr_qos_flow_info->gbr_ul = gbr_in->gbr_ul;
    dRB_QoS->gbr_qos_flow_info->mbr_dl = gbr_in->mbr_dl;
    dRB_QoS->gbr_qos_flow_info->mbr_ul = gbr_in->mbr_ul;
  }
}
