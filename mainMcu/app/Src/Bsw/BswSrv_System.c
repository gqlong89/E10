 /*
 * @Author: zhoumin 
 * @Date: 2018-10-12 14:15:23 
 * @Last Modified by: zhoumin
 * @Last Modified time: 2018-10-27 14:35:37
 */

#include "includes.h"
#include "gd32f30x.h"
#include "BswSrv_System.h"
#include "BswSrv_FlashUsr.h"
#include "BswDrv_Usart.h"
#include "BswDrv_Debug.h"
#include "BswDrv_sc8042b.h"
#include "BswDrv_Adc.h"
#include "BswDrv_GPIO.h"



SYSTEM_INFO_T	SystemInfo = {0,};
GLOBAL_INFO_T	GlobalInfo = {0,};


#define MAGIC_NUM_BASE                  0x123456AE

#define WIFI_NAME             		"\"OFFICE\""
#define WIFI_PASSWORD             	"\"Chargerlink-608\""


void EXTI0_IRQHandler(void)
{
	if (RESET != exti_interrupt_flag_get(EXTI_0)) {
        exti_interrupt_flag_clear(EXTI_0);

		if(READ_LVD_IN() == 0)
		{
			//低电压..
		}
    }
}

/**
 *加载系统参数
 */
void BswSrv_LoadSystemInfo(void)
{
    const uint8_t station_id[8] = {0x00,0x00,0x00,0x83,0x83,0x90,0x00,0x30};
    
    FlashReadSysInfo(&SystemInfo,sizeof(SystemInfo));

    if ((MAGIC_NUM_BASE) == SystemInfo.magic_number) 
    {
        printf("\n\n\n==============================================\n");
        CL_LOG("设备启动\n");
    }
    else
    {
        printf("\n\n\n**********************************************\n");
        CL_LOG("设备初次启动\n");
        memset((void*)&SystemInfo, 0, sizeof(SystemInfo));
        SystemInfo.magic_number = MAGIC_NUM_BASE;
        memcpy(SystemInfo.stationId, station_id,sizeof(station_id));
        FlashWriteSysInfo(&SystemInfo,sizeof(SystemInfo));
    }
    CL_LOG("设备版本号:%d.%d\n", FW_VERSION, FW_VERSION_SUB1);
    PrintfData("设备号", SystemInfo.stationId, sizeof(SystemInfo.stationId));
    PrintfData("注册码", SystemInfo.idCode, sizeof(SystemInfo.idCode));
    CL_LOG("host=%s port=%d \r\n",NET_SERVER_IP,NET_SERVER_PORT);
    CL_LOG("\n 文件编译时间, 月日年 %s 时分秒 %s \n", __DATE__, __TIME__);
    
    #if 0
    //测试--配置wifi信息
    strcpy(SystemInfo.WifiName,WIFI_NAME);
    strcpy(SystemInfo.WifiPasswd,WIFI_PASSWORD);

    //测试--添加烟雾报警设备
    // 0x3D8585
    // 0x3EAAB5
    SystemInfo.RfDev.bandSize = 2;
    SystemInfo.RfDev.Unit[0].num = 1;
    SystemInfo.RfDev.Unit[0].address = 0x3D858;
    
    SystemInfo.RfDev.Unit[1].num = 2;
    SystemInfo.RfDev.Unit[1].address = 0x3EAAB;

    #endif 
	
    //全局变量初始化
    memset(&GlobalInfo,0,sizeof(GlobalInfo));
    GlobalInfo.readCard_Callback = NULL;
    GlobalInfo.AppCan_HandleCallBack = NULL;
		
    if(BswSrv_Tool_isArraryEmpty(SystemInfo.idCode,8) != CL_OK)
    {
        GlobalInfo.isRegister = 1;  //已经注册
    }
	
	//蓝牙名称--使用桩号作为蓝牙名
    BswSrv_Tool_BCDToString(GlobalInfo.BlueName,(void*)&SystemInfo.stationId[3], 5);

    CL_LOG("wifi username=%s \r\n",SystemInfo.WifiName);
    CL_LOG("wifi passwd=%s \r\n",SystemInfo.WifiPasswd);

    CL_LOG("RfDev bandSize = %d \r\n",SystemInfo.RfDev.bandSize);
    for(uint8_t i = 0;i<SystemInfo.RfDev.bandSize;i++)
    {
        CL_LOG("RfDev Num=%d  address=%X \r\n",SystemInfo.RfDev.Unit[i].num,SystemInfo.RfDev.Unit[i].address);
    }
}



void BswSrv_SystemResetRecord(void)
{
    if(rcu_flag_get(RCU_FLAG_EPRST) == SET)
    {
        CL_LOG("系统外部复位管教复位\n");
    }
    else if(rcu_flag_get(RCU_FLAG_PORRST) == SET)
    {
        CL_LOG("系统上电复位\n");
    }
    else if(rcu_flag_get(RCU_FLAG_SWRST) == SET)
    {
        CL_LOG("系统软件复位\n");
    }
    else if(rcu_flag_get(RCU_FLAG_FWDGTRST) == SET)
    {
        CL_LOG("系统独立看门狗复位\n");
    }
    else if(rcu_flag_get(RCU_FLAG_WWDGTRST) == SET)
    {
        CL_LOG("系统窗口看门狗复位\n");
    }
    else if(rcu_flag_get(RCU_FLAG_LPRST) == SET)
    {
        CL_LOG("系统低功耗复位\n");
    }
    rcu_all_reset_flag_clear();
}


