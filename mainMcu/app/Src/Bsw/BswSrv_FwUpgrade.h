#ifndef __APP_FWUPGRADE_H__
#define __APP_FWUPGRADE_H__

#include "includes.h"

#define CARD_UPGRADE_SIZE       64

enum{
	FW_U8 = 0,  //U8����
	FW_U8_BAK,  //ˢ����
	DW_NUM,     //�̼�����
};

typedef enum
{
    UPGRADE_FTP = 0,
    UPGRADE_HTTP,
    UPGRADE_UART,
    UPGRADE_BLUE
}UPGRADE_TYPE;

#pragma pack(1)

typedef struct
{
    uint8_t  upgradeFlag;       //0-û��������Ϣ 1-��������Ϣ
    uint8_t  fwVer;             //�̼��汾
    uint16_t checkSum;          //У���
    uint32_t startAddrs;        //�̼��洢��ƫ�Ƶ�ַ
    uint32_t fsize;             //�̼���С
}FW_HEAD_INFO_T;

typedef struct{
	uint16_t headFlag;           //0x55AA
    FW_HEAD_INFO_T fw[DW_NUM];
}SYS_UPDATE_HEAD_INFO_T;

typedef struct{
    uint8_t  upgradeFlag;
	uint32_t fsize;			
	uint16_t checkSum;
    uint8_t  fw_version;
    uint32_t startAddrs;        //�̼��洢��ƫ�Ƶ�ַ
	uint16_t package_num;
	uint16_t current_package; 	
	uint8_t lastIndex;
    uint32_t wirteSize;
}UPGRADE_INFO_STR_T;

typedef struct{
    uint8_t  aa;
    uint8_t  five;
    uint8_t  fwCnt;
    uint8_t  fwVer1;
    uint8_t  fwVer2;
}FW_HEAD_STR;

typedef struct{
    uint32_t size;
    uint16_t checkSum;
    uint8_t  name[10];
}FW_INFO_STR;
#pragma pack()


void OTA_Start(UPGRADE_TYPE type);
void OAT_Init(uint8_t upgradeFlag,uint32_t startAddrs,uint32_t fileSize,uint16_t checkSum,uint8_t fwVer);
void OAT_FwWrite(uint8_t type,uint8_t *data,uint16_t len);
int OTA_Check(uint8_t type);
void OTA_Finish(uint8_t result,uint8_t type);

void BswSrv_Upgrade_SendNotify(uint8_t transAction);
void BswSrv_StartCardBoard_UpgradeTask(void);
int BswSrv_Upgrade_ReadHeadInfo(uint8_t type,FW_HEAD_INFO_T *info);

extern UPGRADE_INFO_STR_T upgradeInfo;

#endif

