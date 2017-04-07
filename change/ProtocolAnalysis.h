#ifndef PROTOCOLANALYSIS_H
#define PROTOCOLANALYSIS_H

#pragma once
#include "softBusSystem.h"
#include "FrameHead.h"
#include "SoftBusMsg.h"
#include "CSoftBus.h"
#include "TransRecModel.h"

#define LOCAL_LOGIC_ID 10;

class CSoftBus;
class CTransRecModel;

class CProtocolAnalysis
{
public:
    CProtocolAnalysis(void);
    ~CProtocolAnalysis(void);

public:
    FunctionHeader* commonHeader_gongneng;
    SendFrameHeader* commonHeader_shoufa;
    CTransRecModel* m_pTransRecModel_ProAnalyse;
    CSoftBus* m_pSoftBus_ProtocolAnalysis;
    //public:
    //	HANDLE hRecvSem;
public:
    bool initial(CSoftBus* m_psoftbus);
    bool functionFrameSend(CSoftBusMsg* msg);
    bool receive(CSoftBusMsg* msg);
};

#endif // PROTOCOLANALYSIS_H
