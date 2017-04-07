#ifndef STRATEGYPRIORITY_H
#define STRATEGYPRIORITY_H

#pragma once
#include "SoftBusMsg.h"
#include <queue>
#include <stdio.h>
#include "HardwareAdapter.h"
#include <pthread.h>
#include <unistd.h>
#include "CSoftBus.h"
#include "StrategyRepeat.h"
#include "ProtocolAnalysis.h"

using namespace std;
class CStrategyRepeat;
class CProtocolAnalysis;

class CStrategyPriority
{
public:
    CStrategyPriority(void);
    ~CStrategyPriority(void);
public:
    bool initial(CSoftBus* m_pSoftBus);
    bool transmit(CSoftBusMsg* msg);
    bool receive(CSoftBusMsg* msg);
    friend void * queueManage(void * arg);
public:
    CStrategyRepeat*   m_pStrategyRepeat_priority;
    CHardwareAdapter*  m_pHardwareAdapter_Priority;
    CProtocolAnalysis* m_pProtocolAnalysis_priority;

public:
    queue<CSoftBusMsg*> queue_0;
    queue<CSoftBusMsg*> queue_1;
    queue<CSoftBusMsg*> queue_2;

    pthread_mutex_t     mutex_queue0; //保护命令帧队列
    pthread_mutex_t     mutex_queue1;//保护变量帧队列
    pthread_mutex_t     mutex_queue2;//保护数据块传输帧队列
};


#endif // STRATEGYPRIORITY_H
