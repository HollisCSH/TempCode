//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Gprs868.c
//创建人  	: 
//创建日期	:
//描述	    : Sim模组的SIM868 2G/4G处理文件
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
#include "BmsSystem.h"
#include "Fsm.h"
#include "sim.h"

//=============================================================================================
//定义接口函数
//=============================================================================================

//关闭回显响应处理
ATCMD_RC Gprs_RspATE0(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
    if(state == AT_SPECIFIED_OK)
	{
        Gprs_SetCsclk(DISABLE_SLOW_CLK);
        *pNextAtCmd = Sim_GetCmdByInd(CMD_SIM_CSCLK);
        return ATCRC_DONE;
    }
    else
    {
        if(g_pGprs->resetCounter <= 4)
            g_pGprs->resetCounter++;
        return ATCRC_RESEND;
    }
}

//设置发送接收格式
ATCMD_RC Gprs_Rsp_SET_FORMAT(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
    if(state == AT_SPECIFIED_OK)
	{
        return ATCRC_SUCCESS;
    }
    else
    {
        return ATCRC_RESEND;
    }
}


//暂不使用
ATCMD_RC Gprs_RspCPIN(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	g_pGprs->isConnected = False;
	if(state == AT_SPECIFIED_OK)
	{
		return IS_FOUND(pRsp, "+CPIN: READY") ? ATCRC_SUCCESS : ATCRC_RESEND;
	}
	return  ATCRC_SIMCARD_INVALID;
}

//查询信号质量响应处理函数
ATCMD_RC Gprs_RspCSQ(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	ATCMD_RC nRet = ATCRC_DONE;
	int value = 0;
	
	//启动10定时器, 检测连接状态
	if(state == AT_SPECIFIED_OK)
	{
		pRsp = strstrex(pRsp, "+CSQ: ");
		value = strtokenValueByIndex(pRsp, ",", 0);
		g_pSimCard->csq = value;    //CSQ值
        
        gGSMCond.gsmcsq = value;
		if(value <= 0)
		{
            //5S后重新查询CSQ值
			Gprs_DelaySendCmdInd(CMD_CSQ, 5);
			return ATCRC_DONE;
		}
	}
	else
	{
        
#ifdef CFG_SIM_SLEEP
		if(state == AT_TIMEOUT)
		{
            //响应超时，可能是SIM休眠导致的，关闭休眠
			Sim_SyncSendAtCmd("AT+CSCLK=0", Null, 0);
		}
#endif	
        
		nRet = ATCRC_RESEND;
	}
	
	if((value > 0) && g_pGprs->isConnected)
	{
#ifdef CFG_NO_GPS
		Sim_SendCmdInd(CMD_CIPSEND);
#else
//		if(!g_pGps->isLocation)
//		{
            //未定位则定位
//			Gps_Location(True);	
			//防止GPS和GPRS两个命令重复。
			//Gprs_DelaySendCmdInd(CMD_CIPSEND, 3);
//		}
//		else
//		{
            //SIM卡信号无异常，则开始发送TCP数据到服务器，心跳包或者gps信息
            Sim_SendCmdInd(CMD_CIPSEND);
//		}
#endif
	}
	else if(!g_pGprs->isGatt)
	{
        //信号无异常，继续查询网络附着状态
		*pNextAtCmd = Sim_GetCmdByInd(CMD_CGATT);
	}

	return nRet;
}

//查询是否附着网络响应函数
ATCMD_RC Gprs_RspCGATT(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	ATCMD_RC nRet = ATCRC_RESEND;
	
	if(state == AT_SPECIFIED_OK)
	{
		int cgatt = 0;
		pRsp = strstrex(pRsp, "+CGATT: ");
		cgatt = pRsp[0]-'0';
		g_pGprs->isGatt = (cgatt > 0);  //1代表网络附着成功，0代表失败
        
        //GPRS 附着状态:0 分离
		if(cgatt == 0)
		{
            //1S后重新查询信号质量
			Gprs_DelaySendCmdInd(CMD_CSQ, 1);
			nRet = ATCRC_DONE;
		}
        //GPRS 附着状态:1 附着
		else
		{
			nRet = ATCRC_SUCCESS;
			//Printf("GPRS Att Ok.\n");
		}
	}
	
    //若接收不成功，重发
	return nRet;
}

//设置网络附着
ATCMD_RC Gprs_RspCGATT_SET(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	ATCMD_RC nRet = ATCRC_RESEND;
	
	if(state == AT_SPECIFIED_OK)
	{
        //附着成功后，立刻查询CCID号
		*pNextAtCmd = Sim_GetCmdByInd(CMD_CCID);
	}
	
	return nRet;
}

