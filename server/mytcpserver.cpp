#include "mytcpserver.h"

myTcpserver::myTcpserver()
{
    read_pointer=0;
    qDebug()<<"构造了 myTcpserver";
}

void myTcpserver::incomingConnection(qintptr socketDes){
    qDebug()<<"incommingConnection..."<<socketDes;
    m_socketList.insert(socketDes,"unKnown");
    emit Someone_arrive(QString("%1").arg(socketDes));

    myThread *new_thread=new myThread(socketDes);

    connect(new_thread,SIGNAL(socket_Leave(int)),this,SLOT(cilentLeaveSLOT(int)));
    connect(new_thread,SIGNAL(finished()),new_thread,SLOT(deleteLater()));
    new_thread->start();
}


void myTcpserver::cilentLeaveSLOT(int socketNum){
    qDebug()<<"client Leave!"<<socketNum;
    emit Someone_leave(QString("%1").arg(socketNum));
}
