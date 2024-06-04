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
/*! \file nfapi/oai_integration/wls_integration/wls_pnf.c
 * \brief
 * \author Ruben S. Silva
 * \date 2024
 * \version 0.1
 * \company OpenAirInterface Software Alliance
 * \email: contact@openairinterface.org, rsilva@allbesmart.pt
 * \note
 * \warning
 */

#include "nfapi/oai_integration/wls_integration/include/wls_pnf.h"
#include "pnf.h"
#define WLS_TEST_DEV_NAME "wls"
#define WLS_TEST_MSG_ID 1
#define WLS_TEST_MSG_SIZE 100
#define WLS_MAC_MEMORY_SIZE 0x3EA80000
#define WLS_PHY_MEMORY_SIZE 0x18000000
#define NUM_PHY_MSGS 16

typedef void *WLS_HANDLE;
void *g_shmem;
uint64_t g_shmem_size;

WLS_HANDLE g_phy_wls;


uint8_t phy_dpdk_init(void);
uint8_t phy_wls_init(const char *dev_name, uint64_t nWlsMacMemSize, uint64_t nWlsPhyMemSize);
uint64_t phy_mac_recv();
uint8_t phy_mac_send();

static pnf_t *_this = NULL;
static nfapi_pnf_config_t *cfg ;

void *wls_fapi_pnf_nr_start_thread(void *ptr)
{
  NFAPI_TRACE(NFAPI_TRACE_INFO, "[PNF] IN WLS PNF NFAPI start thread %s\n", __FUNCTION__);
  cfg = (nfapi_pnf_config_t *)ptr;
  struct sched_param sp;
  sp.sched_priority = 20;
  pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp);
  wls_fapi_nr_pnf_start();
  return (void *)0;
}

int wls_fapi_nr_pnf_start()
{
  int64_t ret;
  uint64_t p_msg;
  // Verify that config is not null
  if (cfg == 0)
    return -1;

  NFAPI_TRACE(NFAPI_TRACE_INFO, "%s\n", __FUNCTION__);

  _this = (pnf_t *)(cfg->user_data);
  _this->terminate = 0;
  // Init PNF config
  nfapi_pnf_phy_config_t* phy = (nfapi_pnf_phy_config_t*)malloc(sizeof(nfapi_pnf_phy_config_t));
  memset(phy, 0, sizeof(nfapi_pnf_phy_config_t));

  phy->state = NFAPI_PNF_PHY_IDLE;
  phy->phy_id = 0;
  phy->next = (_this->_public).phys;
  (_this->_public).phys = phy;

  // DPDK init
  ret = phy_dpdk_init();
  if (!ret) {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "[PNF]  DPDK Init - Failed\n");
    return false;
  }
  NFAPI_TRACE(NFAPI_TRACE_INFO, "\n[PHY] DPDK Init - Done\n");

  // WLS init
  ret = phy_wls_init(WLS_TEST_DEV_NAME, WLS_MAC_MEMORY_SIZE, WLS_PHY_MEMORY_SIZE);
  if (!ret) {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "[PNF]  WLS Init - Failed\n");
    return false;
  }
  NFAPI_TRACE(NFAPI_TRACE_INFO, "\n[PHY] WLS Init - Done\n");

  // Start Receiving loop from the VNF
  printf("Start Receiving loop from the VNF\n");
  p_msg = phy_mac_recv();
  if (!p_msg) {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "[PNF]  Receive from FAPI - Failed\n");
    return false;
  }
  NFAPI_TRACE(NFAPI_TRACE_INFO, "\n[PHY] Receive from FAPI - Done\n");


  // Should never get here, to be removed, align with present implementation of PNF loop
  // Send to MAC WLS
  ret = phy_mac_send();
  if (!ret) {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "[PNF]  Send to FAPI FAPI - Failed\n");
    return false;
  }
  NFAPI_TRACE(NFAPI_TRACE_INFO, "\n[PHY] Send to FAPI - Done\n");

  NFAPI_TRACE(NFAPI_TRACE_INFO, "\n[PHY] Exiting...\n");

  return true;
}

