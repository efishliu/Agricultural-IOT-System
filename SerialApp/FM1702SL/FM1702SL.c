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
#include "fm1702sl.h"
#include "spi.h"

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
#define SERIAL_APP_BAUD  HAL_UART_BR_19200
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
#define M1AREA 		0x01     //要操作的扇区号
 

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
static uint8 RevBuffer[24];
uint8  CardNo[5];
/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt );
void SerialApp_OTAData(afAddrType_t *txaddr,uint8 ID,void *p,uint8 len);
static void SerialApp_CallBack(uint8 port, uint8 event);
static void GPIOInit(void);
void  WriteReg(int8 reg, int8 data);
int8  ReadReg(int8 reg_ad);
void  InitFM1702SL(void);
uint8 Request(uint8 mode);
void  Fifo_Write(uint8 *s, uint8 count);
uint8 FM1702BusSel(void);
uint8 Clear_FIFO(void);
uint8 Fifo_Read(uint8 *buff);
uint8 Command_Send(uint8 count, uint8 *buff, uint8 Comm_Set);
uint8 Judge_Req(uint8 *buff);
uint8 ReadCardNum(void);
uint8 AntiColl(void);
uint8 Check_UID(void);
uint8 Select_Card(void);
void  keyto(void);
uint8 block_numset(uint8 block_num);
uint8 Load_key_CPY(uint8 *buff);
int8  M500HostCodeKey(uint8 *uncoded, uint8 *coded);   
uint8 Authentication(uint8 *UID, uint8 SecNR);
uint8 MIF_READ(uint8 *buff, uint8 Block_Adr);
uint8 MIF_Write(uint8 *buff, uint8 Block_Adr);

