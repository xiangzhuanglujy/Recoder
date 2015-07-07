#include "define.h"


/***********************************************************************************************
*函数名 ：maxMinIndex
*函数功能描述 ：寻找数组最大值 最小值以及对应坐标
*函数参数 ：buf数组 size数组大小 *max最大值 *min最小值 *indexMax最大值坐标 *indexMin最小值坐标
*函数返回值 ：无
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
void maxMinIndex(ushort buf[],int size,ushort *max,ushort *min,int *indexMax,int *indexMin)
{
    ushort big=buf[0];
    ushort small=buf[0];
    int index1=0;
    int index2=0;
    for(int i=0;i<size;i++)
    {
        if(buf[i]>big)
        {
            big=buf[i];
            index1=i;
        }

        if(buf[i]<small)
        {
            small=buf[i];
            index2=i;
        }
    }
    *max=big;
    *min=small;
    *indexMax=index1;
    *indexMin=index2;
}
