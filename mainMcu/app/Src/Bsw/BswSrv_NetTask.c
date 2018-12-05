/*
 * @Author: zhoumin 
 * @Date: 2018-10-12 18:16:39 
 * @def :网络管理任务
 * @Last Modified by: zhoumin
 * @Last Modified time: 2018-11-02 18:43:26
 */
#include "includes.h"
#include "BswDrv_Usart.h"
#include "BswSrv_NetTask.h"
#include "BswDrv_Debug.h"
#include "BswSrv_WG215.h"
#include "BswSrv_Air720.h"
#include "BswSrv_System.h"
#include "BswSrv_FwUpgrade.h"

//网络连接顺序
uint8_t netStep = 1;    //1-wifi  2-4G

/*网络数据接收处理buf*/
uint8_t netBuffer[256];

/*wifi应用层接收fifo*/
extern FIFO_S_t rWifiFifo;
/*4G模块接收数据fifo*/
extern FIFO_S_t gGsmRecvFifo;

/*蓝牙和wifi初始化标志*/
extern uint8_t wifiInitFlag;    ////0--没有初始化  1--正在被初始化


static void RecvNetData(uint8_t netType);

/**
 * TCP网络数据回调函数定义 
 */
__weak 
void BswSrv_Net_RxData_Callback(uint8_t netType,uint8_t *data,uint16_t len)
{
	//northing
	UNUSED(netType);
	UNUSED(data);
	UNUSED(len);
}



void RecvNetData(uint8_t netType)
{
    FIFO_S_t* fifo;
    uint8_t  *pktBuff = &netBuffer[0];
    uint8_t  data;
    static uint8_t  step = FIND_AA;
    static uint8_t  len;
    static uint8_t  pktLen;
    static uint8_t  length;
    static uint8_t  sum;

	if(netType == NETTYPE_GPRS){
        fifo = &gGsmRecvFifo;
	}
	else if(netType == NETTYPE_WIFI){
		fifo = &rWifiFifo;
	}
	else{
		return ;
	}

    while (CL_OK == FIFO_S_Get(fifo,&data))
	{
        //printf("c=%X %c\r\n",data,data);
        switch (step) 
        {
            case FIND_AA:
                if (data == 0xAA) 
				{
                    step = FIND_55;
                    pktBuff[0] = 0xAA;
                    pktLen = 1;
                }
                break;

            case FIND_55:
                if (data == 0x55) 
				{
                    step = FIND_CHARGER_TYPE;
                    pktBuff[1] = 0x55;
                    pktLen++;
                }
				else if (data == 0xAA) 
				{
                    step = FIND_55;
                    pktBuff[0] = 0xAA;
                    CL_LOG("can not find 55.\n");
                }
				else
				{
                    step = FIND_AA;
                    CL_LOG("can not find 55.\n");
                }
                break;

            case FIND_CHARGER_TYPE:
                pktBuff[pktLen++] = data;
                len = 0;
                step = FIND_CHAGER_SN;
                break;
            case FIND_CHAGER_SN:
                pktBuff[pktLen++] = data;
                if (CHARGER_SN_LEN == ++len) 
				{
                    len = 0;
                    step = FIND_LEN;
                }
                break;
            case FIND_LEN:
                pktBuff[pktLen++] = data;
                if (2 == ++len) 
				{
                    length = (pktBuff[pktLen-1]<<8) | pktBuff[pktLen-2];
                    if (length >= (sizeof(netBuffer)-8)) 
					{
                        CL_LOG("length=%d,error.\n",length);
                        step = FIND_AA;
                    }
					else
					{
                        sum = 0;
                        step = FIND_VER;
                    }
                }
                break;
            case FIND_VER:
                pktBuff[pktLen++] = data;
                len = 0;
                sum += data;
                step = FIND_SERNUM;
                break;

            case FIND_SERNUM:
                pktBuff[pktLen++] = data;
                sum += data;
                if (2 == ++len) 
				{
                    step = FIND_CMD;
                }
                break;
            case FIND_CMD:
                pktBuff[pktLen++] = data;
                sum += data;
                if (4 > length) 
				{
                    CL_LOG("length=%d,error.\n",length);
                    if (FIND_AA == data) 
					{
                        step = FIND_55;
                        pktBuff[0] = 0xAA;
                        pktLen = 1;
                    }
					else
					{
                        step = FIND_AA;
                    }
                }
				else
				{
                    len = length - 4;
                    step = len ? RX_DATA : FIND_CHK;
                }
                break;
            case RX_DATA:
                pktBuff[pktLen++] = data;
                sum += data;
                if (1 == --len) 
				{
                    step = FIND_CHK;
                }
                break;
            case FIND_CHK:
                pktBuff[pktLen++] = data;
                if (data == sum)
				{
					BswSrv_Net_RxData_Callback(netType,pktBuff, pktLen);
                }
				else
				{
                    CL_LOG("recv data checksum error,sum=%#x,pkt sum=%#x.\n",sum,data);
                }
                step = FIND_AA;
                break;
            default:
                step = FIND_AA;
                break;
        }
    }
}


void BswSrv_GetSignal(void)
{
    if(GlobalInfo.netType == NETTYPE_GPRS)
    {
        BswSrv_Air720_GetSignal(1);
    }
}


int BswSrv_CheckSocketState(void)
{
    int state;
    if(GlobalInfo.netType == NETTYPE_GPRS)
    {
        if((state = BswSrv_Air720_GetSocketState(1)) != CL_FAIL)
        {
            if(state != CONNECT_OK)
            {
                return CL_FAIL;
            }
        }
    }
    else if(GlobalInfo.netType == NETTYPE_WIFI)
    {
        //todo 获取WiFi socket连接状态
        
    }

    return CL_OK;
}

