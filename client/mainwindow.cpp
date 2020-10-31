#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDialog>
#include <QMessageBox>
#include <QTcpSocket>
#include <QtNetwork>
#include <QLabel>
#include <QObject>
#include <QDateTimeEdit>
#include <QObject>
#include <QDateTime>
#include "timer.h"
#include "newlog.h"
#include "work.h"

#define ip "127.0.0.1"
#define port 8001

#define UPDATE 360
#pragma execution_character_set("utf-8")
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initWidget();
    //计时器改为timer
    initMap();
    init_mySocket();
    connect_to_server();
    timer=new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(receivetimer()));


    ui->ResultLabel->setHidden(true);


    ui->changeBtn->setHidden(true);
    ui->cancleBtn->setHidden(true);

    ui->startTimeEdit->setDate(QDateTime::currentDateTime().date());
    ui->endTimeEdit->setDate(QDateTime::currentDateTime().date());
    ui->startTimeEdit->setTime(QTime(QDateTime::currentDateTime().time().hour(),0,0,0));
    ui->endTimeEdit->setTime(QTime(QDateTime::currentDateTime().time().hour(),0,0,0));


    //以下是内核和UI交互的信号与槽
    connect(ui->TraverlerBox,SIGNAL(currentIndexChanged(int)),this,SLOT(traverler_changed(int)));
    QString str=QString("电脑时间:%1 用户进入程序").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
    mynewlog.write_to_log(str);

    this->show();
//    int count=0;
//    QMultiMap<int,dataitem>::iterator iter=newlog::dataset.begin();
//    while(iter!=newlog::dataset.end()){
//        qDebug()<<count++<<iter.key()<<IntToCity(iter->from)<<IntToCity(iter->to)<<iter->cost<<iter->shiftID<<
//                  iter->vehicle<<iter->depatureTime<<iter->arrivalTime<<iter->spendTime;
//        iter++;
//    }

    win_datetime=new QDateTime(ui->startTimeEdit->dateTime());
    traggle=0;
    ischange=0;
}

void MainWindow::mainwindow_close(){
    this->~MainWindow();
}

//以下是网络通信部分

void MainWindow::init_mySocket(){
    this->mysocket=new QTcpSocket(this);
    //发送错误时执行displayError函数
    connect(mysocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayError(QAbstractSocket::SocketError)));
}

void MainWindow::displayError(QAbstractSocket::SocketError){
    qDebug()<<"mainwindow:display error"<<mysocket->errorString();//输出错误信息
}

void MainWindow::connect_to_server(){
    qDebug()<<"try to connect_to_server...";
    mysocket->abort();
    mysocket->connectToHost(ip,port);
    connect(mysocket,SIGNAL(readyRead()),this,SLOT(read_message()));//接收服务器发来的信息
}

void MainWindow::read_message(){
    QDataStream in(mysocket);
    in.setVersion(QDataStream::Qt_5_12);
    plan_packet myplan;

    in>>myplan.IF_Changed_Plan;
    in>>myplan.myplan>>myplan.TotalCost;
    if(myplan.IF_Changed_Plan){
        in>>myplan.final_arriveTime;
        this->rec_next_plan(myplan.myplan,myplan.TotalCost,myplan.final_arriveTime);

        connect(timer,SIGNAL(timeout()),this,SLOT(map_change_model()));
        traggle=1;
        changePath.clear();
        for(int i=0;i<recv_plan_Map[current_traveler_index].length();i++){            
            planItem* ph=&recv_plan_Map[current_traveler_index][i];
            changePath.push_back(cityPos[ph->item.from].x);
            changePath.push_back(cityPos[ph->item.from].y);
            changePath.push_back(cityPos[ph->item.to].x);
            changePath.push_back(cityPos[ph->item.to].y);
        }
        myplan.IF_Changed_Plan=false;
    }
    else{
        in>>myplan.TotalSpendTime;
        this->rec_best_plan(myplan.myplan,myplan.TotalCost,myplan.TotalSpendTime);
        map_model();
    }

}


//注册，登陆，请求计划，退出
void MainWindow::sendData(){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);

    out<<mydemand.IF_Change<<mydemand.start_City<<mydemand.End_City<<mydemand.mid_City<<mydemand.startTime
      <<mydemand.latestTime<<mydemand.strategy<<mydemand.duration_time_int;
    mysocket->write(block);

}

//以上是网络通信部份
MainWindow::~MainWindow()
{
    QList<int> lst=threadMap.keys();
    for(int i=0;i<lst.length();i++){
        threadMap[lst[i]]->terminate();
    }
    qDebug()<<"MainWindow关闭了";
    QString str=QString("电脑时间:%1 用户退出程序").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
    mynewlog.write_to_log(str);
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent*){
    QList<int> lst=threadMap.keys();
    for(int i=0;i<lst.length();i++){
        threadMap[lst[i]]->terminate();
    }
}

