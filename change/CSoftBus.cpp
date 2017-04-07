#include "CSoftBus.h"
#include "SoftBusMsg.h"
#include "softBusSystem.h"
#include "XMLAnalyse.h"
#include "ProtocolAnalysis.h"
#include "FrameHead.h"
#include"StrategyPriority.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include<stdlib.h>

using namespace std;

void *  ReceiveAll (void *  arg);
void *  SingleLinkDetect_Thread(void *  lparam);
extern bool StrategyRepeat_istrue;
extern  CommandInfo     commandInfo;
extern  VariInfo        variInfo;
extern  DatablockInfo   datablockInfo;

extern bool HardwareAdapter_Ethernet;
extern bool HardwareAdapter_CAN;
extern bool HardwareAdapter_RS422;

CSoftBus::CSoftBus(void)
{
    m_pXMLAnalyse = NULL;//初始化指针为空
    m_ProtocolAnalysis = NULL;
    m_pTransRecModel = NULL;
    m_pHardwareAdapter = NULL;
}

CSoftBus::~CSoftBus(void)
{
}

bool CSoftBus::initial(char* xmlPathName)
{
    pthread_mutex_init(&queue_receive_lock,NULL);//初始化接收队列
    receive_data_break = false;
    receive_data = NULL;
    m_pXMLAnalyse = new CXMLAnalyse();//为需要用到的类申请内存
    m_ProtocolAnalysis = new CProtocolAnalysis();
    m_pTransRecModel = new CTransRecModel();
    m_pHardwareAdapter = new CHardwareAdapter(m_pTransRecModel);

    if (false == m_pXMLAnalyse->initial(this))//解析导入的XML文件
        return false;
    m_pXMLAnalyse->XMLAnalyse(xmlPathName);
   //各模块初始化
    if (false == m_ProtocolAnalysis->initial(this))
        return false;

    if (false == m_pTransRecModel->initial(this))
        return false;


    if (false == m_pHardwareAdapter->initial(this))
        return false;

    return true;
}

bool CSoftBus::reset()
{
    printf("System Reset ...");
    //CSoftBus::~CSoftBus();
    return true;
}

//minglingzhen  fa song
bool CSoftBus::commandSend(char* para, int paraLen, int commandCode )//命令帧发送
{
    CSoftBusMsg * p_Msg = new CSoftBusMsg();//为发送帧申请内存
    p_Msg->type = SOFT_BUS_TYPE_COMMAND ;//类型为命令帧
    p_Msg->commandCode = commandCode;//命令码
    p_Msg->paraLen = paraLen;//命令参数长
    //进行码，命令集，优先级和目的地址的对应
    OrderFrameMapStruct* orderframe_mapstruct = (OrderFrameMapStruct*)malloc(sizeof(OrderFrameMapStruct));
    OrderFrameLinkIterator = OrderFrameLink.find(commandCode);

        if(OrderFrameLinkIterator != OrderFrameLink.end())//填写了正确的命令码
        {
            orderframe_mapstruct = OrderFrameLinkIterator->second;
            p_Msg->commandSet = orderframe_mapstruct->commandSet;
            p_Msg->priority = orderframe_mapstruct->priority;
            commandInfo.SA = orderframe_mapstruct->srcID;
            p_Msg->sourceID = orderframe_mapstruct->srcID;
            commandInfo.CL = 2;
            commandInfo.CPL = paraLen;
            commandInfo.CS = orderframe_mapstruct->commandSet;
            commandInfo.DA = orderframe_mapstruct->destID;
            p_Msg->destID = orderframe_mapstruct->destID;
        }

        if (para == NULL)
            {
                p_Msg->dataCopy(orderframe_mapstruct->data, orderframe_mapstruct->datalen);
                commandInfo.CPL = orderframe_mapstruct->datalen;
                p_Msg->paraLen =  orderframe_mapstruct->datalen;//命令参数长
                orderframe_mapstruct=NULL;
            }
            else
            {
                p_Msg->dataCopy(para, paraLen);//对数据区域进行赋值
            }


    if(true == m_ProtocolAnalysis->functionFrameSend(p_Msg))//功能帧发送
        return true;
    else
        return false;
}

