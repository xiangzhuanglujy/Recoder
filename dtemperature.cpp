#include "dtemperature.h"

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>
#include <QTime>
#include <QCoreApplication>

DTemperature::DTemperature(QWidget *parent) :
    QDialog(parent)
{
//    setAttribute (Qt::WA_DeleteOnClose);
    setMinimumSize(300,250);

    lb1=new QLabel("温度上限");
    lb2=new QLabel("温度阶数");
    lb3=new QLabel("加温时间");
    lb1_1=new QLabel("°C");
    lb3_1=new QLabel("minute");

    QStringList list;
    list<<"1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7"<<"8"<<"9"<<"10"
          <<"11"<<"12"<<"13"<<"14"<<"15"<<"16"<<"17"<<"18"<<"19"<<"20"
            <<"21"<<"22"<<"23"<<"24"<<"25"<<"26"<<"27"<<"28"<<"29"<<"30";
    combo=new QComboBox();
    combo->addItems(list);

    timeLE=new QLineEdit();
    tempLE=new QLineEdit();
    QRegExp regExp("[0-9]{4}");
    timeLE->setValidator(new QRegExpValidator(regExp, this));
    regExp.setPattern("[0-9]{1,3}[.][0-9]");
    tempLE->setValidator(new QRegExpValidator(regExp, this));

    okBtn=new QPushButton("发送");
    runBtn=new QPushButton("运行");
    pauseBtn=new QPushButton("暂停");
    stopBtn=new QPushButton("停止");

    QGridLayout *grid=new QGridLayout();
    grid->addWidget(lb1,0,0);
    grid->addWidget(lb2,1,0);
    grid->addWidget(lb3,2,0);
    grid->addWidget(lb1_1,0,2);
    grid->addWidget(lb3_1,2,2);
    grid->addWidget(tempLE,0,1);
    grid->addWidget(combo,1,1);
    grid->addWidget(timeLE,2,1);
    grid->addWidget(okBtn,3,1);
    grid->addWidget(runBtn,4,0);
    grid->addWidget(pauseBtn,4,1);
    grid->addWidget(stopBtn,4,2);

    setLayout(grid);

    //
    connect(okBtn,SIGNAL(clicked()),this,SLOT(okBtn_click()));
    connect(runBtn,SIGNAL(clicked()),this,SLOT(runBtn_click()));
    connect(pauseBtn,SIGNAL(clicked()),this,SLOT(pauseBtn_click()));
    connect(stopBtn,SIGNAL(clicked()),this,SLOT(stop_click()));
}

void DTemperature::read_check(int daihao, int addr, int *chech_low, int *check_hign)
{
    int check=daihao*256+82+addr;
    *chech_low=check%256;
    *check_hign=check/256;
}

void DTemperature::okBtn_click()
{
    int temp=tempLE->text().toDouble()*10;
    int time=timeLE->text().toInt();
    int temp_low=temp%256;
    int temp_hign=temp/256;
    int time_low=time%256;
    int time_hign=time/256;
    int temp_addr=26+combo->currentIndex()*2;
    int time_addr=27+combo->currentIndex()*2;

    int check_low,check_hign;
    write_check(temp_addr,temp,0x01,&check_low,&check_hign);
    QByteArray array_temp,array_time;
    array_temp.append(0x81);
    array_temp.append(0x81);
    array_temp.append(0x43);
    array_temp.append(temp_addr);
    array_temp.append(temp_low);
    array_temp.append(temp_hign);
    array_temp.append(check_low);
    array_temp.append(check_hign);

    write_check(time_addr,time,0x01,&check_low,&check_hign);
    array_time.append(0x81);
    array_time.append(0x81);
    array_time.append(0x43);
    array_time.append(time_addr);
    array_time.append(time_low);
    array_time.append(time_hign);
    array_time.append(check_low);
    array_time.append(check_hign);

    emit tempAndTime(array_temp);
    QTime dieTime = QTime::currentTime().addMSecs(150);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    emit tempAndTime(array_time);
}

void DTemperature::runBtn_click()
{
    int check_low,check_hign;
    write_check(0x15,0x00,0x01,&check_low,&check_hign);
    int sz=0;
    QByteArray array_temp;
    array_temp.append(0x81);
    array_temp.append(0x81);
    array_temp.append(0x43);
    array_temp.append(0x15);
    array_temp.append(sz);
    array_temp.append(sz);
    array_temp.append(check_low);
    array_temp.append(check_hign);
    emit tempAndTime(array_temp);
}

void DTemperature::pauseBtn_click()
{
    int check_low,check_hign;
    write_check(0x15,0x04,0x01,&check_low,&check_hign);
    int sz=0;
    QByteArray array_temp;
    array_temp.append(0x81);
    array_temp.append(0x81);
    array_temp.append(0x43);
    array_temp.append(0x15);
    array_temp.append(0x04);
    array_temp.append(sz);
    array_temp.append(check_low);
    array_temp.append(check_hign);
    emit tempAndTime(array_temp);
}

void DTemperature::stop_click()
{
    int check_low,check_hign;
    write_check(0x15,0x0C,0x01,&check_low,&check_hign);
    int sz=0;
    QByteArray array_temp;
    array_temp.append(0x81);
    array_temp.append(0x81);
    array_temp.append(0x43);
    array_temp.append(0x15);
    array_temp.append(0x0C);
    array_temp.append(sz);
    array_temp.append(check_low);
    array_temp.append(check_hign);
    emit tempAndTime(array_temp);
}

void DTemperature::write_check(int daihao,int value,int addr,int *chech_low,int *check_hign)
{
    int check=(daihao*256+67+value+addr)%65536;
    *chech_low=check%256;
    *check_hign=check/256;
}
