#include "plotdialog.h"
#include "qcustomplot.h"
#include "define.h"

#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include <QLabel>


#define MAP_X_END                   300


PlotDialog::PlotDialog(QWidget *parent) :
    QDialog(parent)
{
    setMinimumSize(600,480);

    LE_path=new QLineEdit();
    LE_path->setReadOnly(true);

    PB_export=new QPushButton("导入文件");

    customPlot=new QCustomPlot();
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");
    customPlot->addGraph();

    LB_result=new QLineEdit();
    LB_result->setReadOnly(true);


    QGridLayout *grid=new QGridLayout(this);
    grid->setColumnStretch(0,3);
    grid->setColumnStretch(1,1);
    grid->addWidget(LE_path,0,0);
    grid->addWidget(PB_export,0,1);
    grid->addWidget(LB_result,2,0,1,2);
    grid->addWidget(customPlot,1,0,1,2);

    //信号槽
    connect(PB_export,SIGNAL(clicked()),this,SLOT(PB_export_clicked()));

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
void PlotDialog::plotMap(QCustomPlot *plot, double a, double b, int size)
{
    QVector<double> x(size),y(size);

    for(int i=0;i<size;i++)
    {
        x[i]=i*1.0;
        y[i]=a+b*x[i];
    }
    plot->graph(0)->setData(x,y);
    plot->graph(0)->rescaleAxes();
    plot->replot();
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
void PlotDialog::PB_export_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,"导入文件",".","CSV(*.csv)");
    if(path.isEmpty())
    {
        return;
    }
    LE_path->setText(path);

    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        QString lineS;

        double xValue[1000];
        double yValue[1000];
        int index=0;

        while(!stream.atEnd())
        {
            lineS=stream.readLine();
            QStringList list=lineS.split(',');
            //qDebug()<<list.at(0).toDouble();
            //qDebug()<<list.at(1).toDouble();
            //qDebug()<< stream.readLine();
            xValue[index]=list.at(0).toDouble();
            yValue[index]=list.at(1).toDouble();
            index++;
        }

        double x1=squareAnd(xValue,0,index);
        double x2=accumulationAnd(xValue,0,index);
        double x3=product(xValue,yValue,0,index);
        double y1=accumulationAnd(yValue,0,index);

//        qDebug()<<x1<<x2<<x3<<y1;
        double a=(x1*y1-x2*x3)/(x1*index-x2*x2);
        double b=(index*x3-x2*y1)/(x1*index-x2*x2);
        qDebug()<<"a"<<a<<"b"<<b;
        LB_result->setText(QString("y=%1+%2*x").arg(a).arg(b));

        plotMap(customPlot,a,b,MAP_X_END);

        file.close();
    }
}

