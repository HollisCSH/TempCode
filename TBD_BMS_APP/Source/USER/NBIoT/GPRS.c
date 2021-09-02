//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Gprs.c
//创建人  	: 
//创建日期	:
//描述	    : Sim模组的2G/4G处理文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "Gprs.h"
#include "AtCmdTab.h"
#include "Debug.h"
#include "BmsSystem.h"
#include "MOSCtrl.h"
#include "sim.h"
#include "Fsm.h"
#include "Utp.h"
#include "sign.h"
#include "Sms.h"
#include "Gprs868.h"
#include "GprsTlv.h"
#include "ParaCfg.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "WIFIDeal.h"
//#include "PCF85063.h"
#include "RTC.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
//static Utp g_GprsUtp;
SwTimer g_GprsCmdTimer;
static SwTimer g_RcvRspTimer;
static SwTimer g_GprsWdTimer;
Gprs * g_pGprs = Null;
AtCmdItem* g_pAtCmdItem;
static uint8 g_GprsBuf[256];
static uint8 g_GprsTranBuf[256];
static uint8 g_SmsCsclk = DISABLE_SLOW_CLK;
Bool g_isPostEnable = True;
#ifdef CFG_LOG
static int    g_PostLogRsp = 0;
static int    g_PostLogInd = 0;
static uint8  g_LogReadSector = 0;
#endif

//=============================================================================================
//定义接口函数
//=============================================================================================

//GPRS基本初始化
void Gprs_InitBase()
{
	static Gprs __g_Gprs;

	g_pGprs = &__g_Gprs;
	memset(g_pGprs, 0, sizeof(Gprs));

	g_pGprs->heartBeatInterval = TIME_HEARBEAT_SHORT;

	SwTimer_Init(&g_pGprs->m_HbTimer, 0, 0);
	g_pGprs->IsSendPortChanged = Gprs_IsPortChanged;
	g_pSimAtCmdCtrl->Filter	= Gprs_Filter;
	g_pSimAtCmdCtrl->IsAllowResend = Gprs_IsAllowResend;

	/*GprsTlv_Init();*/
}

//GPRS 2G通信初始化函数
void Gprs_Init()
{
	Gprs_InitBase();
	
	g_pGprs->GetDataPkt 		= Gprs_GetDataPkt;
	g_pGprs->GetPortStatePkt 	= Bunker_GetPortStateChangedPkt;
}

//GPRS 2G复位变量和停止定时器函数
void Gprs_Reset()
{
	Gprs_Sleep();   
	SwTimer_Stop(&g_pGprs->m_HbTimer);
	g_pGprs->tcpPktlen = 0;
	g_pGprs->isConnected = False;
	g_pGprs->isSimReady = SIMCARD_UNKNOWN;
	g_pGprs->isIndentify = False;
	g_pGprs->isGatt = 0;

	g_pGprs->isAuth = 0;
	g_pGprs->isLowPower = 0;
	g_pSimCard->csq = 0;
	g_pGprs->isReg = False;

//	Gprs_SetHearbeatInterval(TIME_HEARBEAT_SHORT);
#ifdef SIM868
	g_pGprs->m_isStart = False;
#endif
	g_pGprs->isReadSms = False;
	g_pGprs->isSendGps = False;
	g_pGprs->isSendWifiMac = False;    
}

//GPRS启动
void Gprs_Start(void)
{
//	PFL(DL_SIM, "%s\n", _FUNC_);
	AtCmdCtrl_Reset(g_pSimAtCmdCtrl);
	//Sim_DelaySendCmdInd(CMD_ATE, 1000);     //发送ATE0关闭回显
    //Sim_SyncSendAtCmd("AT+QRST=1", Null, 1000);
	Gps_Start();
	Gprs_WdStart();
}


//GPRS唤醒
void Gprs_Wakeup(void)
{
	g_pGprs->m_HbCount = 0;
	g_pGprs->m_GpsCount = 0;
    g_pGprs->m_WifiCount = 0;
	SwTimer_Stop(&g_pGprs->m_HbTimer);  //启动gprs心跳定时器
	Gprs_WdStart();                     //启动gprs看门狗定时器
}

//GPRS睡眠
void Gprs_Sleep(void)
{
	g_pGprs->isConnected = False;       //清除连接成功标志
	g_pGprs->isRcvSms = False;      
	g_pGprs->isReadSms = False;
	SwTimer_Stop(&g_pGprs->m_HbTimer);	//停止心跳定时器
	SwTimer_Stop(&g_GprsCmdTimer);      //停止GPRS 命令延时定时器
	Gprs_WdStop();                      //停止看门狗定时器
}

//Gprs watch dog opertion
//启动gprs看门狗定时器，300S后启动，5Min的喂狗阈值
void Gprs_WdStart()
{
	SwTimer_Start(&g_GprsWdTimer, 300000, 0);
}

//看门狗停止
void Gprs_WdStop()
{
	SwTimer_Stop(&g_GprsWdTimer);
}

//喂狗，重新启动定时器
void Gprs_WdFeed()
{
//	SwTimer_ReStart(&g_GprsWdTimer);
}

