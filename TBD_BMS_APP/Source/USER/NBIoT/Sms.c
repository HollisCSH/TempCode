//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Sms.c
//创建人  	: 
//创建日期	:
//描述	    : Sim模组SMS短信处理文件
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
#ifdef CFG_SIM_SLEEP
#include "Sms.h"
#include "Sim.h"
#include "Fsm.h"
#include "BmsSystem.h"
#include "AtCmdTab.h"
//#include "GprsFw.h"
#include "ParaCfg.h"
#include "Storage.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
static time_t g_UnlockSmsTime = 0;
u8 gSmsReset = False;
time_t g_GetLocationSmsTime = 0;

//=============================================================================================
//定义接口函数
//=============================================================================================

//SMS复位时间
void Sms_ResetTime()
{
	g_UnlockSmsTime = 0;
}

//接收到上电READY后，随后设置短信格式，短信中心地址等
void Sms_CmdSmsReady(const char* pReq, int reqLen)
{
    if(gPCBTest.presentstep == 0x0B)
    {
        SwTimer_Stop(&g_pGprs->m_HbTimer);   //handry 停止心跳
        g_pGprs->isConnected = True;    //GPRS连接服务器成功
        g_pGprs->isIndentify = True; //已通过服务器的认证
        g_pGprs->isSyncTime = True; //本地时间已同步
    }      
    else
    {
        //等NB模块上电结束后，启动WIFI任务
        BSPTaskStart(TASK_ID_WIFI_TASK, 50);
        Sim_SendCmdInd(CMD_ATE);
    }
}

//解析收到的短信内容
//+CMGL: 1,"REC UNREAD","1064899102515","","17/09/22,14:05:22+32"
void Sms_ParserMgl(const char* str, int* ind, struct tm* pDt)
{
	#define MGL_COUNT 6
	char * strArray[MGL_COUNT];
	
	strcpy((char*)g_CommonBuf, str);
	if(MGL_COUNT != str_tok((char*)g_CommonBuf, ",", 0, strArray, MGL_COUNT)) return;

	//"17/09/22,14:05:22+32"
	//sscanf((char*)g_CommonBuf, "+CMGL: %d", &ind);
    sscanf((char*)g_CommonBuf, "+CMGL: %d", ind);
    //+1是为了避开 " 符号
	sscanf(strArray[4]+1, "%d/%d/%d", &pDt->tm_year, &pDt->tm_mon, &pDt->tm_mday);
	sscanf(strArray[5], "%d:%d:%d", &pDt->tm_hour, &pDt->tm_min, &pDt->tm_sec);

	pDt->tm_year = pDt->tm_year + 2000 - 1900;
	pDt->tm_mon -= 1;

	//Printf("%s\n", asctime(pDt));
}

