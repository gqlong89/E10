#include "includes.h"
#include "BswDrv.h"
#include "BswDrv_Rtc.h"
#include "BswDrv_Usart.h"
#include "BswDrv_GPIO.h"
#include "BswDrv_Timer.h"
#include "BswDrv_SPI.h"
#include "BswDrv_IIC.h"
#include "BswDrv_IC_RF433.h"
#include "BswDrv_Sys_Flash.h"
#include "BswDrv_Delay.h"
#include "BswDrv_sc8042b.h"
#include "BswDrv_Adc.h"


void BswDrv_Init(void)
{	
	/*����ϵͳ�ж����ȼ�����4	*/
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
	
	/*GPIO��ʼ��*/
	BswDrv_GPIO_Init(); 
	
	/*���ڳ�ʼ��*/
	BswDrv_UsartInit();

	/*RCT��ʼ��*/
	BswDrv_RtcInit();
	
	/* SPI��ʼ�� */
	BswDrv_SPI_Init();

	/* IIC��ʼ�� */
	BswDrv_IIC_init();
	
	/*Ƭ��flash��ʼ��*/
	BswDrv_SysFlashInit();
	
	/*��ʱ����ʼ�� */
	BswDrv_Timer_Init();

	/*ACD��ʼ��*/
	BswDrv_ADC_Init();

	/*433ģ���ʼ��*/
	BswDrv_RF433_Init();

	/*����ģ���ʼ��*/
	BswDrv_SC8042B_Init();

}

