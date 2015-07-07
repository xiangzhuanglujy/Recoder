#include "mysql.h"

#include <QDebug>
#include <QSqlError>
#include <QDir>
#include <QFile>
#include <QDateTime>

QSqlDatabase sqldb;                     //数据库全局变量
DATABASETYPE dbType;

MySQL::MySQL()
{
}

void MySQL::addMysql()
{
    dbType=Q_MYSQL;
    sqldb=QSqlDatabase::addDatabase("QMYSQL");
    sqldb.setHostName("127.0.0.1");
    sqldb.setPort(3306);
    sqldb.setUserName("root");
    sqldb.setPassword("601482");
    sqldb.setDatabaseName("family");
}

void MySQL::addSqlite()
{
    dbType=Q_SQLITE;

    sqldb=QSqlDatabase::addDatabase("QSQLITE");
    sqldb.setDatabaseName("Data.db");
}

void MySQL::addSqlServer()
{
    dbType=Q_SQLSERVER;

    sqldb=QSqlDatabase::addDatabase("QODBC");
    // 注意,对于express版本数据库, 一定要加\\sqlexpress这种后缀
    QString dsn="DRIVER={SQL SERVER\\sqlexpress};SERVER=192.168.44.1\\sqlexpress;DATABASE=sqlscada";
    sqldb.setDatabaseName(dsn);
    sqldb.setUserName("sa");
    sqldb.setPassword("scada");
}

void MySQL::addQODBC(const QString &path)
{
    dbType=Q_ACCESS;

    sqldb = QSqlDatabase::addDatabase("QODBC");
//    QString accessFile = QDir::toNativeSeparators(path);
    QString accessFile =path;

    qDebug()<<"sql path"<<path;
    QString dsn = QString("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};FIL={MS Access};DBQ=%1").arg(accessFile);
    sqldb.setDatabaseName(dsn);
}

bool MySQL::connect()
{
    bool bret=sqldb.open();
    if(bret)
    {
        qDebug()<<"database open success";
        return bret;
    }
    QFile file("errorLog.txt");
    if(file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
        qDebug()<<"database:"<<sqldb.lastError();
        QTextStream stream(&file);
        stream<<"database open error"<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")<<endl;
        stream<<sqldb.lastError().databaseText()<<endl;
        stream<<sqldb.lastError().driverText()<<endl;
        file.close();
    }
    return bret;
}

void MySQL::disconnect()
{
    if(sqldb.isOpen())
        sqldb.close();

    switch(dbType)
    {
    case Q_MYSQL:
        QSqlDatabase::removeDatabase("QMYSQL");
        break;
    case Q_SQLITE:
        QSqlDatabase::removeDatabase("QSQLITE");
        break;
    default:
        QSqlDatabase::removeDatabase("QODBC");
        break;
    }
}
