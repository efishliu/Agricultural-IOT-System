/**************************************************************************************************
  Filename:       simplemeter.c
  Revised:        $Date: 2007-08-02 09:20:10 -0700 (Thu,02 Aug 2007) $
  Revision:       $Revision: 15001 $

  Description:    This module implements the Simple Meter functionality and
                  contains the init and event loop functions


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
  exploits the following clusters for a Simple Metering configuration:

  General Basic
  General Alarms
  General Time
  General Key Establishment
  SE     Simple Metering

  Key control:
    SW1:  Join Network
    SW2:  N/A
    SW3:  N/A
    SW4:  N/A

*********************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "OSAL.h"
#include "OSAL_Clock.h"
#include "ZDApp.h"
#include "AddrMgr.h"

#include "se.h"
#include "simplemeter.h"
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

#define SIMPLEMETER_MIN_REPORTING_INTERVAL       5

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

static uint8 simpleMeterTaskID;                                    // osal task id of simple meter
static uint8 simpleMeterTransID;                                   // transaction id
static devStates_t simpleMeterNwkState;                            // network state variable
static afAddrType_t ESPAddr;                                       // esp destination address
static zclReportCmd_t *pReportCmd;                                 // report command structure
static uint8 numAttr = 1;                                          // number of attributes in report
extern uint8 simpleMeterCurrentSummationDelivered[];               // defined in simplemeter_data.c
static uint8 linkKeyStatus;                                        // status return from get link key routine

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void simplemeter_HandleKeys( uint8 shift, uint8 keys );

#if defined (ZCL_KEY_ESTABLISH)
static uint8 simplemeter_KeyEstablish_ReturnLinkKey( uint16 shortAddr );
#endif // ZCL_KEY_ESTABLISH

#if defined ( ZCL_ALARMS )
static void simplemeter_ProcessAlarmCmd( uint8 srcEP, afAddrType_t *dstAddr,
                        uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data );
#endif // ZCL_ALARMS

static void simplemeter_ProcessIdentifyTimeChange( void );

/*************************************************************************/
/*** Application Callback Functions                                    ***/
/*************************************************************************/

// Foundation Callback functions
static uint8 simplemeter_ValidateAttrDataCB( zclAttrRec_t *pAttr, zclWriteRec_t *pAttrInfo );

// General Cluster Callback functions
static void simplemeter_BasicResetCB( void );
static void simplemeter_IdentifyCB( zclIdentify_t *pCmd );
static void simplemeter_IdentifyQueryRspCB( zclIdentifyQueryRsp_t *pRsp );
static void simplemeter_AlarmCB( zclAlarm_t *pAlarm );

// SE Callback functions
static void simplemeter_GetProfileCmdCB( zclCCGetProfileCmd_t *pCmd,
                                       afAddrType_t *srcAddr, uint8 seqNum );
static void simplemeter_GetProfileRspCB( zclCCGetProfileRsp_t *pCmd,
                                       afAddrType_t *srcAddr, uint8 seqNum );
static void simplemeter_ReqMirrorCmdCB( afAddrType_t *srcAddr, uint8 seqNum );
static void simplemeter_ReqMirrorRspCB( zclCCReqMirrorRsp_t *pCmd,
                                       afAddrType_t *srcAddr, uint8 seqNum );
static void simplemeter_MirrorRemCmdCB( afAddrType_t *srcAddr, uint8 seqNum );
static void simplemeter_MirrorRemRspCB( zclCCMirrorRemRsp_t *pCmd,
                                       afAddrType_t *srcAddr, uint8 seqNum );


/************************************************************************/
/***               Functions to process ZCL Foundation                ***/
/***               incoming Command/Response messages                 ***/
/************************************************************************/
static void simplemeter_ProcessZCLMsg( zclIncomingMsg_t *msg );
#if defined ( ZCL_READ )
static uint8 simplemeter_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif // ZCL_READ
#if defined ( ZCL_WRITE )
static uint8 simplemeter_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif // ZCL_WRITE
#if defined ( ZCL_REPORT )
static uint8 simplemeter_ProcessInConfigReportCmd( zclIncomingMsg_t *pInMsg );
static uint8 simplemeter_ProcessInConfigReportRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 simplemeter_ProcessInReadReportCfgCmd( zclIncomingMsg_t *pInMsg );
static uint8 simplemeter_ProcessInReadReportCfgRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 simplemeter_ProcessInReportCmd( zclIncomingMsg_t *pInMsg );
#endif // ZCL_REPORT
static uint8 simplemeter_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
#if defined ( ZCL_DISCOVER )
static uint8 simplemeter_ProcessInDiscRspCmd( zclIncomingMsg_t *pInMsg );
#endif // ZCL_DISCOVER

