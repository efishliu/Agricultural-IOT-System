/***************************************************************************************************
  Filename:       MT_UTIL.c
  Revised:        $Date: 2009-03-31 13:02:19 -0700 (Tue, 31 Mar 2009) $
  Revision:       $Revision: 19612 $

  Description:    MonitorTest Utility Functions

  Copyright 2007 - 2009 Texas Instruments Incorporated. All rights reserved.

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

 ***************************************************************************************************/

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "ZComDef.h"

#include "OnBoard.h"   /* This is here because of the key reading */
#include "hal_key.h"
#include "hal_led.h"
#include "OSAL_Nv.h"
#include "NLMEDE.h"
#include "ZDApp.h"
#include "MT.h"
#include "MT_UTIL.h"
#include "MT_ZDO.h"
#include "MT_SAPI.h"
#include "MT_NWK.h"
#include "MT_AF.h"
#include "MT_MAC.h"

/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/
#define MT_UTIL_DEVICE_INFO_RESPONSE_LEN 14

/***************************************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************************************/
#if defined (MT_UTIL_FUNC)
void MT_UtilGetDeviceInfo(void);
void MT_UtilGetNvInfo(void);
void MT_UtilSetPanID(uint8 *pBuf);
void MT_UtilSetChannels(uint8 *pBuf);
void MT_UtilSetSecLevel(uint8 *pBuf);
void MT_UtilSetPreCfgKey(uint8 *pBuf);
void MT_UtilCallbackSub(uint8 *pData);
void MT_UtilKeyEvent(uint8 *pBuf);
void MT_UtilHeartBeat(uint8 *pBuf);
void MT_UtilTimeAlive(void);
void MT_UtilLedControl(uint8 *pBuf);
#endif /* MT_UTIL_FUNC */

#if defined (MT_UTIL_FUNC)
/***************************************************************************************************
 * @fn      MT_UtilProcessing
 *
 * @brief   Process all the DEBUG commands that are issued by test tool
 *
 * @param   pBuf  - pointer to received SPI data message
 *
 * @return  status
 ***************************************************************************************************/
uint8 MT_UtilCommandProcessing(uint8 *pBuf)
{
  uint8 status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_UTIL_GET_DEVICE_INFO:
      MT_UtilGetDeviceInfo();
      break;

    case MT_UTIL_GET_NV_INFO:
      MT_UtilGetNvInfo();
      break;

    case MT_UTIL_SET_PANID:
      MT_UtilSetPanID(pBuf);
      break;

    case MT_UTIL_SET_CHANNELS:
      MT_UtilSetChannels(pBuf);
      break;

    case MT_UTIL_SET_SECLEVEL:
      MT_UtilSetSecLevel(pBuf);
      break;

    case MT_UTIL_SET_PRECFGKEY:
      MT_UtilSetPreCfgKey(pBuf);
      break;

    case MT_UTIL_CALLBACK_SUB_CMD:
      MT_UtilCallbackSub(pBuf);
      break;

    case MT_UTIL_KEY_EVENT:
#if (defined HAL_KEY) && (HAL_KEY == TRUE)
      MT_UtilKeyEvent(pBuf);
#endif
      break;

    case MT_UTIL_LED_CONTROL:
#if (defined HAL_LED) && (HAL_LED == TRUE)
      MT_UtilLedControl(pBuf);
#endif
      break;

    case MT_UTIL_HEARTBEAT:
      MT_UtilHeartBeat(pBuf);
      break;

    case MT_UTIL_TIME_ALIVE:
      MT_UtilTimeAlive();
      break;

    default:
      status = MT_RPC_ERR_COMMAND_ID;
      break;
  }

  return status;
}

/***************************************************************************************************
 * @fn      MT_UtilGetDeviceInfo
 *
 * @brief   The Get Device Info serial message.
 *
 * @param   void
 *
 * @return  void
 ***************************************************************************************************/
