#ifndef __DATE_TIME__H_
#define __DATE_TIME__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "time.h"

#define RTC_CLOCK_24         1                /*!< RTC 24 Hour */
typedef struct {
	uint32 u32Year;          /*!<  Year value */
	uint32 u32Month;         /*!<  Month value */
	uint32 u32Day;           /*!<  Day value */
	uint32 u32DayOfWeek;     /*!<  Day of week value */
	uint32 u32Hour;          /*!<  Hour value */
	uint32 u32Minute;        /*!<  Minute value */
	uint32 u32Second;        /*!<  Second value */
	uint32 u32TimeScale;     /*!<  12-Hour, 24-Hour */
	uint32 u32AmPm;          /*!<  Only Time Scale select 12-hr used */
} S_RTC_TIME_DATA_T;

char* DateTime_ToStr(uint32 seconds);
void DateTime_ToTm(const S_RTC_TIME_DATA_T* localDt, struct tm* dt);
void DateTime_FromTm(const struct tm* dt, S_RTC_TIME_DATA_T* localDt);

void DateTime_Add(S_RTC_TIME_DATA_T* dt, uint32 second);
S_RTC_TIME_DATA_T* DateTime_MakeRtc(int year, int month, int day, int hour, int minute, int sec);

//两个时间相减，返回秒数 2016 12 12 
int DateTime_Sub(const S_RTC_TIME_DATA_T* srcDt, const S_RTC_TIME_DATA_T* dstDt);
time_t DateTime_GetSeconds(const S_RTC_TIME_DATA_T* localDt);
void DateTime_SecondsToRtc(time_t time, S_RTC_TIME_DATA_T* rtcTime);

#ifdef __cplusplus
}
#endif

#endif


