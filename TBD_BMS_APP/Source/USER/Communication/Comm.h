//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Comm.h
//创建人  	: Handry
//创建日期	: 
//描述	    : 通信头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef __COMM_H
#define __COMM_H

//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"
#include "stm32g0xx_hal.h"

//=============================================================================================
//数据结构定义
//=============================================================================================
typedef struct
{
    u8 pcb;
    u8 cid;
    u8 nad;
    u8 addr;
    u8 funcode;
    u8 data[254];	//包含2字节crc
}t_MSG_PDU;

typedef struct
{
    u8  recflag;//收到一条轮询后就置1
    u16 timeout;
	u8  commoff;
}t_COMM_POLL;

//=============================================================================================
//宏参数设定
//=============================================================================================
#define 	COMM_CRC_A 		1
#define 	COMM_CRC_B 		2

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern t_COMM_POLL gCommPoll;

//=============================================================================================
//声明接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: u8 CommInit(void)
//输入参数	: void
//输出参数	: 初始化是否成功，TURE:成功  FALSE:失败
//静态变量	: void
//功    能	: 通信协议初始化函数
//注    意	:
//=============================================================================================
u8 CommInit(void);

//=============================================================================================
//函数名称	: void CommMainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 通信协议主任务函数
//注    意	:
//=============================================================================================
void CommMainTask(void *p);

//=============================================================================================
//函数名称	: u8 CommCompareTimeout(u16 timeout)
//输入参数	: timeout:超时时间
//输出参数	: void
//静态变量	: void
//功    能	: 通信协议检测超时时间
//注    意	:
//=============================================================================================
u8 CommCompareTimeout(u16 timeout);

//=============================================================================================
//函数名称	: void CommCheckTimeout(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 通信协议断线超时任务
//注    意	:
//=============================================================================================
void CommCheckOffTimeout(void);

//=============================================================================================
//函数名称	: void CommClearTimeout(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 通信协议清零超时计数器
//注    意	:
//=============================================================================================
void CommClearTimeout(void);

//=============================================================================================
//函数名称	: void CommClearCtrl(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 通信协议清除通信控制
//注    意	:
//=============================================================================================
void CommClearCtrl(void);

//=============================================================================================
//函数名称	: u16 CommCalcCRC(int CRCType,u8 *Data, u16 Length,u8 *TransmitFirst, u8 *TransmitSecond)
//输入参数	: CRCType：CRC计算类型；Data：数组指针；Length：计算的长度；TransmitFirst：CRC低字节；TransmitSecond：CRC高字节
//输出参数	: CRC:计算的CRC
//函数功能	: NFC通信计算CRC
//注意事项	:
//=============================================================================================
u16 CommCalcCRC(int CRCType,u8 *Data, u16 Length,u8 *TransmitFirst, u8 *TransmitSecond);

//=============================================================================================
//函数名称	: u8 CommCheckCRCA(u8 *msg, u8 cnt)
//输入参数	: msg：比较的数据指针；cnt：比较的数据的长度
//输出参数	: TRUE：CRC正确；FALSE：CRC错误
//函数功能	: NFC通信检测CRC A方式是否正确
//注意事项	:
//=============================================================================================
u8 CommCheckCRCA(u8 *msg, u8 cnt);

//=============================================================================================
//函数名称	: u8 CommStringComp(const u8 *msg1, const u8 *msg2, u16 cnt)
//输入参数	: msg1：比较字符串1；msg2：比较的字符串2；cnt：比较的长度
//输出参数	: TRUE：比较正确；FALSE：比较错误
//函数功能	: 字符串比较
//注意事项	:
//=============================================================================================
u8 CommStringComp(const u8 *msg1, const u8 *msg2, u16 cnt);

//=============================================================================================
//函数名称	: u8 CommStringComp(const u8 *msg1, const u8 *msg2, u16 cnt)
//输入参数	: dest：目标字符串地址； *sour：源字符串地址；cnt：复制的长度
//输出参数	:
//函数功能	: 字符串复制
//注意事项	:
//=============================================================================================
void CommStringCopy(u8 *dest, const u8 *sour, u16 cnt);

//=============================================================================================
//函数名称	: u16 CommBigEndian16Get(u8 *msg)
//输入参数	: msg：要提取的字符串地址
//输出参数	: 提取的数据
//函数功能	: 在字符串中提取大端模式的16位数据
//注意事项	:
//=============================================================================================
u16 CommBigEndian16Get(u8 *msg);

//=============================================================================================
//函数名称	: void CommBigEndian16Put(u8 *msg, u16 data)
//输入参数	: msg：放置的地址；data：放置的数据
//输出参数	:
//函数功能	: 向字符串中输出大端模式的16位数据
//注意事项	:
//=============================================================================================
void CommBigEndian16Put(u8 *msg, u16 data);

//=============================================================================================
//函数名称	: u32 CommBigEndian32Get(u8 *msg)
//输入参数	: msg：要提取的字符串地址
//输出参数	: 提取的数据
//函数功能	: 在字符串中提取大端模式的32位数据
//注意事项	:
//=============================================================================================
u32 CommBigEndian32Get(u8 *msg);

//=============================================================================================
//函数名称	: void CommBigEndian32Put(u8 *msg, u32 data)
//输入参数	: msg：放置的地址；data：放置的数据
//输出参数	:
//函数功能	: 向字符串中输出大端模式的32位数据
//注意事项	:
//=============================================================================================
void CommBigEndian32Put(u8 *msg, u32 data);

#endif

/*****************************************end of Comm.h*****************************************/
