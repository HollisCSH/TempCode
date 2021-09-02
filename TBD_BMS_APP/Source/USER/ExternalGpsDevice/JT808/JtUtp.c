
#include "common.h"
#include "JtUtp.h"
#include "CheckSum.h"

/*********************************
函数功能：帧校验，实现2个功能；
	1）如果pReq=Null ,仅仅检验接收到的数据是否完整。
	2）如果pReq!=Null,仅检验接收的数据帧是否和Req匹配。
参数说明：
	pUtp：UTP对象指针。
	rxFrame：要检验的数据帧,不包含帧头和帧尾字节。
	frameLen：pFrame的长度。
	req：请求帧，如果req=Null，表示不需要检验Req
返回值：校验结果。
***********************************/
static Bool Utp_VerifyFrame(Utp* pUtp, const JtUtp* pFrame, int frameLen, const JtUtp* pReq)
{
	//如果pReq != Null, 仅检验接收的数据帧是否和Req匹配。
	if (pReq)
	{
		if (pFrame->cmd != pReq->cmd)
		{
//			Printf("Cmd[0x%02x, 0x%02x] not match:", pReq->cmd, pFrame->cmd);
			goto Error;
		}
	}
	else
	{
		//检验帧长度
		if (frameLen < pUtp->frameCfg->dataByteInd)
		{
//			Printf("frame len error.(%d)\n", frameLen);
			goto Error;
		}

		//如果pReq=Null ,仅仅检验接收到的数据是否完整。
		uint16_t dstCheckSum = 0;
		dstCheckSum = (uint8_t)CheckSum_Get(&dstCheckSum, &pFrame->vendor, frameLen - 2);
		if (pFrame->checkSum != dstCheckSum)
		{
//			Printf("Crc[0x%02x, 0x%02x] error:", pFrame->checkSum, dstCheckSum);
			goto Error;
		}
	}
	return True;

Error:
//	DUMP_BYTE(pFrame, frameLen);
	return False;
}

/*********************************
函数功能：把要发送的数据数据打包成为一个请求数据帧或者响应帧，由参数reqFrame决定。
	1）如果pReq=Null ,打包一个请求帧。
	2）如果pReq!=Null,打包一个响应帧。
参数说明：
	pUtp：UTP对象指针。
	cmd：命令码。
	pData：数据指针。
	len：数据长度
	reqFrame：如果=NULL，表示根据请求帧，打包为响应帧；如果!=NULL，表示打包一个请求帧。
	rspFrame：输出参数，要打包的数据帧指针
返回值：数据帧长度。
***********************************/
static int Utp_BuildFrame(Utp* pUtp, uint8_t cmd, const void* pData, int len, const JtUtp* pReq, JtUtp* frame)
{
	uint16_t checkSum = 0;

//	Assert(pUtp->frameCfg->txBufLen > (len + pUtp->frameCfg->dataByteInd));

	if (pReq)	//打包一个响应帧
	{
		memcpy(frame, pReq, pUtp->frameCfg->dataByteInd);
	}
	else
	{	//打包一个请求帧
		frame->ver = 0;
		frame->vendor = 0;
		frame->devType = 0;
		frame->cmd = cmd;
		frame->len = len;
	}
	memcpy(frame->data, pData, len);

	frame->checkSum = (uint8_t)CheckSum_Get(&checkSum, &frame->vendor, len + 5);

	return len + pUtp->frameCfg->dataByteInd;
}

static uint8_t g_JtUtp_txBuff[128];			//发送数据缓冲区
static uint8_t g_JtUtp_transcodeBuf[150];	//接收数据的转码缓冲区
static uint8_t g_JtUtp_rxBuff[200];			//接收数据缓冲区
static uint8_t g_JtUtp_txRspBuf[128];
const UtpFrameCfg g_jtFrameCfg =
{
	//帧特征配置
	.head = 0x7E,
	.tail = 0x7E,
	.transcode = 0x7D,
	.transcodeHead = 0x02,
	.transcodeTail = 0x02,
	.transcodeShift = 0x01,
	.cmdByteInd  = 5,
	.dataByteInd = 7,

	//分配协议缓冲区
	.txBufLen = sizeof(g_JtUtp_txBuff),
	.txBuf	  = g_JtUtp_txBuff,
	.rxBufLen = sizeof(g_JtUtp_rxBuff),
	.rxBuf	  = g_JtUtp_rxBuff,
	.transcodeBufLen = sizeof(g_JtUtp_transcodeBuf),
	.transcodeBuf = g_JtUtp_transcodeBuf,
	.txRspBuf = g_JtUtp_txRspBuf,
	.txRspBufLen = sizeof(g_JtUtp_txRspBuf),

	//返回码定义
	.result_SUCCESS = OP_SUCCESS,
	.result_UNSUPPORTED = OP_UNSUPPORTED,

	//配置传输参数
	.waitRspMsDefault = 500,		// 1000
	.rxIntervalMs = 1000,
	.sendCmdIntervalMs = 10,		// 10ms

	//初始化函数指针
	.FrameVerify = (UtpFrameVerifyFn)Utp_VerifyFrame,
	.FrameBuild  = (UtpBuildFrameFn)Utp_BuildFrame,
};
