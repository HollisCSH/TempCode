//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: BatteryInfo.h
//创建人  	: Handry
//创建日期	:
//描述	    : 电池数据信息头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	: Handry
//版本	    : 
//修改日期	: 2020/07/14
//描述	    : 
//1.创明电池满充电压改为4.01V。
//2.增加BMS电流特征值结构体。
//=============================================================================================
#ifndef __BATTERYINFO_H
#define	__BATTERYINFO_H

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTypeDef.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
/***********************************电芯选择***********************************/
//#define		BMS_BAT_12000MAH_CH		1							//12AH国产电芯凯德 三元电池
//#define		BMS_BAT_12000MAH_CM		1							//12AH国产电芯创明 三元电池
//#define		BMS_BAT_15000MAH_CH		1							//15AH国产电芯凯德 三元电池
//#define		BMS_BAT_15000MAH_CM		1							//15AH国产电芯创明 三元电池
//#define		BMS_BAT_12500MAH_EVE	1							//12.5AH国产电芯EVE 三元电池
//#define		BMS_BAT_15000MAH_EVE	1							//15AH国产电芯EVE 三元电池
//#define		BMS_BAT_13000MAH_BAK	1							//13AH国产电芯BAK 三元电池
//#define		BMS_BAT_12500MAH_SAM	1							//12.5AH韩国电芯三星 三元电池
//#define		LFP_HL_25000MAH_16S	1							//12AH国产电芯力旋 磷酸铁锂电池

/***********************************电芯参数***********************************/
//12AH国产电芯凯德 三元电池
#ifdef 		BMS_BAT_12000MAH_CH
#define 	BAT_NORM_CAP 		 	11000						//电池标称容量
#define 	BAT_NORM_VOLT 		 	3700						//电池标称电压
#define 	BAT_TYPE 			 	2							//电池类型
#define 	BAT_CSOP_RES 		 	10							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 		 	200 						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	4100						//最高单体充电电压

//15AH国产电芯凯德 三元电池
#elif 		defined(BMS_BAT_15000MAH_CH)
#define 	BAT_NORM_CAP 			14000						//电池标称容量
#define 	BAT_NORM_VOLT 			3700						//电池标称电压
#define 	BAT_TYPE 				3							//电池类型
#define 	BAT_CSOP_RES 			8							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200   						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	4100						//最高单体充电电压
#define 	BMS_BAT_13S				1						    //13串电池

//12AH国产电芯创明 三元电池
#elif 		defined(BMS_BAT_12000MAH_CM)
#define 	BAT_NORM_CAP 			12000						//电池标称容量
#define 	BAT_NORM_VOLT 			3600						//电池标称电压
#define 	BAT_TYPE 				4							//电池类型
#define 	BAT_CSOP_RES 			10							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200 						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	4010						//最高单体充电电压

//15AH国产电芯创明 三元电池
#elif 		defined(BMS_BAT_15000MAH_CM)
#define 	BAT_NORM_CAP 			15000						//电池标称容量
#define 	BAT_NORM_VOLT 			3600						//电池标称电压
#define 	BAT_TYPE 				5							//电池类型
#define 	BAT_CSOP_RES 			8							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200  						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	4010						//最高单体充电电压
#define 	BMS_BAT_13S				1						    //13串电池

//12.5AH国产电芯EVE 三元电池
#elif 		defined(BMS_BAT_12500MAH_EVE)
#define 	BAT_NORM_CAP 			12000						//电池标称容量
#define 	BAT_NORM_VOLT 			3600						//电池标称电压
#define 	BAT_TYPE 				6							//电池类型
#define 	BAT_CSOP_RES 			8							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200   						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	4100						//最高单体充电电压

//15AH国产电芯EVE 三元电池
#elif 		defined(BMS_BAT_15000MAH_EVE)
#define 	BAT_NORM_CAP 			15000						//电池标称容量
#define 	BAT_NORM_VOLT 			3600						//电池标称电压
#define 	BAT_TYPE 				7							//电池类型
#define 	BAT_CSOP_RES 			8							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200 						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	4100						//最高单体充电电压
#define 	BMS_BAT_13S				1						    //13串电池

//13AH国产电芯BAK 三元电池
#elif 		defined(BMS_BAT_13000MAH_BAK)
#define 	BAT_NORM_CAP 			12000						//电池标称容量
#define 	BAT_NORM_VOLT 			3600						//电池标称电压
#define 	BAT_TYPE 				8							//电池类型
#define 	BAT_CSOP_RES 			8							//充电电池阻抗估值mR
#define 	MAX_CHG_VOLT 		  	4100						//最高单体充电电压
#define 	BAT_SLEEP_CURR 			200 

