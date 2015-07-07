#include "ctltemptd.h"

#include "serial/win_qextserialport.h"

#include <QSettings>
#include <QMessageBox>
#include <QDebug>


CtlTempTd::CtlTempTd(QObject *parent) :
    QThread(parent)
{
    QSettings *configIniRead = new QSettings(".\\Config.ini", QSettings::IniFormat);
    QString ipResult = configIniRead->value("/COM/com").toString();
    delete configIniRead;

    myCom = new Win_QextSerialPort(ipResult,QextSerialBase::EventDriven);
    connect(myCom,SIGNAL(readyRead()),this,SLOT(readCom()));
    if(myCom->open(QIODevice::ReadWrite))
    {
        myCom->setBaudRate(BAUD9600);
        myCom->setDataBits(DATA_8);
        myCom->setParity(PAR_NONE);
        myCom->setStopBits(STOP_2);
        myCom->setFlowControl(FLOW_OFF); //设置数据流控制，我们使用无数据流控制的默认设置
        myCom->setTimeout(10); //设置延时
    }
    else
    {
        //QMessageBox::information(0,"提示","温箱串口打开失败");
    }
}

CtlTempTd::~CtlTempTd()
{
    if(myCom->isOpen())
        myCom->close();
    delete myCom;
}

void CtlTempTd::run()
{
}

void CtlTempTd::readCom()
{
    QByteArray array=myCom->readAll();
    qDebug()<<"thread";
}

void CtlTempTd::getTempTime(QByteArray array1)
{
    myCom->write(array1);
}

void CtlTempTd::recvByte(QByteArray array)
{
    myCom->write(array);
}