void MainWindow::from_Dialog_signal(QString data){

}

void MainWindow::initialCityName(){
    cityList[0]=tr("北京");cityList[5]=tr("广州");cityList[10]=tr("上海");
    cityList[1]=tr("哈尔滨");cityList[6]=tr("成都");cityList[11]=tr("重庆");
    cityList[2]=tr("济南");cityList[7]=tr("武汉");cityList[12]=tr("兰州");
    cityList[3]=tr("南京");cityList[8]=tr("杭州");cityList[13]=tr("香港");
    cityList[4]=tr("西安");cityList[9]=tr("昆明");cityList[14]=tr("天津");
}

void MainWindow::initWidget(){
    initialCityName();
    ui->endTimeEdit->setHidden(true);
    ui->endTimeLabel->setHidden(true);
}

void MainWindow::updateMap(){
    palette.setBrush(this->backgroundRole(),QBrush(image.scaled(this->size())));
    this->setAutoFillBackground(true);
    this->setPalette(palette);
}

void MainWindow::initMap(){
    QFile fp(":/citypos.txt");
    if(!fp.open(QFile::ReadOnly|QFile::Text)){
        qDebug()<<tr("error!");
        QMessageBox::warning(this,tr("Warning"),tr("file establish or open error!"));
     }
    else{
        QTextStream ts(&fp);
        cityPos.clear();
        while(!ts.atEnd()){
            QStringList ls=ts.readLine().split(' ');
            if(ls.length()>=4){
                Pos pos;
                pos.x=ls[2].toInt();
                pos.y=ls[3].toInt();
                cityPos.insert(ls[0].toInt(),pos);
            }
            else{qDebug()<<ls[0]<<' '<<ls[1]<<' '<<ls[2]<<' '<<ls[3]<<' '<<ls[4];}
        }
    }
    for(int f=0;f<15;f++){
           for(int g=0;g<15;g++){
               int dx=cityPos[f].x-cityPos[g].x;
               int dy=cityPos[f].y-cityPos[g].y;
               newlog::distance[f][g]=(int)sqrt(dx*dx+dy*dy);
           }
       }
    QImage map;
    map.load(":/map_3.png");
    this->resize(map.width(),map.height());
    QImage *p=new QImage(map.width(),map.height(),QImage::Format_RGB32);//此格式需与Work中的格式统一
    image=*p;//初始化一张图;
    QPainter ptr(&image);
    ptr.setCompositionMode(QPainter::CompositionMode_SourceOver);
    ptr.drawImage(0,0,map);
    updateMap();
    /*连接多线程绘图*/
}

void MainWindow::receive_pixmap(QPixmap px,QThread* th){
    int i;
    QList<int> lst=threadMap.keys();
    for(i=0;i<lst.length();i++){
        if(threadMap[lst[i]]->isFinished()&&pixmap.contains(lst[i])){
            pixmap[lst[i]].fill(Qt::transparent);
        }
    }

    for(i=0;i<lst.length();i++){
      if(th==threadMap[lst[i]]) {
          if(pixmap.contains(lst[i]))
                pixmap[lst[i]]=px;
          else pixmap.insert(lst[i],px);
          break;
      }
    }

    if(i==lst.length()){
        qDebug()<<"######################                 something haven't considered            ############";
        pixmap.insert(current_traveler_index,px);
    }

    image.load(":/map_3.png");
    QPainter pt(&image);
    pt.setCompositionMode(QPainter::CompositionMode_SourceOver);//两个Image叠加效果,如何擦除原来轨迹？
    for(int j=0;j<pixmap.keys().length();j++) pt.drawPixmap(0,0,pixmap[pixmap.keys().at(j)]);
    if(ischange==0&&traggle==0) changePath.clear();
    if(threadMap.contains(current_traveler_index)&&threadMap[current_traveler_index]->isRunning()&&(ischange||traggle)){
        MapPainter mappt(image.width(),image.height());
        if(mappt.drawPath(changePath,Qt::gray)!=0){
            pt.drawPixmap(0,0,*mappt.drawPath(changePath,Qt::red));
        }
    }  
    this->updateMap();
}

void MainWindow::to_creat_work(){
    pwork=new Work();                                    //新建画图工作室
    connect(this,&MainWindow::sendtimer,pwork,&Work::UpDate);
    connect(this,SIGNAL(sendPath(QString,QString,QString)),pwork,SLOT(drawPath(QString,QString,/*QVector<QDateTime>*/QString)));
    connect(pwork,SIGNAL(finished(QPixmap,QThread*)),this,SLOT(receive_pixmap(QPixmap,QThread*)));
    connect(pwork,&Work::modfiyContinue,this,&MainWindow::sendMapContinue);
    QThread *thread=new QThread(this);
    if(threadMap.contains(current_traveler_index)&&threadMap[current_traveler_index]->isRunning()){
           threadMap[current_traveler_index]->terminate();
    }
    threadMap.insert(current_traveler_index,thread);
    connect(pwork,&Work::drawfinished,thread,&QThread::quit);          //在画完之后结束此进程,与QThread::destroyed()区别？//将工作转入多线程
    pwork->moveToThread(thread);
    thread->start();
}

