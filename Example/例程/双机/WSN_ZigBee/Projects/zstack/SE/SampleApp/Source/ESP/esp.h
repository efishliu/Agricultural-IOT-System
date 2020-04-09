/**************************************************************************************************
  Filename:       esp.h
  Revised:        $Date: 2007-07-20 11:28:20 -0700 (Fri, 20 Jul 2007) $
  Revision:       $Revision: 14957 $

  Description:    Header file for the ESP functionality


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

#ifndef ESP_H
#define ESP_H

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
#define ESP_ENDPOINT                 0x09

#define ESP_MAX_ATTRIBUTES           53

#define ESP_MAX_OPTIONS              8

#define ESP_UPDATE_TIME_PERIOD       1000     // Update time event in seconds

#define HVAC_DEVICE_CLASS            0x000001 // HVAC compressor or furnace - bit 0 is set
#define ONOFF_LOAD_DEVICE_CLASS      0x000080 // simple misc residential on/off loads - bit 7 is set

// Application Events
#define ESP_IDENTIFY_TIMEOUT_EVT     0x0001
#define ESP_UPDATE_TIME_EVT          0x0002
#define SIMPLE_DESC_QUERY_EVT_1      0x0004
#define SIMPLE_DESC_QUERY_EVT_2      0x0008

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */



/*********************************************************************
 * VARIABLES
 */
extern SimpleDescriptionFormat_t espSimpleDesc;
extern CONST zclAttrRec_t espAttrs[];
extern zclOptionRec_t espOptions[];
extern uint8 espDeviceEnabled;
extern uint16 espTransitionTime;
extern uint16 espIdentifyTime;
extern uint32 espTime;
extern uint8 espSignature[];
/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Initialization for the task
  */
extern void esp_Init( uint8 task_id );
/*
 *  Event Process for the task
 */
extern uint16 esp_event_loop( uint8 task_id, uint16 events );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* esp_H */
