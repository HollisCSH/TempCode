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
#include "Comm.h"
//#include "MBServer.h"
//#include "BSPTask.h"
//#include "BSPTaskDef.h"
//#include "BSPCan.h"
//#include "BSPTimer.h"
//#include "BSPWatchDog.h"
//#include "MBServer.h"
//#include "CommCtrl.h"
#include "pt.h"
#include "BSP_CAN.h"
#include "BSP_IWDG.h"

//=============================================================================================
//全局变量
//=============================================================================================
/* CAN处理数据结构体 */
t_CAN_MSG gCanDealMsg = 
{
    .DealSta = eCAN_StartRec,

    .RecdataBuff = {0},
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
	.rxBufLen =  sizeof(gCanDealMsg.RecdataBuff),
	.rxBuf	  = gCanDealMsg.RecdataBuff,
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

//CRC 循环冗余表
const u8 auchCRCHi_exp[] = {
    0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81,
	0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0,
	0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01,
	0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41,
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81,
	0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0,
	0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01,
	0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40,
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81,
	0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0,
	0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01,
	0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41,
	0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81,
	0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0,
	0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01,
	0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41,
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81,
	0x40
};

const  u8 auchCRCLo_exp[] = {
	0x00, 0xc0, 0xc1, 0x01, 0xc3, 0x03, 0x02, 0xc2, 0xc6, 0x06, 0x07, 0xc7, 0x05, 0xc5, 0xc4,
	0x04, 0xcc, 0x0c, 0x0d, 0xcd, 0x0f, 0xcf, 0xce, 0x0e, 0x0a, 0xca, 0xcb, 0x0b, 0xc9, 0x09,
	0x08, 0xc8, 0xd8, 0x18, 0x19, 0xd9, 0x1b, 0xdb, 0xda, 0x1a, 0x1e, 0xde, 0xdf, 0x1f, 0xdd,
	0x1d, 0x1c, 0xdc, 0x14, 0xd4, 0xd5, 0x15, 0xd7, 0x17, 0x16, 0xd6, 0xd2, 0x12, 0x13, 0xd3,
	0x11, 0xd1, 0xd0, 0x10, 0xf0, 0x30, 0x31, 0xf1, 0x33, 0xf3, 0xf2, 0x32, 0x36, 0xf6, 0xf7,
	0x37, 0xf5, 0x35, 0x34, 0xf4, 0x3c, 0xfc, 0xfd, 0x3d, 0xff, 0x3f, 0x3e, 0xfe, 0xfa, 0x3a,
	0x3b, 0xfb, 0x39, 0xf9, 0xf8, 0x38, 0x28, 0xe8, 0xe9, 0x29, 0xeb, 0x2b, 0x2a, 0xea, 0xee,
	0x2e, 0x2f, 0xef, 0x2d, 0xed, 0xec, 0x2c, 0xe4, 0x24, 0x25, 0xe5, 0x27, 0xe7, 0xe6, 0x26,
	0x22, 0xe2, 0xe3, 0x23, 0xe1, 0x21, 0x20, 0xe0, 0xa0, 0x60, 0x61, 0xa1, 0x63, 0xa3, 0xa2,
	0x62, 0x66, 0xa6, 0xa7, 0x67, 0xa5, 0x65, 0x64, 0xa4, 0x6c, 0xac, 0xad, 0x6d, 0xaf, 0x6f,
	0x6e, 0xae, 0xaa, 0x6a, 0x6b, 0xab, 0x69, 0xa9, 0xa8, 0x68, 0x78, 0xb8, 0xb9, 0x79, 0xbb,
	0x7b, 0x7a, 0xba, 0xbe, 0x7e, 0x7f, 0xbf, 0x7d, 0xbd, 0xbc, 0x7c, 0xb4, 0x74, 0x75, 0xb5,
	0x77, 0xb7, 0xb6, 0x76, 0x72, 0xb2, 0xb3, 0x73, 0xb1, 0x71, 0x70, 0xb0, 0x50, 0x90, 0x91,
	0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9c, 0x5c,
	0x5d, 0x9d, 0x5f, 0x9f, 0x9e, 0x5e, 0x5a, 0x9a, 0x9b, 0x5b, 0x99, 0x59, 0x58, 0x98, 0x88,
	0x48, 0x49, 0x89, 0x4b, 0x8b, 0x8a, 0x4a, 0x4e, 0x8e, 0x8f, 0x4f, 0x8d, 0x4d, 0x4c, 0x8c,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
	0x40
};


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
    memset(gCanDealMsg.RecdataBuff, 0, CAN_BUFF_LEN);
    SafeBuf_Init(&gCanDealMsg.gCanRecSafeBuf,gCanDealMsg.RecdataBuff,CAN_BUFF_LEN);
    
	return result;
}