void MainWindow::on_addCityBtn_clicked()
{
    QString midCityStr=tr("");
    if(ui->startCityBox->currentText()==ui->endCityBox->currentText())
        QMessageBox::warning(this,tr("温馨提示"),tr("出发城市和到达城市重复！"));
    else if(ui->midCityBox->currentText()==ui->startCityBox->currentText()||
            ui->midCityBox->currentText()==ui->endCityBox->currentText())
        QMessageBox::warning(this,tr("温馨提示"),tr("此城市已在出发城市或到达城市！"));
    else{//往cityVec插入。。。也往duration_time_int插入
        if(!cityVec.empty()){
            bool repeat=false;
            for(int i=0;i<cityVec.length();i++)//该循环为检查是否有重复
                if(cityVec[i]==ui->midCityBox->currentText()){
                    QMessageBox::warning(this,tr("温馨提示"),tr("此城市已存在!"));
                    repeat=true;
                    break;
                }
            if(!repeat){
                cityVec.push_back(ui->midCityBox->currentText());//往队尾插入
                duration_time_int.push_back(ui->durationhour->text().toInt());
            }
        }
        else{
            cityVec.push_back(ui->midCityBox->currentText());//往队尾插入
            duration_time_int.push_back(ui->durationhour->text().toInt());
        }
        for(int i=0;i<cityVec.length();i++){//这里相当于每次add完后都重新定义midCityStr,也许将来可以修改
            midCityStr+=tr("%1.").arg(i+1)+cityVec[i]+tr("   停留时间:%1小时").arg(duration_time_int[i])+tr("\n");
        }
        ui->midCityTextBrowser->setText(midCityStr);
    }
}

void MainWindow::on_dltCityBtn_clicked()
{
    QString midCityStr=tr("");
    if(!cityVec.empty()){
        cityVec.pop_back();
        duration_time_int.pop_back();
        //下面重新定义（应改变的）midCityStr
        for(int i=0;i<cityVec.length();i++){
            midCityStr+=tr("%1.").arg(i+1)+cityVec[i]+tr("   停留时间:%1小时").arg(duration_time_int[i])+tr("\n");
        }
        ui->midCityTextBrowser->setText(midCityStr);
    }
}

void MainWindow::on_stgyBox_currentTextChanged(const QString &arg1)
{
    QString str=tr("限时费用最少");

    if(arg1==str){
        ui->endTimeLabel->setHidden(false);
        ui->endTimeEdit->setHidden(false);
    }
    else{
        ui->endTimeLabel->setHidden(true);
        ui->endTimeEdit->setHidden(true);
    }
}

void MainWindow::map_model(){
    planItem* ph;
    QString s0,sdt,trs;
    sdt=ui->startTimeEdit->dateTime().toString("yyyy-MM-dd-hh:mm:ss");
    int jk=recv_plan_Map[current_traveler_index].length();
    qDebug()<<tr("has point num: %1").arg(jk);
    int x0,y0;
    trs="";
    for(int i=0;i<jk;i++){
        ph=&recv_plan_Map[current_traveler_index][i];
        qDebug()<<ph->item.vehicle;
        if(ph->item.vehicle==tr("飞机")) trs+="1";
        else if(ph->item.vehicle==tr("火车")) trs+="2";
        else trs+="3";
        sdt+=ph->departureTime.toString(" yyyy-MM-dd-hh:mm:ss");
        sdt+=ph->arriveTime.toString(" yyyy-MM-dd-hh:mm:ss");
       qDebug()<<ph->item.from<<ph->item.to<<ph->departureTime<<ph->arriveTime;
       x0=cityPos[ph->item.to].x;
       y0=cityPos[ph->item.to].y;
       if(i==0){
           s0=tr("%1 %2").arg(cityPos[ph->item.from].x).arg(cityPos[ph->item.from].y);
           s0+=tr(" %1 %2").arg(x0).arg(y0);
       }
       else s0+=tr(" %1 %2").arg(x0).arg(y0);
    }
    to_creat_work();
    emit sendPath(s0,sdt,trs);
}

