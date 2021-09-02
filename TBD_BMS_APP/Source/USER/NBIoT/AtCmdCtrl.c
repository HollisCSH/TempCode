//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: AtcmdCtrl.c
//创建人  	: 
//创建日期	:
//描述	    : Sim模组AT命令处理操作文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SimCommon.h"
#include "AtCmdCtrl.h"
#include <stdarg.h>

//=============================================================================================
//定义接口函数
//=============================================================================================

//AT命令处理初始化
void AtCmdCtrl_Init(AtCmdCtrl* pAtCmdCtrl
	, AtCmdTxFn txFn
	, SendCmdItemFn	SendCmdItem
	, const AtReqItem* pReqArray
	, const AtCmdItem* pCmdArray
	, const char* okTag
	, const char* errorTag
	)
{
	memset(pAtCmdCtrl, 0, sizeof(AtCmdCtrl));

	SafeBuf_Init(&pAtCmdCtrl->m_SafeBuf, pAtCmdCtrl->m_Buffer, sizeof(pAtCmdCtrl->m_Buffer));
	SwTimer_Init(&pAtCmdCtrl->m_Timer, 0, 0);

	pAtCmdCtrl->m_pErrorTag	= errorTag;
	pAtCmdCtrl->m_pReqArray	= pReqArray;
	pAtCmdCtrl->m_pCmdArray	= (AtCmdItem*)pCmdArray;

	pAtCmdCtrl->Tx 			= txFn;
	pAtCmdCtrl->SendCmdItem = SendCmdItem;

	pAtCmdCtrl->Filter		= AtCmdCtrl_IsLineEnd;
	pAtCmdCtrl->RspProc		= AtCmdCtrl_AtCmdRspProc;
	pAtCmdCtrl->ReqProc		= AtCmdCtrl_AtCmdReqProc;
	pAtCmdCtrl->IsAllowResend = AtCmdCtrl_IsAllowResend;
	pAtCmdCtrl->m_MaxTxCounter = MAX_RE_TX_COUNT;
}

//AT命令处理信息打印
void AtCmdCtrl_Dump(AtCmdCtrl* pAtCmdCtrl)
{
//	ATCMD_PRINTF("AtCmdCtrl dump:\n");
//	ATCMD_PRINTF("\t State = %d\n", pAtCmdCtrl->m_State);
//	ATCMD_PRINTF("\t ReqLen=%d\n" , pAtCmdCtrl->m_nReqLen);
//	ATCMD_PRINTF("\t RspLen=%d\n" , pAtCmdCtrl->m_nRspLen);
//	ATCMD_PRINTF("\t isSearchAckByByte=%d\n", pAtCmdCtrl->m_isSearchAckByByte);
//	ATCMD_PRINTF("\t nTag=%d\n"   , pAtCmdCtrl->m_nTag);
//	
//	ATCMD_PRINTF("\t Time.isStart = %d\n", pAtCmdCtrl->m_Timer.m_isStart);
//	ATCMD_PRINTF("\t Time.timeOutTicks = %d\n", pAtCmdCtrl->m_Timer.m_TimeOutTicks);
}

//判断当前字节是否可打印
Bool AtCmdCtrl_isPrintChar(uint8 byte)
{
	return (byte >= 0x20 && byte <= 0x7E) || byte == '\r' || byte == '\n';
}

//AT命令请求处理复位
void AtCmdCtrl_ReqReset(AtCmdCtrl * pAtCmdCtrl)
{
	//ATCMD_PRINTF("ReqReset()\n");
	pAtCmdCtrl->m_State 	= AT_INIT;
	pAtCmdCtrl->m_nReqLen 	= 0;
	SwTimer_Stop(&pAtCmdCtrl->m_PendingTimer);
	memset(pAtCmdCtrl->m_Req, 0, sizeof(pAtCmdCtrl->m_Req));
}
 
//AT命令响应处理复位
void AtCmdCtrl_RspReset(AtCmdCtrl* pAtCmdCtrl)
{
	//ATCMD_PRINTF("RspReset()\n");
	pAtCmdCtrl->m_State 	= AT_INIT;
	pAtCmdCtrl->m_nRspLen 	= 0;
	pAtCmdCtrl->m_pAck 		= Null;
	pAtCmdCtrl->m_nTag		= 0;
	pAtCmdCtrl->m_MaxTxCounter = MAX_RE_TX_COUNT;
	pAtCmdCtrl->m_isSearchAckByByte = False;
	SwTimer_Stop(&pAtCmdCtrl->m_Timer);
	memset(pAtCmdCtrl->m_Rsp, 0, sizeof(pAtCmdCtrl->m_Rsp));
}

