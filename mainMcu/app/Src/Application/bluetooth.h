#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#include "includes.h"
#include "BswDrv_FIFO.h"


#define CKB_MAX_PKT_LEN             256

#define BLUETOOTH_POWER_PORT        HT_GPIOE
#define BLUETOOTH_POWER_PIN         GPIO_Pin_8
#define BluetoothPowerOn()          HT_GPIO_BitsSet(BLUETOOTH_POWER_PORT,BLUETOOTH_POWER_PIN)
#define BluetoothPowerOff()         HT_GPIO_BitsReset(BLUETOOTH_POWER_PORT,BLUETOOTH_POWER_PIN)


#define BLUETOOTH_REST_PORT       //  HT_GPIOG
#define BLUETOOTH_REST_PIN        //  GPIO_Pin_0
#define BluetoothResetHigh()      //  HT_GPIO_BitsSet(BLUETOOTH_REST_PORT,BLUETOOTH_REST_PIN)
#define BluetoothResetLow()       //  HT_GPIO_BitsReset(BLUETOOTH_REST_PORT,BLUETOOTH_REST_PIN)


#define BLUETOOTH_C_D_PORT       //   HT_GPIOG
#define BLUETOOTH_C_D_PIN        //   GPIO_Pin_1
#define BluetoothCDHigh()        //   HT_GPIO_BitsSet(BLUETOOTH_C_D_PORT,BLUETOOTH_C_D_PIN)
#define BluetoothCDLow()         //   HT_GPIO_BitsReset(BLUETOOTH_C_D_PORT,BLUETOOTH_C_D_PIN)
#define BluetoothCDState()       //   HT_GPIO_BitsRead(BLUETOOTH_C_D_PORT,BLUETOOTH_C_D_PIN)

#define BLUETOOTH_IRQ_PORT      //    HT_GPIOA
#define BLUETOOTH_IRQ_PIN       //    GPIO_Pin_8
#define BluetoothIrqState()      //   HT_GPIO_BitsRead(BLUETOOTH_IRQ_PORT,BLUETOOTH_IRQ_PIN)


typedef struct {

	FIFO_S_t rxBtBuff;
    #ifdef X6
		FIFO_S_t rx24GBuff;
	#endif
	SemaphoreHandle_t bluetoothRxSemaphore;

}BLUETOOTH_CTRL_STR;


enum{
    NODE_BLUE = 0,
    NODE_24G
};

enum {
    BLUE_AB,
    BLUE_CD,
    BLUE_TARGET,
    BLUE_ADDR,
    BLUE_LEN,
    BLUE_RX_DATA,
    BLUE_CHK,
    BLUE_END,
};

typedef enum {
    FIND_AA,
    FIND_55,
    FIND_CHARGER_TYPE,
    FIND_CHAGER_SN,
    FIND_SRC,
    FIND_DEST,
    FIND_LEN,
    FIND_VER,
    FIND_SERNUM,
    FIND_CMD,
    RX_DATA,
    FIND_CHK,
    FIND_END,
} PROTO_MSG_STR;

enum {
    BT_FIND_EE,
    BT_FIND_CMD,
    BT_FIND_LEN,
    BT_RX_DATA,
    BT_FIND_CHK,
    BT_FIND_END,
};
	
enum{
    BLUE_RESET = 0,
	BLUE_MODE_ECHO,
	BLUE_MODE_VER,
	BLUE_MODE_GETNAME,
#if SET_BLUE_NAME
	BLUE_MODE_SETNAME,
#endif
	BLUE_MODE_PAIR,
    BLUE_STATE_NUM
};

#define BLUE_CONNECT_TICK				(120)
#define SET_BLUE_NAME					0

#define CKB24_POWER_EN()    gpio_bit_set(GPIOA, GPIO_PIN_8) 
#define CKB24_POWER_DIS()   gpio_bit_reset(GPIOA, GPIO_PIN_8)

#define CKB24_RST_HIGH()    gpio_bit_set(GPIOB, GPIO_PIN_8) 
#define CKB24_RST_LOW()     gpio_bit_reset(GPIOB, GPIO_PIN_8) 

#define CKB24_CD_HIGH()     gpio_bit_set(GPIOB, GPIO_PIN_9)
#define CKB24_CD_LOW()      gpio_bit_reset(GPIOB, GPIO_PIN_9)
#define GET_CD_STATE()		gpio_input_bit_get(GPIOB, GPIO_PIN_9)

#define GET_BLUE_IRQ()      gpio_input_bit_get(GPIOB, GPIO_PIN_15)

extern void BluetoothTask(void);
extern int StringToBCD(unsigned char *BCD, const char *str);
extern char *BCDToString(char *dest, unsigned char *BCD, int bytes);

#endif


