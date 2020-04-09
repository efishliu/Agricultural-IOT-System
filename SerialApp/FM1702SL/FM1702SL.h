#ifndef FM1702SL_H
#define FM1702SL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"

/*********************************************************************
 * CONSTANTS
 */

// These constants are only for example and should be changed to the
// device's needs
#define SERIALAPP_ENDPOINT           11

#define SERIALAPP_PROFID             0x0F05
#define SERIALAPP_DEVICEID           0x0001
#define SERIALAPP_DEVICE_VERSION     0
#define SERIALAPP_FLAGS              0

#define SERIALAPP_MAX_CLUSTERS       2
#define SERIALAPP_CLUSTERID1         1
#define SERIALAPP_CLUSTERID2         2

#define SERIALAPP_SEND_EVT           0x0001
#define PERIOD_EVT                   0x0002
#define RFID_EVT                     0x0004

// OTA Flow Control Delays
#define SERIALAPP_ACK_DELAY          1
#define SERIALAPP_NAK_DELAY          16

// OTA Flow Control Status
#define OTA_SUCCESS                  ZSuccess
#define OTA_DUP_MSG                 (ZSuccess+1)
#define OTA_SER_BUSY                (ZSuccess+2)
  
#define FM1702_RSTPD                P1_2
  
/*********************************************/
  
#define         PageSel            0x00	         //页写寄存器 
#define	        RFIDCommand	   0x01	         //开始（停止）命令的执行
#define       	FIFODaTa           0x02	         //64字节FIFO的输入输出
#define       	PrimaryStatus	   0x03	         //接收器/传送器/FIFO的状态标志
#define       	FIFOLength	   0x04	         //FIFO中存储数据的字节数
#define       	SecondaryStatus	   0x05	         //不同的状态标志
#define       	InterruptEn        0x06	         //使能请求中断传送的控制位
#define         InterruptRq        0x07	         //中断请求标志
/*********第1页,控制和状态********************/
#define         Control	           0x09	         //不同的控制标志，例如：定时、功耗等
#define       	ErrorFlag          0x0A	         //显示最后一次执行的命令的错误状态的标志
#define       	CollPos		   0x0B	         //在RF接口检测到的第一个冲突位的位置
#define       	TimerValue         0x0C	         //定时器的实际值
#define       	CRCResultLSB       0x0D	         //CRC协处理器寄存器的最低有效字节
#define       	CRCResultMSB       0x0E	         //CRC协处理器寄存器的最高有效字节
#define         BitFraming         0x0F	         //调整位定向帧
/*********第2页,传送器和编码控制**************/
#define         TxControl          0x11	         //控制天线驱动引脚Tx1、Tx2的逻辑行为
#define       	CWConductance      0x12	         //选择天线驱动引脚Tx1、Tx2的电导
#define       	PreSet13           0x13	         //这些值不可以改变
#define       	PreSet14           0x14	         //这些值不可以改变
#define       	ModWidth           0x15	         //选择调制脉冲的宽度
#define       	PreSet16           0x16	         //这些值不可以改变
#define         PreSet17           0x17	         //这些值不可以改变
/*********第3页,接收器及解码控制**************/
#define         RxControl1         0x19	         //控制接收器行为
#define       	DecoderControl     0x1A	         //控制解码器行为
#define       	BitPhase           0x1B	         //选择接收器和传送器时钟间的位相
#define       	RxThreshold        0x1C	         //选择位解码器的阈值
#define       	PreSet1D           0x1D	         //这些值不可以改变
#define       	RxControl2         0x1E	         //控制解码器行为并定义接收器的输入源 
#define         ClockQControl      0x1F	         //控制时钟产生
/*********第4页,射频时间和通道冗余*************/
#define         RxWait             0x21	         //选择在传送之后，接收器工作之前的时间间隔
#define       	ChannelRedundancy  0x22	         //选择验证RF通道数据完整性得类型和模式
#define       	CRCPresetLSB       0x23	         //CRC寄存器预置值的最低有效字节
#define       	CRCPresetMSB       0x24	         //CRC寄存器预置值的最高有效字节
#define       	PreSet25           0x25	         //这些值不可以改变
#define       	MFOUTSelect        0x26	         //选择应用到MFOUT引脚的内部信号
#define         PreSet27           0x27	         //这些值不可以改变
/*********第5页,FIFO、定时器及中断引脚配置******/
#define         FIFOLevel          0x29	         //定义FIFO的大小，是
#define       	TimerClock         0x2A	         //选择时钟的分频
#define       	TimerControl       0x2B	         //选择定时器的开始和结束条件
#define       	TimerReload        0x2C	         //义定时器的预置值
#define       	IRQPinConfig       0x2D	         //配置引脚IRQ的输出状态
#define       	PreSet2E           0x2E	         //这些值不可以改变
#define         PreSet2F           0x2F	         //这些值不可以改变
/**************第6,7页,预留***********************/
#define        	CryptoSelect       0x31	         //论证模式选择,可选择飞利蒲标准和上海标准		
#define		PCD_IDLE           0x00          //取消当前指令
#define        	PICC_BLOCK         0x08          //对第8块号操作,M1卡块号范围从0-63,16个扇区统一编址的
//#define			BLOCK_N	           0x3D	         //3DH存放要操作的块

  
  //FM1715命令码
