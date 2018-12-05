#ifndef __APP_NETPROTO_H__
#define __APP_NETPROTO_H__


#include "includes.h"

#define OUT_NET_PKT_LEN					256
#define CHARGER_SN_LEN			        8   //bcd�ĳ���
#define ORDER_SECTION_LEN				10

enum {
	MESSAGE_VER_NOENCRYPT = 1,
	MESSAGE_VER_ENCRYPT = 2,
};


enum {
    CARD_AUTH = 0,
    CARD_CHECK_MONEY,
	CARD_MONTH,
    CARD_IN_DOOR,
    CARD_OUT_DOOR,
};

enum {
    EVENT_PROMPT = 1,
    EVENT_ALARM,
    EVENT_FAULT,
};


enum {
    MQTT_CMD_REGISTER 		= 0,				//�豸ע��
	MQTT_CMD_START_UP 		= 1,                //��½
	MQTT_CMD_CARD_ID_REQ 	= 2,               	//����Ȩ
	MQTT_CMD_HEART_BEAT 	= 7,                //����
	MQTT_CMD_DFU_DOWN_FW_INFO = 0x0A,       	//�̼�����
	MQTT_CMD_REMOTE_CTRL 	= 0x0B,            	//Զ�̿���
    MQTT_CMD_EVENT_NOTICE 	= 0x0D,           	//�¼�֪ͨ
	MQTT_CMD_DOWN_BAND_DEV	= 0x10,				//�·��豸��
	MQTT_CMD_REQ_BAND_DEV	= 0x11,				//�����豸��
	MQTT_CMD_BAND_DEV_NOTIF	= 0x16,				//�豸�����֪ͨ
	MQTT_CMD_AES_REQ		=0x12,				//�豸������Կ
	MQTT_CMD_UPDATE_AES_NOTICE=0x13,			//��̨������Կ֪ͨ
};


enum {
	EVENT_START             = 1,                    //1.
	EVENT_SHUT_DOWN         = 2,                    //2.
	EVENT_POWER_DROP        = 3,                    //3.
	EVENT_NET_BREAK         = 4,                    //4.
	EVENT_NET_RECONNECT     = 5,                    //5.
	EVENT_CARD_FAULT        = 0x0A,                 //0x0A
    EVENT_ENV_TEMP_HIGH     = 0x0E,                 //0x0E
    EVENT_CHIP_TEMP_HIGH    = 0x0F,                 //0x0F
    EVENT_CHIP_FAULT        = 0x12,                 //0x12 //�������� 
    EVENT_SMOKE_WARING      = 0x15,                 //�̸б���
    EVENT_DOOR_OPEN_WARING  = 0x16,                 //����ǿ��
};

/**
 *	Զ�̿���
 * 1��ϵͳ�������� 2������ǹͷ 3���ر�ǹͷ 4������ά��״̬���رճ����� 5������������ 6���趨����������
 * 7: �趨�������ʷ�ֵ	8:�趨����ʱ�䷧ֵ	9: �趨��ǹʱ�䷧ֵ		10: ��ǹ�Ƿ�ֹͣ��綩��	11: ����ˢ���幤��ģʽ
 */
enum {
    SYSTEM_REBOOT = 1,
    CTRL_OPEN_GUN,                                  //0x02
    CTRL_CLOSE_GUN,                                 //0x03
    CTRL_SET_SERVER_STATUS,                         //0x04
    CTRL_SET_OUT_POWER,                             //0x05
    CTRL_SET_FULL_POWER,                            //0x06
    CTRL_SET_FULL_TIME,                             //0x07
    CTRL_SET_PULL_GUN_TIME,                         //0x08
    CTRL_SET_PULL_GUN_STOP,                         //0x09
    CTRL_SET_CARD_WORK,                             //0x0a
    CTRL_CLEAR_DATA,                                //0x0b
    CTRL_SET_PRINT_SWITCH,                          //0x0c
	CTRL_SET_DISTURBING_TIME,						//0x0D
	CTRL_SET_CHARGING_FULL_STOP = 0x11,             //0x11
	CTRL_OPEN_DOOR				= 0x12,
};

