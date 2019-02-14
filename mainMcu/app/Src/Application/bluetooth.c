
#include "bluetooth.h"
#include "blue_proto.h"
#include "BswDrv_Usart.h"
#include "BswDrv_Debug.h"
#include "includes.h"


BLUETOOTH_CTRL_STR BluetoothCtrl;
uint8_t gBlueRxData[CKB_MAX_PKT_LEN];
uint8_t BluetoothRxbuffer[CKB_MAX_PKT_LEN];
uint8_t gCK24RxData[CKB_MAX_PKT_LEN];

#ifdef X6
	uint8_t g24GRouteRxData[CKB_MAX_PKT_LEN];
	uint8_t g24GRxBuff[CKB_MAX_PKT_LEN];
#endif

void BluetoothFifoInit(void)
{
    #ifdef X6
		FIFO_S_Init(&BluetoothCtrl.rx24GBuff, (void*)g24GRxBuff, sizeof(g24GRxBuff));
	#endif
	FIFO_S_Init(&BluetoothCtrl.rxBtBuff, (void*)BluetoothRxbuffer, sizeof(BluetoothRxbuffer));
	BluetoothCtrl.bluetoothRxSemaphore = xSemaphoreCreateMutex();
}



//蓝牙检测
int BlueCheck(void)
{
	if(SystemInfo.blue_connectstate == 0)
    {
	//	return BlueTest(3);
	}
	return CL_OK;
}

void CKBRecvProc(void)
{
	uint8_t  *pktBuff = (void*)gCK24RxData;
    static uint8_t  step = BLUE_AB;
    uint8_t  data;
    static uint8_t  sum;
    static uint16_t pktLen;
    static uint16_t len;
	static uint8_t  target;
	static uint16_t msgLen;

	while (CL_OK == BswDrv_UsartGetOneData(BLUETOOTH_PORT, &data)) {
        //printf("%02x",data);
		switch (step) {
			case BLUE_AB:
				if (data == 0xAB) {
					step = BLUE_CD;
					pktBuff[0] = 0xAB;
					pktLen = 1;
					sum = 0xAB;
				}
				break;

			case BLUE_CD:
				if (data == 0xcd) {
					step = BLUE_TARGET;
					pktBuff[1] = 0xcd;
					pktLen++;
					sum += data;
				}else if (data == 0xAB) {
					step = BLUE_CD;
					pktBuff[0] = 0xAB;
					pktLen = 1;
					sum = 0xAB;
					CL_LOG("can not find cd.\n");
				}else{
					step = BLUE_AB;
					CL_LOG("can not find cd.\n");
				}
				break;

			case BLUE_TARGET:
				pktBuff[pktLen++] = data;
				len = 0;
				if(data == NODE_BLUE)
					step = BLUE_LEN;
				else
					step = BLUE_ADDR;
				sum += data;
				target = data;
				break;

			case BLUE_ADDR:
				pktBuff[pktLen++] = data;
				sum += data;
				if (6 == ++len) {
					len = 0;
					step = BLUE_LEN;
				}
				break;

			case BLUE_LEN:
				pktBuff[pktLen++] = data;
				sum += data;
				if (2 == ++len) {
					len = (pktBuff[pktLen-1]<<8) | pktBuff[pktLen-2];
					msgLen = len;
					if ((255) < len) {
						step = BLUE_AB;
						CL_LOG("len=%d,error.\n", len);
					}else if(len == 0){
						step = BLUE_CHK;
					}else{
						step = BLUE_RX_DATA;
					}
				}
				break;
			case BLUE_RX_DATA:
				pktBuff[pktLen++] = data;
				sum += data;
				if (0 == --len) {
					step = BLUE_CHK;
				}
				break;

			case BLUE_CHK:
				pktBuff[pktLen++] = data;
				if (data == sum) {
					step = BLUE_END;
					len = 0;
				}else{
					CL_LOG("recv data checksum error,sum=%#x,pkt sum=%#x.\n",sum,data);
					step = BLUE_AB;
				}
				break;

			case BLUE_END:
				if (2 == ++len) {
                    //PrintfData("CKBRecvProc",pktBuff,pktLen);
					uint8_t *p = NULL;
					if (NODE_24G == target)
                    {
						#ifdef X6
						p = pktBuff+sizeof(BLUE_HEAD_STR);
						for (len=0; len<msgLen; ) {
							if (CL_OK == FIFO_S_Put(&gCKBCtrl.rx24GBuff, p[len])) {
                                len++;
                            }else{
                                CL_LOG("2.4g buff over flow error.\n");
                                vTaskDelay(2);
                            }
						}
						#endif
					}
                    else
                    {
						p = pktBuff+sizeof(BLUE_RX_HEAD_STR);
						for (len=0; len<msgLen; ) {
							if (CL_OK == FIFO_S_Put(&BluetoothCtrl.rxBtBuff, p[len])) {
								len++;
							}else{
								CL_LOG("bt buff over flow error.\n");
								vTaskDelay(2);
							}
						}
					}
					step = BLUE_AB;
				}
				break;

			default:
				step = BLUE_AB;
				break;
		}
	}
}


