#include "TransmitNode.h"
#include "StrategyRepeat.h"
#include "HardwareAdapter.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

void*  dealQueueFull(void*  arg);
void*  dealQueueBuffer(void*  arg);
void*  linkDetect(void*  arg);

CTransmitNode::CTransmitNode(CStrategyRepeat* pCR, CHardwareAdapter* pHA, unsigned char src, unsigned char dst)
{
    //构造函数
    state = link_break;
    seq = 1;
    Queue_Repeat_Max_Len = pCR->m_pSoftbus->SendWindow;
    TimeDelay =  pCR->m_pSoftbus->TimeDelay;
    destNodeRecSeq = 0;
    dealQueueFullCount = 0;
    m_pStrategyRepeat = pCR;
    m_pHardwareAdapter = pHA;
    sourceID = src;
    destID = dst;
    //hThread = ::CreateThread(NULL, NULL, linkDetect, this, 0, &dwThread);
    //WaitForSingleObject(hThread, 3000);
    //GetExitCodeThread(hThread, &retVal);
    //if (retVal)
     pthread_t  tid;
     void * tret = NULL;
     pthread_create(&tid,NULL,linkDetect,(void *)this);  //需要等待该线程结束后再执行其他程序，线程阻塞
     pthread_join(tid,(void **)&tret) ;
     if((unsigned short *) tret == 0)
        state = link_break;
     else
        state = normal;
}


CTransmitNode::~CTransmitNode(void)
{
}


int CTransmitNode::transmit(CSoftBusMsg* msg, bool ackReqFlag)//发送函数
{
    if (msg != NULL)//如果不为空
    {
        msg->seq = seq;
        seq++;
    }
    if (state == link_break)//连接断开
    {
        if (msg != NULL)//不为空
        {
            delete(msg);//释放内存
            msg = NULL;//置为空
        }
        pthread_t  tid4;
        void * tret;
        pthread_create(&tid4,NULL,linkDetect,this);//重新创建线程
        if(pthread_join(tid4,&tret) == 0)
            state = normal;
        else
            state = link_break;
    }
    if (state == queue_full)//队列满了
    {
        queueBuffer.push(msg);
        return(queue_full);
    }
    if (state == normal)//正常状态
    {
        if (msg == NULL && queueRepeat.empty())//msg和重传队列均为空
            ;
        else//有一个不为空
        {
            transAndCheck(msg, ackReqFlag);//发送并检查
            if (queueRepeat.size() >= Queue_Repeat_Max_Len)//重传队列的尺寸达到最大
            {
                printf("\n");
                printf("---- dealQueueFull Thread start\n");
                printf("\n");
                //hThread = ::CreateThread(NULL, NULL, dealQueueFull, this, 0, &dwThread);
                pthread_t   tid5;
                pthread_create(&tid5,NULL,dealQueueFull,(void *)this);
                state = queue_full;//设置为满状态
            }
        }
        return(normal);
    }
}



