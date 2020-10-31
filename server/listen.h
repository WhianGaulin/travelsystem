#ifndef LISTEN_H
#define LISTEN_H

#include <QMainWindow>
#include "mytcpserver.h"
#include "newlog.h"

namespace Ui {
class listen;
}

class listen : public QMainWindow
{
    Q_OBJECT

public:
    explicit listen(QWidget *parent = nullptr);
    ~listen();
    myTcpserver *m_Tcpserver;

public slots:
    void insert_ListItem(QString);
    void remove_ListItem(QString);

private slots:
    void on_listenBtn_clicked();

private:
    Ui::listen *ui;
};

#endif // LISTEN_H
