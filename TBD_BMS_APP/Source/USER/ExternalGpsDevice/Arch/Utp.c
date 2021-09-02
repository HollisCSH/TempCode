/*
 * Copyright (c) 2020-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 * UTP(Uart transfer protocol)实现文件
 */

#include "ArchDef.h"
#include "Utp.h"
//#include "flexcan_driver.h"
//#include "BSPCan.h"
#include "BSP_CAN.h"
#include "bsptypedef.h"

#ifdef CANBUS_MODE_JT808_ENABLE	

#ifdef BMS_DEBUG_ENABLE
#define		_UTP_DEBUG_MSG(fmt,...)		printf(fmt,##__VA_ARGS__)
#else
#define		_UTP_DEBUG_MSG(fmt,...)
#endif 


extern uint16_t gCurRevLen ;
static Bool Utp_RspProc(Utp* pUtp, const uint8_t* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode);
static void Utp_RcvRsp(Utp* pUtp, const uint8_t* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode);

Bool Utp_isIdle(const Utp* pUtp)
{
	return pUtp->state == UTP_FSM_INIT;
}

/*
把Payload数据加上帧头和帧为，并且转码。
参数说明：
	pSrc:指向Payload数据，包含检验和，不包括帧头帧尾
	srcLen：Src数据长度。
	srcInd: 处理位置
	pDst: 目标数据Buff
	dstLen：目标Buff数据长度
*/
Bool Utp_FramePkt(const UtpFrameCfg* frameCfg, const uint8_t* pSrc, uint16_t srcLen, int* srcInd, uint8_t* pDst, int* dstSize)
{
	int j = 0;
	int i = 0;
	int bRet = True;
	
	if (*srcInd == 0)
		pDst[j++] = frameCfg->head;
	
	for(i = *srcInd; i < srcLen; i++)
	{	
		if (frameCfg->head == pSrc[i])
		{
			pDst[j++] = frameCfg->transcode;
			pDst[j++] = frameCfg->transcodeHead;
		}
		else if (frameCfg->tail == pSrc[i])
		{
			pDst[j++] = frameCfg->transcode;
			pDst[j++] = frameCfg->transcodeTail;
		}
		else if (frameCfg->transcode == pSrc[i])
		{
			pDst[j++] = frameCfg->transcode;
			pDst[j++] = frameCfg->transcodeShift;
		}
		else
		{
			pDst[j++] = pSrc[i];
		}

		//pDst buffer 满
		if(j >= (*dstSize) - 2)
		{
			++i;
			if(i == srcLen)
			{
				break;
			}
			else
			{
				bRet = False;
				goto End;
			}
		}
	}
	
	pDst[j++] = frameCfg->tail;
	
End:	
	*srcInd = i;
	*dstSize = j;
	return bRet;
}

//网络字节转换为
Bool Utp_ConvertToHost(const UtpFrameCfg* frameCfg, uint8_t* dst, uint16_t dstSize, uint16_t* dstInd, FRAME_STATE* state, uint8_t byte)
{
	Bool bRet = False;
	int i = *dstInd;
#define FRAME_RESET() {i = 0; *state = FRAME_INIT;}

	if (*state == FRAME_INIT)
	{
		if (frameCfg->head == byte)
		{
			i = 0;
			*state = FRAME_FOUND_HEAD;

			dst[i++] = byte;
		}
	}
	else if (*state == FRAME_FOUND_HEAD)
	{
		if (frameCfg->transcode == byte)
		{
			*state = FRAME_FOUND_ESCAPE;
		}
		else if (frameCfg->tail == byte)
		{
			dst[i++] = byte;
			*state = FRAME_DONE;
			bRet = True;
			goto End;
		}
		else
		{
			dst[i++] = byte;
		}
	}
	else if (*state == FRAME_FOUND_ESCAPE)
	{
		if (frameCfg->transcodeHead == byte)		byte = frameCfg->head;
		else if (frameCfg->transcodeTail == byte)	byte = frameCfg->tail;
		else if (frameCfg->transcodeShift == byte)	byte = frameCfg->transcode;
		else
		{
			FRAME_RESET();
			goto End;
		}
		dst[i++] = byte;
		*state = FRAME_FOUND_HEAD;
	}

	if (i >= dstSize)
	{
		//Printf("*Buf is full*.\n");
//		ASRT(i < dstSize);
		FRAME_RESET();
	}

End:

	*dstInd = i;
	return bRet;
}