//看门狗检测，超时喂狗则重新上电SIM模块
void Gprs_WdCheck()
{
//    //5MIN的看门狗时间到
//	if(SwTimer_isTimerOut(&g_GprsWdTimer))
//	{
//		//Printf("Gprs Wd timeout!\n");
//		Sim_PowerReset(SIM_GPRS_WDT);
//		return;
//	}

}

#ifdef CFG_LOG
void Gprs_SetLogSec(uint8 sec, int ind)
{
	g_PostLogInd = ind;
	g_LogReadSector = sec;
}
#endif

//GPRS处理信息打印
void Gprs_Dump()
{
//	Printf("Gprs dump:\n");
//	Printf("\t HbTimer(%d,%d,%d)\n", g_pGprs->m_HbTimer.m_Id, g_pGprs->m_HbTimer.m_isStart, g_pGprs->m_HbTimer.m_TimeOutTicks);
//	Printf("\t WdTime(%d,%d)\n", g_pGprs->m_HbTimer.m_isStart, g_pGprs->m_HbTimer.m_TimeOutTicks);
//	Printf("\t GpsisPowerOn = %d|%d\n"	, g_pGps->isPowerOn, g_pGps->isPowerOnNew);
//	Printf("\t isLowPower = %d\n"	, g_pGprs->isLowPower);
//	Printf("\t isAuth = %d\n"		, g_pGprs->isAuth);
//	Printf("\t isSimReady = %d\n"		, g_pGprs->isSimReady);
//	Printf("\t isConnected = %d\n"		, g_pGprs->isConnected);
//	Printf("\t isIndentify = %d\n"	, g_pGprs->isIndentify);
//	Printf("\t isSimSleep = %d|%d\n"	, g_pSimCard->isSimSleep, g_pSimCard->isSimSleepNew);
//	Printf("\t heartBeatInterval = %d\n"	, g_pGprs->heartBeatInterval);
//	Printf("\t IsPortChanged = %d\n"	, Gprs_IsPortChanged());
//	Printf("\t isReadSms = %d\n"	, g_pGprs->isReadSms);
//	Printf("\t isSendGps = %d\n"	, g_pGprs->isSendGps);
//	
//	
//	Printf("\t CSQ = %d\n"			, g_pSimCard->csq);
//	Printf("\t ICCID = %s\n"		, g_pSimCard->iccid);
	
}

#ifdef CFG_SIM_SLEEP

//GPRS设置睡眠
void Gprs_SetCsclk(uint8 csclk)
{
//	if(g_SmsCsclk == csclk) return;
	
	g_SmsCsclk = csclk;
	g_pSimCard->isSimSleepNew = (csclk == ENABLE_SLOW_CLK); //唤醒的话，此值为1
	g_pSimCard->isSimSleep = !g_pSimCard->isSimSleepNew;    //唤醒的话，此值为0
	
}

//GPRS是否设置睡眠
Bool Gprs_IsSlowClk() 
{
    return g_SmsCsclk == ENABLE_SLOW_CLK;
}

//GPRS获取睡眠状态
uint8 Gprs_GetCsclk() 
{
    return g_SmsCsclk;
}

#endif

//GPRS延迟发送函数
void Gprs_DelaySendCmdInd(int cmdInd, uint32 timeOutS)
{
	AtCmdItem* pCmd = Sim_GetCmdByInd(cmdInd);
	SwTimer_Start(&g_GprsCmdTimer, timeOutS * 1000, (uint32)pCmd);
}

//cmd:发送的命令/请求索引
//pData：已打包好的数据
//dataLen：已打包好的数据长度
//pBuf：需要放置的buff
//bufLen：需要放置的buff长度
int Gprs_FramePkt(uint8 cmd, const void* pData, int dataLen, void* pBuf, int bufLen)
{
	int srcInd = 0;
	int i = 0,j = 0;
	uint8_t *p= (uint8_t*)(&gRdTimeAndDate);
	UtpFrame* pFrame = (UtpFrame*)g_CommonBuf;

	pFrame->cmd = cmd;
	pFrame->data[i++] = GPRS_PROTOCOL_VER;  //放入协议版本号
	memcpy(&pFrame->data[i], g_Settings.mac, sizeof(g_Settings.mac));
	i += sizeof(g_Settings.mac);
    
    if(cmd == WIFI_MAC_POINT)
    {
        for(j = 0;j < 6;j++)
        {
            pFrame->data[i + j] = *(p + j);
        }
        i += 6;
    }

	if(dataLen)
	{
		memcpy(&pFrame->data[i], pData, dataLen);
		i += dataLen;
	}

	pFrame->len = i;

    //继续打包utp数据包,把Payload数据加上帧头和帧尾，并且转码。
	Utp_FramePkt_NB((uint8*)pFrame, i + UTP_HEAD_LEN, &srcInd, (uint8*)pBuf, &bufLen);

	return bufLen;
}

