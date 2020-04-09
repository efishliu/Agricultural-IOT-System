/**************************************************************************************************
  Filename:       _hal_uart_spi.c
  Revised:        $Date: 2010-03-10 20:36:55 -0800 (Wed, 10 Mar 2010) $
  Revision:       $Revision: 21890 $

  Description: This file contains the interface to the H/W UART driver by SPI, by ISR.
               Note that this is a trivial implementation only to support the boot loader and
               is not fit for a general SPI communication protocol which would have to be more
               sophisticated like the RPC for ZNP.


  Copyright 2010 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
 * INCLUDES
 */

#include "hal_types.h"
#include "hal_board.h"
#include "hal_defs.h"
#include "hal_mcu.h"
#include "hal_uart.h"
#include "SPI.h"
#include "OnBoard.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static uartSPICfg_t spiCfg;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

void HalUARTInitSPI(void);
int8  SPIOneByte(int8 x);

int8 SPIOneByte(int8 sendbyte)
{
  char i,temp;
  for(i=0;i<8;i++)
  {  
    sck=0;
    if(sendbyte & 0x80)                //位运算，判断最高位是否为1
    { 
      si=1;
    }
    else
    { 
      si=0;
    }
    sendbyte <<= 1;    
    sck=1;
    temp <<= 1;
    if(so)
      temp |= 0x01;
  }
  sck=0;
  MicroWait(1);
  si=0;
  return (temp);
}

/******************************************************************************
 * @fn      HalUARTInitSPI
 *
 * @brief   Initialize the UART
 *
 * @param   none
 *
 * @return  none
 *****************************************************************************/
void HalUARTInitSPI(void)
{
  //P17输入-MISO，P16输出-MOSI，P15输出-SCK,P14输出-CS,P12输出-RST  0111 X1XX
  P1DIR |= 0x70;
} 

/*****************************************************************************
 * @fn      HalUARTReadSPI
 *
 * @brief   Execute a blocking read from master if Rx buffer is empty. Otherwise return up to 'len'
 *          bytes of read data.
 *
 * @param   buf  - valid data buffer at least 'len' bytes in size
 *          len  - max length number of bytes to copy to 'buf'
 *
 * @return  length of buffer that was read
 *****************************************************************************/
//uint16 HalUARTReadSPI(uint8 *buf, uint16 len) //主机 SPI读数据
//{
//  return 0;
//}

/******************************************************************************
 * @fn      HalUARTWriteSPI
 *
 * @brief   Write a buffer to the UART.
 * @brief   Execute a blocking write to the 从机.
 *
 * @param   buf - pointer to the buffer that will be written, not freed
 *          len - length of
 *
 * @return  length of the buffer that was sent
 *****************************************************************************/
//uint16 HalUARTWriteSPI(uint8 *buf, uint16 len) //SPI口写数据
//{
//  return 0;
//}

/******************************************************************************
******************************************************************************/
