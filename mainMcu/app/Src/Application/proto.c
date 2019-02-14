/*proto.c
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "proto.h"
//#include "card.h"
//#include "aes.h"
#include "tfs.h"
//#include "md5.h"
//#include "ui.h"
//#include "emu.h"
#include "BswDrv_Rtc.h"
#include "GM510.h"
#include "BswDrv_sc8042b.h"
#include "BswDrv_Sys_Flash.h"
#include "BswSrv_FlashUsr.h"
#include "server.h"
#include "BswSrv_System.h"


uint8_t req_cnt[GUN_NUM_MAX] = {0};
uint8_t gProtoSendBuff[OUT_NET_PKT_LEN];
uint8_t gCardReqFlag = 0;
uint8_t  gSendHearBeatCnt = 0;
uint8_t AesKeyUpdateFlag=0;
uint8_t  gID2[TFS_ID2_LEN+1] = {0};
uint8_t SysToken[16] = "1234567891234567";


void UpdateSysToken(void)
{
    if (0x7e < ++SysToken[0]) {
        SysToken[0] = 0x21;
    }
}


//int GetPktSum(uint8_t *pData, uint16_t len)
//{
//    int i;
//    uint8_t  sum = 0;

//    for (i=0; i<len; i++) {
//        sum += pData[i];
//    }
//    return sum;
//}


int GetNoZeroSum(uint8_t *pData, uint16_t len)
{
    int sum = GetPktSum(pData, len);

    if (0 == sum) {
        sum = 1;
    }
    return sum;
}


int CheckCostTemplate(void)
{
    return CL_OK;
}


//pData 指向静荷 len静荷长度
int SendProtoPkt(uint16_t sn, uint8_t cmd, PKT_STR *pPkt, uint16_t len, uint8_t decrypetFlag)
{
//    uint16_t encryLen;

    if (gChgInfo.sendPktFlag) 
	{
        CL_LOG("sendPktFlag=%d,can not send.\n",gChgInfo.sendPktFlag);
        return CL_FAIL;
    }

    pPkt->head.aa = 0xaa;
    pPkt->head.five = 0x55;
    pPkt->head.type = CHARGER_TYPE;
    if ((MQTT_CMD_REGISTER == cmd) || (MQTT_CMD_AES_REQ == cmd) || (MQTT_CMD_UPDATE_AES_NOTICE == cmd)) 
	{
        memcpy(pPkt->head.chargerSn, SystemInfo.stationId, sizeof(pPkt->head.chargerSn));
    }
	else
	{
        memcpy(pPkt->head.chargerSn, SystemInfo.idCode, sizeof(SystemInfo.idCode));
    }
    pPkt->head.len = len + 5;
    pPkt->head.ver = MESSAGE_VER_NOENCRYPT;
    pPkt->head.sn = sn;
    pPkt->head.cmd = cmd;
    pPkt->data[len] = GetPktSum((void*)&pPkt->head.ver, len+4);

 //   if (LOCAL_NET != SystemInfo.netType) 
//	{ //非本地网络发送不加密
 //       return PutOutNetPkt((void*)pPkt, sizeof(PKT_HEAD_STR)+len+1, REQ_SEND_PKT);
 //   }
    if (0 == decrypetFlag) 
	{
        return GM510SocketSend(1, (void*)pPkt, sizeof(PKT_HEAD_STR)+len+1);
    }
    
    return 0;
}


//调用该函数需要注意是否发生递归
int SendSysLog(uint8_t gunId, uint8_t logType, char *pStr)
{
    return CL_OK;
}


int CheckStartPara(uint8_t gun_id)
{    
	return CL_OK;
}


int SendRegister(void)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
    REGISTER_REQ_STR *msg = (void*)pkt->data;

    xSemaphoreTake(gProtoSendMux,1000);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    memcpy(msg->device_type, STATION_MACHINE_TYPE, strlen(STATION_MACHINE_TYPE));
    memcpy(msg->register_code, REGISTER_CODE, strlen(REGISTER_CODE));
	memcpy(msg->hwId, SystemInfo.iccid, sizeof(msg->hwId));
	PrintfData("SendRegister origin data", (void*)msg, sizeof(REGISTER_REQ_STR));
	SendProtoPkt(SystemInfo.mqtt_sn++, MQTT_CMD_REGISTER, (void*)&pkt->protoHead, sizeof(REGISTER_REQ_STR), ID2);
    xSemaphoreGive(gProtoSendMux);
    return CL_OK;
}


//flag 1:上电启动			2:离线恢复重发
int SendStartUpNotice(int flag)
{
   
    return CL_OK;
}


// flag: 1：仅查询余额 0：刷卡鉴权，开始充电
int SendCardAuthReq(int flag)
{
    return CL_OK;
}


int SendEventNotice(uint8_t gunId, uint8_t event, uint8_t para1, uint32_t para2, uint8_t status, char *pDisc)
{
    
    return CL_OK;
}


int SendStartChargingNotice(int gun_id, uint32_t startTime, int flag)
{
   
    return CL_OK;
}


int SendStartChargingAck(uint8_t gunId, int sn, int result, int reason)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	START_CHARGING_ACK_STR *mqtt_remote_set_power_on_ack = (void*)pkt->data;

    xSemaphoreTake(gProtoSendMux,1000);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    mqtt_remote_set_power_on_ack->gun_id = gunId;
	mqtt_remote_set_power_on_ack->result = result;
    mqtt_remote_set_power_on_ack->failReason = reason;
	PrintfData("SendStartChargingAck origin data", (void*)mqtt_remote_set_power_on_ack, sizeof(START_CHARGING_ACK_STR));
	SendProtoPkt(sn, MQTT_CMD_REMOTE_SET_POWER_ON, (void*)&pkt->protoHead, sizeof(START_CHARGING_ACK_STR), ID2);
    xSemaphoreGive(gProtoSendMux);
    return CL_OK;
}


int SendTradeRecordNotice(HISTORY_ORDER_STR *order)
{
    return CL_OK;
}


int SendStopChargingAck(uint8_t gunId, int sn, int result)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	STOP_CHARGING_ACK_STR *mqtt_remote_set_power_off_ack = (void*)pkt->data;

   xSemaphoreTake(gProtoSendMux,1000);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    mqtt_remote_set_power_off_ack->gun_id = gunId;
	mqtt_remote_set_power_off_ack->result = result;
	PrintfData("SendStopChargingAck origin data", (void*)mqtt_remote_set_power_off_ack, sizeof(STOP_CHARGING_ACK_STR));
	SendProtoPkt(sn, MQTT_CMD_REMOTE_SET_POWER_OFF, (void*)&pkt->protoHead, sizeof(STOP_CHARGING_ACK_STR), ID2);
    xSemaphoreGive(gProtoSendMux);
    return CL_OK;
}

int SendHeartBeatFun(uint8_t gun_id)
{

    return CL_OK;
}


void HeartBeatHandle(void)
{
	if (CL_OK == IsSysOnLine()) 
	{
		SendHeartBeatFun(0);
        gSendHearBeatCnt++;
        if (2 < gSendHearBeatCnt) 
		{
            gSendHearBeatCnt = 0;
            SystemInfo.is_socket_0_ok = CL_FALSE;
            CL_LOG("long time no recv heart beat ack,error.\n");
            gChgInfo.netStatus |= 0x01;
            OptFailNotice(111);
        }
	}
}


int SendReqCostTemplate(uint8_t gunId)
{
    return 0;
}


void CostTemplateReq(void)
{
}


int SendCostTemplateAck(uint32_t sn, uint8_t result, uint8_t gunId)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
    COST_TEMPLATE_ACK_STR *mqtt_cost_down_ack = (void*)pkt->data;

	xSemaphoreTake(gProtoSendMux,1000);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    mqtt_cost_down_ack->gunId = gunId;
    mqtt_cost_down_ack->result = result;
	PrintfData("SendCostTemplateAck origin data", (void*)mqtt_cost_down_ack, sizeof(COST_TEMPLATE_ACK_STR));
	SendProtoPkt(sn, MQTT_CMD_COST_DOWN, (void*)&pkt->protoHead, sizeof(COST_TEMPLATE_ACK_STR), ID2);
    xSemaphoreGive(gProtoSendMux);
    return CL_OK;
}


int SendDeviceAesReq(uint32_t time_utc, uint8_t reason)
{
	
    return CL_OK;
}


int SendUpdateAesAck(uint16_t sn, uint8_t result)
{
	FRAME_STR *pkt = (void*)gProtoSendBuff;
    UPDATE_AES_ACK_STR *pAesAckStr = (void*)pkt->data;

    xSemaphoreTake(gProtoSendMux,1000);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    pAesAckStr->result = result;
	PrintfData("SendUpdateAesAck", (void*)pkt->data, sizeof(UPDATE_AES_ACK_STR));
	SendProtoPkt(sn, MQTT_CMD_UPDATE_AES_NOTICE, (void*)&pkt->protoHead, sizeof(UPDATE_AES_ACK_STR), 0);
    xSemaphoreGive(gProtoSendMux);
    return CL_OK;
}



void StartCharging(uint8_t startMode, uint32_t money, uint8_t *pCardSn, uint8_t *pOrder, uint8_t ordersource)
{
    
}


void StopCharging(uint8_t gunId)
{
}


int CostTempCopy(COST_TEMPLATE_HEAD_STR *pcost)
{
    int ret = CL_OK;
    return ret;
}


void CostTemplateProc(PKT_STR *pPkt)
{
    COST_TEMPLATE_HEAD_STR *pcost = (void*)pPkt->data;
    int ret = CostTempCopy((void*)pPkt->data);

    SendCostTemplateAck(pPkt->head.sn, ret, pcost->gunId);
}


void SetUpgradeInfo(DOWN_FW_REQ_STR *pfwInfo)
{
    char url[50] = {0};
    char usrName[6] = {0};
    char psw[6] = {0};
    char fileName[10] = {0};
    int i;

//	vTaskDelete(MainTaskHandle_t);
//    vTaskDelete(EmuTaskHandle_t);
//	vTaskDelete(BlueTaskHandle_t);
//	vTaskDelete(CkbTaskHandle_t);
    gChgInfo.lastOpenTime = GetRtcCount();
    gChgInfo.sendPktFlag = 2;
    memcpy(url, pfwInfo->url, sizeof(pfwInfo->url));
    memcpy(usrName, pfwInfo->usrName, sizeof(pfwInfo->usrName));
    memcpy(psw, pfwInfo->psw, sizeof(pfwInfo->psw));
    memcpy(fileName, pfwInfo->fileName, sizeof(pfwInfo->fileName));
    CL_LOG("url=%s,usrName=%s,psw=%s,fileName=%s,checkSum=%#x.\n",url,usrName,psw,fileName,pfwInfo->checkSum);
    for (i=0; i<4; i++) 
	{
        Sc8042bSpeech(VOIC_START_UPGRADE);
        if (CL_OK == GM510FtpGet(url, usrName, psw, fileName, pfwInfo->checkSum)) 
		{
            break;
        }
        OS_DELAY_MS(5000);
    }
    ResetSysTem();
}


void SendRemoCtrlAck(PKT_STR *pRemoCtrlReq, uint8_t result)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
    BLUE_REMO_CTRL_ACK_STR *pRemoCtrlack = (void*)pkt->data;
    REMO_CTRL_REQ_STR *pReq = (void*)pRemoCtrlReq->data;

    xSemaphoreTake(gProtoSendMux,1000);
    pRemoCtrlack->optCode = pReq->optCode;
    pRemoCtrlack->para = pReq->para;
    pRemoCtrlack->result = result;
    PrintfData("SendRemoCtrlAck", (void*)pRemoCtrlack, sizeof(BLUE_REMO_CTRL_ACK_STR));
	SendProtoPkt(pRemoCtrlReq->head.sn, MQTT_CMD_REMOTE_CTRL, (void*)&pkt->protoHead, sizeof(BLUE_REMO_CTRL_ACK_STR), 0);
    xSemaphoreGive(gProtoSendMux);
}


void SetChargingTime(PKT_STR *pFrame)
{
    
}

void SetPrintfSwitch(PKT_STR *pFrame)
{

}


void SetPullGunStop(PKT_STR *pFrame)
{

}

void SetChargeChangePowerFuncy(uint8_t chargePower)
{

}

void RemoSetChargeChangePower(PKT_STR *pFrame)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pFrame->data;

    SetChargeChangePowerFuncy(pReq->para);
    SendRemoCtrlAck(pFrame, CL_OK);
}

void SetChargerFullStop(PKT_STR *pFrame)
{
    
}


void RemoteCtrlProc(PKT_STR *pFrame)
{
    switch (pFrame->data[0]) {
        case SYSTEM_REBOOT:
            CL_LOG("reboot system now.\n");
            SendRemoCtrlAck(pFrame, CL_OK);
            ResetSysTem();
            break;
        case CTRL_OPEN_GUN:
        //    RemoteOpenGun(pFrame);
            break;
        case CTRL_CLOSE_GUN:
        //    RemoteCloseGun(pFrame);
            break;
        case CTRL_SET_FULL_TIME:
			SetChargingTime(pFrame);
            break;
        case CTRL_SET_PULL_GUN_TIME:
           // SetPullGunStopTime(pFrame);
            break;
        case CTRL_SET_PULL_GUN_STOP:
            SetPullGunStop(pFrame);
            break;
        case CTRL_SET_PRINT_SWITCH:
            SetPrintfSwitch(pFrame);
            break;
		case CTRL_SET_DISTURBING_TIME:
			
			break;
        case CTRL_SET_CHARGING_FULL_STOP:
            SetChargerFullStop(pFrame);
            break;
		case CTRL_SET_CHARGE_CHANGE_POWER:
            RemoSetChargeChangePower(pFrame);
            break;
		default:
			break;
    }
}




int RecvServerData(PKT_STR *pFrame, uint16_t len)
{
    uint32_t now = GetRtcCount();

	if (CHARGER_TYPE != pFrame->head.type) 
	{
        CL_LOG("type=%d,error, pkt drop.\n",pFrame->head.type);
        OptFailNotice(110);
        return CL_FAIL;
    }

	if ((MQTT_CMD_REGISTER == pFrame->head.cmd) || (MQTT_CMD_AES_REQ == pFrame->head.cmd) || 
		(MQTT_CMD_UPDATE_AES_NOTICE == pFrame->head.cmd)) 
	{
        if (memcmp(pFrame->head.chargerSn, SystemInfo.stationId, sizeof(pFrame->head.chargerSn))) 
		{
            CL_LOG("sn diff error, pkt drop,cmd=%d.\n",pFrame->head.cmd);
            OptFailNotice(109);
            return CL_FAIL;
        }
    } 
	else 
	{
		if (memcmp(pFrame->head.chargerSn, SystemInfo.idCode, 8)) 
		{
			CL_LOG("idCode diff error, pkt drop,cmd=%d.\n",pFrame->head.cmd);
			OptFailNotice(109);
			return CL_FAIL;
		}
    }

	switch (pFrame->head.cmd) 
	{
        case MQTT_CMD_REGISTER:
        {
            PrintfData("RecvServerData: register ack", (void*)pFrame, len);
        }
        break;

        case MQTT_CMD_START_UP:
        {
            PrintfData("RecvServerData: start up ack", (void*)pFrame, len);
            
        }
        break;

        case MQTT_CMD_CARD_ID_REQ:
        {
            
        }
        break;

        case MQTT_CMD_REMOTE_SET_POWER_ON:
        {
            
        }
        break;

        case MQTT_CMD_REMOTE_SET_POWER_OFF:
        {
            
        }
        break;

        case MQTT_CMD_REPORT_POWER_ON://开启充电通知
        {
            
        }
        break;

        case MQTT_CMD_REPORT_POWER_OFF://停止充电通知响应
        {
            
        }
        break;

        case MQTT_CMD_HEART_BEAT:
        {
            
        }
        break;

        case MQTT_CMD_COST_DOWN:
        {
            PrintfData("RecvServerData: recv cost template", (void*)pFrame, len);
        }
        break;

        case MQTT_CMD_DFU_DOWN_FW_INFO:
        {
            
        }
        break;

        case MQTT_CMD_REMOTE_CTRL:
        {
            
        }
        break;

        case MQTT_CMD_EVENT_NOTICE:
            
		break;
		
	#if (1 == ID2)
		case MQTT_CMD_AES_REQ://秘钥请求回复
		{
			//AesInfoHandle(pFrame, len);
		}
		break;

		case MQTT_CMD_UPDATE_AES_NOTICE:
		{
			PrintfData("RecvServerData: server update aes notice", (void*)pFrame, len);
			UPDATE_AES_NOTICE_STR* pUpdateAesNotic = (UPDATE_AES_NOTICE_STR*)pFrame->data;
			CL_LOG("server update aes notice, reason: %d.\n", pUpdateAesNotic->reason);
			//更新系统时间
			SycTimeCount(pUpdateAesNotic->time_utc);
			SetRtcCount(pUpdateAesNotic->time_utc);
			SendUpdateAesAck(pFrame->head.sn, 0);
            AesKeyUpdateFlag = 0;
            gChgInfo.ReqKeyReason = 2;
            gChgInfo.netStatus |= (1<<7);
            SoundCode(34);
		}
		break;
	#endif

        default:
            CL_LOG("mqtt unkown command %d.\n",pFrame->head.cmd);
            OptFailNotice(108);
		break;
	}
    
    return 0;
}