//AT命令处理复位
void AtCmdCtrl_Reset(AtCmdCtrl* pAtCmdCtrl)
{
	pAtCmdCtrl->m_ReTxCounter = 0;
	pAtCmdCtrl->m_ErrCounter = 0;
	AtCmdCtrl_ReqReset(pAtCmdCtrl);
	AtCmdCtrl_RspReset(pAtCmdCtrl);
	
	SwTimer_Stop(&pAtCmdCtrl->m_PendingTimer);
	SafeBuf_Reset(&pAtCmdCtrl->m_SafeBuf);
	
	pAtCmdCtrl->m_nPendingLen = 0;
	memset(pAtCmdCtrl->m_PendingBuf, 0, sizeof(pAtCmdCtrl->m_PendingBuf));
}

//AT命令处理是否忙
Bool AtCmdCtrl_IsBusy(AtCmdCtrl* pAtCmdCtrl)
{
	return pAtCmdCtrl->m_State != AT_INIT;
}

//AT命令处理是否空闲
Bool AtCmdCtrl_IsIdle(AtCmdCtrl* pAtCmdCtrl)
{
	return pAtCmdCtrl->m_State == AT_INIT && !pAtCmdCtrl->m_PendingTimer.m_isStart;
}

//AT命令响应处理结束
static void AtCmdCtrl_RspDone(AtCmdCtrl* pAtCmdCtrl, AtCmdState state)
{
	//注意:在回调函数ReqProc中，可能会立刻发起新的AtCmd请求，
	//这样会导致pAtCmdCtrl->m_State状态被修改为非 AT_INIT 
	pAtCmdCtrl->m_State = AT_INIT;	//设置AT_INIT状态，释放pAtCmdCtrl
	pAtCmdCtrl->RspProc(pAtCmdCtrl, (AtCmdItem*)pAtCmdCtrl->m_nTag, state, pAtCmdCtrl->m_Rsp, pAtCmdCtrl->m_nRspLen);
	pAtCmdCtrl->m_nRspLen 	= 0;
	
	if(pAtCmdCtrl->m_State == AT_INIT)
	{
		//ATCMD_PRINTF("Stop timer\n");
		SwTimer_Stop(&pAtCmdCtrl->m_Timer);
	}
}

//AT命令检查是否有请求数据
const char* AtCmdCtrl_CheckReqWord(AtCmdCtrl* pAtCmdCtrl, const char* pData)
{
	const AtReqItem* pReq = pAtCmdCtrl->m_pReqArray;
	const char* dst = Null;
	
	for(pReq = pAtCmdCtrl->m_pReqArray; pReq->reqStr; pReq++)
	{
		dst = strstr(pData, pReq->reqStr);
		if(dst)
		{
			return dst;
		}
	}

	return False;
}

//功能:移动rsp的数据(从位置startInd起,到末尾)到m_ReqFilter中
//startInd: Rsp的起始位置.
void AtCmdCtrl_MoveDataToReqFilter(AtCmdCtrl* pAtCmdCtrl, uint16 startInd)
{
	uint16 len = pAtCmdCtrl->m_nRspLen - startInd;
	
	if(pAtCmdCtrl->m_nReqLen + len <= sizeof(pAtCmdCtrl->m_Req))
	{
		memcpy(&pAtCmdCtrl->m_Req[pAtCmdCtrl->m_nReqLen], &pAtCmdCtrl->m_Rsp[startInd], len);
		pAtCmdCtrl->m_nReqLen += len;
/*
		memset(g_CommonBuf, 0, sizeof(g_CommonBuf));
		memcpy(g_CommonBuf, &pAtCmdCtrl->m_Rsp[startInd], len); 
		ATCMD_PRINTF("============ReqFilter In[%d]:%s", len, g_CommonBuf);
*/		
	}
	else
	{
		//Buff满，丢弃该数据包
		//ATPFL(DL_WARNING, "%s: FILTER REQ buffer is full.\n", _FUNC_);
		pAtCmdCtrl->m_nReqLen = 0;
		memset(pAtCmdCtrl->m_Req, 0, sizeof(pAtCmdCtrl->m_Req));
		return;
	}
	
	//Removed data from rsp buf.
	pAtCmdCtrl->m_nRspLen = startInd;
	memset(&pAtCmdCtrl->m_Rsp[startInd], 0, len);
}