//12.5AH韩国电芯三星 三元电池
#elif 		defined(BMS_BAT_12500MAH_SAM)//12.5AH三星电芯
#define 	BAT_NORM_CAP 			12000						//电池标称容量
#define 	BAT_NORM_VOLT 			3600						//电池标称电压
#define 	BAT_TYPE 				9							//电池类型
#define 	BAT_CSOP_RES 			8							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200 						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	4100						//最高单体充电电压

//12AH国产比亚迪 磷酸铁锂电池
//#elif		defined(LFP_HL_25000MAH_16S)
//#define 	BAT_NORM_CAP 		 	15000						//电池标称容量
//#define 	BAT_NORM_CAP_SOX 		15000						//电池标称容量 SOX
//#define 	BAT_NORM_VOLT 		 	3200						//电池标称电压
//#define 	BAT_TYPE 			 	0							//电池类型 磷酸铁锂电芯，从型号0开始
//#define 	BAT_CSOP_RES 			110							//充电电池阻抗估值mR
//#define 	BAT_SLEEP_CURR 			200 						//浅睡眠保护电流0.20A
//#define 	MAX_CHG_VOLT 		  	3600						//最高单体充电电压
//#define     MAX_CHG_CURR            850                         //充电器最大充电电流
//#define     HW_VER_MAIN_BAT_TYPE    15                          //磷酸铁锂电芯

//25Ah国产鹏辉 磷酸铁锂电池
#elif		defined(LFP_HL_25000MAH_16S)
#define 	BAT_NORM_CAP 		 	25000						//电池标称容量
#define 	BAT_NORM_CAP_SOX 		24500						//电池标称容量 SOX
#define 	BAT_NORM_VOLT 		 	3200						//电池标称电压
#define 	BAT_TYPE 			 	2							//电池类型 磷酸铁锂电芯，从型号0开始
#define 	BAT_CSOP_RES 			110							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200 						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	3600						//最高单体充电电压
#define     MAX_CHG_CURR            850                         //充电器最大充电电流
#define     HW_VER_MAIN_BAT_TYPE    15                          //磷酸铁锂电芯
#define 	BMS_BAT_16S				1						    //16串电池

//25Ah国产赣锋 磷酸铁锂电池
#elif		defined(LFP_GF_25000MAH_16S)
#define 	BAT_NORM_CAP 		 	25000						//电池标称容量
#define 	BAT_NORM_CAP_SOX 		24500						//电池标称容量 SOX
#define 	BAT_NORM_VOLT 		 	3200						//电池标称电压
#define 	BAT_TYPE 			 	3							//电池类型 磷酸铁锂电芯，从型号0开始
#define 	BAT_CSOP_RES 			110							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200 						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	3600						//最高单体充电电压
#define     MAX_CHG_CURR            850                         //充电器最大充电电流
#define     HW_VER_MAIN_BAT_TYPE    15                          //磷酸铁锂电芯
#define 	BMS_BAT_16S				1						    //16串电池

//32Ah国产孚能 三元电池
#elif		defined(SY_PB_32000MAH_14S)
#define 	BAT_NORM_CAP 		 	32000						//电池标称容量
#define 	BAT_NORM_CAP_SOX 		32000						//电池标称容量 SOX
#define 	BAT_NORM_VOLT 		 	3600						//电池标称电压
#define 	BAT_TYPE 			 	3							//电池类型 磷酸铁锂电芯，从型号0开始
#define 	BAT_CSOP_RES 			110							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200 						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	4100						//最高单体充电电压
#define     MAX_CHG_CURR            850                         //充电器最大充电电流
#define     HW_VER_MAIN_BAT_TYPE    13                          //三元电芯
#define 	BMS_BAT_14S				1						    //14串电池

//32Ah国产孚能 三元电池
#elif		defined(SY_PB_32000MAH_16S)
#define 	BAT_NORM_CAP 		 	32000						//电池标称容量
#define 	BAT_NORM_CAP_SOX 		32000						//电池标称容量 SOX
#define 	BAT_NORM_VOLT 		 	3600						//电池标称电压
#define 	BAT_TYPE 			 	5							//电池类型 磷酸铁锂电芯，从型号0开始
#define 	BAT_CSOP_RES 			110							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200 						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	4100						//最高单体充电电压
#define     MAX_CHG_CURR            850                         //充电器最大充电电流
#define     HW_VER_MAIN_BAT_TYPE    13                          //三元电芯
#define 	BMS_BAT_16S				1						    //16串电池

