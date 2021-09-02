/****************************************************************************/
/* 	File    	BSP_CAN.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_CAN_H__
#define __BSP_CAN_H__
#include <stdint.h>

#define CAN_BAUD            250000                          //250K
#define CAN_DATA_PRESCALER  (SystemCoreClock/16)/CAN_BAUD   //250K (NominalTimeSeg1 + NominalTimeSeg2 + 1 =16)

#define SOLID_RX_ID         (0x1130UL)	//���յ�Ŀ���ַ����Ի�������λ��   
#define SOLID_TX_ID 	    (0x3011UL)	//��ص�Դ��ַ����Ի�������λ��

#define EX_MODU_RX_ID	    (0x1120UL)  //���յ�Ŀ���ַ���������ģ�� 
#define EX_MODU_TX_ID	    (0x2011UL)  //��ص�Դ��ַ���������ģ��

#define VARI_RX_ID	        (0xFF30UL)  //���յ�Ŀ���ַ�����ID�ɱ��豸 ��λ�̶� 0x--30  VARI_RX_ID VARI_RX_ID
#define VARI_TX_ID	        (0xFFFFUL)  //��ص�Դ��ַ�����ID�ɱ��豸

//#define DEST_ADDRESS_CAN1 0x1111UL
//#define DEST_ADDRESS_CAN2 0x1130UL	//���͵�Ŀ���ַ
//void MX_FDCAN1_Init(void);
//void MX_FDCAN2_Init(void);

/*****************************************************
	* ��������: CAN�豸 ��Դ ��ʼ��
	* �������: void
	* �� �� ֵ: void
	* ˵    ����
******************************************************/
void BSPCan_Init(void);

/*****************************************************
	* ��������: CAN�豸 ��Դ �ض�
	* �������: void
	* �� �� ֵ: void
	* ˵    ����
******************************************************/
void BSPCan_DeInit(void);

/*****************************************************
	* ��������: CAN�豸 ��Դ ��λ
	* �������: void
	* �� �� ֵ: void
	* ˵    ����
******************************************************/
void BSPCan_Reset(void);

/*****************************************************
	* ��������: CAN2���ݷ���
    * �������: ֡ID exfid	| ���ݵ�ַpbuff	|���ݳ���len
	* �� �� ֵ: uint8_t 0 Ϊ���ͳɹ�	��0��ʾ����ʧ��
	* ˵    �������ͳ�ʱ������
******************************************************/
uint8_t SendCANData(unsigned int exfid, unsigned char * pbuff , unsigned int len );

//=============================================================================================
//��������	: u32 FlexCanChangeRx3Canid(u8 id)
//��������	:
//�������	: ���ĺ��CAN ID
//��̬����	:
//��	��	: ���ӽ���ID
//ע	��	:
//=============================================================================================
uint32_t FlexCanChangeRx3Canid(uint8_t id);
#endif /* __BSP_CAN_H__ */

