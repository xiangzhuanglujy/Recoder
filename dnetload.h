#ifndef DNETLOAD_H
#define DNETLOAD_H

#include <QDialog>
#include <QUrlInfo>
#include <QModelIndex>

class QFtp;
class QLabel;
class QTextBrowser;
class QPushButton;
class QTreeWidget;
class QProgressBar;
class QTreeView;


class DNetLoad : public QDialog
{
    Q_OBJECT
public:
    explicit DNetLoad(QWidget *parent = 0);
    
signals:
    
public slots:
    void ftpCommandStarted(int);                //开始
    void ftpCommandFinished(int,bool);          //完成
    void loadBtn_click();                       //下载
    void addToList(const QUrlInfo &urlInfo);
    void progressBar_update(qint64,qint64);
    void rootTree_click(QModelIndex);

protected:
    void closeEvent(QCloseEvent *);

private:
    QFtp *ftp;
    QLabel *label;
    QTextBrowser *textBrowser;
    QPushButton *loadBtn;
    QTreeWidget *tree;
    QTreeView *rootTree;

    QProgressBar *progressBar;
};

#endif // DNETLOAD_H
