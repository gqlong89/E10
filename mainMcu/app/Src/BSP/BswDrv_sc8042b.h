#ifndef __SC8042B_H__
#define __SC8042B_H__

#define AU_POWER_EN()   gpio_bit_set(GPIOC,GPIO_PIN_6)
#define AU_POWER_DIS()  gpio_bit_reset(GPIOC,GPIO_PIN_6)

#define AU_DATA_HIHG()  gpio_bit_set(GPIOB,GPIO_PIN_2)
#define AU_DATA_LOW()   gpio_bit_reset(GPIOB,GPIO_PIN_2)

#define AU_RST_EN()     gpio_bit_set(GPIOB,GPIO_PIN_0)
#define AU_RST_DIS()    gpio_bit_reset(GPIOB,GPIO_PIN_0)

#define READ_AU_BUSY()  gpio_input_bit_get(GPIOB,GPIO_Pin_1)


typedef enum{
	VOIC_NULL1=1,							//��
	VOIC_WELCOME=2,							//��ӭʹ��
	VOIC_NULL2=3,							//��
	VOIC_SHARE_CHARGE=4,					//������
	VOIC_START_UPGRADE=5,                   //��ʼ����
	VOIC_READING_CARD=6,               		//���ڶ���
	VOIC_DEVICE_EXCEPTION=7,				//�豸����
	VOIC_CARD_BANLANCE=8,					//��Ƭ���
	VOIC_CARD_RECHARGER=9,					//������,���ֵ
	VOIC_CARD_INVALID=10,					//��Ƭ��Ч
	VOIC_CARD_CHARGING=11,					//��İ������ڳ��
	VOIC_INPUT_SOCKET_NUM=12,				//������������,��ȷ�ϼ�
	VOIC_INPUT_CHARGRE_MONEY=13,			//����������,��ȷ�ϼ�.���践����һ��,�밴���ؼ�
	VOIC_SOCKET_OCCUPID=14,					//������ռ��,��ѡ����������
	VOIC_SOCKET_ERROR=15,					//��������,��ѡ����������
	VOIC_SOCKET_NUM_INVALID=16,				//���������Ч,����������
	VOIC_NIN_YI_XUANZE=17,					//����ѡ��
	VOIC_HAO_CHAZUO=18,						//�Ų���
	VOIC_CHARGER_MONEY=19,					//�����
	VOIC_YUAN=20,							//Ԫ
	VOIC_VERIFIED_PLUG=21,					//��ȷ�ϳ������ͳ��˲������Ѳ��
	VOIC_CARD_BANLANCE_INSUFFICIENT=22,		//����,����������
	VOIC_START_CHARGING=23,					//��ʼ���
	VOIC_STOP_CHARGER_TIP=24,				//����������,�ɰε���ͷ
	VOIC_PLUG_IN_PLUG=25,					//�뽫��ͷ����
	VOIC_CARD=26,                          	//�֣�ˢ������
	VOIC_KEY=27,                           	//�⣨���Ҽ���������
	VOIC_BLUETOOTH_ONLINE=28,				//����������
	VOIC_BLUETOOTH_OFFLINE=29,				//�����ѶϿ�
	VOIC_ERROR=30,							//����
	VOIC_0=31,                             	//0 (����0)
    VOIC_1=32,                             	//1.
    VOIC_2=33,                             	//2.
    VOIC_3=34,                             	//3.
    VOIC_4=35,                             	//4.
    VOIC_5=36,                             	//5.
    VOIC_6=37,                             	//6.
    VOIC_7=38,                            	//7.
    VOIC_8=39,                             	//8.
    VOIC_9=40,                             	//9
	VOIC_TEST_TIP=41,						//1��׮��,2��ʶ����,3�̵�������
	VOIC_INPUT_SOCKET_TEST=42,				//������Ű�ȷ������
	VOIC_HUNDRED=43,						//��
	VOIC_TEN=44,							//ʮ
	VOIC_YUAN1=45,							//Ԫ
	VOIC_POINT=46,							//��
	VOIC_DEVICE_REBOOT=47,					//�豸����
	VOIC_SUCCESS=48,						//�ɹ�
	VOIC_A=49,								//a
	VOIC_B=50,								//b
	VOIC_C=51,								//c
	VOIC_D=52,								//d
	VOIC_E=53,								//e
	VOIC_F=54,								//f
	VOIC_TEM=55,							//�¶�
	VOIC_DEGREE=56,							//��
	VOIC_THOUSAND=57,						//ǧ
	VOIC_START_CHARGER_FAIL=58,				//�������ʧ��
	VOIC_POWER_TOO_LARGE=59,                //�豸���ʹ����޷����
	VOICE_NUM,
}SC8042B_VOICE_TYPE;

void BswDrv_Sc8042bSpeech(SC8042B_VOICE_TYPE cnt);
void BswDrv_SC8042B_Init(void);


#define Sc8042bSpeech	BswDrv_Sc8042bSpeech

#endif

