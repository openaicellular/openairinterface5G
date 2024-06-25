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

#ifndef F1AP_MESSAGES_TYPES_H_
#define F1AP_MESSAGES_TYPES_H_

#include <netinet/in.h>
#include <netinet/sctp.h>
#include "s1ap_messages_types.h"
#include "ngap_messages_types.h"

//-------------------------------------------------------------------------------------------//
// Defines to access message fields.

#define F1AP_CU_SCTP_REQ(mSGpTR)                   (mSGpTR)->ittiMsg.f1ap_cu_sctp_req

#define F1AP_DU_REGISTER_REQ(mSGpTR)               (mSGpTR)->ittiMsg.f1ap_du_register_req

#define F1AP_SETUP_REQ(mSGpTR)                     (mSGpTR)->ittiMsg.f1ap_setup_req
#define F1AP_SETUP_RESP(mSGpTR)                    (mSGpTR)->ittiMsg.f1ap_setup_resp
#define F1AP_GNB_CU_CONFIGURATION_UPDATE(mSGpTR)   (mSGpTR)->ittiMsg.f1ap_gnb_cu_configuration_update
#define F1AP_GNB_CU_CONFIGURATION_UPDATE_ACKNOWLEDGE(mSGpTR)   (mSGpTR)->ittiMsg.f1ap_gnb_cu_configuration_update_acknowledge
#define F1AP_GNB_CU_CONFIGURATION_UPDATE_FAILURE(mSGpTR)   (mSGpTR)->ittiMsg.f1ap_gnb_cu_configuration_update_failure
#define F1AP_GNB_DU_CONFIGURATION_UPDATE(mSGpTR)   (mSGpTR)->ittiMsg.f1ap_gnb_du_configuration_update
#define F1AP_GNB_DU_CONFIGURATION_UPDATE_ACKNOWLEDGE(mSGpTR)   (mSGpTR)->ittiMsg.f1ap_gnb_du_configuration_update_acknowledge
#define F1AP_GNB_DU_CONFIGURATION_UPDATE_FAILURE(mSGpTR)   (mSGpTR)->ittiMsg.f1ap_gnb_du_configuration_update_failure

#define F1AP_SETUP_FAILURE(mSGpTR)                 (mSGpTR)->ittiMsg.f1ap_setup_failure

#define F1AP_LOST_CONNECTION(mSGpTR)   (mSGpTR)->ittiMsg.f1ap_lost_connection

#define F1AP_INITIAL_UL_RRC_MESSAGE(mSGpTR)        (mSGpTR)->ittiMsg.f1ap_initial_ul_rrc_message
#define F1AP_UL_RRC_MESSAGE(mSGpTR)                (mSGpTR)->ittiMsg.f1ap_ul_rrc_message
#define F1AP_UE_CONTEXT_SETUP_REQ(mSGpTR)          (mSGpTR)->ittiMsg.f1ap_ue_context_setup_req
#define F1AP_UE_CONTEXT_SETUP_RESP(mSGpTR)         (mSGpTR)->ittiMsg.f1ap_ue_context_setup_resp
#define F1AP_UE_CONTEXT_MODIFICATION_REQ(mSGpTR)   (mSGpTR)->ittiMsg.f1ap_ue_context_modification_req
#define F1AP_UE_CONTEXT_MODIFICATION_RESP(mSGpTR)  (mSGpTR)->ittiMsg.f1ap_ue_context_modification_resp
#define F1AP_UE_CONTEXT_MODIFICATION_FAIL(mSGpTR)  (mSGpTR)->ittiMsg.f1ap_ue_context_modification_fail
#define F1AP_UE_CONTEXT_MODIFICATION_REQUIRED(mSGpTR)   (mSGpTR)->ittiMsg.f1ap_ue_context_modification_required
#define F1AP_UE_CONTEXT_MODIFICATION_CONFIRM(mSGpTR)  (mSGpTR)->ittiMsg.f1ap_ue_context_modification_confirm
#define F1AP_UE_CONTEXT_MODIFICATION_REFUSE(mSGpTR)  (mSGpTR)->ittiMsg.f1ap_ue_context_modification_refuse

#define F1AP_DL_RRC_MESSAGE(mSGpTR)                (mSGpTR)->ittiMsg.f1ap_dl_rrc_message
#define F1AP_UE_CONTEXT_RELEASE_REQ(mSGpTR)        (mSGpTR)->ittiMsg.f1ap_ue_context_release_req
#define F1AP_UE_CONTEXT_RELEASE_CMD(mSGpTR)        (mSGpTR)->ittiMsg.f1ap_ue_context_release_cmd
#define F1AP_UE_CONTEXT_RELEASE_COMPLETE(mSGpTR)   (mSGpTR)->ittiMsg.f1ap_ue_context_release_complete

#define F1AP_PAGING_IND(mSGpTR)                    (mSGpTR)->ittiMsg.f1ap_paging_ind

/*Position Information Transfer related NRPPA messages*/
#define F1AP_POSITIONING_INFORMATION_REQ(mSGpTR) (mSGpTR)->ittiMsg.f1ap_positioning_information_req
#define F1AP_POSITIONING_INFORMATION_RESP(mSGpTR) (mSGpTR)->ittiMsg.f1ap_positioning_information_resp
#define F1AP_POSITIONING_INFORMATION_FAILURE(mSGpTR) (mSGpTR)->ittiMsg.f1ap_positioning_information_failure
#define F1AP_POSITIONING_INFORMATION_UPDATE(mSGpTR) (mSGpTR)->ittiMsg.f1ap_positioning_information_update
#define F1AP_POSITIONING_ACTIVATION_REQ(mSGpTR) (mSGpTR)->ittiMsg.f1ap_positioning_activation_req
#define F1AP_POSITIONING_ACTIVATION_RESP(mSGpTR) (mSGpTR)->ittiMsg.f1ap_positioning_activation_resp
#define F1AP_POSITIONING_ACTIVATION_FAILURE(mSGpTR) (mSGpTR)->ittiMsg.f1ap_positioning_activation_failure
#define F1AP_POSITIONING_DEACTIVATION(mSGpTR) (mSGpTR)->ittiMsg.f1ap_positioning_deactivation

/*TRP Information Transfer related NRPPA messages*/
#define F1AP_TRP_INFORMATION_REQ(mSGpTR) (mSGpTR)->ittiMsg.f1ap_trp_information_req
#define F1AP_TRP_INFORMATION_RESP(mSGpTR) (mSGpTR)->ittiMsg.f1ap_trp_information_resp
#define F1AP_TRP_INFORMATION_FAILURE(mSGpTR) (mSGpTR)->ittiMsg.f1ap_trp_information_failure

/*Measurement Information Transfer related NRPPA messages*/
#define F1AP_MEASUREMENT_REQ(mSGpTR) (mSGpTR)->ittiMsg.f1ap_measurement_req
#define F1AP_MEASUREMENT_RESP(mSGpTR) (mSGpTR)->ittiMsg.f1ap_measurement_resp
#define F1AP_MEASUREMENT_FAILURE(mSGpTR) (mSGpTR)->ittiMsg.f1ap_measurement_failure
#define F1AP_MEASUREMENT_REPORT(mSGpTR) (mSGpTR)->ittiMsg.f1ap_measurement_report
#define F1AP_MEASUREMENT_UPDATE(mSGpTR) (mSGpTR)->ittiMsg.f1ap_measurement_update
#define F1AP_MEASUREMENT_FAILURE_IND(mSGpTR) (mSGpTR)->ittiMsg.f1ap_measurement_failure_ind
#define F1AP_MEASUREMENT_ABORT(mSGpTR) (mSGpTR)->ittiMsg.f1ap_measurement_abort

/* Length of the transport layer address string
 * 160 bits / 8 bits by char.
 */
#define F1AP_TRANSPORT_LAYER_ADDRESS_SIZE (160 / 8)

#define F1AP_MAX_NB_CU_IP_ADDRESS 10

// Note this should be 512 from maxval in 38.473
#define F1AP_MAX_NB_CELLS 2

#define F1AP_MAX_NO_OF_TNL_ASSOCIATIONS 32
#define F1AP_MAX_NO_UE_ID 1024

typedef net_ip_address_t f1ap_net_ip_address_t;

typedef struct f1ap_net_config_t {
  f1ap_net_ip_address_t CU_f1_ip_address;
  f1ap_net_ip_address_t DU_f1c_ip_address;
  char *DU_f1u_ip_address;
  uint16_t CUport;
  uint16_t DUport;
} f1ap_net_config_t;

typedef struct f1ap_plmn_t {
  uint16_t mcc;
  uint16_t mnc;
  uint8_t  mnc_digit_length;
} f1ap_plmn_t;

typedef enum f1ap_mode_t { F1AP_MODE_TDD = 0, F1AP_MODE_FDD = 1 } f1ap_mode_t;

typedef struct f1ap_nr_frequency_info_t {
  uint32_t arfcn;
  int band;
} f1ap_nr_frequency_info_t;

typedef struct f1ap_transmission_bandwidth_t {
  uint8_t scs;
  uint16_t nrb;
} f1ap_transmission_bandwidth_t;

typedef struct f1ap_fdd_info_t {
  f1ap_nr_frequency_info_t ul_freqinfo;
  f1ap_nr_frequency_info_t dl_freqinfo;
  f1ap_transmission_bandwidth_t ul_tbw;
  f1ap_transmission_bandwidth_t dl_tbw;
} f1ap_fdd_info_t;

typedef struct f1ap_tdd_info_t {
  f1ap_nr_frequency_info_t freqinfo;
  f1ap_transmission_bandwidth_t tbw;
} f1ap_tdd_info_t;

typedef struct f1ap_served_cell_info_t {
  // NR CGI
  f1ap_plmn_t plmn;
  uint64_t nr_cellid; // NR Global Cell Id

  // NR Physical Cell Ids
  uint16_t nr_pci;

  /* Tracking area code */
  uint32_t *tac;

  // Number of slice support items (max 16, could be increased to as much as 1024)
  uint16_t num_ssi;
  nssai_t nssai[16];

  f1ap_mode_t mode;
  union {
    f1ap_fdd_info_t fdd;
    f1ap_tdd_info_t tdd;
  };

  uint8_t *measurement_timing_config;
  int measurement_timing_config_len;
} f1ap_served_cell_info_t;

