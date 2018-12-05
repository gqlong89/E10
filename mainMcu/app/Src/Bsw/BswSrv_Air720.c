/*
 * @Author: zhoumin 
 * @Date: 2018-10-12 18:08:48 
 * @def :Air4Gģ����������
 * @Last Modified by: zhoumin
 * @Last Modified time: 2018-10-29 20:35:10
 */

#include "BswDrv_Usart.h"
#include "BswSrv_Air720.h"
#include "BswSrv_System.h"
#include "BswSrv_FwUpgrade.h"

/*4Gģ�����at��Ӧbuf*/
uint8_t  gprsBuffer[512];

/*4Gģ���������buf*/
static uint8_t gGsmRecvFifoBuffer[256] = {0};
/*4Gģ���������fifo*/
FIFO_S_t gGsmRecvFifo;

static GSM_STEP istage;
static SOCKET_STATE socketStatus = SOCKET_ERROR;	//SOCKET ״̬
static uint8_t tcp_tx_error_times = 0;
static uint8_t connect_timers = 0;		//���ӷ���������

static int gsm_check_reset(char ok, uint8_t retry);
static int gsm_check_Test(char ok, uint8_t retry);
static int gsm_check_ATE(char ok, uint8_t retry);
static int gsm_check_ATI(char ok, uint8_t retry);
static int gsm_check_CCID(char ok, uint8_t retry);
static int gsm_check_CPIN(char ok, uint8_t retry);
static int gsm_check_CSQ(char ok, uint8_t retry);
#if(SET_ATS0 == 1)
static int gsm_check_ATS0(char ok, uint8_t retry);
#endif
static int gsm_check_CIPSHUT(char ok, uint8_t retry);
static int gsm_check_CIPMUX(char ok, uint8_t retry);
static int gsm_check_CIPRXGET(char ok, uint8_t retry);
static int gsm_check_CIPQSEND(char ok, uint8_t retry);
static int gsm_ipopen(char ok, uint8_t retry);

static void gsm_power_on(void);
static void gsm_power_off(void);
static void gsm_reset(void);

static void GetCcidSn(char *pStr);

static int SOCKET_open_set(char* addr, int port);
static int GprsSendCmd(char *cmd, char *ack, uint16_t waittime, int flag);
static int GprsCheckRes(char *cmd, char *res, uint16_t tmo);
static int GprsSendData(char *data, int len, char *ack, uint16_t waittime);

//ģ���Դ��������
#define GSM_POWER_EN() 			gpio_bit_set(GPIOA,GPIO_PIN_7)
#define GSM_POWER_DISEN()		gpio_bit_reset(GPIOA,GPIO_PIN_7)

//ģ�鿪����������
#define GSM_PWRKEY_HIGH()		gpio_bit_set(GPIOA,GPIO_PIN_5)
#define GSM_PWRKEY_LOW()		gpio_bit_reset(GPIOA,GPIO_PIN_5)

//ģ�黽�ѿ�������
#define GSM_WAKEUP_HIGH()		gpio_bit_set(GPIOA,GPIO_PIN_4)
#define GSM_WAKEUP_LOW()		gpio_bit_reset(GPIOA,GPIO_PIN_4)

//ģ��������������
#define GSM_RESET_HIGH()		gpio_bit_set(GPIOA,GPIO_PIN_6)
#define GSM_RESRT_LOW()			gpio_bit_reset(GPIOA,GPIO_PIN_6)


