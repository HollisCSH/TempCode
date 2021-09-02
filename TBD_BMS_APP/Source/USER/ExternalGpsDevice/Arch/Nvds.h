/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __NVDS_H_
#define __NVDS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "BSPTypeDef.h"
#include "EppromMgr.h"

#define EEPROM_FIRST_BYTE	0x55
#define EEPROM_LATEST_BYTE	0xAA

	/*Nvds事件定义*/
	typedef enum
	{
		BE_DATA_ERROR,	//存储区数据检验失败，数据无效
		BE_DATA_OK,		//存储区数据检验通过，数据有效
		BE_ON_WRITE_BEFORE,	//写数据操作之前, 如果该事件返回False，则不执行写操作
	}NvdsEventID;

	//事件函数定义
	typedef Bool (*NvdsEventFn)(void* pData, NvdsEventID eventId);

	//Nvds数据项定义
	typedef struct _NvdsItem
	{
		//NvdsID
		uint8 id;
		//NvdsID存储的扇区配置
		SectorCfg secCfg;
		//Nnds扇区管理对象指针
		SectorMgr* sectorMgr;

		//NvdsID存储的事件函数定义
		NvdsEventFn Event;

		//secCfg.storage的镜像对象，用于和原值作比较，决定是否写入FLASH
		//如果mirror=Null，表示写入Flash时不和原值比较
		void* mirror;
	}NvdsItem;

	typedef struct _Nvds
	{
		const NvdsItem* nvdsItemArray;
		int nvdsItemArrayCount;
	}Nvds;

	//Nvds初始化函数
	void Nvds_Init(const NvdsItem* nvdsItemArray, int count);
	void Nvds_write(uint8_t nvdsId);

#ifdef __cplusplus
}
#endif

#endif