//*********************************************************/

#pragma pack(1)

/**
 *
 */
typedef struct {
    uint8_t  aa;                                //0.
    uint8_t  five;                              //1.
    uint8_t  type;                              //2��x9 2��x10
    uint8_t  chargerSn[CHARGER_SN_LEN];         //3.
    uint16_t len;                               //4. ������ �汾�� �� У����� �������ֽ���
    uint8_t  ver;                               //5. b0:Ϊ1���ǲ�֧�ּ���;0֧�ּ��ܣ�b1:�������Ƿ���ܣ�b2~3:�����㷨����0:AES ECB
    uint16_t sn;                                //6.
    uint8_t  cmd;                               //7.
}PKT_HEAD_STR;

typedef struct {
    PKT_HEAD_STR head;
    uint8_t  data[OUT_NET_PKT_LEN];
}FRAME_STR;


/**
 *
 */
typedef struct {
    uint8_t  device_type[20];                   //1.�豸����
    uint8_t  register_code[16];                 //2.ע����
	uint8_t	 HWId[20];							//Ӳ��id
}REGISTER_REQ_STR;

typedef struct {
    uint8_t  result;
    uint8_t  idcode[8];                         //����ƽ̨������豸ʶ���� bcd
}REGISTER_ACK_STR;


/**
 *
 */
typedef struct {
	uint8_t  device_type[20];                   //1.
    uint8_t  chargerSn[CHARGER_SN_LEN];         //2.׮��� bcd
	uint8_t  fw_version;                        //3. x10�汾��
	uint8_t  fw_1_ver;                          //4. X10������汾��
    uint8_t  sim_iccid[20];                     //5.
    uint8_t  onNetWay;                          //6.������ʽ  1������������2��485·��������3��2.4G·������ 4:wifi����
    uint8_t  modeType;                          //7. 1��2G��2��4G��3��nbIot-I��4��nbIot-II
	uint8_t  login_reason;						//8. 1:�ϵ�����			2:���߻ָ��ط�
	uint8_t  gun_number;                        //9. ���׮���ó������������128���˿�
	uint8_t  device_status;                     //10. ָʾ�豸����ʱ�������ֹ��Ĵ���״̬
	uint8_t  statistics_info[8];				//11. ͳ����Ϣ [0~2]:���Ŵ��� [3]����ʱ��,���� [4]��ǹ/������ͣ���� [5-6]����ʱ�� [7]������̼��汾
    uint8_t  downloadType;                      //12 0-ftp 1-http
}START_UP_REQ_STR;

typedef struct {
    uint8_t  result;							//1.��¼���
	uint32_t time_utc;							//2.������ʱ��
}START_UP_ACK_STR;



/**
 *
 */
typedef struct {
	uint8_t  gun_id;                            //0.
	uint8_t  card_type;                         //1.
    uint8_t  optType;                           //2�� 0��ˢ����Ȩ����ʼ��磻1������ѯ��2���ֻ��û������Ȩ 3��ˢ�������� 4��ˢ��������
	uint8_t  card_id[16];                       //3.
	uint8_t  card_psw[8];                       //4.
    uint8_t  mode;                              //5.���ģʽ  0�����ܳ��� 1������� 2����ʱ�� 3��������
	uint32_t chargingPara;                      //6.������  ���ܳ�����Ϊ0  �����ֱ���1��  ��ʱ�����ֱ���1����  ���������ֱ���0.01kwh
}CARD_AUTH_REQ_STR;

typedef struct {
    uint8_t  gun_id;                            //0.
	uint8_t  result;                            //1.��Ȩ���
	uint8_t  cardType;                          //2. 1�¿�
	uint8_t  rsv;								//3
	uint32_t user_momey;                        //4.�˻���� ��
	uint8_t  order[ORDER_SECTION_LEN];          //5.������
}CARD_AUTH_ACK_STR;



