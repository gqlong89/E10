#include "BswSrv.h"
#include "BswSrv_System.h"
#include "BswDrv_SPI_Fm175xx.h"
#include "BswDrv_IC_RF433.h"
#include "BswSrv_RF433.h"
#include "BswSrv_Air720.h"
#include "BswSrv_WG215.h"
#include "BswSrv_CardBoard.h"
#include "BswSrv_Key.h"


void BswSrv_Init(void)
{
	/*����ϵͳ����*/
	BswSrv_LoadSystemInfo();

	/*433ģ�����ӿڳ�ʼ��*/
	BswSrv_RF433Init();

	/*4Gģ���ʼ��*/
	BswSrv_Air720_Init();

	/*wifiģ���ʼ��*/
	BswSrv_WG215_Init();
	
	/*ˢ����*/
	BswSrv_CB_Init();

	BswSrv_Key_Init();
	
    /*��ӡ������־*/
	BswSrv_SystemResetRecord();
}


