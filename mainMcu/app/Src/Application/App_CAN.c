/*
 * @Author: quqian 
 * @Date: 2018-12-05 14:14:54 
 * @Last Modified by: quqian
 * @Last Modified time: 2018-12-05 17:57:35
 */

#include "App_CAN.h"
#include "BswDrv_Rtc.h"
#include "BswDrv_Debug.h"
#include "BswDrv_GPIO.h"
#include "BswSrv_System.h"
#include "BswSrv_FlashUsr.h"
#include "BswSrv_NetTask.h"
#include "BswSrv_Air720.h"
#include "BswSrv_WG215.h"
#include "BswSrv_FwUpgrade.h"



int canMessageTransmit(uint32_t can_periph, can_trasnmit_message_struct* transmit_message)
{
	uint8_t MailboxNumber = 0;
	uint8_t timesFlag = 0;
	MailboxNumber = can_message_transmit(can_periph, transmit_message);
	while(CAN_TRANSMIT_FAILED == can_transmit_states(CAN0, MailboxNumber))
	{
		vTaskDelay(30);
		if(10 < timesFlag++)
		{
			timesFlag = 0;
			break;
		}
	}
    
    return 0;
}


/*****************************************************************************
** Function name:       AppCan_SendData
** Descriptions:        
** input parameters:    SourceAddr:节点来源
						pFrame:数据
						len: 结构数据长度
** Returned value:	  	
** Author:              quqian
*****************************************************************************/
int AppCan_SendData(uint8_t SourceAddr, APP_CAN_STR_t *pFrame, uint16_t len)
{
	can_trasnmit_message_struct TrasnmitMessage = {0,};
	uint8_t	PackNum = 0;
	uint32_t i = 0;
	
	TrasnmitMessage.ExtendFrame.TxEfid.SA = GlobalInfo.DestAddr;		//SA为源地址
	TrasnmitMessage.ExtendFrame.TxEfid.PS = SourceAddr;					//PS为目标地址
	
	TrasnmitMessage.ExtendFrame.TxEfid.DP = 0;
	TrasnmitMessage.ExtendFrame.TxEfid.R = 0;
	TrasnmitMessage.ExtendFrame.TxEfid.P = 0;
	TrasnmitMessage.ExtendFrame.TxEfid.ThreeBitZero = 0;
	
	if(8 >= len)		//单帧
	{
		TrasnmitMessage.ExtendFrame.TxEfid.PF_Bit7 = 0;
		TrasnmitMessage.ExtendFrame.TxEfid.PF_Bit0ToBit7 = pFrame->head.cmd;
		//memset(TrasnmitMessage.tx_data, 0, 8);
		memcpy(TrasnmitMessage.tx_data, &pFrame->data[0], 8);
		canMessageTransmit(CAN0, &TrasnmitMessage);
	}
	else 	//多包帧
	{
		if(0 == (len + sizeof(APP_CAN_HEAD_STR) + CAN_SUM_LEN + CAN_R_N_LEN) % 8)
		{
			PackNum = (len + sizeof(APP_CAN_HEAD_STR) + CAN_SUM_LEN + CAN_R_N_LEN) / 8;
		}
		else
		{
			PackNum = ((len + sizeof(APP_CAN_HEAD_STR) + CAN_SUM_LEN + CAN_R_N_LEN) / 8) + 1;
		}
		TrasnmitMessage.ExtendFrame.TxEfid.PF_Bit7 = 1;
		memset(TrasnmitMessage.tx_data, 0, 8);
		for(i = 0; i < PackNum; i++)
		{
			TrasnmitMessage.ExtendFrame.TxEfid.PF_Bit0ToBit7 = GlobalInfo.BitFieldFlag.PF_Bit0ToBit7++;
			//memset(TrasnmitMessage.tx_data, 0, 8);
			memcpy(TrasnmitMessage.tx_data, &pFrame->data[i * 8], 8);
			canMessageTransmit(CAN0, &TrasnmitMessage);
			vTaskDelay(10);
		}
	}
	
    return 0;
}

void AppCan_PackHead(APP_CAN_STR_t* Frame, uint8_t cmd)
{
	Frame->head.five = 0x55;
	Frame->head.aa = 0xAA;
	Frame->head.sn = GlobalInfo.SerialNum++;
	Frame->head.cmd = cmd;
}

