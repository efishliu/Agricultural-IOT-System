/**************************************************************************************************
  Filename:       SerialApp.c
  Revised:        $Date: 2009-03-29 10:51:47 -0700 (Sun, 29 Mar 2009) $
  Revision:       $Revision: 19585 $

  Description -   Serial Transfer Application (no Profile).


  Copyright 2004-2009 Texas Instruments Incorporated. All rights reserved.

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
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT STOUCH
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

#include "AF.h"
#include "OnBoard.h"
#include "OSAL_Tasks.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "hal_drivers.h"
#include "hal_key.h"
#if defined ( LCD_SUPPORTED )
  #include "hal_lcd.h"
#endif
#include "hal_led.h"
#include "hal_uart.h"
#include "Public.h"
#include "TempAndHum.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#if !defined( SERIAL_APP_PORT )
#define SERIAL_APP_PORT  0
#endif

#if !defined( SERIAL_APP_BAUD )
#define SERIAL_APP_BAUD  HAL_UART_BR_115200
#endif

// When the Rx buf space is less than this threshold, invoke the Rx callback.
#if !defined( SERIAL_APP_THRESH )
#define SERIAL_APP_THRESH  64
#endif

#if !defined( SERIAL_APP_RX_SZ )
#define SERIAL_APP_RX_SZ  128
#endif

#if !defined( SERIAL_APP_TX_SZ )
#define SERIAL_APP_TX_SZ  128
#endif

// Millisecs of idle time after a byte is received before invoking Rx callback.
#if !defined( SERIAL_APP_IDLE )
#define SERIAL_APP_IDLE  6
#endif

// Loopback Rx bytes to Tx for throughput testing.
#if !defined( SERIAL_APP_LOOPBACK )
#define SERIAL_APP_LOOPBACK  FALSE
#endif

// This is the max byte count per OTA message.
#if !defined( SERIAL_APP_TX_MAX )
#define SERIAL_APP_TX_MAX  20
#endif

#define SERIAL_APP_RSP_CNT  4

// This list should be filled with Application specific Cluster IDs.
const cId_t SerialApp_ClusterList[SERIALAPP_MAX_CLUSTERS] =
{
  SERIALAPP_CLUSTERID1,
  SERIALAPP_CLUSTERID2
};

const SimpleDescriptionFormat_t SerialApp_SimpleDesc =
{
  SERIALAPP_ENDPOINT,              //  int   Endpoint;
  SERIALAPP_PROFID,                //  uint16 AppProfId[2];
  SERIALAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  SERIALAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  SERIALAPP_FLAGS,                 //  int   AppFlags:4;
  SERIALAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)SerialApp_ClusterList,  //  byte *pAppInClusterList;
  SERIALAPP_MAX_CLUSTERS,          //  byte  AppNumOutClusters;
  (cId_t *)SerialApp_ClusterList   //  byte *pAppOutClusterList;
};

const endPointDesc_t SerialApp_epDesc =
{
  SERIALAPP_ENDPOINT,
 &SerialApp_TaskID,
  (SimpleDescriptionFormat_t *)&SerialApp_SimpleDesc,
  noLatencyReqs
};

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

uint8 SerialApp_TaskID;    // Task ID for internal task/event processing.
devStates_t  SampleApp_NwkState;
static UART_Format UART0_Format;
static uint16 Temperature;
static uint16 Humidity;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8 SerialApp_MsgID;
static afAddrType_t SerialApp_TxAddr;
static uint8 SerialApp_TxBuf[SERIAL_APP_TX_MAX];
static uint8 SerialApp_TxLen;
/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt );
void SerialApp_OTAData(afAddrType_t *txaddr,uint8 ID,void *p,uint8 len);
static void SerialApp_CallBack(uint8 port, uint8 event);
static void GPIOInit(void);
static uint16 ReadSHT10(uint8 param);

static void GPIOInit(void) 
{
  P0SEL &= ~(1<<6);
  P0DIR |= (1<<6);
  P0SEL &= ~(1<<7);
  P0DIR |= (1<<7);
}
/*********************************************************************
 * @fn      SerialApp_Init
 *
 * @brief   This is called during OSAL tasks' initialization.
 *
 * @param   task_id - the Task ID assigned by OSAL.
 *
 * @return  none
 */
