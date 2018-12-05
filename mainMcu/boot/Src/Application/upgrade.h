#ifndef __UPGRADE_H__
#define __UPGRADE_H__


#include <stdint.h>
#include "SysFlash.h"

enum{
	FW_U8 = 0,
	FW_U8_BAK,
	DW_NUM,
};

#pragma pack(1)

typedef struct{
	uint16_t headFlag;            //0x55AA
    struct{
        uint8_t  upgradeFlag;       //0-û��������Ϣ 1-��������Ϣ
        uint8_t  fwVer;             //�̼��汾
        uint16_t checkSum;          //У���
        uint32_t startAddrs;        //�̼��洢��ƫ�Ƶ�ַ
        uint32_t fsize;             //�̼���С
    }fw[DW_NUM]; 
}SYS_UPDATE_INFO_T;

#pragma pack()


int UpdateFromAppBkp(uint32_t startAddr,uint32_t fsize, uint16_t checksum);

#endif

