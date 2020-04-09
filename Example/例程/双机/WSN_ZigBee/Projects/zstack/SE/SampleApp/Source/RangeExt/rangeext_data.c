/**************************************************************************************************
  Filename:       rangeext_data.c
  Revised:        $Date: 2007-08-29 10:52:11 -0700 (Wed, 29 Aug 2007) $
  Revision:       $Revision: 15242 $

  Description:    File that contains attribute and simple descriptor
                  definitions for the Range Extender


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

#include "ZDConfig.h"

#include "se.h"
#include "rangeext.h"
#include "zcl_general.h"
#include "zcl_key_establish.h"

/*********************************************************************
 * CONSTANTS
 */
#define RANGEEXT_DEVICE_VERSION      0
#define RANGEEXT_FLAGS               0

#define RANGEEXT_HWVERSION           1
#define RANGEEXT_ZCLVERSION          1

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
const uint8 rangeExtZCLVersion = RANGEEXT_ZCLVERSION;
const uint8 rangeExtHWVersion = RANGEEXT_HWVERSION;
const uint8 rangeExtManufacturerName[] = { 16, 'T','e','x','a','s','I','n','s','t','r','u','m','e','n','t','s' };
const uint8 rangeExtModelId[] = { 16, 'T','I','0','0','0','1',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 rangeExtDateCode[] = { 16, '2','0','0','6','0','8','3','1',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 rangeExtPowerSource = POWER_SOURCE_MAINS_1_PHASE;

uint8 rangeExtLocationDescription[] = { 16, ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
uint8 rangeExtPhysicalEnvironment = PHY_UNSPECIFIED_ENV;
uint8 rangeExtDeviceEnabled = DEVICE_ENABLED;

// Identify Cluster Attributes
uint16 rangeExtIdentifyTime = 0;
uint32 rangeExtTime = 0;
uint8 rangeExtTimeStatus = 0x01;

// Key Establishment
uint8 zclRangeExt_KeyEstablishmentSuite = CERTIFICATE_BASED_KEY_ESTABLISHMENT;

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses Cluster IDs
 */
CONST zclAttrRec_t rangeExtAttrs[RANGEEXT_MAX_ATTRIBUTES] =
{

  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_ZCL_VERSION,           // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&rangeExtZCLVersion         // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_HW_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&rangeExtHWVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)rangeExtManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)rangeExtModelId
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)rangeExtDateCode
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&rangeExtPowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)rangeExtLocationDescription
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENV,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&rangeExtPhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&rangeExtDeviceEnabled
    }
  },

  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&rangeExtIdentifyTime
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
      (void *)&rangeExtTime
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
      (void *)&rangeExtTimeStatus
    }
  },


  // SE Attributes
  {
    ZCL_CLUSTER_ID_GEN_KEY_ESTABLISHMENT,
    {  // Attribute record
      ATTRID_KEY_ESTABLISH_SUITE,
      ZCL_DATATYPE_BITMAP16,
      ACCESS_CONTROL_READ,
      (void *)&zclRangeExt_KeyEstablishmentSuite
    }
  },
};

/*********************************************************************
 * CLUSTER OPTION DEFINITIONS
 */
zclOptionRec_t rangeExtOptions[RANGEEXT_MAX_OPTIONS] =
{
  // *** General Cluster Options ***
  {
    ZCL_CLUSTER_ID_GEN_TIME,                     // Cluster IDs - defined in the foundation (ie. zcl.h)
    ( AF_EN_SECURITY /*| AF_ACK_REQUEST*/ ),     // Options - Found in AF header (ie. AF.h)
  },
};

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
#define RANGEEXT_MAX_INCLUSTERS       2
const cId_t rangeExtInClusterList[RANGEEXT_MAX_INCLUSTERS] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY
};

#define RANGEEXT_MAX_OUTCLUSTERS      2
const cId_t rangeExtOutClusterList[RANGEEXT_MAX_OUTCLUSTERS] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY
};

SimpleDescriptionFormat_t rangeExtSimpleDesc =
{
  RANGEEXT_ENDPOINT,                    //  uint8 Endpoint;
  ZCL_SE_PROFILE_ID,                    //  uint16 AppProfId[2];
  ZCL_SE_DEVICEID_RANGE_EXTENDER,       //  uint16 AppDeviceId[2];
  RANGEEXT_DEVICE_VERSION,              //  int   AppDevVer:4;
  RANGEEXT_FLAGS,                       //  int   AppFlags:4;
  RANGEEXT_MAX_INCLUSTERS,              //  uint8  AppNumInClusters;
  (cId_t *)rangeExtInClusterList,       //  cId_t *pAppInClusterList;
  RANGEEXT_MAX_OUTCLUSTERS,             //  uint8  AppNumInClusters;
  (cId_t *)rangeExtOutClusterList       //  cId_t *pAppInClusterList;
};

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/****************************************************************************
****************************************************************************/


