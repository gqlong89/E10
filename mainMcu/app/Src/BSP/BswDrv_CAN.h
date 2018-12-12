#ifndef __BSW_DRV_CAN_H__
#define __BSW_DRV_CAN_H__


#include "CAN_Config.h"



#define CAN_TX_SFID                        	((uint32_t)0x331)
#define CAN_TX_EFID                        	((uint32_t)0x12345678)
#define CAN_SA_ADDR                        	(0x12)		//�ɲ��뿪�����õ�Դ��ַ
#define CAN_PS_ADDR0                        (0x12)		//�ɲ��뿪�����õ�Ŀ�ĵ�ַ, ��0���ڵ��Ŀ�ĵ�ַ  

#define MESSAGE_QUEUE_NUM					(0x0f)		//��Ϣ���и���


#pragma pack(1)

typedef struct
{
	uint8_t CanDrvRxBuff[8];
}CAN_BSW_DRV_BUFF_STR;


typedef struct
{
	uint8_t Cmd;
	uint8_t SA;		//����Դ�ĵ�ַ
	uint8_t Len;
	uint8_t RxBuff[8];
}CAN_DRV_MESSAGE_QUEUE_STR;

typedef struct
{
	uint8_t WriteIndex;
	uint8_t ReadIndex;
	uint8_t lastReadIndex;
}CAN_READ_WRITE_INDEX_STR;

#pragma pack()






#endif



