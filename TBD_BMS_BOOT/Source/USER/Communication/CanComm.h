//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: CanComm.h
//创建人  	: Handry
//创建日期	: 
//描述	    : CAN通信头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef __CAN_COMM_H
#define __CAN_COMM_H

//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"
#include "SafeBuf.h"
#include "Common.h"

//=============================================================================================
//宏定义
//=============================================================================================
#define     CAN_BUFF_LEN        256     /* 接收发送缓冲区大小 */
#define     CAN_REC_OVER_TIME   (50)    /* 接收判断超时时间 */
#define	    CAN_TX_TIMEOUT_MS   (10)    /* 发送判断超时时间 */

#define     MODBUS_MASTER_ADDR  0X01    /* modbus主机地址 */
#define     PROTOCOL_FIRST_BYTE 0x7E    /* 帧报文首个字节 */
#define     PROTOCOL_END_BYTE   0x7E    /* 帧报文最末尾字节 */
#define     MODBUS_MIN_PDU_LEN  (11)     /* modbus最小包长度 + CAN UTP包长度 */

#define     BOOT_FUNCODE        (0x67)  /* BOOTLOADER 通信功能码 */

#define     CAN_RX_CHECK_INT_PT           PORTB                //用于清除中断
#define     CAM_RX_CHECK_INT_PN           0                   //用于清除中断
#define		CAN_RX_CHECK_PIN_MASK		 ((u32)1 << CAM_RX_CHECK_INT_PN)

//=============================================================================================
//数据结构定义
//=============================================================================================
/* 数据处理状态枚举 */
typedef enum 
{        
    eCAN_StartRec = 0,    /* 开始接收 */
    eCAN_Recing  = 1,     /* 正在接收 */
    eCAN_RecOK  = 2,      /* 接收结束 */
    eCAN_ClrBuff = 3,     /* 清空BUFF */
    eCAN_MsgDeal = 4,     /* 数据处理 */
}e_DealSta;

/* modbus PDU */
typedef struct
{
    u8 addr;
    u8 funcode;
    u8 data[254];	/* 包含2字节crc */
}t_Modbus_PDU;

/* CAN处理信息结构体 */
typedef struct
{
    e_DealSta DealSta;              /* 处理状态 */
    
    union
	{
		struct
		{
            u8 IsCommTimeOut    :1; //CAN通信超时标志
			u8 RVSD          	:7; //保留
		}CommFlagBit;
		u8 CommFlagByte;
	}CommFlag;          
    
    u32 StartTimer;                 /* 接收帧超时时间 */
    u32 TimeOut;                    /* CAN通信超时时间 */
    SafeBuf gCanRecSafeBuf;         /* CAN处理安全BUFF */
    int RecLen;                     /* 接收数据长度 */
    u8 RecdataBuff[CAN_BUFF_LEN];	/* 接收数据缓存 */
    int TxLen;                      /* 发送字节长度 */
    u8 TxDataBuff[CAN_BUFF_LEN];    /* 发送数据缓存 */
    int DealLen;                    /* 处理字节长度 */
    u8 DealBuff[CAN_BUFF_LEN];      /* 处理数据缓存 */
    int TranLen;                    /* 转换处理字节长度 */
    u8 TranBuff[CAN_BUFF_LEN];      /* 转换处理数据缓存 */   
    int TempLen;                    /* 暂缓字节长度 */
    u8 TempBuff[CAN_BUFF_LEN];      /* 暂缓数据缓存 */      
}t_CAN_MSG;

//CAN Utp帧配置特征
typedef struct _CanUtpFrameCfg
{
	uint8_t head;		//帧头
	uint8_t tail;		//帧尾
	uint8_t transcode;		//转换码
	uint8_t transcodeHead;	//帧头转换码
	uint8_t transcodeTail;	//帧尾转换码
	uint8_t transcodeShift;	//转换码-转换

	uint8_t cmdByteInd;		//命令字节偏移
	uint8_t dataByteInd;	//数据字节偏移

	int      txBufLen;	//txBuf的长度
	uint8_t* txBuf;		//存放发送出去数据，转码前
	int      rxBufLen;	//rxBuf的长度
	uint8_t* rxBuf;		//存放接收到的RAW数据，转码前
	int		 transcodeBufLen;//transcodeBuf的长度
	uint8_t* transcodeBuf;	 //存放rxBuf转码后的帧数据

	/*****************************************************
	如果要实现在等待响应的同时能够接收新的请求功能，必须初始化txRspBuf指针。
		!=Null: 用于保存要发送的响应数据
		= Null：在等待响应数据的同时，丢弃接收到新的请求数据
	*****************************************************/
	uint8_t* txRspBuf;		
	int      txRspBufLen;	//txRspBuf的长度

	uint8_t result_SUCCESS;		//返回码定义：成功
	uint8_t result_UNSUPPORTED;	//返回码定义：接收到不支持的请求

	uint32_t waitRspMsDefault;	//命令的默认的等待响应时间，如果命令要修改为非默认值，可以在命令的事件函数UTP_TX_START中修改pUtp->waitRspMs
	uint32_t rxIntervalMs;		//接收数据间隔
	uint32_t sendCmdIntervalMs;	//发送2个命令之间的间隔时间

//	UtpFrameVerifyFn FrameVerify;	//帧校验函数
//	UtpBuildFrameFn	 FrameBuild;	//帧打包函数
}CanUtpFrameCfg;

