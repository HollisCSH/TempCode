/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifdef CONFIG_CMDLINE

#ifndef  _CMDLINE_EXPORT_DEF_H_
#define  _CMDLINE_EXPORT_DEF_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "CmdLine.h"

#define SHELL_USED __attribute__((used))
#define SECTION(x) __attribute__((section(x)))

typedef struct _CmdLineExCfg
{
	char* cmdLineBuf;			//命令长度Buffer
	OutPutFun printf;
}CmdLineExCfg;

#define EXPORT_SHELL_FUNC(fun, cmdline)                      \
	const char __shell_##fun##_name[] SECTION(".rodata.name") = #cmdline;    \
	static CmdItemEx __shell##fun##Ex;	\
	SHELL_USED const CmdItem __shell_##fun SECTION("ShellFun")= \
	{                           \
		&__shell##fun##Ex,	\
		__shell_##fun##_name,    \
		fun,    \
	};

	//Sample:EXPORT_SHELL_FUNC(test, test(uint8 p))

#define EXPORT_SHELL_VAR(name, type, len, desc, fmt)              \
	const char __vsym_##name##_name[] SECTION(".rodata.name") = #name;          \
	const char __vsym_##name##_desc[] SECTION(".rodata.name") = #desc;          \
	SHELL_USED const struct CmdLineVar __vsym_##name SECTION("ShellVar")=       \
	{                           \
		__vsym_##name##_name,   \
		__vsym_##name##_desc,   \
		type,                   \
		len,                    \
		(void*)&name,            \
		fmt,	\
	};

#define EXPORT_SHELL_VAR_INT8(name)			EXPORT_SHELL_VAR(name, DT_INT8	, 1, Null, Null)
#define EXPORT_SHELL_VAR_UINT8(name)		EXPORT_SHELL_VAR(name, DT_UINT8 , 1, Null, Null)
#define EXPORT_SHELL_VAR_INT16(name)		EXPORT_SHELL_VAR(name, DT_UINT16, 2, Null, Null)
#define EXPORT_SHELL_VAR_UINT16(name)		EXPORT_SHELL_VAR(name, DT_UINT16, 2, Null, Null)
#define EXPORT_SHELL_VAR_INT32(name)		EXPORT_SHELL_VAR(name, DT_INT32 , 4, Null, Null)
#define EXPORT_SHELL_VAR_UINT32(name)		EXPORT_SHELL_VAR(name, DT_UINT32, 4, Null, Null)
#define EXPORT_SHELL_VAR_BYTES(name, len)   EXPORT_SHELL_VAR(name, DT_BYTES , len, Null, Null)

void Shell_init();
void Shell_rxCmd(const char* str);

#ifdef __cplusplus
}
#endif

#endif 

#endif