typedef struct f1ap_gnb_du_system_info_t {
  uint8_t *mib;
  int mib_length;
  uint8_t *sib1;
  int sib1_length;
} f1ap_gnb_du_system_info_t;

typedef struct f1ap_setup_req_s {
  /// ulong transaction id
  uint64_t transaction_id;

  // F1_Setup_Req payload
  uint64_t gNB_DU_id;
  char *gNB_DU_name;

  /// rrc version
  uint8_t rrc_ver[3];

  /// number of DU cells available
  uint16_t num_cells_available; //0< num_cells_available <= 512;
  struct {
    f1ap_served_cell_info_t info;
    f1ap_gnb_du_system_info_t *sys_info;
  } cell[F1AP_MAX_NB_CELLS];
} f1ap_setup_req_t;

typedef struct f1ap_du_register_req_t {
  f1ap_setup_req_t setup_req;
  f1ap_net_config_t net_config;
} f1ap_du_register_req_t;

typedef struct served_cells_to_activate_s {
  f1ap_plmn_t plmn;
  // NR Global Cell Id
  uint64_t nr_cellid;
  /// NRPCI
  uint16_t nrpci;
  /// num SI messages per DU cell
  uint8_t num_SI;
  /// SI message containers (up to 21 messages per cell)
  uint8_t *SI_container[21];
  int      SI_container_length[21];
  int SI_type[21];
} served_cells_to_activate_t;

typedef struct f1ap_setup_resp_s {
  /// ulong transaction id
  uint64_t transaction_id;
  /// string holding gNB_CU_name
  char     *gNB_CU_name;
  /// number of DU cells to activate
  uint16_t num_cells_to_activate; //0< num_cells_to_activate <= 512;
  served_cells_to_activate_t cells_to_activate[F1AP_MAX_NB_CELLS];

  /// rrc version
  uint8_t rrc_ver[3];

} f1ap_setup_resp_t;

typedef struct f1ap_gnb_cu_configuration_update_s {
  /* Connexion id used between SCTP/F1AP */
  uint16_t cnx_id;

  /* SCTP association id */
  sctp_assoc_t assoc_id;

  /* Number of SCTP streams used for a mme association */
  uint16_t sctp_in_streams;
  uint16_t sctp_out_streams;

  /// string holding gNB_CU_name
  char     *gNB_CU_name;
  /// number of DU cells to activate
  uint16_t num_cells_to_activate; //0< num_cells_to_activate/mod <= 512;
  served_cells_to_activate_t cells_to_activate[F1AP_MAX_NB_CELLS];
} f1ap_gnb_cu_configuration_update_t;

typedef struct f1ap_setup_failure_s {
  uint16_t cause;
  uint16_t time_to_wait;
  uint16_t criticality_diagnostics; 
} f1ap_setup_failure_t;

typedef struct f1ap_gnb_cu_configuration_update_acknowledge_s {
  uint16_t num_cells_failed_to_be_activated;
  f1ap_plmn_t plmn[F1AP_MAX_NB_CELLS];
  uint64_t nr_cellid[F1AP_MAX_NB_CELLS];
  uint16_t cause[F1AP_MAX_NB_CELLS];
  int have_criticality;
  uint16_t criticality_diagnostics; 
  uint16_t noofTNLAssociations_to_setup;
  uint16_t have_port[F1AP_MAX_NO_OF_TNL_ASSOCIATIONS];
  in_addr_t tl_address[F1AP_MAX_NO_OF_TNL_ASSOCIATIONS]; // currently only IPv4 supported
  uint16_t noofTNLAssociations_failed;
  in_addr_t tl_address_failed[F1AP_MAX_NO_OF_TNL_ASSOCIATIONS]; // currently only IPv4 supported
  uint16_t cause_failed[F1AP_MAX_NO_OF_TNL_ASSOCIATIONS];
  uint16_t noofDedicatedSIDeliveryNeededUEs;
  uint32_t gNB_CU_ue_id[F1AP_MAX_NO_UE_ID]; 
  f1ap_plmn_t ue_plmn[F1AP_MAX_NO_UE_ID];
  uint64_t ue_nr_cellid[F1AP_MAX_NO_UE_ID];  
} f1ap_gnb_cu_configuration_update_acknowledge_t;

typedef struct f1ap_gnb_cu_configuration_update_failure_s {
  uint16_t cause;
  uint16_t time_to_wait;
  uint16_t criticality_diagnostics;
} f1ap_gnb_cu_configuration_update_failure_t;

/*DU configuration messages*/
typedef struct f1ap_gnb_du_configuration_update_s {
  /*TODO UPDATE TO SUPPORT DU CONFIG*/

  /* Transaction ID */
  uint64_t transaction_id;
  /// int cells_to_add
  uint16_t num_cells_to_add;
  struct {
    f1ap_served_cell_info_t info;
    f1ap_gnb_du_system_info_t *sys_info;
  } cell_to_add[F1AP_MAX_NB_CELLS];

  /// int cells_to_modify
  uint16_t num_cells_to_modify;
  struct {
    f1ap_plmn_t old_plmn;
    uint64_t old_nr_cellid; // NR Global Cell Id
    f1ap_served_cell_info_t info;
    f1ap_gnb_du_system_info_t *sys_info;
  } cell_to_modify[F1AP_MAX_NB_CELLS];

  /// int cells_to_delete
  uint16_t num_cells_to_delete;
  struct {
    // NR CGI
    f1ap_plmn_t plmn;
    uint64_t nr_cellid; // NR Global Cell Id
  } cell_to_delete[F1AP_MAX_NB_CELLS];

  /// string holding gNB_CU_name
  uint64_t *gNB_DU_ID;
} f1ap_gnb_du_configuration_update_t;

typedef struct f1ap_gnb_du_configuration_update_acknowledge_s {
  /// ulong transaction id
  uint64_t transaction_id;
  /// string holding gNB_CU_name
  char *gNB_CU_name;
  /// number of DU cells to activate
  uint16_t num_cells_to_activate; // 0< num_cells_to_activate <= 512;
  served_cells_to_activate_t cells_to_activate[F1AP_MAX_NB_CELLS];
} f1ap_gnb_du_configuration_update_acknowledge_t;

typedef struct f1ap_gnb_du_configuration_update_failure_s {
  /*TODO UPDATE TO SUPPORT DU CONFIG*/
  uint16_t cause;
  uint16_t time_to_wait;
  uint16_t criticality_diagnostics;
} f1ap_gnb_du_configuration_update_failure_t;

typedef struct f1ap_dl_rrc_message_s {
  uint32_t gNB_CU_ue_id;
  uint32_t gNB_DU_ue_id;
  uint32_t *old_gNB_DU_ue_id;
  uint8_t  srb_id;
  uint8_t  execute_duplication;
  uint8_t *rrc_container;
  int      rrc_container_length;
  union {
    // both map 0..255 => 1..256
    uint8_t en_dc;
    uint8_t ngran;
  } RAT_frequency_priority_information;
} f1ap_dl_rrc_message_t;

typedef struct f1ap_initial_ul_rrc_message_s {
  uint32_t gNB_DU_ue_id;
  f1ap_plmn_t plmn;
  /// nr cell id
  uint64_t nr_cellid;
  /// crnti
  uint16_t crnti;
  uint8_t *rrc_container;
  int      rrc_container_length;
  uint8_t *du2cu_rrc_container;
  int      du2cu_rrc_container_length;
} f1ap_initial_ul_rrc_message_t;

typedef struct f1ap_ul_rrc_message_s {
  uint32_t gNB_CU_ue_id;
  uint32_t gNB_DU_ue_id;
  uint8_t  srb_id;
  uint8_t *rrc_container;
  int      rrc_container_length;
} f1ap_ul_rrc_message_t;

typedef struct f1ap_up_tnl_s {
  in_addr_t tl_address; // currently only IPv4 supported
  teid_t  teid;
  uint16_t port;
} f1ap_up_tnl_t;

typedef enum preemption_capability_e {
  SHALL_NOT_TRIGGER_PREEMPTION,
  MAY_TRIGGER_PREEMPTION,
} preemption_capability_t;

typedef enum preemption_vulnerability_e {
  NOT_PREEMPTABLE,
  PREEMPTABLE,
} preemption_vulnerability_t;

typedef struct f1ap_qos_characteristics_s {
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
} f1ap_qos_characteristics_t;

typedef struct f1ap_ngran_allocation_retention_priority_s {
  uint16_t priority_level;
  preemption_capability_t preemption_capability;
  preemption_vulnerability_t preemption_vulnerability;
} f1ap_ngran_allocation_retention_priority_t;

typedef struct f1ap_qos_flow_level_qos_parameters_s {
  f1ap_qos_characteristics_t qos_characteristics;
  f1ap_ngran_allocation_retention_priority_t alloc_reten_priority;
} f1ap_qos_flow_level_qos_parameters_t;

typedef struct f1ap_flows_mapped_to_drb_s {
  long qfi; // qos flow identifier
  f1ap_qos_flow_level_qos_parameters_t qos_params;
} f1ap_flows_mapped_to_drb_t;

typedef struct f1ap_drb_information_s {
  f1ap_qos_flow_level_qos_parameters_t drb_qos;
  f1ap_flows_mapped_to_drb_t *flows_mapped_to_drb;
  uint8_t flows_to_be_setup_length;
} f1ap_drb_information_t;

typedef struct f1ap_drb_to_be_setup_s {
  long           drb_id;
  f1ap_up_tnl_t  up_ul_tnl[2];
  uint8_t        up_ul_tnl_length;
  f1ap_up_tnl_t  up_dl_tnl[2];
  uint8_t        up_dl_tnl_length;
  f1ap_drb_information_t drb_info;
  rlc_mode_t     rlc_mode;
  nssai_t nssai;
} f1ap_drb_to_be_setup_t;

typedef struct f1ap_srb_to_be_setup_s {
  long           srb_id;
  uint8_t        lcid;
} f1ap_srb_to_be_setup_t;

typedef struct f1ap_rb_failed_to_be_setup_s {
  long           rb_id;
} f1ap_rb_failed_to_be_setup_t;

typedef struct f1ap_drb_to_be_released_t {
  long rb_id;
} f1ap_drb_to_be_released_t;