void MT_UtilGetDeviceInfo(void)
{
  uint8  *buf;
  uint8  *pBuf;
  uint8  bufLen = MT_UTIL_DEVICE_INFO_RESPONSE_LEN;
#if defined( RTR_NWK ) && !defined( NONWK )
  uint8  assocCnt = 0;
#endif
  uint16 *assocList = NULL;

#if defined( RTR_NWK ) && !defined( NONWK )
  assocList = AssocMakeList( &assocCnt );
  bufLen += (assocCnt * sizeof(uint16));
#endif

  buf = osal_mem_alloc( bufLen );
  if ( buf )
  {
    pBuf = buf;

    *pBuf++ = ZSUCCESS; // Status

    osal_nv_read( ZCD_NV_EXTADDR, 0, Z_EXTADDR_LEN, pBuf );
    pBuf += Z_EXTADDR_LEN;

#if defined( NONWK )
    // Skip past ZStack only parameters for NONWK
    *pBuf++ = 0;
    *pBuf++ = 0;
    *pBuf++ = 0;
    *pBuf++ = 0;
    *pBuf = 0;
#else
    {
      uint16 shortAddr = NLME_GetShortAddr();
      *pBuf++ = LO_UINT16( shortAddr );
      *pBuf++ = HI_UINT16( shortAddr );
    }

    /* Return device type */
    *pBuf++ = ZSTACK_DEVICE_BUILD;

    /*Return device state */
    *pBuf++ = (uint8)devState;

#if defined( RTR_NWK )
    *pBuf++ = assocCnt;

    if ( assocCnt )
    {
      uint8 x;
      uint16 *puint16 = assocList;

      for ( x = 0; x < assocCnt; x++, puint16++ )
      {
        *pBuf++ = LO_UINT16( *puint16 );
        *pBuf++ = HI_UINT16( *puint16 );
      }
    }
#else
    *pBuf++ = 0;
#endif
#endif

    MT_BuildAndSendZToolResponse( ((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_UTIL),
                                 MT_UTIL_GET_DEVICE_INFO,
                                 bufLen, buf );

    osal_mem_free( buf );
  }

  if ( assocList )
  {
    osal_mem_free( assocList );
  }
}

/***************************************************************************************************
 * @fn      MT_UtilGetNvInfo
 *
 * @brief   The Get NV Info serial message.
 *
 * @param   byte *msg - pointer to the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_UtilGetNvInfo(void)
{
  uint8 len;
  uint8 stat;
  uint8 *buf;
  uint8 *pBuf;
  uint16 tmp16;
  uint32 tmp32;

  /*
    Get required length of buffer
    Status + ExtAddr + ChanList + PanID  + SecLevel + PreCfgKey
  */
  len = 1 + Z_EXTADDR_LEN + 4 + 2 + 1 + SEC_KEY_LEN;

  buf = osal_mem_alloc( len );
  if ( buf )
  {
    /* Assume NV not available */
    osal_memset( buf, 0xFF, len );

    /* Skip over status */
    pBuf = buf + 1;

    /* Start with 64-bit extended address */
    stat = osal_nv_read( ZCD_NV_EXTADDR, 0, Z_EXTADDR_LEN, pBuf );
    if ( stat ) stat = 0x01;
    pBuf += Z_EXTADDR_LEN;

    /* Scan channel list (bit mask) */
    if (  osal_nv_read( ZCD_NV_CHANLIST, 0, sizeof( tmp32 ), &tmp32 ) )
      stat |= 0x02;
    else
    {
      pBuf[0] = BREAK_UINT32( tmp32, 3 );
      pBuf[1] = BREAK_UINT32( tmp32, 2 );
      pBuf[2] = BREAK_UINT32( tmp32, 1 );
      pBuf[3] = BREAK_UINT32( tmp32, 0 );
    }
    pBuf += sizeof( tmp32 );

    /* ZigBee PanID */
    if ( osal_nv_read( ZCD_NV_PANID, 0, sizeof( tmp16 ), &tmp16 ) )
      stat |= 0x04;
    else
    {
      pBuf[0] = LO_UINT16( tmp16 );
      pBuf[1] = HI_UINT16( tmp16 );
    }
    pBuf += sizeof( tmp16 );

    /* Security level */
    if ( osal_nv_read( ZCD_NV_SECURITY_LEVEL, 0, sizeof( uint8 ), pBuf++ ) )
      stat |= 0x08;

    /* Pre-configured security key */
    if ( osal_nv_read( ZCD_NV_PRECFGKEY, 0, SEC_KEY_LEN, pBuf ) )
      stat |= 0x10;

    /* Status bit mask - bit=1 indicates failure */
    *buf = stat;

    MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_UTIL), MT_UTIL_GET_NV_INFO,
                                  len, buf );

    osal_mem_free( buf );
  }
}

