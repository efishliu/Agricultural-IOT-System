/**************************************************************************************************
  Filename:       esp_data.c
  Revised:        $Date: 2007-08-29 10:52:11 -0700 (Wed, 29 Aug 2007) $
  Revision:       $Revision: 15242 $

  Description:    File that contains attribute and simple descriptor
                  definitions for the ESP


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
#include "ZComDef.h"
#include "OSAL_Clock.h"
#include "ZDConfig.h"

#include "se.h"
#include "esp.h"
#include "zcl_general.h"
#include "zcl_se.h"
#include "zcl_key_establish.h"

/*********************************************************************
 * CONSTANTS
 */
#define ESP_DEVICE_VERSION      0
#define ESP_FLAGS               0

#define ESP_HWVERSION           1
#define ESP_ZCLVERSION          1

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
const uint8 espZCLVersion = ESP_ZCLVERSION;
const uint8 espHWVersion = ESP_HWVERSION;
const uint8 espManufacturerName[] = { 16, 'T','e','x','a','s','I','n','s','t','r','u','m','e','n','t','s' };
const uint8 espModelId[] = { 16, 'T','I','0','0','0','1',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 espDateCode[] = { 16, '2','0','0','6','0','8','3','1',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 espPowerSource = POWER_SOURCE_MAINS_1_PHASE;

uint8 espLocationDescription[] = { 16, ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
uint8 espPhysicalEnvironment = PHY_UNSPECIFIED_ENV;
uint8 espDeviceEnabled = DEVICE_ENABLED;

// Identify Cluster Attributes
uint16 espIdentifyTime = 0;
uint32 espTime = 0;
uint8 espTimeStatus = 0x01;

// Simple Metering Cluster - Reading Information Set Attributes
uint8 espCurrentSummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentSummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentMaxDemandDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentMaxDemandReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentTier1SummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentTier1SummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentTier2SummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentTier2SummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentTier3SummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentTier3SummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentTier4SummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentTier4SummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentTier5SummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentTier5SummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentTier6SummationDelivered[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espCurrentTier6SummationReceived[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint8 espDFTSummation[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
uint16 espDailyFreezeTime = 0x01;
int8 espPowerFactor = 0x01;
UTCTime espSnapshotTime = 0x00;
UTCTime espMaxDemandDeliverdTime = 0x00;
UTCTime espMaxDemandReceivedTime = 0x00;

// Simple Metering Cluster - Meter Status Attributes
uint8 espStatus = 0x12;

// Simple Metering Cluster - Formatting Attributes
uint8 espUnitOfMeasure = 0x01;
uint24 espMultiplier = 0x01;
uint24 espDivisor = 0x01;
uint8 espSummationFormating = 0x01;
uint8 espDemandFormatting = 0x01;
uint8 espHistoricalConsumptionFormatting = 0x01;

// Simple Metering Cluster - esp Historical Consumption Attributes
uint24 espInstanteneousDemand = 0x01;
uint24 espCurrentdayConsumptionDelivered = 0x01;
uint24 espCurrentdayConsumptionReceived = 0x01;
uint24 espPreviousdayConsumptionDelivered = 0x01;
uint24 espPreviousdayConsumtpionReceived = 0x01;
UTCTime espCurrentPartialProfileIntervalStartTime = 0x1000;
uint24 espCurrentPartialProfileIntervalValue = 0x0001;
uint8 espMaxNumberOfPeriodsDelivered = 0x01;

// Demand Response and Load Control
uint8 espUtilityDefinedGroup = 0x00;
uint8 espStartRandomizeMinutes = 0x00;
uint8 espStopRandomizeMinutes = 0x00;
uint8 espSignature[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

// Key Establishment
uint16 espKeyEstablishmentSuite = CERTIFICATE_BASED_KEY_ESTABLISHMENT;

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses Cluster IDs
 */
CONST zclAttrRec_t espAttrs[ESP_MAX_ATTRIBUTES] =
{

  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_ZCL_VERSION,           // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&espZCLVersion              // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_HW_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&espHWVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)espManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)espModelId
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)espDateCode
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&espPowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)espLocationDescription
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENV,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&espPhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&espDeviceEnabled
    }
  },

  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&espIdentifyTime
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
      (void *)&espTime
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
      (void *)&espTimeStatus
    }
  },


  // SE Attributes
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,          // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_SUMMATION_DELIVERED,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                      // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                      // Variable access control - found in zcl.h
      (void *)espCurrentSummationDelivered      // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,          // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_SUMMATION_RECEIVED,     // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                      // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                      // Variable access control - found in zcl.h
      (void *)espCurrentSummationReceived       // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,          // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_MAX_DEMAND_DELIVERED,   // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                      // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                      // Variable access control - found in zcl.h
      (void *)espCurrentMaxDemandDelivered      // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,          // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_MAX_DEMAND_RECEIVED,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                      // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                      // Variable access control - found in zcl.h
      (void *)espCurrentMaxDemandReceived       // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,              // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER1_SUMMATION_DELIVERED,  // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                          // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                          // Variable access control - found in zcl.h
      (void *)espCurrentTier1SummationDelivered     // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,              // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER1_SUMMATION_RECEIVED,   // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                          // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                          // Variable access control - found in zcl.h
      (void *)espCurrentTier1SummationReceived      // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,                // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER2_SUMMATION_DELIVERED,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                            // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                            // Variable access control - found in zcl.h
      (void *)espCurrentTier2SummationDelivered       // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,              // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER2_SUMMATION_RECEIVED,   // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                          // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                          // Variable access control - found in zcl.h
      (void *)espCurrentTier2SummationReceived      // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,                // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER3_SUMMATION_DELIVERED,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                            // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                            // Variable access control - found in zcl.h
      (void *)espCurrentTier3SummationDelivered       // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,               // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER3_SUMMATION_RECEIVED,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                           // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                           // Variable access control - found in zcl.h
      (void *)espCurrentTier3SummationReceived       // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,                // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER4_SUMMATION_DELIVERED,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                            // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                            // Variable access control - found in zcl.h
      (void *)espCurrentTier4SummationDelivered       // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,               // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER4_SUMMATION_RECEIVED,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                           // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                           // Variable access control - found in zcl.h
      (void *)espCurrentTier4SummationReceived       // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,                // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER5_SUMMATION_DELIVERED,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                            // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                            // Variable access control - found in zcl.h
      (void *)espCurrentTier5SummationDelivered       // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,               // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER5_SUMMATION_RECEIVED,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                           // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                           // Variable access control - found in zcl.h
      (void *)espCurrentTier5SummationReceived       // Pointer to attribute variable
    }
  },
   {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,                // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER6_SUMMATION_DELIVERED,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                            // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                            // Variable access control - found in zcl.h
      (void *)espCurrentTier6SummationDelivered       // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,               // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_TIER6_SUMMATION_RECEIVED,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                           // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                           // Variable access control - found in zcl.h
      (void *)espCurrentTier6SummationReceived       // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,       // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_DFT_SUMMATION,               // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT48,                   // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                   // Variable access control - found in zcl.h
      (void *)espDFTSummation                // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,       // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_DAILY_FREEZE_TIME,           // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT16,                   // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                   // Variable access control - found in zcl.h
      (void *)&espDailyFreezeTime            // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,       // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_POWER_FACTOR,                // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_INT8,                     // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                   // Variable access control - found in zcl.h
      (void *)&espPowerFactor                // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,       // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_READING_SNAPSHOT_TIME,       // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UTC,                      // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                   // Variable access control - found in zcl.h
      (void *)&espSnapshotTime               // Pointer to attribute variable
    }
  },
    {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,               // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_MAX_DEMAND_DELIVERD_TIME,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UTC,                              // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                           // Variable access control - found in zcl.h
      (void *)&espMaxDemandDeliverdTime              // Pointer to attribute variable
    }
  },
    {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,               // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_CURRENT_MAX_DEMAND_RECEIVED_TIME,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UTC,                              // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                           // Variable access control - found in zcl.h
      (void *)&espMaxDemandReceivedTime              // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,       // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_STATUS,                      // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_BITMAP8,                  // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                   // Variable access control - found in zcl.h
      (void *)&espStatus                     // Pointer to attribute variable
    }
  },

  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,       // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_UNIT_OF_MEASURE,             // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_ENUM8,                    // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                   // Variable access control - found in zcl.h
      (void *)&espUnitOfMeasure              // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,       // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_MULTIPLIER,                  // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT24,                   // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                   // Variable access control - found in zcl.h
      (void *)&espMultiplier                 // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,      // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_DIVISOR,                    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT24,                  // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                  // Variable access control - found in zcl.h
      (void *)&espDivisor                   // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,      // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_SUMMATION_FORMATTING,       // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_BITMAP8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                  // Variable access control - found in zcl.h
      (void *)&espSummationFormating        // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,       // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_DEMAND_FORMATTING,           // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_BITMAP8,                  // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                   // Variable access control - found in zcl.h
      (void *)&espDemandFormatting           // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,                // Cluster IDs - defined in the profile (ie. se.h)
    {  // Attribute record
      ATTRID_SE_HISTORICAL_CONSUMPTION_FORMATTING,    // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_BITMAP8,                           // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                            // Variable access control - found in zcl.h
      (void *)&espHistoricalConsumptionFormatting     // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_INSTANTANEOUS_DEMAND,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&espInstanteneousDemand
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENTDAY_CONSUMPTION_DELIVERED,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&espCurrentdayConsumptionDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENTDAY_CONSUMPTION_RECEIVED,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&espCurrentdayConsumptionReceived
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_PREVIOUSDAY_CONSUMPTION_DELIVERED,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&espPreviousdayConsumptionDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_PREVIOUSDAY_CONSUMPTION_RECEIVED,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&espPreviousdayConsumtpionReceived
    }
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_PARTIAL_PROFILE_INTERVAL_START_TIME,
      ZCL_DATATYPE_UTC,
      ACCESS_CONTROL_READ,
      (void *)&espCurrentPartialProfileIntervalStartTime
    }
  },

  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_CURRENT_PARTIAL_PROFILE_INTERVAL_VALUE,
      ZCL_DATATYPE_UINT24,
      ACCESS_CONTROL_READ,
      (void *)&espCurrentPartialProfileIntervalValue
    }
  },

  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    {  // Attribute record
      ATTRID_SE_MAX_NUMBER_OF_PERIODS_DELIVERED,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&espMaxNumberOfPeriodsDelivered
    }
  },
  {
    ZCL_CLUSTER_ID_SE_LOAD_CONTROL,
    {  // Attribute record
      ATTRID_SE_UTILITY_DEFINED_GROUP,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&espUtilityDefinedGroup
    }
  },
  {
    ZCL_CLUSTER_ID_SE_LOAD_CONTROL,
    {  // Attribute record
      ATTRID_SE_START_RANDOMIZE_MINUTES,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&espStartRandomizeMinutes
    }
  },
  {
    ZCL_CLUSTER_ID_SE_LOAD_CONTROL,
    {  // Attribute record
      ATTRID_SE_STOP_RANDOMIZE_MINUTES,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&espStopRandomizeMinutes
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_KEY_ESTABLISHMENT,
    {  // Attribute record
      ATTRID_KEY_ESTABLISH_SUITE,
      ZCL_DATATYPE_BITMAP16,
      ACCESS_CONTROL_READ,
      (void *)&espKeyEstablishmentSuite
    }
  },
};

