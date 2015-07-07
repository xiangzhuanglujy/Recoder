/*******************************************************
 *暂时没用到
 *主要用于导入阮志林那边的数据
 *
 *
 *
 *
 *
 *
 *****************************************************/



#ifndef PLOTDIALOG_H
#define PLOTDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class QCustomPlot;
class QLabel;


class PlotDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PlotDialog(QWidget *parent = 0);
    
    void plotMap(QCustomPlot *plot,double a,double b,int size);

signals:
    
public slots:
    void PB_export_clicked();


private:
    QLineEdit *LE_path;
    QPushButton *PB_export;
    QCustomPlot *customPlot;
    QLineEdit *LB_result;

};

#endif // PLOTDIALOG_H
