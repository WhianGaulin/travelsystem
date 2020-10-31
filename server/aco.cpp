#include "aco.h"
bool if_nodeNeed;
int FloydDistance[N_NODE_COUNT][N_NODE_COUNT];
double pheromone[N_NODE_COUNT][MAX_OutDegree];//两两节点的信息素
list<int> initInfluenceSet;
int IF_influence[N_NODE_COUNT]={0};
int StayTime[N_NODE_COUNT]={0};//不是按顺序了，就是按城市编号的下标
std::default_random_engine eng((unsigned)time(NULL));
int IF_time_limit=1;
double rnd(double dbLow, double dbUpper)
{
    double dbTemp = rand() / ((double)RAND_MAX + 1.0);
    return dbLow + dbTemp*(dbUpper - dbLow);
}

Ant::Ant()
{
    default_Shift.to=-1;
}

void Ant::Init(){
    for (int i = 0; i<N_NODE_COUNT; i++)
        {
            m_nAllowedNode[i] = 1;					   //设置全部节点为没有去过，即都可去
            m_Shift[i]=default_Shift;
        }
    ///用初始的吸引力集合初始化influenceSet
    IF_Dead=0;
    influenceSet = initInfluenceSet;
    includeNodeNum=influenceSet.size();
    if(includeNodeNum>0){
        m_nAllowedNode[end]=0;//有必经城市，经过所有必经城市后才允许到达目标城市
    }
    m_nCurNodeNo=start;m_nAllowedNode[start]=0;
    m_nPathLength=MAX_Length;
        ///包括开始点
    m_nMovedNodeCount = 1;
        ///路径的所有节点包含m_nIncluding个including set中的节点
    m_nIncluding = 0;
        ///完成路径搜索标志位为0
    m_finish = false;

}

void Ant::deleteElement(int key){
    list<int>::iterator Itor;
        for (Itor = influenceSet.begin(); Itor != influenceSet.end();)
        {
            if (*Itor == key)
            {
                Itor = influenceSet.erase(Itor);
            }
            else
            {
                Itor++;
            }
        }
}

int Ant::getInfluence(int num)
{
    list<int>::iterator Itor;
    for (Itor = influenceSet.begin(); Itor != influenceSet.end();Itor++)
    {
        int val = MAX_Length;
        if (val > FloydDistance[num][*Itor])
        {
            val = FloydDistance[num][*Itor];
        }
        return val;
    }
    return 1;
}

///		蚂蚁选择下一个Shift
///     @return      蚂蚁选择的下一个Shift
///     @note		  蚂蚁根据到各个吸引力点的最小路径计算出转移概率，随后用轮盘随机的方法选择下一个要走的Shift
dataitem Ant::ChooseNextShift(){
    //此刻蚂蚁还没有走完特定点
    dataitem nSelectShift;
    nSelectShift.to=-1;//返回结果，先暂时把它设置为无意义
    double dbTotal = 0.0;
    dataset::iterator iter=newlog::dataset.find(m_nCurNodeNo);
    int up=newlog::dataset.count(m_nCurNodeNo);
    int count=0;
    double prob[up];memset(prob,-1,sizeof (prob)); ///保存蚂蚁从当前节点到它的邻接点去的概率（转移概率）
    dataitem shift[up]; ///与prob数组的概率对应，表示相应概率对应的shift
    for(int i=0;i<up;i++)
        shift[i]=default_Shift;
    //计算当前节点的相连节点的转移概率
    int i = 0;
    while (count != up) {
        if(m_nAllowedNode[ iter->to ]){///可以去/没去过 ( 用来限制蚂蚁不直接去终点，而是经过所有必经点后才去 )
            double flu= getInfluence(iter->to)+iter->cost;
            prob[i]= pow(pheromone[m_nCurNodeNo][iter->to], alpha)*pow(1/flu, beta);
            shift[i]=iter.value();
            dbTotal+=prob[i];
            i++;
        }
        iter++; count++;
    }//最后，i为当前节点下一步可以走的路径的数目
    if (i == 0)
    {
        return nSelectShift;//表示当前节点下一步无路可走，返回-1.（i=0表明当前节点的相邻节点都已经去过）
    }
    int nAvailable = i;//表示当前节点下一步可以走的路径的数目

    double dbTemp = 0.0;
    if (dbTotal > 0.0)
    {
        dbTemp = rnd(0.0, dbTotal);//轮盘赌  概率设置不合理，非常不合理
        for (int i = 0; i < nAvailable; i++)
        {
            dbTemp = dbTemp - prob[i];
            if (dbTemp < 0.0)
            {
                nSelectShift=shift[i];
                break;
            }
        }
    }
    if (nSelectShift.to == -1)
    {
        nSelectShift = shift[0];//补充一下而已，特殊情况就是 dbTemp==dbTotal
    }

    return nSelectShift;	//返回结果，就是要走的shift
}

