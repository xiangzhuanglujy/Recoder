#ifndef WOSCILLOSCOPE_H
#define WOSCILLOSCOPE_H

#include <QDialog>

class QDial;
class QLineEdit;
class QPushButton;

class WOscilloscope : public QDialog
{
    Q_OBJECT
public:
    explicit WOscilloscope(QWidget *parent = 0);
    
signals:
    void sendByte(QByteArray byte);
    
public slots:
    void dial_move(int);
    void okBtn_click();
    
private:
    QDial *dial;
    QLineEdit *edit;
    QPushButton *okBtn;
};

#endif // WOSCILLOSCOPE_H