//  把Payload数据加上帧头和帧尾，并且转码。
//  参数说明：
//	pSrc:指向Payload数据，包含检验和，不包括帧头帧尾
//	srcLen：Src数据长度。
//	srcInd: 处理位置
//	pDst: 目标数据Buff
//	dstLen：目标Buff数据长度
Bool Utp_FramePkt_NB(const uint8* pSrc, uint16 srcLen, int* srcInd, uint8* pDst, int* dstSize)
{
#define     UTP_FRAME_HEAD 		        0x7E	//0x8C 0x81
#define     UTP_FRAME_TAIL 		        0xFF	//0x8C 0x00
#define     UTP_FRAME_ESCAPE 	        0x8C	//0x8C 0x73
#define     UTP_RSP_CODE     	        0x80
#define     UTP_FRAME_ESCAPE_HREAD      0x81
#define     UTP_FRAME_ESCAPE_TAIL       0x00
#define     UTP_FRAME_ESCAPE_ESCAPE     0x73
#define     UTP_ENDLESS                 0xFF
    
	int j = 0;
	int i = 0;
	int bRet = True;
	
	if(*srcInd == 0)
		pDst[j++] = UTP_FRAME_HEAD;
	
	for(i = *srcInd; i < srcLen; i++)
	{		
		switch(pSrc[i])
		{
			case UTP_FRAME_HEAD:    //帧头，0x7E
				pDst[j++] = UTP_FRAME_ESCAPE;
				pDst[j++] = UTP_FRAME_ESCAPE_HREAD;
				break;
			case UTP_FRAME_TAIL:    //帧尾，0xff
				pDst[j++] = UTP_FRAME_ESCAPE;
				pDst[j++] = UTP_FRAME_ESCAPE_TAIL;
				break;
			case UTP_FRAME_ESCAPE:  //转化符，0x8c
				pDst[j++] = UTP_FRAME_ESCAPE;
				pDst[j++] = UTP_FRAME_ESCAPE_ESCAPE;
				break;
			default:	
				pDst[j++] = pSrc[i];
				break;
		}

		//pDst buffer 满
		if(j >= (*dstSize) - 2)
		{
			++i;
			if(i == srcLen)
			{
				break;
			}
			else
			{
				bRet = False;
				goto End;
			}
		}
	}
	
	pDst[j++] = UTP_FRAME_TAIL;
	
End:	
	*srcInd = i;
	*dstSize = j;
	return bRet;
}

//将接收的网络字节转换为正常的数据格式
Bool Utp_ConvertToHost_NB(uint8* dst, uint16 dstSize, uint16* dstInd, FRAME_STATE *state, uint8 byte)
{
#define     UTP_FRAME_HEAD 		        0x7E	//0x8C 0x81
#define     UTP_FRAME_TAIL 		        0xFF	//0x8C 0x00

#define     UTP_FRAME_ESCAPE 	        0x8C	//0x8C 0x73
#define     UTP_RSP_CODE     	        0x80
#define     UTP_FRAME_ESCAPE_HREAD      0x81
#define     UTP_FRAME_ESCAPE_TAIL       0x00
#define     UTP_FRAME_ESCAPE_ESCAPE     0x73
#define     UTP_ENDLESS                 0xFF
    
	Bool bRet = False;
	int i = *dstInd;
	#define FRAME_RESET() {i = 0; *state = FRAME_INIT;}

	if(UTP_FRAME_HEAD == byte)
	{
		i = 0;
		*state = FRAME_FOUND_HEAD;
		
		dst[i++] = byte;
	}
	else if(*state == FRAME_FOUND_HEAD)
	{
		if(UTP_FRAME_ESCAPE == byte)
		{
			*state = FRAME_FOUND_ESCAPE;
		}
		else if(UTP_FRAME_TAIL == byte)
		{
			dst[i++] = byte;
			*state = FRAME_DONE;
			bRet = True;
			goto End;
		}
		else
		{
			dst[i++] = byte;
		}
	}
	else if(*state == FRAME_FOUND_ESCAPE)
	{
		switch(byte)
		{
			case UTP_FRAME_ESCAPE_HREAD : byte = UTP_FRAME_HEAD  ; break;
			case UTP_FRAME_ESCAPE_TAIL  : byte = UTP_FRAME_TAIL  ; break;
			case UTP_FRAME_ESCAPE_ESCAPE: byte = UTP_FRAME_ESCAPE; break;
			default:
			{	//Data is invalid, discard.
				//Printf("Frame error, byte = 0x%x\n", byte);
				FRAME_RESET();
				goto End;
			}
		}
		dst[i++] = byte;
		*state = FRAME_FOUND_HEAD;
	}
	
	if(i >= dstSize)
	{
		//Printf("*Buf is full*.\n");
//		ASRT(i < dstSize);
		FRAME_RESET();
	}

End:
	
	* dstInd = i;
	return bRet;
}

