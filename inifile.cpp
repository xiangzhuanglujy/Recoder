#include "inifile.h"

#include <QSettings>

INIFile::INIFile()
{
}

/***********************************************************************************************
*函数名 ：readINI
*函数功能描述 ：读取ini中某个节点的键的值
*函数参数 ：node节点名称 key键名 fileName .ini文件名
*函数返回值 ：键值
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
QString INIFile::readINI(QString node, QString key, const QString &fileName)
{
    QSettings *configIniRead = new QSettings(fileName, QSettings::IniFormat);
    QString name=QString("/%1/%2").arg(node).arg(key);
    QString ipResult = configIniRead->value(name).toString();
    delete configIniRead;
    return ipResult;
}


/***********************************************************************************************
*函数名 ：writeINI
*函数功能描述 ：向ini文件写入某个键值
*函数参数 ：node节点名称 key键名 value键值 fileName .ini文件名
*函数返回值 ：键值
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
bool INIFile::writeINI(QString node, QString key, QString value, const QString &fileName)
{
    QSettings *configIniWrite = new QSettings(fileName, QSettings::IniFormat);
    QString name=QString("/%1/%2").arg(node).arg(key);
    configIniWrite->setValue(name,value);
    delete configIniWrite;
    return true;
}