typedef struct cu_to_du_rrc_information_s {
  uint8_t * cG_ConfigInfo;
  uint32_t   cG_ConfigInfo_length;
  uint8_t * uE_CapabilityRAT_ContainerList;
  uint32_t   uE_CapabilityRAT_ContainerList_length;
  uint8_t * measConfig;
  uint32_t   measConfig_length;
}cu_to_du_rrc_information_t;

typedef struct du_to_cu_rrc_information_s {
  uint8_t * cellGroupConfig;
  uint32_t  cellGroupConfig_length;
  uint8_t * measGapConfig;
  uint32_t  measGapConfig_length;
  uint8_t * requestedP_MaxFR1;
  uint32_t  requestedP_MaxFR1_length;
}du_to_cu_rrc_information_t;

typedef enum QoS_information_e {
  NG_RAN_QoS    = 0,
  EUTRAN_QoS    = 1,
} QoS_information_t;

typedef enum ReconfigurationCompl_e {
  RRCreconf_info_not_present = 0,
  RRCreconf_failure          = 1,
  RRCreconf_success          = 2,
} ReconfigurationCompl_t;

typedef struct f1ap_ue_context_setup_s {
  uint32_t gNB_CU_ue_id;
  uint32_t gNB_DU_ue_id;
  // SpCell Info
  f1ap_plmn_t plmn;
  uint64_t nr_cellid;
  uint8_t servCellIndex;
  uint8_t *cellULConfigured;
  uint32_t servCellId;
  cu_to_du_rrc_information_t *cu_to_du_rrc_information;
  uint8_t  cu_to_du_rrc_information_length;
  //uint8_t *du_to_cu_rrc_information;
  du_to_cu_rrc_information_t *du_to_cu_rrc_information;
  uint32_t  du_to_cu_rrc_information_length;
  f1ap_drb_to_be_setup_t *drbs_to_be_setup;
  uint8_t  drbs_to_be_setup_length;
  f1ap_drb_to_be_setup_t *drbs_to_be_modified;
    uint8_t  drbs_to_be_modified_length;
  QoS_information_t QoS_information_type;
  uint8_t  drbs_failed_to_be_setup_length;
  f1ap_rb_failed_to_be_setup_t *drbs_failed_to_be_setup;
  f1ap_srb_to_be_setup_t *srbs_to_be_setup;
  uint8_t  srbs_to_be_setup_length;
  uint8_t  srbs_failed_to_be_setup_length;
  uint8_t drbs_to_be_released_length;
  f1ap_drb_to_be_released_t *drbs_to_be_released;
  f1ap_rb_failed_to_be_setup_t *srbs_failed_to_be_setup;
  ReconfigurationCompl_t ReconfigComplOutcome;
  uint8_t *rrc_container;
  int      rrc_container_length;
} f1ap_ue_context_setup_t, f1ap_ue_context_modif_req_t, f1ap_ue_context_modif_resp_t;

typedef enum F1ap_Cause_e {
  F1AP_CAUSE_NOTHING,  /* No components present */
  F1AP_CAUSE_RADIO_NETWORK,
  F1AP_CAUSE_TRANSPORT,
  F1AP_CAUSE_PROTOCOL,
  F1AP_CAUSE_MISC,
} f1ap_Cause_t;

typedef struct f1ap_ue_context_modif_required_t {
  uint32_t gNB_CU_ue_id;
  uint32_t gNB_DU_ue_id;
  du_to_cu_rrc_information_t *du_to_cu_rrc_information;
  f1ap_Cause_t cause;
  long cause_value;
} f1ap_ue_context_modif_required_t;

typedef struct f1ap_ue_context_modif_confirm_t {
  uint32_t gNB_CU_ue_id;
  uint32_t gNB_DU_ue_id;
  uint8_t *rrc_container;
  int      rrc_container_length;
} f1ap_ue_context_modif_confirm_t;

typedef struct f1ap_ue_context_modif_refuse_t {
  uint32_t gNB_CU_ue_id;
  uint32_t gNB_DU_ue_id;
  f1ap_Cause_t cause;
  long cause_value;
} f1ap_ue_context_modif_refuse_t;

typedef struct f1ap_ue_context_release_s {
  uint32_t gNB_CU_ue_id;
  uint32_t gNB_DU_ue_id;
  f1ap_Cause_t  cause;
  long          cause_value;
  uint8_t      *rrc_container;
  int           rrc_container_length;
  int           srb_id;
} f1ap_ue_context_release_req_t, f1ap_ue_context_release_cmd_t, f1ap_ue_context_release_complete_t;

typedef struct f1ap_paging_ind_s {
  uint16_t ueidentityindexvalue;
  uint64_t fiveg_s_tmsi;
  uint8_t  fiveg_s_tmsi_length;
  f1ap_plmn_t plmn;
  uint64_t nr_cellid;
  uint8_t  paging_drx;
} f1ap_paging_ind_t;

typedef struct f1ap_lost_connection_t {
  int dummy;
} f1ap_lost_connection_t;

/*NRPPA IE*/ // adeel
/* IE structures for Positioning related messages as per TS 38.473 V16.3.1*/
typedef struct nrppa_f1ap_info_s {
  uint32_t nrppa_transaction_id;
  instance_t instance;
  int32_t gNB_ue_ngap_id;
  int64_t amf_ue_ngap_id;
  uint16_t ue_rnti;
  /* routing ID */
  uint8_t *routing_id_buffer;
  uint32_t routing_id_length; /* Length of the octet string */
  // ngap_routing_id_t routing_id;
} nrppa_f1ap_info_t;

typedef enum f1ap_type_of_error_t { F1AP_NOT_UNDERSTOOD = 0, F1AP_MISSING = 1 } f1ap_type_of_error_t;

typedef struct f1ap_criticality_diagnostics_s { // IE 9.3.1.3 (TS 38.473 V16.3.1)
  uint16_t ie_id;
  uint16_t ie_criticality;
  f1ap_type_of_error_t type_of_error;

  /*F1AP_ProcedureCode_t	*procedureCode;	// OPTIONAL
 F1AP_TriggeringMessage_t	*triggeringMessage;	// OPTIONAL
 F1AP_Criticality_t	*procedureCriticality;	// OPTIONAL
 F1AP_TransactionID_t	*transactionID;	 //OPTIONAL */
} f1ap_criticality_diagnostics_t;

typedef enum f1ap_cause_e {
  f1ap_cause_nothing, /* No components present */
  f1ap_cause_radio_network,
  f1ap_cause_transport,
  f1ap_cause_protocol,
  f1ap_cause_misc,
} f1ap_cause_pr;

typedef union f1ap_cause_c {
  uint8_t radioNetwork; // refer to TS 38.473 V16.3.1
  uint8_t transport; // refer to TS 38.473 V16.3.1
  uint8_t protocol; // refer to TS 38.473 V16.3.1
  uint8_t misc; // refer to TS 38.473 V16.3.1
} f1ap_cause_u;

typedef struct f1ap_cause_s {
  f1ap_cause_pr present;
  f1ap_cause_u choice;
} f1ap_cause_t;

typedef union f1ap_bandwidth_srs_c {
  uint8_t fR1; // (M)
  uint8_t fR2; // (M)
} f1ap_bandwidth_srs_u;

typedef enum f1ap_bandwidth_srs_e {
  f1ap_bandwidth_srs_pr_nothing,
  f1ap_bandwidth_srs_pr_fR1,
  f1ap_bandwidth_srs_pr_fR2
} f1ap_bandwidth_srs_pr;

typedef struct f1ap_bandwidth_srs_s {
  f1ap_bandwidth_srs_pr present;
  f1ap_bandwidth_srs_u choice;
} f1ap_bandwidth_srs_t;

typedef struct f1ap_scs_specific_carrier_s {
  uint32_t offsetToCarrier; // (M)
  uint8_t subcarrierSpacing; // (M) kHz15	= 0, kHz30	= 1, kHz60	= 2, kHz120	= 3
  uint16_t carrierBandwidth; // (M)
} f1ap_scs_specific_carrier_t;

typedef struct f1ap_uplink_channel_bw_per_scs_list_s { // A_SEQUENCE_OF(struct F1AP_SCS_SpecificCarrier) list;
  f1ap_scs_specific_carrier_t *scs_specific_carrier;
  uint32_t scs_specific_carrier_list_length;
} f1ap_uplink_channel_bw_per_scs_list_t;

typedef enum f1ap_transmission_comb_e {
  f1ap_transmission_comb_pr_nothing, /* No components present */
  f1ap_transmission_comb_pr_n2,
  f1ap_transmission_comb_pr_n4
} f1ap_transmission_comb_pr;

typedef struct f1ap_transmission_comb_n2_s {
  uint8_t combOffset_n2; // (M) range (0,1)
  uint8_t cyclicShift_n2; // (M) range (0,1,...7)
} f1ap_transmission_comb_n2_t, f1ap_transmission_comb_pos_n2_t;

typedef struct f1ap_transmission_comb_n4_s {
  uint8_t combOffset_n4; // (M) (0,1,2,3)
  uint8_t cyclicShift_n4; // (M) (0,1,...11)
} f1ap_transmission_comb_n4_t, f1ap_transmission_comb_pos_n4_t;

typedef union f1ap_transmission_comb_c {
  f1ap_transmission_comb_n2_t n2;
  f1ap_transmission_comb_n4_t n4;
} f1ap_transmission_comb_u;

typedef struct f1ap_transmission_comb_s {
  f1ap_transmission_comb_pr present;
  f1ap_transmission_comb_u choice;
} f1ap_transmission_comb_t;

typedef struct f1ap_resource_type_periodic_s {
  uint8_t periodicity; // slot1= 0, slot2= 1, slot4= 2, slot5= 3, slot8= 4, slot10= 5, slot16= 6, slot20= 7, slot32= 8, slot40= 9,
                       // slot64= 10, slot80= 11,slot160= 12, slot320= 13,slot640= 14, slot1280= 15, slot2560= 16
  uint16_t offset;
} f1ap_resource_type_periodic_t, f1ap_resource_type_semi_persistent_t;

typedef struct f1ap_resource_type_aperiodic_s {
  uint8_t aperiodicResourceType; // true	= 0
} f1ap_resource_type_aperiodic_t;