bool CSoftBus::variableSend(char * data, int dataLen, int templetID)//数据帧发送
{
    CSoftBusMsg * p_Msg = new CSoftBusMsg();
    p_Msg->type = SOFT_BUS_TYPE_VARIABLE;//类型为数据帧
    p_Msg->templetID = templetID;//数据码
    p_Msg->dataLen = dataLen;//数据长
    p_Msg->templetID = templetID;
    VariFrameMapStruct * variframe_mapstruct = (VariFrameMapStruct *)malloc(sizeof(VariFrameMapStruct));//对应
    VariFrameLinkIterator = VariFrameLink.find(templetID);

        if(VariFrameLinkIterator != VariFrameLink.end())//填写了正确的数据码
        {
            variframe_mapstruct = VariFrameLinkIterator->second;
            variInfo.SA = variframe_mapstruct->srcID;
            variInfo.DA = variframe_mapstruct->destID;
            variInfo.Priority = variframe_mapstruct->priority;
            p_Msg->priority = variframe_mapstruct->priority;
            p_Msg->sourceID = variframe_mapstruct->srcID;
            p_Msg->destID = variframe_mapstruct->destID;
        }

        else//填写了错误的数据码
        {
            printf("发送的数据码与XML配置文件不匹配\n");
            return false;
        }

        if (data == NULL)
            {
                p_Msg->dataCopy(variframe_mapstruct->data, variframe_mapstruct->datalen);
                p_Msg->dataLen = variframe_mapstruct->datalen;
                variframe_mapstruct= NULL;
            }
            else
            {
                p_Msg->dataCopy(data, dataLen);//对数据区域进行赋值
            }
    if (true == m_ProtocolAnalysis->functionFrameSend(p_Msg))
          return true;
    else
          return false;
}

bool CSoftBus::DatablockSend(char * data, int dataLen, bool moreFragment, int offset, char destID, char priority)//批量传输帧
{
    CSoftBusMsg * p_Msg = new CSoftBusMsg();
    p_Msg->type = SOFT_BUS_TYPE_DATABLOCK;//类型为批量传输帧
    p_Msg->moreFragment = moreFragment;//多片
    p_Msg->offset = offset;//偏移量
    p_Msg->dataLen = dataLen;//参数长
    datablockInfo.MF = moreFragment;//多片
    datablockInfo.OFFSET = offset;//偏移量

    DatablockMapStruct * datablock_mapstruct = (DatablockMapStruct*) malloc(sizeof(DatablockMapStruct));
    DataBlockLinkIterator = DataBlockLink.find(destID);

    if(DataBlockLinkIterator != DataBlockLink.end())
    {
            datablockInfo.SA = DataBlockLinkIterator->second;
            p_Msg->sourceID = DataBlockLinkIterator->second;
    }
    else
    {
            printf("发送的目的地址与XML配置文件不匹配\n");
            return false;
    }

    datablockInfo.DA = destID;//目的地址
    p_Msg->destID = destID;
    datablockInfo.Priority = priority;//优先级
    p_Msg->priority = priority;//优先级
    p_Msg->dataCopy(data, dataLen);//数据域赋值
    if (true == m_ProtocolAnalysis->functionFrameSend(p_Msg))//功能帧发送
        return true;
    else
        return false;
}
//2016.12.02
bool CSoftBus::SingleLinkDetect(char srcID, char destID)
{
    //构造一个独立的链路检测包,此包的优先级最高，值为0；为命令帧，命令集为SYSTEM_COMMAND_SET,命令码为4;
    //接收方返回的链路检测ACK包，此包的优先级最高，值为0；为命令帧，命令集为SYSTEM_COMMAND_SET，命令码为5;
    //每次调用单独的链路检测函数时，开启一个线程来检测有没有接收到链路检测ACK包，该线程最多持续时间为1s，若检测到有链路检测ACK包返回，则该线程结束，函数返回true
    //就像正常发送普通包一样发出去
    SingleLinkDetect_IsThrough = false;  //每次发送前先将链路检测标志变量置为false
    CSoftBusMsg * linkdetect_msg = new CSoftBusMsg();
    linkdetect_msg->type = SOFT_BUS_TYPE_COMMAND;
    linkdetect_msg->commandSet = SYSTEM_COMMAND_SET;
    linkdetect_msg->commandCode = SINGLE_DETECT;
    linkdetect_msg->sourceID = srcID;
    linkdetect_msg->destID = destID;
    linkdetect_msg->priority = 0;
    linkdetect_msg->paraLen = 0;
    m_pHardwareAdapter->transmit(linkdetect_msg);
    int *  ret_val = NULL;
    pthread_t tid10;
    pthread_create(&tid10,NULL,SingleLinkDetect_Thread,(void *)this);//开启监听链路检测ACK包线程
    pthread_join(tid10,(void**)&ret_val);
    if(*ret_val)
    {
        printf("链路检测成功\n");
        return true;
    }
    else
    {
        printf("链路检测失败\n");
        return false;
    }
}


