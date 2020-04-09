/**************************************************************************************************
  Filename:       esp.c
  Revised:        $Date: 2007-08-02 09:20:10 -0700 (Thu,02 Aug 2007) $
  Revision:       $Revision: 15001 $

  Description:    This module implements the ESP functionality and contains the
                  init and event loop functions


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
  This application is designed for the test purpose of the SE profile which
  exploits the following clusters for an ESP configuration:

  General Basic
  General Alarms
  General Time
  General Key Establishment
  SE     Price
  SE     Demand Response and Load Control
  SE     Simple Metering
  SE     Message

  Key control:
    SW1:  Send out Cooling Load Control Event to PCT
    SW2:  Send out Load Control Event to Load Control Device
    SW3:  Send out Message to In Premise Display
    SW4:  Send out Service Discovery messages to get 16-bit network
          address of PCT and Load Control Device
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "OSAL.h"
#include "OSAL_Clock.h"
#include "MT.h"
#include "MT_APP.h"
#include "ZDObject.h"
#include "AddrMgr.h"

#include "se.h"
#include "esp.h"
#include "zcl_general.h"
#include "zcl_se.h"
#include "zcl_key_establish.h"

#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"


/*********************************************************************
 * MACROS
 */

// There is no attribute in the Mandatory Reportable Attribute list for now
#define zcl_MandatoryReportableAttribute( a ) ( a == NULL )

/*********************************************************************
 * CONSTANTS
 */

#define ESP_MIN_REPORTING_INTERVAL       5

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8 espTaskID;                              // esp osal task id
static afAddrType_t ipdAddr;                         // destination address of in premise display
static afAddrType_t pctAddr;                         // destination address of PCT
static afAddrType_t loadControlAddr;                 // destination address of load control device
static zAddrType_t simpleDescReqAddr[2];             // destination addresses for simple desc request
static uint8 matchRspCount = 0;                      // number of received match responses
static zAddrType_t dstMatchAddr;                     // generic destination address used for match descriptor request
static zclCCLoadControlEvent_t loadControlCmd;       // command structure for load control command

// cluster list used for match desc req
static const cId_t matchClusterList[1] =
{
  ZCL_CLUSTER_ID_SE_LOAD_CONTROL
};
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void esp_HandleKeys( uint8 shift, uint8 keys );
static void esp_ProcessAppMsg( uint8 *msg );

#if defined (ZCL_KEY_ESTABLISH)
static uint8 esp_KeyEstablish_ReturnLinkKey( uint16 shortAddr );
#endif // ZCL_KEY_ESTABLISH

#if defined (ZCL_ALARMS)
static void esp_ProcessAlarmCmd( uint8 srcEP, afAddrType_t *dstAddr,
                        uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data );
#endif // ZCL_ALARMS

static void esp_ProcessIdentifyTimeChange( void );

/*************************************************************************/
/*** Application Callback Functions                                    ***/
/*************************************************************************/

// Foundation Callback functions
static uint8 esp_ValidateAttrDataCB( zclAttrRec_t *pAttr, zclWriteRec_t *pAttrInfo );

// General Cluster Callback functions
static void esp_BasicResetCB( void );
static void esp_IdentifyCB( zclIdentify_t *pCmd );
static void esp_IdentifyQueryRspCB( zclIdentifyQueryRsp_t *pRsp );
static void esp_AlarmCB( zclAlarm_t *pAlarm );

// SE Callback functions
static void esp_GetProfileCmdCB( zclCCGetProfileCmd_t *pCmd,
                                       afAddrType_t *srcAddr, uint8 seqNum );
static void esp_GetProfileRspCB( zclCCGetProfileRsp_t *pCmd,
                                       afAddrType_t *srcAddr, uint8 seqNum );
static void esp_ReqMirrorCmdCB( afAddrType_t *srcAddr, uint8 seqNum );
static void esp_ReqMirrorRspCB( zclCCReqMirrorRsp_t *pCmd,
                                       afAddrType_t *srcAddr, uint8 seqNum );
static void esp_MirrorRemCmdCB( afAddrType_t *srcAddr, uint8 seqNum );
static void esp_MirrorRemRspCB( zclCCMirrorRemRsp_t *pCmd,
                                       afAddrType_t *srcAddr, uint8 seqNum );
static void esp_GetCurrentPriceCB( zclCCGetCurrentPrice_t *pCmd,
                                         afAddrType_t *srcAddr, uint8 seqNum );
static void esp_GetScheduledPriceCB( zclCCGetScheduledPrice_t *pCmd,
                                         afAddrType_t *srcAddr, uint8 seqNum );
static void esp_PublishPriceCB( zclCCPublishPrice_t *pCmd,
                                      afAddrType_t *srcAddr, uint8 seqNum );
static void esp_DisplayMessageCB( zclCCDisplayMessage_t *pCmd,
                                        afAddrType_t *srcAddr, uint8 seqNum );
static void esp_CancelMessageCB( zclCCCancelMessage_t *pCmd,
                                       afAddrType_t *srcAddr, uint8 seqNum );
static void esp_GetLastMessageCB( afAddrType_t *srcAddr, uint8 seqNum );
static void esp_MessageConfirmationCB( zclCCMessageConfirmation_t *pCmd,
                                             afAddrType_t *srcAddr, uint8 seqNum );
static void esp_LoadControlEventCB( zclCCLoadControlEvent_t *pCmd,
                                          afAddrType_t *srcAddr, uint8 status, uint8 seqNum);
static void esp_CancelLoadControlEventCB( zclCCCancelLoadControlEvent_t *pCmd,
                                                afAddrType_t *srcAddr, uint8 seqNum );
static void esp_CancelAllLoadControlEventsCB( zclCCCancelAllLoadControlEvents_t *pCmd,
                                                    afAddrType_t *srcAddr, uint8 seqNum);
static void esp_ReportEventStatusCB( zclCCReportEventStatus_t *pCmd,
                                           afAddrType_t *srcAddr, uint8 seqNum );
static void esp_GetScheduledEventCB( zclCCGetScheduledEvent_t *pCmd,
                                           afAddrType_t *srcAddr, uint8 seqNum);

/************************************************************************/
/***               Functions to process ZCL Foundation                ***/
/***               incoming Command/Response messages                 ***/
/************************************************************************/
static void esp_ProcessZCLMsg( zclIncomingMsg_t *msg );
#if defined ( ZCL_READ )
static uint8 esp_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif // ZCL_READ
#if defined ( ZCL_WRITE )
static uint8 esp_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif // ZCL_WRITE
#if defined ( ZCL_REPORT )
static uint8 esp_ProcessInConfigReportCmd( zclIncomingMsg_t *pInMsg );
static uint8 esp_ProcessInConfigReportRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 esp_ProcessInReadReportCfgCmd( zclIncomingMsg_t *pInMsg );
static uint8 esp_ProcessInReadReportCfgRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 esp_ProcessInReportCmd( zclIncomingMsg_t *pInMsg );
#endif // ZCL_REPORT
static uint8 esp_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
#if defined ( ZCL_DISCOVER )
static uint8 esp_ProcessInDiscRspCmd( zclIncomingMsg_t *pInMsg );
#endif // ZCL_DISCOVER

