#ifndef __BSW_SRV_CAN_H__
#define __BSW_SRV_CAN_H__


#include "includes.h"
#include "CAN_Config.h"
#include "BswDrv_CAN.h"
#include "BswDrv_FIFO.h"
#include "BswDrv_Watchdog.h"





#define RECEIVE_MAX_PKT_NUM			2



enum {
    BSW_SRV_CAN_FIVE,
    BSW_SRV_CAN_AA,
    BSW_SRV_CAN_LEN,
    BSW_SRV_CAN_SN,
    BSW_SRV_CAN_CMD,
    BSW_SRV_CAN_RX_DATA,
    BSW_SRV_CAN_CHK,
    BSW_SRV_CAN_END,
};


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

typedef struct
{
    FIFO_S_t CanRxBuff;                //接收缓存控制信息
}CAN_BSW_FIFO_STR;

typedef struct
{
    uint8_t Cache[256];                //
}CAN_BSW_CACHE_STR;

extern CAN_BSW_SRV_BUFF_STR CanBuffNode[CAN_NODE_NUMB];
extern CAN_BSW_FIFO_STR CanBswSrvFifo[CAN_NODE_NUMB];	


extern void BswSrv_CanFifoInit(void);


#endif