//15Ah国产国轩 磷酸铁锂电池
#elif		defined(LFP_AK_15000MAH_16S)
#define 	BAT_NORM_CAP 		 	15000						//电池标称容量
#define 	BAT_NORM_CAP_SOX 		15000						//电池标称容量 SOX
#define 	BAT_NORM_VOLT 		 	3200						//电池标称电压
#define 	BAT_TYPE 			 	0							//电池类型 磷酸铁锂电芯，从型号0开始
#define 	BAT_CSOP_RES 			110							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200 						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	3600						//最高单体充电电压
#define     MAX_CHG_CURR            850                         //充电器最大充电电流
#define     HW_VER_MAIN_BAT_TYPE    15                          //磷酸铁锂电芯
#define 	BMS_BAT_15S				1						    //13串电池

//20Ah国产拓邦 磷酸铁锂电池
#elif		defined(LFP_TB_20000MAH_20S)
#define 	BAT_NORM_CAP 		 	20000						//电池标称容量
#define 	BAT_NORM_CAP_SOX 		19000						//电池标称容量 SOX
#define 	BAT_NORM_VOLT 		 	3200						//电池标称电压
#define 	BAT_TYPE 			 	0							//电池类型 磷酸铁锂电芯，从型号0开始
#define 	BAT_CSOP_RES 			110							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200 						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	3600						//最高单体充电电压
#define     MAX_CHG_CURR            850                         //充电器最大充电电流
#define     HW_VER_MAIN_BAT_TYPE    15                          //磷酸铁锂电芯
#define 	BMS_BAT_20S				1						    //20串电池

//20Ah国产鹏辉 磷酸铁锂电池
#elif		defined(LFP_PH_20000MAH_20S)
#define 	BAT_NORM_CAP 		 	20000						//电池标称容量
#define 	BAT_NORM_CAP_SOX 		19000						//电池标称容量 SOX
#define 	BAT_NORM_VOLT 		 	3200						//电池标称电压
#define 	BAT_TYPE 			 	1							//电池类型 磷酸铁锂电芯，从型号0开始
#define 	BAT_CSOP_RES 			110							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200 						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	3600						//最高单体充电电压
#define     MAX_CHG_CURR            850                         //充电器最大充电电流
#define     HW_VER_MAIN_BAT_TYPE    15                          //磷酸铁锂电芯
#define 	BMS_BAT_20S				1						    //20串电池

//32Ah国产孚能 三元电池
#elif		defined(SY_PB_32000MAH_17S)
#define 	BAT_NORM_CAP 		 	32000						//电池标称容量
#define 	BAT_NORM_CAP_SOX 		28500						//电池标称容量 SOX
#define 	BAT_NORM_VOLT 		 	3600						//电池标称电压
#define 	BAT_TYPE 			 	4							//电池类型 磷酸铁锂电芯，从型号0开始
#define 	BAT_CSOP_RES 			110							//充电电池阻抗估值mR
#define 	BAT_SLEEP_CURR 			200 						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	4100						//最高单体充电电压
#define     MAX_CHG_CURR            850                         //充电器最大充电电流
#define     HW_VER_MAIN_BAT_TYPE    13                          //三元电芯
#define 	BMS_BAT_17S				1						    //16串电池

//10AH韩国电芯MF1 LG 三元电池
#else		
#define 	BAT_NORM_CAP 			10000						//电池标称容量
#define 	BAT_NORM_VOLT 			3650						//电池标称电压
#define 	BAT_TYPE 				1							//电池类型
#define 	BAT_CSOP_RES 			10							//充电电池阻抗估值mR	
#define 	BAT_SLEEP_CURR 			200  						//浅睡眠保护电流0.20A
#define 	MAX_CHG_VOLT 		  	4100						//最高单体充电电压
#endif

/***********************************电芯包参数***********************************/
//电池配置
#ifdef 		BMS_BAT_13S										    //13串电池
#define 	BAT_CELL_NUM         	13  						//电池包单体数目
#define 	BAT_VER_SE_CODE 	 	1 							//硬件版本中串数代码

#elif       defined(BMS_BAT_14S)
#define 	BAT_CELL_NUM         	14  						//电池包单体数目
#define 	BAT_VER_SE_CODE 	 	2 							//硬件版本中串数代码

#elif       defined(BMS_BAT_16S)
#define 	BAT_CELL_NUM         	16  						//电池包单体数目
#define 	BAT_VER_SE_CODE 	 	0 							//硬件版本中串数代码

#elif       defined(BMS_BAT_17S)
#define 	BAT_CELL_NUM         	17  						//电池包单体数目
#define 	BAT_VER_SE_CODE 	 	3 							//硬件版本中串数代码