//手机蓝牙数据接收处理
void RecvBtData(void)
{
    uint8_t  *pBuff = gBlueRxData;
    static uint16_t len;
    static uint16_t pktLen;
    uint8_t  data;
    static uint8_t  step = BT_FIND_EE;
    static uint8_t  sum;
    static uint32_t time;

    if (BT_FIND_EE != step) {
        if (2 < (uint32_t)(GetRtcCount() - time)) {
            CL_LOG("too long no recv data,step=%d,error.\n",step);
            step = BT_FIND_EE;
        }
    }
    while (CL_OK == FIFO_S_Get(&BluetoothCtrl.rxBtBuff, &data)) {
        switch (step) {
            case BT_FIND_EE:
                if (0xee == data) {
                    time = GetRtcCount();
                    pktLen = 0;
                    pBuff[pktLen++] = data;
                    step = BT_FIND_CMD;
                    sum = 0xee;
                }
                break;

            case BT_FIND_CMD:
                pBuff[pktLen++] = data;
                sum += data;
                step = BT_FIND_LEN;
                break;

            case BT_FIND_LEN:
                pBuff[pktLen++] = data;
                sum += data;
                len = data;
                if (OUT_NET_PKT_LEN < len) {
                    step = BT_FIND_EE;
                }else if (0 == len) {
                    step = FIND_CHK;
                }else{
                    step = BT_RX_DATA;
                }
                break;

            case BT_RX_DATA:
                pBuff[pktLen++] = data;
                sum += data;
                if (0 == --len) {
                    step = FIND_CHK;
                }
                break;

           case FIND_CHK:
                pBuff[pktLen++] = data;
                if (sum == data) {
                    //PrintfData("RecvBtData", pBuff, pktLen);
					BlueRecvProc((void*)pBuff, pktLen);
                }else{
					PrintfData("RecvBtData", pBuff, pktLen);
                    CL_LOG("sum=%02x,pkt sum=%02x,error,drop pkt.\n",sum,data);
                }
                step = BT_FIND_EE;
                break;
        }
    }
}


int BlueCheckRes(char *cmd, char *res, uint16_t tmo, uint8_t *pbuff, uint16_t size)
{
    char *ret=NULL;
    uint8_t c;
    int cnt = 0;
    int retv = CL_FAIL;
    uint16_t time = 0;
    char  temp[20];
	uint32_t BlueTimeTicksFlag = GetRtcCount();
	
    memset(pbuff, 0, size);

    for(time = 0; time < tmo; time+= 50) 
	{
        vTaskDelay(50);
		BlueTimeTicksFlag = GetRtcCount();
		while(((BlueTimeTicksFlag + 10) >= GetRtcCount()) && (BlueTimeTicksFlag <= GetRtcCount()))
		{
			while(CL_OK == FIFO_S_Get(&(UartPortHandleInfo->rxBuffCtrl), &c)) 
			{
				BlueTimeTicksFlag = GetRtcCount();
			//	printf("c:[%x, %c].\n", c, c);
	            if (c) 
				{
	                if (size > cnt) 
					{
	                    pbuff[cnt++] = c;
	                }
					else
					{
	                    //CL_LOG("cnt=%d,error.\n",cnt);
	                }
	            }
	        }
		}
        
        ret = strstr((const char*)pbuff, (const char*)res);
        if (ret) 
		{
			//获取版本号
			if(strstr(cmd,"AT+VERS?"))
			{
				char  temp[21];
				memset(temp,0,21);
				sscanf((char*)pbuff,"+VERS: %s",temp);
				if(strlen(temp) >= 20)
				{
				//	memcpy(SystemInfo.blue_version, temp,20);
				}
				else
				{
				//	strcpy((char*)SystemInfo.blue_version,temp);
				}				
				//CL_LOG("blue ver:%s.\n", SystemInfo.blue_version);
			}
			//获取蓝牙名称
			if(strstr(cmd,"AT+NAME?"))
			{
				memset(temp,0,20);
				sscanf((char*)pbuff,"+NAME: %s",temp);
				if(strlen(temp) >= 12)
				{
				//	memcpy(SystemInfo.blue_name, temp,12);
				}
				else
				{
				//	strcpy(SystemInfo.blue_name, temp);
				}	
				//CL_LOG("blue name:%s.\n", SystemInfo.blue_name);
			}
			
			//获取网关mac
			if(strstr(cmd,"AT+GWID?"))
			{
				char  temp[20];
				memset(temp,0,20);
				sscanf((char*)pbuff,"+GWID: %s",temp);
			//	StringToBCD(gwMac,temp);
			//	CL_LOG("gwmac:%s.\n",temp);
			}
			//查询是否终端设备
			if(strstr(cmd,"AT+DEVT?"))
			{
			//	sscanf((char*)pbuff,"+DEVT: %d",&isDevt);
			}
            retv = CL_OK;
            break;
        }
    }

    return retv;
}

