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





void AppCan_ShakeHand(uint8_t *data)
{
	APP_CAN_STR_t AppCanBuff = {0,};
	APP_CAN_SHAKE_HAND_STR *CanShakeHand = (void*)AppCanBuff.data;
		
	CanShakeHand->gun = 1;
    //发送握手消息
    
}

void AppCan_ShakeHandAck(uint8_t *data)
{
	//APP_CAN_SHAKE_HAND_ACK_STR *CanShakeHandack = (void*)data;
	
	//CanShakeHandack->gun;
    return;
}

void App_CanFrameHandle(uint8_t *data, uint16_t len, uint8_t cmd)
{
    switch (cmd)
    {
        case CAN_CMD_HAND:
            AppCan_ShakeHandAck(data);
		break;
        default:
            CL_LOG("no find module..\r\n");
		break;
    }
}

void AppCan_CallBack(uint8_t *data, uint16_t len, uint8_t cmd)
{
    App_CanFrameHandle(data, len, cmd);
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
void AppCan_SingleFrameHandle(uint8_t *data, uint16_t len, uint8_t cmd)
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
				GlobalInfo.AppCan_HandleCallBack(GlobalInfo.CanReceiveMessage.RxBuff, GlobalInfo.CanReceiveMessage.Len, GlobalInfo.CanReceiveMessage.Cmd);
			}
			else
			{
				AppCan_CallBack(GlobalInfo.CanReceiveMessage.RxBuff, GlobalInfo.CanReceiveMessage.Len, GlobalInfo.CanReceiveMessage.Cmd);
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
					if ((256 - CAN_AA_55_LEN) < len[NodeIndex])
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
                AppCan_CallBack(pAppCanPkt->data, pAppCanPkt->head.len + CAN_AA_55_LEN, pAppCanPkt->head.cmd);
				step[NodeIndex] = BSW_SRV_CAN_FIVE;
			break;
			default:
				step[NodeIndex] = BSW_SRV_CAN_FIVE;
			break;
		}
	}
}
#endif