typedef union f1ap_resource_type_c {
  f1ap_resource_type_periodic_t periodic;
  f1ap_resource_type_semi_persistent_t semi_persistent;
  f1ap_resource_type_aperiodic_t aperiodic;
} f1ap_resource_type_u;

typedef enum f1ap_resource_type_e {
  f1ap_resource_type_pr_nothing, /* No components present */
  f1ap_resource_type_pr_periodic,
  f1ap_resource_type_pr_semi_persistent,
  f1ap_resource_type_pr_aperiodic,
} f1ap_resource_type_pr;

typedef struct f1ap_resource_type_s {
  f1ap_resource_type_pr present;
  f1ap_resource_type_u choice;
} f1ap_resource_type_t;

typedef struct f1ap_transmission_comb_n8_s {
  uint8_t combOffset_n8; // (M) range (0,1,...7)
  uint8_t cyclicShift_n8; // (M) range (0,1,...5)
} f1ap_transmission_comb_pos_n8_t;

typedef union f1ap_transmission_comb_pos_c {
  f1ap_transmission_comb_pos_n2_t n2;
  f1ap_transmission_comb_pos_n4_t n4;
  f1ap_transmission_comb_pos_n8_t n8;
} f1ap_transmission_comb_pos_u;

typedef enum f1ap_transmission_comb_pos_e {
  f1ap_transmission_comb_pos_pr_NOTHING,
  f1ap_transmission_comb_pos_pr_n2,
  f1ap_transmission_comb_pos_pr_n4,
  f1ap_transmission_comb_pos_pr_n8
} f1ap_transmission_comb_pos_pr;

typedef struct f1ap_transmission_comb_pos_s {
  f1ap_transmission_comb_pos_pr present;
  f1ap_transmission_comb_pos_u choice;
} f1ap_transmission_comb_pos_t;

typedef struct f1ap_ssb_pos_s {
  uint16_t pCI_NR; // (O)
  uint8_t ssb_index; //(M)
} f1ap_ssb_pos_t;

typedef struct f1ap_prs_information_pos_s {
  uint8_t pRS_IDPos; /* OPTIONAL */
  uint8_t pRS_Resource_Set_IDPos;
  uint8_t pRS_Resource_IDPos;
} f1ap_prs_information_pos_t;

typedef struct f1ap_resource_type_periodic_pos_s {
  uint16_t periodicity; // slot1= 0, slot2= 1, slot4= 2, slot5= 3, slot8= 4, slot10= 5, slot16= 6, slot20= 7, slot32= 8, slot40= 9,
                        // slot64= 10, slot80= 11,slot160= 12, slot320= 13,slot640= 14, slot1280= 15, slot2560= 16, slot5120= 17,
                        // slot10240= 18, slot20480= 19, slot40960= 20, slot81920= 21
  uint16_t offset;
} f1ap_resource_type_periodic_pos_t, f1ap_resource_type_semi_persistent_pos_t;

typedef struct f1ap_resource_type_aperiodic_pos_s {
  uint8_t slotOffset;
} f1ap_resource_type_aperiodic_pos_t;

typedef union f1ap_resource_type_pos_c {
  f1ap_resource_type_periodic_pos_t periodic;
  f1ap_resource_type_semi_persistent_pos_t semi_persistent;
  f1ap_resource_type_aperiodic_pos_t aperiodic;
} f1ap_resource_type_pos_u;

typedef enum f1ap_resource_type_pos_e {
  f1ap_resource_type_pos_pr_NOTHING,
  f1ap_resource_type_pos_pr_periodic,
  f1ap_resource_type_pos_pr_semi_persistent,
  f1ap_resource_type_pos_pr_aperiodic
} f1ap_resource_type_pos_pr;

typedef struct f1ap_resource_type_pos_s {
  f1ap_resource_type_pos_pr present;
  f1ap_resource_type_pos_u choice;
} f1ap_resource_type_pos_t;

typedef union f1ap_spatial_relation_pos_c {
  f1ap_ssb_pos_t sSBPos;
  f1ap_prs_information_pos_t pRSInformationPos;
} f1ap_spatial_relation_pos_u;

typedef struct f1ap_srs_resource_id_list_s { // A_SEQUENCE_OF(F1AP_SRSResourceID_t) list;
  long *srs_resource_id;
  uint8_t srs_resource_id_list_length; // maximum no of SRS resources per resource set is 16
} f1ap_srs_resource_id_list_t;

typedef struct f1ap_resource_set_type_periodic_s {
  uint8_t periodicSet; // true	= 0
} f1ap_resource_set_type_periodic_t;

typedef struct f1ap_resource_set_type_semi_persistent_s {
  uint8_t semi_persistentSet; // persistentSet_true	= 0
} f1ap_resource_set_type_semi_persistent_t;

typedef struct f1ap_resource_set_type_aperiodic_s {
  uint8_t sRSResourceTrigger; //(M)
  long slotoffset; //(M)
} f1ap_resource_set_type_aperiodic_t;

typedef union f1ap_resource_set_type_c {
  f1ap_resource_set_type_periodic_t periodic;
  f1ap_resource_set_type_semi_persistent_t semi_persistent;
  f1ap_resource_set_type_aperiodic_t aperiodic;
} f1ap_resource_set_type_u;

typedef enum f1ap_resource_set_type_e {
  f1ap_resource_set_type_pr_nothing, /* No components present */
  f1ap_resource_set_type_pr_periodic,
  f1ap_resource_set_type_pr_semi_persistent,
  f1ap_resource_set_type_pr_aperiodic,
} f1ap_resource_set_type_pr;

typedef struct f1ap_resource_set_type_s {
  f1ap_resource_set_type_pr present;
  f1ap_resource_set_type_u choice;
} f1ap_resource_set_type_t;

typedef struct f1ap_pos_srs_resource_id_list_s { // A_SEQUENCE_OF(F1AP_SRSPosResourceID_t) list;
  long *srs_pos_resource_id;
  uint32_t pos_srs_resource_id_list_length;
} f1ap_pos_srs_resource_id_list_t;

typedef struct f1ap_pos_resource_set_type_pr_s {
  uint8_t posperiodicSet; // true	= 0
} f1ap_pos_resource_set_type_pr_t;

typedef struct f1ap_pos_resource_set_type_sp_s {
  uint8_t possemi_persistentSet; // true	= 0
} f1ap_pos_resource_set_type_sp_t;

typedef struct f1ap_pos_resource_set_type_ap_s {
  uint8_t sRSResourceTrigger_List;
} f1ap_pos_resource_set_type_ap_t;

typedef union f1ap_pos_resource_set_type_c {
  f1ap_pos_resource_set_type_pr_t periodic;
  f1ap_pos_resource_set_type_sp_t semi_persistent;
  f1ap_pos_resource_set_type_ap_t aperiodic;
} f1ap_pos_resource_set_type_u;

typedef enum f1ap_pos_resource_set_type_e {
  f1ap_pos_resource_set_type_pr_nothing,
  f1ap_pos_resource_set_type_pr_periodic,
  f1ap_pos_resource_set_type_pr_semi_persistent,
  f1ap_pos_resource_set_type_pr_aperiodic,
} f1ap_pos_resource_set_type_pr;

typedef struct f1ap_pos_resource_set_type_s {
  f1ap_pos_resource_set_type_pr present;
  f1ap_pos_resource_set_type_u choice;
} f1ap_pos_resource_set_type_t;

typedef struct f1ap_srs_resource_s {
  uint32_t sRSResourceID; //(M)
  uint8_t nrofSRS_Ports; //(M) port1	= 0, ports2	= 1, ports4	= 2
  f1ap_transmission_comb_t transmissionComb; // choices
  uint8_t startPosition; //(M)
  uint8_t nrofSymbols; //(M)  n1	= 0, n2	= 1, n4	= 2
  uint8_t repetitionFactor; //(M)  n1	= 0, n2	= 1, n4	= 2
  uint8_t freqDomainPosition; //(M)
  uint16_t freqDomainShift; //(M)
  uint8_t c_SRS; //(M)
  uint8_t b_SRS; //(M)
  uint8_t b_hop; //(M)
  long groupOrSequenceHopping; //(M) neither	= 0, groupHopping	= 1, sequenceHopping	= 2
  f1ap_resource_type_t resourceType; //(M) choice
  uint16_t slotOffset; //(M)
  uint16_t sequenceId; //(M)
} f1ap_srs_resource_t;

typedef struct f1ap_pos_srs_resource_item_s {
  uint32_t srs_PosResourceId; // (M)
  f1ap_transmission_comb_pos_t transmissionCombPos; // (M)
  uint8_t startPosition; // (M)  range (0,1,...13)
  uint8_t nrofSymbols; // (M)  n1	= 0, n2	= 1, n4	= 2, n8	= 3, n12 = 4
  uint16_t freqDomainShift; // (M)
  uint8_t c_SRS; // (M)
  uint8_t groupOrSequenceHopping; // (M)  neither	= 0, groupHopping	= 1, sequenceHopping	= 2
  f1ap_resource_type_pos_t resourceTypePos; // (M)
  uint32_t sequenceId; //(M)
  f1ap_spatial_relation_pos_u spatialRelationPos; /* OPTIONAL */
} f1ap_pos_srs_resource_item_t;

typedef struct f1ap_srs_resource_set_s {
  uint8_t sRSResourceSetID; // (M)
  f1ap_srs_resource_id_list_t sRSResourceID_List; // (M)F1AP_SRSResourceID_List_t	 sRSResourceID_List;
  f1ap_resource_set_type_t resourceSetType; //(M) F1AP_ResourceSetType_t	 resourceSetType;
} f1ap_srs_resource_set_t;

typedef struct f1ap_pos_srs_resource_set_item_s {
  uint8_t possrsResourceSetID; // (M)
  f1ap_pos_srs_resource_id_list_t possRSResourceID_List; // F1AP_PosSRSResourceID_List_t	 possRSResourceID_List;
  f1ap_pos_resource_set_type_t posresourceSetType; // F1AP_PosResourceSetType_t	 posresourceSetType;
} f1ap_pos_srs_resource_set_item_t;

// IE for srs_config
typedef struct f1ap_srs_resource_list_s { // A_SEQUENCE_OF(struct F1AP_SRSResource) list;
  f1ap_srs_resource_t *srs_resource;
  uint32_t srs_resource_list_length;
} f1ap_srs_resource_list_t;