//GPRS打包DeviceReset Request复位请求
uint8 Gprs_GetDeviceResetPkt(void* pBuf, int bufLen)
{
	int i = 0;
	VerDesc* pVerDesc = Null;
	uint8 buf[50];

    buf[0] = BMS_DEVICE_TYPE;       //第一个字节为设备类型
	memcpy(&buf[1], g_pSimCard->iccid, CCID_LEN);
	i = 1+CCID_LEN;

	pVerDesc = (VerDesc*)&buf[i];
	pVerDesc->m_HwMainVer 	= cAPPInfo.hwmain;//g_pDataRom->m_HwMainVer;
	pVerDesc->m_HwSubVer  	= cAPPInfo.hwsub;//g_pDataRom->m_HwSubVer;
	pVerDesc->m_AppMainVer 	= cAPPInfo.swmain;//g_pDataRom->m_AppMainVer;
	pVerDesc->m_AppSubVer  	= cAPPInfo.swsub1;//g_pDataRom->m_AppSubVer;
	pVerDesc->m_AppMinorVer = cAPPInfo.swsub2;//g_pDataRom->m_AppMinorVer;
	pVerDesc->m_AppBuildeNum  = ((u32)cAPPInfo.swbuild[0]) | (((u32)cAPPInfo.swbuild[1])<<8) 
                                | (((u32)cAPPInfo.swbuild[2])<<16) | (((u32)cAPPInfo.swbuild[3])<<24);
    
	i += sizeof(VerDesc);

    //此处不用，只需BMS版本号即可
//	memcpy(&buf[i], pVerDesc, sizeof(VerDesc));
//	i += sizeof(VerDesc);
		
	return Gprs_FramePkt(GPRS_RESET, buf, i, pBuf, bufLen);
}

//GPRS打包定时唤醒间隔请求
uint8 Gprs_GetWakeUpInternalPkt(void* pBuf, int bufLen)
{
	int i = 0;
	uint8 buf[4];
    
    //memcpy(buf,&gGSMCond.WakeInternal,4);
    buf[3] = (u8)((gGSMCond.WakeInternal & 0xff000000) >> 24);
    buf[2] = (u8)((gGSMCond.WakeInternal & 0x00ff0000) >> 16);    
    buf[1] = (u8)((gGSMCond.WakeInternal & 0x0000ff00) >> 8);    
    buf[0] = (u8)((gGSMCond.WakeInternal & 0x000000ff) >> 0);
    
	i += sizeof(gGSMCond.WakeInternal);
		
	return Gprs_FramePkt(GPRS_WAKEINTR_CHANGE, buf, i, pBuf, bufLen);
}

//回复服务器发送过来的请求
void Gprs_TCP_SendData(const uint8* data, uint16 len)
{
	g_pGprs->pTcpPkt   = data;
	g_pGprs->tcpPktlen = len;
	AtCmdItem* pCmd = Sim_GetCmdByInd(CMD_CIPSEND);     //此处TCP数据是为了回复服务器发送过来的请求
	AtCmdCtrl_AnsySendFmt(g_pSimAtCmdCtrl, pCmd->atAck, 2000, (uint32)pCmd, pCmd->atCmd, len);
}

//TCP数据发送
Bool Gprs_SendData(const AtCmdItem* pCmd)
{
	Bool bRet = False;
	uint16 len=0;
//	uint8 pBuf[100];
	
    //复位请求已经得到确认
	if(g_pGprs->isIndentify)
    {
		g_pGprs->m_TxInitPacket = 0;
	}
    
    memset(g_GprsBuf,sizeof(g_GprsBuf),0);
    
    //第一次要发送复位请求
	if(!g_pGprs->isIndentify)
	{
		len = Gprs_GetDeviceResetPkt(g_GprsBuf, sizeof(g_GprsBuf));
	}
    else if((True == g_pGprs->isSendWifiMac))
	{
		len = g_pGprs->GetDataPkt(pCmd, g_GprsBuf, sizeof(g_GprsBuf));
	}
    
//    //打包GPS数据或者心跳数据发送出去
//    else if((True == g_pGprs->isSendGps) || (True == g_pGprs->isSendLbs))
//	{
//		len = g_pGprs->GetDataPkt(pCmd, g_GprsBuf, sizeof(g_GprsBuf));
//	}
//    //发送定时唤醒间隔通知
//    else if(g_pGprs->isSendWakeIntr)
//    {
//        len = Gprs_GetWakeUpInternalPkt(g_GprsBuf, sizeof(g_GprsBuf));
//    }    
//    //事件发生；打包成TLV发送出去
//	else if(len = GprsTlv_GetChangedTlv(pBuf, 100), len > 0)
//	{
//		len = Gprs_FramePkt(GPRS_EVENT_CHANGED, pBuf, len, g_GprsBuf, 100);
//		//DUMP_BYTE(g_GprsBuf, len);
//	}
    else
    {
        ;
    }

	ASRT(len <= sizeof(g_GprsBuf));
	if(len)
	{
        //字节数组转字符串类型
        extern int hexbyte_to_hex_str (char *src, char *obj, uint16_t length);
        len = hexbyte_to_hex_str((char *)g_GprsBuf,(char *)g_GprsTranBuf,len);
        
        g_pGprs->pTcpTranPkt = g_GprsTranBuf;
		g_pGprs->pTcpPkt   = g_GprsBuf;
		g_pGprs->tcpPktlen = len;
		//DUMP_BYTE_EX("TX:", g_GprsBuf, len);
        bRet = AtCmdCtrl_AnsySendFmt(g_pSimAtCmdCtrl, pCmd->atAck, 2000, (uint32)pCmd, pCmd->atCmd, len/2, g_pGprs->pTcpTranPkt);
	}

	return bRet;
}

