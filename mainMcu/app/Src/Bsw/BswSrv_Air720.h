#ifndef __AIRM2M_H__
#define __AIRM2M_H__

#include "includes.h"

#define SET_ATS0        1	//�Ƿ������Զ������绰

typedef enum{
    GSM_RESET = 0,      //0.ģ�鸴λ
    GSM_TEST,           //ģ�����
	GSM_ATE,			//���û���
	GSM_ATI,			//��ʾ��ƷID��Ϣ
	GSM_CCID,		    //�鿴SIM����CCID
	GSM_CPIN,
	GSM_CSQ,			//�鿴�ź�����
    #if(SET_ATS0 == 1)
	GSM_ATS0,		    //�Զ�����
    #endif
	GSM_CIPSHUT,		//�ر��ƶ�����
	GSM_CIPMUX,		    //����TCP������ 
	GSM_CIPRXGET,		//�Զ�����
	GSM_CIPQSEND,	    //���÷�͸�����ݷ���ģʽ
	GSM_IPOPEN,		    //����TCP����
	GSM_STATE_NUM,
}GSM_STEP;

typedef enum{
	IP_INITIAL = 0,
	IP_START,
	IP_CONFIG,
	IP_GPRSACT,
	IP_STATUS,
	TCP_CONNECTING,
	CONNECT_OK,
	TCP_CLOSING,
	TCP_CLOSED,
	PDP_DEACT,
	SOCKET_ERROR 
}SOCKET_STATE;

typedef struct{
    char *cmd;
    char *res;
    int wait;                               //atָ��ȴ�Ӧ��ʱ��
    int nwait;                              //���Լ��ʱ��
    int (*process)(char ok, uint8_t retry);
}gsm_inittab;

int BswSrv_Air720_GetSocketState(int retry);
int BswSrv_Air720_GetSignal(int retry);
void BswSrv_Air720_CloseSocket(void);
int BswSrv_Air720_SendData(int socket,uint8_t* data, uint16_t len);

int BswSrv_Air720_Reconnect(GSM_STEP step);

int BswSrv_Air720_FtpGet(const char* serv, const char* un, const char* pw, const char* file);

int BswSrv_Air720_StartUp(void);
void BswSrv_Air720_Init(void);


#endif
