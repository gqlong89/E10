
#include "includes.h"
#include "GM510.h"
#include "BswSrv_System.h"
#include "BswDrv_Usart.h"
#include "BswDrv_sc8042b.h"
#include "BswDrv_Rtc.h"
#include "BswDrv_Watchdog.h"
#include "BswSrv_FlashUsr.h"
#include "BswDrv_Debug.h"
#include "server.h"

__IO uint8_t gGprsRxBuff[OUT_NET_PKT_LEN];        //TCP接收数据缓冲区
volatile uint8_t gNetSignal=0; 
FIFO_S_t gSocketPktRxCtrl;
volatile uint8_t  gprsBuffer[BUFFER_SIZE];
volatile uint16_t gGprsRxLen;
volatile uint8_t istage; 
volatile uint32_t gSimStatus=0;
SemaphoreHandle_t gGprsSendMux;

const GPRS_INIT_TAB_STR gGM510InitTab[] = 
{
    {NULL, "OK", 3000, 0, GM510_check_test},					//
    {"ATE0\r", "OK", 5000, 10, GM510_check_ATE},				//
    {"AT+GMM\r","OK",1000,3, GM510_check_GMM},
#if (0 == TEST_AIR_WIRE)
    {"ATI\r", "OK", 3000, 10, GM510_check_ATI},				//显示产品ID信息
    {"AT+CPIN?\r", "READY", 500, 10, GM510_check_cpin},		//指示用户是否需要密码
    {"AT+ZGETICCID\r", "OK", 500, 10, GM510_check_CCID},		//查询SIM卡的CCID
	{"AT+CREG?\r","OK", 1000, 50, GM510_check_CREG},			//检查GSM网络注册状态
	{"AT+COPS?\r","OK",1000,50,GM510_check_COPS},				//获取 网络状态
	{"AT+ZPAS?\r","OK",1000,50,GM510_check_ZPAS},				//获取 网络状态
	{"AT+ZIPCALL?\r","OK",3000,50,GM510_check_ZIPCALL},		//查看链接状态---如果链路是打开需要关闭后再连接
	{"AT+ZIPCALL=0\r","OK",5000,100,GM510_check_ZIPCALL0},		//关闭链接
	{"AT+ZIPCALL=1\r","OK",5000,100,GM510_check_ZIPCALL1},		//建立链路 获取IP地址
    {"AT+CSQ\r", "OK", 1000, 10, GM510_check_CSQ},             //查看信号强度
	{NULL, NULL, 0, 0, GM510_ipopen},
#endif
};


#if 0
//******************************************************************
//! \brief  	gprs_power_off
//! \param
//! \retval
//! \note   	4g模块关机
//******************************************************************
void gprs_power_off(void)
{
	CL_LOG("4G modle is power off.\n");

	//通过AT指令关机 关机时间15s
	for (int i=0; i<2; i++) {
        if (GM510SendCmd("AT+ZTURNOFF\r", "OK", 1000, 0)==0) {
            CL_LOG("4G power off ok.please wait 15s...\n");
			break;
        }
		CL_LOG("4G power off failed.retry:%d\n",i+1);
        OS_DELAY_MS(3000);
    }
	vTaskDelay(1000*15);

	//关闭电源
    GPRS_POWER_DISABLE();
    CL_LOG("power off 4G\r\n");
}
#endif


//******************************************************************
//! \brief  	gprs_power_on
//! \param
//! \retval
//! \note   	4g模块开机
//******************************************************************
void MeGprsPowerOn(void)
{
	//打开电源
	GPRS_POWER_ENABLE();
	vTaskDelay(3000);
	GPRS_PWRKEY_LOW();
	vTaskDelay(200);	//拉低0.2s
	GPRS_PWRKEY_HIGH();
	vTaskDelay(1000*15);//等待开机完成
    CL_LOG("power on 4G\r\n");
}


//******************************************************************
//! \brief  	gprs_reset
//! \param
//! \retval
//! \note   	4g模块复位
//******************************************************************
void GM510Reset(void)
{
	//关闭电源
    GPRS_POWER_DISABLE();
	vTaskDelay(1000);
	MeGprsPowerOn();
}