/*********************************************************************
 * ZCL General Clusters Callback table
 */
static zclGeneral_AppCallbacks_t simplemeter_GenCmdCallbacks =
{
  simplemeter_BasicResetCB,              // Basic Cluster Reset command
  simplemeter_IdentifyCB,                // Identify command
  simplemeter_IdentifyQueryRspCB,        // Identify Query Response command
  NULL,                                  // On/Off cluster commands
  NULL,                                  // Level Control Move to Level command
  NULL,                                  // Level Control Move command
  NULL,                                  // Level Control Step command
  NULL,                                  // Level Control Stop command
  NULL,                                  // Group Response commands
  NULL,                                  // Scene Store Request command
  NULL,                                  // Scene Recall Request command
  NULL,                                  // Scene Response command
  simplemeter_AlarmCB,                   // Alarm (Response) command
  NULL,                                  // RSSI Location command
  NULL,                                  // RSSI Location Response command
};

/*********************************************************************
 * ZCL SE Clusters Callback table
 */
static zclSE_AppCallbacks_t simplemeter_SECmdCallbacks =			
{
  simplemeter_GetProfileCmdCB,                        // Get Profile Command
  simplemeter_GetProfileRspCB,                        // Get Profile Response
  simplemeter_ReqMirrorCmdCB,                         // Request Mirror Command
  simplemeter_ReqMirrorRspCB,                         // Request Mirror Response
  simplemeter_MirrorRemCmdCB,                         // Mirror Remove Command
  simplemeter_MirrorRemRspCB,                         // Mirror Remove Response
  NULL,                                               // Get Current Price
  NULL,                                               // Get Scheduled Price
  NULL,                                               // Publish Price
  NULL,                                               // Display Message Command
  NULL,                                               // Cancel Message Command
  NULL,                                               // Get Last Message Command
  NULL,                                               // Message Confirmation
  NULL,                                               // Load Control Event
  NULL,                                               // Cancel Load Control Event
  NULL,                                               // Cancel All Load Control Events
  NULL,                                               // Report Event Status
  NULL,                                               // Get Scheduled Event
};

/*********************************************************************
 * @fn          simplemeter_Init
 *
 * @brief       Initialization function for the ZCL App Application.
 *
 * @param       uint8 task_id - simple meter task id
 *
 * @return      none
 */
void simplemeter_Init( uint8 task_id )
{
  simpleMeterTaskID = task_id;
  simpleMeterNwkState = DEV_INIT;
  simpleMeterTransID = 0;

  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

  // ESP destination address init
  ESPAddr.addrMode = (afAddrMode_t)Addr16Bit;
  ESPAddr.endPoint = SIMPLEMETER_ENDPOINT;
  ESPAddr.addr.shortAddr = 0;

  // register for SE endpoint
  zclSE_Init( &simpleMeterSimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SIMPLEMETER_ENDPOINT, &simplemeter_GenCmdCallbacks );

  // Register the ZCL SE Cluster Library callback functions
  zclSE_RegisterCmdCallbacks( SIMPLEMETER_ENDPOINT, &simplemeter_SECmdCallbacks );

  // Register the application's attribute list
  zcl_registerAttrList( SIMPLEMETER_ENDPOINT, SIMPLEMETER_MAX_ATTRIBUTES, simpleMeterAttrs );

  // Register the application's cluster option list
  zcl_registerClusterOptionList( SIMPLEMETER_ENDPOINT, SIMPLEMETER_MAX_OPTIONS, simpleMeterOptions );

  // Register the application's attribute data validation callback function
  zcl_registerValidateAttrData( simplemeter_ValidateAttrDataCB );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( simpleMeterTaskID );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( simpleMeterTaskID );

  // Start the timer to sync SimpleMeter timer with the osal timer
  osal_start_timerEx( simpleMeterTaskID, SIMPLEMETER_UPDATE_TIME_EVT, SIMPLEMETER_UPDATE_TIME_PERIOD );

  // setup attribute IDs of interest for Simple Meter

  pReportCmd = (zclReportCmd_t *)osal_mem_alloc( sizeof( zclReportCmd_t ) + ( numAttr * sizeof( zclReport_t ) ) );
  if ( pReportCmd != NULL )
  {
    pReportCmd->numAttr = numAttr;

    // Set up the first attribute
    pReportCmd->attrList[0].attrID = ATTRID_SE_CURRENT_SUMMATION_DELIVERED;
    pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT48;
    pReportCmd->attrList[0].attrData = simpleMeterCurrentSummationDelivered;

    // Set up the remaining attributes
  }
}

