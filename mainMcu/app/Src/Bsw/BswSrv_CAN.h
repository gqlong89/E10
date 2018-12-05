#ifndef __BSW_SRV_CAN_H__
#define __BSW_SRV_CAN_H__


#include "includes.h"
#include "CAN_Config.h"
#include "BswDrv_CAN.h"



typedef struct
{
	union
	{
	#if CAN_BUFF_512_BYTE
		CAN_BSW_DRV_BUFF_STR DrvBuff[64];
		uint8_t CanBswSrvRxBuff[512];
	#else
		CAN_BSW_DRV_BUFF_STR DrvBuff[128];
		uint8_t CanBswSrvRxBuff[1024];
	#endif
	}UnionCanBuff;
}CAN_BSW_SRV_BUFF_STR;

extern CAN_BSW_SRV_BUFF_STR CanBuffNode[CAN_NODE_NUMB];
extern uint8_t BswSrvCache[256];

#endif