int GM510GetCREG(char *pStr)
{
    char *p = strstr((void *)pStr,"+CREG: ");
    int  state = 0;
    int  netstate = 0;
    if(p)
    {
        sscanf(p,"+CREG: %d,%d",&state,&netstate);
        CL_LOG("CREG : state = %d,netstate = %d\r\n",state,netstate);

        if((netstate == 1) || (netstate == 5))
        {
            return CL_OK;
        }
    }
    return CL_FAIL;
}

int GM510GetModuleId(char *pStr)
{
    char *p = pStr;
    char moduleId[8] = {0};
    if(p)
    {
        sscanf(p,"\n%[^\r]",moduleId);
        CL_LOG("get Module ID : %s\r\n",moduleId);
        if(strncmp(moduleId,"GM510",strlen("GM510")) == 0)
        {
//            gGetModuleType = NET_MODE_4G_GM510;
//            if(gGetModuleType == gCurrentModuleInitType)
              return CL_OK;
        }
    }
    return CL_FAIL;
}

int GM510GetSocketState(char *pStr)
{
    //+ZIPSTAT: <Socket id>,<Status><CR><LF>
    //+ZIPSTAT: 1,0
    int socket = 0,state = 0;

    char *p = strstr(pStr,"+ZIPSTAT: ");
//    CL_LOG("socketState = %s\n",p);
    if(p)
    {
        sscanf(p,"+ZIPSTAT: %d,%d",&socket,&state);
        CL_LOG("socket = %d,state = %d\n",socket,state);
        if((socket == 1)&& (state == 1 || state == 2 || state == 3))    //state = 1,连接打开，2:连接打开但缓冲区满,3:连接正在打开
        {
            return CL_OK;
        }
        else
            return CL_FAIL;
    }
    return CL_FAIL;
}

uint8_t GM510GetNetSignal(void)
{
	return gNetSignal;
}

void GetCcidSn(char *pStr)
{
    int i;
    int flag = 0;

    for (i=0; i<strlen(pStr); i++) {
        switch (flag) {
            case 0:
                if ((0x0a == pStr[i]) || (0x0d == pStr[i])) {
                    flag = 1;
                }
                break;
            case 1:
                if ((0x30 <= pStr[i]) && (pStr[i] <= 0x39)) 
				{
                    if (memcmp(SystemInfo.iccid, &pStr[i], ICCID_LEN)) 
					{
                        memcpy(SystemInfo.iccid, &pStr[i], ICCID_LEN);
                        SystemInfo.iccid[ICCID_LEN] = 0;
                        FlashWriteSysInfo(&SystemInfo, sizeof(SystemInfo));
                    }
                    CL_LOG("SIM iccid: %s.\n", SystemInfo.iccid);
                    return;
                }
                break;
        }
    }
}

int GM510CheckRes(char *cmd, char *res, uint16_t tmo)
{
    char *ret=NULL;
    uint8_t c;
    int cnt = 0;
    int retv = CL_FAIL;

    memset((void*)gprsBuffer, 0, sizeof(gprsBuffer));
    //CL_LOG("gprs_check_res : recv data=");
    for(uint16_t time = 0; time < tmo; time+= 50) {

        vTaskDelay(50);
        while(UsartGetOneData(PHY_UART_GPRS_PORT, &c) == 0) {
            #if (1 == GM510_DEBUG)
            printf("%02x ",c);
            #endif
            if (c) {
                if (sizeof(gprsBuffer) > cnt) {
                    gprsBuffer[cnt++] = c;
                }else{
                    CL_LOG("cnt=%d,error.\n",cnt);
                }
            }
        }

        ret = strstr((const char*)gprsBuffer, (const char*)res);
        if (ret) {
            #if (1 == GM510_DEBUG)
            printf("\r\n");
            #endif
            retv = CL_OK;
            if (NULL != cmd) {
                if (strstr(cmd, "AT+CSQ")) {
                    ret = strstr((char*)gprsBuffer, "CSQ: ");//+CSQ: 23,99
                    if (ret != NULL) {
                        gNetSignal = atoi(&ret[5]);
            			c = atoi(&ret[6]);
                        CL_LOG("4G signal value=%d, ber=%d.\n", gNetSignal,c);
                    }
                }

				else if (strstr(cmd, "AT+ZGETICCID")) {
					GetCcidSn((void*)gprsBuffer);
				}

				else if(strstr(cmd, "AT+ZIPCALL?")){

					//断开状态
					if(strstr((void*)gprsBuffer,"+ZIPCALL: 0")){
						istage++;
					}
				}
                else if(strstr(cmd,"AT+CREG")) {
                    retv = GM510GetCREG((void*)gprsBuffer);
                }
                else if(strstr(cmd,"AT+GMM")) {
                    retv = GM510GetModuleId((void*)gprsBuffer);
                }
                else if(strstr(cmd,"AT+ZIPSTAT")) {
                    retv = GM510GetSocketState((void*)gprsBuffer);
                }
            }
            break;
        }
    }

    #if (1 == GM510_DEBUG)
    CL_LOG("<<< [%s].\n",gprsBuffer);
    #endif

    return retv;
}


