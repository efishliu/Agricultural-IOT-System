/****************************************************************************
* 文 件 名: main.c
* 作    者: Andy
* 修    订: 2013-01-10
* 版    本: 1.0
* 描    述: 将采集到的温湿度通过串口发送到串口调试助手上显示 115200 8N1
****************************************************************************/
#include <ioCC2530.h>
#include <string.h>
#include "UART.H" 
#include "DHT11.H" 

/****************************************************************************
* 程序入口函数
****************************************************************************/
void main(void)
{  
    uchar temp[3]; 
    uchar humidity[3];   
    uchar strTemp[13]="Temperature:";
    uchar strHumidity[10]="Humidity:";
    
    Delay_ms(1000);          //让设备稳定
    InitUart();              //串口初始化
    while(1)
    {         
        memset(temp, 0, 3);
        memset(humidity, 0, 3);
        
        DHT11();             //获取温湿度

        //将温湿度的转换成字符串
        temp[0]=wendu_shi+0x30;
        temp[1]=wendu_ge+0x30;
        humidity[0]=shidu_shi+0x30;
        humidity[1]=shidu_ge+0x30;
        
        //获得的温湿度通过串口输出到电脑显示
        //UartSendString(strTemp, 12);
        UartSendString(temp, 2);
        UartSendString("   ", 3);
        //UartSendString(strHumidity, 9);
        UartSendString(humidity, 2);
        UartSendString("\n", 1);
        
        Delay_ms(2000);  //延时，2S读取1次 
    }
}
