#ifndef FILEWR_H
#define FILEWR_H

#include <QStringList>

#define DEFAULT_EXCEL_EXE_PATH      "C:\\Program Files\\Microsoft Office\\OFFICE11\\EXCEL.exe"

#define SYSTEMLOG                   ".\\system\\ErrorLog.csv"		//系统日志地址
#define INITWAVE_PATH               ".\\user\\GratingConfig\\InitWave.csv"
#define THRESHOLD_PATH              ".\\user\\GratingConfig\\Threshold.csv"
#define WAVELENGTH_STORE_PATH       ".\\user\\DataStor\\history.csv"
#define LOG_PATH                    ".\\Log.txt"
#define STANDED_RANGE               ".\\user\\DataStor\\stand_range.csv"
#define STANDED_POS                 ".\\user\\DataStor\\stand_pos.csv"


QStringList ReadCSV(QString path);                  //读CSV文件
bool WriteCSV(QString str);                         //写字符串
bool WriteCSV(QStringList list);                    //写字符串列表
bool StoreWaveLength(QStringList context);          //保存波长

bool writeLog(QString date,QString context,QString path=LOG_PATH);

#endif // FILEWR_H

bool writeStanded_range(QString context);
bool writeStanded_pos(QString context);
