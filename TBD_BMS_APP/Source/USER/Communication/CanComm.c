//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: CanComm.c
//创建人  	: Handry
//创建日期	: 
//描述	    : CAN通信代码源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "CanComm.h"
#include "MBServer.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
//#include "BSPCan.h"
#include "BSP_CAN.h"
#include "MBServer.h"
#include "CommCtrl.h"
#include "pt.h"
#include "CheckSum.h"
#include "version.h"
#include "stdio.h"
//=============================================================================================
//全局变量
//=============================================================================================
/* CAN处理数据结构体 */
t_CAN_MSG gCanDealMsg = 
{
    .DealSta = eCAN_StartRec,

    .RecdataBuff_SolidId = {0},
    .RecdataBuff_ChangeId = {0},    
    .TxDataBuff = {0},
};

//帧解析结构体
const CanUtpFrameCfg cCanFrameCfg =
{
	//帧特征配置
	.head = 0x7E,
	.tail = 0x7E,
	.transcode = 0x7D,
	.transcodeHead = 0x02,
	.transcodeTail = 0x02,
	.transcodeShift = 0x01,
	.dataByteInd = 4,

	//分配协议缓冲区
	.txBufLen = sizeof(gCanDealMsg.TxLen),
	.txBuf	  = gCanDealMsg.TxDataBuff,
	.rxBufLen =  sizeof(gCanDealMsg.RecdataBuff_SolidId),
	.rxBuf	  = gCanDealMsg.RecdataBuff_SolidId,
	.transcodeBufLen = sizeof(gCanDealMsg.TranBuff),
	.transcodeBuf = gCanDealMsg.TranBuff,
	.txRspBuf = gCanDealMsg.DealBuff,
	.txRspBufLen = sizeof(gCanDealMsg.DealBuff),

	//返回码定义
	.result_SUCCESS = 0x00,
	.result_UNSUPPORTED = 0x02,

	//配置传输参数
	.waitRspMsDefault = 500,		// 1000
	.rxIntervalMs = 1000,
	.sendCmdIntervalMs = 10,		// 10ms

	//初始化函数指针
//	.FrameVerify = (UtpFrameVerifyFn)Utp_VerifyFrame,
//	.FrameBuild  = (UtpBuildFrameFn)Utp_BuildFrame,
};

//申请动态修改CAN ID 
t_Can_Req gStructCanReq;
//CAN通信设备状态
t_Can_Flag gStructCanFlag;

//=============================================================================================
//静态函数声明
//=============================================================================================

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: u8 CanCommInit(void)
//输入参数	: void
//输出参数	: 初始化是否成功，TURE:成功  FALSE:失败
//静态变量	: void
//功    能	: CAN通信协议初始化函数
//注    意	:
//=============================================================================================
u8 CanCommInit(void)
{
	u8 result = TRUE;
    
    gCanDealMsg.DealSta = eCAN_StartRec;
    
    //BUFF初始化
    memset(gCanDealMsg.TxDataBuff, 0, CAN_BUFF_LEN);
    memset(gCanDealMsg.RecdataBuff_SolidId, 0, CAN_BUFF_LEN);
    //SafeBuf_Init(&gCanDealMsg.gCanRecSafeBuf_SolidId,gCanDealMsg.RecdataBuff_SolidId,CAN_BUFF_LEN);
    kfifo_init(&gCanDealMsg.gCanRecSafeBuf_SolidId,gCanDealMsg.RecdataBuff_SolidId,CAN_BUFF_LEN);
    memset(gCanDealMsg.RecdataBuff_ChangeId, 0, CAN_BUFF_LEN);
    //SafeBuf_Init(&gCanDealMsg.gCanRecSafeBuf_ChangeId,gCanDealMsg.RecdataBuff_ChangeId,CAN_BUFF_LEN);
    kfifo_init(&gCanDealMsg.gCanRecSafeBuf_ChangeId,gCanDealMsg.RecdataBuff_ChangeId,CAN_BUFF_LEN);    
    //申请CANID
    gStructCanReq.SendFlag.SendFlagBit.IsSendReq = True;
    gStructCanReq.RamdonNum = 0;
    
//    gStructCanReq.TxCanId = VARI_TX_ID;
//    gStructCanReq.RxCanId = VARI_RX_ID;
    
    //设备CAN通信标志清除
    gStructCanFlag.DeviceFlag.DeviceFlagByte = 0;
    
	return result;
}

