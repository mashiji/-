#ifndef HARDWAREETHERNET_H
#define HARDWAREETHERNET_H

#pragma once
#include <pcap.h>
//#include <libnet.h>
#include "SoftBusMsg.h"
#include "softBusSystem.h"
#include <map>
#include <malloc.h>
#include <fstream>
#include "HardwareAdapter.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <sched.h>
#include "FrameHead.h"

using namespace std;
class CHardwareAdapter;
#define  SOFT_BUS_HARDWARE_ETHERNET_MAX_LEN  1450
class   HardwareAdapter;


class   CHardwareEthernet
{
public:
            CHardwareEthernet();
            ~CHardwareEthernet();
public:
            CHardwareAdapter * m_phardwareadapter_ethernet;
public:
            char *        device;
            char  errbuf[PCAP_ERRBUF_SIZE];
            pcap_t  *   pcap_device;
            const struct pcap_pkthdr* pcap_pkthdr_ether;
            struct ether_header *eptr;  /* net/ethernet.h            */
            struct bpf_program fp;      /* hold compiled program     */
            unsigned char  srcmac[6];
            unsigned char  destmac[6];
            unsigned short  ipprotocol_type = 0x1111;
            Mac_Header*  mac_header;
            comHeader*   comheader;
public:
            bool    ethernetinitial(CHardwareAdapter * m_pHardwareAdapter);
            bool    transmit(CSoftBusMsg  *  msg);
};

#endif // HARDWAREETHERNET_H