/**
 * 获取芯片唯一id
 * 12个字节 96位
 */
int BswSrv_GetHWId(uint8_t id[])
{
	if(id == NULL) return CL_FAIL;
	
    //获取MCU唯一ID
    uint32_t Unique_ID1 = *(uint32_t *)(0x1FFFF7E8);        //UNIQUE_ID[31: 0]
    uint32_t Unique_ID2 = *(uint32_t *)(0x1FFFF7EC);        //UNIQUE_ID[63:32]
    uint32_t Unique_ID3 = *(uint32_t *)(0x1FFFF7F0);        //UNIQUE_ID[95:63]
	
	id[0] = (Unique_ID1 >> 24) & 0xFF;
	id[1] = (Unique_ID1 >> 16) & 0xFF;
	id[2] = (Unique_ID1 >> 8) & 0xFF;
	id[3] = (Unique_ID1) & 0xFF;
	
	id[4] = (Unique_ID2 >> 24) & 0xFF;
	id[5] = (Unique_ID2 >> 16) & 0xFF;
	id[6] = (Unique_ID2 >> 8) & 0xFF;
	id[7] = (Unique_ID2) & 0xFF;
	
	id[8] = (Unique_ID3 >> 24) & 0xFF;
	id[9] = (Unique_ID3 >> 16) & 0xFF;
	id[10] = (Unique_ID3 >> 8) & 0xFF;
	id[11] = (Unique_ID3) & 0xFF;
	
	return CL_OK;
}


int BswSrv_GetCpuTemp(void)
{
    int temp = (int)BswDrv_ADC_GetSensorTemp();
    return temp;
}

//4031877
void BswSrv_RF433AddrToChar(uint32_t addr,uint8_t ch[])
{
    // char temp[10] = {0};
    // char str[10] = {0};
    // sprintf(temp,"%d",addr);
    // if(strlen(temp) % 2 != 0){
    //     str[0] = '0';
    // }
    // strcat(str,temp);
    // BswSrv_Tool_StringToBCD(&ch[1],str);
    // ch[0] = 0;

    ch[0] = 0;
    ch[1] = (addr>>24) & 0xFF;
    ch[2] = (addr>>16) & 0xFF;
    ch[3] = (addr>>8) & 0xFF;
    ch[4] = (addr) & 0xFF;
}


uint32_t BswSrv_CharToRF433Addr(uint8_t ch[])
{
    uint32_t addr = 0;
    addr = (uint32_t)( (ch[1]<<24)|(ch[2]<<16)|(ch[3]<<8)|ch[4]);
    // char temp[10]={0};
    // sprintf(temp,"%02X%02X%02X%02X",ch[1],ch[2],ch[3],ch[4]);
    // addr = atoi(temp);
    return addr;
}


void BswSrv_SystemReboot(void)
{
    Sc8042bSpeech(VOIC_DEVICE_REBOOT);
    osDelay(1000);
    NVIC_SystemReset();
}


uint16_t BswSrv_Tool_CheckSum(uint8_t *data,uint16_t len)
{
    uint16_t sum = 0;

    for(uint16_t i = 0; i < len; i++)
    {
        sum += data[i];
    }
    
    return sum;
}


int BswSrv_Tool_isArraryEmpty(uint8_t *array,int len)
{
	for(int i = 0;i<len ; i++){
		if(array[i] != 0x00)
        {
			return CL_FAIL;
		}
	}
	return CL_OK;
}

static unsigned char Asc2Int(char ch) 
{
    unsigned char val = 0;
    if ((ch >= '0') && (ch <= '9')) {
        val = ch - '0';
    } else if ((ch >= 'A') && (ch <= 'F')) {
        val = ch - 'A' + 10;
    } else if ((ch >= 'a') && (ch <= 'f')) {
        val = ch - 'a' + 10;
    }
    return val;
}


int BswSrv_Tool_StringToBCD(unsigned char *BCD, const char *str) 
{
    unsigned char chh, chl;
    int length = strlen(str);
    int index = 0;

    for (index = 0; index < length; index += 2) {
        chh = Asc2Int(str[index]);
        chl = Asc2Int(str[index + 1]);
       
        BCD[index / 2] = (chh << 4) | chl;
    }
    return (length / 2);
}

char *BswSrv_Tool_BCDToString(char *dest, unsigned char *BCD, int bytes) 
{
    char  temp[] = "0123456789ABCDEF";
    int index = 0;
    int length = 0;
    if (BCD == NULL || bytes <= 0)
        return NULL;
    
    for (index = 0; index < bytes; index++) {
        dest[length++] = temp[(BCD[index] >> 4) & 0x0F];
        dest[length++] = temp[BCD[index] & 0x0F];
    }
    dest[length] = '\0';
    return dest;
}