void MainWindow::map_model2(){
    threadMap[current_traveler_index]->terminate();
    qDebug()<<"#####     threadMap[current_traveler_index]->terminate()      #########";
    planItem* ph;
    QString s0,sdt,trs;
    int jk=recv_plan_Map[current_traveler_index].length();
    int x0,y0;
    trs="";
    bool bl=1;int hj=0;
    for(int i=0;i<jk;i++){
        ph=&recv_plan_Map[current_traveler_index][i];
        if(ph->item.from==nextCity_for_newPlan) {bl=0;hj=i;sdt=win_datetime->toString("yyyy-MM-dd-hh:mm:ss");}
        if(bl) continue;
        if(ph->item.vehicle==tr("飞机")) trs+="1";
        else if(ph->item.vehicle==tr("火车")) trs+="2";
        else if(ph->item.vehicle==tr("汽车")) trs+="3";
        else qDebug()<<"########          ph->item.vehicle can't match          #########";
        sdt+=ph->departureTime.toString(" yyyy-MM-dd-hh:mm:ss");
        sdt+=ph->arriveTime.toString(" yyyy-MM-dd-hh:mm:ss");
       x0=cityPos[ph->item.to].x;
       y0=cityPos[ph->item.to].y;
       if(i==hj){
           s0=tr("%1 %2").arg(cityPos[ph->item.from].x).arg(cityPos[ph->item.from].y);
           s0+=tr(" %1 %2").arg(x0).arg(y0);
       }
       else s0+=tr(" %1 %2").arg(x0).arg(y0);
    }
    to_creat_work();
    emit sendPath(s0,sdt,trs);
    qDebug()<<"#####     new work start     ##########";
}

void MainWindow::map_change_model(){
    QDateTime arrTime=traveler_state_Map[current_traveler_index].shift_arrive_DateTime;
    int is_on_shift=traveler_state_Map[current_traveler_index].IF_on_the_shift;
    if(!is_on_shift&&traggle){
        map_model2();
        traggle=0;
        ischange=1;
    }
    else if(arrTime==*win_datetime&&traggle){
        map_model2();
        traggle=0;
        ischange=1;
    }
}

void MainWindow::on_confBtn_clicked()//在这里也要发出信号，把Ui上的值传给属于traveler类的myTraveler
{
    if(ui->TraverlerBox->currentIndex()>=0) ;
    else{
        QMessageBox::information(this,"tips","请添加旅客!");
        return;
    }

    if(ui->startCityBox->currentText()==ui->endCityBox->currentText()){
        QMessageBox::information(this,"tips","出发城市与目的城市相同!");
        return;
    }

    QVector<planItem> recv_plan;
    recv_plan_Map.insert(current_traveler_index,recv_plan);

    traveler_state a_traveler_state;
    traveler_state_Map.insert(current_traveler_index,a_traveler_state);
    traveler_state_Map[current_traveler_index].location=ui->startCityBox->currentText();

    simulation_time_Map.insert(current_traveler_index,ui->startTimeEdit->dateTime());
    qDebug()<<"current_traveler_index:"<<current_traveler_index;

    my_travelers[current_traveler_index]->my_index=current_traveler_index;
    my_travelers[current_traveler_index]->Earlist_StartTime=ui->startTimeEdit->dateTime();//出发时间  比较特殊
    my_travelers[current_traveler_index]->Show_Button_Method=1;
    //发送ui的信息
    my_travelers[current_traveler_index]->IF_Conf=1;

    timer->stop();//计算方案是要停止时间
    log_content.clear();
    ui->planTextBrowser->clear();
    ui->textBrowser->clear();

    show_Button_method(1);

    ui->startCityBox->setEnabled(false);
    ui->startTimeEdit->setEnabled(false);

    write_operation_to_log();

    mydemand.IF_Change=0;
    mydemand.start_City=ui->startCityBox->currentText();
    mydemand.End_City=ui->endCityBox->currentText();
    mydemand.mid_City=cityVec;
    mydemand.startTime=ui->startTimeEdit->dateTime();
    mydemand.latestTime=ui->endTimeEdit->dateTime();
    mydemand.strategy=ui->stgyBox->currentText();
    mydemand.duration_time_int=duration_time_int;

    sendData();
    qDebug()<<"sent Data yet";

}

QDateTime MainWindow::simulation_time_TO_startTime(){
    int gap_minutes=0;
        if(simulation_time_Map[current_traveler_index].time().minute()>0)
            gap_minutes=60-simulation_time_Map[current_traveler_index].time().minute();
    QDateTime  next_startTime=simulation_time_Map[current_traveler_index].addSecs(gap_minutes*60);
    return next_startTime;
}

