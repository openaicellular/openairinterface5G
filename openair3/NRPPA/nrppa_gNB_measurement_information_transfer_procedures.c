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

/*! \file nrppa_gNB_measurement_information_transfer_procedures.c
 * \brief NRPPA gNB tasks related to measurement information transfer
 * \author Adeel Malik
 * \email adeel.malik@eurecom.fr
 *\date 2023
 * \version 1.0
 * @ingroup _nrppa
 */

#include "intertask_interface.h"

#include "nrppa_common.h"
#include "nrppa_gNB_measurement_information_transfer_procedures.h"
#include "nrppa_gNB_itti_messaging.h"
#include "nrppa_gNB_encoder.h"

// DOWLINK

/* Measurement (Parent) procedure for  MeasurementRequest, MeasurementResponse, and MeasurementFailure*/
int nrppa_gNB_handle_Measurement(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu)
{
  LOG_I(NRPPA, "Processing Received MeasurementRequest \n");
  DevAssert(pdu != NULL);
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, pdu);

  // Prepare forward request for RRC
  MessageDef *msg = itti_alloc_new_message(TASK_RRC_GNB, 0, F1AP_MEASUREMENT_REQ);
  f1ap_measurement_req_t *f1ap_req = &F1AP_MEASUREMENT_REQ(msg);
  f1ap_req->nrppa_msg_info.instance = nrppa_msg_info->instance;
  f1ap_req->nrppa_msg_info.gNB_ue_ngap_id = nrppa_msg_info->gNB_ue_ngap_id;
  f1ap_req->nrppa_msg_info.amf_ue_ngap_id = nrppa_msg_info->amf_ue_ngap_id;
  f1ap_req->nrppa_msg_info.routing_id_buffer = nrppa_msg_info->routing_id_buffer;
  f1ap_req->nrppa_msg_info.routing_id_length = nrppa_msg_info->routing_id_length;
  f1ap_req->ran_measurement_id = 2; // TODO add actual not in NRPPA but in F1AP;

  // Processing Received MeasurmentRequest
  NRPPA_MeasurementRequest_t *container = NULL;
  NRPPA_MeasurementRequest_IEs_t *ie = NULL;

  // IE 9.2.3 Message type (M)
  container = &pdu->choice.initiatingMessage->value.choice.MeasurementRequest;
  // IE 9.2.4 nrppatransactionID (M)
  f1ap_req->nrppa_msg_info.nrppa_transaction_id = pdu->choice.initiatingMessage->nrppatransactionID;
  f1ap_req->transaction_id = pdu->choice.initiatingMessage->nrppatransactionID;

  // IE LMF_Measurement_ID (M)
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementRequest_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_LMF_Measurement_ID, true);
  NRPPA_Measurement_ID_t LMF_Meas_ID = ie->value.choice.Measurement_ID;
  f1ap_req->lmf_measurement_id = LMF_Meas_ID;

  // IE TRP Measurement Request List (M)
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementRequest_IEs_t,
                              ie,
                              container,
                              NRPPA_ProtocolIE_ID_id_TRP_MeasurementRequestList,
                              false);
  if (ie != NULL) {
    NRPPA_TRP_MeasurementRequestList_t measurement_request_list = ie->value.choice.TRP_MeasurementRequestList;
    int maxNoMeasTRP = measurement_request_list.list.count;
    f1ap_req->trp_measurement_request_list.trp_measurement_request_list_length = maxNoMeasTRP;
    f1ap_req->trp_measurement_request_list.trp_measurement_request_item =
        malloc(maxNoMeasTRP * sizeof(f1ap_trp_measurement_request_item_t));
    DevAssert(f1ap_req->trp_measurement_request_list.trp_measurement_request_item);
    f1ap_trp_measurement_request_item_t *trp_measurement_request_item =
        f1ap_req->trp_measurement_request_list.trp_measurement_request_item;
    for (int k = 0; k < maxNoMeasTRP; k++) {
      NRPPA_TRP_MeasurementRequestItem_t *trp_meas_req_item = measurement_request_list.list.array[k];
      trp_measurement_request_item->tRPID = trp_meas_req_item->tRP_ID;
      // trp_measurement_request_item.search_window_information.delayUncertainty =
      // trp_meas_req_item->search_window_information.delayUncertainty; // OPTIONAL
      // trp_measurement_request_item.search_window_information.expectedPropagationDelay =
      // trp_meas_req_item->search_window_information.expectedPropagationDelay; // OPTIONAL
      if (k < maxNoMeasTRP - 1) {
        trp_measurement_request_item++;
      }
    }
  }
  // IE Report Characteristics (M)
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementRequest_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_ReportCharacteristics, false);
  if (ie != NULL) {
    NRPPA_ReportCharacteristics_t report_characteristics = ie->value.choice.ReportCharacteristics;
    f1ap_req->pos_report_characteristics = report_characteristics;
  }
  // IE Measurement Periodicity (M if Report Characteristics is periodic )
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementRequest_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_MeasurementPeriodicity, false);
  if (ie != NULL) {
    NRPPA_MeasurementPeriodicity_t measurement_periodicity = ie->value.choice.MeasurementPeriodicity;
    f1ap_req->pos_measurement_periodicity = measurement_periodicity;
  }
  // IE TRP Measurement Quantities (M)
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementRequest_IEs_t,
                              ie,
                              container,
                              NRPPA_ProtocolIE_ID_id_TRPMeasurementQuantities,
                              false);
  if (ie != NULL) {
    NRPPA_TRPMeasurementQuantities_t measurement_quantities = ie->value.choice.TRPMeasurementQuantities;
    int maxNoPosMeas = measurement_quantities.list.count;
    f1ap_req->pos_measurement_quantities.f1ap_pos_measurement_quantities_length = maxNoPosMeas;
    f1ap_req->pos_measurement_quantities.pos_measurement_quantities_item =
        malloc(maxNoPosMeas * sizeof(f1ap_pos_measurement_quantities_item_t));
    DevAssert(f1ap_req->pos_measurement_quantities.pos_measurement_quantities_item);
    f1ap_pos_measurement_quantities_item_t *pos_measurement_quantities_item =
        f1ap_req->pos_measurement_quantities.pos_measurement_quantities_item;
    for (int j = 0; j < maxNoPosMeas; j++) {
      NRPPA_TRPMeasurementQuantitiesList_Item_t *meas_quant_item = measurement_quantities.list.array[j];
      pos_measurement_quantities_item->posMeasurementType = meas_quant_item->tRPMeasurementQuantities_Item; // posMeasurementType;
      // pos_measurement_quantities_item->timingReportingGranularityFactor=meas_quant_item->timingReportingGranularityFactor; //
      // OPTIONal
      if (j < maxNoPosMeas - 1) {
        pos_measurement_quantities_item++;
      }
    }
  }

  // IE SFNInitialisationTime (Optional)
  // NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementRequest_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_SFNInitialisationTime,
  // false); NRPPA_SFNInitialisationTime_t sfn_time = ie->value.choice.SFNInitialisationTime;

  // IE SRSConfiguration (Optional)
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementRequest_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_SRSConfiguration, false);
  if (ie != NULL) {
    /*
    NRPPA_SRSConfiguration_t nrppa_srs_config = ie->value.choice.SRSConfiguration;
    f1ap_srs_configuration_t *f1ap_srs_config = &f1ap_req->srs_configuration;
    f1ap_srs_config->srs_carrier_list.srs_carrier_list_length = 1;
    f1ap_srs_config->srs_carrier_list.srs_carrier_list_item = malloc(f1ap_srs_config->srs_carrier_list.srs_carrier_list_length* sizeof(f1ap_srs_carrier_list_item_t));
    for (int srs_idx = 0; srs_idx<f1ap_srs_config->srs_carrier_list.srs_carrier_list_length; srs_idx++) {
      f1ap_srs_config_t *sRSConfig = &f1ap_srs_config->srs_carrier_list.srs_carrier_list_item[srs_idx].active_ul_bwp.sRSConfig;

      sRSConfig->sRSResource_List.srs_resource_list_length = 1;
      sRSConfig->sRSResource_List.srs_resource = malloc(sRSConfig->sRSResource_List.srs_resource_list_length*sizeof(f1ap_srs_resource_t));
	
      sRSConfig->sRSResourceSet_List.srs_resource_set_list_length = 1;
      sRSConfig->sRSResourceSet_List.srs_resource_set = malloc( sRSConfig->sRSResourceSet_List.srs_resource_set_list_length*sizeof(f1ap_srs_resource_set_t));
    */  
    NRPPA_SRSConfiguration_t srs_config = ie->value.choice.SRSConfiguration;
    int maxnoSRScarrier = srs_config.sRSCarrier_List.list.count;
    f1ap_req->srs_configuration.srs_carrier_list.srs_carrier_list_length= maxnoSRScarrier;
    f1ap_req->srs_configuration.srs_carrier_list.srs_carrier_list_item = malloc(maxnoSRScarrier * sizeof(f1ap_srs_carrier_list_item_t));
    DevAssert(f1ap_req->srs_configuration.srs_carrier_list.srs_carrier_list_item);
    f1ap_srs_carrier_list_item_t *srs_carrier_list_item = f1ap_req->srs_configuration.srs_carrier_list.srs_carrier_list_item;
    LOG_D(NRPPA,"Preparing srs_carrier_list for F1AP maxnoSRScarrier= %d \n", f1ap_req->srs_configuration.srs_carrier_list.srs_carrier_list_length);
    
    for (int i = 0; i < maxnoSRScarrier; i++) {
      NRPPA_SRSCarrier_List_Item_t *CarrItem= srs_config.sRSCarrier_List.list.array[i];
      srs_carrier_list_item->pointA = CarrItem->pointA; // (M)
      srs_carrier_list_item->pci = CarrItem->pCI ? *CarrItem->pCI : 0; // Optional Physical cell ID of the cell that contians the SRS carrier
      // Preparing Active UL BWP information IE of SRSCarrier_List f1ap_active_ul_bwp_ active_ul_bwp; //(M)
      f1ap_active_ul_bwp_t *f1_ul_bwp= &srs_carrier_list_item->active_ul_bwp;
      NRPPA_ActiveULBWP_t *Nrppa_ULBWP =&CarrItem->activeULBWP;   
      f1_ul_bwp->locationAndBandwidth = Nrppa_ULBWP->locationAndBandwidth; 
      f1_ul_bwp->subcarrierSpacing = Nrppa_ULBWP->subcarrierSpacing;
      f1_ul_bwp->cyclicPrefix = Nrppa_ULBWP->cyclicPrefix; 
      f1_ul_bwp->txDirectCurrentLocation = Nrppa_ULBWP->txDirectCurrentLocation;
      f1_ul_bwp->shift7dot5kHz = Nrppa_ULBWP->shift7dot5kHz ? *Nrppa_ULBWP->shift7dot5kHz : 0; 
      
      f1ap_srs_config_t *f1_srsConf= &f1_ul_bwp->sRSConfig;  
      // Preparing sRSResource_List IE of SRSConfig (IE of activeULBWP)
      NRPPA_SRSResource_List_t *NrppaRes_list =Nrppa_ULBWP->sRSConfig.sRSResource_List;
      int maxnoSRSResources = NrppaRes_list->list.count; 
      f1_srsConf->sRSResource_List.srs_resource_list_length = maxnoSRSResources;
      f1_srsConf->sRSResource_List.srs_resource = malloc(maxnoSRSResources * sizeof(f1ap_srs_resource_t));
      DevAssert(f1_srsConf->sRSResource_List.srs_resource);
      f1ap_srs_resource_t *F1ResItem = f1_srsConf->sRSResource_List.srs_resource;
      LOG_D(NRPPA,"Preparing sRSResource_List for F1AP maxnoSRSResources=%d \n",
            f1_srsConf->sRSResource_List.srs_resource_list_length);
      for (int k = 0; k < maxnoSRSResources; k++) { // Preparing SRS Resource List
        NRPPA_SRSResource_t *srs_res = NrppaRes_list->list.array[k];
        F1ResItem->sRSResourceID = srs_res->sRSResourceID; //(M)
        F1ResItem->nrofSRS_Ports = srs_res->nrofSRS_Ports; //(M) port1	= 0, ports2	= 1, ports4	= 2
        F1ResItem->startPosition = srs_res->startPosition; //(M)
        F1ResItem->nrofSymbols = srs_res->nrofSymbols; //(M)  n1	= 0, n2	= 1, n4	= 2
        F1ResItem->repetitionFactor = srs_res->repetitionFactor; //(M)  n1	= 0, n2	= 1, n4	= 2
        F1ResItem->freqDomainPosition = srs_res->freqDomainPosition; //(M)
        F1ResItem->freqDomainShift = srs_res->freqDomainShift; //(M)
        F1ResItem->c_SRS = srs_res->c_SRS; //(M)
        F1ResItem->b_SRS = srs_res->b_SRS; //(M)
        F1ResItem->b_hop = srs_res->b_hop; //(M)
        F1ResItem->groupOrSequenceHopping = srs_res->groupOrSequenceHopping; //(M) neither	= 0, groupHopping	= 1, sequenceHopping	= 2
        F1ResItem->slotOffset = srs_res->slotOffset; // (M)
        F1ResItem->sequenceId = srs_res->sequenceId; //(M)

          // IE transmissionComb
        switch (srs_res->transmissionComb.present) {
          case NRPPA_TransmissionComb_PR_n2: 
            F1ResItem->transmissionComb.present = f1ap_transmission_comb_pr_n2;
            F1ResItem->transmissionComb.choice.n2.combOffset_n2 = srs_res->transmissionComb.choice.n2->combOffset_n2;
            F1ResItem->transmissionComb.choice.n2.cyclicShift_n2 = srs_res->transmissionComb.choice.n2->cyclicShift_n2;
            break;
          case NRPPA_TransmissionComb_PR_n4:
            F1ResItem->transmissionComb.present = f1ap_transmission_comb_pr_n4;
            F1ResItem->transmissionComb.choice.n4.combOffset_n4 = srs_res->transmissionComb.choice.n4->combOffset_n4;
            F1ResItem->transmissionComb.choice.n4.cyclicShift_n4 = srs_res->transmissionComb.choice.n4->cyclicShift_n4;
            break;
          case NRPPA_TransmissionComb_PR_NOTHING:
            F1ResItem->transmissionComb.present = f1ap_transmission_comb_pr_nothing;
            break;
          default:
            LOG_E(NRPPA, "Unknown Resource Item TransmissionComb\n");
            break;
        }

        // IE  resourceType
        switch (srs_res->resourceType.present) {
          case NRPPA_ResourceType_PR_periodic:
            F1ResItem->resourceType.present = f1ap_resource_type_pr_periodic;
            F1ResItem->resourceType.choice.periodic.periodicity= srs_res->resourceType.choice.periodic->periodicity;
            F1ResItem->resourceType.choice.periodic.offset=srs_res->resourceType.choice.periodic->offset;
            break;
          case NRPPA_ResourceType_PR_aperiodic:
            F1ResItem->resourceType.present = f1ap_resource_type_pr_aperiodic;
            F1ResItem->resourceType.choice.aperiodic.aperiodicResourceType=srs_res->resourceType.choice.aperiodic->aperiodicResourceType;
            break;
          case NRPPA_ResourceType_PR_semi_persistent:
            F1ResItem->resourceType.present = f1ap_resource_type_pr_semi_persistent;
            F1ResItem->resourceType.choice.semi_persistent.periodicity=srs_res->resourceType.choice.semi_persistent->periodicity;
            F1ResItem->resourceType.choice.semi_persistent.offset=srs_res->resourceType.choice.semi_persistent->offset;
            break;
          case NRPPA_ResourceType_PR_NOTHING:
            F1ResItem->resourceType.present = f1ap_resource_type_pr_nothing;
            break;
          default:
            LOG_E(NRPPA, "Unknown Resource Item resourceType\n");
            break;
        }
        if (k < maxnoSRSResources - 1) {
          F1ResItem++;
        }
      } // for(int k=0; k < nb_srsresource; k++)

      // Preparing sRSResourceSet_List IE of SRSConfig (IE of activeULBWP)
      NRPPA_SRSResourceSet_List_t *NrppaResSet_list =Nrppa_ULBWP->sRSConfig.sRSResourceSet_List;
      int maxnoSRSResourceSets = NrppaResSet_list->list.count;
      f1_srsConf->sRSResourceSet_List.srs_resource_set_list_length = maxnoSRSResourceSets;
      f1_srsConf->sRSResourceSet_List.srs_resource_set =
          malloc(maxnoSRSResourceSets * sizeof(f1ap_srs_resource_set_t));
      DevAssert(f1_ul_bwp->sRSConfig.sRSResourceSet_List.srs_resource_set);
      f1ap_srs_resource_set_t *F1ResSetItem =f1_srsConf->sRSResourceSet_List.srs_resource_set;
      LOG_D(NRPPA, "Preparing sRSResourceSet_List for F1AP  maxnoSRSResourceSets=%d \n", maxnoSRSResourceSets);
      for (int y = 0; y < maxnoSRSResourceSets; y++) { // Preparing SRS Resource Set List
        NRPPA_SRSResourceSet_t *srs_resSet= NrppaResSet_list->list.array[y];  

        // IE sRSResourceSetID (M)
        F1ResSetItem->sRSResourceSetID= srs_resSet->sRSResourceSetID;

        // IE resourceSetType
        switch (srs_resSet->resourceSetType.present) {
          case NRPPA_ResourceSetType_PR_periodic:
            F1ResSetItem->resourceSetType.present = f1ap_resource_set_type_pr_periodic;
            F1ResSetItem->resourceSetType.choice.periodic.periodicSet =srs_resSet->resourceSetType.choice.periodic->periodicSet;
            break;
          case NRPPA_ResourceSetType_PR_aperiodic:
            F1ResSetItem->resourceSetType.present = f1ap_resource_set_type_pr_aperiodic;
            F1ResSetItem->resourceSetType.choice.aperiodic.sRSResourceTrigger = srs_resSet->resourceSetType.choice.aperiodic->sRSResourceTrigger;
            F1ResSetItem->resourceSetType.choice.aperiodic.slotoffset = srs_resSet->resourceSetType.choice.aperiodic->slotoffset; //1; // range 1-32
            break;
          case NRPPA_ResourceSetType_PR_semi_persistent:
            F1ResSetItem->resourceSetType.present = f1ap_resource_set_type_pr_semi_persistent;
            F1ResSetItem->resourceSetType.choice.semi_persistent.semi_persistentSet = srs_resSet->resourceSetType.choice.semi_persistent->semi_persistentSet;
            break;
          case NRPPA_ResourceSetType_PR_NOTHING:
            F1ResSetItem->resourceSetType.present = f1ap_resource_set_type_pr_nothing;
            break;
          default:
            LOG_E(NRPPA, "Unknown NRPPA_SRS_ResourceSet__resourceType \n");
            break;
        }

        // IE sRSResourceID_List
        int maxnoSRSResourcePerSets = srs_resSet->sRSResourceID_List.list.count;
        F1ResSetItem->sRSResourceID_List.srs_resource_id_list_length= maxnoSRSResourcePerSets;
        F1ResSetItem->sRSResourceID_List.srs_resource_id = malloc(maxnoSRSResourcePerSets * sizeof(uint8_t));
        DevAssert(F1ResSetItem->sRSResourceID_List.srs_resource_id);
        long *F1ResID= F1ResSetItem->sRSResourceID_List.srs_resource_id;
        for (int z = 0; z < maxnoSRSResourcePerSets; z++) {
          F1ResID = srs_resSet->sRSResourceID_List.list.array[z];//(M)
          if (z < maxnoSRSResourcePerSets -1){
            F1ResID++;
          }
        }
        if (y < maxnoSRSResourceSets - 1) {
          F1ResSetItem++;
        }
      } // for(int y=0; y < maxnoSRSResourceSets; y++)

      // Preparing posSRSResource_List IE of SRSConfig (IE of activeULBWP) IE not found in  OAI srs_config so filled zero values
      NRPPA_PosSRSResource_List_t  *NrppaPosRes_list = Nrppa_ULBWP->sRSConfig.posSRSResource_List;
      int maxnoPosSRSResources = NrppaPosRes_list->list.count;
      f1_srsConf->posSRSResource_List.pos_srs_resource_list_length = maxnoPosSRSResources;
      f1_srsConf->posSRSResource_List.pos_srs_resource_item =
          malloc(maxnoPosSRSResources * sizeof(f1ap_pos_srs_resource_item_t));
      DevAssert(f1_srsConf->posSRSResource_List.pos_srs_resource_item);
      f1ap_pos_srs_resource_item_t *F1PosResItem = f1_srsConf->posSRSResource_List.pos_srs_resource_item;
      LOG_D(NRPPA,
            "Preparing posSRSResource_List IE for F1AP maxnoPosSRSResources=%d \n",
            f1_srsConf->posSRSResource_List.pos_srs_resource_list_length);
      for (int z = 0; z < maxnoPosSRSResources; z++) { // Preparing Pos SRS Resource List
        NRPPA_PosSRSResource_Item_t *srs_PosResItem= NrppaPosRes_list->list.array[z]; 
        
        F1PosResItem->srs_PosResourceId = srs_PosResItem->srs_PosResourceId; // (M)
        F1PosResItem->startPosition = srs_PosResItem->startPosition; // (M)  range (0,1,...13)
        F1PosResItem->nrofSymbols = srs_PosResItem->nrofSymbols; // (M)  n1	= 0, n2	= 1, n4	= 2, n8	= 3, n12 = 4
        F1PosResItem->freqDomainShift = srs_PosResItem->freqDomainShift; // (M)
        F1PosResItem->c_SRS = srs_PosResItem->c_SRS; // (M)
        F1PosResItem->groupOrSequenceHopping = srs_PosResItem->groupOrSequenceHopping; // (M)  neither	= 0, groupHopping	= 1, sequenceHopping	= 2
        F1PosResItem->sequenceId = srs_PosResItem->sequenceId; //(M)
        // pos_resource_item->spatialRelationPos;	// OPTIONAL

        // IE transmissionCombPos
        NRPPA_TransmissionCombPos_t *nrppaTran = &srs_PosResItem->transmissionCombPos;
        f1ap_transmission_comb_pos_t *F1Tran= &F1PosResItem->transmissionCombPos; 
        switch (nrppaTran->present)
        {
        case NRPPA_TransmissionCombPos_PR_n2:
          F1Tran->present = f1ap_transmission_comb_pos_pr_n2;
          F1Tran->choice.n2.combOffset_n2 = nrppaTran->choice.n2->combOffset_n2;
          F1Tran->choice.n2.cyclicShift_n2 = nrppaTran->choice.n2->cyclicShift_n2;
          break;
        case NRPPA_TransmissionCombPos_PR_n4:
          F1Tran->present = f1ap_transmission_comb_pos_pr_n4;
          F1Tran->choice.n4.combOffset_n4 =nrppaTran->choice.n4->combOffset_n4;
          F1Tran->choice.n4.cyclicShift_n4 = nrppaTran->choice.n4->cyclicShift_n4;
          break;
        case NRPPA_TransmissionCombPos_PR_n8:
          F1Tran->present = f1ap_transmission_comb_pos_pr_n8;
          F1Tran->choice.n8.combOffset_n8 =nrppaTran->choice.n8->combOffset_n8;
          F1Tran->choice.n8.cyclicShift_n8 = nrppaTran->choice.n8->cyclicShift_n8;
          break;
        case NRPPA_TransmissionCombPos_PR_NOTHING:
          F1Tran->present = f1ap_transmission_comb_pos_pr_NOTHING;
          break;
        default:
          LOG_E(NRPPA, "Unknown Pos Resource Item TransmissionComb\n");
          break;
        }

        //IE resourceTypePos
        NRPPA_ResourceTypePos_t *nrppaResTy = &srs_PosResItem->resourceTypePos;
        f1ap_resource_type_pos_t *F1ResTy= &F1PosResItem->resourceTypePos; 
        switch (nrppaResTy->present)
        {
        case NRPPA_ResourceTypePos_PR_periodic:
          F1ResTy->present= f1ap_resource_type_pos_pr_periodic;
          F1ResTy->choice.periodic.offset=nrppaResTy->choice.periodic->offset;
          F1ResTy->choice.periodic.periodicity=nrppaResTy->choice.periodic->periodicity;
          break;
        case NRPPA_ResourceTypePos_PR_aperiodic:
          F1ResTy->present=f1ap_resource_type_pos_pr_aperiodic;
          F1ResTy->choice.aperiodic.slotOffset= nrppaResTy->choice.aperiodic->slotOffset;
          break;
        case NRPPA_ResourceTypePos_PR_semi_persistent:
          F1ResTy->present=f1ap_resource_type_pos_pr_semi_persistent;
          F1ResTy->choice.semi_persistent.offset= nrppaResTy->choice.semi_persistent->offset;
          F1ResTy->choice.semi_persistent.periodicity= nrppaResTy->choice.semi_persistent->periodicity;
          break;
        case NRPPA_ResourceTypePos_PR_NOTHING:
          F1ResTy->present=f1ap_resource_type_pos_pr_NOTHING;
          break;
        default:
          LOG_E(NRPPA, "Unknown Pos Resource Item resourceTypePos\n");
          break;
        }
        if (z < maxnoPosSRSResources - 1) {
          F1PosResItem++;
        }
      } // for(int z=0; z < maxnoPosSRSResources; z++)

      // Preparing posSRSResourceSet_List IE of SRSConfig (IE of activeULBWP) TODO IE not found in  OAI srs_config
      NRPPA_PosSRSResourceSet_List_t *NrppaPosResSet_list= Nrppa_ULBWP->sRSConfig.posSRSResourceSet_List;
      int maxnoPosSRSResourceSets = NrppaPosResSet_list->list.count;
      f1_srsConf->posSRSResourceSet_List.pos_srs_resource_set_list_length = maxnoPosSRSResourceSets;
      f1_srsConf->posSRSResourceSet_List.pos_srs_resource_set_item =
          malloc(maxnoPosSRSResourceSets * sizeof(f1ap_pos_srs_resource_set_item_t));
      DevAssert(f1_srsConf->posSRSResourceSet_List.pos_srs_resource_set_item);
      f1ap_pos_srs_resource_set_item_t *F1PosResSetItem=f1_srsConf->posSRSResourceSet_List.pos_srs_resource_set_item;
      //*pos_resourceSet_item= F1PosResSetItem
      LOG_D(NRPPA, "Preparing posSRSResourceSet_List for F1AP  maxnoPosSRSResourceSets=%d \n", maxnoPosSRSResourceSets);
      for (int f = 0; f < maxnoPosSRSResourceSets; f++) { // Preparing Pos SRS Resource Set List
        NRPPA_PosSRSResourceSet_Item_t *srs_PosresSetItem= NrppaPosResSet_list->list.array[f];
        // IE possrsResourceSetID
        F1PosResSetItem->possrsResourceSetID=srs_PosresSetItem->possrsResourceSetID; //(M)
        // IE possRSResourceID_List;
        int maxnoPosSRSResourcePerSets = srs_PosresSetItem->possRSResourceID_List.list.count;
        F1PosResSetItem->possRSResourceID_List.pos_srs_resource_id_list_length = maxnoPosSRSResourcePerSets;
        F1PosResSetItem->possRSResourceID_List.srs_pos_resource_id = malloc(maxnoPosSRSResourcePerSets * sizeof(uint8_t));
        DevAssert(F1PosResSetItem->possRSResourceID_List.srs_pos_resource_id);
        long *F1PosResID= F1PosResSetItem->possRSResourceID_List.srs_pos_resource_id;
        for (int z = 0; z < maxnoPosSRSResourcePerSets; z++) {
          F1PosResID= srs_PosresSetItem->possRSResourceID_List.list.array[z]; // TODO pointer address update
          if (z < maxnoPosSRSResourcePerSets - 1){
            F1PosResID++;
          }
        }
        //  IE posresourceSetType
        NRPPA_PosResourceSetType_t *nrppaPosResSetTy= &srs_PosresSetItem->posresourceSetType;
        f1ap_pos_resource_set_type_t *F1PosResSetTy= &F1PosResSetItem->posresourceSetType;
        switch (nrppaPosResSetTy->present)
        {
        case NRPPA_PosResourceSetType_PR_periodic:
          F1PosResSetTy->present=f1ap_pos_resource_set_type_pr_periodic;
          F1PosResSetTy->choice.periodic.posperiodicSet=nrppaPosResSetTy->choice.periodic->posperiodicSet;
          break;
        case NRPPA_PosResourceSetType_PR_aperiodic:
          F1PosResSetTy->present=f1ap_pos_resource_set_type_pr_aperiodic;
          F1PosResSetTy->choice.aperiodic.sRSResourceTrigger_List=nrppaPosResSetTy->choice.aperiodic->sRSResourceTrigger;
          break;
        case NRPPA_PosResourceSetType_PR_semi_persistent:
          F1PosResSetTy->present=f1ap_pos_resource_set_type_pr_semi_persistent;
          F1PosResSetTy->choice.semi_persistent.possemi_persistentSet=nrppaPosResSetTy->choice.semi_persistent->possemi_persistentSet;
          break;
        case NRPPA_PosResourceSetType_PR_NOTHING:
          F1PosResSetTy->present=f1ap_pos_resource_set_type_pr_nothing;
          break;
        default:
          LOG_E(NRPPA, "Unknown NRPPA_PosSRS_ResourceSet__resourceType \n");
          break;
        }
        if (f < maxnoSRSResourceSets-1){
          F1PosResSetItem++;
        }
      } // for(int f=0; f < maxnoSRSResourceSets; f++)

      //  Preparing Uplink Channel BW Per SCS List IE of SRSCarrier_List (M)
      NRPPA_UplinkChannelBW_PerSCS_List_t *nrppa_UlChBW= &CarrItem->uplinkChannelBW_PerSCS_List;
      f1ap_uplink_channel_bw_per_scs_list_t *F1UlChBW= &srs_carrier_list_item->uplink_channel_bw_per_scs_list;
      int maxnoSCSs = nrppa_UlChBW->list.count;
      F1UlChBW->scs_specific_carrier_list_length = maxnoSCSs;
      F1UlChBW->scs_specific_carrier = malloc(maxnoSCSs * sizeof(f1ap_scs_specific_carrier_t));
      DevAssert(F1UlChBW->scs_specific_carrier);
      f1ap_scs_specific_carrier_t *F1ScsCar = F1UlChBW->scs_specific_carrier;
      //*scs_specific_carrier_item *F1ScsCar
      LOG_D(NRPPA, "Preparing Uplink Channel BW Per SCS List for F1AP maxnoSCSs=%d \n", maxnoSCSs);
      for (int a = 0; a < maxnoSCSs; a++) {
        NRPPA_SCS_SpecificCarrier_t *nrppaScsCar=  nrppa_UlChBW->list.array[a];
        F1ScsCar->offsetToCarrier = nrppaScsCar->offsetToCarrier; // (M)
        F1ScsCar->subcarrierSpacing = nrppaScsCar->subcarrierSpacing; // (M)
        F1ScsCar->carrierBandwidth = nrppaScsCar->carrierBandwidth; // (M)
        if (a < maxnoSCSs-1){
          F1ScsCar++;
        }
      } // for(int a=0; a < maxnoSCSs; a++)
      if (i < maxnoSRScarrier -1){
        srs_carrier_list_item++;
      }
    } // for (int i = 0; i < maxnoSRScarrier; i++)
  }
  
  // IE MeasurementBeamInfoRequest (Optional)
  // NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementRequest_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_MeasurementBeamInfoRequest,
  // false); NRPPA_MeasurementBeamInfoRequest_t measurement_beam_info_request = ie->value.choice.MeasurementBeamInfoRequest;

  // IE SystemFrameNumber (Optional)
  // NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementRequest_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_SystemFrameNumber, false);
  // NRPPA_SystemFrameNumber_t frame_num = ie->value.choice.SystemFrameNumber;

  // IE SlotNumber (Optional)
  // NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementRequest_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_SlotNumber, false);
  // NRPPA_SlotNumber_t slot_num = ie->value.choice.SlotNumber;

  LOG_I(NRPPA,
        "Forwarding to RRC MeasurementRequest lmf_measurement_id=%d, ran_measurement_id=%d  \n",
        f1ap_req->lmf_measurement_id,
        f1ap_req->ran_measurement_id);
  itti_send_msg_to_task(TASK_RRC_GNB, 0, msg);
  return 0;
}

