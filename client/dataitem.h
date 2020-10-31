#ifndef DATAITEM_H
#define DATAITEM_H
#define MAX 15
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QObject>
#include <qdatastream.h>

class dataitem{
public:
    dataitem(){}
    dataitem(int from,int to,int departureHour,int arrivalHour,int spendTime,int cost,QString shiftID,QString vehicle){
        this->from=from;
        this->to=to;
        this->depatureTime=departureHour;
        this->arrivalTime=arrivalHour;
        this->spendTime=spendTime;
        this->cost=cost;
        this->shiftID=shiftID;
        this->vehicle=vehicle;
    }
    int from;
    int to;
    QString vehicle;
    QString shiftID;
    int depatureTime;
    int arrivalTime;
    int spendTime;
    int cost;
    friend QDataStream& operator<<(QDataStream& out,const dataitem&item){
        out<<item.from<<item.to<<item.depatureTime<<item.arrivalTime<<item.spendTime
          <<item.cost<<item.shiftID<<item.vehicle;
        return out;
    }
    friend QDataStream& operator>>(QDataStream& in,dataitem&item ){
        in>>item.from>>item.to>>item.depatureTime>>item.arrivalTime>>item.spendTime
           >>item.cost>>item.shiftID>>item.vehicle;
        return in;
    }
};

class traveler_state
{
public:
    int my_cost=0;
    int my_spend_Secs=0;
    int IF_on_the_shift=0;
    int IF_start=1;
    dataitem shift;
    QString location;//用文字表述
    QDateTime shift_arrive_DateTime;
    friend QDataStream& operator<<(QDataStream& out,const traveler_state& item){
        out<<item.my_cost<<item.my_spend_Secs<<item.IF_on_the_shift
          <<item.IF_start<<item.shift<<item.location<<item.shift_arrive_DateTime;
        return out;
    }
    friend QDataStream& operator>>(QDataStream& in,traveler_state& item){
        in>>item.my_cost>>item.my_spend_Secs>>item.IF_on_the_shift
         >>item.IF_start>>item.shift>>item.location>>item.shift_arrive_DateTime;
        return in;
    }
};

typedef struct tag_planItem planItem;
typedef struct tag_planItem{
    QDateTime departureTime;
    QDateTime arriveTime;
    dataitem  item;
    friend QDataStream& operator<<(QDataStream &out,const planItem&item){
        out<<item.departureTime<<item.arriveTime<<item.item;
        return out;
    }
    friend QDataStream& operator>>(QDataStream &in,planItem &item){
        in>>item.departureTime>>item.arriveTime>>item.item;
        return in;
    }
} planItem;
typedef QVector<planItem> plan;

#endif // DATAITEM_H