//验证payload数据包头部尾部
Bool Gprs_VerifyIpPkt(const uint8* pData, int datalen)
{
	//Verify frame len;
	if(datalen < 4 || (pData[2] + 4 != datalen)) goto Error;
	
	//Verify frame head and tail.验证数据包头部和尾部
	if(pData[0] != UTP_FRAME_HEAD || pData[datalen-1] != UTP_FRAME_TAIL) goto Error;

	return True;

	Error:
	//Printf("Ip pkt error:");
	DUMP_BYTE(pData, datalen);
	return False;
}

//2,0101,\r\n
//验证tcp数据包
Bool Gprs_VerifyIpDataPkt(const char* pReq, int reqLen, const uint8** ppData, int* datalen)
{
	char* pData = Null;

    //获取payload域里的数据长度，包括头和尾
	if(sscanf(pReq, "%d,", datalen) == 0 || *datalen < 4)
		return False;
    
	pData = strstrex(pReq, ",\""); //数据域
    
	if(Null == *pData) 
        return False;
	if(memcmp(&pReq[reqLen - 3], "\"\r\n", 3) != 0)
        return False;	

    hexStrToByteArray(pData,*datalen*2,g_GprsTranBuf);
    //+IPDATA,11,HEXDATA
    pData = strstrex((char *)g_GprsTranBuf, "+IPDATA,"); //数据域
	if(sscanf(pData, "%d,", datalen) == 0 || *datalen < 4)
		return False;
    pData = strstrex(pData, ","); //数据域
	if(Null == *pData) 
        return False;
    * ppData = (uint8*)pData;
	//*ppData = (uint8*)g_GprsTranBuf;
    
	return True;//(*datalen >= (reqLen - (pData - pReq) - 3));
}

//检查req是否我一个完整的req数据包,针对命令+IPDATA,<len>,<data>,\r\n特殊处理，因为该数据包的数据部分有可能包含\r\n.
//+IPDATA,2,0101,\r\n
Bool Gprs_Filter(const char* pRcv, uint16* len)
{
	const uint8* pData = Null;
	int dataLen = 0;
	const char* pKeyWords = "+QIURC: \"recv\",0,";
	
	//如果不是\r\n结尾，肯定不是一个完整的请求包。
	if(!Sim_Filter(pRcv, len)) return False;
	
	pKeyWords = strstrex(pRcv, pKeyWords);
	if(pKeyWords)
	{
		g_pGprs->resetCounter = 0;
		//针对命令+IPDATA,<len>,<data>,\r\n特殊处理，因为该数据包的数据部分有可能包含\r\n.
        return Gprs_VerifyIpDataPkt(pKeyWords, *len-(pKeyWords-pRcv), &pData, &dataLen);
	}
	
#ifdef CFG_SIM_SLEEP
	if(IS_FOUND(pRcv, "+CMGL: "))
	{
		AtCmdItem* pItem = Null;
        //短信接收处理，返回地址头后的数据长度，目的是保留头部数据"+CMGL: "
		uint16 procLen = Sms_FilterCMGL(pRcv, *len);
		if(procLen && (*len) >= procLen)
		{
			*len -= procLen; //移除该段数据
			//Printf("procLen = %d, *len=%d\n", procLen, *len);
			memset((char*)(&pRcv[*len]), 0, procLen);
		}
		
        //确认是否接收到 \r\nOK\r\n
		pItem = Sim_GetCmdByInd(GPS_CMD_CMGL);
		return (IS_FOUND(pRcv, pItem->atAck) != Null);
	}
#endif
#ifdef CFG_GSV
	//$GPGSV,3,1,10,20,78,331,45,01,59,235,47,22,41,069,,13,32,252,45*70 
	extern void Gsv_Parse(const char* pMsg);
	pKeyWords = strstrex(pRcv, "$GPGSV,");
	if(pKeyWords)
	{
		Gsv_Parse(pRcv);
		*len = 0;
	}
	else if(    IS_FOUND(pRcv, "$GNGGA")
		|| IS_FOUND(pRcv, "$GPGSA")
		|| IS_FOUND(pRcv, "$GLGSA")
//		|| IS_FOUND(pRcv, "$GPGSV")
		|| IS_FOUND(pRcv, "$GLGSV")
		|| IS_FOUND(pRcv, "$GNRMC")
		|| IS_FOUND(pRcv, "$GNVTG")
		)
	{
		*len = 0;
	}
#endif
	
	return True;
}

