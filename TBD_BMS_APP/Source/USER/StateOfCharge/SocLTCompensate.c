//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SocLTCompensate.c
//创建人  	: Handry
//创建日期	:
//描述	    : SOC 低温补偿模块
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SocTypeDef.h"
#include "SocLTCompensate.h"
#include "SocOCVCorr.h"
#include "SocCapCalc.h"
#include "CurrIntegral.h"
#include "SysState.h"
#include "SocCalcHook.h"
#include "SocEepHook.h"
#include "SocPointData.h"
#include "SocSlideShow.h"
#include "DataDeal.h"
#include "NTC.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
static const t_ComPensatePoint gLTCompensateTab[SOC_LT_COMPENSATE_POINT_NUM] =  //补偿系数表
{
	{-200,750},	//-20℃ 系数75%		
	{-100,850},	//-10℃ 系数85%	
	{0,900},	//0℃ 系数90%	
};

static const float gSlopeLTCompensateTab[SOC_LT_COMPENSATE_POINT_NUM] =  //补偿系数斜率表(1/1℃)
{
    0.000000,100.000000,50.000000,
};

t_ComPensateCrt SocCompCrt = {0};	//SOC低温补偿结构

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void SocLTCompensateInit(void)
//输入参数	: void
//输出参数	: void
//函数功能	: SOC低温补偿模块初始化
//注意事项	: 
//=============================================================================================
void SocLTCompensateInit(void)
{
	SocCompCrt.LastCompTemp = 21;
	SocCompCrt.CompState.CompStateBit.IsFirstComp = True;
	/*
	u8 i = 0;

	printf("static const float gSlopeLTCompensateTab[SOC_LT_COMPENSATE_POINT_NUM] =  //补偿系数斜率表(1%/1℃)\n");
	printf("{\n");
	printf("    ");
	for (i = 1; i < SOC_LT_COMPENSATE_POINT_NUM; i++)
	{
		gSlopeLTCompensateTab[i] = (float)(gLTCompensateTab[i].factor - gLTCompensateTab[i - 1].factor); \
			(float)(gLTCompensateTab[i].temp - gLTCompensateTab[i - 1].temp);
		
	}
	for (i = 0; i < SOC_LT_COMPENSATE_POINT_NUM; i++)
	{
		printf("%f,", gSlopeLTCompensateTab[i]);
	}
	printf("\n};\n");
	*/
}

//=============================================================================================
//函数名称	: void SocLTCompensateEnterShDn(void)
//输入参数	: void
//输出参数	: void
//函数功能	: SOC低温补偿模块进入休眠处理任务
//注意事项	: 
//=============================================================================================
void SocLTCompensateEnterShDn(void)
{
	SocCompCrt.SumTemp = 0;
	SocCompCrt.Timer = 0;   	
}

//=============================================================================================
//函数名称	: void SocLTCompensateCalcFactor(void)
//输入参数	: void
//输出参数	: void
//函数功能	: SOC低温补偿系数计算
//注意事项	: 
//=============================================================================================
void SocLTCompensateCalcFactor(void)
{
	s16 temp;
	s16 tempTab[SOC_LT_COMPENSATE_POINT_NUM];
	s16 tempIndex;
	u16 factor;
	u16 factorTab[SOC_LT_COMPENSATE_POINT_NUM];
	s16 tempTemp;	//临时保存的温度
	float deltatemp = 0;	//温差
	float deltafactor= 0;   //根据斜率计算增加的系数

	temp = SocCompCrt.CompTemp;

	for (tempIndex = 0; tempIndex < SOC_LT_COMPENSATE_POINT_NUM; tempIndex++)
	{
		tempTab[tempIndex] = gLTCompensateTab[tempIndex].temp;
		factorTab[tempIndex] = gLTCompensateTab[tempIndex].factor;
	}

	tempIndex = (u8)SOCSearchAscendingTabs16((s16)temp, (const s16 *)tempTab, SOC_LT_COMPENSATE_POINT_NUM);

	if (tempIndex > 0)
	{
		tempTemp = tempTab[tempIndex - 1];	//查表中对应的温度值
		SocCompCrt.CompSlope = gSlopeLTCompensateTab[tempIndex];

		deltatemp = (float)temp - (float)tempTemp;
		deltafactor = deltatemp * SocCompCrt.CompSlope / 100;
		factor = factorTab[tempIndex - 1];
		//系数有效值判断
		SocCompCrt.CompFactor = ((factor + (u16)deltafactor) >= 1000) ? 1000: (factor + (u16)deltafactor);
	}
	else
	{
		factor = factorTab[tempIndex];
		SocCompCrt.CompFactor = factor;
		SocCompCrt.CompSlope = gSlopeLTCompensateTab[tempIndex];
	}

	//printf("Compensate factor is %d\n", CompCrt.CompFactor);
}

