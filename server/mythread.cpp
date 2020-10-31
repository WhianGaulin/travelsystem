#include "mythread.h"

myThread::myThread(int socketDes,QObject *parent):
    QThread (parent),
    m_socketDesc(socketDes){
}

myThread::~myThread(){
    m_socket->close();
}

void myThread::run(){
    //必须要放这里
    m_socket=new myTcpsocket(m_socketDesc);
    connect(m_socket,SIGNAL(disconnected()),this,SLOT(SLOT_socket_Leave()));
    if(!m_socket->setSocketDescriptor(m_socketDesc)){
        qDebug()<<"my thread run()出错了";
        return;
    }
    this->exec();
}

void myThread::SLOT_socket_Leave(){
    emit socket_Leave(m_socketDesc);
    m_socket->disconnectFromHost();
}

