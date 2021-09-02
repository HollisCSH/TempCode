//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: BSPTask.c
//创建人  	: Handry
//创建日期	:
//描述	    : 任务调度模块
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPHardWareIntf.h"
#include "BSPCriticalZoom.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
static t_Task sTaskTbl[TASK_MAX_NUM];  //任务调度数据结构数组，用于多任务处理
static u32 sTaskSysTimer = 0;          //任务系统总计数器
static u32 sPowerOnTimer = 0;          //用于计算休眠唤醒的时候，经历的时长
static u32 sTaskDelayMax = 0;          //最大系统延迟，单位：1ms
static u8  sTaskRunNum	= 0;		   //当前运行的任务个数

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void BSPTaskInit(void)
//输入参数	: void  
//输出参数	: void
//函数功能	: 任务调度初始化
//注意事项	:
//=============================================================================================
void BSPTaskInit(void)
{
	u8 i = 0;
	t_Task *ptask;

	sTaskSysTimer = 0;					//初始化任务系统总计数器
	sTaskRunNum = 0;					//当前运行任务个数0
	ptask = &sTaskTbl[0];
	
	for(i = 0; i < TASK_MAX_NUM; i++)	//初始化任务调度数据
	{
		ptask->TaskEn  = 0;
		ptask->TaskCnt = 0xffffffff;
		ptask->TaskRdy = 0;
		ptask++;
	}
}

//=============================================================================================
//函数名称	: void BSPTaskCreate(u8 id, void (*func)(void *), void *arg)
//输入参数	: id:任务ID 		*func:执行函数 		arg:参数
//输出参数	: void
//函数功能	: 创建任务相应的函数及输入参数
//注意事项	:  
//=============================================================================================
void BSPTaskCreate(u8 id, void (*func)(void *), void *arg)
{
	t_Task *ptask;

	if (id < TASK_MAX_NUM)
	{
		BSPEnterCritical();				//关中断
		ptask             = &sTaskTbl[id];
		ptask->TaskFunc    = func;      //配置任务函数
		ptask->TaskFuncArg = arg;       //配置任务参数
		sTaskRunNum++;					//任务个数+1
		BSPExitCritical();				//开中断
	}
}

//=============================================================================================
//函数名称	: void BSPTaskCreate(u8 id)
//输入参数	: id:任务ID
//输出参数	: void
//函数功能	: 删除指定的任务
//注意事项	:
//=============================================================================================
void BSPTaskDelete(u8 id)
{
	t_Task *ptask;

	if (id < TASK_MAX_NUM)
	{
		BSPEnterCritical();				//关中断
		ptask           = &sTaskTbl[id];
		ptask->TaskEn  	= 0;
		ptask->TaskCnt  = 0xffffffff;	//执行时间设置为最大
		ptask->TaskRdy  = 0;
		ptask->TaskFunc = NULL;         //任务函数清空
		sTaskRunNum--;					//任务个数-1
		BSPExitCritical();				//开中断
	}
}

//=============================================================================================
//函数名称	: void BSPTaskDelay(u8 id, u32 delay)
//输入参数	: id:任务号 delay:延迟时间
//输出参数	: void
//函数功能	: 设置任务延迟时间 
//注意事项	:  
//=============================================================================================
void BSPTaskDelay(u8 id, u32 delay)
{
	t_Task *ptask;

	if (id < TASK_MAX_NUM)
	{
		ptask = &sTaskTbl[id];
		BSPEnterCritical();				//关中断
		ptask->TaskCnt = delay + sTaskSysTimer;
		BSPExitCritical();				//开中断
	}
}

//=============================================================================================
//函数名称	: u8 BSPTaskStart(u8 id, u32 delay)
//输入参数	: id:任务ID delay:下次执行延迟时间
//输出参数	: u8 返回1，配置pt一起使用
//函数功能	: 设定任务在延迟delay时间后启动,设置为0，则任务立刻就绪
//注意事项	:  
//=============================================================================================
u8 BSPTaskStart(u8 id, u32 delay)
{
	t_Task *ptask;

	if (id < TASK_MAX_NUM)
	{
		ptask = &sTaskTbl[id];

		BSPEnterCritical();				//关中断

		ptask->TaskCnt = delay + sTaskSysTimer;	//延迟时间 + 当前时间轴时间
		ptask->TaskEn = 1;

		if(delay == 0) 		//设置为0，则任务立刻就绪
		{
			ptask->TaskRdy = 1;
		}
		else
		{
			ptask->TaskRdy = 0;
		}
		BSPExitCritical();				//开中断
	}

	return 1;   //u8 返回1，配置pt一起使用
}

//=============================================================================================
//函数名称	: void BSPTaskStop(u8 id)
//输入参数	: id:任务ID
//输出参数	: void
//函数功能	: 停止指定任务
//注意事项	:  
//=============================================================================================
void BSPTaskStop(u8 id)
{
	t_Task *ptask;

	if (id < TASK_MAX_NUM)
	{
		ptask = &sTaskTbl[id];
		BSPEnterCritical();				//关中断
		ptask->TaskEn  = 0;
		ptask->TaskRdy = 0;
		ptask->TaskCnt = 0xffffffff;
		BSPExitCritical();				//开中断
	}
}

