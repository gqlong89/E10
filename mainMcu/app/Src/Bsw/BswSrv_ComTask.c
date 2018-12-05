/*
 * @Author: zhoumin 
 * @Date: 2018-10-26 19:13:14 
 * @def :����ͨ������-ˢ�����ȡ/��λ������
 * @Last Modified by: zhoumin
 * @Last Modified time: 2018-11-02 18:01:00
 */

#include "BswSrv_ComTask.h"
#include "BswDrv_Rtc.h"
#include "BswSrv_System.h"
#include "BswDrv_Usart.h"
#include "BswSrv_CardBoard.h"
#include "BswSrv_ConfigTool.h"
#include "BswSrv_FwUpgrade.h"




void ComTask(void)
{
	uint32_t tick = 0;

  	ComRxSem = osSemaphoreCreate(NULL,10);

	while(1)
	{
		if(osOK == osSemaphoreWait(ComRxSem,1000))
		{
			BswSrv_CB_RecvData();			//����ˢ��������
			BswSrv_ConfigTool_RecvData();	//������λ������
		}
		
		//��������
		if((uint32_t)(osGetTimes() - GlobalInfo.lastRecvCBTime) > (1000*180)) 
		{
			GlobalInfo.lastRecvCBTime = osGetTimes();
			GlobalInfo.CBInitOK = 0;
			//����������
			BswSrv_CB_Reset();
		}

		//60���Ӽ����һ�ΰ������Ƿ�������
		if(GlobalInfo.CBInitOK && (uint32_t)(osGetTimes() - tick) > (1000*3600))
		{
			tick = osGetTimes();
			if(BswSrv_Upgrade_ReadHeadInfo(FW_U8_BAK, NULL) == CL_OK)
			{
				CL_LOG("��⵽ˢ�����и���..\r\n");
				BswSrv_StartCardBoard_UpgradeTask();
			}
		}
	}
}