//AT命令检查请求命令是否结束
Bool AtCmdCtrl_FilterReqData(AtCmdCtrl* pAtCmdCtrl)
{
	const char* req = Null;
	
	req = AtCmdCtrl_CheckReqWord(pAtCmdCtrl, pAtCmdCtrl->m_Rsp);
	if(req)
	{
		pAtCmdCtrl->m_isFilter = True;
		AtCmdCtrl_MoveDataToReqFilter(pAtCmdCtrl, req - pAtCmdCtrl->m_Rsp);
		return True;
	}

	return False;
}

//AT命令检查命令是否结束
Bool AtCmdCtrl_Filter(AtCmdCtrl* pAtCmdCtrl, char* pReq, uint16* len)
{
	uint16 size = *len;
	Bool ret = pAtCmdCtrl->Filter(pReq, &size);

	if(*len > size)
	{
		memset(&pReq[size], 0, *len - size);
		
		*len = size;
	}
	return ret;
}

//检查req是否我一个完整的req数据包
Bool AtCmdCtrl_IsLineEnd(const char* pReq, uint16* len)
{
	return (*len > 2 && pReq[*len-2] == '\r' && pReq[*len-1] == '\n');
}

//接收请求处理，一个字节一个字节接收
Bool AtCmdCtrl_ReqHandler(AtCmdCtrl* pAtCmdCtrl)
{
	char* pData = pAtCmdCtrl->m_Req;

    //判断是否接收结束
	if(AtCmdCtrl_Filter(pAtCmdCtrl, pData, &pAtCmdCtrl->m_nReqLen))
	{
		pAtCmdCtrl->m_State = AT_INIT;
		pAtCmdCtrl->ReqProc(pAtCmdCtrl, pData, pAtCmdCtrl->m_nReqLen);
		pAtCmdCtrl->m_nReqLen = 0;
		memset(pAtCmdCtrl->m_Req, 0, sizeof(pAtCmdCtrl->m_Req));
		return True;
	}
	return False;
}

//AT命令响应处理函数
AtCmdState AtCmdCtrl_RspHandler(AtCmdCtrl* pAtCmdCtrl)
{
	AtCmdState state = AT_UNKNOWN;
	char* pData = pAtCmdCtrl->m_Rsp;
	
	if(pAtCmdCtrl->m_isSearchAckByByte)
	{
		if(pData[pAtCmdCtrl->m_nRspLen-1] == *pAtCmdCtrl->m_pAck)
		{
			state =  AT_SPECIFIED_OK;
		}
		else if(AtCmdCtrl_IsLineEnd(pData, &pAtCmdCtrl->m_nRspLen))
		{
			if(strstr(pData, pAtCmdCtrl->m_pErrorTag))
			{
				state =  AT_ERROR;
			}
			else
			{
				AtCmdCtrl_FilterReqData(pAtCmdCtrl);
			}
		}
	}
	else if(AtCmdCtrl_Filter(pAtCmdCtrl, pData, &pAtCmdCtrl->m_nRspLen))
	{
		//如果在处理响应时，收到请求，缓存起来，在AtCmdCtrl_Run()中处理。
		AtCmdCtrl_FilterReqData(pAtCmdCtrl);
		
		if(strstr(pData, pAtCmdCtrl->m_pErrorTag))
		{
			state =  AT_ERROR;
		}
		else if(strstr(pData, pAtCmdCtrl->m_pAck))
		{
			state = AT_SPECIFIED_OK;
		}
	}

	return state;
}

