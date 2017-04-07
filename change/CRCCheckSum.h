#ifndef CRCCHECKSUM_H
#define CRCCHECKSUM_H

#pragma once

typedef unsigned short WORD;

class CCheckSum
{
public:
    CCheckSum(void);
    ~CCheckSum(void);

public:
    WORD    n_checksum;
    //WORD   GenerateCheckSum(int  frametype,unsigned char seq,
    //						unsigned char commandSet, unsigned char commandCode,unsigned char paralen,
    //						unsigned char templetID,
    //						char* data,int datalen);  //发送时生成校验和
   WORD GenerateCheckSum(char* data, int datalen); //发送时生成校验和
    bool   CheckSum(char* data,int datalen);                    //接收时检验校验和
};

#endif // CRCCHECKSUM_H
