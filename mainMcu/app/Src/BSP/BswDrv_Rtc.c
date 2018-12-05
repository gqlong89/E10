#include "BswDrv_Rtc.h"
#include "includes.h"


static int LinuxTickToDay(time_t timestamp, uint8_t *pDay);


/*******************************************************
函数原型:   int LinuxTickToDay(time_t timestamp, uint8_t *pDay)
函数参数:   time_t timestamp:Linux时间戳
            uint8_t *pDay：Linux时间戳转换后的时间
函数功能:   将Linux时间戳转换为当前的时间
返回值  :   无
********************************************************/
int LinuxTickToDay(time_t timestamp, uint8_t *pDay)
{
    struct tm *time_now;

	time_now = localtime(&timestamp);
    pDay[0] = time_now->tm_wday;
    pDay[1] = time_now->tm_year-100;
    pDay[2] = time_now->tm_mon+1;
    pDay[3] = time_now->tm_mday;
    pDay[4] = time_now->tm_hour;
    pDay[5] = time_now->tm_min;
    pDay[6] = time_now->tm_sec;
    return CL_OK;
}



/*******************************************************
函数原型:   char* GetCurrentTime(void)
函数参数:   无
函数功能:   得到当前的时间
返回值  :   char *
********************************************************/
static char gTimeStr[32];
char* GetCurrentTime(void)
{
    uint32_t time;
    uint8_t day[8];
	
    time = rtc_counter_get();
    LinuxTickToDay(time,day);

    sprintf(gTimeStr,"%02u-%02u %02u:%02u:%02u", day[2], day[3], day[4], day[5], day[6]);
	
    return gTimeStr;
}


void GetRtcTime(void* pRTCTime)
{
    uint32_t time = 0;
    time = rtc_counter_get();
    LinuxTickToDay(time,pRTCTime);
}



/*******************************************************
函数原型:   void SetRtcCount(time_t timestamp)
函数参数:   time_t timestamp：Linux时间戳
函数功能:   将Linux时间戳转换为当前的时间并设置到RTC寄存器
返回值  :
********************************************************/
void SetRtcCount(time_t timestamp)
{
    rtc_counter_set(timestamp+8*60*60);
}

/*******************************************************
函数原型:   time_t GetTimeStamp(void)
函数参数:   无
函数功能:   获取当前的时间戳
返回值  :   时间戳
********************************************************/
time_t GetTimeStamp(void)
{
	struct tm *time_now;
	
	uint32_t tick = rtc_counter_get();
	
	time_now = localtime(&tick);
	time_now->tm_hour -= 8;
	
	return mktime(time_now);	
}


void BswDrv_RtcInit(void)
{
	rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
	
	pmu_backup_write_enable();

	/* reset backup domain */
	bkp_deinit();

	rcu_osci_on(RCU_IRC40K);         //使能内部低速时钟
	rcu_osci_stab_wait(RCU_IRC40K);

	rcu_rtc_clock_config(RCU_RTCSRC_IRC40K);

	rcu_periph_clock_enable(RCU_RTC);
	rtc_register_sync_wait();

	/* wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();
	/* set RTC prescaler: set RTC period to 1s */
	rtc_prescaler_set(40000);//
	/* wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();
	
//	rcu_periph_clock_enable(RCU_BKPI);
//    rcu_periph_clock_enable(RCU_PMU);
//	rcu_periph_clock_enable(RCU_RTC);
//    if (bkp_read_data(BKP_DATA_0) != 0xA5A5){

//        CL_LOG("rtc first configure..\r\n");
//        
//        /* allow access to BKP domain */
//        pmu_backup_write_enable();

//        /* reset backup domain */
//        bkp_deinit();

//        rcu_osci_on(RCU_IRC40K);         //使能内部低速时钟
//        rcu_osci_stab_wait(RCU_IRC40K);
//        
//        rcu_rtc_clock_config(RCU_RTCSRC_IRC40K);

//        rcu_periph_clock_enable(RCU_RTC);
//        rtc_register_sync_wait();

//        /* wait until last write operation on RTC registers has finished */
//        rtc_lwoff_wait();
//        /* set RTC prescaler: set RTC period to 1s */
//        rtc_prescaler_set(40000);//
//        /* wait until last write operation on RTC registers has finished */
//        rtc_lwoff_wait();


//        /* wait until last write operation on RTC registers has finished */
//        rtc_lwoff_wait();
//        /* change the current time */
//        rtc_counter_set(1540516692);
//        /* wait until last write operation on RTC registers has finished */
//        rtc_lwoff_wait();

//        bkp_write_data(BKP_DATA_0, 0xA5A5);
//    }else{
//		
//        pmu_backup_write_enable();

//        printf("No need to configure RTC....");
//        /* wait for RTC registers synchronization */
//        rtc_register_sync_wait();

//        /* wait until last write operation on RTC registers has finished */
//        rtc_lwoff_wait();
//    }

//    rcu_all_reset_flag_clear();
}

