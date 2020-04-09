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
  Software unless you agree to abide by the  v  terms of the License. The License
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
#include "Coordinator.h"
#include "osal_clock.h"
#include "mac_mcu.h"
#include "gprs.h"
#include "GUI.h"
/*********************************************************************
wifi设置：网络名称：lipengjun  无密码
          IP地址：192.168.1.122
         子网掩码：255.155.255.0
         网关地址：192.168.1.1
              DNS: 192.168.1.1
         协议类型：TCP
         C/S模式：服务器
         端口号：9999

*******************************************************************/

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
static uint8         SerialApp_MsgID;
static afAddrType_t  SerialApp_TxAddr;
static uint8         SerialApp_TxBuf[SERIAL_APP_TX_MAX];
static uint8         SerialApp_TxLen;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt );
void SerialApp_OTAData(afAddrType_t *txaddr,uint8 ID,void *p,uint8 len);
static void SerialApp_CallBack(uint8 port, uint8 event);
void SerialApp_HandleKeys( uint8 shift, uint8 keys );

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
  //GPRS_Initial();
  RegisterForKeys( task_id );

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
  UART0_Format.NodeID   = Coor;
  UART0_Format.Tailer   = 0xff;
  
  SerialApp_TxAddr.addrMode =(afAddrMode_t) AddrBroadcast; //发送地址初始化
  SerialApp_TxAddr.endPoint = SERIALAPP_ENDPOINT;
  SerialApp_TxAddr.addr.shortAddr = 0xffff;
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
        SerialApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
        break;

      case AF_INCOMING_MSG_CMD:
        SerialApp_ProcessMSGCmd( MSGpkt );
        break;

      case ZDO_STATE_CHANGE:
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if(SampleApp_NwkState == DEV_ZB_COORD) //判定当前设备类型
          {
#if LCD_SUPPORTED
            LCD_Clear(GUI_BLACK);
            GUI_SetColor(GUI_RED);
            
            GUI_SetFont(&GUI_Font32B_ASCII);
            GUI_DispStringAt("ChinaSofti", 50, 0);
            GUI_SetFont(&GUI_Font8x16);
            
            //GUI_DispStringAt("www.ChinaSofti.com",40,0);
            GUI_DispStringAt("Coord,PANID =     ,CH =  ", 5, 40);
            GUI_DispHexAt(ZDAPP_CONFIG_PAN_ID, 110, 40, 4);
            GUI_DispHexAt(DEFAULT_CHANLIST, 190, 40, 6);
            GUI_SetColor(GUI_GREEN);
            
            GUI_DispStringAt("GYROS",0,60);   
            GUI_DispStringAt("TE&HU",0,80);        
            GUI_DispStringAt("ALCOH",0,100);
            GUI_DispStringAt("ACCEL",0,120);
            GUI_DispStringAt("FLAME",0,140);
            GUI_DispStringAt("PYROE",0,160);
            GUI_DispStringAt("RELAY",0,180);
            GUI_DispStringAt("TOUCH",0,200);
            GUI_DispStringAt("ULTRA            mm",0,200);
#endif
            HalLedSet(HAL_LED_1,HAL_LED_MODE_OFF);
            HalLedBlink(HAL_LED_2,5,50,200);
            osal_start_timerEx(SerialApp_TaskID, PERIOD_EVT, 5000);
          }
        break;
      default:
        break;
      }

      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    return ( events ^ SYS_EVENT_MSG );
  }
  
  if ( events & SERIALAPP_SEND_EVT )  //将串口数据通过RF消息发送
  {
    SerialApp_OTAData(&SerialApp_TxAddr,SERIALAPP_CLUSTERID1,SerialApp_TxBuf, sizeof(UART_Format));
    return ( events ^ SERIALAPP_SEND_EVT );
  }  
  
  if ( events & PERIOD_EVT ) //周期消息处理
  {
    UART0_Format.Command = MSG_PERIOD;
    HalUARTWrite(SERIAL_APP_PORT, (uint8*)(&UART0_Format), sizeof(UART_Format)); 
    osal_start_timerEx(SerialApp_TaskID, PERIOD_EVT, 5000);
    return ( events ^ PERIOD_EVT );
  }

  return ( 0 );  // Discard unknown events.
}