//===========================================================================
//函数名称	: u16 MbRtuCheckCode(u8 *chkbuf, u8 len)
//输入参数	: chkbuf: 指向需要验证的ModBus消息帧的指针
//			: len   : 要效验的Modbus消息帧字节数，不包含效验码CRC
//输出参数	: UINT16: 校验码CRC	
//静态变量	: 
//功	能	: 计算并返回ModBus RTU模式下消息的CRC效验码
//注	意	:  
//===========================================================================
u16 MbRtuCheckCode(u8 *chkbuf, u8 len)
{
	u8   uchCRCHi = 0xff;   //CRC高字节初始化
	u8   uchCRCLo = 0xff;   //CRC低字节初始化
	u16  uIndex;            //查询表索引
	//u16  temp_code;

	while (len)
	{
		uIndex = (u16)(uchCRCHi ^ *chkbuf++);  
		uchCRCHi = (u8)(uchCRCLo ^ auchCRCHi_exp[uIndex]);
		uchCRCLo = auchCRCLo_exp[uIndex];
		len--;
	}
	
	//temp_code = (u16)uchCRCHi;
	//temp_code = (u16)(temp_code << 8);
	
	//return(u16)(temp_code | uchCRCLo);
    return (u16)uchCRCHi | (u16)uchCRCLo << 8;
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
//函数名称	: uint16_t CheckSum_Get(uint16_t* pCheckSum, const void* pData, uint8_t len)
//输入参数	: pCheckSum：crc存放指针；pData：计算的数组指针，len：计算的长度
//输出参数	: CRC:计算的CRC SUM
//函数功能	: 计算CRC SUM
//注意事项	:
//=============================================================================================
uint16_t CheckSum_Get(uint16_t* pCheckSum, const void* pData, uint8_t len)
{
	const uint8_t* pByte = (uint8_t*)pData;
	uint16_t i = 0;

	for(i=0; i<len; i++)
	{
		*pCheckSum += pByte[i];
	}
	
	return *pCheckSum;
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
//函数名称	: int CanCommTxData(const uint8_t* pData, uint32_t len)
//输入参数	: pData：发送数据指针；len：发送数据长度
//输出参数	: 发送长度
//静态变量	: void
//功    能	: CAN通信发送数据到总线
//注    意	:
//=============================================================================================
//int CanCommTxData(const uint8_t* pData, uint32_t len)
//{
//	uint32_t send_len = 0;
//	uint32_t tx_timeout = 0;

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
//		SendCANData(TX_MAILBOX, TX_MSG_ID2, pData, send_len);
//		tx_timeout = GET_TICKS();
//		while( FLEXCAN_DRV_GetTransferStatus(0, 1) == STATUS_BUSY &&
//			GET_TICKS() - tx_timeout < CAN_TX_TIMEOUT_MS);

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
//函数名称	: u8 CanCommFrameRx(uint8_t * buf, uint16_t * cnt, uint32_t timeout)
//输入参数	: buf：要接收放置的地址；cnt：接收的长度指针；timeout：超时时间
//输出参数	: 是否接收成功，FALSE:失败；TRUE：成功
//静态变量	: void
//功    能	: can接收帧
//注    意	:
//=============================================================================================
u8 CanCommFrameRx(uint8_t * buf, uint16_t * cnt, uint32_t timeout)
{
    u32 TimeoutTimer = GET_TICKS(); //设置超时
    u32 SendWaitTimer = 0;    
    static u8 MB_EXP = 0;
    u8 Fcode = 0;
    
    #define XMODEM_NCG 67
    u8 DataNcg = XMODEM_NCG;
    
    LOOP:    
    SendWaitTimer = GET_TICKS();
    while(TIMEPASS(TimeoutTimer) < timeout)
    {
        extern u8 IsXmodem;
        if((True == IsXmodem) && (GET_TICKS() - SendWaitTimer > 2000))
        {
            SendWaitTimer = GET_TICKS();
            CanCommFrameTx(BOOT_FUNCODE, &DataNcg,1,TIMEOUTMS(10));//发送'C'
        }
        
//        BSPFeedWDogTask();
        FeedIDog();
//				CanCommFrameTx(BOOT_FUNCODE, &DataNcg,1,TIMEOUTMS(10));//发送'C'	//测试使用
//				HAL_Delay(500);
        if(CanCommMainTask())   //接收报文成功
        {
            goto RX_CMP;
        }
    }
    return FALSE;
    
RX_CMP:    
    //功能码
    if(BOOT_FUNCODE != gCanDealMsg.TranBuff[cCanFrameCfg.dataByteInd])
    {
        MB_EXP = 0x01;
        Fcode = gCanDealMsg.TranBuff[cCanFrameCfg.dataByteInd]|0x80;
        CanCommFrameTx(Fcode,&MB_EXP,1,TIMEOUTMS(10));
        goto LOOP;
    }   
    
    //7e crcsum procotol len 01 67 data[] rtucrc1 rtucrc2 7e
    //去掉头尾，长度 -7 ，包括头尾，长度 -9    
    if((gCanDealMsg.TranLen - 7) > *cnt)//数据最大长度错误
    {
        MB_EXP = 0x02;
        Fcode = gCanDealMsg.TranBuff[cCanFrameCfg.dataByteInd]|0x80;
        CanCommFrameTx(Fcode,&MB_EXP,1,TIMEOUTMS(10));
        goto LOOP;
    }    
    
    //7e crcsum procotol len 01 67 data[] rtucrc1 rtucrc2 7e
    //去掉头尾，长度 -7 ，包括头尾，长度 -9
    *cnt = gCanDealMsg.TranLen - 7;
    //StringCopy(buf,gCanDealMsg.TranBuff + 2 + (cCanFrameCfg.dataByteInd - 1),*cnt);
    StringCopy(buf,gCanDealMsg.TranBuff + 5,*cnt);
    
    return TRUE;
}

//=============================================================================================
//函数名称	: u8 CanCommFrameTx(uint8_t fcode, uint8_t const * buf, uint16_t cnt, uint32_t timeout)
//输入参数	: fcode：功能码；buf：要发送的数据；timeout：超时时间
//输出参数	: 是否发送成功，FALSE:失败；TRUE：成功
//静态变量	: void
//功    能	: can发送帧
//注    意	:
//=============================================================================================
u8 CanCommFrameTx(uint8_t fcode, uint8_t const * buf, uint16_t cnt, uint32_t timeout)
{
    u8 ind = 0;
    u16 crc = 0;
    
    (void)timeout;

    gCanDealMsg.TempBuff[ind++] = MODBUS_MASTER_ADDR;
    gCanDealMsg.TempBuff[ind++] = fcode;   

    StringCopy(gCanDealMsg.TempBuff + ind,buf,cnt);
    cnt = cnt + ind;
    
    crc = MbRtuCheckCode(gCanDealMsg.TempBuff,cnt);
    
    gCanDealMsg.TempBuff[cnt++] = (u8)(crc >> 8);
    gCanDealMsg.TempBuff[cnt++] = (u8)(crc);
    gCanDealMsg.TempLen = cnt;
    
    /* 打包 */
    gCanDealMsg.TranLen = CanUtpBuildFrame(gCanDealMsg.TempBuff,gCanDealMsg.TempLen,(CanUtp*)gCanDealMsg.TranBuff,&cCanFrameCfg);
    int SrcInd = 0;
    gCanDealMsg.TxLen = sizeof(gCanDealMsg.TxDataBuff);
    CanCommFramePkt(gCanDealMsg.TranBuff,gCanDealMsg.TranLen,&SrcInd,gCanDealMsg.TxDataBuff,&gCanDealMsg.TxLen,&cCanFrameCfg);
    
    //return CanCommTxData(gCanDealMsg.TxDataBuff,gCanDealMsg.TxLen);
		return FDCAN2_SendMsg(gCanDealMsg.TxDataBuff,gCanDealMsg.TxLen);
}

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
//函数名称	: u8 CanCommMainTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: CAN通信协议主任务函数
//注    意	:
//=============================================================================================
u8 CanCommMainTask(void)
{
	u8 byte;
    u8 res = 0;

    switch(gCanDealMsg.DealSta)
    {
        /* 开始接收 */
        case eCAN_StartRec:
            if(SafeBuf_Read(&gCanDealMsg.gCanRecSafeBuf, &byte, 1))
            {
                //首个字节
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
            
            res = 0;
            //2ms周期运行
            //BSPTaskStart(TASK_ID_CAN_COMM_TASK, 2);
            
            break;
          
        /* 正在接收 */
        case eCAN_Recing:
            //将数据全部读上来
            while(SafeBuf_Read(&gCanDealMsg.gCanRecSafeBuf, &byte, 1))
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
                            //break;
                        }
                    }
                }
            }
            //else
            //{
                //解析转码数据、校验数据，并开始提取数据域
                if(PROTOCOL_END_BYTE == gCanDealMsg.DealBuff[gCanDealMsg.RecLen-1])
                {
                    if(True == CanUtpDecodeRecData(gCanDealMsg.DealBuff,gCanDealMsg.RecLen,gCanDealMsg.TranBuff,&gCanDealMsg.TranLen,&cCanFrameCfg))
                    {
                        gCanDealMsg.DealSta = eCAN_MsgDeal; //进行数据处理
                        gCanDealMsg.DealLen = gCanDealMsg.RecLen;
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
            //}
            res = 0;
            //BSPTaskStart(TASK_ID_CAN_COMM_TASK, 1);
            
            break;
        
        /* 接收处理 */
        case eCAN_MsgDeal:
            
//            gCanDealMsg.TxLen = MBServerMsgProcessEx((t_Modbus_PDU *)&gCanDealMsg.DealBuff[0],\
//                                                  (t_Modbus_PDU *)&gCanDealMsg.TxDataBuff[0]);
//            if(gCanDealMsg.TxLen != 0)
//            {
//                CanCommCalcCRC((u8*)gCanDealMsg.TxDataBuff, gCanDealMsg.TxLen, (u8*)&gCanDealMsg.TxDataBuff+gCanDealMsg.TxLen, (u8*)&gCanDealMsg.TxDataBuff+gCanDealMsg.TxLen+1);
//                gCanDealMsg.TxLen += 2;
//                /* CAN数据发送 */
//                CanCommTxData(gCanDealMsg.TxDataBuff,gCanDealMsg.TxLen);
//                
//                gCanDealMsg.TimeOut = 0;
//            }
            
            gCanDealMsg.DealSta = eCAN_StartRec;
            res = 1;
            //BSPTaskStart(TASK_ID_CAN_COMM_TASK, 2);
            break;
        
        /* 清除BUFF */
        case eCAN_ClrBuff:
            while(SafeBuf_Read(&gCanDealMsg.gCanRecSafeBuf, &byte, 1))
            {
                ;
            }
            
            SafeBuf_Reset(&gCanDealMsg.gCanRecSafeBuf);
            gCanDealMsg.DealSta = eCAN_StartRec;
            gCanDealMsg.RecLen = 0;
            res = 0;
            //BSPTaskStart(TASK_ID_CAN_COMM_TASK, 2);
        
        break;   

        /* 默认 */
        default:
            gCanDealMsg.DealSta = eCAN_StartRec;
            res = 0;
            //BSPTaskStart(TASK_ID_CAN_COMM_TASK, 2);
            break;
            
    }
    
    return res;
}

/*****************************************end of CanComm.c*****************************************/