#elif       defined(BMS_BAT_15S)
#define 	BAT_CELL_NUM         	15  						//电池包单体数目
#define 	BAT_VER_SE_CODE 	 	4 							//硬件版本中串数代码

#elif       defined(BMS_BAT_20S)
#define 	BAT_CELL_NUM         	20  						//电池包单体数目
#define 	BAT_VER_SE_CODE 	 	5 							//硬件版本中串数代码

#else														    //16串电池
#define 	BAT_CELL_NUM         	16  						//电池包单体数目
#define 	BAT_VER_SE_CODE 	 	0 							//硬件版本中串数代码
#endif

#define		BAT_TEMP_NUM		 	(2)  						//电池包单体温度采样数目
#define		BAT_ALL_TEMP_NUM		(2)//(3)  					//电池包最大单体温度采样数目

//信息列表子索引长度定义
#define 	INDEX_FLT_LEN        	(4)                        //主索引故障信息长度
#define 	INDEX_ALM_LEN        	(2)                        //主索引告警信息长度
//=============================================================================================
//定义数据类型
//=============================================================================================

/****************************************状态列表声明****************************************/

//电池保护状态值
typedef enum 
{        
     eProt_Norm  = 0,        //正常状态
     eProt_Full  = 1,        //充满状态
     //eProt_Empty = 2,        //放空状态
}e_ProtStatus;

//工作状态值
typedef enum 
{        
     eWork_Init  = 0,        //初始化状态
     eWork_Sleep  = 1,       //睡眠状态
     eWork_ShutDown = 2,     //关机状态
     eWork_WakeUp   = 3,     //唤醒状态
     eWork_Normal   = 4      //正常工作状态
}e_WorkStatus;

//充放电状态
typedef enum 
{        
     eChgDchg_Idle = 0,      //空闲状态,不充不放
     eChgDchg_Chg  = 1,      //充电状态
     eChgDchg_Dchg = 2,      //放电状态
     eChgDchg_MAX = 3,      //最大
}e_ChgDchgStatus;

//工作模式
typedef enum 
{        
     eMode_Norm = 0,         //正常模式
     eMode_Test = 1,         //测试模式
     eMode_Sleep = 2,        //睡眠模式
     eMode_ShutDown = 3      //关机模式
}e_Mode;

//数据准备标志
typedef enum
{
     e_AFERdy = 0,         //AFE数据准备
	 e_FuelRdy = 1,        //电量计数据准备
	 e_SampleRdy = 2,      //NTC、电压等数据准备
	 e_RTCRdy = 3,         //RTC数据准备
	 e_TempDataRdy = 4,    //温度处理数据准备    
	 e_VoltDataRdy = 5,    //电压处理数据准备      
     e_SoxDattaRdy = 6,     //SOX数据准备
     eDataRdhMax,
}e_DataRdy;

/****************************************结构体声明****************************************/
//BMS主要信息结构体
typedef struct _MAIN_INFO
{
    u16 CellVolt[BAT_CELL_NUM];         //单体电压
    s16 CellTemp[BAT_TEMP_NUM];         //单体温度   
    s16 TolVolt;                        //总电压
    s16 Curr;                           //电流
    s16 Power;                          //功率
    u16 SOC;       	                    //SOC；单位：1%，电量计的SOC
    u16 FiltSOC;                        //滤波后的SOC，用于显示
	u16 SOH;		                    //SOH；单位：1%
    u16 ChgDchgTimes;                   //充放电次数
    u16 TolCap;                         //总容量；单位mAH
    u16 RemainCap;                      //剩余容量；单位mAH
    u16 RemainPower;                    //剩余能量；单位0.1wh
    u16 FullChgTime;                    //充满需要的时间；单位min
    
    /*美信电量计 算法*/
    u16 MAX_SOC;       	                //SOC；单位：1%，SOX的SOC    
	u16 MAX_SOH;		                //SOH；单位：1%
    u16 MAX_ChgDchgTimes;               //充放电次数
    u16 MAX_TolCap;                     //总容量；单位mAH
    u16 MAX_RemainCap;                  //剩余容量；单位mAH
    u16 MAX_RemainPower;                //剩余能量；单位0.1wh    
}t_MainInfo;

//BMS系统状态结构体
typedef struct _STATUS_INFO
{
    e_WorkStatus    WorkSta;    //工作状态
    e_ChgDchgStatus ChgDchgSta; //充放电状态
    e_ProtStatus    ProtSta;    //电池保护状态
    e_Mode          Mode;       //工作模式
    u8 IOStatus;                //IO状态
    u8 MOSStatus;               //MOS状态
    u8 DataReady;               //电池数据准备好标志  bit0:AFE数据；bit1：电量计数据；bit2：NTC、电压等数据；bit3:RTC时间；bit4：电压特征数据；bit5：温度特征数据;
                                //新增bit6:SOX数据
}t_StatusInfo;  