//payload数据包处理过程
void Gprs_UtpRspProc(uint8 req, OP_CODE op, const uint8* data, int len)
{
	if(g_pGprs->rspProc && g_pGprs->rspProc(req, op, data, len))
	{
		return;
	}
	
	if(GPRS_RESET == req)
	{
		//Printf("GPRS_RESET\n");
        SwTimer_Stop(&g_pGprs->m_HbTimer);   //handry 停止心跳
		g_pGprs->isIndentify = True;         //已通过服务器的认证
		//g_pGprs->isSyncTime = True;        //本地时间已同步，更改为在读取短信后再进行标志同步
		g_pGprs->isSendSms = True;           //发送test短信
        
		if(len >= 6)
		{
			S_RTC_TIME_DATA_T localDt = {0};
			localDt.u32Year   = data[0] + 2000;
			localDt.u32Month  = data[1];
			localDt.u32Day    = data[2];
			
			localDt.u32Hour   = data[3];
			localDt.u32Minute = data[4];
			localDt.u32Second = data[5];
			
			DateTime_dump(&localDt);
			LocalTimeSync(&localDt);
			g_pGprs->m_syncSeconds = DateTime_GetSeconds(&localDt);

			Sign_TimerReset();
		}
		
        g_pGprs->SendRstCnt = 0;
        //启动查询wifi任务
        gWifiDealMsg.WifiFlag.WifiFlagBit.IsUpdInfo = False;
        BSPTaskStart(TASK_ID_WIFI_TASK, 500);
	}
	
	if(GPRS_PORT_STATE_CHANGED == req)
	{
		g_pGprs->g_ServerPort = g_pGprs->g_ReportedPort;
//		if(op == SUCCESS && len >= 2)
//		{
//			Pms_SetBatVerify(data[0], data[1]);
//		}
	}	
//	if(GPRS_EVENT_CHANGED == req)
//	{
//        SwTimer_Stop(&g_pGprs->m_HbTimer);	//停止心跳定时器，不继续发送tlv包
//        PostMsg(MSG_TLV_UPDATE);            //更新了TLV包
//		GprsTlv_UpdateSentTlv();
//        gGSMCond.tlvupdflg = 0;              //已经更新了
//	}	
	#ifdef CFG_LOG
	else if(GPRS_POST_LOG == req)
	{
		if(++g_PostLogRsp == 0) g_PostLogRsp++;	//防止溢出归0
		if(op == 0)
		{
			g_SysCfg.isLogValid = 1;
			g_SysCfg.postLogInd = g_PostLogInd;
			g_SysCfg.readLogStartSector = g_LogReadSector;
			g_SysCfg.readLogCount = (g_SysCfg.readLogCount > MAX_POST_LOG_COUNT) ? (g_SysCfg.readLogCount - MAX_POST_LOG_COUNT) : 0;
			Nvds_Write_SysCfg();
			PFL(DL_LOG, "Log Sent: Sec=%d, ind=%d, Remain=%d\n"
				, g_SysCfg.readLogStartSector, g_SysCfg.postLogInd, g_SysCfg.readLogCount);
		}
	}
	#endif
	
	//启动一个心跳定时器，定时查询电池状态
	//SwTimer_ReStart(&g_pPms->m_Timer);
}

//判断GPRS是否允许重发
Bool Gprs_IsAllowResend(AtCmdCtrl* pAtCmdCtrl, const AtCmdItem* pAtCmd, int reTxCount)
{
	//Printf("errCounter=%d, reTxCount=%d/%d\n", pAtCmdCtrl->m_ErrCounter, reTxCount,pAtCmdCtrl->m_MaxTxCounter);
	if(!AtCmdCtrl_IsAllowResend(pAtCmdCtrl, pAtCmd, reTxCount))
	{
		Sim_PowerReset(SIM_ATCMD_ERR);
		return False;
	}
	return True;
}

