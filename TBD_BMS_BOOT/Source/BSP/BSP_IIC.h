/****************************************************************************/
/* 	File    	BSP_IIC.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_IIC_H__
#define __BSP_IIC_H__

#include <stdint.h>
#include "main.h"
#define AFE_IIC_CH		I2C1
#define ACC_IIC_CH		I2C2
#define I2C_ADDRESS 0xAE
//#define I2C_ADDRESS 0x34
#define		SH367309_WRITE_ADDR			0x34	//读取8位地址
#define		SH367309_READ_ADDR			SH367309_WRITE_ADDR | 0x01

#define		DA213_WRITE_ADDR			0X4e	//读取8位地址
#define		DA213_READ_ADDR				DA213_WRITE_ADDR | 0x01

#define REG_ADDRESS 0x03B0

void MX_I2C1_Init(void);
void MX_I2C2_Init(void);
void MX_I2C1_REAR(uint8_t *pData, uint16_t Size, uint32_t Timeout);
void MX_I2C1_WRITE(uint8_t *pData, uint16_t Size, uint32_t Timeout);
//void I2C_REAR( uint32_t RegAdress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
//void I2C_WRITE(uint32_t RegAdress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
//void I2C_REAR_ADDLEN(uint32_t RegAdress, uint8_t *pData, uint16_t Size, uint32_t Timeout);


void I2C_WRITE(I2C_TypeDef *hi2cx ,uint32_t RegAdress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
void I2C_REAR(I2C_TypeDef *hi2cx , uint32_t RegAdress, uint8_t *pData, uint16_t Size, uint32_t Timeout);

#endif /* __BSP_IIC_H__ */