void SerialApp_Init( uint8 task_id )
{
  halUARTCfg_t uartConfig;

  SerialApp_TaskID = task_id;

  afRegister( (endPointDesc_t *)&SerialApp_epDesc );

  RegisterForKeys( task_id );
  GPIOInit();

  uartConfig.configured           = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.baudRate             = SERIAL_APP_BAUD;
  uartConfig.flowControl          = FALSE;
  uartConfig.flowControlThreshold = SERIAL_APP_THRESH; // 2x30 don't care - see uart driver.
  uartConfig.rx.maxBufSize        = SERIAL_APP_RX_SZ;  // 2x30 don't care - see uart driver.
  uartConfig.tx.maxBufSize        = SERIAL_APP_TX_SZ;  // 2x30 don't care - see uart driver.
  uartConfig.idleTimeout          = SERIAL_APP_IDLE;   // 2x30 don't care - see uart driver.
  uartConfig.intEnable            = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.callBackFunc         = SerialApp_CallBack;
  HalUARTOpen (SERIAL_APP_PORT, &uartConfig);
  
  UART0_Format.Header_1 = 0xee;
  UART0_Format.Header_2 = 0xcc;
  UART0_Format.NodeSeq  = 0x01;
  UART0_Format.NodeID   = TempAndHum;
  UART0_Format.Tailer   = 0xff;
  
  SerialApp_TxAddr.addrMode =(afAddrMode_t)Addr16Bit;//发送地址初始化
  SerialApp_TxAddr.endPoint = SERIALAPP_ENDPOINT;
  SerialApp_TxAddr.addr.shortAddr = 0x0000;
  TXPOWER = 0xf5;
}

/*********************************************************************
 * @fn      SerialApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events   - Bit map of events to process.
 *
 * @return  Event flags of all unprocessed events.
 */
UINT16 SerialApp_ProcessEvent( uint8 task_id, UINT16 events )
{
  (void)task_id;  // Intentionally unreferenced parameter
  
  if ( events & SYS_EVENT_MSG )
  {
    afIncomingMSGPacket_t *MSGpkt;

    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SerialApp_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {         
      case KEY_CHANGE:
        //SerialApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
        break;

      case AF_INCOMING_MSG_CMD:
        SerialApp_ProcessMSGCmd( MSGpkt );
        break;

      case ZDO_STATE_CHANGE:
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if(SampleApp_NwkState == DEV_END_DEVICE) //判定当前设备类型
          {
            HalLedBlink(HAL_LED_1,1,50,500);
            HalLedBlink(HAL_LED_2,1,50,500);
            osal_start_timerEx(SerialApp_TaskID, PERIOD_EVT, 1000); //启动周期消息
            osal_set_event(SerialApp_TaskID, TEMPANDHUM_READ_EVT); 
          }
        break;
      default:
        break;
      }

      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    return ( events ^ SYS_EVENT_MSG );
  }
  
  if ( events & PERIOD_EVT ) //周期消息处理
  {
    UART0_Format.Command = MSG_PERIOD;
    UART0_Format.Data[0] = 0x00;
    UART0_Format.Data[1] = 0x00; 
    UART0_Format.Data[2] = 0x00;
    UART0_Format.Data[3] = 0x00; 
    SerialApp_OTAData(&SerialApp_TxAddr, SERIALAPP_CLUSTERID1, &UART0_Format, sizeof(UART_Format));
    osal_start_timerEx(SerialApp_TaskID, PERIOD_EVT, 5000);
    return ( events ^ PERIOD_EVT );
  }
  
  if ( events & SERIALAPP_SEND_EVT ) //发送RF消息
  {
    SerialApp_OTAData(&SerialApp_TxAddr,SERIALAPP_CLUSTERID1, &UART0_Format, sizeof(UART_Format));
    return ( events ^ SERIALAPP_SEND_EVT );
  }

  if ( events & TEMPANDHUM_READ_EVT )  //读取温湿度
  {
    Temperature = ReadSHT10(TEMPERATURE);
    UART0_Format.Command = 0x01;
    UART0_Format.Data[0] = Temperature>>8;
    UART0_Format.Data[1] = Temperature;
    Humidity = ReadSHT10(HUMIDITY);
    UART0_Format.Data[2] = Humidity>>8;
    UART0_Format.Data[3] = Humidity;      
    osal_set_event(SerialApp_TaskID,SERIALAPP_SEND_EVT);
    osal_start_timerEx(SerialApp_TaskID, TEMPANDHUM_READ_EVT, 1000);
    
    return ( events ^ TEMPANDHUM_READ_EVT );
  }

  return ( 0 );  // Discard unknown events.
}

