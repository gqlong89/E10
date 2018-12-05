#ifndef __BSWSRV_RF433_H__
#define __BSWSRV_RF433_H__


#include <stdint.h>
#include <time.h>

/**
 *烟感设备基本数据结构
 */
typedef struct 
{
	uint8_t num;		//编号
	uint32_t address;	//设备地址 
	uint8_t flag;		//报警标志 0--无报警   1-有报警
	time_t time;		//上次报警时间
}RF_Unit_Typedef;



/**
 *
 */
typedef struct
{	
	RF_Unit_Typedef device[20]; 	//最大绑定个数
	uint16_t bandSize;						//绑定设备数量
}RFDev_Reg_t;


int BswSrv_RF433_BindDevice(uint8_t num,uint32_t address);
int BswSrv_RF433_GetWaringDevice(RF_Unit_Typedef *dev);
void BswSrv_RF433_ClearFlag(uint32_t address);

void BswSrv_RF433Init(void);


#endif
