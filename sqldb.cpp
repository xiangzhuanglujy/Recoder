#include "sqldb.h"

#include <QMessageBox>
#include <QDebug>
#include <QDir>


Sqldb::Sqldb(QObject *parent) :
    QObject(parent)
{
    db=QSqlDatabase::addDatabase("QSQLITE");
    QDir dir;
    QString path=QString("%1/database.db").arg(dir.currentPath());
    qDebug()<<path;
    db.setDatabaseName(path);
    if(!db.open())
    {
        QMessageBox::critical(NULL,tr("Tips"),tr("connect error"));
    }
}


QSqlDatabase Sqldb::getDB()
{
//    qDebug()<<"db"<<db.isOpen();
    return db;
}

Sqldb::~Sqldb()
{
    if(db.isOpen())
        db.close();
}
