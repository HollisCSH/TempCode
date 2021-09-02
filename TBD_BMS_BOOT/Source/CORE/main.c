//=======================================Copyright(c)==========================================
// 								  ���������Ƽ����޹�˾
//---------------------------------------�ļ���Ϣ----------------------------------------------
//�ļ���   	: main.c
//������  	: Hollis
//��������	:
//����	    : ������
//-----------------------------------------------��ǰ�汾�޶�----------------------------------
//�޸���   	:
//�汾	    :
//�޸�����	:
//����	    :
//=============================================================================================

//=============================================================================================
//ͷ�ļ�����
//=============================================================================================
#include "main.h"
#include "BSP_CAN.h"
#include "BSP_GPIO.h"
#include "BSP_IWDG.h"
#include "BSP_SYSCLK.h"
#include "BootLoader.h"
#include "CanComm.h"
//#define 	BOOT_DIRECT_JUMP		    //ֱ����ת��ȡ��boot���飬�������
#ifdef DEBUG
#include "BSP_UART.h"
#include "cm_backtrace.h"
#endif

int main(void)
{
    HAL_Init();							//HAL���ʼ��
    SystemClock_Config();				//ϵͳʱ�ӳ�ʼ��	
	#ifdef BOOT_DIRECT_JUMP
    JumpToApplication();			    //ֱ����ת�汾boot
	#endif	
    
    #ifdef DEBUG
	BSPUART_Init(UART_WIFI_DEBUG,UART_WIFI_DEBUG_BAUD);	//WiFi���� + ���Կ�
    _Debug_Printf("BMS Boot Start,Bulid Time:%s %s\r\n", __DATE__, __TIME__);
	cm_backtrace_init("TBD_BMS_BOOT", "1.0", "1.0");
    #endif   
    
	BootloaderInit();                   //BootLoader��ʼ�� ���APP������ֱ����ת ����ִ�����´���
	BSPGPIOInit();	                    //GPIO��ʼ��	
	MX_FDCAN2_Init();					//CAN��
	MX_IWDG_Init(4095);					//���Ź���ʼ��	
	INT_SYS_EnableIRQGlobal();          //���ж�	
	CanCommInit();                      //CAN���ݴ洢��ʼ��
	CAN_POWER_ENABLE();                 //��ص�Դʹ��
    HAL_Delay(10);
    while (1)
    {
        BootLoaderDealTask();           //BootLoader��������
    }
}

/************************ (C) COPYRIGHT IMMOTOR *****END OF FILE****/