//=============================================================================================
//函数名称	: u16 CANCommCalcCRC(int CRCType,u8 *Data, u16 Length,u8 *TransmitFirst, u8 *TransmitSecond)
//输入参数	: Data：数组指针；Length：计算的长度；TransmitFirst：CRC低字节；TransmitSecond：CRC高字节
//输出参数	: CRC:计算的CRC
//函数功能	: CAN通信计算CRC
//注意事项	:
//=============================================================================================
u16 CanCommCalcCRC(u8 *Data, u16 Length,u8 *TransmitFirst, u8 *TransmitSecond)
{
	u16 wCrc;

   wCrc = MbRtuCheckCode(Data,Length);

	if(0 != TransmitFirst)
	{
		*TransmitFirst = (u8) ((wCrc >> 8) & 0xFF);
	}

	if(0 != TransmitSecond)
	{
		*TransmitSecond = (u8) (wCrc & 0xFF);
	}    
    
	return wCrc;
}

//=============================================================================================
//函数名称	: u8 CanCommCheckCRCA(u8 *msg, u8 cnt)
//输入参数	: msg：比较的数据指针；cnt：比较的数据的长度
//输出参数	: TRUE：CRC正确；FALSE：CRC错误
//函数功能	: CAN通信检测CRc是否正确
//注意事项	:
//=============================================================================================
u8 CanCommCheckCRC(u8 *msg, u8 cnt)
{
    u8 crc1,crc2;

    CanCommCalcCRC(msg, cnt, &crc1, &crc2);
    if((crc1 != msg[cnt]) || (crc2 != msg[cnt+1]))
    {
        return FALSE;
    }
    return TRUE;
}

//=============================================================================================
//函数名称	: u8 CommCompareTimeout(u16 timeout)
//输入参数	: timeout:超时时间
//输出参数	: void
//静态变量	: void
//功    能	: CAN通信协议检测超时时间
//注    意	:
//=============================================================================================
u8 CanCommCompareTimeout(u16 timeout)
{
    return True == gCanDealMsg.CommFlag.CommFlagBit.IsCommTimeOut;
}

//=============================================================================================
//函数名称	: void CanCommCheckTimeout(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: CAN通信协议断线超时任务
//注    意	:
//=============================================================================================
void CanCommCheckOffTimeout(void)
{
    if(gCanDealMsg.TimeOut < 60000)
    {
        gCanDealMsg.TimeOut++;
    }

    if(gCanDealMsg.TimeOut > 1600)
    {
    	gCanDealMsg.CommFlag.CommFlagBit.IsCommTimeOut = True;	//无通信16s，判断为断线
    }
	else
	{
		gCanDealMsg.CommFlag.CommFlagBit.IsCommTimeOut = False;	//无通信16s，判断为断线
	}
}

//=============================================================================================
//函数名称	: int CanCommTxData(u32 canid,const uint8_t* pData, uint32_t len)
//输入参数	: canid:canid；pData：发送数据指针；len：发送数据长度
//输出参数	: 发送长度
//静态变量	: void
//功    能	: CAN通信发送数据到总线
//注    意	:
//=============================================================================================
//int CanCommTxData(u32 canid,const uint8_t* pData, uint32_t len)
//{
//	uint32_t send_len = 0;
//	uint32_t tx_timeout = 0;
//    static u8 TxFaultCnt = 0;

