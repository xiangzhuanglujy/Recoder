#include "FILEWR.h"

#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QDateTime>


/***********************************************************************************************
*函数名 ：writeLogCSV
*函数功能描述 ：写log日志
*函数参数 ：str 日志内容
*函数返回值 ：成功true 失败false
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
bool FILEWARE::writeLogCSV(QString str)
{
    QString path=QString(".\\system\\%1-Main.txt").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    QFile file(path);
    if(file.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))
    {
        QString string=QString("%1-%2\n")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                .arg(str);
        file.write(string.toAscii());
        file.close();
        return true;
    }
    return false;
}


/***********************************************************************************************
*函数名 ：writeLogCSVThread
*函数功能描述 ：写线程日志
*函数参数 ：str 写入的内容
*函数返回值 ：成功true 失败false
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
bool FILEWARE::writeLogCSVThread(QString str)
{
    QString path=QString(".\\system\\%1-Thread.txt").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    QFile file(path);
    if(file.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))
    {
        QString string=QString("%1-%2\n")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                .arg(str);
        file.write(string.toAscii());
        file.close();
        return true;
    }
    return false;
}


/***********************************************************************************************
*函数名 ：writePositionC
*函数功能描述 ：保存数据到csv文件中
*函数参数 ：path路径 str写入的数据
*函数返回值 成功true 失败false
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
bool FILEWARE::writePositionC(const QString &path, QString str)
{
    QFile file(path);
    if(file.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))
    {
        QString string=QString("%1%2\n")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(str);
        file.write(string.toAscii());
        file.close();
        return true;
    }
    return false;
}


/***********************************************************************************************
*函数名 ：writeWaveLengthC
*函数功能描述 ：保存数据到csv文件中
*函数参数 ：path路径 str写入的数据
*函数返回值 成功true 失败false
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
bool FILEWARE::writeWaveLengthC(const QString &path, QString str)
{
    QFile file(path);
    if(file.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))
    {
        QString string=QString("%1%2\n")
//                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))

                .arg(str);
//        qDebug()<<string;
        file.write(string.toAscii());
        file.close();
        return true;
    }
    return false;
}


/***********************************************************************************************
*函数名 ：ReadCSV
*函数功能描述 ：读取csv文件内容
*函数参数 ：path文件路径
*函数返回值 ：字符串队列
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
QStringList FILEWARE::ReadCSV(const QString &path)
{
    QFile csvFile(path);
    QStringList list;
    if(csvFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&csvFile);
        while(!stream.atEnd())
        {
            list.push_back(stream.readLine());
        }
        csvFile.close();
    }
    return list;
}
