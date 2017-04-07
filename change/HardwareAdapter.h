#ifndef HARDWAREADAPTER_H
#define HARDWAREADAPTER_H


#pragma once
#include "SoftBusMsg.h"
#include <queue>
#include "CSoftBus.h"
#include "FrameHead.h"
#include "CRCCheckSum.h"
#include "StrategyPriority.h"
#include <pthread.h>
#include <unistd.h>
#include "HardwareEthernet.h"
#include "HardwareSerial.h"
#include "HardwareCANbus.h"

using namespace std;

class CSoftBus;
class CTransRecModel;
class CCheckSum;
class CStrategyPriority;
class CHardwareEthernet;
class CHardwareSerial;
class CHardwareCANbus;

class CHardwareAdapter
{
public:
    bool flag;
    CHardwareAdapter(CTransRecModel* p);
    ~CHardwareAdapter(void);

public:
    CCheckSum* crc_check;
    unsigned char  SendData[65535];

public:
    SendRev_CRC_Retrans_OD*			  sendrev_crc_retrans_OD;
    SendRev_CRC_OD*					  sendrev_crc_OD;
    SendRev_CRC_Retrans_VARI*		  sendrev_crc_retrans_VARI;
    SendRev_CRC_VARI*				  sendrev_crc_VARI;
    SendRev_CRC_Retrans_DATABLOCK*    sendrev_crc_retrans_DATABLOCK;
    SendRev_CRC_DATABLOCK*			  sendrev_crc_DATABLOCK;

public:
    bool initial(CSoftBus* m_csoftbus);
    CSoftBus* m_pSoftbus;
    CTransRecModel* m_pTransRecModel;
    CStrategyPriority* m_pStrategyPriority;
    CHardwareEthernet* m_pHardwareEthernet;
    CHardwareSerial* m_pHardwareSerial;
    CHardwareCANbus* m_pHardwareCANbus;
    bool transmit(CSoftBusMsg* msg);
    bool receive(CSoftBusMsg* msg);
    friend  void *  channelSim(void * arg);
    queue<CSoftBusMsg*> queueChannel;
    //CRITICAL_SECTION csQueueChannel;
    pthread_mutex_t  QueueChannel_lock;
};

#endif // HARDWAREADAPTER_H