/***************************************************************************************************
 * @fn      MT_UtilSetPanID
 *
 * @brief   Set PanID message
 *
 * @param   byte *msg - pointer to the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_UtilSetPanID(uint8 *pBuf)
{
  uint16 temp16;
  uint8 retValue = ZFailure;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  temp16 = BUILD_UINT16(pBuf[0], pBuf[1]);
  pBuf += sizeof(uint16);

  retValue = osal_nv_write(ZCD_NV_PANID, 0, osal_nv_item_len( ZCD_NV_PANID ), &temp16);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_UTIL), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_UtilSetChannels
 *
 * @brief   Set Channels
 *
 * @param   byte *msg - pointer to the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_UtilSetChannels(uint8 *pBuf)
{
  uint32 tmp32;
  uint8 retValue = ZFailure;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  tmp32 = BUILD_UINT32(pBuf[0], pBuf[1], pBuf[2], pBuf[3]);

  retValue = osal_nv_write(ZCD_NV_CHANLIST, 0, osal_nv_item_len( ZCD_NV_CHANLIST ), &tmp32);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_UTIL), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_UtilSetSecLevel
 *
 * @brief   Set Sec Level
 *
 * @param   byte *msg - pointer to the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_UtilSetSecLevel(uint8 *pBuf)
{
  uint8 retValue = ZFailure;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = osal_nv_write( ZCD_NV_SECURITY_LEVEL, 0, osal_nv_item_len( ZCD_NV_SECURITY_LEVEL ), pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_UTIL), cmdId, 1, &retValue );

}

/***************************************************************************************************
 * @fn      MT_UtilSetPreCfgKey
 *
 * @brief   Set Pre Cfg Key
 *
 * @param   byte *msg - pointer to the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_UtilSetPreCfgKey(uint8 *pBuf)
{
  uint8 retValue = ZFailure;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = osal_nv_write( ZCD_NV_PRECFGKEY, 0, osal_nv_item_len( ZCD_NV_PRECFGKEY ), pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_UTIL), cmdId, 1, &retValue );

}

/***************************************************************************************************
 * @fn      MT_UtilCallbackSub
 *
 * @brief   The Callback subscribe.
 *
 * @param   byte *msg - pointer to the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_UtilCallbackSub(uint8 *pBuf)
{
  uint8 cmdId = pBuf[MT_RPC_POS_CMD1];
  uint8 retValue = ZFailure;

#if defined(MT_MAC_CB_FUNC) || defined(MT_NWK_CB_FUNC) || defined(MT_ZDO_CB_FUNC) || defined(MT_AF_CB_FUNC) || defined(MT_SAPI_CB_FUNC) || defined(MT_SAPI_CB_FUNC)
  uint8 subSystem;
  uint16 subscribed_command;

  // Move past header
  retValue = ZSuccess;
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Command */
  subscribed_command = BUILD_UINT16(pBuf[0], pBuf[1]);
  pBuf += 2;

  /* Subsystem - 5 bits on the MSB of the command */
  subSystem = HI_UINT16(subscribed_command) & 0x1F ;

  /* What is the action - SUBSCRIBE or !SUBSCRIBE */
  if (*pBuf)
  {
    /* Turn ON */
  #if defined( MT_MAC_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_MAC) || (subscribed_command == 0xFFFF))
      _macCallbackSub = 0xFFFF;
  #endif

  #if defined( MT_NWK_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_NWK) || (subscribed_command == 0xFFFF))
      _nwkCallbackSub = 0xFFFF;
  #endif

  #if defined( MT_ZDO_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_ZDO) || (subscribed_command == 0xFFFF))
      _zdoCallbackSub = 0xFFFFFFFF;
  #endif

  #if defined( MT_AF_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_AF) || (subscribed_command == 0xFFFF))
      _afCallbackSub = 0xFFFF;
  #endif

  #if defined( MT_SAPI_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_SAPI) || (subscribed_command == 0xFFFF))
      _sapiCallbackSub = 0xFFFF;
  #endif
  }
  else
  {
    /* Turn OFF */
  #if defined( MT_MAC_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_MAC) || (subscribed_command == 0xFFFF))
      _macCallbackSub = 0x0000;
  #endif

  #if defined( MT_NWK_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_NWK) || (subscribed_command == 0xFFFF))
      _nwkCallbackSub = 0x0000;
  #endif

  #if defined( MT_ZDO_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_ZDO) || (subscribed_command == 0xFFFF))
      _zdoCallbackSub = 0x00000000;
  #endif

  #if defined( MT_AF_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_AF) || (subscribed_command == 0xFFFF))
      _afCallbackSub = 0x0000;
  #endif

  #if defined( MT_SAPI_CB_FUNC )
    if ((subSystem == MT_RPC_SYS_SAPI) || (subscribed_command == 0xFFFF))
        _sapiCallbackSub = 0x0000;
  #endif
  }
#endif  // MT_MAC_CB_FUNC || MT_NWK_CB_FUNC || MT_ZDO_CB_FUNC || MT_AF_CB_FUNC || MT_SAPI_CB_FUNC || MT_SAPI_CB_FUNC

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_UTIL), cmdId, 1, &retValue );
}

