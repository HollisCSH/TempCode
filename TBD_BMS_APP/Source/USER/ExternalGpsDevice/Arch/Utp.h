/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */
 
 /******************************************************
UTP：串口传输协议，实现“车载终端与外接设备通讯协议”或者类型功能的协议。

本协议适合以下格式的传输协议
1 协议包含head字节和Tail字节，head字节和Tail字节可以相同，也可以不同，在frameCfg中定义head和Tail的值
2 协议内容中如果出现head字节和Tail字节，必须要转码，在frameCfg配置转码规则。
3 协议包含一个CMD码，CMD码的位置可以在帧头任意位置，可选包含SEQ和ADDR等其他字节，在frameCfg中定义检验函数和组帧函数
4 协议包含一个CRC码，CRC码可以位于PAYLOAD的前面或者后面。
5 如果是响应帧，DATA的第一个字节为RESULT字节
6 支持四种帧类型
	1) READ:读命令类型，主机发送到从机，接收者必须要响应.
	2）WRITE:写命令类型,主机发送到从机，接收者必须要响应.
	3) EVENT：事件类型，从机发送到主机，接受者必须要响应。
	3）NOTIFY：通知类型,双发都可以发送，接收者不需要应答。

具体格式定义如下：

          帧头    payload         crc
      |---------|---------------|----|
|-----|---------|---------------|----|----|
  head   *cmd*         data      crc  tail

**********************************************************/

#ifndef __UTP__H_
#define __UTP__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "BSPTypeDef.h"
#include "SwTimer.h"
#include "cirbuffer.h"
#include "queue.h"

//重发标志，无限次重发
#define UTP_ENDLESS 0xFF

/***************
传输事件定义
***************/
typedef enum
{
	UTP_TX_START,		//发送命令开始, 可在该事件中配置通信参数：waitRspMs, maxTxCount，transferData, transferLen
	UTP_TX_DONE,		// 发送命令结束，等待回应
	UTP_CHANGED_BEFORE,	//pStorage值改变之前
	UTP_CHANGED_AFTER,	//pStorage值改变之后
	UTP_GET_RSP,		//接收到请求,获取响应，请求的数据在pStorage中，应该在改事件中配置transferData指针，作为RSP

	UTP_REQ_FAILED,		//请求结束，失败，原因：可能传输超时，或者响应返回失败码。
	UTP_REQ_SUCCESS,	//传输结束，成功，传输成功并且响应返回成功码
}UTP_TXF_EVENT;

typedef enum
{
	UTP_EVENT_RC_SUCCESS = 0,	//事件处理成功
	UTP_EVENT_RC_FAILED,		//事件处理失败
	UTP_EVENT_RC_DO_NOTHING,	//事件没有处理
}UTP_EVENT_RC;

//UTP 传输状态
typedef enum
{
	UTP_FSM_INIT = 0	//初始状态，空闲
	, UTP_FSM_RX_REQ	//接收到请求
	, UTP_FSM_WAIT_RSP	//等待响应
}UTP_FSM;

typedef enum
{
	UTP_READ = 0	//读命令类型，需要应答
	, UTP_WRITE		//写命令类型，需要应答
	, UTP_NOTIFY	//通知类型  ，发送给对方的通知，不需要应答

	, UTP_EVENT			//事件类型，对方发送给本方的请求，需要应答
	, UTP_EVENT_NOTIFY	//事件通知类型，对方发送给本方的通知，不需要应答
}UTP_CMD_TYPE;

typedef enum _UTP_RCV_RSP_RC
{
	RSP_SUCCESS = 0
	, RSP_TIMEOUT
	, RSP_FAILED
	, RSP_CANCEL		//取消发送
}UTP_RCV_RSP_RC;

//frame state;
typedef enum _FRAME_STATE
{
	FRAME_INIT = 0
	, FRAME_FOUND_HEAD
	, FRAME_FOUND_ESCAPE
	, FRAME_DONE
}FRAME_STATE;

//函数类型定义
struct _UtpCmd;
struct _Utp;
typedef UTP_EVENT_RC(*UtpEventFn)(void* pOwnerObj, const struct _UtpCmd* pCmd, UTP_TXF_EVENT ev);
typedef int (*UtpTxFn)(uint8_t cmd, const uint8_t* pData, uint32_t len);

/*********************************
函数功能：帧校验，实现2个功能；
	1）如果req=Null,仅仅检验接收到的数据是否完整。
	2）如果req!=Null,检验接收的数据帧是否和Req匹配。
	3）如果协议中包含Seq字节和Addr字节，也可以使用该函数检验和过滤
参数说明：
	pUtp：UTP对象指针。
	rxFrame：要检验的数据帧。
	frameLen：帧长度。
	req：请求帧，如果req=Null，表示不需要检验Req
返回值：校验结果。
***********************************/
typedef Bool(*UtpFrameVerifyFn)(struct _Utp* pUtp, const uint8_t* rxFrame, int frameLen, const uint8_t* req);

