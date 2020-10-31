//必经点的吸引力
//限制蚂蚁走过的节点的数量？
//更新信息素方法：
//1：每只蚂蚁走完后更新信息素+一次迭代后的最优蚂蚁更新信息素
//2：一批蚂蚁中的最优蚂蚁更新信息素+每五次更新全局最优蚂蚁的信息素
//挥发：更新的同时挥发
#ifndef ACO_H
#define ACO_H
#include <random>
#include <time.h>
#include "newlog.h"
#include <list>
#include <stack>
#include <cmath>
#include <QDateTime>
#include <queue>
using std::list;
using std::stack;
using std::queue;
#define N_ANT_COUNT 200// 蚂蚁数量
#define N_IT_COUNT 100 // 迭代次数
#define N_NODE_COUNT 15 // 最大节点数量
#define MAX_OutDegree 250
#define alpha 1  //信息素重要程度
#define beta 3   //启发式因子/距离 重要程度
#define rou 0.8  //信息素残留参数
#define DBQ 100.0	//总的信息素
#define m_dbRatio 15
#define MAX_Length 1000000
#define C 10000  //信息素初始值
#include "traveler.h";
extern bool if_nodeNeed;
extern int FloydDistance[N_NODE_COUNT][N_NODE_COUNT];
extern double pheromone[N_NODE_COUNT][MAX_OutDegree];//两两节点的信息素
extern list<int> initInfluenceSet;
extern double rnd(double dbLow, double dbUpper);
extern int IF_influence[N_NODE_COUNT];
extern int StayTime[N_NODE_COUNT];
extern int IF_time_limit;
class Ant
{
public:
    Ant();
    int start;
    int end;
    dataitem default_Shift;
    QDateTime m_currentTime;
    QDateTime startTime;
    QDateTime endTime;
    int IF_Dead=0;
    dataitem m_Shift[N_NODE_COUNT];
    QDateTime m_DepatureTime[N_NODE_COUNT];
    QDateTime m_arriveTime[N_NODE_COUNT];
    QDateTime getArriveTime(dataitem iter,QDateTime Time);
    QDateTime getDepartureTime(QDateTime arriveTime,int departureHour);
    dataitem pre_shift[N_NODE_COUNT];
    int m_nAllowedNode[N_NODE_COUNT];
    int m_nPathLength=0;
    /// 已经去过的节点数量
    int m_nMovedNodeCount;
    int m_nIncluding;
    int m_nCurNodeNo;
    bool m_finish;//是否成功到达终点
    list<int> influenceSet;//必经城市
    int includeNodeNum;
    ///		getInfluence函数

    ///		取得指定编号的节点到所有吸引力节点的最小距离
    ///     @param num 输入的节点编号
    ///     @return      到吸引力点的最小路径值
    int getInfluence(int num);
    ///		ChooseNextShift函数

    ///		蚂蚁选择下一个Shift
    ///     @return      蚂蚁选择的下一个Shift
    ///     @note		  蚂蚁根据到各个吸引力点的最小路径计算出转移概率，随后用轮盘随机的方法选择下一个要走的Shift
    dataitem ChooseNextShift();
    ///		Init函数

    ///		初始化蚂蚁信息
    ///     @note		  初始化蚂蚁的成员变量，包括起始点、终止点、必经点集合等等
    void Init();
    ///		Move函数

    ///		蚂蚁在节点间移动
    ///     @return      若蚂蚁可以走到下一个节点则返回1，反之蚂蚁无路可走则返回2
    ///     @note		  无路可走说明：必过点没有走完(因为走完了就不会进入这个Move函数)，同时又没有下一条可走的路
    int Move();
    ///		Search函数

    ///		蚂蚁搜索路径
    ///     @note		  蚂蚁进行路径地搜索，若满足搜索要求则退出搜索并进行路径长度计算，反之则一直处于Move寻路状态
    void Search();
    ///		CalPathLength函数

    ///		计算蚂蚁走过的路径长度
    ///     @note		  用Dijkstra算法进行与终止点的最短距离相连，并且计算总的路径长度
    void CalPathLength();
    ///		deleteElement函数

    ///		用于蚂蚁移动后，删除吸引力集合的元素
    ///     @param num   输入的必经点点编号
    ///     @note		  蚂蚁经过一个必经点后，从吸引力集合中删除此节点，表明该必经点将不再起吸引的作用
    void deleteElement(int num);

    int isIncluding(int num){
        return IF_influence[num];
    }

};

class AntSystem//在里面用到了Ant
{
public:
    AntSystem() ;
public:
    int IF_Success;
    traveler my_traveler;
    int start;
    int end;
    QDateTime startTime;
    QDateTime endTime;
    /// 蚂蚁数组
    Ant	 m_cAntAry[N_ANT_COUNT];
    /// 全局最优蚂蚁
    Ant	 m_cGlobalBestAnt;
    /// 本次迭代最优蚂蚁
    Ant	 m_cIterationBestAnt;
    /// 临时蚂蚁
    Ant	 m_cTempAnt;
    /// 信息素最大值
    double	Qmax;
    /// 信息素最小值
    double	Qmin;
    /// 最大值和最小值的比率
    double	m_dbRate;

    void set_demand(int startCity,int EndCity,QDateTime startTime,QDateTime endTime);

    ///		InitData函数

    ///		初始化蚂蚁的信息素分布,Floyd距离等等
    ///     @note		  初始化topo图中所有相连边的信息素大小为10000
    void InitData();

    ///		Search函数

    ///		寻找全局最优蚂蚁来取得最优路径，同时以约束模型判断此次寻路是否有解
    ///     @note		  若满足所有条件，即有解则返回最优解；若无解则根据约束模型再次进行寻路直到满足条件输出一个次优解
    void Search();

    ///		UpdateTrial函数

    ///		更新路径的信息素分布
    ///     @param flag  表示是否使用全局最优解，1表示使用全局最优解，0表示使用迭代最优解
    ///     @note   使用全局最优和迭代最优交替更新的策略，每5次迭代更新一次信息素
    void Updatepheromone(int flag);

    ///		printPath函数

    ///		输出最优的路径
    ///     @note      将全局最优蚂蚁所走的路径和所用的花费，进行打印输出
    void printPath();

    void makePlan();
};



#endif // ACO_H
