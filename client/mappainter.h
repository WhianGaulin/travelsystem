#ifndef MAPPAINTER_H
#define MAPPAINTER_H
#include <QObject>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QColor>
#include <QDebug>

#define CITY_NUM 15

class MapPainter:public QObject {
    Q_OBJECT
public:
    MapPainter();
    MapPainter(int width,int height);
    ~MapPainter();
    void initial();
    QPixmap* drawPath(QVector<int> path,QColor color);
    QPixmap* drawPoint(int x,int y,QString color);
public slots:

signals:

private:
   int width=0;
   int height=0;
   QPixmap *pixmap=NULL;
};

#endif // MAPPAINTER_H