//flag = 1是发>
int GM510SendCmd(char *cmd, char *ack, uint16_t waittime, int flag)
{
	uint8_t res = 1;

    xSemaphoreTake(gGprsSendMux,1000);
    BswDrv_UsartSend(PHY_UART_GPRS_PORT, (void*)cmd, strlen(cmd));

    #if (1 == GM510_DEBUG)
	CL_LOG(">>> %s.\n",cmd);
    #endif

    if ((ack==NULL) || (waittime==0)) {
       xSemaphoreGive(gGprsSendMux);
        return CL_OK;
    }

    if (CL_OK == GM510CheckRes(cmd, ack, waittime)) {
        res = CL_OK; /*check success, retrun 0*/
    }else{
        res = (1==flag) ? 0 : 1;
    }
	xSemaphoreGive(gGprsSendMux);
	return res;
}

int GM510trim(char * data)
{
    int i,j;

    for(i=0,j=0; i<strlen((char *)data); i++) {
        if (data[i] != ' ')
            data[j++] = data[i];
    }
    data[j] = '\0';
    return 0;
}


int GM510CheckResnospace(char * cmd,char * ack)
{
    uint8_t data;
    uint8_t flag = 0;

    while (UsartGetOneData(PHY_UART_GPRS_PORT, &data) == 0) {
        if (gGprsRxLen < BUFFER_SIZE) {
            flag = 1;
            gprsBuffer[gGprsRxLen] = data;
			gGprsRxLen++;
        }
        //OS_DELAY_MS(2);
        FeedWatchDog();
    }

    if (flag) {
        //gprsBuffer[gGprsRxLen] = 0;
        //GM510trim((char *)gprsBuffer);
        if(strstr(cmd,"AT$ZFTPGET") != NULL) {
            return 1;
        }
        else {
            if(strstr((char *)gprsBuffer,ack) == NULL) {
                CL_LOG("error <<< %s\n",gprsBuffer);
                return 0;
            } else {
				//PrintfData("GM510CheckResnospace gprsBuffer", (void*)&gprsBuffer[0], 512);
				return 1;
			}    
        }

    }
    return 0;
}


//flag = 1是发>
int GM510SendCmdnospace(char * cmd, char * ack, int waitCnt, int waittime, uint8_t *data)
{
	int i;
    int k;

	xSemaphoreTake(gGprsSendMux,1000);
    for (i=0; i<3; i++) {
        gGprsRxLen = 0;
		UsartFlush(PHY_UART_GPRS_PORT);
        BswDrv_UsartSend(PHY_UART_GPRS_PORT, (void *)cmd, strlen(cmd));
    	if (ack) {
        	for (k=0; k<waitCnt; k++) {
            	OS_DELAY_MS(waittime);		
                FeedWatchDog();
            	if (GM510CheckResnospace(cmd,ack)) {
					xSemaphoreGive(gGprsSendMux);
                	return 0;
            	}
        	}
    	}else{
    		xSemaphoreGive(gGprsSendMux);
        	return 0;
    	}
    }
	xSemaphoreGive(gGprsSendMux);
    return -1;
}


int GM510CheckModeCmd(char ok, uint8_t retry, uint16_t delay)
{
    if (ok==0) {
        istage++;
        return CL_OK;
    }else if (retry > 10) {/*retry 10 times*/
        gSimStatus |= (1<<istage);
        istage = GM510_RESET; /* goto reset */
	}
    return CL_FAIL;
}


