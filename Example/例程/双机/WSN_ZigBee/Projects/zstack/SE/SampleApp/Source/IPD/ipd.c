/**************************************************************************************************
  Filename:       ipd.c
  Revised:        $Date: 2007-08-02 09:20:10 -0700 (Thu,02 Aug 2007) $
  Revision:       $Revision: 15001 $

  Description:    This module implements the IPD functionality and contains the
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
  exploits the following clusters for an IPD configuration:

  General Basic
  General Alarms
  General Time
  General Key Establishment
  SE     Price
  SE     Message


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
#include "ipd.h"
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

static uint8 ipdTaskID;            // osal task id for IPD
static uint8 ipdTransID;           // transaction id
static devStates_t ipdNwkState;    // network state variable
static afAddrType_t ESPAddr;       // ESP destination address
static uint8 linkKeyStatus;        // status variable returned from get link key function
static uint8 option;               // tx options field

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void ipd_HandleKeys( uint8 shift, uint8 keys );

#if defined (ZCL_KEY_ESTABLISH)
static uint8 ipd_KeyEstablish_ReturnLinkKey( uint16 shortAddr );
#endif // ZCL_KEY_ESTABLISH

#if defined ( ZCL_ALARMS )
static void ipd_ProcessAlarmCmd( uint8 srcEP, afAddrType_t *dstAddr,
                        uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data );
#endif // ZCL_ALARMS

static void ipd_ProcessIdentifyTimeChange( void );

/*************************************************************************/
/*** Application Callback Functions                                    ***/
/*************************************************************************/

// Foundation Callback functions
static uint8 ipd_ValidateAttrDataCB( zclAttrRec_t *pAttr, zclWriteRec_t *pAttrInfo );

// General Cluster Callback functions
static void ipd_BasicResetCB( void );
static void ipd_IdentifyCB( zclIdentify_t *pCmd );
static void ipd_IdentifyQueryRspCB( zclIdentifyQueryRsp_t *pRsp );
static void ipd_AlarmCB( zclAlarm_t *pAlarm );

// SE Callback functions
static void ipd_GetCurrentPriceCB( zclCCGetCurrentPrice_t *pCmd,
                                         afAddrType_t *srcAddr, uint8 seqNum );
static void ipd_GetScheduledPriceCB( zclCCGetScheduledPrice_t *pCmd,
                                         afAddrType_t *srcAddr, uint8 seqNum );
static void ipd_PublishPriceCB( zclCCPublishPrice_t *pCmd,
                                      afAddrType_t *srcAddr, uint8 seqNum );
static void ipd_DisplayMessageCB( zclCCDisplayMessage_t *pCmd,
                                        afAddrType_t *srcAddr, uint8 seqNum );
static void ipd_CancelMessageCB( zclCCCancelMessage_t *pCmd,
                                       afAddrType_t *srcAddr, uint8 seqNum );
static void ipd_GetLastMessageCB( afAddrType_t *srcAddr, uint8 seqNum );
static void ipd_MessageConfirmationCB( zclCCMessageConfirmation_t *pCmd,
                                             afAddrType_t *srcAddr, uint8 seqNum );


/************************************************************************/
/***               Functions to process ZCL Foundation                ***/
/***               incoming Command/Response messages                 ***/
/************************************************************************/
static void ipd_ProcessZCLMsg( zclIncomingMsg_t *msg );
#if defined ( ZCL_READ )
static uint8 ipd_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif // ZCL_READ
#if defined ( ZCL_WRITE )
static uint8 ipd_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif // ZCL_WRITE
static uint8 ipd_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
#if defined ( ZCL_DISCOVER )
static uint8 ipd_ProcessInDiscRspCmd( zclIncomingMsg_t *pInMsg );
#endif // ZCL_DISCOVER

/*********************************************************************
 * ZCL General Clusters Callback table
 */
static zclGeneral_AppCallbacks_t ipd_GenCmdCallbacks =
{
  ipd_BasicResetCB,              // Basic Cluster Reset command
  ipd_IdentifyCB,                // Identify command
  ipd_IdentifyQueryRspCB,        // Identify Query Response command
  NULL,                          // On/Off cluster commands
  NULL,                          // Level Control Move to Level command
  NULL,                          // Level Control Move command
  NULL,                          // Level Control Step command
  NULL,                          // Level Control Stop command
  NULL,                          // Group Response commands
  NULL,                          // Scene Store Request command
  NULL,                          // Scene Recall Request command
  NULL,                          // Scene Response command
  ipd_AlarmCB,                   // Alarm (Response) command
  NULL,                          // RSSI Location command
  NULL,                          // RSSI Location Response command
};

