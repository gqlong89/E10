#include "includes.h"
#include "BswSrv_System.h"
#include "BswDrv_Debug.h"
#include "BswDrv_Usart.h"
#include "App_CardBoardProto.h"
#include "BswSrv_FwUpgrade.h"

uint8_t gCBSendBuffer[CB_PKT_LEN];

static uint8_t packageSn = 0;

static void App_CBFrameHandle(uint8_t *data,uint16_t len);
static void App_CB_SendData(CB_STR_t *ptk,uint16_t len);

static void App_CB_Handle_BaseInfo(CB_STR_t *ptk);
static void App_CB_Handle_CardInfo(CB_STR_t *ptk);
static void App_CB_Handle_UpgradeInfo(CB_STR_t *ptk);



void BswSrv_CB_RxData_Callback(uint8_t *data,uint16_t len)
{
    App_CBFrameHandle(data,len);
}



void App_CB_SendData(CB_STR_t *ptk,uint16_t len)
{
    ptk->head.five = 0x55;
    ptk->head.aa = 0xAA;
    ptk->head.sn = packageSn++;
    ptk->head.len = 5+len;
    ptk->head.ver = 0x01;

    ptk->data[len] = BswSrv_Tool_CheckSum((uint8_t*)&ptk->head.ver,len+4);


    BswSrv_CB_SendData((void*)ptk,sizeof(CB_HEAD_STR)+len+1);
}


/**
 * д��
 * data���ȵ���16
 */ 
void App_CB_SendWriteCard(uint8_t sector,uint8_t block ,uint16_t *data)
{
    // uint8_t pBuffer[64] = {0};
    CB_STR_t *ptk = (void*)&gCBSendBuffer[0];

    CB_WRITE_CARDINFO_t *writeCard = (void*)ptk->data;
    writeCard->sector = sector;
    writeCard->block = block;
    memcpy(writeCard->data,data,16);

    ptk->head.module = MODUL_CARD;
    ptk->head.cmd = 0x02;

    App_CB_SendData(ptk,sizeof(CB_WRITE_CARDINFO_t));
}


/**
 * ��������
 */ 
void App_CB_SendStartUpgrade(uint32_t fileSize,uint32_t package,uint16_t checkSum,uint8_t verson)
{
    // uint8_t pBuffer[64] = {0};
    CB_STR_t *ptk = (void*)&gCBSendBuffer[0];

    CB_START_UPGRADE_t *startUpgrade = (void*)ptk->data;
    memset(startUpgrade,0,sizeof(CB_START_UPGRADE_t));

    startUpgrade->filesize = fileSize;
    startUpgrade->package = package;
    startUpgrade->checkSum = checkSum;
    startUpgrade->fw_verson = verson;
    
    ptk->head.module = MODUL_UPGRADE;
    ptk->head.cmd = 0x01;

    App_CB_SendData(ptk,sizeof(CB_START_UPGRADE_t));
}

/**
 * �̼��·�
 */ 
void App_CB_DownFW(uint8_t package,uint8_t *data,uint16_t len)
{
    if(len > PACKAGE_SIZE)
    {
        CL_LOG("fw length is too loog.\r\n");
        return;
    } 
    // uint8_t pBuffer[128] = {0};
    CB_STR_t *ptk = (void*)&gCBSendBuffer[0];
    CB_DOWN_FW_t *fw = (void*)ptk->data;
    memset(fw,0,sizeof(CB_DOWN_FW_t));

    fw->index = package;
    memcpy(fw->data,data,len);
    
    ptk->head.module = MODUL_UPGRADE;
    ptk->head.cmd = 0x02;

    App_CB_SendData(ptk,len+1);
}

//������Ϣ
void App_CB_Handle_BaseInfo(CB_STR_t *ptk)
{
    uint8_t cmd = ptk->head.cmd;
    uint8_t result = 0;
    if(cmd == 0x01) //��������
    {
        CB_STARTUP_t *startUp = (void*)ptk->data;
        CL_LOG("CardBoard start up,cardStatus=%d verson=%d.r\n",startUp->cardState,startUp->fw_verson);
        GlobalInfo.CBInitOK = startUp->cardState;
        GlobalInfo.CBVerson = startUp->fw_verson;
        GlobalInfo.lastRecvCBTime = osGetTimes();
        if(BswSrv_Upgrade_ReadHeadInfo(FW_U8_BAK,NULL) == CL_OK)
        {
            CL_LOG("��⵽ˢ�����и���..\r\n");
            BswSrv_StartCardBoard_UpgradeTask();
        }
    }
    else if(cmd == 0x02)//����
    {
        CB_HEARTBAT_t *heartBat = (void*)ptk->data;
        CL_LOG("heartBat state=%d \r\n",heartBat->cardState);
        GlobalInfo.CBInitOK = heartBat->cardState;
        GlobalInfo.lastRecvCBTime = osGetTimes();
    }

    //send ack
    CB_RESULE_ACK_t *pACK = (void*)ptk->data;
    memset(pACK,0,sizeof(CB_RESULE_ACK_t));
    pACK->result = result;
    App_CB_SendData(ptk,sizeof(CB_RESULE_ACK_t));
}

//������
void App_CB_Handle_CardInfo(CB_STR_t *ptk)
{
    uint8_t cmd = ptk->head.cmd;
    if(cmd == 0x01) //���ű�
    {
        CB_UP_CARDINFO_t *upCard = (void*)ptk->data;
        CL_LOG("cardId=%s \r\n",upCard->cardId);
        if(GlobalInfo.readCard_Callback)
        {
            GlobalInfo.readCard_Callback(3,(uint8_t*)upCard->cardId);
        }

        //send ack
        CB_RESULE_ACK_t *pACK = (void*)ptk->data;
        memset(pACK,0,sizeof(CB_RESULE_ACK_t));
        pACK->result = 0;
        App_CB_SendData(ptk,sizeof(CB_RESULE_ACK_t)); 
    }
    else if(cmd == 0x02)//д��
    {
        CB_RESULE_ACK_t *pRet = (void*)ptk->data;
        if(pRet->result == 0)
        {
            //todo
        }
    }
}


//Զ������
void App_CB_Handle_UpgradeInfo(CB_STR_t *ptk)
{
    uint8_t cmd = ptk->head.cmd;
    if(cmd == 0x01) //��ʼ����
    {
        CB_RESULE_ACK_t *pRet = (void*)ptk->data;
        if(pRet->result == 0)
        {
            BswSrv_Upgrade_SendNotify(0);
        }
    }
    else if(cmd == 0x02)//�̼��·�
    {
        CB_DOWN_FW_ACK_t *FWAck = (void*)ptk->data;
        if(FWAck->result == 0)
        {
            BswSrv_Upgrade_SendNotify(FWAck->index);
        }        
    }
}


void App_CBFrameHandle(uint8_t *data,uint16_t len)
{
    // PrintfData("recv cardBoard data:",data,len);
    CB_STR_t *ptk = (void*)data;

    switch (ptk->head.module)
    {
        case MODUL_BASE:    //������Ϣ
            App_CB_Handle_BaseInfo(ptk);
            break;
        case MODUL_CARD:    //������
            App_CB_Handle_CardInfo(ptk);
            break;
      case MODUL_UPGRADE:   //�̼�����
            App_CB_Handle_UpgradeInfo(ptk);
            break;
        default:
            CL_LOG("no find module..\r\n");
            break;
    }
}