//发送一个数据包并检查recAck这个map中与自己的目的节点对应的队列
//如果ackReqFlag为true那么发完数据包后顺带发送一个ackRequest包，该包序列号为刚才发送的那个数据包的序列号
//如果数据包指针为NULL,ackReqFlag为true，那么光放一个ackRequest，该包序列号为本TransmitNode中最近发送的一个数据包的序列号
int CTransmitNode::transAndCheck(CSoftBusMsg *msg, bool ackReqFlag)
{
    if (msg != NULL)//如果不为空
    {
        m_pHardwareAdapter->transmit(msg);//发送
        queueRepeat.push(msg);//置入队列中
    }
    if (ackReqFlag)//是ACK
    {
        //参数赋值
        CSoftBusMsg* newMsg = new CSoftBusMsg();
        newMsg->type = SOFT_BUS_TYPE_COMMAND;
        newMsg->commandSet = SYSTEM_COMMAND_SET;
        newMsg->commandCode = ACK_REQUEST;
        newMsg->sourceID = sourceID;
        newMsg->destID = destID;
        newMsg->seq = queueRepeat.back()->seq;//最后一个元素
        m_pHardwareAdapter->transmit(newMsg);
    }
    queue<CSoftBusMsg*>* queuePtr;
    CSoftBusMsg* tempAckMsg;
    pthread_mutex_lock(&m_pStrategyRepeat->MapAckRec_lock);//用来保护recAck
    m_pStrategyRepeat->recAckIterTemp = m_pStrategyRepeat->recAck.find(destID);//寻找目的地址
    if (m_pStrategyRepeat->recAckIterTemp != m_pStrategyRepeat->recAck.end())//不指向最后一个
    {
        queuePtr = m_pStrategyRepeat->recAckIterTemp->second;
        if (!queuePtr->empty())//队列不为空
        {
            tempAckMsg = queuePtr->front();//取出第一个元素
            queuePtr->pop();//移除第一个元素
            //更新已知的对方节点收到的seq
            if (tempAckMsg->ackGenerator >= destNodeRecSeq)
            {
                destNodeRecSeq = tempAckMsg->ackGenerator;//新来的ackGenerator比本地的新
            }
            else
            {
                if (destNodeRecSeq - tempAckMsg->ackGenerator >150)//新来的ackGenerator虽然比本地的小但是小太多，说明该序列号经过了循环，新来的序列号比本地的靠前
                {
                    destNodeRecSeq = tempAckMsg->ackGenerator;
                }
                else
                    //为什么会出现新来的序列号比本地的小这种情景。因为接收方总是在收到一个msg的时候才去判断该不该发ack该ack参数是多少
                    //ackGenerator的含义也是如此表示这个ack是接收方在收到哪个包时产生的，而不是接收方接收到的最大序列号现在为多少
                    //所以可能出现接收方接收到重传包将断层补齐或者计算出新的断层，发送了ack，此时ack的ackGenerator并非是接收方收到的最新的序列号
                    //所以在这里做一个记录，记录接收方到目前为止接收到的最新的序列号是多少
                    destNodeRecSeq = destNodeRecSeq;//新来的没有本地的大，不变
            }
            //接收方可能受到三种包，一种是正常按序排列而来的数据包，一种是按正常排序而来的ackReq包，还有一种
            //比较特殊，是重传来的包，它不是按序来的。那么接受方发送ack时，ackGenerator大部分情况下是等于接收
            //方收到的最新的包的序号，也有部分情况是等于重传包的序号，也就是重传包填补断层而凑够8个的情况下。
            //所以这里没有采用== 而是采用>=，>就是为了应对收到旧的重传包而产生的ack。
            //只要确认的ack等于接收方收到的最新的ack或者 新于 接收方收到的最新的ack则认为正常无丢包
            if (tempAckMsg->ackSeq >= destNodeRecSeq)
            {
                // 253 254 255 0 1 2 3 4 5 6
                //大于不一定代表新于。比如收到ack(255,1)255还是旧与1，判断应该重传
                if (tempAckMsg->ackSeq - destNodeRecSeq >150)
                {
                    //在收到ack时都要先判断queueRepeat能不能pop到收到ack的seq处，为什么要进行这样的判断呢
                    //为什么不直接来了ack我就pop呢，ack的seq确实是一直往前推进的，但是有可能出现相邻的两个ack
                    //的seq相同，比如收满8个并且受到了ackReq，这时会产生两个seq相同的ack.因为发送方在处理前面
                    //的ack时，将它对应的seq已经弹出了，所以在处理后面的ack时，就不应该弹出任何东西。但此处
                    //所写的逻辑当来了ack时至少弹出一个。所以要先进行判断。
                    if (tempAckMsg->ackSeq >= queueRepeat.front()->seq || (queueRepeat.front()->seq - tempAckMsg->ackSeq >150))
                    {
                        while (queueRepeat.front()->seq != tempAckMsg->ackSeq)
                            queueRepeat.pop();
                        queueRepeat.pop();
                    }
                    //收到异常ack，弹出已经确认的，然后重传位于队首的一个包。
                    m_pHardwareAdapter->transmit(queueRepeat.front());
                }
                else
                {
                    if (tempAckMsg->ackSeq >= queueRepeat.front()->seq || (queueRepeat.front()->seq - tempAckMsg->ackSeq >150))
                    {
                        while (queueRepeat.front()->seq != tempAckMsg->ackSeq)
                            queueRepeat.pop();
                        queueRepeat.pop();
                    }
                }
            }
            else
            {
                //当收到ack的seq 旧与 接收方收到的最近的seq时。判断为重传
                if (destNodeRecSeq - tempAckMsg->ackSeq >150)
                {
                    if (tempAckMsg->ackSeq >= queueRepeat.front()->seq || (queueRepeat.front()->seq - tempAckMsg->ackSeq >150))
                    {
                        while (queueRepeat.front()->seq != tempAckMsg->ackSeq)
                            queueRepeat.pop();
                        queueRepeat.pop();
                    }
                }
                else
                {
                    if (tempAckMsg->ackSeq >= queueRepeat.front()->seq || (queueRepeat.front()->seq - tempAckMsg->ackSeq >150))
                    {
                        while (queueRepeat.front()->seq != tempAckMsg->ackSeq)
                            queueRepeat.pop();
                        queueRepeat.pop();
                    }
                    m_pHardwareAdapter->transmit(queueRepeat.front());//发送重传队列的第一个
                }
            }
            //delete(tempAckMsg);
        }
    }
    pthread_mutex_unlock(&m_pStrategyRepeat->MapAckRec_lock);//离开临界区
    return true;
}



