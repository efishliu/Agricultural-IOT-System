/**************************************************************************************************
  Filename:       ipd_data.c
  Revised:        $Date: 2007-08-29 10:52:11 -0700 (Wed, 29 Aug 2007) $
  Revision:       $Revision: 15242 $

  Description:    File that contains attribute and simple descriptor
                  definitions for the IPD


  Copyright 2009 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "OSAL.h"
#include "ZDConfig.h"

#include "se.h"
#include "ipd.h"
#include "zcl_general.h"
#include "zcl_key_establish.h"

/*********************************************************************
 * CONSTANTS
 */
#define IPD_DEVICE_VERSION      0
#define IPD_FLAGS               0

#define IPD_HWVERSION           1
#define IPD_ZCLVERSION          1

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Basic Cluster
const uint8 ipdZCLVersion = IPD_ZCLVERSION;
const uint8 ipdHWVersion = IPD_HWVERSION;
const uint8 ipdManufacturerName[] = { 16, 'T','e','x','a','s','I','n','s','t','r','u','m','e','n','t','s' };
const uint8 ipdModelId[] = { 16, 'T','I','0','0','0','1',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 ipdDateCode[] = { 16, '2','0','0','6','0','8','3','1',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 ipdPowerSource = POWER_SOURCE_MAINS_1_PHASE;

uint8 ipdLocationDescription[] = { 16, ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
uint8 ipdPhysicalEnvironment = PHY_UNSPECIFIED_ENV;
uint8 ipdDeviceEnabled = DEVICE_ENABLED;

// Identify Cluster Attributes
uint16 ipdIdentifyTime = 0;
uint32 ipdTime = 0;
uint8 ipdTimeStatus = 0x01;

// Key Establishment
uint16 ipdKeyEstablishmentSuite = CERTIFICATE_BASED_KEY_ESTABLISHMENT;

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses Cluster IDs
 */
CONST zclAttrRec_t ipdAttrs[IPD_MAX_ATTRIBUTES] =
{

  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_ZCL_VERSION,           // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&ipdZCLVersion              // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_HW_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&ipdHWVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)ipdManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)ipdModelId
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)ipdDateCode
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&ipdPowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)ipdLocationDescription
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENV,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&ipdPhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&ipdDeviceEnabled
    }
  },

  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&ipdIdentifyTime
    }
  },

  // *** Time Cluster Attribute ***
  // In SE domain, only master clock will be used. Therefore
  // mark the access control to only readable.
  {
    ZCL_CLUSTER_ID_GEN_TIME,
    { // Attribute record
      ATTRID_TIME_TIME,
      ZCL_DATATYPE_UTC,
      ACCESS_CONTROL_READ,
      (void *)&ipdTime
    }
  },
  // In SE domain, only master clock will be used. Therefore
  // mark the access control to only readable.
  {
    ZCL_CLUSTER_ID_GEN_TIME,
    { // Attribute record
      ATTRID_TIME_STATUS,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ,
      (void *)&ipdTimeStatus
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_KEY_ESTABLISHMENT,
    {  // Attribute record
      ATTRID_KEY_ESTABLISH_SUITE,
      ZCL_DATATYPE_BITMAP16,
      ACCESS_CONTROL_READ,
      (void *)&ipdKeyEstablishmentSuite
    }
  },
};

/*********************************************************************
 * CLUSTER OPTION DEFINITIONS
 */
zclOptionRec_t ipdOptions[IPD_MAX_OPTIONS] =
{
  // *** General Cluster Options ***
  {
    ZCL_CLUSTER_ID_GEN_TIME,                     // Cluster IDs - defined in the foundation (ie. zcl.h)
    ( AF_EN_SECURITY /*| AF_ACK_REQUEST*/ ),     // Options - Found in AF header (ie. AF.h)
  },

  // *** Smart Energy Cluster Options ***
  {
    ZCL_CLUSTER_ID_SE_PRICING,
    ( AF_EN_SECURITY ),
  },
  {
    ZCL_CLUSTER_ID_SE_MESSAGE,
    ( AF_EN_SECURITY ),
  },
};

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
#define IPD_MAX_INCLUSTERS       4
const cId_t ipdInClusterList[IPD_MAX_INCLUSTERS] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_SE_PRICING,
  ZCL_CLUSTER_ID_SE_MESSAGE
};

#define IPD_MAX_OUTCLUSTERS       4
const cId_t ipdOutClusterList[IPD_MAX_OUTCLUSTERS] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_SE_PRICING,
  ZCL_CLUSTER_ID_SE_MESSAGE
};

SimpleDescriptionFormat_t ipdSimpleDesc =
{
  IPD_ENDPOINT,                       //  uint8 Endpoint;
  ZCL_SE_PROFILE_ID,                  //  uint16 AppProfId[2];
  ZCL_SE_DEVICEID_IN_PREMISE_DISPLAY, //  uint16 AppDeviceId[2];
  IPD_DEVICE_VERSION,                 //  int   AppDevVer:4;
  IPD_FLAGS,                          //  int   AppFlags:4;
  IPD_MAX_INCLUSTERS,                 //  uint8  AppNumInClusters;
  (cId_t *)ipdInClusterList,          //  cId_t *pAppInClusterList;
  IPD_MAX_OUTCLUSTERS,                //  uint8  AppNumInClusters;
  (cId_t *)ipdOutClusterList          //  cId_t *pAppInClusterList;
};

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/****************************************************************************
****************************************************************************/