void MainWindow::on_changeBtn_clicked()
{
    qDebug()<<"on_changeBtn_clicked...";
    plan_Map[current_traveler_index].clear();
    ui->planTextBrowser->clear();
    my_travelers[current_traveler_index]->Show_Button_Method=1;
    my_travelers[current_traveler_index]->IF_Changed=1;
    QString str=QString("电脑时间:%1 用户途径改变当前计划").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
    mynewlog.write_to_log(str);
    timer->stop();//无论如何  时间停止，等待计算最佳方案
    mydemand.IF_Change=1;
    if(my_travelers[current_traveler_index]->dfs_succ==0){
        mydemand.start_City=ui->startCityBox->currentText();
        mydemand.End_City=ui->endCityBox->currentText();
        mydemand.strategy=ui->stgyBox->currentText();
        mydemand.startTime=ui->startTimeEdit->dateTime();
        mydemand.mid_City=cityVec;
        mydemand.duration_time_int=duration_time_int;
        mydemand.latestTime=ui->endTimeEdit->dateTime();

    }
    else{//之前是存在计划的
    if(traveler_state_Map[current_traveler_index].IF_on_the_shift){//在路上
        mydemand.start_City=IntToCity(traveler_state_Map[current_traveler_index].shift.to);
        mydemand.End_City=ui->endCityBox->currentText();
        mydemand.strategy=ui->stgyBox->currentText();
        mydemand.startTime=traveler_state_Map[current_traveler_index].shift_arrive_DateTime;
        mydemand.mid_City=cityVec;
        mydemand.duration_time_int=duration_time_int;
        mydemand.latestTime=ui->endTimeEdit->dateTime();
    }
    else {//停留在某个城市
        mydemand.start_City=traveler_state_Map[current_traveler_index].location;
        mydemand.End_City=ui->endCityBox->currentText();
        mydemand.strategy=ui->stgyBox->currentText();
        mydemand.startTime=simulation_time_TO_startTime();
        mydemand.mid_City=cityVec;
        mydemand.duration_time_int=duration_time_int;
        mydemand.latestTime=ui->endTimeEdit->dateTime();
    }
    }
    sendData();

}



void MainWindow::on_cancleBtn_clicked()
{
    //initMap();
    ischange=0;
    traggle=0;
    QString str=QString("旅客%1 电脑时间:%2 用户取消当前计划").arg(current_traveler_index+1).arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
    mynewlog.write_to_log(str);
    //threadArray[current_traveler_index]->terminate();
    if(threadMap.keys().contains(current_traveler_index)){
        threadMap[current_traveler_index]->terminate();
        if(pixmap.keys().contains(current_traveler_index)){
            pixmap[current_traveler_index].fill(Qt::transparent);
            initMap();
        }
        else qDebug()<<"##########           there is something      #####";
    }

    my_travelers[current_traveler_index]->Show_Button_Method=0;
    show_Button_method(0);

    ui->startCityBox->setEnabled(true);
    ui->startTimeEdit->setEnabled(true);
    ui->midCityTextBrowser->clear();
    ui->planTextBrowser->clear();
    ui->textBrowser->clear();
    plan_Map[current_traveler_index].clear();
    ui->ResultLabel->clear();
    cityVec.clear();
    duration_time_int.clear();
    recv_plan_Map[current_traveler_index].clear();//当前页面
    my_travelers[current_traveler_index]->IF_Conf=0;
    my_travelers[current_traveler_index]->IF_Changed=0;
}

void MainWindow::write_operation_to_log(){
    qDebug()<<"进入 write_operatin_to_log";
    QString str=QString("电脑时间:%1  旅客%2 进行查询操作,出发城市:%3 目的城市:%4 要求:%5").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"))
                                                                       .arg(current_traveler_index)
                                                                       .arg(ui->startCityBox->currentText())
                                                                       .arg(ui->endCityBox->currentText())
                                                                       .arg(ui->stgyBox->currentText());
    str+=QString(" 出发时间:%1").arg(my_travelers[current_traveler_index]->startTime.toString("yyyy.MM.dd hh:mm:ss"));
    if(ui->stgyBox->currentText()==tr("限时费用最少")){
        str+=QString(" 要求最晚到达时间:%1").arg(my_travelers[current_traveler_index]->startTime.toString("yyyy.MM.dd hh:mm:ss"));
    }
    int size=cityVec.size();
    if(size>0){//有途径城市
        int i=0;
        str+=QString(" 途经城市:");
        while (i<size) {
            str+=QString(" %1 最少停留时间:%2小时").arg(cityVec[i]).arg(duration_time_int[i]);
            i++;
        }
    }
    qDebug()<<"write_operation_to_log:"<<str;
    mynewlog.write_to_log(str);
}

int MainWindow::cost_add_up(int i){
    int cost=0;
    for (int j=0;j<=i;j++) {
        cost+=recv_plan_Map[current_handle_traveler_index][j].item.cost;//轮询
    }
    my_travelers[current_handle_traveler_index]->cost_Up_To_Now=cost;//轮询的index
    return cost;
}

