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

#define SOLID_RX_ID         (0x1130UL)	//接收的目标地址，针对换电柜或上位机   
#define SOLID_TX_ID 	    (0x3011UL)	//电池的源地址，针对换电柜或上位机

#define EX_MODU_RX_ID	    (0x1120UL)  //接收的目标地址，针对外置模块 
#define EX_MODU_TX_ID	    (0x2011UL)  //电池的源地址，针对外置模块

#define VARI_RX_ID	        (0xFF30UL)  //接收的目标地址，针对ID可变设备 低位固定 0x--30  VARI_RX_ID VARI_RX_ID
#define VARI_TX_ID	        (0xFFFFUL)  //电池的源地址，针对ID可变设备

//#define DEST_ADDRESS_CAN1 0x1111UL
//#define DEST_ADDRESS_CAN2 0x1130UL	//发送的目标地址
//void MX_FDCAN1_Init(void);
//void MX_FDCAN2_Init(void);

/*****************************************************
	* 函数功能: CAN设备 电源 初始化
	* 输入参数: void
	* 返 回 值: void
	* 说    明：
******************************************************/
void BSPCan_Init(void);

/*****************************************************
	* 函数功能: CAN设备 电源 关断
	* 输入参数: void
	* 返 回 值: void
	* 说    明：
******************************************************/
void BSPCan_DeInit(void);

/*****************************************************
	* 函数功能: CAN设备 电源 复位
	* 输入参数: void
	* 返 回 值: void
	* 说    明：
******************************************************/
void BSPCan_Reset(void);

/*****************************************************
	* 函数功能: CAN2数据发送
    * 输入参数: 帧ID exfid	| 数据地址pbuff	|数据长度len
	* 返 回 值: uint8_t 0 为发送成功	非0表示发送失败
	* 说    明：发送超时需配置
******************************************************/
uint8_t SendCANData(unsigned int exfid, unsigned char * pbuff , unsigned int len );

//=============================================================================================
//函数名称	: u32 FlexCanChangeRx3Canid(u8 id)
//函数参数	:
//输出参数	: 更改后的CAN ID
//静态变量	:
//功	能	: 增加接收ID
//注	意	:
//=============================================================================================
uint32_t FlexCanChangeRx3Canid(uint8_t id);
#endif /* __BSP_CAN_H__ */

