/*************************************************************************
 *qt数据库驱动类
 *version:1.0
 *author:lujy
 *date:2015/01/23
 *************************************************************************/


#ifndef MYSQL_H
#define MYSQL_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRelationalTableModel>
#include <QSqlTableModel>

typedef enum _DATABASETYPE
{
    Q_MYSQL,
    Q_SQLITE,
    Q_SQLSERVER,
    Q_ACCESS
}DATABASETYPE;


class MySQL
{
public:
    MySQL();

    static void addMysql();                         //加载mysql驱动
    static void addSqlite();                        //加载sqlite驱动
    static void addSqlServer();                     //加载sqlserver驱动
    static void addQODBC(const QString&);           //加载ACCESS驱动
    static bool connect();                          //打开数据库
    static void disconnect();                       //关闭数据库

private:
};

extern QSqlDatabase sqldb;                          //公用数据库连接对象

#endif // MYSQL_H