int MainWindow::spendSecs(){
    int secs=my_travelers[current_handle_traveler_index]->Earlist_StartTime.secsTo(simulation_time_Map[current_handle_traveler_index]);
    return  secs;
}

void MainWindow::make_basic_item_content(int traveler_index){
    //qDebug()<<"start make_basic_item_content";
    item_content.clear();
    item_content="";
    item_content+=QString("旅客:%1 ").arg(traveler_index+1);
    item_content+=QString("当前时间:%1").arg(QDateTime_to_QString(simulation_time_Map[traveler_index]));
    item_content+=QString("  旅客已花费时间:%1").arg(int_to_interval(traveler_state_Map[traveler_index].my_spend_Secs));
    item_content+=QString("  旅客已花费路费:%1").arg(traveler_state_Map[traveler_index].my_cost);
    item_content+=QString("  所在位置:%1").arg(traveler_state_Map[traveler_index].location);
    if(traveler_state_Map[traveler_index].IF_on_the_shift){//在某个班次上(路途中)
        item_content+=QString(" 交通工具:%1  %2").arg(traveler_state_Map[traveler_index].shift.vehicle)
                                                .arg(traveler_state_Map[traveler_index].shift.shiftID);
    }
    else {
        item_content+=QString("  停留在该城市");
    }
    item_content+="\n";//换行
    //qDebug()<<"end make_basic_item_content";
}

void MainWindow::update_traveler_status_table(){//受update_traveler_state()函数驱动
    //qDebug()<<"update_traveler_statues_table";
    mynewlog.write_to_log(item_content);
    log_content.push_front(item_content);
    ui->textBrowser->setText(log_content);
    //qDebug()<<"end update_traveler_statues_table";
}

void MainWindow::update_traveler_state(int traveler_index){//受receivetimer()函数驱动,根据时间更新traveler_state,轮询index
    qDebug()<<"update_traveler_state.."<<simulation_time_Map[traveler_index];
    if(my_travelers[traveler_index]->Earlist_StartTime<=simulation_time_Map[traveler_index]&&
            simulation_time_Map[traveler_index]<=recv_plan_Map[traveler_index][0].departureTime){//特殊情况下的判断,还没出发
        traveler_state_Map[traveler_index].location=QString("%1")
                .arg(IntToCity(recv_plan_Map[traveler_index][0].item.from));
        traveler_state_Map[traveler_index].my_cost=0;
        traveler_state_Map[traveler_index].my_spend_Secs=spendSecs();
        traveler_state_Map[traveler_index].IF_on_the_shift=0;
        traveler_state_Map[traveler_index].IF_start=0;//还没开始，特殊标记一下
        make_basic_item_content(traveler_index);
        latest_state_Map[traveler_index]=item_content;//存储当前最新的状态
        update_traveler_status_table();
        return;
    }
    else traveler_state_Map[traveler_index].IF_start=1;//已经出发了，要把该标记改为1
    int plan_size=recv_plan_Map[traveler_index].size();
    if(simulation_time_Map[traveler_index]>recv_plan_Map[traveler_index][plan_size-1].arriveTime){//按时间判断已经到达了目的地
        //应该增加一些函数，因为既然已经模拟完了
        item_content=QString("到达目的地:%1!  ").arg(IntToCity(recv_plan_Map[traveler_index][plan_size-1].item.to));
        latest_state_Map[traveler_index]=item_content;//存储当前最新的状态
        update_traveler_status_table();
        my_travelers[traveler_index]->IF_Conf=0;
        my_travelers[traveler_index]->IF_Changed=0;
        return;
    }
    int i=0;
    for(;i<plan_size;i++){
        if(recv_plan_Map[current_handle_traveler_index][i].departureTime<=simulation_time_Map[traveler_index]
                &&simulation_time_Map[traveler_index]<=recv_plan_Map[current_handle_traveler_index][i].arriveTime){//在航班之中
            traveler_state_Map[traveler_index].location=QString("在%1到%2之间")
                    .arg(IntToCity(recv_plan_Map[current_handle_traveler_index][i].item.from))
                    .arg(IntToCity(recv_plan_Map[current_handle_traveler_index][i].item.to));
            traveler_state_Map[traveler_index].my_cost=cost_add_up(i);
            traveler_state_Map[traveler_index].my_spend_Secs=spendSecs();
            traveler_state_Map[traveler_index].IF_on_the_shift=1;
            traveler_state_Map[traveler_index].shift=recv_plan_Map[current_handle_traveler_index][i].item;//所在的航班
            traveler_state_Map[traveler_index].shift_arrive_DateTime=recv_plan_Map[current_handle_traveler_index][i].arriveTime;
            break;
        }
        else if(recv_plan_Map[traveler_index][i].arriveTime<=simulation_time_Map[traveler_index]&&
                simulation_time_Map[traveler_index]<=recv_plan_Map[traveler_index][i+1].departureTime){//在两个航班的间隔之间，即停留在某个城市
            //到达了某个城市，应该在这里向 画图模块 传递信息
            traveler_state_Map[traveler_index].location=QString("%1").arg(IntToCity(recv_plan_Map[traveler_index][i].item.to));
            traveler_state_Map[traveler_index].my_spend_Secs=spendSecs();
            traveler_state_Map[traveler_index].IF_on_the_shift=0;
            traveler_state_Map[traveler_index].shift=recv_plan_Map[traveler_index][i].item;//来到这个城市的航班
            break;
        }
    }
    make_basic_item_content(traveler_index);
    latest_state_Map[traveler_index]=item_content;
    update_traveler_status_table();
}

