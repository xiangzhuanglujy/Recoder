#include "dnetload.h"

#include <QFtp>
#include <QLabel>
#include <QTextBrowser>
//#include <QVBoxLayout>
#include <QDebug>
#include <QFile>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QGridLayout>
#include <QProgressBar>
#include <QTreeView>
#include <QDirModel>


DNetLoad::DNetLoad(QWidget *parent) :
    QDialog(parent)
{
    showMaximized();

    label=new QLabel(this);
    textBrowser=new QTextBrowser();
    loadBtn=new QPushButton("下载");
    tree=new QTreeWidget();
    rootTree=new QTreeView();
    progressBar=new QProgressBar();

    QStringList list;
    list<<"name"<<"size"<<"owner"<<"group"<<"date";
    tree->setHeaderLabels(list);
    tree->setColumnWidth(0,250);

    QDirModel *model = new QDirModel;
    rootTree->setModel(model);
    rootTree->setRootIndex(model->index("/"));
    rootTree->show();
    rootTree->setColumnWidth(0,300);


    QGridLayout *layout=new QGridLayout();
    layout->addWidget(tree,0,0,10,1);
    layout->addWidget(rootTree,0,1,18,1);
    layout->addWidget(loadBtn,10,0,1,1);
    layout->addWidget(textBrowser,11,0,5,1);
    layout->addWidget(progressBar,16,0);
    layout->addWidget(label,17,0);
    setLayout(layout);

    ftp = new QFtp(this);
    ftp->connectToHost("ftp.qt.nokia.com"); //连接到服务器
    ftp->login();   //登录
    ftp->cd("qt/source");   //跳转到“qt”目录下
    ftp->list();

    connect(ftp,SIGNAL(commandStarted(int)),this,SLOT(ftpCommandStarted(int)));
    //当每条命令开始执行时发出相应的信号
    connect(ftp,SIGNAL(commandFinished(int,bool)),this,SLOT(ftpCommandFinished(int,bool)));
    connect(ftp,SIGNAL(listInfo(QUrlInfo)),this,SLOT(addToList(QUrlInfo)));
    connect(ftp,SIGNAL(dataTransferProgress(qint64,qint64)),
            this,SLOT(progressBar_update(qint64,qint64)));

    connect(rootTree,SIGNAL(clicked(QModelIndex)),this,SLOT(rootTree_click(QModelIndex)));

    connect(loadBtn,SIGNAL(clicked()),this,SLOT(loadBtn_click()));
}

void DNetLoad::ftpCommandStarted(int)
{
    if(ftp->currentCommand() == QFtp::ConnectToHost)
    {
        label->setText("正在连接到服务器…");
    }
    else if(ftp->currentCommand() == QFtp::Login)
    {
        label->setText("正在登录…");
    }
    else if(ftp->currentCommand() == QFtp::Get)
    {
        label->setText("正在下载…");
    }
    else if (ftp->currentCommand() == QFtp::Close)
    {
        label->setText("正在关闭连接…");
    }
    else if (ftp->currentCommand() == QFtp::List)
    {
        label->setText("正在加载…");
    }
}

void DNetLoad::ftpCommandFinished(int, bool error)
{
    if(ftp->currentCommand() == QFtp::ConnectToHost)
    {
        if(error)
            label->setText(QString("连接服务器出现错误：%1").arg(ftp->errorString()));
        else
            label->setText("连接到服务器成功");
    }
    else if (ftp->currentCommand() == QFtp::Login)
    {
        if(error)
            label->setText(QString("登录出现错误：%1").arg(ftp->errorString()));
        else
            label->setText("登录成功");
    }
    else if (ftp->currentCommand() == QFtp::List)
    {
        if(error)
            label->setText(QString("加载出现错误：%1").arg(ftp->errorString()));
        else
            label->setText("加载完成");
    }
    else if(ftp->currentCommand() == QFtp::Get)
    {
        if(error)
        {
            label->setText(QString("下载出现错误：%1").arg(ftp->errorString()));
        }
        else
        {
            QFile file(".\\load.txt");
            QByteArray byte=ftp->readAll();
            QString str;
            str.prepend(byte);
            textBrowser->setText(str);

            if(file.open(QIODevice::WriteOnly|QIODevice::Text))
            {
                file.write(byte);
                file.close();
            }
            label->setText("已经完成下载");
        }
    }
    else if (ftp->currentCommand() == QFtp::Close)
    {
        label->setText("已经关闭连接");
    }
}

void DNetLoad::loadBtn_click()
{
    textBrowser->clear();
    ftp->get("md5sums.txt");  //下载“INSTALL”文件
}

void DNetLoad::addToList(const QUrlInfo &urlInfo)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, urlInfo.name());
    item->setText(1, QString::number(urlInfo.size()));
    item->setText(2, urlInfo.owner());
    item->setText(3, urlInfo.group());
    item->setText(4, urlInfo.lastModified().toString("yyyy-MM-dd"));

    tree->addTopLevelItem(item);

//    if(!tree->currentItem())
//    {
//        tree->setCurrentItem(tree->topLevelItem(0));
////        tree->setEnabled(true);
//    }
}

void DNetLoad::progressBar_update(qint64 value1, qint64 value2)
{
    progressBar->setMaximum(value2);
    progressBar->setValue(value1);
}

void DNetLoad::rootTree_click(QModelIndex model)
{
    Q_UNUSED(model);
//    qDebug()<<model.column()<<model.row();
//    qDebug()<<"index";
}

void DNetLoad::closeEvent(QCloseEvent *)
{
    ftp->close();   //关闭连接
}
