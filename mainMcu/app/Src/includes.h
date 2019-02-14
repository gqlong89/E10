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
#include "BswDrv_Rtc.h"
#include "BswDrv_Watchdog.h"


#define FW_VERSION                      1
#define FW_VERSION_SUB1                 0


#define     ID2                         0
#define     ENVI_TYPE                   1 //0-正式 1-测试  2-预发布  3-开发


#if ENVI_TYPE == 1
#define NET_SERVER_IP                   "test.access.chargerlink.com"
#define NET_SERVER_PORT                 10010
#define REGISTER_CODE                   "016987547845"
#elif ENVI_TYPE == 3
#define NET_SERVER_IP                   "47.97.238.64"
#define NET_SERVER_PORT                 10010
#define REGISTER_CODE                   "016987547845"
#endif

#define DEVICE_TYPE						14	//设备类型 -- U8
#define STATION_MACHINE_TYPE            "E10"
#define CHARGER_NAME                    "E10"

#define FLASH_SIZE                      0x80000     //设备flash大小512K
#define FLASH_PAGE_SIZE					2048


#define CL_OK                           0
#define CL_FAIL                         (-1)
#define CL_TRUE                         1
#define CL_FALSE                        0


#define RF_DEV_MAX						20 //烟感设备最大绑定数量


#define osGetTimes                      osKernelSysTick
#define GetRtcCount 	                GetTimeStamp /*(xTaskGetTickCount()*portTICK_PERIOD_MS/1000)*/
#define UNUSED(X) 						(void)X
#define FeedWatchDog					BswDrv_FeedWatchDog
#define GetPktSum                       BswSrv_Tool_CheckSum
#define OS_DELAY_MS                     vTaskDelay
#define UsartGetOneData                 BswDrv_UsartGetOneData


#define ORDER_SECTION_LEN               10
#define GUN_NUM_MAX				        1
#define OUT_NET_PKT_LEN                 (1024+32)
#define EVEN_DISCRI_LEN                 32

#define CHARGER_TYPE                    15  //15:X10F   1:X9  2:X10

extern char* GetCurrentTime(void);
#define CL_LOG(fmt,args...) do {    \
    printf("%s|[U8]:%s (%d)" fmt, GetCurrentTime(), __func__, __LINE__, ##args); \
}while(0)

#define PRINT_LOG(fmt,args...) do \
{    \
    printf("[%s]"fmt,GetCurrentTime(), ##args); \
}while(0)

#endif



