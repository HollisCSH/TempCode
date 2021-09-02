//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: MBServer.h
//创建人  	: Handry
//创建日期	: 
//描述	    : Modbus从机处理代码头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _MB_SERVER_H
#define _MB_SERVER_H

//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"
#include "Comm.h"
#include "CanComm.h"

//=============================================================================================
//宏定义
//=============================================================================================
//功能码
#define MODBUS_READ_HOLDREGISTERS  		0x03	 //读寄存器
#define MODBUS_WRITE_REGISTER           0x06	 //写单寄存器
#define MODBUS_WRITE_MULT_REG           0x10   	 //写多寄存器
#define	MODBUS_CUSTOMER_REG				0x46	 //读写用户寄存器，自定义，用于读取历史记录
#define	MODBUS_CERTIFICATE_REG			0x68	 //认证寄存器，用于电池加密认证

#define	MODBUS_NB_TEST_REG			    0x69	 //用于NB模块测试的功能码

#define	MODBUS_REQUEST_MOD_CANID		0xC4	 //申请更改CANID
//#define	MODBUS_RESPOND_MOD_CANID	0xC5	 //更改CANID回复
#define	MODBUS_FORCE_MOD_CANID	        0xC8	 //主机强制从机重新申请地址

#define	MODBUS_NO_FAULT					0x00	 //无异常
#define ILLEGAL_FUNCTION_CODE           0x01     //功能码错误
#define ILLEGAL_DATA_ADDRESS        	0x02     //地址错误
#define ILLEGAL_DATA_VALUE          	0x03     //请求数量错误
#define CHILD_DEVICE_MALFUNCTION        0x04     //从设备故障

#define	MODBUS_ONETIME_MAX_NUM			125		//modbus一次最大请求的16进制数据长度

//=============================================================================================
//函数名称	: void MBServerMsgProcess(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；
//输出参数	: 发送缓冲的数据长度
//静态变量	: void
//功    能	: Modbus作从机的处理函数
//注    意	:
//=============================================================================================
u8 MBServerMsgProcess(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg);

//=============================================================================================
//函数名称	: void MBServerMsgProcessEx(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；
//输出参数	: 发送缓冲的数据长度
//静态变量	: void
//功    能	: Modbus作从机的处理函数
//注    意	: 此处用于Can通信下调用
//=============================================================================================
u8 MBServerMsgProcessEx(t_Modbus_PDU *PmbRxMsg,t_Modbus_PDU *PmbTxMsg);

//===========================================================================
//函数名称	: u16 MbRtuCheckCode(u8 *chkbuf, u8 len)
//输入参数	: chkbuf: 指向需要验证的ModBus消息帧的指针
//			: len   : 要效验的Modbus消息帧字节数，不包含效验码CRC
//输出参数	: UINT16: 校验码CRC	
//静态变量	: 
//功	能	: 计算并返回ModBus RTU模式下消息的CRC效验码
//注	意	:  
//===========================================================================
u16 MbRtuCheckCode(u8 *chkbuf, u8 len);

#endif

/*****************************************end of MBServer.h*****************************************/
