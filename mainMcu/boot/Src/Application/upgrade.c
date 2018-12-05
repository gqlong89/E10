#include "includes.h"
#include "upgrade.h"



uint8_t FlashBuffer[FLASH_PAGE_SIZE] = {0};
	
SYS_UPDATE_INFO_T updateInfo;



int UpdateFromAppBkp(uint32_t startAddr,uint32_t fsize, uint16_t checksum)
{
	uint16_t ck = 0;
	uint16_t page = fsize/FLASH_PAGE_SIZE + 1;
	
	startAddr += AppUpBkpAddr;	//��������U8����������ַ
	
	printf("startAddr=%X fsize=%d checksum=%X \r\n",startAddr,fsize,checksum);
	
	//��ⱸ������������Ƿ���ȷ
	for(uint32_t i = 0;i<fsize;i++)
	{
		ck += REG8(startAddr+i);
	}
	
	if(ck != checksum)
	{
		printf("�����������ݼ��ʧ��..\r\n");
		return CL_FAIL;
	}
	
	printf("���ڿ������ݵ�APP����.\r\n");

	for(uint16_t i = 0; i < page; i++)
	{	
		//����app����
		BswDrv_SysFlashErase(AppFlashAddr + i * FLASH_PAGE_SIZE);
		
		//�ӱ�������ȡ����
		BswDrv_SysFlashRead(startAddr + i * FLASH_PAGE_SIZE,FlashBuffer,FLASH_PAGE_SIZE);
		
		//д���ݵ�app����
		BswDrv_SysFlashWrite(AppFlashAddr + i * FLASH_PAGE_SIZE,FlashBuffer,FLASH_PAGE_SIZE);
		
	}
	
	ck = 0;
	//���APP���������
	for(uint32_t i = 0;i<fsize;i++)
	{
		ck += REG8(AppFlashAddr+i);
	}
	
	if(ck != checksum)
	{
		printf("App checksum fail, update failed!\r\n");
		return CL_FAIL;
	}
	
	printf("checksum ok, update success!\r\n");
	
	//�޸�ͷ��Ϣ
	memset(&updateInfo.fw[FW_U8],0,sizeof(updateInfo.fw[FW_U8]));
	
	BswDrv_SysFlashErase(SysUpInfoAddr);
	BswDrv_SysFlashWrite(SysUpInfoAddr,(void*)&updateInfo,sizeof(updateInfo));
	
	return CL_OK;
}
