#ifndef SOFTBUSMSG_H
#define SOFTBUSMSG_H

#pragma once
#include "FrameHead.h"


class CSoftBusMsg
{
public:
    CSoftBusMsg(void);
    ~CSoftBusMsg(void);
public:
    //帧头信息
    unsigned char version;
    unsigned char type;
    unsigned char device;
    unsigned char priority;
    unsigned char seq;
    unsigned char commandSet;   //命令集（命令用）
    short commandCode;  //命令码（命令用）
    unsigned char paraLen;		//命令码长度(命令用)
    unsigned char templetID;	//数据帧模板号（数据帧用）
    unsigned char sourceID;
    unsigned char destID;
    bool	 moreFragment; //MF,多片（数据块用）
    unsigned int  offset;  //偏移量 （数据块用）

    short    checkSum; //计算校验和

    //用于存储接收到的ack包的序列号
    unsigned char ackSeq;
    //用于ackRequest包
    unsigned char ackRequestSeq;
    //用于ack包
    unsigned char ackGenerator;

    CommandInfo*    pcommandInfo;
    VariInfo*	    pvariInfo;
    DatablockInfo*  pdatablockInfo;
    //数据信息
    char*   data;
    int		dataLen;

public:
    bool    dataCopy(char* srcData, int srcDataLen);
};

#endif // SOFTBUSMSG_H
