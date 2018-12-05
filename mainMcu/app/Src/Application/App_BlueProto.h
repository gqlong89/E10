#ifndef __APP_BLUEPROTO_H__
#define __APP_BLUEPROTO_H__

#include <stdint.h>


enum {
    B_SHAKE_REQ = 1,			//��������
    B_OPPO_SITE_AUTH = 2,		//�û���Ȩ
    B_START_CHARGING = 3,		//�������
    B_STOP_CHARGING = 4,		//�������
    B_HEART_BEAT = 5,			//����
    B_COST_TEMPLATE_UPLOAD = 6,	//�Ʒ�ģ���ϴ�
    B_REQ_COST_TEMPLATE = 7,	//����Ʒ�ģ��
    B_FW_UPGRADE_NOTICE = 8,	//�̼�������ʼ֪ͨ
    B_FW_DOWN_LOAD = 9,			//�̼��·�
    B_REQ_BREAK = 0x0A,			//����Ͽ���������
	B_DEV_REGISTER	= 0x0B,		//�豸ע��
	B_DEV_LOGIN	= 0x0C,			//�豸��½
	B_HISTORY_ORDER_UPLOAD=0x0D,//��ʷ�����ϴ�
	B_COST_TEMPLATE_DOWNLOAD=0x0E,//�Ʒ�ģ���·�
	B_SET_DEV_SERIALNUM = 0x0F,	//���ó��׮���
	B_SET_DEV_CODER	= 0x10,		//���ó��׮ʶ����
	B_SET_GW_ADDR = 0x11,		//����2.4G���ص�ַ
	B_SET_SERVER_IP	 = 0x12,	//���÷�����IP
	B_SET_SERVER_PORT = 0x13,	//���÷������˿�
	B_SET_TERMINAL_NUM = 0x14,	//�����ն˱����Ϣ
	B_REMOTE_CTRL = 0x15,		//Զ�̿���
	B_HISTORY_ORDER_ENSURE = 0x16,//��ʷ�����ϱ�ȷ��
	B_UPLOAD_DEVICE_STATE = 0x17,	//
	B_SET_WIFI_INFO		= 0x18,		//����WiFi��Ϣ
};


#pragma pack(1)

typedef struct {
    uint8_t  start;
    uint8_t  cmd;
    uint8_t  len;
}BLUE_PROTO_HEAD_STR;

typedef struct {
    BLUE_PROTO_HEAD_STR head;
    uint8_t  data[255];
}BLUE_PROTO_STR;


/***********�ֻ���������**********/
typedef struct {
    uint32_t time;
	char phonesSn[12];
}BLUE_SHAKE_REQ_STR;

typedef struct {
    char     name[4];                               //0. �豸�ͺ�
    uint8_t  chargerSn[5];                          //1. ׮���
    uint8_t  fwVer;                                 //2. �̼��汾��
    uint8_t  portCnt;                               //3. ������Ŀ
    uint8_t  startNo;                               //4. ������ʼ���
}BLUE_SHAKE_ACK_STR;



/**********ң�ż�����*********/ 
typedef struct {
    uint8_t  simSignal;                             //0. Sim���ź�
    uint8_t  temp;                                  //1. �����¶� �� -50��ƫ��  -50~200
    uint8_t  portCnt;                               //2. ���α����������ĳ��ӿ���Ŀ 0
}HEART_BEAT_STR;

typedef struct {
    uint32_t time;                                  //0. ϵͳʱ��
    uint8_t  result;                                //1. 0���ճɹ���1����ʧ��
}BLUE_HEART_BEAT_ACK_STR;



/**********�̼�����********/ 
typedef struct{
	uint32_t     fw_size;
	uint32_t     package_num;                       
	uint16_t     checkSum;
    uint8_t      fw_version;
}BLUE_DOWN_FW_REQ_STR;
typedef struct {
	uint8_t result;                             //0: �����ɹ�  1: ����ʧ�� 2: У��ʧ�� 3: д��ʧ��
}BLUE_DOWN_FW_ACK_STR;

typedef struct {
	uint8_t data[64];
}BLUE_FW_DOWNLOAD_REQ_STR;
typedef struct {
	uint8_t result;
	uint8_t index;
}BLUE_FW_DOWNLOAD_ACK_STR;


/**********����Ͽ�����********/ 
typedef struct {
	uint32_t     timestamp;	
}BLUE_DISCONNECT_DEV_REQ_STR;
typedef struct {
	uint8_t     status;	
}BLUE_DISCONNECT_DEV_ACK_STR;


/**********���ó��׮���*********/
typedef struct {
	uint8_t  chargerSn[8];
}BLUE_SET_CHARGERSN_REQ_STR;
typedef struct {
	uint8_t  result;
}BLUE_SET_CHARGERSN_ACK_STR;

/**********���ó��׮ʶ����*********/
typedef struct {
	uint8_t idcode[8];
}BLUE_SET_CHARGERIDCODE_REQ_STR;
typedef struct {
	uint8_t  result;
}BLUE_SET_CHARGERIDCODE_ACK_STR;


/**********Զ�̿���*********/
typedef struct {
	uint8_t  cmd;
	uint32_t pararm;
}BLUE_REMOTE_CTRL_REQ_STR;
typedef struct {
	uint8_t  cmd;
	uint32_t pararm;
	uint8_t result;
}BLUE_REMOTE_CTRL_ACK_STR;



typedef struct{
	char ssid[32];
	char passwd[32];
}BLUE_SET_WIFI_INFO_STR;

typedef struct{
	uint8_t result;
}BLUE_SET_WIFI_INFO_ACK;

#pragma pack()


void App_Blue_SendHeartBat(void);

#endif


