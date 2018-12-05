/*
 * @Author: quqian 
 * @Date: 2018-12-05 14:13:14 
 * @def :CAN总线驱动层
 * @Last Modified by: quqian
 * @Last Modified time: 2018-12-06 18:01:00
 */
#include "includes.h"
#include "BswDrv_CAN.h"



uint8_t receive_flag = 0;

can_trasnmit_message_struct transmit_message;
can_receive_message_struct receive_message;
CAN_BSW_DRV_BUFF_STR CanReceiveData[CAN_NODE_NUMB] = {0,};

void CanBusGpioConfig(void)
{
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOD);
    
    /* configure CAN0 GPIO, CAN0_TX(PD1) and CAN0_RX(PD0) */
    gpio_init(GPIOD, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
    
    gpio_init(GPIOD, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    
    gpio_pin_remap_config(GPIO_CAN_FULL_REMAP,ENABLE);

	/* configure CAN0 NVIC */
    nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn,0,0);
}

/*****************************************************************************
** Function name:       CanBusInit
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  	None
** Author:              quqian
*****************************************************************************/
void CanBusInit(void)
{
    can_parameter_struct can_parameter;
    can_filter_parameter_struct can_filter;

	CanBusGpioConfig();
	
    can_deinit(CAN0);
    
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = DISABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = ENABLE;	//当TFO为1，所有等待发送的邮箱按照先来先发送（FIFO）的顺序进行。
												//当TFO为0，具有最小标识符（Identifier）的邮箱最先发送。如果所有的标识符（Identifier）相等，具有最小邮箱编号的邮箱最先发送。
    can_parameter.working_mode = CAN_LOOPBACK_MODE;//CAN_NORMAL_MODE;

	/* 
		CAN 波特率 = RCC_APB1Periph_CAN0 / prescaler / (resync_jump_width + time_segment_1 + time_segment_2);

		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		本例中，设置CAN波特率为1Mbps		
		CAN 波特率 = 42000000 / 6 / (0 + 5 + 2) / = 1 Mbps		
	*/
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_6TQ;
    can_parameter.time_segment_2 = CAN_BT_BS2_3TQ;
    /* baudrate 1Mbps */
    can_parameter.prescaler = 12;
    can_init(CAN0, &can_parameter);

    /* initialize filter */
    /* CAN0 filter number */
    can_filter.filter_number = 0;

    /* initialize filter */    
	can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;  
    can_filter.filter_enable = ENABLE;
    can_filter_init(&can_filter);

	/* enable CAN receive FIFO0 not empty interrupt */
    can_interrupt_enable(CAN0, CAN_INTEN_RFNEIE0);
}


void USBD_LP_CAN0_RX0_IRQHandler(void)
{
	static uint8_t Len = 0;
	
    /* check the receive message */
    can_message_receive(CAN0, CAN_FIFO0, &receive_message);
//    CanReceiveData[0].ReceiveLen = receive_message.rx_dlen;//can_receive_message_length_get(CAN0, CAN_FIFO0);
    #if 0
    if((CAN_TX_SFID == receive_message.rx_sfid) && (CAN_FF_STANDARD == receive_message.rx_ff))
	{
		Len++;
		memcpy(CanReceiveDataLenStruct[0].ReceiveData, receive_message.rx_data, 8);
		CanReceiveDataLenStruct[0].rx_fi = receive_message.rx_fi;
		CanReceiveDataLenStruct[0].rx_efid = receive_message.rx_efid;
        receive_flag = 1;
		
	//	CanReceiveData[]
    }
	#else
	if((CAN_TX_EFID == receive_message.ExtendFrame.rx_efid) && (CAN_FF_EXTENDED == receive_message.rx_ff))
	{
		Len++;
	//	memcpy(CanReceiveData[0].ReceiveData, receive_message.rx_data, 8);
	//	CanReceiveData[0].rx_fi = receive_message.rx_fi;
	//	CanReceiveData[0].rx_efid = receive_message.ExtendFrame.rx_efid;
        receive_flag = 1;
		
	//	CanReceiveData[]
    }
	#endif
}









