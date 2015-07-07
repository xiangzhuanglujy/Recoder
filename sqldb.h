#ifndef SQLDB_H
#define SQLDB_H

#include <QObject>
#include <QSqlDatabase>

class Sqldb : public QObject
{
    Q_OBJECT
public:
    explicit Sqldb(QObject *parent = 0);
    QSqlDatabase getDB();
    ~Sqldb();

signals:
    
public slots:
    
private:
    QSqlDatabase db;//建立数据库的文件描述符
};

#endif // SQLDB_H
