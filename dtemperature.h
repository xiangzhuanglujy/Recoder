#ifndef DTEMPERATURE_H
#define DTEMPERATURE_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;

class DTemperature : public QDialog
{
    Q_OBJECT
public:
    explicit DTemperature(QWidget *parent = 0);
    
    void read_check(int daihao,int addr,int *chech_low,int *check_hign);                //读校验
    void write_check(int daihao,int value,int addr,int *chech_low,int *check_hign);     //写校验
signals:
    void tempAndTime(QByteArray);
    
public slots:
    void okBtn_click();             //发送温度时间
    void runBtn_click();            //运行
    void pauseBtn_click();          //暂停
    void stop_click();              //停止
    
private:
    QLabel *lb1;
    QLabel *lb2;
    QLabel *lb3;
    QLabel *lb1_1;
    QLabel *lb3_1;

    QComboBox *combo;
    QLineEdit *tempLE;
    QLineEdit *timeLE;

    QPushButton *okBtn;

    QPushButton *runBtn;
    QPushButton *pauseBtn;
    QPushButton *stopBtn;
};

#endif // DTEMPERATURE_H
