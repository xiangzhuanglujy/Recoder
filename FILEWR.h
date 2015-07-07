/**************************************************************
 *保存数据的类
 *
 *
 *
 *
 *
 *
 *
 *
 *************************************************************/


#ifndef FILEWR_H
#define FILEWR_H

#include <QStringList>

#define SYSTEMLOG                           ".\\system\\mainLog.txt"
#define SYSTEMLOG_THREAD                    ".\\system\\threadLod.txt"

#define INITWAVE_PATH                       ".\\user\\GratingConfig\\InitWave.csv"
#define DLL_LOAD_ADDRESS                    ".\\GDemodual.dll"


class FILEWARE
{
public:
    FILEWARE();

    static QStringList ReadCSV(const QString& path);                    //读CSV文件
    static bool writeLogCSV(QString str);                               //写字符串
    static bool writePositionC(const QString& path,QString str);        //中心坐标温度
    static bool writeWaveLengthC(const QString& path,QString str);      //波长温度
    static bool writeLogCSVThread(QString str);

};

#endif // FILEWR_H
