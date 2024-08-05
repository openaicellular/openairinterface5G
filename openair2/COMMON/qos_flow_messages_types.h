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

#ifndef QOSFLOW_MESSAGES_TYPES_H_
#define QOSFLOW_MESSAGES_TYPES_H_

#define STANDARIZED_5QI_NUM 26
#define FIVEQI_1 1
#define FIVEQI_2 2
#define FIVEQI_3 3
#define FIVEQI_4 4
#define FIVEQI_65 65
#define FIVEQI_66 66
#define FIVEQI_67 67
#define FIVEQI_71 71
#define FIVEQI_72 72
#define FIVEQI_73 73
#define FIVEQI_74 74
#define FIVEQI_76 76
#define FIVEQI_5 5
#define FIVEQI_6 6
#define FIVEQI_7 7
#define FIVEQI_8 8
#define FIVEQI_9 9
#define FIVEQI_69 69
#define FIVEQI_70 70
#define FIVEQI_79 79
#define FIVEQI_80 80
#define FIVEQI_82 82
#define FIVEQI_83 83
#define FIVEQI_84 84
#define FIVEQI_85 85
#define FIVEQI_86 86
#define PRIORITY_20 20
#define PRIORITY_40 40
#define PRIORITY_30 30
#define PRIORITY_50 50
#define PRIORITY_7 7
#define PRIORITY_20 20
#define PRIORITY_15 15
#define PRIORITY_56 56
#define PRIORITY_10 10
#define PRIORITY_60 60
#define PRIORITY_70 70
#define PRIORITY_80 80
#define PRIORITY_90 90
#define PRIORITY_5 5
#define PRIORITY_55 55
#define PRIORITY_65 65
#define PRIORITY_68 68
#define PRIORITY_19 19
#define PRIORITY_22 22
#define PRIORITY_24 24
#define PRIORITY_21 21
#define PRIORITY_18 18

typedef enum { non_dynamic_5qi, dynamic_5qi } fiveQI_t;
typedef enum { gbr, non_gbr, delay_critical_gbr } qos_flow_type_t;

typedef enum preemption_capability_e {
  SHALL_NOT_TRIGGER_PREEMPTION,
  MAY_TRIGGER_PREEMPTION,
} preemption_capability_t;

typedef enum preemption_vulnerability_e {
  NOT_PREEMPTABLE,
  PREEMPTABLE,
} preemption_vulnerability_t;

typedef struct qos_characteristics_s {
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
  fiveQI_t qos_type;
} qos_characteristics_t;

typedef struct ngran_allocation_retention_priority_s {
  uint16_t priority_level;
  preemption_capability_t preemption_capability;
  preemption_vulnerability_t preemption_vulnerability;
} ngran_allocation_retention_priority_t;

typedef struct gbr_qos_flow_information_s {
  long mbr_dl;
  long mbr_ul;
  long gbr_dl;
  long gbr_ul;
} gbr_qos_flow_information_t;

typedef struct qos_flow_level_qos_parameters_s {
  qos_characteristics_t qos_characteristics;
  ngran_allocation_retention_priority_t alloc_reten_priority;
  gbr_qos_flow_information_t *gbr_qos_flow_info;
} qos_flow_level_qos_parameters_t;

typedef struct standard_5QI_characteristics_e {
  uint64_t five_QI;
  uint64_t priority_level;
  uint64_t resource_type;
} standard_5QI_characteristics_t;

#endif /* QOSFLOW_MESSAGES_TYPES_H_ */