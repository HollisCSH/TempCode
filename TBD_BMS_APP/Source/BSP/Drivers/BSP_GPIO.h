/****************************************************************************/
/* 	File    	BSP_GPIO.h 			 
 * 	Author		Hollis
 *	Notes		MCUʹ�õ����йܽ�  λ�ڴ˴�����
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__
#include <stdint.h>
#include "main.h"
//��ʱ��Ŷ���
//#define		HALL_CHECK_SHIFT			0X01
//#define		REMOVE_CHECK_SHIFT			0X02
//#define		PRE_DCHG_CHECK_SHIFT		0X04
//#define		EXTERN_MODULE_CHECK_SHIFT	0X08    //����ģ��
//#define		MAX1720X_FULLCHG_FLAG		0X0080      //�����־λ��
//#define 	STATE_HALL 		            0x0008

////////////////////////////////////////////////////////////////////////////////////////
/*����Ϊ��ͨIO��***********************************************************************/
/*GPIOA OUT*/
#define	CAN_5V_PORT 		GPIOA			//CANͨѶԭ�ߵ�Դ����ʹ��		PA1
#define	CAN_5V_PIN 			GPIO_PIN_1		//�ߵ�ƽ��Ч �ܽ���������ʵ��ΪCAN3.3��Դ����  CAN5V�� PWR_EN_14V_5V_PIN ����

#define	CTL_AFE_PORT 		GPIOA			//AFE��ŵ����ȿ��ƶ�			PA3
#define	CTL_AFE_PIN			GPIO_PIN_3 		//����֮��ǿ�ƹرճ�ŵ�MOS

#define	SH367309_ON_PORT 	GPIOA			//AFE����ģʽѡ�񿪹�			PA4
#define	SH367309_ON_PIN 	GPIO_PIN_4 		//Ĭ�����ߣ�����֮��AFE�������ģʽ������2uA��

#define	VCC_4G_CTL_PORT 	GPIOA			//����ģ�鹩�����				PA8
#define	VCC_4G_CTL_PIN 		GPIO_PIN_8 		//���ߺ���ڲ�ģ�鹩�磬���ͺ�رո��ڲ�ģ��Ĺ���

#define	PRE_DSG_EN_PORT 	GPIOA			//Ԥ��MOS������Դʹ��			PA10
#define	PRE_DSG_EN_PIN 		GPIO_PIN_10 	//Ԥ�ŵ翪�أ�Ĭ������

#define	FLASH_CS_PORT 		GPIOA			//FLASHоƬCS���ƶ�				PA15
#define	FLASH_CS_PIN 		GPIO_PIN_15 	//ͨѶ֮ǰ������  ����������

/*GPIOB OUT*/	
#define	PRE_DSG_NTC_PORT 	GPIOB			//Ԥ��NTC����·���ƶ˿�		PB1
#define	PRE_DSG_NTC_PIN 	GPIO_PIN_1		//����Ԥ��ǰ�����ͣ����Ԥ�ŵ������¶ȵ����趨ֵ֮��ſ��Կ���Ԥ�ţ�Ԥ�ŵ��ڼ�Ҳ����ʹ���¶ȼ�⣻��Ԥ��ʱ����

#define	MOS_TVS_NTC_PORT 	GPIOB			//NTC����·���ƶ˿�			PB2
#define	MOS_TVS_NTC_PIN 	GPIO_PIN_2		//TVS��MOS�˵�NTC�����ƣ�������Ч

/*GPIOC OUT*/
#define	FLASH_POW_EN_PORT 	GPIOC			//Flash����ʹ��					PC8
#define	FLASH_POW_EN_PIN 	GPIO_PIN_8 		//Flash���磬����дFlashʱĬ������ߵ�ƽ

#define	VPRO_AFE_PORT 		GPIOC			//AFE������дʹ�ܿ������		PC11
#define	VPRO_AFE_PIN 		GPIO_PIN_11		//Ĭ�����ͣ���AFEдEEPROM����ʱ��������

#define	PWR_EN_14V_5V_PORT 	GPIOC			//DC/DCʹ�ܿ���					PC13
#define	PWR_EN_14V_5V_PIN	GPIO_PIN_13 	//DCDC��ʹ�����ţ�MCU���Ѻ��������ߣ�������ǰ��ʱ100mS���͡�14V

/*GPIOD OUT*/
#define	DSG_EN_PORT 		GPIOD			//�ŵ�������ضϿ���			PD2
#define	DSG_EN_PIN 			GPIO_PIN_2 		//�ŵ���������ٿ���

