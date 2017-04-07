#include "HardwareAdapter.h"
#include "TransRecModel.h"
#include <string.h>
#include <fstream>
#include <stdlib.h>
#include <cstring>
#include "FrameHead.h"
extern bool StrategyRepeat_istrue;
extern bool HardwareAdapter_Ethernet;
extern bool HardwareAdapter_CAN;
extern bool HardwareAdapter_RS422;
extern  Project_Inf     projectinf;
CHardwareAdapter::CHardwareAdapter(CTransRecModel* p)
{
    m_pTransRecModel = p;
    flag = true;
}


CHardwareAdapter::~CHardwareAdapter(void)
{
}


////两点之间做测试
bool CHardwareAdapter::initial(CSoftBus* m_csoftbus)//初始化
{
    pthread_mutex_init(&QueueChannel_lock,NULL);//初始化临界区
    m_pSoftbus = m_csoftbus;
    m_pStrategyPriority = m_csoftbus->m_pTransRecModel->m_pStrategyPriority;
    m_pTransRecModel = m_csoftbus->m_pTransRecModel;
    crc_check = new CCheckSum();
    if(HardwareAdapter_Ethernet)//以太网初始化
    {
            m_pHardwareEthernet = new CHardwareEthernet();
            m_pHardwareEthernet->ethernetinitial(this);
    }
    else if(HardwareAdapter_CAN)//CAN初始化
    {
           m_pHardwareCANbus = new CHardwareCANbus();
           m_pHardwareCANbus->canbus_initial(this);
    }
    else  if(HardwareAdapter_RS422)//422初始化
    {
           m_pHardwareSerial = new  CHardwareSerial();
           m_pHardwareSerial->serial_initial(this);
    }
    return true;
}



//丢一个测试
//bool transOnce = true;
ofstream SaveFile("fasongshuju.txt");

