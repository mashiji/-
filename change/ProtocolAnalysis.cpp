#include "ProtocolAnalysis.h"
#include "FrameHead.h"
#include "SoftBusMsg.h"
#include <iostream>
#include <stdlib.h>
#include <cstring>

extern  CommandInfo     commandInfo;
extern  VariInfo        variInfo;
extern  DatablockInfo   datablockInfo;

using namespace std;


CProtocolAnalysis::CProtocolAnalysis(void)
{
}


CProtocolAnalysis::~CProtocolAnalysis(void)
{
}


bool CProtocolAnalysis::initial(CSoftBus* m_psoftbus)//初始化
{
    m_pSoftBus_ProtocolAnalysis = m_psoftbus;
    commonHeader_gongneng = NULL;
    commonHeader_shoufa = NULL;
    m_pTransRecModel_ProAnalyse = m_psoftbus->m_pTransRecModel;
    return true;
}

//进行封帧的操作并调用收发模块的发送函数
bool CProtocolAnalysis::functionFrameSend(CSoftBusMsg* msg)
{
    switch(msg->type)
    {
        case SOFT_BUS_TYPE_COMMAND: //命令帧
            {
            //对各参数进行赋值
            char* dataTemp = (char*)malloc(sizeof(CommandInfo) + msg->paraLen + 2);
            msg->pcommandInfo = (CommandInfo*) dataTemp;
            msg->pcommandInfo->Version = commandInfo.Version;
            msg->pcommandInfo->FrameType = commandInfo.FrameType;
            msg->pcommandInfo->retain_1 = commandInfo.retain_1;
            msg->pcommandInfo->CID = commandInfo.CID;
            msg->pcommandInfo->SN = commandInfo.SN;
            msg->pcommandInfo->Priority = msg->priority;
            msg->pcommandInfo->SA = commandInfo.SA;
            msg->pcommandInfo->DA = commandInfo.DA;
            msg->pcommandInfo->retain_2 = commandInfo.retain_2;
            msg->pcommandInfo->CL = commandInfo.CL;
            msg->pcommandInfo->CS = commandInfo.CS;
            msg->pcommandInfo->CPL = commandInfo.CPL;
            memcpy(dataTemp + sizeof(CommandInfo), &msg->commandCode,2);
            memcpy(dataTemp + sizeof(CommandInfo) +2,msg->data,msg->paraLen);
            //对命令帧进行封帧完毕
            delete(msg->data);
            msg->data = dataTemp;
            dataTemp = NULL;
            msg->dataLen = msg->paraLen + 2 + sizeof(CommandInfo);  //paraLen为命令帧专用,表示命令参数长度  此时msg里的dataLen变量为命令帧的总长度

            m_pTransRecModel_ProAnalyse->transmit(msg);
            return true;
            }
    case SOFT_BUS_TYPE_VARIABLE://数据帧
            {
                char* dataTemp = (char *)malloc(msg->dataLen * sizeof(char) + sizeof(VariInfo) + 1);
                msg->pvariInfo = (VariInfo*) dataTemp;
                msg->pvariInfo->Version = variInfo.Version;
                msg->pvariInfo->FrameType = variInfo.FrameType;
                msg->pvariInfo->retain_1 = variInfo.retain_1;
                msg->pvariInfo->CID = variInfo.CID;
                msg->pvariInfo->SN = variInfo.SN;
                msg->pvariInfo->Priority = variInfo.Priority;
                msg->pvariInfo->SA = variInfo.SA;
                msg->pvariInfo->DA = variInfo.DA;
                msg->pvariInfo->retain_2 = variInfo.retain_2;
                msg->pvariInfo->vari_len = msg->dataLen;
                memcpy(dataTemp + sizeof(VariInfo),(char*)&(msg->templetID),1);
                memcpy(dataTemp + sizeof(VariInfo) + 1 , msg->data, msg->dataLen);
                //对变量帧进行封帧完
                delete(msg->data);
                msg->data = dataTemp;
                dataTemp = NULL;
                msg->dataLen = msg->dataLen + sizeof(VariInfo) + 1;
                m_pTransRecModel_ProAnalyse->transmit(msg);
                return true;
            }
            case SOFT_BUS_TYPE_DATABLOCK://批量传输帧
            {
                char* dataTemp = (char *)malloc(msg->dataLen * sizeof(char) + sizeof(DatablockInfo));
                //对各参数进行赋值
                msg->pdatablockInfo = (DatablockInfo *) dataTemp;
                msg->pdatablockInfo->Version = datablockInfo.Version;
                msg->pdatablockInfo->FrameType = datablockInfo.FrameType;
                msg->pdatablockInfo->retain_1 = datablockInfo.retain_1;
                msg->pdatablockInfo->CID = datablockInfo.CID;
                msg->pdatablockInfo->SN = datablockInfo.SN;
                msg->pdatablockInfo->Priority = datablockInfo.Priority;
                msg->pdatablockInfo->MF = datablockInfo.MF;
                msg->pdatablockInfo->retain_2 = datablockInfo.retain_2;
                msg->pdatablockInfo->OFFSET = datablockInfo.OFFSET;
                msg->pdatablockInfo->SA = datablockInfo.SA;
                msg->pdatablockInfo->DA = datablockInfo.DA;
                msg->pdatablockInfo->datablock_len = msg->dataLen;
                memcpy(dataTemp + sizeof(DatablockInfo), msg->data, msg->dataLen);
                 //对数据块传输帧进行封帧完毕
                delete(msg->data);
                msg->data = dataTemp;
                dataTemp = NULL;
                msg->dataLen = msg->dataLen + sizeof(datablockInfo);
                m_pTransRecModel_ProAnalyse->transmit(msg);
                return true;
            }
               default:
                        return false;
    }
              return false;
}



bool CProtocolAnalysis::receive(CSoftBusMsg* msg)//接收函数
{
    pthread_mutex_lock(&m_pSoftBus_ProtocolAnalysis->queue_receive_lock);//进入临界区
    m_pSoftBus_ProtocolAnalysis->receive_queue.push(msg);
    pthread_mutex_unlock(&m_pSoftBus_ProtocolAnalysis->queue_receive_lock);//离开临界区


    return true;
}
















































