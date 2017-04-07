#include "TransRecModel.h"
#include "StrategyPriority.h"
#include "StrategyRepeat.h"
#include "HardwareAdapter.h"

extern bool StrategyRepeat_istrue;
CTransRecModel::CTransRecModel(void)
{

    m_pStrategyPriority = new CStrategyPriority();
    m_pStrategyRepeat = new CStrategyRepeat();
}


CTransRecModel::~CTransRecModel(void)
{
    delete(m_pStrategyPriority);
    delete(m_pStrategyRepeat);
}

bool CTransRecModel::initial(CSoftBus* p_csoftbus)
{
        m_pHardwareAdapter = p_csoftbus->m_pHardwareAdapter;
        if(StrategyRepeat_istrue)//如果使用重传策略
        {
            bool retVal_1 = m_pStrategyPriority->initial(p_csoftbus);
            bool retVal_2 = m_pStrategyRepeat->initial(p_csoftbus);
            return (retVal_1 && retVal_2);
        }
        else//不采用重传策略
        {
            bool retVal_1 = m_pStrategyPriority->initial(p_csoftbus);
            return retVal_1;
        }
}

//数据发送
bool CTransRecModel::transmit(CSoftBusMsg* msg)
{
    //注意：发送前有收发模块对功能帧进行简化的过程(将此条改为在收发模块之后做   王志浩)
    //因为经过重传模块才能分配序列号，若没有定义重传模块就没有序列号
    //对数据帧的简化以及计算校验和都放在收发模块之后，硬件适配模块的最前面做
    //后经过商议，优先级模块一定有

    if(StrategyRepeat_istrue)//采用重传
//#if (defined SOFT_BUS_STRATEGY_PRIORITY) && (defined SOFT_BUS_STRATEGY_REPEAT)
        m_pStrategyPriority->transmit(msg);
//#endif

//#if (!defined SOFT_BUS_STRATEGY_PRIORITY) && (defined SOFT_BUS_STRATEGY_REPEAT)
//	m_pStrategyRepeat->transmit(msg);
//#endif
    else//不采用重传
//#if (defined SOFT_BUS_STRATEGY_PRIORITY) && (!defined SOFT_BUS_STRATEGY_REPEAT)
        m_pStrategyPriority->transmit(msg);
//#endif

//#if (!defined SOFT_BUS_STRATEGY_PRIORITY) && (!defined SOFT_BUS_STRATEGY_REPEAT)
//	m_pHardwareAdapter->transmit(msg);
//#endif
    return true;
}


//数据接收
bool CTransRecModel::receive(CSoftBusMsg* msg)
{
    //后经过商议，优先级模块一定有
    if(StrategyRepeat_istrue)
//#if (defined SOFT_BUS_STRATEGY_PRIORITY) && (defined SOFT_BUS_STRATEGY_REPEAT)
    m_pStrategyRepeat->receive(msg);
//#endif
//#if (!defined SOFT_BUS_STRATEGY_PRIORITY) && (defined SOFT_BUS_STRATEGY_REPEAT)
//	m_pStrategyRepeat->receive(msg);
//#endif
    else
        ;
//#if (defined SOFT_BUS_STRATEGY_PRIORITY) && (!defined SOFT_BUS_STRATEGY_REPEAT)
    //调用优先级模块的receive
//#endif
//#if (!defined SOFT_BUS_STRATEGY_PRIORITY) && (!defined SOFT_BUS_STRATEGY_REPEAT)
//	//调用协议解析模块的receive
//#endif
    return true;
}