/*********************************
函数功能：把要发送的数据数据打包成为一个请求数据帧或者响应帧，由参数reqFrame决定。
参数说明：
	pUtp：UTP对象指针。
	cmd：命令码。
	pData：数据指针。
	len：数据长度
	reqFrame：如果=NULL，表示根据请求帧打包响应帧；如果!=NULL，表示打包一个请求帧。
	rspFrame：输出参数，要打包的数据帧指针
返回值：数据帧长度。
***********************************/
typedef int (*UtpBuildFrameFn)(struct _Utp* pUtp, uint8_t cmd, const void* pData, int len, const uint8* reqFrame, uint8* rspFrame);

//Utp帧配置特征
typedef struct _UtpFrameCfg
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

	UtpFrameVerifyFn FrameVerify;	//帧校验函数
	UtpBuildFrameFn	 FrameBuild;	//帧打包函数
}UtpFrameCfg;

//Utp其他配置项
typedef struct _UtpCfg
{
	int					  cmdCount;	//命令数组总数
	const struct _UtpCmd* cmdArray;	//命令数组

	UtpTxFn		TxFn;				//数据的发送到总线的函数指针
	void* pCbObj;				//回调函数参数
	UtpEventFn	TresferEvent;		//事件回调函数
}UtpCfg;

typedef struct _UtpCmdEx
{
	/*
	如果是EVENT，
		在UTP_GET_RSP事件之前transferData指向接收到的数据
		在UTP_GET_RSP事件中transferData默认指向UtpCmd->pData，也可以
			在事件函数中修改，指向要应答的数据指针，由上层程序决定
	如果是READ/WRITE,指向应答数据
	*/
	const uint8_t* transferData;	//传输数据协议包的数据域指针，
	uint8_t transferLen;		//传输数据长度

	/*****************
	发送请求或接收响应时的时间戳Ticks.
	对于UTP_READ/UTP_WRITE类型：保存接收响应时的Ticks：
	对于UTP_EVENT类型：不使用
	对于UTP_EVENT_NOTIFY类型：不使用
	对于UTP_NOTIFY类型：发送请求时的Ticks。
	******************/
	uint32_t rxRspTicks;

	/*****************
	发送标志:
		0：表示不发送，
		其他值：表示延时指定的时间MS发送
	******************/
	uint32_t sendDelayMs;

	UTP_RCV_RSP_RC rcvRspErr;	//接收响应错误码
}UtpCmdEx;

typedef struct _UtpCmd
{
	UtpCmdEx* pExt;	//UtpCmd扩展

	UTP_CMD_TYPE type;	//读或者写命令，参考UTP_CMD_TYPE定义
	uint8_t cmd;			//命令码
	const char* cmdName;//命令名称

	//如果是READ，保存读回来的数据
	//如果是WRITE，保存要发送的写命令参数
	//如果是EVENT，保存Event传回来的数据
	//如果是UTP_EVENT_NOTIFY，保存Event传回来的数据
	//如果是NOTIFY，表示发送参数
	//如果值为Null,表示该值无意义
	uint8_t* pStorage;
	int storageLen;

	//如果是READ，pData指向要发送的读命令参数
	//如果是WRITE，pData指向已经发送成功的数据，用于和pStorage比较是否发生变化，确定是否需要发送新的写命令
	//如果是EVENT，pData指向事件响应参数数据指针
	//如果是UTP_EVENT_NOTIFY，不需要应答，pData=Null
	//如果是NOTIFY类型,不需要应答，pData=Null
	//如果值为Null,表示该值无意义
	uint8_t* pData;
	int dataLen;

	UtpEventFn Event;	//事件回调函数
}UtpCmd;

typedef struct _Utp
{
	UTP_FSM state;
	FRAME_STATE FrameState;

	uint16_t txBufLen;		//发送帧长度
	uint16_t transcodeBufLen;	//转码后的帧长度
	//Queue rxBufQueue;		//管理rxBuf的Queue。
	CirBuff	 rxBuffCirBuff;	// 管理rxBuf的Cirbuff

	//搜索帧头和帧尾变量
	int    searchIndex;	//搜索偏移
	Bool   headIndex;		//帧头偏移
	uint8_t*   head;		//帧头,指向帧头数据

	//记录接收数据帧的当前Ticks，用于计算接收一个数据帧内2个数据之间是否超时
	uint32_t rxDataTicks;

	/*****************************************************************/
	uint8_t reTxCount;	//重发次数
	uint8_t maxTxCount;	//最大重发次数

	uint32_t rxRspTicks;			//接收响应的Ticks，用于计算发送帧间隔

	uint32_t waitRspMs;			//等待响应时间
	SwTimer waitRspTimer;		//等待响应定时器

	const UtpCmd* pWaitRspCmd;	//当前的执行的请求，正在等待RSP的命令，可能是READ/WRITE，Null表示没有

	/*****************************************************************/
	const UtpCfg* cfg;
	const UtpFrameCfg* frameCfg;
}Utp;