//每次发送一帧字节
static uint16_t Utp_SendFrame(Utp* pUtp, uint8_t cmd, const void* pData, uint16_t len)
{
	#define BUF_SIZE 64
	int i = 0;
	uint8_t byte[BUF_SIZE];
	int j = 0;
	if( !pUtp || !pData || !len ) return 0;

	while(i < len)
	{
		j = sizeof(byte);
		Utp_FramePkt(pUtp->frameCfg, (uint8_t*)pData, len, &i, byte, &j);
		pUtp->cfg->TxFn(cmd, byte, j);
	}
	
	return len;
}

static const UtpCmd* Utp_FindCmdItem(Utp* pUtp, uint8_t cmd)
{
	const UtpCmd* pCmd = pUtp->cfg->cmdArray;
	for (int i = 0; i < pUtp->cfg->cmdCount; i++, pCmd++)
	{
		if (cmd == pCmd->cmd)
		{
			return pCmd;
		}
	}
	return Null;
}

static void Utp_ResetTxBuf(Utp* pUtp)
{
	pUtp->state = UTP_FSM_INIT;
	pUtp->txBufLen = 0;
	pUtp->reTxCount = 0;
}
/*
static void Utp_ResetRxBuf(Utp* pUtp)
{
	//Queue_reset(&pUtp->rxBufQueue);
	CirBuffReset(&pUtp->rxBuffCirBuff);
	pUtp->searchIndex = 0;
	pUtp->head = 0;
	pUtp->rxDataTicks = 0;
}
*/
/************************************
函数功能：协议对象复位，
	取消已经发送正在等待响应的命令，
	取消所有等待发送的命令
	清除Rx缓冲区和Req缓冲区
参数说明：
	pUtp：Utp对象。
返回值无
***************************************/
void Utp_Reset(Utp* pUtp)
{
	//如果当前有正在发送的命令，立刻终止
	if(pUtp->pWaitRspCmd)
	{
		Utp_RspProc(pUtp, Null, 0, RSP_CANCEL);
	}
	
	//Utp_ResetRxBuf(pUtp);
	Utp_ResetTxBuf(pUtp);
	
	const UtpCmd* pCmd = pUtp->cfg->cmdArray;
	for (int i = 0; i < pUtp->cfg->cmdCount; i++, pCmd++)
	{
		//停止所有待命令发送
		pCmd->pExt->sendDelayMs = 0;
		if(pCmd->type == UTP_WRITE && pCmd->pData && pCmd->pStorage && pCmd->dataLen )
		{
			memcpy(pCmd->pData, pCmd->pStorage,pCmd->dataLen );
		}
	}
}

static UTP_EVENT_RC Utp_Event(Utp* pUtp, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	const UtpCfg* cfg = pUtp->cfg;
	UTP_EVENT_RC opRc = cfg->TresferEvent ? cfg->TresferEvent(cfg->pCbObj, pCmd, ev) : UTP_EVENT_RC_SUCCESS;
	if (opRc != UTP_EVENT_RC_SUCCESS) return opRc;
	
	opRc = pCmd->Event ? pCmd->Event(cfg->pCbObj, pCmd, ev) : UTP_EVENT_RC_SUCCESS;

	return opRc;
}