int BlueSendCmd(char *cmd, char *ack, uint16_t waittime)
{
	uint8_t gBlueRecvData[128];
	uint8_t res = 1;

    CKB24_CD_LOW();
	FeedWatchDog();
    vTaskDelay(10);
    //UsartFlush(BLUE_TOOTH);
    CL_LOG("发送命令[%s].\n", cmd);
	BswDrv_UsartSend(DEBUG_INDEX,(void*)cmd, strlen(cmd));
    if ((ack==NULL) || (waittime==0)) 
	{
        CKB24_CD_HIGH();
        return CL_OK;
    }

    if (CL_OK == BlueCheckRes(cmd, ack, waittime, gBlueRecvData, sizeof(gBlueRecvData))) 
	{
        res = CL_OK; /*check success, retrun 0*/
    }
    CKB24_CD_HIGH();
	return res;
}

int BuleCheckResetProcess(void)
{
    uint8_t i = 0;
    
//    CKB24_UM_HardReset();
	
//  SystemInfo.blue_state = 1;
//    SystemStatus.blue_state = 1;
	
    for (i = 0; i < 2; i++) 
	{
        if (BlueSendCmd("AT\r\n", "OK", 3000) == 0) 
		{
            CL_LOG("blue test ok.\n");
            return CL_OK;
        }
        CL_LOG("blue reset fail,retry.\n");
		FeedWatchDog();
        vTaskDelay(2000);
    }
    CL_LOG("blue reset fail.\n");
    return CL_FAIL;
}

int BlueTest(int retry)
{
    uint8_t i = 0;
    
    for (i = 0; i < retry; i++) 
	{
        if (BlueSendCmd("AT\r\n", "OK", 3000) == 0) 
		{
        //    CL_LOG("AT命令检测成功.\n");
			
            return CL_OK;
        }
        if(retry > 1)
		{
            FeedWatchDog();
            vTaskDelay(1000);
        }
    }
	
    return ERROR;
}

int BuleModeEchoProcess(void)
{
    uint8_t i = 0;
    
    for (i = 0; i < 2; i++) 
	{
        if (BlueSendCmd("AT+ECHO=1\r\n", "OK", 3000) == 0) 
		{
            CL_LOG("回显 ok.\n");
            return CL_OK;
        }
        CL_LOG("蓝牙回显失败,retry.\n");
        vTaskDelay(2000);
    }
    CL_LOG("回显失败.\n");
    return CL_FAIL;
}

int BuleModeVerProcess(void)
{
    uint8_t i = 0;
    
    for (i = 0; i < 2; i++) 
	{
        if (BlueSendCmd("AT+VERS?\r\n", "OK", 3000) == 0) 
		{
            CL_LOG("获取版本 ok.\n");
            return CL_OK;
        }
        CL_LOG("获取版本失败,retry.\n");
        vTaskDelay(2000);
    }
    CL_LOG("获取版本失败.\n");
    return CL_FAIL;
}

int BuleGetNameProcess(void)
{
    uint8_t i = 0;
    
    for (i = 0; i < 2; i++) 
	{
        if (BlueSendCmd("AT+NAME?\r\n", "OK", 3000) == 0) 
		{
            CL_LOG("获取名字 ok.\n");
            return CL_OK;
        }
        CL_LOG("获取名字失败,retry.\n");
        vTaskDelay(2000);
    }
    CL_LOG("获取名字失败.\n");
	
    return CL_FAIL;
}