int nrppa_gNB_handle_MeasurementUpdate(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu)
{
  LOG_I(NRPPA, "Processing Received MeasurementUpdate \n");
  DevAssert(pdu != NULL);
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, pdu);

  // Forward request to RRC
  MessageDef *msg = itti_alloc_new_message(TASK_RRC_GNB, 0, F1AP_MEASUREMENT_UPDATE);
  f1ap_measurement_update_t *f1ap_req = &F1AP_MEASUREMENT_UPDATE(msg);
  f1ap_req->nrppa_msg_info.instance = nrppa_msg_info->instance;
  f1ap_req->nrppa_msg_info.gNB_ue_ngap_id = nrppa_msg_info->gNB_ue_ngap_id;
  f1ap_req->nrppa_msg_info.amf_ue_ngap_id = nrppa_msg_info->amf_ue_ngap_id;
  f1ap_req->nrppa_msg_info.routing_id_buffer = nrppa_msg_info->routing_id_buffer;
  f1ap_req->nrppa_msg_info.routing_id_length = nrppa_msg_info->routing_id_length;

  // Processing Received MeasurementUpdate
  NRPPA_MeasurementUpdate_t *container = NULL;
  NRPPA_MeasurementUpdate_IEs_t *ie = NULL;

  // IE 9.2.3 Message type (M)
  container = &pdu->choice.initiatingMessage->value.choice.MeasurementUpdate;

  // IE 9.2.4 nrppatransactionID (M)
  f1ap_req->nrppa_msg_info.nrppa_transaction_id = pdu->choice.initiatingMessage->nrppatransactionID;
  f1ap_req->transaction_id = pdu->choice.initiatingMessage->nrppatransactionID;

  // IE LMF_Measurement_ID (M)
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementUpdate_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_LMF_Measurement_ID, true);
  NRPPA_Measurement_ID_t LMF_Meas_ID = ie->value.choice.Measurement_ID;
  f1ap_req->lmf_measurement_id = LMF_Meas_ID;

  // IE RAN_Measurement_ID (M)
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementUpdate_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_RAN_Measurement_ID, true);
  NRPPA_Measurement_ID_t RAN_Meas_ID = ie->value.choice.Measurement_ID_1;
      // TODO adeel check if it is with Measurement_ID_1 or Measurement_ID
  f1ap_req->ran_measurement_id = RAN_Meas_ID;

  // IE SRSConfiguration (Optional)
  // NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementRequest_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_SRSConfiguration, true);
  // NRPPA_SRSConfiguration_t srs_config = ie->value.choice.SRSConfiguration; // TODO process this information

  LOG_I(NRPPA,
        "Procesing MeasurementUpdate lmf_measurement_id=%d, ran_measurement_id=%d \n",
        f1ap_req->lmf_measurement_id,
        f1ap_req->ran_measurement_id);
  itti_send_msg_to_task(TASK_RRC_GNB, 0, msg);
  return 0;
}