/*********************************************************************
 * CLUSTER OPTION DEFINITIONS
 */
zclOptionRec_t espOptions[ESP_MAX_OPTIONS] =
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
    ZCL_CLUSTER_ID_SE_LOAD_CONTROL,
    ( AF_EN_SECURITY ),
  },
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    ( AF_EN_SECURITY ),
  },
  {
    ZCL_CLUSTER_ID_SE_MESSAGE,
    ( AF_EN_SECURITY ),
  },
  {
    ZCL_CLUSTER_ID_SE_SE_TUNNELING,
    ( AF_EN_SECURITY ),
  },
  {
    ZCL_CLUSTER_ID_SE_PRE_PAYMENT,
    ( AF_EN_SECURITY ),
  },
};

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
#define ESP_MAX_INCLUSTERS       6
const cId_t espInClusterList[ESP_MAX_INCLUSTERS] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_SE_PRICING,
  ZCL_CLUSTER_ID_SE_LOAD_CONTROL,
  ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
  ZCL_CLUSTER_ID_SE_MESSAGE
};

#define ESP_MAX_OUTCLUSTERS       6
const cId_t espOutClusterList[ESP_MAX_OUTCLUSTERS] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_SE_PRICING,
  ZCL_CLUSTER_ID_SE_LOAD_CONTROL,
  ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
  ZCL_CLUSTER_ID_SE_MESSAGE
};

SimpleDescriptionFormat_t espSimpleDesc =
{
  ESP_ENDPOINT,                   //  uint8 Endpoint;
  ZCL_SE_PROFILE_ID,              //  uint16 AppProfId[2];
  ZCL_SE_DEVICEID_ESP,            //  uint16 AppDeviceId[2];
  ESP_DEVICE_VERSION,             //  int   AppDevVer:4;
  ESP_FLAGS,                      //  int   AppFlags:4;
  ESP_MAX_INCLUSTERS,             //  uint8  AppNumInClusters;
  (cId_t *)espInClusterList,      //  cId_t *pAppInClusterList;
  ESP_MAX_OUTCLUSTERS,            //  uint8  AppNumInClusters;
  (cId_t *)espOutClusterList      //  cId_t *pAppInClusterList;
};

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/****************************************************************************
****************************************************************************/


