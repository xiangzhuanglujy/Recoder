/****************************************************************
 *程序主界面
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
 *
 ***************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>

#include "qcustomplot.h"
#include "define.h"


class QextSerialPort;
class QAction;
class QCustomPlot;
class QPushButton;
class QComboBox;
class QLineEdit;
class MyThread;
class QListWidget;
class QTreeView;
class DInsetPic;
class QLabel;
class QTextBrowser;
class QTimer;
class QCheckBox;


#define STAND_DATA_LENGTH           4000                                    //标准具个数
#define GRATING_DATA_LENGTH         4000                                    //光栅个数


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void error(QString message);                                                //错误提示
    void plotStandJu();                                                         //画标准具
    void plotAgain(double yinzi);                                               //画标准具

    void plotGrating();                                                         //画光栅
    void initWindow();                                                          //初始化控件
    void receiveover_event();                                                   //数据接收完处理
    int getPeakPos(ushort buf[],int size);                                      //取最大值
    int getPeakPos(ushort buf[]);
    void saveData(ushort buf[]);                                                //保存数据到txt
//    double getPaValue(double up,double down);
    uint power(ushort num);                                                     //幂计算

signals:
    void paramsToThread(int,int,int,int);                                       //标准具光栅的4个参数
    void addressToThread(PtrAddr *ptr);                                         //数据交换地址
    void increasement(int);
    
public slots:
    void readCom();                                                             //串口接收数据槽
    void openAct_click();                                                       //打开串口槽
    void closeAct_click();                                                      //关闭串口槽
    void comCombo_changeText(QString str);                                      //串口下拉框内容改变槽
    void fourParamsBtn_click();                                                 //设置标准具
    void setXYBtn_click();                                                      //设置xy轴
    void backResult_event(QStringList list1,CenterPos *pos,WavePos *wave);      //线程回复结果槽
    void psgCombo_change(int index);                                            //切换通道数
    void hasTailCombo_selected(bool);
    void slotTimer();
    void slotTemperatureCombo(bool);

protected:

private:
    QextSerialPort *myCom;                                                  //串口类

    QCheckBox *tempCheckBox;                                                //计算温度
    QCheckBox *dlbCheckBox;
    QCheckBox *ttCheckBox;

    QTimer *wakeupTimer;

    QLabel *temp_display;                                                   //状态栏
    QLabel *temp_display2;                                                   //状态栏
    QLabel *collectLB;

    QAction *openAct;                                                       //打开串口
    QAction *closeAct;                                                      //关闭串口

    //这两个成员变量用于绘制曲线 上面一个是标准具的 下面一个是光栅的
    QCustomPlot *topCustomPlot;                                             //标准具波形
    QCustomPlot *centerCustomPlot;                                          //光栅波形

    QListWidget *topView;                                                   //标准具波峰坐标列表
    QTreeView *centerView_1;                                                //光栅波峰坐标列表
    QTreeView *centerView_2;                                                //光栅波长标列表
    QTreeView *centerView_3;                                                //光栅波长标列表

    QPushButton *fourParamsBtn;                                             //四个参数按钮
    QPushButton *setXYBtn;                                                  //设置xy坐标

    QComboBox *comCombo;                                                    //串口选择下拉框
    QComboBox *psgCombo;                                                    //通道数

    QLineEdit *stVTH;                                                       //标准具输入框
    QLineEdit *stVEXC;
    QLineEdit *grVTH;                                                       //光栅输入框
    QLineEdit *grVEXC;
    QLineEdit *xSLE;                                                        //x开始坐标输入框
    QLineEdit *xELE;
    QLineEdit *ySLE;                                                        //y开始坐标输入框
    QLineEdit *yELE;

    MyThread *thread;                                                       //计算线程

    double aValue[7];                                                          //温度计算系数1
    double bValue[7];                                                          //温度计算系数2
    double wenduValue;                                                      //回传的温度
    qint64 mRecoder;                                                        //data数据量

    //数值变量
    ushort stand_buffer[STAND_DATA_LENGTH];                                 //标准具接收数组
    ushort grating_buffer[GRATING_DATA_LENGTH];                             //光栅数组
    ushort data1_buffer[GRATING_DATA_LENGTH];
    ushort data2_buffer[GRATING_DATA_LENGTH];
    ushort data3_buffer[GRATING_DATA_LENGTH];
    ushort data4_buffer[GRATING_DATA_LENGTH];
    ushort data5_buffer[GRATING_DATA_LENGTH];
    ushort data6_buffer[GRATING_DATA_LENGTH];
    ushort data7_buffer[GRATING_DATA_LENGTH];

    QStringList tantouList;
};

#endif // MAINWINDOW_H
