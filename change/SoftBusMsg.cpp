#include "SoftBusMsg.h"
#include "FrameHead.h"
#include <iostream>
#include <string.h>
#include<stdlib.h>

using namespace std;

extern  CommandInfo     commandInfo;
extern  VariInfo        variInfo;
extern  DatablockInfo   datablockInfo;

CSoftBusMsg::CSoftBusMsg(void)
{
    version = 0;
    type = 0xff;
    templetID = 0;//变量用
    device = 0;
    priority = 0;
    seq = 0;
    commandSet = 0;//命令用
    commandCode = 0;//命令用
    paraLen = 0;//命令用
    sourceID = 0;
    destID = 0;
    moreFragment = 0;//数据块用
    offset = 0;//数据块用
    checkSum = 0; //校验和

    data = NULL;
    dataLen = 0;
    pcommandInfo = NULL;
    pvariInfo = NULL;
    pdatablockInfo = NULL;
}

CSoftBusMsg::~CSoftBusMsg(void)
{
    free(data);
    data = NULL;
    pcommandInfo = NULL;
    pvariInfo = NULL;
    pdatablockInfo = NULL;
}


bool CSoftBusMsg::dataCopy(char* srcData, int srcDataLen)//数据区域赋值函数，可不用再申请内存
{
    data = (char*)malloc(srcDataLen*sizeof(char));//申请内存
    memcpy(data, srcData, srcDataLen);//赋值
    return true;
}
