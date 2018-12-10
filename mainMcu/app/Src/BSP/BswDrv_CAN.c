/*
 * @Author: quqian 
 * @Date: 2018-12-05 14:13:14 
 * @def :CAN总线驱动层
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
	        else if(((uint16_t)FrameSerial * 8) < (CanBswSrvFifo[0].CanRxBuff.lastReadIndex - 8))	//防止读取太慢, 前面的数据被后面覆盖
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
				//帧序号超出范围, 错误
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
				//帧序号超出范围, 错误
			}
	    }
    }
	else if (CanBswSrvFifo[0].CanRxBuff.writeIndex < (CanBswSrvFifo[0].CanRxBuff.lastReadIndex - 8)) //writeIndex 是8的倍数
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
			//读取太慢, 写空间不足
		}
    }
	else
	{
		//读取太慢, 写空间不足
	}
}

void USBD_LP_CAN0_RX0_IRQHandler(void)
{
	uint8_t FrameSerial = 0;
	
    /* check the receive message */
    can_message_receive(CAN0, CAN_FIFO0, &receive_message);

	switch(receive_message.rx_ft)
	{
		case CAN_FT_DATA:	//如果是数据帧
		{
			if((CAN_SA_ADDR == receive_message.ExtendFrame.rx_efid_struct.SA))		//源地址
			{
				switch(receive_message.rx_ff)
				{
					case CAN_FF_EXTENDED:	//如果是扩展帧
					{
						switch(receive_message.ExtendFrame.rx_efid_struct.PS)	//目的地址, 是发给我的?
						{
							case CAN_PS_ADDR0:	//接收第0个节点数据
							{
								if(0 == receive_message.ExtendFrame.rx_efid_struct.PF_Bit7)		//如果是单帧
								{
									//uxQueueMessagesWaiting(GlobalInfo.CanMessageQueue)		//使用了的队列
									//uxQueueSpacesAvailable(GlobalInfo.CanMessageQueue)		//还剩余的队列
									GlobalInfo.CanSendMessage.Cmd = receive_message.ExtendFrame.rx_efid_struct.PF_Bit0ToBit7;
									GlobalInfo.CanSendMessage.Len = receive_message.rx_dlen;
									memcpy(GlobalInfo.CanSendMessage.RxBuff, receive_message.rx_data, 8);
									if((GlobalInfo.CanMessageQueue != NULL))
									{
										if(0 < uxQueueSpacesAvailable(GlobalInfo.CanMessageQueue))	//剩余队列大于0个
										{
											if(pdTRUE == xQueueSend(GlobalInfo.CanMessageQueue, (void*)&GlobalInfo.CanSendMessage, (TickType_t)1))	//向队列中发送数据
											{
												memset((void*)&GlobalInfo.CanSendMessage.Cmd, 0, sizeof(GlobalInfo.CanSendMessage));	//清除数据接收缓冲区
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
								else if(1 == receive_message.ExtendFrame.rx_efid_struct.PF_Bit7)	//如果是多帧包
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
					
					case CAN_FF_STANDARD:	//如果是标准帧
					break;
					default:
					break;
				};
			}
		}
		break;
		
		case CAN_FT_REMOTE:	//如果是遥控帧
		break;
		default:
		break;
	}
}









