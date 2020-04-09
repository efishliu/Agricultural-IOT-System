/**************************************************************************************************
  Filename:       se.h
  Revised:        $Date: 2007-11-16 10:54:05 -0800 (Fri, 16 Nov 2007) $
  Revision:       $Revision: 15941 $

  Description:    This file contains the Advanced Metering Initiative (SE)
                  Profile definitions.


  Copyright 2006-2007 Texas Instruments Incorporated. All rights reserved.

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

#ifndef SE_H
#define SE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */
// Zigbee SE Profile Identification
#define ZCL_SE_PROFILE_ID                                      0x0109

// Generic Device IDs
#define ZCL_SE_DEVICEID_RANGE_EXTENDER                         0x0008

// SE Device IDs
#define ZCL_SE_DEVICEID_ESP                                    0x0500
#define ZCL_SE_DEVICEID_METER                                  0x0501
#define ZCL_SE_DEVICEID_IN_PREMISE_DISPLAY                     0x0502
#define ZCL_SE_DEVICEID_PCT                                    0x0503
#define ZCL_SE_DEVICEID_LOAD_CTRL_EXTENSION                    0x0504
#define ZCL_SE_DEVICEID_SMART_APPLIANCE                        0x0505
#define ZCL_SE_DEVICEID_PREPAY_TERMINAL                        0x0506

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * FUNCTIONS
 */

 /*
  *  ZCL SE Profile initialization function
  */
extern void zclSE_Init( SimpleDescriptionFormat_t *simpleDesc );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_SE_H */
  

