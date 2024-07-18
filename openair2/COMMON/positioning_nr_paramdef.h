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

/*! \file positioning_nr_paramdef.h
 * \brief definition of configuration parameters rquired for 5G positioning
 * \author
 * \date 2024
 * \version 0.1
 * \company Firecell
 * \email: adeel.malik@firecell.io
 * \note
 * \warning
 */

#ifndef __POSITIONING_NR_PARAMDEF__H__
#define __POSITIONING_NR_PARAMDEF__H__

/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/* Positioning configuration section names */
#define CONFIG_STRING_POSITIONING_CONFIG                            "positioning_config"


/* Global parameters */

/* PRS configuration parameters names */
#define CONFIG_STRING_POSITIONING_NUM_TRPS                     "NumTRPs"
#define CONFIG_STRING_POSITIONING_TRP_IDS_LIST                 "TRPIDs"
#define CONFIG_STRING_POSITIONING_TRP_X_AXIS_LIST              "TRPxAxis"
#define CONFIG_STRING_POSITIONING_TRP_Y_AXIS_LIST              "TRPyAxis"
#define CONFIG_STRING_POSITIONING_TRP_Z_AXIS_LIST              "TRPzAxis"

/* Help string for Positioning parameters */
#define HELP_STRING_POSITIONING_NUM_TRPS                       "Number of TRPs connected with gNB(max 4)\n"
#define HELP_STRING_POSITIONING_TRP_IDS_LIST                   "User defined IDs for each TRP \n"
#define HELP_STRING_POSITIONING_TRP_X_AXIS_LIST                "x-axis value of each TRP \n"
#define HELP_STRING_POSITIONING_TRP_Y_AXIS_LIST                "y-axis value of each TRP \n"
#define HELP_STRING_POSITIONING_TRP_Z_AXIS_LIST                "z-axis value of each TRP \n"

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*                                            PRS configuration                parameters                                                                         */
/*   optname                                         helpstr                  paramflags    XXXptr              defXXXval                  type           numelt  */
/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/
// clang-format off
#define POSITIONING_PARAMS_DESC { \
  {CONFIG_STRING_POSITIONING_NUM_TRPS,             HELP_STRING_POSITIONING_NUM_TRPS,            0,  .uptr=NULL,         .defuintval=0,              TYPE_UINT,       0},  \
  {CONFIG_STRING_POSITIONING_TRP_IDS_LIST,         HELP_STRING_POSITIONING_TRP_IDS_LIST,        0,  .uptr=NULL,         .defintarrayval=0,          TYPE_UINTARRAY,  0},  \
  {CONFIG_STRING_POSITIONING_TRP_X_AXIS_LIST,      HELP_STRING_POSITIONING_TRP_X_AXIS_LIST,     0,  .uptr=NULL,         .defintarrayval=0,          TYPE_UINTARRAY,  0},  \
  {CONFIG_STRING_POSITIONING_TRP_Y_AXIS_LIST,      HELP_STRING_POSITIONING_TRP_Y_AXIS_LIST,     0,  .uptr=NULL,         .defintarrayval=0,          TYPE_UINTARRAY,  0},  \
  {CONFIG_STRING_POSITIONING_TRP_Z_AXIS_LIST,      HELP_STRING_POSITIONING_TRP_Z_AXIS_LIST,     0,  .uptr=NULL,         .defintarrayval=0,          TYPE_UINTARRAY,  0},  \
}
// clang-format on

#define POSITIONING_NUM_TRPS                         0
#define POSITIONING_TRP_IDS_LIST                      1
#define POSITIONING_TRP_X_AXIS_LIST                  2
#define POSITIONING_TRP_Y_AXIS_LIST                  3
#define POSITIONING_TRP_Z_AXIS_LIST                  4
/*----------------------------------------------------------------------------------------------------------------------------------------------------*/

#endif