//UTP 错误码定义
typedef enum _OP_CODE
{
    OP_SUCCESS = 0
    ,OP_PARAM_INVALID	= 1 //该参数无效
    ,OP_UNSUPPORTED = 2 //该命令不支持
    ,OP_CRC_ERROR		= 3	//校验错误
    ,OP_NOT_READY		= 4	//设备没准备好
    ,OP_USERID_ERROR	= 5	//USER_ID错误
    ,OP_HW_ERROR		= 6	//执行失败
    ,OP_18650BAT_V_OVER_LOW	= 7	//18650电压太低
    ,OP_NOT_ALLOW_DISCHARGE	= 8	//不支持放电
    
    ,OP_PENDING			= 0xF0	//执行等待
    ,OP_NO_RSP = 0xF1	//No response
    ,OP_FAILED			= 0xFF	//执行失败
}OP_CODE;

void Utp_Init(Utp* pUtp, const UtpCfg* cfg, const UtpFrameCfg* frameCfg);
void Utp_Run(Utp* pUtp);

/************************************
函数功能：协议对象复位，
	取消已经发送正在等待响应的命令，
	取消所有等待发送的命令
	清除Rx缓冲区和Req缓冲区
参数说明：
	pUtp：Utp对象。
返回值无
***************************************/
void Utp_Reset(Utp* pUtp);

/************************************
函数功能：从总线或者中断中接收数据，保存到接收缓冲区rxBufQueue队列中
参数说明：
	pUtp：Utp对象。
	pData：从总线或者中断中接收到的数据。
	len：数据长度。
返回值无
***************************************/
void Utp_RxData(Utp* pUtp, const uint8_t* pData, int len);


/*******************************************
函数功能：发送命令。
参数说明：
	pUtp：Utp对象。
	cmd：发送的命令码。
返回值：无
*******************************************/
void Utp_SendCmd(Utp* pUtp, uint8_t cmd);

/*******************************************
函数功能：延时发送命令。
参数说明：
	pUtp：Utp对象。
	cmd：发送的命令码。
	delayMs：延时发送事件，0表示不延时，立即发送
返回值：无
*******************************************/
void Utp_DelaySendCmd(Utp* pUtp, uint8_t cmd, uint32_t delayMs);

//协议栈是否空闲
Bool Utp_isIdle(const Utp* pUtp);

/*
	删除发送指令
*/
void Utp_DeleteCmd( Utp *pUtp , uint8_t cmd );

/* NB模块 utp.h文件 */
//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTypeDef.h"
#include "SafeBuf.h"
#include "SwTimer.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#define     UTP_PROTOCOL_VER	1

#define     UTP_HEAD_LEN 	    2
#define     UTP_MIN_LEN 	    4	    //不包含帧头和帧尾

#define     UTP_PURE_DATA_SIZE          128	    //UTP纯数据大小
#define     UTP_REMOTE_REQ_BUF_SIZE     (UTP_PURE_DATA_SIZE + 20)	//考虑在固件OTA时，最大包纯数据为128个字节。
#define     UTP_REQ_BUF_SIZE 	        (UTP_REMOTE_REQ_BUF_SIZE)
#define     UTP_RSP_BUF_SIZE 	        (UTP_REMOTE_REQ_BUF_SIZE)
#define     UTP_SAFEBUF_SIZE 	        (UTP_REMOTE_REQ_BUF_SIZE)

#define     UTP_FRAME_HEAD 		        0x7E	//0x8C 0x81
#define     UTP_FRAME_TAIL 		        0xFF	//0x8C 0x00

#define     UTP_FRAME_ESCAPE 	        0x8C	//0x8C 0x73
#define     UTP_RSP_CODE     	        0x80

#define     UTP_FRAME_ESCAPE_HREAD      0x81
#define     UTP_FRAME_ESCAPE_TAIL       0x00
#define     UTP_FRAME_ESCAPE_ESCAPE     0x73
#define     UTP_ENDLESS                 0xFF

#define GET_CHECKSUM_POINTER(_pFrame) (uint16*)&(((uint8*)_pFrame)[ ((uint8*)_pFrame)[3] + 4 ])
#define UTP_PRINTF Printf

//=============================================================================================
//定义数据类型
//=============================================================================================
typedef struct _UtpFrame
{
//	uint8  head;
	uint8  cmd;
	uint8  len;
	uint8  data[1];
	//uint16 checkSum;
	//uint8 tail
}UtpFrame;
    
//=============================================================================================
//声明接口函数
//=============================================================================================
//NB模块使用
Bool Utp_FramePkt_NB(const uint8* pSrc, uint16 srcLen, int* srcInd, uint8* pDst, int* dstSize);
Bool Utp_ConvertToHost_NB(uint8* dst, uint16 dstSize, uint16* dstInd, FRAME_STATE *state, uint8 byte);

#ifdef __cplusplus
}
#endif

#endif