//发送CIPSEND后，等待输入数据的处理函数
ATCMD_RC Gprs_RspCIPSEND(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	ATCMD_RC nRet = ATCRC_DONE;
	
	if(IS_FOUND(pRsp, ">"))
	{
		//必须清除掉所有Pending的命令
		AtCmdCtrl_ResetPendingCmd(g_pSimAtCmdCtrl);
		Sim_SendCmdInd(CMD_CIPSENDDATA);
	}	
	else if(AT_ERROR== state)
	{
        //重新进行TCP连接
		g_pGprs->tcpPktlen = 0;
		*pNextAtCmd = Sim_GetCmdByInd(CMD_IS_CIPOPEN);
	}

	return nRet;
}

//TCP数据发送成功完毕后的，响应处理函数
ATCMD_RC Gprs_RspCIPSENDDATA(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	ATCMD_RC nRet = ATCRC_DONE;
	UtpFrame* pFrame = (UtpFrame*)&g_pGprs->pTcpPkt[1];
	
	
	if(state == AT_SPECIFIED_OK)
	{
        //心跳包发送成功
		if(GPRS_HEART_BEAT == pFrame->cmd)
		{
            //协议自己组装
			//g_pGprs->isSendCabinLock = False;
			Gprs_WdFeed();
			PostMsg(MSG_GPRS_UPDATE);
			SwTimer_ReStart(&g_pGprs->m_HbTimer);
			g_pGprs->m_HbCount++;

			if(g_pGprs->m_HbCount % 10 == 0)	//每发满10次(每次1分钟,大概10分钟)记录一次
			{
				//LOG2(ET_GPRS_HEARBEAT, g_pGprs->m_HbCount, g_pGprs->m_GpsCount);
			}
//            //开始发送短信
//			if(g_pGprs->isSendSms)
//			{
//				Sim_DelaySendCmdInd(CMD_CMGS, 5000);
//			}
		}
		else if(GPRS_GPS_POINT == pFrame->cmd)
		{
			PostMsg(MSG_GPS_UPDATE);
			g_pGprs->isSendGps = False;
            
            if(True == g_pGprs->isSendLbs)  //如果这个点是由基站定位发送，则切换到GPS进行定位
            {
                Sim_SendCmdInd(CMD_SAPBR_CLOSE);  //关闭IP应用            
                g_pGprs->isSendLbs = False;
            }
			g_pGprs->m_GpsCount++;
            
            //N ≥ 361，代表要一直刷新，直到电池欠压后关闭刷新。
            if((0 < gGSMCond.RefreshNum) && (360 > gGSMCond.RefreshNum))
            {
                gGSMCond.RefreshNum--;
            }
		}
        else if(GPRS_WAKEINTR_CHANGE == pFrame->cmd)
        {
            //通知发送一次
            g_pGprs->isSendWakeIntr = False;
        }
		else if(WIFI_MAC_POINT == pFrame->cmd)
		{
			PostMsg(MSG_WIFI_MAC_UPDATE);
			g_pGprs->isSendWifiMac = False;
            
			g_pGprs->m_WifiCount++;
		}        
		else
		{
            if(GPRS_RESET == pFrame->cmd)
            {
                //复位包超过3次没有回复，则复位一次
                if(g_pGprs->SendRstCnt++ > 3)
                {
                    g_pGprs->SendRstCnt = 0;
                    Sim_PowerReset(SIM_CONNECT_ERROR);
                }
            }
            //复位包等待服务器响应
			Gprs_WaitForRsp(3000);
		}
		
	}
	else
	{
		//Printf("Send data failed**************.\n");
		* pNextAtCmd = Sim_GetCmdByInd(CMD_NETCLOSE);
		//LOG2(ET_GPRS_SEND_FAILED, Mcu_GetRound5V(), g_pSimCard->csq);
	}
	
	return nRet;
}

