
#include "blue_proto.h"
#include "BswDrv_sc8042b.h"
#include "bluetooth.h"


BLUE_STATUS_STR gBlueStatus;


int SendCKB24Pkt(uint8_t nodeType, BLUE_PKT_STR *pkt, uint16_t len)
{
    return CL_OK;
}


//协议消息包认证
int CheckBluePkt(BLUE_PROTO_STR *pbuff, uint16_t len)
{
    return CL_FAIL;
}


//心跳响应
void BlueSendHeartBeatACK(BLUE_PROTO_STR *pMsg)
{
    
}


void ProcBtHeartBeat(void)
{
    
}


//心跳  设备登陆后 周期发送心跳数据
void BlueSendHeartBeat(void)
{
    
}


//固件升级通知
void BlueFWUpgradeNotice(BLUE_PROTO_STR *pMsg)
{
    
}


//固件下发
void BlueFWDownLoad(BLUE_PROTO_STR *pMsg)
{
    
}


//手机时间暂时不处理
void ShakeReqProc(BLUE_PROTO_STR *pMsg)
{
    
}


void BlueReqBreak(BLUE_PROTO_STR *pMsg)
{
 //   LcdDisplayBlutooth(LCD_CLEAR);
    gBlueStatus.status = 0;
    //OpenBluetoothRadio();//打开蓝牙广播
    Sc8042bSpeech(VOIC_BLUETOOTH_OFFLINE);
}

void BlueRecvProc(uint8_t *pbuff, uint16_t len)
{
	BLUE_PROTO_STR *pMsg = (void*)(pbuff);

	switch (pMsg->head.cmd) {
        case B_SHAKE_REQ:	//握手请求
            CL_LOG("shake req.\n");
            ShakeReqProc(pMsg);
            break;
        case B_START_CHARGING://开始充电请求
            CL_LOG("BlueProtoProc start charging.\n");
            break;
        case B_STOP_CHARGING://结束充电请求
            CL_LOG("BlueProtoProc stop charging.\n");
            break;
		case B_HEART_BEAT://心跳响应
			CL_LOG("BlueProtoProc heart beat ack.\n");
			BlueSendHeartBeatACK(pMsg);
			break;
        case B_REQ_BREAK://请求断开蓝牙链接
            CL_LOG("BlueProtoProc req break.\n");
            BlueReqBreak(pMsg);
            break;
		case B_DEV_REGISTER://设备注册
			CL_LOG("BlueProtoProc dev register ack.\n");
			break;
		case B_DEV_LOGIN://登陆响应
			CL_LOG("BlueProtoProc dev login ack.\n");
			break;
		case B_HISTORY_ORDER_UPLOAD://历史订单上传
			CL_LOG("BlueProtoProc upload history order ack.\n");
			break;
		case B_REQ_COST_TEMPLATE://请求计费模板
			CL_LOG("BlueProtoProc req cost template.\n");
			break;
		case B_COST_TEMPLATE_UPLOAD://计费模板上传
			CL_LOG("BlueProtoProc cost template upload ack.\n");
			break;
		case B_COST_TEMPLATE_DOWNLOAD://计费模板下发
			CL_LOG("BlueProtoProc cost template download.\n");
			break;
		case B_SET_DEV_SERIALNUM://设置充电桩编号
			CL_LOG("BlueProtoProc set chargersn.\n");
			break;
		case B_SET_DEV_CODER://设置充电桩识别码
			CL_LOG("BlueProtoProc set dev code.\n");
			break;
		case B_REMOTE_CTRL://远程控制
			CL_LOG("BlueProtoProc remote ctrl.\n");
            //RemoCtrlProc(pMsg);
			break;
		case B_FW_UPGRADE_NOTICE://固件升级请求
		    BlueFWUpgradeNotice(pMsg);
			break;
		case B_FW_DOWN_LOAD://固件下发
		    BlueFWDownLoad(pMsg);
			break;
		case B_HISTORY_ORDER_ENSURE://历史订单确认
			CL_LOG("BlueProtoProc order ensure.\n");
			break;
    }
}


