#include "BswDrv_Usart.h"
#include "includes.h"
#include "BswSrv_System.h"

static int UARTx_Init(uint8_t index, ControlStatus intEnable, uint32_t Baud);

#define UART_RX_BUFF_ENABLE         0

__IO uint8_t UARTx_RxBuff0[128] = {0,};		//��λ������
__IO uint8_t UARTx_RxBuff1[256] = {0,};		//WIFIͨ��
__IO uint8_t UARTx_RxBuff2[512] = {0,};		//4G
__IO uint8_t UARTx_RxBuff3[128] = {0,};		//������
//__IO uint8_t UARTx_RxBuff4[128] = {0,};		//��ӡ��־



//CONFIG  , WIFI,  4G,  CardBoard, LOG
uint32_t USARTX[5] = {USART0, USART1, USART2, UART3, UART4};


UART_INFO_STR UartPortHandleInfo[5] = {0,};

extern FIFO_S_t gGsmRecvFifo;
extern FIFO_S_t wifiBlueFifo;

osSemaphoreId ComRxSem = NULL;

rcu_periph_enum RCU_GPIOX[5] = 	{RCU_GPIOA,     RCU_GPIOA,      RCU_GPIOB,  	RCU_GPIOC,  	RCU_GPIOC};
rcu_periph_enum RCU_USARTX[5] = {RCU_USART0,    RCU_USART1,     RCU_USART2, 	RCU_UART3,  	RCU_UART4};
uint32_t RCU_USART_TX_PORT[5] = {GPIOA,         GPIOA,          GPIOB,      	GPIOC,      	GPIOC};
uint32_t RCU_USART_RX_PORT[5] = {GPIOA,         GPIOA,          GPIOB,      	GPIOC,      	GPIOD};

uint32_t GPIO_PIN_TX[5] = 		{GPIO_PIN_9,  	GPIO_PIN_2, 	GPIO_PIN_10, 	GPIO_PIN_10, 	GPIO_PIN_12};
uint32_t GPIO_PIN_RX[5] = 		{GPIO_PIN_10, 	GPIO_PIN_3, 	GPIO_PIN_11, 	GPIO_PIN_11, 	GPIO_PIN_2};

uint8_t UARTx_IRQn[5] = 		{USART0_IRQn, 	USART1_IRQn, 	USART2_IRQn, 	UART3_IRQn, 	UART4_IRQn};


int UARTx_Init(uint8_t index, ControlStatus intEnable, uint32_t Baud)
{
	UART_INFO_STR *pUartInfo = &UartPortHandleInfo[index];

    rcu_periph_clock_enable(RCU_GPIOX[index]);
    rcu_periph_clock_enable(RCU_USARTX[index]);
	
	if(4 == index)
	{
		rcu_periph_clock_enable(RCU_GPIOD);
	}
    gpio_init(RCU_USART_TX_PORT[index], GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_TX[index]);
    gpio_init(RCU_USART_RX_PORT[index], GPIO_MODE_IPU, GPIO_OSPEED_MAX, GPIO_PIN_RX[index]);

    usart_deinit(USARTX[index]);
    usart_baudrate_set(USARTX[index], Baud);            //������
    usart_parity_config(USARTX[index], USART_PM_NONE);             //��żУ��
    usart_word_length_set(USARTX[index], USART_WL_8BIT);           //8λ����λ
    usart_stop_bit_set(USARTX[index], USART_STB_1BIT);             //1λֹͣλ

    usart_receive_config(USARTX[index], USART_RECEIVE_ENABLE);     //����ʹ��
    usart_transmit_config(USARTX[index], USART_TRANSMIT_ENABLE);   //����ʹ��
	
	/*����������*/
	pUartInfo->uartMutex = xSemaphoreCreateMutex();

	if(ENABLE == intEnable)
	{
		usart_interrupt_enable(USARTX[index], USART_INT_RBNE);	//�����ж�
		
	    nvic_irq_enable(UARTx_IRQn[index], 3, 0);				//
		
		#if UART_RX_BUFF_ENABLE
		FIFO_S_Init(&pUartInfo->rxBuffCtrl, (void*)UARTx_RxBuff[index].RxBuff, sizeof(UARTx_RxBuff[index].RxBuff));
		#else
		switch(index)
		{
			case 0://��λ��ͨ��
				usart_interrupt_enable(USARTX[index], USART_INT_IDLE);
				FIFO_S_Init(&pUartInfo->rxBuffCtrl, (void*)UARTx_RxBuff0, sizeof(UARTx_RxBuff0));
			break;
			case 1:
				FIFO_S_Init(&pUartInfo->rxBuffCtrl, (void*)UARTx_RxBuff1, sizeof(UARTx_RxBuff1));
			break;
			case 2:
				FIFO_S_Init(&pUartInfo->rxBuffCtrl, (void*)UARTx_RxBuff2, sizeof(UARTx_RxBuff2));
			break;
			case 3://ˢ����
				usart_interrupt_enable(USARTX[index], USART_INT_IDLE);
				FIFO_S_Init(&pUartInfo->rxBuffCtrl, (void*)UARTx_RxBuff3, sizeof(UARTx_RxBuff3));
			break;
			case 4:
				usart_interrupt_enable(USARTX[index], USART_INT_IDLE);
				//FIFO_S_Init(&pUartInfo->rxBuffCtrl, (void*)UARTx_RxBuff4, sizeof(UARTx_RxBuff4));
			break;
			default:
			return 1;
		}
		#endif
		FIFO_S_Flush(&pUartInfo->rxBuffCtrl);
	}	

	usart_enable(USARTX[index]);
    
    return 0;
}