int GM510SetZMBN(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}


int GM510QueZMBN(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}


int GM510_check_ATE(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}

int GM510_check_GMM(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}

int GM510_check_ATI(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}

int GM510_check_cpin(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}

int GM510_check_CCID(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}


int GM510_check_ZIPOPEN(char ok,uint8_t retry)
{
	CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}


int GM510_check_ZIPCALL(char ok,uint8_t retry)
{
	CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}

int GM510_check_ZIPCALL0(char ok,uint8_t retry)
{
	CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}

int GM510_check_ZIPCALL1(char ok,uint8_t retry)
{
	CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}


int GM510_check_CSQ(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}

int GM510_check_CREG(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}

int GM510_check_COPS(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}

int GM510_check_ZPAS(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}


int GM510_check_test(char ok, uint8_t retry)
{
    CL_LOG("reset GM510 module\n");
    GM510Reset();

//	while(1)
//	{
//		OS_DELAY_MS(4000);
//	}
	
    for (int i=0; i<10; i++) {
        if (GM510SendCmd("AT\r", "OK", gGM510InitTab[GM510_RESET].wait, 0)==0) {
            CL_LOG("4G Test ok ...\n");
            istage++;
            return CL_OK;
        }
		CL_LOG("4G Test fail..\n");
        OS_DELAY_MS(2000);
		FeedWatchDog();
    }
    gSimStatus |= (1<<istage);
    return CL_FAIL;
}


int GM510_check_ack(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return GM510CheckModeCmd(ok, retry, 0);
}


int GM510OpenSet(int socket, char* addr, int port)
{
    char cmd_req[64] = {0};

    sprintf(cmd_req,"AT+ZIPOPEN=%d,0,%s,%d\r", socket, addr, port);
    if(!GM510SendCmd(cmd_req, "OK", 5000, 0)) 
    {
        CL_LOG("[INFO] link %d open success,addr=%s,port=%d.\n", socket,addr,port);
        SystemInfo.is_socket_0_ok = 1;
        return 0;
    }
    else
    {
        CL_LOG("[INFO] link %d open failure.\n", socket);
        SystemInfo.is_socket_0_ok = 0;
        return -1;
    }
}


int GM510_ipopen(char ok, uint8_t retry)
{
    static uint32_t state=0;
	int res = 0;
    int ready = 0;

    CL_LOG("in.\n");
    switch(state) {
        case NET_STATE_SOCKET0_OPEN:
        {
            ready = CL_FALSE;
            for(int i = 0; i < 10; i++)
            {
                res = GM510OpenSet(SOCKET_ID, NET_SERVER_IP, NET_SERVER_PORT);
                if(!res)
                {
                    ready = CL_TRUE;
                    break;
                }
                OS_DELAY_MS(1000);
            }

            if(CL_TRUE == ready)
            {
                SystemInfo.is_socket_0_ok = 1;
                state = NET_STATE_READY;
            }
            else
            {
                state = NET_STATE_FAILURE;
            }
        }
        break;

        case NET_STATE_READY:
        {
			state = NET_STATE_SOCKET0_OPEN;
            istage = GM510_STATE_NUM;
        }
        break;

        case NET_STATE_FAILURE:
        {
            state=0;
            gSimStatus |= (1<<istage);
            istage = GM510_RESET;
        }
        break;
    }
    return CL_OK;
}


int GM510_default(char ok, uint8_t retry)
{
    if (istage < GM510_STATE_NUM) {
        istage++;
    }
    return CL_OK;
}


int GM510SocketStateCheck(void)
{
    int res = 1;
    if(SystemInfo.is_socket_0_ok == 0)
        return CL_FAIL;

    res = GM510SendCmd("AT+ZIPSTAT=1\r", "OK", 3000, 0);
    if(res != CL_OK)                                    //当前socket连接异常，则直接返回进行网络的重新连接
    {
        CL_LOG("socket 连接状态异常，重启网络.\n");
        SystemInfo.tcp_tx_error_times = 0;
        SystemInfo.is_socket_0_ok = CL_FALSE;
        return CL_FAIL;
    }
    return CL_OK;

}
//返回 0成功
int GM510SendData(char *data, int len, char *ack, uint16_t waittime)
{
	int res = 1;

    BswDrv_UsartSend(PHY_UART_GPRS_PORT, (void*)data, len);
    #if (1 == GM510_DEBUG)
	PrintfData("GM510", (void*)data, len);
    #endif
    if ((ack == NULL) || (waittime == 0)) return CL_OK;
    if (CL_OK == GM510CheckRes(NULL, ack, waittime)) {
        res = 0; /*check success, retrun 0*/
    }
	return res;
}


