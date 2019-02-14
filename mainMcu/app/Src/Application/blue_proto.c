
#include "blue_proto.h"
#include "BswDrv_sc8042b.h"
#include "bluetooth.h"


BLUE_STATUS_STR gBlueStatus;


int SendCKB24Pkt(uint8_t nodeType, BLUE_PKT_STR *pkt, uint16_t len)
{
    return CL_OK;
}


//Э����Ϣ����֤
int CheckBluePkt(BLUE_PROTO_STR *pbuff, uint16_t len)
{
    return CL_FAIL;
}


//������Ӧ
void BlueSendHeartBeatACK(BLUE_PROTO_STR *pMsg)
{
    
}


void ProcBtHeartBeat(void)
{
    
}


//����  �豸��½�� ���ڷ�����������
void BlueSendHeartBeat(void)
{
    
}


//�̼�����֪ͨ
void BlueFWUpgradeNotice(BLUE_PROTO_STR *pMsg)
{
    
}


//�̼��·�
void BlueFWDownLoad(BLUE_PROTO_STR *pMsg)
{
    
}


//�ֻ�ʱ����ʱ������
void ShakeReqProc(BLUE_PROTO_STR *pMsg)
{
    
}


void BlueReqBreak(BLUE_PROTO_STR *pMsg)
{
 //   LcdDisplayBlutooth(LCD_CLEAR);
    gBlueStatus.status = 0;
    //OpenBluetoothRadio();//�������㲥
    Sc8042bSpeech(VOIC_BLUETOOTH_OFFLINE);
}

void BlueRecvProc(uint8_t *pbuff, uint16_t len)
{
	BLUE_PROTO_STR *pMsg = (void*)(pbuff);

	switch (pMsg->head.cmd) {
        case B_SHAKE_REQ:	//��������
            CL_LOG("shake req.\n");
            ShakeReqProc(pMsg);
            break;
        case B_START_CHARGING://��ʼ�������
            CL_LOG("BlueProtoProc start charging.\n");
            break;
        case B_STOP_CHARGING://�����������
            CL_LOG("BlueProtoProc stop charging.\n");
            break;
		case B_HEART_BEAT://������Ӧ
			CL_LOG("BlueProtoProc heart beat ack.\n");
			BlueSendHeartBeatACK(pMsg);
			break;
        case B_REQ_BREAK://����Ͽ���������
            CL_LOG("BlueProtoProc req break.\n");
            BlueReqBreak(pMsg);
            break;
		case B_DEV_REGISTER://�豸ע��
			CL_LOG("BlueProtoProc dev register ack.\n");
			break;
		case B_DEV_LOGIN://��½��Ӧ
			CL_LOG("BlueProtoProc dev login ack.\n");
			break;
		case B_HISTORY_ORDER_UPLOAD://��ʷ�����ϴ�
			CL_LOG("BlueProtoProc upload history order ack.\n");
			break;
		case B_REQ_COST_TEMPLATE://����Ʒ�ģ��
			CL_LOG("BlueProtoProc req cost template.\n");
			break;
		case B_COST_TEMPLATE_UPLOAD://�Ʒ�ģ���ϴ�
			CL_LOG("BlueProtoProc cost template upload ack.\n");
			break;
		case B_COST_TEMPLATE_DOWNLOAD://�Ʒ�ģ���·�
			CL_LOG("BlueProtoProc cost template download.\n");
			break;
		case B_SET_DEV_SERIALNUM://���ó��׮���
			CL_LOG("BlueProtoProc set chargersn.\n");
			break;
		case B_SET_DEV_CODER://���ó��׮ʶ����
			CL_LOG("BlueProtoProc set dev code.\n");
			break;
		case B_REMOTE_CTRL://Զ�̿���
			CL_LOG("BlueProtoProc remote ctrl.\n");
            //RemoCtrlProc(pMsg);
			break;
		case B_FW_UPGRADE_NOTICE://�̼���������
		    BlueFWUpgradeNotice(pMsg);
			break;
		case B_FW_DOWN_LOAD://�̼��·�
		    BlueFWDownLoad(pMsg);
			break;
		case B_HISTORY_ORDER_ENSURE://��ʷ����ȷ��
			CL_LOG("BlueProtoProc order ensure.\n");
			break;
    }
}


