#include "XMLAnalyse.h"
#include "libxml2/libxml/parser.h"
#include "libxml2/libxml/tree.h"
#include <string>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include<math.h>
const char * frame_c ="命令帧";
const char * frame_d= "数据帧";
const char * frame_p= "批量传输帧";
CommandInfo  commandInfo;
VariInfo  variInfo;
DatablockInfo   datablockInfo;
Project_Inf		projectinf;

bool StrategyRepeat_istrue = false;
bool HardwareAdapter_Ethernet = false;
bool HardwareAdapter_CAN = false;
bool HardwareAdapter_RS422 = false;
bool HardwareAdapter_1553b = false;

using namespace std;

CXMLAnalyse::CXMLAnalyse(void)
{
}

CXMLAnalyse::~CXMLAnalyse(void)
{
}


bool  CXMLAnalyse::initial(CSoftBus* csoftbus)
{
    xmlpathname = NULL;
    m_csoftbus_xml = csoftbus;
    //命令帧（初始化）
    commandInfo.Version = 0;
    commandInfo.FrameType = 0;
    commandInfo.retain_1 = 0;
    commandInfo.CID = 0x00;
    commandInfo.SN = 0x00;
    commandInfo.Priority = 0x00;
    commandInfo.retain_2 = 0x00;
    commandInfo.CS = 0;
    commandInfo.CL = 0;
    commandInfo.CPL = 0x00;
    commandInfo.SA = 0x00;
    commandInfo.DA = 0x00;
    //变量帧（初始化）
    variInfo.Version = 0;
    variInfo.FrameType = 0;
    variInfo.retain_1 = 0;
    variInfo.CID = 0x00;
    variInfo.SN = 0x00;
    variInfo.Priority = 0x00;
    variInfo.retain_2 = 0x00;
    variInfo.SA = 0x00;
    variInfo.DA = 0x00;
    variInfo.vari_len = 0;
    //数据块传输帧（初始化）
    datablockInfo.Version = 0;
    datablockInfo.FrameType = 0;
    datablockInfo.retain_1 = 0;
    datablockInfo.CID = 0x00;
    datablockInfo.SN = 0x00;
    datablockInfo.Priority = 0x00;
    datablockInfo.MF = 0;
    datablockInfo.retain_2 = 0;
    datablockInfo.OFFSET = 0;
    datablockInfo.SA = 0x00;
    datablockInfo.DA = 0x00;
    datablockInfo.datablock_len = 0;

    Version = 0;
    SN = 0;
    Retrans_is_Configure = false;
    SendWindow = 0;
    ACKWindow = 0;
    TimeDelay = 0;
    return true;
}

