#ifndef CTLTEMPTD_H
#define CTLTEMPTD_H

#include <QThread>

class Win_QextSerialPort;

class CtlTempTd : public QThread
{
    Q_OBJECT
public:
    explicit CtlTempTd(QObject *parent = 0);
    ~CtlTempTd();
    
signals:

protected:
    void run();
    
public slots:
    void readCom();
    void getTempTime(QByteArray array1);
    void recvByte(QByteArray array);
    
private:
    Win_QextSerialPort *myCom;
};

#endif // CTLTEMPTD_H
