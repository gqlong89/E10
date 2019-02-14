#ifndef __USART_H__
#define __USART_H__

#include "includes.h"
#include "BswDrv_FIFO.h"



#define GSM_UART_INDEX 			   				2	//4Gͨ��
#define DBG_UARTX_INDEX            				4	//��ӡ��־
#define WIFI_UART_INDEX			    			1	//wifi
#define CONFIG_UARTX_INDEX						0	//��λ������
#define CARD_UART_INDEX							3	//������
#define  BLUETOOTH_PORT             			3
#define DEBUG_INDEX                 			3
#define  PHY_UART_GPRS_PORT						1

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


extern void BswDrv_UsartSend(uint8_t portIndex,uint8_t *data,uint16_t len);
extern int BswDrv_UsartGetOneData(int portIndex, uint8_t *pData);
extern void BswDrv_UsartFifo_Flush(int portIndex);
extern void UsartFlush(int portIndex);
extern void BswDrv_UsartInit(void);

#endif





