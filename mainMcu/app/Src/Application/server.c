
#include "server.h"
#include "BswSrv_System.h"
//#include "outNetProto.h"
#include "tfs.h"
//#include "ui.h"
//#include "emu.h"
#include "BswDrv_Rtc.h"
#include "BswDrv_Usart.h"
#include "GM510.h"
#include "BswDrv_sc8042b.h"
#include "BswDrv_Sys_Flash.h"
#include "BswSrv_FlashUsr.h"
//#include "lcd.h"
//#include "gun.h"
//#include "ntc.h"




const uint8_t gZeroArray[64] = {0};
CHG_INFO_STR gChgInfo;
uint8_t SendRecordTimers = 0;//订单发送次数
SemaphoreHandle_t gProtoSendMux;



//返回 0 是在线
int IsSysOnLine(void)
{
    if (SystemInfo.isRecvStartUpAck && SystemInfo.is_socket_0_ok) 
    {
        return CL_OK;
    }
    return CL_FAIL;
}

int restoi(char* buf, char* start, char end)
{
    char i;
    char *p = strstr(buf, start);

    p+=strlen(start);
    if(!p) 
    {
        return 0;
    }
    for(i=0; i<10; i++) 
    {
        if(p[i]==end) 
        {
            p[i]='\0';
            break;
        }
    }
    
    return atoi(p);
}

// 0x30 -> 0; a -> 0x0a; A -> 0x0a
uint8_t Val(uint8_t ch)
{
    uint8_t val = (uint8_t)-1;

    if ((ch >= '0') && (ch <='9')) {
        val = ch - '0';
        return val;
    }
    if ((ch >= 'A') && (ch <='F')) {
        val = ch - 'A' + 10;
        return val;
    }
    if ((ch >= 'a') && (ch <='f')) {
        val = ch - 'a' + 10;
        return val;
    }
    return val;
}


// des  src:  len:为 src的字符串长度
void DeviceBcd2str(char *des, unsigned char *src , int len)
{
    int i;
    unsigned char *p;
    char tmp[4];

    p = src;
    for(i=0; i<len; i++) {
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%02x", *p++);
        strcat(des, tmp);
    }
}


// 0x12  <= {0x31,0x32}   0x1210 <= {0x31,0x32,0x31} 0x1210 <= {0x31,0x32,0x31,0x30}
// srcLen : pbSrc的长度
int StrToHex(uint8_t *pbDest, const char *pbSrc, int srcLen)
{
    int32_t i=0,j=0;
    uint8_t chl,chh;

    memset(pbDest, 0, srcLen/2);
    while (i < srcLen) {
        if ((i+1) == srcLen) {  //最后一个,是奇数
            chl = Val(pbSrc[i]);
            pbDest[j] = chl<<4;
            j ++;
        }else{
            chh = Val(pbSrc[i]);
            chl = Val(pbSrc[i+1]);
            pbDest[j] = (chh << 4) | chl;
            j ++;
        }
        i = i+2;
    }
    return j;
}

void SycTimeCount(uint32_t second)
{
//    gOutNetStatus.lastRecvTime = second;
    gChgInfo.lastRecvKbMsgTime = second;
    gChgInfo.turnOnLcdTime = second;
}


void ResetSysTem(void)
{
    Sc8042bSpeech(VOIC_DEVICE_REBOOT);
//	OperateMaintain(OptMainTainResetSys, 0);
    vTaskDelay(1000);
    NVIC_SystemReset();
}


int CheckChargerSn(uint8_t *pChargerSn, uint8_t len)
{
    int i;

    for (i=0; i<len; i++) {
        if (9 < (pChargerSn[i] & 0x0f)) {
            return CL_FAIL;
        }
        if (9 < (pChargerSn[i] >> 4)) {
            return CL_FAIL;
        }
    }
    return CL_OK;
}

void PrintSysCfgInfo(void)
{

}


void ServerTask(void)
{
	uint8_t  first = 1;
    uint32_t time;
    uint8_t  pktBuff[SERVER_PKT_MAX_LEN];
    uint8_t  data;
    uint8_t  step;
    uint8_t  len;
    uint8_t  pktLen;
    uint8_t  length;
    uint8_t  sum;
//    uint16_t overFlow = 0;

	gProtoSendMux = xSemaphoreCreateMutex();//信号量互斥锁
    SycTimeCount(GetRtcCount());
    gChgInfo.lastBlueStatus = 2;
	GM510Init();

    while (1) 
	{
        SystemInfo.is_socket_0_ok = CL_FALSE;
	//	OpenNetDevice();
        gChgInfo.errCode = 0;
        gSendHearBeatCnt = 0;
        SystemInfo.tcp_tx_error_times = 0;
		if (1 == first) 
		{
            OS_DELAY_MS(1000);
            first = 0;
        }
		else
		{ //离线后的网络恢复，随机等待一定时间(2分钟之内)再重新发送数据到后台，避免后台故障恢复后桩端同时密集发送数据到后台，以缓解后台消息处理压力
            step = gChgInfo.second & 0x7f;
            CL_LOG("wait %ds\n", step);
            OS_DELAY_MS(step * 1000);
        }
        SystemInfo.is_socket_0_ok = CL_TRUE;
        step = FIND_AA;
        while (1) 
		{
            OS_DELAY_MS (60);

        //    if (overFlow != GetUsartOverFlow(PHY_UART_GPRS_PORT)) 
		//	{
        //        CL_LOG("gprs fifo of.\n");
        //        overFlow = GetUsartOverFlow(PHY_UART_GPRS_PORT);
        //    }
            if (SystemInfo.is_socket_0_ok == CL_FALSE) 
			{
                CL_LOG("net break.\n");
                break;
            }
			#if 0
            //本来是本地网络的，如果检测到485网络，就切换到485网络
            if ((LOCAL_NET == SystemInfo.netType) && (OUT_485_NET == gOutNetStatus.connect)) 
			{
                break;
            }
			#endif
            if (FIND_AA != step) 
            {
                if (5 < (uint32_t)(GetRtcCount() - time)) 
                {
                    CL_LOG("long time no recv data,step=%d,error.\n",step);
                    step = FIND_AA;
                }
            }
            while (CL_OK == FIFO_S_Get(&gSocketPktRxCtrl, &data)) 
			{
                //printf("%02x ",data);
                switch (step) 
				{
                    case FIND_AA:
                        if (data == 0xAA) 
						{
                            time = GetRtcCount();
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
                            if (length >= (SERVER_PKT_MAX_LEN-8)) 
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
                            step = FIND_AA;
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
                            RecvServerData((void*)pktBuff, pktLen);
                        }
						else
						{
                            CL_LOG("recv error,sum=%#x,pkt sum=%#x.\n",sum,data);
                        }
                        step = FIND_AA;
                        break;

                    default:
                        step = FIND_AA;
					break;
                }
            }
			
        }
    }
}


