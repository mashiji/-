#ifndef FRAMEHEAD_H
#define FRAMEHEAD_H

#pragma once
#pragma pack(1) // 内存对齐

#include <stdio.h>

typedef unsigned short WORD;

/////////////////////////////////////////////////////
//XML解析关联结构
//命令帧关联结构体
typedef struct
{
    unsigned char commandSet;
    unsigned char priority;
    unsigned char srcID;
    unsigned char destID;
    char*         data;
    int           datalen;
}OrderFrameMapStruct;    //关联命令码与该结构

//数据帧关联结构体
typedef struct
{
    unsigned char priority;
    unsigned char srcID;
    unsigned char destID;
    char*         data;
    int           datalen;
}VariFrameMapStruct;     //关联数据码与该结构

//数据传输帧关联结构体
typedef struct
{
    unsigned char  srcID;
    unsigned char  dstID;
}DatablockMapStruct;


//功能帧结构体
//命令帧帧头信息 9字节
typedef struct
{
    unsigned char	 retain_1 : 3;	//保留
    unsigned char	 FrameType : 2;	//帧类型
    unsigned char	 Version : 3;	//版本
    unsigned char	 CID;	//设备构型
    unsigned char    SN;	//规范号
    unsigned char	 Priority;	//优先级
    unsigned char    SA;	//源设备号
    unsigned char    DA;	//目的设备号
    unsigned char    retain_2;	//保留
    unsigned char    CL : 2;	//命令码长
    unsigned char    CS : 6;	//命令集
    unsigned char    CPL;	//命令参数长
}CommandInfo;


//变量帧帧头信息  9字节
typedef	 struct
{
    unsigned char	 retain_1 : 3;	//保留
    unsigned char	 FrameType : 2;	//帧类型
    unsigned char	 Version : 3;	//版本
//	unsigned char    VTN;	//数据帧模板号  （数据帧模板号挪到数据域的第一个字节）
    unsigned char	 CID;	//设备构型
    unsigned char    SN;	//规范号
    unsigned char	 Priority;	//优先级
    unsigned char    SA;	//源设备号
    unsigned char    DA;	//目的设备号
    unsigned char    retain_2;	//保留
    unsigned short	 vari_len;   //变量帧数据域长度
}VariInfo;


//数据块传输帧帧头信息 11字节
typedef	 struct
{
    unsigned char	 retain_1 : 3;	//保留
    unsigned char	 FrameType : 2;	//帧类型
    unsigned char	 Version : 3;	//版本
    unsigned char	 CID;	//设备构型
    unsigned char    SN;	//规范号
    unsigned char	 Priority;	//优先级
    unsigned char    SA;	//源设备号
    unsigned char    DA;	//目的设备号
    unsigned char    retain_2 : 7;	//保留
    unsigned char    MF : 1;   //多片
    unsigned short	 OFFSET;   //偏移量
    unsigned short	 datablock_len;   //数据块传输帧数据域长度
}DatablockInfo;


//三种帧共有的帧头结构，以此结构中的type来判断是哪一种帧
typedef struct
{
    unsigned char	 retain_1 : 3;	//保留
    unsigned char	 FrameType : 2;	//帧类型
    unsigned char	 Version : 3;	//版本
}FunctionHeader;  //功能帧中三种帧共有的结构


typedef struct
{
    unsigned char retain : 6;
    unsigned char frametype : 2;
}SendFrameHeader;   //收发帧中三种帧共有的结构


typedef struct
{
    short  commandCode; //命令码
}CommandCode;


typedef struct
{
    unsigned char variCode; // 数据号（变量模板号）
}VariCode;

/////以下为收发帧的数据结构

//收发帧：命令帧（CRC、Retrans）
typedef struct
{
    unsigned char	CS : 6;			 // 命令集
    unsigned char   frametype : 2;   // 帧类型
    unsigned char   sequence;		 // 序列号
    unsigned char   order_para_len;  // 命令参数长
    unsigned char   srcID;
    unsigned char   destID;
    WORD            checksum;		 // 校验和（16位）
}SendRev_CRC_Retrans_OD;

//收发帧：命令帧（CRC）
typedef struct
{
    unsigned char   CS : 6;			 // 命令集
    unsigned char   frametype : 2;	 // 帧类型
    unsigned char   order_para_len;  // 命令参数长
    unsigned char   srcID;
    unsigned char   destID;
    WORD            checksum;		 // 校验和（16位）
}SendRev_CRC_OD;


//收发帧：数据帧（CRC、Retrans）
typedef struct
{
    unsigned char	retain : 6;     // 保留
    unsigned char   frametype : 2;  // 帧类型
    unsigned char   sequence;		// 序列号
    unsigned short  variframe_len;  // 帧长
    unsigned char   srcID;
    unsigned char   destID;
    WORD            checksum;		// 校验和（16位）
}SendRev_CRC_Retrans_VARI;



//收发帧：数据帧（CRC）
typedef struct
{
    unsigned char   retain : 6;		 // 保留
    unsigned char   frametype : 2;	 // 帧类型
    unsigned short  variframe_len;   // 帧长
    unsigned char   srcID;
    unsigned char   destID;
    WORD            checksum;		 // 校验和（16位）
}SendRev_CRC_VARI;


//收发帧：数据块批量传输帧（CRC、Retrans）
typedef struct
{
    unsigned char	retain : 6;     // 保留
    unsigned char   frametype : 2;  // 帧类型
    unsigned char   sequence;		// 序列号
    unsigned char    retain_2 : 7;	//保留
    unsigned char    MF : 1;        //多片
    unsigned short	 OFFSET;        //偏移量
    unsigned short  datablockframe_len;  // 帧长
    unsigned char   srcID;
    unsigned char   destID;
    WORD            checksum;		// 校验和（16位）
}SendRev_CRC_Retrans_DATABLOCK;


//收发帧：数据块批量传输帧（CRC）
typedef struct
{
    unsigned char	 retain : 6;     // 保留
    unsigned char    frametype : 2;  // 帧类型
    unsigned char    retain_2 : 7;	//保留
    unsigned char    MF : 1;        //多片
    unsigned short	 OFFSET;        //偏移量
    unsigned short  datablockframe_len;  // 帧长
    unsigned char   srcID;
    unsigned char   destID;
    WORD            checksum;		// 校验和（16位）
}SendRev_CRC_DATABLOCK;


//Ethernet  Frame  struct
typedef  struct
{
        unsigned  int  len: 31;
        unsigned  int  moreFrag:1;
}comHeader;


typedef  struct
{
        unsigned char  destmac[6];
        unsigned char  srcmac[6];
        unsigned short  ippro_type;
}Mac_Header;
typedef struct
{
        char projectname[16];
        char company[16];
        char workers[16];
        char date[22];
        char version[8];
        char CID[18];
}Project_Inf;


#endif // FRAMEHEAD_H
