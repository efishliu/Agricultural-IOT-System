#include "iocc2530.h"
//#include "MT_UART.h"
//#include "MT_APP.h"
//#include "MT.h"
#define uint unsigned int 
#define uchar unsigned char
void Start_I2c(void);                  //起始信号
void  Stop_I2c(void);                    //停止信号
void BH1750_SendACK(void);       //应答ACK
void BH1750_SendNCK(void) ;    //应答ACK
uchar RcvByte(void);
void  SendByte(unsigned char c);  //IIC单个字节写
uchar ISendByte(uchar sla,uchar c);//
uchar IRcvByte(uchar sla,uchar *c); //IIC单个字节读
uchar IRcvStrExt(uchar sla,uchar *s,uchar no);        //连续的读取内部寄存器数据 
void Init_BH1750(void);//初始
//void conversion(uint temp_data) ;
void Delay_us1(uint timeout);
void Delay_ms1(uint Time);//n ms延时
void light(void);

#define DPOWR  0X00         //断电
#define POWER  0X01         //SHANG DIAN
#define RESET    0X07         //CHONG ZHI
#define CHMODE  0X10        //连续H分辨率
#define CHMODE2 0X11         //连续H分辨率2
#define CLMODE   0X13           //连续低分辨
#define H1MODE   0X20           //一次H分辨率
#define H1MODE2 0X21          //一次H分辨率2
#define L1MODE    0X23           //一次L分辨率模式
#define  SCL P0_0      //IIC时钟引脚定义
#define  SDA P1_2     //IIC数据引脚定义
#define	 SlaveAddress   0x46 //定义器件在IIC总线中的从地址,根据ALT  ADDRESS地址引脚不同修改
                              //ALT  ADDRESS引脚接地时地址为0xA6，接电源时地址为0x3A
uchar ack;
uchar buf[2];                         //接收数据缓存区  
//uint sun;
//uchar lx[5];//ge,shi,bai,qian,wan;            //显示变量
//float s;

//-----------------------------------

//void Initial() //系统初始化
//{
  //CLKCONCMD = 0x80;      //选择32M振荡器
  //while(CLKCONSTA&0x40); //等待晶振稳定
  //UartInitial();         //串口初始化
  //P1SEL &= 0xfb;         //DS18B20的io口初始化
//}

/****************************
        延时函数
*****************************/
void Delay_us1(uint timeout) //1 us延时uint16 timeout )
{
  while (timeout--)
  {
    asm("NOP");
    asm("NOP");
    asm("NOP");
  } 
}


void Delay_ms1(uint Time)//n ms延时
{
  unsigned char i;
  while(Time--)
  {
    for(i=0;i<100;i++)
     Delay_us1(10);
  }
}
//*********************************************************

/**************************************
起始信号
**************************************/
void Start_I2c()
{
    P1DIR|=(1<<2);
    P0DIR|=1;
    SDA = 1;                    //拉高数据线
    SCL = 1;                    //拉高时钟线
    Delay_us1(5);                 //延时
    SDA = 0;                    //产生下降沿
    Delay_us1(5);                 //延时
    SCL = 0;                    //拉低时钟线
}


/**************************************
停止信号
**************************************/
void  Stop_I2c()
{
     P1DIR|=(1<<2);
    P0DIR|=1;
    SDA = 0;                    //拉低数据线
    SCL = 1;                    //拉高时钟线
    Delay_us1(5);               //延时
    SDA = 1;                    //产生上升沿
    Delay_us1(5);                //延时
}

/**************************************
发送应答信号
入口参数:ack (0:ACK 1:NAK)
**************************************/
void BH1750_SendACK()
{
     P1DIR|=(1<<2);
    P0DIR|=1;
    SDA = 0;                  //写应答信号
    SCL = 1;                    //拉高时钟线
    Delay_us1(5);                //延时
    SCL = 0;                    //拉低时钟线
    SDA = 1;  
}

void BH1750_SendNCK()
{
      P1DIR|=(1<<2);
    P0DIR|=1;
    SDA = 1;                  //写应答信号
    SCL = 1;                    //拉高时钟线
    Delay_us1(5);                //延时
    SCL = 0;                    //拉低时钟线
    SDA = 0; 
}