/**
 *
 */
typedef struct {
    uint8_t  netSigle;                          //1. Sim���ź�
	uint8_t  envTemp;                           //2. �����¶� �� -50��ƫ��  -50~200
	uint8_t  KberrCnt;
	uint8_t  doorState;
	uint8_t	 rev[2];
    uint8_t  gunCnt;                            // U8��ĿΪ0
}HEART_BEAT_REQ_STR;

typedef struct {
    uint32_t time;
    uint8_t  status;                            //0��	���ճɹ�   1��	����ʧ��
}HEART_BEAT_ACK_STR;


/**
 * �̼�����
 */
typedef struct{
	char     url[48];                           //������������ַ�����㲹0
	char     usrName[4];                        //��¼�û���
	char     psw[4];                            //����
    char     fileName[8];                       //�ļ���
	uint32_t checkSum;                          //�ļ��ֽ��ۼӺ�
    uint32_t httpLen;               
    char     httpUrl[128];
}DOWN_FW_REQ_STR;

typedef struct {
	uint8_t result;                             //0: �����ɹ�  1: ����ʧ�� 2: У��ʧ�� 3: д��ʧ��
}DOWN_FW_ACK_STR;


/**
 * Զ�̿���
 */
typedef struct {
    uint8_t  optCode;                           //���ƴ���
    uint32_t para;                              //���Ʋ���
}REMO_CTRL_REQ_STR;

typedef struct {
    uint8_t  optCode;
    uint8_t  result;
}REMO_CTRL_ACK_STR;



/**
 * �¼�֪ͨ
 */
typedef struct {
    uint8_t  gun_id;                            //0. �����0��ʾ��׮,1~128,�����ӿ�
    uint8_t  code;                              //1. �¼�����
    uint8_t  para1;                             //2.
    uint32_t para2;                             //3.
    uint8_t  status;                            //4.1������  2���ָ�
    uint8_t  level;                             //5.�¼��ȼ� 1��ʾ  2�澯  3���ع���
    char     discrip[32];          				//6.�¼���ϸ����
}EVENT_NOTICE_STR;

typedef struct {
    uint8_t  result;                            //0�����ճɹ���1������ʧ��
    uint8_t  gun_id;                            //0. �����0��ʾ��׮,1~128,�����ӿ�
    uint8_t  code;                              //1. �¼�����
    uint8_t  para1;                             //2.
    uint32_t para2;                             //3.
    uint8_t  status;                            //4.1������  2���ָ�
    uint8_t  level;                             //5.�¼��ȼ� 1��ʾ  2�澯  3���ع���
}EVENT_NOTICE_ACK_STR;



/**
 * �豸��
 */
typedef struct{
	uint8_t subDeviceNum;
	uint8_t subDeviceAdd[RF_DEV_MAX][5];
}BIND_DEVICE_STR;

typedef struct{
	uint8_t result;
}BIND_DEVICE_ACK_STR;

typedef struct{
	uint8_t rev;
}BIND_DEVICE_REQ_STR;


/**
 * �豸�����֪ͨ
 */

typedef struct{
	uint8_t result;
}BIND_DEVICE_NOTIF_ACK_STR;


#pragma pack()


int App_NetProto_SendRegister(void);
int App_NetProto_SendStartUpNotice(int flag);
int App_NetProto_SendCardAuthReq(uint8_t cardId[],int flag);
int App_NetProto_SendEventNotice(uint8_t gunId, uint8_t event, uint8_t para1, uint32_t para2, uint8_t status, char *pDisc);
int App_NetProto_SendHeartBeat(void);
void App_NetProto_SendRemoCtrlAck(FRAME_STR *pRemoCtrlReq, uint8_t result);
int App_NetProto_SendReqBindDev(void);
void App_NetProto_SendBindDeviceAck(uint8_t result);
int App_NetProto_SendBindDevFinishedNotif(void);

#endif