/*********************************************************************
 * @fn      SerialApp_ProcessMSGCmd
 *
 * @brief   Data message processor callback. This function processes
 *          any incoming data - probably from other devices. Based
 *          on the cluster ID, perform the intended action.
 *
 * @param   pkt - pointer to the incoming message packet
 *
 * @return  TRUE if the 'pkt' parameter is being used and will be freed later,
 *          FALSE otherwise.
 */
void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt )  //处理接收到的RF消息
{
  static UART_Format *receiveData;
  switch ( pkt->clusterId )
  {
   case SERIALAPP_CLUSTERID1:  //处理各个传感器节数据    
     receiveData = (UART_Format *)(pkt->cmd.Data);
     HalLedBlink(HAL_LED_1,1,50,200);
     if((receiveData->Header_1==0xcc)&&(receiveData->Header_2==0xee)&&(receiveData->Tailer==0xff)) //校验包头包尾
     {
       if(receiveData->NodeID == PhotoRes) //地址
       {
         
       }
     }
    break;

  case SERIALAPP_CLUSTERID2:
    break;

    default:
      break;
  }
}

/*********************************************************************
 */

void SerialApp_OTAData(afAddrType_t *txaddr, uint8 cID, void *p, uint8 len) //发送函数
{
  if (afStatus_SUCCESS != AF_DataRequest(txaddr, //发送地址
                                           (endPointDesc_t *)&SerialApp_epDesc, //endpoint描述
                                            cID, //clusterID
                                            len, p, //发送数据包的长度和地址
                                            &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS))
  {
  }
  else
  {
    HalLedBlink(HAL_LED_1,1,50,200);
  }
}

/*********************************************************************
 * @fn      SerialApp_CallBack
 *
 * @brief   Send data OTA.
 *
 * @param   port - UART port.
 * @param   event - the UART port event flag.
 *
 * @return  none
 */
static void SerialApp_CallBack(uint8 port, uint8 event)
{
  (void)port;

  if ((event & (HAL_UART_RX_FULL | HAL_UART_RX_ABOUT_FULL | HAL_UART_RX_TIMEOUT)) && !SerialApp_TxLen) //串口接收到数据包
  {
    SerialApp_TxLen = HalUARTRead(SERIAL_APP_PORT, SerialApp_TxBuf, SERIAL_APP_TX_MAX); //将串口数据读入buf
    SerialApp_TxLen = 0;  
  }
}

