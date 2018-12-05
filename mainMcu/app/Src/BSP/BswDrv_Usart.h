#ifndef __USART_H__
#define __USART_H__

#include "includes.h"
#include "BswDrv_FIFO.h"



#define GSM_UART_INDEX 			   	2	//4Gͨ��
#define DBG_UARTX_INDEX            	4	//��ӡ��־
#define WIFI_UART_INDEX			    1	//wifi
#define CONFIG_UARTX_INDEX			0	//��λ������
#define CARD_UART_INDEX				3	//������

typedef struct
{
	uint8_t RxBuff[256];
}UART_BUFF_STR;


typedef struct
{
	SemaphoreHandle_t uartMutex;
    FIFO_S_t rxBuffCtrl;                //���ջ��������Ϣ
}UART_INFO_STR;



extern UART_INFO_STR UartPortHandleInfo[5] ;
extern uint32_t USARTX[5];
extern osSemaphoreId ComRxSem;


void BswDrv_UsartSend(uint8_t portIndex,uint8_t *data,uint16_t len);
int BswDrv_UsartGetOneData(int portIndex, uint8_t *pData);
void BswDrv_UsartFifo_Flush(int portIndex);

void BswDrv_UsartInit(void);

#endif





