/**************************************









***************************************/
#include "ioCC2530.h"
#include <stdio.h>
#include <string.h>
#include "DHT11.h"
#include "BH1750.h"
#include "motor.h"
#include "remote.h"

#define MQ_PIN P0_6           //定义P0.6口为传感器的输入端
#define Relay_PIN P0_5
#define LED P1_0

char is_serial_receive = 0;
void main(void)
{
  unsigned int temp_now,hum_now,Guang;
  unsigned char ltemp[3]; 
  unsigned char lhumidity[3];
  uchar MQ_data=1;
  uchar Window=0;
  uchar mode = 0;
  uchar j=0;
  CLKCONCMD &= ~0x40;      // 设置系统时钟源为 32MHZ晶振
  while(CLKCONSTA & 0x40); // 等待晶振稳定 
  CLKCONCMD &= ~0x47;      // 设置系统主时钟频率为 32MHZ
  
  P1DIR |= 0x01;                              // P1.0 发光LED
  LED = 1;
  
  P0DIR &= ~0x40;              //P0.6定义为输入口 MQ-2
  P0DIR |= 0x20;          //P0.5定义为继电器输出
  //P0SEL &=0x7f;
  Relay_PIN = 1;          //继电器低电平触发，初始化为高
  Init_BH1750();          // 光照度传感器
  InitMotor();            // 步进电机初始化
  
  EA = 0;
 
  InitKey();
  uart0_init();                               //115200
  timer1_init();
  rf_init();
 
  EA = 1;

  printf("Server start!\r\n");
  unsigned char senddata[16];
  while(1)
  {
    memset(ltemp, 0, 2);
    memset(lhumidity, 0, 2);
    memset(senddata, 0, 16);
    DHT11();             //获取温湿度
    
    temp_now=wendu_shi*10+wendu_ge;
    hum_now=shidu_shi*10+shidu_ge;
      
    ltemp[0]=wendu_shi+0x30;
    ltemp[1]=wendu_ge+0x30;
    ltemp[2]='\0';
    lhumidity[0]=shidu_shi+0x30;
    lhumidity[1]=shidu_ge+0x30;
    lhumidity[2]='\0';
   
   //unsigned char *ntemp=(unsigned char*)temp_now;
   //unsigned char *nhum=(unsigned char*)hum_now;
   
    //温湿度发送
   strcpy(senddata,"SD01");
   strcat(senddata,ltemp);
   strcat(senddata,lhumidity);
   strcat(senddata,"00000000");
   uart0_sendbuf(senddata,16);
   Delay_ms(200); 
   /*
   uart0_sendbuf("temp: ",6);
   uart0_sendbuf(ltemp,2);
   uart0_sendbuf(" hum: ",6);
   uart0_sendbuf(lhumidity,2);
   uart0_sendbuf("\n",1);
   */
   //Delay_ms(2000);  //延时，2S读取1次 
   
   
   
      uart0_flush_rxbuf();
    }
  }


