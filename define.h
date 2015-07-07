/*********************************************
 *自定义参数
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *******************************************/

#ifndef DEFINE_H
#define DEFINE_H

#include <QStringList>

typedef unsigned short ushort;

#define RECV_OVER_LENGTH                80000                           //接收结束长度(uchar)
#define STANDARDNUM                     200                             //标准具峰的个数
#define GRATINGNUM                      200                             //光栅的峰的个数
#define SAMPLEPOINTNUM                  4000                            //采样点个数
#define INITWAVE_SIZE                   40                              //波长计算数组大小
#define STAND_LENGTH                    4000
#define TEMP_LENGTH                     4000
#define PRESS_LENGTH                    4000


//接收数据长度
typedef enum _OVERLENGTH
{
    CHANNEL_4000=8002,
    CHANNEL_8000=16002,
    CHANNEL_12000=24002,
    CHANNEL_28000=56002,
    CHANNEL_32000=64002,
    CHANNEL_36000=72002
}OVERLENGTH;


//是否有电路板温度 探头温度
typedef enum _TFORMAT
{
    TAIL_NO_DATA=0,
    TAIL_HAS_DATA=4,
    TAIL_WITHOUT_TEMP=0,
    TAIL_WITH_TEMP=4
}TFORMAT;


//暂时没用到
typedef struct tagCOEFFICIENT
{
    double m_PressureWave;  //初始压力波长
    double m_TemWave;		//初始温度波长
    double m_Pratio;		//压力系数
    double m_LiqRatio;		//液位系数
    double m_TemRatio;		//温度系数
    double m_CasePressure;	//套压参数
    double m_Deepth;		//传感器深度
    double m_Pay1;			//补偿1
    double m_Pay2;			//补偿2
    double m_Pay3;			//补偿3
    double dSTemWave;
    double dETemWave;
    double dSPreWave;
    double dEPreWave;
    double m_PreTemRate;
}COEFFICIENT;


//标准具 光栅传递地址
typedef struct tagAddr
{
    ushort *ptr1;               //标准具地址
    ushort *ptr2;               //光栅地址
    ushort *ptr3;               //光栅地址
    ushort *ptr4;               //光栅地址
    ushort *ptr5;               //光栅地址
    ushort *ptr6;               //光栅地址
    ushort *ptr7;               //光栅地址
    ushort *ptr8;               //光栅地址
    ushort *ptr9;               //光栅地址
    double wendu;               //温度1
    double wendu2;              //温度2
}PtrAddr;


//光栅中心坐标
class CenterPos
{
public:
    CenterPos()
    {}

    QStringList pos[8];           //光栅中心坐标

    void clear()
    {
        pos[0].clear();
        pos[1].clear();
        pos[2].clear();
        pos[3].clear();
        pos[4].clear();
        pos[5].clear();
        pos[6].clear();
        pos[7].clear();
    }
};


//光栅波长
class WavePos
{
public:
    WavePos()
    {
        yinzi=1.0;
        temperature=0.0;
        temperature2=0.0;
    }

    QStringList wave[8];      //光栅中心波长
    double temperature;
    double temperature2;
    double yinzi;

    void clear()
    {
        wave[0].clear();
        wave[1].clear();
        wave[2].clear();
        wave[3].clear();
        wave[4].clear();
        wave[5].clear();
        wave[6].clear();
        wave[7].clear();
        temperature=0.0;
        temperature2=0.0;
    }
};


//dll中的导出函数
typedef unsigned short (*PCalStandardPeakPos)(unsigned short usStandard[],unsigned short usPointNum,float fPos[],unsigned short* pusPeakNum,unsigned short usVTHTemp,unsigned short usVEXCTemp,int flag);
typedef unsigned short (*PCalGratingPeakPos)(unsigned short usGrating[],unsigned short usPointNum,float fPos[],unsigned short* pusPeakNum,unsigned short usVTHTemp,unsigned short usVEXCTemp);
typedef double (*PCalWaveLength)( double dwavePosition, int num, double IniWave[], float fPosition[] );
typedef double (*PFilterTem1)(double Inputval,unsigned short usFlag);
typedef double (*PFilterTem2)(double Inputval,unsigned short usFlag);
typedef double (*PFilterTem3)(double Inputval,unsigned short usFlag);
typedef double (*PFilterTem4)(double Inputval,unsigned short usFlag);
typedef double (*PFilterTem5)(double Inputval,unsigned short usFlag);
typedef double (*PFilterTem6)(double Inputval,unsigned short usFlag);
typedef double (*PFilterTem7)(double Inputval,unsigned short usFlag);
typedef double (*PFilterTem8)(double Inputval,unsigned short usFlag);
typedef unsigned short (*PCalGratingPeakPosDown)(unsigned short usGrating[],unsigned short usPointNum,float fPos[],unsigned short* pusPeakNum,unsigned short usVTHTemp,unsigned short usVEXCTemp);
typedef unsigned short (*PCalStandardPeakPosAndRange)(unsigned short usStandard[],unsigned short usPointNum,float fPos[],float range[],unsigned short* pusPeakNum,unsigned short usVTHTemp,unsigned short usVEXCTemp,int *flag);
typedef unsigned short (*PCalGratingPeakPosAndRange)(unsigned short usGrating[],unsigned short usPointNum,float fPos[],float range[],unsigned short* pusPeakNum,unsigned short usVTHTemp,unsigned short usVEXCTemp);
typedef unsigned short (*PCalOnePosAndDown)(unsigned short usGrating[],unsigned short,float pos[]);
typedef unsigned short (*PCalStandardPeakPosAdvance)(unsigned short usStandard[],unsigned short usPointNum,float fPos[],unsigned short* pusPeakNum,unsigned short usVTHTemp,unsigned short usVEXCTemp,int flag,double *db);
typedef void (*PChangePeakValue)(double *,double *);
typedef unsigned short (*PCalGratingPeakPosYinzi)(unsigned short usGrating[],unsigned short usPointNum,float fPos[],unsigned short* pusPeakNum,unsigned short usVTHTemp,unsigned short usVEXCTemp,double _yinzi);

//找最大值 最小值 下标
void maxMinIndex(ushort buf[],int size,ushort *max,ushort *min,int *indexMax,int *indexMin);

template<class T>
T squareAnd(T buf[],unsigned int start,unsigned int end)                  //平方和
{
    T sum=0;
    for(unsigned int i=start;i<end;i++)
    {
        sum+=buf[i]*buf[i];
    }
    return sum;
}

template<class T>
T accumulationAnd(T buf[],unsigned int start,unsigned int end)           //累加和
{
    T sum=0;
    for(unsigned int i=start;i<end;i++)
    {
        sum+=buf[i];
    }
    return sum;
}

template<class T>
T product(T buf1[],T buf2[],unsigned int start,unsigned int end)         //积
{
    T sum=0;
    for(unsigned int i=start;i<end;i++)
    {
        sum+=buf1[i]*buf2[i];
    }
    return sum;
}

#endif // DEFINE_H