//	while(len)
//	{
//		if(len > 8)
//		{
//			send_len = 8;
//		}
//		else
//		{
//			send_len = len;
//		}
//		SendCANData(TX_MAILBOX, canid, pData, send_len);
//		tx_timeout = GET_TICKS();
//		while( FLEXCAN_DRV_GetTransferStatus(0, 1) == STATUS_BUSY &&
//			GET_TICKS() - tx_timeout < CAN_TX_TIMEOUT_MS);
//        
//        if(GET_TICKS() - tx_timeout >= CAN_TX_TIMEOUT_MS)
//            TxFaultCnt++;
//        
//        if(TxFaultCnt > 10)
//        {
//            BSPCan_Reset();
//            TxFaultCnt = 0;
//        }
//        
//		if(len > 8)
//		{
//			pData = pData + 8;
//			len = len - 8;
//		}
//		else
//		{
//			break;
//		}
//	}
//	
//	return len;
//}


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
int CanUtpDecodeRecData(const u8* pRecData, int Reclen, u8 *pDealData, int* pDealLen,const CanUtpFrameCfg *FrameCfg)
{
    int nRet = True;
    int i = 0;
    u8 j = 0;
    
    
    if(FrameCfg->head != pRecData[0] && FrameCfg->tail != pRecData[Reclen - 1])
        return (nRet = False);
    
    //将数据包解析成正常数据，遇到转码则进行转码
    for(i = 1;i < Reclen - 1;i++)   //从中间开始解析
    {
        
        if(pRecData[i] == FrameCfg->transcode)
        {
            //要转换为0x7e
            if(pRecData[i+1] == FrameCfg->transcodeHead)
            {
                i++;
                pDealData[j++] = FrameCfg->head;
            }
            //要转换为0x7d
            else if(pRecData[i+1] == FrameCfg->transcodeShift)
            {
                i++;
                pDealData[j++] = FrameCfg->transcode;
            }
        }
        else
        {
            pDealData[j++] = pRecData[i];
        }
    }
    
    *pDealLen = j;
    
    //校验CRC
    uint16_t dstCheckSum = 0;
    dstCheckSum = (uint8_t)CheckSum_Get(&dstCheckSum, &(pDealData[1]), j - 1);
    if((u8)dstCheckSum != pDealData[0])
        return (nRet = False);
    
    //校验数据域LEN
    if(pDealData[FrameCfg->dataByteInd-2] != (j - 3))
        return (nRet = False);
    
    //Modbus RTU校验
    if(FALSE == CanCommCheckCRC((u8*)(&pDealData[FrameCfg->dataByteInd-1]),pDealData[FrameCfg->dataByteInd-2]-2))
        return (nRet = False);
        
    return nRet;
}

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
int CanUtpBuildFrame(const void* pData, int len, CanUtp* frame,const CanUtpFrameCfg *FrameCfg)
{
	uint16_t checkSum = 0;

    frame->protocolver = FW_PROTOCOL_VER;
    frame->len = len;
	
	memcpy(frame->data, pData, len);

	frame->checkSum = (uint8_t)CheckSum_Get(&checkSum, &frame->protocolver, len + 2);

	return len + FrameCfg->dataByteInd - 1;
}

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
Bool CanCommFramePkt(const uint8_t* pSrc, uint16_t srcLen, int* srcInd, uint8_t* pDst, int* dstSize,const CanUtpFrameCfg* frameCfg)
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