int nrppa_gNB_handle_MeasurementAbort(nrppa_gnb_ue_info_t *nrppa_msg_info, NRPPA_NRPPA_PDU_t *pdu)
{
  LOG_I(NRPPA, "Processing Received MeasurementAbort \n");
  DevAssert(pdu != NULL);
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, pdu);

  // Forward request to RRC
  MessageDef *msg = itti_alloc_new_message(TASK_RRC_GNB, 0, F1AP_MEASUREMENT_ABORT);
  f1ap_measurement_abort_t *f1ap_req = &F1AP_MEASUREMENT_ABORT(msg);
  f1ap_req->nrppa_msg_info.instance = nrppa_msg_info->instance;
  f1ap_req->nrppa_msg_info.gNB_ue_ngap_id = nrppa_msg_info->gNB_ue_ngap_id;
  f1ap_req->nrppa_msg_info.amf_ue_ngap_id = nrppa_msg_info->amf_ue_ngap_id;
  f1ap_req->nrppa_msg_info.routing_id_buffer = nrppa_msg_info->routing_id_buffer;
  f1ap_req->nrppa_msg_info.routing_id_length = nrppa_msg_info->routing_id_length;

  // Processing Received MeasurementAbort
  NRPPA_MeasurementAbort_t *container = NULL;
  NRPPA_MeasurementAbort_IEs_t *ie = NULL;

  // IE 9.2.3 Message type (M)
  container = &pdu->choice.initiatingMessage->value.choice.MeasurementAbort;

  // IE 9.2.4 nrppatransactionID (M)
  f1ap_req->nrppa_msg_info.nrppa_transaction_id = pdu->choice.initiatingMessage->nrppatransactionID;
  f1ap_req->transaction_id = pdu->choice.initiatingMessage->nrppatransactionID;

  // IE LMF_Measurement_ID
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementAbort_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_LMF_Measurement_ID, true);
  NRPPA_Measurement_ID_t LMF_Meas_ID = ie->value.choice.Measurement_ID;
  f1ap_req->lmf_measurement_id = LMF_Meas_ID;

  // IE RAN_Measurement_ID
  NRPPA_FIND_PROTOCOLIE_BY_ID(NRPPA_MeasurementAbort_IEs_t, ie, container, NRPPA_ProtocolIE_ID_id_RAN_Measurement_ID, true);
  NRPPA_Measurement_ID_t RAN_Meas_ID = ie->value.choice.Measurement_ID;
  f1ap_req->ran_measurement_id = RAN_Meas_ID;

  LOG_I(NRPPA,
        "Procesing MeasurementABORT lmf_measurement_id=%d, ran_measurement_id=%d \n",
        f1ap_req->lmf_measurement_id,
        f1ap_req->ran_measurement_id);
  itti_send_msg_to_task(TASK_RRC_GNB, 0, msg);
  return 0;
}

