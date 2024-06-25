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

/*! \file nrppa_gNB_decoder.c
 * \brief NRPPA pdu decode procedures for gNB
 * \author Adeel Malik
 * \email adeel.malik@eurecom.fr
 *\date 2023
 * \version 1.0
 * @ingroup _nrppa
 */

#include <stdio.h>
#include "assertions.h"
#include "intertask_interface.h"
#include "nrppa_common.h"
#include "nrppa_gNB_decoder.h"

static int nrppa_gNB_decode_initiating_message(NRPPA_NRPPA_PDU_t *pdu)
{
  asn_encode_to_new_buffer_result_t res = {NULL, {0, NULL, NULL}};
  DevAssert(pdu != NULL);

  switch (pdu->choice.initiatingMessage->procedureCode) {
    case NRPPA_ProcedureCode_id_positioningInformationExchange: // Parent procedure for PositioningInformationRequest,
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Positioning Information Request initiating message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_positioningActivation:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Positioning Activation Request initiating message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_positioningInformationUpdate:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Positioning Information Update initiating message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_positioningDeactivation:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Positioning Deactivation Request initiating message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_tRPInformationExchange: // Parent procedure for TRPInformationRequest,
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "TRP Information Request initiating message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_Measurement: // Parent procedure for Measurement Request,
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Measurement Request initiating message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_MeasurementReport:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Measurement Report initiating message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_MeasurementFailureIndication:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Measurement Failure Indication initiating message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_MeasurementAbort:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Measurement Abort initiating message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_MeasurementUpdate:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Measurement Update initiating message\n");
      free(res.buffer);
      break;
      /*TODO add other procedures  check TABLE 8.1-2 and TABLE 8.1-1 of NRPPA TS38.455 v16*/
      // TODO ad**l add remaining DOWNLINK type NRPPA Procedure code

    default:
      NRPPA_ERROR("Unknown procedure ID (%d) for initiating message\n", (int)pdu->choice.initiatingMessage->procedureCode);
      AssertFatal(0, "Unknown procedure ID (%d) for initiating message\n", (int)pdu->choice.initiatingMessage->procedureCode);
      return -1;
  }
  return 0;
}

static int nrppa_gNB_decode_successful_outcome(NRPPA_NRPPA_PDU_t *pdu)
{
  asn_encode_to_new_buffer_result_t res = {NULL, {0, NULL, NULL}};
  DevAssert(pdu != NULL);

  switch (pdu->choice.successfulOutcome->procedureCode) {
    case NRPPA_ProcedureCode_id_positioningInformationExchange:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Positioning Information Response successfull outcome message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_positioningActivation:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Positioning Activation Response successfull outcome message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_tRPInformationExchange:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "TRP Information Response successfull outcome message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_Measurement:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Measurement Response successfull outcome message\n");
      free(res.buffer);
      break;
      // ad**l TODO add other procedures  check TABLE 8.1-2 and TABLE 8.1-1 of NRPPA TS38.455 v16

    default:
      NRPPA_ERROR("Unknown procedure ID (%d) for successfull outcome message\n", (int)pdu->choice.initiatingMessage->procedureCode);
      AssertFatal(0,
                  "Unknown procedure ID (%d) for successfull outcome message\n",
                  (int)pdu->choice.initiatingMessage->procedureCode);
      return -1;
  }
  return 0;
}

static int nrppa_gNB_decode_unsuccessful_outcome(NRPPA_NRPPA_PDU_t *pdu)
{
  asn_encode_to_new_buffer_result_t res = {NULL, {0, NULL, NULL}};
  DevAssert(pdu != NULL);

  switch (pdu->choice.unsuccessfulOutcome->procedureCode) {
    case NRPPA_ProcedureCode_id_positioningInformationExchange:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Positioning Information Failure unsuccessfull outcome message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_positioningActivation:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Positioning Activation Failure unsuccessfull outcome message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_tRPInformationExchange:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "TRP Information Failure unsuccessfull outcome message\n");
      free(res.buffer);
      break;

    case NRPPA_ProcedureCode_id_Measurement:
      res = asn_encode_to_new_buffer(NULL, ATS_CANONICAL_XER, &asn_DEF_NRPPA_NRPPA_PDU, pdu);
      LOG_I(NRPPA, "Measurement Failure unsuccessfull outcome message\n");
      free(res.buffer);
      break;
      // ad**l TODO add other procedures  check TABLE 8.1-2 and TABLE 8.1-1 of NRPPA TS38.455 v16

    default:
      NRPPA_ERROR("Unknown procedure ID (%d) for unsuccessfull outcome message\n",
                  (int)pdu->choice.initiatingMessage->procedureCode);
      AssertFatal(0,
                  "Unknown procedure ID (%d) for unsuccessfull outcome message\n",
                  (int)pdu->choice.initiatingMessage->procedureCode);
      return -1;
  }
  return 0;
}

int nrppa_gNB_decode_pdu(NRPPA_NRPPA_PDU_t *pdu, const uint8_t *const buffer, const uint32_t length)
{
  asn_dec_rval_t dec_ret;
  DevAssert(pdu != NULL);
  DevAssert(buffer != NULL);
  asn_codec_ctx_t st = {.max_stack_size = 100 * 1000}; // if we enable asn1c debug the stack size become large  // ad**l todo
  dec_ret = aper_decode(&st, &asn_DEF_NRPPA_NRPPA_PDU, (void **)&pdu, buffer, length, 0, 0);
  if (dec_ret.code != RC_OK) {
    NRPPA_ERROR("Failed to decode pdu\n");
    return -1;
  }

  switch (pdu->present) {
    case NRPPA_NRPPA_PDU_PR_initiatingMessage:
      return nrppa_gNB_decode_initiating_message(pdu);

    case NRPPA_NRPPA_PDU_PR_successfulOutcome:
      return nrppa_gNB_decode_successful_outcome(pdu);

    case NRPPA_NRPPA_PDU_PR_unsuccessfulOutcome:
      return nrppa_gNB_decode_unsuccessful_outcome(pdu);

    default:
      LOG_I(NRPPA, "Unknown presence (%d) or not implemented\n", (int)pdu->present);
      break;
  }

  return -1;
}
