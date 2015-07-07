#ifndef SPLASN_H
#define SPLASN_H

#include <QSplashScreen>

class NeHeWidget;

class Splasn : public QSplashScreen
{
    Q_OBJECT
public:
    explicit Splasn(QWidget *parent = 0);
    
signals:
    
public slots:
    
private:
    NeHeWidget *nehe;
};

#endif // SPLASN_H