// UPLINK
// adeel TODO fill F1AP msg for rrc
int nrppa_gNB_MeasurementResponse(instance_t instance, MessageDef *msg_p)
{
  f1ap_measurement_resp_t *resp = &F1AP_MEASUREMENT_RESP(msg_p);
  LOG_I(NRPPA,
        "Received MEASUREMENTResponse info from RRC  lmf_measurement_id=%d, ran_measurement_id=%d  rnti= %04x\n",
        resp->lmf_measurement_id,
        resp->ran_measurement_id,
        resp->nrppa_msg_info.ue_rnti);

  // Prepare NRPPA Measurement Response
  NRPPA_NRPPA_PDU_t pdu ={0};

  /* Prepare the NRPPA message to encode for successfulOutcome MeasurementResponse */
  // IE: 9.2.3 Message Type successfulOutcome MeasurementResponse  mandatory
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = NRPPA_NRPPA_PDU_PR_successfulOutcome;
  asn1cCalloc(pdu.choice.successfulOutcome, head);
  head->procedureCode = NRPPA_ProcedureCode_id_Measurement;
  head->criticality = NRPPA_Criticality_reject;
  head->value.present = NRPPA_SuccessfulOutcome__value_PR_MeasurementResponse;

  // IE 9.2.4 nrppatransactionID  mandatory
  head->nrppatransactionID = resp->nrppa_msg_info.nrppa_transaction_id;
  NRPPA_MeasurementResponse_t *out = &head->value.choice.MeasurementResponse;

  // IE = LMF  Measurement ID  mandatory
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementResponse_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_LMF_Measurement_ID;
    ie->criticality = NRPPA_Criticality_reject;
    ie->value.present = NRPPA_MeasurementResponse_IEs__value_PR_Measurement_ID;
    ie->value.choice.Measurement_ID = resp->lmf_measurement_id;
  }

  // IE = RAN  Measurement ID  //mandatory
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementResponse_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_RAN_Measurement_ID;
    ie->criticality = NRPPA_Criticality_reject;
    ie->value.present = NRPPA_MeasurementResponse_IEs__value_PR_Measurement_ID_1; // TODO adeel check if it is with Measurement_ID_1
                                                                                  // or Measurement_ID
    ie->value.choice.Measurement_ID_1 = resp->ran_measurement_id;
  }

  // IE = TRP Measurement Response List
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementResponse_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_TRP_MeasurementResponseList;
    ie->criticality = NRPPA_Criticality_reject;
    ie->value.present = NRPPA_MeasurementResponse_IEs__value_PR_TRP_MeasurementResponseList;

    int nb_meas_TRPs = resp->pos_measurement_result_list.pos_measurement_result_list_length;
    f1ap_pos_measurement_result_list_item_t *meas_res_list_item =
        resp->pos_measurement_result_list.pos_measurement_result_list_item;
    LOG_I(NRPPA, "Positioning_measurement_response nb_meas_TRPs= %d \n", nb_meas_TRPs);
    for (int i = 0; i < nb_meas_TRPs; i++) {
      asn1cSequenceAdd(ie->value.choice.TRP_MeasurementResponseList.list, NRPPA_TRP_MeasurementResponseItem_t, item);
      item->tRP_ID = meas_res_list_item->tRPID; // IE 9.2.24 long NRPPA_TRP_ID_t

      // Preparing measurementResult list an IE of MeasurementResponseItem
      int nb_pos_measurement = meas_res_list_item->posMeasurementResult.f1ap_pos_measurement_result_length;
      f1ap_pos_measurement_result_item_t *pos_meas_result_item =
          meas_res_list_item->posMeasurementResult.pos_measurement_result_item;
      LOG_I(NRPPA, "trp ID=%d nb_pos_measurement= %d \n", item->tRP_ID, nb_pos_measurement);
      for (int jj = 0; jj < nb_pos_measurement; jj++) {
        asn1cSequenceAdd(item->measurementResult.list, NRPPA_TrpMeasurementResultItem_t, measItem);
        // IE  measuredResultsValue
        switch (pos_meas_result_item->measuredResultsValue.present) {
          case f1ap_measured_results_value_pr_ul_angleofarrival:
            LOG_I(NRPPA, "Positioning_measurement_response Case NRPPA_TrpMeasuredResultsValue_PR_uL_AngleOfArrival\n");
            measItem->measuredResultsValue.present = NRPPA_TrpMeasuredResultsValue_PR_uL_AngleOfArrival;
            asn1cCalloc(measItem->measuredResultsValue.choice.uL_AngleOfArrival, meas_uL_AngleOfArrival);
            meas_uL_AngleOfArrival->azimuthAoA =
                pos_meas_result_item->measuredResultsValue.choice.uL_AngleOfArrival.azimuthAoA; // (M)
            // meas_uL_AngleOfArrival->zenithAoA = pos_meas_result_item->measuredResultsValue.choice.uL_AngleOfArrival.zenithAoA ;
            // // OPTIONAL meas_uL_AngleOfArrival->angleCoordinateSystem =
            // pos_meas_result_item->measuredResultsValue.choice.uL_AngleOfArrival.angleCoordinateSystem; // OPTIONAL
            // TODO parameter of future interest not filled in f1ap message
            break;
          case f1ap_measured_results_value_pr_ul_srs_rsrp:
            LOG_I(NRPPA, "Positioning_measurement_response Case NRPPA_TrpMeasuredResultsValue_PR_uL_SRS_RSRP\n");
            measItem->measuredResultsValue.present = NRPPA_TrpMeasuredResultsValue_PR_uL_SRS_RSRP;
            measItem->measuredResultsValue.choice.uL_SRS_RSRP = pos_meas_result_item->measuredResultsValue.choice.uL_SRS_RSRP;
            // TODO parameter of future interest not filled in f1ap message
            break;
          case f1ap_measured_results_value_pr_ul_rtoa:
            LOG_I(NRPPA, "Positioning_measurement_response Case NRPPA_TrpMeasuredResultsValue_PR_uL_RTOA \n");
            measItem->measuredResultsValue.present = NRPPA_TrpMeasuredResultsValue_PR_uL_RTOA;
            asn1cCalloc(measItem->measuredResultsValue.choice.uL_RTOA, meas_uL_RTOA);
            switch (pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.present) {
              case f1ap_ulrtoameas_pr_NOTHING:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_NOTHING;
                break;
              case f1ap_ulrtoameas_pr_k0:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_k0;
                meas_uL_RTOA->uLRTOAmeas.choice.k0 =
                    pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.choice.k0;
                break;
              case f1ap_ulrtoameas_pr_k1:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_k1;
                meas_uL_RTOA->uLRTOAmeas.choice.k1 =
                    pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.choice.k1;
                break;
              case f1ap_ulrtoameas_pr_k2:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_k2;
                meas_uL_RTOA->uLRTOAmeas.choice.k2 =
                    pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.choice.k2;
                break;
              case f1ap_ulrtoameas_pr_k3:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_k3;
                meas_uL_RTOA->uLRTOAmeas.choice.k3 =
                    pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.choice.k3;
                break;
              case f1ap_ulrtoameas_pr_k4:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_k4;
                meas_uL_RTOA->uLRTOAmeas.choice.k4 =
                    pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.choice.k4;
                break;
              case f1ap_ulrtoameas_pr_k5:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_k5;
                meas_uL_RTOA->uLRTOAmeas.choice.k5 =
                    pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.choice.k5;
                break;
              default:
                NRPPA_ERROR("Positioning_measurement_response Unknown measured Results Value of uL_RTOA_MeasurementItem \n");
                break;
            }
            // TODO struct NRPPA_AdditionalPathList	*additionalPathList;	/* OPTIONAL */
            break;
          case f1ap_measured_results_value_pr_gnb_rxtxtimediff:
            LOG_I(NRPPA, "Positioning_measurement_response Case NRPPA_TrpMeasuredResultsValue_PR_RxTxTimeDiff \n");
            measItem->measuredResultsValue.present = NRPPA_TrpMeasuredResultsValue_PR_gNB_RxTxTimeDiff;
            asn1cCalloc(measItem->measuredResultsValue.choice.gNB_RxTxTimeDiff, meas_gNB_RxTxTimeDiff);

            switch (pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.present) {
              case f1ap_gnbrxtxtimediffmeas_pr_NOTHING:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_NOTHING;
                break;
              case f1ap_gnbrxtxtimediffmeas_pr_k0:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_k0;
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.choice.k0 =
                    pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.choice.k0;
                break;
              case f1ap_gnbrxtxtimediffmeas_pr_k1:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_k1;
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.choice.k1 =
                    pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.choice.k1;
                break;
              case f1ap_gnbrxtxtimediffmeas_pr_k2:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_k2;
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.choice.k2 =
                    pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.choice.k2;
                break;
              case f1ap_gnbrxtxtimediffmeas_pr_k3:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_k3;
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.choice.k3 =
                    pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.choice.k3;
                break;
              case f1ap_gnbrxtxtimediffmeas_pr_k4:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_k4;
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.choice.k4 =
                    pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.choice.k4;
                break;
              case f1ap_gnbrxtxtimediffmeas_pr_k5:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_k5;
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.choice.k5 =
                    pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.choice.k5;
                break;
              default:
                NRPPA_ERROR("Unknown measured Results Value of GNBRxTxTimeDiffMeas \n");
                break;
            }
            // TODO struct NRPPA_AdditionalPathList	*additionalPathList;	/* OPTIONAL */
            break;

          case f1ap_measured_results_value_pr_nothing:
            measItem->measuredResultsValue.present = NRPPA_TrpMeasuredResultsValue_PR_NOTHING;
            break;

          default:
            NRPPA_ERROR("PositioningMeasurementResponse Unknown measured Results Value\n");
            break;
        }

        // IE Time Stamp
        measItem->timeStamp.systemFrameNumber = pos_meas_result_item->timeStamp.systemFrameNumber;

        // IE timeStamp.measurementTime
        // measItem->timeStamp.measurementTime = NULL; // TODO adeel type bit string retrive relevent info

        // IE Time Stamp slotIndex TODO
        //measItem->timeStamp.slotIndex.present = NRPPA_TimeStampSlotIndex_PR_sCS_15;
        //measItem->timeStamp.slotIndex.choice.sCS_15 = 0;

        switch (pos_meas_result_item->timeStamp.slotIndex.present) {
          case f1ap_time_stamp_slot_index_pr_NOTHING:
            measItem->timeStamp.slotIndex.present = NRPPA_TimeStampSlotIndex_PR_NOTHING;
            break;

          case f1ap_time_stamp_slot_index_pr_sCS_15:
            measItem->timeStamp.slotIndex.present = NRPPA_TimeStampSlotIndex_PR_sCS_15;
            measItem->timeStamp.slotIndex.choice.sCS_15 = pos_meas_result_item->timeStamp.slotIndex.choice.sCS_15;
            break;

          case f1ap_time_stamp_slot_index_pr_sCS_30:
            measItem->timeStamp.slotIndex.present = NRPPA_TimeStampSlotIndex_PR_sCS_30;
            measItem->timeStamp.slotIndex.choice.sCS_30 = pos_meas_result_item->timeStamp.slotIndex.choice.sCS_30;
            break;

          case f1ap_time_stamp_slot_index_pr_sCS_60:
            measItem->timeStamp.slotIndex.present = NRPPA_TimeStampSlotIndex_PR_sCS_60;
            measItem->timeStamp.slotIndex.choice.sCS_60 = pos_meas_result_item->timeStamp.slotIndex.choice.sCS_60;
            break;

          case f1ap_time_stamp_slot_index_pr_sCS_120:
            measItem->timeStamp.slotIndex.present = NRPPA_TimeStampSlotIndex_PR_sCS_120;
            measItem->timeStamp.slotIndex.choice.sCS_120 = pos_meas_result_item->timeStamp.slotIndex.choice.sCS_120;
            break;

          default:
            NRPPA_ERROR("PositioningMeasurementResponse Unknown timeStamp slot Index\n");
            break;
        }

        // IE measurementQuality (Optional)
        // measItem->measurementQuality = NULL; // TODO paramenter of future interest
        // IE measurementBeamInfo (Optional)
        // measItem->measurementBeamInfo = NULL; // TODO paramenter of future interest

        if (jj < nb_pos_measurement - 1) {
          pos_meas_result_item++;
        }
      } // for (int jj = 0; jj < nb_pos_measurement; jj++)

      if (i < nb_meas_TRPs - 1) {
        meas_res_list_item++;
      }
    } // for (int i = 0; i < nb_meas_TRPs; i++)

  } // IE = TRP Measurement Response List

  /*//  TODO IE 9.2.2 CriticalityDiagnostics (O)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementResponse_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_CriticalityDiagnostics;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_MeasurementResponse_IEs__value_PR_CriticalityDiagnostics;
  }*/

  LOG_I(NRPPA, "Calling encoder for MeasurementResponse \n");
  xer_fprint(stdout, &asn_DEF_NRPPA_NRPPA_PDU, &pdu);

  /* Encode NRPPA message */
  uint8_t *buffer = NULL;
  uint32_t length = 0;
  if (nrppa_gNB_encode_pdu(&pdu, &buffer, &length) < 0) {
    NRPPA_ERROR("Failed to encode Uplink NRPPa MeasurementResponse\n");
    return -1;
  }

  /* Forward the NRPPA PDU to NGAP */ 
  nrppa_f1ap_info_t *info=&resp->nrppa_msg_info;
  if (info->gNB_ue_ngap_id > 0 && info->amf_ue_ngap_id > 0) {
    LOG_D(NRPPA,
          "Sending UplinkUEAssociatedNRPPa (MeasurementResponse) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkUEAssociatedNRPPa(info->instance,
                                                info->gNB_ue_ngap_id,
                                                info->amf_ue_ngap_id,
                                                info->routing_id_buffer,
                                                info->routing_id_length,
                                                buffer,
                                                length);
    return length;
  } else if (info->gNB_ue_ngap_id == -1 && info->amf_ue_ngap_id == -1) {
    LOG_D(NRPPA,
          "Sending UplinkNonUEAssociatedNRPPa (MeasurementResponse) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa(info->instance,
                                                   info->routing_id_buffer,
                                                   info->routing_id_length,
                                                   buffer,
                                                   length);
    return length;
  } else {
    NRPPA_ERROR("Failed to find context for Uplink NonUE/UE Associated NRPPa MeasurementResponse\n");
    return -1;
  }
}

