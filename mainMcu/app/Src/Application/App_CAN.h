#ifndef __APP_CAN_H__
#define __APP_CAN_H__


#include "includes.h"
#include "CAN_Config.h"
#include "BswSrv_CAN.h"


enum {
    CAN_CMD_HAND 				= 1,				//��������
	CAN_CMD_REMOTE_CONTROL 		= 2,                //Զ�̿���
	CAN_CMD_EVENT_NOTICE 		= 3,                //�¼�֪ͨ
	CAN_CMD_REQUES_UPGRADE		= 4,                //����̼�����
	CAN_CMD_DOWN_FW_INFO		= 5,                //�̼��·�
};


#pragma pack(1)

typedef struct {
    uint8_t  five;                                    //0.
    uint8_t  aa;                                  	//1.
	uint16_t len;								    //2.����
    uint8_t  sn;                                    //4. �����Ų����ʾ�����ط�
    uint8_t  cmd;                                   //6.�������
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
}APP_CAN_SHAKE_HAND_STR;

typedef struct{
    uint8_t gun;						//���ǹ�� ��1��ʼ
    uint8_t SourceEdgeVeb;				//Դ���������еĹ̼��汾��
    uint8_t SubEdgeVeb;					//����flash�л���ĸ��߹̼��汾��
    uint8_t SourceEdgeStatus;			//1:���У�2:ռ�ã�3:����У�4:���ϣ�5:�̼�������
    uint8_t SourceEdgeErrorCode;		//
    uint8_t SubEdgeStatus;			//0:δ��⣻1:���У�3:����У�4:���ϣ�5:�̼�������
    uint8_t SubEdgeErrorCode;		//
}APP_CAN_SHAKE_HAND_ACK_STR;

#pragma pack()




extern void AppCan_CallBack(uint8_t *data, uint16_t len, uint8_t cmd);

#if USER_ANOTHER_THREAD
extern CAN_BSW_FIFO_STR CanBswSrvAppFifo[CAN_NODE_NUMB];	
extern CAN_BSW_CACHE_STR CanApp_Cache[CAN_NODE_NUMB];	
extern int AppCan_GetOneData(int portIndex, uint8_t *pData);
extern void AppCan_FifoInit(void);
#endif



#endif