//查询网络注册情况的响应处理函数
ATCMD_RC Gprs_RspCREG(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	ATCMD_RC nRet = ATCRC_SUCCESS;
	int value = 0;

	g_pGprs->isReg = False;
	if(state == AT_SPECIFIED_OK)
	{
		g_pGprs->isConnected = False;
		value = strtokenValueByIndex(pRsp, ",", 1);

        //未注册； ME 当前没有搜索要注册业务的新运营商
		if(value == 0)
		{	
            //发送注册命令
			Sim_SyncSendAtCmd("AT+CEREG=1", NULL, 100);
			nRet = ATCRC_RESEND;
		}
        //已注册：1是本地网，5是漫游网
		else if((value == 1) || (value == 5))
		{
			g_pGprs->isReg = True;
			//Printf("GPRS reg Ok.\n");
			nRet = ATCRC_SUCCESS;
		}
		else if(value == 3 || value == 4)
		{	
            //GSM测试状态,SIM卡是否注册失败，停机
            if(0x0A == gPCBTest.presentstep)
            {
                BITSET16(gGSMCond.gsmteststat,3);
            }            
            //注册失败，停机
			nRet = ATCRC_REG_DENIED;
		}
        //正在注册，继续发送查询
		else
        {
			nRet = ATCRC_RESEND;
        }
	}
	else
	{
		nRet = ATCRC_RESEND;    //若接收不成功，包括接收到error或者超时，重发
	}
	return nRet;
}

//查询 SIM 卡 ICCID 号码响应回复处理
//8944502511169276250F
ATCMD_RC Gprs_RspCCID(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	ATCMD_RC nRet = ATCRC_SUCCESS;

	if(state == AT_SPECIFIED_OK)
	{	
		nRet = ATCRC_SUCCESS;
        //最多复制 n 个字符。当 src 的长度小于 n 时，dest 的剩余部分将用空字节填充。
		strncpy(g_pSimCard->iccid, &pRsp[10], CCID_LEN);
	}
	else
	{
		nRet = ATCRC_NETWORK_REG_FAIL;
	}
	return nRet;
}

//+CIPOPEN: 0,0
//建立TCP连接，响应处理函数
ATCMD_RC Gprs_RspCIPOPEN(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	ATCMD_RC nRet = ATCRC_DONE;
	static int count = 0;

	if(state == AT_SPECIFIED_OK)
	{
		Sim_SetTxDataTime(GPRS_SENDDATA_TIME_LONG);
		count = 0;
	}
	else if(state == AT_ERROR)
	{
		count++;
		if(count <= 1) //复位一次
		{
			Sim_PowerReset(SIM_CIPOPEN_ERROR);
			return ATCRC_DONE;
		}
		else
		{
			count = 0;
			nRet = ATCRC_NETWORK_REG_FAIL;
		}
	}
	else
	{
        //无响应
		 nRet = ATCRC_RESEND;
	}

	//延迟10秒查询状态，10S连接不成功，则重发
	Gprs_DelaySendCmdInd(CMD_IS_CIPOPEN, 10);
	return nRet;
}

//关闭移动场景响应处理函数
ATCMD_RC Gprs_RspSHUTDOWN(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	if(state == AT_SPECIFIED_OK)
	{
	#ifdef GPRS_FW_UPGRADE
		if(GprsFw_IsUrlReady())	//判断是否进行网络升级
		{
			*pNextAtCmd = Sim_GetCmdByInd(CMD_SAPBR1);
		}
		else
	#endif
		{
            //关闭成功后，设置APN
			*pNextAtCmd = Sim_GetCmdByInd(CMD_SOCKCFG);
		}
	}
	else
	{
		Sim_PowerReset(SIM_SHUTDOWN_ERROR);
	}
	return ATCRC_DONE;
}

//关闭 TCP 或 UDP 连接响应处理函数
ATCMD_RC Gprs_RspNETCLOSE(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	#ifdef SIM_SMS
	*pNextAtCmd = Sim_GetCmdByInd(CMD_CMGF);
	#else
    if(False == g_pSimCard->isSimSleepNew)
    {
        //*pNextAtCmd = Sim_GetCmdByInd(CMD_IS_CIPOPEN);
        *pNextAtCmd = Sim_GetCmdByInd(CMD_CIPOPEN);
    }
    else
    {
        ;
    }
	#endif
	return ATCRC_DONE;
}

//暂不使用
void Gprs_CmdState(const char* pReq, int reqLen)
{
	
	g_pGprs->isConnected = (Null != IS_FOUND(pReq,"CONNECT OK"));
	if(g_pGprs->isConnected)
	{
		Sim_SendCmdInd(CMD_CSQ);
	}
	else
	{
		if(IS_FOUND(pReq,"CLOS"))
		{
			Sim_SendCmdInd(CMD_CIPOPEN);
		}
		else if(IS_FOUND(pReq,"IP INITIAL"))
		{
			Gprs_DelaySendCmdInd(CMD_CIPOPEN, 1);
		}
		else if(IS_FOUND(pReq,"ING"))
		{
			Gprs_DelaySendCmdInd(CMD_IS_CIPOPEN, 5);
		}
		else
		{
			AtCmdCtrl_ResetEx(g_pSimAtCmdCtrl);
			Sim_SendCmdInd(CMD_NETCLOSE);
		}
	}
}

