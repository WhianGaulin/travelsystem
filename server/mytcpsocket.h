#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H
#include <QTcpSocket>
#include <QHostAddress>
#include <QMap>
#include <QtNetwork>
#include "dataitem.h"
#include "traveler.h"
#include "aco.h"
struct tag_demand{
    int IF_Change;
    QString start_City;
    QString End_City;
    QVector<QString> mid_City;
    QDateTime startTime;
    QDateTime latestTime;
    QString strategy;
    QVector<int> duration_time_int;
    traveler_state travelerState;
};
typedef struct tag_demand demand;


struct tag_plan{
    int IF_Changed_Plan;
    QVector<planItem> myplan;
    int TotalCost;
    QString TotalSpendTime;
    QDateTime final_arriveTime;
};
typedef struct tag_plan plan_packet;

class myTcpsocket:public QTcpSocket
{
    Q_OBJECT
public:
    myTcpsocket();
    myTcpsocket(int SocketDesc,QObject *parent=nullptr);
    traveler *mytraveler;
    AntSystem *myAntSystem;
    newlog mynewlog;
private:
    int m_socketDesc;//有用??
signals:
    void transfer_RecvData(int socketDesc,QByteArray data);
public slots:
    void sendData(plan_packet &);
    void RecvData();

};

#endif // MYTCPSOCKET_H
