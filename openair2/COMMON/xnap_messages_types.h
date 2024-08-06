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

/*! \file xnap_messages_types.h
 * \author Sreeshma Shiv <sreeshmau@iisc.ac.in>
 * \date August 2023
 * \version 1.0
 */
#ifndef XNAP_MESSAGES_TYPES_H_
#define XNAP_MESSAGES_TYPES_H_

#define XNAP_MAX_NB_GNB_IP_ADDRESS 4

typedef struct xnap_sctp_s {
  uint16_t sctp_in_streams;
  uint16_t sctp_out_streams;
} xnap_sctp_t;

typedef struct xnap_net_config_t {
  uint8_t nb_xn;
  char* gnb_xn_interface_ip_address;
  char* target_gnb_xn_ip_address[XNAP_MAX_NB_GNB_IP_ADDRESS];
  uint32_t gnb_port_for_XNC;
  xnap_sctp_t sctp_streams;
} xnap_net_config_t;

typedef struct xnap_plmn_t {
  uint16_t mcc;
  uint16_t mnc;
  uint8_t  mnc_digit_length;
} xnap_plmn_t;

typedef struct xnap_amf_regioninfo_s {
  xnap_plmn_t plmn;
  uint8_t amf_region_id;
} xnap_amf_regioninfo_t;

typedef enum xnap_mode_t { XNAP_MODE_TDD = 0, XNAP_MODE_FDD = 1 } xnap_mode_t;

typedef struct xnap_nr_frequency_info_t {
  uint32_t arfcn;
  int band;
} xnap_nr_frequency_info_t;

typedef struct xnap_transmission_bandwidth_t {
  uint8_t scs;
  uint16_t nrb;
} xnap_transmission_bandwidth_t;

typedef struct xnap_fdd_info_t {
  xnap_nr_frequency_info_t ul_freqinfo;
  xnap_nr_frequency_info_t dl_freqinfo;
  xnap_transmission_bandwidth_t ul_tbw;
  xnap_transmission_bandwidth_t dl_tbw;
} xnap_fdd_info_t;

typedef struct xnap_tdd_info_t {
  xnap_nr_frequency_info_t freqinfo;
  xnap_transmission_bandwidth_t tbw;
} xnap_tdd_info_t;

typedef struct xnap_snssai_s {
  uint8_t sst;
  uint8_t sd;
} xnap_snssai_t;

typedef struct xnap_served_cell_info_t {
  // NR CGI
  xnap_plmn_t plmn;
  uint64_t nr_cellid; // NR Global Cell Id
  uint16_t nr_pci;// NR Physical Cell Ids

  /* Tracking area code */
  uint32_t tac;

  xnap_mode_t mode;
  union {
    xnap_fdd_info_t fdd;
    xnap_tdd_info_t tdd;
  };

  uint8_t *measurement_timing_information;
} xnap_served_cell_info_t;

typedef struct xnap_setup_req_s {
  uint64_t gNB_id;
  /* Tracking area code */
  uint16_t num_tai;
  uint32_t tai_support;
  xnap_plmn_t plmn_support;
  // Number of slide support items
  uint16_t num_snssai;
  xnap_snssai_t snssai[2];
  xnap_amf_regioninfo_t amf_region_info;
  uint8_t num_cells_available;
  xnap_served_cell_info_t info;
} xnap_setup_req_t;

#endif /* XNAP_MESSAGES_TYPES_H_ */