//GPRS命令处理过程
void Gprs_CmdProc(const UtpFrame* pFrame)
{
//	static Bool g_isReq = False;
	static uint8 rspRaw[32];
//	UtpFrame* pRspFrame = (UtpFrame*)rspRaw;

//	PFL(DL_GPRS, "Gprs Cmd[%d]:", pFrame->len + 2);
//	DUMP_BYTE_LEVEL(DL_GPRS, pFrame, pFrame->len + 2);
//	Bool isChanged = False;
//	Bool isTrue = False;
	int len = 0;
	uint8* pRsp = rspRaw;
	const uint8* pReq = pFrame->data;

    //设备发出的请求，服务器的响应
	if(pFrame->cmd >= GPRS_RESET && pFrame->cmd < GPRS_REQ_MAX)
	{
		Gprs_UtpRspProc(pFrame->cmd, (OP_CODE)pFrame->data[0], &pReq[1], pFrame->len - 1);
		SwTimer_Stop(&g_RcvRspTimer);
	}
    //服务器发出的请求，收到后设备进行响应
    //放电禁止控制
	else if(GPRS_FORBID_DISCHARGE == pFrame->cmd)
	{
        //此处填充BMS的代码
        //gMCData.forbidDchgChg = pReq[0]  & (BIT_0 | BIT_1);
        pRsp[len++] = 0;
        
//		isTrue = pReq[0] & BIT_0;
//		isChanged = (g_Settings.IsForbidDischarge != isTrue);
//		SetForbidDischarge(isTrue);
//		
//		isTrue = (pReq[0] & BIT_1)>0;
//		isChanged += (g_Settings.IsAlarmMode != isTrue);
//		Fsm_GoAlarmMode(isTrue);

//		//g_Settings.BatCutOffMode = ((pReq[0] & BIT_2) > 0);
//		g_pPms->m_PowerOffAtOnce = ((pReq[0] & BIT_2) > 0);
//		Pms_SendCmdDischarge();
//		isChanged = True;
//		
//		if(isChanged)
//		{
//			Nvds_Write_Setting();
//		}

//		isTrue = 0;
//		pRsp[len++] = 0;
	}
    //激活设备，暂不使用
    /*
	else if(GPRS_ACTIVE_DEVICE == pFrame->cmd)
	{
		Bool isChanged = False;
		isTrue = ((pReq[0] & BIT_0) > 0);
		if(IsActive() != isTrue)
		{
			SetActive(isTrue);
			isChanged = True;
		}

		isTrue = ((pReq[0] & BIT_1) > 0);
		if(IsSignEn() != isTrue)
		{
			SetSignEn(isTrue);
			isChanged = True;
		}

		if(isChanged)
		{
			Sign_TimerReset();
//			Nvds_Write_Setting();
		}	

		pRsp[len++] = 0;
		
	}
    */
    //设备复位，暂不使用
    /*
	else if(GPRS_DEV_RESET == pFrame->cmd && pFrame->len >= 1)
	{
		if(pReq[0] == 0)	//MCU Reset
		{
//			BootWithReason(MCURST_GPRS);
		}
		else
		{
			Sim_PowerReset(SIM_GPRS_DEV_RESET);
		}
	}
    */
    //设置电池身份校验功能使能，暂不使用
    /*
	else if(GPRS_SET_DEV_STATE == pFrame->cmd && pFrame->len >= 1)	
	{
		if(pReq[0] < 3)
		{
			isChanged = 0;
//			if(g_Settings.IsBatVerifyEn != pReq[0])
//			{
//				g_Settings.IsBatVerifyEn = pReq[0];
//				Nvds_Write_Setting();
//			}
		}
		else
		{
			isChanged = 1;
		}
		pRsp[len++] = isChanged;
	}
    */
    //当服务器检测到插入的电池非法时， 必须向设备发送电池非法请求，
    //如果电池合法时，可以不用向设备发送该命令。暂不使用。
	/*
    else if(GPRS_BAT_VERIFY == pFrame->cmd)
	{
		if(pFrame->len >= 2)
		{
//			Pms_SetBatVerify(pReq[0], pReq[1]);
		}
		pRsp[len++] = (pFrame->len >= 2) ? 0 : 1;
	}
    */
#ifdef CFG_LOG
	else if(GPRS_LOG_GET_INFO == pFrame->cmd)
	{
		LogItem logItem = {0};
		len = 0;
		pRsp[len++] = 0;
		memcpy(&pRsp[len], &g_LogRecord.total, 4);
		len += 4;
		if(Log_Read(&logItem, 1, 0))
		{
			memcpy(&pRsp[5], &logItem.dateTime, 4);
			len += 4;
		}
	}
	else if(GPRS_LOG_READ == pFrame->cmd)
	{
		DUMP_BYTE(pReq, pFrame->len);
		uint32 seconds = AS_UINT32(pReq[3], pReq[2], pReq[1], pReq[0]) + SECOND_OFFSET;
		g_SysCfg.postLogInd = Log_FindByTime(seconds);
		int count = AS_UINT32(pReq[7], pReq[6], pReq[5], pReq[4]);
		Log_SetPostInd(g_SysCfg.postLogInd, count);

		PFL(DL_LOG, "LogRead[%s]:Sec=%d, ind=%d, count=%d\n"
			, DateTime_ToStr(seconds), g_SysCfg.readLogStartSector, g_SysCfg.postLogInd, g_SysCfg.readLogCount);
		pRsp[len++] = SUCCESS;
	}
#endif

	if(len)
	{
		len = Gprs_FramePkt(pFrame->cmd, pRsp, len, g_GprsBuf, sizeof(g_GprsBuf));
		//DUMP_BYTE(g_GprsBuf, len);
		Gprs_TCP_SendData(g_GprsBuf, len);  
	}
}

//改变心跳时间
void Gprs_SetHearbeatInterval(uint32 ms)
{
	SwTimer* pTimer = &g_pGprs->m_HbTimer;
	if(ms == g_pGprs->heartBeatInterval && pTimer->m_TimeOutTicks == ms && pTimer->m_isStart)
	{
		return ;
	}

	g_pGprs->heartBeatInterval = ms;
	
	if(g_pGprs->isConnected)
	{
		//PFL(DL_GPRS, "HeartbeatTime=%d S.\n", ms/1000);
		SwTimer_Start(pTimer, g_pGprs->heartBeatInterval, TIMERID_HEARBEAT);	
	}
}

