#include "ioCC2530.h"
#include <stdio.h>
#include <string.h>

#define LED P1_0
#define RDATA P1_3        //红外接收口
#define REMOTE_ID 0 
#define FLAG   (1<<3)
#define u32 unsigned long
#define u8  unsigned char


u32 Remote_Odr=0;  	 //命令暂存处
u8  Remote_Cnt=0;        //按键次数,此次按下键的次数
u8  Remote_Rdy=0;        //红外接收到数据   
u8 res=0;                //中断变量
u8 OK=0; 
u8 RODATA=0;  
unsigned int key[4];
u8 bufer[7];

char rf_rx_buf[128];
char serial_rxbuf[128];
char rfbbf[128];
int  serial_rxpos = 0;
int  serial_rxlen = 0;
char is_serial_receive = 0;

void uart0_init();
void uart0_sendbuf(char *pbuf , int len);
void uart0_flush_rxbuf();

void timer1_init();
void timer1_disbale();
void timer1_enable();
u8 zh(u8 *g , u8  *d);

/*@@@@@@@@@@@  add  @@@@@@@@@@@@@@@@*/
void InitKey();

void rf_send( char *pbuf , int len);
void rf_receive_isr();

void uart0_init()
{
  CLKCONCMD &= ~0x40;                          //设置系统时钟源为32MHZ晶振
  while(CLKCONSTA & 0x40);                     //等待晶振稳定
  CLKCONCMD &= ~0x47; 
  PERCFG = 0x00;
  P0SEL |= 0x0C;
  U0CSR |= 0xC0;
  U0GCR |= 11;
  U0BAUD = 216;               // 115200
  UTX0IF = 1;
  
  URX0IE = 1;
}

void uart0_flush_rxbuf()
{
  serial_rxpos = 0;
  serial_rxlen = 0;
  memset(bufer,0,7);
}

void timer1_init()
{
  T1CTL = 0x0C;
  T1CCTL0 = 0x44;
  T1STAT = 0x00;
  T1IE = 1;
  T1CC0L = 250;
  T1CC0H = 0; 
}

void timer1_disbale()
{
  T1CTL &= ~( 1<< 1);
}

void timer1_enable()
{
  T1CTL |= ( 1 << 1 );
  T1STAT = 0x00;
  T1CNTH = 0;
  T1CNTL = 0;
}

void rf_init()
{
  FRMFILT0  = 0x0C;
  TXPOWER   = 0xD5;
  FREQCTRL  = 0x0B;
  
  CCACTRL0  = 0xF8;
  FSCAL1 =    0x00;                 
  TXFILTCFG = 0x09;
  AGCCTRL1 =  0x15;
  AGCCTRL2 =  0xFE;       
  TXFILTCFG = 0x09;                 
  
  RFIRQM0 |= (1<<6);
  IEN2 |= (1<<0);

  RFST = 0xED;
  RFST = 0xE3;
}

void rf_send( char *pbuf , int len)
{
  RFST = 0xE3;                   
  while( FSMSTAT1 & (( 1<<1 ) | ( 1<<5 ))); 
  
  RFIRQM0 &= ~(1<<6); 
  IEN2 &= ~(1<<0);

  RFST = 0xEE;
  RFIRQF1 = ~(1<<1);
  RFD = len + 2;        
  for (int i = 0; i < len; i++) 
  {
    RFD = *pbuf++;
  }

  RFST = 0xE9;
  while (!(RFIRQF1 & (1<<1)));
  RFIRQF1 = ~(1<<1);
  
  RFIRQM0 |= (1<<6);
  IEN2 |= (1<<0); 
}

void rf_receive_isr()
{
  int rf_rx_len = 0;
  int rssi = 0;
  char crc_ok = 0;
  
  rf_rx_len = RFD - 2;
  rf_rx_len &= 0x7F;
  for (int i = 0; i < rf_rx_len; i++)
  {
    rf_rx_buf[i] = RFD;
  }
  rssi = RFD - 73;
  crc_ok = RFD; 
  RFST = 0xED;
  if( crc_ok & 0x80 )
  {
    if(rf_rx_buf[0] == 'L')
    {
      uart0_sendbuf( rf_rx_buf , rf_rx_len-1);
      printf("[%d]",rssi);
    }
    else
    {
    }
  }
  else
  {
    printf("\r\nCRC Error\r\n");
  }
}

void main(void)
{
  P1DIR |= 0x01;                              // P2.0 
  
  LED = 1;

  EA = 0;
  
  InitKey();
  uart0_init();                               //115200
  timer1_init();
  rf_init();
  
  EA = 1;


  printf("Server start!\r\n");

  while(1)
  {
    if( is_serial_receive )
    {
      is_serial_receive = 0;
      uart0_sendbuf(bufer,sizeof(bufer));
      
      switch(bufer[3])
      {
      case 0x0C:
        LED = 0;
      break;
      case 0x18:
        LED = 1;
        break;
      default:
        break;
      }
      uart0_flush_rxbuf();
    }
  }
}