bool CHardwareAdapter::transmit(CSoftBusMsg* msg)//传输函数
{
    if (msg->seq==6 && m_pSoftbus->is_packet_miss==true)
        {
            m_pSoftbus->is_packet_miss=false;
            return false;
        }
        //::EnterCriticalSection(&csQueueChannel);
        //在写入queueChannel队列里之前，先要将功能帧简化为发送帧，并进行校验和计算
        //简化功能帧，将之变为收发帧，并进行校验和的计算

        CSoftBusMsg  send_msg;
        send_msg.type = msg->type;
        send_msg.dataCopy(msg->data,msg->dataLen);
        send_msg.dataLen = msg->dataLen;
        send_msg.commandSet = msg->commandSet;
        send_msg.commandCode = msg->commandCode;
        send_msg.ackSeq = msg->ackSeq;
        send_msg.sourceID = msg->sourceID;
        send_msg.destID = msg->destID;
        send_msg.seq = msg->seq;
        send_msg.ackGenerator = msg->ackGenerator;
        send_msg.ackRequestSeq =msg->ackRequestSeq;
        send_msg.checkSum =msg->checkSum;
        send_msg.device = msg->device;
        send_msg.pcommandInfo =msg->pcommandInfo;
        send_msg.pdatablockInfo =msg->pdatablockInfo;
        send_msg.pvariInfo =msg->pvariInfo;
        send_msg.priority =msg->priority;
        send_msg.moreFragment = msg->moreFragment;
        send_msg.templetID = msg->templetID;
        send_msg.offset =msg->offset;
        send_msg.paraLen =msg->paraLen;
        send_msg.version =msg->version;

        switch (send_msg.type)//对类型进行区分
        {
            case SOFT_BUS_TYPE_COMMAND://发送的是命令帧
            {
                //选择了重传策略，则收发帧中会有序列号
                if(StrategyRepeat_istrue)
                {
                //#if (defined SOFT_BUS_STRATEGY_REPEAT)
                    if (send_msg.commandSet == SYSTEM_COMMAND_SET && (send_msg.commandCode == ACK ))
                    {
                        //如果是ACK包，msg的ackseq和ackGenerator都需要发送过去
                        //ackseq占用帧头,ackGenerator占用数据域的一个字节
                        send_msg.paraLen=1;
                        send_msg.dataLen=10;
                        crc_check->n_checksum = 0;
                        char* data_temp_od_ack = (char*)malloc(sizeof(SendRev_CRC_Retrans_OD)+2+1);
                        memset(data_temp_od_ack,0,sizeof(SendRev_CRC_Retrans_OD)+2+1);
                        memset(data_temp_od_ack+sizeof(SendRev_CRC_Retrans_OD)+2,send_msg.ackGenerator,1);
                        sendrev_crc_retrans_OD = (SendRev_CRC_Retrans_OD*)data_temp_od_ack;
                        CommandCode* commandcode =(CommandCode*)(data_temp_od_ack + sizeof(SendRev_CRC_Retrans_OD));
                        commandcode->commandCode = send_msg.commandCode;//msg->commandCode;
                        sendrev_crc_retrans_OD->frametype = SOFT_BUS_TYPE_COMMAND;
                        sendrev_crc_retrans_OD->CS = SYSTEM_COMMAND_SET;
                        sendrev_crc_retrans_OD->sequence = send_msg.ackSeq;
                        sendrev_crc_retrans_OD->order_para_len = 1;
                        sendrev_crc_retrans_OD->srcID = send_msg.sourceID;
                        sendrev_crc_retrans_OD->destID = send_msg.destID;
                        sendrev_crc_retrans_OD->checksum = crc_check->n_checksum;
                        send_msg.data = data_temp_od_ack;
                        data_temp_od_ack = NULL;
                        //计算校验和
                        crc_check->n_checksum = crc_check->GenerateCheckSum(send_msg.data,sizeof(SendRev_CRC_Retrans_OD)+2+1);
                        memcpy(send_msg.data + sizeof(SendRev_CRC_Retrans_OD)-2,&crc_check->n_checksum,2);
                        break;
                    }
                    else if(send_msg.commandSet == SYSTEM_COMMAND_SET && (send_msg.commandCode == ACK_REQUEST || send_msg.commandCode == DETECT ||send_msg.commandCode == SINGLE_DETECT ||send_msg.commandCode == SINGLE_DETECT_ACK))
                    {
                        send_msg.paraLen=0;
                        send_msg.dataLen=9;
                        crc_check->n_checksum = 0;
                        char* data_temp_od_ack = (char*)malloc(sizeof(SendRev_CRC_Retrans_OD)+2);
                        memset(data_temp_od_ack,0,sizeof(SendRev_CRC_Retrans_OD)+2);
                        sendrev_crc_retrans_OD = (SendRev_CRC_Retrans_OD*)data_temp_od_ack;
                        CommandCode* commandcode =(CommandCode*)(data_temp_od_ack + sizeof(SendRev_CRC_Retrans_OD));
                        commandcode->commandCode = send_msg.commandCode;//msg->commandCode;
                        sendrev_crc_retrans_OD->frametype = SOFT_BUS_TYPE_COMMAND;
                        sendrev_crc_retrans_OD->CS = SYSTEM_COMMAND_SET;
                        sendrev_crc_retrans_OD->sequence = send_msg.seq;
                        sendrev_crc_retrans_OD->order_para_len = 0;
                        sendrev_crc_retrans_OD->srcID = send_msg.sourceID;
                        sendrev_crc_retrans_OD->destID = send_msg.destID;
                        sendrev_crc_retrans_OD->checksum = crc_check->n_checksum;
                        send_msg.data = data_temp_od_ack;
                        data_temp_od_ack = NULL;
                        //计算校验和
                        crc_check->n_checksum = crc_check->GenerateCheckSum(send_msg.data,sizeof(SendRev_CRC_Retrans_OD)+2);
                        memcpy(send_msg.data + sizeof(SendRev_CRC_Retrans_OD)-2,&crc_check->n_checksum,2);
                        break;
                    }
                    else
                    {
                        //计算校验和
                        crc_check->n_checksum = 0;//crc_check->GenerateCheckSum(msg->type,msg->seq,msg->commandSet,msg->commandCode,msg->paraLen,msg->templetID,msg->data,msg->dataLen-sizeof(CommandInfo));
                        char* dataTemp_od = (char *)malloc(sizeof(SendRev_CRC_Retrans_OD)+send_msg.dataLen - sizeof(CommandInfo));
                        sendrev_crc_retrans_OD = (SendRev_CRC_Retrans_OD*)dataTemp_od;
                        sendrev_crc_retrans_OD->frametype = SOFT_BUS_TYPE_COMMAND;
                        sendrev_crc_retrans_OD->CS = send_msg.commandSet;
                        sendrev_crc_retrans_OD->sequence = send_msg.seq;
                        sendrev_crc_retrans_OD->order_para_len = send_msg.paraLen;
                        sendrev_crc_retrans_OD->srcID = send_msg.sourceID;
                        sendrev_crc_retrans_OD->destID = send_msg.destID;
                        sendrev_crc_retrans_OD->checksum = crc_check->n_checksum;
                        memcpy(dataTemp_od + sizeof(SendRev_CRC_Retrans_OD),send_msg.data + sizeof(CommandInfo),send_msg.dataLen - sizeof(CommandInfo));
                        delete(send_msg.data);
                        send_msg.data = dataTemp_od;
                        dataTemp_od = NULL;
                        send_msg.dataLen = send_msg.dataLen - sizeof(CommandInfo) + sizeof(SendRev_CRC_Retrans_OD);
                        //计算校验和
                        crc_check->n_checksum = crc_check->GenerateCheckSum(send_msg.data,send_msg.dataLen);
                        memcpy(send_msg.data + sizeof(SendRev_CRC_Retrans_OD)-2,&crc_check->n_checksum,2);
                    }
                //#endif
                }
                else  //没有选择重传策略，收发帧中没有序列号
                {
                        if(send_msg.commandCode == SINGLE_DETECT || send_msg.commandCode == SINGLE_DETECT_ACK)
                        {
                            crc_check->n_checksum = 0;//crc_check->GenerateCheckSum(msg->type,msg->seq,msg->commandSet,msg->commandCode,msg->paraLen,msg->templetID,msg->data,msg->dataLen-sizeof(CommandInfo));

                            char* dataTemp_od = (char*)malloc(8);
                            sendrev_crc_OD = (SendRev_CRC_OD*)dataTemp_od;
                            sendrev_crc_OD->frametype = SOFT_BUS_TYPE_COMMAND;
                            sendrev_crc_OD->CS = send_msg.commandSet;
                            sendrev_crc_OD->order_para_len =send_msg.paraLen;
                            sendrev_crc_OD->srcID = send_msg.sourceID;
                            sendrev_crc_OD->destID = send_msg.destID;
                            sendrev_crc_OD->checksum = crc_check->n_checksum;
                            memcpy(dataTemp_od + sizeof(SendRev_CRC_OD), &send_msg.commandCode,2);
                            send_msg.data = dataTemp_od;
                            dataTemp_od = NULL;
                            send_msg.dataLen = 8;
                            //计算校验和
                            crc_check->n_checksum = crc_check->GenerateCheckSum(send_msg.data,send_msg.dataLen);
                            memcpy(send_msg.data + sizeof(SendRev_CRC_OD)-2,&crc_check->n_checksum,2);
                            break;
                        }

                        //计算校验和
                        crc_check->n_checksum = 0;//crc_check->GenerateCheckSum(msg->type,msg->seq,msg->commandSet,msg->commandCode,msg->paraLen,msg->templetID,msg->data,msg->dataLen-sizeof(CommandInfo));
                        char* dataTemp_od = (char*)malloc(sizeof(SendRev_CRC_OD)+ send_msg.dataLen - sizeof(CommandInfo));
                        sendrev_crc_OD = (SendRev_CRC_OD*)dataTemp_od;
                        sendrev_crc_OD->frametype = SOFT_BUS_TYPE_COMMAND;
                        sendrev_crc_OD->CS = send_msg.commandSet;
                        sendrev_crc_OD->order_para_len = send_msg.paraLen;
                        sendrev_crc_OD->srcID = send_msg.sourceID;
                        sendrev_crc_OD->destID = send_msg.destID;
                        sendrev_crc_OD->checksum = crc_check->n_checksum;
                        memcpy(dataTemp_od + sizeof(SendRev_CRC_OD), send_msg.data + sizeof(CommandInfo), send_msg.dataLen - sizeof(CommandInfo));
                        delete(send_msg.data);
                        send_msg.data = dataTemp_od;
                        dataTemp_od = NULL;
                        send_msg.dataLen = send_msg.dataLen - sizeof(CommandInfo) + sizeof(SendRev_CRC_OD);
                        //计算校验和
                        crc_check->n_checksum = crc_check->GenerateCheckSum(send_msg.data,send_msg.dataLen);
                        memcpy(send_msg.data + sizeof(SendRev_CRC_OD)-2,&crc_check->n_checksum,2);
                }
                //#endif
                break;
            }

            case SOFT_BUS_TYPE_VARIABLE://帧类型是数据帧
            {
                if(StrategyRepeat_istrue) //有重传策略
                {
                    //计算校验和并且对参数进行赋值
                    crc_check->n_checksum = 0 ; // crc_check->GenerateCheckSum(msg->type,msg->seq,msg->commandSet,msg->commandCode,msg->paraLen,msg->templetID,msg->data,msg->dataLen-sizeof(VariInfo));
                    char* dataTemp_vari = (char*)malloc(sizeof(SendRev_CRC_Retrans_VARI)+send_msg.dataLen - sizeof(VariInfo));
                    sendrev_crc_retrans_VARI = (SendRev_CRC_Retrans_VARI*)dataTemp_vari;
                    sendrev_crc_retrans_VARI->frametype = SOFT_BUS_TYPE_VARIABLE;
                    sendrev_crc_retrans_VARI->retain = 0;
                    sendrev_crc_retrans_VARI->sequence = send_msg.seq;
                    sendrev_crc_retrans_VARI->variframe_len = send_msg.dataLen - sizeof(VariInfo) - 1; // 此长度表示变量模板号之后的变量帧的数据长度;
                    sendrev_crc_retrans_VARI->srcID = send_msg.sourceID;
                    sendrev_crc_retrans_VARI->destID = send_msg.destID;
                    sendrev_crc_retrans_VARI->checksum = crc_check->n_checksum;
                    memcpy(dataTemp_vari + sizeof(SendRev_CRC_Retrans_VARI), send_msg.data + sizeof(VariInfo),send_msg.dataLen - sizeof(VariInfo));
                    delete(send_msg.data);
                    send_msg.data = dataTemp_vari;
                    dataTemp_vari = NULL;
                    send_msg.dataLen = send_msg.dataLen - sizeof(VariInfo) + sizeof(SendRev_CRC_Retrans_VARI);
                    //计算校验和
                    crc_check->n_checksum = crc_check->GenerateCheckSum(send_msg.data,send_msg.dataLen);
                    memcpy(send_msg.data + sizeof(SendRev_CRC_Retrans_VARI)-2,&crc_check->n_checksum,2);
                }
                else //没有重传策略
                {
                    //计算校验和并对参数进行赋值
                    crc_check->n_checksum = 0; // crc_check->GenerateCheckSum(msg->type,msg->seq,msg->commandSet,msg->commandCode,msg->paraLen,msg->templetID,msg->data,msg->dataLen-sizeof(VariInfo));
                    char* dataTemp_vari = (char*)malloc(sizeof(SendRev_CRC_VARI)+send_msg.dataLen - sizeof(VariInfo));
                    sendrev_crc_VARI = (SendRev_CRC_VARI*)dataTemp_vari;
                    sendrev_crc_VARI->frametype = SOFT_BUS_TYPE_VARIABLE;
                    sendrev_crc_VARI->retain = 0;
                    sendrev_crc_VARI->variframe_len = send_msg.dataLen - sizeof(VariInfo) - 1; // 此长度表示变量模板号之后的变量帧的数据长度;
                    sendrev_crc_VARI->srcID = send_msg.sourceID;
                    sendrev_crc_VARI->destID = send_msg.destID;
                    sendrev_crc_VARI->checksum = crc_check->n_checksum;
                    memcpy(dataTemp_vari + sizeof(SendRev_CRC_VARI), send_msg.data + sizeof(VariInfo), send_msg.dataLen - sizeof(VariInfo));
                    delete(send_msg.data);
                    send_msg.data = dataTemp_vari;
                    dataTemp_vari = NULL;
                    send_msg.dataLen = send_msg.dataLen - sizeof(VariInfo) + sizeof(SendRev_CRC_VARI);
                    //计算校验和
                    crc_check->n_checksum = crc_check->GenerateCheckSum(send_msg.data,send_msg.dataLen);
                    memcpy(send_msg.data + sizeof(SendRev_CRC_VARI)-2,&crc_check->n_checksum,2);
                }

                break;
            }


            case SOFT_BUS_TYPE_DATABLOCK://数据块传输帧
            {
                if(StrategyRepeat_istrue) //有重传策略
                {
                //#if (defined SOFT_BUS_STRATEGY_REPEAT)
                //计算校验和并且对各参数进行赋值
                    crc_check->n_checksum = 0;//crc_check->GenerateCheckSum(msg->type,msg->seq,msg->commandSet,msg->commandCode,msg->paraLen,msg->templetID,msg->data,msg->dataLen-sizeof(DatablockInfo));
                    char* dataTemp_datablock = (char*)malloc(sizeof(SendRev_CRC_Retrans_DATABLOCK)+send_msg.dataLen-sizeof(DatablockInfo));
                    sendrev_crc_retrans_DATABLOCK = (SendRev_CRC_Retrans_DATABLOCK*)dataTemp_datablock;
                    sendrev_crc_retrans_DATABLOCK->frametype = SOFT_BUS_TYPE_DATABLOCK;
                    sendrev_crc_retrans_DATABLOCK->retain = 0;
                    sendrev_crc_retrans_DATABLOCK->sequence = send_msg.seq;
                    sendrev_crc_retrans_DATABLOCK->retain_2 = 0;
                    sendrev_crc_retrans_DATABLOCK->MF = send_msg.moreFragment;
                    sendrev_crc_retrans_DATABLOCK->OFFSET = send_msg.offset;
                    sendrev_crc_retrans_DATABLOCK->datablockframe_len = send_msg.dataLen - sizeof(DatablockInfo);  //此处长度表示数据块传输帧的数据域的长度
                    sendrev_crc_retrans_DATABLOCK->srcID = send_msg.sourceID;
                    sendrev_crc_retrans_DATABLOCK->destID = send_msg.destID;
                    sendrev_crc_retrans_DATABLOCK->checksum =  crc_check->n_checksum;
                    memcpy(dataTemp_datablock + sizeof(SendRev_CRC_Retrans_DATABLOCK),send_msg.data + sizeof(DatablockInfo),send_msg.dataLen-sizeof(DatablockInfo));
                    delete(send_msg.data);
                    send_msg.data = dataTemp_datablock;
                    dataTemp_datablock = NULL;
                    send_msg.dataLen = send_msg.dataLen - sizeof(DatablockInfo) + sizeof(SendRev_CRC_Retrans_DATABLOCK);
                    //计算校验和
                    crc_check->n_checksum = crc_check->GenerateCheckSum(send_msg.data,send_msg.dataLen);
                    memcpy(send_msg.data + sizeof(SendRev_CRC_Retrans_DATABLOCK)-2,&crc_check->n_checksum,2);
                }
                else //没有重传策略
                {
                //#if (!defined SOFT_BUS_STRATEGY_REPEAT)
                //计算校验和并且对各参数进行赋值
                    crc_check->n_checksum = 0;//crc_check->GenerateCheckSum(msg->type,msg->seq,msg->commandSet,msg->commandCode,msg->paraLen,msg->templetID,msg->data,msg->dataLen-sizeof(DatablockInfo));
                    char* dataTemp_datablock = (char*)malloc(sizeof(SendRev_CRC_DATABLOCK)+send_msg.dataLen - sizeof(DatablockInfo));
                    sendrev_crc_DATABLOCK = (SendRev_CRC_DATABLOCK*)dataTemp_datablock;
                    sendrev_crc_DATABLOCK->frametype = SOFT_BUS_TYPE_DATABLOCK;
                    sendrev_crc_DATABLOCK->retain = 0;
                    sendrev_crc_DATABLOCK->retain_2 = 0;
                    sendrev_crc_DATABLOCK->MF = send_msg.moreFragment;
                    sendrev_crc_DATABLOCK->OFFSET = send_msg.offset;
                    sendrev_crc_DATABLOCK->datablockframe_len =  send_msg.dataLen - sizeof(DatablockInfo);  //此处长度表示数据块传输帧的数据域的长度
                    sendrev_crc_DATABLOCK->srcID = send_msg.sourceID;
                    sendrev_crc_DATABLOCK->destID = send_msg.destID;
                    sendrev_crc_DATABLOCK->checksum = crc_check->n_checksum;
                    memcpy(dataTemp_datablock + sizeof(SendRev_CRC_DATABLOCK), send_msg.data + sizeof(DatablockInfo), send_msg.dataLen - sizeof(DatablockInfo));
                    delete(send_msg.data);
                    send_msg.data = dataTemp_datablock;
                    dataTemp_datablock = NULL;
                    send_msg.dataLen =send_msg.dataLen - sizeof(DatablockInfo) + sizeof(SendRev_CRC_DATABLOCK);
                    //计算校验和
                    crc_check->n_checksum = crc_check->GenerateCheckSum(send_msg.data,send_msg.dataLen);
                    memcpy(send_msg.data + sizeof(SendRev_CRC_DATABLOCK)-2,&crc_check->n_checksum,2);
                }
                break;
            }

            default:
                break;
        }
        if (send_msg.type==SOFT_BUS_TYPE_COMMAND )//在这里加上判断是不是链路 检测报文*******************************************************//是命令帧
            {
                if (send_msg.commandCode == SINGLE_DETECT || send_msg.commandCode == SINGLE_DETECT_ACK)//是链路检测报文
                {
                    int whole_data_length=send_msg.dataLen+sizeof(projectinf);
                    char linkdata[110]={0};//创建一个新的数组来保存数据域信息
                    memcpy(linkdata, send_msg.data, send_msg.dataLen);//将原先的值放进去
                    memcpy(linkdata+send_msg.dataLen,(char*)&projectinf,sizeof(projectinf));//加入默认的项目信息报文
                    send_msg.dataCopy(linkdata,whole_data_length);//数据域赋值
                    send_msg.dataLen=whole_data_length;
                }
            }

//    //把发出去的数据打印出来 (打印到文件中)
//    if((msg->paraLen == 1 || msg->paraLen == 0) && msg->type == SOFT_BUS_TYPE_COMMAND)
//    {
//        if(StrategyRepeat_istrue)
//        {
//            if(msg->type == SOFT_BUS_TYPE_COMMAND && msg->commandSet == SYSTEM_COMMAND_SET && msg->commandCode == ACK)
//            {
//                //printf("ACK Send ---- ackSeq %d -- ackGenerator %d \n", msg->ackSeq, msg->ackGenerator);
//                SaveFile<<"ACK Send ---- ackSeq ";
//                SaveFile<< (unsigned int)msg->ackSeq;
//                SaveFile<< " -- ackGenerator ";
//                SaveFile<<(unsigned int) msg->ackGenerator<<endl;
//            }
//            else
//            {
//                switch (msg->commandCode)
//                {
//                    case ACK_REQUEST:{
//                                        // printf("ACK_REQUEST Send ---- seq %d \n",msg->seq);
//                                         SaveFile<<"ACK_REQUEST Send ---- seq ";
//                                         SaveFile<<(unsigned int)msg->seq<<endl;
//                                         break;
//                    }
//                    case DETECT:{
//                                //	printf("DETECT Send ---- seq %d \n",msg->seq);
//                                    SaveFile<<"DETECT Send ----  seq";
//                                    SaveFile<<(unsigned int)msg->seq<<endl;
//                                    break;
//                    }
//                }
//            }
//        }
//    }
//    else
//    {
//        switch(msg->type)
//        {
//            case SOFT_BUS_TYPE_COMMAND:
//            {
//                if(StrategyRepeat_istrue)
//                {
//                    SendRev_CRC_Retrans_OD * orderframe_trans = (SendRev_CRC_Retrans_OD*)(msg->data);
//                    CommandCode* orderframe_commandcode = (CommandCode*)(msg->data + sizeof(SendRev_CRC_Retrans_OD));
//                    /*printf("命令帧----帧头--帧类型：%d,命令集：%d,序列号：%d,命令参数长：%d，CRC校验：0x%x----数据域--命令码：%d，数据负载：",
//                            orderframe_trans->frametype,orderframe_trans->CS,orderframe_trans->sequence,orderframe_trans->order_para_len,orderframe_trans->checksum,
//                            orderframe_commandcode->commandCode);*/
//                     SaveFile<<"命令帧----帧头--帧类型：";
//                     SaveFile<<(unsigned int)orderframe_trans->frametype;
//                     SaveFile<<",命令集：";
//                     SaveFile<<(unsigned int)orderframe_trans->CS;
//                     SaveFile<<",序列号：";
//                     SaveFile<<(unsigned int)orderframe_trans->sequence;
//                     SaveFile<<",命令参数长：";
//                     SaveFile<<(unsigned int)orderframe_trans->order_para_len;
//                     SaveFile<<",源地址：";
//                     SaveFile<<(unsigned int)orderframe_trans->srcID;
//                     SaveFile<<",目的地址：";
//                     SaveFile<<(unsigned int)orderframe_trans->destID;
//                     SaveFile<<",CRC校验：";
//                     SaveFile<<(unsigned int)orderframe_trans->checksum;
//                     SaveFile<<"----数据域--命令码：";
//                     SaveFile<<(unsigned int)orderframe_commandcode->commandCode;
//                     SaveFile<<"，数据负载：";
//                    for(int i = 0; i < msg->dataLen-sizeof(SendRev_CRC_Retrans_OD)-2; i++)
//                    {
//                        //printf("0x%x ",*(msg->data + sizeof(SendRev_CRC_Retrans_OD)+2+i));
//                        SaveFile<<(unsigned int)(*(msg->data + sizeof(SendRev_CRC_Retrans_OD)+2+i));
//                        SaveFile<<"  ";
//                    }
//                    //printf("\n");
//                    SaveFile<<endl;
//                }
//                else
//                {
//                    SendRev_CRC_OD * orderframe_trans = (SendRev_CRC_OD*)(msg->data);
//                    CommandCode* orderframe_commandcode = (CommandCode*)(msg->data + sizeof(SendRev_CRC_OD));
//                    /*printf("命令帧----帧头--帧类型：%d,命令集：%d,命令参数长：%d，CRC校验：0x%x----数据域--命令码：%d，数据负载：",
//                            orderframe_trans->frametype,orderframe_trans->CS,orderframe_trans->order_para_len,orderframe_trans->checksum,
//                            orderframe_commandcode->commandCode);*/
//                    SaveFile<<"命令帧----帧头--帧类型：";
//                    SaveFile<<(unsigned int)orderframe_trans->frametype;
//                    SaveFile<<",命令集：";
//                    SaveFile<<(unsigned int)orderframe_trans->CS;
//                    SaveFile<<",命令参数长：";
//                    SaveFile<<(unsigned int)orderframe_trans->order_para_len;
//                    SaveFile<<",源地址：";
//                    SaveFile<<(unsigned int)orderframe_trans->srcID;
//                    SaveFile<<",目的地址：";
//                    SaveFile<<(unsigned int)orderframe_trans->destID;
//                    SaveFile<<",CRC校验：";
//                    SaveFile<<(unsigned int)orderframe_trans->checksum;
//                    SaveFile<<"----数据域--命令码：";
//                    SaveFile<<(unsigned int)orderframe_commandcode->commandCode;
//                    SaveFile<<"，数据负载：";
//                    for(int i = 0; i < msg->dataLen-sizeof(SendRev_CRC_OD)-2; i++)
//                    {
//                        //printf("0x%x  ",*(msg->data + sizeof(SendRev_CRC_OD)+2+i));
//                        SaveFile<<(unsigned int)(*(msg->data + sizeof(SendRev_CRC_OD)+2+i));
//                        SaveFile<<"  ";
//                    }
//                    //printf("\n");
//                    SaveFile<<endl;
//                }
//                break;
//            }
//            case SOFT_BUS_TYPE_VARIABLE:
//            {
//                if(StrategyRepeat_istrue)
//                {
//                    SendRev_CRC_Retrans_VARI* variframe_trans = (SendRev_CRC_Retrans_VARI*)(msg->data);
//                    VariCode* variframe_code = (VariCode*)(msg->data + sizeof(SendRev_CRC_Retrans_VARI));
//                    /*printf("数据帧----帧头--帧类型：%d,序列号：%d，数据负载长度：%d,CRC校验：0x%x----数据域--数据码：%d，数据负载：",
//                        variframe_trans->frametype,variframe_trans->sequence,variframe_trans->variframe_len,variframe_trans->checksum,variframe_code->variCode);*/
//                    SaveFile<<"数据帧----帧头--帧类型：";
//                    SaveFile<<(unsigned int)variframe_trans->frametype;
//                    SaveFile<<",序列号";
//                    SaveFile<<(unsigned int)variframe_trans->sequence;
//                    SaveFile<<",数据负载长度：";
//                    SaveFile<<(unsigned int)variframe_trans->variframe_len;
//                    SaveFile<<",源地址：";
//                    SaveFile<<(unsigned int)variframe_trans->srcID;
//                    SaveFile<<",目的地址：";
//                    SaveFile<<(unsigned int)variframe_trans->destID;
//                    SaveFile<<",CRC校验：";
//                    SaveFile<<(unsigned int)variframe_trans->checksum;
//                    SaveFile<<"----数据域--数据码：";
//                    SaveFile<<(unsigned int)variframe_code->variCode;
//                    SaveFile<<"，数据负载：";
//                    for(int i = 0; i < msg->dataLen-sizeof(SendRev_CRC_Retrans_VARI)-1; i++)
//                    {
//                        //printf("0x%x  ",*(msg->data + sizeof(SendRev_CRC_Retrans_VARI)+1+i));
//                        SaveFile<<(unsigned int)(*(msg->data + sizeof(SendRev_CRC_Retrans_VARI)+1+i));
//                        SaveFile<<"  ";
//                    }
//                    //printf("\n");
//                    SaveFile<<endl;
//                }
//                else
//                {
//                    SendRev_CRC_VARI* variframe_trans = (SendRev_CRC_VARI*)(msg->data);
//                    VariCode* variframe_code = (VariCode*)(msg->data + sizeof(SendRev_CRC_VARI));
//                    /*printf("数据帧----帧头--帧类型：%d,数据负载长度：%d,CRC校验：0x%x----数据域--数据码：%d，数据负载：",
//                        variframe_trans->frametype,variframe_trans->variframe_len,variframe_trans->checksum,variframe_code->variCode);*/
//                    SaveFile<<"数据帧----帧头--帧类型：";
//                    SaveFile<<(unsigned int)variframe_trans->frametype;
//                    SaveFile<<",数据负载长度：";
//                    SaveFile<<(unsigned int)variframe_trans->variframe_len;
//                    SaveFile<<",源地址：";
//                    SaveFile<<(unsigned int)variframe_trans->srcID;
//                    SaveFile<<",目的地址：";
//                    SaveFile<<(unsigned int)variframe_trans->destID;
//                    SaveFile<<",CRC校验：";
//                    SaveFile<<(unsigned int)variframe_trans->checksum;
//                    SaveFile<<"----数据域--数据码：";
//                    SaveFile<<(unsigned int)variframe_code->variCode;
//                    SaveFile<<"，数据负载：";
//                    for(int i = 0; i < msg->dataLen-sizeof(SendRev_CRC_VARI)-1; i++)
//                    {
//                        //printf("0x%x  ",*(msg->data + sizeof(SendRev_CRC_VARI)+1+i));
//                        SaveFile<<(unsigned int)(*(msg->data + sizeof(SendRev_CRC_VARI)+1+i));
//                        SaveFile<<"  ";
//                    }
//                    //printf("\n");
//                    SaveFile<<endl;
//                }
//                break;
//            }
//            case SOFT_BUS_TYPE_DATABLOCK:
//            {
//                if(StrategyRepeat_istrue)
//                {
//                    SendRev_CRC_Retrans_DATABLOCK* datablockframe_trans = (SendRev_CRC_Retrans_DATABLOCK*)(msg->data);
//                    /*printf("批量传输帧----帧头--帧类型:%d,序列号：%d,多片:%d,偏移量：%d,数据负载长度：%d,CRC校验:0x%x----数据域--数据负载：",
//                        datablockframe_trans->frametype,datablockframe_trans->sequence,datablockframe_trans->MF,datablockframe_trans->OFFSET,datablockframe_trans->datablockframe_len,datablockframe_trans->checksum);*/
//                    SaveFile<<"批量传输帧----帧头--帧类型:";
//                    SaveFile<<(unsigned int)datablockframe_trans->frametype;
//                    SaveFile<<",序列号：";
//                    SaveFile<<(unsigned int)datablockframe_trans->sequence;
//                    SaveFile<<",多片:";
//                    SaveFile<<(unsigned int)datablockframe_trans->MF;
//                    SaveFile<<",偏移量：";
//                    SaveFile<<(unsigned int)datablockframe_trans->OFFSET;
//                    SaveFile<<",数据负载长度：";
//                    SaveFile<<(unsigned int)datablockframe_trans->datablockframe_len;
//                    SaveFile<<",源地址：";
//                    SaveFile<<(unsigned int)datablockframe_trans->srcID;
//                    SaveFile<<",目的地址：";
//                    SaveFile<<(unsigned int)datablockframe_trans->destID;
//                    SaveFile<<",CRC校验:";
//                    SaveFile<<(unsigned int)datablockframe_trans->checksum;
//                    SaveFile<<"----数据域--数据负载：";
//                    for(int i = 0; i < msg->dataLen - sizeof(SendRev_CRC_Retrans_DATABLOCK); i++)
//                    {
//                        //printf("0x%x  ",*(msg->data + sizeof(SendRev_CRC_Retrans_DATABLOCK)+i));
//                        SaveFile<<(unsigned int)(*(msg->data + sizeof(SendRev_CRC_Retrans_DATABLOCK)+i));
//                        SaveFile<<"  ";
//                    }
//                    //printf("\n");
//                    SaveFile<<endl;
//                }
//                else
//                {
//                    SendRev_CRC_DATABLOCK* datablockframe_trans = (SendRev_CRC_DATABLOCK*)(msg->data);
//                    /*printf("批量传输帧----帧头--帧类型:%d,多片:%d,偏移量：%d,数据负载长度：%d,CRC校验:0x%x----数据域--数据负载：",
//                        datablockframe_trans->frametype,datablockframe_trans->MF,datablockframe_trans->OFFSET,datablockframe_trans->datablockframe_len,datablockframe_trans->checksum);*/
//                    SaveFile<<"批量传输帧----帧头--帧类型:";
//                    SaveFile<<(unsigned int)datablockframe_trans->frametype;
//                    SaveFile<<",多片:";
//                    SaveFile<<(unsigned int)datablockframe_trans->MF;
//                    SaveFile<<",偏移量：";
//                    SaveFile<<(unsigned int)datablockframe_trans->OFFSET;
//                    SaveFile<<",数据负载长度：";
//                    SaveFile<<(unsigned int)datablockframe_trans->datablockframe_len;
//                    SaveFile<<",源地址：";
//                    SaveFile<<(unsigned int)datablockframe_trans->srcID;
//                    SaveFile<<",目的地址：";
//                    SaveFile<<(unsigned int)datablockframe_trans->destID;
//                    SaveFile<<",CRC校验:";
//                    SaveFile<<(unsigned int)datablockframe_trans->checksum;
//                    SaveFile<<"----数据域--数据负载：";
//                    for(int i = 0; i < msg->dataLen-sizeof(SendRev_CRC_DATABLOCK); i++)
//                    {
//                        //printf("0x%x  ",*(msg->data + sizeof(SendRev_CRC_DATABLOCK)+i));
//                        SaveFile<<(unsigned int)(*(msg->data + sizeof(SendRev_CRC_DATABLOCK)+i));
//                        SaveFile<<"  ";
//                    }
//                    //printf("\n");
//                    SaveFile<<endl;
//                }
//                break;
//            }
//            default:
//                break;
//        }
//    }

    //自发自收，调用自己收发模块的接收函数
    //m_pTransRecModel->receive(msg);
    //this->receive(msg);

    //if (msg->seq == 10 && transOnce)
    //{
    //	transOnce = false;
    //}
    //else
//		queueChannel.push(msg);
//	::LeaveCriticalSection(&csQueueChannel);

    /***************************************
     调用底层硬件接口函数，将数据发送出去
     ***************************************/
    //2016.12.1
        /*选择的硬件是以太网*/
                if(true == HardwareAdapter_Ethernet)
                {
                        m_pHardwareEthernet->transmit(&send_msg);
                }
        /*选择的硬件是CAN总线*/
                else if(true == HardwareAdapter_CAN)
                {
                        m_pHardwareCANbus->canbus_send(&send_msg);
                }
        /*选择的硬件是串口RS422*/
                else if(true == HardwareAdapter_RS422)
                {
                        m_pHardwareSerial->serial_send(&send_msg);
                }
    return true;
}