static Bool Utp_RspProc(Utp* pUtp, const uint8_t* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode)
{
	pUtp->pWaitRspCmd->pExt->rcvRspErr = rspCode;
	if(rspCode == RSP_SUCCESS)
	{
		Utp_RcvRsp(pUtp, pRsp, frameLen, rspCode);
	}
	else
	{
		pUtp->pWaitRspCmd->pExt->transferData = Null;
		pUtp->pWaitRspCmd->pExt->transferLen = 0;
		Utp_Event(pUtp, pUtp->pWaitRspCmd, UTP_REQ_FAILED);
	}
	
	//响应处理结束
	SwTimer_Stop(&pUtp->waitRspTimer);
	pUtp->pWaitRspCmd = Null;
	pUtp->rxRspTicks = GET_TICKS();

	Utp_ResetTxBuf(pUtp);
	return True;
}

static void Utp_ReqProc(Utp* pUtp, const uint8_t* pReq, int frameLen)
{	
	const UtpFrameCfg* frameCfg = pUtp->frameCfg;
	uint8_t rc = frameCfg->result_UNSUPPORTED;
	uint8* txBuf = frameCfg->txRspBuf ? frameCfg->txRspBuf : frameCfg->txBuf;
	int txBufLen = frameCfg->txRspBuf ? frameCfg->txRspBufLen : frameCfg->txBufLen;
	const uint8* pData = &pReq[frameCfg->dataByteInd];
	const UtpCmd* pCmd = Utp_FindCmdItem(pUtp, pReq[frameCfg->cmdByteInd]);
	int dlc = 1;


	_UTP_DEBUG_MSG("%s Rev Len:%d\r\n",pCmd->cmdName , frameLen );
	
	if( pCmd && (pCmd->type == UTP_EVENT || pCmd->type == UTP_EVENT_NOTIFY ))
	{
		rc = frameCfg->result_SUCCESS;
		frameLen -= frameCfg->dataByteInd;

		//传输数据
		pCmd->pExt->transferData = (uint8*)pData;
		pCmd->pExt->transferLen = frameLen;
		gCurRevLen = MIN(pCmd->storageLen, frameLen) ;
		//
		if (pCmd->pStorage && pCmd->storageLen)
		{
			if (memcmp(pCmd->pStorage, pData, pCmd->storageLen) != 0)
			{
				UTP_EVENT_RC evRc = Utp_Event(pUtp, pCmd, UTP_CHANGED_BEFORE);
				if (evRc == UTP_EVENT_RC_SUCCESS)
				{
					memcpy(pCmd->pStorage, pData, MIN(pCmd->storageLen, frameLen));
					Utp_Event(pUtp, pCmd, UTP_CHANGED_AFTER);
				}
			}
		}
		//预置默认的应答数据指针
		pCmd->pExt->transferData = pCmd->pData;
		pCmd->pExt->transferLen = pCmd->dataLen;
		rc = Utp_Event(pUtp, pCmd, UTP_GET_RSP);
		if (rc == frameCfg->result_SUCCESS && pCmd->pExt->transferData && pCmd->pExt->transferLen )
		{
			if (dlc + pCmd->pExt->transferLen <= txBufLen)
			{
				memcpy(&txBuf[frameCfg->dataByteInd + 1], pCmd->pExt->transferData, pCmd->pExt->transferLen);
				dlc += pCmd->pExt->transferLen;
			}
			else
			{
				//分配的Buff长度不够
				_UTP_DEBUG_MSG("%s size not enough.", frameCfg->txRspBuf ? "txRspBuf" : "txBuf");
				Assert(False);
			}
		}

		Utp_Event(pUtp, pCmd, UTP_REQ_SUCCESS);
	}
	
	if (pCmd==Null || pCmd->type == UTP_EVENT)
	{
		int txlen = 0 ;
		txBuf[frameCfg->cmdByteInd] = pReq[frameCfg->cmdByteInd];
		txBuf[frameCfg->dataByteInd] = rc;

		txlen = frameCfg->FrameBuild(pUtp
			, pReq[frameCfg->cmdByteInd]
			, &txBuf[frameCfg->dataByteInd]
			, dlc
			, Null//pReq
			, txBuf );
		Utp_SendFrame(pUtp, pReq[frameCfg->cmdByteInd], txBuf, txlen );
	}
	//
	//if( pUtp->state != UTP_FSM_WAIT_RSP )
	//{
	//	Utp_ResetTxBuf(pUtp);
	//}
}


