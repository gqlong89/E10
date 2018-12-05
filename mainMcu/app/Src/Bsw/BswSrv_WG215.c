 /* @Author: zhoumin 
 * @Date: 2018-10-12 18:25:48 
 * @def :wifi模块管理
 * @Last Modified by: zhoumin
 * @Last Modified time: 2018-10-29 20:33:44
 */
#include "BswDrv_Usart.h"
#include "BswDrv_Debug.h"
#include "BswSrv_WG215.h"
#include "BswSrv_System.h"
#include "BswSrv_FwUpgrade.h"


// const char *at_cmd[WIFI_CMD_END] = 
// {
// 	"AT\r\n",
// };


/*wifi模块 AT指令接收buf*/
static uint8_t WifiRecvAtBuff[RECEIVE_AT_LEN] = {0,};


/*wifi应用层接收buf*/
static uint8_t rWifiFifoBuffer[256] = {0,};
/*蓝牙应用层接收buf*/
static uint8_t rBlueFifoBuffer[256] = {0,};

/*wifi模块底层数据接收buf*/
static uint8_t wifiBlueFifiBuffer[WIFI_BLUE_BUF_LEN] = {0,};

/*wifi应用层接收fifo*/
FIFO_S_t rWifiFifo;
/*蓝牙应用层接收fifo*/
FIFO_S_t rBlueFifo;
/*wifi模块底层数据接收fifo*/
FIFO_S_t wifiBlueFifo;


// uint16_t gWifiRxLen;

static WIFI_MSG_ENUM WifiCmdStep = WIFI_AT_CMD;

static uint8_t tcp_tx_error_times = 0;


static int BSPSendWifiData(char *data, int len, char *ack, uint16_t waittime);
static int BSPSendBlueData(char *data, int len);


void OS_WIFI_Delays(uint16_t delay)
{
    uint16_t i;

	for(i = 0; i < delay; i++)
	{
		osDelay(1000);
	}

	return;
}


int WifiCheckAck(char *cmd, char *ack, uint16_t waittime)
{
    char *FirstAddr = NULL;
    uint8_t RecvData;
    uint8_t RecvCount = 0;
    int returnData = CL_FAIL;

	uint8_t *pbuff = &WifiRecvAtBuff[0];
	uint32_t FirstTime;

    memset(pbuff, 0, RECEIVE_AT_LEN);
	osDelay(50);
    
    FirstTime = osGetTimes();

	while((uint32_t)(FirstTime + waittime) >= osGetTimes())
	{
		while(BswDrv_UsartGetOneData(WIFI_UART_INDEX,&RecvData) == CL_OK)
	    {
			//printf("c=%c \r\n",RecvData);
	        if(RecvData)
	        {
	            if (RECEIVE_AT_LEN > (RecvCount+2))
	            {
	                pbuff[RecvCount++] = RecvData;
	            }
	            else
				{
	                // CL_LOG("RecvCount = %d , RecvData = %d ,error.\n", RecvCount , RecvData);
	            }
	        }
	        if((RECEIVE_AT_LEN - 2) <= RecvCount)
	        {
	            // CL_LOG("RecvCount = %d\n", RecvCount);
	            break;
	        }
	    }

	    FirstAddr = strstr((const char*)pbuff, (const char*)ack);
	    if(FirstAddr)
	    {
			//获取版本号
			if(strstr(cmd, "AT+VERS?"))
	        {
	   
			}
			//获取蓝牙名称
			if(strstr(cmd, "AT+NAME?"))
	        {

			}
			            
	        returnData = CL_OK;
			break;
	    }
		osDelay(20);
	}

	#ifdef WG215_DBG
	CL_LOG("WifiCheckAck=%s \r\n",pbuff);
	#endif
	
    return returnData;
}

