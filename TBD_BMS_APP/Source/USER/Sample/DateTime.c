//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: DateTime.c
//创建人  	: 
//创建日期	:
//描述	    : 读写时间
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "DateTime.h"
#include "RTC.h"			//用于替代PCF85063.h
//#include "PCF85063.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
//tm格式注释
//struct tm {
//   int tm_sec;         /* 秒，范围从 0 到 59                */
//   int tm_min;         /* 分，范围从 0 到 59                */
//   int tm_hour;        /* 小时，范围从 0 到 23                */
//   int tm_mday;        /* 一月中的第几天，范围从 1 到 31                    */
//   int tm_mon;         /* 月份，范围从 0 到 11                */
//   int tm_year;        /* 自 1900 起的年数                */
//   int tm_wday;        /* 一周中的第几天，范围从 0 到 6                */
//   int tm_yday;        /* 一年中的第几天，范围从 0 到 365                    */
//   int tm_isdst;       /* 夏令时                        */    
//};

S_RTC_TIME_DATA_T sInitTime;   //本地时间

//=============================================================================================
//定义接口函数
//=============================================================================================

//打印时间
void DateTime_dump(S_RTC_TIME_DATA_T* dt)
{
	S_RTC_TIME_DATA_T sCurTime;
	if(dt == Null)
	{
	    /* Get the current time */
	    RTC_GetDateAndTime(&sCurTime);
		dt = &sCurTime;
	}
//    Printf("DataTime:%d/%02d/%02d %02d:%02d:%02d\n",dt->u32Year,dt->u32Month,
//           dt->u32Day,dt->u32Hour,dt->u32Minute,dt->u32Second);
}

//读取本地时间到S_RTC_TIME_DATA_T格式中
void RTC_GetDateAndTime(S_RTC_TIME_DATA_T *sPt)
{
    //年
    sPt->u32Year = PCF85063_BCD_to_HEC(gRdTimeAndDate.year) + RTC_YEAR2000;
    //月
    sPt->u32Month = PCF85063_BCD_to_HEC(gRdTimeAndDate.month);
    //日
    sPt->u32Day = PCF85063_BCD_to_HEC(gRdTimeAndDate.day);
    //时
    sPt->u32Hour = PCF85063_BCD_to_HEC(gRdTimeAndDate.hour);
    //分
    sPt->u32Minute = PCF85063_BCD_to_HEC(gRdTimeAndDate.minute);
    //秒
    sPt->u32Second = ((((gRdTimeAndDate.second & 0x70) >> 4) * 10) + (gRdTimeAndDate.second & 0x0f));
}

//根据S_RTC_TIME_DATA_T时间格式设置时间到RTC
void RTC_SetDateAndTime(S_RTC_TIME_DATA_T *sPt)
{
    gWrTimeAndDate.year = PCF85063_HEC_to_BCD((sPt->u32Year - RTC_YEAR2000));
    gWrTimeAndDate.month = PCF85063_HEC_to_BCD(sPt->u32Month);
    gWrTimeAndDate.day = PCF85063_HEC_to_BCD(sPt->u32Day);
    gWrTimeAndDate.hour = PCF85063_HEC_to_BCD(sPt->u32Hour);
    gWrTimeAndDate.minute = PCF85063_HEC_to_BCD(sPt->u32Minute);
    gWrTimeAndDate.second = PCF85063_HEC_to_BCD(sPt->u32Second);

    //标定时间
	PCF85063SetCtrl(PCF85063_START_WRITE);    
}

//设置S_RTC_TIME_DATA_T格式的时间
S_RTC_TIME_DATA_T* DateTime_MakeRtc(int year, int month, int day, int hour, int minute, int sec)
{
	static S_RTC_TIME_DATA_T g_dt = {0};
	g_dt.u32Year   = year;
	g_dt.u32Month  = month;
	g_dt.u32Day    = day ;
	
	g_dt.u32Hour   = hour;
	g_dt.u32Minute = minute;
	g_dt.u32Second = sec;
	return &g_dt;
}

//获取当前时间的Seconds
time_t DateTime_GetSeconds(const S_RTC_TIME_DATA_T* localDt)
{
	struct tm dt = {0};
	
	if(Null == localDt)
	{
		S_RTC_TIME_DATA_T g_dt = {0};
		RTC_GetDateAndTime(&g_dt);
		localDt = &g_dt;
	}
	
	DateTime_ToTm(localDt, &dt);
	
    //将时间转换为自1970年1月1日以来持续时间的秒数，发生错误时返回-1。
	return mktime(&dt);
}

