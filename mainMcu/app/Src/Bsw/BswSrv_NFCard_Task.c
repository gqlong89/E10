/*
 * @Author: zhoumin 
 * @Date: 2018-10-18 15:10:28 
 * @def: �������Ͱ�������
 * @Last Modified by: zhoumin
 * @Last Modified time: 2018-10-19 17:32:35
 */

#include "BswSrv_NFCard_Task.h"
#include "BswSrv_System.h"
#include "BswDrv_Usart.h"
#include "BswDrv_Debug.h"
#include "BswDrv_nfc.h"
#include "BswSrv_Aes.h"
#include "BswDrv_Rtc.h"
#include "BswDrv_sc8042b.h"


#define USER_NFC_CARD     			1

//������
#define SECTOR_MAIN					12
#define BLOCK_MAIN					49

//��������
#define	SECTOR_BAK					2
#define BLOCK_BAK					9


/*���뿨Ĭ����֤����KEYA*/
static unsigned char SecretCardKEY_A[6] = {'6', 'f', '7', 'd', '2', 'k'};
/*AES������Կ*/
static unsigned char SeedKeyA[16]="chargerlink1234";

/*������־1-���� 0-��ֹ����*/
static uint8_t readCardFlag = 1;
/*�ϴζ�����ʱ�䣬5�ڲ����ظ�����*/
static uint32_t readCardTime = 0;






//��������
void Check_M1_Card(void)
{
	uint8_t PICC_ATQA[2],PICC_SAK[3],PICC_UID[4];
	
	if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
	{
		readCardFlag = 0;
		readCardTime = osGetTimes();
		// PrintfData("PICC_UID:",PICC_UID,4);
        // PrintfData("PICC_ATQA1:",PICC_ATQA,2);
		//��Կ��
		if(Mifare_Auth(0,2,SecretCardKEY_A,PICC_UID) == OK)
        {
			CL_LOG("this is SecretCard.\r\n");
			Sc8042bSpeech(VOIC_CARD);
		}
        else
		{	//����Կ��
			uint8_t ucardkeyA[16]={0};
			AES_KEY aes;
			memset(&aes,0,sizeof(AES_KEY));
			AES_set_encrypt_key(SeedKeyA, 128, &aes);
            
			uint8_t card[16] ;
			memset(card,0,16);
			memcpy(card,PICC_UID,4);
			AES_encrypt(card, ucardkeyA, &aes);
            
			if((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
            {
				uint8_t result = 0;
				uint8_t block_data[16];
				memset((void*)block_data,0,16);
			    // PrintfData("PICC_ATQA2:",PICC_ATQA,2);

				//������������֤
				if(Mifare_Auth(0,SECTOR_MAIN,ucardkeyA,PICC_UID) == OK)
				{
					if(Mifare_Blockread(BLOCK_MAIN,block_data) == OK) 
					{
						result = 1;
					}
				}
                else if(Mifare_Auth(0,SECTOR_BAK,ucardkeyA,PICC_UID) == OK)//��������������֤
				{	
					if(Mifare_Blockread(BLOCK_BAK,block_data) == OK)
					{
						result = 1;
					}
				}
				else
				{
					CL_LOG("��Ƭ������֤ʧ��...\r\n");
					Sc8042bSpeech(VOIC_CARD_INVALID);
				}
				if(result == 1)
				{
					uint8_t cardId[16]={0,};
					sprintf((char*)cardId, "%2x%02x%02x%02x%02x",block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);
					// CL_LOG("block_data[4] = %02x ,block_data[5] = %02x ,block_data[6] = %02x ,block_data[7] = %02x ,block_data[8] = %02x .\n",
					// 	block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);
					if(GlobalInfo.readCard_Callback)
					{
						Sc8042bSpeech(VOIC_READING_CARD);
						GlobalInfo.readCard_Callback(3,cardId);
					}

				}
			}
		}
		TypeA_Halt();
	}
}

void ReadCardHandle(void)
{
	//��������
	Check_M1_Card();
}


void NFCardTask(void)
{
    uint32_t TimeTicks = 0;
    uint32_t authCardTicks = 0;

#if USER_NFC_CARD
	if(BswDrv_FM175XX_Init() == CL_OK)
	{
		GlobalInfo.card_state = 1;
		CL_LOG("FM175XX_Init OK.\r\n");
	}
	else
	{
		GlobalInfo.card_state = 0;
		CL_LOG("FM175XX_Init Error.\r\n");
	}
#endif

	while(1)
	{
		osDelay(100);

#if USER_NFC_CARD
		
		if(GlobalInfo.card_state == 0)/**���������**/
		{
			if(60000 <= (osGetTimes() - TimeTicks))
			{
				TimeTicks = osGetTimes();
				if(BswDrv_FM175XX_Init() == CL_OK)
				{
					GlobalInfo.card_state = 1;
					CL_LOG("FM175XX_Init OK.\r\n");
				}
				else
				{
					CL_LOG("FM175XX_Init Error.\r\n");
				}
			}
		}
		else/**Ѱ��*/
		{
			//Ѱ�� 500msѰһ�ο�
			if (readCardFlag == 1 && (500 <= (osGetTimes() - authCardTicks)))
			{
				authCardTicks = osGetTimes();
				ReadCardHandle();
			}
			//��������5s�ڲ���Ѱ��
			if(osGetTimes() - readCardTime >= 3000)
			{
				readCardTime = osGetTimes();
				readCardFlag = 1;
			}

			//��ʱ����ȡ�����Ƿ����� 60s���һ��
			if(60000 <= (osGetTimes() - TimeTicks))
			{
				TimeTicks = osGetTimes();
				if(BswDrv_FM175XX_Check() != CL_OK)
				{
					GlobalInfo.card_state = 0;
					CL_LOG("���������ʧ�ܣ�����ģ��...\r\n");
				}
			}
		}
	#endif

	}
}


