//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SocOCVCorr.c
//创建人  	: Handry
//创建日期	:
//描述	    : SOC OCV修正模块
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
#include "SocOCVCorr.h"
#include "SocCapCalc.h"
#include "CurrIntegral.h"
#include "SysState.h"
#include "DataDeal.h"
#include "SocCalcHook.h"
#include "SocEepHook.h"
#include "SocPointData.h"
#include "SocSlideShow.h"
#include "SocLTCompensate.h"
#include "DateTime.h"
#include "DTCheck.h"
#include "DTTimer.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
#if defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S) || defined(SY_PB_32000MAH_17S)
static const float gChangeOCVTab[SOC_T_LEN][SOC_V_LEN] =  //ΔSOC/ΔOCV表(1mV/1)
{
    {
    0.000000,0.200000,0.113636,0.166667,0.206897,0.285714,0.333333,0.428571,0.400000,0.315789,
    0.185185,0.139535,0.101695,0.115385,0.098039,0.111111,0.107143,0.100000,0.057143,
    },
    {
    0.000000,0.193548,0.116279,0.166667,0.181818,0.315789,0.333333,0.428571,0.375000,0.315789,
    0.185185,0.133333,0.103448,0.122449,0.098039,0.111111,0.107143,0.100000,0.058824,
    },
    {
    0.000000,0.111111,0.119048,0.150000,0.181818,0.200000,0.277778,0.400000,0.375000,0.315789,
    0.200000,0.146341,0.105263,0.125000,0.098039,0.111111,0.105263,0.098361,0.050000,
    },
    {
    0.000000,0.162162,0.121951,0.157895,0.193548,0.187500,0.250000,0.400000,0.375000,0.315789,
    0.192308,0.139535,0.109091,0.122449,0.098039,0.111111,0.107143,0.101695,0.054054,
    },
};
#elif defined(LFP_HL_25000MAH_16S)
static const float gChangeOCVTab[SOC_T_LEN][SOC_V_LEN] =  //ΔSOC/ΔOCV表(1mV/1)
{
    {
    0.000000,0.011990,0.020080,0.024510,0.038760,0.068493,0.131579,0.166667,0.217391,0.384615,
    0.500000,0.555556,0.625000,0.625000,0.625000,0.714286,0.833333,0.833333,1.666667,1.666667,
    0.018051,
    },
    {
    0.000000,0.007764,0.015576,0.036232,0.090909,0.172414,0.312500,0.454545,0.714286,0.625000,
    1.000000,1.250000,0.833333,0.625000,0.555556,0.625000,0.833333,1.000000,2.500000,2.500000,
    0.026042,
    },
    {
    0.000000,0.004583,0.058824,0.250000,0.200000,0.294118,0.312500,0.555556,1.000000,1.666667,
    5.000000,2.500000,1.250000,0.454545,0.357143,0.625000,1.666667,2.500000,5.000000,5.000000,
    0.021552,
    },
    {
    0.000000,0.004545,0.058140,0.263158,0.208333,0.312500,0.277778,0.357143,1.666667,2.500000,
    5.000000,1.666667,1.000000,0.263158,0.714286,2.500000,2.500000,5.000000,5.000000,2.500000,
    0.034722,
    },
};
#elif defined(LFP_TB_20000MAH_20S)
static const float gChangeOCVTab[SOC_T_LEN][SOC_V_LEN] =  //ΔSOC/ΔOCV表(1mV/1)
{
    {
    0.000000,1.000000,1.000000,0.833333,0.555556,0.555556,1.000000,1.666667,5.000000,20.000000,
    15.000000,10.000000,0.040650,
    },
    {
    0.000000,0.625000,10.000000,5.000000,1.666667,1.000000,0.454545,0.333333,0.833333,7.500000,
    15.000000,5.000000,0.076923,
    },
    {
    0.000000,0.013624,0.277778,0.227273,0.263158,0.500000,5.000000,0.769231,0.217391,5.000000,
    15.000000,2.500000,0.042735,
    },
    {
    0.000000,0.012987,0.042735,0.384615,0.172414,0.294118,0.263158,0.882353,2.500000,0.200000,
    2.500000,10.000000,0.133333,
    },
};
#else
static float gChangeOCVTab[SOC_T_LEN][SOC_V_LEN] =  //ΔSOC/ΔOCV表(1mV/1%)
{
    0
};
#endif

