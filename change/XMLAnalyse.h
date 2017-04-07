#ifndef XMLANALYSE_H
#define XMLANALYSE_H

#include "FrameHead.h"
#include "CSoftBus.h"
typedef unsigned long long ULONGLONG;
class CSoftBus;


class CXMLAnalyse
{
public:
    CXMLAnalyse(void);
    ~CXMLAnalyse(void);

    //XML文件路径
    char * xmlpathname;
    CSoftBus * m_csoftbus_xml;

public:
    bool  initial(CSoftBus* csoftbus);
    bool  XMLAnalyse(char* xmlPathName);
    ULONGLONG HexToDecimal(char* HexString,int StrLength);//十六进制转十进制

public:
    int   Version;      //版本
    int    SN;           //规范号
    char   CID[20];		 //设备构型

    bool   Retrans_is_Configure; //是否有重传模块
    unsigned int    SendWindow;           //发送窗口大小
    unsigned int    ACKWindow;            //接收窗口大小
    unsigned int    TimeDelay;            //时延 (多长时间接收不到)
    int     sourceID;
};

#endif // XMLANALYSE_H