void SerialApp_HandleKeys( uint8 shift, uint8 keys )
{
    /*uint8 getup[]  = "ATA\r";
    uint8 setMsgModeTxt[]  = "AT+CMGF=1\r"; //发短信命令TXT
    uint8 setDstNum[]      = "AT+CMGS=\"+8618301009405\"\r";  //指定号码  注意“”的转义字符
    uint8 msg[] = "Hello";
    uint8 msgEnd[] = {0x1A};*/
    static UART_Format command;
    command.Header_1 = 0xcc;
    command.Header_2 = 0xee;
    command.NodeSeq = 0x01;
    command.NodeID = Relay;
    command.Tailer = 0xff;
    static uint8 relay_1_state = 0x02;  //2   off   1   on
    static uint8 relay_2_state = 0x02;
    if ( keys & HAL_KEY_LEFT )
    {
      if(relay_1_state == 0x01)
      {
        relay_1_state = 0x02;
        //LCD_ShowString(60,160,"Relay1_OFF");
      }
      else if(relay_1_state == 0x02)
      {
        relay_1_state = 0x01;
        //LCD_ShowString(60,160,"Relay1_ON ");
      }
      command.Command = relay_1_state;
      command.Data[0] = 0x01;  
      SerialApp_OTAData(&SerialApp_TxAddr, SERIALAPP_CLUSTERID1, (uint8 *)&command, sizeof(UART_Format));
    }
    if ( keys & HAL_KEY_RIGHT )
    {
      if(relay_2_state == 0x01)
      {
        relay_2_state = 0x02;
        //LCD_ShowString(150,160,"Relay2_OFF");
      }
      else if(relay_2_state == 0x02)
      {
        relay_2_state = 0x01;
        //LCD_ShowString(150,160,"Relay2_ON ");
      }
      command.Command = relay_2_state;
      command.Data[0] = 0x02;  
      SerialApp_OTAData(&SerialApp_TxAddr, SERIALAPP_CLUSTERID1, (uint8 *)&command, sizeof(UART_Format));
    }
    if(keys & HAL_KEY_CENTER)
    {
      
    }
    if(keys & HAL_NO_KEY)
    {

    }
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
  UART_Format *receiveData;
  receiveData = (UART_Format *)(pkt->cmd.Data);
  int8 receive_rssi;
  receive_rssi = pkt->rssi;
  switch ( pkt->clusterId )
  {
  case SERIALAPP_CLUSTERID1:  //处理各个传感器节数据
    if((receiveData->Header_1==0xee)&&(receiveData->Header_2==0xcc)&&(receiveData->Tailer==0xff)) //校验包头包尾
    {
      if(receiveData->Command == MSG_PERIOD) //如果是周期消息
      {
        receiveData->Data[0] = receive_rssi;
      }
      HalUARTWrite(SERIAL_APP_PORT, (uint8*)receiveData, sizeof(UART_Format)); //通过串口发送给网关
#ifdef LCD_SUPPORTED
      int16 temp1,temp2;
      if(receiveData->NodeID == Gyroscope) //如果是光敏传感器发来的消息
      {
        if(receiveData->Command == 0x01) //光敏采样值
        {
          temp1 = (receiveData->Data[0]<<8)|receiveData->Data[1]; //x
          GUI_DispDecAt(temp1, 60,  60, 5);
          temp1 = (receiveData->Data[2]<<8)|receiveData->Data[3]; //y
          GUI_DispDecAt(temp1, 120, 60, 5);
          temp1 = (receiveData->Data[4]<<8)|receiveData->Data[5]; //z
          GUI_DispDecAt(temp1, 180, 60, 5);
        }
      }
      else if(receiveData->NodeID == TempAndHum)
      {
        if(receiveData->Command == 0x01) //光敏采样值
        {
          temp1 = (receiveData->Data[0]<<8)|receiveData->Data[1];
          LCD_ShowNum(80,60,temp1,4);
          temp2 = (receiveData->Data[2]<<8)|receiveData->Data[3];
          LCD_ShowNum(170,60,temp2,4);
        }       
      }
      else if(receiveData->NodeID == Alcohol)
      {
        uint16 temp3;
        if(receiveData->Command == 0x01) //光敏采样值
        {
          temp3  = receiveData->Data[0]<<8|receiveData->Data[1];
          LCD_ShowNum(80, 80, temp3, 5);
        }       
      }
      else if(receiveData->NodeID == Touch)
      {
        if(receiveData->Command == 0x01) //光敏采样值
        {
          if(receiveData->Data[0] == 0x01)  //检测到震动
          {
            LCD_ShowString(100,180,"---$---");
            HalLedBlink(HAL_LED_3,1,50,500);
          }
          else if(receiveData->Data[0] == 0x00)  //震动消失
          {
            LCD_ShowString(100,180,"Nothing");
          }
        }       
      }
      else if(receiveData->NodeID == Flame)
      {
        uint8 call[]   = "ATD10086;\r";
        //uint8 hangup[] = "ATH\r";
        uint16 left,right;
        if(receiveData->Command == 0x01) 
        {
          left  = receiveData->Data[0]<<8|receiveData->Data[1];
          right = receiveData->Data[2]<<8|receiveData->Data[3];
          LCD_ShowNum(80, 120, left, 5);
          LCD_ShowNum(160, 120, right, 5);
          HalUARTWrite(SERIAL_APP_PORT, call, sizeof(call));
        }       
      }
      else if(receiveData->NodeID == Doppler)
      {
        if(receiveData->Command == 0x01) //光敏采样值
        {
          if(receiveData->Data[0] == 0x01)  //检测到震动
          {
            LCD_ShowString(100,140,"Bodymoving");
          }
          else if(receiveData->Data[0] == 0x00)  //震动消失
          {
            LCD_ShowString(100,140,"Bodystatic");
          }
        }       
      }
      else if(receiveData->NodeID == Ultrasound)
      {
        if(receiveData->Command == 0x01) //光敏采样值
        {
          temp1 = (receiveData->Data[0]<<8)|receiveData->Data[1];
          LCD_ShowNum(100,200,temp1,4);
        }       
      }
      else if(receiveData->NodeID == Accele)  //加速度
      {
        if(receiveData->Command == 0x01) //
        {
          temp1 = (receiveData->Data[0]<<8)|receiveData->Data[1]; //x
          LCD_ShowNum(60,100,temp1,4);
          temp1 = (receiveData->Data[2]<<8)|receiveData->Data[3]; //y
          LCD_ShowNum(120,100,temp1,4);
          temp1 = (receiveData->Data[4]<<8)|receiveData->Data[5]; //z
          LCD_ShowNum(180,100,temp1,4);
        }       
      }
#endif
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
  UART_Format *p;
  if ((event & (HAL_UART_RX_FULL | HAL_UART_RX_ABOUT_FULL | HAL_UART_RX_TIMEOUT)) &&
#if SERIAL_APP_LOOPBACK
      (SerialApp_TxLen < SERIAL_APP_TX_MAX))
#else
      !SerialApp_TxLen)
#endif
  {
    SerialApp_TxLen = HalUARTRead(SERIAL_APP_PORT, SerialApp_TxBuf, SERIAL_APP_TX_MAX); //将串口数据读入buf
    if(SerialApp_TxLen > 0)
    {
      //GPRS_Status = GetGPRSStatus(SerialApp_TxBuf);
      p = (UART_Format*)SerialApp_TxBuf;
      if((p->Header_1==0xcc)&&(p->Header_2==0xee)&&(p->Tailer==0xff))//包头包尾校验
      {
        if(p->NodeID != Coor) //确定不是发送给网关的消息
        {
          osal_set_event(SerialApp_TaskID, SERIALAPP_SEND_EVT); //将串口数据通过RF发送
        }
      } 
    }
    SerialApp_TxLen = 0; 
  }
}

/*********************************************************************
*********************************************************************/