static t_OCVCrt sSocOCVCrt = {0};               //SOC修正结构

#if defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S) || defined(SY_PB_32000MAH_17S) || defined(LFP_HL_25000MAH_16S) || defined(LFP_TB_20000MAH_20S)
    
#else    
static u8 sInitExpFlag = 0;                     //初始化异常标志
#endif

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: static void SocOCVTabInit(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 自反馈SOC修正ΔOCV表初始化
//注意事项	: 
//=============================================================================================
static void SocOCVTabInit(void)
{
#if defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S) || defined(SY_PB_32000MAH_17S) || defined(LFP_HL_25000MAH_16S) || defined(LFP_TB_20000MAH_20S)
    
#else
    u16 temp16 = 0;
    u8 j = 0,k = 0;
    const u8 * socB = (void *)0;
    const u16 * OcvTab = (void *)0;

    if(sInitExpFlag == 1)                   //数据初始化异常
    {
        return;
    }
    
    //初始化不同温度的静置电压ΔSOC/ΔOCV表(1%/1mV)
    for(j = 0;j < SOC_T_LEN; j++)
    {
        socB = cTempOCVTab[j].SOCTab;
        OcvTab = cTempOCVTab[j].OCVTab;
        
        for(k = 1; k < SOC_V_LEN; k++)
        {
            if((OcvTab[k] < OcvTab[k - 1])
                || (socB[k] <= socB[k - 1]))
            {
                sInitExpFlag |= 1;
                return;  
            }
            
            temp16 = OcvTab[k] - OcvTab[k - 1];
            gChangeOCVTab[j][k] = ((float)(socB[k] - socB[k - 1]) / temp16);  
        }
    }
#endif
}

//=============================================================================================
//函数名称	: void SocOCVInit(void)
//输入参数	: void
//输出参数	: void
//函数功能	: SOC OCV修正方法模块初始化
//注意事项	: 
//=============================================================================================
void SocOCVInit(void)
{
    //SOC修正ΔOCV表初始化
    SocOCVTabInit();
    
    sSocOCVCrt.IsOcvCorr = False;
    sSocOCVCrt.IsOcvCorrPeroid = False;
}

//=============================================================================================
//函数名称	: static void SocOCVCalcSocB(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 推定SOCb计算
//注意事项	: 
//=============================================================================================
static void SocOCVCalcSocB(void)
{
    u8 socb = 0;
    u16 calcsoc = 0;
    u16 volt = 0;
    u16 tempvolt = 0;   //临时保存电压    
    float deltavolt = 0;  //压差
    float deltasoc = 0;   //根据斜率计算增加的SOC

    u8 voltIndex = 0; 
    s16 temp = 0;
    u8 tempIndex = 0;
    
    const u8 * socB = (void *)0;    
    const u16 *OcvTbl = (void *)0;
    
    temp = sSocOCVCrt.avgTemp;
    tempIndex = (u8)SOCSearchAscendingOCVPointTabs((s16)temp, cTempOCVTab, SOC_T_LEN);

    OcvTbl = cTempOCVTab[tempIndex].OCVTab;
    socB = cTempOCVTab[tempIndex].SOCTab;
    
    //推定平均电压下的SOCb 
    volt = sSocOCVCrt.avgVolt;
    voltIndex = (u8)SOCSearchAscendingTabs16((s16)volt, (const s16 *)OcvTbl, SOC_V_LEN);
    
    if(voltIndex > 0)   //获取斜率,根据斜率计算SOC
    {
        tempvolt = OcvTbl[voltIndex - 1];    //查表中对应的电压值
        sSocOCVCrt.avgOCV = gChangeOCVTab[tempIndex][voltIndex];    
        
        deltavolt = volt - tempvolt;
        deltasoc = deltavolt * sSocOCVCrt.avgOCV; 
        socb = socB[voltIndex - 1];      
        //SOC有效范围判断
        sSocOCVCrt.avgSocb = ((calcsoc = socb + (u16)deltasoc) >= 100)? 100:calcsoc;  
    }
    else
    {
        socb = socB[voltIndex]; 
        sSocOCVCrt.avgSocb = socb;
        sSocOCVCrt.avgOCV = gChangeOCVTab[tempIndex][voltIndex];    
    }    
    
    //推定最大SOCb 
    volt = sSocOCVCrt.maxVolt;
    voltIndex = (u8)SOCSearchAscendingTabs16((s16)volt, (const s16 *)OcvTbl, SOC_V_LEN);
    
    if(voltIndex > 0)   //获取斜率,根据斜率计算SOC
    {
        tempvolt = OcvTbl[voltIndex - 1];    //查表中对应的电压值
        sSocOCVCrt.maxOCV = gChangeOCVTab[tempIndex][voltIndex];    
        
        deltavolt = volt - tempvolt;
        deltasoc = deltavolt * sSocOCVCrt.maxOCV; 
        socb = socB[voltIndex - 1];      
        //SOC有效范围判断
        sSocOCVCrt.maxSocb = ((calcsoc = socb +  (u16)deltasoc) >= 100)? 100:calcsoc;  
    }
    else
    {
        socb = socB[voltIndex]; 
        sSocOCVCrt.maxSocb = socb;
        sSocOCVCrt.maxOCV = gChangeOCVTab[tempIndex][voltIndex];    
    }
    
    //推定最小SOCb   
    volt = sSocOCVCrt.minVolt;
    voltIndex = (u8)SOCSearchAscendingTabs16((s16)volt, (const s16 *)OcvTbl, SOC_V_LEN);
    
    if(voltIndex > 0)   //获取斜率,根据斜率计算SOC
    {
        tempvolt = OcvTbl[voltIndex - 1];    //查表中对应的电压值
        sSocOCVCrt.minOCV = gChangeOCVTab[tempIndex][voltIndex];    
        
        deltavolt = volt - tempvolt;
        deltasoc = deltavolt * sSocOCVCrt.minOCV; 
        socb = socB[voltIndex - 1];        
        //SOC有效范围判断
        sSocOCVCrt.minSocb = ((calcsoc = socb +  (u16)deltasoc) >= 100)? 100:calcsoc;  
    }
    else
    {
        socb = socB[voltIndex]; 
        sSocOCVCrt.minSocb = socb;
        sSocOCVCrt.minOCV = gChangeOCVTab[tempIndex][voltIndex];    
    }    
}

//=============================================================================================
//函数名称	: static void CrtCurrentSocPercent(u8 nowSoc, s32 chanSoc)
//输入参数	: nowSoc当前SOC chanSoc 需修正的SOC差 %
//输出参数	: void
//函数功能	: 修正当前SOC值
//注意事项	: 
//=============================================================================================
static void CrtCurrentSocPercent(u8 nowSoc, s32 chanSoc)
{
    s32 aimSoc = 0;

    aimSoc = (s32)nowSoc + chanSoc;
    
    if(aimSoc < 0)
    {
        aimSoc = 0;
    }
    else if(aimSoc > 100)
    {
        aimSoc = 100;
    }
    
    CorrNowCapBySoc(aimSoc * 10);
    SocClrEepFirstFlag();
    //第一次，写SOC配置标志
    SocWriteEepValueByAddr16(e_SOC_Config_Flag,SOC_CONFIG_FLAG); 
    #ifdef SOC_LT_COMPENSATE_ENABLE
    if(True == SocCompCrt.CompState.CompStateBit.IsNeedComp)
        SocWriteEepValueByAddr16(e_SOC_TolCap_Addr,SocCompCrt.LtcAllCap);
    else
        SocWriteEepValueByAddr16(e_SOC_TolCap_Addr,gSOCInfo.tolcap);
    #else
    SocWriteEepValueByAddr16(e_SOC_TolCap_Addr,gSOCInfo.tolcap);
    #endif  
}

//=============================================================================================
//函数名称	: SocOCVCorrSoc(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 目标SOC计算校正
//注意事项	: 
//=============================================================================================
static void SocOCVCorrSoc(void)
{
    u8 nowSoc = 0;
    u8 socb = 0;
    s32 changSoc = 0;
    s8 CorrRange = 0;
    
    nowSoc = (u8)(gSOCInfo.realSOC / 10);       //当前积分SOC
    
    if(nowSoc > sSocOCVCrt.maxSocb)       //比最大电压推出来的SOC还大
    {
        socb = sSocOCVCrt.maxSocb;
    }
    else if(nowSoc < sSocOCVCrt.minSocb)   //比最小电压推出来的SOC还小
    {
        socb = sSocOCVCrt.minSocb;
    }
    else
    {
        socb = sSocOCVCrt.avgSocb;          //两者中间则取平均值
    }    
    
    changSoc = (s32)socb - (s32)nowSoc;
    
    if(True == sSocOCVCrt.IsOcvCorr || True == SocGetEepFirstFlag())
    {
        sSocOCVCrt.IsOcvCorr = False;  //OCV修正结束
        
        //充满时，尽量保持100%
        CorrRange = (gBatteryInfo.Status.ProtSta == eProt_Full)? 2:1;
        
        //第一次修正，不管什么范围，都进行修正
       #if defined(LFP_TB_20000MAH_20S) | defined(LFP_HL_25000MAH_16S)
        if(False == SocGetEepFirstFlag())  //修正在1%内
        #else
        if(IsInside((s8)(0-CorrRange), changSoc, CorrRange) && False == SocGetEepFirstFlag())  //修正在1%内
        #endif  
        {
            ;
        }  
        else
        {
            CrtCurrentSocPercent(nowSoc, changSoc);  
            if((DT_N == DTTimerGetState(DT_ID_VL_LV1))) //避免欠压时直接修正了SOC
            {
                SocSetIsSlideShow(False);   //不平滑显示了，直接显示真实SOC
            }
        }
    }
    else if(True == sSocOCVCrt.IsOcvCorrPeroid)
    {
        //周期修正
        sSocOCVCrt.IsOcvCorrPeroid = False;
        
        #if defined(LFP_TB_20000MAH_20S) | defined(LFP_HL_25000MAH_16S)
        CorrRange = 50;
        #else
        CorrRange = 10;
        #endif
        
        if(IsInside((s8)(0-CorrRange), changSoc, CorrRange))  //修正在10%内
        {
            nowSoc = (u8)(gSOCInfo.displaySOC / 10);       //当前积分SOC
            if(nowSoc > sSocOCVCrt.maxSocb)       //比最大电压推出来的SOC还大
            {
                socb = sSocOCVCrt.maxSocb;
            }
            else if(nowSoc < sSocOCVCrt.minSocb)   //比最小电压推出来的SOC还小
            {
                socb = sSocOCVCrt.minSocb;
            }
            else
            {
                socb = sSocOCVCrt.avgSocb;          //两者中间则取平均值
            }    
            
            changSoc = (s32)socb - (s32)nowSoc;
            if(!IsInside((s8)(0-CorrRange), changSoc, CorrRange))
            {
                SocSetIsSlideShow(False);   //不平滑显示了，直接显示真实SOC
            }
        }  
        else
        {
            CrtCurrentSocPercent(nowSoc, changSoc);  
            SocSetIsSlideShow(False);   //不平滑显示了，直接显示真实SOC
        }        
    }
}

//=============================================================================================
//函数名称	: void SocOCVCalcTask(void)
//输入参数	: void
//输出参数	: void
//函数功能	: SOC OCV修正方法任务
//注意事项	: 1s
//=============================================================================================
void SocOCVCalcTask(void)
{
	s16 avgCurr = 0;			    //一段时间内的平均电流
    static u16 sTimer = 0;
	static u32 sumMaxV = 0;		    //一段时间内的最大电压值求和
	static u32 sumMinV = 0;		    //一段时间内的最小电压值求和
    static u32 sumAvgV = 0;         //一段时间内的平均电压值求和
	static s16 sumTemp = 0;			//一段时间内的温度求和
    
    #ifdef ENABLE_OCV_CORR_ALLTIME
    
    e_ChgDchgStatus NowState = eChgDchg_MAX;   
    static u8 FirstFlag = True;
    
    if(True == FirstFlag)
    {
        if(True == DataDealGetBMSDataRdy(0x08))
        {
            FirstFlag = False;
            sSocOCVCrt.OcvStartTim = DateTime_GetSeconds(Null);    //获取起始时间
        }
        else
        {
            return;
        }   
    }
    
    NowState = SysStateGetChgDchgSta(); //获取当前系统状态
    
    if(eChgDchg_Idle == NowState && False == sSocOCVCrt.IsOcvCorr)   //空闲状态，静置
    {        
        sSocOCVCrt.OcvInte = DateTime_GetSeconds(Null) - sSocOCVCrt.OcvStartTim; //秒
        
        //避免时间出错
        if(sSocOCVCrt.OcvInte > OCV_CORR_IDLE_TIME)
        {
            sSocOCVCrt.OcvStartTim = DateTime_GetSeconds(Null);    //获取起始时间，下一次OCV修正至少在OCV_CORR_IDLE_TIME小时后
            sSocOCVCrt.OcvInte = 0;
            sSocOCVCrt.IsOcvCorr = True;    //需要OCV修正
        }
        else if(0 == (sSocOCVCrt.OcvInte % OCV_CORR_PEROID_IDLE_TIME))  //每5秒进行一次OCV检查，确认是否偏差过大
        {
            sSocOCVCrt.IsOcvCorrPeroid = True;    //需要周期OCV修正
        }
    }
    else
    {
        sSocOCVCrt.OcvStartTim = DateTime_GetSeconds(Null);    //获取起始时间
        sSocOCVCrt.OcvInte = False;
        sSocOCVCrt.IsOcvCorrPeroid = False;
    }
    #else
    if(True == SocGetEepFirstFlag())
         sSocOCVCrt.IsOcvCorr = True;
    #endif
    
    if(0 == DataDealGetBMSDataRdy(0x35) //AFE数据已准备好,电压，温度特征值未准备好
        || ((False == SocGetEepFirstFlag()) //SOC不是第一次运行
            && (False == sSocOCVCrt.IsOcvCorr) && (False == sSocOCVCrt.IsOcvCorrPeroid))  //无需OCV修正
    )
    {
    	sSocOCVCrt.avgCurr = 0;
    	sumMaxV = 0;
    	sumMinV = 0;
        sumAvgV = 0;
    	sumTemp = 0;
    	sTimer = 0;    
        
        return;
    }    

	avgCurr = gBatteryInfo.Data.Curr;               //获取电流

	if((ABS(avgCurr, sSocOCVCrt.avgCurr) > 5))                      //电流波动大于50mA
	{
		sSocOCVCrt.avgCurr = avgCurr;
		sumMaxV = 0;
		sumMinV = 0;
        sumAvgV = 0;
		sumTemp = 0;
		sTimer = 0;
		return;
	}    
	sTimer++;

	if(sTimer <= SOC_OCV_CORR_PERIOD)
	{
		sumMaxV +=	gBatteryInfo.VoltChara.MaxVolt;		//获取平均最大电压
		sumMinV +=  gBatteryInfo.VoltChara.MinVolt;		//获取平均最小电压
        sumAvgV += gBatteryInfo.VoltChara.PerVolt;      //获取平均电压
		sumTemp += gBatteryInfo.TempChara.PerTemp;	    //获取平均温度值
	}    
	else
	{
		sSocOCVCrt.avgTemp = (s16)(sumTemp / SOC_OCV_CORR_PERIOD);	
    	
		sSocOCVCrt.maxVolt = (u16)(sumMaxV / SOC_OCV_CORR_PERIOD);
		sSocOCVCrt.minVolt = (u16)(sumMinV / SOC_OCV_CORR_PERIOD); 
        sSocOCVCrt.avgVolt = (u16)(sumAvgV / SOC_OCV_CORR_PERIOD);
        

		SocOCVCalcSocB();             //计算SOCb
		SocOCVCorrSoc();              //目标SOC计算校正

		sumMaxV = 0;
		sumMinV = 0;
        sumAvgV = 0;
		sumTemp = 0;
		sTimer = 0;
	}    
}

/*****************************************end of SocOCVCorr.c*****************************************/
