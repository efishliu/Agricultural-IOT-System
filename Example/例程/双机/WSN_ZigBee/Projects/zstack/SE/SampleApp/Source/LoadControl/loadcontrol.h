/**************************************************************************************************
  Filename:       loadcontrol.h
  Revised:        $Date: 2007-07-20 11:28:20 -0700 (Fri, 20 Jul 2007) $
  Revision:       $Revision: 14957 $

  Description:    Header file for the Load Control Device functionality


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

#ifndef LOADCONTROL_H
#define LOADCONTROL_H

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
#define LOADCONTROL_ENDPOINT                 0x09

#define LOADCONTROL_MAX_ATTRIBUTES           16
#define LOADCONTROL_MAX_OPTIONS              2

#define LOADCONTROL_UPDATE_TIME_PERIOD       1000       // Update time event in seconds
#define LOADCONTROL_LOAD_CTRL_PERIOD         60000      // Load control event is 1 min long, this number is in millisecs
#define LOADCONTROL_EVENT_ID                 0x12345678 // Event ID for the load control event
#define START_TIME_NOW                       0x00000000 // Start time of NOW
#define HVAC_DEVICE_CLASS                    0x000001   // HVAC compressor or furnace - bit 0 is set
#define ONOFF_LOAD_DEVICE_CLASS              0x000080   // simple misc residential on/off loads - bit 7 is set

// Application Events
#define LOADCONTROL_IDENTIFY_TIMEOUT_EVT           0x0001
#define LOADCONTROL_UPDATE_TIME_EVT                0x0002
#define LOADCONTROL_KEY_ESTABLISHMENT_REQUEST_EVT  0x0004
#define LOADCONTROL_LOAD_CTRL_EVT                  0x0008

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */



/*********************************************************************
 * VARIABLES
 */
extern SimpleDescriptionFormat_t loadControlSimpleDesc;
extern CONST zclAttrRec_t loadControlAttrs[];
extern zclOptionRec_t loadControlOptions[];
extern uint8 loadControlDeviceEnabled;
extern uint16 loadControlTransitionTime;
extern uint16 loadControlIdentifyTime;
extern uint32 loadControlTime;
extern uint8 loadControlSignature[];
/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Initialization for the task
  */
extern void loadcontrol_Init( uint8 task_id );

/*
 *  Event Process for the task
 */
extern uint16 loadcontrol_event_loop( uint8 task_id, uint16 events );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* LOADCONTROL_H */