/**************************************************************
函数功能描述：读取SHT10温湿度数据
入口参数：0-温度  1-湿度
返回：    测量结果    格式：xx.xx
***************************************************************/
uint16 ReadSHT10(uint8 param)
{
  double temp;
  uint8  i;
  uint16 result;
  uint16 SORH = 0;
  DATA_OUTPUT;
  DATA_HIGH;
  SCK_OUTPUT;
  SCK_LOW;

  //通讯复位
  for( i=0; i<10; i++ )
  {
    SCK_HIGH;
    MicroWait(30);
    SCK_LOW;
    MicroWait(30);
  }
 /////////////////////////
  SCK_HIGH;
  MicroWait(15);
  DATA_LOW;
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);
  SCK_HIGH;
  MicroWait(15);
  DATA_HIGH;
  MicroWait(15);
  SCK_LOW;

  //发送命令字：00000101
  MicroWait(15);
  DATA_LOW;
  MicroWait(15);

  //1
  SCK_HIGH;
  MicroWait(30);
  SCK_LOW;
  MicroWait(30);

  //2
  SCK_HIGH;
  MicroWait(30);
  SCK_LOW;
  MicroWait(30);

  //3
  SCK_HIGH;
  MicroWait(30);
  SCK_LOW;
  MicroWait(30);

  //4
  SCK_HIGH;
  MicroWait(30);
  SCK_LOW;
  MicroWait(30);
  
  if(param == HUMIDITY)
  {
    SCK_HIGH;
    MicroWait(30);
    SCK_LOW;
    MicroWait(15);
    DATA_HIGH;
    MicroWait(15);

    SCK_HIGH;
    MicroWait(30);
    SCK_LOW;
    MicroWait(15);
    DATA_LOW;
    MicroWait(15);

    SCK_HIGH;
    MicroWait(30);

    SCK_LOW;
    MicroWait(15);
    DATA_HIGH;
    MicroWait(15);

    SCK_HIGH;
    MicroWait(30);

    DATA_INPUT;

    SCK_LOW;
    MicroWait(30);
  }
  else if(param == TEMPERATURE)
  {
    SCK_HIGH;
    MicroWait(30);
    SCK_LOW;
    MicroWait(30);
    SCK_HIGH;
    MicroWait(30);
    SCK_LOW;
    MicroWait(15);
    DATA_HIGH;
    MicroWait(15);

    SCK_HIGH;
    MicroWait(30);
    SCK_LOW;
    MicroWait(30);

    SCK_HIGH;
    MicroWait(30);

    DATA_INPUT;

    SCK_LOW;
    MicroWait(30);
  }
  else
  {
    return 0;
  }

  SCK_HIGH;
  MicroWait(30);
  SCK_LOW;
  MicroWait(30);

  //等待测量结束
  while(SHT10_DATA);

  //读取三个字节数据
  MicroWait(15);

  //高二/四位无效
  SCK_HIGH;
  MicroWait(30);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(30);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  if(param == TEMPERATURE)
  {
    SORH |= (P0_7<<13);
  }
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  if(param == TEMPERATURE)
  {
  SORH |= (P0_7<<12);
  }
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  SORH |= (P0_7<<11);
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  SORH |= (P0_7<<10);
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  SORH |= (P0_7<<9);
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  SORH |= (P0_7<<8);
  MicroWait(15);
  SCK_LOW;
  MicroWait(15);

  //发ACK
  DATA_OUTPUT;
  DATA_LOW;
  MicroWait(15);

  SCK_HIGH;
  MicroWait(30);
  SCK_LOW;
  MicroWait(15);

  DATA_INPUT;
  MicroWait(15);

  //低8数据位
  SCK_HIGH;
  MicroWait(15);
  SORH |= (P0_7<<7);
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  SORH |= (P0_7<<6);
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  SORH |= (P0_7<<5);
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  SORH |= (P0_7<<4);
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  SORH |= (P0_7<<3);
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  SORH |= (P0_7<<2);
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  SORH |= (P0_7<<1);
  MicroWait(15);
  SCK_LOW;
  MicroWait(30);

  SCK_HIGH;
  MicroWait(15);
  SORH |= (P0_7<<0);
  MicroWait(15);
  SCK_LOW;
  MicroWait(15);

  DATA_OUTPUT;
  DATA_LOW;
  MicroWait(15);

  SCK_HIGH;
  MicroWait(30);

  SCK_LOW;
  MicroWait(15);

  DATA_HIGH;
  MicroWait(15);

  if(param == TEMPERATURE)
  {
    temp = 0.01*SORH - 39.635;
    result = (uint16)(temp*100);
  }
  else if(param == HUMIDITY)
  {
    temp = (-2.8)*0.000001*SORH*SORH;
    temp = temp + 0.0405*SORH - 4;
    result = (uint16)(temp*100);
  }
  else
  {
    return 0;
  }
  return result;
}
/*********************************************************************
*********************************************************************/
