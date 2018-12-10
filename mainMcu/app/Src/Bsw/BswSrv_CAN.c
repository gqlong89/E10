/*
 * @Author: quqian 
 * @Date: 2018-12-05 14:13:14 
 * @def :CAN总线中间层
 * @Last Modified by: quqian
 * @Last Modified time: 2018-12-06 18:01:00
 */

#include "BswSrv_CAN.h"
#include "BswDrv_Rtc.h"
#include "BswSrv_System.h"
#include "BswDrv_Usart.h"
#include "BswSrv_CardBoard.h"
#include "BswSrv_ConfigTool.h"
#include "BswSrv_FwUpgrade.h"
#include "BswDrv_Debug.h"
#include "App_CAN.h"

CAN_BSW_SRV_BUFF_STR CanBuffNode[CAN_NODE_NUMB] = {0,};		//每个节点对应的buff
CAN_BSW_FIFO_STR CanBswSrvFifo[CAN_NODE_NUMB] = {0,};	


int BswSrv_CanGetOneData(int portIndex, uint8_t *pData)
{
    CAN_BSW_FIFO_STR *pCan= &CanBswSrvFifo[portIndex];

    return FIFO_S_Get(&pCan->CanRxBuff, pData);
}

void BswSrv_CanFifoInit(void)
{
	uint32_t Index = 0;
	
	for(Index = 0; Index < CAN_NODE_NUMB; Index++)
	{
		FIFO_S_Init(&CanBswSrvFifo[Index].CanRxBuff, (void*)CanBuffNode[Index].UnionCanBuff.CanBswSrvRxBuff, sizeof(CanBuffNode[Index].UnionCanBuff.CanBswSrvRxBuff));
		FIFO_S_Flush(&CanBswSrvFifo[Index].CanRxBuff);
	}
}



