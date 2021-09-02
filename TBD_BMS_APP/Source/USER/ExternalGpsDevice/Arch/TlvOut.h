/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __TLV_OUT_H_
#define __TLV_OUT_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "BSPTypeDef.h"
#include "Tlv.h"

typedef enum _TlvOutFlag
{
	TF_OUT_EMPTY = 0,
	TF_OUT_CHANGED = BIT_0,		//强制改变，触发一次发送动作
}TlvOutFlag;

typedef struct _TlvOutEx
{
	uint32 ticks;		//上次改变的时间Ticks
	uint8  timeOutSec;	//超时计数器, 单位：1S
	TlvOutFlag  flag;		//TF_OUT_CHANGED
}TlvOutEx;

typedef enum _TlvEvent
{
	TE_CHANGED,			//storage发生改变（和mirror比较）
	TE_UNCHANGED,		//storage没发生改变（和mirror比较）
}TlvEvent;

struct _TlvOut;
typedef Bool(*TlvIsChangedFn)(void* pObj, const struct _TlvOut* pItem, TlvEvent ev);
typedef struct _TlvOut
{
	const char* name;	//Name of TLV
	TlvOutEx* pEx;	//extern of TLV

	uint32 tag;			//Tag of TLV
	uint32 len;			//Len of TLV
	void* storage;		//storage data of TLV
	DType dt;
	void* mirror;		//mirror of pStorage, user for compare with storage to confirm if storage is changes.
	TlvIsChangedFn IsChanged;	//function of changed.

	/******************************************
	如果一个TAG,对应多个对象的value，则在value中必然包含对象的ID信息。通过TAG+ID找到相应的唯一TLV项。
	******************************************/
	uint8 idInd;	//id index in storage, ID信息在storage中的起始位置
	uint8 idLen;	//ID信息的长度,如果为0，表示TAG唯一对应一个storage
}TlvOut;


//Tlv 管理器
typedef struct _TlvOutMgr
{
	const TlvOut* itemArray;
	int itemCount;

	uint8 tagLen;

	//来自初始化函数
	//如果为True，则在接收和发送TLV时，对于Value的DT为16位整数和32位整数的类型自动进行大小端转换.
	Bool isSwap;	
}TlvOutMgr;

void TlvOut_dump(const TlvOut* pItem, int tagLen, DType dt);

void TlvOutMgr_init(TlvOutMgr* mgr, const TlvOut* items, int itemCount, int tagLen, Bool isSwap);
void TlvOutMgr_updateMirror(TlvOutMgr* mgr, const uint8* pTlvBuf, int bufSize);

/******************************************
函数功能：获取发生变化的TLV，
函数参数：
	mgr：Tlv管理对象指针。
	pBuf：输出缓冲区。
	bufSize：输出缓冲区大小。
	tlvCount：输入输出参数，
		作为输入参数：> 0,表示获取指定数量的TLV，=0表示不指定TLV的个数。
		作为输出参数：获取到的TLV的个数。
返回值：总的有效的TLV数据长度
******************************************/
int TlvOutMgr_getChanged(TlvOutMgr* mgr, uint8* pBuf, int bufSize, uint8* tlvCount);
int TlvOutMgr_getValByTag(TlvOutMgr* mgr,uint8* pBuf ,int bufSize ,uint32 tag);

void TlvOutMgr_resetAll(TlvOutMgr* mgr);
void TlvOutMgr_setFlag(TlvOutMgr* mgr, uint32 tag, TlvOutFlag flag);
const TlvOut* TlvOutMgr_find(const TlvOut* pItems, int count, uint32 tag);

#ifdef __cplusplus
}
#endif

#endif