const gsm_inittab ipinit_tab[]={
    {NULL, "OK", 3000, 10, gsm_check_reset},                    //ģ�鿪��
    {NULL, "OK", 3000, 10, gsm_check_Test},                     //ģ�����
    {"ATE0\r", "OK", 5000, 10, gsm_check_ATE},                  //���ò�����
    {"ATI\r", "OK", 5000, 10, gsm_check_ATI},                   //�鿴ģ����Ϣ
    {"AT+ICCID\r", "OK", 5000, 10, gsm_check_CCID},              //�鿴SIM����ccid
    {"AT+CPIN?\r", "OK", 5000, 10, gsm_check_CPIN},             //��ѯPIN����״̬
    {"AT+CSQ\r", "OK", 1000, 150, gsm_check_CSQ},				//�鿴�ź�����
    #if(SET_ATS0 == 1)
    {"ATS0=1\r", "OK", 3000, 10, gsm_check_ATS0},               //�绰�Զ���ͨ
    #endif
    {"AT+CIPSHUT\r", "OK", 3000, 150, gsm_check_CIPSHUT},		//�ر��ƶ�����
    {"AT+CIPMUX=0\r", "OK", 1000, 150, gsm_check_CIPMUX},		//����TCP������
	{"AT+CIPRXGET=0\r", "OK", 1000, 10, gsm_check_CIPRXGET},	//�����Զ�����
    {"AT+CIPQSEND=0\r", "OK", 1000, 150, gsm_check_CIPQSEND},	//���÷�͸�����ݷ���ģʽ
    {NULL, NULL, 0, 0, gsm_ipopen},                             //TCP����
};


int CheckModeCmd(char ok, uint8_t retry, uint16_t delay)
{
    if (ok==0) {
        istage++;
        return CL_OK;
    }else if (retry > 10) {/*retry 10 times*/
       
        istage = GSM_TEST; /* goto Test */
	}
    
	return CL_FAIL;
}

int gsm_check_reset(char ok, uint8_t retry)
{
	CL_LOG("reset 4G module.\r\n");
    gsm_reset();
	istage++;
	return CL_OK;
}
int gsm_check_Test(char ok, uint8_t retry)
{
    for (uint8_t i=0; i<10; i++) {
        if (GprsSendCmd("AT\r", "OK", ipinit_tab[GSM_RESET].wait, 0)==0) {
            istage ++;
            return CL_OK;
        }
    }
    istage = GSM_RESET;
	CL_LOG("4G AT check failed.\r\n");
	return CL_FAIL;
}
int gsm_check_ATE(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}
int gsm_check_ATI(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}
int gsm_check_CCID(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}
int gsm_check_CPIN(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}
int gsm_check_CSQ(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}
#if(SET_ATS0 == 1)
int gsm_check_ATS0(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}
#endif
int gsm_check_CIPSHUT(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}
int gsm_check_CIPRXGET(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}
int gsm_check_CIPMUX(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}
int gsm_check_CIPQSEND(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}
int gsm_ipopen(char ok, uint8_t retry)
{
    int i;
	int ret;
	ret = BswSrv_Air720_GetSocketState(2);
	if(ret == CL_FAIL){
		istage = GSM_TEST;
		return CL_FAIL;
	}

    if(!(socketStatus == IP_INITIAL || socketStatus == IP_STATUS)){
        istage = GSM_TEST;
        return CL_OK;
    }

	connect_timers++;
    for(i = 0; i < 5; i++){
		if(SOCKET_open_set(NET_SERVER_IP, NET_SERVER_PORT) == CL_OK){
			CL_LOG("[INFO] TCP opened and is connectting.connect_timers=%d\n",connect_timers);
			break;
		}
	}
    if(i == 5){
		istage = GSM_TEST;
		return CL_FAIL;
	}

    //��ѯ����״̬ �ȴ�20s
	for (uint16_t time = 0; time < 20; time++) {
		osDelay(1000);
		ret = BswSrv_Air720_GetSocketState(1);
		if(ret != CL_FAIL)
		{
			if(socketStatus == CONNECT_OK)
			{
				istage = GSM_STATE_NUM;
				connect_timers = 0;
				return CL_OK;
			}else if(socketStatus == TCP_CONNECTING)
			{
				CL_LOG("socket is connecting.\n");
			}
			else if(SOCKET_ERROR == socketStatus)
			{
				CL_LOG("connect failed,is reconnectting. socket=%d.\n",socketStatus);
				istage = GSM_CIPSHUT;
				return CL_OK;
			}else{

			}
		}
	}
	
	istage = GSM_CIPSHUT;
	return CL_OK;
}



void GetCcidSn(char *pStr)
{
    int i;
    int flag = 0;

    for (i=0; i<strlen(pStr); i++) {
        switch (flag) {
            case 0:
                if ((0x0a == pStr[i]) || (0x0b == pStr[i])) {
                    flag = 1;
                }
                break;
            case 1:
                if ((0x30 <= pStr[i]) && (pStr[i] <= 0x39)) {
                    memcpy(GlobalInfo.iccid, &pStr[i], ICCID_LEN);
                    GlobalInfo.iccid[ICCID_LEN] = 0;
                    CL_LOG("SIM num: %s.\n", GlobalInfo.iccid);
                    return;
                }
                break;
        }
    }
}