uint8_t phy_dpdk_init(void)
{
  unsigned long i;
  char *argv[] = {"phy_app", "--proc-type=primary", "--file-prefix", "wls", "--iova-mode=pa", "--no-pci"};
  int argc = RTE_DIM(argv);
  /* initialize EAL first */
  NFAPI_TRACE(NFAPI_TRACE_INFO, "\n[PHY] Calling rte_eal_init: ");

  for (i = 0; i < RTE_DIM(argv); i++) {
    NFAPI_TRACE(NFAPI_TRACE_INFO, "%s ", argv[i]);
  }

  NFAPI_TRACE(NFAPI_TRACE_INFO, "\n");

  if (rte_eal_init(argc, argv) < 0)
    rte_panic("Cannot init EAL\n");

  return true;
}

uint8_t phy_wls_init(const char *dev_name, uint64_t nWlsMacMemSize, uint64_t nWlsPhyMemSize)
{
  g_phy_wls = WLS_Open(dev_name, WLS_SLAVE_CLIENT, &nWlsMacMemSize, &nWlsPhyMemSize);
  if (NULL == g_phy_wls) {
    return false;
  }
  g_shmem_size = nWlsMacMemSize + nWlsPhyMemSize;

  g_shmem = WLS_Alloc(g_phy_wls, g_shmem_size);
  if (NULL == g_shmem) {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "Unable to alloc WLS Memory\n");
    return false;
  }
  return true;
}

static void procPhyMessages(uint32_t msgSize, void *msg , uint16_t msgId)
{

  //Should be able to be casted into already present fapi p5 header format, to fix, dependent on changes on L2 to follow fapi_message_header_t in nr_fapi.h
  fapi_msg_t *header  = (fapi_msg_t *)msg;
  printf("[PHY] Received Msg ID 0x%02x\n", msgId);
  NFAPI_TRACE(NFAPI_TRACE_INFO, "[PHY] Received Msg ID 0x%02x\n", msgId);

  switch (msgId) {
    case NFAPI_NR_PHY_MSG_TYPE_CONFIG_REQUEST: {
#if 0 // To print TLVs, only for debug purposes
      fapi_config_req_t *wls_conf_req = (fapi_config_req_t *)msg;
      printf("number_of_tlvs %d \n", wls_conf_req->number_of_tlvs);
      for (int i = 0; i < wls_conf_req->number_of_tlvs; ++i) {
        printf("TLV #%d tag 0x%02x length 0x%02x value 0x%02x\n",
               i,
               wls_conf_req->tlvs[i].tl.tag,
               wls_conf_req->tlvs[i].tl.length,
               wls_conf_req->tlvs[i].value);
      }
#endif
      nfapi_nr_config_request_scf_t req = {0};
      nfapi_pnf_config_t* config = &(_this->_public);

      int unpack_result = fapi_nr_p5_message_unpack(msg, msgSize, &req, sizeof(req), NULL);
      // TODO: Process and use the message
      if(config->state == NFAPI_PNF_RUNNING)
      {
        nfapi_pnf_phy_config_t* phy = nfapi_pnf_phy_config_find(config, req.header.phy_id);
        if(phy)
        {
          if(phy->state != NFAPI_PNF_PHY_RUNNING)
          {
            if(config->nr_config_req)
            {
              (config->nr_config_req)(config, phy, &req);
            }
          }
          else
          {
            nfapi_nr_config_response_scf_t resp;
            memset(&resp, 0, sizeof(resp));
            resp.header.message_id = NFAPI_CONFIG_RESPONSE;
            resp.header.phy_id = req.header.phy_id;
            resp.error_code = NFAPI_MSG_INVALID_STATE;
            printf("Try sending response back NFAPI_MSG_INVALID_STATE\n");
            nfapi_nr_pnf_config_resp(config, &resp);
          }
        }
        else
        {
          nfapi_nr_config_response_scf_t resp;
          memset(&resp, 0, sizeof(resp));
          resp.header.message_id = NFAPI_CONFIG_RESPONSE;
          resp.header.phy_id = req.header.phy_id;
          resp.error_code = NFAPI_MSG_INVALID_CONFIG;
          printf("Try sending response back NFAPI_MSG_INVALID_CONFIG\n");
          nfapi_nr_pnf_config_resp(config, &resp);
        }
      }
      else
      {
        nfapi_nr_config_response_scf_t resp;
        memset(&resp, 0, sizeof(resp));
        resp.header.message_id = NFAPI_CONFIG_RESPONSE;
        resp.header.phy_id = req.header.phy_id;
        resp.error_code = NFAPI_MSG_INVALID_STATE;
        printf("Try sending response back NFAPI_MSG_INVALID_STATE\n");
        nfapi_nr_pnf_config_resp(config, &resp);
      }
      // Free at the end
      free_config_request(&req);
      break;
    }
    default:
      break;
  }
}

