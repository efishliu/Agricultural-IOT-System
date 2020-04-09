#coding=utf8
'''***********************************************
*模块： 协调器
*主要功能： 1.作为其他传感器的网关，与pc机进行连接交互
           2.采集管理各传感器信息
***********************************************'''
import serial
import binascii 
import mysqlcon as mc
import time
'''***********************************************
*函数： CoorConnectPc
*主要功能： 与PC机进行端口连接
*相关参数： 端口：COM3
           波特率：115200
           数据位：8
           停止位：1
           校验位：None           
***********************************************'''
def CoorConnectPc():
    ser = serial.Serial(
        port='COM9',
        baudrate=115200,
        bytesize = 8,
        stopbits = 1,
        parity = 'N',
    )
    return ser

'''***********************************************
*函数： ReadPort
*主要功能： 端口数据读取           
***********************************************'''
def ReadPort(ser):
    data = ''
    #data=str(binascii.b2a_hex(ser.read(16)))
    data=ser.read(16)
    while 1 :    
        if (data)!='':
            print data
            insertlog(data)
            break
    return data

'''***********************************************
*函数： insertlog
*主要功能：   端口日志记录        
***********************************************'''
def insertlog(data):
    con=mc.mysqlconnect()
    cursor=con.cursor()
    cursor.execute('insert into log(addtime,data) values(%s,%s)',(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()),data))
    con.commit()
    cursor.close()
    con.close()
    #linux mysql
    linux_con=mc.linux_mysqlconnect()
    linux_cursor=linux_con.cursor()
    linux_cursor.execute('insert into log(addtime,data) values(%s,%s)',(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()),data))
    linux_con.commit()
    linux_cursor.close()
    linux_con.close()


        
        