#ifndef HARDWARESERIAL_H
#define HARDWARESERIAL_H

#define FALSE 0
#define TRUE 1

#include <iostream>      /*标准输入输出定义*/
#include <stdlib.h>     /*标准函数库定义*/
#include <unistd.h>     /*Unix标准函数定义*/
#include <termios.h>    /*PPSIX终端控制定义*/
#include <errno.h>      /*错误号定义*/
#include <fcntl.h>      /*文件控制定义*/
#include  <sys/types.h>  /**/
#include  <sys/stat.h>   /**/
#include "HardwareAdapter.h"
#include "SoftBusMsg.h"
#include <pthread.h>


class CHardwareAdapter;
class CSoftBusMsg;

class CHardwareSerial
{
public:
    CHardwareSerial();
    ~CHardwareSerial();
public:
    CHardwareAdapter * m_phardwareadapter_serial;
public:
    int fd;
    int fd1;
    int nread;
    char buff[512];
    char *dev ="/dev/ttyM0";
    char *dev1="/dev/ttyM1";

    unsigned short  msg_datalen; 
    bool   serial_initial(CHardwareAdapter * m_phardwareadapter);
    void   serial_send(CSoftBusMsg *  msg);//发送
    pthread_t   tid;

   
    int   set_Parity(int fd,int speed,int databits,int stopbits,int parity);//对串口进行初始化
    int   OpenDev(char *Dev);//开启串口
//    int   send_data(const int fd, const  char *buffer, const int buffer_len);
//    int   receive_data(const int fd, char *read_buffe,const int buffer_lenr);
};

#endif // HARDWARESERIAL_H
