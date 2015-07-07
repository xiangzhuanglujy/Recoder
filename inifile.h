/*********************************************************************
 *ini文件操作类
 *
 *
 *
 *
 *
 *
 *
 *
 ********************************************************************/

#ifndef INIFILE_H
#define INIFILE_H

#include <QString>

#define DEFAULT_INI_PATH            ".\\config.ini"		//默认ini文件地址

class INIFile
{
public:
    INIFile();

    //读取ini某个键值的值
    static QString readINI(QString node,QString key,const QString& fileName=DEFAULT_INI_PATH);
    //向ini写进某个键值
    static bool writeINI(QString node,QString key,QString value,const QString& fileName=DEFAULT_INI_PATH);
};

#endif // INIFILE_H
