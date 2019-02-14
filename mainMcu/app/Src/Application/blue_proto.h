#ifndef __BLUE_PROTO_H__
#define __BLUE_PROTO_H__

#include "includes.h"
#include "BswSrv_System.h"


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
};


//b0:�Ƿ�������������ʧ�� b1:�Ƿ��͹����ͱ���ʧ�� b2:�Ƿ�������� b3:�Ƿ����2.4G b4:�Ƿ������������ b5:�Ƿ������Զ����
enum{
    HB_RECV_FAIL = 0,                           //0.
    HB_SEND_FAIL,                               //1.
    BLUE_EXIT,                                  //2.
    BLUE_24G_EXIT,                              //3.
    BLUE_UPGRADE,                               //4.
    OUT_NET_UPGRADE,                            //5.
};


typedef struct {
    uint8_t  status;                                //0:����δ����  1:����
    uint32_t lastRecvHeartBeat;                     //���һ�ν�������ʱ��
	//FIFO_S_t rxBtBuff;
}BLUE_STATUS_STR;


#pragma pack(1)

typedef struct {
	uint8_t  ab;                                //0.
    uint8_t  cd;                                //1.
    uint8_t  target;                            //2.
    uint16_t len;
}BLUE_RX_HEAD_STR;


typedef struct {
    uint8_t  ab;
    uint8_t  cd;
    uint8_t  type;
    uint16_t len;
}BLUE_BT_HEAD_STR;


typedef struct {
    uint8_t  ab;                                //0.
    uint8_t  cd;                                //1.
    uint8_t  target;                            //2.
    uint8_t  addr[6];                           //3.
    uint16_t len;                               //4.
}BLUE_HEAD_STR;

typedef struct {
    BLUE_HEAD_STR head;
    uint8_t  data[255];
}BLUE_PKT_STR;


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

/***********�û������Ȩ*********/
typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  optType;                               //1. 0��ˢ����Ȩ����ʼ��磻1������ѯ��2���ֻ��û������Ȩ
    uint8_t  usrId[11];                             //2. �û��˺ţ����߿��ţ�
    uint8_t  mode;                                  //3. 0�����ܳ�����1��������������� ��������2����ʱ����3��������
    uint16_t para;                                  //4. ���ܳ�����Ϊ0�������ֱ���1�֣���ʱ�����ֱ���1���ӣ����������ֱ���0.01kwh
}BLUE_OPPO_SITE_AUTH_STR;

typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  result;                                //1. 0:��Ȩ�ɹ���1:�������2:����/�������ꣻ3:����ʹ�ã�4:�˻���Ч��5:����ԭ��
    uint8_t  cardType;                              //2. �¿�,  �����ֻ��û���Ȩ��Ч
    uint8_t  rsv;
    uint32_t money;                                 //3. �˻����ֱ���1��
    uint8_t  order[ORDER_SECTION_LEN];              //4. bcd ������
}BLUE_OPPO_SITE_AUTH_ACK_STR;


/***********����������*********/
typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  order[ORDER_SECTION_LEN];              //1. bcd ������
    uint8_t  mode;                                  //2. 0�����ܳ�����1��������������� ��������2����ʱ����3��������
    uint16_t para;                                  //3. ���ܳ�����Ϊ0�������ֱ���1�֣���ʱ�����ֱ���1���ӣ����������ֱ���0.01kwh
	uint8_t orderSource;							//4. ������Դ
	uint8_t subsidyType;							//5. ��������
	uint16_t subsidyPararm;							//6. ��������
}BLUE_START_CHARGING_STR;

typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  result;                                //1. 0:�����ɹ���1: ����ʧ��
    uint8_t  reason;                                //2. 1���˿ڹ��ϣ�2��û�мƷ�ģ�棻3���Ѿ��ڳ���У�4���豸û��У׼��5����������
	uint32_t startTime;								//3. ����ʱ��
	uint16_t startElec;								//4. ��������
}BLUE_START_CHARGING_ACK_STR;


/***********����������*********/
typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  order[ORDER_SECTION_LEN];              //1. bcd ������
}BLUE_STOP_CHARGING_STR;

typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  result;                                //1. 0:�ɹ���1: ʧ��
	uint32_t stopTime;								//2. ����ʱ��
	uint16_t stopElec;								//3. ��������
	uint8_t stopReason;								//4. ����ԭ��
}BLUE_STOP_CHARGING_ACK_STR;


/**********ң�ż�����*********/
typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  status;                                //1. ����״̬��0 ���У�1 ռ�ã�2 ���ϣ�3 ����
    uint8_t  errCode;                               //2. 1~255  ������״̬Ϊ2����ʱ��Ч��1��״̬�쳣��2������оƬͨ�Ź���
}BLUE_GUN_HEART_BEAT_STR;

typedef struct {
    uint8_t  simSignal;                             //0. Sim���ź�
    uint8_t  temp;                                  //1. �����¶� �� -50��ƫ��  -50~200
    uint8_t  portCnt;                               //2. ���α����������ĳ��ӿ���Ŀ
    BLUE_GUN_HEART_BEAT_STR gunStatus[GUN_NUM_MAX];
}HEART_BEAT_STR;

typedef struct {
    uint32_t time;                                  //0. ϵͳʱ��
    uint8_t  result;                                //1. 0���ճɹ���1����ʧ��
}BLUE_HEART_BEAT_ACK_STR;


/**********����Ʒ�ģ��********/
typedef struct{
	uint8_t gun_id;
	uint32_t template_id;
}BLUE_COST_TEMPLATE_REQ;