#define	FUSE_DET_EN_PORT 	GPIOD			//����˿�۶ϼ���·ʹ��		PD5
#define	FUSE_DET_EN_PIN 	GPIO_PIN_5 		//Ĭ�����ͣ����FUSE״̬ʱ����

#define	VCC_MODULE_PORT 	GPIOD			//�ⲿģ�鹩��ʹ��				PD8
#define	VCC_MODULE_PIN 		GPIO_PIN_8 		//�ⲿ4Gͨ�Ź���,����P+��ѹ���ߵ�ƽʹ��


/*GPIOD IN*/
#define	MODULE_SHORT_PORT 	GPIOA			//ģ���·�������				PA9
#define	MODULE_SHORT_PIN 	GPIO_PIN_9 	  	//�ⲿģ���·Ϊ�ߵ�ƽ���ⲿģ������Ϊ�͵�ƽ��Ӳ����ضϣ�ʹ��IO��ѯ

#define	HW_REV0_PORT 		GPIOD			//Ӳ���汾��ʶ��1				PD0
#define	HW_REV0_PIN 		GPIO_PIN_0 		//

#define	HW_REV1_PORT 		GPIOD			//Ӳ���汾��ʶ��2				PD1		
#define	HW_REV1_PIN 		GPIO_PIN_1 		//

#define	FUSE_DET_PORT 		GPIOD			//����˿�۶ϼ��				PD4
#define	FUSE_DET_PIN 		GPIO_PIN_4 		//FUSE�쳣ʱ��⵽�ߵ�ƽ��FUSE����ʱ��⵽�͵�ƽ

//#define CAN_POWER_ENABLE()	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_SET)
//#define CAN_POWER_DISABLE()	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_RESET)
/*����Ϊ��ͨIO��***********************************************************************/
////////////////////////////////////////////////////////////////////////////////////////
/*����Ϊ�ж�����IO*********************************************************************/
#define	ALARM_AFE_PORT 			GPIOA		//AFE�澯�ź�����					PA0
#define	ALARM_AFE_PIN 			GPIO_PIN_0 	//�ж��źţ��͵�ƽ���壬AFE������������ɲɼ����½�����
#define	REMOVE_INT_PORT 		GPIOA		//�Ƴ����								PA2
#define	REMOVE_INT_PIN 			GPIO_PIN_2 	//����ʱΪ�ߵ�ƽ����ر��𿪼����Ϊ�͵�ƽ���½�����
#define	MODULE_INT_PORT 		GPIOA		//4G����ģ���ж�					PA11
#define	MODULE_INT_PIN 			GPIO_PIN_11 //�ⲿģ���������͵�ƽ�źţ�δ����Ϊ�ߵ�ƽ���½�����

#define	ACC_INT1_PORT 			GPIOB		//������ټ��ж�1					PB8
#define	ACC_INT1_PIN 			GPIO_PIN_8 	//���ټ��ж��ź�1�������ش���

#define	INT_4G_PORT 			GPIOC		//�ڲ�����ģ���ж�					PC6
#define	INT_4G_PIN 				GPIO_PIN_6 	//�ڲ�4Gģ����룬�͵�ƽ��ʾ���룬�½�����
#define	ACC_INT2_PORT 			GPIOC		//������ټ��ж�2					PC10
#define	ACC_INT2_PIN 			GPIO_PIN_10 //���ټ��ж��ź�2�������ش���

/*����Ϊ��ͨIO��***********************************************************************/
////////////////////////////////////////////////////////////////////////////////////////
/*����Ϊģ������IO*********************************************************************/
#define TVS_NTC_PIN 			GPIO_PIN_5	//���TVS���¶�						PA5		
#define MOS_NTC_PIN 			GPIO_PIN_6	//���MOS���¶�						PA6		
#define PRE_NTC_PIN 			GPIO_PIN_7	//���Ԥ�������¶�				PA7		
#define NTC_PORT 				GPIOA

#define PRE_DSG_SHORT_DET_PIN 	GPIO_PIN_0	//Ԥ��/��·�������					PB0
#define PRE_DSG_SHORT_DET_PORT 	GPIOB		//Ԥ�ŵ��·��⣬ADֵ���� 

