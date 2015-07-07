/*****************************************************
 *该类用于自动寻找计算机上的串口
 *
 *
 *
 ****************************************************/

#ifndef FINDCOMPUTERCOM_H
#define FINDCOMPUTERCOM_H

#include <QStringList>

class FindComputerCom
{
public:
    FindComputerCom();
    static void queryCom();
    static QString getcomm(int index,QString keyorvalue);      //搜索注册表
    //罗列本机所有串口
    static QStringList enumLocalCom();

private:

};

#endif // FINDCOMPUTERCOM_H
