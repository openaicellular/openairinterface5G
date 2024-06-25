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

/*! \file nrppa_common.h
 * \brief nrppa common procedures
 * \author Adeel Malik
 * \email adeel.malik@eurecom.fr
 * \date 2023
 * \version 0.1
 */

#ifndef NRPPA_COMMON_H_
#define NRPPA_COMMON_H_

#include "common/utils/LOG/log.h"
#include "oai_asn1.h"

/*#include "NRPPA_ProtocolIE-ID.h"
#include "NRPPA_Criticality.h"
#include <ANY.h>
#include <asn_ioc.h>
#include "NRPPA_OTDOA-Information-Type-Item.h"
#include "NRPPA_Presence.h"
#include <OPEN_TYPE.h>
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>
#include "NRPPA_MeasurementQuantities-Item.h"
#include "NRPPA_ResultSS-RSRP.h"
#include "NRPPA_ResultSS-RSRQ.h"
#include "NRPPA_ResultCSI-RSRP.h"
#include "NRPPA_ResultCSI-RSRQ.h"
#include "NRPPA_UL-AoA.h"
#include "NRPPA_TDD-Config-EUTRA-Item.h"
#include "NRPPA_OtherRATMeasurementQuantities-Item.h"
#include "NRPPA_ResultNR.h"
#include "NRPPA_ResultEUTRA.h"
#include "NRPPA_WLANMeasurementQuantities-Item.h"
#include "NRPPA_UE-Measurement-ID.h"
#include "NRPPA_ReportCharacteristics.h"
#include "NRPPA_MeasurementPeriodicity.h"
#include "NRPPA_MeasurementQuantities.h"
#include "NRPPA_OtherRATMeasurementQuantities.h"
#include "NRPPA_WLANMeasurementQuantities.h"
#include "NRPPA_E-CID-MeasurementResult.h"

#include "NRPPA_Cell-Portion-ID.h"
#include "NRPPA_OtherRATMeasurementResult.h"
#include "NRPPA_WLANMeasurementResult.h"

#include "NRPPA_OTDOA-Information-Type.h"
#include "NRPPA_OTDOACells.h"
#include "NRPPA_Assistance-Information.h"
#include "NRPPA_Broadcast.h"
#include "NRPPA_PositioningBroadcastCells.h"
#include "NRPPA_AssistanceInformationFailureList.h"

#include "NRPPA_TRPList.h"
#include "NRPPA_TRPInformationTypeList.h"




*/

/* Start: ad**l todo add all nrppa ASN genrated header files here */
#include "NRPPA_NRPPA-PDU.h"
#include "NRPPA_InitiatingMessage.h"
#include "NRPPA_SuccessfulOutcome.h"
#include "NRPPA_UnsuccessfulOutcome.h"

#include "NRPPA_ProtocolIE-ID.h"
#include "NRPPA_ProtocolIE-Field.h"
#include "NRPPA_ProtocolIE-Container.h"
#include "NRPPA_ProtocolExtensionField.h"
#include "NRPPA_ProtocolIE-ContainerList.h"
#include "NRPPA_ProtocolExtensionContainer.h"
#include "NRPPA_ProtocolIE-Single-Container.h"
#include "NRPPA_asn_constant.h"

// Position Information Transfer Procedures
#include "NRPPA_PositioningActivationFailure.h"
#include "NRPPA_PositioningActivationRequest.h"
#include "NRPPA_PositioningActivationResponse.h"
#include "NRPPA_PositioningBroadcastCells.h"
#include "NRPPA_PositioningDeactivation.h"
#include "NRPPA_PositioningInformationFailure.h"
#include "NRPPA_PositioningInformationRequest.h"
#include "NRPPA_PositioningInformationResponse.h"
#include "NRPPA_PositioningInformationUpdate.h"

// IEs of Position Information Transfer Procedures
#include "NRPPA_Cause.h"
#include "NRPPA_RequestedSRSTransmissionCharacteristics.h"

#include "NRPPA_SRSConfiguration.h"
#include "NRPPA_SRSCarrier-List.h"
#include "NRPPA_SRSCarrier-List-Item.h"
#include "NRPPA_UplinkChannelBW-PerSCS-List.h"
#include "NRPPA_SCS-SpecificCarrier.h"
#include "NRPPA_ActiveULBWP.h"
#include "NRPPA_SRSConfig.h"
#include "NRPPA_SRSResource-List.h"
#include "NRPPA_SRSResource.h"

