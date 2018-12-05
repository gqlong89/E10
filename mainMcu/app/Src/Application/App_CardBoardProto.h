#ifndef __APP_CARDBOARD_PROTO_H__
#define __APP_CARDBOARD_PROTO_H__


#include <stdint.h>
#include "BswSrv_CardBoard.h"

#define PACKAGE_SIZE    64


#define MODUL_BASE      0x01
#define MODUL_CARD      0x02
#define MODUL_UPGRADE   0x03


#pragma pack(1)

typedef struct {
    uint8_t  aa;                                    //0.
    uint8_t  five;                                  //1.
	uint16_t len;								    //2.����
    uint8_t  ver;                                   //3.�汾��
    uint8_t  sn;                                    //4.������ˮ��
	uint8_t  module;							    //5. 01-��������   02-������  03-Զ������
    uint8_t  cmd;                                   //6.�������
}CB_HEAD_STR;

typedef struct {
    CB_HEAD_STR head;
    uint8_t  data[CB_PKT_LEN-sizeof(CB_HEAD_STR)];
}CB_STR_t;


typedef struct{
    uint8_t fw_verson;
    uint8_t cardState;
    uint8_t reseverd[4];
}CB_STARTUP_t;

typedef struct{
    uint8_t cardState;
    uint8_t reseverd[4];
}CB_HEARTBAT_t;

typedef struct{
    uint8_t cardType;
    char cardId[16];
    uint8_t reseverd[4];
}CB_UP_CARDINFO_t;

typedef struct{
    uint8_t sector;
    uint8_t block;
    uint8_t data[16];
}CB_WRITE_CARDINFO_t;

typedef struct{
    uint32_t filesize;
    uint32_t package;
    uint16_t checkSum;
    uint8_t  fw_verson;
}CB_START_UPGRADE_t;


typedef struct{
    uint8_t    index;
    uint8_t  data[PACKAGE_SIZE];
}CB_DOWN_FW_t;

typedef struct{
    uint8_t result;
    uint8_t    index;
}CB_DOWN_FW_ACK_t;

typedef struct{
    uint8_t result;
}CB_RESULE_ACK_t;

#pragma pack()


void App_CB_SendWriteCard(uint8_t sector,uint8_t block ,uint16_t *data);
void App_CB_SendStartUpgrade(uint32_t fileSize,uint32_t package,uint16_t checkSum,uint8_t verson);


#endif