// Functions to handle ZDO messages
static void esp_ProcessZDOMsg( zdoIncomingMsg_t *inMsg );
/*********************************************************************
 * ZCL General Clusters Callback table
 */
static zclGeneral_AppCallbacks_t esp_GenCmdCallbacks =
{
  esp_BasicResetCB,              // Basic Cluster Reset command
  esp_IdentifyCB,                // Identify command
  esp_IdentifyQueryRspCB,        // Identify Query Response command
  NULL,                          // On/Off cluster commands
  NULL,                          // Level Control Move to Level command
  NULL,                          // Level Control Move command
  NULL,                          // Level Control Step command
  NULL,                          // Level Control Stop command
  NULL,                          // Group Response commands
  NULL,                          // Scene Store Request command
  NULL,                          // Scene Recall Request command
  NULL,                          // Scene Response command
  esp_AlarmCB,                   // Alarm (Response) command
  NULL,                          // RSSI Location command
  NULL,                          // RSSI Location Response command
};

/*********************************************************************
 * ZCL SE Clusters Callback table
 */
static zclSE_AppCallbacks_t esp_SECmdCallbacks =			
{
  esp_GetProfileCmdCB,                     // Get Profile Command
  esp_GetProfileRspCB,                     // Get Profile Response
  esp_ReqMirrorCmdCB,                      // Request Mirror Command
  esp_ReqMirrorRspCB,                      // Request Mirror Response
  esp_MirrorRemCmdCB,                      // Mirror Remove Command
  esp_MirrorRemRspCB,                      // Mirror Remove Response
  esp_GetCurrentPriceCB,                   // Get Current Price
  esp_GetScheduledPriceCB,                 // Get Scheduled Price
  esp_PublishPriceCB,                      // Publish Price
  esp_DisplayMessageCB,                    // Display Message Command
  esp_CancelMessageCB,                     // Cancel Message Command
  esp_GetLastMessageCB,                    // Get Last Message Command
  esp_MessageConfirmationCB,               // Message Confirmation
  esp_LoadControlEventCB,                  // Load Control Event
  esp_CancelLoadControlEventCB,            // Cancel Load Control Event
  esp_CancelAllLoadControlEventsCB,        // Cancel All Load Control Events
  esp_ReportEventStatusCB,                 // Report Event Status
  esp_GetScheduledEventCB,                 // Get Scheduled Event
};

/*********************************************************************
 * @fn          esp_Init
 *
 * @brief       Initialization function for the ZCL App Application.
 *
 * @param       uint8 task_id - esp task id
 *
 * @return      none
 */
