#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QDebug>


#include "mainwindow.h"


/***********************************************************************************************
*函数名 ：main
*函数功能描述 ：程序入口函数
*函数参数 ：argc argv
*函数返回值 ：int
*作者 ：lujy
*函数创建日期 ：2015/07/01
*函数修改日期 ：
*修改人 ：
*修改原因 ：
*版本 ：
*历史版本 ：1.0
***********************************************************************************************/
int main(int argc,char **argv)
{
    //创建主App
    QApplication app(argc,argv);

    //设置支持中文显示
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));

    //加载样式表
    QFile file(":/images/style1128.qss");
    if(file.open(QIODevice::ReadOnly))
    {
        QString style=file.readAll();
        app.setStyleSheet(style);
        file.close();
    }

    //创建程序主窗口
    MainWindow main_window;
    main_window.show();

    //开始消息循环
    return app.exec();
}
