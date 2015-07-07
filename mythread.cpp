#include "mythread.h"
#include "FILEWR.h"
#include "inifile.h"

#include <QDebug>
#include <QMutex>
#include <QDateTime>
#include <qmath.h>
#include <QFile>

#define LVBO_COUNT          1

/******7通道对应的时间间隔************/
//#define SLEEP_TIME          4000
//#define MUTEX_TIME          1000

/******2通道对应的时间间隔************/
#define SLEEP_TIME          1000
#define MUTEX_TIME          500


extern int OVER_LENGTH;
extern bool isCollecting;
bool isOnce=false;

//互斥锁 线程同步机制
QMutex mutex;

//算法GDemodual.dll中的导出函数
//PCalStandardPeakPos CalStandardPeakPos;
PCalStandardPeakPosAdvance CalStandardPeakPos;
PCalGratingPeakPos	CalGratingPeakPos;
PCalGratingPeakPosDown CalGratingPeakPosDown;
PCalWaveLength	CalWaveLength;
PCalStandardPeakPosAndRange CalStandardPeakPosAndRange;
PCalGratingPeakPosAndRange	CalGratingPeakPosAndRange;
PFilterTem1 FilterTem1;
PFilterTem2 FilterTem2;
PFilterTem3 FilterTem3;
PFilterTem4 FilterTem4;
PFilterTem5 FilterTem5;
PFilterTem6 FilterTem6;
PFilterTem7 FilterTem7;
PFilterTem8 FilterTem8;
PChangePeakValue ChangePeakValue;
//PCalGratingPeakPosYinzi	CalGratingPeakPos;


MyThread::MyThread(QObject *parent) :
    QThread(parent)
{
    //加载动态库
    myLib.setFileName(DLL_LOAD_ADDRESS);
    if(myLib.load())
    {
//        CalStandardPeakPos = (PCalStandardPeakPos)myLib.resolve("CalStandardPeakPos");
        CalStandardPeakPos = (PCalStandardPeakPosAdvance)myLib.resolve("CalStandardPeakPosAdvance");
        CalGratingPeakPos = (PCalGratingPeakPos)myLib.resolve("CalGratingPeakPos");
//        CalGratingPeakPos = (PCalGratingPeakPosYinzi)myLib.resolve("CalGratingPeakPosYinzi");

        CalGratingPeakPosDown=(PCalGratingPeakPosDown)myLib.resolve("CalGratingPeakPosDown");
        CalWaveLength = (PCalWaveLength)myLib.resolve("CalWaveLength");
        CalStandardPeakPosAndRange = (PCalStandardPeakPosAndRange)myLib.resolve("CalStandardPeakPosAndRange");
        CalGratingPeakPosAndRange = (PCalGratingPeakPosAndRange)myLib.resolve("CalGratingPeakPosAndRange");
        FilterTem1=(PFilterTem1)myLib.resolve("FilterTem1");
        FilterTem2=(PFilterTem2)myLib.resolve("FilterTem2");
        FilterTem3=(PFilterTem3)myLib.resolve("FilterTem3");
        FilterTem4=(PFilterTem4)myLib.resolve("FilterTem4");
        FilterTem5=(PFilterTem5)myLib.resolve("FilterTem5");
        FilterTem6=(PFilterTem6)myLib.resolve("FilterTem6");
        FilterTem7=(PFilterTem7)myLib.resolve("FilterTem7");
        FilterTem8=(PFilterTem8)myLib.resolve("FilterTem8");
        ChangePeakValue=(PChangePeakValue)myLib.resolve("ChangePeadValue");

        //记录dll加载成功
        FILEWARE::writeLogCSVThread(QString("dll load success"));

        qDebug()<<"dll load success";
    }


    //获取标准具阈值
    flag=INIFile::readINI("Trough","trough").toInt();
    qDebug()<<"Trough"<<flag;

    //加载40个波长数组
    QStringList list=FILEWARE::ReadCSV(INITWAVE_PATH);
    if(list.size()>=INITWAVE_SIZE)
    {
        for(int i=0;i<list.size();i++)
        {
            dIniWave[i]=list.at(i).toDouble();
        }
    }

    //设置波长参考值 在参考值上下0.5内进行滚动平均
    standed1=INIFile::readINI("STANDED","grating1").toDouble();
    standed2=INIFile::readINI("STANDED","grating2").toDouble();
    standed3=INIFile::readINI("STANDED","grating3").toDouble();
    standed4=INIFile::readINI("STANDED","grating4").toDouble();
    standed5=INIFile::readINI("STANDED","grating5").toDouble();
    standed6=INIFile::readINI("STANDED","grating6").toDouble();
    standed7=INIFile::readINI("STANDED","grating7").toDouble();
    standed8=INIFile::readINI("STANDED","grating8").toDouble();

    //保存计算后的波长和中心坐标的类
    pos=new CenterPos();
    wave=new WavePos();
}

