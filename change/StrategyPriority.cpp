#include "StrategyPriority.h"
#include <pthread.h>
#include <unistd.h>

extern bool StrategyRepeat_istrue;
void * queueManage(void * arg);

CStrategyPriority::CStrategyPriority(void)
{
}


CStrategyPriority::~CStrategyPriority(void)
{

}


bool  CStrategyPriority::initial(CSoftBus* m_pSoftBus)//优先级初始化
{
        m_pProtocolAnalysis_priority = m_pSoftBus->m_ProtocolAnalysis;
        m_pStrategyRepeat_priority = m_pSoftBus->m_pTransRecModel->m_pStrategyRepeat;
        m_pHardwareAdapter_Priority = m_pSoftBus->m_pHardwareAdapter;

        pthread_mutex_init(&mutex_queue0,NULL);//初始化队列0
        pthread_mutex_init(&mutex_queue1,NULL);//初始化队列1
        pthread_mutex_init(&mutex_queue2,NULL);//初始化队列2

        pthread_t   tid1;
        pthread_create(&tid1,NULL,queueManage,(void *)this);//创建队列管理线程
        return true;
}


bool CStrategyPriority::transmit(CSoftBusMsg* pMsg)  //发送
{
    //一共是三种帧，每种帧都有不同的优先级
    //优先级需要用户自己去设置，调用的csoftbus的发送函数有priority参数
    switch(pMsg->priority)
    {
        case 0://优先级为0
                {
                    pthread_mutex_lock(&mutex_queue0);//进入队列
                    queue_0.push(pMsg);
                    pthread_mutex_unlock(&mutex_queue0);//离开队列
                    break;
                }
                case 1://优先级为1
                {
                    pthread_mutex_lock(&mutex_queue1);//进入队列
                    queue_1.push(pMsg);
                    pthread_mutex_unlock(&mutex_queue1);//离开队列
                    break;
                }
                case 2://优先级为2
                {
                    pthread_mutex_lock(&mutex_queue2);//进入队列
                    queue_2.push(pMsg);
                    pthread_mutex_unlock(&mutex_queue2);//离开队列
                    break;
                }
                default:
                {
                    printf("无法找到消息优先级所属的队列，优先级设置错误\n");
                    return false;
                }
    }
    return true;
}




void * queueManage(void * arg)//队列管理
{
        CStrategyPriority* pSP = new CStrategyPriority();
        pSP = (CStrategyPriority*)arg;
        CSoftBusMsg* msgTemp = new CSoftBusMsg();

       while(1)//一直执行
       {
              pthread_mutex_lock(&pSP->mutex_queue0);//进入队列0
              if(pSP->queue_0.empty())//如果队列0为空
                    pthread_mutex_unlock(&pSP->mutex_queue0);//离开队列
              else if(!pSP->queue_0.empty())//如果队列不为空
              {
                    msgTemp = pSP->queue_0.front();
                    pSP->queue_0.pop();
                    pthread_mutex_unlock(&pSP->mutex_queue0);

                    if(StrategyRepeat_istrue)//有重传
                            pSP->m_pStrategyRepeat_priority->transmit(msgTemp);
                    else//没有重传
                            pSP->m_pHardwareAdapter_Priority->transmit(msgTemp);
                    continue;
              }

              pthread_mutex_lock(&pSP->mutex_queue1);//进入队列1
              if(pSP->queue_1.empty())//如果队列1为空
                    pthread_mutex_unlock(&pSP->mutex_queue1);//离开队列
              else if(!pSP->queue_1.empty())//如果队列不为空
              {
                    msgTemp = pSP->queue_1.front();
                    pSP->queue_1.pop();
                    pthread_mutex_unlock(&pSP->mutex_queue1);
                    if(StrategyRepeat_istrue)//有重传
                           pSP->m_pStrategyRepeat_priority->transmit(msgTemp);
                   else//没有重传
                           pSP->m_pHardwareAdapter_Priority->transmit(msgTemp);
                    continue;
              }

              pthread_mutex_lock(&pSP->mutex_queue2);//进入队列2
              if(pSP->queue_2.empty())//如果队列2为空
                    pthread_mutex_unlock(&pSP->mutex_queue2);//离开队列
              else if(!pSP->queue_2.empty())//如果队列不为空
              {
                    msgTemp = pSP->queue_2.front();
                    pSP->queue_2.pop();
                    pthread_mutex_unlock(&pSP->mutex_queue2);

                    if(StrategyRepeat_istrue)//有重传
                            pSP->m_pStrategyRepeat_priority->transmit(msgTemp);
                    else//没有重传
                            pSP->m_pHardwareAdapter_Priority->transmit(msgTemp);
                    continue;
              }
       }
}



//优先级模块的接收函数
bool  CStrategyPriority::receive(CSoftBusMsg* msg)
{
    //调用协议功能解析模块的接收函数
    if(true == m_pProtocolAnalysis_priority->receive(msg))
        return true;
    else
        return false;
}
