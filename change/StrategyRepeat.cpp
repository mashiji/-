#include "StrategyRepeat.h"
#include "HardwareAdapter.h"
#include <pthread.h>
#include <unistd.h>

void *  remainPush(void*  arg);
void *  repeatControl(void * arg);

extern bool HardwareAdapter_Ethernet;
extern bool HardwareAdapter_CAN;
extern bool HardwareAdapter_RS422;

CStrategyRepeat::CStrategyRepeat(void)
{
}


CStrategyRepeat::~CStrategyRepeat(void)
{
}

bool CStrategyRepeat::initial(CSoftBus* m_softbus)//初始化
{
    m_pSoftbus = m_softbus;
    m_pStrategyPriority = m_softbus->m_pTransRecModel->m_pStrategyPriority;
    m_pHardwareAdapter = m_softbus->m_pHardwareAdapter;
    //初始化临界区
    pthread_mutex_init(&queuemain_lock,NULL);
    pthread_mutex_init(&MapAckRec_lock,NULL);
    pthread_mutex_init(&TransNode_lock,NULL);
   //创建相应线程
    pthread_t   tid2,tid3;

//    pthread_attr_t  thread_attr;
//    struct  sched_param  schedule_param;
//    pthread_attr_init(&thread_attr);
//    schedule_param.__sched_priority = 99;
//    pthread_attr_setinheritsched(&thread_attr,PTHREAD_EXPLICIT_SCHED);
//    pthread_create(&tid2,&thread_attr,repeatControl,(void *)this);

    pthread_create(&tid2,NULL,repeatControl,(void *) this);
//    pthread_create(&tid3,NULL,remainPush,(void *)this);

    return true;
}


//为什么要在发送方弄这么一个线程呢，就是怕发送方发的太慢，2秒钟一个包，包不能及时得到确认。
//每隔0.5s发送一个ackReq，来推一下TransmitNode节点中的queueRepeat的包进行确认或重传。
//方法就是遍历每个CTransmitNode对象，为每一个对象发送一个ackReq
void *  remainPush(void*  arg)
{
    CStrategyRepeat* pSR = (CStrategyRepeat*)arg;
    CTransmitNode *tempNode;
    while (1)
    {
        pthread_mutex_lock(&pSR->TransNode_lock);//进入临界区
        //遍历节点，并发送ack
        for (pSR->TransmitNodeMapIterTemp = pSR->TransmitNodeMap.begin(); pSR->TransmitNodeMapIterTemp != pSR->TransmitNodeMap.end(); pSR->TransmitNodeMapIterTemp++)
        {
            tempNode = pSR->TransmitNodeMapIterTemp->second;
            tempNode->transmit(NULL, true);
        }
        pthread_mutex_unlock(&pSR->TransNode_lock);//离开临界区
        usleep(500000);
    }
}



bool CStrategyRepeat::transmit(CSoftBusMsg* msg)//发送
{
    pthread_mutex_lock(&queuemain_lock);//进入临界区
    queueMain.push(msg);//将msg传入队列中
    pthread_mutex_unlock(&queuemain_lock);//离开临界区
    return true;
}