/*********************************************************************
 * @fn          simplemeter_event_loop
 *
 * @brief       Event Loop Processor for the simple meter.
 *
 * @param       uint8 task_id - the osal task id
 * @param       uint16 events - the event bitmask
 *
 * @return      none
 */
uint16 simplemeter_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( simpleMeterTaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZCL_INCOMING_MSG:
          // Incoming ZCL foundation command/response messages
          simplemeter_ProcessZCLMsg( (zclIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          simplemeter_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case ZDO_STATE_CHANGE:
          simpleMeterNwkState = (devStates_t)(MSGpkt->hdr.status);

          if (ZG_SECURE_ENABLED)
          {
            if ( (simpleMeterNwkState == DEV_ROUTER) || (simpleMeterNwkState == DEV_END_DEVICE) )
            {
              // check to see if link key had already been established
              linkKeyStatus = simplemeter_KeyEstablish_ReturnLinkKey(ESPAddr.addr.shortAddr);

              if (linkKeyStatus != ZSuccess)
              {
                // send out key establishment request
                osal_set_event( simpleMeterTaskID, SIMPLEMETER_KEY_ESTABLISHMENT_REQUEST_EVT);

              }
              else
              {
                // link key already established, resume sending reports
                osal_start_timerEx( simpleMeterTaskID, SIMPLEMETER_REPORT_ATTRIBUTE_EVT, SIMPLEMETER_REPORT_PERIOD );
              }
            }
          }
          else
          {
            osal_start_timerEx( simpleMeterTaskID, SIMPLEMETER_REPORT_ATTRIBUTE_EVT, SIMPLEMETER_REPORT_PERIOD );
          }

          NLME_SetPollRate ( SE_DEVICE_POLL_RATE ); // per smart energy spec end device polling requirement of not to poll < 7.5 seconds

          break;

#if defined( ZCL_KEY_ESTABLISH )
        case ZCL_KEY_ESTABLISH_IND:
          osal_start_timerEx( simpleMeterTaskID, SIMPLEMETER_REPORT_ATTRIBUTE_EVT, SIMPLEMETER_REPORT_PERIOD );

          break;
#endif

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // event to intiate key establishment request
  if ( events & SIMPLEMETER_KEY_ESTABLISHMENT_REQUEST_EVT )
  {
    zclGeneral_KeyEstablish_InitiateKeyEstablishment(simpleMeterTaskID, &ESPAddr, simpleMeterTransID);

    return ( events ^ SIMPLEMETER_KEY_ESTABLISHMENT_REQUEST_EVT );
  }

  // event to send report attribute
  if ( events & SIMPLEMETER_REPORT_ATTRIBUTE_EVT )
  {
    if ( pReportCmd != NULL )
    {
      zcl_SendReportCmd( SIMPLEMETER_ENDPOINT, &ESPAddr,
                         ZCL_CLUSTER_ID_SE_SIMPLE_METERING, pReportCmd,
                         ZCL_FRAME_SERVER_CLIENT_DIR, 1, 0 );

      osal_start_timerEx( simpleMeterTaskID, SIMPLEMETER_REPORT_ATTRIBUTE_EVT, SIMPLEMETER_REPORT_PERIOD );
    }

    return ( events ^ SIMPLEMETER_REPORT_ATTRIBUTE_EVT );
  }

  // handle processing of identify timeout event triggered by an identify command
  if ( events & SIMPLEMETER_IDENTIFY_TIMEOUT_EVT )
  {
    if ( simpleMeterIdentifyTime > 0 )
    {
      simpleMeterIdentifyTime--;
    }
    simplemeter_ProcessIdentifyTimeChange();

    return ( events ^ SIMPLEMETER_IDENTIFY_TIMEOUT_EVT );
  }

  // event to get current time
  if ( events & SIMPLEMETER_UPDATE_TIME_EVT )
  {
    simpleMeterTime = osal_getClock();
    osal_start_timerEx( simpleMeterTaskID, SIMPLEMETER_UPDATE_TIME_EVT, SIMPLEMETER_UPDATE_TIME_PERIOD );

    return ( events ^ SIMPLEMETER_UPDATE_TIME_EVT );
  }

  // Discard unknown events
  return 0;
}



/*********************************************************************
 * @fn      simplemeter_ProcessIdentifyTimeChange
 *
 * @brief   Called to blink led for specified IdentifyTime attribute value
 *
 * @param   none
 *
 * @return  none
 */
static void simplemeter_ProcessIdentifyTimeChange( void )
{
  if ( simpleMeterIdentifyTime > 0 )
  {
    osal_start_timerEx( simpleMeterTaskID, SIMPLEMETER_IDENTIFY_TIMEOUT_EVT, 1000 );
    HalLedBlink ( HAL_LED_4, 0xFF, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME );
  }
  else
  {
    HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
    osal_stop_timerEx( simpleMeterTaskID, SIMPLEMETER_IDENTIFY_TIMEOUT_EVT );
  }
}

#if defined ( ZCL_KEY_ESTABLISH )
/*********************************************************************
 * @fn      simplemeter_KeyEstablish_ReturnLinkKey
 *
 * @brief   This function get the requested link key
 *
 * @param   shortAddr - short address of the partner.
 *
 * @return  none
 */
static uint8 simplemeter_KeyEstablish_ReturnLinkKey( uint16 shortAddr )
{
  APSME_LinkKeyData_t* keyData;
  uint8 status = ZFailure;
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
    }
  }
  else
  {
    // It's an unknown device
    status = ZInvalidParameter;
  }

  return status;
}
#endif // ZCL_KEY_ESTABLISH
/*********************************************************************
 * @fn      simplemeter_HandleKeys
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
static void simplemeter_HandleKeys( uint8 shift, uint8 keys )
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
      ZDOInitDevice(0); // join the network
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
}

/*********************************************************************
 * @fn      simplemeter_ValidateAttrDataCB
 *
 * @brief   Check to see if the supplied value for the attribute data
 *          is within the specified range of the attribute.
 *
 * @param   pAttr - pointer to attribute
 * @param   pAttrInfo - pointer to attribute info
 *
 * @return  TRUE if data valid. FALSE otherwise.
 */
