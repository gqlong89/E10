#ifndef __APP_CAN_H__
#define __APP_CAN_H__


#include "includes.h"
#include "CAN_Config.h"
#include "BswSrv_CAN.h"


#define CAN_AA_55_SRC_DEST_LEN             			(4)
#define CAN_R_N_LEN             					(2)
#define CAN_SUM_LEN             					(1)
#define CAN_SN_CMD_LEN             					(2)

enum {
    CAN_CMD_HAND 				= 1,				//��������
	CAN_CMD_REMOTE_CONTROL 		= 2,                //Զ�̿���
	CAN_CMD_EVENT_NOTICE 		= 3,                //�¼�֪ͨ
	CAN_CMD_REQ_UPGRADE			= 4,                //����̼�����
	CAN_CMD_DOWN_FW_INFO		= 5,                //�̼��·�
};


#pragma pack(1)

typedef struct {
    uint8_t  five;                                  //0.
    uint8_t  aa;                                  	//1.
    uint8_t  src_SA;                            	//2. Դ��ַ
    uint8_t  dest_PS;                           	//3. Ŀ�ĵ�ַ
	uint16_t len;								    //2. ���� ��sn��sum����sum
    uint8_t  sn;                                    //4. �����Ų����ʾ�����ط�
    uint8_t  cmd;                                   //6. �������
} APP_CAN_HEAD_STR;

typedef struct {
    APP_CAN_HEAD_STR head;
    uint8_t  data[256 - sizeof(APP_CAN_HEAD_STR)];
} APP_CAN_STR_t;


typedef struct{
    uint8_t gun;						//���ǹ�� ��1��ʼ
    uint32_t systemTime;				// ϵͳʱ��
    uint8_t SourceEdgeVeb;				//����flash�л����Դ�߹̼��汾��
    uint8_t SubEdgeVeb;					//����flash�л���ĸ��߹̼��汾��
    uint8_t Preserve;					//����
}APP_CAN_SHAKE_HAND_STR;				//��������(����?Դ��)

typedef struct{
    uint8_t gun;						//���ǹ�� ��1��ʼ
    uint8_t SourceEdgeVeb;				//Դ���������еĹ̼��汾��
    uint8_t SubEdgeVeb;					//����flash�л���ĸ��߹̼��汾��
    uint8_t SourceEdgeStatus;			//1:���У�2:ռ�ã�3:����У�4:���ϣ�5:�̼�������
    uint8_t SourceEdgeErrorCode;		//
    uint8_t SubEdgeStatus;				//0:δ��⣻1:���У�3:����У�4:���ϣ�5:�̼�������
    uint8_t SubEdgeErrorCode;			//
    uint8_t Preserve;					//����
}APP_CAN_SHAKE_HAND_ACK_STR;			//����Ӧ��(Դ��?����)

typedef struct{
    uint8_t gun;						//���ǹ�� ��1��ʼ
    uint8_t ControlCmd;					//	1��	ϵͳ����
										//	2��	ϵͳ�Լ�
										//	3��	�ϱ����BMS��̬��Ϣ
										//	4��	�ϱ����BMS�����ң����Ϣ
										//	5��	��ʼ���
										//	6��	�������
    uint8_t Preserve[6];				//����
}APP_CAN_REMOTE_CONTROL_STR;			//Զ�̿�������(����?Դ��)

typedef struct{
    uint8_t gun;						//���ǹ�� ��1��ʼ
    uint8_t ControlCmd;
	uint8_t result;						//0���ɹ���1��ʧ��
    uint8_t Preserve;					//����
}APP_CAN_REMOTE_CONTROL_ACK_STR;			//Զ�̿���Ӧ��(Դ��?����)


typedef struct{
    uint8_t gun;						//���ǹ�� ��1��ʼ
    uint8_t EventCode;					//1��ϵͳ�������
										//	2��ϵͳ�Լ����
										//	3����ʼ������֪ͨ
										//	4������������֪ͨ
										//	5��ϵͳ����
										//	6����ѹ
										//	7������
										//	8��Ƿѹ
										//	9��Դ���븱��ͨ���쳣
										//	10����⵽����״̬�仯
										//	11����⵽���߷�������
    uint8_t Para1;						//
    uint16_t Para2;						//
    uint8_t EventState;					//1��������2���ָ� ��Щ�¼��в����ͻָ���״̬
    uint8_t EventLevel;					//1��ʾ   2�澯   3���ع���
}APP_CAN_EVENT_NOTIFY_STR;				//�¼�֪ͨ(Դ��?����)

typedef struct{
    uint8_t gun;						//���ǹ�� ��1��ʼ
    uint8_t EventCode;					//
    uint8_t Para1;						//
    uint16_t Para2;						//
    uint8_t EventState;					//1��������2���ָ� ��Щ�¼��в����ͻָ���״̬
    uint8_t EventLevel;					//1��ʾ   2�澯   3���ع���
}APP_CAN_EVENT_NOTIFY_ACK_STR;			//�¼�֪ͨӦ��(����?Դ��)



typedef struct{
    uint8_t gun;						//���ǹ�� ��1��ʼ
    uint8_t FirmwareType;				//�̼����� 1:Դ�߹̼���2:���߹̼�
    uint8_t FirmwareVeb;				//�̼��汾
    uint32_t FirmwareOffset;			//�̼��·�����ʼ��ַƫ������Ϊ��֧�ֶϵ�����
}APP_CAN_UPGRADE_STR;					//�̼���������(����?Դ��) 

typedef struct{
    uint8_t gun;						//���ǹ�� ��1��ʼ
    uint8_t FirmwareType;				//�̼����� 1:Դ�߹̼���2:���߹̼�
    uint8_t Result;						//0������������1������������2����������
    uint32_t FirmwareOffset;			//
}APP_CAN_UPGRADE_ACK_STR;				//�̼�����Ӧ�� (Դ��?����)


typedef struct{
    uint8_t PackNum;					//�������
    uint8_t sum;						//�̼���Ƭ�ֽ��ۼӺ�
    uint8_t len;						//�̼���Ƭ����
    uint8_t data[64];					//�̼���Ƭ����
}APP_CAN_FIRMWARE_DOWN_STR;				//�̼��·�����(����?Դ��) 

typedef struct{
    uint8_t result;						//��� 0�����ճɹ���1������ʧ�ܣ�2��ֹͣ����
    uint8_t PackNum;					//�������
}APP_CAN_FIRMWARE_DOWN_ACK_STR;			//�̼��·�Ӧ�� (Դ��?����)

#pragma pack()




extern void AppCan_CallBack(uint8_t *data, uint16_t len, uint8_t cmd, uint8_t SourceAddr);

#if USER_ANOTHER_THREAD
extern CAN_BSW_FIFO_STR CanBswSrvAppFifo[CAN_NODE_NUMB];	
extern CAN_BSW_CACHE_STR CanApp_Cache[CAN_NODE_NUMB];	
extern int AppCan_GetOneData(int portIndex, uint8_t *pData);
extern void AppCan_FifoInit(void);
#endif



#endif




