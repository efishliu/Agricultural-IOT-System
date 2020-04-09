/**************************************************************************************************
  Filename:       ipd.h
  Revised:        $Date: 2007-07-20 11:28:20 -0700 (Fri, 20 Jul 2007) $
  Revision:       $Revision: 14957 $

  Description:    Header file for the IPD functionality


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

#ifndef IPD_H
#define IPD_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"

/*********************************************************************
 * CONSTANTS
 */
#define IPD_ENDPOINT                 0x09

#define IPD_MAX_ATTRIBUTES           13

#define IPD_MAX_OPTIONS              3

#define IPD_UPDATE_TIME_PERIOD       1000   // Update time event in seconds
#define IPD_GET_PRICING_INFO_PERIOD  5000   // Interval for get pricing info command
#define SE_DEVICE_POLL_RATE          8000   // Poll rate for SE end device

  // Application Events
#define IPD_IDENTIFY_TIMEOUT_EVT           0x0001
#define IPD_UPDATE_TIME_EVT                0x0002
#define IPD_KEY_ESTABLISHMENT_REQUEST_EVT  0x0004
#define IPD_GET_PRICING_INFO_EVT           0x0008

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */



/*********************************************************************
 * VARIABLES
 */
extern SimpleDescriptionFormat_t ipdSimpleDesc;
extern CONST zclAttrRec_t ipdAttrs[];
extern zclOptionRec_t ipdOptions[];
extern uint8 ipdDeviceEnabled;
extern uint16 ipdTransitionTime;
extern uint16 ipdIdentifyTime;
extern uint32 ipdTime;
/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Initialization for the task
  */
extern void ipd_Init( uint8 task_id );

/*
 *  Event Process for the task
 */
extern uint16 ipd_event_loop( uint8 task_id, uint16 events );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* IPD_H */