static uint8 simplemeter_ValidateAttrDataCB( zclAttrRec_t *pAttr, zclWriteRec_t *pAttrInfo )
{
  uint8 valid = TRUE;

  switch ( pAttrInfo->dataType )
  {
    case ZCL_DATATYPE_BOOLEAN:
      if ( ( *(pAttrInfo->attrData) != 0 ) && ( *(pAttrInfo->attrData) != 1 ) )
        valid = FALSE;
      break;

    default:
      break;
  }

  return ( valid );
}

/*********************************************************************
 * @fn      simplemeter_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library to set all
 *          the attributes of all the clusters to their factory defaults
 *
 * @param   none
 *
 * @return  none
 */
static void simplemeter_BasicResetCB( void )
{
  // user should handle setting attributes to factory defaults here
}

/*********************************************************************
 * @fn      simplemeter_IdentifyCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Command for this application.
 *
 * @param   pCmd - pointer to structure for identify command
 *
 * @return  none
 */
static void simplemeter_IdentifyCB( zclIdentify_t *pCmd )
{
  simpleMeterIdentifyTime = pCmd->identifyTime;
  simplemeter_ProcessIdentifyTimeChange();
}

/*********************************************************************
 * @fn      simplemeter_IdentifyQueryRspCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Query Response Command for this application.
 *
 * @param   pRsp - pointer to structure for identify query response
 *
 * @return  none
 */
static void simplemeter_IdentifyQueryRspCB( zclIdentifyQueryRsp_t *pRsp )
{
  // add user code here
}



