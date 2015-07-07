#include "mainwindow.h"
#include "mythread.h"
#include "serial/qextserialport.h"
#include "FILEWR.h"
#include "inifile.h"
#include "findcomputercom.h"

#include <QDebug>
#include <QToolBar>
#include <QAction>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QErrorMessage>
#include <QMessageBox>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QListWidget>
#include <QVBoxLayout>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QDateTime>
#include <QTableView>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QLabel>
#include <QDockWidget>
#include <QTextEdit>
#include <QDir>
#include <QTextBrowser>
#include <QTimer>
#include <QCoreApplication>
#include <QCheckBox>


bool isCollecting=false;                                    //线程运行标志

uchar tempBuf[RECV_OVER_LENGTH];                            //数据存放缓冲区

int OVER_LENGTH=CHANNEL_4000;                               //通道长度
int WENDU_TAIL=TAIL_NO_DATA;                                //电路板尾巴长度
int WENDU_TAIL2=TAIL_WITHOUT_TEMP;                          //光栅尾巴长度
int END_LENGTH=WENDU_TAIL+OVER_LENGTH+WENDU_TAIL2;          //接收长度
int recvLen=0;                                              //通道数据实时长度


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{    
    //窗口尺寸(全屏 最大化)
    showMaximized();

    //标题名字
    setWindowTitle("光纤数据采集上位机-"+QString::number(END_LENGTH));

    //没有Data目录就在当前目录下创建
    QDir *tempD=new QDir;
    bool exist=tempD->exists(".\\Data");
    if(!exist)
    {
        tempD->mkdir(".\\Data");
    }

    //新建串口
    myCom = new QextSerialPort();
    connect(myCom,SIGNAL(readyRead()),this,SLOT(readCom()));
    myCom->setBaudRate(BAUD115200);
    myCom->setDataBits(DATA_8);
    myCom->setParity(PAR_NONE);
    myCom->setStopBits(STOP_1);
    myCom->setFlowControl(FLOW_OFF); //设置数据流控制，我们使用无数据流控制的默认设置
    myCom->setTimeout(10); //设置延时

    //初始化控件
    initWindow();

    //计算线程
    thread=new MyThread(this);
    connect(this,SIGNAL(paramsToThread(int,int,int,int)),
            thread,SLOT(getFourParams(int,int,int,int)));
    connect(this,SIGNAL(addressToThread(PtrAddr*)),thread,SLOT(getBeginAddress(PtrAddr*)));
    connect(thread,SIGNAL(resultBack(QStringList,CenterPos*,WavePos*)),
            this,SLOT(backResult_event(QStringList,CenterPos*,WavePos*)));

    //y=a+b*x的a b参数
    aValue[0]=INIFile::readINI("WENDUPARAM","a1").toDouble();
    aValue[1]=INIFile::readINI("WENDUPARAM","a2").toDouble();
    aValue[2]=INIFile::readINI("WENDUPARAM","a3").toDouble();
    aValue[3]=INIFile::readINI("WENDUPARAM","a4").toDouble();
    aValue[4]=INIFile::readINI("WENDUPARAM","a5").toDouble();
    aValue[5]=INIFile::readINI("WENDUPARAM","a6").toDouble();
    aValue[6]=INIFile::readINI("WENDUPARAM","a7").toDouble();
    bValue[0]=INIFile::readINI("WENDUPARAM","b1").toDouble();
    bValue[1]=INIFile::readINI("WENDUPARAM","b2").toDouble();
    bValue[2]=INIFile::readINI("WENDUPARAM","b3").toDouble();
    bValue[3]=INIFile::readINI("WENDUPARAM","b4").toDouble();
    bValue[4]=INIFile::readINI("WENDUPARAM","b5").toDouble();
    bValue[5]=INIFile::readINI("WENDUPARAM","b6").toDouble();
    bValue[6]=INIFile::readINI("WENDUPARAM","b7").toDouble();

    //原始数据记录的节点
    mRecoder=INIFile::readINI("Recode","recode").toLongLong();

    //标准具和光栅四个参数
    int value1=INIFile::readINI("BASICINFO","VTH").toInt();
    int value2=INIFile::readINI("BASICINFO","VEXC").toInt();
    int value3=INIFile::readINI("BASICINFO","GTH").toInt();
    int value4=INIFile::readINI("BASICINFO","GEXC").toInt();

    //向计算线程发送
    emit paramsToThread(value1,value2,value3,value4);

    //主界面显示参数
    stVTH->setText(QString::number(value1));
    stVEXC->setText(QString::number(value2));
    grVTH->setText(QString::number(value3));
    grVEXC->setText(QString::number(value4));


    //守护进程时钟 防止死机
    wakeupTimer=new QTimer(this);
    connect(wakeupTimer,SIGNAL(timeout()),this,SLOT(slotTimer()));
    wakeupTimer->start(600000);
}

MainWindow::~MainWindow()
{   
    //删除串口
    if(NULL!=myCom)
    {
        delete myCom;
        myCom=NULL;
    }

    //线程退出
    isCollecting=false;
    if(NULL!=thread)
    {
        thread->wait();
        delete thread;
        thread=NULL;
    }

    //记录保存的次数
    mRecoder=(mRecoder/10+1)*10;
    INIFile::writeINI("Recode","recode",QString::number(mRecoder));
}

