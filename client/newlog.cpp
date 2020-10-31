#include "newlog.h"
QMultiMap<int,dataitem> newlog::dataset;//定义
int newlog::distance[15][15];
newlog::newlog()
{
    QFile file(":/a1000.csv");
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "打不开data";return;
    }
    else{
        QTextStream read(&file);
        while(!read.atEnd()){
            QStringList line=read.readLine().split(',');
            dataitem to_insert(CityToInt(line[0]),CityToInt(line[1]),line[2].toInt(),line[3].toInt(),line[4].toInt(),line[5].toInt(),
                               line[6],line[7]);
            dataset.insert(CityToInt(line[0]),to_insert);
        }
    }
}

void newlog::write_to_log(QString &str){
    QFile file("log.txt");
    file.open(QFile::WriteOnly|QIODevice::Append);
    QTextStream in(&file);

    in<<str<<"\r\n";
}

QString IntToCity(int num){
    if(num==0)       return"北京";
    else if(num==1)  return "哈尔滨";
    else if(num==2)  return "济南";
    else if(num==3)  return "南京";
    else if(num==4)  return "西安";
    else if(num==5)  return "广州";
    else if(num==6)  return "成都";
    else if(num==7)  return "武汉";
    else if(num==8)  return "杭州";
    else if(num==9)  return "昆明";
    else if(num==10) return "上海";
    else if(num==11) return "重庆";
    else if(num==12) return "兰州";
    else if(num==13) return "香港";
    else if(num==14) return "天津";
    else             return "不存在";
}

int CityToInt(QString city){
        if(city=="北京") return 0;
        else if(city=="哈尔滨") return 1;
        else if(city=="济南") return 2;
        else if(city=="南京") return 3;
        else if(city=="西安") return 4;
        else if(city=="广州") return 5;
        else if(city=="成都") return 6;
        else if(city=="武汉") return 7;
        else if(city=="杭州") return 8;
        else if(city=="昆明") return 9;
        else if(city=="上海") return 10;
        else if(city=="重庆") return 11;
        else if(city=="兰州") return 12;
        else if(city=="香港") return 13;
        else if(city=="天津") return 14;
        else return -1;
}