int GM510SocketSend(int socket, uint8_t* data, uint16_t len)
{
    char cmd_req[64] = {0};
    int res=0;

    if ((1 == socket) && (CL_FALSE == SystemInfo.is_socket_0_ok)) {
        CL_LOG("socket 0 is closed.\n");
        return -1;
    }

    for (int i=0; i<2; i++) {
		sprintf(cmd_req,"AT+ZIPSENDRAW=%d,%d\r", 1, len);
        res = GM510SendCmd(cmd_req, ">", 1000, 1);
        if (0 != res) {
            CL_LOG("call GM510SendCmd=%d,error.\n",res);
        }

        if (CL_OK == (res = GM510SendData((char*)data, len, "OK", 10000))) {
            break;
        }else{
            if(GM510SocketStateCheck() != CL_OK) {
                CL_LOG("发送数据过程中，socket连接断开\n");
                return CL_FAIL;
            }
            SystemInfo.tcp_tx_error_times++;
            OS_DELAY_MS (1000);
        }
    }

    if (res == 0) {
        SystemInfo.tcp_tx_error_times = 0;
    }else {
        CL_LOG("send fail: res=%d,tcp_tx_error_times=%d.\n",res,SystemInfo.tcp_tx_error_times);
        if (SystemInfo.tcp_tx_error_times >= TX_FAIL_MAX_CNT) {
            SystemInfo.tcp_tx_error_times = 0;
			CL_LOG("send fail,try to restart net.\n");
			SystemInfo.is_socket_0_ok = CL_FALSE;
            gSimStatus |= (1<<(GM510_STATE_NUM+1));
        }
    }
    return res;
}


int UsartGetline(char *cmd,uint16_t readLen,uint16_t tmo)
{
	uint8_t c;
	uint16_t i = 0;

	memset((void*)gprsBuffer, 0, sizeof(gprsBuffer));
	UsartFlush(PHY_UART_GPRS_PORT);
    BswDrv_UsartSend(PHY_UART_GPRS_PORT, (void *)cmd, strlen(cmd));
	
	while (--tmo) {
		
		OS_DELAY_MS(20);
		
		while(UsartGetOneData(PHY_UART_GPRS_PORT, &c) == CL_OK) {
			if(i<sizeof(gprsBuffer)){
				gprsBuffer[i++] = c;
			}else{
				break;
			}
		}
		
		if(i >= readLen)
		{
			if((strstr((char*)&gprsBuffer[readLen],"\r\nOK\r\n")) != NULL)
			{
				return CL_OK;
			}
			else if((strstr((char*)&gprsBuffer[readLen],"\r\nERROR\r\n")) != NULL)
			{
				return CL_FAIL;;
			}
		}
	}
	
	CL_LOG("read data timeout.\r\n");
	return CL_FAIL;
}

