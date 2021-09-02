
#ifndef __BLE_TPU_H_
#define __BLE_TPU_H_

#ifdef __cplusplus
extern "C"{
#endif

//#include "typedef.h"
#include "BSPTypeDef.h"
#include "utp.h"

#define BLE_MAC_SIZE 6
	//Utp帧配置特征
	typedef struct _BleTpuFrameCfg
	{
		uint8_t cmdByteInd;		//命令字节偏移
		uint8_t dataByteInd;	//数据字节偏移

		int      txBufLen;	//txBuf的长度
		uint8_t* txBuf;		//存放发送出去数据，转码前
		int      rxBufLen;	//rxBuf的长度
		uint8_t* rxBuf;		//存放接收到的RAW数据，转码前
		int		 transcodeBufLen;//transcodeBuf的长度
		uint8_t* transcodeBuf;	 //存放rxBuf转码后的帧数据

		uint8_t result_SUCCESS;		//返回码定义：成功
		uint8_t result_UNSUPPORTED;	//返回码定义：接收到不支持的请求

		uint32_t waitRspMsDefault;	//命令的默认的等待响应时间，如果命令要修改为非默认值，可以在命令的事件函数UTP_TX_START中修改pUtp->waitRspMs
		uint32_t rxIntervalMs;		//接收数据间隔
		uint32_t sendCmdIntervalMs;	//发送2个命令之间的间隔时间
	}BleTpuFrameCfg;

	typedef struct _BleTpu
	{
		UTP_FSM state;
		FRAME_STATE FrameState;

		uint16_t txBufLen;		//发送帧长度
		uint16_t transcodeBufLen;	//转码后的帧长度
		Queue rxBufQueue;		//管理rxBuf的Queue。

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
		const BleTpuFrameCfg* frameCfg;
	}BleTpu;

	void BleTpu_Init(BleTpu* pBleTpu, const UtpCfg* cfg, const BleTpuFrameCfg* frameCfg);
	void BleTpu_Run(BleTpu* pBleTpu);
	uint8* BleTpu_ReqProc(BleTpu* pBleTpu, const uint8_t* pReq, int frameLen, uint8* rspLen);

#ifdef __cplusplus
}
#endif

#endif