// adeel TODO fill F1AP msg for rrc
int nrppa_gNB_MeasurementFailure(instance_t instance, MessageDef *msg_p)
{
  f1ap_measurement_failure_t *failure_msg = &F1AP_MEASUREMENT_FAILURE(msg_p);
  LOG_I(NRPPA,
        "Received MEASUREMENTFailure info from RRC  lmf_measurement_id=%d, ran_measurement_id=%d  rnti= %04x\n",
        failure_msg->lmf_measurement_id,
        failure_msg->ran_measurement_id,
        failure_msg->nrppa_msg_info.ue_rnti);

  // Prepare the NRPPA message to encode for unsuccessfulOutcome MeasurementFailure
  NRPPA_NRPPA_PDU_t pdu ={0};

  // IE: 9.2.3 Message Type  mandatory
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = NRPPA_NRPPA_PDU_PR_unsuccessfulOutcome;
  asn1cCalloc(pdu.choice.unsuccessfulOutcome, head);
  head->procedureCode = NRPPA_ProcedureCode_id_Measurement;
  head->criticality = NRPPA_Criticality_reject;
  head->value.present = NRPPA_UnsuccessfulOutcome__value_PR_MeasurementFailure;

  // IE 9.2.4 nrppatransactionID   mandatory
  head->nrppatransactionID = failure_msg->nrppa_msg_info.nrppa_transaction_id;
  NRPPA_MeasurementFailure_t *out = &head->value.choice.MeasurementFailure;

  // IE = LMF  Measurement ID  /* mandatory */
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementFailure_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_LMF_Measurement_ID;
    ie->criticality = NRPPA_Criticality_reject;
    ie->value.present = NRPPA_MeasurementFailure_IEs__value_PR_Measurement_ID;
    ie->value.choice.Measurement_ID = failure_msg->lmf_measurement_id;
  }

  // TODO IE 9.2.1 Cause (M)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementFailure_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_Cause;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_MeasurementFailure_IEs__value_PR_Cause;
    switch (failure_msg->cause.present) {
      case f1ap_cause_nothing:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_NOTHING;
        break;
      case f1ap_cause_radio_network:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_radioNetwork;
        ie->value.choice.Cause.choice.radioNetwork = failure_msg->cause.choice.radioNetwork;
        break;
      // case f1ap_cause_transport:
      // ie->value.choice.Cause.present = NRPPA_Cause_PR_transport;
      // ie->value.choice.Cause.choice.transport = 0;
      // break; // IE not in nrppa specification
      case f1ap_cause_protocol:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_protocol;
        ie->value.choice.Cause.choice.protocol = failure_msg->cause.choice.protocol;
        break;
      case f1ap_cause_misc:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_misc;
        ie->value.choice.Cause.choice.misc = failure_msg->cause.choice.misc;
        break;
      default:
        NRPPA_ERROR(" MeasurementFailure Unknown Cause\n");
        break;
    }
  }

  /*//  TODO IE 9.2.2 CriticalityDiagnostics (O)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementFailure_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_CriticalityDiagnostics;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_MeasurementFailure_IEs__value_PR_CriticalityDiagnostics;
  }*/

  /* Encode NRPPA message */
  uint8_t *buffer = NULL;
  uint32_t length = 0;
  if (nrppa_gNB_encode_pdu(&pdu, &buffer, &length) < 0) {
    NRPPA_ERROR("Failed to encode Uplink NRPPa MeasurementFailure \n");
    /* Encode procedure has failed... */
    return -1;
  }

  /* Forward the NRPPA PDU to NGAP */
  nrppa_f1ap_info_t *info=&failure_msg->nrppa_msg_info;
  if (info->gNB_ue_ngap_id > 0 && info->amf_ue_ngap_id > 0) {
    LOG_D(NRPPA,
          "Sending UplinkUEAssociatedNRPPa (MeasurementFailure) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkUEAssociatedNRPPa(info->instance,
                                                info->gNB_ue_ngap_id,
                                                info->amf_ue_ngap_id,
                                                info->routing_id_buffer,
                                                info->routing_id_length,
                                                buffer,
                                                length); // tx_nrppa_pdu=buffer, nrppa_pdu_length=length
    return length;
  } else if (info->gNB_ue_ngap_id == -1 && info->amf_ue_ngap_id == -1) //
  {
    LOG_D(NRPPA,
          "Sending UplinkNonUEAssociatedNRPPa (MeasurementFailure) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa(info->instance,
                                                   info->routing_id_buffer,
                                                   info->routing_id_length,
                                                   buffer,
                                                   length);
    return length;
  } else {
    NRPPA_ERROR("Failed to find context for Uplink NonUE/UE Associated NRPPa MeasurementFailure \n");

    return -1;
  }
}

