#include "CRCCheckSum.h"
#include "FrameHead.h"
#include "softBusSystem.h"


CCheckSum::CCheckSum(void)
{

}


CCheckSum::~CCheckSum(void)
{

}


//WORD CCheckSum::GenerateCheckSum(int  frametype,unsigned char seq,
//								 unsigned char commandSet, unsigned char commandCode,unsigned char paralen, //命令帧专用
//								 unsigned char templetID, //数据帧专用
//								 char* data,int datalen)
WORD CCheckSum::GenerateCheckSum(char* data,int datalen)//生成CRC校验和
{
    WORD   crc_sum = 0; //CRC校验和
    unsigned char*  crc_sum_temp = NULL;//初始化指针为空
    for(int i = 0; i < datalen; i++)//校验和的计算
    {
        crc_sum_temp = (unsigned char*)(data+i);
        crc_sum = crc_sum + *crc_sum_temp;
    }
    return ~crc_sum;
}

bool CCheckSum::CheckSum(char* data,int datalen)
{
    WORD  crc_sum = 0;
    unsigned char* crc_sum_temp = NULL;

    for(int i = 0; i < datalen; i++)
    {
        crc_sum_temp = (unsigned char*)(data+i);
        crc_sum = crc_sum + *crc_sum_temp;
    }

    if (crc_sum)
        return true;
    else
        return true;
}