void AppCan_ShakeHandReq(uint8_t SourceAddr)
{
	APP_CAN_STR_t AppCanBuff;
	APP_CAN_SHAKE_HAND_STR *CanShakeHandReq = (void*)AppCanBuff.data;

	memset(&AppCanBuff.head.five, 0, sizeof(APP_CAN_STR_t));
	AppCanBuff.head.len = sizeof(APP_CAN_SHAKE_HAND_STR) + CAN_SN_CMD_LEN + CAN_SUM_LEN;
	AppCan_PackHead(&AppCanBuff, (uint8_t)CAN_CMD_HAND);
	
	CanShakeHandReq->gun = 1;

	AppCanBuff.data[AppCanBuff.head.len - CAN_SN_CMD_LEN] = '\r';
	AppCanBuff.data[AppCanBuff.head.len - CAN_SN_CMD_LEN + 1] = '\n';
    //发送握手请求
    AppCan_SendData(SourceAddr, &AppCanBuff, sizeof(APP_CAN_SHAKE_HAND_STR));

	return;
}

void AppCan_ShakeHandAck(uint8_t *data, uint8_t SourceAddr)
{
//	APP_CAN_SHAKE_HAND_ACK_STR *CanShakeHandAck = (void*)data;

//	CanShakeHandAck->gun;

    return;
}

void AppCan_RemoteControlReq(uint8_t SourceAddr)
{
	APP_CAN_STR_t AppCanBuff;
	APP_CAN_REMOTE_CONTROL_STR *RemoteControlReq = (void*)AppCanBuff.data;

	memset(&AppCanBuff.head.five, 0, sizeof(APP_CAN_STR_t));
	AppCanBuff.head.len = sizeof(APP_CAN_REMOTE_CONTROL_STR) + CAN_SN_CMD_LEN + CAN_SUM_LEN;
	AppCan_PackHead(&AppCanBuff, (uint8_t)CAN_CMD_REMOTE_CONTROL);
	
	RemoteControlReq->gun = 1;

	AppCanBuff.data[AppCanBuff.head.len - CAN_SN_CMD_LEN] = '\r';
	AppCanBuff.data[AppCanBuff.head.len - CAN_SN_CMD_LEN + 1] = '\n';
    //发送握手请求
    AppCan_SendData(SourceAddr, &AppCanBuff, sizeof(APP_CAN_REMOTE_CONTROL_STR));

	return;
}

void AppCan_RemoteControlAck(uint8_t *data, uint8_t SourceAddr)
{
//	APP_CAN_REMOTE_CONTROL_ACK_STR *CanRemoteCtrolAck = (void*)data;

//	CanRemoteCtrolAck->gun;

    return;
}


void AppCan_EventNotifyAck(uint8_t *data, uint8_t SourceAddr)
{
//	APP_CAN_EVENT_NOTIFY_ACK_STR *CanEventNotifyAck = (void*)data;

//	CanEventNotifyAck->gun;

    return;
}


void AppCan_UpgradeReq(uint8_t SourceAddr)
{
	APP_CAN_STR_t AppCanBuff;
	APP_CAN_UPGRADE_STR *UpgradeReq = (void*)AppCanBuff.data;

	memset(&AppCanBuff.head.five, 0, sizeof(APP_CAN_STR_t));
	AppCanBuff.head.len = sizeof(APP_CAN_UPGRADE_STR) + CAN_SN_CMD_LEN + CAN_SUM_LEN;
	AppCan_PackHead(&AppCanBuff, (uint8_t)CAN_CMD_REQ_UPGRADE);
	
	UpgradeReq->gun = 1;

	AppCanBuff.data[AppCanBuff.head.len - CAN_SN_CMD_LEN] = '\r';
	AppCanBuff.data[AppCanBuff.head.len - CAN_SN_CMD_LEN + 1] = '\n';
    //发送握手请求
    AppCan_SendData(SourceAddr, &AppCanBuff, sizeof(APP_CAN_UPGRADE_STR));

	return;
}

void AppCan_UpgradeAck(uint8_t *data, uint8_t SourceAddr)
{
//	APP_CAN_UPGRADE_ACK_STR *UpgradeAck = (void*)data;

//	UpgradeAck->gun;

    return;
}

void AppCan_FirmDownReq(uint8_t SourceAddr)
{
	APP_CAN_STR_t AppCanBuff;
	APP_CAN_FIRMWARE_DOWN_STR *FirmDownReq = (void*)AppCanBuff.data;

	memset(&AppCanBuff.head.five, 0, sizeof(APP_CAN_STR_t));
	AppCanBuff.head.len = sizeof(APP_CAN_FIRMWARE_DOWN_STR) + CAN_SN_CMD_LEN + CAN_SUM_LEN;
	AppCan_PackHead(&AppCanBuff, (uint8_t)CAN_CMD_DOWN_FW_INFO);
	
	FirmDownReq->PackNum = 1;
    
	AppCanBuff.data[AppCanBuff.head.len - CAN_SN_CMD_LEN] = '\r';
	AppCanBuff.data[AppCanBuff.head.len - CAN_SN_CMD_LEN + 1] = '\n';
    //发送握手请求
    AppCan_SendData(SourceAddr, &AppCanBuff, sizeof(APP_CAN_FIRMWARE_DOWN_STR));

	return;
}

