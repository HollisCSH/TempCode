//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: DTTimer.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 故障诊断计时器滤波实现文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "DTTimer.h"
#include "DTCheck.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
 t_DTTimer gDTTable[DT_ID_FIN];       //DT检测列表

//=============================================================================================
//声明静态函数
//=============================================================================================

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	:  void DTTimerInit(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  初始化所有自定义计数器
//注意事项	:  
//=============================================================================================
void DTTimerInit(void)
{
	u8 i = 0;

	for(i = 0; i < DT_CHECK_CFG_LEN;i++)
	{
		//初始化故障滤波器
		DTTimerConfig((e_DT_ID)gDTConfig[i].id, gDTConfig[i].dtlevel, DT_N,\
					  *gDTConfig[i].mattime * 1000, *gDTConfig[i].restime * 1000);//单位转化为ms

		if(1 == gDTConfig[i].enable)
		{
			//启动故障滤波计时器
			DTTimerStart((e_DT_ID)gDTConfig[i].id);
		}
		else
		{
			//停止故障滤波计时器
			DTTimerStop((e_DT_ID)gDTConfig[i].id);
		}

	}
}

//=============================================================================================
//函数名称	:  void DTTimerConfig(e_DT_ID id, e_DTLevel faultlevel, e_DTState initstate, u16 NtoFTime, u16 FtoNTime)
//输入参数	:  id(故障id), faultlevel(故障等级) initstate(初始状态), NtoFTime(故障成熟时间ms), FtoNTime(故障恢复时间ms)
//输出参数	:  void
//函数功能	:  配置DT数字信号滤波器
//注意事项	:  
//=============================================================================================
void DTTimerConfig(e_DT_ID id, e_DTLevel faultlevel, e_DTState initstate, u16 NtoFTime, u16 FtoNTime)
{
	t_DTTimer * pdt;
	
	if(id < DT_ID_FIN)
	{
		pdt               = &(gDTTable[id]);
		pdt->faultlevel   = faultlevel;
		pdt->currentstate = initstate;
		pdt->state        = initstate;
		pdt->NtoFTime     = NtoFTime / DT_TICK;     //故障成熟时间
		pdt->FtoNTime     = FtoNTime / DT_TICK;     //故障消除时间
		pdt->timer        = 0;
	}
}

//=============================================================================================
//函数名称	:  void DTTimerChangeConfig(e_DT_ID id, u16 NtoFTime, u16 FtoNTime)
//输入参数	:  id(故障id), NtoFTime(故障成熟时间ms), FtoNTime(故障恢复时间ms)
//输出参数	:  void
//函数功能	:  更改配置DT数字信号滤波器
//注意事项	:  
//=============================================================================================
void DTTimerChangeConfig(e_DT_ID id, u16 NtoFTime, u16 FtoNTime)
{
	t_DTTimer * pdt;
	
	if(id < DT_ID_FIN)
	{
		pdt               = &(gDTTable[id]);
		pdt->NtoFTime     = NtoFTime / DT_TICK;     //故障成熟时间
		pdt->FtoNTime     = FtoNTime / DT_TICK;     //故障消除时间
	}
}

//=============================================================================================
//函数名称	:  void DTTimerStart(e_DT_ID id)
//输入参数	:  id(故障id)
//输出参数	:  void
//函数功能	:  启动DT数字信号滤波器
//注意事项	:  
//=============================================================================================
void DTTimerStart(e_DT_ID id)
{
	if(id < DT_ID_FIN)              //输入参数正确
	{
		gDTTable[id].enable = 1;    //使能
	}
}

//=============================================================================================
//函数名称	:  void DTTimerStop(e_DT_ID id)
//输入参数	:  id(故障id)
//输出参数	:  void
//函数功能	:  停止DT数字信号滤波器
//注意事项	:  
//=============================================================================================
void DTTimerStop(e_DT_ID id)
{
	if(id < DT_ID_FIN)              //输入参数正确
	{
		gDTTable[id].enable = 0;    //禁止
	}
}

//=============================================================================================
//函数名称	:  void DTTimerSetState(e_DT_ID id, e_DTState state)
//输入参数	:  id(故障id), state(状态)
//输出参数	:  void
//函数功能	:  设置DT状态值
//注意事项	:  
//=============================================================================================
void DTTimerSetState(e_DT_ID id, e_DTState state)
{
	if(id < DT_ID_FIN)                          //输入参数错误
	{   
		gDTTable[id].state = state;             //设置状态值
	}
}

//=============================================================================================
//函数名称	:  void DTTimerSetCurrentState(e_DT_ID id, e_DTState state)
//输入参数	:  id(故障id), state(状态)
//输出参数	:  void
//函数功能	:  设置DT瞬态值
//注意事项	:  
//=============================================================================================
void DTTimerSetCurrentState(e_DT_ID id, e_DTState state)
{
	if(id < DT_ID_FIN)                          //输入参数正确
	{
		gDTTable[id].currentstate = state;      //设置瞬态值
	}
}

//=============================================================================================
//函数名称	: e_DTLevel DTTimerGetFaultLevel(e_DT_ID id)
//输入参数	:  id(故障id)
//输出参数	:  DT等级，0到4级故障等级
//函数功能	:  获得检测DT的异常等级
//注意事项	:  
//=============================================================================================
e_DTLevel DTTimerGetFaultLevel(e_DT_ID id)
{
	if(id >= DT_ID_FIN)				            //参数检测
	{
		return (e_DTLevel)0;
	}
    
	return(gDTTable[id].faultlevel);			    //返回DT的异常类型
}

//=============================================================================================
//函数名称	:  e_DTState DTTimerGetState(e_DT_ID id)
//输入参数	:  id(故障id)
//输出参数	:  0(正常) 1(故障)
//函数功能	:  获得当前滤波状态，将过渡态作为相应滤波态输出
//注意事项	:  N-F返回N F-N返回F
//=============================================================================================
e_DTState DTTimerGetState(e_DT_ID id)
{
	if(id >= DT_ID_FIN)						    //参数检查
	{
		return DT_END;
	}
      
	if(IsEqual(gDTTable[id].state, DT_NtoF))    //如果N to F 则返回N
	{
		return DT_N;
	}

	if(IsEqual(gDTTable[id].state, DT_FtoN))    //如果F to N 则返回F
	{
		return DT_F;
	}
	
	return(gDTTable[id].state);				    //返回两种情况
}

//=============================================================================================
//函数名称	:  e_DTState DTTimerGetTransitState(e_DT_ID id)
//输入参数	:  id(故障id)
//输出参数	:  当前过渡态
//函数功能	:  获得当前内部过渡状态
//注意事项	:  
//=============================================================================================
e_DTState DTTimerGetTransitState(e_DT_ID id)
{	
    if(id >= DT_ID_FIN)                         //输入参数正确
	{
		return DT_END;
	}

	return(gDTTable[id].state);
}

//=============================================================================================
//函数名称	:  e_DTState DTTimerGetCurrentState(e_DT_ID id)
//输入参数	:  id(故障id)
//输出参数	:  当前瞬态状态
//函数功能	:  获得当前瞬态状态
//注意事项	:  
//=============================================================================================
e_DTState DTTimerGetCurrentState(e_DT_ID id)
{
	if(id >= DT_ID_FIN)                         //输入参数错误
	{
		return DT_END;
	}

	return(gDTTable[id].currentstate);
}

//=============================================================================================
//函数名称	:  void DTTimerTask(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  DT检测数字信号滤波任务
//注意事项	:  100ms
//=============================================================================================
void DTTimerTask(void)
{
	u8 i = 0;
	t_DTTimer * pdt;

	for(i = 0; i < DT_CHECK_CFG_LEN; i++)                              //逐个检测指定DTid
	{
		pdt = &(gDTTable[gDTConfig[i].id]);

		if(IsEqual(pdt->enable, 1))                         //该DTid使能检测
		{
			switch(pdt->state)
			{
				case DT_N:                                  //正常状态
					if(IsEqual(pdt->currentstate, DT_F))    //如果其瞬态值是DT_F的时候，我们设置其为正常到故障的过渡态
					{
						pdt->state = DT_NtoF;
						pdt->timer = 0;
					}
					break;

				case DT_NtoF:                               //正常到故障的过渡态
					if(IsEqual(pdt->currentstate, DT_F))
					{
						pdt->timer++;
						if(pdt->timer >= pdt->NtoFTime)      //如果次数达到了设置值，则变化为故障状态
						{
							pdt->state = DT_F;
							pdt->timer = 0;
						}
					}

					if(IsEqual(pdt->currentstate, DT_N))    //过渡状态下如果其瞬态值是DT_N的时候，设置其为正常
					{
						pdt->state = DT_N;
						pdt->timer = 0;
					}
					break;

				case DT_F:                                  //故障状态
					if(IsEqual(pdt->currentstate, DT_N))    //如果其瞬态值是DT_N的时候，我们设置其为故障到正常的过渡态
					{
						pdt->state = DT_FtoN;
						pdt->timer = 0;
					}
					break;

				case DT_FtoN:                               //故障到正常的过渡态
					if(IsEqual(pdt->currentstate, DT_N))
					{
						pdt->timer++;
						if(pdt->timer >= pdt->FtoNTime)      //如果次数达到了设置值，则变化为正常状态
						{
							pdt->state = DT_N;
							pdt->timer = 0;
						}
					}

					if(IsEqual(pdt->currentstate, DT_F))    //过渡状态下如果其瞬态值是DT_F的时候，我们设置其为故障
					{
						pdt->state = DT_F;
						pdt->timer = 0;
					}
					break;

				default:
					break;
			}
		}
	}
}

/*****************************************end of DTTimer.c*****************************************/