QDateTime Ant::getArriveTime(dataitem iter, QDateTime Time){
    int CrossDays=0;//计算跨越天数
    if(iter.arrivalTime<iter.depatureTime)  CrossDays++;      //班次的出发时刻大于到达时刻（绝对值），必定跨天
    CrossDays+=iter.spendTime/24;                             //班次的花费小时
    CrossDays+=iter.arrivalTime/24;                           //不要让24:00变成今天的0:00,而是变成明天的0:00
    if(Time.time()>IntToQTime(iter.depatureTime))   CrossDays++;
    return QDateTime(QDate(Time.date().addDays(CrossDays)),QTime(iter.arrivalTime,0,0,0));
}

QDateTime Ant::getDepartureTime(QDateTime arriveTime, int departureHour){
    int CrossDays=0;
    int arriveHour=arriveTime.time().hour();
    if(arriveHour>departureHour){//必定跨天
        CrossDays++;
    }
    return QDateTime(QDate(arriveTime.date().addDays(CrossDays)),QTime(departureHour,0,0,0));
}


int Ant::Move()
{
    dataitem nShift=ChooseNextShift();   //选择下一个shift
    if (nShift.to != -1 )							 //说明还有节点可以走
    {
        m_Shift[m_nMovedNodeCount]=nShift;
        if(isIncluding(nShift.from))  m_currentTime=add(m_currentTime,StayTime[nShift.from]);
        ///对应m_Shift的出发时间
        m_DepatureTime[m_nMovedNodeCount]=getDepartureTime(m_currentTime,nShift.depatureTime);
        m_currentTime=getArriveTime(nShift,m_currentTime);//此时current已经假设为到达Shift->to的状态
        ///对应m_Shift的到达时间
        m_arriveTime[m_nMovedNodeCount]=m_currentTime;

        m_nCurNodeNo = nShift.to;										//改变当前所在节点为选择的节点
        m_nMovedNodeCount++;											//已经去过的节点数量加1
        m_nAllowedNode[m_nCurNodeNo]=0;
        /*检测这次是否要走 还没有走的必经点*/
        if(IF_time_limit&&m_currentTime>endTime){
            IF_Dead=1;
            return 2;//超时了，就返回2，无路可走了
        }
        if (isIncluding(nShift.to))//如果下一步是途径城市的话
        {
            m_nIncluding++;
            deleteElement(nShift.to);	//如果选择的节点是influence set里面的点，将其从influence set中删除
            if(m_nIncluding == includeNodeNum)
                m_nAllowedNode[end] = 1;
        }
        else if (nShift.to==end) {//已经到达了目的地
            m_nMovedNodeCount--;//多加了
            return 2;//不用走了，到了目的地，走到头了
        }
        return 1;
    }
    else{											 //nNode==-1, 说明无路可走了，此只蚂蚁走到头了
        m_nMovedNodeCount--;//多加了
        return 2;
    }
}

///		Search函数

///		蚂蚁搜索路径
///     @note		  蚂蚁进行路径地搜索，若满足搜索要求则退出搜索并进行路径长度计算，反之则一直处于Move寻路状态
void Ant::Search()
{
    ///蚂蚁初始化
    Init();
    while (  1==Move() ){
        ;
    }
    m_finish=true;
    ///完成搜索，计算路径长度
    if(IF_Dead){
        m_nPathLength=MAX_Length;
    }
    else if(m_nIncluding == includeNodeNum&&m_nCurNodeNo==end){//已经走完了所有必经点
        CalPathLength();
    }
}


///		CalPathLength函数

///		计算蚂蚁走过的路径长度
///     @note		  计算总的路径长度
void Ant::CalPathLength()
{
    int i = 1;
    m_nPathLength = 0;
    while (m_Shift[i].to != -1&&i<cityNum-1)
    {
        m_nPathLength +=m_Shift[i].cost;
        i++;
    }
}