int WifiSendCmd(char *cmd, char *ack, uint16_t waittime, int flag)
{
	uint8_t returnData = 1;

	#ifdef WG215_DBG
	CL_LOG("WifiSendCmd=%s \r\n",cmd);
	#endif
    BswDrv_UsartSend(WIFI_UART_INDEX,(void *)cmd, strlen(cmd));
	// UsartSend(portIndex, (void *)cmd, strlen(cmd));
	
    if ((ack == NULL) || (waittime == 0))
	{
        return CL_OK; 
    }
    if (CL_OK == WifiCheckAck(cmd, ack, waittime)) 
	{
        returnData = CL_OK; /*check success, retrun 0*/
    }
	else
	{
        returnData = (1 == flag) ? 0 : 1;
    }
	
	return returnData;
}

int WifiSendCmdHandle(char *cmd, char *ack, uint16_t waittime)
{
	if(0 == WifiSendCmd(cmd, ack, waittime, 0))
	{
	    // printf("WifiCmdStep = [%d], cmd send OK.\n", WifiCmdStep);
	    WifiCmdStep++;
	    return CL_OK;
	}
    
    return CL_FAIL;
}



int WifiBuleSendATCom(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};

    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT\r\n");

	if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int WifiDisconnectServer(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};

    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+CIPCLOSE\r\n");

	if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int WifiBuleReset(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};

    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+RST\r\n");

	if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int BuleSetClient(uint16_t waittime, uint8_t Init)
{
	char  CmdBuff[32] = {0,};

    memset(CmdBuff, 0, sizeof(CmdBuff));
	sprintf(CmdBuff, "AT+BLEINIT=%d\r\n", Init);

	if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int BuleSetName(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};
	char name[32] = {0,};

	strcpy(name, GlobalInfo.BlueName);
    memset(CmdBuff, 0, sizeof(CmdBuff));
	sprintf(CmdBuff, "AT+BLENAME=\"%s\"\r\n", name);
	CL_LOG("发送设置蓝牙名字命令[%s].\n", CmdBuff);

	if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int BuleSetBLEADVPARAM(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};

    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+BLEADVPARAM=50,50,0,0,7\r\n");
	//CL_LOG("[%s].\n", CmdBuff);

	if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int BuleSetBLEADVDATA(uint16_t waittime)
{
	char  CmdBuff[128] = {0,};

    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+BLEADVDATA=\"0201061AFF4C000215FDA50693A4E24FB1AFCFC6EB07647825273CB9C69C\"\r\n");
	#ifdef WG215_DBG
	CL_LOG("设置广播数据[%s].\n", CmdBuff);
	#endif

	if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int BuleScanRSPDATA(uint16_t waittime)
{
	char  CmdBuff[128] = {0,};
	char  StrBuff[4] = {0,};
	char  NameBuff[64] = {0,};
    uint8_t i = 0;
	
	memset(NameBuff, 0, sizeof(NameBuff));
    for(i = 0; i < strlen(GlobalInfo.BlueName); i++)
    {
    	memset(StrBuff, 0, sizeof(StrBuff));
		sprintf(StrBuff, "%01X%01X", (((uint8_t)GlobalInfo.BlueName[i])/16), (uint8_t)GlobalInfo.BlueName[i]%16);
		strcat(NameBuff, StrBuff);
    }
    memset(CmdBuff, 0, sizeof(CmdBuff));
	sprintf(CmdBuff, "AT+BLESCANRSPDATA=\"%02X09%s\"\r\n", (strlen(GlobalInfo.BlueName) + 1), NameBuff);
	#ifdef WG215_DBG
	CL_LOG("设置扫描响应数据[%s].\n", CmdBuff);
	#endif

	if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int BuleCreatGATTSSRVCRE(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};

    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+BLEGATTSSRVCRE\r\n");

	if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int BuleStartGATTSSRVCRE(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};
    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+BLEGATTSSRVSTART\r\n");

	if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int BuleStartBroadCast(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};

    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+BLEADVSTART\r\n");

	if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int WifiBuleCheckATCmd(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};

    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT\r\n");
	
    if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int WifiBuleCheckVERCmd(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};
		
    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+VER?\r\n");
	
    if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}


int WifiBlueSetATE(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};
		
    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "ATE0\r\n");
	
    if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
    return CL_FAIL;
}

