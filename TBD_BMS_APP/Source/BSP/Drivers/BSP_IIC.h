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

//#define		SH367309_WRITE_ADDR			0x34	//读取8位地址
//#define		SH367309_READ_ADDR			SH367309_WRITE_ADDR | 0x01
//#define		DA213_WRITE_ADDR			0X4e	//读取8位地址
//#define		DA213_READ_ADDR				DA213_WRITE_ADDR | 0x01

//#define I2C_ADDRESS 0xAE
//#define REG_ADDRESS 0x03B0

typedef enum
{
	IIC_Sta_Rd_Busy = 2,	//忙读
    IIC_Sta_Wr_Busy = 1,	//忙写
    IIC_Sta_Done 	= 0,	//空闲状态
    IIC_Sta_ERR 	= -1,	//通讯异常
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
	* 函数功能: IIC初始化
	* 输入参数: void
	* 返 回 值: void
	* 说    明：兼容外部代码更改函数名
******************************************************/
void BSPI2C_Init(I2C_TypeDef *I2cHandle);

/*****************************************************
	* 函数功能: IIC关断
	* 输入参数: void
	* 返 回 值: void
	* 说    明：兼容外部代码更改函数名
******************************************************/
void BSPI2C_DeInit(I2C_TypeDef *I2cHandle);


/*****************************************************
	* 函数功能: IIC写入寄存器数据
	* 输入参数: *hi2c , 寄存器地址 RegAdress, 存放指针 *pData, 数据长度 Size
	* 返 回 值: 返回0设置成功
	* 说    明：中断发送，设置成功非发送成功，等发送完成中断
******************************************************/
uint8_t BSP_I2C_MEM_WRITE(I2C_TypeDef *hi2cx ,uint32_t RegAdress, uint8_t *pData, uint16_t Size);

/*****************************************************
	* 函数功能: IIC读取寄存器数据
	* 输入参数: *hi2c , 寄存器地址 RegAdress, 存放指针 *pData, 数据长度 Size
	* 返 回 值: 返回0设置成功
	* 说    明：中断发送，设置成功非发送成功，等发送完成中断
******************************************************/
uint8_t BSP_I2C_MEM_REAR(I2C_TypeDef *hi2cx , uint32_t RegAdress, uint8_t *pData, uint16_t Size);

//=============================================================================================
//函数名称	: I2C_TransferReturn_TypeDef BSPIICChannalStateGet(uint8_t ch)
//输入参数	: ch: IIC 通道编号
//输出参数	: IIC收发状态
//静态变量	: void
//功	能	: 查询IIC通道收发状态
//注	意	:
//=============================================================================================
int8_t BSPIIC_StateGet(I2C_TypeDef *hi2cx);//查询当前IIC的状态

//=============================================================================================
//函数名称	: uint32_t BSPIICRead( uint8_t ch, uint16_t addr, uint8_t *pstr, uint16_t cnt)
//输入参数	: ch: IIC 通道编号;addr: IIC从机地址;pstr: 接收缓存指针;cnt: 接收的字节数
//输出参数	: 0
//静态变量	: void
//功	能	: IIC读取
//注	意	:
//=============================================================================================
uint32_t BSPIICRead( I2C_TypeDef *hi2cx, uint16_t addr, uint8_t *pstr, uint16_t cnt);

//=============================================================================================
//函数名称	: uint32_t BSPIICWrite(uint8_t ch, uint16_t addr, uint8_t *pstr, uint16_t cnt)
//输入参数	: ch: IIC 通道编号;addr: IIC从机地址;pstr: 发送缓存指针;cnt: 发送的字节数
//输出参数	: 0
//静态变量	: void
//功	能	: IIC发送
//注	意	:
//=============================================================================================
uint32_t BSPIICWrite(I2C_TypeDef *hi2cx, uint16_t addr, uint8_t *pstr, uint16_t cnt);

//=============================================================================================
//函数名称	: uint32_t BSPIICWriteAndRead(uint8_t ch, uint16_t addr, uint8_t *pwstr, uint16_t wcnt, uint8_t *prstr, uint16_t rcnt)
//输入参数	: ch: IIC 通道编号;addr: IIC从机地址;pwstr: 发送缓存指针;wcnt: 发送的字节数;prstr: 接收缓存指针;rcnt: 接收的字节数
//输出参数	: 0
//静态变量	: void
//功	能	: IIC先发送后restart 再读
//注	意	:
//=============================================================================================
uint32_t BSPIICWriteAndRead(I2C_TypeDef *hi2cx, uint16_t addr, uint8_t *pwstr, uint16_t wcnt, uint8_t *prstr, uint16_t rcnt);

#endif /* __BSP_IIC_H__ */

