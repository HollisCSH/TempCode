//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: ParaCfg.h
//创建人  	: Handry
//创建日期	: 
//描述	    : 参数配置头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _PARACFG_H_
#define _PARACFG_H_
//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#define     PARA_ENABLE_ACC     1
#define 	PROTECT_MAX_LEVEL 	4		//最大故障等级数
#define 	PARA_CFG_FLAG 		0x1237	//配置参数宏

#if defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S) || defined(SY_PB_32000MAH_17S)
#define     PROT_MAX_CHG_CURR     3300    //最大充电保护电流为35A
#elif defined(LFP_TB_20000MAH_20S)
#define     PROT_MAX_CHG_CURR     2000    //最大充电保护电流为20A
#elif defined(LFP_HL_25000MAH_16S)
#define     PROT_MAX_CHG_CURR     2500    //最大充电保护电流为25A
#elif defined(LFP_GF_25000MAH_16S)
#define     PROT_MAX_CHG_CURR     2500    //最大充电保护电流为25A
#elif defined(LFP_PH_20000MAH_20S)
#define     PROT_MAX_CHG_CURR     2000    //最大充电保护电流为20A
#else
#define     PROT_MAX_CHG_CURR     1200    //最大充电保护电流为12A
#endif

//=============================================================================================
//数据结构定义
//=============================================================================================
typedef struct
{
    s16 matth[PROTECT_MAX_LEVEL];
    s16 resth[PROTECT_MAX_LEVEL];
    u16 mattime[PROTECT_MAX_LEVEL];
    u16 restime[PROTECT_MAX_LEVEL];
}t_DT_PARA;

typedef struct
{
    u16 mattime;
    u16 restime;
}t_DT_BOOL_PARA;

//标定配置参数
typedef struct
{
    int16_t sampleres;	//电流采样电阻uR
    int16_t sampleoff;	//电流采样 零飘
    int16_t b16gain;	//uV/bit 16bit
    int16_t b16offset;	//mV
    uint16_t fuelrense; //电量计分流器阻值
    u8 FuelAtLesRem;    //电量计至少剩余的更新次数
    s16 COff;           //电量计零飘
    u16 CGain;          //电量计k值斜率
}t_CAL_PARA;

typedef struct
{
    u16 en;     //均衡使能
    s16 uv;     //均衡电压上限值   
    s16 lv;     //均衡电压下限值
    s16 deltav; //均衡压差阈值
}t_BALANCE_PARA;

#ifdef     PARA_ENABLE_ACC
typedef struct
{
    u16 TapEn;      //是否使能Tap中断
    u16 ActiveDur;
    u16 ActiveTh;
    u16 FreeFallDur;
    u16 FreeFallTh;
}t_ACC_PARA;
#endif

/*
Handry Add 20-07-02
3C电流检测 配置参数:
最大放电电流监测门限    Bit7-Bit0     R/W     2A/bit      0~254A      默认30A
最大放电电流监测周期    Bit11-Bit8    R/W     10s/bit     0~150s      默认30s
最小放电电流监测周期    Bit15-Bit12   R/W     10s/bit     0~150s      默认30s
*/
typedef struct
{
    uint16_t highcurrdet;	//3C电流检测 配置参数
    
}t_CURR_DET_PARA;	

/* 额外的配置参数 */
typedef struct
{
	t_CURR_DET_PARA currdet;
    
    t_DT_PARA preotemp;			//B11使用，预启动电阻温度过高 21.01.26
	
}t_CONFIG_PARA_ADD;