////目前做自发自收，做测试之用
//void *  channelSim(void *  arg)
//{
//    CHardwareAdapter* pCA = (CHardwareAdapter*)arg;
//    CSoftBusMsg* msg;
//    while (1)
//    {
//        pthread_mutex_lock(&pCA->QueueChannel_lock);
//        if (!pCA->queueChannel.empty())
//        {
//            msg = pCA->queueChannel.front();
//            //printf("    0优先级队列发送一个消息\n");
//            //进行校验
//            //pCA->crc_check->CheckSum(msg->data,msg->dataLen);
//            pCA->queueChannel.pop();
//            pCA->m_pTransRecModel->receive(msg);
//        }
//        pthread_mutex_unlock(&pCA->QueueChannel_lock);
//    }
//}


ofstream SaveFile1("jieshoushuju.txt");
bool CHardwareAdapter::receive(CSoftBusMsg* msg)
{
    //first: qu zhen tou
    if(true == HardwareAdapter_Ethernet)
    {
            comHeader  *  comheader = (comHeader *) (msg->data + 14);
            msg->dataLen  =  comheader->len - 4;
            comheader  =  NULL;
            char  *  etherdata_temp  = (char *) malloc(msg->dataLen);
            memcpy(etherdata_temp,msg->data + sizeof(Mac_Header) + sizeof(comHeader) , msg->dataLen);
            free(msg->data);
            msg->data  =  etherdata_temp;
            etherdata_temp  =  NULL;
    }
    //命令帧用paralen，其余两种用datalen
    //对进来的数据进行解帧
    if(msg->data != NULL)//数据不为空
    {
        SendFrameHeader* sendframeHeader = (SendFrameHeader*) (msg->data);
        msg->type = sendframeHeader->frametype;

        switch(msg->type)//对帧类型进行区分
        {
            case SOFT_BUS_TYPE_COMMAND://如果帧类型是命令帧
            {
                if(StrategyRepeat_istrue)//有重传
                {
                    //对数据进行解帧
                    SendRev_CRC_Retrans_OD* sendrev_crc_retrans_od = (SendRev_CRC_Retrans_OD*) (msg->data);
                    CommandCode* commandcode = (CommandCode*)(msg->data + sizeof(SendRev_CRC_Retrans_OD));
                    if(sendrev_crc_retrans_od->CS == SYSTEM_COMMAND_SET &&  commandcode->commandCode == 1) //如果是ACK帧，需要解析ackSeq和ackGenerator
                    {
                            msg->commandSet = sendrev_crc_retrans_od->CS;
                            msg->ackSeq = sendrev_crc_retrans_od->sequence;
                            msg->paraLen = 1;
                            msg->sourceID = sendrev_crc_retrans_od->srcID;
                            msg->destID = sendrev_crc_retrans_od->destID;
                            msg->commandCode = commandcode->commandCode;
                            msg->ackSeq = sendrev_crc_retrans_od->sequence;
                            msg->ackGenerator = *(msg->data + sizeof(SendRev_CRC_Retrans_OD)+2);
                            delete(msg->data);
                            msg->data = NULL;
                    }
                    else if(commandcode->commandCode == 4 || commandcode->commandCode == 5)
                                        {
                                            msg->commandSet = sendrev_crc_retrans_od->CS;
                                            //msg->ackSeq = sendrev_crc_retrans_od->sequence;
                                            msg->paraLen = 0;
                                            msg->sourceID = sendrev_crc_retrans_od->srcID;
                                            msg->destID = sendrev_crc_retrans_od->destID;
                                            msg->commandCode = commandcode->commandCode;
                                            //msg->ackSeq= sendrev_crc_retrans_od->sequence;
                                            //msg->ackGenerator= *(msg->data + sizeof(SendRev_CRC_Retrans_OD)+2);
                                            msg->data = NULL;
                                            if (true == m_pStrategyPriority->receive(msg))
                                                return true;
                                        }
                    else
                    {
                            msg->commandSet = sendrev_crc_retrans_od->CS;
                            msg->seq = sendrev_crc_retrans_od->sequence;
                            msg->paraLen = sendrev_crc_retrans_od->order_para_len;
                            msg->sourceID = sendrev_crc_retrans_od->srcID;
                            msg->destID = sendrev_crc_retrans_od->destID;
                            msg->commandCode = commandcode->commandCode;
                            msg->ackSeq = msg->seq;
                            char * data_od = (char *) malloc(msg->paraLen);
                            memcpy(data_od,msg->data + sizeof(SendRev_CRC_Retrans_OD)+2,msg->paraLen);
                            delete(msg->data);
                            msg->data = data_od;
                            data_od = NULL;
                    }
                }
                else//如果没有重传
                {
                    //对数据进行解帧
                    SendRev_CRC_OD* sendrev_crc_od = (SendRev_CRC_OD*)(msg->data);
                    CommandCode* commandcode = (CommandCode*)(msg->data + sizeof(SendRev_CRC_OD));
                    msg->commandSet = sendrev_crc_od->CS;
                    msg->paraLen = sendrev_crc_od->order_para_len;
                    msg->sourceID = sendrev_crc_od->srcID;
                    msg->destID = sendrev_crc_od->destID;
                    msg->commandCode = commandcode->commandCode;
                    char * data_od = (char*)malloc(msg->paraLen);
                    memcpy(data_od,msg->data+sizeof(SendRev_CRC_OD)+2,msg->paraLen);
                    delete(msg->data);
                    msg->data = data_od;
                    data_od = NULL;
                }
                break;
            }
            case SOFT_BUS_TYPE_VARIABLE://如果是数据帧
            {
                if(StrategyRepeat_istrue)//有重传
                {
                    SendRev_CRC_Retrans_VARI* sendrev_crc_vari = (SendRev_CRC_Retrans_VARI*)(msg->data);
                    VariCode* varicode = (VariCode*)(msg->data + sizeof(SendRev_CRC_Retrans_VARI));
                    msg->dataLen = sendrev_crc_vari->variframe_len;
                    msg->seq = sendrev_crc_vari->sequence;
                    msg->sourceID = sendrev_crc_vari->srcID;
                    msg->destID = sendrev_crc_vari->destID;
                    msg->templetID = varicode->variCode;
                    char* data_vari = (char*)malloc(msg->dataLen);
                    memcpy(data_vari,msg->data+sizeof(SendRev_CRC_Retrans_VARI)+1,msg->dataLen);
                    delete(msg->data);
                    msg->data = data_vari;
                    data_vari = NULL;
                }
                else//没有重传
                {
                    SendRev_CRC_VARI* sendrev_crc_vari = (SendRev_CRC_VARI*)(msg->data);
                    VariCode* varicode = (VariCode*)(msg->data + sizeof(SendRev_CRC_VARI));
                    msg->dataLen = sendrev_crc_vari->variframe_len;
                    msg->sourceID = sendrev_crc_vari->srcID;
                    msg->destID = sendrev_crc_vari->destID;
                    msg->templetID = varicode->variCode;
                    char* data_vari = (char*)malloc(msg->dataLen);
                    memcpy(data_vari,msg->data+sizeof(SendRev_CRC_VARI)+1,msg->dataLen);
                    delete(msg->data);
                    msg->data = data_vari;
                    data_vari = NULL;
                }
                break;
            }
            case SOFT_BUS_TYPE_DATABLOCK://如果是批量传输帧
            {
                if(StrategyRepeat_istrue)//有重传
                {
                    SendRev_CRC_Retrans_DATABLOCK* sendrev_crc_datablock = (SendRev_CRC_Retrans_DATABLOCK*)(msg->data);
                    msg->moreFragment = sendrev_crc_datablock->MF;
                    msg->offset = sendrev_crc_datablock->OFFSET;
                    msg->dataLen = sendrev_crc_datablock->datablockframe_len;
                    msg->seq = sendrev_crc_datablock->sequence;
                    msg->sourceID = sendrev_crc_datablock->srcID;
                    msg->destID = sendrev_crc_datablock->destID;
                    char* data_block = (char*)malloc(msg->dataLen);
                    memcpy(data_block,msg->data+sizeof(SendRev_CRC_Retrans_DATABLOCK),msg->dataLen);
                    delete(msg->data);
                    msg->data = data_block;
                    data_block = NULL;
                }
                else//没有重传
                {
                    SendRev_CRC_DATABLOCK* sendrev_crc_datablock = (SendRev_CRC_DATABLOCK*)(msg->data);
                    msg->moreFragment = sendrev_crc_datablock->MF;
                    msg->offset = sendrev_crc_datablock->OFFSET;
                    msg->dataLen = sendrev_crc_datablock->datablockframe_len;
                    msg->sourceID = sendrev_crc_datablock->srcID;
                    msg->destID = sendrev_crc_datablock->destID;
                    char* data_block = (char*)malloc(msg->dataLen);
                    memcpy(data_block,msg->data+sizeof(SendRev_CRC_DATABLOCK),msg->dataLen);
                    delete(msg->data);
                    msg->data = data_block;
                    data_block = NULL;
                }
                break;
            }
            default:
                break;
        }
    }

 /***************************************
    da yin dao  SaveFile1 zhong
 ***************************************/
       if(StrategyRepeat_istrue)
       {
           if(msg->type == SOFT_BUS_TYPE_COMMAND && msg->commandSet == SYSTEM_COMMAND_SET && msg->commandCode == ACK)
           {
               //printf("ACK Send ---- ackSeq %d -- ackGenerator %d \n", msg->ackSeq, msg->ackGenerator);
               SaveFile1<<"ACK Receive ---- ackSeq ";
               SaveFile1<< (unsigned int)msg->ackSeq;
               SaveFile1<< " -- ackGenerator ";
               SaveFile1<<(unsigned int) msg->ackGenerator<<endl;
           }
       }

/***************************************
 调用上层模块的接收函数，有重传/无重传
 ***************************************/
            if(StrategyRepeat_istrue)//有重传
            {
                if (true == m_pTransRecModel->receive(msg))
                    return true;
            }
            else//没重传
            {
                if (true == m_pStrategyPriority->receive(msg))
                    return true;
            }

}