#include "NRPPA_ResourceType.h"
#include "NRPPA_ResourceTypePeriodic.h"
#include "NRPPA_ResourceTypeAperiodic.h"
#include "NRPPA_ResourceTypeSemi-persistent.h"

#include "NRPPA_SRSResourceSet-List.h"
#include "NRPPA_SRSResourceSet.h"
#include "NRPPA_SRSResourceID-List.h"
#include "NRPPA_SRSResourceSetID.h"
#include "NRPPA_ResourceSetType.h"
#include "NRPPA_ResourceSetTypePeriodic.h"
#include "NRPPA_ResourceSetTypeAperiodic.h"
#include "NRPPA_ResourceSetTypeSemi-persistent.h"

#include "NRPPA_PosSRSResource-List.h"
#include "NRPPA_PosSRSResource-Item.h"
#include "NRPPA_PosSRSResourceID-List.h"
#include "NRPPA_SRSPosResourceID.h"
#include "NRPPA_ResourceTypePos.h"
#include "NRPPA_ResourceTypePeriodicPos.h"
#include "NRPPA_ResourceTypeAperiodicPos.h"
#include "NRPPA_ResourceTypeSemi-persistentPos.h"

#include "NRPPA_PosSRSResourceSet-List.h"
#include "NRPPA_PosSRSResourceSet-Item.h"
#include "NRPPA_PosResourceSetType.h"
#include "NRPPA_PosResourceSetTypePeriodic.h"
#include "NRPPA_PosResourceSetTypeAperiodic.h"
#include "NRPPA_PosResourceSetTypeSemi-persistent.h"

#include "NRPPA_SFNInitialisationTime.h"
#include "NRPPA_CriticalityDiagnostics.h"
#include "NRPPA_Criticality.h"

#include "NRPPA_SRSType.h"
#include "NRPPA_SemipersistentSRS.h"
#include "NRPPA_AperiodicSRS.h"
#include "NRPPA_ActivationTime.h"
#include "NRPPA_AbortTransmission.h"

// TRP Information Transfer
#include "NRPPA_TRPInformationFailure.h"
#include "NRPPA_TRPInformationRequest.h"
#include "NRPPA_TRPInformationResponse.h"
//
//#include ".h"
#include "NRPPA_TRPInformationList.h"
#include "NRPPA_TRPInformationItem.h"

// Measurement Transfer
#include "NRPPA_MeasurementRequest.h"
#include "NRPPA_MeasurementFailure.h"
#include "NRPPA_MeasurementResponse.h"
#include "NRPPA_MeasurementReport.h"
#include "NRPPA_MeasurementUpdate.h"
#include "NRPPA_MeasurementAbort.h"
#include "NRPPA_MeasurementFailureIndication.h"

//#include ".h"
#include "NRPPA_TRP-MeasurementRequestList.h"
#include "NRPPA_TRP-MeasurementRequestItem.h"
#include "NRPPA_TRPMeasurementQuantities.h"

#include "NRPPA_TRPMeasurementQuantitiesList-Item.h"

#include "NRPPA_TRP-MeasurementResponseList.h"
#include "NRPPA_TRP-MeasurementResponseItem.h"
#include "NRPPA_TrpMeasurementResult.h"
#include "NRPPA_TrpMeasurementResultItem.h"
#include "NRPPA_TrpMeasuredResultsValue.h"
#include "NRPPA_MeasuredResults.h"
#include "NRPPA_MeasuredResultsValue.h"
#include "NRPPA_MeasurementQuantities.h"
#include "NRPPA_UL-AoA.h"
#include "NRPPA_UL-RTOAMeasurement.h"
#include "NRPPA_ULRTOAMeas.h"
#include "NRPPA_GNBRxTxTimeDiffMeas.h"
#include "NRPPA_GNB-RxTxTimeDiff.h"
#include "NRPPA_Measurement-ID.h"
#include "NRPPA_MeasurementBeamInfo.h"
#include "NRPPA_SystemFrameNumber.h"
#include "NRPPA_SlotNumber.h"
#include "NRPPA_TRPReferencePointType.h"
#include "NRPPA_ReferencePoint.h"
#include "NRPPA_TRPPositionReferenced.h"
#include "NRPPA_RelativeGeodeticLocation.h"
#include "NRPPA_RelativeCartesianLocation.h"
#include "NRPPA_TRPPositionDefinitionType.h"