//查询当前连接状态响应处理函数
ATCMD_RC Gprs_RspCIPSTATUS(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	int remainMin = Sign_GetRemainMin();
	if(state == AT_SPECIFIED_OK)
	{
		g_pGprs->isConnected = (Null != IS_FOUND(pRsp,"CONNECT OK"));
        
        //找到"CONNECT OK"字符串，代表TCP建立连接成功
		if(g_pGprs->isConnected)
		{
			//LOG2(ET_GPRS_CNT, (uint8)remainMin, g_pSimCard->csq);
			//Gprs_DelaySendCmdInd(CMD_CSQ, 1);
		}
        //连接中，继续查询
		else if(IS_FOUND(pRsp,"ING"))
		{
			Gprs_DelaySendCmdInd(CMD_IS_CIPOPEN, 1);
			return ATCRC_DONE;
		}
		else
		{
			int reason = GPRS_LINK_BROKEN_UNKNOWN;
            //连接断开，则重新建立连接
			if(IS_FOUND(pRsp,"CLOS"))
			{
				reason = GPRS_LINK_BROKEN_CLOSED;
				* pNextAtCmd = Sim_GetCmdByInd(CMD_CIPOPEN);
			}
            //初始化中，则重新建立连接
			else if(IS_FOUND(pRsp,"IP INITIAL"))
			{
				reason = GPRS_LINK_BROKEN_IPINIT;
				* pNextAtCmd = Sim_GetCmdByInd(CMD_CIPOPEN);
			}
            //场景被释放，关闭移动场景后，重新进行建立APN进行连接服务器
			else if(IS_FOUND(pRsp,"PDP DEACT"))
			{	//AT+CGDCONT=1,"IP","CMNET"
				* pNextAtCmd = Sim_GetCmdByInd(CMD_SHUTDOWN);
				reason = GPRS_LINK_BROKEN_PDP_DEACT;
			}
			
//			LOG2(ET_GPRS_DIS_CNT, reason, g_pSimCard->csq);
			g_pGprs->isIndentify = False; //清除服务器认证，重新登录
			if(GPRS_LINK_BROKEN_UNKNOWN == reason)
			{
				//STATE: PDP DEACT 	GPRS is disconnected by network
                //找不到原因，则重新复位电源
				Sim_PowerReset(SIM_CIPSTATUS_ERROR);
			}
		}
		return ATCRC_DONE;
	}
    else if(state == AT_TIMEOUT) //超时，无连接
    {
        //无连接
        Sim_PowerReset(SIM_NO_CONNECT);
    }
	return ATCRC_RESEND;
}

//SIM卡相关事件URC响应处理函数
void Gprs_CmdSimCardEvent(const char* pReq, int reqLen)
{
	//+CPIN: NOT READY  SIM卡未准备好
	if(IS_FOUND(pReq, "READY"))
	{
		g_pGprs->isSimReady = SIMCARD_NOT_READY;
        if(0x0A != gPCBTest.presentstep)
        {
            Sim_PowerReset(SIM_CPIN_NOT_READY);
        }
        else
        {
            g_pGprs->isSyncTime = True;
        }        
		
		return;
	}
	else if(IS_FOUND(pReq, "INSERTED"))	//+CPIN: NOT INSERTED
	{
		//Printf("Sim card: NOT INSERTED\n");
        //GSM测试状态,SIM卡是否插入
        if(0x0A == gPCBTest.presentstep)
        {
            BITSET16(gGSMCond.gsmteststat,2);
        }
		g_pGprs->isSimReady = SIMCARD_NOT_INSERTED;
	}
	
	SetErrorCode(ERR_TYPE_GPRS, TC_GPRS_SIMCARD_INVALID, ET_ALARM);
	PostMsg(MSG_SIM_FAILED);
    
    if(0x0A != gPCBTest.presentstep)
    {
        Sim_PowerReset(SIM_CPIN_NOT_INSERTED);
    }
    else
    {
        
		g_pGprs->isSyncTime = True;
    }
}