uint64_t phy_mac_recv()
{
  uint32_t numMsgToGet; /* Number of Memory blocks to get */
  uint64_t l2Msg; /* Message received */
  void *l2MsgPtr;
  uint32_t msgSize;
  uint16_t msgType;
  uint16_t flags;
  uint32_t i = 0;
  p_fapi_api_queue_elem_t currElem = NULL;
  while (_this->terminate == 0) {
    printf("Before WLS_Wait\n");
    numMsgToGet = WLS_Wait(g_phy_wls);
    if (numMsgToGet == 0) {
      continue;
    }
    printf("After WLS_Wait, numMsgToGet = %d\n",numMsgToGet);

    while (numMsgToGet--) {
      currElem = NULL;
      l2Msg = (uint64_t)NULL;
      l2MsgPtr = NULL;
      printf("Before WLS_Get\n");
      l2Msg = WLS_Get(g_phy_wls, &msgSize, &msgType, &flags);
      printf("After WLS_Get\n");
      if (l2Msg) {
        printf("\n[PHY] MAC2PHY WLS Received Block %d\n", i);
        NFAPI_TRACE(NFAPI_TRACE_INFO, "\n[PHY] MAC2PHY WLS Received Block %d\n", i);
        i++;
        l2MsgPtr = WLS_PA2VA(g_phy_wls, l2Msg);
        currElem = (p_fapi_api_queue_elem_t)l2MsgPtr;

        uint8_t * msgt = (uint8_t *)(currElem + 1);
        for (int x = 0; x < msgSize; ++x) {
          printf("0x%02x ", msgt[x]);
        }

        printf("\n");

        if (currElem->msg_type != FAPI_VENDOR_MSG_HEADER_IND) {
          procPhyMessages(currElem->msg_len, (void *)(currElem + 1), currElem->msg_type);
        }
        currElem = NULL;
      } else {
        NFAPI_TRACE(NFAPI_TRACE_ERROR, "\n[PHY] MAC2PHY WLS Get Error for msg %d\n", i);
        break;
      }
      if (flags & WLS_TF_FIN) {
        // Don't return, the messages responses will be handled in procPhyMessages
      }
    }
  }
  return l2Msg;
}

uint8_t phy_mac_send()
{
  uint64_t pa_block = 0;
  uint8_t ret = false;
  uint32_t i;

  for (i = 0; i < NUM_PHY_MSGS; i++) {
    pa_block = (uint64_t)WLS_DequeueBlock((void *)g_phy_wls);
    if (!pa_block) {
      NFAPI_TRACE(NFAPI_TRACE_ERROR, "\n[PHY] FAPI2PHY WLS Dequeue block %d error\n", i);
      return false;
    }

    ret = WLS_Put(g_phy_wls, pa_block, WLS_TEST_MSG_SIZE, WLS_TEST_MSG_ID, (i == (NUM_PHY_MSGS - 1)) ? WLS_TF_FIN : 0) == 0 ? true
                                                                                                                            : false;
    NFAPI_TRACE(NFAPI_TRACE_INFO, "\n[PHY] FAPI2PHY WLS Put Msg %d \n", i);
    if (ret == false) {
      NFAPI_TRACE(NFAPI_TRACE_ERROR, "\n[PHY] FAPI2PHY WLS Put Msg Error %d \n", i);
    }
  }
  return ret;
}