MyThread::~MyThread()
{
    //删除对象 释放资源
    if(pos!=NULL)
    {
        delete pos;
        pos=NULL;
    }
    if(wave!=NULL)
    {
        delete wave;
        wave=NULL;
    }

    //释放资源
    if(myLib.isLoaded())
    {
        myLib.unload();
    }
}

/***********************************************************************************************
*函数名 ：
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
void MyThread::getFourParams(int value1, int value2, int value3, int value4)
{
    stVTH_value=value1;
    stVEXC_value=value2;
    grVTH_value=value3;
    grVEXC_value=value4;
}


/***********************************************************************************************
*函数名 ：getBeginAddress
*函数功能描述 ：复制主界面区存储的数据到线程区
*函数参数 ：ptr 主界面数据地址指针
*函数返回值 ：无
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
void MyThread::getBeginAddress(PtrAddr *ptr)
{
    mutex.tryLock(MUTEX_TIME);

    if(CHANNEL_4000==OVER_LENGTH)
    {
        memcpy(stand_begin,ptr->ptr1,sizeof(ushort)*STAND_LENGTH);
        bTemperature[0]=ptr->wendu;
        bTemperature[1]=ptr->wendu2;
    }
    else if(CHANNEL_8000==OVER_LENGTH)
    {
        memcpy(stand_begin,ptr->ptr1,sizeof(ushort)*STAND_LENGTH);
        memcpy(grating_begin,ptr->ptr2,sizeof(ushort)*STAND_LENGTH);
        bTemperature[0]=ptr->wendu;
        bTemperature[1]=ptr->wendu2;
    }
    else if(CHANNEL_12000==OVER_LENGTH)
    {
        memcpy(stand_begin,ptr->ptr1,sizeof(ushort)*STAND_LENGTH);
        memcpy(grating_begin,ptr->ptr2,sizeof(ushort)*STAND_LENGTH);
        memcpy(data1_begin,ptr->ptr3,sizeof(ushort)*STAND_LENGTH);
        bTemperature[0]=ptr->wendu;
        bTemperature[1]=ptr->wendu2;
    }
    else if(CHANNEL_28000==OVER_LENGTH)
    {
        memcpy(stand_begin,ptr->ptr1,sizeof(ushort)*STAND_LENGTH);
        memcpy(grating_begin,ptr->ptr2,sizeof(ushort)*STAND_LENGTH);
        memcpy(data1_begin,ptr->ptr3,sizeof(ushort)*STAND_LENGTH);
        memcpy(data2_begin,ptr->ptr4,sizeof(ushort)*STAND_LENGTH);
        memcpy(data3_begin,ptr->ptr5,sizeof(ushort)*STAND_LENGTH);
        memcpy(data4_begin,ptr->ptr6,sizeof(ushort)*STAND_LENGTH);
        memcpy(data5_begin,ptr->ptr7,sizeof(ushort)*STAND_LENGTH);
        bTemperature[0]=ptr->wendu;
        bTemperature[1]=ptr->wendu2;
    }
    else if(CHANNEL_32000==OVER_LENGTH)
    {
        memcpy(stand_begin,ptr->ptr1,sizeof(ushort)*STAND_LENGTH);
        memcpy(grating_begin,ptr->ptr2,sizeof(ushort)*STAND_LENGTH);
        memcpy(data1_begin,ptr->ptr3,sizeof(ushort)*STAND_LENGTH);
        memcpy(data2_begin,ptr->ptr4,sizeof(ushort)*STAND_LENGTH);
        memcpy(data3_begin,ptr->ptr5,sizeof(ushort)*STAND_LENGTH);
        memcpy(data4_begin,ptr->ptr6,sizeof(ushort)*STAND_LENGTH);
        memcpy(data5_begin,ptr->ptr7,sizeof(ushort)*STAND_LENGTH);
        memcpy(data6_begin,ptr->ptr8,sizeof(ushort)*STAND_LENGTH);
        bTemperature[0]=ptr->wendu;
        bTemperature[1]=ptr->wendu2;
    }
    else if(CHANNEL_36000==OVER_LENGTH)
    {
        memcpy(stand_begin,ptr->ptr1,sizeof(ushort)*STAND_LENGTH);
        memcpy(grating_begin,ptr->ptr2,sizeof(ushort)*STAND_LENGTH);
        memcpy(data1_begin,ptr->ptr3,sizeof(ushort)*STAND_LENGTH);
        memcpy(data2_begin,ptr->ptr4,sizeof(ushort)*STAND_LENGTH);
        memcpy(data3_begin,ptr->ptr5,sizeof(ushort)*STAND_LENGTH);
        memcpy(data4_begin,ptr->ptr6,sizeof(ushort)*STAND_LENGTH);
        memcpy(data5_begin,ptr->ptr7,sizeof(ushort)*STAND_LENGTH);
        memcpy(data6_begin,ptr->ptr8,sizeof(ushort)*STAND_LENGTH);
        memcpy(data7_begin,ptr->ptr9,sizeof(ushort)*STAND_LENGTH);
        bTemperature[0]=ptr->wendu;
        bTemperature[1]=ptr->wendu2;
    }

    isOnce=true;

    mutex.unlock();
}


/***********************************************************************************************
*函数名 ：run
*函数功能描述 ：线程运行函数
*函数参数 ：无
*函数返回值 ：无
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
void MyThread::run()
{
    ushort buffer1[STAND_LENGTH];
    ushort buffer2[STAND_LENGTH];
    ushort buffer3[STAND_LENGTH];
    ushort buffer4[STAND_LENGTH];
    ushort buffer5[STAND_LENGTH];
    ushort buffer6[STAND_LENGTH];
    ushort buffer7[STAND_LENGTH];
    ushort buffer8[STAND_LENGTH];
    ushort buffer9[STAND_LENGTH];
    double temperature1=0.0;
    double temperature2=0.0;

    float  fpos[STANDARDNUM];                 //标准具中心坐标
    float  fTemPos[STANDARDNUM];              //光栅中心坐标
    float  fTemPos_1[STANDARDNUM];            //光栅中心坐标
    float  fTemPos_2[STANDARDNUM];            //光栅中心坐标
    float  fTemPos_3[STANDARDNUM];            //光栅中心坐标
    float  fTemPos_4[STANDARDNUM];            //光栅中心坐标
    float  fTemPos_5[STANDARDNUM];            //光栅中心坐标
    float  fTemPos_6[STANDARDNUM];            //光栅中心坐标
    float  fTemPos_7[STANDARDNUM];            //光栅中心坐标

    ushort usStandardPeakNum=0;               //标准具波峰个数
    ushort usTemPeakNum=0;                    //光栅波峰个数
    ushort usTemPeakNum_1=0;                  //光栅波峰个数
    ushort usTemPeakNum_2=0;                  //光栅波峰个数
    ushort usTemPeakNum_3=0;                  //光栅波峰个数
    ushort usTemPeakNum_4=0;                  //光栅波峰个数
    ushort usTemPeakNum_5=0;                  //光栅波峰个数
    ushort usTemPeakNum_6=0;                  //光栅波峰个数
    ushort usTemPeakNum_7=0;                  //光栅波峰个数

    double dTemWaveLength[GRATINGNUM];        //光栅波长数组
    double dTemWaveLength_1[GRATINGNUM];      //光栅波长数组
    double dTemWaveLength_2[GRATINGNUM];      //光栅波长数组
    double dTemWaveLength_3[GRATINGNUM];      //光栅波长数组
    double dTemWaveLength_4[GRATINGNUM];      //光栅波长数组
    double dTemWaveLength_5[GRATINGNUM];      //光栅波长数组
    double dTemWaveLength_6[GRATINGNUM];      //光栅波长数组
    double dTemWaveLength_7[GRATINGNUM];      //光栅波长数组

    unsigned short usFlag1 = 0;		//滤波器清零标识
    unsigned short usFlag2 = 0;		//滤波器清零标识
    unsigned short usFlag3 = 0;		//滤波器清零标识
    unsigned short usFlag4 = 0;		//滤波器清零标识
    unsigned short usFlag5 = 0;		//滤波器清零标识
    unsigned short usFlag6 = 0;		//滤波器清零标识
    unsigned short usFlag7 = 0;		//滤波器清零标识
    unsigned short usFlag8 = 0;		//滤波器清零标识

    while(isCollecting)
    {
        FILEWARE::writeLogCSVThread(QString("Thread Run"));

        if(isOnce==false)
        {
            msleep(500);
            continue;
        }

        FILEWARE::writeLogCSVThread(QString("Thread Begin"));

        mutex.tryLock(MUTEX_TIME);

        if(CHANNEL_4000==OVER_LENGTH)
        {
            memcpy(buffer1,stand_begin,sizeof(ushort)*STAND_LENGTH);
            temperature1=bTemperature[0];
            temperature2=bTemperature[1];
        }
        else if(CHANNEL_8000==OVER_LENGTH)
        {
            memcpy(buffer1,stand_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer2,grating_begin,sizeof(ushort)*STAND_LENGTH);
            temperature1=bTemperature[0];
            temperature2=bTemperature[1];
        }
        else if(CHANNEL_12000==OVER_LENGTH)
        {
            memcpy(buffer1,stand_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer2,grating_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer3,data1_begin,sizeof(ushort)*STAND_LENGTH);
            temperature1=bTemperature[0];
            temperature2=bTemperature[1];
        }
        else if(CHANNEL_28000==OVER_LENGTH)
        {
            memcpy(buffer1,stand_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer2,grating_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer3,data1_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer4,data2_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer5,data3_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer6,data4_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer7,data5_begin,sizeof(ushort)*STAND_LENGTH);
            temperature1=bTemperature[0];
            temperature2=bTemperature[1];
        }
        else if(CHANNEL_32000==OVER_LENGTH)
        {
            memcpy(buffer1,stand_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer2,grating_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer3,data1_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer4,data2_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer5,data3_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer6,data4_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer7,data5_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer8,data6_begin,sizeof(ushort)*STAND_LENGTH);
            temperature1=bTemperature[0];
            temperature2=bTemperature[1];
        }
        else if(CHANNEL_36000==OVER_LENGTH)
        {
            memcpy(buffer1,stand_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer2,grating_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer3,data1_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer4,data2_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer5,data3_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer6,data4_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer7,data5_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer8,data6_begin,sizeof(ushort)*STAND_LENGTH);
            memcpy(buffer9,data7_begin,sizeof(ushort)*STAND_LENGTH);
            temperature1=bTemperature[0];
            temperature2=bTemperature[1];
        }

        isOnce=false;
        mutex.unlock();

        qDebug()<<"thread start";

        switch(OVER_LENGTH)
        {
        case CHANNEL_4000:
            CalStandardPeakPos(buffer1,STAND_LENGTH,fpos,&usStandardPeakNum,stVTH_value,stVEXC_value,flag,&(wave->yinzi));  //标准具
//            CalStandardPeakPos(buffer1,STAND_LENGTH,fpos,&usStandardPeakNum,stVTH_value,stVEXC_value,flag);  //标准具
                    break;
        case CHANNEL_8000:
//            CalStandardPeakPos(buffer1,STAND_LENGTH,fpos,&usStandardPeakNum,stVTH_value,stVEXC_value,flag);  //标准具
            CalStandardPeakPos(buffer1,STAND_LENGTH,fpos,&usStandardPeakNum,stVTH_value,stVEXC_value,flag,&(wave->yinzi));  //标准具
            CalGratingPeakPos(buffer2,TEMP_LENGTH,fTemPos,&usTemPeakNum,grVTH_value,grVEXC_value);         //温度
            break;
        case CHANNEL_12000:
//            CalStandardPeakPos(buffer1,STAND_LENGTH,fpos,&usStandardPeakNum,stVTH_value,stVEXC_value,flag);  //标准具
            CalStandardPeakPos(buffer1,STAND_LENGTH,fpos,&usStandardPeakNum,stVTH_value,stVEXC_value,flag,&(wave->yinzi));  //标准具
            CalGratingPeakPos(buffer2,TEMP_LENGTH,fTemPos,&usTemPeakNum,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer3,TEMP_LENGTH,fTemPos_1,&usTemPeakNum_1,grVTH_value,grVEXC_value);         //温度
            break;
        case CHANNEL_28000:
//            CalStandardPeakPos(buffer1,STAND_LENGTH,fpos,&usStandardPeakNum,stVTH_value,stVEXC_value,flag);  //标准具
            CalStandardPeakPos(buffer1,STAND_LENGTH,fpos,&usStandardPeakNum,stVTH_value,stVEXC_value,flag,&(wave->yinzi));  //标准具
            CalGratingPeakPos(buffer2,TEMP_LENGTH,fTemPos,&usTemPeakNum,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer3,TEMP_LENGTH,fTemPos_1,&usTemPeakNum_1,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer4,TEMP_LENGTH,fTemPos_2,&usTemPeakNum_2,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer5,TEMP_LENGTH,fTemPos_3,&usTemPeakNum_3,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer6,TEMP_LENGTH,fTemPos_4,&usTemPeakNum_4,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer7,TEMP_LENGTH,fTemPos_5,&usTemPeakNum_5,grVTH_value,grVEXC_value);         //温度
            break;
        case CHANNEL_32000:
//            CalStandardPeakPos(buffer1,STAND_LENGTH,fpos,&usStandardPeakNum,stVTH_value,stVEXC_value,flag);  //标准具
            CalStandardPeakPos(buffer1,STAND_LENGTH,fpos,&usStandardPeakNum,stVTH_value,stVEXC_value,flag,&(wave->yinzi));  //标准具
            CalGratingPeakPos(buffer2,TEMP_LENGTH,fTemPos,&usTemPeakNum,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer3,TEMP_LENGTH,fTemPos_1,&usTemPeakNum_1,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer4,TEMP_LENGTH,fTemPos_2,&usTemPeakNum_2,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer5,TEMP_LENGTH,fTemPos_3,&usTemPeakNum_3,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer6,TEMP_LENGTH,fTemPos_4,&usTemPeakNum_4,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer7,TEMP_LENGTH,fTemPos_5,&usTemPeakNum_5,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer8,TEMP_LENGTH,fTemPos_6,&usTemPeakNum_6,grVTH_value,grVEXC_value);         //温度
//            CalGratingPeakPos(buffer2,TEMP_LENGTH,fTemPos,&usTemPeakNum,grVTH_value,grVEXC_value,wave->yinzi);         //温度
//            CalGratingPeakPos(buffer3,TEMP_LENGTH,fTemPos_1,&usTemPeakNum_1,grVTH_value,grVEXC_value,wave->yinzi);         //温度
//            CalGratingPeakPos(buffer4,TEMP_LENGTH,fTemPos_2,&usTemPeakNum_2,grVTH_value,grVEXC_value,wave->yinzi);         //温度
//            CalGratingPeakPos(buffer5,TEMP_LENGTH,fTemPos_3,&usTemPeakNum_3,grVTH_value,grVEXC_value,wave->yinzi);         //温度
//            CalGratingPeakPos(buffer6,TEMP_LENGTH,fTemPos_4,&usTemPeakNum_4,grVTH_value,grVEXC_value,wave->yinzi);         //温度
//            CalGratingPeakPos(buffer7,TEMP_LENGTH,fTemPos_5,&usTemPeakNum_5,grVTH_value,grVEXC_value,wave->yinzi);         //温度
//            CalGratingPeakPos(buffer8,TEMP_LENGTH,fTemPos_6,&usTemPeakNum_6,grVTH_value,grVEXC_value,wave->yinzi);         //温度

            break;
        case CHANNEL_36000:
//            CalStandardPeakPos(buffer1,STAND_LENGTH,fpos,&usStandardPeakNum,stVTH_value,stVEXC_value,flag);  //标准具
            CalStandardPeakPos(buffer1,STAND_LENGTH,fpos,&usStandardPeakNum,stVTH_value,stVEXC_value,flag,&(wave->yinzi));  //标准具
            CalGratingPeakPos(buffer2,TEMP_LENGTH,fTemPos,&usTemPeakNum,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer3,TEMP_LENGTH,fTemPos_1,&usTemPeakNum_1,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer4,TEMP_LENGTH,fTemPos_2,&usTemPeakNum_2,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer5,TEMP_LENGTH,fTemPos_3,&usTemPeakNum_3,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer6,TEMP_LENGTH,fTemPos_4,&usTemPeakNum_4,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer7,TEMP_LENGTH,fTemPos_5,&usTemPeakNum_5,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer8,TEMP_LENGTH,fTemPos_6,&usTemPeakNum_6,grVTH_value,grVEXC_value);         //温度
            CalGratingPeakPos(buffer9,TEMP_LENGTH,fTemPos_7,&usTemPeakNum_7,grVTH_value,grVEXC_value);         //温度
            break;
        }


        pos->clear();
        wave->clear();
        wave->temperature=temperature1;
        wave->temperature2=temperature2;

        //保存标准具波峰坐标
        QStringList standPosX;
        for(int i=0;i<usStandardPeakNum;i++)
        {
            standPosX.append(QString::number(fpos[i],'f',3));
        }
        //保存光栅中心坐标
        for(int i=0;i<usTemPeakNum;i++)
        {
            pos->pos[0].append(QString::number(fTemPos[i],'f',3));
        }
        for(int i=0;i<usTemPeakNum_1;i++)
        {
            pos->pos[1].append(QString::number(fTemPos_1[i],'f',3));
        }
        for(int i=0;i<usTemPeakNum_2;i++)
        {
            pos->pos[2].append(QString::number(fTemPos_2[i],'f',3));
        }
        for(int i=0;i<usTemPeakNum_3;i++)
        {
            pos->pos[3].append(QString::number(fTemPos_3[i],'f',3));
        }
        for(int i=0;i<usTemPeakNum_4;i++)
        {
            pos->pos[4].append(QString::number(fTemPos_4[i],'f',3));
        }
        for(int i=0;i<usTemPeakNum_5;i++)
        {
            pos->pos[5].append(QString::number(fTemPos_5[i],'f',3));
        }
        for(int i=0;i<usTemPeakNum_6;i++)
        {
            pos->pos[6].append(QString::number(fTemPos_6[i],'f',3));
        }
        for(int i=0;i<usTemPeakNum_7;i++)
        {
            pos->pos[7].append(QString::number(fTemPos_7[i],'f',3));
        }

        qDebug()<<"yinzi"<<wave->yinzi;

        //记录下上次和新的阈值
        double xx,yy;
        for(int i = 0;i < usTemPeakNum;i++ )
        {
            //计算光栅波长
            dTemWaveLength[i] = CalWaveLength( fTemPos[i],usStandardPeakNum,dIniWave,fpos);
            if(0==i&&qAbs(dTemWaveLength[i]-standed1)<=0.5)
            {
                    //在参考值范围内就进行滚动平均
                    dTemWaveLength[i]=FilterTem1(dTemWaveLength[i],LVBO_COUNT);
                    usFlag1++;
                //实时改变阈值
                ChangePeakValue(&xx,&yy);
                qDebug()<<"XXXXXXXXXXXXXXX"<<xx<<yy;
            }
            wave->wave[0].append(QString::number(dTemWaveLength[i],'f',3));
            break;
        }



        for(int i = 0;i < usTemPeakNum_1;i++ )
        {
            dTemWaveLength_1[i] = CalWaveLength( fTemPos_1[i],usStandardPeakNum,dIniWave,fpos);
            if(0==i&&qAbs(dTemWaveLength_1[i]-standed2)<=0.5)
            {
                dTemWaveLength_1[i]=FilterTem2(dTemWaveLength_1[i],LVBO_COUNT);
                usFlag2++;
            }

            wave->wave[1].append(QString::number(dTemWaveLength_1[i],'f',3));
            break;
        }

        for(int i = 0;i < usTemPeakNum_2;i++ )
        {
            dTemWaveLength_2[i] = CalWaveLength( fTemPos_2[i],usStandardPeakNum,dIniWave,fpos);

//            if(0==i&&qAbs(dTemWaveLength_2[i]-standed3)<=0.5)
//            {
//                dTemWaveLength_2[i]=FilterTem3(dTemWaveLength_2[i],LVBO_COUNT);
//                usFlag3++;
//            }
            wave->wave[2].append(QString::number(dTemWaveLength_2[i],'f',3));
        }

        for(int i = 0;i < usTemPeakNum_3;i++ )
        {
            dTemWaveLength_3[i] = CalWaveLength( fTemPos_3[i],usStandardPeakNum,dIniWave,fpos);

//            if(0==i&&qAbs(dTemWaveLength_3[i]-standed4)<=0.5)
//            {
//                dTemWaveLength_3[i]=FilterTem4(dTemWaveLength_3[i],LVBO_COUNT);
//                usFlag4++;
//            }
            wave->wave[3].append(QString::number(dTemWaveLength_3[i],'f',3));
        }

        for(int i = 0;i < usTemPeakNum_4;i++ )
        {
            dTemWaveLength_4[i] = CalWaveLength( fTemPos_4[i],usStandardPeakNum,dIniWave,fpos);

//            if(0==i&&qAbs(dTemWaveLength_4[i]-standed5)<=0.5)
//            {
//                dTemWaveLength_4[i]=FilterTem5(dTemWaveLength_4[i],LVBO_COUNT);
//                usFlag5++;
//            }
            wave->wave[4].append(QString::number(dTemWaveLength_4[i],'f',3));
        }

        for(int i = 0;i < usTemPeakNum_5;i++ )
        {
            dTemWaveLength_5[i] = CalWaveLength( fTemPos_5[i],usStandardPeakNum,dIniWave,fpos);

//            if(0==i&&qAbs(dTemWaveLength_5[i]-standed6)<=0.5)
//            {
//                dTemWaveLength_5[i]=FilterTem6(dTemWaveLength_5[i],LVBO_COUNT);
//                usFlag6++;
//            }
            wave->wave[5].append(QString::number(dTemWaveLength_5[i],'f',3));
        }

        for(int i = 0;i < usTemPeakNum_6;i++ )
        {
            dTemWaveLength_6[i] = CalWaveLength( fTemPos_6[i],usStandardPeakNum,dIniWave,fpos);

//            if(0==i&&qAbs(dTemWaveLength_6[i]-standed7)<=0.5)
//            {
//                dTemWaveLength_6[i]=FilterTem7(dTemWaveLength_6[i],LVBO_COUNT);
//                usFlag7++;
//            }
            wave->wave[6].append(QString::number(dTemWaveLength_6[i],'f',3));
        }

        for(int i = 0;i < usTemPeakNum_7;i++ )
        {
            dTemWaveLength_7[i] = CalWaveLength( fTemPos_7[i],usStandardPeakNum,dIniWave,fpos);

//            if(0==i&&qAbs(dTemWaveLength_7[i]-standed8)<=0.5)
//            {
//                dTemWaveLength_7[i]=FilterTem8(dTemWaveLength_7[i],LVBO_COUNT);
//                usFlag8++;
//            }
            wave->wave[7].append(QString::number(dTemWaveLength_7[i],'f',3));
        }

        qDebug()<<"thread end";


        if( usFlag1 == 0 )
        {
            FilterTem1( 0,usFlag1 );
        }
        if( usFlag2 == 0 )
        {
            FilterTem2( 0,usFlag2 );
        }
        if( usFlag3 == 0 )
        {
            FilterTem3( 0,usFlag3 );
        }
        if( usFlag4 == 0 )
        {
            FilterTem4( 0,usFlag4 );
        }
        if( usFlag5 == 0 )
        {
            FilterTem5( 0,usFlag5 );
        }
        if( usFlag6 == 0 )
        {
            FilterTem6( 0,usFlag6 );
        }
        if( usFlag7 == 0 )
        {
            FilterTem7( 0,usFlag7 );
        }
        if( usFlag8 == 0 )
        {
            FilterTem8( 0,usFlag8 );
        }

        usFlag1 = 0;		//滤波器清零标识
        usFlag2 = 0;		//滤波器清零标识
        usFlag3 = 0;		//滤波器清零标识
        usFlag4 = 0;		//滤波器清零标识
        usFlag5 = 0;		//滤波器清零标识
        usFlag6 = 0;		//滤波器清零标识
        usFlag7 = 0;		//滤波器清零标识
        usFlag8 = 0;		//滤波器清零标识


        //向主窗口发送计算结果 显示
        emit resultBack(standPosX,pos,wave);

        //记录下每一次计算成功
        FILEWARE::writeLogCSVThread(QString("Thread End"));

        //线程睡眠 等待下次计算
        msleep(SLEEP_TIME);
    }
}
