#include "APP.h"
#include "BswSrv_System.h"
#include "App_Main.h"
#include "BswDrv_GPIO.h"
#include "BswDrv_Adc.h"



void APP_Init(void)
{
    GlobalInfo.readCard_Callback = App_ReadCard_CallBack;

    BswDrv_ADC_Start();

    LED_R_ON();
}

