//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: IOCheck.h
//创建人  	: Handry
//创建日期	: 
//描述	    : 输入IO检测和保险丝检测头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _IOCHECK_H_
#define _IOCHECK_H_
//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"
#include "BSPSystemCfg.h"
//#include "BSPGPIO.h"
#include "BSP_GPIO.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#if defined(USE_B22_IM_PCBA)
#define		MODULE_CAN_3V3_ON()     //使用隔离CAN
#define		MODULE_CAN_3V3_OFF()
#elif   defined(USE_B20_TOPBAND_PCBA) || defined(USE_B21_IM_PCBA)
//#define		MODULE_CAN_3V3_ON()			BSPGPIOClrPin(MODULE_CAN_3V3_PORT,MODULE_CAN_3V3_PIN);
//#define		MODULE_CAN_3V3_OFF()		BSPGPIOSetPin(MODULE_CAN_3V3_PORT,MODULE_CAN_3V3_PIN);
#define		MODULE_CAN_3V3_ON()			HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_SET);		//原理图管脚描述错误 CAN_5V_PORT 控制3.3V
#define		MODULE_CAN_3V3_OFF()		HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_RESET);
#endif

#if   defined(USE_B20_TOPBAND_PCBA)
#define		MODULE_CAN_5V_ON()
#define		MODULE_CAN_5V_OFF()
#else
//#define		MODULE_CAN_5V_ON()			BSPGPIOSetPin(MODULE_CAN_5V_PORT,MODULE_CAN_5V_PIN);
//#define		MODULE_CAN_5V_OFF()			BSPGPIOClrPin(MODULE_CAN_5V_PORT,MODULE_CAN_5V_PIN);
#define		MODULE_CAN_5V_ON()			HAL_GPIO_WritePin(PWR_EN_14V_5V_PORT, PWR_EN_14V_5V_PIN, GPIO_PIN_SET);		//原理图管脚描述错误 PWR_EN_14V_5V_PORT 控制5V
#define		MODULE_CAN_5V_OFF()			HAL_GPIO_WritePin(PWR_EN_14V_5V_PORT, PWR_EN_14V_5V_PIN, GPIO_PIN_RESET);
#endif

#if defined(USE_B20_TOPBAND_PCBA) || defined(USE_B22_IM_PCBA)
#define		MODULE_CAN_STANDBY()    //使用隔离CAN
#define		MODULE_CAN_NORMAL()
#elif defined(USE_B21_IM_PCBA)
//#define		MODULE_CAN_STANDBY()		BSPGPIOSetPin(MODULE_CAN_STB_PORT,MODULE_CAN_STB_PIN)
//#define		MODULE_CAN_NORMAL()			BSPGPIOClrPin(MODULE_CAN_STB_PORT,MODULE_CAN_STB_PIN)
#endif

#define	 	WAKE_COND_HALL_MASK 		0x01
#define 	WAKE_COND_NFC_MASK 			0x02
#define 	WAKE_COND_ACC_MASK 			0x04
#define 	WAKE_COND_PHOTO_MASK 		0x08
#define 	WAKE_COND_RESPHOTO_MASK 	0x10    //光敏恢复中断，用于唤醒
#define 	WAKE_COND_EXMODULE_MASK 	0x20    //外置模块中断，用于唤醒

#define 	WAKE_COND_ACC_INT_MASK 		0x40    //加速度模块中断，用于唤醒

#define 	WAKE_COND_CAN_INT_MASK 		0x80    //CAN接收中断，用于唤醒

#define		HALL_CHECK_SHIFT			0X01
#define		REMOVE_CHECK_SHIFT			0X02
#define		PRE_DCHG_CHECK_SHIFT		0X04
#define		EXTERN_MODULE_CHECK_SHIFT	0X08    //外置模块

//#define		EXTERN_MODULE_OFF_TIME  	200 //10000 / 50    //单位 50ms 外置模块断开持续时间
#define		    EXTERN_MODULE_OFF_TIME  	40 //1000 / 50    //单位 50ms 外置模块断开持续时间

//小电流检测功能
#ifdef BMS_ENABLE_LITTLE_CURRENT_DET
#define		LOW_CURRENT0_3V3_ON()		BSPGPIOSetPin(OP_3V3_0_OFF_PORT,OP_3V3_0_OFF_PIN)
#define		LOW_CURRENT0_3V3_OFF()		BSPGPIOClrPin(OP_3V3_0_OFF_PORT,OP_3V3_0_OFF_PIN)

#define		LOW_CURRENT1_3V3_ON()		BSPGPIOClrPin(OP_3V3_1_OFF_PORT,OP_3V3_1_OFF_PIN)
#define		LOW_CURRENT1_3V3_OFF()		BSPGPIOSetPin(OP_3V3_1_OFF_PORT,OP_3V3_1_OFF_PIN)
#endif

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern u8 gFuseDetEn;	//保险丝检测使能 1：使能；0：不使能
extern u8 gFuseFault;	//保险丝故障 1：故障；0：正常
extern u8 gExModuleFault;//外置模块故障 1：故障；0：正常
extern u8 gExModuleOff;  //外置模块断开标志
extern u8 gExModuleOffLine;//外置模块完全断开标志，可以进入休眠

