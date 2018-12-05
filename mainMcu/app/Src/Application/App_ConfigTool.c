#include "includes.h"
#include "BswSrv_System.h"
#include "App_ConfigTool.h"
#include "BswDrv_sc8042b.h"
#include "BswDrv_Debug.h"
#include "BswDrv_Rtc.h"

static uint8_t ct_sn = 0;

static void App_CT_FrameHandle(uint8_t *data,uint16_t len);

void BswSrv_CT_RxData_Callback(uint8_t *data,uint16_t len)
{
	App_CT_FrameHandle(data,len);
}



int App_CT_SendDebugPkt(uint8_t nodeIndex, OUT_PKT_STR *pFrame, uint16_t len)
{
    pFrame->head.aa = 0xaa;
    pFrame->head.five = 0x55;
    pFrame->head.dest[0] = nodeIndex;
    pFrame->head.len = 4 + len;
    pFrame->head.ver = 1;
	pFrame->head.sn = ct_sn++;
    pFrame->data[len] = BswSrv_Tool_CheckSum((void*)&pFrame->head.ver, len + 3);

	BswSrv_ConfigTool_SendData((void*)pFrame, sizeof(OUT_NET_HEAD_STR) + len + 1);

	PrintfData("SendDebugPkt", (void*)pFrame, sizeof(OUT_NET_HEAD_STR) + len + 1);	
	return CL_OK;
}



void App_CT_FrameHandle(uint8_t *data,uint16_t len)
{
    uint16_t dataLen = 1;
    OUT_PKT_STR *pBuff = (void*)data;

	if(pBuff->head.cmd != DEBUG_CMD_FW_DOWNLOAD){
		PrintfData("DebugRecvProc", (void*)pBuff, len);
	}

    switch (pBuff->head.cmd) {
		case DEBUG_CMD_TEST:	//请求测试
			pBuff->data[1] = 0;
			dataLen = 2;
			break;
		case DEBUG_CMD_SIM:	//2G测试
		
			break;
		case DEBUG_CMD_TRUMPTE://喇叭测试
			CL_LOG("DEBUG_CMD_TRUMPTE \n");
			Sc8042bSpeech(VOIC_WELCOME);
			pBuff->data[0] = 0;
			break;
		case DEBUG_CMD_WRITEPCB:	//写PCB编号
            
			break;
		case DEBUG_CMD_READPCB:		//读PCB编号
			CL_LOG("DEBUG_CMD_READPCB data[0]=%d \n",pBuff->data[0]);
			if(pBuff->data[0] == 0){
				
			}
			break;
		case DEBUG_CMD_GETSVER:		//获取版本号
			CL_LOG("DEBUG_CMD_GETSVER data[0]=%d \n",pBuff->data[0]);
			if(pBuff->data[0] == 0){//主版本
			
			}else{//副版本
				
			}
			break; 
		case DEBUG_CMD_BLUE:		//蓝牙
			
			dataLen = 1;
			break;
		case DEBUG_CMD_SETRTC:		//设置RTC
		{
			uint32_t time = (uint32_t)(pBuff->data[0] << 24 | pBuff->data[1] << 16 | pBuff->data[2] << 8 | pBuff->data[3]);
			CL_LOG("DEBUG_CMD_SETRTC time=%d \n",time);
			if(time != 0){
				SetRtcCount(time);
				pBuff->data[0] = 0;
			}else{
				pBuff->data[0] = 1;
			}
		}
			break;
		case DEBUG_CMD_READRTC:		//读RTC
		{
			uint32_t time = GetTimeStamp();
			CL_LOG("DEBUG_CMD_READRTC. cuuuent time=%d\n",time);
			pBuff->data[0] = (time>>24) & 0xFF;
			pBuff->data[1] = (time>>16) & 0xFF;
			pBuff->data[2] = (time>>8) & 0xFF;
			pBuff->data[3] = time & 0xFF;
			dataLen = 4;
		}
			break;
		case DEBUG_CMD_FLASHTEST:	//Flash读写
			pBuff->data[0] = 0;
			dataLen = 1;
			break;
		case DEBUG_CMD_ICode:		//设置注册码
			
			pBuff->data[0] = 0;
			break;
        case DEBUG_CMD_SN:	//设置桩号

            break;
        case DEBUG_CMD_REBOOT:	//重启系统
			pBuff->data[0] = 0;
            App_CT_SendDebugPkt(0, pBuff, 1);
            osDelay(500);
            NVIC_SystemReset();
            break;
		case DEBUG_CMD_LED:	//LED灯测试
			
			pBuff->data[0] = 0;
			break;
		case DEBUG_CMD_SERVER_COM://后台通信接口
		
			break;
		case DEBUG_CMD_GETSN:	//获取桩编号
			
			dataLen = 8;
			break;
		case DEBUG_CMD_GETICODE:		//识别码
			memcpy(pBuff->data,SystemInfo.idCode,8);
			dataLen = 8;
			break;
		case DEBUG_CMD_AGE_TEST://启动老化测试
			
			break;
		case DEBUG_CMD_GET_AGE_RESULT://获取老化测试结果
			
			return ;
		case DEBUG_CMD_FW_UPGRADE://固件升级
		{
			
		}
			break;
		case DEBUG_CMD_FW_DOWNLOAD://固件下发
		{
			
		}
			break;
		default:
			CL_LOG("no find cmd=%x\n",pBuff->head.cmd);
			pBuff->data[0] = 1;
			break;
    }
    App_CT_SendDebugPkt(0, pBuff, dataLen);
}