//=============================================================================================
//函数名称	: void BSPTaskTickHandler(void)
//输入参数	: void  
//输出参数	: void
//函数功能	: 中断执行函数,时间轴总计数器计数，设置任务标志
//注意事项	: 1ms定时中断调用 
//=============================================================================================
void BSPTaskTickHandler(void)
{
	u8 i = 0;
	t_Task *ptask;

	sTaskSysTimer++;	//时间轴计数器计数
	sPowerOnTimer++;
    
	ptask = &sTaskTbl[0];
	
	//系统时间轴大于可运行时间时，为了防止越界，时间轴需要减去一段时间
	if(sTaskSysTimer >= TASK_MAX_USEABLE_TIME)
	{
		for(i = 0; i < TASK_MAX_NUM; i++)
		{
			if(ptask->TaskCnt != 0x0ffffffff)
			{
				ptask->TaskCnt -= TASK_MAX_USEABLE_TIME;
			}
			ptask++;
		}
		
		sTaskSysTimer -= TASK_MAX_USEABLE_TIME;
	}

	ptask = &sTaskTbl[0];
	for(i = 0; i < TASK_MAX_NUM; i++)
	{
		if(ptask->TaskEn == 1)
		{
			//任务的执行时间到了，就绪标志置1
			if(ptask->TaskCnt <= sTaskSysTimer)
			{
				ptask->TaskRdy = 1;
			}
		}
		ptask++;
	}
}

//=============================================================================================
//函数名称	: void BSPTaskProcess (void)
//输入参数	: *p:参数格式需要
//输出参数	: void
//函数功能	: 负责检测各任务是否启动,并执行相应任务函数 
//注意事项	: main函数调用 
//=============================================================================================
void BSPTaskProcess(void)
{
	u8 i = 0;
	u32 temp = 0;
	t_Task *ptask;
	void *parg;
	void (*pfunc)(void *);
	
	pfunc = (void (*)(void *))0;
	parg  = (void *)0;

	ptask = &sTaskTbl[0];

	BSPEnterCritical();									//禁止中断
	for(i = 0; i < TASK_MAX_NUM; i++)
	{ 
		if((ptask->TaskRdy == 1) 						//任务准备就绪
			&&(ptask->TaskEn == 1))						//任务使能
		{
			if(sTaskSysTimer > ptask->TaskCnt)          //达到执行延时
			{
				temp = sTaskSysTimer - ptask->TaskCnt;	//记录最大延迟
			}
			else
			{
				;
			}

			if(temp > sTaskDelayMax)                    //求最大系统延迟
			{
				sTaskDelayMax = temp;
			}

			pfunc = ptask->TaskFunc;
			parg  = ptask->TaskFuncArg;
			ptask->TaskCnt = 0xffffffff;                //清除下次执行延时
			ptask->TaskRdy = 0;							//清除就绪标志
			ptask->TaskEn  = 2;							//进入执行状态

			if(pfunc != (void (*)(void *))0)            //不是空函数
			{  
				BSPExitCritical();						//打开中断
				(*pfunc)(parg);                       	//执行任务对应函数
				BSPEnterCritical();						//禁止中断
			}

			if(ptask->TaskCnt == 0xffffffff)			//如果下次延时没有被设置,则默认为停止
			{
				ptask->TaskEn = 0;               		//禁止任务
			}
			else
			{
				ptask->TaskEn = 1;						//使能任务
			}
			BSPExitCritical();							//打开中断
			return;										//退出函数(一次只执行一个任务)，优先级高先执行
		}

		ptask++;
	}

	BSPExitCritical();
	return;	
}

//=============================================================================================
//函数名称	: u32 BSPTaskGetSysTimer(void)
//输入参数	: void
//输出参数	: 上电时间(1s)
//函数功能	: 获取任电时间
//注意事项	:
//=============================================================================================
u32 BSPTaskGetSysTimer(void)
{
	return(sPowerOnTimer / 1000);
}

//=============================================================================================
//函数名称	: void BSPTaskClrSysTimer(void)
//输入参数	: void
//输出参数	: 清除上电时间(1s)
//函数功能	: 清除上电时间
//注意事项	:
//=============================================================================================
void BSPTaskClrSysTimer(void)
{
	sPowerOnTimer = 0;
}

//=============================================================================================
//函数名称	: u32 BSPTaskGetTaskNum(void)
//输入参数	: void  
//输出参数	: void
//函数功能	: 获取任务个数
//注意事项	:  
//=============================================================================================
u32 BSPTaskGetTaskNum(void)
{
	return(sTaskRunNum);
}

//=============================================================================================
//函数名称	: u8 BSPTaskGetIsTaskStop(u8 id)
//输入参数	: void  
//输出参数	: 0:任务停止；1：任务运行
//函数功能	: 获取任务是否被停止
//注意事项	:  
//=============================================================================================
u8 BSPTaskGetIsTaskStop(u8 id)
{
	t_Task *ptask;

	if (id < TASK_MAX_NUM)
	{
		ptask = &sTaskTbl[id];
        return ptask->TaskEn;
	}
    return 0;
}

/*****************************************end of BSPTask.c*****************************************/
