#include "ReceiveWindow.h"
#include "softBusSystem.h"
#include <stdlib.h>
#include <cstring>

CReceiveWindow::CReceiveWindow(CSoftBus* m_psoftbus)
{
    receive_window_len = m_psoftbus->ACKWindow;
    memset(recMsg, NULL, 256 * sizeof(CSoftBusMsg*));//初始化全部为空
    winBegin = 0;
    breakFlag = -1;
}


CReceiveWindow::~CReceiveWindow(void)
{
}


//接收窗示意图见说明文件夹里的接收窗图片。
bool CReceiveWindow::insert(CSoftBusMsg* msg)
{
    isSendAck = false;
    ackSeq = 0;
    isHandover = false;
    //应对稀疏情况,收到ackReq时从窗口开始检测，一直检测到为NULL的序列号，也就是检测出一段连续的序列号，然后
    //发送ack,移窗。更新winBegin
    if (msg->type == SOFT_BUS_TYPE_COMMAND && msg->commandSet == SYSTEM_COMMAND_SET
        && msg->commandCode == ACK_REQUEST)
    {
        unsigned char ptr = winBegin;
        while (recMsg[ptr] != NULL)
        {
            ptr++;
            //必须得取余，否则会出现非常诡异的事情，那就是queueMain队列中数据被修改。
            //因为如果不取余的话，编译器认为减出来是个负数，数组越界指到别的地方去了，而刚好指到了queueMain
            //中的元素，造成元素被赋为NULL.
            unsigned char tempForDebug = (ptr + 256 - MSG_RECEIVED_STORE_LEN) % 256;
            recMsg[tempForDebug] = NULL;
        }
        isSendAck = true;
        ackSeq = (ptr + 256 - 1) % 256;
        isHandover = false;
        winBegin = ptr;
    }
    else
    {
        //来了重复的包，什么都不做
        if (recMsg[msg->seq] != NULL)
        {    
             isSendAck = false;
            ackSeq = ackSeq;
            isHandover = false;
        }
        //来了没有收到过的包
        else
        {
            isHandover = true;//先放入队列中
            recMsg[msg->seq] = msg;
            bool winFullFlag = false;
            if (breakFlag >= 0)//如果当前处于断层状态，判断新来的包是不是缺的那个包，如果不是什么都不做
            {
                if (msg->seq == breakFlag)
                    breakFlag = -1;//如果是，取消断层态，进入下面的正常检测
            }
            if (breakFlag<0)
            {
                bool findBreak = false;
                unsigned char i = 0;
                while (recMsg[(winBegin + i) % 256] != NULL)//1.窗口满检测
                {
                    i++;
                    unsigned char tempForDebug = (winBegin + 256 - MSG_RECEIVED_STORE_LEN) % 256 + i;
                    recMsg[tempForDebug] = NULL;
                }
                for (unsigned char j = 0; j<receive_window_len; j++)//2.窗口内有断层检测
                {
                    if (recMsg[(winBegin + j) % 256] == NULL && recMsg[(winBegin + j + 1) % 256] != NULL)
                        findBreak = true;
                }
                //1.2互斥，窗口满就不可能有断层，有断层就不可能满
                if (i>receive_window_len - 1)//如果满了
                {
                    winBegin = (winBegin + i) % 256;
                    isSendAck = true;
                    ackSeq = (winBegin + 256 - 1) % 256;
                }
                else
                {
                    if (findBreak)//如果有断层了
                    {
                        for (unsigned char i = 0; i<receive_window_len; i++)
                        {
                            if (recMsg[(winBegin + i - 1 + 256) % 256] != NULL && recMsg[(winBegin + i + 256) % 256] == NULL)
                            {
                                ackSeq = (winBegin + i - 1 + 256) % 256;
                                winBegin = (winBegin + i) % 256;
                                breakFlag = winBegin;//winBegin会停留在断层点
                                break;
                            }
                        }
                        isSendAck = true;
                    }
                    else
                    {
                        isSendAck = false;
                    }
                }
            }
        }
    }
    return true;
}