//AT命令接收处理总函数
void AtCmdCtrl_RcvProc(AtCmdCtrl* pAtCmdCtrl)
{
	AtCmdState state = AT_UNKNOWN;
	uint8 byte;

	while(SafeBuf_Read(&pAtCmdCtrl->m_SafeBuf, &byte, 1))
	{
		//ATPFL(DL_ATCCMD, AtCmdCtrl_isPrintChar(byte) ? "%c":"%02x ", byte);
        //printf(AtCmdCtrl_isPrintChar(byte) ? "%c":"%02x ", byte);
        //初始化或者接收请求
		if(pAtCmdCtrl->m_State == AT_INIT || pAtCmdCtrl->m_State == AT_RX_REQ)
		{	
			if(pAtCmdCtrl->m_nReqLen >= sizeof(pAtCmdCtrl->m_Req))
			{
				//ATCMD_PRINTF("Req buf full.\n");
				AtCmdCtrl_ReqReset(pAtCmdCtrl);
				continue;
			}
			
			pAtCmdCtrl->m_Req[pAtCmdCtrl->m_nReqLen++] = byte;
			AtCmdCtrl_ReqHandler(pAtCmdCtrl);
		}
        //接收响应
		else if(pAtCmdCtrl->m_State == AT_TX_REQ || pAtCmdCtrl->m_State == AT_RX_RSP)	//receive a rsp
		{
			pAtCmdCtrl->m_State = AT_RX_RSP;
			if(pAtCmdCtrl->m_nRspLen >= sizeof(pAtCmdCtrl->m_Rsp))
			{
				//ATCMD_PRINTF("Rsp buf full.\n");
				AtCmdCtrl_RspReset(pAtCmdCtrl);
				continue;
			}
			
			pAtCmdCtrl->m_Rsp[pAtCmdCtrl->m_nRspLen++] = byte;
			state = AtCmdCtrl_RspHandler(pAtCmdCtrl);
			
			if(SwTimer_isTimerOut(&pAtCmdCtrl->m_Timer))
			{
				//ATCMD_PRINTF( "Rsp %d ms time out, len=%d\r\n", pAtCmdCtrl->m_Timer.m_TimeOutTicks, pAtCmdCtrl->m_nRspLen);
				state = AT_TIMEOUT;
			}

			if(AT_UNKNOWN != state)
			{
				AtCmdCtrl_RspDone(pAtCmdCtrl, state);
			}
		}	
	}
	
	if(pAtCmdCtrl->m_State == AT_INIT && pAtCmdCtrl->m_isFilter)
	{
		int i = 0;
		static uint32 ticks = 0;
		if(ticks == 0)	//如果处理Pending数据超过10s,则丢弃该数据包。
		{
			ticks = GET_TICKS();
		}
		else if(SwTimer_isTimerOutEx(ticks, 10000))
		{
			pAtCmdCtrl->m_nPendingLen = 0;
			memset(&pAtCmdCtrl->m_PendingBuf, 0, sizeof(pAtCmdCtrl->m_PendingBuf));
			pAtCmdCtrl->m_isFilter = False;
			ticks = 0;
			return;
		}
		
		if(pAtCmdCtrl->m_nReqLen && sizeof(pAtCmdCtrl->m_PendingBuf) > pAtCmdCtrl->m_nPendingLen + pAtCmdCtrl->m_nReqLen)
		{
			//移动所有数据从req buf 到 Pending buf。
			memcpy(&pAtCmdCtrl->m_PendingBuf[pAtCmdCtrl->m_nPendingLen], pAtCmdCtrl->m_Req, pAtCmdCtrl->m_nReqLen);
			pAtCmdCtrl->m_nPendingLen += pAtCmdCtrl->m_nReqLen;
		}
		
		pAtCmdCtrl->m_nReqLen = 0;
		for(i = 0; i < pAtCmdCtrl->m_nPendingLen; i++)
		{
			//Pending的数据可能包含多行命令，逐行移动数据从Pending buf移到req buf，再处理。
			pAtCmdCtrl->m_Req[pAtCmdCtrl->m_nReqLen++] = pAtCmdCtrl->m_PendingBuf[i];
			if(i > 2 
				&& pAtCmdCtrl->m_Req[i-1] == '\r' 
				&& pAtCmdCtrl->m_Req[i] == '\n'
				&& AtCmdCtrl_Filter(pAtCmdCtrl, pAtCmdCtrl->m_Req, &pAtCmdCtrl->m_nReqLen))
			{
				pAtCmdCtrl->m_Req[pAtCmdCtrl->m_nReqLen] = 0;
				pAtCmdCtrl->m_State = AT_INIT;
				pAtCmdCtrl->ReqProc(pAtCmdCtrl, pAtCmdCtrl->m_Req, pAtCmdCtrl->m_nReqLen);

                //pending buff的数据往前推移
				pAtCmdCtrl->m_nPendingLen -= pAtCmdCtrl->m_nReqLen;
				memcpy(pAtCmdCtrl->m_PendingBuf, &pAtCmdCtrl->m_PendingBuf[pAtCmdCtrl->m_nReqLen], pAtCmdCtrl->m_nPendingLen);
				memset(&pAtCmdCtrl->m_PendingBuf[pAtCmdCtrl->m_nPendingLen], 0, sizeof(pAtCmdCtrl->m_PendingBuf) - pAtCmdCtrl->m_nPendingLen);
				ticks = 0;
				break;
			}
		}
		
		pAtCmdCtrl->m_nReqLen = 0;
		memset(pAtCmdCtrl->m_Req, 0, sizeof(pAtCmdCtrl->m_Req));
		pAtCmdCtrl->m_isFilter = (pAtCmdCtrl->m_nPendingLen > 0);
 	}
}

