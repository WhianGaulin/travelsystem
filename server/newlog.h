#ifndef NEWLOG_H
#define NEWLOG_H
#include <QString>
#include <QVector>
#include <QMultiMap>
#include <QDebug>
#include "dataitem.h"
#include <QFile>
#include <QFileDevice>
#include <QPair>
#include <QIODevice>
typedef QMultiMap<int,dataitem> dataset;
int CityToInt(QString city);
QString IntToCity(int num);
class newlog
{
public:
    newlog();
    static QMultiMap<int,dataitem> dataset;//声明
    void write_to_log(QString &str);
    static int distance[15][15];
};

#endif // NEWLOG_H
