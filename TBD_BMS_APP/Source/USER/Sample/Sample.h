//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Sample.h  
//创建人  	: Handry
//创建日期	:  
//描述	    : 采样任务头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	: 
//版本	    : 
//修改日期	: 
//描述	    :
//=============================================================================================
#ifndef _SAMPLE_H
#define _SAMPLE_H

//========== ===================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"
//#include "BSPGPIO.h"

//========== ===================================================================================
//宏定义
//=============================================================================================
#define 	AD_FILTER_NUM       		5      	    //AD采样值滤波个数
#define 	V16_AD_FILTER_NUM       	10          //V16 AD采样值滤波个数
#define		ADC_SAMP_NUM				5			//AD采样通道数

//#define		MCU_3V3_ADC_CHAN			2		//ADC采样通道,通道2,关闭PCB CTR后为3.3V的采样
//#define		PRE_DSG_SHORT_ADC_CHAN		8		//ADC采样通道,通道X，短路检测电压采样通道
//#define		PCB_TEMP_ADC_CHAN		    2		//ADC采样通道,通道2,PCB温度采样通道
//#define		TVS_TEMP_ADC_CHAN		    4		//ADC采样通道,通道4,TVS管温度采样通道
//#define		PRE_DSG_TEMP_ADC_CHAN		0		//ADC采样通道,通道0,预放电电阻温度采样通道

#define		MCU_3V3_ADC_CHAN				6		//ADC采样通道,通道2,关闭PCB CTR后为3.3V的采样
#define		PRE_DSG_SHORT_ADC_CHAN	8		//ADC采样通道,通道X，短路检测电压采样通道
#define		PCB_TEMP_ADC_CHAN		    6		//ADC采样通道,通道2,PCB温度采样通道
#define		TVS_TEMP_ADC_CHAN		    5		//ADC采样通道,通道4,TVS管温度采样通道
#define		PRE_DSG_TEMP_ADC_CHAN		7		//ADC采样通道,通道0,预放电电阻温度采样通道

//#define     PCB_TVS_NTC_SAMP_ON     BSPGPIOClrPin(PCB_TVS_NTC_CTR_PORT,PCB_TVS_NTC_CTR_PIN);		//开启PCB TVS NTC采样控制，测PCB温度
//#define     PCB_TVS_NTC_SAMP_OFF    BSPGPIOSetPin(PCB_TVS_NTC_CTR_PORT,PCB_TVS_NTC_CTR_PIN);		//关闭PCB TVS NTC采样控制

//#define     TVS_NTC_SAMP_ON     BSPGPIOClrPin(TVS_NTC_CTR_PORT,TVS_NTC_CTR_PIN);		//开启TVS NTC采样控制，测TVS温度
//#define     TVS_NTC_SAMP_OFF    BSPGPIOSetPin(TVS_NTC_CTR_PORT,TVS_NTC_CTR_PIN);		//关闭TVS NTC采样控制

//#define     PRE_DSG_NTC_SAMP_ON     BSPGPIOClrPin(PRE_DSG_NTC_ONOFF_PORT,PRE_DSG_NTC_ONOFF_PIN);    //开启预放电 NTC采样控制，测预放电电阻温度
//#define     PRE_DSG_NTC_SAMP_OFF    BSPGPIOSetPin(PRE_DSG_NTC_ONOFF_PORT,PRE_DSG_NTC_ONOFF_PIN);	//关闭预放电 NTC采样控制

//========== ===================================================================================
//定义数据类型
//=============================================================================================
//BMS限值信息结构体
typedef struct _SAMPLE_DATA
{
    s16 ConnTemp;		//连接器温度
    s16 PCBTemp;		//PCB板温度
    s16 TVSTemp;	    //TVS管温度
    u16 PreVolt;	    //预放电管短路电压
    u16 MCU3v3;			//MCU 3.3V电压实际采样值
	s16 FuseTemp;	    //FUSE管温度 
	s16 PreTemp;	    //预放电电阻温度 
    
    union
	{
		struct
		{
			u8 IsNTCSampIng :1;	//NTC采样标志，0：采样完成；1:正在采样
            u8 IsPreSampOk  :1;	//预放电电流采样标志，1：采样完成；0:正在采样
			u8 RVS          :6; //保留
		}SampFlagBit;
		u8 SampFlagByte;
	}SampFlag;
    
    #ifdef BMS_ENABLE_LITTLE_CURRENT_DET
    union
	{
		struct
		{
			u8 IsSampIng    :1;	//小电流采样P1 P2 AD正在采样标志
            u8 IsSampBaseOK :1;	//小电流采样P1 P2 AD基准采样完成标志
			u8 IsSampOK     :1;	//小电流采样P1 P2 AD采样完成标志
            u8 IsInitAll    :1;	//小电流采样是否要初始化采样
			u8 RVS          :4;   //保留
		}LowCurrFlagBit;
		u8 LowCurrFlagByte;
	}LowCurrFlag;
    
    s16 P1ADBaseVolt;   //P1基准采样的电压
    s16 P2ADBaseVolt;   //P2基准采样的电压    
    s16 P1LowCurrVal; //P1小电流
    s16 P2LowCurrVal; //P2小电流    
    #endif
    
}t_SampleData;

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern t_SampleData gSampleData;   //采样的数据

//=============================================================================================
//声明接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void SampleInit(void)
//输入参数	: void  
//输出参数	: void
//静态变量	: void
//功	能	: 采样任务初始化 
//注	意	:  
//=============================================================================================
void SampleInit(void);

//=============================================================================================
//函数名称	: void SampleMainTask(void *p)
//输入参数	: void  
//输出参数	: void
//静态变量	: void
//功	能	: 采样主任务
//注	意	:  
//=============================================================================================
void SampleMainTask(void *p);

//小电流检测功能
#ifdef BMS_ENABLE_LITTLE_CURRENT_DET
//=============================================================================================
//函数名称	: void SampleLowCurrentTask(void *p)
//输入参数	: void  
//输出参数	: void
//静态变量	: void
//功	能	: 小电流采样主任务
//注	意	:  
//=============================================================================================
void SampleLowCurrentTask(void *p);
#endif

#endif

/*****************************************end of Sample.h*****************************************/