//AT命令发送数据
static Bool AtCmdCtrl_Tx(AtCmdCtrl* pAtCmdCtrl, const char* pData, int len)
{
	SwTimer_ReStart(&pAtCmdCtrl->m_Timer);
	return pAtCmdCtrl->Tx(pData, len);
}

//获取参数长度
Bool AtCmdCtrl_isSearchAckByByte(const char* pStr, const char** ppStr)
{
	* ppStr = pStr;
	
	if(Null == pStr) 
		return False;
	
	if(pStr[0] == '!')
	{
		* ppStr = &pStr[1];
		return True;
	}

	return False;
}

//异步发送请求，不需要/r/n时使用
Bool AtCmdCtrl_AnsySendData(AtCmdCtrl* pAtCmdCtrl, const void* pData, uint16 len, const char* pAck, int waitMs, uint32 nTag)
{
	char* pBuf = pAtCmdCtrl->m_Rsp;
	if(pAtCmdCtrl->m_State != AT_INIT)
	{
		//ATCMD_PRINTF( "AtCmdCtrl is busy, send request failed!");
		return False;
	}
	AtCmdCtrl_RspReset(pAtCmdCtrl);
	
	pAtCmdCtrl->m_isSearchAckByByte = AtCmdCtrl_isSearchAckByByte(pAck, &pAtCmdCtrl->m_pAck);
	
	pAtCmdCtrl->m_nTag = nTag;
	SwTimer_Init(&pAtCmdCtrl->m_Timer, waitMs, TIMID_WAIT_RSP);
	
	memcpy(pBuf, pData, len);

	if(!AtCmdCtrl_Tx(pAtCmdCtrl, pBuf, len))
	{
		return False;
	}
	memset(pAtCmdCtrl->m_Rsp, 0, sizeof(pAtCmdCtrl->m_Rsp));

	pAtCmdCtrl->m_State = AT_TX_REQ;
	
	return True;
}

//atcmd需要/r/n时用这个接口，不需要/r/n时使用AtCmdCtrl_AnsySendData
Bool AtCmdCtrl_AnsySend(AtCmdCtrl* pAtCmdCtrl, const char* pAtCmd, const char* pAck, int waitMs, uint32 nTag)
{
	char* pBuf = pAtCmdCtrl->m_Rsp;
	int cmdLen = strlen(pAtCmd);
	
	if(pAtCmdCtrl->m_State != AT_INIT)
	{
		//ATCMD_PRINTF( "AtCmdCtrl is busy, send request failed!");
		return False;
	}
	
	AtCmdCtrl_RspReset(pAtCmdCtrl);

	pAtCmdCtrl->m_isSearchAckByByte = AtCmdCtrl_isSearchAckByByte(pAck, &pAtCmdCtrl->m_pAck);
	
	pAtCmdCtrl->m_nTag = nTag;
	SwTimer_Init(&pAtCmdCtrl->m_Timer, waitMs, TIMID_WAIT_RSP);
	
	strcpy(pBuf, pAtCmd);
	//Added '\n' if cmd not include '\n'
	if(pAtCmd[cmdLen-1] != '\n')
	{
		if(cmdLen+2 > sizeof(pAtCmdCtrl->m_Rsp))
		{
			return False;
		}		
		memcpy(&pBuf[cmdLen], "\r\n", 2);
		cmdLen += 2;
	}

	if(!AtCmdCtrl_Tx(pAtCmdCtrl, pBuf, cmdLen))
	{
		return False;
	}
	memset(pAtCmdCtrl->m_Rsp, 0, sizeof(pAtCmdCtrl->m_Rsp));

	if(0 == waitMs) return True;

	pAtCmdCtrl->m_State     = AT_TX_REQ;
	
	return True;
}
 
//AT命令发送数据
int AtCmdCtrl_AnsySendFmt(AtCmdCtrl* pAtCmdCtrl, const char* pAck, int waitMs, uint32 nTag, const char* lpszFormat, ...)
{
	va_list ptr;
	char pBuf[MAX_RSP_BUF_SIZE];

	va_start(ptr, lpszFormat);
	vsnprintf(pBuf, MAX_RSP_BUF_SIZE, lpszFormat, ptr);
	va_end(ptr);
		
	return AtCmdCtrl_AnsySend(pAtCmdCtrl, pBuf, pAck, waitMs, nTag);
}

