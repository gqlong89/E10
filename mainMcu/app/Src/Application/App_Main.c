#include "includes.h"
#include "BswSrv_System.h"
#include "BswDrv_Rtc.h"
#include "BswDrv_GPIO.h"
#include "BswDrv_Debug.h"
#include "BswDrv_sc8042b.h"
#include "BswSrv_RF433.h"
#include "App_Main.h"
#include "App_NetProto.h"
#include "App_BlueProto.h"
#include "BswDrv_Sys_Flash.h"
#include "App_ConfigTool.h"
#include "BswSrv_Key.h"
#include "BswSrv_WG215.h"
#include "BswSrv_FlashUsr.h"


/**
 *读卡器读卡回调函数
 * direction：0--进  1-出
 */
void App_ReadCard_CallBack(uint8_t direction,uint8_t *cardId)
{
	CL_LOG("cardId=%s \r\n",cardId);
	if(GlobalInfo.isLogin)
	{
		App_NetProto_SendCardAuthReq(cardId,direction);
	}
}


void App_Test(void)
{
	// uint8_t buf[64] = {0,};
	// BswDrv_SysFlashErase(OtherInfoAddr,1);
	// BswDrv_SysFlashWrite(OtherInfoAddr,"a1",2);
	// FlashRead(OtherInfoAddr,buf,64);
	// CL_LOG("read buf=%s \r\n",buf);

//	SetRtcCount(1540450744);
//	osDelay(1000);
//	time_t t = GetTimeStamp();
//	CL_LOG("time=%X \r\n",t);
	// uint8_t a[] = {0x00,0x04,0x03,0x18,0x77};
	// CL_LOG("addrss = %X \r\n",BswSrv_CharToRF433Addr(a));
	// uint8_t ch[5];
	// BswSrv_RF433AddrToChar(0x03D858,ch);

	// PrintfData("bcd=",&ch[1],4);

	// char httpurl[] = "device.sharecharger.com//resource//DEVICE_UPGRADE_PACKAGE/release/2018/1016/7319.bin";
	// char HttpIP[32] = {0};
	// char FilePath[80] = {0};

	// //解析url
	// char* sp1 = strchr(httpurl, '/');
    // strncpy(HttpIP,httpurl,sp1-httpurl);
	// strcpy(FilePath,sp1);

	// CL_LOG("HttpIP=%s, FilePath=%s.\n", HttpIP, FilePath);
}



void App_MainTask(void)
{
	uint8_t first = 1;
	time_t tick = 0;
	uint32_t second = 0;
	RF_Unit_Typedef rf433Dev;

	osDelay(1000);
	
	Sc8042bSpeech(VOIC_WELCOME);
	
	App_Test();
	
	while(1)
	{
		osDelay(200);

		//低电压检测..
		if(READ_LVD_IN() == 0)
		{
			//todo
		}
		CL_LOG("hello world!!! \r\n");

		BswSrv_Key_Loop();

		if(GetRtcCount() != tick)
		{
			tick = GetRtcCount();
			
			//获取报警设备信息
			if(CL_OK == BswSrv_RF433_GetWaringDevice(&rf433Dev))
			{
				CL_LOG("check rf433 waring:addr=%X \r\n",rf433Dev.address);
				App_NetProto_SendEventNotice(0,EVENT_SMOKE_WARING,rf433Dev.num,rf433Dev.address,1,NULL);
				//清除标志
				BswSrv_RF433_ClearFlag(rf433Dev.address);
			}
			
			//网络处理
			if(GlobalInfo.is_socket_0_ok && GlobalInfo.isRecvServerData && GlobalInfo.upgradeFlag == 0)
			{
				/*注册*/
				if(GlobalInfo.isRegister == 0)
				{
					if(first)
					{
						App_NetProto_SendRegister();
						first = 0;
					}
					else
					{
						if(second % 60 == 0){
							App_NetProto_SendRegister();
						}
					}
				}
				else
				{
					/*登陆*/
					if(GlobalInfo.isLogin == 0)
					{
						if(first){
							App_NetProto_SendStartUpNotice(1);
							first = 0;
						}else{
							if(second % 60 == 0){
								App_NetProto_SendStartUpNotice(1);
							}
						}
					}
					else
					{
						LED_G_TOGGLE();
						/*心跳*/
						if(second % 60 == 0){
							App_NetProto_SendHeartBeat();
						}
						
						//请求绑定设备 todo
					}
				}
			}else{
				first = 1;
				LED_G_OFF();
			}

			//蓝牙处理
			if(GlobalInfo.isBlueConnect)
			{
				//如果蓝牙已经登陆--定时发送心跳
				if(GlobalInfo.isBlueLogin && second%10 == 0)
				{
					App_Blue_SendHeartBat();
				}
				if(osGetTimes() - GlobalInfo.blueLastOnlineTime > 1000*30)
				{
					CL_LOG("check blue connect timeout.\n");
					GlobalInfo.isBlueConnect = 0;
					
					if(GlobalInfo.isBlueLogin)
					{
						GlobalInfo.isBlueLogin = 0;
						Sc8042bSpeech(VOIC_BLUETOOTH_OFFLINE);
					}
					//断开蓝牙连接
					BswSrv_Blue_Disconnent();
				}
			}
			second++;
		}
	}
}
