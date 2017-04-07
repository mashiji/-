#include "HardwareEthernet.h"
#include<string.h>

void    getpacket(u_char * arg,const struct pcap_pkthdr* pcap_pkthdr,const u_char * packet);
void *  ReceiveData(void * arg);
CHardwareEthernet::CHardwareEthernet()
{
}


CHardwareEthernet::~CHardwareEthernet()
{
}


bool CHardwareEthernet::ethernetinitial(CHardwareAdapter * m_pHardwareAdapter)//初始化函数
{
            mac_header = (Mac_Header*)malloc(sizeof(Mac_Header));//分配内存
            comheader  = (comHeader*)malloc(sizeof(comHeader));//分配内存
    //reading  a  file  and  construct  a map
            m_phardwareadapter_ethernet = m_pHardwareAdapter;
            destmac [0]= 0x14;
            destmac [1]= 0xcf;
            destmac [2]= 0x92;
            destmac [3]= 0xef;
            destmac [4]= 0x3d;
            destmac [5]= 0xdb;
            mac_header->destmac[0] = destmac[0];
            mac_header->destmac[1] = destmac[1];
            mac_header->destmac[2] = destmac[2];
            mac_header->destmac[3] = destmac[3];
            mac_header->destmac[4] = destmac[4];
            mac_header->destmac[5] = destmac[5];
            mac_header->ippro_type = ipprotocol_type;
            device = pcap_lookupdev(errbuf);//查找设备

            pcap_device = pcap_open_live(device,BUFSIZ,1,-1,errbuf);//打开网口

//            struct   sched_param  sch;
//            pthread_attr_t  attr;
//               pthread_t   tid;
//            pthread_attr_init(&attr);
//            pthread_attr_getschedparam(&attr,&sch);
//            sch.__sched_priority = 99;
//            pthread_attr_setschedparam(&attr,&sch);
//            pthread_create(&tid,NULL,ReceiveData,(void *)this);
            /*construct a filter*/
            struct bpf_program filter;
            pcap_compile(pcap_device,&filter,"ether proto 0x1111",1,0);//设置过滤器
            pcap_setfilter(pcap_device,&filter);

            pthread_t  tid;
            pthread_create(&tid,NULL,ReceiveData,(void *) this);//创建线程
            //pcap_loop(pcap_device,-1,getpacket,(u_char *)(this->m_phardwareadapter_ethernet));
        return true;
}


bool CHardwareEthernet::transmit(CSoftBusMsg  *  msg)//发送函数
{
    //查找映射表，来获取对应的源地址
    srcmac[0] = 0x00;
    srcmac[1] = 0x21;
    srcmac[2] = 0x9b;
    srcmac[3] = 0x6b;
    srcmac[4] = 0x72;
    srcmac[5] = 0x9c;
    //利用msg->data和msg->datalen，以及源地址、目的地址、协议类型(0x11)构造mac帧
    //并修改msg->data和msg->datalen，然后发送出去00:21:9b:6b:72:9c
    mac_header->srcmac[0] = srcmac[0];
    mac_header->srcmac[1] = srcmac[1];
    mac_header->srcmac[2] = srcmac[2];
    mac_header->srcmac[3] = srcmac[3];
    mac_header->srcmac[4] = srcmac[4];
    mac_header->srcmac[5] = srcmac[5];

    while(msg->dataLen > SOFT_BUS_HARDWARE_ETHERNET_MAX_LEN)
    {
            comheader->len = SOFT_BUS_HARDWARE_ETHERNET_MAX_LEN + 4;
            comheader->moreFrag = 1;
            char * data_temp = (char *) malloc(14 + sizeof(comHeader) + SOFT_BUS_HARDWARE_ETHERNET_MAX_LEN);
            memcpy(data_temp,(const void*)mac_header,14);
            memcpy(data_temp + 14,comheader,sizeof(comHeader));
            memcpy(data_temp + 14 + sizeof(comHeader),msg->data,SOFT_BUS_HARDWARE_ETHERNET_MAX_LEN);
            pcap_sendpacket(pcap_device, (const u_char*)data_temp , SOFT_BUS_HARDWARE_ETHERNET_MAX_LEN + 14 + sizeof(comHeader)) ;//发送
            delete(data_temp);
            data_temp = NULL;
            msg->data = msg->data + SOFT_BUS_HARDWARE_ETHERNET_MAX_LEN;
            msg->dataLen = msg->dataLen - SOFT_BUS_HARDWARE_ETHERNET_MAX_LEN;
            continue;
    }

    comheader->len = msg->dataLen+4;
    comheader->moreFrag = 0;

    char * data_temp = (char *) malloc(14 + sizeof(comHeader) + msg->dataLen);
    memcpy(data_temp,(const void*)mac_header,14);
    memcpy(data_temp + 14,comheader,sizeof(comHeader));
    memcpy(data_temp + 14 + sizeof(comHeader),msg->data,msg->dataLen);

//    for(int z = 0; z <msg->dataLen + 14+sizeof(comHeader);z++ )
//            printf("%02x  ", *(unsigned char *)(data_temp + z));
//            printf("\n");

    pcap_sendpacket(pcap_device, (const u_char*)data_temp , msg->dataLen + 14 + sizeof(comHeader)) ;//发送
    delete(data_temp);
    data_temp = NULL;
    return true;
}

void *  ReceiveData(void * arg)//接收函数
{
        CHardwareEthernet  *  m_phardwareethernet = (CHardwareEthernet *) arg;

//        struct bpf_program filter;
//        pcap_compile(m_phardwareethernet->pcap_device,&filter,"ether proto 0x1111",1,0);
//        pcap_setfilter(m_phardwareethernet->pcap_device,&filter);
        pcap_loop(m_phardwareethernet->pcap_device,-1,getpacket,(u_char*)m_phardwareethernet->m_phardwareadapter_ethernet);//接收
}


//回调函数
void getpacket(u_char * arg,const struct pcap_pkthdr* pcap_pkthdr,const u_char * packet)
{
    CHardwareAdapter * m_pHardwareAdapter = (CHardwareAdapter *) arg;
    CSoftBusMsg* ether_msg = new CSoftBusMsg();
    ether_msg->dataCopy((char*) packet,pcap_pkthdr->len);
    m_pHardwareAdapter->receive(ether_msg);
}