int GprsCheckRes(char *cmd, char *res, uint16_t tmo)
{
    char *ret=NULL;
    uint8_t c;
    int cnt = 0;
    int retv = CL_FAIL;

    memset(gprsBuffer, 0, sizeof(gprsBuffer));
	
	for (uint16_t time = 0; time < tmo; time+= 50) {
		osDelay(50);
		while(BswDrv_UsartGetOneData(GSM_UART_INDEX,&c) == CL_OK)
		{
			if (c) 
			{
                if (sizeof(gprsBuffer) > cnt) 
				{
                    gprsBuffer[cnt++] = c;
                }
				else
				{
                    //CL_LOG("cnt=%d,error.\n",cnt);
                }
            }
		}
		
		//��ѯ��ǰ����״̬--��Ҫ�������� 
		if(cmd != NULL && strstr(cmd,"AT+CIPSTATUS")){
			uint8_t result = 1;
			retv = CL_OK;
			if(strstr((const char*)gprsBuffer, "CONNECT OK")){
				socketStatus = CONNECT_OK;
			}else if(strstr((const char*)gprsBuffer, "TCP CONNECTING")){
				socketStatus = TCP_CONNECTING;
			}else if(strstr((const char*)gprsBuffer, "TCP CLOSED")){
				socketStatus = TCP_CLOSED;
			}else if(strstr((const char*)gprsBuffer, "IP START")){
				socketStatus = IP_START;
			}else if(strstr((const char*)gprsBuffer, "PDP DEACT")){
				socketStatus = PDP_DEACT;
			}else if(strstr((const char*)gprsBuffer, "IP INITIAL")){
				socketStatus = IP_INITIAL;
			}else if(strstr((const char*)gprsBuffer, "IP GPRSACT")){
				socketStatus = IP_GPRSACT;
			}else if(strstr((const char*)gprsBuffer, "IP STATUS")){
				socketStatus = IP_STATUS;
			}else if(strstr((const char*)gprsBuffer, "IP CONFIG")){
				socketStatus = IP_CONFIG;
			}else if(strstr((const char*)gprsBuffer, "TCP CLOSING")){
				socketStatus = TCP_CLOSING;
			}else{
				socketStatus = SOCKET_ERROR;
				retv = CL_FAIL;
				result = 0;
			}
			if(result == 1){
				break;
			}
		}
		else if(cmd != NULL && strstr(cmd,"AT+CIPSTART"))//�ж�socket����״̬
		{
			if(strstr((const char*)gprsBuffer,"OK") != NULL || strstr((const char*)gprsBuffer,"ALREADY CONNECT") != NULL){
				retv = CL_OK;
				break;
			}
		}
		else
		{
			ret = strstr((const char*)gprsBuffer, (const char*)res);
			if (ret) 
			{
				retv = CL_OK;
				if (cmd != NULL && strstr(cmd, "AT+CSQ")) 
				{
					int temp;
					ret = strstr((char*)gprsBuffer, "+CSQ: ");//+CSQ: 29,0
					if(ret)
					{
						sscanf((const char*)ret,"+CSQ: %d,",&temp);
						GlobalInfo.simSignal = temp;
						CL_LOG("gNetSignal>>>%d.\n",temp);
					}
				}
				else if (strstr(cmd, "AT+ICCID")) 
				{
					GetCcidSn((void*)gprsBuffer);
				}
				else if(strstr(cmd,"ATI"))
				{
					CL_LOG("%s.\n",gprsBuffer);
				}
				break;
			}
		}
	}
 	// CL_LOG("<<< [%s].\n",gprsBuffer);
    return retv;
}

