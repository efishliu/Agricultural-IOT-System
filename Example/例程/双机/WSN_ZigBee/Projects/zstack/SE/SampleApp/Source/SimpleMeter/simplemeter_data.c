/**************************************************************************************************
  Filename:       simplemeter_data.c
  Revised:        $Date: 2007-08-29 10:52:11 -0700 (Wed, 29 Aug 2007) $
  Revision:       $Revision: 15242 $

  Description:    File that contains attribute and simple descriptor
                  definitions for the Simple Meter


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
#include "OSAL_Clock.h"
#include "ZDConfig.h"

#include "se.h"
#include "simplemeter.h"
#include "zcl_general.h"
#include "zcl_se.h"
#include "zcl_key_establish.h"

/*********************************************************************
 * CONSTANTS
 */
#define SIMPLEMETER_DEVICE_VERSION      0
#define SIMPLEMETER_FLAGS               0

#define SIMPLEMETER_HWVERSION           1
#define SIMPLEMETER_ZCLVERSION          1

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
const uint8 simpleMeterZCLVersion = SIMPLEMETER_ZCLVERSION;
const uint8 simpleMeterHWVersion = SIMPLEMETER_HWVERSION;
const uint8 simpleMeterManufacturerName[] = { 16, 'T','e','x','a','s','I','n','s','t','r','u','m','e','n','t','s' };
const uint8 simpleMeterModelId[] = { 16, 'T','I','0','0','0','1',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 simpleMeterDateCode[] = { 16, '2','0','0','6','0','8','3','1',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 simpleMeterPowerSource = POWER_SOURCE_MAINS_1_PHASE;

uint8 simpleMeterLocationDescription[] = { 16, ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
uint8 simpleMeterPhysicalEnvironment = PHY_UNSPECIFIED_ENV;
uint8 simpleMeterDeviceEnabled = DEVICE_ENABLED;

// Identify Cluster Attributes
uint16 simpleMeterIdentifyTime = 0;
uint32 simpleMeterTime = 0;
uint8 simpleMeterTimeStatus = 0x01;

// Simple Metering Cluster - Reading Information Set Attributes
uint8 simpleMeterCurrentSummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentSummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentMaxDemandDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentMaxDemandReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentTier1SummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentTier1SummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentTier2SummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentTier2SummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentTier3SummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentTier3SummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentTier4SummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentTier4SummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentTier5SummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentTier5SummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentTier6SummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterCurrentTier6SummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 simpleMeterDFTSummation[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint16 simpleMeterDailyFreezeTime = 0x01;
int8 simpleMeterPowerFactor = 0x01;
UTCTime simpleMeterSnapshotTime = 0x00;
UTCTime simpleMeterMaxDemandDeliverdTime = 0x00;
UTCTime simpleMeterMaxDemandReceivedTime = 0x00;

// Simple Metering Cluster - Meter Status Attributes
uint8 simpleMeterStatus = 0x12;

// Simple Metering Cluster - Formatting Attributes
uint8 simpleMeterUnitOfMeasure = 0x01;
uint24 simpleMeterMultiplier = 0x01;
uint24 simpleMeterDivisor = 0x01;
uint8 simpleMeterSummationFormating = 0x01;
uint8 simpleMeterDemandFormatting = 0x01;
uint8 simpleMeterHistoricalConsumptionFormatting = 0x01;

// Simple Metering Cluster - SimpleMeter Historical Consumption Attributes
uint24 simpleMeterInstanteneousDemand = 0x01;
uint24 simpleMeterCurrentdayConsumptionDelivered = 0x01;
uint24 simpleMeterCurrentdayConsumptionReceived = 0x01;
uint24 simpleMeterPreviousdayConsumptionDelivered = 0x01;
uint24 simpleMeterPreviousdayConsumtpionReceived = 0x01;
UTCTime simpleMeterCurrentPartialProfileIntervalStartTime = 0x1000;
uint24 simpleMeterCurrentPartialProfileIntervalValue = 0x0001;
uint8 simpleMeterMaxNumberOfPeriodsDelivered = 0x01;

// Key Establishment
uint16 simpleMeterKeyEstablishmentSuite = CERTIFICATE_BASED_KEY_ESTABLISHMENT;

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses Cluster IDs
 */
CONST zclAttrRec_t simpleMeterAttrs[SIMPLEMETER_MAX_ATTRIBUTES] =
{

  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,              // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_ZCL_VERSION,           // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&simpleMeterZCLVersion      // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_HW_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterHWVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterModelId
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterDateCode
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterPowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)simpleMeterLocationDescription
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENV,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&simpleMeterPhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&simpleMeterDeviceEnabled
    }
  },

  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&simpleMeterIdentifyTime
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
      (void *)&simpleMeterTime
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
      (void *)&simpleMeterTimeStatus
    }
  },


  // SE Attributes
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,              // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_SUMMATION_DELIVERED,        // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                          // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                          // Variable access control - found in zcl.h
      (void *)simpleMeterCurrentSummationDelivered  // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_SUMMATION_RECEIVED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentSummationReceived
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_MAX_DEMAND_DELIVERED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentMaxDemandDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_MAX_DEMAND_RECEIVED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentMaxDemandReceived
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER1_SUMMATION_DELIVERED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentTier1SummationDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER1_SUMMATION_RECEIVED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentTier1SummationReceived
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER2_SUMMATION_DELIVERED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentTier2SummationDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER2_SUMMATION_RECEIVED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentTier2SummationReceived
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER3_SUMMATION_DELIVERED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentTier3SummationDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER3_SUMMATION_RECEIVED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentTier3SummationReceived
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER4_SUMMATION_DELIVERED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentTier4SummationDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER4_SUMMATION_RECEIVED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentTier4SummationReceived
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER5_SUMMATION_DELIVERED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentTier5SummationDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER5_SUMMATION_RECEIVED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentTier5SummationReceived
    }
  },
   {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER6_SUMMATION_DELIVERED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentTier5SummationDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER6_SUMMATION_RECEIVED,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterCurrentTier5SummationReceived
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_DFT_SUMMATION,
      ZCL_DATATYPE_UINT48,
      ACCESS_CONTROL_READ,
      (void *)simpleMeterDFTSummation
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_DAILY_FREEZE_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterDailyFreezeTime
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_POWER_FACTOR,
      ZCL_DATATYPE_INT8,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterPowerFactor
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_READING_SNAPSHOT_TIME,
      ZCL_DATATYPE_UTC,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterSnapshotTime
    }
  },
    {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_MAX_DEMAND_DELIVERD_TIME,
      ZCL_DATATYPE_UTC,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterMaxDemandDeliverdTime
    }
  },
    {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_MAX_DEMAND_RECEIVED_TIME,
      ZCL_DATATYPE_UTC,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterMaxDemandReceivedTime
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_STATUS,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterStatus
    }
  },

  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_UNIT_OF_MEASURE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterUnitOfMeasure
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_MULTIPLIER,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterMultiplier
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_DIVISOR,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterDivisor
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_SUMMATION_FORMATTING,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterSummationFormating
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_DEMAND_FORMATTING,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterDemandFormatting
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_HISTORICAL_CONSUMPTION_FORMATTING,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterHistoricalConsumptionFormatting
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_INSTANTANEOUS_DEMAND,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterInstanteneousDemand
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENTDAY_CONSUMPTION_DELIVERED,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterCurrentdayConsumptionDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENTDAY_CONSUMPTION_RECEIVED,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterCurrentdayConsumptionReceived
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_PREVIOUSDAY_CONSUMPTION_DELIVERED,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterPreviousdayConsumptionDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_PREVIOUSDAY_CONSUMPTION_RECEIVED,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterPreviousdayConsumtpionReceived
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_PARTIAL_PROFILE_INTERVAL_START_TIME,
      ZCL_DATATYPE_UTC,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterCurrentPartialProfileIntervalStartTime
    }
  },

  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_PARTIAL_PROFILE_INTERVAL_VALUE,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterCurrentPartialProfileIntervalValue
    }
  },

  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_MAX_NUMBER_OF_PERIODS_DELIVERED,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterMaxNumberOfPeriodsDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_KEY_ESTABLISHMENT,
    {  // Attribute record
      ATTRID_KEY_ESTABLISH_SUITE,
      ZCL_DATATYPE_BITMAP16,
      ACCESS_CONTROL_READ,
      (void *)&simpleMeterKeyEstablishmentSuite
    }
  },
};