typedef struct _CanUtp
{
    uint8_t  checkSum;        
    uint8_t  protocolver;   //协议版本
    uint8_t  len;        
    uint8_t  data[1];
}CanUtp;

//=============================================================================================
//宏参数设定
//=============================================================================================

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern t_CAN_MSG gCanDealMsg;
extern u8 gIDFlag;        //ID信号中断Flag

//=============================================================================================
//声明接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: u8 CanCommInit(void)
//输入参数	: void
//输出参数	: 初始化是否成功，TURE:成功  FALSE:失败
//静态变量	: void
//功    能	: CAN通信协议初始化函数
//注    意	:
//=============================================================================================
u8 CanCommInit(void);

//=============================================================================================
//函数名称	: int CanCommTxData(const uint8_t* pData, uint32_t len)
//输入参数	: pData：发送数据指针；len：发送数据长度
//输出参数	: 发送长度
//静态变量	: void
//功    能	: CAN通信发送数据到总线
//注    意	:
//=============================================================================================
int CanCommTxData(const uint8_t* pData, uint32_t len);

//=============================================================================================
//函数名称	: u8 CanCommFrameRx(uint8_t * buf, uint16_t * cnt, uint32_t timeout)
//输入参数	: buf：要接收放置的地址；cnt：接收的长度指针；timeout：超时时间
//输出参数	: 是否接收成功，FALSE:失败；TRUE：成功
//静态变量	: void
//功    能	: can接收帧
//注    意	:
//=============================================================================================
u8 CanCommFrameRx(uint8_t * buf, uint16_t * cnt, uint32_t timeout);

//=============================================================================================
//函数名称	: u8 CanCommFrameTx(uint8_t fcode, uint8_t const * buf, uint16_t cnt, uint32_t timeout)
//输入参数	: fcode：功能码；buf：要发送的数据；timeout：超时时间
//输出参数	: 是否发送成功，FALSE:失败；TRUE：成功
//静态变量	: void
//功    能	: can发送帧
//注    意	:
//=============================================================================================
u8 CanCommFrameTx(uint8_t fcode, uint8_t const * buf, uint16_t cnt, uint32_t timeout);

//=============================================================================================
//函数名称	: u8 CommCompareTimeout(u16 timeout)
//输入参数	: timeout:超时时间
//输出参数	: void
//静态变量	: void
//功    能	: CAN通信协议检测超时时间
//注    意	:
//=============================================================================================
u8 CanCommCompareTimeout(u16 timeout);

//=============================================================================================
//函数名称	: void CanCommCheckTimeout(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: CAN通信协议断线超时任务
//注    意	:
//=============================================================================================
void CanCommCheckOffTimeout(void);

//=============================================================================================
//函数名称	: CanUtpDecodeRecData(const u8* pRecData, int Reclen, u8 *pDealData, int* pDealLen,const CanUtpFrameCfg *FrameCfg)
//输入参数	:
//	pRecData：接收数据指针。
//	Reclen：接收数据长度。
//	pDealData：处理完的数据指针。
//	pDealLen：处理完的数据长度指针。
//  const CanUtpFrameCfg* frameCfg：转码信息配置
//输出参数	: 解析结果。0：解析失败；1：解析成功
//静态变量	: void
//功    能	: 把接收的数据进行转码解析、校验，并将处理的数组结果放入pDealData指针
//注    意	:7E 30 0A 09 01 03 04 00 0A 00 00 31 DA 7E
//=============================================================================================
int CanUtpDecodeRecData(const u8* pRecData, int Reclen, u8 *pDealData, int* pDealLen,const CanUtpFrameCfg *FrameCfg);

//=============================================================================================
//函数名称	: int CanUtpBuildFrame(const void* pData, int len, CanUtp* frame,const CanUtpFrameCfg *FrameCfg)
//输入参数	:
//	pData：源数据指针。
//	len：要打包的数据长度
//  CanUtp* frame：目标打包数据指针
//  const CanUtpFrameCfg* frameCfg：转码信息配置
//输出参数	: 打包的长度
//静态变量	: void
//功    能	: 把要发送的数据数据打包成为一个协议格式的数据帧
//注    意	:
//=============================================================================================
int CanUtpBuildFrame(const void* pData, int len, CanUtp* frame,const CanUtpFrameCfg *FrameCfg);

//=============================================================================================
//函数名称	: Bool CanCommFramePkt(const uint8_t* pSrc, uint16_t srcLen, int* srcInd, uint8_t* pDst, int* dstSize,const CanUtpFrameCfg* frameCfg)
//输入参数	:
//  pSrc:指向Payload数据，包含检验和，不包括帧头帧尾
//  srcLen：Src数据长度。
//	srcInd: 处理位置
//	pDst: 目标数据Buff
//	dstLen：目标Buff数据长度
//  const CanUtpFrameCfg* frameCfg：转码信息配置
//输出参数	: 是否打包成功。0：打包失败，1：打包成功
//静态变量	: void
//功    能	: 把数据加上帧头和帧尾，并且转码
//注    意	:
//=============================================================================================
Bool CanCommFramePkt(const uint8_t* pSrc, uint16_t srcLen, int* srcInd, uint8_t* pDst, int* dstSize,const CanUtpFrameCfg* frameCfg);

//=============================================================================================
//函数名称	: u8 CanCommMainTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: CAN通信协议主任务函数
//注    意	:
//=============================================================================================
u8 CanCommMainTask(void);

#endif

/*****************************************end of Comm.h*****************************************/