//接收帧处理
void Utp_RcvFrameHandler(Utp* pUtp, const uint8* pFrame, int frameLen)
{
	_UTP_DEBUG_MSG("CAN Frame:[0x%02X]%d:%d\r\n",pFrame[5],frameLen,pUtp->state);
	
	if (UTP_FSM_WAIT_RSP == pUtp->state)
	{
		if (pFrame[pUtp->frameCfg->cmdByteInd] == pUtp->frameCfg->txBuf[pUtp->frameCfg->cmdByteInd])
		{
			//判断请求帧和响应帧是否匹配
			if (pUtp->frameCfg->FrameVerify(pUtp, pFrame, frameLen, pUtp->frameCfg->txBuf))
			{
				Utp_RspProc(pUtp, pFrame, frameLen, RSP_SUCCESS);	//响应处理
			}
		}
		else if(pUtp->frameCfg->txRspBuf)
		{
			Utp_ReqProc(pUtp, pFrame, frameLen);	//请求处理
		}
	}
	else
	{
		Utp_ReqProc(pUtp, pFrame, frameLen);	//请求处理
	}

	return;
}

static Bool Utp_SendReq(Utp* pUtp, const UtpCmd* pCmd)
{
	if(pUtp->state != UTP_FSM_INIT) 
	{
		//Printf("Utp is busy.\n");
		return False;
	}
	//设置默认的发送参数
	pUtp->waitRspMs = pUtp->frameCfg->waitRspMsDefault;	//默认等待响应的时间为1秒
	pUtp->maxTxCount = 3;		//默认的重发次数为3
	if (UTP_EVENT_RC_SUCCESS != Utp_Event(pUtp, pCmd, UTP_TX_START)) return False;

	Utp_ResetTxBuf(pUtp);
	//Utp_ResetRxBuf(pUtp);

	pUtp->txBufLen = pUtp->frameCfg->FrameBuild(pUtp, pCmd->cmd, pCmd->pExt->transferData, pCmd->pExt->transferLen, Null, pUtp->frameCfg->txBuf);

	pUtp->reTxCount = 1;
	Utp_SendFrame(pUtp, pCmd->cmd, pUtp->frameCfg->txBuf, pUtp->txBufLen);

	if (UTP_EVENT_RC_SUCCESS != Utp_Event(pUtp, pCmd, UTP_TX_DONE)) return False;
	if(pUtp->waitRspMs)
	{
		pUtp->state = UTP_FSM_WAIT_RSP;
		SwTimer_Start(&pUtp->waitRspTimer, pUtp->waitRspMs, 0);
	}
	else
	{
		Utp_ResetTxBuf(pUtp);
	}

	return True;
}

/*******************************************
函数功能：发送命令。
参数说明：
	pUtp：Utp对象。
	cmd：发送的命令码。
返回值：无
*******************************************/
void Utp_SendCmd(Utp* pUtp, uint8_t cmd)
{
	Utp_DelaySendCmd(pUtp, cmd, 0);
}

/*******************************************
函数功能：延时发送命令。
参数说明：
	pUtp：Utp对象。
	cmd：发送的命令码。
	delayMs：延时发送事件，0表示不延时，立即发送
返回值：无
*******************************************/
void Utp_DelaySendCmd(Utp* pUtp, uint8_t cmd, uint32_t delayMs)
{
	const UtpCmd* pCmd = Utp_FindCmdItem(pUtp, cmd);
	if (delayMs == 0) delayMs = 1;	//delayMs=1：表示立即发送
	if (pCmd && pCmd->pExt)
	{
		pCmd->pExt->sendDelayMs = delayMs;
	}
}

void Utp_DeleteCmd( Utp *pUtp , uint8_t cmd )
{
	const UtpCmd* pCmd = Utp_FindCmdItem(pUtp, cmd);
	const UtpCmd* pWaitRspCmd = pUtp->pWaitRspCmd ;

	pCmd->pExt->sendDelayMs = 0x00;
	if( pCmd->cmd == pWaitRspCmd->cmd )	// 要删除的指令,正在等待回应
	{
		if(pUtp->pWaitRspCmd)
		{
			Utp_RspProc(pUtp, Null, 0, RSP_CANCEL);
		}
		
		//Utp_ResetRxBuf(pUtp);
		Utp_ResetTxBuf(pUtp);
		//
		SwTimer_Stop(&pUtp->waitRspTimer);
		pUtp->pWaitRspCmd = Null;
		pUtp->rxRspTicks = GET_TICKS();
	}
}


