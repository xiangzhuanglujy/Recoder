/****************************************************
 *Name:MyThread
 *Author:lujy
 *Date:2015/06/16
 *Modify:
 *2015/06/26  添加滚动平均
 *
 *
 *
 *
 *
 ****************************************************/

#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QStringList>
#include <QLibrary>

#include "define.h"

#define TEMPERATURE_COUNT   2


class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = 0);
    ~MyThread();

signals:
    void resultBack(QStringList,CenterPos *pos,WavePos *wave);
    void count(uint);

public slots:
    void getFourParams(int value1,int value2,int value3,int value4);        //初始化固定参数
    void getBeginAddress(PtrAddr *ptr);

protected:
    void run();

private:
    int stVTH_value;                                        //标准具
    int stVEXC_value;                                       //标准具
    int grVTH_value;                                        //光栅
    int grVEXC_value;                                       //光栅
    double dIniWave[INITWAVE_SIZE];                         //40个参数数组

    ushort stand_begin[STAND_LENGTH];                       //标准具起始地址
    ushort grating_begin[STAND_LENGTH];                     //光栅起始地址
    ushort data1_begin[STAND_LENGTH];                       //光栅起始地址
    ushort data2_begin[STAND_LENGTH];                       //光栅起始地址
    ushort data3_begin[STAND_LENGTH];                       //光栅起始地址
    ushort data4_begin[STAND_LENGTH];                       //光栅起始地址
    ushort data5_begin[STAND_LENGTH];                       //光栅起始地址
    ushort data6_begin[STAND_LENGTH];                       //光栅起始地址
    ushort data7_begin[STAND_LENGTH];                       //光栅起始地址
//    double t_wendu;
//    double t_wendu2;
    double bTemperature[TEMPERATURE_COUNT];

    CenterPos *pos;
    WavePos *wave;

    QLibrary myLib;                                         //动态库
    int flag;

    double standed1;
    double standed2;
    double standed3;
    double standed4;
    double standed5;
    double standed6;
    double standed7;
    double standed8;

};

#endif // MYTHREAD_H
