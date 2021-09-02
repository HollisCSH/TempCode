//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: MBFindMap.h
//创建人  	: Handry
//创建日期	: 
//描述	    : Modbus从机地址映射处理头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

#ifndef _MB_FINDMAP_H
#define _MB_FINDMAP_H

//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"
#include "BspSystemCfg.h"
#include "ParaCfg.h"

//=============================================================================================
//宏定义
//=============================================================================================

#define 	MB_DEVICE_INFO_START			(0)			//BMS设备信息起始地址
#define 	MB_READONLY_DATA_START 			(256)		//BMS只读数据起始地址
#define 	MB_RUN_CTRL_START 				(512)		//BMS运行控制起始地址
#define 	MB_PARA_CFG_START 				(768)		//BMS参数配置起始地址
#define 	MB_USER_DATA_DATA_START 		(4096)		//用户数据区起始地址

#define 	MB_DEVICE_INFO_NUM 				(52)        //BMS设备信息长度
#define 	MB_READONLY_DATA_NUM 			(106)       //BMS只读数据长度

#if defined(USE_B20_TOPBAND_PCBA) || defined(USE_B22_IM_PCBA)
#define 	MB_RUN_CTRL_NUM 				(44)		//BMS运行控制长度
#elif defined(USE_B21_IM_PCBA)
#define 	MB_RUN_CTRL_NUM 				(43)		//BMS运行控制长度
#endif

#define 	MB_PARA_CFG_NUM 				(98)//(71)		//BMS参数配置长度
#define 	MB_USER_DATA_NUM 				(21)//(5)		//用户数据区长度

#define 	MB_DEVICE_INFO_END 				(MB_DEVICE_INFO_START + MB_DEVICE_INFO_NUM-1)		//BMS设备信息结束地址
#define 	MB_READONLY_DATA_END 			(MB_READONLY_DATA_START + MB_READONLY_DATA_NUM-1)	//BMS只读数据结束地址
#define 	MB_RUN_CTRL_END 				(MB_RUN_CTRL_START + MB_RUN_CTRL_NUM-1)				//BMS运行控制结束地址
#define 	MB_PARA_CFG_END 				(MB_PARA_CFG_START + MB_PARA_CFG_NUM-1)				//BMS参数配置结束地址
#define 	MB_USER_DATA_END 				(MB_USER_DATA_DATA_START + MB_USER_DATA_NUM-1)		//用户数据区结束地址

#define 	MB_AREA_READ_NUM 				5			//读数据，数组长度
#define 	MB_AREA_WRITE_NUM 				3			//写数据，数组长度

typedef u8 (*t_MBREG)(u16 mbaddr,u16 *data);			//定义函数指针，输入绝对地址号，返回

//=============================================================================================
//声明外部变量
//=============================================================================================
//读接口外部声明
extern const u16 cMbRegReadStart[MB_AREA_READ_NUM];					//读寄存器起始地址数组
extern const u16 cMbRegReadEnd[MB_AREA_READ_NUM];					//读寄存器结束地址数组

extern t_MBREG const cMBDevInfoRdFunc[MB_DEVICE_INFO_NUM];			//BMS设备信息读函数集合
extern t_MBREG const cMBReadonlyDataRdFunc[MB_READONLY_DATA_NUM];	//BMS只读数据读函数集合
extern t_MBREG const cMBRunCTRLRdFunc[MB_RUN_CTRL_NUM];				//BMS运行控制读函数集合
extern t_MBREG const cMBParaCfgRdFunc[MB_PARA_CFG_NUM];				//BMS参数配置读函数集合
extern t_MBREG const cMBUserDataRdFunc[MB_USER_DATA_NUM];			//用户数据区读函数集合
extern const t_MBREG * const cMBReadRegFunc[MB_AREA_READ_NUM];		//读寄存器函数集合

//写接口外部声明
extern const u16 cMbRegWriteStart[MB_AREA_WRITE_NUM];				//写寄存器起始地址数组
extern const u16 cMbRegWriteEnd[MB_AREA_WRITE_NUM];					//写寄存器结束地址数组

extern t_MBREG const cMBRunCTRLWrFunc[MB_RUN_CTRL_NUM];				//BMS运行控制写函数集合
extern t_MBREG const cMBParaCfgWrFunc[MB_PARA_CFG_NUM];				//BMS参数配置写函数集合
extern t_MBREG const cMBUserDataWrFunc[MB_USER_DATA_NUM];			//用户数据区写函数集合
extern const t_MBREG * const cMBWriteRegFunc[MB_AREA_WRITE_NUM];	//写寄存器函数集合

//=============================================================================================
//声明接口函数
//=============================================================================================
extern u8 mb_ctrl_ctrl_w(uint16_t mbaddr,uint16_t *data);
extern u8 mb_ctrl_rec_w(uint16_t mbaddr,uint16_t *data);
extern u8 mb_ctrl_balctl_w(uint16_t mbaddr,uint16_t *data);
extern u8 mb_ctrl_balctl_17_20_w(uint16_t mbaddr,uint16_t *data);

#endif

/*****************************************end of MBFindMap.h*****************************************/