static void Utp_RcvRsp(Utp* pUtp, const uint8_t* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode)
{
	const UtpFrameCfg* frameCfg = pUtp->frameCfg;
	uint8_t rspDlc = frameLen - frameCfg->dataByteInd;
	const UtpCmd* pCmd = pUtp->pWaitRspCmd;

	if(pCmd == Null) return;
	if(pCmd->cmd != pRsp[frameCfg->cmdByteInd]) return;

	pCmd->pExt->transferData = (uint8*)&pRsp[frameCfg->dataByteInd];
	pCmd->pExt->transferLen = rspDlc;
	pCmd->pExt->rxRspTicks = GET_TICKS();

	int minlen = MIN(rspDlc, pCmd->storageLen);


	_UTP_DEBUG_MSG("%s Rev Len:%d\r\n",pCmd->cmdName , frameLen );

	if(pRsp[frameCfg->dataByteInd] == frameCfg->result_SUCCESS)
	{
		if(pRsp && pCmd->pStorage && pCmd->storageLen) 
		{
			const uint8_t* pRspData = &pRsp[frameCfg->dataByteInd + 1];
			if(pCmd->type == UTP_WRITE)
			{
				if(rspDlc && pCmd->pData) 
				{
					//更新storage值
					memcpy(pCmd->pData, pRspData, MIN(rspDlc, pCmd->dataLen));
				}
			}
			else if(pCmd->type == UTP_READ && pCmd->pStorage)
			{
				if(memcmp(pCmd->pStorage, pRspData, minlen) != 0)
				{
					UTP_EVENT_RC evRc = Utp_Event(pUtp, pCmd, UTP_CHANGED_BEFORE);
					if(evRc == UTP_EVENT_RC_SUCCESS)
					{
						//更新storage值
						memcpy(pCmd->pStorage, pRspData, minlen);
						Utp_Event(pUtp, pCmd, UTP_CHANGED_AFTER);
					}
				}
			}
		}
		Utp_Event(pUtp, pCmd, UTP_REQ_SUCCESS);
	}
	else
	{
		Utp_Event(pUtp, pCmd, UTP_REQ_FAILED);
	}
	
}

static void Utp_CheckReq(Utp* pUtp)
{
	if(UTP_FSM_INIT != pUtp->state) return;

	UtpCmdEx* pExt;
	const UtpCmd* pCmd = pUtp->cfg->cmdArray;
	for (int i = 0; i < pUtp->cfg->cmdCount; i++, pCmd++)
	{
		pExt = pCmd->pExt;
		if(pExt == Null || pCmd->type == UTP_EVENT) continue;

		//帧间隔是否超时，
		if(!SwTimer_isTimerOutEx(pUtp->rxRspTicks, pUtp->frameCfg->sendCmdIntervalMs)) break;


		//是否有待发的READ/WRITE命令(pExt->sendDelayMs > 0)
		if(pExt->sendDelayMs && SwTimer_isTimerOutEx(pExt->rxRspTicks, pExt->sendDelayMs))
		{
			if (pCmd->type == UTP_READ)
			{
				pExt->transferData = pCmd->pData;
				pExt->transferLen  = pCmd->dataLen;
			}
			else
			{
				pExt->transferData = pCmd->pStorage;
				pExt->transferLen  = pCmd->storageLen;
			}

			Utp_SendReq(pUtp, pCmd);
			//清除发送标志
			pExt->sendDelayMs = 0;

			if (pCmd->type == UTP_NOTIFY)
			{
				pCmd->pExt->rxRspTicks = GET_TICKS();
				Utp_ResetTxBuf(pUtp);
				Utp_Event(pUtp, pCmd, UTP_REQ_SUCCESS);
			}
			else
			{
				pUtp->pWaitRspCmd = pCmd;
			}
			break;
		}

		//是否有待发的WRITE命令
		if(pCmd->type == UTP_WRITE && pCmd->pStorage && pCmd->pData)
		{
			if(memcmp(pCmd->pStorage, pCmd->pData, pCmd->storageLen) != 0)
			{
				pExt->transferData = pCmd->pStorage;
				pExt->transferLen  = pCmd->storageLen;

				Utp_SendReq(pUtp, pCmd);

				pExt->sendDelayMs = 0;
				pUtp->pWaitRspCmd = pCmd;
				break;
			}
		}
	}
}