//=============================================================================================
//函数名称	: void CanCommSendRequsetTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: CAN通信协议申请id函数
//注    意	:
//=============================================================================================
void CanCommSendRequsetTask(void)
{
    u8 uCanReqLen = 2;
    u8 uCanReqTempBuff[COMM_CAN_REQUEST_LENGTH] = {MODBUS_MASTER_ADDR,MODBUS_REQUEST_MOD_CANID};
//    static u8 sflag = True;
    static u32 sResetTick = 0;
    static u8 sRandomNum = 10;
    static u8 sRandomCnt = 0;    
    
    if(sRandomCnt++ > sRandomNum)
    {
        sRandomCnt = 0;
        sRandomNum = GET_TICKS() & 0x00000007 + 3;  //3 - 10 随机数
        if(True == gStructCanReq.SendFlag.SendFlagBit.IsSendReq)
        {
    //        if(True == sflag)
    //        {
    //            sflag = False;
                gStructCanReq.RxCanId = FlexCanChangeRx3Canid(0xFF);
                //gStructCanReq.TxCanId = TX_MSG_ID3;
                gStructCanReq.TxCanId = EX_MODU_TX_ID;
    //        }
        }
    //    else
    //    {
    //        sflag = True;
    //    }

        if(True == gStructCanReq.SendFlag.SendFlagBit.IsSendReq && BITGET(gBatteryInfo.Status.IOStatus,3)
            && False == gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule)
    {
        /* 放入MCU ID */
        //memcpy(&uCanReqTempBuff[uCanReqLen],(u8 *)&(SIM->UIDH),16
        memcpy(&uCanReqTempBuff[uCanReqLen],(u8 *)(UID_BASE+8U),16);
         
        uCanReqLen += 16;

            /*放入随机数 */
            memcpy(&uCanReqTempBuff[uCanReqLen],(u8 *)&(gStructCanReq.RamdonNum),sizeof(gStructCanReq.RamdonNum));
            uCanReqLen += 4; 
            gStructCanReq.RamdonNum++;
            
            /* 计算CRC */
            CanCommCalcCRC((u8*)uCanReqTempBuff, uCanReqLen, (u8*)&uCanReqTempBuff+uCanReqLen, (u8*)&uCanReqTempBuff+uCanReqLen+1);
            uCanReqLen += 2;
            
            /* CAN数据发送 开始*/
            /* 打包 */
            uCanReqLen = CanUtpBuildFrame(uCanReqTempBuff,uCanReqLen,(CanUtp*)gStructCanReq.CanReqTab,&cCanFrameCfg);
            int SrcInd = 0;
            gCanDealMsg.TxLen = sizeof(gCanDealMsg.TxDataBuff);
            CanCommFramePkt(gStructCanReq.CanReqTab,uCanReqLen,&SrcInd,gCanDealMsg.TxDataBuff,&gCanDealMsg.TxLen,&cCanFrameCfg);
            /* CAN发送*/
//        CanCommTxData(VARI_TX_ID,gCanDealMsg.TxDataBuff,gCanDealMsg.TxLen);
				SendCANData(EX_MODU_TX_ID,gCanDealMsg.TxDataBuff,gCanDealMsg.TxLen);
        /* CAN数据发送 结束*/      

    //回复测试
    //        /* 放入MCU ID */
    //        memcpy(&uCanReqTempBuff[uCanReqLen],(u8 *)&(SIM->UIDH),16);
    //        uCanReqLen += 16;

    //        /*放入Ack */
    //        uCanReqTempBuff[uCanReqLen] = 0x00;
    //        uCanReqLen += 1;
    //        /*放入地址 */
    //        uCanReqTempBuff[uCanReqLen] = 0x13;
    //        uCanReqLen += 1;        
    //        
    //        /* 计算CRC */
    //        CanCommCalcCRC((u8*)uCanReqTempBuff, uCanReqLen, (u8*)&uCanReqTempBuff+uCanReqLen, (u8*)&uCanReqTempBuff+uCanReqLen+1);
    //        uCanReqLen += 2;
    //        
    //        /* CAN数据发送 开始*/
    //        /* 打包 */
    //        uCanReqLen = CanUtpBuildFrame(uCanReqTempBuff,uCanReqLen,(CanUtp*)gStructCanReq.CanReqTab,&cCanFrameCfg);
    //        int SrcInd = 0;
    //        gCanDealMsg.TxLen = sizeof(gCanDealMsg.TxDataBuff);
    //        CanCommFramePkt(gStructCanReq.CanReqTab,uCanReqLen,&SrcInd,gCanDealMsg.TxDataBuff,&gCanDealMsg.TxLen,&cCanFrameCfg);
    //        /* CAN发送*/
    //        CanCommTxData(TX_MSG_ID3,gCanDealMsg.TxDataBuff,gCanDealMsg.TxLen);
    //        /* CAN数据发送 结束*/  
        }
        
        //外置中控接入 且 已分配好id地址
        if(True == gStructCanFlag.DeviceFlag.DeviceFlagBit.IsControlBoard && False == gStructCanReq.SendFlag.SendFlagBit.IsSendReq)
        {
            BSPTaskStop(TASK_ID_JT808_TASK);
            gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule = False;
        }
        else
        {
            if(!BSPTaskGetIsTaskStop(TASK_ID_JT808_TASK))   //任务被停止则重新启动任务
                BSPTaskStart(TASK_ID_JT808_TASK,10);
        }
        
        //重启机制
        extern u8 can0_receive_flag;
        if(can0_receive_flag == 0x01)
        {
            can0_receive_flag = 0;
            sResetTick = GET_TICKS();
        }
        else
        {
            //外置模块通信复位
            if(True == gStructCanFlag.DeviceFlag.DeviceFlagBit.IsCanRst
                && False == gStructCanFlag.DeviceFlag.DeviceFlagBit.IsCabinet
                && False == gStructCanFlag.DeviceFlag.DeviceFlagBit.IsControlBoard)
            {
                gStructCanFlag.DeviceFlag.DeviceFlagBit.IsCanRst = False;
                sResetTick = GET_TICKS();
                //FlexCanReset();
                BSPCan_Reset();              
            }
            if((GET_TICKS() - sResetTick) > DATA_REV_TIMEOUT_MS)
            {
                sResetTick = GET_TICKS();
                //FlexCanReset();
                BSPCan_Reset();
                //有ID信号，但是无任何通信数据，重新请求
                //无外置模块通信才发请求，避免唤醒
                if(BITGET(gBatteryInfo.Status.IOStatus,3)
                    && False == gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule)
                {
                    /* 放入MCU ID */
                    //memcpy(&uCanReqTempBuff[uCanReqLen],(u8 *)&(SIM->UIDH),16);
                    memcpy(&uCanReqTempBuff[uCanReqLen],(u8 *)(UID_BASE+8U),16);
                    uCanReqLen += 16;

                    /*放入随机数 */
                    memcpy(&uCanReqTempBuff[uCanReqLen],(u8 *)&(gStructCanReq.RamdonNum),sizeof(gStructCanReq.RamdonNum));
                    uCanReqLen += 4; 
                    gStructCanReq.RamdonNum++;
                    
                    /* 计算CRC */
                    CanCommCalcCRC((u8*)uCanReqTempBuff, uCanReqLen, (u8*)&uCanReqTempBuff+uCanReqLen, (u8*)&uCanReqTempBuff+uCanReqLen+1);
                    uCanReqLen += 2;
                    
                    /* CAN数据发送 开始*/
                    /* 打包 */
                    uCanReqLen = CanUtpBuildFrame(uCanReqTempBuff,uCanReqLen,(CanUtp*)gStructCanReq.CanReqTab,&cCanFrameCfg);
                    int SrcInd = 0;
                    gCanDealMsg.TxLen = sizeof(gCanDealMsg.TxDataBuff);
                    CanCommFramePkt(gStructCanReq.CanReqTab,uCanReqLen,&SrcInd,gCanDealMsg.TxDataBuff,&gCanDealMsg.TxLen,&cCanFrameCfg);
                    /* CAN发送*/
                    //CanCommTxData(TX_MSG_ID3,gCanDealMsg.TxDataBuff,gCanDealMsg.TxLen);
                    SendCANData(EX_MODU_TX_ID,gCanDealMsg.TxDataBuff,gCanDealMsg.TxLen);
                    /* CAN数据发送 结束*/  
                }
            }
        }
    }    
}

