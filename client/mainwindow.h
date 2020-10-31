#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QPixmap>
#include <QImage>
#include <QPalette>
#include <QMap>
#include <QVector>
#include <QThread>
#include <QBrush>
#include <QPen>
#include <QTextStream>
#include <QMessageBox>
#include <QTcpSocket>
#include <QtNetwork>
#include "dataitem.h"
#include "traveler.h"
#include "newlog.h"
#include "work.h"
#include "mappainter.h"


namespace Ui {
class MainWindow;
}
class Pos{
public :
    int x;
    int y;
};

/*
void traveler::set_Value(QString start_City, QString End_City, QVector<QString> mid_City,
                         QDateTime startTime, QDateTime latest_Time, QString strategy,
                         QVector<int> duration_time_int)

*/
struct tag_demand{
    int IF_Change;
    QString start_City;
    QString End_City;
    QVector<QString> mid_City;
    QDateTime startTime;
    QDateTime latestTime;
    QString strategy;
    QVector<int> duration_time_int;
};
typedef struct tag_demand demand;


struct tag_plan{
    int IF_Changed_Plan;
    QVector<planItem> myplan;
    int TotalCost;
    QString TotalSpendTime;
    QDateTime final_arriveTime;
};
typedef struct tag_plan plan_packet;

//class traveler_state
//{
//public:
//    int my_cost=0;
//    int my_spend_Secs=0;
//    int IF_on_the_shift=0;
//    int IF_start=1;
//    dataitem shift;
//    QString location;//用文字表述
//    QDateTime shift_arrive_DateTime;
//};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void initialCityName();
    void initWidget();
    void initMap();
    void to_creat_work();
    void updateMap();
    void InitConnection_For_newTraveler(int i);

    //网络通信部分
    QTcpSocket *mysocket;
    int mysocket_num;
    void init_mySocket();
    void connect_to_server();
    void sendData();
    demand mydemand;
    //以上是网络通信部分

    QMap<int,Pos> cityPos;//
    QVector<int> list;//存放路径，将城市标记为int
   // QVector<QThread*> threadArray;//区分多线程
    QMap<int,QPixmap> pixmap;
   // QVector<QPixmap> Pix;
    QMap<int,QThread*> threadMap;
    bool traggle;
    bool ischange;

    QVector<QString> cityVec;//存储途经城市的容器
    QVector<int>   duration_time_int;//存储逗留时间的容器，顺序按照cityVec
    newlog mynewlog;//将来自动初始化
    QString cityList[15];//用于initialCityName()

    //以下是模拟计时部分//
    QString log_content;//日志表
    QString item_content;//一项的内容

    QMap<int,QString> plan_Map;//用来显示的
    QMap<int,QString> latest_state_Map;
    QMap<int,plan> recv_plan_Map;//current_traveler_index
    QMap<int,traveler_state> traveler_state_Map;
    QMap<int,QDateTime> simulation_time_Map;

    QString shift_content;
    QDateTime simulation_time_TO_startTime();
    void update_traveler_state(int traveler_index);
    int cost_add_up(int i);
    int spendSecs();
    void update_traveler_status_table();
    void make_basic_item_content();
    void make_basic_item_content(int index);
    void write_operation_to_log();
    void write_ChangePlanOpreation_to_log();
    void remove_noNeed_plan();

//以下是地图部分
    void map_model();
    void map_model2();
    int nextCity_for_newPlan;

//以下是多用户部分
    int traverler_num=0;//旅客名义上的数量
    //int traverler_Conf_Ceiling=0;//旅客实际的数量
    QMap<int,traveler*> my_travelers;    //在addTravelerBtn()里面新增
    int current_traveler_index;          //代表的是当前页面下的旅客号
    int current_handle_traveler_index;   //代表的是当前正在处理的旅客号(轮询，经常改变)
    void show_Button_method(int method);


signals:
    void send_to_midCity_path();
    //以下是和traveler模块交互
    void convey_start_city(QString);
    void convey_end_city(QString);
    void convey_start_time(QDateTime);
    void convey_latest_time(QDateTime);
    void convey_strategy(QString);
    void convey_mid_city(QVector<QString>);
    void convey_StayTime(QVector<int>);
    void sendPath(QString,QString,QString);
    void sendtimer();
    void sendMapContinue(bool,QThread*);

private slots:
    void displayError(QAbstractSocket::SocketError);
    void read_message();


    void from_Dialog_signal(QString data);
    void on_addCityBtn_clicked();
    void on_dltCityBtn_clicked();
    void on_stgyBox_currentTextChanged(const QString &arg1);
    void on_confBtn_clicked();
    //以下是和traveler模块交互
    void rec_best_plan(QVector<planItem>,int,QString);
    void rec_next_plan(QVector<planItem>,int,QDateTime);
    void Show_NoPlan();
    void rec_total_cost_AND_spendTime(int,QString);
//    void update_simulation_time();
    void on_stopBtn_clicked();
    void on_ContinueBtn_clicked();
    void on_changeBtn_clicked();
    void on_cancleBtn_clicked();

    void receive_pixmap(QPixmap,QThread*);
    void receivetimer();
    void on_addTravelerBtn_clicked();
    void traverler_changed(int);

    void map_change_model();
    void closeEvent(QCloseEvent*);

    void on_searchBtn_clicked();

    void mainwindow_close();


private:
    QVector<int> changePath;
    QString transp;
    QTimer* timer;
    QDateTime* win_datetime;
    Work *pwork;//painting_work
    QImage image;
    QPalette palette;//调色板
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