/*----------------------------------------------------------------              
I2C写入一个8位二进制数，高位在前低位在后
------------------------------------------------------------------*/
void  SendByte(unsigned char c)
{
 unsigned char BitCnt;
  P1DIR|=(1<<2);
    P0DIR|=1;
 for(BitCnt=0;BitCnt<8;BitCnt++)  //要传送的数据长度为8位
    {
     
      if((c<<BitCnt)&0x80) SDA=1;   //判断发送位
       else  SDA=0;                
     SCL=1;               //置时钟线为高，通知被控器开始接收数据位
     Delay_us1(5);         //保证时钟高电平周期大于4μ      
     SCL=0; 
    }  
     SDA=1;
    Delay_us1(5);
    P1DIR&=~(1<<2);          //8位发送完后释放数据线，准备接收应答位
    SCL=1;
    Delay_us1(5);
    if(SDA==1)ack=0;     
       else ack=1;        //判断是否接收到应答信号，“1”：没有
    SCL=0;
}

/*******************************************************************
                 向无子地址器件发送字节数据函数               
函数原型: bit  ISendByte(uchar sla,ucahr c);  
功能:     从启动总线到发送地址，数据，结束总线的全过程,从器件地址sla.
          如果返回1表示操作成功，否则操作有误。
注意：    使用前必须已结束总线。
********************************************************************/
uchar ISendByte(uchar sla,uchar c)
{
   Start_I2c();               /*启动总线*/
   SendByte(sla);             /*发送器件地址*/
   if(ack==0)return(0);
   SendByte(c);               /*发送数据*/
   if(ack==0)return(0);
   Stop_I2c();                /*结束总线*/ 
   return(1);
}

/*******************************************************************
I2C读取一个8位二进制数，也是高位在前低位在后  
****************************************************************/	
uchar RcvByte()
{
  unsigned char retc;
  unsigned char BitCnt;
   retc=0; 
   P1DIR&=~(1<<2);         //置数据线为输入方式
  for(BitCnt=0;BitCnt<8;BitCnt++)
      {       
        SCL=0;      
        Delay_us1(5); //时钟低电平周期大于4.7us
        if(SDA==1)retc=retc+1; //读数据位,接收的数据位放入retc中
        SCL=1;
        retc=retc<<1;
        Delay_us1(5);  
      }   
   SCL=0; 
  return(retc);
}

//*********************************************************
//连续读出BH1750内部数据
//*********************************************************
uchar IRcvStrExt(uchar sla,uchar *s,uchar no)
{   uchar i;	
    Start_I2c();                         //起始信号
    SendByte(sla+1);        //发送设备地址+读信号
    if(ack==0)return(0);
      for (i=0; i<no-1; i++)                      //连续读取6个地址数据，存储中BUF
      {
        *s=RcvByte();      
        BH1750_SendACK();                //回应ACK
        s++;
      }		
        *s=RcvByte();
        BH1750_SendNCK();   //最后一个数据需要回NOACK             
        Stop_I2c();   
        return(1);
}



//初始化BH1750，根据需要请参考pdf进行修改****
void Init_BH1750()
{
   P1DIR|=(1<<2);
   P0DIR|=1;
   ISendByte(0x46,0x01);  
}

//*********************************************************
/*void conversion(uint temp_data)  //  数据转换出 个，十，百，千，万
{  
    lx[0]=temp_data/10000+0x30 ;
    temp_data=temp_data%10000;   //取余运算
     lx[1]=temp_data/1000+0x30 ;
    temp_data=temp_data%1000;    //取余运算
     lx[2]=temp_data/100+0x30  ;
    temp_data=temp_data%100;     //取余运算
     lx[3]=temp_data/10+0x30 ;
    temp_data=temp_data%10;     //取余运算
     lx[4]=temp_data+0x30; 	
}*/
//*********************************************************
//主程序********
//*********************************************************
void light()
{  
    uchar *p=buf;
    Delay_ms1(100);	    //延时100ms	
    Init_BH1750();       //初始化BH1750
    ISendByte(0x46,0x01);   // power on
    ISendByte(0x46,0X20);   // H- resolution mode
    //uchar data[6]="Light="; //串口提示符
   // char data1[2]="lx"; //单位
    Delay_ms1(180);              //延时180ms
    IRcvStrExt(0x46,p,2);       //连续读出数据，存储在BUF中
  /*  sunh=buf[0];
    sun=(sun<<8)+buf[1];//合成数据，即光照数据  
    s=(float)sun/1.2;    
    conversion((uint)s);         //计算数据和显示*/
} 



