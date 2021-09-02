/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "Obj.h"

const Obj* g_ObjList[20] = { 0 };
void ObjList_add(const Obj* pObj)
{
	Printf("Init %s\n", pObj->name);

	for (int i = 0; i < GET_ELEMENT_COUNT(g_ObjList); i++)
	{
		if (g_ObjList[i] == Null)
		{
			g_ObjList[i] = pObj;
			return;
		}
	}

	Printf("Obj array is full.\n");
}

void ObjList_start()
{
	const Obj* pObj = g_ObjList[0];
	for (int i = 0; i < GET_ELEMENT_COUNT(g_ObjList) && pObj; i++, pObj = g_ObjList[i])
	{
		if (pObj->Start) pObj->Start();
	}
}

void ObjList_stop()
{
	const Obj* pObj = g_ObjList[0];
	for (int i = 0; i < GET_ELEMENT_COUNT(g_ObjList) && pObj; i++, pObj = g_ObjList[i])
	{
		if (pObj->Stop) pObj->Stop();
	}
}

void ObjList_run()
{
	const Obj* pObj = g_ObjList[0];
	for (int i = 0; i < GET_ELEMENT_COUNT(g_ObjList) && pObj; i++, pObj = g_ObjList[i])
	{
		if (pObj->Run) pObj->Run();
	}
}
