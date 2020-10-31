#include <QApplication>
#include <QObject>
#include "listen.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    listen l;
    l.show();
    return a.exec();
}