//xm解析，利用linux下的libxml2库，不同与windows
bool  CXMLAnalyse::XMLAnalyse(char* xmlpathname)
{
      xmlNodePtr NodeRoot = NULL;   //根节点
      xmlNodePtr Node = NULL;       //一级节点
      xmlNodePtr NodeChild = NULL ;//二级节点
      xmlNodePtr NodeGrand = NULL; //三级节点
      xmlNodePtr NodeGrand1 = NULL; //三级节点
      xmlNodePtr propNode = NULL;
      char * szDocName = xmlpathname;
      xmlDocPtr doc = xmlReadFile(szDocName,"UTF-8",XML_PARSE_RECOVER);//文档入口
      NodeRoot = xmlDocGetRootElement(doc);//获取根节点
      Node = NodeRoot -> xmlChildrenNode;//一级节点
      string  s_projectinf;
      while(Node != NULL)
      {
               if(xmlHasProp(Node,BAD_CAST"项目名称"))
                  {
                           propNode=Node;
                           xmlAttrPtr attrPtrNode=propNode->properties;
                           while (attrPtrNode != NULL)
                                          {
                               if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "项目名称"))//获取项目名称的属性
                               {
                                  xmlChar* szAttr = xmlGetProp(propNode,BAD_CAST "项目名称");
                                  memcpy(projectinf.projectname,szAttr,sizeof(projectinf.projectname));
                                  xmlFree(szAttr);
                               }
                               if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "单位"))//获取单位的属性
                               {
                                  xmlChar* szAttr = xmlGetProp(propNode,BAD_CAST "单位");
                                  memcpy(projectinf.company,szAttr,sizeof(projectinf.company));
                                  xmlFree(szAttr);
                               }
                               if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "工程人员"))//获取工程人员的属性
                               {
                                  xmlChar* szAttr = xmlGetProp(propNode,BAD_CAST "工程人员");
                                  memcpy(projectinf.workers,szAttr,sizeof(projectinf.workers));
                                  xmlFree(szAttr);
                               }
                               if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "日期"))//获取日期的属性
                               {
                                  xmlChar* szAttr = xmlGetProp(propNode,BAD_CAST "日期");
                                  memcpy(projectinf.date,szAttr,sizeof(projectinf.date));
                                  xmlFree(szAttr);
                               }
                                              if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "版本"))//获取版本的属性
                                              {
                                                 xmlChar* szAttr = xmlGetProp(propNode,BAD_CAST "版本");
                                                 Version =(*szAttr) - 48;
                                                 commandInfo.Version = Version;
                                                 variInfo.Version = Version;
                                                 datablockInfo.Version = Version;
                                                 memcpy(projectinf.version,szAttr,sizeof(projectinf.version));
                                                 xmlFree(szAttr);
                                              }
                                              if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "设备构型"))//获取设备构型的属性
                                              {
                                                  xmlChar* szAttr = xmlGetProp(propNode,BAD_CAST "设备构型");
                                                  memcpy(CID,szAttr,sizeof(CID));
                                                  memcpy(projectinf.CID,szAttr,sizeof(projectinf.CID));//项目信息中的设备构型赋值
                                                  if('E' == CID[0])
                                                      {
                                                          HardwareAdapter_Ethernet = true;
                                                          commandInfo.CID = 0;
                                                          variInfo.CID = 0;
                                                          datablockInfo.CID = 0;
                                                      }
                                                      else if('C' == CID[0])
                                                      {
                                                          HardwareAdapter_CAN = true;
                                                          commandInfo.CID = 1;
                                                          variInfo.CID = 1;
                                                          datablockInfo.CID = 1;
                                                      }
                                                      else if('R' == CID[0])
                                                      {
                                                          HardwareAdapter_RS422 = true;
                                                          commandInfo.CID = 2;
                                                          variInfo.CID = 2;
                                                          datablockInfo.CID = 2;
                                                      }
                                                      else if('1' == CID[0])
                                                      {
                                                          HardwareAdapter_1553b = true;
                                                          commandInfo.CID = 3;
                                                          variInfo.CID = 3;
                                                          datablockInfo.CID = 3;
                                                      }
                                                  xmlFree(szAttr);
                                              }
                                              attrPtrNode= attrPtrNode->next;
                                         }
                 }
              if (xmlHasProp(Node,BAD_CAST "重传"))
              {
                             propNode=Node;
                             xmlAttrPtr attrPtrNode=propNode->properties;
                             while (attrPtrNode != NULL)
                             {
                                             if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "重传"))//获取重传的属性
                                                     {
                                                         xmlChar* szAttr = xmlGetProp(propNode,BAD_CAST "重传");
                                                         Retrans_is_Configure = (bool)((* szAttr) - 48);
                                                         xmlFree(szAttr);
                                                     }
                                                      attrPtrNode = attrPtrNode->next;
                             }
                            NodeChild=Node->xmlChildrenNode;
                            xmlAttrPtr attrPtrNodeChild=NodeChild->properties;
                            while (attrPtrNodeChild != NULL)
                           {
                                           if (!xmlStrcmp(attrPtrNodeChild->name, BAD_CAST "发送窗"))//获取发送窗属性
                                           {
                                               xmlChar* szAttr = xmlGetProp(NodeChild,BAD_CAST "发送窗");
                                               m_csoftbus_xml->SendWindow = atoi((const char*)szAttr);;
                                               xmlFree(szAttr);
                                           }
                                           if (!xmlStrcmp(attrPtrNodeChild->name, BAD_CAST "ACK"))//获取ACK属性
                                           {
                                               xmlChar* szAttr = xmlGetProp(NodeChild,BAD_CAST "ACK");
                                               m_csoftbus_xml->ACKWindow = atoi((const char*)szAttr);
                                               xmlFree(szAttr);
                                           }
                                           if (!xmlStrcmp(attrPtrNodeChild->name, BAD_CAST "延时"))//获取延时属性
                                           {
                                               xmlChar* szAttr = xmlGetProp(NodeChild,BAD_CAST "延时");
                                               m_csoftbus_xml->TimeDelay = atoi((const char*)szAttr);
                                               xmlFree(szAttr);
                                           }
                                            attrPtrNodeChild= attrPtrNodeChild->next;//同级遍历属性
                         }
           }

         NodeChild = Node -> xmlChildrenNode;//指向二级节点
                       while(NodeChild != NULL)
                               {
                                 const char * frame_1;
                                 xmlChar* szAttr1;//提取类型
                                 xmlChar* szAttr2;//提取命令集
                                 xmlChar* szAttr3;//提取优先级
                                 xmlChar* szAttr4;//提取目的地址
                                 xmlChar* szAttr5[100];//提取数值
                                  xmlChar* szAttr6;//提取源地址
                                  xmlChar* szAttr7[100];//提取字节数
                                   xmlChar* szAttr8[100];//类型
                                   xmlChar* szAttr71;//提取字节数1
                                   string  s_bytenum;

                                 if(xmlHasProp(NodeChild,BAD_CAST "类型"))
                                    {
                                               propNode=NodeChild;
                                               xmlAttrPtr attrPtrNode=propNode->properties;
                                               while( attrPtrNode != NULL)
                                              {
                                                          if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "类型"))//获取类型的属性
                                                          {
                                                             szAttr1 = xmlGetProp(propNode,BAD_CAST "类型");
                                                             frame_1=(const char *)szAttr1;
                                                          }
                                                         if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "命令集"))//获取命令集的属性
                                                         {
                                                            szAttr2= xmlGetProp(propNode,BAD_CAST "命令集");
                                                         }
                                                          if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "优先级"))//获取优先级的属性
                                                         {
                                                            szAttr3 = xmlGetProp(propNode,BAD_CAST "优先级");
                                                         }
                                                          if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "源地址"))//获取源地址的属性
                                                         {
                                                            szAttr6 = xmlGetProp(propNode,BAD_CAST "源地址");
                                                         }
                                                          if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "目的地址"))//获取目的地址的属性
                                                         {
                                                            szAttr4 = xmlGetProp(propNode,BAD_CAST "目的地址");
                                                         }
                                                          if (!xmlStrcmp(attrPtrNode->name, BAD_CAST "字节数"))//获取字节数的属性
                                                         {
                                                            szAttr71 = xmlGetProp(propNode,BAD_CAST "字节数");
                                                         }
                                                           attrPtrNode = attrPtrNode->next;
                                                         }

                                                         NodeGrand = NodeChild->xmlChildrenNode;
                                                         //命令帧
                                                         if(!(strcmp(frame_1,frame_c)))
                                                         {
                                                             OrderFrameMapStruct*  orderframe_map_struct = (OrderFrameMapStruct* )malloc(sizeof(OrderFrameMapStruct));
                                                             orderframe_map_struct->commandSet = atoi((const char*)szAttr2);
                                                             orderframe_map_struct->priority = atoi((const char*)szAttr3);
                                                             orderframe_map_struct->destID =atoi((const char*)szAttr4);
                                                              orderframe_map_struct->srcID =atoi((const char*)szAttr6);
                                                              s_bytenum = (const char*)szAttr71;
                                                              int i_s_bytenum_length=s_bytenum.length();
                                                              string original_byte = s_bytenum.substr(0,i_s_bytenum_length-8);//减去8是因为“byte”+一个字符+“bit”为8个字符
                                                              int    i_btye_num=atoi(original_byte.c_str());
                                                              string is_bit        = s_bytenum.substr(i_s_bytenum_length-4,1);//减去4是由于“bit”为3个字符
                                                              if (is_bit!="0")
                                                              {
                                                                  i_btye_num++;
                                                              }
                                                              orderframe_map_struct->datalen = i_btye_num-sizeof(CommandCode);
                                                              //*****************************************解析数据与进行默认结构体填写*************************************
                                                              orderframe_map_struct->data = (char*)malloc(orderframe_map_struct->datalen );//数据域中真正的数据长度为字节数减去命令码长度
                                                               int dataoffset =0;//数据偏移量
                                                              int data_num=0;
                                                             for(int i=0;NodeGrand != NULL;i++,NodeGrand = NodeGrand -> next)
                                                             {
                                                                 xmlAttrPtr attrPtrNodeGrand =NodeGrand->properties;
                                                                 for(;(attrPtrNodeGrand != NULL);attrPtrNodeGrand = attrPtrNodeGrand -> next)
                                                                 {
                                                                     if (!xmlStrcmp(attrPtrNodeGrand->name, BAD_CAST "类型"))//获取类型的属性
                                                                      {
                                                                         szAttr8[i] = xmlGetProp(NodeGrand,BAD_CAST "类型");
                                                                      }
                                                                     if (!xmlStrcmp(attrPtrNodeGrand->name, BAD_CAST "数值"))//获取数值的属性
                                                                      {
                                                                         szAttr5[i] = xmlGetProp(NodeGrand,BAD_CAST "数值");
                                                                      }
                                                                     if(xmlStrcmp(attrPtrNodeGrand->name, BAD_CAST "字节数"))//获取字节数的属性
                                                                     {
                                                                         szAttr7[i] = xmlGetProp(NodeGrand,BAD_CAST "字节数");
                                                                     }
                                                               }
                                                                 data_num=i+1;
                                                           }
                                                             string  s_ordercode;
                                                             s_ordercode     = (const char*)szAttr5[0];
                                                             int n_ordercode ;
                                                             if ( s_ordercode.substr(0,2)=="0x")//十六进制
                                                             {
                                                               n_ordercode = HexToDecimal((char*)s_ordercode.c_str(),s_ordercode.length());
                                                             }
                                                             else
                                                              {
                                                                 n_ordercode = atoi((const char*)szAttr5[0]);
                                                               }
                                                             //***************************************************以下是除命令码外其他数据域*********************************
                                                            string s_value;
                                                            string s_datatype;
                                                            unsigned char bittype_value_total=0,bittype_value_present=0;
                                                            int bitnum = 0;
                                                            int bytenum = 0;
                                                            for (int i=1;i<data_num;i++)
                                                            {
                                                                s_datatype=(const char *)szAttr8[i];
                                                                  if( s_datatype == "bit") //是位类型
                                                                    {
                                                                      //******************************获取数值****************
                                                                      s_value  = (const char*)szAttr5[i];
                                                                      //*************判断进制**************
                                                                      if (s_value.substr(0,2) == "0x")
                                                                     {
                                                                     bittype_value_present = HexToDecimal((char*)s_value.c_str(),s_value.length());
                                                                     }
                                                                    else
                                                                    {
                                                                      bittype_value_present = atoi((const char*)szAttr5[i]);
                                                                    }
                                                                     bittype_value_total += bittype_value_present*(pow(2.0,bitnum));
                                                                      int  bitnum_present     = atoi((const char*)szAttr7[i]);
                                                                      bitnum	 += bitnum_present ;//计算当前总bit数

                                                                      //*************************************看下一个是不是bit,如果不是则直接赋值，如果是，若总位数大于8，也直接赋值
                                                                      s_datatype=(const char *)szAttr8[i+1];
                                                                      if ( s_datatype == "bit")//下一个也是位类型
                                                                       {
                                                                          int next_bitnum=atoi((const char*)szAttr7[i+1]);
                                                                          if ( bitnum+next_bitnum>8)
                                                                               {
                                                                               memcpy( orderframe_map_struct->data+dataoffset,&bittype_value_total,sizeof(char));//赋值
                                                                               dataoffset++;//偏移量+1
                                                                               bittype_value_total=0;//数据复位
                                                                               bittype_value_present=0;
                                                                               bitnum = 0;
                                                                               bytenum = 0;
                                                                            }
                                                                      }
                                                                      else   //下一个不是位类型
                                                                      {
                                                                       memcpy( orderframe_map_struct->data+dataoffset,&bittype_value_total,sizeof(char));//赋值
                                                                      dataoffset++;//偏移量+1
                                                                      bittype_value_total=0;//数据复位
                                                                     bittype_value_present=0;
                                                                     bitnum = 0;
                                                                    bytenum = 0;
                                                                    }
                                                                 }
                                                             else      //不是位类型
                                                             {
                                                                 bytenum=atoi((const char*)szAttr7[i]);
                                                                 s_value =(const char*)szAttr5[i];
                                                                 //判断进制,并转化数值**************************************
                                                                if (s_value.substr(0,2) == "0x")
                                                               {
                                                               ULONGLONG datavalue = HexToDecimal((char*)s_value.c_str(),s_value.length());
                                                                memcpy( orderframe_map_struct->data+dataoffset,&datavalue,bytenum);//赋值
                                                                dataoffset +=bytenum;
                                                                }
                                                                else
                                                                {
                                                                    ULONGLONG datavalue =atoi((const char*)szAttr5[i]);
                                                                    memcpy( orderframe_map_struct->data+dataoffset,&datavalue,bytenum );//赋值
                                                                    dataoffset +=bytenum;
                                                                }
                                                             }
                                                        }
//                                                            for (int i=0;i<orderframe_map_struct->datalen;i++)
//                                                                            {
//                                                                                printf("%02x  ",(unsigned char)orderframe_map_struct->data[i]);
//                                                                            }
//                                                                            printf("\n");
                                                            m_csoftbus_xml->OrderFrameLink.insert(make_pair(n_ordercode,orderframe_map_struct));

                                                     }

                                                           //数据帧
                                                         if(!(strcmp(frame_1,frame_d)))
                                                         {
                                                             VariFrameMapStruct * variframe_map_struct = (VariFrameMapStruct *)malloc(sizeof(VariFrameMapStruct));
                                                             variframe_map_struct->priority = atoi((const char*)szAttr3);
                                                             variframe_map_struct->destID =atoi((const char*)szAttr4);
                                                             variframe_map_struct->srcID =atoi((const char*)szAttr6);
                                                              s_bytenum = (const char*)szAttr71;
                                                              int i_s_bytenum_length=s_bytenum.length();
                                                              string original_byte = s_bytenum.substr(0,i_s_bytenum_length-8);//减去8是因为“byte”+一个字符+“bit”为8个字符
                                                              int    i_btye_num=atoi(original_byte.c_str());
                                                              string is_bit        = s_bytenum.substr(i_s_bytenum_length-4,1);//减去4是由于“bit”为3个字符
                                                              if (is_bit!="0")
                                                              {
                                                                  i_btye_num++;
                                                              }
                                                              variframe_map_struct->datalen = i_btye_num-sizeof(VariCode);
                                                              //*****************************************解析数据与进行默认结构体填写*************************************
                                                              variframe_map_struct->data = (char*)malloc(variframe_map_struct->datalen );//数据域中真正的数据长度为字节数减去命令码长度
                                                              int dataoffset =0;//数据偏移量
                                                              int data_num=0;
                                                             for(int i=0;NodeGrand != NULL;i++,NodeGrand = NodeGrand -> next)
                                                             {
                                                                 xmlAttrPtr attrPtrNodeGrand =NodeGrand->properties;
                                                                 for(;(attrPtrNodeGrand != NULL);attrPtrNodeGrand = attrPtrNodeGrand -> next)
                                                                 {
                                                                     if (!xmlStrcmp(attrPtrNodeGrand->name, BAD_CAST "类型"))//获取类型的属性
                                                                      {
                                                                         szAttr8[i] = xmlGetProp(NodeGrand,BAD_CAST "类型");
                                                                      }
                                                                     if (!xmlStrcmp(attrPtrNodeGrand->name, BAD_CAST "数值"))//获取数值的属性
                                                                      {
                                                                         szAttr5[i] = xmlGetProp(NodeGrand,BAD_CAST "数值");
                                                                      }
                                                                     if(xmlStrcmp(attrPtrNodeGrand->name, BAD_CAST "字节数"))//获取字节数的属性
                                                                     {
                                                                         szAttr7[i] = xmlGetProp(NodeGrand,BAD_CAST "字节数");
                                                                     }
                                                               }
                                                                 data_num=i+1;
                                                           }
                                                             string  s_ordercode;
                                                             s_ordercode     = (const char*)szAttr5[0];
                                                             int n_varicode ;
                                                             if ( s_ordercode.substr(0,2)=="0x")//十六进制
                                                             {
                                                               n_varicode = HexToDecimal((char*)s_ordercode.c_str(),s_ordercode.length());
                                                             }
                                                             else
                                                              {
                                                                 n_varicode = atoi((const char*)szAttr5[0]);
                                                               }
                                                             //***************************************************以下是除命令码外其他数据域*********************************
                                                            string s_value;
                                                            string s_datatype;
                                                            unsigned char bittype_value_total=0,bittype_value_present=0;
                                                            int bitnum = 0;
                                                            int bytenum = 0;
                                                            for (int i=1;i<data_num;i++)
                                                            {
                                                                s_datatype=(const char *)szAttr8[i];
                                                                  if( s_datatype == "bit") //是位类型
                                                                    {
                                                                      //******************************获取数值****************
                                                                      s_value  = (const char*)szAttr5[i];
                                                                      //*************判断进制**************
                                                                      if (s_value.substr(0,2) == "0x")
                                                                     {
                                                                     bittype_value_present = HexToDecimal((char*)s_value.c_str(),s_value.length());
                                                                     }
                                                                    else
                                                                    {
                                                                      bittype_value_present = atoi((const char*)szAttr5[i]);
                                                                    }
                                                                     bittype_value_total += bittype_value_present*(pow(2.0,bitnum));
                                                                      int  bitnum_present     = atoi((const char*)szAttr7[i]);
                                                                      bitnum	 += bitnum_present ;//计算当前总bit数

                                                                      //*************************************看下一个是不是bit,如果不是则直接赋值，如果是，若总位数大于8，也直接赋值
                                                                      s_datatype=(const char *)szAttr8[i+1];
                                                                      if ( s_datatype == "bit")//下一个也是位类型
                                                                       {
                                                                          int next_bitnum=atoi((const char*)szAttr7[i+1]);
                                                                          if ( bitnum+next_bitnum>8)
                                                                               {  for (int i=0;i<variframe_map_struct->datalen;i++)
                                                                              {
                                                                                  printf("%x  ",(unsigned char)variframe_map_struct->data[i]);
                                                                              }
                                                                              printf("\n");
                                                                               memcpy( variframe_map_struct->data+dataoffset,&bittype_value_total,sizeof(char));//赋值
                                                                               dataoffset++;//偏移量+1
                                                                               bittype_value_total=0;//数据复位
                                                                               bittype_value_present=0;
                                                                               bitnum = 0;
                                                                               bytenum = 0;
                                                                            }
                                                                      }
                                                                      else   //下一个不是位类型
                                                                      {
                                                                       memcpy( variframe_map_struct->data+dataoffset,&bittype_value_total,sizeof(char));//赋值
                                                                      dataoffset++;//偏移量+1
                                                                      bittype_value_total=0;//数据复位
                                                                     bittype_value_present=0;
                                                                     bitnum = 0;
                                                                    bytenum = 0;
                                                                    }
                                                                 }
                                                             else      //不是位类型
                                                             {
                                                                 bytenum=atoi((const char*)szAttr7[i]);
                                                                 s_value =(const char*)szAttr5[i];
                                                                 //判断进制,并转化数值**************************************
                                                                if (s_value.substr(0,2) == "0x")
                                                               {
                                                               ULONGLONG datavalue = HexToDecimal((char*)s_value.c_str(),s_value.length());
                                                                memcpy( variframe_map_struct->data+dataoffset,&datavalue,bytenum);//赋值
                                                                dataoffset +=bytenum;
                                                                }
                                                                else
                                                                {
                                                                    ULONGLONG datavalue =atoi((const char*)szAttr5[i]);
                                                                    memcpy( variframe_map_struct->data+dataoffset,&datavalue,bytenum );//赋值
                                                                    dataoffset +=bytenum;
                                                                }
                                                             }
                                                        }
//                                                            for (int i=0;i<variframe_map_struct->datalen;i++)
//                                                                            {
//                                                                                printf("%x  ",(unsigned char)variframe_map_struct->data[i]);
//                                                                            }
//                                                                            printf("\n");
                                                          m_csoftbus_xml->VariFrameLink.insert(make_pair(n_varicode,variframe_map_struct));
                                                     }
                                                            //批量传输帧
                                                          for(int i=0;NodeGrand != NULL&&(!(strcmp(frame_1,frame_p)));NodeGrand = NodeGrand -> next)
                                                          {
                                                             xmlAttrPtr attrPtrNodeGrand =NodeGrand->properties;
                                                            for(;(attrPtrNodeGrand != NULL)&&(i<=0);attrPtrNodeGrand = attrPtrNodeGrand -> next)
                                                             {
                                                                     DatablockMapStruct * datablock_map_struct = (DatablockMapStruct *)malloc(sizeof(DatablockMapStruct));
                                                                     datablock_map_struct->srcID = atoi((const char*)szAttr6);
                                                                     datablock_map_struct->dstID = atoi((const char*)szAttr4);
                                                                     m_csoftbus_xml->DataBlockLink.insert(make_pair(datablock_map_struct->dstID,datablock_map_struct->srcID));
                                                                     i++;                                                                  
                                                             }
                                                       }
                                 }
            NodeChild = NodeChild -> next;//指向三级节点
 }
 Node = Node -> next;//遍历节点
}
           xmlFreeDoc(doc);//释放内存
           if(0 == Retrans_is_Configure)
           {
                   StrategyRepeat_istrue = false;
                   SendWindow = 0;
                   ACKWindow  = 0;
                   TimeDelay  = 0;
                   return true;
           }
           else
           {
               StrategyRepeat_istrue = true;
           }
        return  true;
}


ULONGLONG CXMLAnalyse::HexToDecimal(char* HexString,int StrLength)//十六进制转十进制
{
    char  revstr[22]={0};  //根据十六进制字符串的长度，这里注意数组不要越界
    unsigned long long   count=1;
    int   length=StrLength;
    unsigned long long   result=0;
    unsigned long long   temp=0;
    memcpy(revstr,HexString,length);
    for   (int i=length-1;i>=0;i--)
    {
        if ((revstr[i]>=48) && (revstr[i]<=57))
            temp=revstr[i]-48;//字符0的ASCII值为48
        else if ((revstr[i]>='a') && (revstr[i]<='f'))
            temp=revstr[i]-'a'+10;
        else if ((revstr[i]>='A') && (revstr[i]<='F'))
            temp=revstr[i]-'A'+10;
        else
        {
            if (revstr[i]=='x'||revstr[i]=='X'||revstr[i]==0)
            {
                temp=0;
            }
            else
            {
                printf("十六进制配置字符错误!");
                return 0;
            }
        }

        result=result+temp*count;

            count=count*16;//十六进制(如果是八进制就在这里乘以8)


    }
    return result;
}

