void *  SingleLinkDetect_Thread(void *  lparam)
{
    int  linkdetect_count = 0;
    static int retvalue;
    CSoftBus * m_softbus = (CSoftBus *) lparam;
    while(1)
    {
        usleep(50000);
        if(linkdetect_count >=20)
        {
            retvalue=0;
            pthread_exit((void*)&retvalue);

        }
        else if(m_softbus->SingleLinkDetect_IsThrough)
        {
            retvalue=1;
            pthread_exit((void*)&retvalue);
        }
        else
        {
            linkdetect_count++;
            continue;
        }
    }

}




//监听线程
bool CSoftBus::softBusReceive() // CSoftBusMsg* msg, char &srcID, int &type, int &templetID, int &commandSet, int &commandCode
{
    printf("----接收线程启动----\n");
    pthread_t tid9;
    pthread_create(&tid9,NULL,ReceiveAll,(void *)this);//开启监听线程
    return true;
}

 void *  ReceiveAll(void *  arg)//监听线程定义
{
    CSoftBus * m_softbus = (CSoftBus*)arg;
    CSoftBusMsg* msg_temp = new CSoftBusMsg();
    while (1)//一直执行
    {
        pthread_mutex_lock(&m_softbus->queue_receive_lock);
        if (!m_softbus->receive_queue.empty() && m_softbus->receive_data_break == false)//如果接收队列不为空
        {
            msg_temp = m_softbus->receive_queue.front();
            m_softbus->receive_queue.pop();
            pthread_mutex_unlock(&m_softbus->queue_receive_lock);
            switch (msg_temp->type)
            {
                case SOFT_BUS_TYPE_COMMAND://类型为命令帧
                {
                    m_softbus->receive_data_type=SOFT_BUS_TYPE_COMMAND;
                    if(StrategyRepeat_istrue)//有重传
                    {
                    //#if defined SOFT_BUS_STRATEGY_REPEAT
                    //printf("OrderFrame received ---- seq : %d ---- orderset: %d ---- ordercode : %d \n", msg_temp->seq, msg_temp->commandSet, msg_temp->commandCode);
                    //#endif
                        if(msg_temp->commandSet == SYSTEM_COMMAND_SET && msg_temp->commandCode == SINGLE_DETECT)
                                                {
                                                    CSoftBusMsg * msg_singledetect_ack = new CSoftBusMsg();
                                                    msg_singledetect_ack->type = SOFT_BUS_TYPE_COMMAND;
                                                    msg_singledetect_ack->commandSet = SYSTEM_COMMAND_SET;
                                                    msg_singledetect_ack->commandCode = SINGLE_DETECT_ACK;
                                                    msg_singledetect_ack->sourceID = msg_temp->destID;
                                                    msg_singledetect_ack->destID = msg_temp->sourceID;
                                                    msg_singledetect_ack->priority = 0;
                                                    msg_singledetect_ack->paraLen = 0;                                                   
                                                    m_softbus->m_pHardwareAdapter->transmit(msg_singledetect_ack);
                                                    continue;
                                                }

                                                if(msg_temp->commandSet == SYSTEM_COMMAND_SET && msg_temp->commandCode == SINGLE_DETECT_ACK)
                                                {
                                                    m_softbus->SingleLinkDetect_IsThrough = true;
                                                    continue;
                                                }
                       //用接收到的数据对m_softbus中的各参数进行赋值
                        m_softbus->msg_type=0;
                        m_softbus->order_code = msg_temp->commandCode;
                        m_softbus->receive_data = msg_temp->data;
                        m_softbus->receive_data_len = msg_temp->paraLen;//->dataLen - sizeof(SendRev_CRC_Retrans_OD) - 2;
                        m_softbus->receive_data_break = true;
                    }
                    else//没有重传
                    {
                    //#if !defined SOFT_BUS_STRATEGY_REPEAT
                    //printf("OrderFrame received ---- orderset: %d ---- ordercode : %d \n", msg_temp->commandSet, msg_temp->commandCode);
                    //#endif
                        if(msg_temp->commandSet == SYSTEM_COMMAND_SET && msg_temp->commandCode == SINGLE_DETECT)
                                                {
                                                    CSoftBusMsg * msg_singledetect_ack = new CSoftBusMsg();
                                                    msg_singledetect_ack->type = SOFT_BUS_TYPE_COMMAND;
                                                    msg_singledetect_ack->commandSet = SYSTEM_COMMAND_SET;
                                                    msg_singledetect_ack->commandCode = SINGLE_DETECT_ACK;
                                                    msg_singledetect_ack->sourceID = msg_temp->destID;
                                                    msg_singledetect_ack->destID = msg_temp->sourceID;
                                                    msg_singledetect_ack->priority = 0;
                                                    msg_singledetect_ack->paraLen = 0;-
                                                    m_softbus->m_pHardwareAdapter->transmit(msg_singledetect_ack);
                                                    continue;
                                                }

                                                if(msg_temp->commandSet == SYSTEM_COMMAND_SET && msg_temp->commandCode == SINGLE_DETECT_ACK)
                                                {
                                                    m_softbus->SingleLinkDetect_IsThrough = true;
                                                    continue;
                                                }
                        //用接收到的数据对m_softbus中的各参数进行赋值
                        m_softbus->msg_type=0;
                        m_softbus->order_code = msg_temp->commandCode;
                        m_softbus->receive_data = msg_temp->data;
                        m_softbus->receive_data_len = msg_temp->paraLen;//->dataLen - sizeof(SendRev_CRC_OD) - 2;
                        m_softbus->receive_data_break = true;
                    }
                    break;
                }
                case SOFT_BUS_TYPE_VARIABLE://类型为数据帧
                {
                    if(StrategyRepeat_istrue)//有重传
                    {
                    //#if defined SOFT_BUS_STRATEGY_REPEAT
                    //printf("VariFrame received ---- seq : %d ---- VariTempNum: %d \n", msg_temp->seq, msg_temp->templetID);
                    //#endif
                        //用接收到的数据对m_softbus中的各参数进行赋值
                        m_softbus->msg_type=1;
                        m_softbus->vari_code = msg_temp->templetID;
                        m_softbus->receive_data = msg_temp->data;
                        m_softbus->receive_data_len = msg_temp->dataLen;
                        m_softbus->receive_data_break = true;
                    }
                    else//没有重传
                    {
                    //#if !defined SOFT_BUS_STRATEGY_REPEAT
                    //printf("VariFrame received ---- VariTempNum: %d \n", msg_temp->templetID);
                    //#endif
                        //用接收到的数据对m_softbus中的各参数进行赋值
                        m_softbus->msg_type=1;
                        m_softbus->vari_code = msg_temp->templetID;
                        m_softbus->receive_data = msg_temp->data;
                        m_softbus->receive_data_len = msg_temp->dataLen;
                        m_softbus->receive_data_break = true;
                    }
                    break;
                }
                case SOFT_BUS_TYPE_DATABLOCK://类型为批量传输帧
                {
                    if(StrategyRepeat_istrue)//有重传
                    {
                    //#if defined SOFT_BUS_STRATEGY_REPEAT
                    //printf("DataBlockFrame received ---- seq : %d \n", msg_temp->seq);
                    //#endif
                        //用接收到的数据对m_softbus中的各参数进行赋值
                        m_softbus->msg_type=2;
                        m_softbus->receive_data = msg_temp->data;
                        m_softbus->receive_data_len = msg_temp->dataLen;
                        m_softbus->receive_data_break = true;
                    }
                    else//没有重传
                    {
                    //#if !defined SOFT_BUS_STRATEGY_REPEAT
                    //printf("DataBlockFrame received \n");
                    //#endif
                        //用接收到的数据对m_softbus中的各参数进行赋值
                        m_softbus->msg_type=2;
                        m_softbus->receive_data = msg_temp->data;
                        m_softbus->receive_data_len = msg_temp->dataLen;
                        m_softbus->receive_data_break = true;
                    }
                    break;
                }
            }
            continue;
        }
        else
        {
            pthread_mutex_unlock(&m_softbus->queue_receive_lock);//离开临界区
            continue;
        }
    }
    return 0;
}



