#if (defined HAL_KEY) && (HAL_KEY == TRUE)
/***************************************************************************************************
 * @fn      MT_UtilKeyEvent
 *
 * @brief   Process Key Event
 *
 * @param   byte *msg - pointer to the data
 *
 * @return  void
 ***************************************************************************************************/
void MT_UtilKeyEvent(uint8 *pBuf)
{
  uint8 x = 0;
  uint8 retValue = ZFailure;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Translate between SPI values to device values */
  if ( *pBuf & 0x01 )
    x |= HAL_KEY_SW_1;
  if ( *pBuf & 0x02 )
    x |= HAL_KEY_SW_2;
  if ( *pBuf & 0x04 )
    x |= HAL_KEY_SW_3;
  if ( *pBuf & 0x08 )
  x |= HAL_KEY_SW_4;
#if defined ( HAL_KEY_SW_5 )
  if ( *pBuf & 0x10 )
    x |= HAL_KEY_SW_5;
#endif
#if defined ( HAL_KEY_SW_6 )
  if ( *pBuf & 0x20 )
    x |= HAL_KEY_SW_6;
#endif
#if defined ( HAL_KEY_SW_7 )
  if ( *pBuf & 0x40 )
    x |= HAL_KEY_SW_7;
#endif
#if defined ( HAL_KEY_SW_8 )
  if ( *pBuf & 0x80 )
    x |= HAL_KEY_SW_8;
#endif
  pBuf++;

  retValue = OnBoard_SendKeys(x, *pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_UTIL), cmdId, 1, &retValue );
}
#endif

#if (defined HAL_LED) && (HAL_LED == TRUE)
/***************************************************************************************************
 * @fn      MT_UtilLedControl
 *
 * @brief   Process the LED Control Message
 *
 * @param   pBuf - pointer to the received data
 *
 * @return  None
 ***************************************************************************************************/
void MT_UtilLedControl(uint8 *pBuf)
{
  uint8 iLed, Led, iMode, Mode, cmdId;
  uint8 retValue = ZFailure;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* LED and Mode */
  iLed = *pBuf++;
  iMode = *pBuf;

  if ( iLed == 1 )
    Led = HAL_LED_1;
  else if ( iLed == 2 )
    Led = HAL_LED_2;
  else if ( iLed == 3 )
    Led = HAL_LED_3;
  else if ( iLed == 4 )
    Led = HAL_LED_4;
  else if ( iLed == 0xFF )
    Led = HAL_LED_ALL;
  else
    Led = 0;

  if ( iMode == 0 )
    Mode = HAL_LED_MODE_OFF;
  else if ( iMode == 1 )
    Mode = HAL_LED_MODE_ON;
  else if ( iMode == 2 )
    Mode = HAL_LED_MODE_BLINK;
  else if ( iMode == 3 )
    Mode = HAL_LED_MODE_FLASH;
  else if ( iMode == 4 )
    Mode = HAL_LED_MODE_TOGGLE;
  else
    Led = 0;

  if ( Led != 0 )
  {
    HalLedSet (Led, Mode);
    retValue = ZSuccess;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_UTIL), cmdId, 1, &retValue );

}
#endif /* HAL_LED */

/***************************************************************************************************
 * @fn      MT_UtilHeartBeat
 *
 * @brief   Process the Heart Beat
 *
 * @param   pBuf - pointer to the received data
 *
 * @return  None
 ***************************************************************************************************/
void MT_UtilHeartBeat(uint8 *pBuf)
{
  //TBD
  (void)pBuf;  // Remove this when heart beat function is established.
}

/***************************************************************************************************
 * @fn      MT_UtilTimeAlive
 *
 * @brief   Process Time Alive
 *
 * @param   pBuf - pointer to the received data
 *
 * @return  None
 ***************************************************************************************************/
void MT_UtilTimeAlive(void)
{
  uint8 timeAlive[4];
  uint32 tmp32;

  /* Time since last reset (seconds) */
  tmp32 = osal_GetSystemClock() / 1000;

  /* Convert to high byte first into temp buffer */
  timeAlive[0] = BREAK_UINT32(tmp32, 0);
  timeAlive[1] = BREAK_UINT32(tmp32, 1);
  timeAlive[2] = BREAK_UINT32(tmp32, 2);
  timeAlive[3] = BREAK_UINT32(tmp32, 3);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_UTIL), MT_UTIL_TIME_ALIVE,
                               sizeof(tmp32), timeAlive );
}
#endif /* MT_UTIL_FUNC */
/***************************************************************************************************
 ***************************************************************************************************/
