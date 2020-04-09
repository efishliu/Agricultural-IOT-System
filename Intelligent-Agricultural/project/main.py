#coding=utf-8
'''***********************************************
*模块： 程序主入口
***********************************************'''
import time
import string
import mysqlcon as mc
import Coordinator as ci
import Motor as mt
import TempAndHum as th
import PhotoResisitor as pr
import PWM as G
import thread
from  Display import *

'''***********************************************
*函数： dealdata
*主要功能： 1.实时接收数据并存入数据库
           2.接收到有效数据后进行状态更新
***********************************************'''
def dealdata():

    set_temp_hum={}
    set_temp_hum['min_temp']='30'
    set_temp_hum['min_hum']='50'

    #设置温度值更新
    con=mc.mysqlconnect()
    cursor=con.cursor()
    cursor.execute('update sensorinfo set set_temp=%s,set_hum=%s where id=0',(set_temp_hum['min_temp'],set_temp_hum['min_hum']))
    con.commit()
    cursor.close()
    con.close()

    flag=True

    ser=ci.CoorConnectPc()
    while 1 :
        data=ci.ReadPort(ser)

        if data[0:2]=='SD':
            #温湿度模块
            if data[2:4]=='01':
                now_temp_hum=th.temp_hum_store(data)
                print 'now temp hum:',now_temp_hum['temp'],now_temp_hum['hum']
                mt.status2_motor(now_temp_hum['temp'],now_temp_hum['hum'])
                #print 'set temp hum:',set_temp_hum['min_temp'],set_temp_hum['min_hum']
                #if flag==True:
                    #th.judge_temp_hum(ser,now_temp_hum,set_temp_hum)
            #光敏模块
            if data[2:4]=='02':
                light=pr.photores_store(data)
            #电机模块
            if data[2:4]=='03':
                mt.status_motor(data)
            #可燃气体
            if data[2:4]=='03':
                G.gas(data)
                

'''***********************************************
*函数： main
*主要功能： 程序主入口函数
***********************************************'''
if __name__ == '__main__':
    #多线程处理数据函数
    thread.start_new_thread(dealdata,())
    #图形界面
    time.sleep(15)
    thread.start_new_thread(windows,())
    time.sleep(10000)  