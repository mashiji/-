//#include <iostream>
//#include <stdio.h>
//#include <iostream>
//#include "CSoftBus.h"
//#include <pthread.h>
//#include <unistd.h>
//#include "conditioner_com.h"

//using namespace std;

//void * func1(void * arg);

//void * func1(void * arg)
//{
//    CSoftBus* csoftbus = (CSoftBus*) arg;
//        while(1)
//        {
//        usleep(10000);
//        if(csoftbus->msg_type == 0 && csoftbus->receive_data_break)
//        {
//              if(csoftbus->order_code == 400)
//              {
//                  UPtemp * uptemp =  (UPtemp * )csoftbus->receive_data;
//                  printf("命令帧------------------------：\n");
//                printf("moment : %lld\n",uptemp->moment);
//             printf("range: %d\n",uptemp->range);
//          for(int k = 0; k <csoftbus->receive_data_len; k++)
//                     printf("%02x \t",(unsigned char )*(csoftbus->receive_data + k));
//          printf("\n");
//                  csoftbus->receive_data_break = false;
//                  continue;
//              }
//              if(csoftbus->order_code == 4||csoftbus->order_code == 5)
//              {
//                  csoftbus->receive_data_break = false;
//                  continue;
//              }
//            }
//        else if(csoftbus->msg_type == 1 && csoftbus->receive_data_break)
//        {
//              if(csoftbus->vari_code == 100)
//              {
//                  AMPlitude * amplitude = (AMPlitude *)csoftbus->receive_data;
//                  printf("数据帧------------------------：\n");
//                  printf("width : %d\n",amplitude->width);
//                  printf("height: %d\n",amplitude->height);
//                  printf("thickness: %lld\n",amplitude->thickness);
//          for(int k = 0; k <csoftbus->receive_data_len; k++)
//                      printf("%02x \t",(unsigned char )*(csoftbus->receive_data + k));
//          printf("\n");
//                  csoftbus->receive_data_break = false;
//                  continue;
//              }
//            }
//       else if(csoftbus->msg_type == 2 && csoftbus->receive_data_break)
//       {
//                printf("批量传输帧------------------------：\n");
//                for(int k = 0; k <csoftbus->receive_data_len; k++)
//                printf("%02x \t",(unsigned char )*(csoftbus->receive_data + k));
//                printf("\n");
//                csoftbus->receive_data_break = false;
//                continue;
//       }
//         csoftbus->receive_data_break=false;
//        }
//}

int main()
{
        CSoftBus*  csoftbus = new CSoftBus();
        csoftbus->initial("/home/msj/1.xml");
        // 开启接收线程进行监听
        csoftbus->softBusReceive();
        pthread_t  tid_main;
        pthread_create(&tid_main,NULL,func1,(void *)csoftbus);

        char tempdown[9]={0x59,0xE9,0x7b,0x20,0x22,0x37,0xd5,0xdf,0x7d};
    UPtemp * uptemp_datasend = new UPtemp;
    uptemp_datasend->moment = 128;
    uptemp_datasend->range = 64;
    AMPlitude * amplitude_datasend = new AMPlitude;
    amplitude_datasend->width = 12;
    amplitude_datasend->height = 58;
    amplitude_datasend->thickness = 64;

    printf("***********************************Demo***********************************");
    printf("***********共有以下十种指令***********\n");
    printf("1. 升温: (命令帧)moment 128; range 64\n");
    printf("2. 幅度: (数据帧)width 12; height 58;thickness 64 \n");
    printf("3. 降温: (批量传输帧) 0x59,0xE9,0x7b,0x20,0x22,0x37,0xd5,0xdf,0x7d\n");
    printf("请输入发送的序列号（0-3）:\n");
    printf("输入‘q’退出\n");
    char  user_input;
    user_input = getchar();
    do
    {
        if(user_input == '0')
            csoftbus->SingleLinkDetect(5,10);
      else  if(user_input == '1')
            csoftbus->commandSend((char*)uptemp_datasend,sizeof(UPtemp),400);
        else if	(user_input == '2')
            csoftbus->variableSend((char*)amplitude_datasend,sizeof(AMPlitude),100);
        else if(user_input == '3')
            csoftbus->DatablockSend(tempdown,9,0,0,10,1);
        user_input = getchar();
    }while(user_input != 'q');

        return 0;
}



#include "XMLAnalyse.h"
#include <iostream>
#include <stdio.h>
#include <iostream>
#include "CSoftBus.h"
#include <pthread.h>
#include <unistd.h>

using namespace std;

void * func1(void * arg);

void * func1(void * arg)
{
    CSoftBus* csoftbus = (CSoftBus*) arg;
       int k=0;
        while(1)
        {

            if(csoftbus->receive_data_break)
            {
                k++;
             printf("第%d\n",k);
              for(int i = 0; i < csoftbus->receive_data_len; i ++)
             {
                printf("%02x ",*(csoftbus->receive_data + i));
            }
             printf("\n");
                csoftbus->receive_data_break = false;
                continue;
            }
        }
}

int main()
{
        CSoftBus*  csoftbus = new CSoftBus();
        csoftbus->initial("/home/msj/1.xml");
        // 开启接收线程进行监听
        csoftbus->softBusReceive();



        pthread_t  tid_main;
        pthread_create(&tid_main,NULL,func1,(void *)csoftbus);

        char a[13] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d};

        for (int i = 0; i < 1000; i++)
        {
                csoftbus->commandSend(NULL,0,400);
                csoftbus->variableSend(NULL,0,100);
                csoftbus->DatablockSend(a,13,1,18,5,0);
        }


        for (int j = 0; j < 200000000; j++)
        {
            sleep(1);
        }

        printf("Success\n");

        return 0;
}
