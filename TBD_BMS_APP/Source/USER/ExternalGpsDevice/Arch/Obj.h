/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */
#ifndef __OBJ_H_
#define __OBJ_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "BSPTypeDef.h"

	struct _Obj;
	typedef void (*ObjFn)();
	typedef struct _Obj
	{
		const char* name;
		ObjFn Start;
		ObjFn Stop;
		ObjFn Run;
	}Obj;

	void ObjList_add(const Obj* pObj);

	void ObjList_start(void);
	void ObjList_stop(void);
	void ObjList_run(void);

#ifdef __cplusplus
}
#endif

#endif

