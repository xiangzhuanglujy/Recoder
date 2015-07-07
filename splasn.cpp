#include "splasn.h"

#include "nehewidget.h"

#include <QGridLayout>

Splasn::Splasn(QWidget *parent) :
    QSplashScreen(parent)
{
    setFixedSize(640,480);

    nehe=new NeHeWidget();

    QGridLayout *layout=new QGridLayout();
    layout->addWidget(nehe);
    setLayout(layout);
}