//从主队列中取出一个包，按包的目的地址调用相应TransminNode的发送函数，
//根据主队列相邻包之间的关系，判断是否在发完数据包后跟一个ackReq
void *  repeatControl(void * arg)//重传控制
{
    CStrategyRepeat* pSR = (CStrategyRepeat*)arg;
    CSoftBusMsg *tempMsg1, *tempMsg2;
    CTransmitNode *tempNode;
    int retVal = 0;
    bool queueMainEmpty;//队列是否为空的标识
    while (1)//一直执行
    {
        if(HardwareAdapter_Ethernet)
                usleep(20000);
        else if(HardwareAdapter_RS422)
                usleep(50000);
        else if(HardwareAdapter_CAN)
                usleep(10000);

        pthread_mutex_lock(&pSR->queuemain_lock);//进入临界区
        queueMainEmpty = pSR->queueMain.empty();//队列状态
        pthread_mutex_unlock(&pSR->queuemain_lock);//离开临界区
        if (!queueMainEmpty)//如果队列不为空
        {
            pthread_mutex_lock(&pSR->queuemain_lock);//进入临界区
            tempMsg1 = pSR->queueMain.front();//取出主队列第一个元素
            pSR->queueMain.pop();//移除主队列第一个元素
            queueMainEmpty = pSR->queueMain.empty();//再判断是否为空
            pthread_mutex_unlock(&pSR->queuemain_lock);//离开临界区
             pthread_mutex_lock(&pSR->TransNode_lock);//进入临界区
            if (queueMainEmpty)//如果此时队列为空
            {
                pSR->TransmitNodeMapIterTemp = pSR->TransmitNodeMap.find(tempMsg1->destID);//寻找目的地址
                if (pSR->TransmitNodeMapIterTemp != pSR->TransmitNodeMap.end())//找到影射
                {
                    tempNode = pSR->TransmitNodeMapIterTemp->second;
                    retVal = tempNode->transmit(tempMsg1, false);
                    //cout<<"find node in transmitNodeMap map key is "<<(int)pSR->TransmitNodeMapIterTemp->first<<" flag is true"<<endl;
                }
                else//没找到映射
                {
                    //创建新的映射
                    tempNode = new CTransmitNode(pSR, pSR->m_pHardwareAdapter, tempMsg1->sourceID, tempMsg1->destID);
                    pSR->TransmitNodeMap.insert(make_pair(tempMsg1->destID, tempNode));
                    retVal = tempNode->transmit(tempMsg1, true);
                    //cout<<"create node in transmitNodeMap map key is "<<(int)tempMsg1->destID<<" flag is true"<<endl;
                }
            }
            else//如果此时队列不为空
            {
                pthread_mutex_lock(&pSR->queuemain_lock);//进入临界区
                tempMsg2 = pSR->queueMain.front();//取第一个
                pthread_mutex_unlock(&pSR->queuemain_lock);//离开临界区
                if (tempMsg1->destID != tempMsg2->destID)//如果目的地址不相等
                {
                    pSR->TransmitNodeMapIterTemp = pSR->TransmitNodeMap.find(tempMsg1->destID);
                    if (pSR->TransmitNodeMapIterTemp != pSR->TransmitNodeMap.end())//不是最后一个
                    {
                        tempNode = pSR->TransmitNodeMapIterTemp->second;
                        retVal = tempNode->transmit(tempMsg1, true);
                        //cout<<"find node in transmitNodeMap map key is "<<(int)pSR->TransmitNodeMapIterTemp->first<<" flag is true"<<endl;
                    }
                    else//是最后一个
                    {
                        //新建映射
                        tempNode = new CTransmitNode(pSR, pSR->m_pHardwareAdapter, tempMsg1->sourceID, tempMsg1->destID);
                        pSR->TransmitNodeMap.insert(make_pair(tempMsg1->destID, tempNode));
                        retVal = tempNode->transmit(tempMsg1, true);
                        //cout<<"create node in transmitNodeMap map key is "<<(int)tempMsg1->destID<<" flag is true"<<endl;
                    }
                }
                else//目的地址相等
                {
                    pSR->TransmitNodeMapIterTemp = pSR->TransmitNodeMap.find(tempMsg1->destID);
                    if (pSR->TransmitNodeMapIterTemp != pSR->TransmitNodeMap.end())//不是最后一个
                    {
                        tempNode = pSR->TransmitNodeMapIterTemp->second;
                        retVal = tempNode->transmit(tempMsg1, false);
                        //cout<<"find node in transmitNodeMap map key is "<<(int)pSR->TransmitNodeMapIterTemp->first<<" flag is false"<<endl;
                    }
                    else//是最后一个
                    {
                        //新建映射
                        tempNode = new CTransmitNode(pSR, pSR->m_pHardwareAdapter, tempMsg1->sourceID, tempMsg1->destID);
                        pSR->TransmitNodeMap.insert(make_pair(tempMsg1->destID, tempNode));
                        retVal = tempNode->transmit(tempMsg1, false);
                        //cout<<"create node in transmitNodeMap map key is "<<(int)tempMsg1->destID<<" flag is false"<<endl;
                    }
                }
            }
            pthread_mutex_unlock(&pSR->TransNode_lock);//离开临界区
        }

    }
}



