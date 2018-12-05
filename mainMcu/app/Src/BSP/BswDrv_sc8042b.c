#include "includes.h"
#include "BswDrv_sc8042b.h"
#include "BswDrv_Delay.h"




static SemaphoreHandle_t  mutex;


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

    for(uint8_t i = 0;i < cnt; i++){
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