//=============================================================================================
//函数名称	: void SocLTCompensateTask(void)
//输入参数	: void
//输出参数	: void
//函数功能	: SOC低温补偿模块主任务
//注意事项	: 
//=============================================================================================
//s16 gtesttemp = 250;
void SocLTCompensateTask(void)
{
    #ifdef SOC_LT_COMPENSATE_ENABLE
    
    #else
    return;
    #endif
	//static s16 sumTemp = 0;			//一段时间内的温度求和
    //static u16 sTimer = 0;
	u32 ltcallcap = 0;
	uint8_t i = 0;

    if(0 == DataDealGetBMSDataRdy(0x35)) //AFE数据已准备好,电压，温度特征值未准备好
    {
    	SocCompCrt.SumTemp = 0;
    	SocCompCrt.Timer = 0;
        
        return;
    }
	
    /* 出现电池温度NTC采样断线或短路，不进行低温补偿*/
    if((gBatteryInfo.Addit.FuelTemp == NTC_TEMP_OFF)
        ||(gBatteryInfo.Addit.FuelTemp == NTC_TEMP_SHORT))
    {
        SocCompCrt.CompState.CompStateBit.IsLowTemp = False;
        SocCompCrt.CompState.CompStateBit.IsNeedComp = False;
        SocCompCrt.CompState.CompStateBit.IsCalFactor = False;
        SocCompCrt.CompFactor = 1000;
        SocCompCrt.CompSlope = 0.0;
        return;
    }
        
    for(i = 0;i < BAT_TEMP_NUM;i++)
    {
        if((gBatteryInfo.Data.CellTemp[i] == NTC_TEMP_OFF)
            ||(gBatteryInfo.Data.CellTemp[i] == NTC_TEMP_SHORT))
        {
        SocCompCrt.CompState.CompStateBit.IsLowTemp = False;
        SocCompCrt.CompState.CompStateBit.IsNeedComp = False;
        SocCompCrt.CompState.CompStateBit.IsCalFactor = False;
        SocCompCrt.CompFactor = 1000;
        SocCompCrt.CompSlope = 0.0;
        return;
        }				
    }
    
	SocCompCrt.Timer++;
	if (SocCompCrt.Timer <= SOC_LT_COMPENSATE_PERIOD)
	{
		SocCompCrt.SumTemp += gBatteryInfo.TempChara.MinTemp;	    	//获取最低温度值		
        //SocCompCrt.SumTemp += gtesttemp;
	}
	else
	{
		SocCompCrt.Timer = 0;
		SocCompCrt.CompTemp = (s16)(SocCompCrt.SumTemp / SOC_LT_COMPENSATE_PERIOD);
		SocCompCrt.SumTemp = 0;

		if(SocCompCrt.CompTemp <= SOC_LT_COMPENSATE_TEMP)
		{
			SocCompCrt.CompState.CompStateBit.IsLowTemp = True;
			//低温下温度幅度变化大于2℃，再重新计算系数
			if (ABS(SocCompCrt.LastCompTemp, SocCompCrt.CompTemp) > 20)
			{
				SocCompCrt.LastCompTemp = SocCompCrt.CompTemp;
				SocCompCrt.CompState.CompStateBit.IsCalFactor = True;
			}
		}
		else
		{
			SocCompCrt.LastCompTemp = 21;
			//大于恢复温度，则不进行低温补偿了
			if (SocCompCrt.CompTemp >= SOC_LT_COMPENSATE_RES_TEMP)
			{
				SocCompCrt.CompState.CompStateBit.IsLowTemp = False;
				SocCompCrt.CompState.CompStateBit.IsCalFactor = False;
			}
		}
	}

	//处于低温 并且是首次进入0℃低温或者低温下温度幅度变化大于2℃
	if (//(True == SocCompCrt.CompState.CompStateBit.IsRdyOCV) && 
		(True == SocCompCrt.CompState.CompStateBit.IsCalFactor) &&
		(True == SocCompCrt.CompState.CompStateBit.IsLowTemp))
	{
		SocLTCompensateCalcFactor();	//计算系数
		if (SocCompCrt.CompFactor > 0 && SocCompCrt.CompFactor <= 1000)
		{
			if(True == SocCompCrt.CompState.CompStateBit.IsFirstComp)
			{
				//先将低温补偿前的满充容量保存起来
				SocCompCrt.CompState.CompStateBit.IsFirstComp = False;
				SocCompCrt.LtcAllCap = GetTotalCap();
			}
			SocCompCrt.CompState.CompStateBit.IsCalFactor = False;
			SocCompCrt.CompState.CompStateBit.IsNeedComp = True;
			ltcallcap = SocCompCrt.LtcAllCap * SocCompCrt.CompFactor / 1000;
			LTCompensateCorrTotalCap(ltcallcap);
		}
	}
	else
	{
		//不处于低温环境
		if (False == SocCompCrt.CompState.CompStateBit.IsLowTemp)
		{
			SocCompCrt.LastCompTemp = 21;
			SocCompCrt.CompState.CompStateBit.IsNeedComp = False;
			SocCompCrt.CompState.CompStateBit.IsCalFactor = False;
			SocCompCrt.CompFactor = 1000;
			SocCompCrt.CompSlope = 0.0;

			if((0 != SocCompCrt.LtcAllCap) && (False == SocCompCrt.CompState.CompStateBit.IsFirstComp))
			{
				SocCompCrt.CompState.CompStateBit.IsFirstComp = True;
				//恢复低温补偿的容量
				LTCompensateCorrTotalCap(SocCompCrt.LtcAllCap);
			}
		}
	}
}

/*****************************************end of SocLTCompensate.c*****************************************/