int GprsSendCmd(char *cmd, char *ack, uint16_t waittime, int flag)
{
	uint8_t res = 1;
	
    BswDrv_UsartSend(GSM_UART_INDEX,(void*)cmd, strlen(cmd));

	// CL_LOG(">>> %s.\n",cmd);

    if ((ack==NULL) || (waittime==0)) {
        return CL_OK; 
    }

    if (CL_OK == GprsCheckRes(cmd, ack, waittime)) {
        res = CL_OK; /*check success, retrun 0*/
    }else{
        res = (1==flag) ? 0 : 1;
    }
	return res;
}

//����TCP����
int SOCKET_open_set(char* addr, int port)
{
    char cmd_req[64] = {0};

    sprintf(cmd_req,"AT+CIPSTART=\"TCP\",\"%s\",%d\r", addr, port);

    if(!GprsSendCmd(cmd_req, "OK", 5000, 0)) {
        return CL_OK;
    }else{
        CL_LOG("[INFO] TCP open failure.\n");
        return CL_FAIL;
    }
}

//���� 0�ɹ�
int GprsSendData(char *data, int len, char *ack, uint16_t waittime)
{
	int res = 1;
	
	BswDrv_UsartSend(GSM_UART_INDEX,(void*)data, len);
	
    if ((ack == NULL) || (waittime == 0)) return CL_OK;
	
    if (CL_OK == GprsCheckRes(NULL, ack, waittime)) {
        res = 0; /*check success, retrun 0*/
    }
	return res;
}

void gsm_power_on(void)
{	
	GSM_POWER_EN(); 	//�򿪵�Դ
	osDelay(2000);	
	GSM_PWRKEY_LOW();
	osDelay(1500);
	GSM_PWRKEY_HIGH();
	osDelay(2000);
	CL_LOG("power on 4G\r\n");
}

void gsm_power_off(void)
{
	GSM_POWER_DISEN(); //�رյ�Դ
	CL_LOG("power off 4G\r\n");

}

void gsm_reset(void)
{
	gsm_power_off();		
	osDelay(1000);
	gsm_power_on();		//����
}

int BswSrv_Air720_GetSocketState(int retry)
{
	for(int i = 0; i < retry; i++){
		if(CL_OK == GprsSendCmd("AT+CIPSTATUS\r", "OK", 2000, 0)) 
		{
			return socketStatus;
		}
	}
	return CL_FAIL;
}

int BswSrv_Air720_GetSignal(int retry)
{
	for(int i = 0; i < retry; i++){
		if(CL_OK == GprsSendCmd("AT+CSQ\r", "OK", 2000, 0)) {
			return CL_OK;
		}
	}
	return CL_FAIL;
}


void BswSrv_Air720_CloseSocket(void)
{
	CL_LOG("4G close socket.and reconnect.\r\n");
	if(socketStatus == CONNECT_OK || socketStatus == TCP_CONNECTING){
		GprsSendCmd("AT+CIPCLOSE=1\r","OK",1000, 0);
	}
	GlobalInfo.is_socket_0_ok = 0;
    GlobalInfo.isLogin = 0;
	GlobalInfo.isRecvServerData = 0;
}


int BswSrv_Air720_SendData(int socket,uint8_t* data, uint16_t len)
{
	int res = CL_FAIL;
	char cmd_req[32] = {0};
    if (0 == GlobalInfo.is_socket_0_ok) 
	{
        CL_LOG("socket 0 is closed.\n");
        return CL_FAIL;
    }
	sprintf(cmd_req,"AT+CIPSEND=%d\r", len);
	
	for (int i=0; i<2; i++) 
	{
		res = GprsSendCmd(cmd_req, ">", 1000, 1);
		if (CL_OK != res) {
			CL_LOG("call GprsSendCmd=%d,error.\n",res);
		}
		if (CL_OK == (res = GprsSendData((char*)data, len, "SEND OK", 3000))) 
		{
			// CL_LOG("send OK.\r\n");
            break;
        }
		else
		{
            tcp_tx_error_times++;
			osDelay(100);
        }
	}

	if (res == CL_OK) 
	{
        tcp_tx_error_times = 0;
    }
	else 
	{
        CL_LOG("send fail,tcp_tx_error_times=%d.\n",tcp_tx_error_times);
        if (tcp_tx_error_times >= TX_FAIL_MAX_CNT) {
           	tcp_tx_error_times = 0;
			CL_LOG("send fail,try to restart net.\n");
			BswSrv_Air720_CloseSocket();
        }
    }
    return res;
}