//用于处理queueRepeat满的情况
void CTransmitNode::check()//检测
{
    queue<CSoftBusMsg*>* queuePtr;
    CSoftBusMsg* tempAckMsg;
    pthread_mutex_lock(&m_pStrategyRepeat->MapAckRec_lock);//进入临界区
    m_pStrategyRepeat->recAckIterTemp = m_pStrategyRepeat->recAck.find(destID);//寻找目的ID
    if (m_pStrategyRepeat->recAckIterTemp != m_pStrategyRepeat->recAck.end())//不是最后一个
    {
        queuePtr = m_pStrategyRepeat->recAckIterTemp->second;
        if (!queuePtr->empty())//队列不为空
        {
            tempAckMsg = queuePtr->front();//取出第一个
            queuePtr->pop();//移除第一个
            //更新已知的对方节点收到的seq
            if (tempAckMsg->ackGenerator >= destNodeRecSeq)
            {
                destNodeRecSeq = tempAckMsg->ackGenerator;
            }
            else
            {
                if (destNodeRecSeq - tempAckMsg->ackGenerator >150)
                {
                    destNodeRecSeq = tempAckMsg->ackGenerator;
                }
                else
                    destNodeRecSeq = destNodeRecSeq;
            }
            //之所以ackSeq>=ackGenerator因为可能收到重传来的包而发ack也就是收到过去的包发了现在的ack
            if (tempAckMsg->ackSeq >= destNodeRecSeq)
            {
                if (tempAckMsg->ackSeq - destNodeRecSeq >150)//相差大于150
                {
                    if (tempAckMsg->ackSeq >= queueRepeat.front()->seq || (queueRepeat.front()->seq - tempAckMsg->ackSeq >150))
                    {
                        while (queueRepeat.front()->seq != tempAckMsg->ackSeq)
                            queueRepeat.pop();
                        queueRepeat.pop();
                    }
                    m_pHardwareAdapter->transmit(queueRepeat.front());//发送第一个
                }
                else
                {
                    if (tempAckMsg->ackSeq >= queueRepeat.front()->seq || (queueRepeat.front()->seq - tempAckMsg->ackSeq >150))
                    {
                        while (queueRepeat.front()->seq != tempAckMsg->ackSeq)
                            queueRepeat.pop();
                        queueRepeat.pop();
                    }
                }
            }
            else
            {
                if (destNodeRecSeq - tempAckMsg->ackSeq >150)
                {
                    if (tempAckMsg->ackSeq >= queueRepeat.front()->seq || (queueRepeat.front()->seq - tempAckMsg->ackSeq >150))
                    {
                        while (queueRepeat.front()->seq != tempAckMsg->ackSeq)
                            queueRepeat.pop();
                        queueRepeat.pop();
                    }
                }
                else
                {
                    if (tempAckMsg->ackSeq >= queueRepeat.front()->seq || (queueRepeat.front()->seq - tempAckMsg->ackSeq >150))
                    {
                        while (queueRepeat.front()->seq != tempAckMsg->ackSeq)
                            queueRepeat.pop();
                        queueRepeat.pop();
                    }
                    m_pHardwareAdapter->transmit(queueRepeat.front());//发送第一个
                }
            }
            //delete(tempAckMsg);
        }
    }
    pthread_mutex_unlock(&m_pStrategyRepeat->MapAckRec_lock);
}