//+CMGL: 1,"REC UNREAD","1064899102515","","17/09/22,14:05:22+32"
//UNLOCK
//SMS解析/读取短信，并处理
int Sms_FilterCMGL(const char* pRcv, uint16 len)
{
//	time_t time;
	int nSmsInd = 0;
//	uint8 smsId = 0xff;
	struct tm dt = {0};
	char* p1 = IS_FOUND(pRcv, "+CMGL: "); if(Null == p1) return 0;
	char* p2 = Null;
	char* p3 = Null;
	int asInt = 0;
	int asInt1 = 0;

    // \"转义为“
	p2 = strstrex(p1, "\"\r\n");
	if(p2==Null) return 0;
	
	p3 = strstrex(p2, "\r\n");
	if(p3 == Null) return 0;
	
	g_pGprs->isRcvSms = True;
	
	Sms_ParserMgl(p1, &nSmsInd, &dt);   //解析收到的短信内容
//	time = mktime(&dt);                 //获取收到短信时的时间
	
    //电池唤醒后，设置当前上报的GPS+SOC数据包个数。
	p2 = strstrex(p1, "GET_LOCATION=");
	if(p2)
	{
		//默认值为1，设置为1时代表单次刷新数据包。
        //1＜ N ≤360，代表要刷新的数据包个数。
        //N≥361，代表要一直刷新，直到电池欠压后关闭刷新。
		if(sscanf(p2, "%d", &asInt))
		{
            //接收所有被动设置
            if(gBatteryInfo.VoltChara.MinVolt >= WAKE_UP_INTIME_VOLT1)
            {
                gGSMCond.RefreshNum = asInt;            
            }
            //只接受单次刷新
            else if((gBatteryInfo.VoltChara.MinVolt < WAKE_UP_INTIME_VOLT1)
                && (gBatteryInfo.VoltChara.MinVolt >= WAKE_UP_INTIME_VOLT2))
            {
                gGSMCond.RefreshNum = 1;   
            }
            //终止刷新
            else if(gBatteryInfo.VoltChara.MinVolt < WAKE_UP_INTIME_VOLT2)
            {
                gGSMCond.RefreshNum = 0;   
            }            
            
		}
        
		goto End;
	}
	
    //停止刷新GPS+SOC数据包
	p2 = strstrex(p1, "END_LOCATION");
	if(p2)
	{
        gGSMCond.RefreshNum = 0;
		goto End;
	}
    
    //电池唤醒后，设置休眠定时唤醒间隔为N min。N为u32数。低于3.2V不接受配置。	
	p2 = strstrex(p1, "WAKEUP_INTERNAL=");
	if(p2)
	{
		if(sscanf(p2, "%d", &asInt1))
		{
            if(gBatteryInfo.VoltChara.MinVolt >= WAKE_UP_INTIME_VOLT1)
            {
                //要发送通知到服务器，告诉要更改间隔了        
                g_pGprs->isSendWakeIntr = True;       
                
                //标志需要使用这个参数来进行唤醒间隔
                gGSMCond.ParaWriteFlg = True;
                    
                //配置定时唤醒间隔
                gGSMCond.WakeInternal = asInt1;
            }
            else if((gBatteryInfo.VoltChara.MinVolt < WAKE_UP_INTIME_VOLT1)
                && (gBatteryInfo.VoltChara.MinVolt >= WAKE_UP_INTIME_VOLT2))
            {
                //要发送通知到服务器，但是不对间隔进行更改，电压太低了      
                g_pGprs->isSendWakeIntr = True;  
            }            

        }
        
		goto End;
	}
    
    //电池唤醒后，复位定时唤醒间隔为默认值。
	p2 = strstrex(p1, "RESET_INTERNAL");
	if(p2)
	{
        //要发送通知到服务器，告诉要更改间隔了
        g_pGprs->isSendWakeIntr = True; 
        
        //标志不需要使用这个参数来进行唤醒间隔
        gGSMCond.ParaWriteFlg = False;       

		goto End;
	}
    
	p2 = strstrex(p1, "RESET_MCU");
	if(p2)
	{
        gSmsReset = True;
	}    

End:
	//LOG2(ET_GPRS_SMS, smsId, g_pSimCard->csq);
	//Printf("p1=[%s]",p1);
	//Printf("p2=[%s]",p2);
	//Printf("p3=[%s],%d",p3,p3-p1);
	return p3 - p1;
}

//删除短消息后，开始连接TCP服务器
ATCMD_RC Sms_RspCMGD(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
    if(True == gSmsReset)
    {
        gSmsReset = False;
        SoftReset();
    }

	if(state == AT_SPECIFIED_OK)
	{
        if(g_pSimCard->isWakeFromSleep == False)    //准备从休眠唤醒
        {
            Gps_Wakeup();
            Sim_SendCmdInd(CMD_CGNSPWR);
        }        
		//* pNextAtCmd = Sim_GetCmdByInd(CMD_IS_CIPOPEN);
        g_pGprs->isSyncTime = True; //本地时间已同步
        g_pGprs->isSendWakeIntr = True;    //发送一次定时唤醒间隔到服务器
		return ATCRC_DONE;
	}
	return ATCRC_RESEND;
}

