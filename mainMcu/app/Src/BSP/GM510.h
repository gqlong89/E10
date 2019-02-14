
#ifndef __GM510_H__
#define __GM510_H__


#include "includes.h"
#include "BswDrv_FIFO.h"


#define TX_FAIL_MAX_CNT                 5
#define SOCKET_ID                       1
#define ICCID_LEN                       20
#define BUFFER_SIZE		530

#define GM510_DEBUG                    0


//电源
#define GPRS_POWER_GPIO                 GPIOE
#define GPRS_POWER_PIN                  GPIO_PIN_7
#define GPRS_POWER_ENABLE()	  	        gpio_bit_set(GPRS_POWER_GPIO,GPRS_POWER_PIN)
#define GPRS_POWER_DISABLE()	        gpio_bit_reset(GPRS_POWER_GPIO,GPRS_POWER_PIN)

//开机
#define GPRS_PWRKEY_GPIO                GPIOA
#define GPRS_PWRKEY_PIN                 GPIO_PIN_5
#define GPRS_PWRKEY_HIGH()	  		    gpio_bit_reset(GPRS_PWRKEY_GPIO,GPRS_PWRKEY_PIN)		//pwrkey低有效
#define GPRS_PWRKEY_LOW()	  		    gpio_bit_set(GPRS_PWRKEY_GPIO,GPRS_PWRKEY_PIN)

//复位
#define GPRS_RESET_GPIO                 GPIOA
#define GPRS_RESET_PIN                  GPIO_PIN_6
#define GPRS_RESET_HIGH()	  		    gpio_bit_set(GPRS_RESET_GPIO, GPRS_RESET_PIN)
#define GPRS_RESET_LOW()	  		    gpio_bit_reset(GPRS_RESET_GPIO, GPRS_RESET_PIN)

#define GPRS_WAKE_GPIO					GPIOA
#define GPRS_WAKE_PIN					GPIO_PIN_4
#define GPRS_WAKE_HIGH()				gpio_bit_set(GPRS_WAKE_GPIO,GPRS_WAKE_PIN)
#define GPRS_WAKE_LOW()					gpio_bit_reset(GPRS_WAKE_GPIO,GPRS_WAKE_PIN)


#define PRE_WRITE_FLASH_LEN             512

enum{
    NET_STATE_SOCKET0_OPEN=0,
    NET_STATE_READY,
    NET_STATE_FAILURE,
    NET_NULL,
};

enum {
    GM510_RESET = 0,
    GM510_ATE,
    GM510_GMM,
    #if (0 == TEST_AIR_WIRE)
	GM510_ATI,
    GM510_CPIN,
	GM510_CCID,
	GM510_CREG,
	GM510_COPS,
	GM510_ZPAS,
	GM510_ZIPCALL,
	GM510_ZIPCALL0,
	GM510_ZIPCALL1,
    GM510_CSQ,
    GM510_IPOPEN,
    #endif
    GM510_STATE_NUM
};

typedef struct {
    char *cmd;
    char *res;
    int wait;
    int nwait;
    int (*process)(char ok, uint8_t retry);
}GPRS_INIT_TAB_STR;

#pragma pack(1)
typedef struct{
    uint32_t size;
    uint16_t checkSum;
    uint8_t  name[10];
}DOWN_FW_INFO_STR;
#pragma pack()

int GM510_check_test(char ok, uint8_t retry);
int GM510_check_ATE(char ok, uint8_t retry);
int GM510_check_GMM(char ok, uint8_t retry);
int GM510_check_ATS(char ok, uint8_t retry);
int GM510_check_ATI(char ok, uint8_t retry);
int GM510_check_cpin(char ok, uint8_t retry);
int GM510_check_ack(char ok, uint8_t retry);
int GM510_check_CCID(char ok, uint8_t retry);
int GM510_check_CSCLK(char ok, uint8_t retry);
int GM510_check_CGACT(char ok,uint8_t retry);
int GM510_check_ZIPCFG(char ok,uint8_t retry);
int GM510_check_ZIPOPEN(char ok,uint8_t retry);
int GM510_check_ZIPCALL(char ok,uint8_t retry);
int GM510_check_ZIPCALL0(char ok,uint8_t retry);
int GM510_check_ZIPCALL1(char ok,uint8_t retry);
int GM510_check_CSQ(char ok, uint8_t retry);
int GM510_check_CREG(char ok, uint8_t retry);
int GM510_check_CSTT(char ok, uint8_t retry);
int GM510_check_cgreg(char ok, uint8_t retry);
int GM510_ipopen(char ok, uint8_t retry);
int GM510_default(char ok, uint8_t retry);
int GM510_check_ZPAS(char ok, uint8_t retry);
int GM510_check_COPS(char ok, uint8_t retry);
int GM510_check_CIPQSEND(char ok, uint8_t retry);
int GM510SetZMBN(char ok, uint8_t retry);
int GM510GetModuleId(char *pStr);
int GM510FtpGet(const char* serv, const char* un, const char* pw, const char* file, uint8_t fwType);
int GM510Reconnect(void);
int GM510SocketSend(int socket, uint8_t* data, uint16_t len);
int GM510QueZMBN(char ok, uint8_t retry);
int GM510SendCmd(char *cmd, char *ack, uint16_t waittime, int flag);

extern int GM510Init(void);
extern uint8_t GM510GetNetSignal(void);
extern int GM510SendCmdnospace(char * cmd, char * ack, int waitCnt, int waittime, uint8_t *data);
int GM510SocketStateCheck(void);

extern int GM510SendData(char *data, int len, char *ack, uint16_t waittime);
extern FIFO_S_t gSocketPktRxCtrl;
extern volatile uint32_t gSimStatus;
extern volatile uint8_t gNetSignal;


#endif


