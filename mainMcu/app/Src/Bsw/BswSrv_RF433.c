/*
 * @Author: zhoumin 
 * @Date: 2018-10-12 14:15:14 
 * @Last Modified by:   zhoumin 
 * @Last Modified time: 2018-10-12 14:15:14 
 */
#include "BswSrv_System.h"
#include "BswSrv_RF433.h"
#include "BswDrv_IC_RF433.h"
#include "BswDrv_Rtc.h"
#include "BswSrv_FlashUsr.h"


static RFDev_Reg_t RFDev_Reg;


/**
 *�������ص�����
 *
 */
void RF433_WaringOccurred_CallBack(uint32_t address)
{
	uint16_t bindSize = RFDev_Reg.bandSize;
	
	for(int i = 0; i < bindSize;i++){
		
		if(address == RFDev_Reg.device[i].address)
		{
			RFDev_Reg.device[i].flag = 1;
			RFDev_Reg.device[i].time = GetRtcCount();
			
			return ;
		}
	}
}


/**
 *��ȡ�����豸��Ϣ(��Ҫ�������ж�ʱ���ã����ú���ձ�־)
 *
 */
int BswSrv_RF433_GetWaringDevice(RF_Unit_Typedef *dev)
{
	uint16_t bindSize = RFDev_Reg.bandSize;
	for(int i = 0; i < bindSize;i++){
		
		if(1 == RFDev_Reg.device[i].flag)
		{
			memcpy((void*)dev,(void*)&RFDev_Reg.device[i],sizeof(RF_Unit_Typedef));
			return CL_OK;
		}
	}
	
	return CL_FAIL;
}


/**
 *�����������־
 *
 */
void BswSrv_RF433_ClearFlag(uint32_t address)
{
	uint16_t bindSize = RFDev_Reg.bandSize;
	
	for(int i = 0; i < bindSize;i++){
		
		if(address == RFDev_Reg.device[i].address)
		{
			RFDev_Reg.device[i].flag = 0;
			
			return ;
		}
	}
}


/**
 *�豸��
 *
 */
int BswSrv_RF433_BindDevice(uint8_t num,uint32_t address)
{
	uint16_t bindSize = RFDev_Reg.bandSize;
	
	if(bindSize >= RF_DEV_MAX)
	{
		return CL_FAIL;
	}
	
	for(int i = 0; i < bindSize;i++){
		
		if(address == RFDev_Reg.device[i].address)
		{
			return CL_OK; //�Ѿ��󶨳ɹ�
		}
	}
	
	RFDev_Reg.device[bindSize-1].flag = 0;
	RFDev_Reg.device[bindSize-1].num = num;
	RFDev_Reg.device[bindSize-1].address = address;
	RFDev_Reg.device[bindSize-1].time = 0;
	
	RFDev_Reg.bandSize += 1;//�豸��������1
	
	//�����豸��Ϣ��flash
	SystemInfo.RfDev.Unit[bindSize-1].address = address;
	SystemInfo.RfDev.Unit[bindSize-1].num = num;
	SystemInfo.RfDev.bandSize++;
	FlashWriteSysInfo(&SystemInfo,sizeof(SystemInfo));

	return CL_OK;
}



void BswSrv_RF433Init(void)
{
	memset(&RFDev_Reg,0,sizeof(RFDev_Reg_t));	
	//�������ļ������̸а���Ϣ
	RFDev_Reg.bandSize = SystemInfo.RfDev.bandSize;

	for(uint16_t i = 0;i<RFDev_Reg.bandSize;i++)
	{
		RFDev_Reg.device[i].address = SystemInfo.RfDev.Unit[i].address;
		RFDev_Reg.device[i].num = SystemInfo.RfDev.Unit[i].num;
		RFDev_Reg.device[i].flag = 0;
	}
	
}