//根据当前的time秒数，转换为tm时间格式，再转换为S_RTC_TIME_DATA_T时间格式
void DateTime_SecondsToRtc(time_t time, S_RTC_TIME_DATA_T* rtcTime)
{
	struct tm* dt = localtime(&time);
	DateTime_FromTm(dt, rtcTime);
}

//将S_RTC_TIME_DATA_T的时间转换为tm的格式
void DateTime_ToTm(const S_RTC_TIME_DATA_T* localDt, struct tm* dt)
{
	dt->tm_year = localDt->u32Year - 1900;
	dt->tm_mon  = localDt->u32Month - 1;
	dt->tm_mday  = localDt->u32Day;
	
	dt->tm_hour = localDt->u32Hour;
	dt->tm_min  = localDt->u32Minute;
	dt->tm_sec  = localDt->u32Second;
}

//将tm的时间格式转换为S_RTC_TIME_DATA_T的时间格式
void DateTime_FromTm(const struct tm* dt, S_RTC_TIME_DATA_T* localDt)
{
	localDt->u32Year   = dt->tm_year + 1900;
	localDt->u32Month  = dt->tm_mon + 1;
	localDt->u32Day    = dt->tm_mday ;
	
	localDt->u32Hour   = dt->tm_hour;
	localDt->u32Minute = dt->tm_min ;
	localDt->u32Second = dt->tm_sec ;
}

//修正时间，给S_RTC_TIME_DATA_T时间添加second秒
void DateTime_Add(S_RTC_TIME_DATA_T* localDt, uint32 second)
{
	struct tm dt;
	struct tm* pDt;
	
	DateTime_ToTm(localDt, &dt);
	
	time_t time1 = mktime(&dt) + second;
	pDt = localtime(&time1);
	
	DateTime_FromTm(pDt, localDt);
	
//	DateTime_dump(localDt);
}

//两个时间相减，返回秒数
int DateTime_Sub(const S_RTC_TIME_DATA_T* srcDt, const S_RTC_TIME_DATA_T* dstDt)
{
	struct tm dt;
	
	if(Null == srcDt)
	{
		S_RTC_TIME_DATA_T g_dt = {0};
		RTC_GetDateAndTime(&g_dt);
		srcDt = &g_dt;
	}
	
	DateTime_ToTm(srcDt, &dt);
	time_t time1 = mktime(&dt);
	
	DateTime_ToTm(dstDt, &dt);
	time_t time2 = mktime(&dt);
		
	return time1 - time2;
}

//将秒数转换为S_RTC_TIME_DATA_T时间格式并打印出来
char* DateTime_ToStr(uint32 seconds)
{
	static char buf[64] = {0};

	S_RTC_TIME_DATA_T sCurTime;
	struct tm* pDt = localtime(&seconds);
	DateTime_FromTm(pDt, &sCurTime);

	return buf;
}

//增加秒数测试函数
void DateTime_Add_Test()
{

	S_RTC_TIME_DATA_T dataTime[] = 
	{
		 {2016, 11, 10, 0, 17, 30, 50, RTC_CLOCK_24}
		,{2016, 11, 10, 0, 23, 50, 50, RTC_CLOCK_24}
		,{2016, 11, 30, 0, 23, 50, 50, RTC_CLOCK_24}
		,{2016, 12, 31, 0, 23, 59, 50, RTC_CLOCK_24}
		,{2016, 02, 28, 0, 23, 59, 50, RTC_CLOCK_24}
		,{2016, 02, 29, 0, 23, 59, 50, RTC_CLOCK_24}
	};
//	S_RTC_TIME_DATA_T  dstDt;
	DateTime_dump(&dataTime[0]);

	DateTime_Add(&dataTime[0], 10);
	DateTime_Add(&dataTime[0], 60);
	DateTime_Add(&dataTime[0], 3600);
	DateTime_Add(&dataTime[0], 3600*24);

	
//	Printf("DateTime_Add test passed.\n");
}

//同步时间到本地
void LocalTimeSync(S_RTC_TIME_DATA_T* time )
{
	/* Time Setting */
	//Printf("DataTime sync:");
	//DateTime_dump(&time);
		
	sInitTime.u32Year       = time->u32Year;
	sInitTime.u32Month      = time->u32Month;
	sInitTime.u32Day        = time->u32Day;
	sInitTime.u32Hour       = time->u32Hour;
	sInitTime.u32Minute     = time->u32Minute;
	sInitTime.u32Second     = time->u32Second;

	RTC_SetDateAndTime(&sInitTime);	
}

/*****************************************end of DateTime.c*****************************************/