// adeel TODO fill F1AP msg for rrc
int nrppa_gNB_MeasurementReport(instance_t instance, MessageDef *msg_p)
{
  f1ap_measurement_report_t *report_msg = &F1AP_MEASUREMENT_REPORT(msg_p);
  LOG_I(NRPPA,
        "Received MeasurementReport info from RRC  lmf_measurement_id=%d, ran_measurement_id=%d  rnti= %04x\n",
        report_msg->lmf_measurement_id,
        report_msg->ran_measurement_id,
        report_msg->nrppa_msg_info.ue_rnti);

  // Prepare NRPPA Measurement Report
  NRPPA_NRPPA_PDU_t pdu ={0};

  /* Prepare the NRPPA message to encode for initiatingMessage MeasurementReport */

  // IE: 9.2.3 Message Type initiatingMessage MeasurementReport /* mandatory */
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = NRPPA_NRPPA_PDU_PR_initiatingMessage;
  asn1cCalloc(pdu.choice.initiatingMessage, head);
  head->procedureCode = NRPPA_ProcedureCode_id_MeasurementReport;
  head->criticality = NRPPA_Criticality_reject;
  head->value.present = NRPPA_InitiatingMessage__value_PR_MeasurementReport;

  // IE 9.2.4 nrppatransactionID  /* mandatory */
  head->nrppatransactionID = report_msg->nrppa_msg_info.nrppa_transaction_id;

  NRPPA_MeasurementReport_t *out = &head->value.choice.MeasurementReport;

  // IE = LMF  Measurement ID  /* mandatory */
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementReport_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_LMF_Measurement_ID;
    ie->criticality = NRPPA_Criticality_reject;
    ie->value.present = NRPPA_MeasurementReport_IEs__value_PR_Measurement_ID;
    ie->value.choice.Measurement_ID = report_msg->lmf_measurement_id;
  }

  // IE = RAN  Measurement ID  /* mandatory */
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementReport_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_RAN_Measurement_ID;
    ie->criticality = NRPPA_Criticality_reject;
    ie->value.present =
        NRPPA_MeasurementReport_IEs__value_PR_Measurement_ID_1; // TODO adeel check if it is with Measurement_ID_1 or Measurement_ID
    ie->value.choice.Measurement_ID_1 = report_msg->ran_measurement_id;
  }

  // IE = TRP Measurement Report List  (= TRP_MeasurementResponseList)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementReport_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_TRP_MeasurementReportList;
    ie->criticality = NRPPA_Criticality_reject;
    ie->value.present = NRPPA_MeasurementReport_IEs__value_PR_TRP_MeasurementResponseList; // TRP_MeasurementReportList =
                                                                                           // TRP_MeasurementResponseList
    int nb_meas_TRPs = report_msg->pos_measurement_result_list.pos_measurement_result_list_length;
    f1ap_pos_measurement_result_list_item_t *meas_res_list_item =
        report_msg->pos_measurement_result_list.pos_measurement_result_list_item;
    LOG_I(NRPPA, "Positioning_measurement_Report() nb_meas_TRPs= %d \n", nb_meas_TRPs);
    for (int i = 0; i < nb_meas_TRPs; i++) {
      asn1cSequenceAdd(ie->value.choice.TRP_MeasurementResponseList.list, NRPPA_TRP_MeasurementResponseItem_t, item);
      item->tRP_ID = meas_res_list_item->tRPID; // IE 9.2.24 long NRPPA_TRP_ID_t

      // Preparing measurementResult list an IE of MeasurementResponseItem
      int nb_pos_measurement = meas_res_list_item->posMeasurementResult.f1ap_pos_measurement_result_length;
      f1ap_pos_measurement_result_item_t *pos_meas_result_item =
          meas_res_list_item->posMeasurementResult.pos_measurement_result_item;
      LOG_I(NRPPA, "Positioning_measurement_report() nb_pos_measurement= %d \n", nb_meas_TRPs);
      for (int jj = 0; jj < nb_pos_measurement; jj++) {
        asn1cSequenceAdd(item->measurementResult.list, NRPPA_TrpMeasurementResultItem_t, measItem);

        // IE  measuredResultsValue
        switch (pos_meas_result_item->measuredResultsValue.present) {
          case f1ap_measured_results_value_pr_ul_angleofarrival:
            LOG_I(NRPPA, "Positioning_measurement_report() Case NRPPA_TrpMeasuredResultsValue_PR_uL_AngleOfArrival\n");
            measItem->measuredResultsValue.present = NRPPA_TrpMeasuredResultsValue_PR_uL_AngleOfArrival;
            asn1cCalloc(measItem->measuredResultsValue.choice.uL_AngleOfArrival, meas_uL_AngleOfArrival);
            meas_uL_AngleOfArrival->azimuthAoA =
                pos_meas_result_item->measuredResultsValue.choice.uL_AngleOfArrival.azimuthAoA; // (M)
            // measItem->measuredResultsValue.choice.uL_AngleOfArrival->zenithAoA =
            // pos_meas_result_item->measuredResultsValue.choice.uL_AngleOfArrival.zenithAoA ; // OPTIONAL
            // measItem->measuredResultsValue.choice.uL_AngleOfArrival->angleCoordinateSystem =
            // pos_meas_result_item->measuredResultsValue.choice.uL_AngleOfArrival.angleCoordinateSystem; // OPTIONAL

            // TODO parameter of future interest not filled in f1ap message
            break;

          case f1ap_measured_results_value_pr_ul_srs_rsrp:
            LOG_I(NRPPA, "Positioning_measurement_report() Case NRPPA_TrpMeasuredResultsValue_PR_uL_SRS_RSRP\n");
            measItem->measuredResultsValue.present = NRPPA_TrpMeasuredResultsValue_PR_uL_SRS_RSRP;
            measItem->measuredResultsValue.choice.uL_SRS_RSRP = pos_meas_result_item->measuredResultsValue.choice.uL_SRS_RSRP;
            // TODO parameter of future interest not filled in f1ap message
            break;

          case f1ap_measured_results_value_pr_ul_rtoa:
            LOG_I(NRPPA, "Positioning_measurement_report() Case NRPPA_TrpMeasuredResultsValue_PR_uL_RTOA \n");
            measItem->measuredResultsValue.present = NRPPA_TrpMeasuredResultsValue_PR_uL_RTOA;
            asn1cCalloc(measItem->measuredResultsValue.choice.uL_RTOA, meas_uL_RTOA);

            switch (pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.present) {
              case f1ap_ulrtoameas_pr_NOTHING:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_NOTHING;
                break;
              case f1ap_ulrtoameas_pr_k0:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_k0;
                meas_uL_RTOA->uLRTOAmeas.choice.k0 =
                    pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.choice.k0;
                break;
              case f1ap_ulrtoameas_pr_k1:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_k1;
                meas_uL_RTOA->uLRTOAmeas.choice.k1 =
                    pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.choice.k1;
                break;
              case f1ap_ulrtoameas_pr_k2:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_k2;
                meas_uL_RTOA->uLRTOAmeas.choice.k2 =
                    pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.choice.k2;
                break;
              case f1ap_ulrtoameas_pr_k3:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_k3;
                meas_uL_RTOA->uLRTOAmeas.choice.k3 =
                    pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.choice.k3;
                break;
              case f1ap_ulrtoameas_pr_k4:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_k4;
                meas_uL_RTOA->uLRTOAmeas.choice.k4 =
                    pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.choice.k4;
                break;
              case f1ap_ulrtoameas_pr_k5:
                meas_uL_RTOA->uLRTOAmeas.present = NRPPA_ULRTOAMeas_PR_k5;
                meas_uL_RTOA->uLRTOAmeas.choice.k5 =
                    pos_meas_result_item->measuredResultsValue.choice.uL_RTOA.uL_RTOA_MeasurementItem.choice.k5;
                break;
              default:
                NRPPA_ERROR("Positioning_measurement_report Unknown measured Results Value of uL_RTOA_MeasurementItem \n");
                break;
            }
            // TODO struct NRPPA_AdditionalPathList	*additionalPathList;	/* OPTIONAL */
            break;

          case f1ap_measured_results_value_pr_gnb_rxtxtimediff:
            LOG_I(NRPPA, "Positioning_measurement_report() Case NRPPA_TrpMeasuredResultsValue_PR_RxTxTimeDiff \n");
            measItem->measuredResultsValue.present = NRPPA_TrpMeasuredResultsValue_PR_gNB_RxTxTimeDiff;
            asn1cCalloc(measItem->measuredResultsValue.choice.gNB_RxTxTimeDiff, meas_gNB_RxTxTimeDiff);

            switch (pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.present) {
              case f1ap_gnbrxtxtimediffmeas_pr_NOTHING:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_NOTHING;
                break;
              case f1ap_gnbrxtxtimediffmeas_pr_k0:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_k0;
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.choice.k0 =
                    pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.choice.k0;
                break;
              case f1ap_gnbrxtxtimediffmeas_pr_k1:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_k1;
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.choice.k1 =
                    pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.choice.k1;
                break;
              case f1ap_gnbrxtxtimediffmeas_pr_k2:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_k2;
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.choice.k2 =
                    pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.choice.k2;
                break;
              case f1ap_gnbrxtxtimediffmeas_pr_k3:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_k3;
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.choice.k3 =
                    pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.choice.k3;
                break;
              case f1ap_gnbrxtxtimediffmeas_pr_k4:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_k4;
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.choice.k4 =
                    pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.choice.k4;
                break;
              case f1ap_gnbrxtxtimediffmeas_pr_k5:
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.present = NRPPA_GNBRxTxTimeDiffMeas_PR_k5;
                meas_gNB_RxTxTimeDiff->rxTxTimeDiff.choice.k5 =
                    pos_meas_result_item->measuredResultsValue.choice.gNB_RxTxTimeDiff.rxTxTimeDiff.choice.k5;
                break;
              default:
                NRPPA_ERROR("Unknown measured Results Value of GNBRxTxTimeDiffMeas \n");
                break;
            }
            // TODO struct NRPPA_AdditionalPathList	*additionalPathList;	/* OPTIONAL */
            break;

          case f1ap_measured_results_value_pr_nothing:
            measItem->measuredResultsValue.present = NRPPA_TrpMeasuredResultsValue_PR_NOTHING;
            break;

          default:
            NRPPA_ERROR("PositioningMeasurementReport Unknown measured Results Value\n");
            break;
        }

        // IE Time Stamp
        measItem->timeStamp.systemFrameNumber = pos_meas_result_item->timeStamp.systemFrameNumber;

        // IE timeStamp.measurementTime
        // measItem->timeStamp.measurementTime = NULL; // TODO adeel type bit string retrive relevent info

        // IE Time Stamp slotIndex
        measItem->timeStamp.slotIndex.present = NRPPA_TimeStampSlotIndex_PR_sCS_15;
        measItem->timeStamp.slotIndex.choice.sCS_15 = 0;
        /*switch (pos_meas_result_item->timeStamp.slotIndex.present) {
          case f1ap_time_stamp_slot_index_pr_NOTHING:
            measItem->timeStamp.slotIndex.present = NRPPA_TimeStampSlotIndex_PR_NOTHING;
            break;

          case f1ap_time_stamp_slot_index_pr_sCS_15:
            measItem->timeStamp.slotIndex.present = NRPPA_TimeStampSlotIndex_PR_sCS_15;
            measItem->timeStamp.slotIndex.choice.sCS_15 = pos_meas_result_item->timeStamp.slotIndex.choice.sCS_15;
            break;

          case f1ap_time_stamp_slot_index_pr_sCS_30:
            measItem->timeStamp.slotIndex.present = NRPPA_TimeStampSlotIndex_PR_sCS_30;
            measItem->timeStamp.slotIndex.choice.sCS_30 = pos_meas_result_item->timeStamp.slotIndex.choice.sCS_30;
            break;

          case f1ap_time_stamp_slot_index_pr_sCS_60:
            measItem->timeStamp.slotIndex.present = NRPPA_TimeStampSlotIndex_PR_sCS_60;
            measItem->timeStamp.slotIndex.choice.sCS_60 = pos_meas_result_item->timeStamp.slotIndex.choice.sCS_60;
            break;

          case f1ap_time_stamp_slot_index_pr_sCS_120:
            measItem->timeStamp.slotIndex.present = NRPPA_TimeStampSlotIndex_PR_sCS_120;
            measItem->timeStamp.slotIndex.choice.sCS_120 = pos_meas_result_item->timeStamp.slotIndex.choice.sCS_120;
            break;

          default:
            NRPPA_ERROR("PositioningMeasurementResponse Unknown timeStamp slot Index\n");
            break;
        }*/

        // IE measurementQuality (Optional)
        // measItem->measurementQuality = NULL; // TODO paramenter of future interest
        // IE measurementBeamInfo (Optional)
        // measItem->measurementBeamInfo = NULL; // TODO paramenter of future interest

        if (jj < nb_pos_measurement - 1) {
          pos_meas_result_item++;
        }
      } // for (int jj = 0; jj < nb_pos_measurement; jj++)

      if (i < nb_meas_TRPs - 1) {
        meas_res_list_item++;
      }
    } // for (int i = 0; i < nb_meas_TRPs; i++)

  } // IE = TRP Measurement Response List

  /* Encode NRPPA message */
  uint8_t *buffer = NULL;
  uint32_t length = 0;
  if (nrppa_gNB_encode_pdu(&pdu, &buffer, &length) < 0) {
    NRPPA_ERROR("Failed to encode Uplink NRPPa MeasurementReport\n");
    /* Encode procedure has failed... */
    return -1;
  }

  /* Forward the NRPPA PDU to NGAP */
  nrppa_f1ap_info_t *info=&report_msg->nrppa_msg_info;
  if (info->gNB_ue_ngap_id > 0 && info->amf_ue_ngap_id > 0) {
    LOG_D(NRPPA,
          "Sending UplinkUEAssociatedNRPPa (MeasurementReport) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkUEAssociatedNRPPa(info->instance,
                                                info->gNB_ue_ngap_id,
                                                info->amf_ue_ngap_id,
                                                info->routing_id_buffer,
                                                info->routing_id_length,
                                                buffer,
                                                length); // tx_nrppa_pdu=buffer, nrppa_pdu_length=length
    return length;
  } else if (info->gNB_ue_ngap_id == -1 && info->amf_ue_ngap_id == -1) //
  {
    LOG_D(NRPPA,
          "Sending UplinkNonUEAssociatedNRPPa (MeasurementReport) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa(info->instance,
                                                   info->routing_id_buffer,
                                                   info->routing_id_length,
                                                   buffer,
                                                   length);
    return length;
  } else {
    NRPPA_ERROR("Failed to find context for Uplink NonUE/UE Associated NRPPa MeasurementReport\n");

    return -1;
  }
}

