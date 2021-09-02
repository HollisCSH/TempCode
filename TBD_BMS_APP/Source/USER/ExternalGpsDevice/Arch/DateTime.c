
#include "ArchDef.h"
#include "DateTime.h"

extern void RTC_GetDateAndTime(S_RTC_TIME_DATA_T *sPt);

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

time_t DateTime_GetCurrentSeconds()
{
	extern uint32_t rtc_counter_get(void);
	return rtc_counter_get();
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
	
	return mktime(&dt);
}

void DateTime_SecondsToRtc(time_t time, S_RTC_TIME_DATA_T* rtcTime)
{
	struct tm* dt = localtime(&time);
	DateTime_FromTm(dt, rtcTime);
}

void DateTime_ToTm(const S_RTC_TIME_DATA_T* localDt, struct tm* dt)
{
	dt->tm_year = localDt->u32Year - 1900;
	dt->tm_mon  = localDt->u32Month - 1;
	dt->tm_mday  = localDt->u32Day;
	
	dt->tm_hour = localDt->u32Hour;
	dt->tm_min  = localDt->u32Minute;
	dt->tm_sec  = localDt->u32Second;
}

void DateTime_FromTm(const struct tm* dt, S_RTC_TIME_DATA_T* localDt)
{
	localDt->u32Year   = dt->tm_year + 1900;
	localDt->u32Month  = dt->tm_mon + 1;
	localDt->u32Day    = dt->tm_mday ;
	
	localDt->u32Hour   = dt->tm_hour;
	localDt->u32Minute = dt->tm_min ;
	localDt->u32Second = dt->tm_sec ;
}

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
		
	return (int)(time1 - time2);
}

void DateTime_dump(S_RTC_TIME_DATA_T* dt)
{
	S_RTC_TIME_DATA_T sCurTime;
	if(dt == Null)
	{
	    /* Get the current time */
	    RTC_GetDateAndTime(&sCurTime);
		dt = &sCurTime;
	}
    Printf("DataTime:%d/%02d/%02d %02d:%02d:%02d\n",dt->u32Year,dt->u32Month,
           dt->u32Day,dt->u32Hour,dt->u32Minute,dt->u32Second);
}

char* DateTime_ToStr(uint32 seconds)
{
	static char buf[64] = {0};

	S_RTC_TIME_DATA_T sCurTime;
	struct tm* pDt = localtime((time_t*)&seconds);
	DateTime_FromTm(pDt, &sCurTime);
	
    sprintf(buf, "%d/%02d/%02d %02d:%02d:%02d",sCurTime.u32Year,sCurTime.u32Month,
           sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);

	return buf;
}

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

	
	Printf("DateTime_Add test passed.\n");
}