/*********************************************************************
 * ZCL SE Clusters Callback table
 */
static zclSE_AppCallbacks_t ipd_SECmdCallbacks =			
{
  NULL,                             // Get Profile Command
  NULL,                             // Get Profile Response
  NULL,                             // Request Mirror Command
  NULL,                             // Request Mirror Response
  NULL,                             // Mirror Remove Command
  NULL,                             // Mirror Remove Response
  ipd_GetCurrentPriceCB,            // Get Current Price
  ipd_GetScheduledPriceCB,          // Get Scheduled Price
  ipd_PublishPriceCB,               // Publish Price
  ipd_DisplayMessageCB,             // Display Message Command
  ipd_CancelMessageCB,              // Cancel Message Command
  ipd_GetLastMessageCB,             // Get Last Message Command
  ipd_MessageConfirmationCB,        // Message Confirmation
  NULL,                             // Load Control Event
  NULL,                             // Cancel Load Control Event
  NULL,                             // Cancel All Load Control Events
  NULL,                             // Report Event Status
  NULL,                             // Get Scheduled Event
};

/*********************************************************************
 * @fn          ipd_Init
 *
 * @brief       Initialization function for the ZCL App Application.
 *
 * @param       uint8 task_id - ipd task id
 *
 * @return      none
 */
