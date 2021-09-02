//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//-----------------------------------------文件信息---------------------------------------------
//文件名   	: BSPCriticalZoom.c
//创建人  	: Hardry
//创建日期	: 
//描述	    : 中断屏蔽区相关的接口
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"
#include "BSPCriticalZoom.h"
#include "stm32g0xx_hal.h"
//#include "interrupt_manager.h"

//=============================================================================================
//全局变量定义
//=============================================================================================
static u16 sInterruptTimes = 0;

//=============================================================================================
//静态函数声明
//=============================================================================================


void INT_SYS_DisableIRQGlobal(void)             //关中断
{
	__disable_irq();
}
void INT_SYS_EnableIRQGlobal(void)              //关中断
{
	__enable_irq();	
}
//=============================================================================================
//函数名称:  void BSPEnterCritical(void)
//输	入:  void
//输	出:  void
//静态变量:
//功	能:  进入临界区	
//=============================================================================================
void BSPEnterCritical(void)
{
	if(sInterruptTimes == 0)
    {
		DisableAllInterrrupts;
	}
	sInterruptTimes++;  
}

//=============================================================================================
//函数名称:  void BSPExitCritical(void)
//输	入:  void
//输	出:  void
//静态变量:
//功	能:  退出临界区		
//=============================================================================================
void BSPExitCritical(void)
{
   
    u16 CurrentTime = 0;
    
    CurrentTime = sInterruptTimes;
    
    if(CurrentTime == 0) 
    {
    	EnableAllInterrrupts;
    } 
    else if(CurrentTime == 1) 
    {
        sInterruptTimes = 0;
        EnableAllInterrrupts;
    } 
    else 
    {
        sInterruptTimes--;
    } 
}

/*****************************************end of BSPCriticalZoom.c*****************************************/