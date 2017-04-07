#include "HardwareCANbus.h"

#define CHECK_POINT 5000
#define FRAMES_PER_CHANNEL 100000
#define DEV_TYPE VCI_PCI9820

#if DEV_TYPE == VCI_PCI9810
#define CHANNELS 1
#elif DEV_TYPE == VCI_PCI9820
#define CHANNELS 2
#elif DEV_TYPE == VCI_PCI9820I
#define CHANNELS 2
#elif DEV_TYPE == VCI_PCI9840
#define CHANNELS 4
#else
#error "invalid device type"
#endif

#define DUMP_TX_FRAMES 0
#define DUMP_RX_FRAMES 0

#define TRUE 1
#define FALSE 0

void  *  rx_func(void  *  param);

typedef struct {
    int channel;
    int stop;
} RX_PARAM;

 CHardwareCANbus:: CHardwareCANbus()
 {
 }

 CHardwareCANbus::~ CHardwareCANbus()
 {
 }

bool  CHardwareCANbus::canbus_initial(CHardwareAdapter *m_phardwareadapter)//初始化
{
    if(VCI_OpenDevice(VCI_PCI9820,0,0)!=1)//开启CAN卡
            {
                printf("open deivce error\n");
                VCI_CloseDevice(VCI_PCI9820,0);
            }
            VCI_INIT_CONFIG config;
            config.AccCode=0;
            config.AccMask=0xffffffff;
            config.Filter=0x00;
            config.Mode=0;

            config.Timing0=0x00;
            config.Timing1=0x14;

            if(VCI_InitCAN(VCI_PCI9820,0,1,&config)!=1)//初始化
                {
                    printf("init CAN error\n");
                    VCI_CloseDevice(VCI_PCI9820,0);
                }

                if(VCI_StartCAN(VCI_PCI9820,0,1)!=1)//开启CAN卡
                {
                    printf("Start CAN error\n");
                    VCI_CloseDevice(VCI_PCI9820,0);
                }

        m_pHardwareadapter_CAN = m_phardwareadapter;
        pthread_t  threadid;
        pthread_create(&threadid,NULL,rx_func,(void  *)m_pHardwareadapter_CAN);//创建线程
        return true;
}

void  CHardwareCANbus::canbus_send(CSoftBusMsg *msg)//发送函数
{
        //the last  frame :ID[0] = 0xaa
        //other  frames : ID[0] = 0x00;
        int  send_data_start_location = 0 ;
        if(msg->dataLen > 8)
        {
                do
                {
                        VCI_CAN_OBJ  *can_data_send  =  new  VCI_CAN_OBJ;
                        switch(msg->type)
                        {
                            case SOFT_BUS_TYPE_COMMAND:
                            {
                                 can_data_send->ID[0] = 0xaa;
                                break;
                            }

                           case SOFT_BUS_TYPE_VARIABLE:
                            {
                                    can_data_send->ID[0] = 0xbb;
                                    break;
                            }
                           case SOFT_BUS_TYPE_DATABLOCK:
                            {
                                can_data_send->ID[0] = 0xcc;
                                break;
                           }
                           default:
                                break;
                        }
                        can_data_send->ID[1] = 0x00;
                        can_data_send->ID[2] = 0x00;
                        can_data_send->ID[3] = 0x00;
                        can_data_send->SendType = 0;
                        can_data_send->RemoteFlag = 0;
                        can_data_send->ExternFlag = 0;
                        can_data_send->DataLen = 8;
                        for(int i = 0; i < 8; i++)
                                can_data_send->Data[i] = msg->data[send_data_start_location * 8 + i];
                        send_data_start_location += 1;
                        msg->dataLen -= 8;
                        if(!VCI_Transmit(VCI_PCI9820,0,1,can_data_send,1))
                                printf("PCI-CAN send data Failed! \n");
                       //usleep(1000);
                        delete(can_data_send);
                        can_data_send = NULL;
                }while(msg->dataLen > 8);
        }

        if(msg->dataLen != 0)
        {
                VCI_CAN_OBJ  *can_data_send  =  new  VCI_CAN_OBJ;
                switch(msg->type)
                {
                    case SOFT_BUS_TYPE_COMMAND:
                     can_data_send->ID[0] = 0Xaa;
                     break;

                   case SOFT_BUS_TYPE_VARIABLE:
                   can_data_send->ID[0] =0Xbb;
                   BYTE	Reserve0xbb;
                   break;

                   case SOFT_BUS_TYPE_DATABLOCK:
                     can_data_send->ID[0] = 0xcc;
                   break;
                   default:
                    break;
                }
                can_data_send->ID[1] = 0x01;
                can_data_send->ID[2] = 0x00;
                can_data_send->ID[3] = 0x00;
                can_data_send->SendType = 0;
                can_data_send->RemoteFlag = 0;
                can_data_send->ExternFlag = 0;
                can_data_send->DataLen = msg->dataLen;
                for(int i = 0; i < msg->dataLen; i++)
                        can_data_send->Data[i] = msg->data[send_data_start_location * 8 + i];
                if(!VCI_Transmit(VCI_PCI9820,0,1,can_data_send,1))
                        printf("PCI-CAN send data Failed! \n");
                //usleep(1000);
                delete(can_data_send);
                can_data_send = NULL;
        }

        return;
}


void * rx_func(void *  param)//线程函数
{
        
        CHardwareAdapter  *  m_pHardwareadapter_can = (CHardwareAdapter *) param;
        int reclen=0;
        VCI_CAN_OBJ rec[1];
        while(1)
        {

             int  frame_num = 0;
             char *  data_temp = (char *) malloc(1000);
             memset(data_temp,0,1000);
            while((reclen=VCI_Receive(VCI_PCI9820,0,1,rec,100,1))>0)//接收
            {
                  //对数据包进行重组
                                 
                        if(rec[0].ID[1] != 0x01)
                        {
                                 memcpy(data_temp + frame_num *8,rec[0].Data,8);
                                 frame_num ++;
                        }
                        else
                        {
                                 memcpy(data_temp + frame_num*8,rec[0].Data,rec[0].DataLen);
                                 CSoftBusMsg *  msg = new CSoftBusMsg();
                                 msg->dataCopy(data_temp,frame_num*8+rec[0].DataLen);
                                 free(data_temp);
                                 data_temp = NULL;
                                 m_pHardwareadapter_can->receive(msg);
                               break;
                        }
              }
        }
}