int GM510FtpGet(const char* serv, const char* un, const char* pw, const char* file, uint8_t fwType)
{
    char ipconfig[64] = {0};
    int ret=0;
    int fsize;
    int cfize=0;
    uint16_t chsum_in;
    uint16_t checksum;
    uint8_t  getCnt;
    uint8_t  fwCnt;
	uint8_t  fwVer;
    uint32_t getLen;
    uint32_t offset;
    int i;
    char *sp1;
    char tmp[64];
    uint32_t over_time;
    uint32_t appBackupRecordAddr;
    DOWN_FW_INFO_STR *pFwInfo = NULL;

	gprsBuffer[0] = 0;
    //GM510SendCmd("AT$ZFTPCLOSE\r","OK",1000,0);	//AT$ZFTPCLOSE, OK
	GM510SendCmd("AT+ZIPCLOSE=1\r","OK",1000,0);	//AT+ZIPCLOSE=1, OK
    GM510SendCmd("AT+ZIPCALL=0\r","OK",1000,0);	//AT+ZIPCLOSE=1, OK
	GM510SendCmd("AT+CSQ\r","OK",1000,0);
	GM510SendCmd("AT+ZPAS?\r","OK",1000,0);
	//GM510SendCmd("AT+CGDCONT=1,\"IP\",\"cmnet\"\r","OK",1000,0);
	GM510SendCmd("AT+ZIPCALL=1\r","OK",1500,0);
	GM510SendCmd("AT+ZDNSCFG?\r","OK",1500,0);
	//GM510SendCmd("AT$ZPDPACT=1\r","OK",1500,0);
	
    sp1 = strchr(serv, '/');//查找出现"/"的位置
    memset(tmp, 0, sizeof(tmp));
    strncpy(tmp,serv,sp1-serv);
	strcat(tmp,",");
	strcat(tmp,un);
	strcat(tmp,",");
	strcat(tmp,pw);
    sprintf(ipconfig, "AT$ZFTPCFG=%s\r", tmp);//AT$ZFTPCFG=118.31.246.230,x5,x543
    CL_LOG("zftpcfg=%s.\n",ipconfig);
    if(GM510SendCmdnospace(ipconfig,"OK",5,500,NULL) != 0) {   //设置FTP连接
        CL_LOG("set ftp config error!\n");
        return -1;
    }

	/*ftp get filesize*/
	memset(tmp, 0, sizeof(tmp));
    strncpy(tmp,sp1+1,strlen(sp1+1));
    tmp[strlen(tmp)] = '/';
	strcat(tmp,file);
	memset(ipconfig, 0, sizeof(ipconfig));
    sprintf(ipconfig, "AT$ZFTPSIZE=%s\r", tmp);
	CL_LOG("ftp get file size=%s.\n",ipconfig);
	if (GM510SendCmdnospace(ipconfig, "FTPSIZE:",10,500,NULL) == 0) {//AT$ZFTPSIZE=TMP/T.LOG
		sprintf(tmp,"FTPSIZE:");
        fsize = restoi((char *)gprsBuffer, tmp,'\r');
        CL_LOG("ftp get file size str=%s\n",gprsBuffer);
        if (fsize==0) {
            CL_LOG("fsize=0, fail.\n");
            ret = -2;
            goto EXIT1;
        }
    }
    CL_LOG("fsize=%d.\n",fsize);

    ret = 0;
    over_time = GetRtcCount();
    getLen = 32;
    getCnt = 0;
    offset = 0;
    while(1) {
        FeedWatchDog();
        if (360 < (uint32_t)(GetRtcCount() - over_time)) {
            CL_LOG("download timeOut,error.\n");
            break;
        }

        memset(tmp, 0, sizeof(tmp));
        strncpy(tmp,sp1+1,strlen(sp1+1));
        tmp[strlen(tmp)] = '/';
    	strcat(tmp,file);
    	memset(ipconfig, 0, sizeof(ipconfig));
        sprintf(ipconfig, "AT$ZFTPGET=%s,%d,%d\r", tmp, offset, getLen);
		if(UsartGetline(ipconfig,getLen,100) != -1){
			offset += getLen;
            if (getCnt == 0) {//消息头
				if ((0xaa == gprsBuffer[0]) && (0x55 == gprsBuffer[1])) {
					fwCnt = gprsBuffer[2];
					getCnt++;
					CL_LOG("get aa 55 ok.\n");
					if (0 == fwCnt) 
                    {
						CL_LOG("fwCnt=%d,error.\n",fwCnt);
						return CL_FAIL;
					}
					pFwInfo = (void*)(gprsBuffer+16);
					fsize = pFwInfo->size;
                    chsum_in = pFwInfo->checkSum;
                    if (0 == memcmp("X10G", pFwInfo->name, 4)) {
                        getCnt++;
                        CL_LOG("check fw name ok,fsize=%d,chsum_in=%d.\n",fsize,chsum_in);
                        getLen = PRE_WRITE_FLASH_LEN;
                        
						if ((fsize == SystemInfo.localFwInfo.size) && (chsum_in == SystemInfo.localFwInfo.checkSum)) {
							CL_LOG("X10G fw is the same, no need to download.\n");
							return CL_OK;
						}
						memset(&SystemInfo.localFwInfo, 0, sizeof(SystemInfo.localFwInfo));
						FlashWriteSysInfo(&SystemInfo, sizeof(SystemInfo));
						appBackupRecordAddr = AppUpBkpAddr;
                        checksum = 0;
                        cfize = 0;
                    }
                    else
                    {
                        CL_LOG("fw name=%s, error.\n",pFwInfo->name);
						PrintfData("gprsBuffer", (void*)gprsBuffer, 16);
                        return CL_FAIL;
                    }
				}
            }
			else //数据类容
			{
				for (i=0; i<getLen; i++) 
                {
					checksum += gprsBuffer[i];
				}
				cfize += getLen;
				
				printf("total %d,get %d,[%d%%].\n",fsize,cfize,cfize*100/fsize);
			   
				FlashWriteAppBackup(appBackupRecordAddr, (void*)&gprsBuffer[0], getLen);
				appBackupRecordAddr += getLen;
				getLen = fsize - cfize;
				getLen = (PRE_WRITE_FLASH_LEN <= getLen) ? PRE_WRITE_FLASH_LEN : getLen;

				if (cfize >= fsize) 
                {
					if (checksum == chsum_in) 
                    {
						CL_LOG("write file size %d, checksum %4X,fwVer=%d,ftp get success finish! .\n", fsize, checksum,fwVer);
						WriteUpdateInfo(fsize, checksum);
						SystemInfo.localFwInfo.size = cfize;
						SystemInfo.localFwInfo.checkSum = checksum;
						SystemInfo.localFwInfo.ver = fwVer;
						SystemInfo.localFwInfo.sum = GetPktSum((void*)&SystemInfo.localFwInfo, sizeof(SystemInfo.localFwInfo)-1);
						FlashWriteSysInfo(&SystemInfo, sizeof(SystemInfo));
						ret = 0;
						OptSuccessNotice(801);
					}else{
						CL_LOG("checksum=%d != chsum_in=%d,error.\n",checksum,chsum_in);
						ret = CL_FAIL;
						OptFailNotice(55);
					}
					goto EXIT1;
				}
			}
        }else{
            CL_LOG("call GM510SendCmdnospace fail,offset=%d,getLen=%d.\n",offset, getLen);
        }
    }
EXIT1:
    CL_LOG("ret=%d.\n",ret);
    return ret;
}