/***********************************************************************************************
*函数名 ：error
*函数功能描述 ：错误显示框
*函数参数 ：message 内容
*函数返回值 ：无
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
void MainWindow::error(QString message)
{
    QErrorMessage error;
    error.showMessage(message);
    error.exec();
}

/***********************************************************************************************
*函数名 ：plotStandJu
*函数功能描述 ：绘制标准具曲线
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
void MainWindow::plotStandJu()
{
    topCustomPlot->graph(0)->clearData();
    topCustomPlot->graph(1)->clearData();
    QVector<double> x(STAND_DATA_LENGTH), y(STAND_DATA_LENGTH);
    for (int i=0; i<STAND_DATA_LENGTH; ++i)
    {
      x[i] = i*1.0;
      y[i] = stand_buffer[i]*1.0;
    }
    topCustomPlot->graph(0)->setData(x,y);
    int value=stVTH->text().toInt();
    for (int i=0; i<STAND_DATA_LENGTH; ++i)
    {
      x[i] = i*1.0;
      y[i] = value*1.0;
    }
    topCustomPlot->graph(1)->setData(x,y);

    int xLower=topCustomPlot->xAxis->range().lower;
    int xUpper=topCustomPlot->xAxis->range().upper;
    int yLower=topCustomPlot->yAxis->range().lower;
    int yUpper=topCustomPlot->yAxis->range().upper;

    topCustomPlot->xAxis->setRange(xLower,xUpper);
    topCustomPlot->yAxis->setRange(yLower,yUpper);
    topCustomPlot->replot();
}

/***********************************************************************************************
*函数名 ：plotGrating
*函数功能描述 ：绘制光栅波形
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
void MainWindow::plotGrating()
{
    centerCustomPlot->graph(0)->clearData();
    centerCustomPlot->graph(1)->clearData();
    centerCustomPlot->graph(2)->clearData();
    centerCustomPlot->graph(3)->clearData();
    centerCustomPlot->graph(4)->clearData();
    centerCustomPlot->graph(5)->clearData();
    centerCustomPlot->graph(6)->clearData();
    centerCustomPlot->graph(7)->clearData();
    centerCustomPlot->graph(8)->clearData();

    int xLower=centerCustomPlot->xAxis->range().lower;
    int xUpper=centerCustomPlot->xAxis->range().upper;
    int yLower=centerCustomPlot->yAxis->range().lower;
    int yUpper=centerCustomPlot->yAxis->range().upper;

    centerCustomPlot->xAxis->setRange(xLower,xUpper);
    centerCustomPlot->yAxis->setRange(yLower,yUpper);

    QPen pen;
    pen.setColor(QColor(0,0,150));
    QVector<double> x(GRATING_DATA_LENGTH), y(GRATING_DATA_LENGTH);

    int value=grVTH->text().toInt();
    for (int i=0; i<GRATING_DATA_LENGTH; ++i)
    {
        x[i] = i*1.0;
        y[i] = value*1.0;
    }
    centerCustomPlot->graph(8)->setData(x, y);

    if(CHANNEL_4000==OVER_LENGTH)
    {
        centerCustomPlot->replot();
        return;
    }

    //第一通道
    for (int i=0; i<GRATING_DATA_LENGTH; ++i)
    {
      x[i] = i*1.0;
      y[i] = grating_buffer[i]*1.0;
    }
    centerCustomPlot->graph(0)->setName("第一通道");
    centerCustomPlot->graph(0)->setPen(pen);
    centerCustomPlot->graph(0)->setData(x, y);

    if(CHANNEL_8000==OVER_LENGTH)
    {
        centerCustomPlot->replot();
        return;
    }

    //第二通道
    for (int i=0; i<GRATING_DATA_LENGTH; ++i)
    {
      x[i] = i*1.0;
      y[i] = data1_buffer[i]*1.0;
    }
    pen.setColor(QColor(0,150,150));
    centerCustomPlot->graph(1)->setName("第二通道");
    centerCustomPlot->graph(1)->setPen(pen);
    centerCustomPlot->graph(1)->setData(x, y);


    if(CHANNEL_12000==OVER_LENGTH)
    {
        centerCustomPlot->replot();
        return;
    }

    //第三通道
    for (int i=0; i<GRATING_DATA_LENGTH; ++i)
    {
      x[i] = i*1.0;
      y[i] = data2_buffer[i]*1.0;
    }
    pen.setColor(QColor(150,150,150));
    centerCustomPlot->graph(2)->setName("第三通道");
    centerCustomPlot->graph(2)->setPen(pen);
    centerCustomPlot->graph(2)->setData(x, y);

    //第四通道
    for (int i=0; i<GRATING_DATA_LENGTH; ++i)
    {
      x[i] = i*1.0;
      y[i] = data3_buffer[i]*1.0;
    }
    pen.setColor(QColor(255,150,150));
    centerCustomPlot->graph(3)->setName("第四通道");
    centerCustomPlot->graph(3)->setPen(pen);
    centerCustomPlot->graph(3)->setData(x, y);

    //第五通道
    for (int i=0; i<GRATING_DATA_LENGTH; ++i)
    {
      x[i] = i*1.0;
      y[i] = data4_buffer[i]*1.0;
    }
    pen.setColor(QColor(0,150,150));
    centerCustomPlot->graph(4)->setName("第五通道");
    centerCustomPlot->graph(4)->setPen(pen);
    centerCustomPlot->graph(4)->setData(x, y);

    //第六通道
    for (int i=0; i<GRATING_DATA_LENGTH; ++i)
    {
      x[i] = i*1.0;
      y[i] = data5_buffer[i]*1.0;
    }
    pen.setColor(QColor(255,0,150));
    centerCustomPlot->graph(5)->setName("第六通道");
    centerCustomPlot->graph(5)->setPen(pen);
    centerCustomPlot->graph(5)->setData(x, y);

    //第七通道
    for (int i=0; i<GRATING_DATA_LENGTH; ++i)
    {
      x[i] = i*1.0;
      y[i] = data6_buffer[i]*1.0;
    }
    pen.setColor(QColor(128,128,150));
    centerCustomPlot->graph(6)->setName("第七通道");
    centerCustomPlot->graph(6)->setPen(pen);
    centerCustomPlot->graph(6)->setData(x, y);

    if(CHANNEL_32000==OVER_LENGTH)
    {
        centerCustomPlot->replot();
        return;
    }

    //第八通道
    for (int i=0; i<GRATING_DATA_LENGTH; ++i)
    {
      x[i] = i*1.0;
      y[i] = data7_buffer[i]*1.0;
    }
    pen.setColor(QColor(0,150,255));
    centerCustomPlot->graph(7)->setName("第八通道");
    centerCustomPlot->graph(7)->setPen(pen);
    centerCustomPlot->graph(7)->setData(x, y);

    centerCustomPlot->replot();
}

//y=a+bx
//void MainWindow::calAandB()
//{
//    double temp[WAVE_TEMP_LENGTH]={0.0};
//    int zs=0;
//    for(int i=0;i<JISUAN_END_LENGTH;i++)
//    {
//        zs=waveLength_buf[i]/100;
//        temp[i]=(waveLength_buf[i]-zs*100)*1000;
//    }

//    double x1=squareAnd(wendu_buf,0,JISUAN_END_LENGTH);
//    double x2=accumulationAnd(wendu_buf,0,JISUAN_END_LENGTH);
//    double x3=product(wendu_buf,temp,0,JISUAN_END_LENGTH);
//    double y1=accumulationAnd(temp,0,JISUAN_END_LENGTH);

//    double a=(x1*y1-x2*x3)/(x1*JISUAN_END_LENGTH-x2*x2);
//    double b=(JISUAN_END_LENGTH*x3-x2*y1)/(x1*JISUAN_END_LENGTH-x2*x2);
//    qDebug()<<"a"<<a<<"b"<<b;
//    wendu_buf_index=0;
//    waveLength_buf_index=0;

//    //画温度波长曲线
//    plotTempWave(a,b);

//    plotTempWave(0.0,0.0);
//}


/***********************************************************************************************
*函数名 ：initWindow
*函数功能描述 ：初始化主窗口所有控件
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
void MainWindow::initWindow()
{
    QLabel *iconLB=new QLabel();
    QLabel *comLB=new QLabel("串口选择");
    comCombo=new QComboBox();
    QLabel *passLB=new QLabel("采集个数");
    psgCombo=new QComboBox();
    openAct=new QAction(QIcon(":/images/open"),"打开串口",this);
    closeAct=new QAction(QIcon(":/images/stop"),"关闭串口",this);

    iconLB->setPixmap(QPixmap(":/images/tcp"));

    comCombo->addItems(FindComputerCom::enumLocalCom());

    psgCombo->addItem("4000");
    psgCombo->addItem("8000");
    psgCombo->addItem("12000");
    psgCombo->addItem("28000");
    psgCombo->addItem("32000");
    psgCombo->addItem("36000");

    tempCheckBox=new QCheckBox("显示光栅温度");
    dlbCheckBox=new QCheckBox("显示电路板温度");
    ttCheckBox=new QCheckBox("显示探头温度");

    closeAct->setEnabled(false);

    QToolBar *toolBar=new QToolBar();
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->setMovable(false);
    addToolBar(toolBar);

    toolBar->addWidget(iconLB);
    toolBar->addSeparator();
    toolBar->addWidget(comLB);
    toolBar->addSeparator();
    toolBar->addWidget(comCombo);
    toolBar->addSeparator();
    toolBar->addWidget(passLB);
    toolBar->addSeparator();
    toolBar->addWidget(psgCombo);
    toolBar->addSeparator();
    toolBar->addWidget(dlbCheckBox);
    toolBar->addSeparator();
    toolBar->addWidget(ttCheckBox);
    toolBar->addSeparator();
    toolBar->addWidget(tempCheckBox);
    toolBar->addSeparator();

    toolBar->addAction(openAct);
    toolBar->addAction(closeAct);

    connect(openAct,SIGNAL(triggered()),this,SLOT(openAct_click()));
    connect(closeAct,SIGNAL(triggered()),this,SLOT(closeAct_click()));
    connect(comCombo,SIGNAL(currentIndexChanged(QString)),this,SLOT(comCombo_changeText(QString)));
    connect(psgCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(psgCombo_change(int)));
    connect(dlbCheckBox,SIGNAL(toggled(bool)),this,SLOT(hasTailCombo_selected(bool)));
    connect(ttCheckBox,SIGNAL(toggled(bool)),this,SLOT(slotTemperatureCombo(bool)));


    //状态栏
    temp_display=new QLabel("电路板温度:");
    temp_display2=new QLabel("探头温度:");

    statusBar()->addWidget(temp_display,1);
    statusBar()->addWidget(temp_display2,1);

    //绘图控件
    topCustomPlot=new QCustomPlot();
    centerCustomPlot=new QCustomPlot();

    topCustomPlot->addGraph();
    topCustomPlot->addGraph();
    topCustomPlot->addGraph();

    QPen pen;
    pen.setColor(QColor(255,0,0));
    pen.setStyle(Qt::DashLine);
    topCustomPlot->graph(1)->setPen(pen);
    pen.setColor(QColor(0,255,255));
    pen.setStyle(Qt::SolidLine);
    topCustomPlot->graph(2)->setPen(pen);

//    centerCustomPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    centerCustomPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    topCustomPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);

    centerCustomPlot->addGraph();
    centerCustomPlot->addGraph();
    centerCustomPlot->addGraph();
    centerCustomPlot->addGraph();
    centerCustomPlot->addGraph();
    centerCustomPlot->addGraph();
    centerCustomPlot->addGraph();
    centerCustomPlot->addGraph();
    centerCustomPlot->addGraph();
    centerCustomPlot->graph(8)->setPen(pen);

    topCustomPlot->xAxis->setRange(0,4000);
    topCustomPlot->yAxis->setRange(0,4096);
    centerCustomPlot->xAxis->setRange(0,4000);
    centerCustomPlot->yAxis->setRange(0,4096);

    topView=new QListWidget();
    centerView_1=new QTreeView();
    centerView_2=new QTreeView();
    centerView_3=new QTreeView();

    //操控台
    fourParamsBtn=new QPushButton("设置标准具与光栅参数");
    setXYBtn=new QPushButton("设置xy坐标轴");
    QLabel *lb1=new QLabel("标准具VTH");
    QLabel *lb2=new QLabel("标准具VEXC");
    QLabel *lb3=new QLabel("光栅VTH");
    QLabel *lb4=new QLabel("光栅VEXC");
    QLabel *lb5=new QLabel("x起始坐标");
    QLabel *lb6=new QLabel("x终止坐标");
    QLabel *lb7=new QLabel("y起始坐标");
    QLabel *lb8=new QLabel("y终止坐标");
    stVTH=new QLineEdit();
    stVEXC=new QLineEdit();
    grVTH=new QLineEdit();
    grVEXC=new QLineEdit();
    xSLE=new QLineEdit("0");
    xELE=new QLineEdit("4000");
    ySLE=new QLineEdit("0");
    yELE=new QLineEdit("4096");

    QRegExp regExp("[0-9]{4}");
    stVTH->setValidator(new QRegExpValidator(regExp, this));
    stVEXC->setValidator(new QRegExpValidator(regExp, this));
    grVTH->setValidator(new QRegExpValidator(regExp, this));
    grVEXC->setValidator(new QRegExpValidator(regExp, this));
    xSLE->setValidator(new QRegExpValidator(regExp, this));
    xELE->setValidator(new QRegExpValidator(regExp, this));
    ySLE->setValidator(new QRegExpValidator(regExp, this));
    yELE->setValidator(new QRegExpValidator(regExp, this));

    connect(fourParamsBtn,SIGNAL(clicked()),this,SLOT(fourParamsBtn_click()));
    connect(setXYBtn,SIGNAL(clicked()),this,SLOT(setXYBtn_click()));

    //布局
    QGroupBox *group1=new QGroupBox("标准具波形------波峰坐标");
    QGridLayout *layout1=new QGridLayout(group1);
    layout1->setColumnStretch(0,5);
    layout1->setColumnStretch(1,1);
    layout1->addWidget(topCustomPlot,0,0);
    layout1->addWidget(topView,0,1);
    QGroupBox *group2=new QGroupBox("光栅波形------波峰坐标------中心波长------通道温度");
    QGridLayout *layout2=new QGridLayout(group2);
    layout2->setColumnStretch(0,5);
    layout2->setColumnStretch(1,1);
    layout2->addWidget(centerCustomPlot,0,0,3,1);
    layout2->addWidget(centerView_1,0,1);
    layout2->addWidget(centerView_2,1,1);
    layout2->addWidget(centerView_3,2,1);

    QGroupBox *group4=new QGroupBox("控制台");
    QGridLayout *layout4=new QGridLayout(group4);
    layout4->addWidget(lb1,0,0);
    layout4->addWidget(lb2,1,0);
    layout4->addWidget(lb3,2,0);
    layout4->addWidget(lb4,3,0);
    layout4->addWidget(stVTH,0,1);
    layout4->addWidget(stVEXC,1,1);
    layout4->addWidget(grVTH,2,1);
    layout4->addWidget(grVEXC,3,1);
    layout4->addWidget(fourParamsBtn,4,0,1,2);
    layout4->addWidget(lb5,5,0);
    layout4->addWidget(lb6,6,0);
    layout4->addWidget(lb7,7,0);
    layout4->addWidget(lb8,8,0);
    layout4->addWidget(xSLE,5,1);
    layout4->addWidget(xELE,6,1);
    layout4->addWidget(ySLE,7,1);
    layout4->addWidget(yELE,8,1);
    layout4->addWidget(setXYBtn,9,0,1,2);
    QWidget *center=new QWidget();
    setCentralWidget(center);
    QGridLayout *main=new QGridLayout(center);
    main->setColumnStretch(0,8);
    main->setColumnStretch(1,1);
    QVBoxLayout *devideL=new QVBoxLayout();
    devideL->addWidget(group1,1);
    devideL->addWidget(group2,1);
    main->addLayout(devideL,0,0);
    main->addWidget(group4,0,1);

}


/***********************************************************************************************
*函数名 ：readCom
*函数功能描述 ：读取串口数据
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
void MainWindow::readCom()
{
    //串口读数据
    QByteArray array=myCom->readAll();
    int size=array.size();
    if(size==0)
    {
        return;
    }

    //收到0xF0F0回复0x55开始接收有用数据
    for(int i=0;i<size-1;i++)
    {
        if(0xF0==(uchar)array.at(i)&&0xF0==(uchar)array.at(i+1))
        {
            QByteArray ay;
            ay.append(0x55);
            myCom->write(ay);
            recvLen=0;
            return;
        }
    }

    //把数据存到缓冲区
    for(int i=0;i<size;i++)
    {
        tempBuf[recvLen+i]=(uchar)array.at(i);
    }
    recvLen+=size;

    //判断数据接收长度
    if(recvLen==END_LENGTH)
    {
        recvLen=0;
        //接收完处理
        receiveover_event();
    }
    else if(recvLen>END_LENGTH)
    {
        //接收数据长度清0 防止缓冲区溢出
        recvLen=0;
    }
}


/***********************************************************************************************
*函数名 ：openAct_click
*函数功能描述 ：打开串口
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
void MainWindow::openAct_click()
{
    //设置串口名字
    myCom->setPortName(comCombo->currentText());
    if(myCom->open(QIODevice::ReadWrite))
    {
        openAct->setEnabled(false);
        closeAct->setEnabled(true);
        psgCombo->setEnabled(false);
        comCombo->setEnabled(false);
        dlbCheckBox->setEnabled(false);
        ttCheckBox->setEnabled(false);

        //打开数据处理线程
        isCollecting=true;
        thread->start();

        //发送起始数据
        QByteArray array;
        array.append(0xFF);
        myCom->write(array);

        return;
    }
    else
    {
        error("串口不存在或者被其他程序占用");
    }
}


/***********************************************************************************************
*函数名 ：closeAct_click
*函数功能描述 ：关闭串口
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
void MainWindow::closeAct_click()
{
    //关闭串口
    if(myCom->isOpen())
    {
        myCom->close();
    }
    isCollecting=false;

    openAct->setEnabled(true);
    closeAct->setEnabled(false);
    psgCombo->setEnabled(true);
    comCombo->setEnabled(true);
    dlbCheckBox->setEnabled(true);
    ttCheckBox->setEnabled(true);

    recvLen=0;
}


/***********************************************************************************************
*函数名 ：comCombo_changeText
*函数功能描述 ：串口名字改变触发的事件
*函数参数 ：str 串口名称
*函数返回值 ：无
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
void MainWindow::comCombo_changeText(QString str)
{
    if(myCom->isOpen())
    {
        error("请先关闭串口");
        return;
    }
    //改变串口名字
    myCom->setPortName(str);
}


/***********************************************************************************************
*函数名 ：fourParamsBtn_click
*函数功能描述 ：设置标准具光栅四个参考参数
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
void MainWindow::fourParamsBtn_click()
{
    if(myCom->isOpen())
    {
        QMessageBox::critical(this,"提示","清先关闭串口");
        return;
    }
    int stVTH_value=stVTH->text().toInt();
    int stVEXC_value=stVEXC->text().toInt();
    int grVTH_value=grVTH->text().toInt();
    int grVEXC_value=grVEXC->text().toInt();

    //改变Dll中标准具 光栅四个参数的值
    emit paramsToThread(stVTH_value,stVEXC_value,grVTH_value,grVEXC_value);
    QMessageBox::information(this,"提示","设置成功");
}


/***********************************************************************************************
*函数名 ：receiveover_event
*函数功能描述 ：串口数据接收完的处理函数
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
void MainWindow::receiveover_event()
{
    //温度在AOAO之前
//    if(tempBuf[END_LENGTH-1]!=0xA0||tempBuf[END_LENGTH-2]!=0xA0)
//    {
//        recvLen=0;
//        QByteArray array;
//        array.append(0xFF);
//        myCom->write(array);
//        return;
//    }


    //验证接收数据的正确性 不正确发0xFF重新获取
    if(tempBuf[OVER_LENGTH-1]!=0xA0||tempBuf[OVER_LENGTH-2]!=0xA0)
    {
        recvLen=0;
        QByteArray array;
        array.append(0xFF);
        myCom->write(array);
        return;
    }

//    if(TAIL_WITH_TEMP==WENDU_TAIL2)
//    {
//        if(tempBuf[END_LENGTH-6]!=0xA0||tempBuf[END_LENGTH-5]!=0xA0)
//        {
//            recvLen=0;
//            QByteArray array;
//            array.append(0xFF);
//            myCom->write(array);
//            return;
//        }
//    }
//    else
//    {
//        if(tempBuf[END_LENGTH-2]!=0xA0||tempBuf[END_LENGTH-1]!=0xA0)
//        {
//            recvLen=0;
//            QByteArray array;
//            array.append(0xFF);
//            myCom->write(array);
//            return;
//        }
//    }

    //记录
    FILEWARE::writeLogCSV(QString("Processing Begin"));

    //数据分离
    int size=END_LENGTH/2;
    ushort temp[size];
    for(int i=0;i<size;i++)
    {
        temp[i]=tempBuf[2*i]*256+tempBuf[2*i+1];
    }

    //把缓冲区数据拷贝到标准具光栅数组中
    switch(OVER_LENGTH)
    {
    case CHANNEL_4000:
        memcpy(stand_buffer,temp,sizeof(ushort)*STAND_DATA_LENGTH);
        break;
    case CHANNEL_8000:
        memcpy(stand_buffer,temp,sizeof(ushort)*STAND_DATA_LENGTH);
        memcpy(grating_buffer,&temp[STAND_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        break;
    case CHANNEL_12000:
        memcpy(stand_buffer,temp,sizeof(ushort)*STAND_DATA_LENGTH);
        memcpy(grating_buffer,&temp[STAND_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data1_buffer,&temp[STAND_DATA_LENGTH+GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        break;
    case CHANNEL_28000:
        memcpy(stand_buffer,temp,sizeof(ushort)*STAND_DATA_LENGTH);
        memcpy(grating_buffer,&temp[STAND_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data1_buffer,&temp[STAND_DATA_LENGTH+GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data2_buffer,&temp[STAND_DATA_LENGTH+2*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data3_buffer,&temp[STAND_DATA_LENGTH+3*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data4_buffer,&temp[STAND_DATA_LENGTH+4*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data5_buffer,&temp[STAND_DATA_LENGTH+5*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        break;
    case CHANNEL_32000:
        memcpy(stand_buffer,temp,sizeof(ushort)*STAND_DATA_LENGTH);
        memcpy(grating_buffer,&temp[STAND_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data1_buffer,&temp[STAND_DATA_LENGTH+GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data2_buffer,&temp[STAND_DATA_LENGTH+2*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data3_buffer,&temp[STAND_DATA_LENGTH+3*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data4_buffer,&temp[STAND_DATA_LENGTH+4*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data5_buffer,&temp[STAND_DATA_LENGTH+5*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data6_buffer,&temp[STAND_DATA_LENGTH+6*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        saveData(temp);

        break;
    case CHANNEL_36000:
        memcpy(stand_buffer,temp,sizeof(ushort)*STAND_DATA_LENGTH);
        memcpy(grating_buffer,&temp[STAND_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data1_buffer,&temp[STAND_DATA_LENGTH+GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data2_buffer,&temp[STAND_DATA_LENGTH+2*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data3_buffer,&temp[STAND_DATA_LENGTH+3*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data4_buffer,&temp[STAND_DATA_LENGTH+4*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data5_buffer,&temp[STAND_DATA_LENGTH+5*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data6_buffer,&temp[STAND_DATA_LENGTH+6*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        memcpy(data7_buffer,&temp[STAND_DATA_LENGTH+7*GRATING_DATA_LENGTH],sizeof(ushort)*GRATING_DATA_LENGTH);
        break;
    }

    //记录下电路板和光栅温度
    double wendu_t=0.0;
    double wendu_t2=0.0;

    if(TAIL_HAS_DATA==WENDU_TAIL&&TAIL_WITH_TEMP==WENDU_TAIL2)
    {
        wendu_t=QString("%1.%2").arg(temp[size-4]).arg(temp[size-3]).toDouble();
        wendu_t2=QString("%1.%2").arg(temp[size-2]).arg(temp[size-1]).toDouble();
        temp_display->setText(QString("电路板温度:%1").arg(wendu_t));
        temp_display2->setText(QString("探头温度:%1").arg(wendu_t2));
    }
    else if(TAIL_HAS_DATA==WENDU_TAIL&&TAIL_WITHOUT_TEMP==WENDU_TAIL2)
    {
        wendu_t=QString("%1.%2").arg(temp[size-2]).arg(temp[size-1]).toDouble();
        temp_display->setText(QString("电路板温度:%1").arg(wendu_t));
    }
    else if(TAIL_NO_DATA==WENDU_TAIL&&TAIL_WITH_TEMP==WENDU_TAIL2)
    {
        wendu_t2=QString("%1.%2").arg(temp[size-2]).arg(temp[size-1]).toDouble();
        temp_display2->setText(QString("探头温度:%1").arg(wendu_t2));
    }


    //温度之前
//    if(TAIL_HAS_DATA==WENDU_TAIL)
//    {
//        wendu_t=QString("%1.%2").arg(temp[size-3]).arg(temp[size-2]).toDouble();
//        temp_display->setText(QString("温度1:%1").arg(wendu_t));
//    }


    //
    PtrAddr addr;
    addr.ptr1=stand_buffer;
    addr.ptr2=grating_buffer;
    addr.ptr3=data1_buffer;
    addr.ptr4=data2_buffer;
    addr.ptr5=data3_buffer;
    addr.ptr6=data4_buffer;
    addr.ptr7=data5_buffer;
    addr.ptr8=data6_buffer;
    addr.ptr9=data7_buffer;
    addr.wendu=wendu_t;
    addr.wendu2=wendu_t2;
    //通知线程计算数据
    emit addressToThread(&addr);

    //画标准具和光栅波形
    plotGrating();
    plotStandJu();

    //记录
    FILEWARE::writeLogCSV(QString("Processing End"));
}


/***********************************************************************************************
*函数名 ：getPeakPos
*函数功能描述 ：寻找数组中的最大值
*函数参数 ：buf数组 size数组大小
*函数返回值 ：int 最大值
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
int MainWindow::getPeakPos(ushort buf[], int size)
{
    int peak=buf[4000];
    int count=4000+size*4000;
    for(int i=4001;i<count;i++)
    {
        if(buf[i]>peak)
            peak=buf[i];
    }
    return peak;
}


/***********************************************************************************************
*函数名 ：
*函数功能描述 ：寻找数组中的最大值
*函数参数 ：buf数组 size数组大小(固定4000)
*函数返回值 ：int 最大值
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
int MainWindow::getPeakPos(ushort buf[])
{
    int peak=buf[0];
    for(int i=1;i<4000;i++)
    {
        if(buf[i]>peak)
            peak=buf[i];
    }
    return peak;
}

/***********************************************************************************************
*函数名 ：saveData
*函数功能描述 ：保存原始数据
*函数参数 ：buf原始数据
*函数返回值 ：无
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
void MainWindow::saveData(ushort buf[])
{
    QString path=QString(".\\Data\\%1.txt").arg(mRecoder/10);

    QFile file(path);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))
    {
        return;
    }
    QTextStream stream(&file);
    for(int i=0;i<32000;i++)
    {
        stream<<buf[i]<<" ";
    }
    stream<<endl<<endl;
    file.close();
    mRecoder++;
}

//double MainWindow::getPaValue(double up, double down)
//{
//    double pa=0.0;

//    double H3=(down-B1)/K1;
//    double C3=(up-B3)/K3;
//    double P3=(C3-14.9)*K2+down;
//    double B13=P3-K1*H3;

//    qDebug()<<"H3"<<H3<<"P3"<<P3<<"B13"<<B13<<"C3"<<C3;

//    pa=(down-B13)/K1;

//    return pa;
//}


/***********************************************************************************************
*函数名 ：setXYBtn_click
*函数功能描述 ：设置显示波形的横纵坐标
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
void MainWindow::setXYBtn_click()
{
    int xS_t=xSLE->text().toInt();
    int xE_t=xELE->text().toInt();
    int yS_t=ySLE->text().toInt();
    int yE_t=yELE->text().toInt();

    if(xS_t>=xE_t||yS_t>=yE_t)
    {
        QMessageBox::critical(this,"提示","输入有误");
        return;
    }

    topCustomPlot->xAxis->setRange(xS_t,xE_t);
    topCustomPlot->yAxis->setRange(yS_t,yE_t);
    centerCustomPlot->xAxis->setRange(xS_t,xE_t);
    centerCustomPlot->yAxis->setRange(yS_t,yE_t);
    topCustomPlot->replot();
    centerCustomPlot->replot();

//    QMessageBox::information(this,"提示","设置成功");
}


/***********************************************************************************************
*函数名 ：backResult_event
*函数功能描述 ：线程返回的计算结果
*函数参数 ：list1 标准具波峰坐标 pos光栅中心坐标 wave 光栅波长
*函数返回值 ：无
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
void MainWindow::backResult_event(QStringList list1, CenterPos *pos, WavePos *wave)
{
    //标准具中心坐标
//    plotAgain(wave->yinzi);
    topView->clear();
    topView->addItems(list1);

    //1-8通道中心坐标
    QStandardItemModel *model=new QStandardItemModel();
    QStringList labels;
    labels<<"通道号"<<"中心坐标";
    model->setHorizontalHeaderLabels(labels);
    int index=0;
    int size=pos->pos[0].size();
    for(int i=0;i<size;i++)
    {
        model->setItem(i,0,new QStandardItem("1"));
        model->setItem(i,1,new QStandardItem(pos->pos[0].at(i)));
    }
    index+=size;
    size=pos->pos[1].size();
    for(int i=0;i<size;i++)
    {
        model->setItem(i+index,0,new QStandardItem("2"));
        model->setItem(i+index,1,new QStandardItem(pos->pos[1].at(i)));
    }
    index+=size;
    size=pos->pos[2].size();
    for(int i=0;i<size;i++)
    {
        model->setItem(i+index,0,new QStandardItem("3"));
        model->setItem(i+index,1,new QStandardItem(pos->pos[2].at(i)));
    }
    index+=size;
    size=pos->pos[3].size();
    for(int i=0;i<size;i++)
    {
        model->setItem(i+index,0,new QStandardItem("4"));
        model->setItem(i+index,1,new QStandardItem(pos->pos[3].at(i)));
    }
    index+=size;
    size=pos->pos[4].size();
    for(int i=0;i<size;i++)
    {
        model->setItem(i+index,0,new QStandardItem("5"));
        model->setItem(i+index,1,new QStandardItem(pos->pos[4].at(i)));
    }
    index+=size;
    size=pos->pos[5].size();
    for(int i=0;i<size;i++)
    {
        model->setItem(i+index,0,new QStandardItem("6"));
        model->setItem(i+index,1,new QStandardItem(pos->pos[5].at(i)));
    }
    index+=size;
    size=pos->pos[6].size();
    for(int i=0;i<size;i++)
    {
        model->setItem(i+index,0,new QStandardItem("7"));
        model->setItem(i+index,1,new QStandardItem(pos->pos[6].at(i)));
    }
    index+=size;
    size=pos->pos[7].size();
    for(int i=0;i<size;i++)
    {
        model->setItem(i+index,0,new QStandardItem("8"));
        model->setItem(i+index,1,new QStandardItem(pos->pos[7].at(i)));
    }

    centerView_1->setModel(model);
    centerView_1->setColumnWidth(0,50);
    centerView_1->setColumnWidth(1,60);

    index=0;
    //1-8通道波长
    QStandardItemModel *model2=new QStandardItemModel();
    labels.clear();
    labels<<"通道号"<<"波长";
    model2->setHorizontalHeaderLabels(labels);
    size=wave->wave[0].size();
    for(int i=0;i<size;i++)
    {
        model2->setItem(i+index,0,new QStandardItem("1"));
        model2->setItem(i+index,1,new QStandardItem(wave->wave[0].at(i)));
    }

    index+=size;
    size=wave->wave[1].size();
    for(int i=0;i<size;i++)
    {
        model2->setItem(i+index,0,new QStandardItem("2"));
        model2->setItem(i+index,1,new QStandardItem(wave->wave[1].at(i)));
    }
    index+=size;
    size=wave->wave[2].size();
    for(int i=0;i<size;i++)
    {
        model2->setItem(i+index,0,new QStandardItem("3"));
        model2->setItem(i+index,1,new QStandardItem(wave->wave[2].at(i)));
    }
    index+=size;
    size=wave->wave[3].size();
    for(int i=0;i<size;i++)
    {
        model2->setItem(i+index,0,new QStandardItem("4"));
        model2->setItem(i+index,1,new QStandardItem(wave->wave[3].at(i)));
    }
    index+=size;
    size=wave->wave[4].size();
    for(int i=0;i<size;i++)
    {
        model2->setItem(i+index,0,new QStandardItem("5"));
        model2->setItem(i+index,1,new QStandardItem(wave->wave[4].at(i)));
    }
    index+=size;
    size=wave->wave[5].size();
    for(int i=0;i<size;i++)
    {
        model2->setItem(i+index,0,new QStandardItem("6"));
        model2->setItem(i+index,1,new QStandardItem(wave->wave[5].at(i)));
    }
    index+=size;
    size=wave->wave[6].size();
    for(int i=0;i<size;i++)
    {
        model2->setItem(i+index,0,new QStandardItem("7"));
        model2->setItem(i+index,1,new QStandardItem(wave->wave[6].at(i)));
    }
    index+=size;
    size=wave->wave[7].size();
    for(int i=0;i<size;i++)
    {
        model2->setItem(i+index,0,new QStandardItem("8"));
        model2->setItem(i+index,1,new QStandardItem(wave->wave[7].at(i)));
    }

    centerView_2->setModel(model2);
    centerView_2->setColumnWidth(0,50);
    centerView_2->setColumnWidth(1,60);


    QStringList tempList;
    if(tempCheckBox->isChecked())
    {
        index=0;
        double aT=0.0;
        double TT=0.0;
        QStandardItemModel *model3=new QStandardItemModel();
        labels.clear();
        labels<<"通道号"<<"温度";
        model3->setHorizontalHeaderLabels(labels);
        size=wave->wave[0].size();
        for(int i=0;i<size;i++)
        {
            model3->setItem(i+index,0,new QStandardItem("1"));
            aT=wave->wave[0].at(i).toDouble();
            TT=aT*aValue[0]-bValue[0];
            tempList.append(QString::number(TT,'f',3));
            model3->setItem(i+index,1,new QStandardItem(QString::number(TT,'f',3)));
        }

        index+=size;
        size=wave->wave[1].size();
        for(int i=0;i<size;i++)
        {
            model3->setItem(i+index,0,new QStandardItem("2"));
            aT=wave->wave[1].at(i).toDouble();
            TT=aT*aValue[1]-bValue[1];
            tempList.append(QString::number(TT,'f',3));
            model3->setItem(i+index,1,new QStandardItem(QString::number(TT,'f',3)));
        }
        index+=size;
        size=wave->wave[2].size();
        for(int i=0;i<size;i++)
        {
            model3->setItem(i+index,0,new QStandardItem("3"));
            aT=wave->wave[2].at(i).toDouble();
            TT=aT*aValue[2]-bValue[2];
            tempList.append(QString::number(TT,'f',3));
            model3->setItem(i+index,1,new QStandardItem(QString::number(TT,'f',3)));
        }
        index+=size;
        size=wave->wave[3].size();
        for(int i=0;i<size;i++)
        {
            model3->setItem(i+index,0,new QStandardItem("4"));
            aT=wave->wave[3].at(i).toDouble();
            TT=aT*aValue[3]-bValue[3];
            tempList.append(QString::number(TT,'f',3));
            model3->setItem(i+index,1,new QStandardItem(QString::number(TT,'f',3)));
        }
        index+=size;
        size=wave->wave[4].size();
        for(int i=0;i<size;i++)
        {
            model3->setItem(i+index,0,new QStandardItem("5"));
            aT=wave->wave[4].at(i).toDouble();
            TT=aT*aValue[4]-bValue[4];
            tempList.append(QString::number(TT,'f',3));
            model3->setItem(i+index,1,new QStandardItem(QString::number(TT,'f',3)));
        }
        index+=size;
        size=wave->wave[5].size();
        for(int i=0;i<size;i++)
        {
            model3->setItem(i+index,0,new QStandardItem("6"));
            aT=wave->wave[5].at(i).toDouble();
            TT=aT*aValue[5]-bValue[5];
            tempList.append(QString::number(TT,'f',3));
            model3->setItem(i+index,1,new QStandardItem(QString::number(TT,'f',3)));
        }
        index+=size;
        size=wave->wave[6].size();
        for(int i=0;i<size;i++)
        {
            model3->setItem(i+index,0,new QStandardItem("7"));
            aT=wave->wave[6].at(i).toDouble();
            TT=aT*aValue[6]-bValue[6];
            tempList.append(QString::number(TT,'f',3));
            model3->setItem(i+index,1,new QStandardItem(QString::number(TT,'f',3)));
        }
//        index+=size;
//        size=wave->wave[7].size();
//        for(int i=0;i<size;i++)
//        {
//            model3->setItem(i+index,0,new QStandardItem("8"));
//            model3->setItem(i+index,1,new QStandardItem(wave->wave[7].at(i)));
//        }

        centerView_3->setModel(model3);
        centerView_3->setColumnWidth(0,50);
        centerView_3->setColumnWidth(1,60);

    }
    else
    {
        QStandardItemModel *model4=new QStandardItemModel();
        centerView_3->setModel(model4);
    }


//    if(tantouList.size()<3)
//    {
//        tantouList.append(QString::number(wave->temperature2,'f',3));
//    }
//    else if(tantouList.size()>=3)
//    {
//        if(tantouList.at(0)==tantouList.at(1)&&
//                tantouList.at(0)==tantouList.at(2)&&
//                tantouList.at(2)==tantouList.at(1))
//        {


            QString txt="";
            QString path="";
            for(int i=0;i<8;i++)
            {
                txt+=QString(",%1").arg(i+1);
        //        txt.clear();
                size=wave->wave[i].size();

                for(int j=0;j<size;j++)
                {
                    txt+=QString(",%1").arg(wave->wave[i].at(j));
                }
        //        txt+=QString(",%1").arg(wave->temperature);
        //        txt+=QString(",%1").arg(wave->temperature2);
        //        txt+=QString(",%1").arg(wenduValue);

        //        path=QString(".\\user\\WellConfig\\#%1\\%2.csv").arg(i+1)
        //                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
        //        FILEWARE::writeWaveLengthC(path,txt);

            }

            txt+=QString(",%1").arg(wave->temperature);
            txt+=QString(",%1").arg(wave->temperature2);
        //    txt+=QString(",%1").arg(wenduValue);

            path=QString(".\\user\\WellConfig\\%1.csv").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
            FILEWARE::writeWaveLengthC(path,txt);



            if(tempCheckBox->isChecked())
            {
                path=QString(".\\user\\TempConfig\\%1.csv").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
                txt.clear();
                for(int i=0;i<tempList.size();i++)
                {
                    txt+=QString(",%1").arg(tempList.at(i));
                }
                FILEWARE::writeWaveLengthC(path,txt);
            }

//        }
//        tantouList.clear();
//    }

}


/***********************************************************************************************
*函数名 ：psgCombo_change
*函数功能描述 ：改变传输模式
*函数参数 ：index 组合框序列号
*函数返回值 ：无
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
void MainWindow::psgCombo_change(int index)
{
    if(myCom->isOpen())
    {
        error("请先关闭串口");
        return;
    }

    switch(index)
    {
    case 0:
        OVER_LENGTH=CHANNEL_4000;
        break;
    case 1:
        OVER_LENGTH=CHANNEL_8000;
        break;
    case 2:
        OVER_LENGTH=CHANNEL_12000;
        break;
    case 3:
        OVER_LENGTH=CHANNEL_28000;
        break;
    case 4:
        OVER_LENGTH=CHANNEL_32000;
        break;
    case 5:
        OVER_LENGTH=CHANNEL_36000;
        break;
    }
    END_LENGTH=OVER_LENGTH+WENDU_TAIL+WENDU_TAIL2;
//    progress->setMaximum(END_LENGTH);
    setWindowTitle("光纤数据采集上位机-"+QString::number(END_LENGTH));
}


//        QString context=textBrowser->toPlainText();
//        context+=str;
//        textBrowser->setText(context);
//        QTextCursor cursor = textBrowser->textCursor();
//        cursor.movePosition(QTextCursor::End);
//        textBrowser->setTextCursor(cursor);


/***********************************************************************************************
*函数名 ：hasTailCombo_selected
*函数功能描述 ：是否有电路板温度
*函数参数 ：index=false 无 反之有
*函数返回值 ：无
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
void MainWindow::hasTailCombo_selected(bool index)
{
    if(myCom->isOpen())
    {
        error("请先关闭串口");
        return;
    }

    if(!index)
    {
        WENDU_TAIL=TAIL_NO_DATA;
        END_LENGTH=OVER_LENGTH+WENDU_TAIL+WENDU_TAIL2;
    }
    else
    {
        WENDU_TAIL=TAIL_HAS_DATA;
        END_LENGTH=OVER_LENGTH+WENDU_TAIL+WENDU_TAIL2;
    }
//    progress->setMaximum(END_LENGTH);
    setWindowTitle("光纤数据采集上位机-"+QString::number(END_LENGTH));
}


/***********************************************************************************************
*函数名 ：slotTimer
*函数功能描述 ：防止串口死机 死机唤醒
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
void MainWindow::slotTimer()
{
    int value1=recvLen;

    QTime dieTime = QTime::currentTime().addMSecs(2000);
    while(QTime::currentTime()<dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents,500);
    }

    int value2=recvLen;

    if(value1==value2&&value1!=0)
    {
        recvLen=0;
        QByteArray array;
        array.append(0xFF);
        myCom->write(array);
    }
}

/***********************************************************************************************
*函数名 ：slotTemperatureCombo
*函数功能描述 ：是否有探头温度
*函数参数 ：index=false 无 反之有
*函数返回值 ：无
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
void MainWindow::slotTemperatureCombo(bool index)
{
    if(myCom->isOpen())
    {
        error("请先关闭串口");
        return;
    }

    if(!index)
    {
        WENDU_TAIL2=TAIL_WITHOUT_TEMP;
        END_LENGTH=OVER_LENGTH+WENDU_TAIL+WENDU_TAIL2;
    }
    else
    {
        WENDU_TAIL2=TAIL_WITH_TEMP;
        END_LENGTH=OVER_LENGTH+WENDU_TAIL+WENDU_TAIL2;
    }
    setWindowTitle("光纤数据采集上位机-"+QString::number(END_LENGTH));
}


/***********************************************************************************************
*函数名 ：power
*函数功能描述 ：计算10的n次幂
*函数参数 ：num 几次
*函数返回值 ：幂
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
uint MainWindow::power(ushort num)
{
    uint value=1;
    while(num-->0)
    {
        value*=10;
    }
    return value;
}


/***********************************************************************************************
*函数名 ：plotAgain
*函数功能描述 ：绘制阈值改变后的标准具
*函数参数 ：yinzi阈值改变幅度
*函数返回值 ：无
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
void MainWindow::plotAgain(double yinzi)
{
    topCustomPlot->graph(2)->clearData();
    QVector<double> x(STAND_DATA_LENGTH), y(STAND_DATA_LENGTH);
    for (int i=0; i<STAND_DATA_LENGTH; ++i)
    {
      x[i] = i*1.0;
      y[i] = stand_buffer[i]*yinzi;
    }
    topCustomPlot->graph(2)->setData(x,y);

    int xLower=topCustomPlot->xAxis->range().lower;
    int xUpper=topCustomPlot->xAxis->range().upper;
    int yLower=topCustomPlot->yAxis->range().lower;
    int yUpper=topCustomPlot->yAxis->range().upper;

    topCustomPlot->xAxis->setRange(xLower,xUpper);
    topCustomPlot->yAxis->setRange(yLower,yUpper);
    topCustomPlot->replot();
}