int BswSrv_Air720_Reconnect(GSM_STEP step)
{
	int ok;
	uint8_t retry = 0;
	connect_timers = 0;
	istage = step;
	while(1){
		osDelay(ipinit_tab[istage].nwait*10);
		if (ipinit_tab[istage].cmd) {
			ok = GprsSendCmd(ipinit_tab[istage].cmd,ipinit_tab[istage].res,ipinit_tab[istage].wait, 0);
		}
		if (ipinit_tab[istage].process) {
            if (CL_OK == ipinit_tab[istage].process(ok, retry)) {
                retry = 0;
            }else{
				if(retry++ > 10)
				{
					CL_LOG("4G init failed and exit.\n");
					return CL_FAIL;
				}
            }
        }
		if(connect_timers > 3)
		{
			CL_LOG("4G connect server failed and exit.\n");
			return CL_FAIL;
		}
		if (istage == GSM_STATE_NUM) 
		{
			CL_LOG("4G init and set socket ok.\n");
			return CL_OK;
		}
	}
}



/*********************************FTP***********************************************/

int FtpWaitData(uint16_t tmo)
{
	uint16_t i = 0;
	uint8_t c;
	memset(gprsBuffer, 0, sizeof(gprsBuffer));
	while (--tmo) {
		osDelay(50);
		while(BswDrv_UsartGetOneData(GSM_UART_INDEX,&c) == CL_OK){
			if(i<sizeof(gprsBuffer)){
				gprsBuffer[i++] = c;
			}
		}
		if(strstr((char*)gprsBuffer,"+FTPGET: 1,1")){
			return CL_OK;
		}
	}
	return CL_FAIL;
}

int UsartGetline(char *pBuff,uint16_t tmo)
{
	uint16_t i = 0;
	uint8_t c;
	int len = 0;
	char *p = NULL;
	char *q = NULL;
	int ll = 0;
	while (--tmo) {
		osDelay(30);
		while(BswDrv_UsartGetOneData(GSM_UART_INDEX,&c) == CL_OK){
			if(i<sizeof(gprsBuffer)){
				pBuff[i++] = c;
			}
		}
		if((p = strstr(pBuff,"+FTPGET: 2,")) != NULL){
			if((q = strstr(p,"\r\n")) != NULL){
				sscanf(p,"+FTPGET: 2,%d",&len);
				ll = q - pBuff + 2 ;// 0D 0A
				if(i >= (ll+len)){
					memmove(pBuff,pBuff+ll,len);
					pBuff[len] = '\0';
					return len;
				}
			}
		}else if(strstr(pBuff,"ERROR")){
			CL_LOG("error.\r\n");
			return CL_FAIL;
		}
	}
	CL_LOG("len = %d ll = %d i=%d \n",len,ll,i);
	return CL_FAIL;
}

