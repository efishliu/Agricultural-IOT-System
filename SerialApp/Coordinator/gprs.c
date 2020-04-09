#include "gprs.h"
#include "hal_board.h"
#include "hal_lcd.h"

uint8 GPRS_Status;

void GPRS_ON(void);
void GPRS_OFF(void);
void GPRS_Initial(void);
uint8 GetGPRSStatus(uint8 *p);

//初始化GPRS控制端口
void GPRS_Initial(void)
{
  P2SEL &= ~0x01;
  P2DIR |=  0x01;
  GPRS_Status = IDLE;
}
//打开GPRS
void GPRS_ON(void)
{
  GPRS_SETUP = ON;
}
//关闭GPRS
void GPRS_OFF(void)
{
  GPRS_SETUP = OFF;
}

//根据串口接收到的返回值确定GPRS模块的状态
uint8 GetGPRSStatus(uint8 *p)
{
  uint8 state = IDLE;
  uint8 buf[20];
  uint8 i=2;
  if((*p == 0x0D)&&(*(p+1) == 0x0A)) //gprs返回的包头
  {
    do{
      buf[i-2] = *(p+i);
      i++;
      if((*(p+i) == 0x0D)&&(*(p+i+1) == 0x0A))
      {
        break;
      }
    }while(1);
  }
  if((buf[0] == 'O')&&(buf[1] == 'K')) //命令执行完成
  {
    //LCD_ShowString(10,80,"OK   ");    
  }
  else if((buf[0] == 'R')&&(buf[1] == 'I')&&(buf[2] == 'N')&&(buf[3] == 'G')) //电话接入
  {
    //GPRS_Status = CALL;
    //LCD_ShowString(10,80,"RING "); 
  }
  else 
  {
    //GPRS_Status = IDLE;
    //LCD_ShowString(10,80,"ERROR");
  }
  return state;
}