void BswDrv_UsartSend(uint8_t portIndex, uint8_t *data, uint16_t len)
{
    uint16_t i = 0;
    UART_INFO_STR *pUart = &UartPortHandleInfo[portIndex];

	if((xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) && (pUart->uartMutex != NULL))//ϵͳ�Ѿ�����
	{
		xSemaphoreTake(pUart->uartMutex,1000);
	}
    
    for(i = 0; i < len; i++)
    {
        usart_data_transmit(USARTX[portIndex], (uint8_t)data[i]);
        //while(RESET == usart_flag_get(pUart->uartCom, USART_FLAG_TBE));
        while(RESET == usart_flag_get(USARTX[portIndex], USART_FLAG_TBE));
    }
    
	if((xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) && (pUart->uartMutex != NULL))//ϵͳ�Ѿ�����
	{
		xSemaphoreGive(pUart->uartMutex);
	}
    
    return;
}

//����ֵΪ0����ȡ�ɹ�������:��ȡʧ��
//��ȡһ���ַ��󣬸��ַ��ӻ��������
int BswDrv_UsartGetOneData(int portIndex, uint8_t *pData)
{
    UART_INFO_STR *pUart = &UartPortHandleInfo[portIndex];

    return FIFO_S_Get(&pUart->rxBuffCtrl, pData);
}

void BswDrv_UsartFifo_Flush(int portIndex)
{
	UART_INFO_STR *pUart = &UartPortHandleInfo[portIndex];
	FIFO_S_Flush(&pUart->rxBuffCtrl);
}

void BswDrv_UsartInit(void)
{
	while(1 == UARTx_Init(0, ENABLE, 115200))
	{
		;
	}
	while(1 == UARTx_Init(1, ENABLE, 115200))
	{
		;
	}
	while(1 == UARTx_Init(2, ENABLE, 115200))
	{
		;
	}
	while(1 == UARTx_Init(3, ENABLE, 115200))
	{
		;
	}
	while(1 == UARTx_Init(4, DISABLE, 115200))
	{
		;
	}
}


void IRQHandler(int index)
{
	uint8_t clear = 0;
	uint8_t data = 0;
	UART_INFO_STR *pUart = &UartPortHandleInfo[index];
		
	(void)clear;
	//�����ж�
	if(RESET != usart_interrupt_flag_get(USARTX[index], USART_INT_FLAG_RBNE))
	{
		data = usart_data_receive(USARTX[index]);
		
		if(index == GSM_UART_INDEX)//4Gģ��
		{
			//���ڽ���ATָ����Ӧ����
			FIFO_S_Put(&pUart->rxBuffCtrl, data);
			
			//���ڽ��շ�������Ӧ����
			if(GlobalInfo.is_socket_0_ok == 1 && GlobalInfo.netType == NETTYPE_GPRS)
			{
				FIFO_S_Put(&gGsmRecvFifo, data);
			}
		}
		else if(index == WIFI_UART_INDEX)//wifiģ��
		{
			//���ڽ���ATָ��
			FIFO_S_Put(&pUart->rxBuffCtrl, data);

			//���ڴ���wifi��������
			FIFO_S_Put(&wifiBlueFifo, data);
		}
		else
		{
			FIFO_S_Put(&pUart->rxBuffCtrl, data);
		}
		
		usart_interrupt_flag_clear(USARTX[index], USART_INT_FLAG_RBNE);
	}
	//�����ж�
	if(RESET != usart_interrupt_flag_get(USARTX[index], USART_INT_FLAG_IDLE))
	{
		//
		if(index == CONFIG_UARTX_INDEX || index == CARD_UART_INDEX)
		{
			if(ComRxSem != NULL)
			{
				osSemaphoreRelease(ComRxSem);
			}
		}
		clear = USART_STAT0(USARTX[index]);
		clear = USART_DATA(USARTX[index]);		
	}
}


void USART0_IRQHandler(void)
{
	IRQHandler(0);
}

void USART1_IRQHandler(void)
{
	IRQHandler(1);
}

void USART2_IRQHandler(void)
{
	IRQHandler(2);
}

void UART3_IRQHandler(void)
{
	IRQHandler(3);
}

void UART4_IRQHandler(void)
{
	IRQHandler(4);
}






