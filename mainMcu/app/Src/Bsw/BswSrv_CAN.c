/*
 * @Author: quqian 
 * @Date: 2018-12-05 14:13:14 
 * @def :CAN�����м��
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


CAN_BSW_SRV_BUFF_STR CanBuffNode[CAN_NODE_NUMB] = {0,};		//ÿ���ڵ��Ӧ��buff

uint8_t BswSrvCache[256] = {0,};	//���ڻ����������ݰ�





