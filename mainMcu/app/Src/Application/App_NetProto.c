/*
 * @Author: zhoumin 
 * @Date: 2018-10-12 14:14:54 
 * @Last Modified by: zhoumin
 * @Last Modified time: 2018-11-02 17:57:35
 */

#include "App_NetProto.h"
#include "BswDrv_Rtc.h"
#include "BswDrv_Debug.h"
#include "BswDrv_GPIO.h"
#include "BswSrv_System.h"
#include "BswSrv_FlashUsr.h"
#include "BswSrv_NetTask.h"
#include "BswSrv_Air720.h"
#include "BswSrv_WG215.h"
#include "BswSrv_FwUpgrade.h"

uint8_t gProtoSendBuff[OUT_NET_PKT_LEN];

static uint16_t FrameStrSn = 0;

static int App_NetFrameHandle(uint8_t type,uint8_t *data,uint16_t len);
static int SendProtoPkt(FRAME_STR *pPkt, uint16_t len, uint8_t decrypetFlag);

static void App_NetProto_HandleRegister(FRAME_STR *pFrame);
static void App_NetProto_HandleLogin(FRAME_STR *pFrame);
static void App_NetProto_HandleAuthCard(FRAME_STR *pFrame);
static void App_NetProto_HandleHeartBat(FRAME_STR *pFrame);
static void App_NetProto_HandleUpgrade(FRAME_STR *pFrame);
static void App_NetProto_HandleRemoteCtrl(FRAME_STR *pFrame);
static void App_NetProto_HandleEventNotif(FRAME_STR *pFrame);
static void App_NetProto_HandleDownDevList(FRAME_STR *pFrame);
static void App_NetProto_HandleBindDeNotif(FRAME_STR *pFrame);

/**
 * ����������ݻص�����
 * netType:1--����2G/4G 2--485 3--2.4G 4--wifi
 */
void BswSrv_Net_RxData_Callback(uint8_t netType,uint8_t *data,uint16_t len)
{
	App_NetFrameHandle(netType,data,len);
}


int SendProtoPkt(FRAME_STR *pPkt, uint16_t len, uint8_t decrypetFlag)
{
    pPkt->head.aa = 0xaa;
    pPkt->head.five = 0x55;
    pPkt->head.type = DEVICE_TYPE;
    if ((MQTT_CMD_REGISTER == pPkt->head.cmd) || (MQTT_CMD_AES_REQ == pPkt->head.cmd) || (MQTT_CMD_UPDATE_AES_NOTICE == pPkt->head.cmd)) {
        memcpy(pPkt->head.chargerSn, SystemInfo.stationId, sizeof(pPkt->head.chargerSn));
    }else{
        memcpy(pPkt->head.chargerSn, SystemInfo.idCode, sizeof(SystemInfo.idCode));
    }
    pPkt->head.len = len + 5;
    pPkt->head.ver = (decrypetFlag == 0) ? MESSAGE_VER_NOENCRYPT : MESSAGE_VER_ENCRYPT;
    pPkt->head.sn = FrameStrSn++;
    pPkt->data[len] = (BswSrv_Tool_CheckSum((void*)&pPkt->head.ver, len+4) & 0x00FF);

    if (0 == decrypetFlag) {
        return BswSrv_SendSokcetData((uint8_t*)pPkt, sizeof(PKT_HEAD_STR)+len+1);
    }
	return CL_OK;
}


/*******************************************************************************************************/
//
/*******************************************************************************************************/

/**
 * ע��
 */
int App_NetProto_SendRegister(void)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
    REGISTER_REQ_STR *msg = (void*)pkt->data;

	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    memcpy(msg->device_type, STATION_MACHINE_TYPE, strlen((char*)STATION_MACHINE_TYPE));
    memcpy(msg->register_code, REGISTER_CODE, strlen(REGISTER_CODE));
	BswSrv_GetHWId(msg->HWId);

	pkt->head.cmd = MQTT_CMD_REGISTER;

	PrintfData("SendRegister origin data", (void*)msg, sizeof(REGISTER_REQ_STR));
	
	return SendProtoPkt(pkt,sizeof(REGISTER_REQ_STR),ID2);
}

/**
 * ��½
 */
int App_NetProto_SendStartUpNotice(int flag)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	START_UP_REQ_STR *mqtt_start_up_req = (void*)pkt->data;

	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));

    memcpy(mqtt_start_up_req->device_type, STATION_MACHINE_TYPE, strlen(STATION_MACHINE_TYPE));
    memcpy((void*)&mqtt_start_up_req->chargerSn[3], (void*)&SystemInfo.stationId[3], sizeof(mqtt_start_up_req->chargerSn)-3);
	mqtt_start_up_req->fw_version = FW_VERSION;
    mqtt_start_up_req->fw_1_ver = FW_VERSION_SUB1;
    memcpy(mqtt_start_up_req->sim_iccid, GlobalInfo.iccid, ICCID_LEN);
    mqtt_start_up_req->onNetWay = GlobalInfo.netType;
    mqtt_start_up_req->modeType = GlobalInfo.modeType;
	mqtt_start_up_req->login_reason = flag;
    mqtt_start_up_req->gun_number = 0;
    mqtt_start_up_req->device_status = 0;
    mqtt_start_up_req->statistics_info[0] = GlobalInfo.gSimStatus;
    mqtt_start_up_req->statistics_info[1] = GlobalInfo.gSimStatus>>8;
    mqtt_start_up_req->statistics_info[2] = GlobalInfo.gSimStatus>>16;
    mqtt_start_up_req->statistics_info[3] = 0;
    mqtt_start_up_req->statistics_info[4] = 0;
    mqtt_start_up_req->statistics_info[5] = 0;
    mqtt_start_up_req->statistics_info[6] = 0;
    mqtt_start_up_req->statistics_info[7] = 0;
	
	if(GlobalInfo.netType == NETTYPE_WIFI)
	{
		mqtt_start_up_req->downloadType = 1;
	}
	else{
		mqtt_start_up_req->downloadType = 0;
	}

	pkt->head.cmd = MQTT_CMD_START_UP;

	PrintfData("SendStartUpNotice origin data", (void*)mqtt_start_up_req, sizeof(START_UP_REQ_STR));

	return SendProtoPkt(pkt,sizeof(START_UP_REQ_STR),ID2);
}


/**
 * ���Ϳ���Ȩ
 */
int App_NetProto_SendCardAuthReq(uint8_t cardId[],int flag)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	CARD_AUTH_REQ_STR *pMsg = (void*)pkt->data;

	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
	pMsg->gun_id = 0;
	pMsg->card_type = 0;
	pMsg->optType = flag;
	memcpy((void*)pMsg->card_id, cardId, 16);
	memset(pMsg->card_psw, 0, sizeof(pMsg->card_psw));
	pMsg->mode = 0;
	pMsg->chargingPara = 0;

	pkt->head.cmd = MQTT_CMD_CARD_ID_REQ;
	PrintfData("SendCardAuthReq origin data", (void*)pMsg, sizeof(CARD_AUTH_REQ_STR));

    return SendProtoPkt(pkt,sizeof(CARD_AUTH_REQ_STR),ID2);
}

/**
 * �����¼�֪ͨ
 */
int App_NetProto_SendEventNotice(uint8_t gunId, uint8_t event, uint8_t para1, uint32_t para2, uint8_t status, char *pDisc)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	EVENT_NOTICE_STR *eventNotice = (void*)pkt->data;

	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    eventNotice->gun_id = gunId;
    eventNotice->code = event;
    eventNotice->para1 = para1;
    eventNotice->para2 = para2;
    eventNotice->status = status;
    eventNotice->level = EVENT_ALARM;
    memset(eventNotice->discrip, 0, sizeof(eventNotice->discrip));
    memcpy(eventNotice->discrip, pDisc, strlen(pDisc));

	pkt->head.cmd = MQTT_CMD_EVENT_NOTICE;

    PrintfData("SendEventNotice", (void*)eventNotice, sizeof(EVENT_NOTICE_STR));

    return SendProtoPkt(pkt,sizeof(EVENT_NOTICE_STR),ID2);
}

/**
 * ��������
 */
int App_NetProto_SendHeartBeat(void)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	HEART_BEAT_REQ_STR *mqtt_heart_beat = (void*)pkt->data;

	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
	mqtt_heart_beat->netSigle = GlobalInfo.simSignal;
	mqtt_heart_beat->envTemp = BswSrv_GetCpuTemp()+50;
	mqtt_heart_beat->KberrCnt = 0;
	mqtt_heart_beat->doorState = GlobalInfo.doorState;
    mqtt_heart_beat->gunCnt = 0;	//U8ǹͷ����Ϊ0

	pkt->head.cmd = MQTT_CMD_HEART_BEAT;
    PrintfData("SendHeartBeatFun", (void*)mqtt_heart_beat, sizeof(HEART_BEAT_REQ_STR));

    return SendProtoPkt(pkt,sizeof(HEART_BEAT_REQ_STR),ID2);
}

/**
 * Զ�̿���ack
 */
void App_NetProto_SendRemoCtrlAck(FRAME_STR *pRemoCtrlReq, uint8_t result)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
    REMO_CTRL_ACK_STR *pRemoCtrlack = (void*)pkt->data;
    REMO_CTRL_REQ_STR *pReq = (void*)pRemoCtrlReq->data;

    pRemoCtrlack->optCode = pReq->optCode;
    pRemoCtrlack->result = result;

	pkt->head.cmd = MQTT_CMD_REMOTE_CTRL;
    PrintfData("SendRemoCtrlAck", (void*)pRemoCtrlack, sizeof(REMO_CTRL_ACK_STR));

	SendProtoPkt(pkt,sizeof(REMO_CTRL_ACK_STR),ID2);
}

/**
 * ������Կ֪ͨ 
 */
int App_NetProto_SendDeviceAesReq(uint32_t time_utc, uint8_t reason)
{
	//todo
    return CL_OK;
}

/**
 * ������豸
 */
int App_NetProto_SendReqBindDev(void)
{
	FRAME_STR *pkt = (void*)gProtoSendBuff;
	BIND_DEVICE_REQ_STR *bindReq = (void*)pkt->data;

	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
	bindReq->rev = 0;

	pkt->head.cmd = MQTT_CMD_REQ_BAND_DEV;

	PrintfData("SendReqBindDev", (void*)bindReq, sizeof(BIND_DEVICE_REQ_STR));
	return SendProtoPkt(pkt,sizeof(BIND_DEVICE_REQ_STR),ID2);
}

/**
 * ���豸��Ӧ
 */
void App_NetProto_SendBindDeviceAck(uint8_t result)
{
	FRAME_STR *pkt = (void*)gProtoSendBuff;
	BIND_DEVICE_ACK_STR *bindAck = (void*)pkt->data;

	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
	bindAck->result = result;

	pkt->head.cmd = MQTT_CMD_DOWN_BAND_DEV;

	PrintfData("SendBindDeviceAck", (void*)bindAck, sizeof(BIND_DEVICE_ACK_STR));
	SendProtoPkt(pkt,sizeof(BIND_DEVICE_ACK_STR),ID2);
}

/**
 * �����豸�����֪ͨ
 */
int App_NetProto_SendBindDevFinishedNotif(void)
{
	uint16_t len = 0;
	FRAME_STR *pkt = (void*)gProtoSendBuff;
	BIND_DEVICE_STR *binddev = (void*)pkt->data;
	
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
	binddev->subDeviceNum = SystemInfo.RfDev.bandSize;
	len += 2;
	for(uint16_t i = 0;i<binddev->subDeviceNum;i++)
	{
		BswSrv_RF433AddrToChar(SystemInfo.RfDev.Unit[i].address,binddev->subDeviceAdd[i]);
		len += 5;
	}

	pkt->head.cmd = MQTT_CMD_BAND_DEV_NOTIF;

	PrintfData("SendBindDeviceAck", (void*)binddev, len);
	return SendProtoPkt(pkt,len,ID2);
}


/*******************************************************************************************************/
//
/*******************************************************************************************************/

void App_NetProto_HandleRegister(FRAME_STR *pFrame)
{
	REGISTER_ACK_STR *pRegister = (void*)pFrame->data;
	
	if(pRegister->result == 0)
	{
		memcpy(SystemInfo.idCode,pRegister->idcode,8);
		FlashWriteSysInfo((void*)&SystemInfo,sizeof(SYSTEM_INFO_T));
		CL_LOG("get register dcode, reboot system now.\n");
		BswSrv_SystemReboot();
	}
	else
	{
		CL_LOG("register ack result=%d,error.\n",pRegister->result);
	}
}

void App_NetProto_HandleLogin(FRAME_STR *pFrame)
{
	START_UP_ACK_STR *pStartUp = (void*)pFrame->data;
	
	if(pStartUp->result == 0)
	{
		GlobalInfo.isLogin = 1;
		SetRtcCount(pStartUp->time_utc);
		osDelay(100);
		CL_LOG("login success.\r\n");
	}
	else
	{
		CL_LOG("login failed result=%d \r\n",pStartUp->result);
	}
}


void App_NetProto_HandleAuthCard(FRAME_STR *pFrame)
{
	CARD_AUTH_ACK_STR *pCardAuth = (void*)pFrame->data;
	CL_LOG("recv CardAuthAck,status=%d \r\n",pCardAuth->result);
	if(pCardAuth->result == 0)
	{
		CL_LOG("card author success.\r\n");
		//����
		BswDrv_OpenDoor(TYPE_SYNC);
	}
}

void App_NetProto_HandleHeartBat(FRAME_STR *pFrame)
{
	HEART_BEAT_ACK_STR *pHeartBeatAck = (void*)pFrame->data;
	CL_LOG("recv heartBeatAck,status=%d time=%d\r\n",pHeartBeatAck->status,pHeartBeatAck->time);
	if(pHeartBeatAck->status == 0 && pHeartBeatAck->time)
	{
		time_t now = GetRtcCount();
		int cnt = (now > pHeartBeatAck->time) ? now - pHeartBeatAck->time : pHeartBeatAck->time - now;
		if (30 < cnt) 
		{
			SetRtcCount(pHeartBeatAck->time);
			osDelay(100);
			CL_LOG("set rtc,now=%d,acktime=%d.\n",now,pHeartBeatAck->time);
		}
	}
}



void App_NetProto_HandleUpgrade(FRAME_STR *pFrame)
{
	int ret ;
	DOWN_FW_REQ_STR *upgrad = (void*)pFrame->data;
	upgrad->httpUrl[upgrad->httpLen] = '\0';
	
	CL_LOG("HandleUpgrade:ftpurl=%s uasrname=%s passwd=%s\r\n",upgrad->url,upgrad->usrName,upgrad->psw);
	CL_LOG("HandleUpgrade:httpLen=%d httpUrl=%s \r\n",upgrad->httpLen,upgrad->httpUrl);
	
	//�̼�����
	if(GlobalInfo.netType == NETTYPE_GPRS)
	{
		OTA_Start(UPGRADE_FTP);
		ret = BswSrv_Air720_FtpGet(upgrad->url,upgrad->usrName,upgrad->psw,upgrad->fileName);
	}
	else if(GlobalInfo.netType == NETTYPE_WIFI)
	{
		OTA_Start(UPGRADE_HTTP);
		ret = BswSrv_WIFI_HttpGet(upgrad->httpUrl);
	}

	uint8_t result = (ret==CL_FAIL?1:0);
	OTA_Finish(result,ret);
}



void App_NetProto_HandleRemoteCtrl(FRAME_STR *pFrame)
{
	uint8_t result = 0;
	REMO_CTRL_REQ_STR *remoteCtrl = (void*)pFrame->data;
	
	CL_LOG("remoteCtrl:code=%d\r\n",remoteCtrl->optCode);
	switch (remoteCtrl->optCode)
	{
		case SYSTEM_REBOOT:
			App_NetProto_SendRemoCtrlAck(pFrame,result);
			osDelay(500);
			BswSrv_SystemReboot();
			break;
		case CTRL_OPEN_DOOR:
			//����
			BswDrv_OpenDoor(TYPE_SYNC);
			break;
		default:
			result = 1;
			CL_LOG("no find remoteCtrl cmd.\r\n");
			break;
	}
	
	App_NetProto_SendRemoCtrlAck(pFrame,result);
}


void App_NetProto_HandleEventNotif(FRAME_STR *pFrame)
{
	EVENT_NOTICE_ACK_STR *eventAck =  (void*)pFrame->data;

	CL_LOG("Recv event ACK.result=%d \r\n",eventAck->status);

	if(eventAck->status == 0)
	{
		switch (eventAck->code)
		{
			case EVENT_SMOKE_WARING:
				
				break;
			case EVENT_DOOR_OPEN_WARING:

				break;
		}
	}
}


void App_NetProto_HandleDownDevList(FRAME_STR *pFrame)
{
	uint8_t result = 1;
	BIND_DEVICE_STR *pBindDev =  (void*)pFrame->data;
	CL_LOG("DownDevList ,device num = %d \r\n",pBindDev->subDeviceNum);
	
	if(pBindDev->subDeviceNum > 0){

		SystemInfo.RfDev.bandSize = ((pBindDev->subDeviceNum > RF_DEV_MAX) ? RF_DEV_MAX:pBindDev->subDeviceNum);

		for(uint16_t i = 0;i<SystemInfo.RfDev.bandSize;i++)
		{
			SystemInfo.RfDev.Unit[i].address = BswSrv_CharToRF433Addr(pBindDev->subDeviceAdd[i]);
			SystemInfo.RfDev.Unit[i].num = i+1;
			CL_LOG("bind device ,num=%d address=%X \r\n",SystemInfo.RfDev.Unit[i].num,SystemInfo.RfDev.Unit[i].address);
		}

		FlashWriteSysInfo((void*)&SystemInfo,sizeof(SYSTEM_INFO_T));
	
		result = 0;
	}

	App_NetProto_SendBindDeviceAck(result);
}	



void App_NetProto_HandleBindDeNotif(FRAME_STR *pFrame)
{
	//northing
	UNUSED(pFrame);
}


/**
 *	��������Э�鴦��
 */
int App_NetFrameHandle(uint8_t type,uint8_t *data,uint16_t len)
{
	FRAME_STR *pFrame = (void*)data;
	if(pFrame->head.type != DEVICE_TYPE)
	{
		CL_LOG("type=%d,error, pkt drop.\r\n",pFrame->head.type);
		return CL_FAIL;
	}

	
	if ((MQTT_CMD_REGISTER == pFrame->head.cmd) || (MQTT_CMD_AES_REQ == pFrame->head.cmd) || (MQTT_CMD_UPDATE_AES_NOTICE == pFrame->head.cmd)) {
        if (memcmp(pFrame->head.chargerSn, SystemInfo.stationId, sizeof(pFrame->head.chargerSn))) 
		{
            CL_LOG("sn diff error, pkt drop,cmd=%d.\n",pFrame->head.cmd);
            return CL_FAIL;
        }
    } else {
		if (memcmp(pFrame->head.chargerSn, SystemInfo.idCode, 8)) 
		{
			CL_LOG("idCode diff error, pkt drop,cmd=%d.\n",pFrame->head.cmd);
			return CL_FAIL;
		}
    }

	switch(pFrame->head.cmd){
		case MQTT_CMD_REGISTER:
			App_NetProto_HandleRegister(pFrame);
			break;
		case MQTT_CMD_START_UP:
			App_NetProto_HandleLogin(pFrame);
			break;
		case MQTT_CMD_CARD_ID_REQ:
			App_NetProto_HandleAuthCard(pFrame);
			break;
		case MQTT_CMD_HEART_BEAT:
			App_NetProto_HandleHeartBat(pFrame);
			break;
		case MQTT_CMD_DFU_DOWN_FW_INFO:
			App_NetProto_HandleUpgrade(pFrame);
			break;
		case MQTT_CMD_REMOTE_CTRL:
			App_NetProto_HandleRemoteCtrl(pFrame);
			break;
		case MQTT_CMD_EVENT_NOTICE:
			App_NetProto_HandleEventNotif(pFrame);
			break;
		case MQTT_CMD_DOWN_BAND_DEV:
			App_NetProto_HandleDownDevList(pFrame);
			break;
		case MQTT_CMD_REQ_BAND_DEV:
			App_NetProto_HandleDownDevList(pFrame);
			break;
		case MQTT_CMD_BAND_DEV_NOTIF:
			App_NetProto_HandleBindDeNotif(pFrame);
			break;
		case MQTT_CMD_AES_REQ:

			break;
		case MQTT_CMD_UPDATE_AES_NOTICE:

			break;
		default:
			CL_LOG("no find cmd..\r\n");
			break;	
	}
	return CL_OK;
}


