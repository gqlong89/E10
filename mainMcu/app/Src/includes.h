#ifndef __INCLUDES_H__
#define __INCLUDES_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include "gd32f30x.h"
#include "cmsis_os.h"
#include "main.h"



#define FW_VERSION                      1
#define FW_VERSION_SUB1                 0


#define     ID2                         0
#define     ENVI_TYPE                   1 //0-��ʽ 1-����  2-Ԥ����  3-����


#if ENVI_TYPE == 1
#define NET_SERVER_IP                   "test.access.chargerlink.com"
#define NET_SERVER_PORT                 10010
#define REGISTER_CODE                   "016987547845"
#elif ENVI_TYPE == 3
#define NET_SERVER_IP                   "47.97.238.64"
#define NET_SERVER_PORT                 10010
#define REGISTER_CODE                   "016987547845"
#endif

#define DEVICE_TYPE						14	//�豸���� -- U8
#define STATION_MACHINE_TYPE            "U8"


#define FLASH_SIZE                      0x40000     //�豸flash��С256K
#define FLASH_PAGE_SIZE					2048


#define CL_OK                           0
#define CL_FAIL                         (-1)
#define CL_TRUE                         1
#define CL_FALSE                        0


#define RF_DEV_MAX						20 //�̸��豸��������

#define TX_FAIL_MAX_CNT	                3

#define osGetTimes                      osKernelSysTick
#define GetRtcCount 	                GetTimeStamp /*(xTaskGetTickCount()*portTICK_PERIOD_MS/1000)*/
#define UNUSED(X) (void)X


extern char* GetCurrentTime(void);
#define CL_LOG(fmt,args...) do {    \
    printf("%s|[U8]:%s (%d)" fmt, GetCurrentTime(), __func__, __LINE__, ##args); \
}while(0)

#define PRINT_LOG(fmt,args...) do \
{    \
    printf("[%s]"fmt,GetCurrentTime(), ##args); \
}while(0)

#endif



