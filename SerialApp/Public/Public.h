#ifndef PUBLIC_H
#define PUBLIC_H

#ifdef __cplusplus
extern "C"
{
#endif

#define    MSG_PERIOD    0xAA
#define    MSG_RSP       0xDD
  
typedef enum
{
  Coor = 0,   //网关
  Hall,       //霍尔
  PhotoRes,   //光照
  TempAndHum, //温湿度
  Shake,      //震动
  Reed,       //干簧管
  Accele,     //加速度
  Smoke,      //烟雾
  Doppler,    //多普勒
  Motor,      //电机
  LED_PWM,    //LED调光
  Sound,      //声音
  Voltage,    //电压  保留
  Current,    //电流  保留
  Touch,      //触摸
  Ultrasound, //超声波
  RFID_1356,  //13.56M射频卡
  RFID_125K,  //125K射频卡
  Flame,      //火焰
  Particle,   //微粒
  Color,      //颜色
  Gyroscope,  //陀螺仪
  IR_Code,    //红外编解码
  Alcohol,     //酒精
  Relay,
  RFID_900M
}DeviceAddrList;

typedef struct
{
  uint8 Header_1;
  uint8 Header_2;
  uint8 NodeSeq;
  uint8 NodeID;
  uint8 Command;
  uint8 Data[10];
  uint8 Tailer;
}UART_Format;


#ifdef __cplusplus
}
#endif

#endif 