/************************************
函数功能：从总线或者中断中接收数据，保存到接收缓冲区rxBufQueue队列中
参数说明：
	pUtp：Utp对象。
	pData：从总线或者中断中接收到的数据。
	len：数据长度。
返回值无
***************************************/
#if 0
void Utp_RxData(Utp* pUtp, const uint8_t* pData, int len)
{
	//检查接收数据的间隔是否超时，如果是则必须丢弃之前接收到的数据。
	if (pUtp->rxDataTicks && SwTimer_isTimerOutEx(pUtp->rxDataTicks, pUtp->frameCfg->rxIntervalMs))
	{
		//清除接收缓冲区
		Utp_ResetRxBuf(pUtp);
	}
	pUtp->rxDataTicks = GET_TICKS();

	Queue_writeBytes(&pUtp->rxBufQueue, pData, len);
}

//检测是否接收到数据帧
void Utp_CheckRxFrame(Utp* pUtp)
{
	const UtpFrameCfg* frameCfg = pUtp->frameCfg;
	if (0 == pUtp->head)
	{
		//搜索头字节
		pUtp->searchIndex = 0;
		pUtp->head = Queue_searchByte(&pUtp->rxBufQueue, frameCfg->head, &pUtp->searchIndex);
		if (pUtp->head)
		{
			pUtp->headIndex = pUtp->searchIndex;
			pUtp->searchIndex++;
		}
		else if(pUtp->searchIndex)
		{
			//移除队列中前面无效的数据
			Queue_popElements(&pUtp->rxBufQueue, pUtp->searchIndex + 1);
		}
	}
	else
	{
		uint16_t frameLen = 0;
		Bool isOk = False;
		//搜索尾字节
		if (Queue_searchByte(&pUtp->rxBufQueue, frameCfg->tail, &pUtp->searchIndex))
		{
			FRAME_STATE state = FRAME_INIT;

			for (int i = pUtp->headIndex; i <= pUtp->searchIndex; i++)
			{
				//转码,转码出来的数据包含帧头和帧尾
				isOk = Utp_ConvertToHost(frameCfg, frameCfg->transcodeBuf, frameCfg->transcodeBufLen, &frameLen, &state, pUtp->head[i]);
				if (isOk)
				{
					break;
				}
			}
			pUtp->head = 0;

			//数据队列中移除
			Queue_popElements(&pUtp->rxBufQueue, pUtp->searchIndex + 1);

			//帧校验，去掉帧头和帧尾
			if (isOk && frameCfg->FrameVerify(pUtp, &frameCfg->transcodeBuf[1], frameLen-2, Null))
			{
				//帧处理,去掉帧头和帧尾
				Utp_RcvFrameHandler(pUtp, &frameCfg->transcodeBuf[1], frameLen-2);
			}
		}

		//如果队列满（说明丢失帧尾字节），清除队列所有内容
		if (Queue_isFull(&pUtp->rxBufQueue))
		{
			Utp_ResetRxBuf(pUtp);
		}
	}
}
#else

// 反转义
static uint16_t _ParapBuff( uint8_t* pInbuff,uint8_t*poutbuff,uint16_t len)
{
	uint16_t i = 0 , j = 0 ;
	if( !pInbuff || !pInbuff || !len ) return 0;
	//
	while( j < len )
	{
		if( pInbuff[j] == 0x7D )
		{
			if( j + 1 < len )
			{
				if( pInbuff[j+1] == 0x01 )
				{
					poutbuff[i++] = 0x7d ;
				}
				else if( pInbuff[j+1] == 0x02 )
				{
					poutbuff[i++]= 0x7E ;
				}
				else return 0;
			}
			else return 0;
			j += 2 ;
		}
        else
        {
            poutbuff[i++] = pInbuff[j++];
        }
	}
	return i ;
}



