//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: BSPSysTick.h
//创建人  	: Handry
//创建日期	: 
//描述	    : SysTick模块头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _BSPSYSTICK_H
#define _BSPSYSTICK_H

//=============================================================================================
//头文件
//=============================================================================================
//#include "system_S32K118.h"
#include "BSPTypeDef.h"

//=============================================================================================
//宏定义
//=============================================================================================
#define CORE_CLOCK        0
#define CORE_CLOCK_DIV_16 0

#define  SYSTEM_CLK_KHZ   (DEFAULT_SYSTEM_CLOCK/1000) 	// 芯片系统时钟频率(KHz)
#define  CORE_CLK_KHZ     SYSTEM_CLK_KHZ       			// 芯片内核时钟频率(KHz)
#define  BUS_CLK_KHZ      (SYSTEM_CLK_KHZ/2)          	// 芯片总线时钟频率(KHz)

//=============================================================================================
//函数名称	: void BSPSystickInit(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: Systick初始化函数，清零CTRL/VAL/LOAD寄存器，配置中断优先级
//注    意	:
//=============================================================================================
void BSPSystickInit(void);

//=============================================================================================
//函数名称	: void BSPSysDelay1ms(u16 nms)
//输入参数	: mns：延时的ms时长
//输出参数	: void
//静态变量	: void
//功    能	: 使用Systick进行ms级延时
//注    意	:
//=============================================================================================
void BSPSysDelay1ms(u16 nms);

//=============================================================================================
//函数名称	: void BSPSysDelay1ns(u16 nns)
//输入参数	: nns：延时的ns时长
//输出参数	: void
//静态变量	: void
//功    能	: 使用Systick进行ns级延时
//注    意	:
//=============================================================================================
void BSPSysDelay1ns(u16 nns);

//=============================================================================================
//函数名称	: void BSPSysDelay1s(u16 ns)
//输入参数	: ns：延时的s时长
//输出参数	: void
//静态变量	: void
//功    能	: 使用Systick进行s级延时
//注    意	:
//=============================================================================================
void BSPSysDelay1s(u16 ns);

#endif

/*****************************************end of BSPSysTick.h*****************************************/