typedef struct f1ap_pos_srs_resource_list_s { // A_SEQUENCE_OF(struct F1AP_PosSRSResource_Item) list;
  f1ap_pos_srs_resource_item_t *pos_srs_resource_item;
  uint32_t pos_srs_resource_list_length;
} f1ap_pos_srs_resource_list_t;

typedef struct f1ap_srs_resource_set_list_s { // A_SEQUENCE_OF(struct F1AP_SRSResourceSet) list;
  f1ap_srs_resource_set_t *srs_resource_set;
  uint32_t srs_resource_set_list_length; //
} f1ap_srs_resource_set_list_t;

typedef struct f1ap_pos_srs_resource_set_list_s { // A_SEQUENCE_OF(struct F1AP_PosSRSResourceSet_Item) list;
  f1ap_pos_srs_resource_set_item_t *pos_srs_resource_set_item;
  uint32_t pos_srs_resource_set_list_length;
} f1ap_pos_srs_resource_set_list_t;

typedef struct f1ap_srs_config_s {
  f1ap_srs_resource_list_t sRSResource_List; // (O) A_SEQUENCE_OF(struct F1AP_SRSResource) list;
  f1ap_pos_srs_resource_list_t posSRSResource_List; // (O)
  f1ap_srs_resource_set_list_t sRSResourceSet_List; /* OPTIONAL */ // A_SEQUENCE_OF(struct F1AP_SRSResourceSet) list;
  f1ap_pos_srs_resource_set_list_t posSRSResourceSet_List; /* OPTIONAL */ // A_SEQUENCE_OF(struct F1AP_PosSRSResourceSet_Item) list;
} f1ap_srs_config_t;

typedef struct f1ap_active_ul_bwp_s {
  uint32_t locationAndBandwidth; //  (M)
  uint8_t subcarrierSpacing; // (M) kHz15	= 0, kHz30	= 1, kHz60	= 2, kHz120	= 3
  uint8_t cyclicPrefix; //(M) normal	= 0, extended	= 1
  uint32_t txDirectCurrentLocation;
  uint8_t shift7dot5kHz; // (O)
  f1ap_srs_config_t sRSConfig;
} f1ap_active_ul_bwp_t;

typedef struct f1ap_srs_carrier_list_item_s {
  uint32_t pointA; // (M)
  f1ap_uplink_channel_bw_per_scs_list_t uplink_channel_bw_per_scs_list; // A_SEQUENCE_OF(struct F1AP_SCS_SpecificCarrier) list;
  f1ap_active_ul_bwp_t active_ul_bwp; //(M)
  uint16_t pci; // (O)
} f1ap_srs_carrier_list_item_t;

typedef struct f1ap_srs_carrier_list_s {
  f1ap_srs_carrier_list_item_t *srs_carrier_list_item; // A_SEQUENCE_OF(struct F1AP_SRSCarrier_List_Item) list;
  uint32_t srs_carrier_list_length;
} f1ap_srs_carrier_list_t;

typedef struct f1ap_periodicity_list_item_s {
  uint8_t
      periodicitySRS; // milli seconds(ms) 0.125= 0, 0.25=1,
                      // 0.5=2,0.625=3,1=4,1.25=5,2=6,2.5=7,4=8,5=9,8=10,ms10=11,ms16=12,20=13,32=14,40=15,64=16,80=17,160=18,320=19,640=20,1280=
                      // 21,2560=22,5120=23,10240=24
} f1ap_periodicity_list_item_t;

typedef struct f1ap_periodicity_list_s { // A_SEQUENCE_OF(struct F1AP_PeriodicityList_Item) list;
  f1ap_periodicity_list_item_t *f1ap_periodicity_list_item;
  uint32_t periodicity_list_length;
} f1ap_periodicity_list_t;

typedef struct f1ap_nzp_csi_rs_resource_id_s {
  uint8_t nzp_csi_rs_resource_id;
} f1ap_nzp_csi_rs_resource_id_t;

typedef struct f1ap_ssb_s {
  uint16_t pCI_NR;
  uint8_t ssb_index;
} f1ap_ssb_t;

typedef struct f1ap_srs_resource_id_s {
  uint8_t srs_resource_id;
} f1ap_srs_resource_id_t;

typedef struct f1ap_srs_pos_resource_id_s {
  uint8_t srs_pos_resource_id;
} f1ap_srs_pos_resource_id_t;

typedef struct f1ap_dl_prs_s {
  uint8_t prsid;
  uint8_t dl_PRSResourceSetID;
  uint8_t dl_PRSResourceID; // OPTIONAL
} f1ap_dl_prs_t;

typedef union f1ap_reference_signal_c {
  f1ap_nzp_csi_rs_resource_id_t nZP_CSI_RS; // typedef long	 F1AP_NZP_CSI_RS_ResourceID_t;
  f1ap_ssb_t sSB;
  f1ap_srs_resource_id_t sRS;
  f1ap_srs_pos_resource_id_t positioningSRS;
  f1ap_dl_prs_t dL_PRS;
} f1ap_reference_signal_u;

typedef struct f1ap_spatial_relation_for_resource_id_item_s {
  f1ap_reference_signal_u referenceSignal;
} f1ap_spatial_relation_for_resource_id_item_t;

typedef struct f1ap_spatial_relation_for_resource_id_s { // A_SEQUENCE_OF(struct F1AP_SpatialRelationforResourceIDItem) list;
  f1ap_spatial_relation_for_resource_id_item_t *f1ap_spatial_relation_for_resource_id_item;
  uint32_t f1ap_spatial_relation_for_resource_id_length;
} f1ap_spatial_relation_for_resource_id_t;

typedef struct f1ap_spatial_relation_info_s { //
  f1ap_spatial_relation_for_resource_id_t spatialRelationforResourceID;
} f1ap_spatial_relation_info_t;

typedef union f1ap_pathloss_reference_signal_c {
  f1ap_ssb_t sSB;
  f1ap_dl_prs_t dL_PRS;
} f1ap_pathloss_reference_signal_u;

typedef struct f1ap_pathloss_reference_info_s {
  f1ap_pathloss_reference_signal_u pathlossReferenceSignal;
} f1ap_pathloss_reference_info_t;

typedef struct f1ap_srs_resource_set_item_s {
  uint8_t numSRSresourcesperset; // (O)
  f1ap_periodicity_list_t periodicityList; // (O)
  f1ap_spatial_relation_info_t spatialRelationInfo; // (O)
  f1ap_pathloss_reference_info_t pathlossReferenceInfo; // (O)
} f1ap_srs_resource_set_item_t;

typedef struct f1ap_srs_resource_set__list_s { // A_SEQUENCE_OF(struct F1AP_SRSResourceSetItem) list;
  f1ap_srs_resource_set_item_t *srs_resource_set_item;
  uint32_t srs_resource_set_list_length;
} f1ap_srs_resource_set__list_t;

typedef struct bit_string_s {
  uint8_t *buf; // BIT STRING body */
  size_t size; // Size of the above buffer */
  int bits_unused; // Unused trailing bits in the last octet (0..7)
} bit_string_t;

typedef union f1ap_ssb_positions_in_burst_c {
  bit_string_t shortBitmap;
  bit_string_t mediumBitmap;
  bit_string_t longBitmap;
} f1ap_ssb_positions_in_burst_u;

typedef struct f1ap_ssb_tf_configuration_s { // IE 9.3.1.203 (TS 38.473 V16.3.1)
  uint64_t sSB_frequency;
  uint8_t sSB_subcarrier_spacing; //	kHz15= 0, kHz30	= 1, kHz60	= 2, kHz120	= 3, kHz240	= 4
  int8_t sSB_Transmit_power;
  uint8_t sSB_periodicity; //	 ms5= 0, ms10= 1, ms20= 2, ms40= 3, ms80= 4, ms160= 5
  uint8_t sSB_half_frame_offset;
  uint8_t sSB_SFN_offset;
  f1ap_ssb_positions_in_burst_u sSB_position_in_burst; /* OPTIONAL */
  bit_string_t sFNInitialisationTime; /* OPTIONAL */
} f1ap_ssb_tf_configuration_t;

typedef struct f1ap_ssb_information_item_s {
  f1ap_ssb_tf_configuration_t sSB_Configuration;
  uint16_t pCI_NR;
} f1ap_ssb_information_item_t;

typedef struct f1ap_ssb_information_list_s { // A_SEQUENCE_OF(struct F1AP_SSBInformationItem) list;
  f1ap_ssb_information_item_t *ssb_information_item;
  uint32_t ssb_information_list_length;
} f1ap_ssb_information_list_t;

typedef struct f1ap_ssb_information_s { // IE 9.3.1.202 (TS 38.473 V16.3.1)
  f1ap_ssb_information_list_t sSBInformationList;
} f1ap_ssb_information_t;

typedef struct f1ap_requested_SRS_transmission_characteristics_s {
  long numberOfTransmissions; // (O) no of periodic transmission, 0= infinite no of SRS transmission, Applicable only if
                              // Resource type IE is periodic
  uint8_t resourceType; // (M)
  f1ap_bandwidth_srs_t bandwidth_srs; // (M)
  f1ap_srs_resource_set__list_t sRSResourceSetList; // (O) A_SEQUENCE_OF(struct F1AP_SRSResourceSetItem) list;
  f1ap_ssb_information_t sSBInformation; // OPTIONAL*/

  // TODO optional
  /*  struct F1AP_SRSResourceSetList	*sRSResourceSetList;	// OPTIONAL
    struct F1AP_SSBInformation	*sSBInformation;	// OPTIONAL*/
} f1ap_requested_SRS_transmission_characteristics_t;

typedef struct f1ap_srs_configuration_s { // IE 9.3.1.192 (TS 38.473 V16.3.1)
  f1ap_srs_carrier_list_t srs_carrier_list;
} f1ap_srs_configuration_t;

typedef struct f1ap_semi_persistent_srs_s {
  uint8_t sRSResourceSetID; // (M)
  f1ap_spatial_relation_info_t sRSSpatialRelation; /* OPTIONAL */
} f1ap_semi_persistent_srs_t;

typedef struct f1ap_aperiodic_srs_resource_trigger_list_s { // A_SEQUENCE_OF(F1AP_AperiodicSRSResourceTrigger_t) list;
  uint8_t *AperiodicSRSResourceTrigger;
  uint32_t aperiodic_srs_resource_trigger_list_length;
} f1ap_aperiodic_srs_resource_trigger_list_t;