//用于处理queueRepeat满的情况
void CTransmitNode::transAckReq()
{
    //参数赋值
    CSoftBusMsg* newMsg = new CSoftBusMsg();
    newMsg->type = SOFT_BUS_TYPE_COMMAND;
    newMsg->commandSet = SYSTEM_COMMAND_SET;
    newMsg->commandCode = ACK_REQUEST;
    newMsg->sourceID = sourceID;
    newMsg->destID = destID;
    newMsg->seq = queueRepeat.back()->seq;//返回最后一个元素
    m_pHardwareAdapter->transmit(newMsg);//发送
}

void CTransmitNode::transAckReq(unsigned char seq)
{
    CSoftBusMsg* newMsg = new CSoftBusMsg();
    newMsg->type = SOFT_BUS_TYPE_COMMAND;
    newMsg->commandSet = SYSTEM_COMMAND_SET;
    newMsg->commandCode = ACK_REQUEST;
    newMsg->sourceID = sourceID;
    newMsg->destID = destID;
    newMsg->seq = seq;
    m_pHardwareAdapter->transmit(newMsg);//发送
}
//队列满时意味着发生了不一般的丢包。一般的数据包丢包normal状态就可以处理了，对于发送数据稀疏的情况外边还有
//remainPush线程在推动。
//当发生特殊丢包时 好像只有断层ack丢了会出现接收方沉寂的现象。这时接收方不再有ack回复。只能靠发送方queueRepeat
//满来处理这种特殊情况。这时发送方进入queue_full状态。外部来的传输包暂时被缓存到queueBuffer中，等队列不满时再
//处理。
//队列满的时候意味着对方ack丢失，所以先发送一个ackRequeat,然后等会，然后check.check就意味着检查ack并且重传了。
//如果队列不满了说明收到ack了，进入处理queueBuffer的线程。
void*  dealQueueFull(void*  arg)
{
    CTransmitNode* pCN = (CTransmitNode*)arg;
    pCN->dealQueueFullCount = 0;
    while (1)
    {
        pCN->transAckReq();
        usleep(50000);
        pCN->check();
        if (pCN->queueRepeat.size()<pCN->Queue_Repeat_Max_Len)//尺寸达到最大
        {
            pthread_t tid7;
            pthread_create(&tid7,NULL,dealQueueBuffer,(void *)pCN);//创建处理线程
            printf("\n");
            printf("---- dealQueueFull thread end\n");
            printf("---- dealQueueBuffer thread start\n");
            printf("\n");
        }
        pCN->dealQueueFullCount++;
        if (pCN->dealQueueFullCount > 20)
        {
            printf("link break\n");
            pCN->state = pCN->link_break;//连接中断状态
        }
    }
}



