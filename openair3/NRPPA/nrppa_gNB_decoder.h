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
/*! \file nrppa_gNB_decoder.h
 * \brief NRPPA pdu decode procedures for gNB
 * \author Adeel Malik
 * \email adeel.malik@eurecom.fr
 *\date 2023
 * \version 1.0
 * @ingroup _nrppa
 */

#include <stdint.h>

/* ad**l todo */
#include "nrppa_common.h"
/* ad**l todo */

#ifndef NRPPA_GNB_DECODER_H_
#define NRPPA_GNB_DECODER_H_

// int nrppa_gNB_decode_pdu(NRPPA_NRPPA_PDU_t *pdu, const uint8_t *const buffer, const uint32_t length) __attribute__
// ((warn_unused_result)); // to check ad**l __attribute__ ((warn_unused_result))

int nrppa_gNB_decode_pdu(NRPPA_NRPPA_PDU_t *pdu, const uint8_t *const buffer, const uint32_t length)
    __attribute__((warn_unused_result));
#endif /* NRPPA_GNB_DECODER_H_ */
