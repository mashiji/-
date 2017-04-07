#ifndef RECEIVEWINDOW_H
#define RECEIVEWINDOW_H

#pragma once
#include "SoftBusMsg.h"
#include "CSoftBus.h"

#define MSG_RECEIVED_STORE_LEN 50

class CReceiveWindow
{
public:
    CReceiveWindow(CSoftBus* m_psoftbus);
    ~CReceiveWindow(void);
private:
    //内部数据
    CSoftBusMsg* recMsg[256];
    unsigned char winBegin;//窗口起点
    int breakFlag;//该节点是否处于断层状态

public:
    //对外接口
    bool insert(CSoftBusMsg* msg);
    bool isSendAck;
    unsigned char ackSeq;
    bool isHandover;

public:
    int receive_window_len;
};
//接收方什么时候发送ack
//首先，接收方每接收到一个包时便检测接收了这个包是不是应该发送ack，发送怎样的ack
//然后，接收方只有在一下三种条件下才会触发ack
//1.收到ackReq发送ack，移窗到已经确认过的seq的后一个seq
//2.从窗口起始点开始集齐8个（窗长度）发送一个ack，移窗
//3.发现断层，也就是收到这样的序列 4 5 7收到7时马上发送ack并且移动窗口，此时除非接收到ackReq，否则再也
//集不满8个，也再也不会触发断层，因为接收窗本身就在断层状态中
//接收方给每一个发到这里的源节点建立一个窗口并且管理这个窗口。窗口对象的作用就是
//将收到的包插入到窗口中，窗口判断出该不该发ack，ack序号为多少，该不该上交给上层
//然后接收函数利用窗口提供的信息发送ack,和上交给上层。
//ReceiveWindow作为一个工具，为接收函数所用。

//本来设计的是出现断层后接收方没接收到一个包就发送一个断层包，但是这样发送方会受到很多个并且数量不可控个
//断层ack，非常不利于发送方处理，遂后来设计为断层只发一个ack，如果这个ack丢了，则会触发发送方队列满的状态。
//进行特殊处理

#endif // RECEIVEWINDOW_H