typedef struct
{
    u16 flag;
    u16 sn[4];
    t_DT_PARA cellov;			//单体电池过压
    t_DT_PARA celluv;			//单体电池欠压
    t_DT_PARA totleov;			//总电压过压
    t_DT_PARA totleuv;			//总电压欠压
    t_DT_PARA cellcot;			//单体充电过温
    t_DT_PARA cellcut;			//单体充电欠温
    t_DT_PARA celldot;			//单体放电过温
    t_DT_PARA celldut;			//单体放电欠温
    t_DT_PARA avgot;			//平均温度过温
    t_DT_PARA avgut;			//平均温度欠温
    t_DT_PARA ccurr;			//充电过流
    t_DT_PARA dcurr;			//放电过流
    t_DT_PARA osoc;				//SOC过高
    t_DT_PARA usoc;				//SOC过低
    t_DT_PARA pcbot;			//PCB板温度过高
    t_DT_PARA connot;			//连接器温度过高
    t_DT_PARA predot;			//预放管温度过高
    t_DT_PARA tvsot;			//tvs温度过高
    t_DT_PARA vdiff;			//电压差异过大
    t_DT_PARA tdiff;			//温度差异过大

    //故障字1
    t_DT_BOOL_PARA afeflt;		//AFE故障
    t_DT_BOOL_PARA fuelflt;		//电量计故障
    t_DT_BOOL_PARA rtcflt;		//RTC故障
    t_DT_BOOL_PARA flashflt;	//FLASH故障
    t_DT_BOOL_PARA dmosflt;		//放电MOS失效
    t_DT_BOOL_PARA cmosflt; 	//充电MOS失效
    t_DT_BOOL_PARA preflt;		//预启动电路故障
    t_DT_BOOL_PARA rtccalflt;	//rtc未校准
    t_DT_BOOL_PARA paraflt;		//参数未校准
    t_DT_BOOL_PARA fuseflt;		//保险丝失效
    t_DT_BOOL_PARA accflt;		//加速度计失效
    t_DT_BOOL_PARA othflt; 		//其他设备故障

    //故障字2
    t_DT_BOOL_PARA cntcflt; 	//充电MOS NTC失效
    t_DT_BOOL_PARA dntcflt;		//放电MOS NTC失效
    t_DT_BOOL_PARA fuelntcflt;	//电量计NTC失效
    t_DT_BOOL_PARA conntcflt;	//连接器NTC失效
    t_DT_BOOL_PARA tvsntcflt; 	//TVS NTC失效
    t_DT_BOOL_PARA b1ntcflt;	//电池 NTC1失效
    t_DT_BOOL_PARA b2ntcflt;	//电池 NTC2失效
    
    t_DT_BOOL_PARA exmodflt;	//外置模块短路失效
    
    //故障字3
    t_DT_BOOL_PARA preotflt;	//预启动失败超时
    t_DT_BOOL_PARA preocflt;	//预启动失败外部重载/短路

    //故障字4
    t_DT_BOOL_PARA doclock;		//放电过流保护锁定
    t_DT_BOOL_PARA coclock;		//充电过流保护锁定
    t_DT_BOOL_PARA afeale;		//AFE ALERT故障
    t_DT_BOOL_PARA afexrdy;		//AFE XREADY故障
    t_DT_BOOL_PARA slpoc;		//睡眠模式过流故障

    //标定参数
    t_BALANCE_PARA bala;        
    t_CAL_PARA calpara;    
//    t_BASIC_PARA basic;

#ifdef     PARA_ENABLE_ACC
    t_ACC_PARA accpara;
#endif

    u16 crc16;
    
	/* Handry Add 20-07-03*/
	t_CONFIG_PARA_ADD configadd;
    
}t_CONFIG_PARA;


/*计算配置参数CRC的长度*/
#define		CALC_CRC_CONFIG_SIZE (sizeof(t_CONFIG_PARA) - sizeof(t_CONFIG_PARA_ADD))

//=============================================================================================
//声明外部变量
//=============================================================================================
extern t_CONFIG_PARA gConfig;
extern t_CONFIG_PARA gConfigBuff;//保存配置的输入buffer
extern t_DT_PARA gConfigChgCurr; //充电过流实际使用的配置值
extern const t_CONFIG_PARA cConfigInit;//初始化值通过校准等命令写入Flash

//=============================================================================================
//声明接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void ParaInit(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功	能	: 参数初始化函数
//注	意	:
//=============================================================================================
void ParaInit(void);

#endif

/*****************************************end of ParaCfg.h*****************************************/