//=============================================================================================
//函数名称	: void CanCommMainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: CAN通信协议主任务函数
//注    意	:
//=============================================================================================
void CanCommMainTask(void *p)
{
	u8 byte;
    //static SafeBuf* pSafeBuff;
    static kfifo* sl_kfifo;
		volatile u8 fifo_flag = 0;
    switch(gCanDealMsg.DealSta)
    {
        /* 开始接收 */
        case eCAN_StartRec:
            //if(SafeBuf_Read(&gCanDealMsg.gCanRecSafeBuf_SolidId, &byte, 1
//            fifo_flag = kfifo_get_datalen(&gCanDealMsg.gCanRecSafeBuf_SolidId);
//            if(fifo_flag == 0) return;
            if(kfifo_out(&gCanDealMsg.gCanRecSafeBuf_SolidId, &byte, 1))
            {
                gStructCanReq.SendFlag.SendFlagBit.IsSolidId = True;
                //pSafeBuff = &gCanDealMsg.gCanRecSafeBuf_SolidId;
                sl_kfifo = &gCanDealMsg.gCanRecSafeBuf_SolidId;
                //首个字节
                CommClearTimeout();
                if(PROTOCOL_FIRST_BYTE == byte)
                {
                    gCanDealMsg.DealSta = eCAN_Recing;
                    gCanDealMsg.RecLen = 0;
                    gCanDealMsg.DealBuff[gCanDealMsg.RecLen++] = byte;
                    gCanDealMsg.StartTimer = GET_TICKS();
                }
                else
                {
                    gCanDealMsg.DealSta = eCAN_ClrBuff;
                }
            }
            //else if(SafeBuf_Read(&gCanDealMsg.gCanRecSafeBuf_ChangeId, &byte, 1))
            else if(kfifo_out(&gCanDealMsg.gCanRecSafeBuf_ChangeId, &byte, 1))
            {
                gStructCanReq.SendFlag.SendFlagBit.IsSolidId = False;
                //pSafeBuff = &gCanDealMsg.gCanRecSafeBuf_ChangeId;
                sl_kfifo = &gCanDealMsg.gCanRecSafeBuf_ChangeId;
                
                //首个字节
                CommClearTimeout();
                if(PROTOCOL_FIRST_BYTE == byte)
                {
                    gCanDealMsg.DealSta = eCAN_Recing;
                    gCanDealMsg.RecLen = 0;
                    gCanDealMsg.DealBuff[gCanDealMsg.RecLen++] = byte;
                    gCanDealMsg.StartTimer = GET_TICKS();
                }
                else
                {
                    gCanDealMsg.DealSta = eCAN_ClrBuff;
                }
            }            
            else
            {
                gCanDealMsg.DealSta = eCAN_StartRec;
            }
            
            //2ms周期运行
            BSPTaskStart(TASK_ID_CAN_COMM_TASK, 2);
            
            break;
          
        /* 正在接收 */
        case eCAN_Recing:
            //将数据全部读上来
            //while(SafeBuf_Read(pSafeBuff, &byte, 1))
            while(kfifo_out(sl_kfifo, &byte, 1))
            {
                //gCanDealMsg.StartTimer = GET_TICKS();
                gCanDealMsg.DealBuff[gCanDealMsg.RecLen++] = byte;
                
                if((gCanDealMsg.RecLen > MODBUS_MIN_PDU_LEN))
                    //&&(CommStringComp(cCanPDUHead, &gCanDealMsg.DealBuff[0], 4)))
                {
                    //是否收到完整一帧
                    if(PROTOCOL_END_BYTE == gCanDealMsg.DealBuff[gCanDealMsg.RecLen-1])
                    {
                        //解析转码数据、校验数据，并开始提取数据域
                        if(True == CanUtpDecodeRecData(gCanDealMsg.DealBuff,gCanDealMsg.RecLen,gCanDealMsg.TranBuff,&gCanDealMsg.TranLen,&cCanFrameCfg))
                        {
                            gCanDealMsg.DealSta = eCAN_MsgDeal; //进行数据处理
                            gCanDealMsg.DealLen = gCanDealMsg.RecLen;
                            //判断是否是外置中控通信 亦或是 机柜/PC通信
                            //if(pSafeBuff == &gCanDealMsg.gCanRecSafeBuf_ChangeId
                            if(sl_kfifo == &gCanDealMsg.gCanRecSafeBuf_ChangeId)
                                gStructCanFlag.DeviceFlag.DeviceFlagBit.IsControlBoard = True;
                            //else if(pSafeBuff == &gCanDealMsg.gCanRecSafeBuf_SolidId)
                            else if(sl_kfifo == &gCanDealMsg.gCanRecSafeBuf_SolidId)
                                gStructCanFlag.DeviceFlag.DeviceFlagBit.IsCabinet = True;
                            
                            break;
                        }
                    }
                }
            }

            //解析转码数据、校验数据，并开始提取数据域
            if(PROTOCOL_END_BYTE == gCanDealMsg.DealBuff[gCanDealMsg.RecLen-1])
            {
                if(True == CanUtpDecodeRecData(gCanDealMsg.DealBuff,gCanDealMsg.RecLen,gCanDealMsg.TranBuff,&gCanDealMsg.TranLen,&cCanFrameCfg))
                {
                    gCanDealMsg.DealSta = eCAN_MsgDeal; //进行数据处理
                }
                else
                {
                    //接收超时，丢弃数据
                    gCanDealMsg.DealSta = eCAN_ClrBuff;                
                }
            }
            else
            {
                //帧报文判断是否超时
                if((GET_TICKS() - gCanDealMsg.StartTimer) > CAN_REC_OVER_TIME)
                {
                    //接收超时，丢弃数据
                    gCanDealMsg.DealSta = eCAN_ClrBuff;
                }
                else
                {
                    ;
                }
            }
            
            BSPTaskStart(TASK_ID_CAN_COMM_TASK, 1);
            
            break;
        
        /* 接收处理 */
        case eCAN_MsgDeal:
            ///////////////////////////////////////////////////////////////
//            _UN_NB_Printf("rec can data:\n");
//            for(int sl_temp = 0 ; sl_temp < gCanDealMsg.TranLen ; sl_temp++)
//            {
//              _UN_NB_Printf(" %02X ",gCanDealMsg.TranBuff[sl_temp]);
//            }
//            _UN_NB_Printf("\n");
            ///////////////////////////////////////////////////////////////
            gCanDealMsg.DealLen = MBServerMsgProcessEx((t_Modbus_PDU *)&gCanDealMsg.TranBuff[cCanFrameCfg.dataByteInd-1],\
                                                   (t_Modbus_PDU *)&gCanDealMsg.DealBuff[0]);
  
            if(gCanDealMsg.DealLen != 0)
            {
                CanCommCalcCRC((u8*)gCanDealMsg.DealBuff, gCanDealMsg.DealLen, (u8*)&gCanDealMsg.DealBuff+gCanDealMsg.DealLen, (u8*)&gCanDealMsg.DealBuff+gCanDealMsg.DealLen+1);
                gCanDealMsg.DealLen += 2;
                /* CAN数据发送 开始*/
                /* 打包 */
                gCanDealMsg.TranLen = CanUtpBuildFrame(gCanDealMsg.DealBuff,gCanDealMsg.DealLen,(CanUtp*)gCanDealMsg.TranBuff,&cCanFrameCfg);
                int SrcInd = 0;
                gCanDealMsg.TxLen = sizeof(gCanDealMsg.TxDataBuff);
                CanCommFramePkt(gCanDealMsg.TranBuff,gCanDealMsg.TranLen,&SrcInd,gCanDealMsg.TxDataBuff,&gCanDealMsg.TxLen,&cCanFrameCfg);
                /* CAN发送*/
//                CanCommTxData(True == gStructCanReq.SendFlag.SendFlagBit.IsSolidId?TX_MSG_ID2:gStructCanReq.TxCanId,
//                gCanDealMsg.TxDataBuff,gCanDealMsg.TxLen);
								SendCANData(True == gStructCanReq.SendFlag.SendFlagBit.IsSolidId?SOLID_TX_ID:gStructCanReq.TxCanId,
                gCanDealMsg.TxDataBuff,gCanDealMsg.TxLen);
              
              
                ///////////////////////////////////////////////////////////////
//                _UN_NB_Printf("Tx can data:\n");
//                for(int sl_temp = 0 ; sl_temp < gCanDealMsg.TxLen ; sl_temp++)
//                {
//                  _UN_NB_Printf(" %02X ",gCanDealMsg.TxDataBuff[sl_temp]);
//                }
//                _UN_NB_Printf("\n");
                ///////////////////////////////////////////////////////////////
                /* CAN数据发送 结束*/
                gCanDealMsg.TimeOut = 0;
                CommClearTimeout();
            }
            
            gCanDealMsg.DealSta = eCAN_StartRec;
            BSPTaskStart(TASK_ID_CAN_COMM_TASK, 2);
            break;
        
        /* 清除BUFF */
        case eCAN_ClrBuff:
            //while(SafeBuf_Read(pSafeBuff, &byte, 1))
            while(kfifo_out(sl_kfifo, &byte, 1))
            {
                ;
            }
            
            //SafeBuf_Reset(pSafeBuff);
            kfifo_reset(sl_kfifo);
            gCanDealMsg.DealSta = eCAN_StartRec;
            gCanDealMsg.RecLen = 0;
            
            BSPTaskStart(TASK_ID_CAN_COMM_TASK, 2);
        
        break;   

        /* 默认 */
        default:
            gCanDealMsg.DealSta = eCAN_StartRec;
            BSPTaskStart(TASK_ID_CAN_COMM_TASK, 2);
            break;
            
    }
}

/*****************************************end of CanComm.c*****************************************/