int WifiBuleSetStaApMode(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};
		
    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+CWMODE=1\r\n");
	
    if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int WifiBuleGetStaApMode(uint16_t waittime)
{
	char  CmdBuff[32] = {0,};
		
    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+CWMODE?\r\n");
	
    if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}


int WifiConnecteAp(uint16_t waittime)
{
	char CmdBuff[64] = {0,};
    memset(CmdBuff, 0, sizeof(CmdBuff));
    sprintf(CmdBuff, "AT+CWJAP=%s,%s\r\n", SystemInfo.WifiName, SystemInfo.WifiPasswd);
	
	// #ifdef WG215_DBG
	CL_LOG("连网命令CmdBuff = [%s].\n", CmdBuff);
	// #endif

	
    if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int WifiEnableDns(uint16_t waittime)
{
	char CmdBuff[64] = {0,};
    	
    memset(CmdBuff, 0, sizeof(CmdBuff));
	sprintf(CmdBuff, "AT+CIPDOMAIN=\"%s\"\r\n", NET_SERVER_IP);
	CL_LOG("域名解析命令 [%s].\n", CmdBuff);

	
    if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int WifiEnableDHCP(uint16_t waittime)
{
	char CmdBuff[32] = {0,};
    	
    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+CWDHCP=1,1\r\n");

    if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int WifiGetDHCPState(uint16_t waittime)
{
	char CmdBuff[32] = {0,};

    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+CWDHCP?\r\n");

    if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int WifiGetLocalIP(uint16_t waittime)
{
	char CmdBuff[32] = {0,};
	char ack[16] = {0,};
    	
    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+CIFSR\r\n");
	memset(ack, 0, sizeof(ack));
	strcpy(ack, "OK");
	
    if(CL_OK == WifiSendCmdHandle(CmdBuff, ack, waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int WifiConnecteServer(uint16_t waittime,char *host,uint16_t port)
{
	char CmdBuff[64] = {0,};
	
    memset(CmdBuff, 0, sizeof(CmdBuff));
    sprintf(CmdBuff, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", host, port);
	CL_LOG("连服务器命令[%s].\n", CmdBuff);
	
    if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int WifiSetTransMode(uint16_t waittime, uint8_t Mode)
{
	char CmdBuff[64] = {0,};
	
    memset(CmdBuff, 0, sizeof(CmdBuff));
//	strcpy(CmdBuff, "AT+CIPMODE=%01x\r\n", Mode);
    sprintf(CmdBuff, "AT+CIPMODE=%01x\r\n", Mode);
	CL_LOG("设置发送方式命令[%s].\n", CmdBuff);

    if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}

int WifiPingIP(uint16_t waittime)
{
	char CmdBuff[64] = {0,};
	
    memset(CmdBuff, 0, sizeof(CmdBuff));
	strcpy(CmdBuff, "AT+PING=\"www.baidu.com\"\r\n");
	CL_LOG("Ping包[%s].\n", CmdBuff);

    if(CL_OK == WifiSendCmdHandle(CmdBuff, "OK", waittime))
	{
		return CL_OK;
	}
	
    return CL_FAIL;
}


int BSPSendBlueData(char *data, int len)
{
	int res = 1;
	
    BswDrv_UsartSend(WIFI_UART_INDEX, (void*)data, len);

	// PrintfData("蓝牙底层发送数据", (void*)data, len);
	
	return res;
}

int BSPSendWifiData(char *data, int len, char *ack, uint16_t waittime)
{
	int res = 1;
    BswDrv_UsartSend(WIFI_UART_INDEX, (void*)data, len);
	
    if((ack == NULL) || (waittime == 0))
	{
		return CL_OK;
	}
    if(CL_OK == WifiCheckAck( NULL, ack, waittime)) 
	{
        res = 0; /*check success, retrun 0*/
    }
	else
	{
		CL_LOG("Wifi发送数据错误! \n");
	}
	
	return res;
}

///////////////////////////http//////////////////////////////////////////

int HttpSendCmd(char *host,uint16_t port)
{
	uint8_t i = 0;
    uint8_t k = 0;
	char cmd_req[32] = {0,};
	char cmd_ack[32] = {0,};
	int ret = CL_FAIL;
	
	for(k = 0; k < 6; k++)
	{
		BSPSendWifiData((char*)"+++", strlen("+++"), NULL, 0);
		osDelay(50);
	}
	
	for(k = 0; k < 8; k++)
	{
		if(CL_FAIL == WifiBuleReset(1000))
		{
			CL_LOG("WifiBuleReset错误, [%d]error! \n", i);
			OS_WIFI_Delays(1);
			if(5 < k)
			{
				CL_LOG("hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh.\n");
				return CL_FAIL;
			}
		}
		else
		{
			OS_WIFI_Delays(1);
			break;
		}
	}
	
//#if 1
//	for(k = 0; k < 6; k++)
//	{
//		if(CL_FAIL == WifiConnecteAp(1000))
//		{
//			CL_LOG("WifiConnecteAp错误, [%d]error! \n", i);
//			vTaskDelay(5000);
//		}
//		else
//		{
//			vTaskDelay(1000);
//			break;
//		}
//	}
//#else
//	OS_WIFI_Delays(8);
//#endif
//	
	OS_WIFI_Delays(10);

	for(k = 0; k < 8; k++)
	{
		if(CL_FAIL == WifiConnecteServer(5000,host,port))
		{
			CL_LOG("WifiConnecteServer错误, [%d]error! \n", i);
			OS_WIFI_Delays(1);
			if(5 < k)
			{
				CL_LOG("kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk.\n");
				return CL_FAIL;
			}
		}
		else
		{
			OS_WIFI_Delays(2);
			break;
		}
	}
	for(k = 0; k < 8; k++)
	{
		if(CL_FAIL == WifiSetTransMode(2000, 1))
		{
			CL_LOG("WifiSetTransMode错误, [%d]error! \n", i);
			OS_WIFI_Delays(1);
			if(5 < k)
			{
				CL_LOG("fffffffffffffffffffffffffffff.\n");
				return CL_FAIL;
			}
		}
		else
		{
			OS_WIFI_Delays(1);
			break;
		}
	}
	for(k = 0; k < 6; k++)
	{
		memset(cmd_ack, 0, sizeof(cmd_ack));
		strcpy(cmd_ack, "OK");
		memset(cmd_req, 0, sizeof(cmd_req));
		strcpy(cmd_req,"AT+CIPSEND\r\n");
		CL_LOG("发送数据命令[%s].\n", cmd_req);
		if (0 != (ret = WifiSendCmd(cmd_req, cmd_ack, 3000, 0)))
		{
			OS_WIFI_Delays(1);
			CL_LOG("call WifiSendCmd=%d, error.\n", ret);
			if(5 < k)
			{
				CL_LOG("fffffffffffffffffffffffffffff.\n");
				return CL_FAIL;
			}
		}
		else
		{
			OS_WIFI_Delays(1);
			break;
		}
	}

	return ret;
}


/**
 *  http返回样式：文本
HTTP/1.1 206 Partial Content
Server: nginx/1.2.6
Date: Thu, 01 Nov 2018 09:48:30 GMT
Content-Type: text/plain
Content-Length: 5
Last-Modified: Thu, 01 Nov 2018 06:52:20 GMT
Connection: keep-alive
Access-Control-Allow-Origin: http://wx.chargerlink.com
Content-Range: bytes 0-4/5

12345


//数据流

HTTP/1.1 206 Partial Content
Server: nginx/1.2.6
Date: Thu, 01 Nov 2018 09:50:02 GMT
Content-Type: application/octet-stream
Content-Length: 5
Last-Modified: Thu, 01 Nov 2018 07:58:15 GMT
Connection: keep-alive
Access-Control-Allow-Origin: http://wx.chargerlink.com
Content-Range: bytes 0-4/62576

猆\0

 **/ 
int HttpGetData(char* buf,uint16_t getLen)
{
	uint16_t gWifiRxLen = 0;
	uint8_t c;
	uint16_t tmo = 60;
	char *p1 = NULL;
	char *p2 = NULL;
	char *p3 = NULL;
    uint32_t ll = 0;
	while(tmo--)
	{
		osDelay(50);
		while((CL_OK == FIFO_S_Get(&wifiBlueFifo, &c)))
		{
			if(gWifiRxLen < WIFI_BLUE_BUF_LEN)
			{
				buf[gWifiRxLen++] = c;
			}else{
				break;
			}
		}
		
		p1 = strstr((const char*)buf, (const char*)"Content-Range: bytes");
		if(p1 != NULL)
		{
			//printf("buf=%s \r\n",buf);
			p2 = strstr((const char*)p1, (const char*)"bytes ");
			p3 = strstr((const char*)p2, (const char*)"\r\n\r\n");
			if(p3 != NULL)
			{
				ll = p3-buf+4;
				if(gWifiRxLen >= (ll+getLen)){
					memmove(buf,buf+ll,getLen);
					buf[getLen] = '\0';
					return getLen;
				}
			}
		}
	}
    
    return CL_FAIL;
}



int HttpGetServerData(char *recvBuf,uint32_t ReceiveLen, uint32_t NextLen, char* HttpIP, char* DirectoryPath)
{
	static uint8_t protoBuff[500] = {0,};
	int res = CL_FAIL;

	sprintf((char*)protoBuff,"GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nCache-Control: no-cache\r\nUser-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36\r\nAccept: */*\r\nAccept-Encoding: gzip,deflate\r\nAccept-Language: zh-CN,eb-US;q=0.8\r\nRange: bytes=%d-%d\r\n\r\n", DirectoryPath, HttpIP, ReceiveLen, NextLen);

	for(uint8_t i = 0; i < 3; i++)
	{
		FIFO_S_Flush(&wifiBlueFifo);
		BswDrv_UsartSend(WIFI_UART_INDEX, (void*)protoBuff, strlen((char*)protoBuff));

		if((HttpGetData(recvBuf,NextLen-ReceiveLen+1)) != CL_FAIL)
		{
			return CL_OK;
		}
	}
	
	return res;
}


#define HTTP_GET_DATA_LEN		256

int BswSrv_WIFI_HttpGet(char *httpurl)
{
	uint32_t getLen = 32;
	uint32_t StartAddr = 0;
	uint32_t EndAddr = 0;
	uint32_t cfize = 0;
	uint8_t  getCnt = 0;
	uint16_t checksum = 0;
	uint32_t UpgradeTicks = 0;
	FW_HEAD_STR *pFwHead = NULL;
	FW_INFO_STR *pFwInfo = NULL;
	char HttpIP[40] = {0};
	char FilePath[80] = {0};
	uint8_t upgradeType;
	uint8_t lastPercentage=0,percentage;
	char retry = 0;

	//解析url
	char* sp1 = strchr(httpurl, '/');
    strncpy(HttpIP,httpurl,sp1-httpurl);
	strcpy(FilePath,sp1);

    CL_LOG("HttpIP=%s, FilePath=%s.\n", HttpIP, FilePath);

	if(CL_OK != HttpSendCmd(HttpIP,80))
	{
		CL_LOG("后台请求升级失败! \n");
		return CL_FAIL;
	}

START:
	if(retry++ >= 3)
	{
		CL_LOG("upgrade fail,exit.\n");
		return CL_FAIL;
	}

	CL_LOG("后台请求升级, 开始升级! retry=%d\n",retry);

	StartAddr = 0;
	EndAddr = getLen + StartAddr - 1;
	UpgradeTicks = osGetTimes();

	while(1)
	{
		if((240*1000) < (uint32_t)(osGetTimes() - UpgradeTicks))
		{
			CL_LOG("http 升级超时  ! \r\n");
			goto START;
		}

		memset(&WifiRecvAtBuff,0,sizeof(WifiRecvAtBuff));
		if(CL_FAIL == HttpGetServerData((char*)WifiRecvAtBuff,StartAddr, EndAddr, HttpIP, FilePath))
		{
			CL_LOG("获取数据失败[1] ! StartAddr=%d  EndAddr=%d \r\n",StartAddr,EndAddr);
			continue;
		}

		if(getCnt == 0)
		{
			pFwHead = (void*)WifiRecvAtBuff;
			PrintfData("fw head=",WifiRecvAtBuff,32);
			if(pFwHead->aa == 0xAA && pFwHead->five == 0x55)
			{
				if(pFwHead->fwCnt == 0)
				{
					CL_LOG("fwCnt=%d,error.\n",pFwHead->fwCnt);
					return CL_FAIL;
				}
				pFwInfo = (void*)(WifiRecvAtBuff+16);
				if (0 == memcmp("U8M", pFwInfo->name, 3)) {//主板
					upgradeType = FW_U8;
				}
				else if (0 == memcmp("U8C", pFwInfo->name, 3)) {//刷卡板
					upgradeType = FW_U8_BAK;
				}
				else{
					CL_LOG("fw name=%s, error.\n",pFwInfo->name);
					goto START;
				}
				OAT_Init(upgradeType,0,pFwInfo->size,pFwInfo->checkSum,pFwHead->fwVer1);
				getCnt++;
			}
			else
			{
				CL_LOG("消息头校验失败 ! \r\n");
				goto START;
			}
		}
		else
		{
			for (int i=0; i<getLen; i++) {
				checksum += (unsigned)WifiRecvAtBuff[i];
			}
			cfize += getLen;

			//写数据
			OAT_FwWrite(upgradeType,WifiRecvAtBuff,getLen);

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

		StartAddr += getLen;

		if((cfize+HTTP_GET_DATA_LEN) <= upgradeInfo.fsize)
		{
			getLen = HTTP_GET_DATA_LEN;
		}
		else{
			getLen = upgradeInfo.fsize - cfize;
			CL_LOG("last getLen=%d \r\n",getLen);
		}

		EndAddr = getLen + StartAddr - 1;
	}

	if(cfize == upgradeInfo.fsize && checksum == upgradeInfo.checkSum){
		CL_LOG("ftp download finish .checksum = %X.\n",checksum);
		//固件认证
		if(OTA_Check(upgradeType) == CL_OK)
		{
			return upgradeType;
		}
		else
		{
			CL_LOG("OTA_Check failed .\r\n");
			goto START;
		}
		
	}
	CL_LOG("固件认证失败,退出升级..\r\n");
    return CL_FAIL;
}

///////////////////////////////////////////外部接口///////////////////////////////////////////////////

void BswSrv_WIFI_CloseSocket(void)
{
	CL_LOG("WIFI close socket.and reconnect.\r\n");

	WifiDisconnectServer(1000);

	GlobalInfo.is_socket_0_ok = 0;
    GlobalInfo.isLogin = 0;
	GlobalInfo.isRecvServerData = 0;
}

int BswSrv_WIFI_SendSocketData(uint8_t* data, uint16_t len)
{
	uint8_t i = 0;
    char cmd_req[32] = {0,};
    int res=0;

    if (1 != GlobalInfo.is_socket_0_ok) 
	{
       CL_LOG("socket 0 is closed.\n");
       return -1;
    }

    for(i = 0; i < 3; i++)
	{
		sprintf(cmd_req,"AT+CIPSEND=%d\r\n", len);
		#ifdef WG215_DBG
        CL_LOG("发送数据长度命令[%s].\n", cmd_req);
		#endif
        if (0 != (res = WifiSendCmd(cmd_req, "OK", 3000, 0)))
		{
            CL_LOG("call WifiSendCmd=%d, error.\n",res);
        }
        if (CL_OK == (res = BSPSendWifiData((char*)data, len, "SEND OK", 3000))) 
		{
            //CL_LOG("Wifi发送数据成功! \n");
            break;
        }
		else
		{
        	CL_LOG("Wifi发送数据错误! \n");
			tcp_tx_error_times++;
            osDelay(100);
        }
    }

    if (res == 0) 
	{
        tcp_tx_error_times = 0;
    }
	else 
	{
        CL_LOG("send fail: res=%d,tcp_tx_error_times=%d.\n",res,tcp_tx_error_times);
        if (tcp_tx_error_times >= TX_FAIL_MAX_CNT) 
		{
            tcp_tx_error_times = 0;
			CL_LOG("send fail,try to restart net.\n");
			BswSrv_WIFI_CloseSocket();
        }
    }

    return res;
}


int BswSrv_Bule_SendData(uint8_t* BlueData, uint8_t len)
{
	char  CmdBuff[64] = {0,};
	
    memset(CmdBuff, 0, sizeof(CmdBuff));
	sprintf(CmdBuff, "AT+BLEGATTSNTFY=0,1,2,%d\r\n", len);
    
	// CL_LOG("蓝牙发送数据指令[%s].\n", CmdBuff);
    
	if(0 == WifiSendCmd(CmdBuff, ">", 2000, 1))
	{
		;
	}

	BSPSendBlueData((char *)BlueData, len);
	
    return CL_FAIL;
}


int BswSrv_WG215_StartUp(void)
{
	uint8_t step = WIFI_AT_CMD;
	uint8_t retry = 0;

	WIFI_POWER_OFF();
	osDelay(2000);
	WIFI_POWER_ON();

	for(uint8_t i = 0; i < 5; i++)
	{
		BSPSendWifiData((char*)"+++", strlen("+++"), NULL, 0);
	}

	while(1)
	{
		switch (step)
		{
			case WIFI_AT_CMD:
				if(CL_OK == WifiBuleSendATCom(1000))
				{
					step = WIFI_AT_RST_CMD;
					retry = 0;
				}
				else
				{
					if(retry++ > 10)
					{
						return CL_FAIL;
					}
				}
			break;
			case WIFI_AT_RST_CMD:
				if(CL_OK == WifiBuleReset(1000))
				{
					step = WIFI_AT_ATE;
					retry = 0;
				}
				else
				{
					if(retry++ > 3)
					{
						return CL_FAIL;
					}
				}
				break;
			case WIFI_AT_ATE:
				if(CL_OK == WifiBlueSetATE(1000))
				{
					return CL_OK;
				}
				else
				{
					if(retry++ > 3)
					{
						return CL_FAIL;
					}
				}
				break;
		}
		osDelay(200);
	}
}

int BswSrv_WG215_StartBlue(void)
{
	uint8_t step = BLUE_SET_AT_BLEINIT_CMD;
	uint8_t retry = 0;

	while(1)
	{
		switch (step)
		{
			case BLUE_SET_AT_BLEINIT_CMD:
				if(CL_OK == BuleSetClient(1000, 2))
				{
					step = BLUE_AT_BLENAME_CMD;
					retry = 0;
				}else
				{
					if(retry ++ >5)
					{
						return CL_FAIL;
					}
				}
				break;
			case BLUE_AT_BLENAME_CMD:
				if(GlobalInfo.BlueName == NULL)
				{
					step = BLUE_AT_BLEADVPARAM_CMD;
					retry = 0;
				}
				else{
					if(CL_OK == BuleSetName(1000))
					{
						step = BLUE_AT_BLEADVPARAM_CMD;
						retry = 0;
					}else
					{
						if(retry ++ >5)
						{
							return CL_FAIL;
						}
					}
				}
				break;
			case BLUE_AT_BLEADVPARAM_CMD:
				if(CL_OK == BuleSetBLEADVPARAM(1000))
				{
					step = BLUE_AT_BLEADVDATA_CMD;
					retry = 0;
				}
				else
				{
					if(retry ++ >5)
					{
						return CL_FAIL;
					}
				}
				break;
			case BLUE_AT_BLEADVDATA_CMD:
				if(CL_OK == BuleSetBLEADVDATA(1000))
				{
					step = BLUE_AT_BLESCANRSPDATA_CMD;
					retry = 0;
				}
				else
				{
					if(retry ++ >5)
					{
						return CL_FAIL;
					}
				}
				break;
			case BLUE_AT_BLESCANRSPDATA_CMD:
				if(CL_OK == BuleScanRSPDATA(1000))
				{
					step = BLUE_AT_BLEGATTSSRVCRE_CMD;
					retry = 0;
				}
				else
				{
					if(retry ++ >5)
					{
						return CL_FAIL;
					}
				}
				break;
			case BLUE_AT_BLEGATTSSRVCRE_CMD:
				if(CL_OK == BuleCreatGATTSSRVCRE(1000))
				{
					step = BLUE_AT_BLEGATTSSRVSTART_CMD;
					retry = 0;
				}
				else
				{
					if(retry ++ >5)
					{
						return CL_FAIL;
					}
				}
				break;
			case BLUE_AT_BLEGATTSSRVSTART_CMD:
				if(CL_OK == BuleStartGATTSSRVCRE(1000))
				{
					step = BLUE_AT_BLEADVSTART_CMD;
					retry = 0;
				}
				else
				{
					if(retry ++ >5)
					{
						return CL_FAIL;
					}
				}
				break;
			case BLUE_AT_BLEADVSTART_CMD:
				if(CL_OK == BuleStartBroadCast(1000))
				{
					return CL_OK;
				}
				else
				{
					if(retry ++ >5)
					{
						return CL_FAIL;
					}
				}
				break;
		}
		osDelay(200);
	}
	
}


int BswSrv_WG215_StartWifi(void)
{
	uint8_t step = WIFI_SET_AT_CWMODE_CMD;
	uint8_t retry = 0;

	while(1)
    {
		switch(step)
		{
			case WIFI_SET_AT_CWMODE_CMD:
				if(CL_OK == WifiBuleSetStaApMode(2000))
				{
					step = WIFI_AT_CWJAP_CMD;
					retry = 0;
				}
				else
				{
					if(retry ++ >5)
					{
						return CL_FAIL;
					}
				}
			break;
			case WIFI_AT_CWJAP_CMD:
				if(CL_OK == WifiConnecteAp(8000))
				{
					step = WIFI_AT_CIPDOMAIN_CMD;
					retry = 0;
				}
				else
				{
					if(retry ++ >5)
					{
						return CL_FAIL;
					}
				}
			break;
			case WIFI_AT_CIPDOMAIN_CMD:
				if(CL_OK == WifiEnableDns(2000))
				{
					step = WIFI_SET_AT_CWDHCP_CMD;
					retry = 0;
				}
				else
				{
					if(retry ++ >5)
					{
						return CL_FAIL;
					}
				}
			break;
			case WIFI_SET_AT_CWDHCP_CMD:
				if(CL_OK == WifiEnableDHCP(2000))
				{
					step = WIFI_AT_CIPSTART_CMD;
					retry = 0;
				}
				else
				{
					if(retry ++ >5)
					{
						return CL_FAIL;
					}
				}
			break;
			case WIFI_AT_CIPSTART_CMD:
				if(CL_OK == WifiConnecteServer(5000,NET_SERVER_IP,NET_SERVER_PORT))
				{
					return CL_OK;
				}
				if(retry ++ >5)
				{
					return CL_FAIL;
				}
			break;
		}
	}
}

int BswSrv_Blue_Disconnent(void)
{
	for(uint8_t i = 0; i < 3; i++)
	{
		if(BuleStartBroadCast(1000) == CL_OK)
		{
			return CL_OK;
		}
	}
	
	return CL_FAIL;
}

void BswSrv_WG215_Init(void)
{
	WIFI_POWER_ON();
	
	//wifi底层接收数据fifo
	FIFO_S_Init(&wifiBlueFifo, (void*)wifiBlueFifiBuffer, sizeof(wifiBlueFifiBuffer));

	//蓝牙和wifi应用层数据接收fifo
    FIFO_S_Init(&rWifiFifo, (void*)rWifiFifoBuffer, sizeof(rWifiFifoBuffer));
    FIFO_S_Init(&rBlueFifo, (void*)rBlueFifoBuffer, sizeof(rBlueFifoBuffer));
}