void ipd_Init( uint8 task_id )
{
  ipdTaskID = task_id;
  ipdNwkState = DEV_INIT;
  ipdTransID = 0;

  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

  // setup ESP destination address
  ESPAddr.addrMode = (afAddrMode_t)Addr16Bit;
  ESPAddr.endPoint = IPD_ENDPOINT;
  ESPAddr.addr.shortAddr = 0;

  // Register for SE endpoint
  zclSE_Init( &ipdSimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( IPD_ENDPOINT, &ipd_GenCmdCallbacks );

  // Register the ZCL SE Cluster Library callback functions
  zclSE_RegisterCmdCallbacks( IPD_ENDPOINT, &ipd_SECmdCallbacks );

  // Register the application's attribute list
  zcl_registerAttrList( IPD_ENDPOINT, IPD_MAX_ATTRIBUTES, ipdAttrs );

  // Register the application's cluster option list
  zcl_registerClusterOptionList( IPD_ENDPOINT, IPD_MAX_OPTIONS, ipdOptions );

  // Register the application's attribute data validation callback function
  zcl_registerValidateAttrData( ipd_ValidateAttrDataCB );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( ipdTaskID );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( ipdTaskID );

  // Start the timer to sync IPD timer with the osal timer
  osal_start_timerEx( ipdTaskID, IPD_UPDATE_TIME_EVT, IPD_UPDATE_TIME_PERIOD );
}

/*********************************************************************
 * @fn          ipd_event_loop
 *
 * @brief       Event Loop Processor for ipd.
 *
 * @param       uint8 task_id - ipd task id
 * @param       uint16 events - event bitmask
 *
 * @return      none
 */
uint16 ipd_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( ipdTaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZCL_INCOMING_MSG:
          // Incoming ZCL foundation command/response messages
          ipd_ProcessZCLMsg( (zclIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          ipd_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case ZDO_STATE_CHANGE:
          ipdNwkState = (devStates_t)(MSGpkt->hdr.status);

          if (ZG_SECURE_ENABLED)
          {
            if ( ipdNwkState == DEV_END_DEVICE )
            {
              // check to see if link key had already been established
              linkKeyStatus = ipd_KeyEstablish_ReturnLinkKey(ESPAddr.addr.shortAddr);

              if (linkKeyStatus != ZSuccess)
              {
                // send out key establishment request
                osal_set_event( ipdTaskID, IPD_KEY_ESTABLISHMENT_REQUEST_EVT);
              }
              else
              {
                // link key already established, resume sending reports
                osal_start_timerEx( ipdTaskID, IPD_GET_PRICING_INFO_EVT, IPD_GET_PRICING_INFO_PERIOD );
              }
            }
          }
          else
          {
            osal_start_timerEx( ipdTaskID, IPD_GET_PRICING_INFO_EVT, IPD_GET_PRICING_INFO_PERIOD );
          }

          NLME_SetPollRate ( SE_DEVICE_POLL_RATE ); // per smart energy spec end device polling requirement of not to poll < 7.5 seconds

          break;

#if defined( ZCL_KEY_ESTABLISH )
        case ZCL_KEY_ESTABLISH_IND:
          osal_start_timerEx( ipdTaskID, IPD_GET_PRICING_INFO_EVT, IPD_GET_PRICING_INFO_PERIOD );

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
  if ( events & IPD_KEY_ESTABLISHMENT_REQUEST_EVT )
  {
    zclGeneral_KeyEstablish_InitiateKeyEstablishment(ipdTaskID, &ESPAddr, ipdTransID);

    return ( events ^ IPD_KEY_ESTABLISHMENT_REQUEST_EVT );
  }

  // event to get current price
  if ( events & IPD_GET_PRICING_INFO_EVT )
  {
    zclSE_Pricing_Send_GetCurrentPrice( IPD_ENDPOINT, &ESPAddr, option, TRUE, 0 );

    osal_start_timerEx( ipdTaskID, IPD_GET_PRICING_INFO_EVT, IPD_GET_PRICING_INFO_PERIOD );

    return ( events ^ IPD_GET_PRICING_INFO_EVT );
  }

  // handle processing of identify timeout event triggered by an identify command
  if ( events & IPD_IDENTIFY_TIMEOUT_EVT )
  {
    if ( ipdIdentifyTime > 0 )
    {
      ipdIdentifyTime--;
    }
    ipd_ProcessIdentifyTimeChange();

    return ( events ^ IPD_IDENTIFY_TIMEOUT_EVT );
  }

  // event to get current time
  if ( events & IPD_UPDATE_TIME_EVT )
  {
    ipdTime = osal_getClock();
    osal_start_timerEx( ipdTaskID, IPD_UPDATE_TIME_EVT, IPD_UPDATE_TIME_PERIOD );

    return ( events ^ IPD_UPDATE_TIME_EVT );
  }

  // Discard unknown events
  return 0;
}



/*********************************************************************
 * @fn      ipd_ProcessIdentifyTimeChange
 *
 * @brief   Called to blink led for specified IdentifyTime attribute value
 *
 * @param   none
 *
 * @return  none
 */
static void ipd_ProcessIdentifyTimeChange( void )
{
  if ( ipdIdentifyTime > 0 )
  {
    osal_start_timerEx( ipdTaskID, IPD_IDENTIFY_TIMEOUT_EVT, 1000 );
    HalLedBlink ( HAL_LED_4, 0xFF, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME );
  }
  else
  {
    HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
    osal_stop_timerEx( ipdTaskID, IPD_IDENTIFY_TIMEOUT_EVT );
  }
}

#if defined (ZCL_KEY_ESTABLISH)
/*********************************************************************
 * @fn      ipd_KeyEstablish_ReturnLinkKey
 *
 * @brief   This function get the requested link key
 *
 * @param   shortAddr - short address of the partner.
 *
 * @return  none
 */
static uint8 ipd_KeyEstablish_ReturnLinkKey( uint16 shortAddr )
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
 * @fn      ipd_HandleKeys
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
static void ipd_HandleKeys( uint8 shift, uint8 keys )
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
 * @fn      ipd_ValidateAttrDataCB
 *
 * @brief   Check to see if the supplied value for the attribute data
 *          is within the specified range of the attribute.
 *
 * @param   pAttr - pointer to attribute
 * @param   pAttrInfo - pointer to attribute info
 *
 * @return  TRUE if data valid. FALSE otherwise.
 */
static uint8 ipd_ValidateAttrDataCB( zclAttrRec_t *pAttr, zclWriteRec_t *pAttrInfo )
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
 * @fn      ipd_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library to set all
 *          the attributes of all the clusters to their factory defaults
 *
 * @param   none
 *
 * @return  none
 */
static void ipd_BasicResetCB( void )
{
  // user should handle setting attributes to factory defaults here
}

/*********************************************************************
 * @fn      ipd_IdentifyCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Command for this application.
 *
 * @param   pCmd - pointer to structure for identify command
 *
 * @return  none
 */
static void ipd_IdentifyCB( zclIdentify_t *pCmd )
{
  ipdIdentifyTime = pCmd->identifyTime;
  ipd_ProcessIdentifyTimeChange();
}

/*********************************************************************
 * @fn      ipd_IdentifyQueryRspCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Query Response Command for this application.
 *
 * @param   pRsp - pointer to structure for identify query response
 *
 * @return  none
 */
static void ipd_IdentifyQueryRspCB( zclIdentifyQueryRsp_t *pRsp )
{
  // add user code here
}

/*********************************************************************
 * @fn      ipd_AlarmCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Alarm request or response command for
 *          this application.
 *
 * @param   pAlarm - pointer to structure for alarm command
 *
 * @return  none
 */
static void ipd_AlarmCB( zclAlarm_t *pAlarm )
{
  // add user code here
}

/*********************************************************************
 * @fn      ipd_GetCurrentPriceCB
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
static void ipd_GetCurrentPriceCB( zclCCGetCurrentPrice_t *pCmd,
                                         afAddrType_t *srcAddr, uint8 seqNum )
{
#if defined ( ZCL_PRICING )
  // On receipt of Get Current Price command, the device shall send a
  // Publish Price command with the information for the current time.
  zclCCPublishPrice_t cmd;

  osal_memset( &cmd, 0, sizeof( zclCCPublishPrice_t ) );

  cmd.providerId = 0xbabeface;
  cmd.priceTier = 0xfe;

  zclSE_Pricing_Send_PublishPrice( IPD_ENDPOINT, srcAddr, &cmd, false, seqNum );
#endif // ZCL_PRICING
}

/*********************************************************************
 * @fn      ipd_GetScheduledPriceCB
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
static void ipd_GetScheduledPriceCB( zclCCGetScheduledPrice_t *pCmd, afAddrType_t *srcAddr, uint8 seqNum  )
{
  // On receipt of Get Scheduled Price command, the device shall send a
  // Publish Price command for all currently scheduled price events.
  // The sample code as follows only sends one.

#if defined ( ZCL_PRICING )
  zclCCPublishPrice_t cmd;

  osal_memset( &cmd, 0, sizeof( zclCCPublishPrice_t ) );

  cmd.providerId = 0xbabeface;
  cmd.priceTier = 0xfe;

  zclSE_Pricing_Send_PublishPrice( IPD_ENDPOINT, srcAddr, &cmd, false, seqNum );

#endif // ZCL_PRICING
}

/*********************************************************************
 * @fn      ipd_PublishPriceCB
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
static void ipd_PublishPriceCB( zclCCPublishPrice_t *pCmd,
                                      afAddrType_t *srcAddr, uint8 seqNum )
{
  if ( pCmd )
  {
    // display Provider ID field
    HalLcdWriteString("Provider ID", HAL_LCD_LINE_1);
    HalLcdWriteValue( pCmd->providerId, 10, HAL_LCD_LINE_2 );
  }
}

/*********************************************************************
 * @fn      ipd_DisplayMessageCB
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
static void ipd_DisplayMessageCB( zclCCDisplayMessage_t *pCmd,
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
    HalLcdWriteString( (char*)pCmd->msgString.pStr, HAL_LCD_LINE_3 );
#endif // LCD_SUPPORTED
}

/*********************************************************************
 * @fn      ipd_CancelMessageCB
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
static void ipd_CancelMessageCB( zclCCCancelMessage_t *pCmd,
                                        afAddrType_t *srcAddr, uint8 seqNum )
{
  // add user code here
}

/*********************************************************************
 * @fn      ipd_GetLastMessageCB
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
static void ipd_GetLastMessageCB( afAddrType_t *srcAddr, uint8 seqNum )
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

  zclSE_Message_Send_DisplayMessage( IPD_ENDPOINT, srcAddr, &cmd,
                                     false, seqNum );
#endif // ZCL_MESSAGE
}

/*********************************************************************
 * @fn      ipd_MessageConfirmationCB
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
static void ipd_MessageConfirmationCB( zclCCMessageConfirmation_t *pCmd,
                                             afAddrType_t *srcAddr, uint8 seqNum)
{
 // add user code here
}

/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      ipd_ProcessZCLMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - message to process
 *
 * @return  none
 */
static void ipd_ProcessZCLMsg( zclIncomingMsg_t *pInMsg )
{
  switch ( pInMsg->zclHdr.commandID )
  {
#if defined ( ZCL_READ )
    case ZCL_CMD_READ_RSP:
      ipd_ProcessInReadRspCmd( pInMsg );
      break;
#endif // ZCL_READ
#if defined ( ZCL_WRITE )
    case ZCL_CMD_WRITE_RSP:
      ipd_ProcessInWriteRspCmd( pInMsg );
      break;
#endif // ZCL_WRITE
    case ZCL_CMD_DEFAULT_RSP:
      ipd_ProcessInDefaultRspCmd( pInMsg );
      break;
#if defined ( ZCL_DISCOVER )
    case ZCL_CMD_DISCOVER_RSP:
      ipd_ProcessInDiscRspCmd( pInMsg );
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
 * @fn      ipd_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 ipd_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
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
 * @fn      ipd_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 ipd_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
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

/*********************************************************************
 * @fn      ipd_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 ipd_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.

  return TRUE;
}

#if defined ( ZCL_DISCOVER )
/*********************************************************************
 * @fn      ipd_ProcessInDiscRspCmd
 *
 * @brief   Process the "Profile" Discover Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 ipd_ProcessInDiscRspCmd( zclIncomingMsg_t *pInMsg )
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