void BswSrv_CloseSocket(void)
{
    if(GlobalInfo.netType == NETTYPE_GPRS)
    {
        BswSrv_Air720_CloseSocket();
    }
    else if(GlobalInfo.netType == NETTYPE_WIFI)
    {
        BswSrv_WIFI_CloseSocket();
    }
}


int BswSrv_SendSokcetData(uint8_t *data,uint16_t len)
{
    // PrintfData("send socket data:",data,len);
    if(GlobalInfo.netType == NETTYPE_GPRS)
    {
        BswSrv_Air720_SendData(0,data,len);
    }
    else if(GlobalInfo.netType == NETTYPE_WIFI)
    {
        BswSrv_WIFI_SendSocketData(data,len);
    }
    else
    {
        return CL_FAIL;
    }

    return CL_OK;
}


//记录wifi模块启动时间
uint32_t startUpWifiTime = 0;

/**
 *关闭上网标记，重新启动网络
 */  
void BswSrv_CloseNetWork(void)
{
    GlobalInfo.is_socket_0_ok = 0;
    GlobalInfo.BlueInitOk = 0;
    GlobalInfo.WG215InitOK = 0;
    GlobalInfo.GprsInitOK = 0;
    startUpWifiTime = osGetTimes();
}

/**
 *上网任务 
 */
void SurfNet_Task(void)
{
	uint32_t signalTick = 0;
    uint32_t socketTick = 0;
    uint8_t retry = 0;

    //4G模块检测
    if(BswSrv_Air720_StartUp() == CL_OK)
    {
        GlobalInfo.GprsInitOK = 1;
        CL_LOG("4G模块检测OK..\r\n");
    }
    else
    {
        GlobalInfo.GprsInitOK = 0;
        CL_LOG("4G模块检测失败..\r\n");
    }

    startUpWifiTime = osGetTimes();

	while(1)
	{
        if(netStep == 1)//当前使用wifi连接网络
        {
            if(GlobalInfo.is_socket_0_ok == 0 && GlobalInfo.WG215InitOK)
            {
                if(wifiInitFlag == 0)
                {
                    wifiInitFlag = 1;
                    CL_LOG("正在使用wifi连接网络..retry=%d \r\n",retry);
                    if(BswSrv_WG215_StartWifi() == CL_OK)
                    {
                        GlobalInfo.is_socket_0_ok = 1;
                        GlobalInfo.netType = NETTYPE_WIFI;
                        retry = 0;
                        wifiInitFlag = 0;
                        CL_LOG("WIFI网络连接OK..\r\n");
                    }
                    else
                    {
                        if(retry++ >= 2)
                        {
                            retry = 0;
                            netStep = 2;
                            wifiInitFlag = 0;
                            CL_LOG("WIFI连接网络失败，尝试使用4G连接网络.\r\n");
                        }
                    }
                }
            }
            else if(GlobalInfo.WG215InitOK == 0)
            {
                //等待wifi模块开机 等待60s
                if(osGetTimes() - startUpWifiTime >= (60*1000))
                {
                    retry = 0;
                    netStep = 2;
                    CL_LOG("检测WIFI模块启动失败，尝试使用4G连接网络.\r\n");
                }
            }
        }
        else if(netStep == 2)//当前使用4g连接网络
        {
            if(GlobalInfo.GprsInitOK == 0)
            {
                //4G模块检测
                if(BswSrv_Air720_StartUp() == CL_OK)
                {
                    GlobalInfo.GprsInitOK = 1;
                    retry = 0;
                    CL_LOG("4G模块检测OK..\r\n");
                }
                else
                {
                    GlobalInfo.GprsInitOK = 0;
                    CL_LOG("4G模块检测失败..\r\n");

                    if(retry++ >= 2)
                    {
                        retry = 0;
                        netStep = 1;
                        startUpWifiTime = osGetTimes();
                        CL_LOG("4G检测失败，尝试使用WIFI连接网络.\r\n");
                    }
                }
            }
            else
            {
                CL_LOG("正在尝试使用4G连接网络.retry = %d \r\n",retry);
                if(BswSrv_Air720_Reconnect(GSM_TEST) == CL_OK)
                {
                    GlobalInfo.is_socket_0_ok = 1;
                    GlobalInfo.netType = NETTYPE_GPRS;
                    retry = 0;
                    CL_LOG("4G网络连接OK..\r\n");
                }
                else
                {
                    if(retry++ >= 2)
                    {
                        retry = 0;
                        netStep = 1;
                        startUpWifiTime = osGetTimes();
                        CL_LOG("4G连接网络失败，尝试使用WIFI连接网络.\r\n");
                    }
                }
            }
        }

        
        //开始接收服务器数据
        GlobalInfo.isRecvServerData = 1;

		/*网络链路维护*/
		while(GlobalInfo.is_socket_0_ok)
		{
			//定时获取信号值 1分钟获取一次
            if(osGetTimes() - signalTick >= 60000){
                signalTick = osGetTimes();
                BswSrv_GetSignal();
            }
            
            //定时获取TCP连接状态 10s获取一次
            if(osGetTimes() - socketTick >= 10000)
            {
                socketTick = osGetTimes();
                if(BswSrv_CheckSocketState() != CL_OK)
                {
                    CL_LOG("检测到TCP连接断开.断开socket \r\n");
                    BswSrv_CloseSocket();
                }
            }

			//接收TCP数据
			RecvNetData(GlobalInfo.netType);

			osDelay(50);
		}
        osDelay(1000);
	}
}