#define Transceive              0x1E //发送接收命令
#define Transmit                0x1a //发送命令
#define ReadE2                  0x03 //读FM1715 EEPROM命令
#define WriteE2                 0x01 //写FM1715 EEPROM命令
#define Authent1                0x0c //验证命令认证过程第1步
#define Authent2                0x14 //验证命令认证过程第2步
#define LoadKeyE2               0x0b //将密钥从EEPROM复制到KEY缓存
#define LoadKey                 0x19 //将密钥从FIFO缓存复制到KEY缓存
#define RF_TimeOut              0xfff //发送命令延时时间
#define Req                     0x01
#define Sel                     0x02
  
  //卡片类型定义定义
#define TYPEA_MODE              0 //TypeA模式
#define TYPEB_MODE              1 //TypeB模式
#define SHANGHAI_MODE           2 //上海模式
  
#define TM0_HIGH                0xf0 //定时器0高位,4MS定时
#define TM0_LOW                 0x60 //定时器0低位
#define TIMEOUT                 100 //超时计数器4MS×100=0.4秒
  
#define RF_CMD_REQUEST_STD      0x26
#define RF_CMD_REQUEST_ALL      0x52
#define RF_CMD_ANTICOL          0x93
#define RF_CMD_SELECT           0x93
#define RF_CMD_AUTH_LA          0x60
#define RF_CMD_AUTH_LB          0x61
#define RF_CMD_READ             0x30
#define RF_CMD_WRITE            0xa0
#define RF_CMD_INC              0xc1
#define RF_CMD_DEC              0xc0
#define RF_CMD_RESTORE          0xc2
#define RF_CMD_TRANSFER         0xb0
#define RF_CMD_HALT             0x50
//
  
  /* 函数错误代码定义 ERR CODE  */
#define FM1702_OK		0		/* 正确 */
#define FM1702_NOTAGERR		1		/* 无卡 */
#define FM1702_CRCERR		2		/* 卡片CRC校验错误 */
#define FM1702_EMPTY		3		/* 数值溢出错误 */
#define FM1702_AUTHERR		4		/* 验证不成功 */
#define FM1702_PARITYERR	5		/* 卡片奇偶校验错误 */
#define FM1702_CODEERR		6		/* 通讯错误(BCC校验错) */
#define FM1702_SERNRERR		8		/* 卡片序列号错误(anti-collision 错误) */
#define FM1702_SELECTERR	9		/* 卡片数据长度字节错误(SELECT错误) */
#define FM1702_NOTAUTHERR	10		/* 卡片没有通过验证 */
#define FM1702_BITCOUNTERR	11		/* 从卡片接收到的位数错误 */
#define FM1702_BYTECOUNTERR	12		/* 从卡片接收到的字节数错误仅读函数有效 */
#define FM1702_RESTERR		13		/* 调用restore函数出错 */
#define FM1702_TRANSERR		14		/* 调用transfer函数出错 */
#define FM1702_WRITEERR		15		/* 调用write函数出错 */
#define FM1702_INCRERR		16		/* 调用increment函数出错 */
#define FM1702_DECRERR		17		/* 调用decrement函数出错 */
#define FM1702_READERR		18		/* 调用read函数出错 */
#define FM1702_LOADKEYERR	19		/* 调用LOADKEY函数出错 */
#define FM1702_FRAMINGERR	20		/* FM1702帧错误 */
#define FM1702_REQERR		21		/* 调用req函数出错 */
#define FM1702_SELERR		22		/* 调用sel函数出错 */
#define FM1702_ANTICOLLERR	23		/* 调用anticoll函数出错 */
#define FM1702_INTIVALERR	24		/* 调用初始化函数出错 */
#define FM1702_READVALERR	25		/* 调用高级读块值函数出错 */
#define FM1702_DESELECTERR	26
#define FM1702_CMD_ERR		42		/* 命令错误 */
/*********************************************************************
 * MACROS
 */
#define BEEP                     P0_0
  
#define SEND                     0x01
#define INCREASE                 0x02
#define REDUCE                   0x03  
/*********************************************************************
 * GLOBAL VARIABLES
 */
extern byte SerialApp_TaskID;

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the Serial Transfer Application
 */
extern void SerialApp_Init( byte task_id );

/*
 * Task Event Processor for the Serial Transfer Application
 */
extern UINT16 SerialApp_ProcessEvent( byte task_id, UINT16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif 
