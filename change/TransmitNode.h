#ifndef TRANSMITNODE_H
#define TRANSMITNODE_H

#pragma once
#include "SoftBusMsg.h"
#include "softBusSystem.h"
#include <queue>

class CStrategyRepeat;
class CHardwareAdapter;

using namespace std;

class CTransmitNode
{
public:
    CTransmitNode(CStrategyRepeat* pCR, CHardwareAdapter* pHA, unsigned char src, unsigned char dst);
    ~CTransmitNode(void);
public:
    enum{ normal, queue_full, link_break };
    int state; //节点状态
    unsigned char sourceID;
    unsigned char destID;
    int transmit(CSoftBusMsg *msg, bool ackReqFlag);
public:
    //自用
    CStrategyRepeat* m_pStrategyRepeat;
    CHardwareAdapter* m_pHardwareAdapter;
    queue<CSoftBusMsg*> queueRepeat;  //重传队列
    queue<CSoftBusMsg*> queueBuffer;  //缓冲队列
    int transAndCheck(CSoftBusMsg *msg, bool ackReqFlag);//normal下的处理函数
    friend void *  dealQueueFull(void *  arg);//queue_full下的处理线程，与transAndCheck的执行互斥
    friend void *  dealQueueBuffer(void *  arg);//queue_full下的处理线程，与transAndCheck的执行互斥
    void check();
    void transAckReq();
    void transAckReq(unsigned char seq);
    int dealQueueFullCount;
    friend void *  linkDetect(void *  arg);//只进行一次，在对象建立的时候   链路探测
public:
    unsigned int Queue_Repeat_Max_Len; //发送窗口大小
    unsigned int TimeDelay;            //时延（多长时间接收不到）
private:
    //自用
    unsigned char seq;
    unsigned char destNodeRecSeq;
};

#endif // TRANSMITNODE_H
