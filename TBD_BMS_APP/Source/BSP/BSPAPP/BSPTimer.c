//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: BSPTimer.c
//创建人  	: Handry
//创建日期	:
//描述	    : 软定时器使用模块
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTimer.h"
#include "BSPCriticalZoom.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
u32 	gTimer1ms = 0;							    //全局定时器计数变量
static 	t_TIMER gTimerTbl[TIMER_MAX_NUM + 1];		//软定时器结构体数组

volatile unsigned int system_ms_tick = 0;           //sim868的tick定时器
//=============================================================================================
//定义接口函数
//=============================================================================================

//=============================================================================================
//函数名称	: u8 BSPTimerAllocation(u8* id)
//输入参数	: id:软定时器id
//输出参数	: 1:分配软定时器id  错误；0：分配软定时器id正确
//函数功能	: 分配软定时器id
//注意事项	: 分配软定时器id并对其进行初始化
//=============================================================================================
u8 BSPTimerAllocation(u8* id)
{
    u8 index = 0,ret;

    //BSPEnterCritical();    //关中断

    while((0 != gTimerTbl[index].use) && (TIMER_MAX_NUM > index))	//首先在队列中找到一个空隙
    {
        index++;
    }

    *id = index;

    if(TIMER_MAX_NUM <= index)	//有效性判断
    {
        ret = 1;
    }
    else
    {
        ret = 0;
        gTimerTbl[index].timeup  = 0;
        gTimerTbl[index].delay = 0;
        gTimerTbl[index].period = 0;
        gTimerTbl[index].callback = NULL ;
        gTimerTbl[index].use = 1;
    }

    //BSPExitCritical();    //开中断

    return ret;
}

//=============================================================================================
//函数名称	: u8 BSPTimerConfig(u8 id, u32 delay, u32 period, t_TIMER_CB callback)
//输入参数	: id:软定时器id delay:延时时间	period:软定时器周期	callback:软定时器回调函数
//输出参数	: 1:配置软定时器错误；0：配置软定时器正确
//函数功能	: 配置软定时器
//注意事项	:
//=============================================================================================
u8 BSPTimerConfig(u8 id, u32 delay, u32 period, t_TIMER_CB callback)
{
    u8 ret;

    //BSPEnterCritical();    //关中断

    if(TIMER_MAX_NUM <= id)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
        gTimerTbl[id].timeup  = 0;
        gTimerTbl[id].period = period;
        gTimerTbl[id].callback = callback;
        gTimerTbl[id].delay = delay;

    }

    //BSPExitCritical();    //开中断

    return ret;
}

//=============================================================================================
//函数名称	: u8 BSPTimerCheck(u8 id)
//输入参数	: id:软定时器id
//输出参数	: 1：延时完毕；0：还在延时过程中
//函数功能	: 检查软定时器是否延时完毕
//注意事项	:
//=============================================================================================
u8 BSPTimerCheck(u8 id)
{
    u8 ret;

    //BSPEnterCritical();    //关中断

    if(TIMER_MAX_NUM <= id)
    {
        ret = 0;
    }
    else
    {
        ret = gTimerTbl[id].timeup;

        if(0 < ret)
        {
            gTimerTbl[id].timeup--;
        }
    }

    //BSPExitCritical();    //开中断

    return ret;
}

//=============================================================================================
//函数名称	: u8 BSPTimerRelease(u8 id)
//输入参数	: id:软定时器id
//输出参数	: 1:释放软定时器错误；0：释放软定时器正确
//函数功能	: 释放软定时器
//注意事项	:
//=============================================================================================
u8 BSPTimerRelease(u8 id)
{
    u8 ret;

    //BSPEnterCritical();    //关中断

    if(TIMER_MAX_NUM <= id)
    {
        ret = 1;
    }
    else
    {
        gTimerTbl[id].use = 0;
        ret = 0;
    }
    //BSPExitCritical();    //开中断
    return ret;
}

//=============================================================================================
//函数名称	: void BSPTimerTickHandler(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 软定时器周期处理函数
//注意事项	: 放在定时器2的中断回调函数中调用
//=============================================================================================
void BSPTimerTickHandler(void)
{
    u8 index;

	gTimer1ms++;
    system_ms_tick++;
	for(index = 0; index < TIMER_MAX_NUM; index++)
	{
		if((0 != gTimerTbl[index].use) && (0 != gTimerTbl[index].delay))
		{
			if(0 == (--gTimerTbl[index].delay))
			{
				if(NULL == gTimerTbl[index].callback)
				{
					gTimerTbl[index].timeup++;
				}
				else
				{
					(*gTimerTbl[index].callback)(index);		//运行
				}
				gTimerTbl[index].delay = gTimerTbl[index].period;//保持周期为Period
			}
		}
	}
}

/*****************************************end of BSPTimer.c*****************************************/
