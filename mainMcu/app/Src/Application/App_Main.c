#include "includes.h"
#include "BswSrv_System.h"
#include "BswDrv_Rtc.h"
#include "BswDrv_GPIO.h"
#include "BswDrv_Debug.h"
#include "BswDrv_sc8042b.h"
#include "App_Main.h"
#include "BswDrv_Sys_Flash.h"
#include "App_ConfigTool.h"
#include "BswSrv_Key.h"
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
	//	App_NetProto_SendCardAuthReq(cardId,direction);
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

	osDelay(1000);
	
	Sc8042bSpeech(VOIC_WELCOME);
	
	App_Test();
	
	while(1)
	{
		osDelay(200);

		CL_LOG("hello world!!! \r\n");

		BswSrv_Key_Loop();

	}
}