typedef struct f1ap_srs_resource_trigger_s { // IE 9.3.1.182 (TS 38.473 V16.3.1)
  f1ap_aperiodic_srs_resource_trigger_list_t aperiodicSRSResourceTriggerList;
} f1ap_srs_resource_trigger_t;

typedef struct f1ap_aperiodic_srs_c {
  uint8_t aperiodic; // true	= 0
  f1ap_srs_resource_trigger_t *sRSResourceTrigger; /* OPTIONAL */
} f1ap_aperiodic_srs_t;

typedef union f1ap_srs_type_c {
  f1ap_semi_persistent_srs_t semipersistentSRS;
  f1ap_aperiodic_srs_t aperiodicSRS;
} f1ap_srs_type_u;

typedef enum f1ap_srs_type_e {
  f1ap_srs_type_pr_NOTHING, /* No components present */
  f1ap_srs_type_pr_semipersistentSRS,
  f1ap_srs_type_pr_aperiodicSRS
} f1ap_srs_type_pr;

typedef struct f1ap_srs_type_s {
  f1ap_srs_type_pr present;
  f1ap_srs_type_u choice;
} f1ap_srs_type_t;

typedef union f1ap_abort_transmission_c {
  uint8_t sRSResourceSetID; // (M)
  uint32_t releaseALL;
} f1ap_abort_transmission_u;

typedef enum f1ap_abort_transmission_e {
  f1ap_abort_transmission_pr_NOTHING,
  f1ap_abort_transmission_pr_sRSResourceSetID,
  f1ap_abort_transmission_pr_releaseALL
} f1ap_abort_transmission_pr;

typedef struct f1ap_abort_transmission_s {
  f1ap_abort_transmission_pr present;
  f1ap_abort_transmission_u choice;
} f1ap_abort_transmission_t;

typedef struct f1ap_trp_list_item_s {
  uint32_t tRPID;
} f1ap_trp_list_item_t;

typedef struct f1ap_trp_list_s {
  // A_SEQUENCE_OF(struct F1AP_TRPListItem) list; //
  f1ap_trp_list_item_t *TRPListItem;
  uint32_t trp_list_length;
} f1ap_trp_list_t;

typedef struct f1ap_trp_information_type_item_s {
  uint8_t TRPInformationTypeItem; // nrPCI	= 0, nG_RAN_CGI	= 1, arfcn= 2, pRSConfig= 3, sSBConfig= 4, sFNInitTime= 5,
                                  // spatialDirectInfo= 6, geoCoord= 7
} f1ap_trp_information_type_item_t;

typedef struct f1ap_trp_information_type_list_s {
  // A_SEQUENCE_OF(struct F1AP_ProtocolIE_SingleContainer) list;
  f1ap_trp_information_type_item_t *trp_information_type_item;
  uint32_t trp_information_type_list_length;
} f1ap_trp_information_type_list_t;

typedef struct octet_string_s {
  uint8_t *buf; /* Buffer with consecutive OCTET_STRING bits */
  size_t size; /* Size of the buffer */
} octet_string_t;

typedef struct f1ap_nr_cgi_s { // IE 9.3.1.12 TS 38.473 V16.3.1
  octet_string_t pLMN_Identity; // typedef OCTET_STRING_t	 F1AP_PLMN_Identity_t;
  bit_string_t nRCellIdentity; // typedef BIT_STRING_t	 F1AP_NRCellIdentity_t;
} f1ap_nr_cgi_t;

typedef struct f1ap_prs_configuration_s { // IE 9.3.1.177 TS 38.473 V16.3.1
  // todo
  int TODO;
} f1ap_prs_configuration_t;

typedef struct f1ap_nr_prs_beam_information_s { // IE 9.3.1.198 TS 38.473 V16.3.1
  // F1AP_NR_PRSBeamInformationList_t	 nR_PRSBeamInformationList;
  // F1AP_LCStoGCSTranslationList_t	 lCStoGCSTranslationList;
  // todo
  int TODO_LATER;
} f1ap_nr_prs_beam_information_t;

typedef struct f1ap_spatial_direction_information_s { // IE 9.3.1.179 TS 38.473 V16.3.1
  f1ap_nr_prs_beam_information_t nR_PRSBeamInformation;
} f1ap_spatial_direction_information_t;

typedef struct f1ap_access_point_position_s { // IE 9.3.1.174 TS 38.473 V16.3.1
  long	 latitudeSign;
	long	 latitude;
	long	 longitude;
	long	 directionOfAltitude;
	long	 altitude;
	long	 uncertaintySemi_major;
	long	 uncertaintySemi_minor;
	long	 orientationOfMajorAxis;
	long	 uncertaintyAltitude;
	long	 confidence;
} f1ap_access_point_position_t;

typedef struct f1ap_ngran_high_accuracy_access_point_position_s { // IE 9.3.1.190 TS 38.473 V16.3.1
  long	 latitude;
	long	 longitude;
	long	 altitude;
	long	 uncertaintySemi_major;
	long	 uncertaintySemi_minor;
	long	 orientationOfMajorAxis;
	long	 horizontalConfidence;
	long	 uncertaintyAltitude;
	long	 verticalConfidence;
} f1ap_ngran_high_accuracy_access_point_position_t;

typedef union f1ap_trp_position_direct_accuracy_c {
  f1ap_access_point_position_t tRPPosition;
  f1ap_ngran_high_accuracy_access_point_position_t tRPHAposition;
} f1ap_trp_position_direct_accuracy_u;

typedef enum f1ap_trp_position_direct_accuracy_e {
  f1ap_trp_position_direct_accuracy_pr_NOTHING,
  f1ap_trp_position_direct_accuracy_pr_tRPPosition,
  f1ap_trp_position_direct_accuracy_pr_tRPHAposition
} f1ap_trp_position_direct_accuracy_pr;

typedef struct f1ap_trp_position_direct_accuracy_s {
  f1ap_trp_position_direct_accuracy_pr present;
  f1ap_trp_position_direct_accuracy_u choice;
} f1ap_trp_position_direct_accuracy_t;

typedef struct f1ap_trp_position_direct_s {
  f1ap_trp_position_direct_accuracy_t accuracy;
} f1ap_trp_position_direct_t;

typedef enum f1ap_reference_point_e {
	f1ap_reference_point_pr_NOTHING,
	f1ap_reference_point_pr_coordinateID,
	f1ap_reference_point_pr_referencePointCoordinate,
	f1ap_reference_point_pr_referencePointCoordinateHA
} f1ap_reference_point_pr;

typedef union f1ap_reference_point_c {
	long	 coordinateID;
	f1ap_access_point_position_t	referencePointCoordinate;
	f1ap_ngran_high_accuracy_access_point_position_t	referencePointCoordinateHA;
} f1ap_reference_point_u;

typedef struct f1ap_reference_point_s { // IE 9.3.1.188 TS 38.473 V16.3.1
  f1ap_reference_point_pr present;
  f1ap_reference_point_u choice;
} f1ap_reference_point_t;

typedef struct f1ap_location_uncertainty_s{
  long	 horizontalUncertainty;
	long	 horizontalConfidence;
	long	 verticalUncertainty;
	long	 verticalConfidence;
}f1ap_location_uncertainty_t;

typedef struct f1ap_relative_geodetic_location_s { // IE 9.3.1.186 TS 38.473 V16.3.1
  long	 milli_Arc_SecondUnits;
	long	 heightUnits;
	long	 deltaLatitude;
	long	 deltaLongitude;
	long	 deltaHeight;
	f1ap_location_uncertainty_t	 locationUncertainty;
} f1ap_relative_geodetic_location_t;

typedef struct f1ap_relative_cartesian_location_s { // IE 9.3.1.187 TS 38.473 V16.3.1
  long	 xYZunit;
	long	 xvalue;
	long	 yvalue;
	long	 zvalue;
  f1ap_location_uncertainty_t	 locationUncertainty;
} f1ap_relative_cartesian_location_t;

typedef union f1ap_trp_reference_point_type_c {
  f1ap_relative_geodetic_location_t tRPPositionRelativeGeodetic;
  f1ap_relative_cartesian_location_t tRPPositionRelativeCartesian;
} f1ap_trp_reference_point_type_u;

typedef enum f1ap_trp_reference_point_type_e {
  f1ap_trp_reference_point_type_pr_NOTHING,
  f1ap_trp_reference_point_type_pr_tRPPositionRelativeGeodetic,
  f1ap_trp_reference_point_type_pr_tRPPositionRelativeCartesian
} f1ap_trp_reference_point_type_pr;

typedef struct f1ap_trp_reference_point_type_t {
  f1ap_trp_reference_point_type_pr present;
  f1ap_trp_reference_point_type_u choice;
} f1ap_trp_reference_point_type_t;

typedef struct f1ap_trp_position_referenced_t {
  f1ap_reference_point_t referencePoint;
  f1ap_trp_reference_point_type_t referencePointType;
} f1ap_trp_position_referenced_t;

typedef union f1ap_trp_position_definition_type_c {
  f1ap_trp_position_direct_t direct;
  f1ap_trp_position_referenced_t referenced;
} f1ap_trp_position_definition_type_u;

typedef enum f1ap_trp_position_definition_type_e {
  f1ap_trp_position_definition_type_pr_NOTHING,
  f1ap_trp_position_definition_type_pr_direct,
  f1ap_trp_position_definition_type_pr_referenced
} f1ap_trp_position_definition_type_pr;

typedef struct f1ap_trp_position_definition_type_s {
f1ap_trp_position_definition_type_u choice;
f1ap_trp_position_definition_type_pr present;
} f1ap_trp_position_definition_type_t;

typedef struct f1ap_dl_prs_resource_coordinates_s { // IE 9.3.1.185 TS 38.473 V16.3.1
  // todo
  int TODO_LATER;
} f1ap_dl_prs_resource_coordinates_t;

typedef struct f1ap_geographical_coordinates_s { // IE 9.3.1.184 TS 38.473 V16.3.1
  f1ap_trp_position_definition_type_t tRPPositionDefinitionType;
  f1ap_dl_prs_resource_coordinates_t dLPRSResourceCoordinates; // OPTIONAL
} f1ap_geographical_coordinates_t;