//SMS对列举短消息的命令处理
ATCMD_RC Sms_RspCMGL(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	//因为接收到的数据可能很多，Req buffer可能放不下，因此数据在Gprs_AtCmdFilter中预先处理
	if(state == AT_SPECIFIED_OK)
	{
		g_pGprs->isReadSms = False;
		if(g_pGprs->isRcvSms)
		{
			time_t time = DateTime_GetSeconds(Null);
			
			if(g_UnlockSmsTime)
			{
				g_pGprs->isGetLock = True;
				PFL(DL_MAIN, "Sms delay: %d S\n", time - g_UnlockSmsTime);
				g_UnlockSmsTime = 0;
			}
			g_GetLocationSmsTime = 0;
			//读取后就开始删除已读信息
			*pNextAtCmd = Sim_GetCmdByInd(CMD_CMGD);
		}
		else
		{
            g_pGprs->isSyncTime = True; //本地时间已同步
            g_pGprs->isSendWakeIntr = True;    //发送一次定时唤醒间隔到服务器
			//判断是否被踢出网络而唤醒
//			if(WAKEUP_SIM == GetWakeUpType())	
//			{
//				PostMsg(MSG_TIMEOUT);
//			}
//			else
//			{
//				*pNextAtCmd = Sim_GetCmdByInd(CMD_IS_CIPOPEN);
//			}
			//*pNextAtCmd = Sim_GetCmdByInd(CMD_CIPSEND);
            if(g_pSimCard->isWakeFromSleep == False)    //准备从休眠唤醒
            {
                Gps_Wakeup();
                Sim_SendCmdInd(CMD_CGNSPWR);
            }
		}
		
		g_pGprs->isRcvSms = False;
		return ATCRC_DONE;
	}
	else if(AT_ERROR == state)
	{
		//Gprs_DelaySendCmdInd(CMD_NETCLOSE, 1);
		Sim_SyncSendAtCmd("AT+CMGF=1", Null, 100);
		Sim_SyncSendAtCmd("AT+CSCS=\"GSM\"", Null, 100);
	}

	return ATCRC_RESEND;
}

//SMS对设置短信中心地址的命令处理
ATCMD_RC Sms_CmdGSM(AtCmdState state, char* pRsp, struct _AtCmdItem** pNextAtCmd)
{
	if(state == AT_SPECIFIED_OK || state == AT_ERROR)
		return ATCRC_SUCCESS;
	return ATCRC_RESEND;
}


/*
+CMTI: "SM",27
AT+CMGF=1

OK
AT+CSCS="GSM"

OK
AT+CMGR=1

+CMGR: "REC READ","1064899102515","","17/09/14,16:38:59+32"
test msg1

OK
*/
//SMS对SIM卡收到了短信的处理
void Sms_CmdCMTI(const char* pReq, int reqLen)
{
    if(g_pSimCard->isWakeFromSleep == False)
    {
        PostMsg(MSG_SIM_ACTIVE_WAKEUP);
        Sim_Wakeup();
        return;
    }
    
	if(g_pGprs->isSyncTime)
	{
		AtCmdCtrl_ResetEx(g_pSimAtCmdCtrl);
		if(!Sim_SendCmdInd(GPS_CMD_CMGL))
		{
			g_pGprs->isReadSms = True;
		}
	}
	else
	{
		g_pGprs->isReadSms = True;
	}
}

//发送短信命令响应函数
ATCMD_RC Gprs_RspSMGS(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	ATCMD_RC nRet = ATCRC_DONE;
	if(state == AT_SPECIFIED_OK)
	{
//		static const char* g_buff = "Test\r\n\x1A";
		Sim_DelaySendCmdInd(CMD_SMS_TEXT, 100);
		//AtCmdCtrl_SyncSend(g_pSimAtCmdCtrl, g_buff, "OK", 5000);
	}
	return nRet;
}

//短信发送接收后响应处理函数
ATCMD_RC Gprs_RspSMGSTEXT(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd)
{
	if(state == AT_ERROR || state == AT_SPECIFIED_OK)
	{
        //发送短信结束，等待获取短信
		g_pGprs->isSendSms = False;
		g_pGprs->isReadSms = True;
	}
	
	return ATCRC_DONE;
}

#endif

/*****************************************end of Sms.c*****************************************/