//收到连接服务器CONNECT OK的URC，响应处理函数
void Gprs_CmdConnect(const char* pReq, int reqLen)
{	
	//关闭查询状态通知
	int remainMin = Sign_GetRemainMin();
    int value = 0;
    
	SwTimer_Stop(&g_GprsCmdTimer);
    
	//if(IS_FOUND(pReq, "OK"))//+CIPOPEN: 0,0 表示IP open 成功
    value = strtokenValueByIndex((char*)pReq, ",", 1);
    
    //成功连接
    if(value == 0)
	{
		g_pGprs->isConnected = True;    //GPRS连接服务器成功
		g_pGprs->m_TxInitPacket = 0;
		Sim_SendCmdInd(CMD_CIPSEND);    //此处发送tcp数据是为了发送复位请求
		//LOG2(ET_GPRS_CNT, (uint8)remainMin, g_pSimCard->csq);
    }
	else
	{
		Sim_PowerReset(SIM_CONNECT_ERROR);
	}
	//PFL(DL_MAIN, "%s[%d] connect %s.\n", g_pDataRom->m_IpAddr, g_pDataRom->m_IpPort, g_pGprs->isConnected ? "OK" : "FAILED");
}

//标识 GPRS 被网络释放处理， 此时仍然需要执行 AT+CIPSHUT 来改变状态。
void Gprs_CmdPdpDeact(const char* pReq, int reqLen)
{	
	Sim_PowerReset(SIM_SHUTDOWN_ERROR);
	//LOG2(ET_GPRS_DIS_CNT, GPRS_LINK_BROKEN_PDP_DEACT, g_pSimCard->csq);
	//Sim_SendCmdInd(CMD_SHUTDOWN);
}

//TCP连接关闭，需要重新连接服务器的处理
void Gprs_CmdClosed(const char* pReq, int reqLen)
{	
	//Sim_PowerReset();
	//LOG2(ET_GPRS_DIS_CNT, GPRS_LINK_BROKEN_CLOSED, g_pSimCard->csq);
    g_pGprs->isIndentify = False; //清除服务器认证，重新登录
    g_pGprs->isConnected = False;
	Sim_SendCmdInd(CMD_NETCLOSE);
}

//设置休眠唤醒响应处理函数
ATCMD_RC Dock_RspCSCLK(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	ATCMD_RC nRet = ATCRC_DONE;
	
	if(state == AT_SPECIFIED_OK || state == AT_ERROR)
	{
		g_pSimCard->isSimSleep = g_pSimCard->isSimSleepNew;
        
        if(g_pSimCard->isSimSleep == False)
        {
            g_pSimCard->isSimStartOK = Sim_Wake_OK;
        }
        
		if(ENABLE_SLOW_CLK == Gprs_GetCsclk())
		{
//			LOG2(ET_SIM_SLEEP, g_pGprs->m_HbCount, g_pGprs->m_GpsCount);
			
			PostMsg(MSG_SIM_POWEROFF);

			return nRet;
		}
		else
		{
            *pNextAtCmd = Sim_GetCmdByInd(CMD_SET_FORMAT);
//			LOG2(ET_SIM_WAKEUP, g_Settings.devcfg, Mcu_GetRound5V());
		}
//		if(WAKEUP_SIM == GetWakeUpType())
//		{
//			*pNextAtCmd = Sim_GetCmdByInd(GPS_CMD_CMGL);
//		}
//		else
//		{
//			*pNextAtCmd = Sim_GetCmdByInd(CMD_IS_CIPOPEN);
//		}
    }
	else
	{
		nRet = ATCRC_RESEND;
	}
	
	return nRet;
}

//TCP数据处理函数，包括服务器的请求及响应处理
void Gprs_CmdIpData(const char* pReq, int reqLen)
{
	int i = 0;
	const uint8* pData = Null;
	int dataLen = 0;
    
	FRAME_STATE state = FRAME_INIT;
	static uint8 buf[UTP_REMOTE_REQ_BUF_SIZE];
	UtpFrame* pFrame = (UtpFrame*)&buf[1];
	uint16 ind = 0;

	if(!Gprs_VerifyIpDataPkt(pReq, reqLen, &pData, &dataLen) || !Gprs_VerifyIpPkt(pData, dataLen))
	{
		//Printf("IpData error[%s].\n", pReq);
		return ;
	}

	for(i = 0; i < dataLen; i++)
	{		
		if(Utp_ConvertToHost_NB(buf, sizeof(buf), &ind, &state, pData[i])
			&& pFrame->len + 4 == ind)	//校验帧长度是否有效
		{
            //GPRS命令处理
			Gprs_CmdProc(pFrame);
		}
	}
}

/*****************************************end of Gprs868.c*****************************************/
