#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include <QTcpServer>
#include <QTcpSocket>
#include "mythread.h"
#include "dataitem.h"

class myTcpserver:public QTcpServer
{
    Q_OBJECT
public:
    myTcpserver();
    void SendData(int socketDes,QString data);
private:
    void incomingConnection(qintptr socketDes);
    QMap<int,QString> m_socketList;
    //
    int read_pointer;
    int read_len;
    int read_socketDes;
    QStringList list;
    //
private slots:
    void cilentLeaveSLOT(int socketDes);
//    void Recv_Data(int socketDes,QString data);//triggered by mythread->transfer_recvData()   仅用于接收
signals:
    void Someone_arrive(QString);
    void Someone_leave(QString);
};

#endif // MYTCPSERVER_H
