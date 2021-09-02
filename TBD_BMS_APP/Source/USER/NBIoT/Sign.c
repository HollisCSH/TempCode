//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Sign.c
//创建人  	: 
//创建日期	:
//描述	    : sim模组打卡登记操作文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 实现定时打卡登记功能
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "Sign.h"
#include "DateTime.h"
#include "BmsSystem.h"
#include "GPRS.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
static time_t g_SignTime   = 0; //打卡时间点，秒
Bool g_IsResetDisable = False;  //为ture时，则不复位时间

//=============================================================================================
//定义接口函数
//=============================================================================================

//打卡信息打印
void Sign_Dump(const char* pTag)
{
//	static const char* pStr[] = {"NONE","VERIFY_OK","VERIFY_FAILED"};
//	time_t sec = DateTime_GetSeconds(Null);

//	if(pTag) Printf("%s", pTag);
//	
//	Printf("T_Start=%s\n"	, DateTime_ToStr(g_SignTime));
//	Printf("T_Cur=%s\n"		, DateTime_ToStr(sec));
//	Printf("T_Cur - T_Start=%s\n", DateTime_ToStr(sec - g_SignTime));
//	
//	Printf("Act=%d, SignIsOk=%d,forbid=%d,AlarmMode=%d, SignEn=%d, T_RstEn=%d, RemainTime:%s\n"
//		, IsActive()
//		, Sign_IsOk()
//		, IsForbidDischarge()
//		, g_Settings.IsAlarmMode
//		, IsSignEn()
//		, !g_IsResetDisable
//		, DateTime_ToStr(g_RemainSeconds)
//		);
//	
//	Printf("BatVerifyEn=%d, Port0BatVerfy=%s,Port1BatVerfy=%s\n"
//		, g_Settings.IsBatVerifyEn
//		, pStr[g_pPms->m_Port0Verify]
//		, pStr[g_pPms->m_Port1Verify]
//		);
}

//获取打卡剩余分钟
uint32 Sign_GetRemainMin()
{
	return g_Settings.remainSeconds / 60;
}

//获取打卡时间点
uint32 Sign_GetSignTime()
{
	return g_SignTime;
}

//不使能打卡时间复位
void Sign_DisableTimerReset(Bool isTrue)
{
	g_IsResetDisable = isTrue;
	//Printf("%s timer reset\n", isTrue ? "Disable" : "Enable");
}

//设置剩余时间
void Sign_SetMaxTime(uint32 minute)
{
	g_Settings.loginMaxMinute = minute;
	g_RemainSeconds = LOGIN_MAX_TIME;   //转换为秒单位
	//Printf("T_Remain=%s\n"	, DateTime_ToStr(g_RemainSeconds));
}

//打卡功能使能/去使能
void Sign_SetEnable(Bool isEnable)
{
	if(g_Settings.IsSignEnable != isEnable)
	{
//		Printf("Sign %s.\n", isEnable ? "Ena" : "DisEna");
		if(isEnable)
		{
			g_RemainSeconds = LOGIN_MAX_TIME;
		}
		
		g_Settings.IsSignEnable = isEnable;
	}
}

//是否使能打卡
Bool IsSignEn()
{
    return g_Settings.IsSignEnable;
}


//打卡时间开始计时
void Sign_TimerStart()
{
    //获取当前时间秒数
	g_SignTime = DateTime_GetSeconds(Null);
}

//打卡时间停止计时，计算间隔时间
void Sign_TimerStop()
{
//	if(!IsSignEn()) return;
	
	time_t sec = DateTime_GetSeconds(Null);
	if(sec < g_SignTime)    //发生计时错误
    {
        return;
    }

    //间隔时间
	sec -= g_SignTime; 

	if(g_RemainSeconds > sec)
	{
		g_RemainSeconds -= sec;
	}
	else
	{
		if(g_RemainSeconds)
		{
			//LOG2(ET_SYS_SIGN_FAILED, g_Settings.devcfg, 0);
		}
		g_RemainSeconds = 0;
	}

	//Printf("SignStop(%s)\n", DateTime_ToStr(g_RemainSeconds));
}

//打卡时间复位
void Sign_TimerReset()
{
	//Only for test, Sign timer not reset.
	if(g_IsResetDisable)
	{
		//Printf("Test: Sign Not Reset.\n");
		return;
	}
	//End
	
	g_SignTime = DateTime_GetSeconds(Null);
	g_RemainSeconds = LOGIN_MAX_TIME;
	//Printf("SignReset(%s)\n", DateTime_ToStr(g_RemainSeconds));
}

//确认打卡的时间点是否正常
Bool Sign_IsOk()
{
	Bool nRet = True;
	
    nRet = IsSignEn() ? (g_RemainSeconds > 0) : True;
//	Printf("Sign_IsOk(%d).\n", nRet);
	return nRet;
}

/*****************************************end of Sign.c*****************************************/
