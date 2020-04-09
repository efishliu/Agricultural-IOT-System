#coding=utf8
'''***********************************************
*模块： 图形界面
*主要功能： 1.实时显示各个传感器的信息
           2.实时刷新温湿度信息
           3.实时显示传感器曲线
***********************************************'''
import matplotlib
matplotlib.use('TkAgg')
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg 
from matplotlib.figure import Figure
from  Tkinter import *
import mysqlcon as mc
import pandas as pd
import time
'''***********************************************
*函数： flush
*主要功能： 1.从数据库中读取各传感器状态
           2.将数据进行实时刷新
*相关参数： flag,sleep_flag 触摸开关
           set_temp,set_hum 设定温湿度
           temp,hum,ill 温湿度光照强度
           mo1_st,mo2_st   空调加湿器
***********************************************'''
def flush(flag,set_temp,set_hum,temp,hum,ill,mo1_st,mo2_st,gas,root):
    con=mc.mysqlconnect()
    realtime_data_sql='select * from sensorinfo'
    realtime_data=pd.read_sql(realtime_data_sql,con)
    con.close()
    flag.set(realtime_data['flag'][0])
    #sleep_flag.set(realtime_data['sleep_flag'][0])
    set_temp.set(realtime_data['set_temp'][0])
    set_hum.set(realtime_data['set_hum'][0])
    temp.set(realtime_data['temp'][0])
    hum.set(realtime_data['hum'][0])
    ill.set(realtime_data['ill'][0]) 
    mo1_st.set(realtime_data['motor1_status'][0])
    mo2_st.set(realtime_data['motor2_status'][0])
    gas.set(realtime_data['led'][0]) 
    #500ms刷新数据
    root.after(500,flush,flag,set_temp,set_hum,temp,hum,ill,mo1_st,mo2_st,gas,root)

'''***********************************************
*函数： draw
*主要功能： 绘制并刷新温湿度和光照强度曲线
*相关参数： root 图形根界面
***********************************************'''
def draw(root):
    #读取温湿度光照强度数据10条
    con=mc.mysqlconnect()
    recent_10_temphum_data_sql='select temp,hum,addtime from temp_hum group by id desc limit 10'
    recent_10_temphum_data=pd.read_sql(recent_10_temphum_data_sql,con)
    recent_10_ill_data_sql='select illumination,addtime from photores group by id desc limit 10'
    recent_10_ill_data=pd.read_sql(recent_10_ill_data_sql,con)
    con.close()
    #创建画板并进行图形绘制
    f = Figure(figsize=(10,4), dpi=100)
    temp_photo=f.add_subplot(221)
    hum_photo=f.add_subplot(222)
    ill_photo=f.add_subplot(223)

    th_x=recent_10_temphum_data['addtime']
    temhum_x=[th_x[9],th_x[8],th_x[7],th_x[6],th_x[5],th_x[4],th_x[3],th_x[2],th_x[1],th_x[0]]

    ty=recent_10_temphum_data['temp']
    temp_y=[ty[9],ty[8],ty[7],ty[6],ty[5],ty[4],ty[3],ty[2],ty[1],ty[0]]

    hy=recent_10_temphum_data['hum']
    hum_y=[hy[9],hy[8],hy[7],hy[6],hy[5],hy[4],hy[3],hy[2],hy[1],hy[0]]

    ix=recent_10_ill_data['addtime']
    i_x=[ix[9],ix[8],ix[7],ix[6],ix[5],ix[4],ix[3],ix[2],ix[1],ix[0]]

    iy=recent_10_ill_data['illumination']
    i_y=[iy[9],iy[8],iy[7],iy[6],iy[5],iy[4],iy[3],iy[2],iy[1],iy[0]]   

    #设置横纵坐标
    temp_photo.plot(th_x,temp_y)
    hum_photo.plot(th_x,hum_y)
    ill_photo.plot(i_x,i_y)

    temp_photo.set_xlabel('time')
    temp_photo.set_ylabel('temp')
    hum_photo.set_xlabel('time')
    hum_photo.set_ylabel('hum')
    ill_photo.set_xlabel('time')
    ill_photo.set_ylabel('illumination')

    #图像展示
    temp_photo.grid() 
    hum_photo.grid() 
    ill_photo.grid() 
    dataPlot = FigureCanvasTkAgg(f, master=root)
    dataPlot.show()
    dataPlot.get_tk_widget().pack()

    #root.after(5000,draw,root)

