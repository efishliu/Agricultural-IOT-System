/**************************************************************************************************
  Filename:       mac_low_level.h
  Revised:        $Date: 2007-04-24 11:06:07 -0700 (Tue, 24 Apr 2007) $
  Revision:       $Revision: 14106 $

  Description:    Describe the purpose and contents of the file.


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

#ifndef MAC_LOW_LEVEL_H
#define MAC_LOW_LEVEL_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"
#include "mac_high_level.h"


 /* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
/* identifies low-level code as specific to Chipcon SmartRF03(tm) technology */
#define MAC_LOW_LEVEL_SMARTRF04

#define MAC_BACKOFF_TIMER_DEFAULT_ROLLOVER  (((uint32) MAC_A_BASE_SUPERFRAME_DURATION) << 14)

/* macTxFrame() parameter values for txType */
#define MAC_TX_TYPE_SLOTTED_CSMA            0x00
#define MAC_TX_TYPE_UNSLOTTED_CSMA          0x01
#define MAC_TX_TYPE_SLOTTED                 0x02

/* macSleep() parameter values for sleepState */
#define MAC_SLEEP_STATE_OSC_OFF             0x01
#define MAC_SLEEP_STATE_RADIO_OFF           0x02

/* macRxPromiscuousMode() parameter values */
#define MAC_PROMISCUOUS_MODE_OFF            0x00  /* must be zero; reserved for boolean use */
#define MAC_PROMISCUOUS_MODE_COMPLIANT      0x01
#define MAC_PROMISCUOUS_MODE_WITH_BAD_CRC   0x02


/* ------------------------------------------------------------------------------------------------
 *                                           Global Externs
 * ------------------------------------------------------------------------------------------------
 */
extern uint8 const macTxSlottedDelay;

/* beacon interval margin */
extern uint16 macBeaconMargin[];


/* ------------------------------------------------------------------------------------------------
 *                                           Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/* mac_low_level.c */
void macLowLevelInit(void);
void macLowLevelReset(void);

/* mac_sleep.c */
void macSleepWakeUp(void);
uint8 macSleep(uint8 sleepState);

/* mac_radio.c */
uint8 macRadioRandomByte(void);
void macRadioSetPanCoordinator(uint8 panCoordinator);
void macRadioSetPanID(uint16 panID);
void macRadioSetShortAddr(uint16 shortAddr);
void macRadioSetIEEEAddr(uint8 * pIEEEAddr);
void macRadioSetTxPower(uint8 txPower);
void macRadioSetChannel(uint8 channel);
void macRadioStartScan(uint8 scanType);
void macRadioStopScan(void);
void macRadioEnergyDetectStart(void);
uint8 macRadioEnergyDetectStop(void);

/* mac_backoff_timer.c */
void macBackoffTimerSetRollover(uint32 rolloverBackoff);
void macBackoffTimerSetCount(uint32 backoff);
uint32 macBackoffTimerCount(void);
uint32 macBackoffTimerGetTrigger(void);
void macBackoffTimerSetTrigger(uint32 triggerBackoff);
void macBackoffTimerCancelTrigger(void);
void macBackoffTimerTriggerCallback(void);
void macBackoffTimerRolloverCallback(void);
int32 macBackoffTimerRealign(macRx_t *pMsg);

/* mac_tx.c */
void macTxFrame(uint8 txType);
void macTxFrameRetransmit(void);
void macTxCompleteCallback(uint8 status);

/* mac_rx.c */
bool macRxCheckPendingCallback(void);
bool macRxCheckMACPendingCallback(void);
void macRxCompleteCallback(macRx_t * pMsg);
void macRxPromiscuousMode(uint8 mode);

/* mac_rx_onoff.c */
void macRxEnable(uint8 flags);
void macRxSoftEnable(uint8 flags);
void macRxDisable(uint8 flags);
void macRxHardDisable(void);

/**************************************************************************************************
 */
#endif
