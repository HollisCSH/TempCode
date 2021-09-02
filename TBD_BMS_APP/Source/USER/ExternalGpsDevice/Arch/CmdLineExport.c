/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifdef CONFIG_CMDLINE

#include "ArchDef.h"
#include "Obj.h"
#include "CmdLineExport.h"

static const CmdItem* g_shellFn;
static int g_shellFnCount;
static const CmdItem* CmdLineExport_cmdItemInit(int* count)
{
	extern const uint32_t ShellFun$$Base;
	extern const uint32_t ShellFun$$Limit;

	*count = (ShellFun$$Limit - ShellFun$$Base) / sizeof(CmdItem);
	return (CmdItem*)ShellFun$$Base;
}


/*******************************************************************/
static const CmdLineVar* g_shellVar;
static int g_shellVarCount;
static const CmdLineVar* CmdLineExport_varArrayInit(int* count)
{
	extern const uint32_t ShellVar$$Base;
	extern const uint32_t ShellVar$$Limit;

	*count = (ShellVar$$Limit - ShellVar$$Base) / sizeof(CmdLineVar);
	return (CmdLineVar*)ShellVar$$Base;
}

const CmdLineVar* CmdLineExport_find(const char* name)
{
//	int count;
	const CmdLineVar* var = g_shellVar;
	for (int i = 0; i < g_shellVarCount; i++, var++)
	{
		if (strcmp(name, var->name) == 0) return var;
	}

	return Null;
}

void CmdLineExport_PrinfVar(const char* name, const char* fmt)
{
	if (strlen(name) == 0)
	{
		int count;
		const CmdLineVar* var = CmdLineExport_varArrayInit(&count);
		for (int i = 0; i < count; i++, var++)
		{
			Printf("%s:", var->name);
			if (fmt == Null || strlen(fmt) == 0) fmt = var->fmt;
			Dt_printf(var->val, var->valLen, var->type, fmt);
			Printf("/t %s\n", var->desc);
		}
		return;
	}

	const CmdLineVar* var = CmdLineExport_find(name);
	if (var)
	{
		if (fmt == Null || strlen(fmt) == 0) fmt = var->fmt;
		Printf("%s:", var->name);
		Dt_printf(var->val, var->valLen, var->type, fmt);
		Printf("/t %s\n", var->desc);
	}
	else
	{
		Printf("No var[%s]\n", name);
	}
}
EXPORT_SHELL_FUNC(CmdLineExport_PrinfVar, PrinfVar(char* name, char* fmt))

/////////////////////////////////////////////////////////////////////////////////////////////
static CmdLine g_cmdLine;
static char g_cmdLineBuf[128];
static uint8 g_cmdLineLen;

//接收命令
void Shell_rxCmd(const char* str)
{
	if(g_cmdLine.cmdLineLen == 0) g_cmdLineLen = 0;
	if (g_cmdLineLen + strlen(str) >= sizeof(g_cmdLineBuf))
	{
		g_cmdLineLen = 0;
		return;
	}

	memcpy(&g_cmdLineBuf[g_cmdLineLen], str, strlen(str));
}

//执行命令
void Shell_run(const char* str)
{
	if (g_cmdLineLen && g_cmdLineBuf[g_cmdLineLen - 1] == '\n')
	{
		CmdLine_AddStr(&g_cmdLine, str);
	}

}

void Shell_init()
{
	static CmdLineCfg cmdLineCfg;
	static Obj obj = { "Shell", Null, Null, Shell_run };
	ObjList_add(&obj);

	g_shellVar = CmdLineExport_varArrayInit(&g_shellVarCount);
	g_shellFn = CmdLineExport_cmdItemInit(&g_shellFnCount);

	cmdLineCfg.cmdLineBuf = g_cmdLineBuf;
	cmdLineCfg.printf = printf;
	cmdLineCfg.cmdHandlerArray = g_shellFn;
	cmdLineCfg.cmdHandlerCount = g_shellFnCount;

	CmdLine_Init(&g_cmdLine, &cmdLineCfg, False);
}

#endif
