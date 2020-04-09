#coding=utf8
'''***********************************************
*模块： 数字调光模块
*主要功能： 根据光照强度进行自动LED调光
***********************************************'''
import binascii
import mysqlcon as mc
'''***********************************************
*函数： led_light
*相关参数： num 亮度调节值
*主要功能： 根据num亮度值进行LED调光   并存入数据库  
***********************************************'''
def gas(data):
    #更新状态 
    con=mc.mysqlconnect()
    cursor=con.cursor()
    if data[4:6]=="01":
        cursor.execute("update sensorinfo set led='NO' where id=0")
        con.commit()
        print "no combustible gas"
    if data[4:6]=="02":
        cursor.execute("update sensorinfo set led='YES' where id=0")
        con.commit()
        print "have combustible gas"
    cursor.close()
    con.close()
