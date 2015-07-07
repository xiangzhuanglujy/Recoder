#include "woscilloscope.h"

#include <QDial>
#include <QDebug>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>


WOscilloscope::WOscilloscope(QWidget *parent) :
    QDialog(parent)
{
    setFixedSize(350,300);

    dial=new QDial();
    edit=new QLineEdit();
    okBtn=new QPushButton("ok");

//    qDebug()<<dial->wrapping()<<dial->notchSize()<<dial->singleStep();
    dial->setWrapping(true);
//    dial->setSingleStep(1);
    QHBoxLayout *layout=new QHBoxLayout(this);
    layout->addWidget(edit);
    layout->addWidget(dial);
    layout->addWidget(okBtn);

    connect(dial,SIGNAL(valueChanged(int)),this,SLOT(dial_move(int)));
    connect(okBtn,SIGNAL(clicked()),this,SLOT(okBtn_click()));
}

void WOscilloscope::dial_move(int value)
{
//    qDebug()<<value;
    setWindowTitle(QString("%1").arg(value));
}

void WOscilloscope::okBtn_click()
{
    QByteArray byte=edit->text().toAscii();
    emit sendByte(byte);

//    qDebug()<<byte.at(0);
}