//同步发送请求，等待响应，暂不使用
int AtCmdCtrl_SyncSendWithRsp(AtCmdCtrl* pAtCmdCtrl, const char* pAtCmd, const char* pAck, int waitMs, char** ppRsp)
{
	int nRet = AtCmdCtrl_SyncSend(pAtCmdCtrl, pAtCmd, pAck, waitMs);
	if(AT_SPECIFIED_OK == nRet)
	{
		* ppRsp = pAtCmdCtrl->m_Rsp;
	}

	return nRet;
}

//同步发送请求
int AtCmdCtrl_SyncSend(AtCmdCtrl* pAtCmdCtrl, const char* pAtCmd, const char* pAck, int waitMs)
{
	uint8 byte = 0;
	uint8 preByte = 0;
	int cmdLen = strlen(pAtCmd);
	int res = AT_TIMEOUT;
	uint32 initTicks = 0;
	char* pBuf= pAtCmdCtrl->m_Rsp;
	
	if(pAtCmdCtrl->m_State != AT_INIT)
	{
		//ATCMD_PRINTF( "AtCmdCtrl is busy!\n");
		return AT_BUSY;
	}

	AtCmdCtrl_RspReset(pAtCmdCtrl);

	pAtCmdCtrl->m_isSearchAckByByte = AtCmdCtrl_isSearchAckByByte(pAck, &pAtCmdCtrl->m_pAck);
	
	if(pAtCmd != pBuf)
	{
		strcpy(pBuf, pAtCmd);
	}
	
	//Added '\n' if cmd not include '\n'
	if(pAtCmd[cmdLen-1] != '\n')
	{
		if(cmdLen+2 > sizeof(pAtCmdCtrl->m_Rsp)) return AT_ERROR;
		
		memcpy(&pBuf[cmdLen], "\r\n", 2);
		cmdLen += 2;
	}
	
	if(!pAtCmdCtrl->Tx(pBuf, cmdLen))
	{
		//ATCMD_PRINTF( "%s: Bus error!", _FUNC_);
		return AT_COMM_ERROR;
	}

	if(0 == waitMs) return AT_SPECIFIED_OK;

	initTicks = GET_TICKS();
	while(!SwTimer_isTimerOutEx(initTicks, waitMs))
	{
		if(0 == SafeBuf_Read(&pAtCmdCtrl->m_SafeBuf, &byte, 1)) 
		{
			continue;
		}

		//ATCMD_PRINTF( "%c",byte);
		if(pAtCmdCtrl->m_nRspLen >= sizeof(pAtCmdCtrl->m_Rsp))
		{
			//ATPFL_WARNING("Rsp buf full.\n");
			return AT_COMM_ERROR;
		}
				
		pAtCmdCtrl->m_Rsp[pAtCmdCtrl->m_nRspLen++] = byte;
		
		if((preByte != '\r' || byte != '\n') && !pAtCmdCtrl->m_isSearchAckByByte)
		{
			preByte = byte;
			continue;
		}
		preByte = byte;
		
		//判断接收到的新航数据是否有REQ
		if(pAtCmdCtrl->m_pReqArray)
		{	
			if(AtCmdCtrl_FilterReqData(pAtCmdCtrl))
				continue;
		}
		
		if(pAtCmdCtrl->m_pAck)
		{
			if(strstr(pAtCmdCtrl->m_Rsp, pAtCmdCtrl->m_pAck))
			{
				res = AT_SPECIFIED_OK;
				break;
			}
		}
		else if(strstr(pAtCmdCtrl->m_Rsp, pAtCmdCtrl->m_pErrorTag))
		{
			res =AT_ERROR;
			break;
		}
	}

	return res;
}

//暂不使用
int AtCmdCtrl_SyncSendFmt(AtCmdCtrl* pAtCmdCtrl, const char* pAck, int waitMs, const char* lpszFormat, ...)
{
	va_list ptr;

	char pBuf[MAX_RSP_BUF_SIZE];

	va_start(ptr, lpszFormat);
	vsnprintf(pBuf, MAX_RSP_BUF_SIZE, lpszFormat, ptr);
	va_end(ptr);
		
	return AtCmdCtrl_SyncSend(pAtCmdCtrl, pBuf, pAck, waitMs);
}

