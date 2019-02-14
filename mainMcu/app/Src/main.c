/*****************************************************************************
** @Author: quqian  
** @Date: 2018-12-05 14:20:14 
** @File: main.c
** @MCU: GD32f303VET6   
** @MCU max Speed: 120M
** @MCU Flash: 512K
** @MCU RAM: 64K
** @MCU Package: LQFP100
** @Last Modified by: quqian
** @Last Modified time: 2018-12-05 14:20:14 
*****************************************************************************/
#include "includes.h"
#include "main.h"
#include "BswSrv.h"
#include "APP.h"
#include "App_Main.h"
#include "BswSrv_ComTask.h"
#include "BswSrv_NFCard_Task.h"
#include "BswDrv_GPIO.h"
#include "BswSrv_System.h"
#include "App_CAN.h"
#include "BswDrv_Adc.h"
#include "BswDrv_Usart.h"
#include "BswDrv_Rtc.h"
#include "BswDrv_SPI.h"
#include "BswDrv_Sys_Flash.h"
#include "BswDrv_Timer.h"
#include "BswDrv_sc8042b.h"





//开始任务任务函数
void start_task(void *pvParameters)
{
	xTaskCreate((TaskFunction_t)App_MainTask,   "MainTask", 	384, NULL, 2, NULL);
//	xTaskCreate((TaskFunction_t)SurfNet_Task,   "SurfNet_Task", 512, NULL, 3, NULL);
//	xTaskCreate((TaskFunction_t)NFCardTask,	    "NFCardTask",	256, NULL, 4, NULL);   
//	xTaskCreate((TaskFunction_t)WifiBlueTask,	"BlueTask", 	512, NULL, 5, NULL); 
	xTaskCreate((TaskFunction_t)ComTask,		"ComTask", 	    384, NULL, 6, NULL); 
	
	vTaskDelete(NULL);
}

void BspInit(void)
{
	/*设置系统中断优先级分组4	*/
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

	/*看门狗初始化*/
	BswDrv_WatchDogInit();
	/*GPIO初始化*/
	BswDrv_GPIO_Init(); 
	/*串口初始化*/
	BswDrv_UsartInit();
	/*打印重启标志*/
	BswSrv_SystemResetRecord();
	/*RTC初始化*/
	BswDrv_RtcInit();
	/* SPI初始化 */
	BswDrv_SPI_Init();
	/*片内flash初始化*/
	BswDrv_SysFlashInit();
	/*定时器初始化 */
	BswDrv_Timer_Init();
	/*ADC初始化*/
	BswDrv_ADC_Init();
	/*语音模块初始化*/
	BswDrv_SC8042B_Init();
	/*CAN总线初始化*/
	BswDrv_CanBusInit();
	
	/*加载系统参数*/
	BswSrv_LoadSystemInfo();
    
    GlobalInfo.readCard_Callback = App_ReadCard_CallBack;
	GlobalInfo.AppCan_HandleCallBack = AppCan_CallBack;
	
    BswDrv_ADC_Start();
    LED_R_ON();
}

int main(void)
{	
	/* 设置中断向量表偏移地址 */
//	nvic_vector_table_set(NVIC_VECTTAB_FLASH, BOOT_SIZE);
	
	BspInit();
	
	//创建开始任务
	xTaskCreate((TaskFunction_t)start_task, "start_task", 128, NULL, 1, NULL);  

	vTaskStartScheduler();          //开启任务调度

	while(1)
    {
        vTaskDelay(1000);
    }
}





