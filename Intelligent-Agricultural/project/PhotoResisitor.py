#coding=utf8
'''***********************************************
*模块： 光敏模块
*主要功能： 1.采集光照强度
           2.将采集到的光照强度保存到数据库中
***********************************************'''
import mysqlcon as mc
import time
import PWM as pwm 
'''***********************************************
*函数： temp_hum_store
*主要功能：  将光敏数据进行转换，并存入数据库中
*返回值： 光照强度      
***********************************************'''
def photores_store(data):
    ph=float(int(data[10:12],16))
    pl=float(int(data[12:14],16))
    p=data[4:9]
    con=mc.mysqlconnect()
    cursor=con.cursor()
    cursor.execute('insert into photores(illumination,addtime) values(%s,%s)',(str(p),time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())))
    con.commit()
    #更新状态  
    cursor.execute('update sensorinfo set ill=%s where id=0',str(p))
    con.commit()

    cursor.close()
    con.close()   
    #linux mysql
    linux_con=mc.linux_mysqlconnect()
    linux_cursor=linux_con.cursor()
    linux_cursor.execute('insert into photores(illumination,addtime) values(%s,%s)',(str(p),time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())))
    linux_con.commit()
    linux_cursor.close()
    linux_con.close()
    print 'light: ',p,'xl'
    return p

'''***********************************************
*函数： dimming
*相关参数： light 光照强度
*主要功能： 根据光照强度进行LED调光     
***********************************************'''
def dimming(ser,light):
    if light<50 :
        pwm.led_light(ser,'09')
    elif light<100:
        pwm.led_light(ser,'07')
    elif light<200:
        pwm.led_light(ser,'05')
    elif light<300:
        pwm.led_light(ser,'03')
    else:
        pwm.led_light(ser,'00')

    
    
        
        
    