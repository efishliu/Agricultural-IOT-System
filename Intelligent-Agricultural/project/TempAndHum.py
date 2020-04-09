#coding=utf8
'''***********************************************
*模块： 温湿度模块
*主要功能： 1.采集温度和湿度
           2.将采集到的温度湿度保存到数据库中
***********************************************'''
import mysqlcon as mc
import time
import Motor as mt
'''***********************************************
*函数： temp_hum_store
*主要功能：  将温湿度数据进行转换，并存入数据库中      
***********************************************'''
def temp_hum_store(data):
    temp=data[4:6]
    hum=data[6:8]
    con=mc.mysqlconnect()
    cursor=con.cursor()
    cursor.execute('insert into temp_hum(temp,hum,addtime) values(%s,%s,%s)',(str(temp),str(hum),time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())))
    con.commit()
    #更新状态

    cursor.execute('update sensorinfo set temp=%s,hum=%s where id=0',(str(temp),str(hum)))
    con.commit()
    cursor.close()
    con.close()     
    #linux mysql
    linux_con=mc.linux_mysqlconnect()
    linux_cursor=linux_con.cursor()
    linux_cursor.execute('insert into temp_hum(temp,hum,addtime) values(%s,%s,%s)',(str(temp),str(hum),time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())))
    linux_con.commit()
    linux_cursor.close()
    linux_con.close() 
    temp_hum={'temp':temp,'hum':hum}
    #print 'temp: ',temp,' hum: ',hum
    return temp_hum

'''***********************************************
*函数： judge_temp_hum
*相关参数：  ser 端口句柄
            now_temp_hum 现在的温度和湿度
            set_temp_hum 设定的触发温湿度
*主要功能：  当温度大于设定值打开电机2空调
            小于设定温度时关闭
            当湿度低于设定值时打开电机1加湿器
            大于设定值时关闭     
***********************************************'''
def judge_temp_hum(ser,now_temp_hum,set_temp_hum):
    #开空调
    if now_temp_hum['temp'] > set_temp_hum['min_temp']:
        mt.opp_motor(ser,'open','02')
    else:
        mt.opp_motor(ser,'close','02')
    #开加湿器
    if now_temp_hum['hum'] < set_temp_hum['min_hum']:
        mt.opp_motor(ser,'open','01')
    else:
        mt.opp_motor(ser,'close','01')