typedef union f1ap_trp_information_type_response_item_c {
  uint16_t pCI_NR;
  f1ap_nr_cgi_t nG_RAN_CGI;
  uint32_t nRARFCN;
  f1ap_prs_configuration_t pRSConfiguration;
  f1ap_ssb_information_t sSBinformation;
  bit_string_t sFNInitialisationTime;
  f1ap_spatial_direction_information_t spatialDirectionInformation;
  f1ap_geographical_coordinates_t geographicalCoordinates;
} f1ap_trp_information_type_response_item_u;

typedef enum f1ap_trp_information_type_response_item_e {
  f1ap_trp_information_type_response_item_pr_NOTHING,
  f1ap_trp_information_type_response_item_pr_pCI_NR,
  f1ap_trp_information_type_response_item_pr_nG_RAN_CGI,
  f1ap_trp_information_type_response_item_pr_nRARFCN,
  f1ap_trp_information_type_response_item_pr_pRSConfiguration,
  f1ap_trp_information_type_response_item_pr_sSBinformation,
  f1ap_trp_information_type_response_item_pr_sFNInitialisationTime,
  f1ap_trp_information_type_response_item_pr_spatialDirectionInformation,
  f1ap_trp_information_type_response_item_pr_geographicalCoordinates
} f1ap_trp_information_type_response_item_pr;

typedef struct f1ap_trp_information_type_response_item_s {
  f1ap_trp_information_type_response_item_pr present;
  f1ap_trp_information_type_response_item_u choice;
} f1ap_trp_information_type_response_item_t;

typedef struct f1ap_trp_information_type_response_list_s { // A_SEQUENCE_OF(struct F1AP_TRPInformationTypeResponseItem) list;
  f1ap_trp_information_type_response_item_t *trp_information_type_response_item;
  uint8_t trp_information_type_response_list_length;
} f1ap_trp_information_type_response_list_t;

typedef struct f1ap_trp_information_s { // IE 9.3.1.176 TS 38.473 V16.3.1
  uint32_t tRPID;
  f1ap_trp_information_type_response_list_t tRPInformationTypeResponseList;
} f1ap_trp_information_t;

typedef struct f1ap_trp_information_item_s {
  f1ap_trp_information_t tRPInformation;
} f1ap_trp_information_item_t;

typedef struct f1ap_trp_information_list_s {
  f1ap_trp_information_item_t *trp_information_item;
  uint32_t trp_information_list_length;
} f1ap_trp_information_list_t;

typedef struct f1ap_search_window_information_s { // IE 9.3.1.204 TS 38.473 V16.3.1
  uint32_t expectedPropagationDelay; //
  uint32_t delayUncertainty;
} f1ap_search_window_information_t;

typedef struct f1ap_trp_measurement_request_item_s {
  uint32_t tRPID;
  f1ap_search_window_information_t search_window_information; /* OPTIONAL */
} f1ap_trp_measurement_request_item_t;

typedef struct f1ap_trp_measurement_request_list_s { // A_SEQUENCE_OF(struct F1AP_TRP_MeasurementRequestItem) list;
  f1ap_trp_measurement_request_item_t *trp_measurement_request_item;
  uint32_t trp_measurement_request_list_length;
} f1ap_trp_measurement_request_list_t;

typedef struct f1ap_pos_measurement_quantities_item_s {
  uint8_t posMeasurementType; // gnb_rx_tx	= 0, ul_srs_rsrp	= 1, ul_aoa	= 2, ul_rtoa	= 3
  uint8_t timingReportingGranularityFactor; // (O)
} f1ap_pos_measurement_quantities_item_t;

typedef struct f1ap_pos_measurement_quantities_s {
  // A_SEQUENCE_OF(struct F1AP_PosMeasurementQuantities_Item) list;
  f1ap_pos_measurement_quantities_item_t *pos_measurement_quantities_item;
  uint32_t f1ap_pos_measurement_quantities_length;
} f1ap_pos_measurement_quantities_t;

typedef struct f1ap_ul_aoa_s { // IE 9.3.1.167 TS 38.473 V16.3.1
  uint16_t azimuthAoA;
  uint16_t zenithAoA; // (O)
  uint8_t angleCoordinateSystem; // angleCoordinateSystem_lCS	= 0, angleCoordinateSystem_gCS	= 1
} f1ap_ul_aoa_t;

typedef union f1ap_relative_path_delay_c {
  uint32_t k0;
  uint32_t k1;
  uint32_t k2;
  uint32_t k3;
  uint32_t k4;
  uint32_t k5;
} f1ap_relative_path_delay_u, f1ap_ul_rtoa_measurement_item_u, f1ap_gnb_rx_tx_time_diff_meas_u;

typedef enum f1ap_gnb_rx_tx_time_diff_meas_e {
  f1ap_gnbrxtxtimediffmeas_pr_NOTHING,
  f1ap_gnbrxtxtimediffmeas_pr_k0,
  f1ap_gnbrxtxtimediffmeas_pr_k1,
  f1ap_gnbrxtxtimediffmeas_pr_k2,
  f1ap_gnbrxtxtimediffmeas_pr_k3,
  f1ap_gnbrxtxtimediffmeas_pr_k4,
  f1ap_gnbrxtxtimediffmeas_pr_k5
} f1ap_gnb_rx_tx_time_diff_meas_pr;

typedef struct f1ap_gnb_rx_tx_time_diff_meas_s {
  f1ap_gnb_rx_tx_time_diff_meas_pr present;
  f1ap_gnb_rx_tx_time_diff_meas_u choice;
} f1ap_gnb_rx_tx_time_diff_meas_t;

typedef struct f1ap_timing_measurement_quality_s {
  uint8_t measurementQuality;
  uint8_t resolution; // resolution_ 0.1m = 0, 1m= 1, 10m= 2, 30m= 3
} f1ap_timing_measurement_quality_t;

typedef struct f1ap_angle_measurement_quality_s {
  uint8_t azimuthQuality;
  uint8_t zenithQuality; // (O)
  long resolution; // resolution_deg0dot1	= 0
} f1ap_angle_measurement_quality_t;

typedef union f1ap_trp_measurement_quality_item_c {
  f1ap_timing_measurement_quality_t timingMeasurementQuality;
  f1ap_angle_measurement_quality_t angleMeasurementQuality;
} f1ap_trp_measurement_quality_item_u;

typedef struct f1ap_trp_measurement_quality_s { // IE 9.3.1.172 TS 38.473 V16.3.1
  f1ap_trp_measurement_quality_item_u tRPmeasurementQuality_Item;
} f1ap_trp_measurement_quality_t;

typedef struct f1ap_additional_path_item_s {
  f1ap_relative_path_delay_u relativePathDelay;
  f1ap_trp_measurement_quality_t pathQuality; /* OPTIONAL */
} f1ap_additional_path_item_t;

typedef struct f1ap_additional_path_list_s { // IE 9.3.1.169 TS 38.473 V16.3.1 //A_SEQUENCE_OF(struct F1AP_AdditionalPath_Item)
                                             // list;
  f1ap_additional_path_item_t *additional_path_item;
  uint32_t additional_path_list_length;
} f1ap_additional_path_list_t;

typedef enum f1ap_ul_rtoa_measurement_item_e {
  f1ap_ulrtoameas_pr_NOTHING, /* No components present */
  f1ap_ulrtoameas_pr_k0,
  f1ap_ulrtoameas_pr_k1,
  f1ap_ulrtoameas_pr_k2,
  f1ap_ulrtoameas_pr_k3,
  f1ap_ulrtoameas_pr_k4,
  f1ap_ulrtoameas_pr_k5
} f1ap_ul_rtoa_measurement_item_pr;

typedef struct f1ap_ul_rtoa_measurement_item_s {
  f1ap_ul_rtoa_measurement_item_pr present;
  f1ap_ul_rtoa_measurement_item_u choice;
} f1ap_ul_rtoa_measurement_item_t;

typedef union f1ap_ul_rtoa_measurement_s { // IE 9.3.1.168 TS 38.473 V16.3.1
  f1ap_ul_rtoa_measurement_item_t uL_RTOA_MeasurementItem;
  f1ap_additional_path_list_t additionalPath_List; /* OPTIONAL */
} f1ap_ul_rtoa_measurement_t;

typedef struct f1ap_gnb_rx_tx_time_diff_s { // IE 9.3.1.170 TS 38.473 V16.3.1
  f1ap_gnb_rx_tx_time_diff_meas_t rxTxTimeDiff;
  f1ap_additional_path_list_t additionalPath_List; // OPTIONAL
} f1ap_gnb_rx_tx_time_diff_t;

typedef union f1ap_measured_results_value_c {
  f1ap_ul_aoa_t uL_AngleOfArrival;
  uint8_t uL_SRS_RSRP;
  f1ap_ul_rtoa_measurement_t uL_RTOA;
  f1ap_gnb_rx_tx_time_diff_t gNB_RxTxTimeDiff;
} f1ap_measured_results_value_u;

typedef enum f1ap_measured_results_value_e {
  f1ap_measured_results_value_pr_nothing, /* No components present */
  f1ap_measured_results_value_pr_ul_angleofarrival,
  f1ap_measured_results_value_pr_ul_srs_rsrp,
  f1ap_measured_results_value_pr_ul_rtoa,
  f1ap_measured_results_value_pr_gnb_rxtxtimediff
} f1ap_measured_results_value_pr;

typedef struct f1ap_measured_results_value_s {
  f1ap_measured_results_value_pr present;
  f1ap_measured_results_value_u choice;
} f1ap_measured_results_value_t;

typedef union f1ap_time_stamp_slot_index_c {
  uint8_t sCS_15;
  uint8_t sCS_30;
  uint8_t sCS_60;
  uint8_t sCS_120;
} f1ap_time_stamp_slot_index_u;

typedef enum f1ap_time_stamp_slot_index_e {
  f1ap_time_stamp_slot_index_pr_NOTHING,
  f1ap_time_stamp_slot_index_pr_sCS_15,
  f1ap_time_stamp_slot_index_pr_sCS_30,
  f1ap_time_stamp_slot_index_pr_sCS_60,
  f1ap_time_stamp_slot_index_pr_sCS_120
} f1ap_time_stamp_slot_index_pr;