//延时timeOutMs后发送挂起的命令
void AtCmdCtrl_DelaySendCmdInd(AtCmdCtrl* pAtCmdCtrl, int cmdInd, uint32 timeOutMs)
{
	AtCmdItem* pCmd = AtCmdCtrl_GetCmdInd(pAtCmdCtrl, cmdInd);
	SwTimer_Start(&pAtCmdCtrl->m_PendingTimer, timeOutMs, (uint32)pCmd);
}

//关闭挂起命令的定时器
void AtCmdCtrl_ResetPendingCmd(AtCmdCtrl* pAtCmdCtrl)
{
	SwTimer_Stop(&pAtCmdCtrl->m_PendingTimer);
}

//获取挂起的命令
int AtCmdCtrl_GetPendingCmd(AtCmdCtrl* pAtCmdCtrl)
{
	AtCmdItem* pAtCmdItem = Null;
	pAtCmdItem = (AtCmdItem*)pAtCmdCtrl->m_PendingTimer.m_Id;
	
	return pAtCmdCtrl->m_PendingTimer.m_isStart ? pAtCmdItem->cmdInd : -1;
}

//定时器任务处理
Bool AtCmdCtrl_TimerProc(AtCmdCtrl* pAtCmdCtrl)
{
	AtCmdItem* pAtCmdItem = Null;
	
    //挂起的待处理命令进行处理
	if(SwTimer_isTimerOut(&pAtCmdCtrl->m_PendingTimer))
	{
		pAtCmdItem = (AtCmdItem*)pAtCmdCtrl->m_PendingTimer.m_Id;
		if(pAtCmdItem == AtCmdCtrl_GetCmdInd(pAtCmdCtrl, pAtCmdItem->cmdInd))
		{
			pAtCmdCtrl->SendCmdItem(pAtCmdItem);
		}
		return True;
	}
	
	return False;
}

//服务器的请求或者SIM868的URC处理
void AtCmdCtrl_AtCmdReqProc(AtCmdCtrl* pAtCmdCtrl, char* pReq, uint16 len)
{
	char* p = Null;
	const AtReqItem* pReqItem = Null;
	
	for(pReqItem = pAtCmdCtrl->m_pReqArray; pReqItem->reqStr; pReqItem++)
	{
		p = (char*)bytesSearchEx((uint8*)pReq, len, pReqItem->reqStr);
		if(Null == p) continue;

        //去掉解析的头，从后面开始进行数据处理
		pReqItem->ReqProc(p, len - (p-pReq));
		break;
	}
}

//  响应处理函数；返回值说明:
//	RC_UNKNOWN	: 不认识这个响应, 不是本模块的响应
//	RC_DONOTHING: 认识这个响应, 但是接受到的响应不完整，没有做任何处理。
//	RC_DONE		: 认识这个响应, 并且已经处理完毕。
ATCMD_RC AtCmdCtrl_AtCmdRspProc(AtCmdCtrl* pAtCmdCtrl, const AtCmdItem* pAtCmd, AtCmdState state, const char* pRsp, int len)
{
	Bool isReSend = True;
	ATCMD_RC nRet = ATCRC_SUCCESS;
	AtCmdItem* pAtCmdItem = Null;

	//过滤TAG值，校验pAtCmd是否合法
	if(pAtCmd != AtCmdCtrl_GetCmdInd(pAtCmdCtrl, pAtCmd->cmdInd)) 
	{
		return ATCRC_UNKNOWN;
	}

	pAtCmdCtrl->m_ErrCounter = (AT_SPECIFIED_OK != state) ? (pAtCmdCtrl->m_ErrCounter+1) : 0 ;	//统计连续错误次数
	
	for(pAtCmdItem = pAtCmdCtrl->m_pCmdArray; pAtCmdItem->atCmd; pAtCmdItem++)
	{
		if(pAtCmd == pAtCmdItem)
		{
			ATCMD_RC nRet = (state == AT_SPECIFIED_OK) ? ATCRC_SUCCESS : ATCRC_RESEND;

			if(pAtCmdItem->RspHandler)
			{
				AtCmdItem* pOutAtCmd = Null;
				nRet = pAtCmdItem->RspHandler(state, (char*)pRsp, &pOutAtCmd);
			
				if(pOutAtCmd)           //有下一个处理的命令
				{
					isReSend = False;   
					pAtCmd = pOutAtCmd; //进行下一个命令的倒计时，10ms
					goto Next;
				}
				else if(nRet == ATCRC_DONE)
				{
					isReSend = False;
					return ATCRC_DONE;	//结束当前命令链
				}
			}
			
			if(nRet == ATCRC_SUCCESS)   //配置成功的话，执行下一条命令，只要不是命令链结束
			{
				pAtCmd++;	//执行下一条命令
				isReSend = False;
				if(pAtCmd->atCmd)
				{
				}
				else
				{
					pAtCmd = Null;
					return ATCRC_DONE;	//结束当前命令链
				}
			}
			else
			{
				//Do nothing, pAtCmd保持不变, 重新执行pAtCmd
			}
			break;
		}
	}

Next:
	//不能同步连续执行多个AT命令，否则无法及时处理接收到的AT命令请求, 在定时器中异步执行下一个AT命令
	if(pAtCmd)
	{
		pAtCmdCtrl->m_ReTxCounter = isReSend ? (pAtCmdCtrl->m_ReTxCounter+1) : 0 ;	//置重发计数器

		if(pAtCmdCtrl->IsAllowResend(pAtCmdCtrl, pAtCmd, pAtCmdCtrl->m_ReTxCounter))	//相同的命令最多重发10次
		{
			SwTimer_Start(&pAtCmdCtrl->m_PendingTimer, isReSend ? 1000 : 10, (uint32)pAtCmd);
		}
		else
		{
			nRet = ATCRC_FAILED;
		}
	}

	return nRet;
}

