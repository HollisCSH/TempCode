/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */
 
 /*********************************************************************
模块功能：
	1）支持定义最多32个调试开关位，程序中可以打开或者关闭指定的调试开关位，
		如果打开，可以看到相应的开关位控制的信息输出。
		如果关闭，则不能看到相应的开关位控制的信息输出。
	2）支持存储开关位变量到Flash中，永久存储。
***********************************************************************/

#ifndef  _DBG_H_
#define  _DBG_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus

#include "BSPTypeDef.h"
#include "Bit.h"

#ifdef XDEBUG

	//默认的调试输出位开关
	#define DL_MAIN 		BIT_0
	#define DL_ERROR		BIT_1
	#define DL_WARNING		BIT_2
	//自定义的调试输出位开关必须从BIT_5开始

	#define DL_DEBUG		0xFFFFFFFF
	extern uint32_t g_dwDebugLevel;

	//调试开关位结构定义，用于打印输出开关位的名称
	typedef struct _DbgSwitch
	{
		uint32_t value;		//开关位置
		const char* name;	//开关位名称
	}DbgSwitch;

	//开关位定义宏
	#define DBG_LEV_DEF(_x) {_x, #_x}

	//写Flash函数指针。
	typedef void (*DebugWriteFn)();

	/*********************************************************************
	函数功能：Debug模块初始化，初始化成功之后，Debug具备保存开关位到Flash和打印开关位功能，
	函数参数：
		writeFn：写调试开关位到Flash的函数指针。
		debSwhArray：调试开关位数组。
		count：调试开关位数组元素总数
	返回值：无
	***********************************************************************/
	void Dbg_Init(DebugWriteFn writeFn, const DbgSwitch* debLevArray, int len);

	/*********************************************************************
	函数功能：打开或者关闭指定的一个调试开关位，
	函数参数：
		bitInd：开关位序号，取值范围：0-31。
		isEnable：开关位使能。
	返回值：无
	***********************************************************************/
	void Dbg_SetBit(uint32_t bitInd, Bool isEnable);

	/*********************************************************************
	函数功能：设置多个调试开关位置，
	函数参数：
		value：开关位值，0-0xFFFFFFFF。
	返回值：无
	***********************************************************************/
	void Dbg_SetLevel(uint32_t value);
	
	#define Assert(parenExpr) if(!(parenExpr))	\
			{                                   \
				Printf( "Assertion Failed! %s,%s,%s,line=%d\n", #parenExpr,__FILE__,__FUNCTION__,_LINE_);	\
				while(1){;}	\
			}
			
	#define Printf(...) {printf(__VA_ARGS__); /*while(RESET == usart_flag_get(CCU_DEBUG_COM, USART_FLAG_TC));*/}
	
	#define PFL(level, ...) {if (g_dwDebugLevel & level) {Printf(__VA_ARGS__);}}
	
	#define PFL_WARNING(...) 	PFL(DL_WARNING, "WARNING: %s(),line=%d: ",_FUNC_, _LINE_); PFL(DL_WARNING, __VA_ARGS__);
	#define PFL_ERROR(...) 	PFL(DL_ERROR, "ERROR: %s(),line=%d:",_FUNC_, _LINE_); PFL(DL_ERROR, __VA_ARGS__);
	//#define PFL_ERROR(parenExpr) 	PFL(DL_ERROR, "ERROR: line=%d:",_LINE_); PFL(DL_ERROR, parenExpr);

	#define PFL_FUN_LINE(level) PFL(level, "%s(),line=%d\n",_FUNC_, _LINE_);
	#define PFL_FILE_FUN_LINE(level) PFL(level, ("PFL Line. %s,%s(),line=%d\n", __FILE__,__FUNCTION__,_LINE_))
	#define PFL_VAR(V1) 		Printf("%s(),line=%d,%s=[0x%x](%d)\n",_FUNC_, _LINE_, #V1, V1, V1)
	#define PFL_FAILED() 		PFL(DL_MAIN|DL_WARNING, ("%s() FAILED,line=%d.\n",_FUNC_, _LINE_))
	#define PFL_FAILED_EXPR(parenExpr) 		PFL(DL_WARNING, ("%s() FAILED,line=%d:",_FUNC_, _LINE_)); PFL(DL_WARNING, parenExpr);
	
	#define WAIT(maxMS, parenExpr) {int ms = 0; while(!(parenExpr) && ms++ < (maxMS)) {SLEEP(1);}}

#else	//#ifdef XDEBUG
	#define Dbg_Init(...)
	#define Dbg_SetBit(...)
	#define Dbg_SetLevel(...)

	#define Printf(...)
	#define Assert(...)
	#define PFL_WARNING(...)
	#define PFL_ERROR(...)
	
	#define PFL(...)
	#define PFL_FUN_LINE(...)
	#define PFL_FILE_FUN_LINE(...)
	#define PFL_FAILED() (void)0
	#define PFL_FAILED_EXPR(...)

	#define PFL_VAR(...)
	#define WAIT(...)
#endif	//#ifdef XDEBUG

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