void AppCan_FirmDownAck(uint8_t *data, uint8_t SourceAddr)
{
//	APP_CAN_FIRMWARE_DOWN_ACK_STR *FirmDownAck = (void*)data;

//	FirmDownAck->gun;

    return;
}

void App_CanFrameHandle(uint8_t *data, uint16_t len, uint8_t cmd, uint8_t SourceAddr)
{
    switch (cmd)
    {
        case CAN_CMD_HAND:
            AppCan_ShakeHandAck(data, SourceAddr);
		break;
		case CAN_CMD_REMOTE_CONTROL:
            AppCan_RemoteControlAck(data, SourceAddr);
		break;
		case CAN_CMD_EVENT_NOTICE:
            AppCan_EventNotifyAck(data, SourceAddr);
		break;
		case CAN_CMD_REQ_UPGRADE:
            AppCan_UpgradeAck(data, SourceAddr);
		break;
		case CAN_CMD_DOWN_FW_INFO:
            AppCan_FirmDownAck(data, SourceAddr);
		break;
        default:
            CL_LOG("no find module..\r\n");
		break;
    }
}

void AppCan_CallBack(uint8_t *data, uint16_t len, uint8_t cmd, uint8_t SourceAddr)
{
	App_CanFrameHandle(data, len, cmd, SourceAddr);
}

/*****************************************************************************
** Function name:       AppCan_SingleFrameHandle
** Descriptions:        单帧操作函数
** input parameters:    data : 单帧数据
						len : 单帧数据长度
						cmd : 单帧数据对应的执行命令
** Returned value:	  	None
** Author:              quqian
*****************************************************************************/
void AppCan_SingleFrameHandle(uint8_t *data, uint16_t len, uint8_t cmd, uint8_t SourceAddr)
{
	BaseType_t QueueReceiveReturn;

	if(0 < uxQueueMessagesWaiting(GlobalInfo.CanMessageQueue))	//使用了的队列
	{
		QueueReceiveReturn = xQueueReceive(GlobalInfo.CanMessageQueue, 				/* 消息队列句柄 */
										(void *)&GlobalInfo.CanReceiveMessage, 	/* 这里获取的是结构体的地址 */
										(TickType_t)1);		/* 设置阻塞时间 */
		if(pdTRUE == QueueReceiveReturn)
		{
			if(GlobalInfo.AppCan_HandleCallBack != NULL)
			{
				GlobalInfo.AppCan_HandleCallBack(GlobalInfo.CanReceiveMessage.RxBuff, GlobalInfo.CanReceiveMessage.Len, GlobalInfo.CanReceiveMessage.Cmd, SourceAddr);
			}
			else
			{
				AppCan_CallBack(GlobalInfo.CanReceiveMessage.RxBuff, GlobalInfo.CanReceiveMessage.Len, GlobalInfo.CanReceiveMessage.Cmd, SourceAddr);
			}
		}
	}
}


#if USER_ANOTHER_THREAD

CAN_BSW_FIFO_STR CanBswSrvAppFifo[CAN_NODE_NUMB] = {0,};	

CAN_BSW_CACHE_STR CanApp_Cache[CAN_NODE_NUMB] = {0,};	//用于缓存完整数据包


int AppCan_GetOneData(int portIndex, uint8_t *pData)
{
    CAN_BSW_FIFO_STR *pCan= &CanBswSrvAppFifo[portIndex];

    return FIFO_S_Get(&pCan->CanRxBuff, pData);
}

void AppCan_FifoInit(void)
{
	uint32_t Index = 0;
	
	for(Index = 0; Index < CAN_NODE_NUMB; Index++)
	{
		FIFO_S_Init(&CanBswSrvAppFifo[Index].CanRxBuff, (void*)CanApp_Cache[Index].Cache, sizeof(CanApp_Cache[Index].Cache));
		FIFO_S_Flush(&CanBswSrvAppFifo[Index].CanRxBuff);
	}
}