/**********�Ʒ�ģ���·�ACK*********/

typedef struct {
    uint8_t  gunId;
	uint32_t template_id;
    uint8_t  mode;                              //1�������ʶμƷ� 2��ͳһ�շ�
	uint8_t Data[256];
}BLUE_COST_TEMPLATE_HEAD_STR;

typedef struct {
	uint8_t  result;
}BLUE_COST_TEMPLATE_ACK_STR;


/*********�ϱ���ʷ����********/
typedef struct {
	uint8_t gun_id;
	uint8_t ordersource;				//������Դ
	uint8_t stopReason;					//����ԭ��
	uint8_t stopDetails;				//��������
	uint8_t chargerMode;				//���ģʽ
	uint16_t chargerPararm;				//������
	uint8_t  subsidyType;				//��������
	uint16_t subsidyPararm;				//��������
	uint8_t fw_version;					//�̼��汾
	uint8_t phoneSn[16];
	uint8_t order[ORDER_SECTION_LEN];
	uint32_t startTime;
	uint32_t stopTime;
	uint32_t startElec;
	uint32_t stopElec;
	uint32_t cost_price;			//��������
	uint32_t template_id;			//�Ʒ�ģ��id
	uint16_t power;
	uint8_t sampleTimes;			//��������
	uint8_t sampleCycle;			//��������
	uint16_t samplePower[90];		//��������
}BLUE_UPLOAD_HISTORY_ORDER_REQ_STR;
typedef struct{
	uint8_t gun_id;
	uint8_t result;
}BLUE_UPLOAD_HISTORY_ORDER_ACK_STR;


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

/**********�豸ע��********/
typedef struct {
    uint8_t  device_type[8];                   //1.�豸����
    uint8_t  register_code[16];                 //2.ע����
}BLUE_REGISTER_REQ_STR;
typedef struct {
    uint8_t  result;
    uint8_t  idcode[8];                         //����ƽ̨������豸ʶ���� bcd
}BLUE_REGISTER_ACK_STR;


/**********�豸��¼*******/
typedef struct {
	uint8_t  device_type[8];                   //1.
    uint8_t  chargerSn[CHARGER_SN_LEN];         //2.׮��� bcd
	uint8_t  fw_version;                        //3.
	uint8_t  fw_1_ver;                          //4.
	uint8_t  login_reason;						//8. 1:�ϵ�����			2:���߻ָ��ط�
	uint8_t  gun_number;                        //9. ���׮���ó������������128���˿�
	uint8_t  statistics_info[8];				//11. ͳ����Ϣ
}BLUE_START_UP_REQ_STR;
typedef struct {
    uint8_t  result;
	uint32_t time_utc;
}BLUE_START_UP_ACK_STR;


/**********���ó��׮���*********/
typedef struct {
	uint8_t  chargerSn[CHARGER_SN_LEN];
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

/**********����2.4G���ص�ַ*********/
typedef struct {
	uint8_t gw_addr[5];
}BLUE_SET_GW_ADDR_REQ_STR;
typedef struct {
	uint8_t  result;
}BLUE_SET_GW_ADDR_ACK_STR;

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


/**********��ʷ�����ϱ�ȷ��*********/
typedef struct{
	uint8_t 	gun_id;
	uint32_t 	startTime;
	uint32_t 	stopTime;
}BLUE_Record_ENSURE_REQ_STR;
typedef struct{
	uint8_t 	gun_id;
	uint32_t 	startTime;
	uint8_t 	result;
}BLUE_Record_ENSURE_ACK_STR;


#pragma pack()


typedef struct {
    uint32_t turnOnLcdTime;                     //���һ�δ�lcd��ʱ��
    uint32_t lastRecvKbMsgTime;                 //�ϴν��հ�������Ϣʱ��
    uint32_t lastOpenTime;                      //���ü�ʱ���� ǹͷ���ؼ�ʱ����Զ������ʱ
	uint16_t chargingTotalPower;                //����ܹ��� w
    uint8_t  netStatus;                         //�������״̬: b0:�Ƿ�������������ʧ�� b1:�Ƿ��͹����ͱ���ʧ�� b2:�Ƿ�������� b3:����״̬(1����/0�쳣) b4:�Ƿ������������ b5:�Ƿ������Զ���� b6:��ԿЭ�̳ɹ� b7:�յ���Կ����֪ͨ
    uint8_t  inputCode;                         //�������붯��
    uint32_t lastInputTime;
    uint32_t blueCheck;
	uint32_t size;                              //���ñ���:�̼���С������������ʱ
	uint8_t sendBlueStatus;                     //��������״̬��־ 0δ����1����״̬��0xff �յ�Ӧ��,���Ͷ��û��Ӧ��ֹͣ����
	uint8_t  errCode;                           //�����������
}CHG_INFO_STR;


extern void BlueRecvProc(uint8_t *pbuff, uint16_t len);
void BlueRecvProc(uint8_t *pbuff, uint16_t len);

extern void BlueRegister(void);
extern void BlueLogin(uint8_t method);
extern void BlueSendHeartBeat(void);
extern void BlueCostTemplateReq(uint8_t gunid);
extern void BlueCostTemplateUpload(uint8_t gunid);
extern void BlueUpLoadHistoryOrder(BLUE_UPLOAD_HISTORY_ORDER_REQ_STR *order);
extern int SendCKB24Pkt(uint8_t nodeType, BLUE_PKT_STR *pkt, uint16_t len);
extern void SendTest(void);
extern void ProcBtHeartBeat(void);


extern BLUE_STATUS_STR gBlueStatus;
extern CHG_INFO_STR gChgInfo;

#endif

