/*
 * @Author: quqian 
 * @Date: 2018-12-05 14:13:14 
 * @def :CAN����������
 * @Last Modified by: quqian
 * @Last Modified time: 2018-12-06 18:01:00
 */
#include "includes.h"
#include "BswDrv_CAN.h"
#include "BswSrv_CAN.h"
#include "BswSrv_System.h"
#include "App_CAN.h"




can_trasnmit_message_struct transmit_message;
can_receive_message_struct receive_message;
//CAN_BSW_DRV_BUFF_STR CanReceiveData[CAN_NODE_NUMB] = {0,};

void BswDrv_CanBusGpioConfig(void)
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
void BswDrv_CanBusInit(void)
{
    can_parameter_struct can_parameter;
    can_filter_parameter_struct can_filter;

	BswDrv_CanBusGpioConfig();
	
    can_deinit(CAN0);
    
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = DISABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = ENABLE;	//��TFOΪ1�����еȴ����͵����䰴�������ȷ��ͣ�FIFO����˳����С�
												//��TFOΪ0��������С��ʶ����Identifier�����������ȷ��͡�������еı�ʶ����Identifier����ȣ�������С�����ŵ��������ȷ��͡�
    can_parameter.working_mode = CAN_LOOPBACK_MODE;//CAN_NORMAL_MODE;

	/* 
		CAN ������ = RCC_APB1Periph_CAN0 / prescaler / (resync_jump_width + time_segment_1 + time_segment_2);

		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		�����У�����CAN������Ϊ1Mbps		
		CAN ������ = 42000000 / 6 / (0 + 5 + 2) / = 1 Mbps		
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

	GlobalInfo.CanMessageQueue = xQueueCreate(MESSAGE_QUEUE_NUM, sizeof(CAN_DRV_MESSAGE_QUEUE_STR)); 
	BswSrv_CanFifoInit();
#if USER_ANOTHER_THREAD
	AppCan_FifoInit();
#endif
}

void BswDrv_CanWriteHandle(uint16_t Serial)
{
	uint16_t FrameSerial = Serial;
    uint32_t i = 0;
    
	if (CanBswSrvFifo[0].CanRxBuff.writeIndex >= CanBswSrvFifo[0].CanRxBuff.lastReadIndex) 
	{
		if ((((uint16_t)FrameSerial * 8) < CanBswSrvFifo[0].CanRxBuff.writeIndex))
		{
			if(((uint16_t)FrameSerial * 8) >= CanBswSrvFifo[0].CanRxBuff.lastReadIndex)
			{
				memcpy(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, receive_message.rx_data, 8);
			}
	        else if(((uint16_t)FrameSerial * 8) < (CanBswSrvFifo[0].CanRxBuff.lastReadIndex - 8))	//��ֹ��ȡ̫��, ǰ������ݱ����渲��
			{
				for(i = (CanBswSrvFifo[0].CanRxBuff.writeIndex / 8); i < CAN_BUFF_LEN; i++)
				{
					memset(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, 0, 8);
				}
				for(i = 0; i < FrameSerial; i++)
				{
					memset(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, 0, 8);
				}
				memcpy(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, receive_message.rx_data, 8);
			}
	    }
		else if ((((uint16_t)FrameSerial * 8) == CanBswSrvFifo[0].CanRxBuff.writeIndex) && 
			(((uint16_t)FrameSerial * 8) >= CanBswSrvFifo[0].CanRxBuff.lastReadIndex))
		{
	        memcpy(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, receive_message.rx_data, 8);
			if(((uint32_t)CAN_BUFF_LEN - 1) > FrameSerial)
			{
				CanBswSrvFifo[0].CanRxBuff.writeIndex = ((uint16_t)(FrameSerial + 1) * 8);
			}
			else if(((uint32_t)CAN_BUFF_LEN - 1) == FrameSerial)
			{
				CanBswSrvFifo[0].CanRxBuff.writeIndex = 0;
			}
			else
			{
				//֡��ų�����Χ, ����
			}
	    }
		else if ((((uint16_t)FrameSerial * 8) > CanBswSrvFifo[0].CanRxBuff.writeIndex) && 
			(((uint16_t)FrameSerial * 8) >= CanBswSrvFifo[0].CanRxBuff.lastReadIndex))
		{
			if(FrameSerial == ((uint32_t)CAN_BUFF_LEN - 1))
			{
				for(i = (CanBswSrvFifo[0].CanRxBuff.writeIndex / 8); i < FrameSerial; i++)
				{
					memset(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, 0, 8);
				}
				memcpy(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, receive_message.rx_data, 8);
				CanBswSrvFifo[0].CanRxBuff.writeIndex = 0;
			}
			else if(FrameSerial < ((uint32_t)CAN_BUFF_LEN - 1))
			{
				for(i = (CanBswSrvFifo[0].CanRxBuff.writeIndex / 8); i < FrameSerial; i++)
				{
					memset(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, 0, 8);
				}
				memcpy(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, receive_message.rx_data, 8);
				CanBswSrvFifo[0].CanRxBuff.writeIndex = ((uint16_t)(FrameSerial + 1) * 8);
			}
			else
			{
				//֡��ų�����Χ, ����
			}
	    }
    }
	else if (CanBswSrvFifo[0].CanRxBuff.writeIndex < (CanBswSrvFifo[0].CanRxBuff.lastReadIndex - 8)) //writeIndex ��8�ı���
	{
        if(((uint16_t)FrameSerial * 8) < (CanBswSrvFifo[0].CanRxBuff.lastReadIndex - 8))
        {
        	if ((((uint16_t)FrameSerial * 8) < CanBswSrvFifo[0].CanRxBuff.writeIndex))
			{
				memcpy(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, receive_message.rx_data, 8);
			}
			else if ((((uint16_t)FrameSerial * 8) == CanBswSrvFifo[0].CanRxBuff.writeIndex))
			{
				memcpy(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, receive_message.rx_data, 8);
				CanBswSrvFifo[0].CanRxBuff.writeIndex = ((uint16_t)(FrameSerial + 1) * 8);
			}
			else if ((((uint16_t)FrameSerial * 8) > CanBswSrvFifo[0].CanRxBuff.writeIndex))
			{
				for(i = (CanBswSrvFifo[0].CanRxBuff.writeIndex / 8); i < FrameSerial; i++)
				{
					memset(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, 0, 8);
				}
				memcpy(CanBuffNode[0].UnionCanBuff.DrvBuff[FrameSerial].CanDrvRxBuff, receive_message.rx_data, 8);
				CanBswSrvFifo[0].CanRxBuff.writeIndex = ((uint16_t)(FrameSerial + 1) * 8);
			}
		}
		else
		{
			//��ȡ̫��, д�ռ䲻��
		}
    }
	else
	{
		//��ȡ̫��, д�ռ䲻��
	}
}

void USBD_LP_CAN0_RX0_IRQHandler(void)
{
	uint8_t FrameSerial = 0;
	
    /* check the receive message */
    can_message_receive(CAN0, CAN_FIFO0, &receive_message);

	switch(receive_message.rx_ft)
	{
		case CAN_FT_DATA:	//���������֡
		{
			if((CAN_SA_ADDR == receive_message.ExtendFrame.rx_efid_struct.SA))		//Դ��ַ
			{
				switch(receive_message.rx_ff)
				{
					case CAN_FF_EXTENDED:	//�������չ֡
					{
						switch(receive_message.ExtendFrame.rx_efid_struct.PS)	//Ŀ�ĵ�ַ, �Ƿ����ҵ�?
						{
							case CAN_PS_ADDR0:	//���յ�0���ڵ�����
							{
								if(0 == receive_message.ExtendFrame.rx_efid_struct.PF_Bit7)		//����ǵ�֡
								{
									//uxQueueMessagesWaiting(GlobalInfo.CanMessageQueue)		//ʹ���˵Ķ���
									//uxQueueSpacesAvailable(GlobalInfo.CanMessageQueue)		//��ʣ��Ķ���
									GlobalInfo.CanSendMessage.Cmd = receive_message.ExtendFrame.rx_efid_struct.PF_Bit0ToBit7;
									GlobalInfo.CanSendMessage.Len = receive_message.rx_dlen;
									memcpy(GlobalInfo.CanSendMessage.RxBuff, receive_message.rx_data, 8);
									if((GlobalInfo.CanMessageQueue != NULL))
									{
										if(0 < uxQueueSpacesAvailable(GlobalInfo.CanMessageQueue))	//ʣ����д���0��
										{
											if(pdTRUE == xQueueSend(GlobalInfo.CanMessageQueue, (void*)&GlobalInfo.CanSendMessage, (TickType_t)1))	//������з�������
											{
												memset((void*)&GlobalInfo.CanSendMessage.Cmd, 0, sizeof(GlobalInfo.CanSendMessage));	//������ݽ��ջ�����
												return;
											}
										}
									}
									if(GlobalInfo.AppCan_HandleCallBack != NULL)
									{
										GlobalInfo.AppCan_HandleCallBack(receive_message.rx_data, receive_message.rx_dlen, GlobalInfo.CanSendMessage.Cmd);
									}
									else
									{
										AppCan_CallBack(receive_message.rx_data, receive_message.rx_dlen, GlobalInfo.CanSendMessage.Cmd);
									}
								}
								else if(1 == receive_message.ExtendFrame.rx_efid_struct.PF_Bit7)	//����Ƕ�֡��
								{
									FrameSerial = receive_message.ExtendFrame.rx_efid_struct.PF_Bit0ToBit7;
									#if CAN_BUFF_512_BYTE
									if(64 <= FrameSerial)
									{
										FrameSerial = FrameSerial - 64;
									}

									BswDrv_CanWriteHandle(FrameSerial);
									#else
									BswDrv_CanWriteHandle(FrameSerial);
									#endif
								}
							}
							break;
							default:
							break;
						}
					}
					break;
					
					case CAN_FF_STANDARD:	//����Ǳ�׼֡
					break;
					default:
					break;
				};
			}
		}
		break;
		
		case CAN_FT_REMOTE:	//�����ң��֡
		break;
		default:
		break;
	}
}









