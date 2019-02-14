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





//��ʼ����������
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
	/*����ϵͳ�ж����ȼ�����4	*/
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

	/*���Ź���ʼ��*/
	BswDrv_WatchDogInit();
	/*GPIO��ʼ��*/
	BswDrv_GPIO_Init(); 
	/*���ڳ�ʼ��*/
	BswDrv_UsartInit();
	/*��ӡ������־*/
	BswSrv_SystemResetRecord();
	/*RTC��ʼ��*/
	BswDrv_RtcInit();
	/* SPI��ʼ�� */
	BswDrv_SPI_Init();
	/*Ƭ��flash��ʼ��*/
	BswDrv_SysFlashInit();
	/*��ʱ����ʼ�� */
	BswDrv_Timer_Init();
	/*ADC��ʼ��*/
	BswDrv_ADC_Init();
	/*����ģ���ʼ��*/
	BswDrv_SC8042B_Init();
	/*CAN���߳�ʼ��*/
	BswDrv_CanBusInit();
	
	/*����ϵͳ����*/
	BswSrv_LoadSystemInfo();
    
    GlobalInfo.readCard_Callback = App_ReadCard_CallBack;
	GlobalInfo.AppCan_HandleCallBack = AppCan_CallBack;
	
    BswDrv_ADC_Start();
    LED_R_ON();
}

int main(void)
{	
	/* �����ж�������ƫ�Ƶ�ַ */
//	nvic_vector_table_set(NVIC_VECTTAB_FLASH, BOOT_SIZE);
	
	BspInit();
	
	//������ʼ����
	xTaskCreate((TaskFunction_t)start_task, "start_task", 128, NULL, 1, NULL);  

	vTaskStartScheduler();          //�����������

	while(1)
    {
        vTaskDelay(1000);
    }
}





