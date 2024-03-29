//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//-----------------------------------------文件信息---------------------------------------------
//文件名   	: BSPCriticalZoom.h
//创建人  	: Hardry
//创建日期	: 
//描述	    : 中断屏蔽区头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _BSPCRITICALZOOM_H
#define _BSPCRITICALZOOM_H

#define		EnableAllInterrrupts		INT_SYS_EnableIRQGlobal()
#define 	DisableAllInterrrupts 		INT_SYS_DisableIRQGlobal()
//=============================================================================================
//宏定义
//=============================================================================================

//=============================================================================================
//类型定义
//=============================================================================================

//=============================================================================================
//函数名称:  void BSPEnterCritical(void)
//输	入:  void
//输	出:  void
//静态变量:
//功	能:  进入临界区	
//=============================================================================================
void BSPEnterCritical(void);

//=============================================================================================
//函数名称:  void BSP_EXIT_CRITICAL(void)
//输	入:  void
//输	出:  void
//静态变量:
//功	能:  退出临界区		
//=============================================================================================
void BSPExitCritical(void);

#endif

/*****************************************end of BSPCriticalZoom.h*****************************************/
