#include "findcomputercom.h"

#include <QSettings>
#include <QDebug>
#include <qt_windows.h>

QStringList comVector;


FindComputerCom::FindComputerCom()
{
}

/***********************************************************************************************
*函数名 ：queryCom
*函数功能描述 ：
*函数参数 ：
*函数返回值 ：
*作者 ：
*函数创建日期 ：
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：
***********************************************************************************************/
void FindComputerCom::queryCom()
{
    comVector.clear();
    QString path =QString("HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM\\");
    QSettings *settings = new QSettings( path, QSettings::NativeFormat);
    QStringList key = settings->allKeys();
    int num = (int)key.size();
    QString value;
    for(int i=num-1; i>=0; i--)
    {
        value = getcomm(i,"value");
        if(value.left(3)=="COM")
        {
            qDebug()<<"COM:"<<value;
            comVector.append(value);
        }
    }
    delete settings;
    settings=NULL;
}


/***********************************************************************************************
*函数名 ：getcomm
*函数功能描述 ：
*函数参数 ：
*函数返回值 ：
*作者 ：
*函数创建日期 ：
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：
***********************************************************************************************/
QString FindComputerCom::getcomm(int index, QString keyorvalue)
{
    HKEY hKey;
    wchar_t keyname[256]; //键名数组
    char keyvalue[256];  //键值数组
    DWORD keysize,type,valuesize;
    int indexnum;

    QString commresult;
    if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE,TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"),0,KEY_READ,&hKey)!=0)
    {
        QString error="error";
        return error;
    }
    QString keymessage;//键名
    QString message;
    QString valuemessage;//键值
    indexnum = index;//要读取键值的索引号
    keysize=sizeof(keyname);
    valuesize=sizeof(keyvalue);
     if(::RegEnumValue(hKey,indexnum,keyname,&keysize,0,&type,(BYTE*)keyvalue,&valuesize)==0)//列举键名和值
     {
        for(DWORD i=0;i<keysize;i++)
        {
            message=keyname[i];
            keymessage.append(message);
        }
        for(DWORD j=0;j<valuesize;j++)
        {
            if(keyvalue[j]!=0x00)
            {
                valuemessage.append(keyvalue[j]);
            }
        }
        if(keyorvalue=="key")
        {
            commresult=keymessage;
        }
        else if(keyorvalue=="value")
        {
            commresult=valuemessage;
        }
        }
        else
        {
            commresult="nokey";
        }
        ::RegCloseKey(hKey);//关闭注册表
     return commresult;
}


/***********************************************************************************************
*函数名 ：enumLocalCom
*函数功能描述 ：查询本机可使用的串口
*函数参数 ：无
*函数返回值 ：串口名称的字符串列表
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
QStringList FindComputerCom::enumLocalCom()
{
    queryCom();
    return comVector;
}
