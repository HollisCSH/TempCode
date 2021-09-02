//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: BSPSysTick.c
//创建人  	: Handry
//创建日期	: 
//描述	    : SysTick模块驱动
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//头文件
//=============================================================================================
#include "BSP_SysTick.h"
//#include "interrupt_manager.h"

//=============================================================================================
//函数名称	: void BSPSystickInit()
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: Systick初始化函数，清零CTRL/VAL/LOAD寄存器，配置中断优先级
//注    意	:
//=============================================================================================
void BSPSystickInit()
//{
//	S32_SysTick->CSR = 0;   //设置前先关闭systick
//	S32_SysTick->CVR  = 0;   //清除计数器
//	S32_SysTick->RVR = 0;	 //清零装载值寄存器

    //设定 SysTick优先级为2
//    INT_SYS_SetPriority(SysTick_IRQn,2);
}

//=============================================================================================
//函数名称	: void BSPSysDelay1ms(u16 nms)
//输入参数	: mns：延时的ms时长
//输出参数	: void
//静态变量	: void
//功    能	: 使用Systick进行ms级延时
//注    意	:
//=============================================================================================
void BSPSysDelay1ms(u16 nms)
{
	u32 temp;
	u32 timeload = 0;

	timeload = (u32)nms * CORE_CLK_KHZ;				// 16777215 / 48000 = 349ms 最大

	if(timeload > 0xffffff)
	{
		timeload = 0xffffff;						//349 - 350 ms之间
	}

	S32_SysTick->RVR = timeload;
	S32_SysTick->CVR  = 0x00;

	S32_SysTick->CSR|= S32_SysTick_CSR_CLKSOURCE_MASK;		//使用内核时钟
	S32_SysTick->CSR|= S32_SysTick_CSR_ENABLE_MASK;		//使能计数

	do
	{
		temp = S32_SysTick->CSR;
	}
	while((temp&0x01) && (!(temp&(1<<16))));		//定制器计数到0，等待COUNTFLAG置1

	S32_SysTick->CSR &= ~S32_SysTick_CSR_ENABLE_MASK;		//清除计数使能
	S32_SysTick->CVR  = 0x00;
}

//=============================================================================================
//函数名称	: void BSPSysDelay1ns(u16 nns)
//输入参数	: nns：延时的ns时长
//输出参数	: void
//静态变量	: void
//功    能	: 使用Systick进行ns级延时
//注    意	:
//=============================================================================================
void BSPSysDelay1ns(u16 nns)
{
	u32 temp;
	u32 timeload = 0;

	timeload = (u32)nns * CORE_CLK_KHZ / 1000;		// 16777215 / 48000 * 1000 = 349000ns 最大

	if(timeload > 0xffffff)
	{
		timeload = 0xffffff;
	}

	S32_SysTick->RVR = timeload;
	S32_SysTick->CVR  = 0x00;

	S32_SysTick->CSR|= S32_SysTick_CSR_CLKSOURCE_MASK;		//使用内核时钟
	S32_SysTick->CSR|= S32_SysTick_CSR_ENABLE_MASK;		//使能计数

	do
	{
		temp = S32_SysTick->CSR;
	}
	while((temp&0x01) && (!(temp&(1<<16))));		//定制器计数到0，等待COUNTFLAG置1

	S32_SysTick->CSR &= ~S32_SysTick_CSR_ENABLE_MASK;		//清除计数使能
	S32_SysTick->CVR  = 0x00;
}

//=============================================================================================
//函数名称	: void BSPSysDelay1s(u16 ns)
//输入参数	: ns：延时的s时长
//输出参数	: void
//静态变量	: void
//功    能	: 使用Systick进行s级延时
//注    意	:
//=============================================================================================
void BSPSysDelay1s(u16 ns)
{
	u16 i = 0;

	for(i = 0; i < ns; i++)
	{
		BSPSysDelay1ms(300);
		BSPSysDelay1ms(300);
		BSPSysDelay1ms(300);
		BSPSysDelay1ms(100);
	}
}

/*****************************************end of BSPSysTick.c*****************************************/
