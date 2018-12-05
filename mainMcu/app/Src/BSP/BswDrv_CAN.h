#ifndef __BSW_DRV_CAN_H__
#define __BSW_DRV_CAN_H__


#include "CAN_Config.h"



#define CAN_TX_SFID                        ((uint32_t)0x331)
#define CAN_TX_EFID                        ((uint32_t)0x12345678)


typedef struct
{
	uint8_t CanDrvRxBuff[8];
}CAN_BSW_DRV_BUFF_STR;









#endif