//receive函数要做的判断：什么时候发送ack，窗移动，交付上层
bool CStrategyRepeat::receive(CSoftBusMsg* msg)
{
    if (msg->type == SOFT_BUS_TYPE_COMMAND && msg->commandSet == SYSTEM_COMMAND_SET && msg->commandCode == ACK)//是系统保留命令
    {
        //来的是ack帧，交给发送线程，让他去完成他的算法
    //	printf("ACK received ---- ackSeq %d -- ackGenerator %d \n", msg->ackSeq, msg->ackGenerator);
        queue<CSoftBusMsg*>* ackQueueTemp;
        pthread_mutex_lock(&MapAckRec_lock);//进入临界区
        recAckIterTemp = recAck.find(msg->sourceID);//查找映射
        if (recAckIterTemp != recAck.end())//不是最后一个
        {
            ackQueueTemp = recAckIterTemp->second;
            ackQueueTemp->push(msg);//将msg置入队列
        }
        else //是最后一个
        {
            ackQueueTemp = new queue<CSoftBusMsg*>();
            recAck.insert(make_pair(msg->sourceID, ackQueueTemp));//新建映射
            ackQueueTemp->push(msg);//将msg置入队列
        }
        pthread_mutex_unlock(&MapAckRec_lock);//离开临界区
    }
    else//不是系统保留命令
    {
        switch (msg->commandCode)//检测命令码
        {
            case ACK_REQUEST:{
                                // printf("ACK_REQUEST received ---- seq %d \n",msg->seq);
                                 break;
            }
            case DETECT:{
                            //printf("DETECT received ---- seq %d \n",msg->seq);
                            break;
            }
            default:{
                        //printf("DATA received ---- seq %d \n",msg->seq);
                        //调用优先级模块的接收函数
                    // m_pStrategyPriority->receive(msg);
            }
        }
        CReceiveWindow* recWinTemp;
                RecNodeMapIterTemp = RecNodeMap.find(msg->sourceID);
                if (RecNodeMapIterTemp != RecNodeMap.end())//存在映射
                {
                    recWinTemp = RecNodeMapIterTemp->second;
                    recWinTemp->insert(msg);
                    if (recWinTemp->isSendAck)//发送ACK
                    {
                        //参数赋值
                        CSoftBusMsg* ackMsg = new CSoftBusMsg();
                        ackMsg->type = SOFT_BUS_TYPE_COMMAND;
                        ackMsg->commandSet = SYSTEM_COMMAND_SET;
                        ackMsg->commandCode = ACK;
                        ackMsg->sourceID = msg->destID;
                        ackMsg->destID = msg->sourceID;
                        ackMsg->ackSeq = recWinTemp->ackSeq;
                        ackMsg->ackGenerator = msg->seq;
                        m_pHardwareAdapter->transmit(ackMsg);
                        //send ack
                    }
                    if (recWinTemp->isHandover)
                    {
                       m_pStrategyPriority->receive(msg);

                        //hand over
                    }
                }
                else //不存在映射
                {
                    CReceiveWindow* recWinTemp = new CReceiveWindow(m_pSoftbus);
                    RecNodeMap.insert(make_pair(msg->sourceID, recWinTemp));//新建一个
                    recWinTemp->insert(msg);
                    if (recWinTemp->isSendAck)//ACK发送
                    {
                        //对参数进行赋值
                        CSoftBusMsg* ackMsg = new CSoftBusMsg();
                        ackMsg->type = SOFT_BUS_TYPE_COMMAND;
                        ackMsg->commandSet = SYSTEM_COMMAND_SET;
                        ackMsg->commandCode = ACK;
                        ackMsg->sourceID = msg->destID;
                        ackMsg->destID = msg->sourceID;
                        ackMsg->ackSeq = recWinTemp->ackSeq;
                        ackMsg->ackGenerator = msg->seq;
                        m_pHardwareAdapter->transmit(ackMsg);
                    }
                    if (recWinTemp->isHandover)
                    {
                       m_pStrategyPriority->receive(msg);
                        //hand over
                    }
                }
            }
            return true;
        }