/*********************************************************************
 * @fn      simplemeter_AlarmCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Alarm request or response command for
 *          this application.
 *
 * @param   pAlarm - pointer to structure for alarm command
 *
 * @return  none
 */
static void simplemeter_AlarmCB( zclAlarm_t *pAlarm )
{
  // add user code here
}

/*********************************************************************
 * @fn      simplemeter_GetProfileCmdCB
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
static void simplemeter_GetProfileCmdCB( zclCCGetProfileCmd_t *pCmd, afAddrType_t *srcAddr, uint8 seqNum )
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

  zclSE_SimpleMetering_Send_GetProfileRsp( SIMPLEMETER_ENDPOINT, srcAddr, endTime,
                                           status,
                                           profileIntervalPeriod,
                                           numberOfPeriodDelivered, intervals,
                                           false, seqNum );
#endif // ZCL_SIMPLE_METERING
}

/*********************************************************************
 * @fn      simplemeter_GetProfileRspCB
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
static void simplemeter_GetProfileRspCB( zclCCGetProfileRsp_t *pCmd, afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}

/*********************************************************************
 * @fn      simplemeter_ReqMirrorCmdCB
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
static void simplemeter_ReqMirrorCmdCB( afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}
/*********************************************************************
 * @fn      simplemeter_ReqMirrorRspCB
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
static void simplemeter_ReqMirrorRspCB( zclCCReqMirrorRsp_t *pRsp, afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}
/*********************************************************************
 * @fn      simplemeter_MirrorRemCmdCB
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
static void simplemeter_MirrorRemCmdCB( afAddrType_t *srcAddr, uint8 seqNum )
{
 // add user code here
}
/*********************************************************************
 * @fn      simplemeter_MirrorRemRspCB
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
static void simplemeter_MirrorRemRspCB( zclCCMirrorRemRsp_t *pCmd, afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}


/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      simplemeter_ProcessZCLMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - message to process
 *
 * @return  none
 */