/*****************************************************************************
** Function name:       BswSrv_CanRecvDataToCache
** Descriptions:        将底层数据解析出来, 放入BswSrvCache缓存被应用层调用
** input parameters:    None
** output parameters:   None
** Returned value:	  	
** Author:              quqian
*****************************************************************************/
int BswSrv_CanRecvDataToCache(uint8_t NodeIndex)
{
	uint8_t BswSrvCanCache[256] = {0,};
	uint8_t *pktBuff = (void*)BswSrvCanCache;	
    static uint8_t step = BSW_SRV_CAN_FIVE;
    uint8_t data;
    static uint8_t sum = 0;
    static uint16_t pktLen = 0;
    static uint16_t len = 0;
	uint8_t pktNum = 0;
	static uint16_t WaitFlag[CAN_NODE_NUMB] = {0,};
	APP_CAN_STR_t *pCanPkt = NULL;
		
	while (CL_OK == BswSrv_CanGetOneData(NodeIndex, &data))
    {
		//printf("data = 0x%x ", data);
		BswDrv_FeedWatchDog();
		switch (step)
		{
			case BSW_SRV_CAN_FIVE:
				if (data == 0x55)
				{
					step = BSW_SRV_CAN_AA;
					pktBuff[0] = data;
					pktLen = 1;
					sum = data;
				}
				else
				{
					WaitFlag[NodeIndex]++;
					if((CAN_WAIT_TIMES >= WaitFlag[NodeIndex]) && (CAN_WRITE_DATA_LEN >= (CanBswSrvFifo[NodeIndex].CanRxBuff.writeIndex - CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex)))
					{
						CanBswSrvFifo[NodeIndex].CanRxBuff.readIndex = CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex;
						return 1;
					}
				}
			break;
			case BSW_SRV_CAN_AA:
				if (data == 0xAA)
				{
					step = BSW_SRV_CAN_LEN;
					pktBuff[1] = data;
					pktLen++;
					sum += data;
				}
				else if (data == 0x55)
				{
					step = BSW_SRV_CAN_AA;
					pktBuff[0] = data;
					pktLen = 1;
					sum = data;
				}
				else
				{
					step = BSW_SRV_CAN_FIVE;
					WaitFlag[NodeIndex]++;
					if((CAN_WAIT_TIMES >= WaitFlag[NodeIndex]) && (CAN_WRITE_DATA_LEN >= (CanBswSrvFifo[NodeIndex].CanRxBuff.writeIndex - CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex)))
					{
						CanBswSrvFifo[NodeIndex].CanRxBuff.readIndex = CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex;
						return 1;
					}
				}
			break;
			case BSW_SRV_CAN_LEN:
				pktBuff[pktLen++] = data;
				sum += data;
				if (2 == ++len)
				{
					len = (pktBuff[pktLen-1]<<8) | pktBuff[pktLen-2];
					if ((256 - CAN_AA_55_LEN) < len)	//长度域：包含从 序列号 到 校验和域 的所有字节数 
					{
						CL_LOG("len=%d,接收数据包的长度错误 \n", len);
						step = BSW_SRV_CAN_FIVE;
						WaitFlag[NodeIndex]++;
						if((CAN_WAIT_TIMES >= WaitFlag[NodeIndex]) && (CAN_WRITE_DATA_LEN >= (CanBswSrvFifo[NodeIndex].CanRxBuff.writeIndex - CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex)))
						{
							CanBswSrvFifo[NodeIndex].CanRxBuff.readIndex = CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex;
							return 1;
						}
					}
					else
					{
						step = BSW_SRV_CAN_SN;
					}
				}
			break;
			case BSW_SRV_CAN_SN:
				pktBuff[pktLen++] = data;
				sum += data;
				step = BSW_SRV_CAN_CMD;
			break;
			case BSW_SRV_CAN_CMD:
				pktBuff[pktLen++] = data;
				sum += data;
				step = BSW_SRV_CAN_RX_DATA;
			break;
			case BSW_SRV_CAN_RX_DATA:
				pktBuff[pktLen++] = data;
				sum += data;
				if (0 == --len)
				{
					step = BSW_SRV_CAN_CHK;
				}
			break;
			case BSW_SRV_CAN_CHK:
				pktBuff[pktLen++] = data;
				if (data == sum)
                {
					step = BSW_SRV_CAN_END;
					len = 0;
				}
				else
				{
					CL_LOG("接收数据校验错误,sum = 0x%x,pkt sum = 0x%x.\n",sum, data);
					PrintfData("接收校验错误数据", pktBuff, pktLen);
					step = BSW_SRV_CAN_FIVE;
					WaitFlag[NodeIndex]++;
					if((CAN_WAIT_TIMES >= WaitFlag[NodeIndex]) && (CAN_WRITE_DATA_LEN >= (CanBswSrvFifo[NodeIndex].CanRxBuff.writeIndex - CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex)))
					{
						CanBswSrvFifo[NodeIndex].CanRxBuff.readIndex = CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex;
						return 1;
					}
				}
			break;
			case BSW_SRV_CAN_END:
				if (CAN_R_N_LEN == ++len)
                {
                	pCanPkt = (void*)pktBuff;
                    if((256 - CAN_AA_55_LEN) < pCanPkt->head.len)
                    {
                        CL_LOG("pCanPkt数据长度接收错误！\n");
						step = BSW_SRV_CAN_FIVE;
						WaitFlag[NodeIndex]++;
						if((CAN_WAIT_TIMES >= WaitFlag[NodeIndex]) && (CAN_WRITE_DATA_LEN >= (CanBswSrvFifo[NodeIndex].CanRxBuff.writeIndex - CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex)))
						{
							CanBswSrvFifo[NodeIndex].CanRxBuff.readIndex = CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex;
							return 1;
						}
                    }
					#if USER_ANOTHER_THREAD
                    for (len = 0; len < (pCanPkt->head.len + CAN_AA_55_LEN); ) 
                    {
                        if (CL_OK == FIFO_S_Put(&CanBswSrvAppFifo[NodeIndex].CanRxBuff, (&pCanPkt->head.five)[len]))
                        {
                            len++;
                        }
                        else
						{
                            CL_LOG("pCanPkt buff over flow error.\n");
                            vTaskDelay(5);
                        }
                    }
					#else
					AppCan_CallBack(pCanPkt->data, pCanPkt->head.len + CAN_AA_55_LEN, pCanPkt->head.cmd);
					#endif
					step = BSW_SRV_CAN_FIVE;
					WaitFlag[NodeIndex] = 0;
					CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex = CanBswSrvFifo[NodeIndex].CanRxBuff.readIndex;
                    if(RECEIVE_MAX_PKT_NUM <= ++pktNum)
                    {
                        pktNum = 0;
                        return 0;
                    }
				}
			break;
			default:
				step = BSW_SRV_CAN_FIVE;
				WaitFlag[NodeIndex]++;
				if((CAN_WAIT_TIMES >= WaitFlag[NodeIndex]) && (CAN_WRITE_DATA_LEN >= (CanBswSrvFifo[NodeIndex].CanRxBuff.writeIndex - CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex)))
				{
					CanBswSrvFifo[NodeIndex].CanRxBuff.readIndex = CanBswSrvFifo[NodeIndex].CanRxBuff.lastReadIndex;
					return 1;
				}
			break;
		}
	}

	return 1;
}


void BswSrv_ReadDataToCache(void)
{
	uint32_t nodeIndex = 0;
	int ret = 0;
	uint32_t DelayFlag = 0;
	
	for(nodeIndex = 0; nodeIndex < CAN_NODE_NUMB; nodeIndex++)
	{
		ret = BswSrv_CanRecvDataToCache(nodeIndex);
		if(1 == ret)
		{
			DelayFlag++;
		}
	}
	if(1 <= DelayFlag)
	{
		DelayFlag = 0;
		osDelay(20);
	}
}