// 解出头 尾的包
/*
7E A1 01 00 01 01 93 02 
7E FC 01 00 01 01 81 08 
2C 0D AF 3A 09 D6 50 20 
7E 
08 02 7E 
7E A1 01 00 01 01 93 02 
08 02 7E
*/
static uint16_t _DCodeCirBuff( const UtpFrameCfg *pframeCfg, 
	pCirBuff pcirbuff, uint8_t *poutbuff , uint16_t size )
{
	uint16_t pos = 0 , len = 0 ;
	if( !pcirbuff || !poutbuff || !size || !pframeCfg ) return 0;
	//找到头尾的处理

UTP_REDCODE:
	while( pcirbuff->miHead != pcirbuff->miTail )
	{
		if( pcirbuff->mpBuff[pcirbuff->miHead] == pframeCfg->head ) break ;
		_CIR_LOOP_ADD(pcirbuff->miHead, 1, pcirbuff->miSize );
	}
	pos = pcirbuff->miHead ;
	len = 0 ;
	while( pos != pcirbuff->miTail && len < size )
	{
		poutbuff[len++] = pcirbuff->mpBuff[pos];
		_CIR_LOOP_ADD(pos, 1, pcirbuff->miSize );
		
		if( poutbuff[len-1] == pframeCfg->tail )
		{
			if( len > 8 )
			{
				pcirbuff->miHead = pos ;
				return len ;
			}
			else if( len > 1 )	// 错误包
			{
				_CIR_LOOP_ADD(pcirbuff->miHead, 1, pcirbuff->miSize );
				goto UTP_REDCODE;
			}
		}
	}
	// 错误包
	if( len >= size )	
	{
		_CIR_LOOP_ADD(pcirbuff->miHead, 1, pcirbuff->miSize );
		goto UTP_REDCODE;
	}
	return 0 ;
}



void Utp_CheckRxFrame(Utp* pUtp)
{
	uint16_t frameLen = 0;
	if( !pUtp ) return ;
	const UtpFrameCfg* frameCfg = pUtp->frameCfg;
	//
	//UTP_DEBUG_MSG("[%d-%d]\r\n",pUtp->rxBuffCirBuff.miHead,pUtp->rxBuffCirBuff.miTail);
	while( 0 < ( frameLen = _DCodeCirBuff( frameCfg , &pUtp->rxBuffCirBuff ,frameCfg->transcodeBuf , frameCfg->transcodeBufLen )) )
	{
        frameLen = _ParapBuff( frameCfg->transcodeBuf ,frameCfg->transcodeBuf, frameLen);
		if(frameLen)
		{
			if( frameLen == ( frameCfg->transcodeBuf[7] + 9 ) &&
				frameCfg->FrameVerify(pUtp, &frameCfg->transcodeBuf[1], frameLen-2, Null) )
			{
				//帧处理,去掉帧头和帧尾
				Utp_RcvFrameHandler(pUtp, &frameCfg->transcodeBuf[1], frameLen-2);
			}
		}
	}
}


#endif //

void Utp_Run(Utp* pUtp)
{	
	Utp_CheckRxFrame(pUtp);
	if(UTP_FSM_WAIT_RSP == pUtp->state)	//判断等待响应是否超时
	{
		if(SwTimer_isTimerOut(&pUtp->waitRspTimer))
		{
			if(pUtp->reTxCount >= pUtp->maxTxCount && pUtp->maxTxCount != UTP_ENDLESS)
			{
				Utp_RspProc(pUtp, Null, 0, RSP_TIMEOUT);
			}
			else
			{
				Utp_SendFrame(pUtp, pUtp->pWaitRspCmd->cmd, pUtp->frameCfg->txBuf, pUtp->txBufLen);
				
				pUtp->reTxCount++;
				SwTimer_ReStart(&pUtp->waitRspTimer);
			}
		}
	}
	else
	{
		Utp_CheckReq(pUtp);
	}
}

void Utp_Init(Utp* pUtp, const UtpCfg* cfg, const UtpFrameCfg* frameCfg)
{
	memset(pUtp, 0, sizeof(Utp));

	pUtp->frameCfg = frameCfg;
	pUtp->cfg = cfg;

	//Queue_init(&pUtp->rxBufQueue, pUtp->frameCfg->rxBuf, 1, pUtp->frameCfg->rxBufLen);
	CirBuffInit(&pUtp->rxBuffCirBuff, pUtp->frameCfg->rxBuf, pUtp->frameCfg->rxBufLen);
	SwTimer_Init(&pUtp->waitRspTimer, 0, 0);
}

#endif //#ifdef CANBUS_MODE_JT808_ENABLE	