//判断是否允许重发
Bool AtCmdCtrl_IsAllowResend(AtCmdCtrl* pAtCmdCtrl, const AtCmdItem* pAtCmd, int reTxCount)
{
	return pAtCmdCtrl->m_ErrCounter <= 3 && reTxCount <= pAtCmdCtrl->m_MaxTxCounter;
}

//寻找at命令处理的指针函数
AtCmdItem* AtCmdCtrl_GetCmdInd(AtCmdCtrl* pAtCmdCtrl, int cmdInd)
{
	AtCmdItem* pAtCmdItem = pAtCmdCtrl->m_pCmdArray;

	for(pAtCmdItem = pAtCmdCtrl->m_pCmdArray; pAtCmdItem->atCmd; pAtCmdItem++)
	{
		if(cmdInd == pAtCmdItem->cmdInd)
			return pAtCmdItem;
	}
	
	//ATCMD_PRINTF("No AT CMD[%d].\n", cmdInd);
	return Null;
}

//判断是否命令链结束还是重发,暂不使用
ATCMD_RC AtCmdCtrl_CmdProcDone(AtCmdState state, char* pRsp, struct _AtCmdItem** pNextAtCmd)
{
	if(state == AT_SPECIFIED_OK || state == AT_ERROR)
		return ATCRC_DONE;
	return ATCRC_RESEND;
}

//根据AtCmdItem发送命令
Bool AtCmdCtrl_SendCmdItem(AtCmdCtrl* pAtCmdCtrl, AtCmdItem* pCmd)
{
    //异步发送，3S内等待响应回复
	return AtCmdCtrl_AnsySend(pAtCmdCtrl, pCmd->atCmd, pCmd->atAck, 3000, (uint32)pCmd);
}

//根据命令索引发送命令数据
Bool AtCmdCtrl_SendCmdByInd(AtCmdCtrl* pAtCmdCtrl, uint8 ind)
{
	AtCmdItem* pCmd = AtCmdCtrl_GetCmdInd(pAtCmdCtrl, ind);

	if(pCmd && AtCmdCtrl_IsIdle(pAtCmdCtrl))
		return pAtCmdCtrl->SendCmdItem(pCmd);

	return False;
}

//AT命令处理运行函数
void AtCmdCtrl_Run(AtCmdCtrl* pAtCmdCtrl)
{
    //接收数据处理
	AtCmdCtrl_RcvProc(pAtCmdCtrl);

    //响应超时
	if(SwTimer_isTimerOut(&pAtCmdCtrl->m_Timer))
	{
		AtCmdCtrl_RspDone(pAtCmdCtrl, AT_TIMEOUT);
	}
	
	if(pAtCmdCtrl->m_State == AT_INIT)
	{
		if(pAtCmdCtrl->m_nReqLen)
		{
			//ATCMD_PRINTF("AtCmdCtrl_ReqHandler()\n");
			AtCmdCtrl_ReqHandler(pAtCmdCtrl);
		}
		AtCmdCtrl_TimerProc(pAtCmdCtrl);
	}
}

/*****************************************end of AtCmdCtrl.c*****************************************/