//=============================================================================================
//声明接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void IOCheckInit(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功	能	: IO检测初始化函数
//注	意	:
//=============================================================================================
void IOCheckInit(void);

//=============================================================================================
//函数名称	: void IOCheckHallIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测霍尔引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckHallIntCallback(void);

//=============================================================================================
//函数名称	: void IOCheckRemoveIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测拆开引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckRemoveIntCallback(void);

//=============================================================================================
//函数名称	: void IOCheckRemoveResIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测拆开后恢复引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckRemoveResIntCallback(void);

//=============================================================================================
//函数名称	: void IOCheckNFCIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测NFC中断引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckNFCIntCallback(void);

//=============================================================================================
//函数名称	: void IOCheckACCIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测ACC加速度计中断引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckACCIntCallback(void);

//=============================================================================================
//函数名称	: void IOCheckACCWakeIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测ACC加速度计唤醒中断引脚调用函数
//注    意	:
//=============================================================================================
void IOCheckACCWakeIntCallback(void);

//=============================================================================================
//函数名称	: void IOCheckExModuleIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 外置模块检测中断引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckExModuleIntCallback(void);

//=============================================================================================
//函数名称	: void IOCheckCANIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测CAN中断引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckCANIntCallback(void);

//=============================================================================================
//函数名称	: void IOCheckWakeupCondReset(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测唤醒标志位清零
//注    意	:
//=============================================================================================
void IOCheckWakeupCondReset(void);

//=============================================================================================
//函数名称	: void IOCheckWakeupCondClr(u8 Cond)
//函数参数	: Cond:要设置的唤醒标志位
//输出参数	:
//静态变量	:
//功    能	: IO检测唤醒标志位设置
//注    意	:
//=============================================================================================
void IOCheckWakeupCondClr(u8 Cond);

//=============================================================================================
//函数名称	: u8 IOCheckWakeupCondGet(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 获取IO检测唤醒标志位
//注    意	:
//=============================================================================================
u8 IOCheckWakeupCondGet(void);

//=============================================================================================
//函数名称	: void IOCheckFuseDetectTask(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测保险丝检测任务
//注    意	: 50ms周期
//=============================================================================================
void IOCheckFuseDetectTask(void);

//=============================================================================================
//函数名称	: void IOCheckModuleShortDetectTask(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: IO检测外置模块短路检测任务
//注    意	: 50ms周期
//=============================================================================================
void IOCheckModuleShortDetectTask(void);

//=============================================================================================
//函数名称	: void IOCheckMainTask(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测主任务函数
//注    意	: 50ms周期
//=============================================================================================
void IOCheckMainTask(void);

#ifdef CANBUS_MODE_JT808_ENABLE

//#define			MODULE_PWR_ON()		BSPGPIOSetPin(MODULE_VCC_CTR_PORT,MODULE_VCC_CTR_PIN);
//#define			MODULE_PWR_OFF()	BSPGPIOClrPin(MODULE_VCC_CTR_PORT,MODULE_VCC_CTR_PIN);
#define			MODULE_PWR_ON()		HAL_GPIO_WritePin(VCC_MODULE_PORT, VCC_MODULE_PIN, GPIO_PIN_SET);
#define			MODULE_PWR_OFF()	HAL_GPIO_WritePin(VCC_MODULE_PORT, VCC_MODULE_PIN, GPIO_PIN_RESET);
//=============================================================================================
//函数名称	: void ModuleHWReset(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 外置模块硬件重启
//注    意	: 100ms延时
//=============================================================================================
//void ModuleHWReset(void);

//=============================================================================================
//函数名称	: void ModuleIntPinSetOutput(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 外置模块检测引脚配置成输出
//注    意	: 
//=============================================================================================
void ModuleIntPinSetOutput(void);

#define			MODULE_WAKEUP()		BSPGPIOSetPin(EXTERNAL_MODULE_INT_PORT,EXTERNAL_MODULE_INT_PIN);

//=============================================================================================
//函数名称	: void ModuleIntPinSetInput(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 外置模块检测引脚配置成输入
//注    意	: 
//=============================================================================================
void ModuleIntPinSetInput(void);


#endif // #ifdef CANBUS_MODE_JT808_ENABLE

#if defined(USE_B22_IM_PCBA)
//=============================================================================================
//函数名称	: void IOCheckSCShortDetectTask(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: IO检测电池短路检测任务
//注    意	: 50ms周期
//=============================================================================================
void IOCheckSCShortDetectTask(void);

//=============================================================================================
//函数名称	: void IOCheckSCShortDetectDeal(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: IO检测电池短路处理任务
//注    意	: 
//=============================================================================================
void IOCheckSCShortDetectDeal(void);
#endif

#endif

/*****************************************end of IOCheck.h*****************************************/
