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

#include "s1ap_messages_types.h"
#include "f1ap_messages_types.h"
// Defines to access message fields.

#define XNAP_REGISTER_GNB_REQ(mSGpTR) (mSGpTR)->ittiMsg.xnap_register_gnb_req
#define XNAP_SETUP_REQ(mSGpTR) (mSGpTR)->ittiMsg.xnap_setup_req
#define XNAP_SETUP_RESP(mSGpTR) (mSGpTR)->ittiMsg.xnap_setup_resp
#define XNAP_SETUP_FAILURE(mSGpTR) (mSGpTR)->ittiMsg.xnap_setup_failure
#define XNAP_HANDOVER_REQ(mSGpTR) (mSGpTR)->ittiMsg.xnap_handover_req
#define XNAP_HANDOVER_REQ_FAILURE(mSGpTR) (mSGpTR)->ittiMsg.xnap_handover_req_failure
#define XNAP_HANDOVER_REQ_ACK(mSGpTR) (mSGpTR)->ittiMsg.xnap_handover_req_ack


#define XNAP_MAX_NB_GNB_IP_ADDRESS 4

// gNB application layer -> XNAP messages

typedef struct xnap_net_ip_address_s {
  unsigned ipv4:1;
  unsigned ipv6:1;
  char ipv4_address[16];
  char ipv6_address[46];
} xnap_net_ip_address_t;

typedef struct xnap_sctp_s {
  uint16_t sctp_in_streams;
  uint16_t sctp_out_streams;
} xnap_sctp_t;

typedef struct xnap_net_config_t {
  uint8_t nb_xn;
  xnap_net_ip_address_t gnb_xn_ip_address;
  xnap_net_ip_address_t target_gnb_xn_ip_address[XNAP_MAX_NB_GNB_IP_ADDRESS];
  uint32_t gnb_port_for_XNC;
  xnap_sctp_t sctp_streams;
} xnap_net_config_t;

typedef struct xnap_plmn_t {
  uint16_t mcc;
  uint16_t mnc;
  uint8_t  mnc_digit_length;
} xnap_plmn_t;