//槽函数
void MainWindow::receivetimer(){//更新函数，一切mainwindow的时间更新都由此开始
    qDebug()<<"receivetimer...";
    *win_datetime=win_datetime->addSecs(UPDATE);
    for(int i=0;i<traverler_num;i++){//
        //qDebug()<<"i:"<<i<<"traveler_num:"<<traverler_num;
        if(my_travelers[i]->IF_Conf||my_travelers[i]->IF_Changed){
            qDebug()<<"i:"<<i;
            current_handle_traveler_index=i;
            simulation_time_Map[i]=simulation_time_Map[i].addSecs(UPDATE);
            update_traveler_state(i);
        }
    }
}

void MainWindow::Show_NoPlan(){
    qDebug()<<"No Plan!";
    show_Button_method(0);
    ui->startCityBox->setEnabled(true);
    ui->startTimeEdit->setEnabled(true);
    QString s="不存在!";
    ui->planTextBrowser->setText(s);
}

void MainWindow::rec_best_plan(QVector<planItem> myplan,int TotalCost,QString TotalSpendTime){//此处肯定传送plan是当前页(current_index)
    qDebug()<<"recv best plan!"<<current_traveler_index<<myplan.size();
    if(myplan.size()==0){
        Show_NoPlan();
        return;
    }
    QString s="得到方案:";mynewlog.write_to_log(s);
    rec_total_cost_AND_spendTime(TotalCost,TotalSpendTime);
    recv_plan_Map[current_traveler_index]=myplan;
    QVector<planItem>::iterator iter=recv_plan_Map[current_traveler_index].begin();
    while (iter!=recv_plan_Map[current_traveler_index].end()) {
        shift_content=QString("★班次号:%1 出发城市:%2 到达城市:%3 出发时间:%4 到达时间:%5 交通工具:%6 花费:%7元★")
                .arg(iter->item.shiftID).arg(IntToCity(iter->item.from)).arg(IntToCity(iter->item.to))
                .arg(iter->departureTime.toString("yyyy.MM.dd hh:mm:ss")).arg(iter->arriveTime.toString("yyyy.MM.dd hh:mm:ss"))
                .arg(iter->item.vehicle).arg(iter->item.cost);
        qDebug()<<shift_content;
        plan_Map[current_traveler_index]+=shift_content+"\n";
        mynewlog.write_to_log(shift_content);
        iter++;
    }
    //之前的时钟被 停止了，现在开始
    timer->start(1000);
    ui->planTextBrowser->setText(plan_Map[current_traveler_index]);
}

void MainWindow::remove_noNeed_plan(){
    if(!traveler_state_Map[current_traveler_index].IF_start){//如果旅行甚至还没有开始，那么清空旅行计划即可
        recv_plan_Map[current_traveler_index].clear();
        qDebug()<<"clear recv_plan!";
    }
    else {//如果旅行已经开始了，1.在途中，记乘坐的班次号为A，那么要保留A之前，包括A的所有班次计划  2.如果正在一个城市停留，那么保留来到这个城市之前的所有班次计划
        QVector<planItem>::iterator iter=recv_plan_Map[current_traveler_index].begin();
        int IF_back=0;
        while (iter!=recv_plan_Map[current_traveler_index].end()) {
            if(iter->item.shiftID!=traveler_state_Map[current_traveler_index].shift.shiftID){
                if(IF_back) recv_plan_Map[current_traveler_index].erase(iter);
                else   iter++;
            }
            else {
                IF_back=1;
                iter++;
            }
        }
        return;
    }
}