static void GPIOInit(void)
{
  P1DIR |= 0x04;  //P12  output
  P0DIR |= 0x01;  //P00
  BEEP = 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//名称: rc531_register_write                                                   // 
//功能: 该函数实现通过SPI接口对RC531中一个寄存器写入值                         // 
//                                                                             // 
//输入:                                                                        // 
//     RC531目标寄存器地址和写入值                                             // 
//                                                                             // 
//输出:                                                                        // 
//     N/A                                                                     // 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void WriteReg(int8 reg_ad, int8 reg_data)
{   
    sck=0;                               //时钟
    reg_ad <<= 1;
    cs=0;                              //接口选通
    reg_ad &= 0x7F;                      //最高位为0表示写
    SPIOneByte(reg_ad);     //写地址
    SPIOneByte(reg_data);  //写数据
    cs=1;
    return;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//名称: rc531_register_read                                                    // 
//功能: 该函数实现通过SPI接口读取RC531中一个寄存器的值                         // 
//                                                                             // 
//输入:                                                                        // 
//     RC531目标寄存器地址                                                     // 
//                                                                             // 
//输出:                                                                        // 
//     目标寄存器的值                                                          // 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
int8 ReadReg(int8 reg_ad)
{   
  int8 temp;
  sck=0;       //时钟
  MicroWait(6);
  cs=0;      //接口选通开
  reg_ad <<= 1;                          //reg_ad左移一位付值给reg_ad
  reg_ad |= 0x80;                        //reg_ad跟“0X80”或运算后付值给reg_ad   最高位为1表示读
  SPIOneByte(reg_ad);
  temp=SPIOneByte(0x00);
  cs=1;
  return (temp);
}
/*总线选择*/
uint8 FM1702BusSel(void)
{
  uint8 i, temp;
  WriteReg(PageSel, 0x80); //选择寄存器组0
  for(i = 0; i < 0xff; i++)
  {
    temp = ReadReg(RFIDCommand); //读取正在执行的命令
    if(temp == 0x00)	//如果没有指令执行
    {	
      WriteReg(PageSel,0x00); //恢复寄存器的线性寻址
      return TRUE;	
    }
  }
  return FALSE;
}

/*********************************************************/
//解释：FIFO缓冲器的写函数
//输入：写入的字节个数 ，要写入的值的存放首地址 
//输出：
/********************************************************/
void Fifo_Write(uint8 *s, uint8 count)
{
  uint8 i,temp;
  for(i=0;i<count;i++)
  {
    temp=*(s+i);
    WriteReg(FIFODaTa, temp);
    MicroWait(100);
  }
}

uint8 Fifo_Read(uint8 *buff)
{
  uint8	temp;
  uint8	i;
  temp = ReadReg(FIFOLength);
  if(temp == 0)
  {
    return 0;
  }
  if(temp >= 24)		
  {
    temp = 24;	
  }
  for(i = 0; i < temp; i++)
  {
    *(buff + i) = ReadReg(FIFODaTa);
  }
  return temp;  
}

uint8 Judge_Req(uint8 *buff)
{
  uint8	temp1, temp2;
  temp1 = *buff;
  temp2 = *(buff + 1);
  if((temp1 == 0x02) || (temp1 == 0x04) || (temp1 == 0x05) || (temp1 == 0x53) || (temp1 == 0x03))
  {
    if (temp2 == 0x00)
    {
      return TRUE;
    }
  }
  return FALSE;
}
/////////////////////////////////////////////////////////
void InitFM1702SL(void)
{
  FM1702_RSTPD=1;
  MicroWait(150);			
  FM1702_RSTPD=0;				 
  MicroWait(150);	//复位1702SL
    	
  while(ReadReg(RFIDCommand) != 0)
  {
    MicroWait(10); 
  }
  FM1702BusSel();  
  WriteReg(InterruptEn,0x7f);		     
  WriteReg(InterruptRq,0x7f);            	
  WriteReg(TxControl,0x5b);	
  WriteReg(RxControl2,0x01);
  WriteReg(RxWait,0x07);
  WriteReg(CryptoSelect,0x00);
}  

/****************************************************************/
/*名称: Clear_FIFO */
/*功能: 该函数实现清空FM1715中FIFO的数据*/
/*输入: N/A */
/*输出: TRUE, FIFO被清空 */
/* FALSE, FIFO未被清空 */
/****************************************************************/
uint8 Clear_FIFO(void)
{
  uint8  temp;
  uint16 i;
  temp = ReadReg(Control);
  temp = (temp | 0x01);
  WriteReg(Control, temp);
  for(i = 0; i < RF_TimeOut; i++) //检查FIFO是否被清空
  {
    temp = ReadReg(FIFOLength);
    if(temp == 0)
    {
      return TRUE;
    }
  }
  return FALSE;
}
/****************************************************************/
/*名称: Command_Send */
/*功能: 该函数实现向FM1715发送命令集的功能 */
/*输入: count, 待发送命令集的长度*/
/* buff, 指向待发送数据的指针 */
/* Comm_Set, 命令码 */
/*输出: TRUE, 命令被正确执行*/
/* FALSE, 命令执行错误*/
/****************************************************************/
uint8 Command_Send(uint8 count, uint8 *buff, uint8 Comm_Set)
{
	uint16   j;
	uint8  temp;
	WriteReg(RFIDCommand,0x00);
	Clear_FIFO();
	Fifo_Write(buff, count);   //把26H写入FIFO
	WriteReg(RFIDCommand,Comm_Set);		/* 命令执行 */
	for(j = 0; j < RF_TimeOut; j++) /* 检查命令执行否 */
	{
	  temp = ReadReg(RFIDCommand);
	  if(temp == 0x00)//如果TEMP为真执行IF语句，否则退出IF语句
	  {
	    return TRUE;//退出FOR语句（带参数返回）
	  }
	}
	return FALSE;
}

//A密码的加密格式：00 4个字节的序列号 块号（扇区号），共6个字节的密钥
void keyto(void)
{
  uint8 i;
  RevBuffer[4] = block_numset(3);
  for(i = 0; i < 7; i++)
    RevBuffer[5 + i] = 0xff;		//默认密码
}

uint8 block_numset(uint8 block_num)
{ 	
  unsigned char temp,i;
  i = M1AREA;
  temp = block_num;
  while(i)
  {
    temp=temp+4;
    i--;
  }
  return temp;
}
/****************************************************************/
/*名称: Request */
/*功能: 该函数实现对放入FM1702操作范围之内的卡片的Request操作*/
/*输入: mode: ALL(监测所以FM1702操作范围之内的卡片) */
/* STD(监测在FM1702操作范围之内处于HALT状态的卡片) */
/*输出: FM1702_NOTAGERR: 无卡*/
/* FM1702_OK: 应答正确*/
/* FM1702_REQERR: 应答错误*/
/****************************************************************/
uint8 Request(uint8 mode)
{	
  uint8 temp;	
  RevBuffer[0] = mode; //00x26
  WriteReg(0x11,0x58);//关发射控制
  WriteReg(0x11,0x5b);//开发射控制
  WriteReg(0x0f,0x07);          //address 0FH  调整位的帧格式	
  temp = ReadReg(0x09);
  temp &= 0xf7;
  WriteReg(0x09,temp);
  WriteReg(0x22,0x03);
  
  temp = Command_Send(1, RevBuffer, Transceive); //写0x26到FIFO
  if(temp == FALSE)
  {
    return FALSE;	
  }
  temp = Fifo_Read(RevBuffer);    //读取FIFO里面的数据及数据长度	
  if(temp == 2)
  {
    temp = Judge_Req(RevBuffer); //对卡片复位应答信号的判断返回值是否正确
    if(temp == TRUE)
    {
      return TRUE;	
    }
  }
  return FALSE;
}

/****************************************************************/
/*名称: Load_keyE */
/*功能: 该函数实现把E2中密码存入FM1702的keyRevBuffer中*/
/*输入: Secnr: EE起始地址*/
/*输出: True: 密钥装载成功*/
/* False: 密钥装载失败*/
/****************************************************************/
uint8 Load_key_CPY(uint8 *buff)
{
  int8  status;
  uint8 coded_keys[12];
  uint8 temp;
  M500HostCodeKey(buff, coded_keys);////////////////	
  temp=Command_Send(0x0c,coded_keys,LoadKey);/* LoadKey将密钥从FIFO缓存复制到KEY缓存 0x19*/
  if(temp == FALSE)       //0x0c为12个字节长度 //coded_keys为指向地址寄存器
  {
    return FM1702_LOADKEYERR;
  }
  status = (ReadReg(ErrorFlag)) & 0x40;//判断loadkey执行是否正确
  if (status==0x40)
    return FM1702_AUTHERR;
  return FM1702_OK;
}

// 转换密钥格式
///////////////////////////////////////////////////////////////////////
int8 M500HostCodeKey(uint8 *uncoded, uint8 *coded)   
{
  uint8 cnt = 0;
  uint8 ln  = 0;     
  uint8 hn  = 0;      
  for (cnt = 0; cnt < 6; cnt++)
  {
    ln = uncoded[cnt] & 0x0F;
    hn = uncoded[cnt] >> 4;
    coded[cnt * 2 + 1] = (~ln << 4) | ln;
    coded[cnt * 2 ] = (~hn << 4) | hn;
  }
  return 0;
}

/*读取卡号*/
uint8 ReadCardNum(void)
{
  uint8 st;
  st= AntiColl();
  if(st!=0)				 
    return (1);
  st = Select_Card();  //选择卡片
  if(st!=0)
    return (1);
  keyto();
  st = Load_key_CPY(&RevBuffer[5]);
  if(st!=0)
    return (1);		       
  st = Authentication(CardNo, RevBuffer[4]);
  if(st!=0)
    return (1);
  return (0);
}

/****************************************************************/
/*名称: Authentication */
/*功能: 该函数实现密码认证的过程*/
/*输入: UID: 卡片序列号地址*/
/* SecNR: 扇区号*/
/*输出: FM1702_NOTAGERR: 无卡*/
/* FM1702_PARITYERR: 奇偶校验错*/
/* FM1702_CRCERR: CRC校验错*/
/* FM1702_OK: 应答正确*/
/* FM1702_AUTHERR: 权威认证有错*/
/****************************************************************/
uint8 Authentication(uint8 *UID, uint8 SecNR)
{	
  uint8 i;
  uint8 temp, temp1;	
  RevBuffer[0] = RF_CMD_AUTH_LA;   //密码A   
  RevBuffer[1] = SecNR;  
  for(i = 0; i < 4; i++)
  RevBuffer[2 + i] = UID[i];//把序列号放入缓冲区是否为同一张卡	  
  WriteReg(ChannelRedundancy, 0x0f);  //否则验证不通过
//************ Authent1=0x0c 验证命令认证过程第1步 ****************//	
  temp = Command_Send(6, RevBuffer, Authent1);
  if(temp == FALSE)   			
    return FM1702_AUTHERR;	
//************ Authent2=0x14 验证命令认证过程第2步 ****************//
  temp = Command_Send(0, RevBuffer, Authent2);//如果密码错则第二步不通过
  if(temp == FALSE)			
    return FM1702_AUTHERR;
  temp1 = ReadReg(Control);//读控制标识寄存器为0X08时表示加密单元打开，
  temp1 = temp1 & 0x08;	//通过后该位内部置1 （CRYPTO1ON）
  if(temp1 == 0x08)
    return (0);
  return FM1702_AUTHERR;
}

/****************************************************************/
/*名称: Select_Card */
/*功能: 该函数实现对放入FM1702操作范围之内的某张卡片进行选择*/
/*输入: N/A */
/*输出: FM1702_NOTAGERR: 无卡*/
/* FM1702_PARITYERR: 奇偶校验错*/
/* FM1702_CRCERR: CRC校验错*/
/* FM1702_BYTECOUNTERR: 接收字节错误*/
/* FM1702_OK: 应答正确*/
/* FM1702_SELERR: 选卡出错*/
/****************************************************************/
uint8 Select_Card(void)
{
  uint8 temp, i;	

  RevBuffer[0] = RF_CMD_SELECT;//写0x93命令时关闭发送以及接收CRC校验以及开启奇校验
  RevBuffer[1] = 0x70; //当发送PICC_REQSTD 命令时RegBitFraming 要装入0x07
  for(i = 0; i < 5; i++)
  {
    RevBuffer[i + 2] = CardNo[i];  //把5个卡序列号转放入缓冲区
  }
  WriteReg(ChannelRedundancy, 0x0f);//选择数据校验的种类和模式	
  temp = Command_Send(7, RevBuffer, Transceive);//Transceive=1EH（接收命令）
  if(temp == FALSE)			
  {
    return(1);
  }
  else
  {
    Fifo_Read(RevBuffer);	/* 从FIFO中读取应答信息, 读取卡片容量*/
    temp = *RevBuffer;			//返回卡片容量S50卡:08,S70卡:18
    if((temp == 0x18) || (temp == 0x08) || (temp == 0x28) || (temp == 0x53))	/* 判断应答信号是否正确 */
      return(0);
    else
      return(FM1702_SELERR);
  }
}

/****************************************************************/
/*名称: Check_UID */
/*功能: 该函数实现对收到的卡片的序列号的判断*/
/*输入: N/A */
/*输出: TRUE: 序列号正确*/
/* FALSE: 序列号错误*/
/****************************************************************/
uint8 Check_UID(void)
{
  uint8 temp;
  uint8 i;
  temp = 0x00;
  for(i = 0; i < 5; i++)
  {
    temp = temp ^ CardNo[i];//对卡号异或运算，即二数相同为0，不同为1
  }						//正确结果为0
  if(temp == 0)
  {
    return TRUE;
  }
  return FALSE;
}

/****************************************************************/
/*名称: AntiColl */
/*功能: 该函数实现对放入FM1702操作范围之内的卡片的防冲突检测*/
/*输入: N/A */
/*输出: FM1702_NOTAGERR: 无卡*/
/* FM1702_BYTECOUNTERR: 接收字节错误*/
/* FM1702_SERNRERR: 卡片序列号应答错误*/
/* FM1702_OK: 卡片应答正确*/
/****************************************************************/
uint8 AntiColl(void)
{
  uint8	temp;
  uint8	i;
  RevBuffer[0] = RF_CMD_ANTICOL;    //写寻卡通讯命令93H
  RevBuffer[1] = 0x20;              //写寻卡通讯命令20H
  WriteReg(ChannelRedundancy, 0x03); //address 22H  选择数据校验种类和类型
  temp = Command_Send(2, RevBuffer, Transceive);//Transceive=0X1E命令
  while(1)
  {
    if(temp == FALSE)
    {
      return(1);   //无卡
    }	  
    temp = ReadReg(FIFOLength);   //读0x04里的返回卡号长度
    if(temp == 0)
    {
      return FM1702_BYTECOUNTERR; //接收字节长度错误
    }
    Fifo_Read(RevBuffer);           //读取卡号  					
    for(i = 0; i < temp; i++)
    {
      CardNo[i] = RevBuffer[i]; //把临时寄存器中的卡号放在UID专用寄存器中
    }		
    temp = Check_UID();			/* 校验收到的UID */
    if(temp == FALSE)
    {
      return(FM1702_SERNRERR);
    }
    return(0);
  }
}

/****************************************************************/
/*名称: MIF_Read */
/*功能: 该函数实现读MIFARE卡块的数值*/
/*输入: buff: 缓冲区首地址*/
/* Block_Adr: 块地址*/
/*输出: FM1702_NOTAGERR: 无卡*/
/* FM1702_PARITYERR: 奇偶校验错*/
/* FM1702_CRCERR: CRC校验错*/
/* FM1702_BYTECOUNTERR: 接收字节错误*/
/* FM1702_OK: 应答正确*/
/****************************************************************/
uint8 MIF_READ(uint8 *buff, uint8 Block_Adr)
{
	uint8 temp;
	WriteReg(0x22,0x0f);
	buff[0] = RF_CMD_READ;   //0x30
	buff[1] = Block_Adr;
	temp = Command_Send(2, buff, Transceive);
	if(temp == 0)
	{			
	  return (1);  //错误
	}

	temp = ReadReg(0x04);
	if(temp == 0x10)	
	{
	  Fifo_Read(buff);
	  return (0); //正确
	}
	return (1);
}

/****************************************************************/
/*名称: MIF_Write */
/*功能: 该函数实现写MIFARE卡块的数值*/
/*输入: buff: 缓冲区首地址*/
/* Block_Adr: 块地址*/
/*输出: FM1702_NOTAGERR: 无卡*/
/* FM1702_BYTECOUNTERR: 接收字节错误*/
/* FM1702_NOTAUTHERR: 未经权威认证*/
/* FM1702_EMPTY: 数据溢出错误*/
/* FM1702_CRCERR: CRC校验错*/
/* FM1702_PARITYERR: 奇偶校验错*/
/* FM1702_WRITEERR: 写卡块数据出错*/
/* FM1702_OK: 应答正确*/
/****************************************************************/
uint8 MIF_Write(uint8 *buff, uint8 Block_Adr)
{
	uint8	temp;
	uint8	*F_buff;

	WriteReg(0x23,0x63);
	WriteReg(0x12,0x3f);
	F_buff = buff + 0x10;
	WriteReg(0x22,0x07);    /* Note: this line is for 1702, different from RC500*/
	*F_buff = RF_CMD_WRITE;
	*(F_buff + 1) = Block_Adr;
	temp = Command_Send(2, F_buff, Transceive);
	if(temp == FALSE)
	{
		return(1);
	}

	temp = ReadReg(0x04);
	if(temp == 0)
	{
		return(FM1702_BYTECOUNTERR);
	}

	Fifo_Read(F_buff);
	temp = *F_buff;
	switch(temp)
	{
	case 0x00:	return(FM1702_NOTAUTHERR);	
	case 0x04:	return(FM1702_EMPTY);
	case 0x0a:	break;
	case 0x01:	return(FM1702_CRCERR);
	case 0x05:	return(FM1702_PARITYERR);
	default:	return(FM1702_WRITEERR);
	}

	temp = Command_Send(16, buff, Transceive);
	if(temp == TRUE)
	{
		return(0);
	}
	else
	{
		temp = ReadReg(0x0a);
		if((temp & 0x02) == 0x02)
			return(FM1702_PARITYERR);
		else if((temp & 0x04) == 0x04)
			return(FM1702_FRAMINGERR);
		else if((temp & 0x08) == 0x08)
			return(FM1702_CRCERR);
		else
			return(FM1702_WRITEERR);
	}
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
  HalUARTInitSPI();
  GPIOInit();
  InitFM1702SL();
 
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
  UART0_Format.NodeID   = RFID_1356;
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
            HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);
            HalLedBlink(HAL_LED_2,5,50,500);
            osal_set_event(SerialApp_TaskID, PERIOD_EVT); //启动周期消息
            osal_set_event(SerialApp_TaskID, RFID_EVT);
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
    UART0_Format.Data[4] = 0x00;
    UART0_Format.Data[5] = 0x00; 
    SerialApp_OTAData(&SerialApp_TxAddr, SERIALAPP_CLUSTERID1, &UART0_Format, sizeof(UART_Format));
    osal_start_timerEx(SerialApp_TaskID, PERIOD_EVT, 5000);
    return ( events ^ PERIOD_EVT );
  }
  
  if ( events & SERIALAPP_SEND_EVT ) //发送RF消息
  {
    SerialApp_OTAData(&SerialApp_TxAddr,SERIALAPP_CLUSTERID1, &UART0_Format, sizeof(UART_Format));
    return ( events ^ SERIALAPP_SEND_EVT );
  }
  
  if ( events & RFID_EVT )  
  {
    uint8 state;
    static uint8 hasCardOld = FALSE;
    static uint8 hasCardNow = FALSE;
    hasCardOld = hasCardNow;
    hasCardNow = Request(RF_CMD_REQUEST_STD);
    if(hasCardOld != hasCardNow) //返回1说明检测到卡
    {
      state = ReadCardNum(); //读卡号，存储在CardNo数组里面
      if(!state) //读卡正确  
      {
        BEEP = 1; 
        UART0_Format.Command = SEND;
        UART0_Format.Data[0] = CardNo[0]; //d2345678
        UART0_Format.Data[1] = CardNo[1]; 
        UART0_Format.Data[2] = CardNo[2];
        UART0_Format.Data[3] = CardNo[3]; 
        MIF_READ(RevBuffer, block_numset(1));  //读出数据块0的数据放在RevBuffer里面
        UART0_Format.Data[4] = RevBuffer[2];   //卡内块0中的第一个字节为金额高位
        UART0_Format.Data[5] = RevBuffer[3];   //卡内块0中的第二个字节为金额低位
        osal_set_event(SerialApp_TaskID, SERIALAPP_SEND_EVT);
        MicroWait(50000);
        MicroWait(50000);
        BEEP = 0;
      }
      else
      {
        CardNo[0] = CardNo[1] = CardNo[2] = CardNo[3] = 0;
      }
    }
    osal_start_timerEx(SerialApp_TaskID, RFID_EVT, 1000);
    return ( events ^ RFID_EVT );
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
  static uint8  secNR;
  uint8  status;
  uint8  ctimeH,ctimeL;
  uint16 ctime;
  static uint16 overage;
  static UART_Format Rsp;
  Rsp.Header_1 = 0xee;
  Rsp.Header_2 = 0xcc;
  Rsp.NodeSeq  = 0x01;
  Rsp.NodeID   = RFID_1356;
  Rsp.Command  = MSG_RSP;
  Rsp.Tailer   = 0xff;
  switch ( pkt->clusterId )
  {
   case SERIALAPP_CLUSTERID1:  //处理各个传感器节数据    
     receiveData = (UART_Format *)(pkt->cmd.Data);
     HalLedBlink(HAL_LED_1,1,50,200);
     if((receiveData->Header_1==0xcc)&&(receiveData->Header_2==0xee)&&(receiveData->Tailer==0xff)) //校验包头包尾
     {
       if(receiveData->NodeID == RFID_1356) //收到数据后通过串口发送给RFID模块
       {
         HalLedSet(HAL_LED_2, HAL_LED_MODE_OFF);
 	 do{	
	     status = Request(RF_CMD_REQUEST_STD);
         }while(!status);	
         HalLedSet(HAL_LED_2, HAL_LED_MODE_ON);
	 status = ReadCardNum();//等待读取卡号  
         if(!status)
         {
           if(receiveData->Command == INCREASE)
           {         
             secNR  = block_numset(1);            //操作数据块1
             MIF_READ(RevBuffer, secNR);          //读出数据块0的数据放在RevBuffer里面
             ctimeH = RevBuffer[2];                 //卡内块0中的第一个字节为金额高位
             ctimeL = RevBuffer[3];                 //卡内块0中的第二个字节为金额低位
  
             ctime  = ctimeH<<8|ctimeL;             //操作金额的格式转换     				
             ctime += (receiveData->Data[0]<<8)|receiveData->Data[1];//扣除指定的金额
             overage = ctime;  //得到卡内剩余的金额
  
             RevBuffer[3] = ctime;//把低字节写入缓存器中
             ctime >>= 8;
             RevBuffer[2] = ctime;//把高字节写入缓存器中
  
              /////写数据块的操作////////////
             secNR  = block_numset(1);      //操作数据块1
             MIF_Write(RevBuffer, secNR); //写块
           }
           else if(receiveData->Command == REDUCE)
           {
             secNR  = block_numset(1);            //操作数据块1
             MIF_READ(RevBuffer,secNR);  //读出数据块0的数据放在RevBuffer里面
             ctimeH = RevBuffer[2];                 //卡内块0中的第一个字节为金额高位
             ctimeL = RevBuffer[3];                 //卡内块0中的第二个字节为金额低位
  
             ctime = ctimeH<<8|ctimeL;             //操作金额的格式转换				
             ctime -= (receiveData->Data[0]<<8)|receiveData->Data[1];//扣除指定的金额
             overage = ctime;  //得到卡内剩余的金额
             
             RevBuffer[3] = ctime;//把低字节写入缓存器中
             ctime >>= 8;
             RevBuffer[2] = ctime;//把高字节写入缓存器中
  
              /////写数据块的操作////////////
             secNR  = block_numset(1);      //操作数据块1
             MIF_Write(RevBuffer, secNR); //写块
           }           
           //do{
           //  status = Request(RF_CMD_REQUEST_STD); 
           //}//扣款成功后等待把卡移开感应区
           //while(status);
           Rsp.Data[0] = overage>>8;
           Rsp.Data[1] = overage; 
           SerialApp_OTAData(&SerialApp_TxAddr, SERIALAPP_CLUSTERID1, &Rsp, sizeof(UART_Format));
         }
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
    if(SerialApp_TxLen)
    {
      if(SerialApp_TxBuf[0] == 0xaa && SerialApp_TxBuf[1] == 0xbb) //校验包头
      {
        if(SerialApp_TxBuf[2] == 0x06 && SerialApp_TxBuf[3] == 0x20) //读取卡号
        {
          UART0_Format.Command = 0x01; //第一个命令
          UART0_Format.Data[0] = SerialApp_TxBuf[4];
          UART0_Format.Data[1] = SerialApp_TxBuf[5];   
          UART0_Format.Data[2] = SerialApp_TxBuf[6];
          UART0_Format.Data[3] = SerialApp_TxBuf[7]; 
          osal_set_event(SerialApp_TaskID, SERIALAPP_SEND_EVT);
        }
      }
    }
    SerialApp_TxLen = 0;  
  }
}

/*********************************************************************
*********************************************************************/