AntSystem::AntSystem(){

}

void AntSystem::set_demand(int startCity, int EndCity,QDateTime _startTime,QDateTime _endTime){
    start=startCity;
    end=EndCity;
    startTime=_startTime;
    endTime=_endTime;
    for(int i=0;i<N_ANT_COUNT;i++){
        m_cAntAry[i].start=startCity;
        m_cAntAry[i].end=EndCity;
        m_cAntAry[i].startTime=_startTime;
        m_cAntAry[i].endTime=_endTime;
        m_cAntAry[i].m_currentTime=_startTime;
    }
}

///		InitData函数

///		初始化蚂蚁的信息素分布
///     @note		  初始化topo图中所有相连边的信息素大小为C
void AntSystem::InitData()
{
    my_traveler.myplan.clear();
    set_demand(CityToInt(my_traveler.DeparetureCity),CityToInt(my_traveler.EndCity),
               my_traveler.startTime,my_traveler.latestTime);
    m_cGlobalBestAnt.m_nPathLength = MAX_Length;//初始化全局最优蚂蚁路径长度最大

    for(int i=0;i<N_NODE_COUNT;i++){//初始化信息素
        dataset::iterator iter=newlog::dataset.find(i);
        while (iter!=newlog::dataset.end()&&iter.key()==i) {
            pheromone[i][iter->to]=C;
            iter++;
        }
    }

    for(int source=0;source<cityNum;source++){//计算Floyd距离
        my_traveler.Dijkstra(source);
        for(int to=0;to<cityNum;to++){
            FloydDistance[source][to]=my_traveler.cost[to];
        }
    }

    initInfluenceSet.clear();//初始化途经城市前，把以前的删掉掉！！！
    QVector<QString>::iterator iter=my_traveler.MidCity.begin();//初始化途经城市
    while (iter!=my_traveler.MidCity.end()) {//按顺序插入
         initInfluenceSet.push_back(CityToInt(*iter));
         IF_influence[CityToInt(*iter)]=1;
         iter++;
    }
    for (int i=0;i<N_NODE_COUNT;i++) {
        StayTime[i]=my_traveler.StayTime[i];
    }
    /*第一次迭代时，还没有全局最优解，所有计算不出最大值和最小值，先设置为0.0*/
    Qmax = 0.0;
    Qmin = 0.0;
    m_dbRate = m_dbRatio;//最大信息素和最小信息素的比值为15倍
}

///		UpdateTrial函数

///		更新路径的信息素分布
///     @param flag  表示是否使用全局最优解，1表示使用全局最优解，0表示使用迭代最优解
///     @note			 使用全局最优和迭代最优交替更新的策略，每5次迭代更新一次信息素
void AntSystem::Updatepheromone(int nFlag)
{

    if (nFlag == 1)			//使用全局最优解
    {
        if (m_cGlobalBestAnt.m_nPathLength == MAX_Length)
        {
            return;		//在这次迭代后，全局最优蚂蚁还不存在
        }
        m_cTempAnt = m_cGlobalBestAnt;
    }
    else							//使用迭代最优解
    {
        if (m_cIterationBestAnt.m_nPathLength == MAX_Length)
        {
            return;		//这次迭代没有产生一个解
        }
        m_cTempAnt = m_cIterationBestAnt;
    }
    double dbTempAry[N_NODE_COUNT][MAX_OutDegree];
    memset(dbTempAry, 0, sizeof(dbTempAry));

    /*只用全局最优或者某次迭代的最优蚂蚁去更新信息素*/
    double dbTemp = 1.0 / m_cTempAnt.m_nPathLength;//增加的信息素和距离成反比
    for (int j = 1; j <= m_cTempAnt.m_nMovedNodeCount; j++)
    {
        int m,n;
        m=m_cTempAnt.m_Shift[j].to;
        n=m_cTempAnt.m_Shift[j].from;
        dbTempAry[n][m] = dbTempAry[n][m] + dbTemp;
    }

    /*更新环境信息素，需要参考最大信息素和最小信息素*/
    Qmax = 1.0 / (m_cGlobalBestAnt.m_nPathLength*(1.0 - rou));
    Qmin = Qmax / m_dbRate;
    /*------------------------------------------------------------*/
    for(int i=0;i<N_NODE_COUNT;i++){
        dataset::iterator iter=newlog::dataset.find(i);
        while (iter!=newlog::dataset.end()&&iter.key()==i) {
            int j=iter->to;
            pheromone[i][j]=pheromone[i][j]*rou+dbTempAry[i][j];
            if(pheromone[i][j]<Qmin) pheromone[i][j]=Qmin;
            if(pheromone[i][j]>Qmax) pheromone[i][j]=Qmax;
            iter++;
        }
    }
}


