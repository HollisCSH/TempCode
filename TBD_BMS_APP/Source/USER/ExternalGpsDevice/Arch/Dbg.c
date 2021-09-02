/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "Dbg.h"

#ifdef XDEBUG

unsigned int g_dwDebugLevel = 0;

static DebugWriteFn Dbg_writeFn = Null;

//自定义的调试开关位数组，来自初始化函数
const DbgSwitch* g_gDbgSwh = Null;
//自定义的调试开关位数组元素总数，，来自初始化函数
static int g_gDbgSwhCount = 0;

//根据参数值来查找响应的开关位对象
static const DbgSwitch* DbgSwhs_get(uint32_t value)
{
	//系统默认的调试开关位
	static const DbgSwitch g_defDbgSwh[] =
	{
		DBG_LEV_DEF(DL_MAIN),
		DBG_LEV_DEF(DL_ERROR),
		DBG_LEV_DEF(DL_WARNING),
	};

	//现在系统的开关位中寻找
	const DbgSwitch* p = g_defDbgSwh;
	int count = GET_ELEMENT_COUNT(g_defDbgSwh);
	for (int i = 0; i < count; i++, p++)
	{
		if (p->value == value) return p;
	}

	//再到自定义的开关位数组中查找
	p = g_gDbgSwh;
	count = g_gDbgSwhCount;
	for (int i = 0; i < count; i++, p++)
	{
		if (p->value == value) return p;
	}

	return Null;
}

static void Dbg_dumpAllBit()
{
	const DbgSwitch* p;
	uint8_t bit = 0;
	for (int i = 0; i < 32; i++)
	{
		bit = (g_dwDebugLevel & BIT(i)) > 0;
		p = DbgSwhs_get(BIT(i));
		Printf("\tBIT[%02d].%s\t = %d.\n", i, p == Null ? "" : p->name, bit);
	}
}

void Dbg_SetLevel(uint32_t value)
{
	Dbg_dumpAllBit();
	Printf("g_dwDebugLevel = 0x%08x\n", value);
	if(Dbg_writeFn) Dbg_writeFn();
}

void Dbg_SetBit(uint32_t nIndex, Bool isEnable)
{
	if(isEnable)
	{
		g_dwDebugLevel |= (1 << nIndex);
	}
	else
	{
		g_dwDebugLevel &= ~(1 << nIndex);
	}
	Dbg_dumpAllBit();

	Printf("g_dwDebugLevel = 0x%08x\n", g_dwDebugLevel);	
	if (Dbg_writeFn) Dbg_writeFn();
}

/*********************************************************************
函数功能：Debug模块初始化，初始化成功之后，Debug具备保存开关位到Flash和打印开关位功能，
函数参数：
	writeFn：写调试开关位到Flash的函数指针。
	debSwhArray：调试开关位数组。
	count：调试开关位数组元素总数
***********************************************************************/
void Dbg_Init(DebugWriteFn writeFn, const DbgSwitch* debSwhArray, int count)
{
	g_dwDebugLevel = 0;
	Dbg_writeFn = writeFn;

	g_gDbgSwh = debSwhArray;
	g_gDbgSwhCount = count;
}

#endif