/*********************************************************************
 * CLUSTER OPTION DEFINITIONS
 */
zclOptionRec_t simpleMeterOptions[SIMPLEMETER_MAX_OPTIONS] =
{
  // *** General Cluster Options ***
  {
    ZCL_CLUSTER_ID_GEN_TIME,                      // Cluster IDs - defined in the foundation (ie. zcl.h)
    ( AF_EN_SECURITY /*| AF_ACK_REQUEST*/ ),      // Options - Found in AF header (ie. AF.h)
  },

  // *** Smart Energy Cluster Options ***
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    ( AF_EN_SECURITY ),
  },
};

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
#define SIMPLEMETER_MAX_INCLUSTERS       3
const cId_t simpleMeterInClusterList[SIMPLEMETER_MAX_INCLUSTERS] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_SE_SIMPLE_METERING
};

#define SIMPLEMETER_MAX_OUTCLUSTERS       3
const cId_t simpleMeterOutClusterList[SIMPLEMETER_MAX_OUTCLUSTERS] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_SE_SIMPLE_METERING
};

SimpleDescriptionFormat_t simpleMeterSimpleDesc =
{
  SIMPLEMETER_ENDPOINT,                       //  uint8 Endpoint;
  ZCL_SE_PROFILE_ID,                          //  uint16 AppProfId[2];
  ZCL_SE_DEVICEID_METER,                      //  uint16 AppDeviceId[2];
  SIMPLEMETER_DEVICE_VERSION,                 //  int   AppDevVer:4;
  SIMPLEMETER_FLAGS,                          //  int   AppFlags:4;
  SIMPLEMETER_MAX_INCLUSTERS,                 //  uint8  AppNumInClusters;
  (cId_t *)simpleMeterInClusterList,          //  cId_t *pAppInClusterList;
  SIMPLEMETER_MAX_OUTCLUSTERS,                //  uint8  AppNumInClusters;
  (cId_t *)simpleMeterOutClusterList          //  cId_t *pAppInClusterList;
};

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/****************************************************************************
****************************************************************************/


