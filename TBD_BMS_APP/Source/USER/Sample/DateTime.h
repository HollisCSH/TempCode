//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: DateTime.h
//创建人  	: 
//创建日期	:
//描述	    : Sim模组读写时间RTC头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================
#ifndef __DATE_TIME__H_
#define __DATE_TIME__H_

#ifdef __cplusplus
extern "C"{
#endif

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTypeDef.h"
#include "stdint.h"
#include "time.h"
//#include "Rtc.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#define RTC_YEAR2000         2000            /*!< RTC Reference */
#define RTC_FCR_REFERENCE    32761           /*!< RTC Reference */

#define RTC_CLOCK_12         0               /*!< RTC 12 Hour */
#define RTC_CLOCK_24         1               /*!< RTC 24 Hour */

#define RTC_AM               1               /*!< RTC AM */
#define RTC_PM               2               /*!< RTC PM */

//=============================================================================================
//定义数据类型
//=============================================================================================
/* RTC define Time Data Struct */
typedef struct 
{
    uint32_t u32Year;          /*!<  Year value */
    uint32_t u32Month;         /*!<  Month value */
    uint32_t u32Day;           /*!<  Day value */
    uint32_t u32DayOfWeek;     /*!<  Day of week value */
    uint32_t u32Hour;          /*!<  Hour value */
    uint32_t u32Minute;        /*!<  Minute value */
    uint32_t u32Second;        /*!<  Second value */
    uint32_t u32TimeScale;     /*!<  12-Hour, 24-Hour */
    uint32_t u32AmPm;          /*!<  Only Time Scale select 12-hr used */
} S_RTC_TIME_DATA_T;

//=============================================================================================
//声明接口函数
//=============================================================================================

//打印时间
void DateTime_dump(S_RTC_TIME_DATA_T* dt);

//读取本地时间到S_RTC_TIME_DATA_T格式中
void RTC_GetDateAndTime(S_RTC_TIME_DATA_T *sPt);

//根据S_RTC_TIME_DATA_T时间格式设置时间到RTC
void RTC_SetDateAndTime(S_RTC_TIME_DATA_T *sPt);

//设置S_RTC_TIME_DATA_T格式的时间
S_RTC_TIME_DATA_T* DateTime_MakeRtc(int year, int month, int day, int hour, int minute, int sec);

//获取当前时间的Seconds
time_t DateTime_GetSeconds(const S_RTC_TIME_DATA_T* localDt);

//根据当前的time秒数，转换为tm时间格式，再转换为S_RTC_TIME_DATA_T时间格式
void DateTime_SecondsToRtc(time_t time, S_RTC_TIME_DATA_T* rtcTime);

//将S_RTC_TIME_DATA_T的时间转换为tm的格式
void DateTime_ToTm(const S_RTC_TIME_DATA_T* localDt, struct tm* dt);

//将tm的时间格式转换为S_RTC_TIME_DATA_T的时间格式
void DateTime_FromTm(const struct tm* dt, S_RTC_TIME_DATA_T* localDt);

//修正时间，给S_RTC_TIME_DATA_T时间添加second秒
void DateTime_Add(S_RTC_TIME_DATA_T* localDt, uint32 second);

//两个时间相减，返回秒数
int DateTime_Sub(const S_RTC_TIME_DATA_T* srcDt, const S_RTC_TIME_DATA_T* dstDt);

//将秒数转换为S_RTC_TIME_DATA_T时间格式并打印出来
char* DateTime_ToStr(uint32 seconds);

//增加秒数测试函数
void DateTime_Add_Test(void);

//同步时间到本地
void LocalTimeSync(S_RTC_TIME_DATA_T* time );

#ifdef __cplusplus
}
#endif

#endif

/*****************************************end of DateTime.h*****************************************/