int putchar(int c)
{  
  while( !UTX0IF );  
  UTX0IF = 0;  
  U0DBUF = c;  
  return c;  
} 

void uart0_sendbuf(char *pbuf , int len)
{
  for( int i = 0 ; i < len ; i++)
  {
    while(!UTX0IF);  
    UTX0IF = 0;  
    U0DBUF = *pbuf;
    pbuf++;
  }
}

void InitKey()
{
  P1IEN |= 0X8;                 //P1_3 设置为中断方式 
  PICTL |= 0X1;                 // 下降沿触发   
  IEN2 |= 0X10;                 // 允许P1口中断; 
  P1IFG = 0x00;                 // P1口初始化中断标志位
  EA = 1; 
}
void delay_us(unsigned int i)
{
#define Yanshi 18
  unsigned int j=Yanshi;
  while(i--)
  {
    while(j--);
    j=Yanshi;
  }
}

u8 Pulse_Width_Check(void)
{
  u8 t=0;	 
  while(RDATA)
  {
    t++;
    delay_us(1);
    if(t==250)
    {
      return t;
    }           //超时溢出
  }
  return t;
}

u8 zh(unsigned char *g ,unsigned char *d)
{
  *g =
    ((*d&0x01)<<7)
    |((*d&0x02)<<5)
    |((*d&0x04)<<3)
    |((*d&0x08)<<1)
    |((*d&0x10)>>1)
    |((*d&0x20)>>3)
    |((*d&0x40)>>5)
    |((*d&0x80)>>7);
  return *g;
}


#pragma vector=URX0_VECTOR
__interrupt void UART0_ISR(void)
{
  URX0IF = 0;
  serial_rxbuf[serial_rxpos] = U0DBUF;
  serial_rxpos++;
  serial_rxlen++;
  
  timer1_enable();
}


#pragma vector=T1_VECTOR
__interrupt void Timer1_ISR(void)
{
  T1STAT &= ~( 1<< 0);
  is_serial_receive = 1;
  timer1_disbale();
}


#pragma vector=RF_VECTOR
__interrupt void rf_isr(void) 
{
  LED ^= 1;
  EA = 0;
  if (RFIRQF0 & ( 1<<6 ))
  {
    rf_receive_isr();
    S1CON = 0;
    RFIRQF0 &= ~(1<<6);
  }
  EA = 1;
}

#pragma vector = P1INT_VECTOR    //格式：#pragma vector = 中断向量，紧接着是中断处理程序
  __interrupt void P1_ISR(void) 
 {   
  delay_us(1);
  res=0;
  OK=0; 
  RODATA=0; 
  if((P1IFG&FLAG)==FLAG)
  {
    while(1)
    {
      if(RDATA)//有高脉冲出现
      {
          res=Pulse_Width_Check();//获得此次高脉冲宽度      
          if(res==250)
          {
            break;//非有用信号
          }
          if(res>=190&&res<230)
          {
            OK=1; //获得前导位(4.5ms)
            Remote_Odr=0;
          }
          else if(res>=105-15&&res<105+15)  //按键次数加一(2ms)
          {  							    		 
              Remote_Rdy=1;//接受到数据
              Remote_Cnt++;//按键次数增加
              break;
          }
          else if(res>=50&&res<75+15)
            RODATA=1;//1.5ms
          
          else if(res>=10&&res<50)
            RODATA=0;//500us
          
          if(OK)
          {
              Remote_Odr<<=1;
              Remote_Odr+=RODATA; 
              Remote_Cnt=0; //按键次数清零
          }
      }
    }
  }
  u8 t1,t2;
  u8 t3[4];
  memset(t3,0,4);
  t1=Remote_Odr>>24;                  //红外解码
  t2=(Remote_Odr>>16)&0xff;
  Remote_Rdy=0;                       //清除标记 
  zh((unsigned char *)&t3[0] , (unsigned char *)&t1);
  zh((unsigned char *)&t3[1] , (unsigned char *)&t2);
  t1=Remote_Odr>>8;
  t2=Remote_Odr;
  zh((unsigned char *)&t3[2] , (unsigned char *)&t1);
  zh((unsigned char *)&t3[3] , (unsigned char *)&t2);
  /*
  if(t1==(u8)~t2)
  { 
    t3[0] = t1;
    t3[1] = t2;
    t1=Remote_Odr>>8; 
    t2=Remote_Odr; 	
    if(t1==(u8)~t2)
    {
      t3[2] = t1;
      t3[3] = t2;
    }
  }*/
  
  bufer[0]='L';
  bufer[1]=t3[0];
  bufer[2]=t3[1];
  bufer[3]=t3[2];
  bufer[4]=t3[3];
  is_serial_receive = 1;

    
  delay_us(10000);
  P1IFG = 0;             //清中断标志 
  P1IF = 0;             //清中断标志 
 } 
