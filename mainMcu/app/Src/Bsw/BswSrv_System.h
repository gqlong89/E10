#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "includes.h"
#include "BswDrv_CAN.h"

#define ICCID_LEN                       20
#define CHARGER_SN_LEN			        8

enum{
	NETTYPE_GPRS = 1,
	NETTYPE_485,
	NETTYPE_24G,
	NETTYPE_WIFI,
};

typedef struct {
    uint32_t size;
    uint16_t checkSum;
    uint8_t  ver;
    uint8_t  sum;
}TERMINAL_FW_INFO_STR;

//sizeof(SYSTEM_INFO_T) = 256
typedef struct
{
	uint32_t magic_number;						//
    uint8_t stationId[8];                     	// �豸��bcd, SN=10, 10 byte  0001123456789
	uint8_t idCode[8];							// ע����
	char WifiName[32];							// ssid
	char WifiPasswd[32];						// passwd
	uint8_t reserved0[16];
    uint8_t blue_connectstate;					//��������״̬
    volatile uint8_t is_socket_0_ok;
    TERMINAL_FW_INFO_STR localFwInfo;           //���ع̼���Ϣ
    uint8_t  tcp_tx_error_times;
	uint8_t  iccid[22];                         // ASCII
	uint8_t  netType;                           //����������� 1:����2g 2:485��Զ 3����2.4G: 4:���Ա��� 5:������Զ
	uint32_t mqtt_sn;
	uint8_t  isRecvStartUpAck;                  //0:δ��¼��̨ 1:��¼��̨
}SYSTEM_INFO_T;


typedef struct
{
	uint8_t isRegister;						//0-δע��  1-��ע��
	uint8_t isLogin;						//0-δ��¼	1-�ѵ�½
	// uint8_t current_usr_card_id[16];     //ˢ����ȡ�Ŀ��� �Ƿ��б�Ҫ���棿
	uint8_t card_state;                     //������״̬   0: ��ʾ��ʼ������  1 :��ʾ��ʼ��ok 
	uint8_t GprsInitOK;						//Gprs�Ƿ��ʼ���ɹ� 0��δ��ʼ�� 1-��ʼ��OK
	uint8_t WG215InitOK;					//�Ƿ��⵽WG215ģ�� 0-δ��⵽ 1-���OK
	// uint8_t WifiInitOK;						//WiFi�Ƿ��ʼ�� 0��WiFiģ���ʼ������ 1����ʼ��ok
	uint8_t BlueInitOk;						//�����Ƿ��ʼ�� 0��δ��ʼ��	1���Ѿ���ʼ��
	uint8_t CBInitOK;						//ˢ�����Ƿ��ʼ���ɹ� 0δ��ʼ�� 1-��ʼ���ɹ�
	uint8_t netType;						//������ʽ	1-����2G/4G 2-485  3-2.4G 4-wifi
	uint8_t modeType;						//ģ���ͺ�
	uint8_t gSimStatus;						//sim��״̬
	uint8_t simSignal;						//sim���ź�ֵ
	uint8_t iccid[ICCID_LEN+1];				//sim����iccid
	uint8_t doorState;						//�Ž�״̬ 0--�ر�  1--��
	uint8_t is_socket_0_ok;					//socket�Ƿ������� 0-δ���� 1-������
	char BlueName[16];						//��������
	uint8_t CBVerson;						//ˢ����汾
	uint8_t isRecvServerData;				//�Ƿ�ʼ���շ���������
	uint32_t lastRecvCBTime;				//�ϴν��յ�ˢ���������ʱ��	
	uint8_t isBlueConnect;					//�Ƿ�����������
	uint8_t isBlueLogin;					//�����Ƿ��½
	char phonesSn[20];						//���������ֻ�����
	uint32_t blueLastOnlineTime;			//��������ʱ��
	uint8_t upgradeFlag;					//1-ϵͳ��������  0-Ϊ����
	void (*readCard_Callback)(uint8_t,uint8_t *);
	void (*AppCan_HandleCallBack)(uint8_t *data, uint16_t len, uint8_t cmd, uint8_t SourceAddr);
	QueueHandle_t CanMessageQueue;			//��Ϣ���о��
	CAN_DRV_MESSAGE_QUEUE_STR CanSendMessage;
	CAN_DRV_MESSAGE_QUEUE_STR CanReceiveMessage;
	CAN_READ_WRITE_INDEX_STR pReadWriteIndex;
	uint8_t SerialNum;
	uint8_t DestAddr;	//Can ͨ�ŵ�Ŀ�ĵ�ַ, ����ʱGPIO��ȡ
	struct
	{
		uint32_t PF_Bit0ToBit7:7;
	} BitFieldFlag;
}GLOBAL_INFO_T;



#pragma pack(1)
typedef struct{
	uint16_t updateFlag;
	uint16_t checkSum;
	uint32_t fsize;
}SYS_UPDATE_INFO_T;
#pragma pack()



void BswSrv_LoadSystemInfo(void);
void BswSrv_SystemResetRecord(void);
int BswSrv_GetHWId(uint8_t id[]);
int BswSrv_GetCpuTemp(void);
void BswSrv_SystemReboot(void);
void BswSrv_RF433AddrToChar(uint32_t addr,uint8_t ch[]);
uint32_t BswSrv_CharToRF433Addr(uint8_t ch[]);

uint16_t BswSrv_Tool_CheckSum(uint8_t *data,uint16_t len);
int BswSrv_Tool_isArraryEmpty(uint8_t *array,int len);
int BswSrv_Tool_StringToBCD(unsigned char *BCD, const char *str) ;
char *BswSrv_Tool_BCDToString(char *dest, unsigned char *BCD, int bytes) ;
extern int BswSrv_Tool_StringToBCD(unsigned char *BCD, const char *str);
extern char *BswSrv_Tool_BCDToString(char *dest, unsigned char *BCD, int bytes);

extern SYSTEM_INFO_T	SystemInfo;
extern GLOBAL_INFO_T	GlobalInfo;
extern SYS_UPDATE_INFO_T updateInfo;


#endif


