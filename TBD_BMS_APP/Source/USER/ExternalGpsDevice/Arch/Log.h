/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __LOG_H_
#define __LOG_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "Record.h"

#define	LOG_STR_MAX_SIZE	200
#define LOG_INVALID_IND		0xFFFFFFFF

#define	ET_CATID_UNDEFIND 0	//没定义的CAT ID
#define	ET_SUBID_UNDEFIND 0	//没定义的SUB ID
/************************************************
以下代码应放在LogUser模块中
#define LOG_TRACE1(moduleId, catId, subId, eventId, __val) 			{ uint32 _val = (__val)					 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_TRACE , eventId, _val);}
#define LOG_TRACE2(moduleId, catId, subId, eventId, b1, b0) 		{ uint32 _val = AS_UINT16(b1, b0)		 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_TRACE , eventId, _val);}
#define LOG_TRACE3(moduleId, catId, subId, eventId, asU16H, asU16L) { uint32 _val = (asU16H << 16) | asU16L	 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_TRACE , eventId, _val);}
#define LOG_TRACE4(moduleId, catId, subId, eventId, b3, b2, b1, b0) { uint32 _val = AS_UINT32(b3, b2, b1, b0); Log_Write(g_plogMgr, moduleId, catId, subId, LT_TRACE , eventId, _val);}

#define LOG_WARNING1(moduleId, catId, subId, eventId, __val) 		{ uint32 _val = (__val)					    ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_WARNING , eventId, _val);}
#define LOG_WARNING2(moduleId, catId, subId, eventId, b1, b0) 		{ uint32 _val = AS_UINT16(b1, b0)		    ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_WARNING , eventId, _val);}
#define LOG_WARNING3(moduleId, catId, subId, eventId, asU16H, asU16L) { uint32 _val = (asU16H << 16) | asU16L   ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_WARNING , eventId, _val);}
#define LOG_WARNING4(moduleId, catId, subId, eventId, b3, b2, b1, b0) { uint32 _val = AS_UINT32(b3, b2, b1, b0) ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_WARNING , eventId, _val);}

#define LOG_ERROR1(moduleId, catId, subId, eventId, __val) 			{ uint32 _val = (__val)					 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_ERROR , eventId, _val);}
#define LOG_ERROR2(moduleId, catId, subId, eventId, b1, b0) 		{ uint32 _val = AS_UINT16(b1, b0)		 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_ERROR , eventId, _val);}
#define LOG_ERROR3(moduleId, catId, subId, eventId, asU16H, asU16L)	{ uint32 _val = (asU16H << 16) | asU16L	 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_ERROR , eventId, _val);}
#define LOG_ERROR4(moduleId, catId, subId, eventId, b3, b2, b1, b0)	{ uint32 _val = AS_UINT32(b3, b2, b1, b0); Log_Write(g_plogMgr, moduleId, catId, subId, LT_ERROR , eventId, _val);}
***********************************************/
typedef Bool(*LogItemFilterFn)(void* pItem, uint32 pParam);

typedef enum _LogType
{
	  LT_TRACE
	, LT_WARNING
	, LT_ERROR
}LogType;

//typedef const char* (*LogFmtFn)(const char* str);

//日志事件结构定义
typedef struct _LogEvt
{
	//uint8	moudleId;	//
	uint8	catId;				//类别ID
	uint8	subId;				//子类别ID
	uint8	eventId;			//事件ID
	const char* pEventIdStr;	//事件ID描述，打印输出时取代eventId

/************************************************************
	pFmt：长度必须<=LOG_STR_MAX_SIZE，支持以下格式：
		"%1B"		//输出val.BIT[0], 数值显示为10进制
		"%1BX"		//输出和上面一样
		"%0-1B"		//输出val.BIT[0-1], 数值显示为10进制
		"%2-6BX"	//输出val.BIT[2-6], 数值显示为16进制
		"%7-10B{1:Title1|2:Title2|...}"//输出val.BIT[7-10]，数值使用大括号内的字符串代替,大括号的字符串长度不能超过128个字节。
		"%7-10BX{1:Title1|2:Title2|...}"//输出和上面一样。
************************************************************/
	const char* pEventValueStrFmt;	
}LogEvt;

struct _LogItem;
typedef const char* (*LogToStrFn)(const struct _LogItem* pItem, char* buf, int size);
typedef struct _LogModule
{
	uint8		moduleId;	//模块ID
	const char* name;			//模块名称

	const LogEvt* logEvtArray;	//日志时间数组
	int			logEvtArrayCount;	//日志事件数组元素总数
}LogModule;

#pragma anon_unions
#pragma pack(1) 
//该结构的字节收必须偶数，否则会导致存储性能下降
typedef struct _LogItem
{	
	uint32 dateTime;	//日期时间，从1971年1月1日开始到现在的秒数

	uint8 version : 1;	//版本号
	uint8 logType : 2;	//参考 LogType
	uint8 reserved : 2;
	uint8 catId : 3;	//类别ID，根据subId的种类归类

	uint8 moduleId;		//模块ID，记录触发的log的模块ID
	uint8 subId;		//模块子ID

	uint8 eventId;	//事件ID

	union
	{
		uint8 data[4];	//事件参数
		uint32 asUint32;
	};
}LogItem;
#pragma pack() 

typedef enum _LOG_EVENT
{
	LOG_WRITE_BEFORE,	//before of writing log
	LOG_WRITE_AFTER,	//before of writing log
	LOG_DUMP,			//dump log record
}LOG_EVENT;

struct _LogMgr;
typedef Bool(*LogEventFun)(struct _LogMgr* p, const LogItem * pItem, LOG_EVENT ev);
typedef uint32(*GetCurrentSecondsFn)();
typedef Bool(*LogFilterFun)(struct _LogMgr* p, const LogItem * pItem);

//Log配置结构，用于初始化log模块的存储，和打印
typedef struct LogCfg
{
	//LOG的版本号，用来校验和存储的版本号是否一致
	uint8 logVersion;

	//Log模块的数组
	const LogModule* moduleArray;
	int moduleCount;

	//Log的事件函数，可以为Null
	LogEventFun Event;

	//获取RTC的秒数，可以为Null
	GetCurrentSecondsFn GetCurSec;
}LogCfg;

typedef struct _LogMgr
{
	const LogCfg* cfg;
	const RecordCfg* recordCfg;
	Record record;

	//事件值的Buffer
	char eventValueBuf[LOG_STR_MAX_SIZE+16];
	//事件值的Buffer长度
	int eventValueBufSize;

	char catIdBuf[16];
	int catIdBufSize;

	char subIdBuf[16];
	int subIdBufSize;

	char eventIdBuf[16];
	int eventIdBufSize;
}LogMgr;
extern LogMgr* g_plogMgr;

int SprintfBit(char buf[], const char* str, uint32 val, uint8* numberOfBit);

void Log_Init(LogMgr * logMgr, const LogCfg * cfg, const RecordCfg * recordCfg);
void Log_Write(LogMgr * logMgr, uint8 moduleId, uint8 catId, uint8 subId, LogType logType, uint8 eventId, uint32 val);

void Log_DumpByMask(LogMgr * logMgr, const LogItem * dst, const LogItem * mask, int count);
void Log_DumpByCount(LogMgr * logMgr, int count);
void Log_DumpByInd(LogMgr * logMgr, int ind, int count);

void Log_Tester();
void Log_Dump(LogMgr* logMgr, LogItem* pItem, const char* head, const char* tail);


#ifdef __cplusplus
}
#endif

#endif

