#include "mappainter.h"


MapPainter::MapPainter(){

}

MapPainter::MapPainter(int w,int h){
    width=w;
    height=h;
    initial();
}

MapPainter::~MapPainter(){

}

void MapPainter::initial()
{
    pixmap=new QPixmap(width,height);
    pixmap->fill(Qt::transparent);
}

QPixmap* MapPainter::drawPath(QVector<int> path,QColor clr)
{
    initial();
    if((width==0||height==0)&&path.length()%4!=0) qDebug()<<"############             no initial!        #############";
    else{
        if(pixmap==NULL) {
            qDebug()<<"############             pixmap is none!        #############";
            return pixmap;
        }
        QPainter pt(pixmap);
        pt.setPen(clr);
        qDebug()<<"###########       path.length:    "<<path.length();
        for(int i=0;i<path.length()/4;i++){
            qDebug()<<"#####     drawpath  "<<path[4*i]<<" "<<path[4*i+1]<<" "<<path[4*i+2]<<" "<<path[4*i+3];
            pt.drawLine(path[4*i],path[4*i+1],path[4*i+2],path[4*i+3]);
        }
    }
    return pixmap;
}

QPixmap* MapPainter::drawPoint(int x, int y, QString color)
{
    initial();
    QPixmap *pix=new QPixmap();
    if(color=="blue") pix->load(":/blue.png");
    else if(color=="purple") pix->load(":/perple.png");
    else if(color=="yellow") pix->load((":/yellow.png"));
    else{
        qDebug()<<"############           have no such color picture           #############";
        return pixmap;
    }
    QPainter pt(pixmap);
    pt.drawPixmap(x-pix->width()/2,y-pix->height(),*pix);
    return pixmap;
}

