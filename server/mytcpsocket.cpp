#include "mytcpsocket.h"

myTcpsocket::myTcpsocket(int socketDes,QObject *parent):
    QTcpSocket (parent),
    m_socketDesc(socketDes)
{
    mytraveler=new traveler();
    myAntSystem=new AntSystem();
    connect(this,SIGNAL(readyRead()),this,SLOT(RecvData()));
}

void myTcpsocket::RecvData(){
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_5_12);
    demand his_demand;
    in>>his_demand.IF_Change>>his_demand.start_City>>his_demand.End_City>>his_demand.mid_City
            >>his_demand.startTime>>his_demand.latestTime>>his_demand.strategy
            >>his_demand.duration_time_int;
    qDebug()<<his_demand.start_City<<his_demand.End_City<<his_demand.mid_City
           <<his_demand.startTime<<his_demand.latestTime<<his_demand.strategy
          <<his_demand.duration_time_int;

    mytraveler->set_Value(his_demand.start_City,his_demand.End_City,
                          his_demand.mid_City,his_demand.startTime,
                          his_demand.latestTime,his_demand.strategy,his_demand.duration_time_int);
    mytraveler->Earlist_StartTime=his_demand.startTime;
    int IF_Ant_Search=0;
    if(his_demand.strategy==tr("费用最少")){
        qDebug()<<"费用最少";
        mytraveler->search(1);
    }
    else if(his_demand.strategy==tr("时间最少")){
        mytraveler->search(2);
    }
    else if(his_demand.strategy==tr("限时费用最少")){
        if(his_demand.mid_City.size()>2){//如果途经城市太多了，就果断用蚁群算法
            IF_Ant_Search=1;
            qDebug()<<"限时费用最少 set_Value for traveler of AntSystem...";
            myAntSystem->my_traveler.set_Value(his_demand.start_City,his_demand.End_City,
                                            his_demand.mid_City,his_demand.startTime,
                                            his_demand.latestTime,his_demand.strategy,his_demand.duration_time_int);
            myAntSystem->InitData();
            myAntSystem->Search();
        }
        else{//途经城市少于等于2个，用深度优先搜索
            mytraveler->search(3);
            if(mytraveler->dfs_succ){
                mytraveler->make_myplan();
            }
        }
    }

    if(his_demand.IF_Change){//点击的是改变计划键
        traveler *p;
        if(IF_Ant_Search)   p=&(myAntSystem->my_traveler);
        else                p=mytraveler;
        plan_packet s;
        s.IF_Changed_Plan=1;
        s.myplan=p->myplan;
        s.TotalCost=p->cost[CityToInt(p->EndCity)];
        s.final_arriveTime=p->arrive_time[CityToInt(p->EndCity)];
        sendData(s);
        p=nullptr;
    }
    else{//点击的是确认键
        traveler *p;
        if(IF_Ant_Search)   p=&(myAntSystem->my_traveler);
        else                p=mytraveler;
        plan_packet s;
        s.IF_Changed_Plan=0;
        s.myplan=p->myplan;
        s.TotalCost=p->cost[CityToInt(p->EndCity)];
        s.TotalSpendTime=int_to_interval(sub(p->startTime
                                             ,p->arrive_time[CityToInt(p->EndCity)]));
        sendData(s);
        p=nullptr;
    }
    //mytraveler->output_myplan();
}

void myTcpsocket::sendData(plan_packet &s){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);

    if(s.IF_Changed_Plan){//传送的是变更的计划
        out<<s.IF_Changed_Plan<<s.myplan<<s.TotalCost<<s.final_arriveTime;
    }
    else {
        out<<s.IF_Changed_Plan<<s.myplan<<s.TotalCost<<s.TotalSpendTime;
    }

    write(block);



}


