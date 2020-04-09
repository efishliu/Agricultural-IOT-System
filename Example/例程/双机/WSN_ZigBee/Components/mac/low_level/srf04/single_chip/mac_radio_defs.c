/**************************************************************************************************
  Filename:       mac_radio_defs.c
  Revised:        $Date: 2009-03-27 14:32:42 -0700 (Fri, 27 Mar 2009) $
  Revision:       $Revision: 19584 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2006-2009 Texas Instruments Incorporated. All rights reserved.

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

/* ------------------------------------------------------------------------------------------------
 *                                             Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mac_radio_defs.h"
#include "hal_types.h"
#include "hal_assert.h"
#include "mac_pib.h"


/* ------------------------------------------------------------------------------------------------
 *                                        Global Constants
 * ------------------------------------------------------------------------------------------------
 */
#if defined (HAL_PA_LNA)
const uint8 CODE macRadioDefsTxPowerTable[] =
{
  /*   0 dBm */   0xD5,   /* TBD: place holder */
  /*   1 dBm */   0xD5,   /* TBD: place holder */
  /*   2 dBm */   0xD5,   /* TBD: place holder */
  /*   3 dBm */   0xD5,   /* TBD: place holder */
  /*   4 dBm */   0xD5,   /* TBD: place holder */
  /*   5 dBm */   0xD5,   /* TBD: place holder */
  /*   6 dBm */   0xD5,   /* TBD: place holder */
  /*   7 dBm */   0xD5,   /* TBD: place holder */
  /*   8 dBm */   0xD5,   /* TBD: place holder */
  /*   9 dBm */   0xD5,   /* TBD: place holder */
  /*  10 dBm */   0xD5,   /* TBD: place holder */
  /*  11 dBm */   0xD5,   /* TBD: place holder */
  /*  12 dBm */   0xD5,   /* TBD: place holder */
  /*  13 dBm */   0xD5,   /* TBD: place holder */
  /*  14 dBm */   0xD5,   /* TBD: place holder */
  /*  15 dBm */   0xD5,   /* TBD: place holder */
  /*  16 dBm */   0xD5,   /* TBD: place holder */
  /*  17 dBm */   0xD5,   /* TBD: place holder */
  /*  18 dBm */   0xD5,   /* TBD: place holder */
  /*  19 dBm */   0xD5    /* TBD: place holder */
};
#elif defined (HAL_PA_LNA_CC2590)
const uint8 CODE macRadioDefsTxPowerTable[] =
{
  /*   0 dBm */   0xD5,   /* TBD: place holder */
  /*   1 dBm */   0xD5,   /* TBD: place holder */
  /*   2 dBm */   0xD5,   /* TBD: place holder */
  /*   3 dBm */   0xD5,   /* TBD: place holder */
  /*   4 dBm */   0xD5,   /* TBD: place holder */
  /*   5 dBm */   0xD5,   /* TBD: place holder */
  /*   6 dBm */   0xD5,   /* TBD: place holder */
  /*   7 dBm */   0xD5,   /* TBD: place holder */
  /*   8 dBm */   0xD5,   /* TBD: place holder */
  /*   9 dBm */   0xD5,   /* TBD: place holder */
  /*  10 dBm */   0xD5,   /* TBD: place holder */
  /*  11 dBm */   0xD5    /* TBD: place holder */
};
#else
/* values based on testing PG2.1 */
const uint8 CODE macRadioDefsTxPowerTable[] =
{
  /*   0 dBm */   0xD5,   /* characterized as  0  dBm in datasheet */
  /*  -1 dBm */   0xC5,   /* characterized as -1  dBm in datasheet */
  /*  -2 dBm */   0xC5,
  /*  -3 dBm */   0xB5,   /* characterized as -3  dBm in datasheet */
  /*  -4 dBm */   0xA5,   /* characterized as -4  dBm in datasheet */
  /*  -5 dBm */   0xA5,
  /*  -6 dBm */   0x95,   /* characterized as -6  dBm in datasheet */
  /*  -7 dBm */   0x85,   /* characterized as -7  dBm in datasheet */
  /*  -8 dBm */   0x85,
  /*  -9 dBm */   0x75,   /* characterized as -9  dBm in datasheet */
  /* -10 dBm */   0x75,
  /* -11 dBm */   0x65,   /* characterized as -11 dBm in datasheet */
  /* -12 dBm */   0x65,
  /* -13 dBm */   0x55,   /* characterized as -13 dBm in datasheet */
  /* -14 dBm */   0x55,
  /* -15 dBm */   0x45,   /* characterized as -15 dBm in datasheet */
  /* -16 dBm */   0x45,
  /* -17 dBm */   0x35,   /* characterized as -17 dBm in datasheet */
  /* -18 dBm */   0x35,
  /* -19 dBm */   0x25,   /* characterized as -19 dBm in datasheet */
  /* -20 dBm */   0x25,
  /* -21 dBm */   0x15,   /* characterized as -21 dBm in datasheet */
  /* -22 dBm */   0x05    /* characterized as -23 dBm in datasheet */
};
#endif