//如果queueBuffer不空，就一直发直到它空。
void*   dealQueueBuffer(void*  arg)
{
    CTransmitNode* pCN = (CTransmitNode*)arg;
    CSoftBusMsg* tempMsg;
    while (1)
    {
        if (pCN->queueBuffer.empty())
        {
            pCN->state = pCN->normal;
            printf("\n");
            printf("---- dealQueueBuffer thread end\n");
            printf("\n");
        }
        else
        {
            tempMsg = pCN->queueBuffer.front();
            pCN->transAndCheck(tempMsg, false);
            pCN->queueBuffer.pop();
            if (pCN->queueRepeat.size() >= pCN->Queue_Repeat_Max_Len)
            {
                printf("\n");
                printf("---- dealQueueFull thread start in dealQueueBuffer thread\n");
                printf("\n");
                //hThread = ::CreateThread(NULL, NULL, dealQueueFull, pCN, 0, &dwThread);
                pthread_t  tid6;
                pthread_create(&tid6,NULL,dealQueueFull,(void *)pCN);
                printf("\n");
                printf("---- dealQueueBuffer thread end\n");
                printf("\n");
            }
        }
    }
}
//如果发送方只发一个包，并且该包在传输过程中丢失，那么接收方虽然收不到该包，但是因为remainPush线程的存在，收到一个ackReq包
//这时接收方根据接收到ackReq后的处理机制会回复一个ack(255,0)(255为ackSeq0为ackGenerator).
//发送方收到ack(255,0)会机械的从queueRepeat中弹包直到把255号包弹出，这时就会出现错误。
//解决方法是CTransmitNode对象创建伊始就发送0号包，用0号包来初始化该TransminNode所对应的接收节点的对应本源节点的接收窗口数组
//让recMsg数组最开始有一个不为NULL的元素以便执行后面的判断算法。恰好，这个过程还有链路探测的功能。
void*  linkDetect(void*  arg)//链路检测线程
{
       //参数初始化
        CTransmitNode* pCN = (CTransmitNode*)arg;
        CSoftBusMsg* newMsg = new CSoftBusMsg();
        newMsg->type = SOFT_BUS_TYPE_COMMAND;
        newMsg->commandSet = SYSTEM_COMMAND_SET;
        newMsg->commandCode = DETECT;
        newMsg->sourceID = pCN->sourceID;
        newMsg->destID = pCN->destID;
        newMsg->seq = 0;
        pCN->m_pHardwareAdapter->transmit(newMsg);//发送
        pCN->transAckReq(0);
        queue<CSoftBusMsg*>* queuePtr;
        CSoftBusMsg* tempAckMsg = new CSoftBusMsg();
        unsigned short retVal = 0xff;
        int checkCount = 0;
        while (1)
        {
            usleep(50000);
            pthread_mutex_lock(&pCN->m_pStrategyRepeat->MapAckRec_lock);//进入临界区
            pCN->m_pStrategyRepeat->recAckIterTemp = pCN->m_pStrategyRepeat->recAck.find(pCN->destID);//查找目的地址
            if (pCN->m_pStrategyRepeat->recAckIterTemp != pCN->m_pStrategyRepeat->recAck.end())//不是最后一个
            {
                queuePtr = pCN->m_pStrategyRepeat->recAckIterTemp->second;
                if (!queuePtr->empty())//队列不空
                {
                    tempAckMsg = queuePtr->front();//取出第一个
                    queuePtr->pop();//删除第一个
                    if (tempAckMsg->ackGenerator == 0 && tempAckMsg->seq == 0)
                    {
                        retVal = 1;
                    }
                }
            }
            pthread_mutex_unlock(&pCN->m_pStrategyRepeat->MapAckRec_lock);//离开临界区
            if (retVal == 1)
                break;
            checkCount++;
            if (checkCount>((pCN->TimeDelay)/50))
            {
                break;
                retVal = 0;
            }
        }
        delete(tempAckMsg);
        tempAckMsg = NULL;
        return(&retVal);
}