/*****************************************************************************
** Function name:       AppCan_RecvDataToCache
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  	
** Author:              quqian
*****************************************************************************/
void AppCan_RecvDataToCache(uint8_t NodeIndex)
{
	static uint8_t AppCan_Cache[CAN_NODE_NUMB][256] = {0,};
	uint8_t *pktBuff = (void*)&AppCan_Cache[NodeIndex][0];
    static uint8_t step[CAN_NODE_NUMB] = {BSW_SRV_CAN_FIVE, BSW_SRV_CAN_FIVE, BSW_SRV_CAN_FIVE,
											BSW_SRV_CAN_FIVE, BSW_SRV_CAN_FIVE, BSW_SRV_CAN_FIVE,
											BSW_SRV_CAN_FIVE, BSW_SRV_CAN_FIVE, BSW_SRV_CAN_FIVE};
    uint8_t data;
    static uint8_t sum[CAN_NODE_NUMB] = {0,};
    static uint16_t pktLen[CAN_NODE_NUMB] = {0,};
    static uint16_t len[CAN_NODE_NUMB] = {0,};
	APP_CAN_STR_t *pAppCanPkt = NULL;
	
	while (CL_OK == AppCan_GetOneData(NodeIndex, &data))
    {
		//printf("data = 0x%x ", data);
		BswDrv_FeedWatchDog();
		switch (step[NodeIndex])
		{
			case BSW_SRV_CAN_FIVE:
				if (data == 0x55)
				{
					step[NodeIndex] = BSW_SRV_CAN_AA;
					pktBuff[0] = data;
					pktLen[NodeIndex] = 1;
					sum[NodeIndex] = data;
				}
			break;
			case BSW_SRV_CAN_AA:
				if (data == 0xAA)
				{
					step[NodeIndex] = BSW_SRV_CAN_LEN;
					pktBuff[1] = data;
					pktLen[NodeIndex]++;
					sum[NodeIndex] += data;
				}
				else if (data == 0x55)
				{
					step[NodeIndex] = BSW_SRV_CAN_AA;
					pktBuff[0] = data;
					pktLen[NodeIndex] = 1;
					sum[NodeIndex] = data;
				}
				else
				{
					step[NodeIndex] = BSW_SRV_CAN_FIVE;
				}
			break;
			case BSW_SRV_CAN_LEN:
				pktBuff[pktLen[NodeIndex]++] = data;
				sum[NodeIndex] += data;
				if (2 == ++len[NodeIndex])
				{
					len[NodeIndex] = (pktBuff[pktLen[NodeIndex]-1]<<8) | pktBuff[pktLen[NodeIndex]-2];
					if ((256 - CAN_AA_55_SRC_DEST_LEN) < len[NodeIndex])
					{
						step[NodeIndex] = BSW_SRV_CAN_FIVE;
						CL_LOG("len=%d,接收数据包的长度错误 \n", len[NodeIndex]);
					}
					else
					{
						step[NodeIndex] = BSW_SRV_CAN_SN;
					}
				}
			break;
			case BSW_SRV_CAN_SN:
				pktBuff[pktLen[NodeIndex]++] = data;
				sum[NodeIndex] += data;
				step[NodeIndex] = BSW_SRV_CAN_CMD;
			break;
			case BSW_SRV_CAN_CMD:
				pktBuff[pktLen[NodeIndex]++] = data;
				sum[NodeIndex] += data;
				step[NodeIndex] = BSW_SRV_CAN_RX_DATA;
			break;
			case BSW_SRV_CAN_RX_DATA:
				pktBuff[pktLen[NodeIndex]++] = data;
				sum[NodeIndex] += data;
				if (0 == --len[NodeIndex])
				{
					step[NodeIndex] = BSW_SRV_CAN_CHK;
				}
			break;
			case BSW_SRV_CAN_CHK:
				pktBuff[pktLen[NodeIndex]++] = data;
				if (data == sum[NodeIndex])
                {
					step[NodeIndex] = BSW_SRV_CAN_END;
					len[NodeIndex] = 0;
				}
				else
				{
					CL_LOG("接收数据校验错误,sum = 0x%x,pkt sum = 0x%x.\n",sum[NodeIndex],data);
					PrintfData("24GRecvTask", pktBuff, pktLen[NodeIndex]);
					step[NodeIndex] = BSW_SRV_CAN_FIVE;
				}
			break;
			case BSW_SRV_CAN_END:
            	pAppCanPkt = (void*)pktBuff;
				if(GlobalInfo.AppCan_HandleCallBack != NULL)
				{
					GlobalInfo.AppCan_HandleCallBack(pAppCanPkt->data, pAppCanPkt->head.len + CAN_AA_55_SRC_DEST_LEN, pAppCanPkt->head.cmd, pAppCanPkt->head.src_SA);
				}
				else
				{
					AppCan_CallBack(pAppCanPkt->data, pAppCanPkt->head.len + CAN_AA_55_SRC_DEST_LEN, pAppCanPkt->head.cmd, pAppCanPkt->head.src_SA);
				}
				step[NodeIndex] = BSW_SRV_CAN_FIVE;
			break;
			default:
				step[NodeIndex] = BSW_SRV_CAN_FIVE;
			break;
		}
	}
}
#endif