typedef struct f1ap_time_stamp_slot_index_s {
  f1ap_time_stamp_slot_index_pr present;
  f1ap_time_stamp_slot_index_u choice;
} f1ap_time_stamp_slot_index_t;

typedef struct f1ap_time_stamp_s { // IE 9.3.1.171 TS 38.473 V16.3.1
  uint16_t systemFrameNumber;
  f1ap_time_stamp_slot_index_t slotIndex;
  bit_string_t measurementTime; // OPTIONAL F1AP_SFNInitialisationTime_t
} f1ap_time_stamp_t;

typedef struct f1ap_measurement_beam_info_t { // IE 9.3.1.173 TS 38.473 V16.3.1
  uint8_t pRS_Resource_ID; // OPTIONAL
  uint8_t pRS_Resource_Set_ID; // OPTIONAL
  uint8_t sSB_Index; // OPTIONAL
} f1ap_measurement_beam_info_t;

typedef struct f1ap_pos_measurement_result_item_s {
  f1ap_measured_results_value_t measuredResultsValue;
  f1ap_time_stamp_t timeStamp;
  f1ap_trp_measurement_quality_t measurementQuality; // (O)
  f1ap_measurement_beam_info_t measurementBeamInfo; // (O)
} f1ap_pos_measurement_result_item_t;

typedef struct f1ap_pos_measurement_result_s { // IE 9.3.1.166 TS 38.473 V16.3.1//A_SEQUENCE_OF(struct
                                               // F1AP_PosMeasurementResultItem) list;
  f1ap_pos_measurement_result_item_t *pos_measurement_result_item;
  uint32_t f1ap_pos_measurement_result_length;
} f1ap_pos_measurement_result_t;

typedef struct f1ap_pos_measurement_result_list_item_s {
  f1ap_pos_measurement_result_t posMeasurementResult;
  uint32_t tRPID;
} f1ap_pos_measurement_result_list_item_t;

typedef struct f1ap_pos_measurement_result_list_s { // A_SEQUENCE_OF(struct F1AP_PosMeasurementResultList_Item) list;
  f1ap_pos_measurement_result_list_item_t *pos_measurement_result_list_item;
  uint32_t pos_measurement_result_list_length;
} f1ap_pos_measurement_result_list_t;

/* Structure of Position Information Transfer related NRPPA messages */
typedef struct f1ap_positioning_information_req_s {
  uint32_t gNB_CU_ue_id; // IE 9.3.1.4 (M)
  uint32_t gNB_DU_ue_id; // IE 9.3.1.5 (M)
  nrppa_f1ap_info_t nrppa_msg_info; // TODO check if it is allowed info needed by DL nrppa info needed for UL nrppa
  f1ap_requested_SRS_transmission_characteristics_t req_SRS_info; // IE 9.3.1.175 (O)
} f1ap_positioning_information_req_t;

typedef struct f1ap_positioning_information_resp_s {
  uint32_t gNB_CU_ue_id; // IE 9.3.1.4 (M)
  uint32_t gNB_DU_ue_id; // IE 9.3.1.5 (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_srs_configuration_t srs_configuration; // IE 9.3.1.192 (0)
  f1ap_criticality_diagnostics_t criticality_diagnostics; // IE 9.3.1.3 (O)
  bit_string_t sfn_initialisation_time; // IE 9.3.1.183 (0)
} f1ap_positioning_information_resp_t;

typedef struct f1ap_positioning_information_failure_s {
  uint32_t gNB_CU_ue_id; // IE 9.3.1.4 (M)
  uint32_t gNB_DU_ue_id; // IE 9.3.1.5 (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_cause_t cause; // IE 9.3.1.2 (M)
  f1ap_criticality_diagnostics_t criticality_diagnostics; //  Criticality_Diagnostics // IE 9.3.1.3 (O)
} f1ap_positioning_information_failure_t;

typedef struct f1ap_positioning_information_update_s {
  uint32_t gNB_CU_ue_id; // IE 9.3.1.4 (M)
  uint32_t gNB_DU_ue_id; // IE 9.3.1.5 (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_srs_configuration_t srs_configuration; // IE 9.3.1.192 (0)
  bit_string_t sfn_initialisation_time; // IE 9.3.1.183 (0)
} f1ap_positioning_information_update_t;

typedef struct f1ap_positioning_activation_req_s {
  uint32_t gNB_CU_ue_id; // IE 9.3.1.4 (M)
  uint32_t gNB_DU_ue_id; // IE 9.3.1.5 (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_srs_type_t srs_type;
  bit_string_t activation_time; // type sfn_initialisation_time
} f1ap_positioning_activation_req_t;

typedef struct f1ap_positioning_activation_resp_s {
  uint32_t gNB_CU_ue_id; // IE 9.3.1.4 (M)
  uint32_t gNB_DU_ue_id; // IE 9.3.1.5 (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  uint32_t system_frame_number; // (O)
  uint32_t slot_number; // (O)
  f1ap_criticality_diagnostics_t criticality_diagnostics; //  Criticality_Diagnostics // IE 9.3.1.3 (O)
} f1ap_positioning_activation_resp_t;

typedef struct f1ap_positioning_activation_failure_s {
  uint32_t gNB_CU_ue_id; // IE 9.3.1.4 (M)
  uint32_t gNB_DU_ue_id; // IE 9.3.1.5 (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_cause_t cause; // IE 9.3.1.2 (M)
  f1ap_criticality_diagnostics_t criticality_diagnostics; //  Criticality_Diagnostics // IE 9.3.1.3 (O)
} f1ap_positioning_activation_failure_t;

typedef struct f1ap_positioning_deactivation_s {
  uint32_t gNB_CU_ue_id; // IE 9.3.1.4 (M)
  uint32_t gNB_DU_ue_id; // IE 9.3.1.5 (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_abort_transmission_t abort_transmission;
} f1ap_positioning_deactivation_t;

/* Structure of TRP Information Transfer related NRPPA messages */
typedef struct f1ap_trp_information_req_s {
  uint8_t transaction_id; // IE 9.3.1.23 (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_trp_list_t trp_list; // (M)
  f1ap_trp_information_type_list_t trp_information_type_list; // (M)
} f1ap_trp_information_req_t;

typedef struct f1ap_trp_information_resp_s {
  uint8_t transaction_id; // IE 9.3.1.23 (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_trp_information_list_t trp_information_list; // (M)
  f1ap_criticality_diagnostics_t criticality_diagnostics; //  Criticality_Diagnostics // IE 9.3.1.3 (O)
} f1ap_trp_information_resp_t;

typedef struct f1ap_trp_information_failure_s {
  uint8_t transaction_id; // IE 9.3.1.23 (M)
  f1ap_cause_t cause; // IE 9.3.1.2 (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_criticality_diagnostics_t criticality_diagnostics; // IE 9.3.1.3 (O)
} f1ap_trp_information_failure_t;

/* Structure of Measurement Information Transfer related NRPPA messages */
typedef struct f1ap_measurement_req_s {
  uint8_t transaction_id; // IE 9.3.1.23 (M)
  uint16_t lmf_measurement_id; // (M)
  uint16_t ran_measurement_id; // (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_trp_measurement_request_list_t trp_measurement_request_list; //(M)
  uint8_t pos_report_characteristics; // (M) //	ondemand	= 0, periodic	= 1
  uint8_t pos_measurement_periodicity; //(C) if report characteristics periodic	ms120=0, ms240=1, ms480=2, ms640=3, ms1024=4, ms2048
                                       //=5, ms5120=6, ms10240=7, min1=8, min6= 9,min12= 10,min30= 11,min60= 12
  f1ap_pos_measurement_quantities_t pos_measurement_quantities; // (M)
  bit_string_t sfn_initialisation_time; // IE 9.3.1.183 (0)
  f1ap_srs_configuration_t srs_configuration; // IE 9.3.1.192 (0)
  uint8_t measurement_beam_info_request; // (O) //true	= 0
  uint16_t system_frame_number; // (O)
  uint16_t slot_number; // (O)
} f1ap_measurement_req_t;

typedef struct f1ap_measurement_resp_s {
  uint8_t transaction_id; // IE 9.3.1.23 (M)
  uint16_t lmf_measurement_id; // (M)
  uint16_t ran_measurement_id; // (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_pos_measurement_result_list_t pos_measurement_result_list; // (M)
  f1ap_criticality_diagnostics_t criticality_diagnostics; //  Criticality_Diagnostics // IE 9.3.1.3 (O)
} f1ap_measurement_resp_t;

typedef struct f1ap_measurement_failure_s {
  uint8_t transaction_id; // IE 9.3.1.23 (M)
  uint16_t lmf_measurement_id; // (M)
  uint16_t ran_measurement_id; // (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_cause_t cause; // IE 9.3.1.2 (M)
  f1ap_criticality_diagnostics_t criticality_diagnostics; // IE 9.3.1.3 (O)
} f1ap_measurement_failure_t;

typedef struct f1ap_measurement_report_s {
  uint8_t transaction_id; // IE 9.3.1.23 (M)
  uint16_t lmf_measurement_id; // (M)
  uint16_t ran_measurement_id; // (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_pos_measurement_result_list_t pos_measurement_result_list; //(M)
} f1ap_measurement_report_t;

typedef struct f1ap_measurement_update_s {
  uint8_t transaction_id; // IE 9.3.1.23 (M)
  uint16_t lmf_measurement_id; // (M)
  uint16_t ran_measurement_id; // (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_srs_configuration_t srs_configuration; // IE 9.3.1.192 (0)
} f1ap_measurement_update_t;

typedef struct f1ap_measurement_failure_ind_s {
  uint8_t transaction_id; // IE 9.3.1.23 (M)
  uint16_t lmf_measurement_id; // (M)
  uint16_t ran_measurement_id; // (M)
  nrppa_f1ap_info_t nrppa_msg_info;
  f1ap_cause_t cause; // IE 9.3.1.2 (M)
} f1ap_measurement_failure_ind_t;

typedef struct f1ap_measurement_abort_s {
  uint8_t transaction_id; // (M)  9.3.1.23 (M)
  uint16_t lmf_measurement_id; // (M)
  uint16_t ran_measurement_id; // (M)
  nrppa_f1ap_info_t nrppa_msg_info;
} f1ap_measurement_abort_t;

#endif /* F1AP_MESSAGES_TYPES_H_ */
