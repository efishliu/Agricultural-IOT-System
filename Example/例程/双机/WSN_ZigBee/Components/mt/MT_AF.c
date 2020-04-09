/**************************************************************************************************
  Filename:       MT_AF.c
  Revised:        $Date: 2008-10-10 09:56:24 -0700 (Fri, 10 Oct 2008) $
  Revision:       $Revision: 18257 $


  Description:    MonitorTest functions for the AF layer.


  Copyright 2007 Texas Instruments Incorporated. All rights reserved.

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

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "ZComDef.h"
#include "OSAL.h"
#include "MT.h"
#include "MT_AF.h"
#include "nwk.h"
#include "OnBoard.h"
#include "MT_UART.h"


/***************************************************************************************************
 * GLOBAL VARIABLES
 ***************************************************************************************************/

#if defined ( MT_AF_CB_FUNC )
uint16 _afCallbackSub;
#endif

/***************************************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************************************/
void MT_AfRegister(uint8 *pBuf);
void MT_AfDataRequest(uint8 *pBuf);

/***************************************************************************************************
 * @fn      MT_afCommandProcessing
 *
 * @brief   Process all the AF commands that are issued by test tool
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  status
 ***************************************************************************************************/
uint8 MT_AfCommandProcessing(uint8 *pBuf)
{
  uint8 status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_AF_REGISTER:
      MT_AfRegister(pBuf);
      break;

    case MT_AF_DATA_REQUEST:
      MT_AfDataRequest(pBuf);
      break;

    default:
      status = MT_RPC_ERR_COMMAND_ID;
      break;
  }

  return status;
}

/***************************************************************************************************
 * @fn      MT_AfRegister
 *
 * @brief   Process AF Register command
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
void MT_AfRegister(uint8 *pBuf)
{
  uint8 cmdId;
  uint8 retValue = ZMemError;
  endPointDesc_t *epDesc;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  epDesc = (endPointDesc_t *)osal_mem_alloc(sizeof(endPointDesc_t));
  if ( epDesc )
  {
    epDesc->task_id = &MT_TaskID;
    retValue = MT_BuildEndpointDesc( pBuf, epDesc );
    if ( retValue == ZSuccess )
    {
      retValue = afRegister( epDesc );
    }

    if ( retValue != ZSuccess )
    {
      osal_mem_free( epDesc );
    }
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_AF), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_AfDataRequest
 *
 * @brief   Process AF Register command
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
void MT_AfDataRequest(uint8 *pBuf)
{
  uint8 cmdId, tempLen = 0;
  uint8 retValue = ZFailure;
  endPointDesc_t *epDesc;
  byte transId;
  afAddrType_t dstAddr;
  cId_t cId;
  byte txOpts, radius, srcEP;

    /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Destination address */
  dstAddr.addrMode = afAddr16Bit;
  dstAddr.addr.shortAddr = BUILD_UINT16(pBuf[0], pBuf[1]);
  pBuf += 2;

  /* Destination endpoint */
  dstAddr.endPoint = *pBuf++;

  /* Source endpoint */
  srcEP = *pBuf++;
  epDesc = afFindEndPointDesc( srcEP );

  /* ClusterId */
  cId = BUILD_UINT16(pBuf[0], pBuf[1]);
  pBuf +=2;

  /* TransId */
  transId = *pBuf++;

  /* TxOption */
  txOpts = *pBuf++;

  /* Radius */
  radius = *pBuf++;

  /* Length */
  tempLen = *pBuf++;

  if ( epDesc == NULL )
  {
    retValue = afStatus_INVALID_PARAMETER;
  }
  else
  {
    retValue = AF_DataRequest( &dstAddr, epDesc, cId, tempLen, pBuf, &transId, txOpts, radius );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_AF), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_AfDataConfirm
 *
 * @brief   Process
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
void MT_AfDataConfirm(afDataConfirm_t *pMsg)
{
  uint8 retArray[3];

  retArray[0] = pMsg->hdr.status;
  retArray[1] = pMsg->endpoint;
  retArray[2] = pMsg->transID;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_AF), MT_AF_DATA_CONFIRM, 3, retArray);
}

/***************************************************************************************************
 * @fn          MT_AfIncomingMsg
 *
 * @brief       Process the callback subscription for AF Incoming data.
 *
 * @param       pkt - Incoming AF data.
 *
 * @return      none
 ***************************************************************************************************/
void MT_AfIncomingMsg(afIncomingMSGPacket_t *pMsg)
{
  uint8 respLen;            /* Length of the whole response packet */
  uint8 dataLen;            /* Length of the data section in the response packet */
  uint8 *respPtr, *tempPtr;

  /* dataLen */
  dataLen = pMsg->cmd.DataLength;
  /* respLen */
  respLen = 17 + dataLen;

  /* Allocate memory for the response packet */
  respPtr = osal_mem_alloc(respLen);
  if (!respPtr)
  {
    return;
  }
  tempPtr = respPtr;

  /* Fill in the data */

  /* Group ID */
  *tempPtr++ = LO_UINT16(pMsg->groupId);
  *tempPtr++ = HI_UINT16(pMsg->groupId);

  /* Cluster ID */
  *tempPtr++ = LO_UINT16(pMsg->clusterId);
  *tempPtr++ = HI_UINT16(pMsg->clusterId);

  /* Source Address */
  *tempPtr++ = LO_UINT16(pMsg->srcAddr.addr.shortAddr);
  *tempPtr++ = HI_UINT16(pMsg->srcAddr.addr.shortAddr);

  /* Source EP */
  *tempPtr++ = pMsg->srcAddr.endPoint;

  /* Destination EP */
  *tempPtr++ = pMsg->endPoint;

  /* WasBroadCast */
  *tempPtr++ = pMsg->wasBroadcast;

  /* LinkQuality */
  *tempPtr++ = pMsg->LinkQuality;

  /* SecurityUse */
  *tempPtr++ = pMsg->SecurityUse;

  /* Timestamp */
  *tempPtr++ = BREAK_UINT32(pMsg->timestamp, 0);
  *tempPtr++ = BREAK_UINT32(pMsg->timestamp, 1);
  *tempPtr++ = BREAK_UINT32(pMsg->timestamp, 2);
  *tempPtr++ = BREAK_UINT32(pMsg->timestamp, 3);

  /* Transmit Sequence Number */
  *tempPtr++ = pMsg->cmd.TransSeqNumber;

  /* Data Length */
  *tempPtr++ = dataLen;

  /* Data */
  if (dataLen)
  {
    osal_memcpy(tempPtr, pMsg->cmd.Data, dataLen);
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_AF), MT_AF_INCOMING_MSG, respLen, respPtr );

  /* Free memory */
  osal_mem_free(respPtr);

}

/***************************************************************************************************
***************************************************************************************************/