/* END: ad**l todo add all nrppa ASN genrated header files here*/

/* Checking version of ASN1C compiler */
#if (ASN1C_ENVIRONMENT_VERSION < ASN1C_MINIMUM_VERSION)
#error "You are compiling nrppa with the wrong version of ASN1C"
#endif

extern int asn_debug;
extern int asn1_xer_print;

#if defined(ENB_MODE)
#include "common/utils/LOG/log.h"
#include "ngap_gNB_default_values.h"
#define NRPPA_ERROR(x, args...) LOG_E(NRPPA, x, ##args)
#define NRPPA_WARN(x, args...) LOG_W(NRPPA, x, ##args)
#define NRPPA_TRAF(x, args...) LOG_I(NRPPA, x, ##args)
#define NRPPA_INFO(x, args...) LOG_I(NRPPA, x, ##args)
#define NRPPA_DEBUG(x, args...) LOG_I(NRPPA, x, ##args)
#else
#define NRPPA_ERROR(x, args...)              \
  do {                                       \
    fprintf(stdout, "[NRPPA][E]" x, ##args); \
  } while (0)
#define NRPPA_WARN(x, args...)               \
  do {                                       \
    fprintf(stdout, "[NRPPA][W]" x, ##args); \
  } while (0)
#define NRPPA_TRAF(x, args...)               \
  do {                                       \
    fprintf(stdout, "[NRPPA][T]" x, ##args); \
  } while (0)
#define NRPPA_INFO(x, args...)               \
  do {                                       \
    fprintf(stdout, "[NRPPA][I]" x, ##args); \
  } while (0)
#define NRPPA_DEBUG(x, args...)              \
  do {                                       \
    fprintf(stdout, "[NRPPA][D]" x, ##args); \
  } while (0)
#endif

#define NRPPA_FIND_PROTOCOLIE_BY_ID(IE_TYPE, ie, container, IE_ID, mandatory)                                                  \
  do {                                                                                                                         \
    IE_TYPE **ptr;                                                                                                             \
    ie = NULL;                                                                                                                 \
    for (ptr = container->protocolIEs.list.array; ptr < &container->protocolIEs.list.array[container->protocolIEs.list.count]; \
         ptr++) {                                                                                                              \
      if ((*ptr)->id == IE_ID) {                                                                                               \
        ie = *ptr;                                                                                                             \
        break;                                                                                                                 \
      }                                                                                                                        \
    }                                                                                                                          \
    if (ie == NULL) {                                                                                                          \
      if (mandatory) {                                                                                                         \
        AssertFatal(NRPPA, "NRPPA_FIND_PROTOCOLIE_BY_ID ie is NULL (searching for ie: %ld)\n", IE_ID);                         \
      } else {                                                                                                                 \
        NRPPA_INFO("NRPPA_FIND_PROTOCOLIE_BY_ID ie is NULL (searching for ie: %ld)\n", IE_ID);                                 \
      }                                                                                                                        \
    }                                                                                                                          \
  } while (0);                                                                                                                 \
  if (mandatory && !ie)                                                                                                        \
  return -1

/* ad**l todo */
/* gnb and ue related info in NRPPA emssage */
typedef struct nrppa_gnb_ue_info_s {
  instance_t instance;
  int32_t gNB_ue_ngap_id;
  int64_t amf_ue_ngap_id;
  // routing ID
  uint8_t *routing_id_buffer;
  uint32_t routing_id_length; // Length of the octet string
                              // ngap_routing_id_t routing_id;
} nrppa_gnb_ue_info_t;

/** \brief Function callback prototype.
 **/
typedef int (*nrppa_message_decoded_callback)(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu);

/** \brief Handle criticality
 \param criticality Criticality of the IE
 @returns void
 **/

/* ad**l todo
void nrppa_handle_criticality(NRPPA_Criticality_t criticality);
*/

#endif /* NRPPA_COMMON_H_ */