// adeel TODO fill F1AP msg for rrc
int nrppa_gNB_MeasurementFailureIndication(instance_t instance, MessageDef *msg_p)
{
  f1ap_measurement_failure_ind_t *failure_msg = &F1AP_MEASUREMENT_FAILURE_IND(msg_p);
  LOG_I(NRPPA,
        "Received MEASUREMENTFailureIndication info from RRC  lmf_measurement_id=%d, ran_measurement_id=%d  rnti= %04x\n",
        failure_msg->lmf_measurement_id,
        failure_msg->ran_measurement_id,
        failure_msg->nrppa_msg_info.ue_rnti);
  // Prepare NRPPA Measurement Failure Indication
  NRPPA_NRPPA_PDU_t pdu ={0};
  /* Prepare the NRPPA message to encode for initiatingMessage MeasurementFailureIndication */

  // IE 9.2.3 Message type (M)
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = NRPPA_NRPPA_PDU_PR_initiatingMessage;
  asn1cCalloc(pdu.choice.initiatingMessage, head);
  head->procedureCode = NRPPA_ProcedureCode_id_MeasurementFailureIndication;
  head->criticality = NRPPA_Criticality_reject;
  head->value.present = NRPPA_InitiatingMessage__value_PR_MeasurementFailureIndication;

  // IE 9.2.4 nrppatransactionID  (M)
  head->nrppatransactionID = failure_msg->nrppa_msg_info.nrppa_transaction_id;

  NRPPA_MeasurementFailureIndication_t *out = &head->value.choice.MeasurementFailureIndication;

  // IE = LMF  Measurement ID (M)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementFailureIndication_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_LMF_Measurement_ID;
    ie->criticality = NRPPA_Criticality_reject;
    ie->value.present = NRPPA_MeasurementFailureIndication_IEs__value_PR_Measurement_ID;
    ie->value.choice.Measurement_ID = failure_msg->lmf_measurement_id;
  }

  // IE = RAN  Measurement ID (M)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementFailureIndication_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_RAN_Measurement_ID;
    ie->criticality = NRPPA_Criticality_reject;
    ie->value.present = NRPPA_MeasurementFailureIndication_IEs__value_PR_Measurement_ID_1; // TODO adeel check if it is with
                                                                                           // Measurement_ID_1 or Measurement_ID
    ie->value.choice.Measurement_ID_1 = failure_msg->ran_measurement_id;
  }

  // TODO IE 9.2.1 Cause (M)
  {
    asn1cSequenceAdd(out->protocolIEs.list, NRPPA_MeasurementFailureIndication_IEs_t, ie);
    ie->id = NRPPA_ProtocolIE_ID_id_Cause;
    ie->criticality = NRPPA_Criticality_ignore;
    ie->value.present = NRPPA_MeasurementFailureIndication_IEs__value_PR_Cause;
    switch (failure_msg->cause.present) {
      case f1ap_cause_nothing:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_NOTHING;
        break;
      case f1ap_cause_radio_network:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_radioNetwork;
        ie->value.choice.Cause.choice.radioNetwork = failure_msg->cause.choice.radioNetwork;
        break;
      // case f1ap_cause_transport:
      // ie->value.choice.Cause.present = NRPPA_Cause_PR_transport;
      // ie->value.choice.Cause.choice.transport = 0;
      // break; // IE not in nrppa specification
      case f1ap_cause_protocol:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_protocol;
        ie->value.choice.Cause.choice.protocol = failure_msg->cause.choice.protocol;
        break;
      case f1ap_cause_misc:
        ie->value.choice.Cause.present = NRPPA_Cause_PR_misc;
        ie->value.choice.Cause.choice.misc = failure_msg->cause.choice.misc;
        break;
      default:
        NRPPA_ERROR("Unknown MeasurementFailureIndication Cause\n");
        break;
    }
  }

  /* Encode NRPPA message */
  uint8_t *buffer = NULL;
  uint32_t length = 0;
  if (nrppa_gNB_encode_pdu(&pdu, &buffer, &length) < 0) {
    NRPPA_ERROR("Failed to encode Uplink NRPPa MeasurementFailureIndication \n");
    return -1;
  }

  /* Forward the NRPPA PDU to NGAP */
  nrppa_f1ap_info_t *info=&failure_msg->nrppa_msg_info;
  if (info->gNB_ue_ngap_id > 0 && info->amf_ue_ngap_id > 0) {
    LOG_D(NRPPA,
          "Sending UplinkUEAssociatedNRPPa (MeasurementFailureIndication) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkUEAssociatedNRPPa(info->instance,
                                                info->gNB_ue_ngap_id,
                                                info->amf_ue_ngap_id,
                                                info->routing_id_buffer,
                                                info->routing_id_length,
                                                buffer,
                                                length); // tx_nrppa_pdu=buffer, nrppa_pdu_length=length
    return length;
  } else if (info->gNB_ue_ngap_id == -1 && info->amf_ue_ngap_id == -1) //
  {
    LOG_D(NRPPA,
          "Sending UplinkNonUEAssociatedNRPPa (MeasurementFailureIndication) to NGAP (gNB_ue_ngap_id= %d, amf_ue_ngap_id =%ld)  \n",
          info->gNB_ue_ngap_id,
          info->amf_ue_ngap_id);
    nrppa_gNB_itti_send_UplinkNonUEAssociatedNRPPa(info->instance,
                                                   info->routing_id_buffer,
                                                   info->routing_id_length,
                                                   buffer,
                                                   length);
    return length;
  } else {
    NRPPA_ERROR("Failed to find context for Uplink NonUE/UE Associated NRPPa MeasurementFailureIndication \n");

    return -1;
  }
}