//GPRS开始等待响应
void Gprs_WaitForRsp(uint32 ms)
{
	SwTimer_Start(&g_RcvRspTimer, ms, 0);
}

//GPRS是否在等待响应
Bool Gprs_IsWaitRsp()
{
	if(g_RcvRspTimer.m_isStart)
	{
		SwTimer_isTimerOut(&g_RcvRspTimer);
		return True;
	}

	return False;
}

//GPRS发送端口变化，此处不使用
Bool Gprs_IsPortChanged()
{
	return False;
}

//打包电池槽发生变化的函数，暂不使用
int Gprs_GetPortStateChangedPkt(void* pBuf, int bufLen)
{
	static uint8 pkt[64];
	static int len = 0;
	Gprs_SetHearbeatInterval(TIME_HEARBEAT_SHORT);
	if(g_pGprs->GetPortStatePkt && g_pGprs->isConnected)
	{
		if(g_pGprs->g_ReportedPort == g_pGprs->g_ServerPort || len == 0)
		{
			len = g_pGprs->GetPortStatePkt(pkt, sizeof(pkt));
            /*
			g_pGprs->g_ReportedPort = g_pPms->m_portMask;
            */
			//Printf("portMask=%d\n", g_pGprs->g_ReportedPort);
			Gprs_SetHearbeatInterval(TIME_HEARBEAT_SHORT);
		}
	}

	if(pBuf && len)
	{
		return Gprs_FramePkt(GPRS_PORT_STATE_CHANGED, pkt, len, pBuf, bufLen);
	}
	return 0;
}

//GPRS电池换电变化，此处不使用
int Bunker_GetPortStateChangedPkt(uint8* pBuf, int bufLen)
{
    //port改变的数据打包，此处不用
    return 0;
}

//GPS定位点打包
uint8 Gprs_GetGpsCoorPkt(void* pBuf, int len)
{
	uint8* pCoorPkt = &g_CommonBuf[COM_BUF_SIZE/2];
	int pktLen = Gps_GetCoors(pCoorPkt);
	return Gprs_FramePkt(GPRS_GPS_POINT, pCoorPkt, pktLen, pBuf, len);
}

////LBS基站定位点打包
//uint8 Gprs_GetLbsCoorPkt(void* pBuf, int len)
//{
//	uint8* pCoorPkt = &g_CommonBuf[COM_BUF_SIZE/2];
//	int pktLen = Lbs_GetCoors(pCoorPkt);
//	return Gprs_FramePkt(GPRS_GPS_POINT, pCoorPkt, pktLen, pBuf, len);
//}

//WIFI MAC地址点打包
uint8 Gprs_GetWifiMacCoorPkt(void* pBuf, int len)
{
	uint8* pCoorPkt = &g_CommonBuf[COM_BUF_SIZE/2];
	int pktLen = WifiDealGetCoors(pCoorPkt);
	return Gprs_FramePkt(WIFI_MAC_POINT, pCoorPkt, pktLen, pBuf, len);
}

//GPRS数据发送打包函数：打包GPS经纬度数据和心跳数据
int Gprs_GetDataPkt(const AtCmdItem* pCmd, uint8* buff, int bufLen)
{
	int len = 0;
#ifdef CFG_LOG
	GprsLogPkt gprsLogPkt[MAX_POST_LOG_COUNT]={0};
#endif
	
    //GPS定位
	if(g_pGprs->isSendGps)
	{
		len = Gprs_GetGpsCoorPkt(buff, bufLen);
	}
//    //基站定位
//    else if(g_pGprs->isSendLbs)
//    {
//        len = Gprs_GetLbsCoorPkt(buff, bufLen);
//    }
    //WIFI MAC地址定位
    else if(g_pGprs->isSendWifiMac)
    {
        len = Gprs_GetWifiMacCoorPkt(buff, bufLen);
    }
	else 
	{
#ifdef CFG_LOG
		static int count = 0;
		count++;
		if(g_isPostEnable && g_SysCfg.readLogCount > 0)
		{
			int readCount = MIN(g_SysCfg.readLogCount, MAX_POST_LOG_COUNT); 
			if(count % 30 != 0)
			{
				g_PostLogInd    = g_SysCfg.postLogInd;
				g_LogReadSector = g_SysCfg.readLogStartSector;
				len = Log_GetLogPkt(gprsLogPkt, readCount, &g_LogReadSector, &g_PostLogInd);
				
				if(len)
				{
					len = Gprs_FramePkt(GPRS_POST_LOG, gprsLogPkt, len, buff, bufLen);
					return len;
				}
			}
			else if(g_PostLogRsp == 0)	//如果连续发送10次，都没有收到响应，说明服务器还不支持该命令，终止发送
			{
				g_isPostEnable = False;
			}
		}
		g_PostLogRsp = 0;
		count = 1;
#endif
        //打包心跳包
		//len = Gprs_GetHeartbeatPkt(buff, bufLen);
		//DUMP_BYTE(buff, len);
	}

	return len;
}