static void simplemeter_ProcessZCLMsg( zclIncomingMsg_t *pInMsg )
{
  switch ( pInMsg->zclHdr.commandID )
  {
#if defined ( ZCL_READ )
    case ZCL_CMD_READ_RSP:
      simplemeter_ProcessInReadRspCmd( pInMsg );
      break;
#endif // ZCL_READ
#if defined ( ZCL_WRITE )
    case ZCL_CMD_WRITE_RSP:
      simplemeter_ProcessInWriteRspCmd( pInMsg );
      break;
#endif // ZCL_READ
#if defined ( ZCL_REPORT )
    case ZCL_CMD_CONFIG_REPORT:
      simplemeter_ProcessInConfigReportCmd( pInMsg );
      break;

    case ZCL_CMD_CONFIG_REPORT_RSP:
      simplemeter_ProcessInConfigReportRspCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      simplemeter_ProcessInReadReportCfgCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG_RSP:
      simplemeter_ProcessInReadReportCfgRspCmd( pInMsg );
      break;

    case ZCL_CMD_REPORT:
      simplemeter_ProcessInReportCmd( pInMsg );
      break;
#endif // ZCL_REPORT
    case ZCL_CMD_DEFAULT_RSP:
      simplemeter_ProcessInDefaultRspCmd( pInMsg );
      break;
#if defined ( ZCL_DISCOVER )
    case ZCL_CMD_DISCOVER_RSP:
      simplemeter_ProcessInDiscRspCmd( pInMsg );
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
 * @fn      simplemeter_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 simplemeter_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
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
 * @fn      simplemeter_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 simplemeter_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
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
 * @fn      simplemeter_ProcessInConfigReportCmd
 *
 * @brief   Process the "Profile" Configure Reporting Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  TRUE if attribute was found in the Attribute list,
 *          FALSE if not
 */
static uint8 simplemeter_ProcessInConfigReportCmd( zclIncomingMsg_t *pInMsg )
{
  zclCfgReportCmd_t *cfgReportCmd;
  zclCfgReportRec_t *reportRec;
  zclCfgReportRspCmd_t *cfgReportRspCmd;
  zclAttrRec_t attrRec;
  uint8 status;
  uint8 i, j = 0;

  cfgReportCmd = (zclCfgReportCmd_t *)pInMsg->attrCmd;

  // Allocate space for the response command
  cfgReportRspCmd = (zclCfgReportRspCmd_t *)osal_mem_alloc( sizeof ( zclCfgReportRspCmd_t ) + \
                                        sizeof ( zclCfgReportStatus_t) * cfgReportCmd->numAttr );
  if ( cfgReportRspCmd == NULL )
    return FALSE; // EMBEDDED RETURN

  // Process each Attribute Reporting Configuration record
  for ( i = 0; i < cfgReportCmd->numAttr; i++ )
  {
    reportRec = &(cfgReportCmd->attrList[i]);

    status = ZCL_STATUS_SUCCESS;

    if ( zclFindAttrRec( SIMPLEMETER_ENDPOINT, pInMsg->clusterId, reportRec->attrID, &attrRec ) )
    {
      if ( reportRec->direction == ZCL_SEND_ATTR_REPORTS )
      {
        if ( reportRec->dataType == attrRec.attr.dataType )
        {
          // This the attribute that is to be reported
          if ( zcl_MandatoryReportableAttribute( &attrRec ) == TRUE )
          {
            if ( reportRec->minReportInt < SIMPLEMETER_MIN_REPORTING_INTERVAL ||
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
  zcl_SendConfigReportRspCmd( SIMPLEMETER_ENDPOINT, &(pInMsg->srcAddr),
                              pInMsg->clusterId, cfgReportRspCmd, ZCL_FRAME_SERVER_CLIENT_DIR,
                              true, pInMsg->zclHdr.transSeqNum );
  osal_mem_free( cfgReportRspCmd );

  return TRUE ;
}

/*********************************************************************
 * @fn      simplemeter_ProcessInConfigReportRspCmd
 *
 * @brief   Process the "Profile" Configure Reporting Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 simplemeter_ProcessInConfigReportRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclCfgReportRspCmd_t *cfgReportRspCmd;
  zclAttrRec_t attrRec;
  uint8 i;

  cfgReportRspCmd = (zclCfgReportRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < cfgReportRspCmd->numAttr; i++)
  {
    if ( zclFindAttrRec( SIMPLEMETER_ENDPOINT, pInMsg->clusterId,
                         cfgReportRspCmd->attrList[i].attrID, &attrRec ) )
    {
      // Notify the device of success (or otherwise) of the its original configure
      // reporting command, for each attribute.
    }
  }

  return TRUE;
}

/*********************************************************************
 * @fn      simplemeter_ProcessInReadReportCfgCmd
 *
 * @brief   Process the "Profile" Read Reporting Configuration Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 simplemeter_ProcessInReadReportCfgCmd( zclIncomingMsg_t *pInMsg )
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
    if ( zclFindAttrRec( SIMPLEMETER_ENDPOINT, pInMsg->clusterId,
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

    if ( zclFindAttrRec( SIMPLEMETER_ENDPOINT, pInMsg->clusterId,
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
  zcl_SendReadReportCfgRspCmd( SIMPLEMETER_ENDPOINT, &(pInMsg->srcAddr),
                               pInMsg->clusterId, readReportCfgRspCmd, ZCL_FRAME_SERVER_CLIENT_DIR,
                               true, pInMsg->zclHdr.transSeqNum );
  osal_mem_free( readReportCfgRspCmd );

  return TRUE;
}

/*********************************************************************
 * @fn      simplemeter_ProcessInReadReportCfgRspCmd
 *
 * @brief   Process the "Profile" Read Reporting Configuration Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 simplemeter_ProcessInReadReportCfgRspCmd( zclIncomingMsg_t *pInMsg )
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
 * @fn      simplemeter_ProcessInReportCmd
 *
 * @brief   Process the "Profile" Report Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 simplemeter_ProcessInReportCmd( zclIncomingMsg_t *pInMsg )
{
  zclReportCmd_t *reportCmd;
  uint8 i;

  reportCmd = (zclReportCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < reportCmd->numAttr; i++)
  {
    // add user code here
  }

  return TRUE;
}
#endif // ZCL_REPORT

/*********************************************************************
 * @fn      simplemeter_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 simplemeter_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.

  return TRUE;
}

#if defined ( ZCL_DISCOVER )
/*********************************************************************
 * @fn      simplemeter_ProcessInDiscRspCmd
 *
 * @brief   Process the "Profile" Discover Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 simplemeter_ProcessInDiscRspCmd( zclIncomingMsg_t *pInMsg )
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