int BswSrv_Air720_FtpGet(const char* serv, const char* un, const char* pw, const char* file)
{
	uint8_t i = 0;
    char ipconfig[64] = {0};
	int cfize = 0;
    uint8_t  getCnt;
    uint16_t getLen;
	char retry ;
    int len;
	uint16_t checksum = 0;
    char *sp1;
    char tmp[64];
    uint32_t over_time = 0;
	uint8_t lastPercentage=0,percentage;
	FW_HEAD_STR *pFwHead = NULL;
	FW_INFO_STR *pFwInfo = NULL;
	uint8_t upgradeType ;
	
    GprsSendCmd("AT+CIPCLOSE=0\r","CLOSE OK",1000, 0);
    GprsSendCmd("AT+CIPSHUT\r","SHUT OK",1000, 0);

	//����FTP���ܵĳ�������
	for(i=0;i<3;i++){
		if(GprsSendCmd("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r","OK",300,0) == CL_OK)
		{
			break;
		}else{
			osDelay(1000);
		}
	}
	if(i >= 3){
		CL_LOG("����FTP���ܵĳ�������ʧ��.\r\n");
		return CL_FAIL;
	}
	
	//����FTP���ܵ�APN
	for(i=0;i<3;i++){
		if(GprsSendCmd("AT+SAPBR=3,1,\"APN\",\"CMNET\"\r","OK",3000,0) == CL_OK)
		{
			break;
		}else{
			osDelay(1000);
		}
	}
	if(i >= 3){
		CL_LOG("����FTP���ܵ�APNʧ��.\r\n");
		return CL_FAIL;
	}

	for(i=0;i<3;i++){  //����ó��ص�GPRS PDP������

		if(GprsSendCmd("AT+SAPBR=1,1\r","OK",3000, 0) == CL_OK)
		{
			CL_LOG("����ó��ص�GPRS PDP�����ĳɹ�.\r\n");
			break;
		}else{
			osDelay(1000);
		}
	}

    if(i >= 3){
		CL_LOG("����ó��ص�GPRS PDP������ʧ��.\r\n");
		return CL_FAIL;
	}

	//��ѯ�³��ص�״̬
	for(i = 0;i<3;i++){
		if (GprsSendCmd("AT+SAPBR=2,1\r","+SAPBR: 1,1",3000, 0) == CL_OK) {
			CL_LOG("search SAPBR success.\r\n");
			break;
		}else{
			osDelay(1000);
		}
	}
	if(i >= 3)
	{
		CL_LOG("search SAPBR failed.\r\n");
		return CL_FAIL;
	}

    GprsSendCmd("AT+FTPCID=1\r","\r\nOK\r\n",200, 0);	//����cid
	
	//���÷�������ַ
    sp1 = strchr(serv, '/');
    memset(tmp, 0, sizeof(tmp));
    strncpy(tmp,serv,sp1-serv);
	CL_LOG("ftp server=%s \r\n",tmp);
    sprintf(ipconfig, "AT+FTPSERV=\"%s\"\r", tmp);
    GprsSendCmd(ipconfig,"\r\nOK\r\n",200, 0);
	
	//��������
    memset(ipconfig, 0, sizeof(ipconfig));
	CL_LOG("ftp nsername=%s.\n",un);
    sprintf(ipconfig, "AT+FTPUN=\"%s\"\r", un);
    GprsSendCmd(ipconfig,"\r\nOK\r\n",100, 0);
	
	//��������
    memset(ipconfig, 0, sizeof(ipconfig));
	CL_LOG("ftp passwd=%s.\n",pw);
    sprintf(ipconfig, "AT+FTPPW=\"%s\"\r", pw);
    GprsSendCmd(ipconfig,"\r\nOK\r\n",100, 0);

	//��������·��
    memset(tmp, 0, sizeof(tmp));
    strncpy(tmp,sp1+1,strlen(sp1+1));
    tmp[strlen(tmp)] = '/';
	CL_LOG("ftp path=%s.\n",tmp);
    memset(ipconfig, 0, sizeof(ipconfig));
    sprintf(ipconfig, "AT+FTPGETPATH=\"%s\"\r", tmp);
    GprsSendCmd(ipconfig,"\r\nOK\r\n",100, 0);
	
	//�����ļ���
    memset(ipconfig, 0, sizeof(ipconfig));
	CL_LOG("ftp filename=%s.\n",file);
    sprintf(ipconfig, "AT+FTPGETNAME=\"%s\"\r", file);
    GprsSendCmd(ipconfig,"\r\nOK\r\n",100, 0);
	retry = 0;
	
START:

	if(retry++ >= 3)
	{
		CL_LOG("upgrade fail,exit.\n");
		return CL_FAIL;
	}
	CL_LOG("start ftp retry = %d \n",retry);
	
	cfize = 0;
	getCnt = 0;
	checksum = 0;

	BswDrv_UsartFifo_Flush(GSM_UART_INDEX);
	//��FTP���ػỰ
	if(GprsSendCmd("AT+FTPGET=1\r","OK",3000,0) == CL_OK){
		//�ȴ����ݵ���---ģ����������
		if(FtpWaitData(300) != CL_OK){
			CL_LOG("wait data failed.\n");
			GprsSendCmd("AT+FTPQUIT\r","\r\nOK\r\n",100, 0);//�˳���ǰ�Ự
			goto START;
		}else{
			CL_LOG("has data receive.\n");
		}
        getLen = 32;
        getCnt = 0;
		over_time = osGetTimes();
		while(1){
			osDelay(30);
			if ((uint32_t)(osGetTimes() - over_time) > 120*1000) {
				CL_LOG("ftp data timeout.\n");
				GprsSendCmd("AT+FTPQUIT\r","\r\nOK\r\n",100, 0);//�˳���ǰ�Ự
				goto START;
            }
			//��ȡ����
			memset(ipconfig, 0, sizeof(ipconfig));
			sprintf(ipconfig,"AT+FTPGET=%d,%d\r",2, getLen);
			BswDrv_UsartFifo_Flush(GSM_UART_INDEX);
			memset(gprsBuffer, 0, sizeof(gprsBuffer));
			BswDrv_UsartSend(GSM_UART_INDEX,(void*)ipconfig, strlen(ipconfig));
			len = UsartGetline((char*)gprsBuffer,100);
			if(len>0){
				if(getCnt == 0)
				{
					pFwHead = (void*)gprsBuffer;
					if(pFwHead->aa == 0xAA && pFwHead->five == 0x55)
					{
						if(pFwHead->fwCnt == 0)
						{
							CL_LOG("fwCnt=%d,error.\n",pFwHead->fwCnt);
							GprsSendCmd("AT+FTPQUIT\r","\r\nOK\r\n",100, 0);
							goto START;
						}
						pFwInfo = (void*)(gprsBuffer+16);
						if (0 == memcmp("U8M", pFwInfo->name, 3)) {//����
							upgradeType = FW_U8;
						}
						else if (0 == memcmp("U8C", pFwInfo->name, 3)) {//ˢ����
							upgradeType = FW_U8_BAK;
						}
						else{
							CL_LOG("fw name=%s, error.\n",pFwInfo->name);
							GprsSendCmd("AT+FTPQUIT\r","\r\nOK\r\n",100, 0);
							goto START;
						}
						OAT_Init(upgradeType,0,pFwInfo->size,pFwInfo->checkSum,pFwHead->fwVer1);
						getLen = 256;
						getCnt++;
					}
					else
					{
						CL_LOG("��ϢͷУ��ʧ�� ! \r\n");
						GprsSendCmd("AT+FTPQUIT\r","\r\nOK\r\n",100, 0);
						goto START;
					}
				}
				else
				{
					for (int i=0; i<len; i++) {
						checksum += (unsigned)gprsBuffer[i];
					}
					cfize += len;

					//д����
					OAT_FwWrite(upgradeType,gprsBuffer,len);

					percentage = cfize*100/upgradeInfo.fsize;
					if(percentage - lastPercentage >= 5){
						printf("progress=%d%%.\n",percentage);
						lastPercentage = percentage;
					}
					if(cfize >= upgradeInfo.fsize)
					{
						CL_LOG("download filelen over .cfize = %d.\n",cfize);
						break;
					}
				}
			}else if(len == 0){
				CL_LOG("read data over. checksum=%X\n",checksum);
			}else{
				CL_LOG("read data timeout.\n");
			}
		}

		if(cfize == upgradeInfo.fsize && checksum == upgradeInfo.checkSum){
			CL_LOG("ftp download finish .checksum = %X.\n",checksum);
			//�̼���֤
			if(OTA_Check(upgradeType) == CL_OK)
			{
				return upgradeType;
			}
			else
			{
				CL_LOG("OTA_Check failed .\r\n");
				GprsSendCmd("AT+FTPQUIT\r","\r\nOK\r\n",100, 0);
				goto START;
			}
		}else{
			CL_LOG("checksum error.\n");
			GprsSendCmd("AT+FTPQUIT\r","\r\nOK\r\n",100, 0);
			goto START;
		}
	}

    return CL_FAIL;
}



int BswSrv_Air720_StartUp(void)
{
	//����ģ��
	gsm_reset();

	for (uint8_t i=0; i<8; i++) {
        if (GprsSendCmd("AT\r", "OK", ipinit_tab[GSM_RESET].wait, 0)==0) {

            return CL_OK;
        }
    }
	return CL_FAIL;
}

void BswSrv_Air720_Init(void)
{
	FIFO_S_Init(&gGsmRecvFifo, (void*)gGsmRecvFifoBuffer, sizeof(gGsmRecvFifoBuffer));
}