//Gprs获取心跳包数据组装
uint8 Gprs_GetHeartbeatPkt(void* pBuf, int len)
{
    //心跳包什么内容都不组装，只写入协议版本号和SID
    return Gprs_FramePkt(GPRS_HEART_BEAT, NULL, 0, pBuf, len);
}

//GPRS定时器处理任务
Bool Gprs_TimerProc()
{
    //GPRS 命令定时器时间到
	if(SwTimer_isTimerOut(&g_GprsCmdTimer))
	{
		return Sim_SendCmdItem((AtCmdItem*)g_GprsCmdTimer.m_Id);
	}
    //心跳时间到
	else if(SwTimer_isTimerOut(&g_pGprs->m_HbTimer))
	{
        //先查询信号质量，然后发送心跳数据或者gps数据
		return Sim_SendCmdInd(CMD_CSQ);
	}

	return False;
}

//GPRS运行任务
void Gprs_Run()
{	
	#define SEND_LOG_INTERAL 2000
    
	Gprs_WdCheck();
	
    //若GPRS曾执行过复位，则需要重新执行GPRS启动流程
	if(!g_pGprs->m_isStart)
	{
		Gprs_Start();
		g_pGprs->m_isStart = True;
	}
	
    //忙，则返回
	if(!AtCmdCtrl_IsIdle(g_pSimAtCmdCtrl)) return ;

	//等待响应，则返回
	if(Gprs_IsWaitRsp())
	{
		return;
	}
	
    //GPRS已经连接
	if(g_pGprs->isConnected)
	{
        if((!g_pGprs->isIndentify))
        {
            //设置时间，快速发送
            Gprs_SetHearbeatInterval(5000);
            if(g_pGprs->isConnected && !g_pGprs->m_HbTimer.m_isStart && !g_pSimCard->isSimSleep)
            {
                //若GPRS已连接，并且上次心跳时间计数结束，且GPRS未在睡眠状态，则等待下次心跳时间
                SwTimer_Start(&g_pGprs->m_HbTimer, g_pGprs->heartBeatInterval, TIMERID_HEARBEAT);	
            }            
        }
        
        //发送WIFI MAC地址
		if(g_pGprs->isSendWifiMac)
		{
			Sim_SendCmdInd(CMD_CSQ);
			return;
		}
        
        //如果发生状态改变，或者发生事件，或者复位请求一直未响应，则快速请求发送TCP数据
        //定位更新后才发送事件包        
//        if((True == g_pLbs->isLocationUpdate) || (True == g_pGps->isLocationUpdate))
//        {
//            if((GprsTlv_GetChangedTlv(g_GprsBuf, sizeof(g_GprsBuf))))
//            {
//                Sim_DelaySendCmdInd(CMD_CSQ,100); 
//                return;     
////                //设置心跳时间，快速发送
////                Gprs_SetHearbeatInterval(TIME_HEARBEAT_SHORT);
////                //Sim_SendCmdInd(CMD_CIPSEND);    //此处发送tcp数据是为了发送TLV事件包
////                if(g_pGprs->isConnected && !g_pGprs->m_HbTimer.m_isStart && !g_pSimCard->isSimSleep)
////                {
////                    //若GPRS已连接，并且上次心跳时间计数结束，且GPRS未在睡眠状态，则等待下次心跳时间
////                    SwTimer_Start(&g_pGprs->m_HbTimer, g_pGprs->heartBeatInterval, TIMERID_HEARBEAT);	
////                }            
//            }
//        }

        //加快登陆到服务器
//        if((!g_pGprs->isIndentify))
//        {
//            //设置心跳时间，快速发送
//            Gprs_SetHearbeatInterval(TIME_HEARBEAT_SHORT);
//            //Sim_SendCmdInd(CMD_CIPSEND);    //此处发送tcp数据是为了发送TLV事件包
//            if(g_pGprs->isConnected && !g_pGprs->m_HbTimer.m_isStart && !g_pSimCard->isSimSleep)
//            {
//                //若GPRS已连接，并且上次心跳时间计数结束，且GPRS未在睡眠状态，则等待下次心跳时间
//                SwTimer_Start(&g_pGprs->m_HbTimer, g_pGprs->heartBeatInterval, TIMERID_HEARBEAT);	
//            }            
//        }        
        
//        //应该读短信
//		if(g_pGprs->isReadSms)
//		{
//			Sim_SendCmdInd(GPS_CMD_CMGL);
//			return;
//		}
//        //应该发送gps信息
//		else if(g_pGprs->isSendGps)
//		{
//			Sim_SendCmdInd(CMD_CSQ);    //发送GPS经纬度信息
//			return;
//		}
//        else if(g_pGprs->isSendLbs)
//        {
//			Sim_SendCmdInd(CMD_CSQ);    //发送Lbs经纬度信息，基站定位
//			return;        
//        }
//        //通知服务器配置好的时间间隔
//        else if(g_pGprs->isSendWakeIntr)
//        {
//			Sim_DelaySendCmdInd(CMD_CSQ,500); 
//			return;        
//        }            
	}

	Gprs_TimerProc();
}

/*****************************************end of Gprs.c*****************************************/