/**************************************************************************************************
 * @fn          macRadioTurnOnPower
 *
 * @brief       Logic and sequence for powering up the radio.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioTurnOnPower(void)
{
  /* Enable RF error trap */
  MAC_MCU_RFERR_ENABLE_INTERRUPT();

#if defined (HAL_PA_LNA) || defined (HAL_PA_LNA_CC2590)
  /* AGCCTRL1 for CC2590 or CC2591 */
  AGCCTRL1 = 0x16;

  /* CC2591 PA/LNA control configuration
   *   P0_7 -> HGM
   *   P1_1 -> PA_EN
   *   P1_4 -> EN
   */

  /* P0_7 */
  HAL_PA_LNA_RX_HGM();

  /* P1_1 */
  RFC_OBS_CTRL0 = RFC_OBS_CTRL_PA_PD_INV;
  OBSSEL1       = OBSSEL1_OBS_CTRL0;

  /* P1_4 */
  RFC_OBS_CTRL1 = RFC_OBS_CTRL_LNAMIX_PD_INV;
  OBSSEL4       = OBSSEL4_OBS_CTRL4;
#endif /* defined (HAL_PA_LNA) || defined (HAL_PA_LNA_CC2590) */

  if (macChipVersion <= REV_B)
  {
    /* radio initializations for disappearing RAM; PG1.0 and before only */
    MAC_RADIO_SET_PAN_ID(macPib.panId);
    MAC_RADIO_SET_SHORT_ADDR(macPib.shortAddress);
    MAC_RADIO_SET_IEEE_ADDR(macPib.extendedAddress.addr.extAddr);
  }
}


/**************************************************************************************************
 * @fn          macRadioTurnOffPower
 *
 * @brief       Logic and sequence for powering down the radio.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioTurnOffPower(void)
{
  /* Disable RF error trap */
  MAC_MCU_RFERR_DISABLE_INTERRUPT();

#if defined (HAL_PA_LNA) || defined (HAL_PA_LNA_CC2590)
  /* Make sure the HGM pin is low before entering sleep */
  HAL_PA_LNA_RX_LGM();
#endif
}


/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */
HAL_ASSERT_SIZE(macRadioDefsTxPowerTable, MAC_RADIO_TX_POWER_MAX_DBM+1);  /* array size mismatch */

#if (HAL_CPU_CLOCK_MHZ != 32)
#error "ERROR: The only tested/supported clock speed is 32 MHz."
#endif

#if (MAC_RADIO_RECEIVER_SENSITIVITY_DBM > MAC_SPEC_MIN_RECEIVER_SENSITIVITY)
#error "ERROR: Radio sensitivity does not meet specification."
#endif

#if defined (HAL_PA_LNA) && defined (HAL_PA_LNA_CC2590)
#error "ERROR: HAL_PA_LNA and HAL_PA_LNA_CC2590 cannot be both defined."
#endif

/**************************************************************************************************
 */
