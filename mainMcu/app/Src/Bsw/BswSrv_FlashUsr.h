#ifndef __FLASH_USR_H__
#define __FLASH_USR_H__

#include "includes.h"
#include "BswDrv_Sys_Flash.h"


typedef enum{
	FIRMWARE_PART = 0   ,
	SYSCONF_PART        ,
	UPGRADEHEAD_PART    ,
    RESEVERD_PART       ,
	PART_NUM            ,
}PARTITION;

typedef struct {
    uint32_t s_base;  		//start addr
    uint16_t s_count; 		//section count
}FLASH_PART;



extern void FlashErase(PARTITION n);
extern int FlashWriteSysInfo(void *pSysInfo, uint16_t size);
extern void FlashReadSysInfo(void *pSysInfo, uint16_t size);
extern void WriteUpdateInfo(uint32_t fsize, uint32_t checkSum);
extern void FlashWriteAppBackup(uint32_t app_backup_record_addr, uint8_t* buffer, uint16_t len);

#endif




