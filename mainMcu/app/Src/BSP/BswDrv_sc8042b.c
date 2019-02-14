#include "includes.h"
#include "BswDrv_sc8042b.h"
#include "BswDrv_Delay.h"
#include "GM510.h"
#include "blue_proto.h"


static SemaphoreHandle_t  mutex;



void SoundCode(int code)
{
    uint8_t codeVal[3];

    codeVal[0] = code / 100 % 10;
    codeVal[1] = code / 10 % 10;
    codeVal[2] = code / 1 % 10;
    Sc8042bSpeech((SC8042B_VOICE_TYPE)(codeVal[0]+31));
    vTaskDelay(250);
    Sc8042bSpeech((SC8042B_VOICE_TYPE)(codeVal[1]+31));
    vTaskDelay(250);
    Sc8042bSpeech((SC8042B_VOICE_TYPE)(codeVal[2]+31));
    vTaskDelay(250);
}

//提示操作失败及具体代码
void OptFailNotice(int code)
{
    FeedWatchDog();
    CL_LOG("error code=%03d.\n",code);
    if ((code > GM510_STATE_NUM) && (100 != code) && (101 != code)) 
	{
		SoundCode(code);
	}
	else
	{
		if (code != gChgInfo.errCode) 
		{
          //  UiDisplay_ErrCode(code);
			SoundCode(code);
			gChgInfo.errCode = code;
		}
	}
}


//提示操作成功及具体代码
void OptSuccessNotice(int code)
{
    FeedWatchDog();
    CL_LOG("success code=%03d.\n",code);
    Sc8042bSpeech(VOIC_SUCCESS);
    vTaskDelay(1000);
    SoundCode(code);
}

void BswDrv_Sc8042bSpeech(SC8042B_VOICE_TYPE cnt)
{
    osMutexWait (mutex,1000);
    //    int count = 0;
    //    //判忙  2s超时
    //    while(READ_AU_BUSY() != 0){
    //        Delay_mSec(10);
    //        if(count++ >200){
    //            return -1;
    //        }
    //    }
    AU_RST_EN();
    BswDrv_HwDelay_100us();
    AU_RST_DIS();
    BswDrv_HwDelay_100us();

    for(uint8_t i = 0;i < cnt; i++)
    {
        AU_DATA_HIHG();
        BswDrv_HwDelay_100us();
        AU_DATA_LOW();
        BswDrv_HwDelay_100us();
    }
    osMutexRelease (mutex);
}


void BswDrv_SC8042B_Init(void)
{
    AU_POWER_EN();
    AU_DATA_LOW();
    AU_RST_DIS();

    //创建互斥量
    mutex = osMutexCreate(NULL);
}



