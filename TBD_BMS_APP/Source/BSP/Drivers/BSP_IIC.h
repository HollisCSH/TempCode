/****************************************************************************/
/* 	File    	BSP_IIC.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_IIC_H__
#define __BSP_IIC_H__
#include <stdint.h>
#include "main.h"
#include "DA213.h"
#include "SH367309.h"

//#define SH367309_IIC_CHANNEL		I2C1	
//#define DA213_IIC_CHANNEL				I2C2

//#define		SH367309_WRITE_ADDR			0x34	//��ȡ8λ��ַ
//#define		SH367309_READ_ADDR			SH367309_WRITE_ADDR | 0x01
//#define		DA213_WRITE_ADDR			0X4e	//��ȡ8λ��ַ
//#define		DA213_READ_ADDR				DA213_WRITE_ADDR | 0x01

//#define I2C_ADDRESS 0xAE
//#define REG_ADDRESS 0x03B0

typedef enum
{
	IIC_Sta_Rd_Busy = 2,	//æ��
    IIC_Sta_Wr_Busy = 1,	//æд
    IIC_Sta_Done 	= 0,	//����״̬
    IIC_Sta_ERR 	= -1,	//ͨѶ�쳣
}t_IIC_STA;
//typedef struct
//{
//    unsigned char readtime ;
//    unsigned char writetime ;
//}t_IIC_COMM_TIME;

//extern t_IIC_COMM_TIME gAFE_IIC_COMM_TIME ;
//extern t_IIC_COMM_TIME gACC_IIC_COMM_TIME ;

void  IICTickHandler(void);

/*****************************************************
	* ��������: IIC��ʼ��
	* �������: void
	* �� �� ֵ: void
	* ˵    ���������ⲿ������ĺ�����
******************************************************/
void BSPI2C_Init(I2C_TypeDef *I2cHandle);

/*****************************************************
	* ��������: IIC�ض�
	* �������: void
	* �� �� ֵ: void
	* ˵    ���������ⲿ������ĺ�����
******************************************************/
void BSPI2C_DeInit(I2C_TypeDef *I2cHandle);


/*****************************************************
	* ��������: IICд��Ĵ�������
	* �������: *hi2c , �Ĵ�����ַ RegAdress, ���ָ�� *pData, ���ݳ��� Size
	* �� �� ֵ: ����0���óɹ�
	* ˵    �����жϷ��ͣ����óɹ��Ƿ��ͳɹ����ȷ�������ж�
******************************************************/
uint8_t BSP_I2C_MEM_WRITE(I2C_TypeDef *hi2cx ,uint32_t RegAdress, uint8_t *pData, uint16_t Size);

/*****************************************************
	* ��������: IIC��ȡ�Ĵ�������
	* �������: *hi2c , �Ĵ�����ַ RegAdress, ���ָ�� *pData, ���ݳ��� Size
	* �� �� ֵ: ����0���óɹ�
	* ˵    �����жϷ��ͣ����óɹ��Ƿ��ͳɹ����ȷ�������ж�
******************************************************/
uint8_t BSP_I2C_MEM_REAR(I2C_TypeDef *hi2cx , uint32_t RegAdress, uint8_t *pData, uint16_t Size);

//=============================================================================================
//��������	: I2C_TransferReturn_TypeDef BSPIICChannalStateGet(uint8_t ch)
//�������	: ch: IIC ͨ�����
//�������	: IIC�շ�״̬
//��̬����	: void
//��	��	: ��ѯIICͨ���շ�״̬
//ע	��	:
//=============================================================================================
int8_t BSPIIC_StateGet(I2C_TypeDef *hi2cx);//��ѯ��ǰIIC��״̬

//=============================================================================================
//��������	: uint32_t BSPIICRead( uint8_t ch, uint16_t addr, uint8_t *pstr, uint16_t cnt)
//�������	: ch: IIC ͨ�����;addr: IIC�ӻ���ַ;pstr: ���ջ���ָ��;cnt: ���յ��ֽ���
//�������	: 0
//��̬����	: void
//��	��	: IIC��ȡ
//ע	��	:
//=============================================================================================
uint32_t BSPIICRead( I2C_TypeDef *hi2cx, uint16_t addr, uint8_t *pstr, uint16_t cnt);

//=============================================================================================
//��������	: uint32_t BSPIICWrite(uint8_t ch, uint16_t addr, uint8_t *pstr, uint16_t cnt)
//�������	: ch: IIC ͨ�����;addr: IIC�ӻ���ַ;pstr: ���ͻ���ָ��;cnt: ���͵��ֽ���
//�������	: 0
//��̬����	: void
//��	��	: IIC����
//ע	��	:
//=============================================================================================
uint32_t BSPIICWrite(I2C_TypeDef *hi2cx, uint16_t addr, uint8_t *pstr, uint16_t cnt);

//=============================================================================================
//��������	: uint32_t BSPIICWriteAndRead(uint8_t ch, uint16_t addr, uint8_t *pwstr, uint16_t wcnt, uint8_t *prstr, uint16_t rcnt)
//�������	: ch: IIC ͨ�����;addr: IIC�ӻ���ַ;pwstr: ���ͻ���ָ��;wcnt: ���͵��ֽ���;prstr: ���ջ���ָ��;rcnt: ���յ��ֽ���
//�������	: 0
//��̬����	: void
//��	��	: IIC�ȷ��ͺ�restart �ٶ�
//ע	��	:
//=============================================================================================
uint32_t BSPIICWriteAndRead(I2C_TypeDef *hi2cx, uint16_t addr, uint8_t *pwstr, uint16_t wcnt, uint8_t *prstr, uint16_t rcnt);

#endif /* __BSP_IIC_H__ */