///		Search函数

///		寻找全局最优蚂蚁来取得最优路径，同时以约束模型判断此次寻路是否有解
///     @note		  若满足所有条件，即有解则返回最优解；若无解则根据约束模型再次进行寻路直到满足条件输出一个次优解
void AntSystem::Search()
{
    qDebug()<<"Search...";
    m_cGlobalBestAnt.m_nPathLength = MAX_Length;//初始化全局最优蚂蚁的路径长度为INT_MAX

    while (true)
    {
        for (int i = 0; i < N_IT_COUNT; i++) //迭代
        {
            m_cIterationBestAnt.m_nPathLength = MAX_Length;//初始化本次迭代的最优解为INT_MAX
            //每只蚂蚁搜索一遍
            for (int j = 0; j < N_ANT_COUNT; j++) {//一次迭代放出所有蚂蚁出去搜索
                m_cAntAry[j].Search();
            }//for
            /*-----------------------------------------------*/
            //保存最佳结果
            for (int j = 0; j < N_ANT_COUNT; j++)
            {
                if (m_cAntAry[j].m_finish && m_cAntAry[j].m_nPathLength < m_cGlobalBestAnt.m_nPathLength) {
                    m_cGlobalBestAnt = m_cAntAry[j];//根据路径长度（cost）更新全局的最优蚂蚁
                }
                if (m_cAntAry[j].m_finish&&m_cAntAry[j].m_nPathLength < m_cIterationBestAnt.m_nPathLength) {
                    m_cIterationBestAnt = m_cAntAry[j];//更新本次迭代的最优蚂蚁
                }
            }//for
            //使用全局最优和迭代最优交替更新的策略
            //每5次迭代使用一次全局最优蚂s蚁更新信息素`
            if ((i + 1) % 5 == 0)
            {
                Updatepheromone(1);//使用全局最优更新信息素
            }
            else
            {
                Updatepheromone(0);//使用局部最优更新信息素
            }
        }//for

        if_nodeNeed = m_cGlobalBestAnt.influenceSet.empty();														 /* 必经点==要求数*/
        if (if_nodeNeed&&m_cGlobalBestAnt.m_nPathLength<MAX_Length)
        {
            IF_Success=1;
            printPath();
            makePlan();
            break;
        }
        else
        {
            qDebug()<<"无解";
            IF_Success=0;
            break;
        }
    }//while

}// Search

///		printPath函数

///		输出最优的路径
///     @note      将全局最优蚂蚁所走的路径和所用的花费，进行打印输出
void AntSystem::printPath()
{
    qDebug()<<"printPath";
    qDebug() << "花费 = " << m_cGlobalBestAnt.m_nPathLength;
    qDebug()<< "输出路径为：    ";
    for (int i=1; i <= m_cGlobalBestAnt.m_nMovedNodeCount; i++)
            qDebug()<< "-->"<<IntToCity(m_cGlobalBestAnt.m_Shift[i].from)<<IntToCity(m_cGlobalBestAnt.m_Shift[i].to)<<
                       m_cGlobalBestAnt.m_Shift[i].shiftID<<m_cGlobalBestAnt.m_Shift[i].cost
                    <<m_cGlobalBestAnt.m_DepatureTime[i]<<m_cGlobalBestAnt.m_arriveTime[i];
}


void AntSystem::makePlan(){
    for(int i=1;i<=m_cGlobalBestAnt.m_nMovedNodeCount;i++){
        planItem a;
        a.item=m_cGlobalBestAnt.m_Shift[i];
        a.departureTime=m_cGlobalBestAnt.m_DepatureTime[i];
        a.arriveTime=m_cGlobalBestAnt.m_arriveTime[i];
        my_traveler.myplan.push_back(a);
    }
    my_traveler.cost[end]=m_cGlobalBestAnt.m_nPathLength;
    my_traveler.arrive_time[end]=m_cGlobalBestAnt.m_arriveTime[m_cGlobalBestAnt.m_nMovedNodeCount];
}