void MainWindow::rec_next_plan(QVector<planItem> next_plan,int Nextcost,QDateTime final_arrive_time){
    qDebug()<<"rec_next_plan";
    if(next_plan.size()==0){
        Show_NoPlan();
        return;
    }
    remove_noNeed_plan();
    nextCity_for_newPlan=traveler_state_Map[current_traveler_index].shift.to;
    QVector<planItem>::iterator iter=next_plan.begin();
    while (iter!=next_plan.end()) {
        recv_plan_Map[current_traveler_index].push_back(*iter);
        iter++;
    }
    int predicted_TotalCost=my_travelers[current_traveler_index]->cost_Up_To_Now+Nextcost;
    QString predicted_SpendTime=int_to_interval(sub(my_travelers[current_traveler_index]->Earlist_StartTime,final_arrive_time));
    rec_total_cost_AND_spendTime(predicted_TotalCost,predicted_SpendTime);
    //之前时钟被停止了
    timer->start(1000);
    iter=recv_plan_Map[current_traveler_index].begin();
    while (iter!=recv_plan_Map[current_traveler_index].end()) {
        shift_content=QString("★班次号:%1 出发城市:%2 到达城市:%3 出发时间:%4 到达时间:%5 交通工具:%6 花费:%7元★")
                .arg(iter->item.shiftID).arg(IntToCity(iter->item.from)).arg(IntToCity(iter->item.to))
                .arg(iter->departureTime.toString("yyyy.MM.dd hh:mm:ss")).arg(iter->arriveTime.toString("yyyy.MM.dd hh:mm:ss"))
                .arg(iter->item.vehicle).arg(iter->item.cost);
        plan_Map[current_traveler_index]+=shift_content+"\n";
        mynewlog.write_to_log(shift_content);
        iter++;
    }
    ui->planTextBrowser->setText(plan_Map[current_traveler_index]);
}

void MainWindow::rec_total_cost_AND_spendTime(int cost,QString SpendTimeStr){
    QString s=QString("总花费:%1元,总花费时间:%2").arg(cost).arg(SpendTimeStr);
    mynewlog.write_to_log(s);
    ui->ResultLabel->setText(s);
    ui->ResultLabel->setHidden(false);
}

void MainWindow::on_stopBtn_clicked()
{
    timer->stop();
    emit sendMapContinue(0,threadMap[current_traveler_index]);
    Work::isContinue=0;
}

void MainWindow::on_ContinueBtn_clicked()
{
    emit sendMapContinue(1,threadMap[current_traveler_index]);
    Work::isContinue=1;
    if(timer->isActive())
        qDebug()<<"timer already Active!";
    else {
        timer->start(1000);
    }
}

void MainWindow::InitConnection_For_newTraveler(int i){
    if(my_travelers[i]->IF_Connect) return;
    my_travelers[i]->IF_Connect=1;
    connect(my_travelers[i],SIGNAL(convey_plan(QVector<planItem>,int,QString)),this,SLOT(rec_best_plan(QVector<planItem>,int,QString)));
    connect(my_travelers[i],SIGNAL(convey_next_plan(QVector<planItem>,int,QDateTime)),this,SLOT(rec_next_plan(QVector<planItem>,int,QDateTime)));
}

void MainWindow::on_addTravelerBtn_clicked()//需要处理在其他旅客过程中的情况
{
    qDebug()<<"on addTravelerBtn clicked...";

    QString plan_content;
    plan_Map.insert(traverler_num,plan_content);
    QString item_content;
    latest_state_Map.insert(traverler_num,item_content);
    ui->planTextBrowser->setText(plan_Map.value(traverler_num));

    traverler_num++;//初始化为0,有新旅客就+1

    traveler* new_traveler;
    my_travelers.insert(traverler_num-1,new_traveler);
    my_travelers[traverler_num-1]=new traveler();


    ui->TraverlerBox->addItem(QString("旅客%1").arg(traverler_num));
    ui->TraverlerBox->setCurrentText(QString("旅客%1").arg(traverler_num));
}

void MainWindow::traverler_changed(int index){//index从0开始   往这里要设置diferrent用户的交互界面呢
    qDebug()<<"traverler_changed...";
    current_traveler_index=index;
    qDebug()<<current_traveler_index;

    show_Button_method(my_travelers[index]->Show_Button_Method);
    ui->planTextBrowser->setText(plan_Map[index]);

    if(my_travelers[current_traveler_index]->IF_Conf==0){
        ui->startCityBox->setEnabled(true);
        ui->startTimeEdit->setEnabled(true);
    }
    else {
        ui->startCityBox->setEnabled(false);
        ui->startTimeEdit->setEnabled(false);
    }
}

void MainWindow::show_Button_method(int method){
    if(method==1){
        ui->confBtn->setHidden(true);
        ui->changeBtn->setHidden(false);
        ui->cancleBtn->setHidden(false);
    }
    else {
        ui->confBtn->setHidden(false);
        ui->changeBtn->setHidden(true);
        ui->cancleBtn->setHidden(true);
    }
}

void MainWindow::on_searchBtn_clicked()
{
    QMessageBox::information(this,"当前状态",latest_state_Map[current_traveler_index]);
}


