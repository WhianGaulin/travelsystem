#ifndef MYTHREAD_H
#define MYTHREAD_H
#include "mytcpsocket.h"
#include <QThread>
#include <QMap>

class myThread:public QThread
{
    Q_OBJECT
public:
    myThread(int socketDes,QObject *parent=nullptr);
    ~myThread();//将m_socket关闭
    myTcpsocket *m_socket;
    int m_socketDesc;
private:
    void run();
private slots:
    void SLOT_socket_Leave();
signals:
    void socket_Leave(int socketNum);
};

#endif // MYTHREAD_H
