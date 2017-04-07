#ifndef STRATEGYREPEAT_H
#define STRATEGYREPEAT_H

#pragma once
#include "SoftBusMsg.h"
#include "softBusSystem.h"
#include <queue>
#include <list>
#include <map>
#include "ReceiveWindow.h"
#include "TransmitNode.h"
#include "CSoftBus.h"
#include "StrategyPriority.h"
#include <pthread.h>
#include <unistd.h>

using namespace std;
class CStrategyPriority;
class CReceiveWindow;
class CTransmitNode;

class CStrategyRepeat
{
public:
    CStrategyRepeat(void);
    ~CStrategyRepeat(void);
public:

public:
    //对发送功能讲，为每一个目的地址建立一个管理节点，在节点内部处理各种对应与该目的地址的事务
    map<unsigned char, CTransmitNode*> TransmitNodeMap;
    map<unsigned char, CTransmitNode*>::iterator TransmitNodeMapIterTemp;//用于map遍历和查找
    //对接收功能讲，为每一个源地址建立一个窗口。
    map<unsigned char, CReceiveWindow*> RecNodeMap;
    map<unsigned char, CReceiveWindow*>::iterator RecNodeMapIterTemp;
    //接收功能接收到ack放入该队列供发送方使用
    map<unsigned char, queue<CSoftBusMsg*>*> recAck;
    map<unsigned char, queue<CSoftBusMsg*>*>::iterator recAckIterTemp;
    //CRITICAL_SECTION csQueueMain;//保护主队列
    pthread_mutex_t  queuemain_lock; //保护主队列
    queue<CSoftBusMsg*> queueMain;
    //CRITICAL_SECTION csMapAckRec;//保护recAck队列
    //CRITICAL_SECTION csTransNode;//保护
    pthread_mutex_t  MapAckRec_lock; //保护recAck队列
    pthread_mutex_t  TransNode_lock;
    CSoftBus* m_pSoftbus;
    CHardwareAdapter* m_pHardwareAdapter;
    CStrategyPriority* m_pStrategyPriority;
public:
    bool initial(CSoftBus* m_softbus); //CHardwareAdapter* pHA
    bool transmit(CSoftBusMsg* msg);
    bool receive(CSoftBusMsg* msg);
    friend void*  repeatControl(void * arg);
    friend void*  remainPush(void * arg);
};



#endif // STRATEGYREPEAT_H
