/****************************************************************************
* 文 件 名: main.c
* 作    者: Andy
* 修    订: 2014-03-01
* 版    本: 1.0
* 描    述: MQ-2气体传感器,当测量浓度大于设定浓度时，LED1会闪烁,MQ-2上的DD-LED
*          也会长亮。如果另外一个IO接蜂鸣器就可报警了，自己DIY吧!
****************************************************************************/
#include "ioCC2530.h" 
#include "string.h"


typedef unsigned char uchar;
typedef unsigned int  uint;
typedef signed short int16;
typedef unsigned short uint16;

char TxBuf[5];
uint16 GasData;

uint16 ReadGasData( void );

/****************************************************************************
* 名    称: InitUart()
* 功    能: 串口初始化函数
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitUart(void)
{ 
  PERCFG = 0x00;           //外设控制寄存器 USART 0的IO位置:0为P0口位置1 
  P0SEL = 0x0c;            //P0_2,P0_3用作串口（外设功能）
  P2DIR &= ~0XC0;          //P0优先作为UART0
  
  U0CSR |= 0x80;           //设置为UART方式
  U0GCR |= 11;				       
  U0BAUD |= 216;           //波特率设为115200
  UTX0IF = 0;              //UART0 TX中断标志初始置位0
}

/****************************************************************************
* 名    称: UartSendString()
* 功    能: 串口发送函数
* 入口参数: Data:发送缓冲区   len:发送长度
* 出口参数: 无
****************************************************************************/
void UartSendString(char *Data, int len)
{
  uint i;
  
  for(i=0; i<len; i++)
  {
    U0DBUF = *Data++;
    while(UTX0IF == 0);
    UTX0IF = 0;
  }
}

/****************************************************************************
* 名    称: DelayMS()
* 功    能: 以毫秒为单位延时 16M时约为535,32M时要调整,系统时钟不修改默认为16M
* 入口参数: msec 延时参数，值越大延时越久
* 出口参数: 无
****************************************************************************/
void DelayMS(uint msec)
{  
  uint i,j;
  
  for (i=0; i<msec; i++)
    for (j=0; j<1070; j++);
}

uint16 ReadGasData( void )
{
  uint16 reading = 0;
  
  /* Enable channel */
  ADCCFG |= 0x40;
  
  /* writing to this register starts the extra conversion */
  ADCCON3 = 0x86;// AVDD5 引脚  00： 64 抽取率(7 位ENOB)  0110： AIN6
  
  /* Wait for the conversion to be done */
  while (!(ADCCON1 & 0x80));
  
  /* Disable channel after done conversion */
  ADCCFG &= (0x40 ^ 0xFF); //按位异或。如1010^1111=0101（二进制）
  
  /* Read the result */
  reading = ADCL;
  reading |= (int16) (ADCH << 8); 
  
  reading >>= 8;
  
  return (reading);
}

void main(void)
{
  CLKCONCMD &= ~0x40;         //设置系统时钟源为32MHZ晶振
  while(CLKCONSTA & 0x40);    //等待晶振稳定为32M
  CLKCONCMD &= ~0x47;         //设置系统主时钟频率为32MHZ  
  
  InitUart();                   //调置串口相关寄存器
  
  while(1)
  {
    GasData = ReadGasData();  //读取烟雾传感器引脚上的ad转换值，并没有换算成能表示烟雾浓度的值
    //演示如何使用2530芯片的AD功能，更具体在组网中给出
    
    //读取到的数值转换成字符串，供串口函数输出
    TxBuf[0] = GasData / 100 + '0';
    TxBuf[1] = GasData / 10%10 + '0';
    TxBuf[2] = GasData % 10 + '0';
    TxBuf[3] = '\n';
    TxBuf[4] = 0;
    
    UartSendString(TxBuf, 4); //想串口助手送出数据，波特率是115200      
    DelayMS(2000);            //延时函数
  }
}