int BuleModePairProcess(void)
{
    uint8_t i = 0;
    
    for (i = 0; i < 2; i++) 
	{
        if (BlueSendCmd("AT+PAIR=1\r\n", "OK", 3000) == 0) 
		{
            CL_LOG("蓝牙匹配成功.\n");
            return CL_OK;
        }
        CL_LOG("蓝牙匹配失败,retry.\n");
        vTaskDelay(2000);
    }
    CL_LOG("蓝牙匹配失败.\n");
	
    return CL_FAIL;
}

int BuleReconnect(void)
{
	uint8_t step = BLUE_RESET;
	uint8_t retry = 0;
	uint32_t BuleReconnectTick = GetRtcCount();
		
	while((BuleReconnectTick + BLUE_CONNECT_TICK) >= GetRtcCount())
	{
		switch (step)
		{
			case BLUE_RESET:
				if(CL_OK == BuleCheckResetProcess())
				{
					step = BLUE_MODE_ECHO;
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
			case BLUE_MODE_ECHO:
				if(CL_OK == BuleModeEchoProcess())
				{
					step = BLUE_MODE_VER;
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
			case BLUE_MODE_VER:
				if(CL_OK == BuleModeVerProcess())
				{
					step = BLUE_MODE_GETNAME;
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
			case BLUE_MODE_GETNAME:
				if(CL_OK == BuleGetNameProcess())
				{
					#if SET_BLUE_NAME
					step = BLUE_MODE_SETNAME;
					#else
					step = BLUE_MODE_PAIR;
					#endif
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
		#if SET_BLUE_NAME
			#if 0
			case BLUE_MODE_SETNAME:
				if(CL_OK == SetBlueName())
				{
					step = BLUE_MODE_PAIR;
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
			#endif
		#endif
			case BLUE_MODE_PAIR:
				if(CL_OK == BuleModePairProcess())
				{
				//	SystemInfo.blue_state = 0;
    			//	SystemStatus.blue_state = 0;
					step = BLUE_RESET;
					retry = 0;
					CL_LOG("蓝牙初始化成功.\n");
					return CL_OK;
				}
				else
				{
					if(retry++ > 10)
					{
						return CL_FAIL;
					}
				}
			break;
			default:
				CL_LOG("错误.\n");
			//	step = BLUE_RESET;
			break;
		}
		vTaskDelay(200);
	}
    return CL_FAIL;
}

void BluetoothTask(void)
{
    uint32_t old;
    uint32_t second;
  //  CKB24_UM_Init();
	BluetoothFifoInit();

    while(1)
    {
        BuleReconnect();
        //SendEventNotice(0, EVENT_CHIP_FAULT, CHIP_BLUE, 0, EVENT_OCCUR, NULL);
        while(1)
        {
            vTaskDelay(100);

            #if 0
            if((AgeTestInfo.testFlag == AGE_TEST_FLAG) && (AgeTestInfo.testMode == 1))  //老化测试
            {
                CL_LOG("testMode=%d.\n",AgeTestInfo.testMode);
                AgeTestInfo.item[AGE_TEST_BLUE].successCnt++;
                AgeTestInfo.testCnt++;
                CL_LOG("蓝牙老化测试成功,成功次数:%d,失败次数:%d\n",
                        AgeTestInfo.item[AGE_TEST_BLUE].successCnt,AgeTestInfo.item[AGE_TEST_BLUE].failCnt);
                break;
            }
            #endif
            CKBRecvProc();
            RecvBtData();

            if (GetRtcCount() != old)
            {
                old = GetRtcCount();

                if((second %(30)) == 0) {
                //    if(BlueCheck() != CL_OK)
                //    {
                //        system_info.ckb24_state = 0;
                //        CL_LOG("ckb24 check failed.\n");
                //        break;
                //    }
                //    else
                //    {
                //        CL_LOG("blue check ok.\n");
                //    }
//                    if (CL_OK == IsSysOnLine()) 
//                    {
//                        if (gChgInfo.sendBlueStatus < 10) 
//                        {
//                         //   SendEventNotice(0, EVENT_CHIP_FAULT, CHIP_BLUE, 0, EVENT_OCCUR, NULL);
//                            gChgInfo.sendBlueStatus++;
//                        }
//                    }
                }
                second ++;
            }
        }
    }
}



