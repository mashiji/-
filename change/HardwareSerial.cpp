#include"HardwareSerial.h"
#include <cstring>
void *  serialreceive(void *  arg);

CHardwareSerial::CHardwareSerial()
{

}


CHardwareSerial::~CHardwareSerial()
{

}

bool CHardwareSerial::serial_initial(CHardwareAdapter *m_phardwareadapter)//初始化函数
{
        m_phardwareadapter_serial = m_phardwareadapter;
        fd = OpenDev(dev);//开设备
        fd1 = OpenDev(dev1);

          if (set_Parity(fd,9600,8,1,'N')== 0)//初始化串口
          {
            printf("Set Parity Error\n");
            exit(1);
          }
          if (set_Parity(fd1,9600,8,1,'N')== 0)
          {
            printf("Set Parity Error\n");
            exit(1);
          }

          pthread_create(&tid,NULL,serialreceive,(void *)this);//创建线程
          return true;
}

void CHardwareSerial::serial_send(CSoftBusMsg *msg)//发送函数
{
          msg_datalen = msg->dataLen;
          write(fd1,(unsigned char *)&msg_datalen,2);//先发送数据长度，2个字节
          write(fd1,msg->data,msg->dataLen);//在发送数据
}

void *  serialreceive(void *arg)//接收函数
{
        CHardwareSerial  *  m_pSerial = (CHardwareSerial *) arg;
        while(1)
        {
                unsigned char *  data_len_temp = (unsigned char *) malloc(2);//分配内存读取数据长度
                read(m_pSerial->fd,m_pSerial->buff,1);//先读一个字节的数据长度
                memcpy(data_len_temp,m_pSerial->buff,1);            
                read(m_pSerial->fd,m_pSerial->buff,1);
                memcpy(data_len_temp+1,m_pSerial->buff,1);//将两个字节的数据长度进行拼接              
                int  serial_data_start = 0;
                unsigned short   serial_data_len = *((unsigned  short*)(data_len_temp));
                free(data_len_temp);
                data_len_temp  =  NULL;
                char*  data_temp = (char *) malloc(sizeof(char) * serial_data_len);//分配内存读取数据
                memset(data_temp,0,serial_data_len);//清空缓存
                unsigned short flag=serial_data_len;//标志位
                int n = 0;
                do
                {
                        n = read(m_pSerial->fd,m_pSerial->buff,serial_data_len);//读取数据
                        memcpy(data_temp + serial_data_start,m_pSerial->buff,n);//进行数据拼接
                        serial_data_start += n;
                        serial_data_len -= n;
                }while(serial_data_len > 0);//判断循环
                CSoftBusMsg  *  msg  = new  CSoftBusMsg ();
                msg->dataCopy(data_temp,flag);
                m_pSerial->m_phardwareadapter_serial->receive(msg);
        }
}


int CHardwareSerial::set_Parity(int fd, int speed, int databits, int stopbits, int parity)//初始化
{
    int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300};//波特率设置
        int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200, 300};
     struct termios options;

     unsigned int   i;
        int   status;

        tcgetattr(fd, &options);
        for ( i= 0; i < sizeof(speed_arr) / sizeof(int); i++) {
        if (speed == name_arr[i]) {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &options);
        if (status != 0) {
                perror("tcsetattr fd1");
                return 0;
            }
            tcflush(fd,TCIOFLUSH);
            }
            }


    if ( tcgetattr( fd,&options) != 0)
    {
        perror("SetupSerial 1");
        return(FALSE);
    }
    options.c_cflag &= ~CSIZE;
    switch (databits) //设置数据位数
    {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr,"Unsupported data size\n");
            return (FALSE);
        }
    switch (parity)
        {
        case 'n':
        case 'N':   
            options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /*Input*/
            options.c_oflag &= ~OPOST;   /*Output*/
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
            options.c_iflag |= INPCK;             /* Disnable parity checking */
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;     /* Enable parity */
            options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S':
        case 's': /*as no parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            fprintf(stderr,"Unsupported parity\n");
            return (FALSE);
            }
    /* 设置停止位*/
    switch (stopbits)
        {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            fprintf(stderr,"Unsupported stop bits\n");
            return (FALSE);
        }
    /* Set input parity option */
    if ((parity != 'n')&&(parity != 'N'))
            options.c_iflag |= INPCK;

        options.c_cc[VTIME] = 5; // 0.5 seconds
        options.c_cc[VMIN] = 1;

        options.c_cflag &= ~HUPCL;
        options.c_iflag &= ~INPCK;
        options.c_iflag |= IGNBRK;
        options.c_iflag &= ~ICRNL;
        options.c_iflag &= ~IXON;
        options.c_lflag &= ~IEXTEN;
        options.c_lflag &= ~ECHOK;
        options.c_lflag &= ~ECHOCTL;
        options.c_lflag &= ~ECHOKE;
        options.c_oflag &= ~ONLCR;

    tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0)
        {
            perror("SetupSerial 3");
            return (FALSE);
        }

    return (TRUE);
}


int CHardwareSerial::OpenDev(char *Dev)//打开串口
{
        int	fd = open( Dev, O_RDWR );         //| O_NOCTTY | O_NDELAY
        if (-1 == fd)
            { /*设置数据位数*/
                perror("Can't Open Serial Port");
                return -1;
            }
        else
            return fd;
}