//******************************************************************
//! \brief  	GprsInit
//! \param
//! \retval
//! \note   	2g模块初始化
//******************************************************************
int GM510Init(void)
{
	FIFO_S_Init(&gSocketPktRxCtrl, (void*)gGprsRxBuff, sizeof(gGprsRxBuff));        //初始化相应的Socket缓冲区
    
	//PA4--4G_WAKE_M	PA5--4G_PWRKEY	PA6--4G_RESET	PA7--4G_SLEEP	PC4--4G_RI	PE7--4G_EN
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOE);

	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);		
	gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);

	//复位脚置低
	GPRS_RESET_LOW();

	//WEAK_IN置高
	GPRS_WAKE_HIGH();

	gGprsSendMux = xSemaphoreCreateMutex();

    CL_LOG("GprsInit ok.\n");
	return CL_OK;
}


int GM510Reconnect(void)
{
	char ok;
    uint8_t retry = 0;

    istage = GM510_RESET;
	while(1) 
	{
        FeedWatchDog();
        OS_DELAY_MS(gGM510InitTab[istage].nwait*10);
        if (gGM510InitTab[istage].cmd) 
		{
            ok = GM510SendCmd(gGM510InitTab[istage].cmd, gGM510InitTab[istage].res, gGM510InitTab[istage].wait, 0);
        }

        if (gGM510InitTab[istage].process) 
		{
            if (CL_OK == gGM510InitTab[istage].process(ok, retry)) 
			{
                retry = 0;
            }
			else
			{
                retry++;
            }
        }

		if (istage == GM510_STATE_NUM) 
		{
			CL_LOG("GM510 init and set socket ok.\n");
			return CL_OK;
		}
		else if (GM510_RESET == istage) 
		{
            CL_LOG("GM510 init and set socket fail.\n");
            return CL_FAIL;
        }
	}
}