//BMS电压特征值结构体
typedef struct _VOLTCHARA_INFO
{
    u8  MinVNum;         //最小电压电池号
    u16 MinVolt;         //最小电压值
    u8  MaxVNum;         //最大电压电池号
    u16 MaxVolt;         //最大电压值
	u16 PerVolt;	     //平均电压
    s16 VoltDropSpeed[BAT_CELL_NUM];    //单体电压下降速度信息
    u8  MinDropVoltNum;  //最大电压下降速度电池号
    s16 MinDropVolt;	 //最大电压下降速度
}t_VoltCharaInfo;

//BMS温度特征值结构体
typedef struct _TEMPCHARA_INFO
{
    u8  MinTNum;         //最小温度电池号
    s16 MinTemp;         //最小温度值
    u8  MaxTNum;         //最大温度电池号
    s16 MaxTemp;         //最大温度值
	u16 PerTemp;	     //平均温度
    s16 TempRiseSpeed[BAT_ALL_TEMP_NUM];//单体温度上升速度信息，加上一节电量计温度
    u8  MaxRiseTempNum;  //最大温度上升速度电池号
    s16 MaxRiseTemp;	 //最大温度上升速度
}t_TempCharaInfo;

//BMS电流特征值结构体
typedef struct _CURRCHARA_INFO
{
    s16 MaxChgCurrRecent;   //最近最大充电电流，在机柜中充电至80%时清零
    s16 MaxDchgCurrRecent;  //最近最大放电电流，在机柜中充电至80%时清零

	u8 HighCurrTime;		/* 大放电电流持续时间 10s/bit Handry Add 20-07-14*/
	u8 MaxCurr30S;			/* 30s内最大放电电流 单位2A/bit Handry Add 20-07-14*/
	u8 PerCurr30S;			/* 30s内最小放电电流 单位2A/bit   Handry Add 20-07-14*/

}t_CurrCharaInfo;

//BMS故障告警信息结构体
typedef struct _FAULT_INFO
{
    u16 FaultInfo[INDEX_FLT_LEN];  	  //故障信息
    u16 AlarmInfo[INDEX_ALM_LEN];  	  //告警信息
}t_FaultInfo;

//BMS限值信息结构体
typedef struct _LIMIT_INFO
{
    s16 ChgCurrLim;	    //充电电流限值，单位10mA
	s16 DchgCurrLim;    //放电电流限值，单位10mA
	s16 ChgPowLim;      //放电功率限值
	s16 DchgPowLim;     //放电功率限值
	s16 MaxChgVolt;     //最大充电电压
}t_LimitInfo;

typedef struct _ILLEGAL_CHG_INFO
{
    u8 start_dsg_time_s;    //检测起始RTC的分
    u8 start_dsg_time_m;    //检测起始RTC的秒
    u16 curr_dsg_time;      //检测间隔
    u16 curr_dsg_cnt;       //检测计数
    u8 volt_rise_sta;       //0:null 1:voltage < 3.75V  2:voltage > 4.05V
    u32 ChgCapacity;       //非法充电充入的电量
//    int16_t dsg_min_volt;
//    int16_t volt_diff;   
}t_IllegalChgInfo;

//BMS附加信息
typedef struct _ADDIT_INFO
{
    s16 FuelTemp;	    //电量计采样的温度
	s16 FuelAvgCurr;    //电量计采样的平均电流
    u16 FuelCellVolt;   //电量计采样的单体电压分压
    u16 FuelStaFlag;    //电量计的状态标志
}t_AdditInfo;

//BMS所有信息集合结构体
typedef struct _BATTERY_INFO
{
    t_MainInfo       Data;           //主要数据
    t_StatusInfo     Status;         //状态
    t_VoltCharaInfo  VoltChara;      //电压特征信息
    t_TempCharaInfo  TempChara;      //温度特征信息
    t_CurrCharaInfo  CurrChara;      //电流特征信息
    t_FaultInfo      Fault;          //故障告警信息
    t_LimitInfo      Limit;          //限值信息
    t_IllegalChgInfo IllegalChg;     //非法充电信息
    t_AdditInfo      Addit;          //附加信息
}t_BatteryInfo;  

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern t_BatteryInfo gBatteryInfo;     //电池相关的所有信息集合结构体

#endif

/*****************************************end of BatteryInfo.h*****************************************/
