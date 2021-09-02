//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Common.h
//创建人  	: Handry
//创建日期	:
//描述	    : Sim模组公共文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef __AT_CMD_CTRL_H__
#define __AT_CMD_CTRL_H__

#ifdef __cplusplus
extern "C"
{
#endif

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SwTimer.h"
#include "SafeBuf.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#define     MAX_REQ_BUF_SIZE    128
#define     MAX_RSP_BUF_SIZE    256
#define     MAX_ATCMD_SIZE 	    (MAX_REQ_BUF_SIZE * 2)
#define     MAX_RE_TX_COUNT     50

#define     TIMID_WAIT_RSP  	0
#define     TIMID_RX_REQ  		1
#define     TIMID_WAIT_RESIDUAL 1

#define     ATCMD_PRINTF        _Printf
#define     ATPFL               _PrintfLevel
#define     AtCmdCtrl_ResetEx   AtCmdCtrl_Reset
//=============================================================================================
//定义数据类型
//=============================================================================================
typedef enum _ATCMD_RC
{
	 ATCRC_SUCCESS = 0		//执行成功，执行下一条命令
	,ATCRC_DONE				//执行成功，并且结束命令链
	,ATCRC_RESEND			//执行失败，重发当前命令
	,ATCRC_FAILED			//执行失败，重发当前命令
	,ATCRC_UNKNOWN			//不认识该命令
	
	,ATCRC_MAX				
}ATCMD_RC;

typedef enum _AtCmdState
{
	 AT_SPECIFIED_OK  = 0x00	//返回默认的OK
	,AT_ERROR  		 //返回默认的错误

	,AT_UNKNOWN 	
	,AT_BUSY		
	,AT_COMM_ERROR	
	,AT_TIMEOUT 	
}AtCmdState;

typedef enum _AT_STATE
{
	AT_INIT
	,AT_RX_REQ
	,AT_TX_REQ
	,AT_RX_RSP
}AT_STATE;

typedef void (*AtReqItemProcFn)(const char* pReq, int reqLen);
typedef struct _AtReqItem
{
	const char* reqStr;
	AtReqItemProcFn ReqProc;
}AtReqItem;

typedef struct _AtCmdItem
{
	uint8 cmdInd;
	const char* atCmd;
	const char* atAck;
	ATCMD_RC (*RspHandler)(AtCmdState state, char* pRsp, struct _AtCmdItem** pNextAtCmd);
}AtCmdItem;

struct _AtCmdCtrl;
typedef Bool (*AtCmdTxFn)(const char* pData, int len);

//返回值说明:
//	RC_UNKNOWN	: 不认识这个响应, 不是本模块的响应
//	RC_DONOTHING: 认识这个响应, 但是接受到的响应不完整，没有做任何处理。
//	RC_DONE		: 认识这个响应, 并且已经处理完毕。
typedef ATCMD_RC (*AtCmdRspFn)(struct _AtCmdCtrl* pAtCmdCtrl, const AtCmdItem* pAtCmd, AtCmdState state, const char* pRsp, int len);

typedef void (*AtCmdReqProcFn)(struct _AtCmdCtrl* pAtCmdCtrl, char* pReq, uint16 len);
typedef Bool (*IsFilterFn)(const char* pReq, uint16* len);
typedef Bool (*SendCmdItemFn)(const AtCmdItem* pAtCmd);
typedef Bool (*IsAllowResendFn)(struct _AtCmdCtrl* pAtCmdCtrl, const AtCmdItem* pAtCmd, int reTxCount);

//AT命令处理总结构体
typedef struct _AtCmdCtrl
{
	uint8	m_State:4;
	uint8	m_IsProcDone:1;		//数据是否处理完毕
	uint8   m_isSearchAckByByte:1;//是否每接收到一个字符都要进行搜索是否匹配m_pAck
	uint8   m_isFilter:1;
	uint8   m_Reserved:1;
	
	const char*	m_pAck;					//支持的格式说明:"!string"，有"!"号表示不需要等到接收到'\n'之后再搜索string; 否则必须等到'\n'之后再再搜索string
	const char* m_pErrorTag;
	
	uint32	m_nTag;     //需要处理的命令的地址
	SwTimer	m_Timer;	//等待响应的定时器

	uint16 	m_nReqLen;
	uint16 	m_nRspLen;
	char  	m_Rsp[MAX_RSP_BUF_SIZE];    //接收到的响应放在此buff
	char	m_Req[MAX_REQ_BUF_SIZE];	//接收到REQ时放在此Buff
	
	int 	m_nPendingLen;
	char	m_PendingBuf[MAX_REQ_BUF_SIZE];	//接收到REQ时放在此Buff
	
	char  	m_Buffer[MAX_ATCMD_SIZE];
	SafeBuf	m_SafeBuf;
	
	SwTimer	m_PendingTimer;	//待发的At命令定时器
	uint8	m_MaxTxCounter;	//最大重发计数
	uint8	m_ReTxCounter;	//重发计数
	uint8	m_ErrCounter;	//错误计数

	//初始化时配置请求命令表，最后一项必须为Null
	const AtReqItem* m_pReqArray;	
	//初始化时配置命令表，最后一项必须为Null
	AtCmdItem*		 m_pCmdArray;	

	//初始化时配置的函数指针
	AtCmdTxFn 		Tx;
	AtCmdRspFn 		RspProc;
	AtCmdReqProcFn 	ReqProc;
	SendCmdItemFn	SendCmdItem;
	
	IsFilterFn		Filter;
	IsAllowResendFn	IsAllowResend;
	
}AtCmdCtrl;

//=============================================================================================
//声明接口函数
//=============================================================================================

//AT命令处理初始化
void AtCmdCtrl_Init(AtCmdCtrl* pAtCmdCtrl
	, AtCmdTxFn txFn
	, SendCmdItemFn	SendCmdItem
	, const AtReqItem* pReqArray
	, const AtCmdItem* pCmdArray
	, const char* okTag
	, const char* errorTag
	);

//AT命令处理信息打印
void AtCmdCtrl_Dump(AtCmdCtrl* pAtCmdCtrl);

//AT命令请求处理复位
void AtCmdCtrl_ReqReset(AtCmdCtrl * pAtCmdCtrl);

 
//AT命令响应处理复位
void AtCmdCtrl_RspReset(AtCmdCtrl* pAtCmdCtrl);

//AT命令处理复位
void AtCmdCtrl_Reset(AtCmdCtrl* pAtCmdCtrl);

//AT命令处理是否忙
Bool AtCmdCtrl_IsBusy(AtCmdCtrl* pAtCmdCtrl);

//AT命令处理是否空闲
Bool AtCmdCtrl_IsIdle(AtCmdCtrl* pAtCmdCtrl);

//AT命令响应处理结束
static void AtCmdCtrl_RspDone(AtCmdCtrl* pAtCmdCtrl, AtCmdState state);

//AT命令检查是否有请求数据
const char* AtCmdCtrl_CheckReqWord(AtCmdCtrl* pAtCmdCtrl, const char* pData);

//功能:移动rsp的数据(从位置startInd起,到末尾)到m_ReqFilter中
void AtCmdCtrl_MoveDataToReqFilter(AtCmdCtrl* pAtCmdCtrl, uint16 startInd);

//AT命令检查请求命令是否结束
Bool AtCmdCtrl_FilterReqData(AtCmdCtrl* pAtCmdCtrl);

//AT命令检查命令是否结束
Bool AtCmdCtrl_Filter(AtCmdCtrl* pAtCmdCtrl, char* pReq, uint16* len);

//检查req是否我一个完整的req数据包
Bool AtCmdCtrl_IsLineEnd(const char* pReq, uint16* len);

//接收请求处理，一个字节一个字节接收
Bool AtCmdCtrl_ReqHandler(AtCmdCtrl* pAtCmdCtrl);

//AT命令响应处理函数
AtCmdState AtCmdCtrl_RspHandler(AtCmdCtrl* pAtCmdCtrl);

//AT命令接收处理总函数
void AtCmdCtrl_RcvProc(AtCmdCtrl* pAtCmdCtrl);

//AT命令发送数据
static Bool AtCmdCtrl_Tx(AtCmdCtrl* pAtCmdCtrl, const char* pData, int len);

//获取参数长度
Bool AtCmdCtrl_isSearchAckByByte(const char* pStr, const char** ppStr);

//异步发送请求，不需要/r/n时使用
Bool AtCmdCtrl_AnsySendData(AtCmdCtrl* pAtCmdCtrl, const void* pData, uint16 len, const char* pAck, int waitMs, uint32 nTag);

//atcmd需要/r/n时用这个接口，不需要/r/n时使用AtCmdCtrl_AnsySendData
Bool AtCmdCtrl_AnsySend(AtCmdCtrl* pAtCmdCtrl, const char* pAtCmd, const char* pAck, int waitMs, uint32 nTag);
 
//AT命令发送数据
int AtCmdCtrl_AnsySendFmt(AtCmdCtrl* pAtCmdCtrl, const char* pAck, int waitMs, uint32 nTag, const char* lpszFormat, ...);

//同步发送请求，等待响应，暂不使用
int AtCmdCtrl_SyncSendWithRsp(AtCmdCtrl* pAtCmdCtrl, const char* pAtCmd, const char* pAck, int waitMs, char** ppRsp);

//同步发送请求
int AtCmdCtrl_SyncSend(AtCmdCtrl* pAtCmdCtrl, const char* pAtCmd, const char* pAck, int waitMs);

//延时timeOutMs后发送挂起的命令
void AtCmdCtrl_DelaySendCmdInd(AtCmdCtrl* pAtCmdCtrl, int cmdInd, uint32 timeOutMs);

//关闭挂起命令的定时器
void AtCmdCtrl_ResetPendingCmd(AtCmdCtrl* pAtCmdCtrl);

//获取挂起的命令
int AtCmdCtrl_GetPendingCmd(AtCmdCtrl* pAtCmdCtrl);

//定时器任务处理
Bool AtCmdCtrl_TimerProc(AtCmdCtrl* pAtCmdCtrl);

//服务器的请求或者SIM868的URC处理
void AtCmdCtrl_AtCmdReqProc(AtCmdCtrl* pAtCmdCtrl, char* pReq, uint16 len);

//AT命令响应处理函数
ATCMD_RC AtCmdCtrl_AtCmdRspProc(AtCmdCtrl* pAtCmdCtrl, const AtCmdItem* pAtCmd, AtCmdState state, const char* pRsp, int len);

//判断是否允许重发
Bool AtCmdCtrl_IsAllowResend(AtCmdCtrl* pAtCmdCtrl, const AtCmdItem* pAtCmd, int reTxCount);

//寻找at命令处理的指针函数
AtCmdItem* AtCmdCtrl_GetCmdInd(AtCmdCtrl* pAtCmdCtrl, int cmdInd);

//判断是否命令链结束还是重发,暂不使用
ATCMD_RC AtCmdCtrl_CmdProcDone(AtCmdState state, char* pRsp, struct _AtCmdItem** pNextAtCmd);

//根据AtCmdItem发送命令
Bool AtCmdCtrl_SendCmdItem(AtCmdCtrl* pAtCmdCtrl, AtCmdItem* pCmd);

//根据命令索引发送命令数据
Bool AtCmdCtrl_SendCmdByInd(AtCmdCtrl* pAtCmdCtrl, uint8 ind);

//AT命令处理运行函数
void AtCmdCtrl_Run(AtCmdCtrl* pAtCmdCtrl);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************end of AtCmdCtrl.h*****************************************/

