#coding=utf8
'''***********************************************
*模块： 电机及灯光控制模块
*主要功能： 1.控制电机的转动
           2.控制电机的状态灯光
***********************************************'''
import binascii
import time
import mysqlcon as mc
'''***********************************************
*函数： op_motor
*主要功能： 电机启动和停止
*相关参数： ser 发送端口连接句柄
           op  选择启动或停止操作
           num 选择电机号
***********************************************'''
def opp_motor(ser,op,num):
    con=mc.mysqlconnect()
    cursor=con.cursor()

    if op=='open':
        senddata='ccee'+num+'090b00000000000000000000ff'
        senddata=binascii.b2a_hex(senddata)
        try:
            ser.write(senddata)
            cursor.execute('update sensorinfo set motor%s_status=%s where id=0',(num[:-1],'OPEN'))
            con.commit()
            print 'motor '+num+' status start!'
        except:
            print 'motor '+num+' status close!'

    if op=='close':
        senddata='ccee'+num+'090b00000000000000000000ff'
        senddata=binascii.b2a_hex(senddata)
        try:
            ser.write(senddata)
            cursor.execute('update sensorinfo set motor%s_status=%s where id=0',(num[:-1],'CLOSE'))
            con.commit()
            print 'motor '+num+' status start!'
        except:
            print 'motor '+num+' status close!'
    cursor.close()
    con.close() 
'''***********************************************
*函数： status_motor
*主要功能： 电机状态灯光控制
*相关参数： ser 发送端口连接句柄
           data 电机回复数据
*功能： 1.当电机回复数据为EE CC NO 09 DD 09时
         开启LED1灯]
       2.当电机回复数据为EE CC NO 09 DD 0b时
         关闭LED1灯
        LED1灯为电机状态灯，电机开启时为亮状态
***********************************************'''
def status_motor(data):
    con=mc.mysqlconnect()
    cursor=con.cursor()
    if data[4:6]=='01' :
        cursor.execute("update sensorinfo set motor1_status='ILL CLOSE' where id=0")
        con.commit()
        print 'Motor ILL CLOSE !'
    if data[4:6]=='02' :
        cursor.execute("update sensorinfo set motor1_status='CW OPEN' where id=0")
        con.commit()
        print 'Motor CW !'
    if data[4:6]=='03':
        cursor.execute("update sensorinfo set motor1_status='CCW OPEN' where id=0")
        con.commit()
        print 'Motor CCW !'
    cursor.close()
    con.close() 

def status2_motor(temp,hum):
    con=mc.mysqlconnect()
    cursor=con.cursor()
    if float(temp)>30 or float(hum)>50:
        cursor.execute("update sensorinfo set motor2_status='TH OPEN' where id=0")
        con.commit()
        print 'Motor TEMP_HUM OPEN !'
    else:
        cursor.execute("update sensorinfo set motor2_status='TH CLOSE' where id=0")
        con.commit()
        
    cursor.close()
    con.close() 
'''***********************************************
*函数： time_motor
*主要功能： 电机在开启设定的时间后自动关闭
*相关参数： ser 发送端口连接句柄
           num 选择电机
           set_time 自动关闭延迟时间
***********************************************'''
def time_motor(ser,num,set_time):
    opp_motor(ser,'open',num)
    time.sleep(set_time)
    opp_motor(ser,'close',num)

