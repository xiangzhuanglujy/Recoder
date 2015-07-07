#include "FILEWR.h"

#include <QFile>
#include <QDebug>
#include <QTextStream>


bool WriteCSV(QString str)
{
    QFile file(SYSTEMLOG);

    if(file.open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text))
    {
        file.write(str.toAscii());
        file.close();
        return true;
    }
    return false;
}


bool WriteCSV(QStringList list)
{
    QFile file(SYSTEMLOG);

    if(file.open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text))
    {
        for(int i=0;i<list.size();i++)
        {
            file.write(list.at(i).toAscii()+"\n");
        }
        file.close();
        return true;
    }
    return false;
}

bool StoreWaveLength(QStringList context)
{
    QFile file(WAVELENGTH_STORE_PATH);

    if(context.size()<2)
    {
        return false;
    }

    if(file.open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text))
    {
        QString temp=context.at(0)+","+context.at(1)+"\n";
//        for(int i=0;i<context.size();i++)
//        {
//            file.write(context.at(i).toAscii()+"\n");
//        }
        file.write(temp.toAscii());
        file.close();
        return true;
    }
    return false;
}

QStringList ReadCSV(QString path)
{
    QFile csvFile(path);
    QStringList list;
    list.clear();

    if(csvFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&csvFile);
        while(!stream.atEnd())
        {
            list.push_back(stream.readLine());
        }
        csvFile.close();
    }

//    Q_FOREACH(QString str,list)
//    {
//        qDebug()<<str<<endl;
//    }
    return list;
}

bool writeLog(QString date,QString context,QString path)
{
    QFile file(path);
    if(file.open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text))
    {
        QTextStream stream(&file);
        stream<<date<<"\t"<<context<<"\n";
        file.close();
    }
    return true;
}

bool writeStanded_range(QString context)
{
    QFile file(STANDED_RANGE);
    if(file.open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text))
    {
        QTextStream stream(&file);
        stream<<context;
        file.close();
        return true;
    }
    return false;
}

bool writeStanded_pos(QString context)
{
    QFile file(STANDED_POS);
    if(file.open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text))
    {
        QTextStream stream(&file);
        stream<<context;
        file.close();
        return true;
    }
    return false;
}