/*����Ϊģ������IO*********************************************************************/
////////////////////////////////////////////////////////////////////////////////////////
/*����ΪͨѶ��IO***********************************************************************/
/*IIC*/
#define	ACC_IIC1_PORT 			GPIOB		//ACCͨѶ��IIC						IIC1��DA213оƬ��
#define	ACC_IIC1_SCL 			GPIO_PIN_13 //ACCͨѶ��ʱ�ӿ�					PB13
#define	ACC_IIC1_SDA 			GPIO_PIN_14 //ACCͨѶ�����ݿ�					PB14

#define	AFE_IIC2_PORT 			GPIOB		//AFE��ͨѶIIC						IIC2��SH367309оƬ��
#define	AFE_IIC2_SCL 			GPIO_PIN_6 	//AFEͨѶ��ʱ�ӿ�					PB6
#define	AFE_IIC2_SDA 			GPIO_PIN_7 	//AFEͨѶ�����ݿ�					PB7

/*SPI*/
#define	FLASH_SPI1_PORT 		GPIOB		//FLASHͨѶ��SPI					SPI1��MX25V1606оƬ CSΪPA15��
#define	FLASH_SPI1_CLK 			GPIO_PIN_3 	//FLASHͨѶ��ʱ�ӿ�					PB3
#define	FLASH_SPI1_MISO 		GPIO_PIN_4 	//FLASHͨѶ�����ݿ�					PB4
#define	FLASH_SPI1_MOSI 		GPIO_PIN_5 	//FLASHͨѶ�����ݿ�					PB5

/*CAN*/
#define	CAN2_PORT 				GPIOC		//CANͨѶ��							CAN2��NSI1050-DSPRоƬ��
#define	CAN2_RX 				GPIO_PIN_2 	//CANͨѶ�����ݽ���					PC2
#define	CAN2_TX 				GPIO_PIN_3 	//CANͨѶ�����ݷ���					PC3

/*UART*/
#define	GPS_UASRT1_PORT 		GPIOC		//4G����ͨѶ						UASRT1
#define GPS_UASRT1_TX 			GPIO_PIN_4 	//4G����ͨѶ���ݷ���				PC4
#define	GPS_UASRT1_RX 			GPIO_PIN_5 	//4G����ͨѶ���ݽ���				PC5

#define	WIFI_LPUART1_PORT 		GPIOB		//WIFI����ͨѶ						LPUART1
#define WIFI_LPUART1_RX 		GPIO_PIN_10 //WIFI����ͨѶ���ݽ���				PC10
#define	WIFI_LPUART1_TX 		GPIO_PIN_11 //WIFI����ͨѶ���ݷ���				PC11

/*����ΪͨѶ��IO***********************************************************************/
////////////////////////////////////////////////////////////////////////////////////////
/*������������*************************************************************************/
//#define MCU_RST 	PF2		//��λ��
//#define SWDIO 		PA13	//DEBUG�������������
//#define SWCLK 		PA14	//DEBUG����ͨѶʱ��		ͬʱҲ��boot0 ����֮��ɽ�MCU����������FLASH
/**************************************************************************************/

#define PCB_TVS_NTC_SAMP_ON		HAL_GPIO_WritePin(MOS_TVS_NTC_PORT, MOS_TVS_NTC_PIN, GPIO_PIN_RESET);
#define PCB_TVS_NTC_SAMP_OFF	HAL_GPIO_WritePin(MOS_TVS_NTC_PORT, MOS_TVS_NTC_PIN, GPIO_PIN_SET);

#define PRE_DSG_NTC_SAMP_ON   	HAL_GPIO_WritePin(PRE_DSG_NTC_PORT, PRE_DSG_NTC_PIN, GPIO_PIN_RESET); 	//����Ԥ�ŵ� NTC�������ƣ���Ԥ�ŵ�����¶�
#define PRE_DSG_NTC_SAMP_OFF  	HAL_GPIO_WritePin(PRE_DSG_NTC_PORT, PRE_DSG_NTC_PIN, GPIO_PIN_SET); 	//�ر�Ԥ�ŵ� NTC��������

void MX_GPIO_Init(void);
void BSPGPIOInit(void);
void BSPGPIODeInit(void);

void GPIO_DefaultInit(void);
void CAN_3V_ENABLE(void);
void CAN_3V_DISABLE(void);
void FLASH_POWER_ENABLE(void);
void FLASH_POWER_DISABLE(void);
void FLASH_CS_ENABLE(void);
void FLASH_CS_DISABLE(void);
void PWR_14V_5V_ENABLE(void);
void PWR_14V_5V_DISABLE(void);

void GPIO_SET_INPUT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void GPIO_SET_RISE_EXIT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
    
#endif /* __BSP_GPIO_H__ */

