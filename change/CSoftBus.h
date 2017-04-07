#ifndef CSOFTBUS_H
#define CSOFTBUS_H

#pragma once
#include "XMLAnalyse.h"
#include "SoftBusMsg.h"
#include "ProtocolAnalysis.h"
#include "TransRecModel.h"
#include "HardwareAdapter.h"
#include <queue>
#include <pthread.h>
#include <unistd.h>
#include<map>

class CXMLAnalyse;
class CProtocolAnalysis;
class CTransRecModel;
class CHardwareAdapter;

#define SOFT_BUS_TYPE_COMMAND 0  //minglingzhen
#define SOFT_BUS_TYPE_VARIABLE 1     //shuju zhen
#define SOFT_BUS_TYPE_DATABLOCK 2 //piliang chuanshu zhen

#define SOFT_BUS_STRATEGY_PRIORITY

//系统命令集和命令码
#define SYSTEM_COMMAND_SET 1
#define ACK 1
#define ACK_REQUEST 2
#define DETECT 3
#define SINGLE_DETECT 4
#define SINGLE_DETECT_ACK 5

using  namespace std;

class CSoftBus
{
public:
    CSoftBus(void);
    ~CSoftBus(void);
public:
    CXMLAnalyse * m_pXMLAnalyse;
    CProtocolAnalysis* m_ProtocolAnalysis;
    CTransRecModel * m_pTransRecModel;
    CHardwareAdapter* m_pHardwareAdapter;
public:
    //给用户的接口函数
    bool initial(char* xmlPathName);
    bool reset();
    //用户发送接口函数
    bool commandSend(char* para, int paraLen, int commandCode );
    bool variableSend(char * data, int dataLen, int templetID);
    bool DatablockSend(char * data, int dataLen, bool moreFragment, int offset, char destID, char priority);
    bool SingleLinkDetect(char srcID, char destID);
    //用户接收接口函数  如果无数据到来则阻塞，有数据到来则返回
    bool softBusReceive(); //CSoftBusMsg* msg, char &srcID, int &type, int &templetID, int &commandSet, int &commandCode
//	bool commandParaRec(char* para, int &dataLen);
//	bool variableRec(char* data, int &dataLen);
//	bool blockDataRec(char *data, int dataLen, bool &moreFragment, int &offset);

public:
    unsigned int    SendWindow;           //发送窗口大小
    unsigned int    ACKWindow;            //接收窗口大小
    unsigned int    TimeDelay;            //时延（多长时间接收不到）
    //CRITICAL_SECTION cs_queue_receive;
    pthread_mutex_t  queue_receive_lock;
    queue<CSoftBusMsg*> receive_queue;  //接收队列
    bool   SingleLinkDetect_IsThrough;
public:
    char* receive_data;
    int   receive_data_len;
    bool  receive_data_break; //接收数据中断标志
    unsigned char receive_data_type;
    unsigned char msg_type;
    unsigned int  order_code;
    unsigned int  vari_code;
    bool  is_packet_miss;
public:
    //命令帧关联map
    map<unsigned char,OrderFrameMapStruct*> OrderFrameLink;
    map<unsigned char,OrderFrameMapStruct*> ::iterator OrderFrameLinkIterator;
    //数据帧关联map
    map<unsigned char,VariFrameMapStruct*>  VariFrameLink;
    map<unsigned char,VariFrameMapStruct*> ::iterator VariFrameLinkIterator;
    //批量传输帧关联map
    map<unsigned char, unsigned char > DataBlockLink;
    map<unsigned char, unsigned char> ::iterator  DataBlockLinkIterator;
};

#endif // CSOFTBUS_H
