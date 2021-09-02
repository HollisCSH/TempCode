//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名		: Version.h
//创建人		: Handry
//创建日期	:
//描述	    : 版本信息头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人		:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef __VERSION_H
#define __VERSION_H

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTypeDef.h"

//=============================================================================
//数据结构声明
//=============================================================================
//APP 版本信息结构体
typedef struct
{
    u32 appcrc;			//APP段的crc
    u32 length;			//APP占内存大小
    u8  swmain;			//软件主版本号
    u8  swsub1;			//软件主修订版本号
    u8  swsub2;			//软件子修订版本号
    u8  swbuild[4];		//编译次数，低位在前
    u8  hwmain;			//硬件主版本号
    u8  hwsub;			//硬件子版本号
    u8  reserve[107];	//保留
    u32 info_crc;
}t_VERSION_INFO;

//typedef enum _BOOT_CONFIG_ADDR
//{
//    e_BOOT_STAY_FLAG_ADDR = 2,              //转发模志存放地址
//    e_BOOT_PRO_FLAG_ADDR = 4,               //应用程序标志
//    e_BOOT_PRO_TRY_FLAG_ADDR = 6,           //尝试运行标志
//}e_BOOT_CONFIG_ADDR;
typedef enum _BOOT_CONFIG_ADDR		//用户自定义内存区	0x08007F00-0x08007F7F	共 2* 1024个字节
{
    e_BOOT_STAY_FLAG_ADDR 		= 0x08007800,           //转发模志存放地址
    e_BOOT_PRO_FLAG_ADDR 		= 0x08007808,           //应用程序标志
    e_BOOT_PRO_TRY_FLAG_ADDR 	= 0x08007810,           //尝试运行标志
}e_BOOT_CONFIG_ADDR;

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern t_VERSION_INFO  cAPPInfo;
//=============================================================================================
//宏定义
//=============================================================================================
//#define		LFP_TB_20000MAH_20S	 	1
#define		SY_PB_32000MAH_14S	 	1

/***********************************电芯参数***********************************/
//12AH国产电芯凯德 三元电池
#ifdef 		BMS_BAT_12000MAH_CH
#define 	BAT_TYPE 			 	2							//电池类型

//15AH国产电芯凯德 三元电池
#elif 		defined(BMS_BAT_15000MAH_CH)
#define 	BAT_TYPE 				3							//电池类型

//12AH国产电芯创明 三元电池
#elif 		defined(BMS_BAT_12000MAH_CM)
#define 	BAT_TYPE 				4							//电池类型

//15AH国产电芯创明 三元电池
#elif 		defined(BMS_BAT_15000MAH_CM)
#define 	BAT_TYPE 				5							//电池类型

//12.5AH国产电芯EVE 三元电池
#elif 		defined(BMS_BAT_12500MAH_EVE)
#define 	BAT_TYPE 				6							//电池类型

//15AH国产电芯EVE 三元电池
#elif 		defined(BMS_BAT_15000MAH_EVE)
#define 	BAT_TYPE 				7							//电池类型

//13AH国产电芯BAK 三元电池
#elif 		defined(BMS_BAT_13000MAH_BAK)
#define 	BAT_TYPE 				8							//电池类型

//12.5AH韩国电芯三星 三元电池
#elif 		defined(BMS_BAT_12500MAH_SAM)//12.5AH三星电芯
#define 	BAT_TYPE 				9							//电池类型
//12AH国产比亚迪 磷酸铁锂电池
#elif		defined(LFP_AK_20000MAH_16S)
#define 	BAT_TYPE 			 	1							//电池类型 磷酸铁锂电芯，从型号0开始
//20AH国产拓邦 磷酸铁锂电池
#elif		defined(LFP_TB_20000MAH_20S)
#define 	BAT_TYPE                0							//电池类型 磷酸铁锂电芯，从型号0开始
#define     BMS_BAT_20S             1
//32AH国产孚能 三元电池
#elif		defined(SY_PB_32000MAH_14S)
#define 	BAT_TYPE                3							//电池类型 三元电芯
#define     BMS_BAT_14S             1
//10AH韩国电芯MF1 LG 三元电池
#else
#define 	BAT_TYPE 				1							//电池类型
#endif

//电池配置
#if 		defined(BMS_BAT_13S)										//13串电池
#define 	BAT_CELL_NUM         	13  						//电池包单体数目
#define 	BAT_VER_SE_CODE 	 	1 							//硬件版本中串数代码
#elif       defined(BMS_BAT_20S)	
#define 	BAT_CELL_NUM         	20  						//电池包单体数目
#define 	BAT_VER_SE_CODE 	 	5 							//硬件版本中串数代码
#elif       defined(BMS_BAT_16S)										//16串电池
#define 	BAT_CELL_NUM         	16  						//电池包单体数目
#define 	BAT_VER_SE_CODE 	 	0 							//硬件版本中串数代码
#elif       defined(BMS_BAT_14S)
#define 	BAT_CELL_NUM         	14  						//电池包单体数目
#define 	BAT_VER_SE_CODE 	 	2 							//硬件版本中串数代码
#else
#define 	BAT_CELL_NUM         	16  						//电池包单体数目
#define 	BAT_VER_SE_CODE 	 	0 							//硬件版本中串数代码
#endif

//-----------------------------------BOOT和APP标志---------------------------------------------------
//#define     BOOT_STAY_FLAG      (u16)(0x5AA5)    //停留BOOT标志
//#define     BOOT_PRO_FLAG       (u16)(0x5AA5)    //跳转应用程序标志
//#define     BOOT_PRO_TRY_FLAG   (u16)(0x5AA5)    //已经尝试的标志
#define     BOOT_STAY_FLAG      (u64)(0x1234567812345678)    //停留BOOT标志
#define     BOOT_PRO_FLAG       (u64)(0x5AA55AA55AA55AA5)    //跳转应用程序标志
#define     BOOT_PRO_TRY_FLAG   (u64)(0x9988776655443322)    //已经尝试的标志
#define     CLEAR_FLAG   				(u64)(0xFFFFFFFFFFFFFFFF)    //情况标志
//-----------------------------------版本信息---------------------------------------------------
#define 	FW_VER_MAIN 		1								//主版本号
#define 	FW_VER_S1 			0      							//主修订版本号
#define 	FW_VER_S2 			0     							//子修订版本号
#define 	FW_VER_BUILD 		(FW_APP_BUILD_NUM)				//由pre-build命令自动加1更新
#define 	FW_APP_BUILD_NUM 	1								//编译次数，用于判断是否已经重新编译

#define 	HW_VER_MAIN 		((BAT_VER_SE_CODE << 4) + 13)	//硬件主版本号，包含电池串数信息
#define 	HW_VER_SUB 			((BAT_TYPE << 4) + 0)			//硬件子版本号，包含电池型号信息

//定义硬件版本
#define 	HW_MAIN_VER 		HW_VER_MAIN //BMS Product type
#define 	HW_SUB_VER 			HW_VER_SUB//0x10 // V1R0 硬件版本
//定义Bootloader 版本
#define 	BL_MAIN_VER 		1 
#define 	BL_SUB_VER 			2

#define 	FW_PROTOCOL_VER		0x000A 							//协议版本

#endif

/*****************************************end of Version.h*****************************************/
