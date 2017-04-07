#ifndef TRANSRECMODEL_H
#define TRANSRECMODEL_H

#pragma once
#include "softBusSystem.h"
#include "SoftBusMsg.h"
#include "CSoftBus.h"

class CSoftBus;
class CStrategyPriority;
class CStrategyRepeat;
class CHardwareAdapter;


class CTransRecModel
{
public:
    CTransRecModel(void);
    ~CTransRecModel(void);
    bool initial(CSoftBus* p_csoftbus);
    bool transmit(CSoftBusMsg* msg);
    bool receive(CSoftBusMsg* msg);
public:
    CStrategyPriority* m_pStrategyPriority;
    CStrategyRepeat* m_pStrategyRepeat;
    CHardwareAdapter* m_pHardwareAdapter;
};

#endif // TRANSRECMODEL_H
