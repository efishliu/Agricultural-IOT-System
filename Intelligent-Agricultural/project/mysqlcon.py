#coding=utf8
'''***********************************************
*模块： mysqlcon
*主要功能： 本地mysql数据库连接
***********************************************'''
import pymysql
def mysqlconnect():
    con=pymysql.connect(
        host='localhost',
        port=3306,
        user='root',
        passwd='liugang666',
        database='sensor',
        charset='utf8',
    )
    return con
'''***********************************************
*模块： linux_mysqlcon
*主要功能： 远程mysql数据库连接
***********************************************'''
def linux_mysqlconnect():
    con=pymysql.connect(
        host='59.110.159.69',
        port=3306,
        user='root',
        passwd='liugang666',
        database='sensor',
        charset='utf8',
    )
    return con