typedef struct xnap_amf_regioninfo_s {
  uint16_t mcc;
  uint16_t mnc;
  uint8_t mnc_len;
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
  uint32_t sd;
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

  char *measurement_timing_information;
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

typedef struct xnap_setup_resp_s {
  int64_t gNB_id;
  /* Tracking area code */
  uint16_t num_tai;
  uint32_t tai_support;
  xnap_plmn_t plmn_support;
  // Number of slide support items
  uint16_t num_ssi;
  uint8_t sst;
  uint8_t sd;
  uint16_t nb_xn;//number of gNBs connected
  xnap_served_cell_info_t info;
} xnap_setup_resp_t;

typedef struct xnap_register_gnb_req_s {
  xnap_setup_req_t setup_req;
  xnap_net_config_t net_config;
  char *gNB_name;
} xnap_register_gnb_req_t;

typedef enum xnap_Cause_e {
  XNAP_CAUSE_NOTHING,  /* No components present */
  XNAP_CAUSE_RADIO_NETWORK,
  XNAP_CAUSE_TRANSPORT,
  XNAP_CAUSE_PROTOCOL,
  XNAP_CAUSE_MISC,
} xnap_Cause_t;

typedef struct xnap_setup_failure_s {
  long cause_value;
  xnap_Cause_t cause_type;
  uint16_t time_to_wait;
  uint16_t criticality_diagnostics;
} xnap_setup_failure_t;

typedef struct xnap_guami_s {
  xnap_plmn_t plmn_id;
  uint8_t amf_region_id;
  uint8_t amf_set_id;
  uint8_t amf_pointer;
} xnap_guami_t;

typedef struct xnap_allocation_retention_priority_s {
  uint16_t priority_level;
  preemption_capability_t preemption_capability;
  preemption_vulnerability_t preemption_vulnerability;
} xnap_allocation_retention_priority_t;

typedef struct xnap_qos_characteristics_s {
  union {
    struct {
      long fiveqi;
      long qos_priority_level;
    } non_dynamic;
    struct {
      long fiveqi; // -1 -> optional
      long qos_priority_level;
      long packet_delay_budget;
      struct {
        long per_scalar;
        long per_exponent;
      } packet_error_rate;
    } dynamic;
  };
  fiveQI_type_t qos_type;
} xnap_qos_characteristics_t;

typedef struct xnap_qos_tobe_setup_item_s {
  long qfi;
  xnap_qos_characteristics_t qos_params;
  xnap_allocation_retention_priority_t allocation_retention_priority;
} xnap_qos_tobe_setup_item_t;

typedef struct xnap_qos_tobe_setup_list_s {
  uint8_t num_qos;
  xnap_qos_tobe_setup_item_t qos[QOSFLOW_MAX_VALUE]; //QOSFLOW_MAX_VALUE= 64 Put this?
} xnap_qos_tobe_setup_list_t;

typedef struct xnap_pdusession_tobe_setup_item_s {
  long pdusession_id;
  xnap_snssai_t snssai;
  //xnap_net_ip_address_t up_ngu_tnl_ip_upf;
  in_addr_t up_ngu_tnl_ip_upf;
  teid_t up_ngu_tnl_teid_upf;
  pdu_session_type_t pdu_session_type;
  xnap_qos_tobe_setup_list_t qos_list;
} xnap_pdusession_tobe_setup_item_t;

typedef struct xnap_pdusession_tobe_setup_list_s {
  uint8_t num_pdu;
  xnap_pdusession_tobe_setup_item_t pdu[NGAP_MAX_PDUSESSION]; //Is the limit ok?
} xnap_pdusession_tobe_setup_list_t;

typedef struct xnap_ngran_cgi_t {
  xnap_plmn_t plmn_id;
  uint32_t cgi;
} xnap_ngran_cgi_t;

typedef struct xnap_security_capabilities_s {
  uint16_t encryption_algorithms;
  uint16_t integrity_algorithms;
} xnap_security_capabilities_t;

typedef struct xnap_ambr_s {
  uint64_t br_ul;
  uint64_t br_dl;
} xnap_ambr_t;

typedef struct xnap_uehistory_info_s {
  xnap_ngran_cgi_t last_visited_cgi;
  cell_type_t cell_type; //enumerated -s1ap_messages_types.h
  uint64_t time_UE_StayedInCell;
} xnap_uehistory_info_t; //38.413- 9.3.1.97

typedef struct xnap_ue_context_info_s {
  uint64_t ngc_ue_sig_ref;// 0-2^40-1
  //xnap_net_ip_address_t tnl_ip_source;
  in_addr_t tnl_ip_source;
  uint32_t tnl_port_source;
  xnap_security_capabilities_t security_capabilities;
  //uint8_t kRRCenc[16];
  //uint8_t kRRCint[16];
  uint32_t as_security_key_ranstar;//bitstring 256, why array?
  long as_security_ncc;
  xnap_ambr_t ue_ambr;
  uint8_t rrc_buffer[8192 /* arbitrary, big enough */];
  xnap_pdusession_tobe_setup_list_t pdusession_tobe_setup_list;
  int rrc_buffer_size;//rrc msg type needed?
  int target_assoc_id;
  uint8_t nb_e_rabs_tobesetup;
} xnap_ue_context_info_t;

typedef struct xnap_handover_req_s {
  int ue_id; /* used for RRC->XNAP in source */
  //int xn_id;  /* used for XNAP->RRC in target*/
  uint32_t s_ng_node_ue_xnap_id;
  uint32_t t_ng_node_ue_xnap_id;
  xnap_plmn_t plmn_id;
  xnap_Cause_t cause_type;
  xnap_ngran_cgi_t target_cgi;
  xnap_guami_t guami;
  xnap_ue_context_info_t ue_context;
} xnap_handover_req_t;

typedef struct xnap_handover_req_ack_s {
  int ue_id;
  uint32_t s_ng_node_ue_xnap_id;
  uint32_t t_ng_node_ue_xnap_id;
  xnap_ngran_cgi_t target_cgi;
  xnap_plmn_t plmn_id;
  xnap_guami_t guami;
  xnap_ue_context_info_t ue_context;
  xnap_uehistory_info_t uehistory_info;
  uint8_t rrc_buffer[8192];
  int rrc_buffer_size;
} xnap_handover_req_ack_t;

typedef struct xnap_handover_req_failure_s{
 uint32_t ng_node_ue_xnap_id;
 long cause_value;
 xnap_Cause_t cause_type;
 xnap_ngran_cgi_t target_cgi;
}xnap_handover_req_failure_t;
#endif /* XNAP_MESSAGES_TYPES_H_ */