void esp_Init( uint8 task_id )
{
  espTaskID = task_id;

  // Register for an SE endpoint
  zclSE_Init( &espSimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( ESP_ENDPOINT, &esp_GenCmdCallbacks );

  // Register the ZCL SE Cluster Library callback functions
  zclSE_RegisterCmdCallbacks( ESP_ENDPOINT, &esp_SECmdCallbacks );

  // Register the application's attribute list
  zcl_registerAttrList( ESP_ENDPOINT, ESP_MAX_ATTRIBUTES, espAttrs );

  // Register the application's cluster option list
  zcl_registerClusterOptionList( ESP_ENDPOINT, ESP_MAX_OPTIONS, espOptions );

  // Register the application's attribute data validation callback function
  zcl_registerValidateAttrData( esp_ValidateAttrDataCB );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( espTaskID );

  // register for match descriptor and simple descriptor responses
  ZDO_RegisterForZDOMsg( espTaskID, Match_Desc_rsp );
  ZDO_RegisterForZDOMsg( espTaskID, Simple_Desc_rsp );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( espTaskID );

  // Start the timer to sync esp timer with the osal timer
  osal_start_timerEx( espTaskID, ESP_UPDATE_TIME_EVT, ESP_UPDATE_TIME_PERIOD );

  // setup address mode and destination endpoint fields for PCT
  pctAddr.addrMode = (afAddrMode_t)Addr16Bit;
  pctAddr.endPoint = ESP_ENDPOINT;

  // setup address mode and destination endpoint fields for load control device
  loadControlAddr.addrMode = (afAddrMode_t)Addr16Bit;
  loadControlAddr.endPoint = ESP_ENDPOINT;

  //setup load control command structure
  loadControlCmd.issuerEvent = 0x12345678;            // arbitrary id
  loadControlCmd.deviceGroupClass = 0x000000;         // addresses all groups
  loadControlCmd.startTime = 0x00000000;              // start time = NOW
  loadControlCmd.durationInMinutes = 0x0001;          // duration of one minute
  loadControlCmd.criticalityLevel = 0x01;             // green level
  loadControlCmd.coolingTemperatureSetPoint = 0x076C; // 19 degrees C, 66.2 degress fahrenheit
  loadControlCmd.eventControl = 0x00;                 // no randomized start or end applied
}

/*********************************************************************
 * @fn          esp_event_loop
 *
 * @brief       Event Loop Processor for esp.
 *
 * @param       uint8 task_id - esp task id
 * @param       uint16 events - event bitmask
 *
 * @return      none
 */
uint16 esp_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( espTaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case MT_SYS_APP_MSG:
          // Message received from MT (serial port)
          esp_ProcessAppMsg( ((mtSysAppMsg_t *)MSGpkt)->appData );
          break;

        case ZCL_INCOMING_MSG:
          // Incoming ZCL foundation command/response messages
          esp_ProcessZCLMsg( (zclIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          esp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case ZDO_CB_MSG:
          // ZDO sends the message that we registered for
          esp_ProcessZDOMsg( (zdoIncomingMsg_t *)MSGpkt );
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // handle processing of identify timeout event triggered by an identify command
  if ( events & ESP_IDENTIFY_TIMEOUT_EVT )
  {
    if ( espIdentifyTime > 0 )
    {
      espIdentifyTime--;
    }
    esp_ProcessIdentifyTimeChange();

    return ( events ^ ESP_IDENTIFY_TIMEOUT_EVT );
  }

  // event to get current time
  if ( events & ESP_UPDATE_TIME_EVT )
  {
    espTime = osal_getClock();
    osal_start_timerEx( espTaskID, ESP_UPDATE_TIME_EVT, ESP_UPDATE_TIME_PERIOD );

    return ( events ^ ESP_UPDATE_TIME_EVT );
  }


  // event to get simple descriptor of the first match response
  if ( events & SIMPLE_DESC_QUERY_EVT_1 )
  {
      ZDP_SimpleDescReq( &simpleDescReqAddr[0], simpleDescReqAddr[0].addr.shortAddr,
                        ESP_ENDPOINT, 0);

      return ( events ^ SIMPLE_DESC_QUERY_EVT_1 );
  }

  // event to get simple descriptor of the second match response
  if ( events & SIMPLE_DESC_QUERY_EVT_2 )
  {

      ZDP_SimpleDescReq( &simpleDescReqAddr[1], simpleDescReqAddr[1].addr.shortAddr,
                        ESP_ENDPOINT, 0);

      return ( events ^ SIMPLE_DESC_QUERY_EVT_2 );
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      esp_ProcessAppMsg
 *
 * @brief   Process MT SYS APP MSG to retrieve link key
 *
 * @param   msg - pointer to message
 *          0 - lo byte destination address
 *          1 - hi byte destination address
 *
 * @return  none
 */
static void esp_ProcessAppMsg( uint8 *msg )
{
  afAddrType_t dstAddr;

  dstAddr.addr.shortAddr = BUILD_UINT16( msg[0], msg[1] );
  esp_KeyEstablish_ReturnLinkKey( dstAddr.addr.shortAddr );
}

/*********************************************************************
 * @fn      esp_ProcessIdentifyTimeChange
 *
 * @brief   Called to blink led for specified IdentifyTime attribute value
 *
 * @param   none
 *
 * @return  none
 */
static void esp_ProcessIdentifyTimeChange( void )
{
  if ( espIdentifyTime > 0 )
  {
    osal_start_timerEx( espTaskID, ESP_IDENTIFY_TIMEOUT_EVT, 1000 );
    HalLedBlink ( HAL_LED_4, 0xFF, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME );
  }
  else
  {
    HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
    osal_stop_timerEx( espTaskID, ESP_IDENTIFY_TIMEOUT_EVT );
  }
}


#if defined (ZCL_KEY_ESTABLISH)
/*********************************************************************
 * @fn      esp_KeyEstablish_ReturnLinkKey
 *
 * @brief   This function get the requested link key
 *
 * @param   shortAddr - short address of the partner.
 *
 * @return  none
 */
static uint8 esp_KeyEstablish_ReturnLinkKey( uint16 shortAddr )
{
  APSME_LinkKeyData_t* keyData;
  uint8 status = ZFailure;
  uint8 buf[1+SEC_KEY_LEN];
  uint8 len = 1;
  AddrMgrEntry_t entry;

  // Look up the long address of the device

  entry.user = ADDRMGR_USER_DEFAULT;
  entry.nwkAddr = shortAddr;

  if ( AddrMgrEntryLookupNwk( &entry ) )
  {
    // check for APS link key data
    APSME_LinkKeyDataGet( entry.extAddr, &keyData );

    if ( (keyData != NULL) && (keyData->key != NULL) )
    {
      status = ZSuccess;
      len += SEC_KEY_LEN; // status + key
    }
  }
  else
  {
    // It's an unknown device
    status = ZInvalidParameter;
  }

  buf[0] = status;

  if( status == ZSuccess )
  {
     osal_memcpy( &(buf[1]), keyData->key, SEC_KEY_LEN );
  }

  MT_BuildAndSendZToolResponse( ((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_DBG), MT_DEBUG_MSG,
                                  len, buf );
  return status;
}
#endif // ZCL_KEY_ESTABLISH
/*********************************************************************
 * @fn      esp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_3
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void esp_HandleKeys( uint8 shift, uint8 keys )
{
  // Shift is used to make each button/switch dual purpose.
  if ( shift )
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }
    if ( keys & HAL_KEY_SW_2 )
    {
    }
    if ( keys & HAL_KEY_SW_3 )
    {
    }
    if ( keys & HAL_KEY_SW_4 )
    {
    }
  }
  else
  {
    if ( keys & HAL_KEY_SW_1 )
    {
      // send out cooling event to PCT
      loadControlCmd.deviceGroupClass = HVAC_DEVICE_CLASS; // HVAC compressor or furnace - bit 0 is set
      zclSE_LoadControl_Send_LoadControlEvent( ESP_ENDPOINT, &pctAddr, &loadControlCmd, TRUE, 0 );
    }

    if ( keys & HAL_KEY_SW_2 )
    {
      // send out load control event to load control device
      loadControlCmd.deviceGroupClass = ONOFF_LOAD_DEVICE_CLASS; // simple misc residential on/off loads - bit 7 is set
      zclSE_LoadControl_Send_LoadControlEvent( ESP_ENDPOINT, &loadControlAddr, &loadControlCmd, TRUE, 0 );
    }

    if ( keys & HAL_KEY_SW_3 )
    {
      zclCCDisplayMessage_t displayCmd;             // command structure for message being sent to in premise display
      uint8 msgString[]="Rcvd MESSAGE Cmd";         // message being displayed on in premise display

      // send out display message to in premise display
      displayCmd.msgString.strLen = sizeof(msgString);
      displayCmd.msgString.pStr = msgString;

      zclSE_Message_Send_DisplayMessage( ESP_ENDPOINT, &ipdAddr, &displayCmd, TRUE, 0 );
    }

    if ( keys & HAL_KEY_SW_4 )
    {
      // perform service discovery for PCT, and load control device

       HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
      // Initiate a Match Description Request (Service Discovery)
      dstMatchAddr.addrMode = AddrBroadcast;
      dstMatchAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
      ZDP_MatchDescReq( &dstMatchAddr, NWK_BROADCAST_SHORTADDR,
                        ZCL_SE_PROFILE_ID,
                        1, (cId_t *)matchClusterList,
                        1, (cId_t *)matchClusterList,
                        FALSE );
    }

  }
}

/*********************************************************************
 * @fn      esp_ValidateAttrDataCB
 *
 * @brief   Check to see if the supplied value for the attribute data
 *          is within the specified range of the attribute.
 *
 *
 * @param   pAttr - pointer to attribute
 * @param   pAttrInfo - pointer to attribute info
 *
 * @return  TRUE if data valid. FALSE otherwise.
 */
static uint8 esp_ValidateAttrDataCB( zclAttrRec_t *pAttr, zclWriteRec_t *pAttrInfo )
{
  uint8 valid = TRUE;

  switch ( pAttrInfo->dataType )
  {
    case ZCL_DATATYPE_BOOLEAN:
      if ( ( *(pAttrInfo->attrData) != 0 ) && ( *(pAttrInfo->attrData) != 1 ) )
      {
        valid = FALSE;
      }
      break;

    default:
      break;
  }

  return ( valid );
}

/*********************************************************************
 * @fn      esp_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library to set all
 *          the attributes of all the clusters to their factory defaults
 *
 * @param   none
 *
 * @return  none
 */
static void esp_BasicResetCB( void )
{
  // user should handle setting attributes to factory defaults here
}

/*********************************************************************
 * @fn      esp_IdentifyCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identify Command for this application.
 *
 * @param   pCmd - pointer to structure for identify command
 *
 * @return  none
 */
static void esp_IdentifyCB( zclIdentify_t *pCmd )
{
  espIdentifyTime = pCmd->identifyTime;
  esp_ProcessIdentifyTimeChange();
}

/*********************************************************************
 * @fn      esp_IdentifyQueryRspCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Query Response Command for this application.
 *
 * @param   pRsp - pointer to structure for identify query response
 *
 * @return  none
 */
static void esp_IdentifyQueryRspCB( zclIdentifyQueryRsp_t *pRsp )
{
  // add user code here
}

/*********************************************************************
 * @fn      esp_AlarmCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Alam request or response command for
 *          this application.
 *
 * @param   pAlarm - pointer to structure for alarm command
 *
 * @return  none
 */
static void esp_AlarmCB( zclAlarm_t *pAlarm )
{
  // add user code here
}

/*********************************************************************
 * @fn      esp_GetProfileCmdCB
 *
 * @brief   Callback from the ZCL SE Profile Simple Metering Cluster Library when
 *          it received a Get Profile Command for
 *          this application.
 *
 * @param   pCmd - pointer to get profile command structure
 * @param   srcAddr - pointer to source address
 * @param   seqNum - sequence number of this command
 *
 * @return  none
 */
static void esp_GetProfileCmdCB( zclCCGetProfileCmd_t *pCmd, afAddrType_t *srcAddr, uint8 seqNum )
{
#if defined ( ZCL_SIMPLE_METERING )
  // Upon receipt of the Get Profile Command, the metering device shall send
  // Get Profile Response back.

  // Variables in the following are initialized to arbitrary value for test purpose
  // In real application, user shall look up the interval data captured during
  // the period specified in the pCmd->endTime and return corresponding data.

  uint32 endTime;
  uint8  status = zclSE_SimpleMeter_GetProfileRsp_Status_Success;
  uint8  profileIntervalPeriod = PROFILE_INTERVAL_PERIOD_60MIN;
  uint8  numberOfPeriodDelivered = 5;
  uint24 intervals[] = {0xa00001, 0xa00002, 0xa00003, 0xa00004, 0xa00005};

  // endTime: 32 bit value (in UTC) representing the end time of the most
  // chronologically recent interval being requested.
  // Example: Data collected from 2:00 PM to 3:00 PM would be specified as a
  // 3:00 PM interval (end time).

  // The Intervals block returned shall be the most recent block with
  // its EndTime equal or older to the one in the request (pCmd->endTime).
  // Requested End Time with value 0xFFFFFFFF indicats the most recent
  // Intervals block is requested.

  // Sample Code - assuming the end time of the requested block is the same as
  // it in the request.
  endTime = pCmd->endTime;

  // Send Get Profile Response Command back

  zclSE_SimpleMetering_Send_GetProfileRsp( ESP_ENDPOINT, srcAddr, endTime,
                                           status,
                                           profileIntervalPeriod,
                                           numberOfPeriodDelivered, intervals,
                                           false, seqNum );
#endif // ZCL_SIMPLE_METERING
}

/*********************************************************************
 * @fn      esp_GetProfileRspCB
 *
 * @brief   Callback from the ZCL SE Profile Simple Metering Cluster Library when
 *          it received a Get Profile Response for
 *          this application.
 *
 * @param   pCmd - pointer to get profile response structure
 * @param   srcAddr - pointer to source address
 * @param   seqNum - sequence number of this command
 *
 * @return  none
 */
static void esp_GetProfileRspCB( zclCCGetProfileRsp_t *pCmd, afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}

/*********************************************************************
 * @fn      esp_ReqMirrorCmdCB
 *
 * @brief   Callback from the ZCL SE Profile Simple Metering Cluster Library when
 *          it received a Request Mirror Command for
 *          this application.
 *
 * @param   srcAddr - pointer to source address
 * @param   seqNum - sequence number of this command
 *
 * @return  none
 */
static void esp_ReqMirrorCmdCB( afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}
/*********************************************************************
 * @fn      esp_ReqMirrorRspCB
 *
 * @brief   Callback from the ZCL SE Profile Simple Metering Cluster Library when
 *          it received a Request Mirror Response for
 *          this application.
 *
 * @param   pRsp - pointer to request mirror response structure
 * @param   srcAddr - pointer to source address
 * @param   seqNum - sequence number of this command
 *
 * @return  none
 */
static void esp_ReqMirrorRspCB( zclCCReqMirrorRsp_t *pRsp, afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}
/*********************************************************************
 * @fn      esp_MirrorRemCmdCB
 *
 * @brief   Callback from the ZCL SE Profile Simple Metering Cluster Library when
 *          it received a Mirror Remove Command for
 *          this application.
 *
 * @param   srcAddr - pointer to source address
 * @param   seqNum - sequence number of this command
 *
 * @return  none
 */
static void esp_MirrorRemCmdCB( afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}
/*********************************************************************
 * @fn      esp_MirrorRemRspCB
 *
 * @brief   Callback from the ZCL SE Profile Simple Metering Cluster Library when
 *          it received a Mirror Remove Response for
 *          this application.
 *
 * @param   pCmd - pointer to mirror remove response structure
 * @param   srcAddr - pointer to source address
 * @param   seqNum - sequence number of this command
 *
 * @return  none
 */
static void esp_MirrorRemRspCB( zclCCMirrorRemRsp_t *pCmd, afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}
/*********************************************************************
 * @fn      esp_GetCurrentPriceCB
 *
 * @brief   Callback from the ZCL SE Profile Pricing Cluster Library when
 *          it received a Get Current Price for
 *          this application.
 *
 * @param   pCmd - pointer to structure for Get Current Price command
 * @param   srcAddr - source address
 * @param   seqNum - sequence number for this command
 *
 * @return  none
 */
static void esp_GetCurrentPriceCB( zclCCGetCurrentPrice_t *pCmd,
                                         afAddrType_t *srcAddr, uint8 seqNum )
{
#if defined ( ZCL_PRICING )
  // On receipt of Get Current Price command, the device shall send a
  // Publish Price command with the information for the current time.
  zclCCPublishPrice_t cmd;

  osal_memset( &cmd, 0, sizeof( zclCCPublishPrice_t ) );

  cmd.providerId = 0xbabeface;
  cmd.priceTier = 0xfe;

  // copy source address of display device that requested current pricing info so
  // that esp can send messages to it using destination address of IPDAddr

  osal_memcpy( &ipdAddr, srcAddr, sizeof ( afAddrType_t ) );

  zclSE_Pricing_Send_PublishPrice( ESP_ENDPOINT, srcAddr, &cmd, false, seqNum );
#endif // ZCL_PRICING
}

/*********************************************************************
 * @fn      esp_GetScheduledPriceCB
 *
 * @brief   Callback from the ZCL SE Profile Pricing Cluster Library when
 *          it received a Get Scheduled Price for
 *          this application.
 *
 * @param   pCmd - pointer to structure for Get Scheduled Price command
 * @param   srcAddr - source address
 * @param   seqNum - sequence number for this command
 *
 * @return  none
 */
static void esp_GetScheduledPriceCB( zclCCGetScheduledPrice_t *pCmd, afAddrType_t *srcAddr, uint8 seqNum  )
{
  // On receipt of Get Scheduled Price command, the device shall send a
  // Publish Price command for all currently scheduled price events.
  // The sample code as follows only sends one.

#if defined ( ZCL_PRICING )
  zclCCPublishPrice_t cmd;

  osal_memset( &cmd, 0, sizeof( zclCCPublishPrice_t ) );

  cmd.providerId = 0xbabeface;
  cmd.priceTier = 0xfe;

  zclSE_Pricing_Send_PublishPrice( ESP_ENDPOINT, srcAddr, &cmd, false, seqNum );

#endif // ZCL_PRICING
}

/*********************************************************************
 * @fn      esp_PublishPriceCB
 *
 * @brief   Callback from the ZCL SE Profile Pricing Cluster Library when
 *          it received a Publish Price for this application.
 *
 * @param   pCmd - pointer to structure for Publish Price command
 * @param   srcAddr - source address
 * @param   seqNum - sequence number for this command
 *
 * @return  none
 */
static void esp_PublishPriceCB( zclCCPublishPrice_t *pCmd,
                                      afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}

/*********************************************************************
 * @fn      esp_DisplayMessageCB
 *
 * @brief   Callback from the ZCL SE Profile Message Cluster Library when
 *          it received a Display Message Command for
 *          this application.
 *
 * @param   pCmd - pointer to structure for Display Message command
 * @param   srcAddr - source address
 * @param   seqNum - sequence number for this command
 *
 * @return  none
 */
static void esp_DisplayMessageCB( zclCCDisplayMessage_t *pCmd,
                                        afAddrType_t *srcAddr, uint8 seqNum )
{
  // Upon receipt of the Display Message Command, the device shall
  // display the message. If the Message Confirmation bit indicates
  // the message originator require a confirmation of receipt from
  // a Utility Customer, the device should display the message or
  // alert the user until it is either confirmed via a button or by
  // selecting a confirmation option on the device.  Confirmation is
  // typically used when the Utility is sending down information
  // such as a disconnection notice, or prepaid billing information.
  // Message duration is ignored when confirmation is requested and
  // the message is displayed until confirmed.

#if defined ( LCD_SUPPORTED )
    HalLcdWriteString( (char*)pCmd->msgString.pStr, HAL_LCD_LINE_1 );
#endif // LCD_SUPPORTED
}

/*********************************************************************
 * @fn      esp_CancelMessageCB
 *
 * @brief   Callback from the ZCL SE Profile Message Cluster Library when
 *          it received a Cancel Message Command for
 *          this application.
 *
 * @param   pCmd - pointer to structure for Cancel Message command
 * @param   srcAddr - source address
 * @param   seqNum - sequence number for this command
 *
 * @return  none
 */
static void esp_CancelMessageCB( zclCCCancelMessage_t *pCmd,
                                        afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}

/*********************************************************************
 * @fn      esp_GetLastMessageCB
 *
 * @brief   Callback from the ZCL SE Profile Message Cluster Library when
 *          it received a Get Last Message Command for
 *          this application.
 *
 * @param   pCmd - pointer to structure for Get Last Message command
 * @param   srcAddr - source address
 * @param   seqNum - sequence number for this command
 *
 * @return  none
 */
static void esp_GetLastMessageCB( afAddrType_t *srcAddr, uint8 seqNum )
{
  // On receipt of Get Last Message command, the device shall send a
  // Display Message command back to the sender

#if defined ( ZCL_MESSAGE )
  zclCCDisplayMessage_t cmd;
  uint8 msg[10] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29 };

  // Fill in the command with information for the last message
  cmd.messageId = 0xaabbccdd;
  cmd.messageCtrl.transmissionMode = 0;
  cmd.messageCtrl.importance = 1;
  cmd.messageCtrl.confirmationRequired = 1;
  cmd.durationInMinutes = 60;

  cmd.msgString.strLen = 10;
  cmd.msgString.pStr = msg;

  zclSE_Message_Send_DisplayMessage( ESP_ENDPOINT, srcAddr, &cmd,
                                     false, seqNum );
#endif // ZCL_MESSAGe
}

/*********************************************************************
 * @fn      esp_MessageConfirmationCB
 *
 * @brief   Callback from the ZCL SE Profile Message Cluster Library when
 *          it received a Message Confirmation Command for
 *          this application.
 *
 * @param   pCmd - pointer to structure for Message Confirmation command
 * @param   srcAddr - source address
 * @param   seqNum - sequence number for this command
 *
 * @return  none
 */
static void esp_MessageConfirmationCB( zclCCMessageConfirmation_t *pCmd,
                                             afAddrType_t *srcAddr, uint8 seqNum)
{
  // add user code here
}

#if defined (ZCL_LOAD_CONTROL)
/*********************************************************************
 * @fn      esp_SendReportEventStatus
 *
 * @brief   Callback from the ZCL SE Profile Message Cluster Library when
 *          it received a Load Control Event Command for
 *          this application.
 *
 * @param   afAddrType_t *srcAddr - pointer to source address
 * @param   uint8 seqNum - sequence number for this event
 * @param   uint32 eventID - event ID for this event
 * @param   uint32 startTime - start time for this event
 * @param   uint8 eventStatus - status for this event
 * @param   uint8 criticalityLevel - criticality level for this event
 * @param   uint8 eventControl - event control for this event
 *
 * @return  none
 */
static void esp_SendReportEventStatus( afAddrType_t *srcAddr, uint8 seqNum,
                                              uint32 eventID, uint32 startTime,
                                              uint8 eventStatus, uint8 criticalityLevel,
                                              uint8 eventControl )
{
  zclCCReportEventStatus_t *pRsp;

  pRsp = (zclCCReportEventStatus_t *)osal_mem_alloc( sizeof( zclCCReportEventStatus_t ) );

  if ( pRsp != NULL)
  {
    // Mandatory fields - use the incoming data
    pRsp->issuerEventID = eventID;
    pRsp->eventStartTime = startTime;
    pRsp->criticalityLevelApplied = criticalityLevel;
    pRsp->eventControl = eventControl;
    pRsp->eventStatus = eventStatus;
    pRsp->signatureType = SE_PROFILE_SIGNATURE_TYPE_ECDSA;

    // esp_Signature is a static array.
    // value can be changed in esp_data.c
    osal_memcpy( pRsp->signature, espSignature, 16 );

    // Optional fields - fill in with non-used value by default
    pRsp->coolingTemperatureSetPointApplied = SE_OPTIONAL_FIELD_TEMPERATURE_SET_POINT;
    pRsp->heatingTemperatureSetPointApplied = SE_OPTIONAL_FIELD_TEMPERATURE_SET_POINT;
    pRsp->averageLoadAdjustment = SE_OPTIONAL_FIELD_INT8;
    pRsp->dutyCycleApplied = SE_OPTIONAL_FIELD_UINT8;

    // Send response back
    // DisableDefaultResponse is set to false - it is recommended to turn on
    // default response since Report Event Status Command does not have
    // a response.
    zclSE_LoadControl_Send_ReportEventStatus( ESP_ENDPOINT, srcAddr,
                                            pRsp, false, seqNum );
    osal_mem_free( pRsp );
  }
}
#endif // ZCL_LOAD_CONTROL

/*********************************************************************
 * @fn      esp_LoadControlEventCB
 *
 * @brief   Callback from the ZCL SE Profile Load Control Cluster Library when
 *          it received a Load Control Event Command for
 *          this application.
 *
 * @param   pCmd - pointer to load control event command
 * @param   srcAddr - source address
 * @param   status - event status
 * @param   seqNum - sequence number of this command
 *
 * @return  none
 */
static void esp_LoadControlEventCB( zclCCLoadControlEvent_t *pCmd,
                                               afAddrType_t *srcAddr, uint8 status,
                                               uint8 seqNum)
{
#if defined ( ZCL_LOAD_CONTROL )
  // According to the Smart Metering Specification, upon receipt
  // of the Load Control Event command, the receiving device shall
  // send Report Event Status command back.
  uint8 eventStatus;

  if ( status == ZCL_STATUS_INVALID_FIELD )
  {
    // If the incoming message has invalid fields in it
    // Send response back with status: rejected
    eventStatus = EVENT_STATUS_LOAD_CONTROL_EVENT_REJECTED;
  }
  else
  { // Send response back with status: received
    eventStatus = EVENT_STATUS_LOAD_CONTROL_EVENT_RECEIVED;
  }

  // Send response back
  esp_SendReportEventStatus( srcAddr, seqNum, pCmd->issuerEvent,
                                   pCmd->startTime, eventStatus,
                                   pCmd->criticalityLevel, pCmd->eventControl);

  if ( status != ZCL_STATUS_INVALID_FIELD )
  {
    // add user load control event handler here
  }
#endif // ZCL_LOAD_CONTROL
}

/*********************************************************************
 * @fn      esp_CancelLoadControlEventCB
 *
 * @brief   Callback from the ZCL SE Profile Load Control Cluster Library when
 *          it received a Cancel Load Control Event Command for
 *          this application.
 *
 * @param   pCmd - pointer to structure for Cancel Load Control Event command
 * @param   scrAddr - source address
 * @param   seqNum - sequence number for this command
 *
 * @return  none
 */
static void esp_CancelLoadControlEventCB( zclCCCancelLoadControlEvent_t *pCmd,
                                                afAddrType_t *srcAddr, uint8 seqNum )
{
#if defined ( ZCL_LOAD_CONTROL )
  if ( 0 )  // User shall replace the if condition with "if the event exist"
  {
    // If the event exist, stop the event, and respond with status: cancelled

    // Cancel the event here

    // Use the following sample code to send response back.
    /*
    esp_SendReportEventStatus( srcAddr, seqNum, pCmd->issuerEventID,
                                     // startTime
                                     EVENT_STATUS_LOAD_CONTROL_EVENT_CANCELLED, // eventStatus
                                     // Criticality level
                                     // eventControl };
    */

  }
  else
  {
    // If the event does not exist, respond with status: rejected
    // The rest of the mandatory fields are not available, therefore,
    // set to optional value
    esp_SendReportEventStatus( srcAddr, seqNum, pCmd->issuerEventID,
                                     SE_OPTIONAL_FIELD_UINT32,                  // startTime
                                     EVENT_STATUS_LOAD_CONTROL_EVENT_RECEIVED,  // eventStatus
                                     SE_OPTIONAL_FIELD_UINT8,                   // Criticality level
                                     SE_OPTIONAL_FIELD_UINT8 );                 // eventControl
  }

#endif // ZCL_LOAD_CONTROL
}

/*********************************************************************
 * @fn      esp_CancelAllLoadControlEventsCB
 *
 * @brief   Callback from the ZCL SE Profile Load Control Cluster Library when
 *          it received a Cancel All Load Control Event Command for
 *          this application.
 *
 * @param   pCmd - pointer to structure for Cancel All Load Control Event command
 * @param   scrAddr - source address
 * @param   seqNum - sequence number for this command
 *
 * @return  none
 */
static void esp_CancelAllLoadControlEventsCB( zclCCCancelAllLoadControlEvents_t *pCmd,
                                                    afAddrType_t *srcAddr, uint8 seqNum )
{
  // Upon receipt of Cancel All Load Control Event Command,
  // the receiving device shall look up the table for all events
  // and send a seperate response for each event

}

/*********************************************************************
 * @fn      esp_ReportEventStatusCB
 *
 * @brief   Callback from the ZCL SE Profile Load Control Cluster Library when
 *          it received a Report Event Status Command for
 *          this application.
 *
 * @param   pCmd - pointer to structure for Report Event Status command
 * @param   scrAddr - source address
 * @param   seqNum - sequence number for this command
 *
 * @return  none
 */
static void esp_ReportEventStatusCB( zclCCReportEventStatus_t *pCmd,
                                           afAddrType_t *srcAddr, uint8 seqNum)
{
  // add user code here
}
/*********************************************************************
 * @fn      esp_GetScheduledEventCB
 *
 * @brief   Callback from the ZCL SE Profile Load Control Cluster Library when
 *          it received a Get Scheduled Event Command for
 *          this application.
 *
 * @param   pCmd - pointer to structure for Get Scheduled Event command
 * @param   scrAddr - source address
 * @param   seqNum - sequence number for this command
 *
 * @return  none
 */
static void esp_GetScheduledEventCB( zclCCGetScheduledEvent_t *pCmd,
                                           afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}


/******************************************************************************
 *
 *  Functions for processing ZDO incoming messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      esp_ProcessZDOMsg
 *
 * @brief   Process the incoming ZDO messages.
 *
 * @param   inMsg - message to process
 *
 * @return  none
 */
static void esp_ProcessZDOMsg( zdoIncomingMsg_t *inMsg )
{
  switch ( inMsg->clusterID )
  {
     case Match_Desc_rsp:
      {
        ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( inMsg );
        if ( pRsp )
        {
          matchRspCount++; // found a match, increment the rsp count
          if ( pRsp->status == ZSuccess && pRsp->cnt )
          {
            if ( matchRspCount == 1 )
            {
              simpleDescReqAddr[0].addrMode = (afAddrMode_t)Addr16Bit;
              simpleDescReqAddr[0].addr.shortAddr = pRsp->nwkAddr;

              // Light LED
              HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );

              // set simple descriptor query event
              osal_set_event( espTaskID, SIMPLE_DESC_QUERY_EVT_1 );
            }
            else if ( matchRspCount == 2 )
            {
              matchRspCount = 0; // reset rsp counter
              simpleDescReqAddr[1].addrMode = (afAddrMode_t)Addr16Bit;
              simpleDescReqAddr[1].addr.shortAddr = pRsp->nwkAddr;

              // Light LED
              HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );

              // set simple descriptor query event
              osal_set_event( espTaskID, SIMPLE_DESC_QUERY_EVT_2 );
            }

          }
           osal_mem_free( pRsp );
        }
      }
      break;
    case Simple_Desc_rsp:
      {
        ZDO_SimpleDescRsp_t *pSimpleDescRsp;   // pointer to received simple desc response
        pSimpleDescRsp = (ZDO_SimpleDescRsp_t *)osal_mem_alloc( sizeof( ZDO_SimpleDescRsp_t ) );

        if(pSimpleDescRsp)
        {
          ZDO_ParseSimpleDescRsp( inMsg, pSimpleDescRsp );
          if( pSimpleDescRsp->simpleDesc.AppDeviceId == ZCL_SE_DEVICEID_PCT ) // this is a PCT
          {
            pctAddr.addr.shortAddr = pSimpleDescRsp->nwkAddr;
          }
          else if ( pSimpleDescRsp->simpleDesc.AppDeviceId == ZCL_SE_DEVICEID_LOAD_CTRL_EXTENSION ) // this is a load control device
          {
            loadControlAddr.addr.shortAddr = pSimpleDescRsp->nwkAddr;
          }
          osal_mem_free( pSimpleDescRsp );
        }
      }

      break;
  }
}


/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      esp_ProcessZCLMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - message to process
 *
 * @return  none
 */
static void esp_ProcessZCLMsg( zclIncomingMsg_t *pInMsg )
{
  switch ( pInMsg->zclHdr.commandID )
  {
#if defined ( ZCL_READ )
    case ZCL_CMD_READ_RSP:
      esp_ProcessInReadRspCmd( pInMsg );
      break;
#endif // ZCL_READ
#if defined ( ZCL_WRITE )
    case ZCL_CMD_WRITE_RSP:
      esp_ProcessInWriteRspCmd( pInMsg );
      break;
#endif // ZCL_WRITE
#if defined ( ZCL_REPORT )
    case ZCL_CMD_CONFIG_REPORT:
      esp_ProcessInConfigReportCmd( pInMsg );
      break;

    case ZCL_CMD_CONFIG_REPORT_RSP:
      esp_ProcessInConfigReportRspCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      esp_ProcessInReadReportCfgCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG_RSP:
      esp_ProcessInReadReportCfgRspCmd( pInMsg );
      break;

    case ZCL_CMD_REPORT:
      esp_ProcessInReportCmd( pInMsg );
      break;
#endif // ZCL_REPORT
    case ZCL_CMD_DEFAULT_RSP:
      esp_ProcessInDefaultRspCmd( pInMsg );
      break;
#if defined ( ZCL_DISCOVER )
    case ZCL_CMD_DISCOVER_RSP:
      esp_ProcessInDiscRspCmd( pInMsg );
      break;
#endif // ZCL_DISCOVER
    default:
      break;
  }

  if ( pInMsg->attrCmd != NULL )
  {
    // free the parsed command
    osal_mem_free( pInMsg->attrCmd );
    pInMsg->attrCmd = NULL;
  }
}

#if defined ( ZCL_READ )
/*********************************************************************
 * @fn      esp_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 esp_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
  uint8 i;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < readRspCmd->numAttr; i++)
  {
    // Notify the originator of the results of the original read attributes
    // attempt and, for each successfull request, the value of the requested
    // attribute
  }

  return TRUE;
}
#endif // ZCL_READ

#if defined ( ZCL_WRITE )
/*********************************************************************
 * @fn      esp_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 esp_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8 i;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < writeRspCmd->numAttr; i++)
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return TRUE;
}
#endif // ZCL_WRITE

#if defined ( ZCL_REPORT )
/*********************************************************************
 * @fn      esp_ProcessInConfigReportCmd
 *
 * @brief   Process the "Profile" Configure Reporting Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  TRUE if attribute was found in the Attribute list,
 *          FALSE if not
 */
static uint8 esp_ProcessInConfigReportCmd( zclIncomingMsg_t *pInMsg )
{
  zclCfgReportCmd_t *cfgReportCmd;
  zclCfgReportRec_t *reportRec;
  zclCfgReportRspCmd_t *cfgReportRspCmd;
  zclAttrRec_t attrRec;
  uint8 status;
  uint8 i, j = 0;

  cfgReportCmd = (zclCfgReportCmd_t *)pInMsg->attrCmd;

  // Allocate space for the response command
  cfgReportRspCmd = (zclCfgReportRspCmd_t *)osal_mem_alloc( sizeof ( zclCfgReportRspCmd_t ) +
                                        sizeof ( zclCfgReportStatus_t) * cfgReportCmd->numAttr );
  if ( cfgReportRspCmd == NULL )
    return FALSE; // EMBEDDED RETURN

  // Process each Attribute Reporting Configuration record
  for ( i = 0; i < cfgReportCmd->numAttr; i++ )
  {
    reportRec = &(cfgReportCmd->attrList[i]);

    status = ZCL_STATUS_SUCCESS;

    if ( zclFindAttrRec( ESP_ENDPOINT, pInMsg->clusterId, reportRec->attrID, &attrRec ) )
    {
      if ( reportRec->direction == ZCL_SEND_ATTR_REPORTS )
      {
        if ( reportRec->dataType == attrRec.attr.dataType )
        {
          // This the attribute that is to be reported
          if ( zcl_MandatoryReportableAttribute( &attrRec ) == TRUE )
          {
            if ( reportRec->minReportInt < ESP_MIN_REPORTING_INTERVAL ||
                 ( reportRec->maxReportInt != 0 &&
                   reportRec->maxReportInt < reportRec->minReportInt ) )
            {
              // Invalid fields
              status = ZCL_STATUS_INVALID_VALUE;
            }
            else
            {
              // Set the Min and Max Reporting Intervals and Reportable Change
              //status = zclSetAttrReportInterval( pAttr, cfgReportCmd );
              status = ZCL_STATUS_UNREPORTABLE_ATTRIBUTE; // for now
            }
          }
          else
          {
            // Attribute cannot be reported
            status = ZCL_STATUS_UNREPORTABLE_ATTRIBUTE;
          }
        }
        else
        {
          // Attribute data type is incorrect
          status = ZCL_STATUS_INVALID_DATA_TYPE;
        }
      }
      else
      {
        // We shall expect reports of values of this attribute
        if ( zcl_MandatoryReportableAttribute( &attrRec ) == TRUE )
        {
          // Set the Timeout Period
          //status = zclSetAttrTimeoutPeriod( pAttr, cfgReportCmd );
          status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE; // for now
        }
        else
        {
          // Reports of attribute cannot be received
          status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        }
      }
    }
    else
    {
      // Attribute is not supported
      status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }

    // If not successful then record the status
    if ( status != ZCL_STATUS_SUCCESS )
    {
      cfgReportRspCmd->attrList[j].status = status;
      cfgReportRspCmd->attrList[j++].attrID = reportRec->attrID;
    }
  } // for loop

  if ( j == 0 )
  {
    // Since all attributes were configured successfully, include a single
    // attribute status record in the response command with the status field
    // set to SUCCESS and the attribute ID field omitted.
    cfgReportRspCmd->attrList[0].status = ZCL_STATUS_SUCCESS;
    cfgReportRspCmd->numAttr = 1;
  }
  else
  {
    cfgReportRspCmd->numAttr = j;
  }

  // Send the response back
  zcl_SendConfigReportRspCmd( ESP_ENDPOINT, &(pInMsg->srcAddr),
                              pInMsg->clusterId, cfgReportRspCmd, ZCL_FRAME_SERVER_CLIENT_DIR,
                              true, pInMsg->zclHdr.transSeqNum );
  osal_mem_free( cfgReportRspCmd );

  return TRUE ;
}

/*********************************************************************
 * @fn      esp_ProcessInConfigReportRspCmd
 *
 * @brief   Process the "Profile" Configure Reporting Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 esp_ProcessInConfigReportRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclCfgReportRspCmd_t *cfgReportRspCmd;
  zclAttrRec_t attrRec;
  uint8 i;

  cfgReportRspCmd = (zclCfgReportRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < cfgReportRspCmd->numAttr; i++)
  {
    if ( zclFindAttrRec( ESP_ENDPOINT, pInMsg->clusterId,
                         cfgReportRspCmd->attrList[i].attrID, &attrRec ) )
    {
      // Notify the device of success (or otherwise) of the its original configure
      // reporting command, for each attribute.
    }
  }

  return TRUE;
}

/*********************************************************************
 * @fn      esp_ProcessInReadReportCfgCmd
 *
 * @brief   Process the "Profile" Read Reporting Configuration Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 esp_ProcessInReadReportCfgCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadReportCfgCmd_t *readReportCfgCmd;
  zclReadReportCfgRspCmd_t *readReportCfgRspCmd;
  zclReportCfgRspRec_t *reportRspRec;
  zclAttrRec_t attrRec;
  uint8 reportChangeLen;
  uint8 *dataPtr;
  uint8 hdrLen;
  uint8 dataLen = 0;
  uint8 status;
  uint8 i;

  readReportCfgCmd = (zclReadReportCfgCmd_t *)pInMsg->attrCmd;

  // Find out the response length (Reportable Change field is of variable length)
  for ( i = 0; i < readReportCfgCmd->numAttr; i++ )
  {
    // For supported attributes with 'analog' data type, find out the length of
    // the Reportable Change field
    if ( zclFindAttrRec( ESP_ENDPOINT, pInMsg->clusterId,
                         readReportCfgCmd->attrList[i].attrID, &attrRec ) )
    {
      if ( zclAnalogDataType( attrRec.attr.dataType ) )
      {
         reportChangeLen = zclGetDataTypeLength( attrRec.attr.dataType );

         // add padding if neede
         if ( PADDING_NEEDED( reportChangeLen ) )
           reportChangeLen++;
         dataLen += reportChangeLen;
      }
    }
  }

  hdrLen = sizeof( zclReadReportCfgRspCmd_t ) + ( readReportCfgCmd->numAttr * sizeof( zclReportCfgRspRec_t ) );

  // Allocate space for the response command
  readReportCfgRspCmd = (zclReadReportCfgRspCmd_t *)osal_mem_alloc( hdrLen + dataLen );
  if ( readReportCfgRspCmd == NULL )
    return FALSE; // EMBEDDED RETURN

  dataPtr = (uint8 *)( (uint8 *)readReportCfgRspCmd + hdrLen );
  readReportCfgRspCmd->numAttr = readReportCfgCmd->numAttr;
  for (i = 0; i < readReportCfgCmd->numAttr; i++)
  {
    reportRspRec = &(readReportCfgRspCmd->attrList[i]);

    if ( zclFindAttrRec( ESP_ENDPOINT, pInMsg->clusterId,
                         readReportCfgCmd->attrList[i].attrID, &attrRec ) )
    {
      if ( zcl_MandatoryReportableAttribute( &attrRec ) == TRUE )
      {
        // Get the Reporting Configuration
        // status = zclReadReportCfg( readReportCfgCmd->attrID[i], reportRspRec );
        status = ZCL_STATUS_UNREPORTABLE_ATTRIBUTE; // for now
        if ( status == ZCL_STATUS_SUCCESS && zclAnalogDataType( attrRec.attr.dataType ) )
        {
          reportChangeLen = zclGetDataTypeLength( attrRec.attr.dataType );
          //osal_memcpy( dataPtr, pBuf, reportChangeLen );
          reportRspRec->reportableChange = dataPtr;

          // add padding if needed
          if ( PADDING_NEEDED( reportChangeLen ) )
            reportChangeLen++;
          dataPtr += reportChangeLen;
        }
      }
      else
      {
        // Attribute not in the Mandatory Reportable Attribute list
        status = ZCL_STATUS_UNREPORTABLE_ATTRIBUTE;
      }
    }
    else
    {
      // Attribute not found
      status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }

    reportRspRec->status = status;
    reportRspRec->attrID = readReportCfgCmd->attrList[i].attrID;
  }

  // Send the response back
  zcl_SendReadReportCfgRspCmd( ESP_ENDPOINT, &(pInMsg->srcAddr),
                               pInMsg->clusterId, readReportCfgRspCmd, ZCL_FRAME_SERVER_CLIENT_DIR,
                               true, pInMsg->zclHdr.transSeqNum );
  osal_mem_free( readReportCfgRspCmd );

  return TRUE;
}

/*********************************************************************
 * @fn      esp_ProcessInReadReportCfgRspCmd
 *
 * @brief   Process the "Profile" Read Reporting Configuration Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 esp_ProcessInReadReportCfgRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadReportCfgRspCmd_t *readReportCfgRspCmd;
  zclReportCfgRspRec_t *reportRspRec;
  uint8 i;

  readReportCfgRspCmd = (zclReadReportCfgRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < readReportCfgRspCmd->numAttr; i++ )
  {
    reportRspRec = &(readReportCfgRspCmd->attrList[i]);

    // Notify the device of the results of the its original read reporting
    // configuration command.

    if ( reportRspRec->status == ZCL_STATUS_SUCCESS )
    {
      if ( reportRspRec->direction == ZCL_SEND_ATTR_REPORTS )
      {
        // add user code here
      }
      else
      {
        // expecting attribute reports
      }
    }
  }

  return TRUE;
}

/*********************************************************************
 * @fn      esp_ProcessInReportCmd
 *
 * @brief   Process the "Profile" Report Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 esp_ProcessInReportCmd( zclIncomingMsg_t *pInMsg )
{
  zclReportCmd_t *reportCmd;
  zclReport_t *reportRec;
  uint8 i;
  uint8 *meterData;
  char lcdBuf[13];

  reportCmd = (zclReportCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < reportCmd->numAttr; i++)
  {
    // Device is notified of the latest values of the attribute of another device.
    reportRec = &(reportCmd->attrList[i]);

    if ( reportRec->attrID == ATTRID_SE_CURRENT_SUMMATION_DELIVERED )
    {
      // process simple metering current summation delivered attribute
      meterData = reportRec->attrData;

      // process to convert hex to ascii
      for(i=0; i<6; i++)
      {
        if(meterData[5-i] == 0)
        {
          lcdBuf[i*2] = '0';
          lcdBuf[i*2+1] = '0';
        }
        else if(meterData[5-i] <= 0x0A)
        {
          lcdBuf[i*2] = '0';
          _ltoa(meterData[5-i],(uint8*)&lcdBuf[i*2+1],16);
        }
        else
        {
          _ltoa(meterData[5-i],(uint8*)&lcdBuf[i*2],16);
        }
      }

      // print out value of current summation delivered in hex
      HalLcdWriteString("Zigbee Coord esp", HAL_LCD_LINE_1);
      HalLcdWriteString("Curr Summ Dlvd", HAL_LCD_LINE_2);
      HalLcdWriteString(lcdBuf, HAL_LCD_LINE_3);
    }
  }
  return TRUE;
}
#endif // ZCL_REPORT

/*********************************************************************
 * @fn      esp_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 esp_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.

  return TRUE;
}

#if defined ( ZCL_DISCOVER )
/*********************************************************************
 * @fn      esp_ProcessInDiscRspCmd
 *
 * @brief   Process the "Profile" Discover Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 esp_ProcessInDiscRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverRspCmd_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return TRUE;
}
#endif // ZCL_DISCOVER

/****************************************************************************
****************************************************************************/