'''***********************************************
*函数： windows
*主要功能： 主窗口
***********************************************'''
def windows():
    root=Tk()
    root.title('农业检测')
    #root.geometry("400x300")

    #设置状态更新
    flag=StringVar()
    #sleep_flag=StringVar()
    set_temp=StringVar()
    set_hum=StringVar()
    mo1_st=StringVar()
    mo2_st=StringVar()
    temp=StringVar()
    hum=StringVar()
    ill=StringVar()
    gas=StringVar()


    Label(root, text='基于Zigbee的智能农业大棚环境检测系统',font=('宋体', 20)).pack(pady=10)

    fm_flag_set=Frame(root)

    flaglabel_name=Label(fm_flag_set,text='总开关',font=('宋体', 15))
    flaglabel_name.pack(side=LEFT,pady=10)
    flaglabel_val=Label(fm_flag_set,textvariable=flag,font=('宋体', 15))
    flaglabel_val.pack(side=LEFT,padx=30)

    mo2_stlabel_name=Label(fm_flag_set,text='电机1',font=('宋体', 15))
    mo2_stlabel_name.pack(side=LEFT,pady=10)
    mo2_stlabel_val=Label(fm_flag_set,textvariable=mo2_st,font=('宋体', 15))
    mo2_stlabel_val.pack(side=LEFT,padx=30)

    mo1_stlabel_name=Label(fm_flag_set,text='电机2',font=('宋体', 15))
    mo1_stlabel_name.pack(side=LEFT,pady=10)
    mo1_stlabel_val=Label(fm_flag_set,textvariable=mo1_st,font=('宋体', 15))
    mo1_stlabel_val.pack(side=LEFT,padx=30)
    fm_flag_set.pack(side=TOP,pady=10)

    #设定温湿度显示
    fm_flag_set2=Frame(root)
    set_templabel_name=Label(fm_flag_set2,text='设定温度阈值',font=('宋体', 15))
    set_templabel_name.pack(side=LEFT,pady=10)
    set_templabel_val=Label(fm_flag_set2,textvariable=set_temp,font=('宋体', 15))
    set_templabel_val.pack(side=LEFT,padx=30)

    set_humlabel_name=Label(fm_flag_set2,text='设定湿度阈值',font=('宋体', 15))
    set_humlabel_name.pack(side=LEFT,pady=10)
    set_humlabel_val=Label(fm_flag_set2,textvariable=set_hum,font=('宋体', 15))
    set_humlabel_val.pack(side=LEFT,padx=30)
    fm_flag_set2.pack(side=TOP,pady=10)

    #温度-湿度-光照强度模块实时展示
    fm1=Frame(root)
    templabel_name=Label(fm1,text='温度',font=('宋体', 15))
    templabel_name.pack(side=LEFT,pady=10)
    templabel_val=Label(fm1,textvariable=temp,font=('宋体', 15))
    templabel_val.pack(side=LEFT,padx=30)

    humlabel_name=Label(fm1,text='湿度',font=('宋体', 15))
    humlabel_name.pack(side=LEFT,pady=10)
    humlabel_val=Label(fm1,textvariable=hum,font=('宋体', 15))
    humlabel_val.pack(side=LEFT,padx=30)

    illlabel_name=Label(fm1,text='光照强度',font=('宋体', 15))
    illlabel_name.pack(side=LEFT,pady=10)
    illlabel_val=Label(fm1,textvariable=ill,font=('宋体', 15))
    illlabel_val.pack(side=LEFT,padx=30)

    gaslabel_name=Label(fm1,text='有可燃气体',font=('宋体', 15))
    gaslabel_name.pack(side=LEFT,pady=10)
    gaslabel_val=Label(fm1,textvariable=gas,font=('宋体', 15))
    gaslabel_val.pack(side=LEFT,padx=30)
    fm1.pack(side=TOP,pady=15)

    #刷新各传感器状态和图像
    flush(flag,set_temp,set_hum,temp,hum,ill,mo1_st,mo2_st,gas,root)
    draw(root)
    root.update_idletasks()
    root.mainloop()
