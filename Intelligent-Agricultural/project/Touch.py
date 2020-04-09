#coding=utf8
'''***********************************************
*模块： 触摸模块
*主要功能： 1.智能家居总开关touch1
           2.睡眠模式开关touch2
***********************************************'''
import mysqlcon as mc
'''***********************************************
*函数： touch_check
*相关参数： num 选择总开关或者睡眠开关
*主要功能： 根据num来选择开关的开和关    
***********************************************'''
def touch_check(ser,num,flag,sleep_flag):
    #总开关触摸传感器
    if num=='01':
        flag=not flag
        if flag==True:
            f1='OPEN'
        else:
            f1='CLOSE'
        #更新状态
        con=mc.mysqlconnect()
        cursor=con.cursor()
        cursor.execute('update sensorinfo set flag=%s where id=0',f1)
        con.commit()        
    #节能开关传感器
    if num=='02':
        sleep_flag=not sleep_flag
        if sleep_flag==True:
            f2='OPEN'
        else:
            f2='CLOSE'
        con=mc.mysqlconnect()
        cursor=con.cursor()
        cursor.execute('update sensorinfo set sleep_flag=%s where id=0',f2)
        con.commit()   
    cursor.close()
    con.close()
