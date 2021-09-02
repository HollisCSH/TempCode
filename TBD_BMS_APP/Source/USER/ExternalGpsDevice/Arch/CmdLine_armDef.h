/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifdef CONFIG_CMDLINE

#ifndef  _CMDLINE_H_
#define  _CMDLINE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "ArchDef.h"
#include "CmdLineDef.h"

/*
//Typedef ============================================
#ifdef _WINDOWS
#include <windows.h>
#endif

#include <stdio.h>
#include <String.h>

typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef int int32;

#ifdef Bool
#undef Bool
#endif
#define Bool uint8

#define True 	1
#define False 	0
#define Null	0
*/

////////////////////////////////////////////////////////
#define	KEY_CR		'\n'
#define	KEY_LF		'\r'
#define	KEY_BACK	0x08
#define KEY_BEEP	0x07
#define KEY_UP		0x41
#define KEY_DOWN	0x42
#define KEY_RIGHT	0x43
#define KEY_LEFT	0x44
#define KEY_HOME	0x48
#define KEY_END		0x4b
#define	KEY_ESC		0x1b
#define	KEY_EOF		0x1a
#define KEY_CANCEL  0x03
#define KEY_EOT     0x04

typedef void (*FnArg0)();
typedef void (*FnArg01)(uint32);
typedef void (*FnArg02)(uint32,uint32);
typedef void (*FnArg03)(uint32,uint32,uint32);
typedef void (*FnArg04)(uint32,uint32,uint32,uint32);
typedef void (*FnArg05)(uint32,uint32,uint32,uint32,uint32);
typedef void (*FnArg06)(uint32,uint32,uint32,uint32,uint32,uint32);
typedef void (*FnArg07)(uint32,uint32,uint32,uint32,uint32,uint32,uint32);
typedef void (*FnArg08)(uint32,uint32,uint32,uint32,uint32,uint32,uint32,uint32);
typedef void (*FnArg09)(uint32,uint32,uint32,uint32,uint32,uint32,uint32,uint32,uint32);

typedef enum _ArgType
{
	 ARGT_NONE = 0
	,ARGT_DEC 
	,ARGT_HEX
	,ARGT_STR
	,ARGT_ERROR
}ArgType;


#define MAX_ARG_COUNT  5
typedef struct _CmdFnEx
{
	//参数总数
	uint8 argCount;
}CmdItemEx;

typedef struct _FnDef
{
	void* 		handler;	//命令处理函数
	const char* name;		//命令行名称和参数，例如：Test(uint32 val)
	CmdItemEx*	ex;			//命令行扩展
}CmdItem;

typedef void (*OutPutFun)(const char* string);
#define MAX_CMDLINE_LEN 64

typedef struct CmdLine
{
	unsigned char isEcho:1;	//是否输入回显
	unsigned char m_Reserved:7;

	const char* cmdLineBuf;			//命令行Buffer
	int   cmdLineBufSize;		//命令行Buffer长度
	unsigned char cmdLineLen;	//命令行字符串长度

	const CmdItem* cmdItems;	//命令处理函数数组
	unsigned char cmdItemCount;	//命令处理函数数组长度

	OutPutFun printf;
}CmdLine;

extern CmdLine g_CmdLine;

void CmdLine_Init(CmdLine* cmdLine, const char* cmdLineBuf, int cmdLineBufSize, Bool isEcho, const CmdItem* CmdItems, int count, OutPutFun printf);
int CmdLine_GetArgCount(const char* str);
void CmdLine_AddStrEx(CmdLine* cmdLine, const char* str, int len);
void CmdLine_AddStr(CmdLine* cmdLine, const char* str);

#ifdef __cplusplus
}
#endif

#endif 

#endif
