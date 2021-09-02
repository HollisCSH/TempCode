//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: CurrIntegral.c
//创建人  	: Handry
//创建日期	:
//描述	    : 电流积分模块
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
#include "CurrIntegral.h"
#include "SocSlideShow.h"
#include "SocEepHook.h"
#include "DataDeal.h"
#include "SysState.h"
#include "DTCheck.h"
#include "DTTimer.h"
#include "DateTime.h"
#include "CanComm.h"
#include "ShutDownMode.h"
#include "BSPTimer.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
static t_CAPINT sCapInt = 
{
    0,      //充电电量积分量(单位:10mA1ms)
    0,      //放电电量积分量(单位:10mA1ms)
    0,      //充电电量积分量(单位:1mAH)
    0,      //放电电量积分量(单位:1mAH)
    0       //充放电电量变化量(单位:1mAH)
};

static t_ENERINT sEnerInt = 
{
	0,      //充电电能积分量(单位:1w1ms,即1AV1ms)
	0,      //放电电能积分量(单位:1w1ms,即1AV1ms)
	0,      //充电电能积分量(单位:1WH)
	0,      //放电电能积分量(单位:1WH)
    0       //充放电电能变化量(单位:1WH)
};

static t_HISCAP sHisCap = 
{
    0,      //历史充电电量(单位:mAH)
    0       //历史放电电量(单位:mAH)
};

static t_SLEEPCAP sSlpCap = 
{
    0,      //静置起始时间
    0.0,    //静置时长
    0.0,    //静置消耗容量
};

//=============================================================================================
//声明静态函数
//=============================================================================================

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	:  void CurrentIntInit()
//输入参数	:  void
//输出参数	:  void
//函数功能	:  积分任务初始化函数
//注意事项	:  
//=============================================================================================
void CurrentIntInit(void)
{
    sSlpCap.isSlpCap = False;
    sSlpCap.SlpInte = 0.0;
    sSlpCap.SlpCap = 0.0;
}    

//=============================================================================================
//函数名称	:  void CurrentIntTask()
//输入参数	:  void
//输出参数	:  void
//函数功能	:  积分任务
//注意事项	:  10ms调用
//=============================================================================================
void CurrentIntTask(void)
{
    s32 realCurr10mA = 0;//电流
    u16 tolvolt = 0;     //电压
    static u32 sLastTimer = 0;
    u8 factor = 0;  //调整系数
    
    #ifdef ENABLE_CALC_STANDBY_CURR
    static u8 cnt = 0;   //用于移除静置
    #endif
    
    if((0 == DataDealGetBMSDataRdy(0x01)) || (DT_F == DTTimerGetState(DT_ID_AFE_FAULT)))    //AFE、电量计数据已准备好,或者中途发送AFE故障，则不进行积分
    {
    	return;
    }
    
    //增加调整系数，用于修正误差
    if(sLastTimer > 0)
    {
        factor = gTimer1ms - sLastTimer;
        if(factor >= 50)
            factor = 50;
        else if(factor <= 1)
            factor = 2;
    }
    else
    {
        factor = 10;
    }
    
    realCurr10mA = gBatteryInfo.Data.Curr * factor / 10;                 
    tolvolt = gBatteryInfo.Data.TolVolt;

    if(realCurr10mA <= (0 - (s16)CURR_INT_IDLE_POINT))   //放电积分
    {
        DchgCapIntTask(0 - realCurr10mA);
        DchgEnerIntTask((0 - realCurr10mA),tolvolt);
    }
    else if(realCurr10mA >= (s16)CURR_INT_IDLE_POINT)   //充电积分
    {
        ChgCapIntTask(realCurr10mA);
        ChgEnerIntTask(realCurr10mA, tolvolt);
    }     
    else if((True == sSlpCap.isSlpCap) && (True == DataDealGetBMSDataRdy(0x08)))   //静置积分
    {
        CurrIntegralEndSleep();
        SocSetIsSlideShow(False);   //不平滑显示了，直接显示真实SOC
    }
    
    #ifdef ENABLE_CALC_STANDBY_CURR
    //0.1mA转换为10mA，减去板子消耗的电流
    cnt++;
    if(cnt >= TRAN_0_1MA_TO_10MA)
    {
        cnt = 0;
        if(True == gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule && True == gSHDNCond.IdleFlag.IdleFlagBit.EnterLightSleep)
        {
            DchgCapIntTask(WORK_CURR_WITH_SLEEP_EXMODULE);
            DchgEnerIntTask(WORK_CURR_WITH_SLEEP_EXMODULE,tolvolt);         
        }
        else if(True == gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule && False == gSHDNCond.IdleFlag.IdleFlagBit.EnterLightSleep)
        {
            DchgCapIntTask(WORK_CURR_WITH_EXMODULE);
            DchgEnerIntTask(WORK_CURR_WITH_EXMODULE,tolvolt);         
        }
        else
        {
            DchgCapIntTask(WORK_PWR_CURR);
            DchgEnerIntTask(WORK_PWR_CURR,tolvolt);    
        }
    }
    #endif
    
    sLastTimer = gTimer1ms;
}

//=============================================================================================
//函数名称	:  void HisCapCalcTask(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  历史充放电电量计算任务
//注意事项	:  100ms调用
//=============================================================================================
void HisCapCalcTask(void)
{
    u32 chgCap = 0;                             //充电电量(1mAH)
    u32 dchgCap = 0;                            //放电电量(1mAH)
    e_ChgDchgStatus nowState = eChgDchg_Idle;   //当前充放电状态
    u32 chgChangCap = 0;                        //充电电量变化量(1mAH)
    u32 dchgChangCap = 0;                       //放电电量变化量(1mAH)
    static e_ChgDchgStatus sHisState = eChgDchg_Idle;                   //历史充放电状态
    static u32 sHisChgCap = 0;                  //记录上次充电电量(1mAH)
    static u32 sHisDchgCap = 0;                 //记录上次放电电量(1mAH)
    static u32 sEepChgCap = 0;                  //上次写EEP充电电量(1mAH)
    static u32 sEepDchgCap = 0;                 //上次写EEP放电电量(1mAH)
    
    chgCap = GetChgIntCap();
    dchgCap = GetDchgIntCap();
    nowState = SysStateGetChgDchgSta();
    
    if(chgCap > sHisChgCap)
    {
        chgChangCap = chgCap - sHisChgCap;
    }
    else
    {
        chgChangCap = 0;        //历史值错误则抛弃本次计算值
    }
    
    if(dchgCap > sHisDchgCap)
    {
        dchgChangCap = dchgCap - sHisDchgCap;
    }
    else
    {
        dchgChangCap = 0;        //历史值错误则抛弃本次计算值
    }
    
    sHisCap.HisChgCap += chgChangCap;
    sHisCap.HisDchgCap += dchgChangCap;
    
    if((sHisCap.HisChgCap >= (sEepChgCap + HISCAP_CHANG_TO_WRITE_EEP))   //达到写EEP变化值
        || ((sHisCap.HisChgCap > sEepChgCap)                             //与EEP值不一致
        && (eChgDchg_Idle == nowState) && (eChgDchg_Idle != sHisState))) //由充放电转为待机
    {
        //电池历史充电电量写EEP
        SocWriteEepValueByAddr32(e_SOC_HisChgCap_Addr,sHisCap.HisChgCap);
        sEepChgCap = sHisCap.HisChgCap;
        if(SOC_CONFIG_FLAG != SocReadEepValueByAddr16(e_SOC_HisSave_Flag))
        {
            SocWriteEepValueByAddr16(e_SOC_HisSave_Flag,SOC_CONFIG_FLAG);
        }
    }
    
    if((sHisCap.HisDchgCap >= (sEepDchgCap + HISCAP_CHANG_TO_WRITE_EEP)) //达到写EEP变化值
        || ((sHisCap.HisDchgCap > sEepDchgCap)                           //与EEP值不一致
        && (eChgDchg_Idle == nowState) && (eChgDchg_Idle != sHisState))) //由充放电转为待机
    {
        //电池串历史放电电量写EEP
        SocWriteEepValueByAddr32(e_SOC_HisDchgCap_Addr,sHisCap.HisDchgCap);
        sEepDchgCap = sHisCap.HisDchgCap;
        if(SOC_CONFIG_FLAG != SocReadEepValueByAddr16(e_SOC_HisSave_Flag))
        {
            SocWriteEepValueByAddr16(e_SOC_HisSave_Flag,SOC_CONFIG_FLAG);
        }        
    }
    
    sHisChgCap = chgCap;
    sHisDchgCap = dchgCap;
    sHisState = nowState;
}

//=============================================================================================
//函数名称	:  void ChgCapIntTask(u32 curr)
//输入参数	:  curr:采样电流值(单位10mA)
//输出参数	:  void
//函数功能	:  充电电量积分任务
//注意事项	:  10ms调用
//=============================================================================================
void ChgCapIntTask(u32 curr)
{
    sCapInt.ChgCap10ma10ms += curr;
    
    //计算充电电量变化量(正数)
    if(sCapInt.ChgCap10ma10ms >= TRAN_10MA10MS_TO_MAH)                                 //变化1mAH
    {
        sCapInt.ChangeCap1ma1h += (s32)(sCapInt.ChgCap10ma10ms / TRAN_10MA10MS_TO_MAH); //充电变化量递增
        
        sCapInt.ChgCap1ma1h += (sCapInt.ChgCap10ma10ms / TRAN_10MA10MS_TO_MAH);        //转化为1mAH
        
        sCapInt.ChgCap10ma10ms %= TRAN_10MA10MS_TO_MAH;    //变化小于1mAH的其他部分，让其继续积分
    }
}

//=============================================================================================
//函数名称	:  void DchgCapIntTask(u32 curr)
//输入参数	:  curr:采样电流值(单位10mA)
//输出参数	:  void
//函数功能	:  放电电量积分任务
//注意事项	:  10ms调用
//=============================================================================================
void DchgCapIntTask(u32 curr)
{
    sCapInt.DchgCap10ma10ms += curr;
    
    //计算放电电量变化量(负数)
    if(sCapInt.DchgCap10ma10ms >= TRAN_10MA10MS_TO_MAH)                                //变化1mAH
    {
        sCapInt.ChangeCap1ma1h -= (s32)(sCapInt.DchgCap10ma10ms / TRAN_10MA10MS_TO_MAH);//放电变化量递减
        
        sCapInt.DchgCap1ma1h += (sCapInt.DchgCap10ma10ms / TRAN_10MA10MS_TO_MAH);      //转化为1mAH
        
        sCapInt.DchgCap10ma10ms %= TRAN_10MA10MS_TO_MAH;    //变化小于1mAH的其他部分，让其继续积分
    }
}

//=============================================================================================
//函数名称	:  void ChgEnerIntTask(u32 curr, u16 volt)
//输入参数	:  curr:采样电流值(单位10mA), volt采样电压值(单位:10mV)
//输出参数	:  void
//函数功能	:  充电电能积分任务
//注意事项	:  10ms调用
//=============================================================================================
void ChgEnerIntTask(u32 curr, u16 volt)
{
    static u32 sChg1mw1ms = 0;
    
    sChg1mw1ms += (curr * (u32)volt);                                         //10mA*10mV*10ms = 1mAVms，将其转化为1mw1ms
    
    if(sChg1mw1ms >= TRAN_MW1MS_TO_WMS)                                      //达到1WMS
    {
        sEnerInt.ChgEner1w1ms += (sChg1mw1ms / TRAN_MW1MS_TO_WMS);           //转化为1WMS
        
        sChg1mw1ms %= TRAN_MW1MS_TO_WMS;
    }
    
    //计算充电电能变化量(正数)
    if(sEnerInt.ChgEner1w1ms >= TRAN_W1MS_TO_WH)                             //变化1WH
    {
        sEnerInt.ChangEner1w1h += (s32)(sEnerInt.ChgEner1w1ms / TRAN_W1MS_TO_WH);
        
        sEnerInt.ChgEner1w1h += (sEnerInt.ChgEner1w1ms / TRAN_W1MS_TO_WH);   //转化为1WH
        
        sEnerInt.ChgEner1w1ms %= TRAN_W1MS_TO_WH;
    }
}

//=============================================================================================
//函数名称	:  void DchgEnerIntTask(u32 curr, u16 volt)
//输入参数	:  curr:采样电流值(单位10mA), volt采样电压值(单位:10mV)
//输出参数	:  void
//函数功能	:  放电电能积分任务
//注意事项	:  10ms调用
//=============================================================================================
void DchgEnerIntTask(u32 curr, u16 volt)
{
    static u32 sDchg1mw1ms = 0;
    
    sDchg1mw1ms += (curr * (u32)volt);                                            //10mA*10mV*10ms = 1mAVms，将其转化为1mw1ms
    
    if(sDchg1mw1ms >= TRAN_MW1MS_TO_WMS)                                         //达到1WMS
    {
        sEnerInt.DchgEner1w1ms += (sDchg1mw1ms / TRAN_MW1MS_TO_WMS);             //转化为1WMS
        
        sDchg1mw1ms %= TRAN_MW1MS_TO_WMS;
    }
    
    //计算放电电能变化量(负数)
    if(sEnerInt.DchgEner1w1ms >= TRAN_W1MS_TO_WH)                                //变化1WH
    {
        sEnerInt.ChangEner1w1h -= (s32)(sEnerInt.DchgEner1w1ms / TRAN_W1MS_TO_WH);
        
        sEnerInt.DchgEner1w1h += (sEnerInt.DchgEner1w1ms / TRAN_W1MS_TO_WH);     //转化为1WH
        
        sEnerInt.DchgEner1w1ms %= TRAN_W1MS_TO_WH;
    }
}

//=============================================================================================
//函数名称	:  u32 GetChgIntCap(void)
//输入参数	:  void
//输出参数	:  充电积分电量值(单位:1mAH)
//函数功能	:  获取充电积分电量值
//注意事项	:  
//=============================================================================================
u32 GetChgIntCap(void)
{
    return(sCapInt.ChgCap1ma1h);
}

//=============================================================================================
//函数名称	:  u32 GetDchgIntCap(void)
//输入参数	:  void
//输出参数	:  放电积分电量值(单位:1mAH)
//函数功能	:  获取放电积分电量值
//注意事项	:  
//=============================================================================================
u32 GetDchgIntCap(void)
{
    return(sCapInt.DchgCap1ma1h);
}

//=============================================================================================
//函数名称	:  s32 GetChgDchgChangCap(void)
//输入参数	:  void
//输出参数	:  充放电电量变化值(单位:1mAH,正数为充电量,负数为放电量)
//函数功能	:  获取充放电电量变化值
//注意事项	:  
//=============================================================================================
s32 GetChgDchgChangCap(void)
{
    return(sCapInt.ChangeCap1ma1h);
}

//=============================================================================================
//函数名称	:  u32 GetChgIntEner(void)
//输入参数	:  void
//输出参数	:  充电积分电能值(单位:1WH)
//函数功能	:  获取充电积分电能值
//注意事项	:  
//=============================================================================================
u32 GetChgIntEner(void)
{
    return(sEnerInt.ChgEner1w1h);
}

//=============================================================================================
//函数名称	:  u32 GetDchgIntEner(void)
//输入参数	:  void
//输出参数	:  放电积分电能值(单位:1WH)
//函数功能	:  获取放电积分电能值
//注意事项	:  
//=============================================================================================
u32 GetDchgIntEner(void)
{
    return(sEnerInt.DchgEner1w1h);
}

//=============================================================================================
//函数名称	:  s32 GetChgDchgChangEner(void)
//输入参数	:  void
//输出参数	:  充放电电能变化值(单位:1WH,正数为充电能,负数为放电能)
//函数功能	:  获取充放电电能变化值
//注意事项	:  
//=============================================================================================
s32 GetChgDchgChangEner(void)
{
    return(sEnerInt.ChangEner1w1h);
}

//=============================================================================================
//函数名称	:  u32 GetHisChgCap(void)
//输入参数	:  void
//输出参数	:  历史充电电量(mAH)
//函数功能	:  获取历史充电电量
//注意事项	:  
//=============================================================================================
u32 GetHisChgCap(void)
{
    return (sHisCap.HisChgCap);
}

//=============================================================================================
//函数名称	:  u32 GetHisDchgCap(void)
//输入参数	:  void
//输出参数	:  历史充电电量(mAH)
//函数功能	:  获取历史放电电量
//注意事项	:  
//=============================================================================================
u32 GetHisDchgCap(void)
{
    return (sHisCap.HisDchgCap);
}

//=============================================================================================
//函数名称	:  u32 SetHisChgCap(u32 cap)
//输入参数	:  cap：写入的历史充电电量
//输出参数	:  void
//函数功能	:  写历史充电电量
//注意事项	:  
//=============================================================================================
void SetHisChgCap(u32 cap)
{
    if(0xFFFFFFFF != cap)
    {
        sHisCap.HisChgCap = cap;
    }
    else
    {
        sHisCap.HisChgCap = 0;
    }
}

//=============================================================================================
//函数名称	:  u32 SetHisDchgCap(u32 cap)
//输入参数	:  cap：写入的历史放电电量
//输出参数	:  void
//函数功能	:  写历史放电电量
//注意事项	:  
//=============================================================================================
void SetHisDchgCap(u32 cap)
{
    if(0xFFFFFFFF != cap)
    {    
        sHisCap.HisDchgCap = cap;
    }
    else
    {
        sHisCap.HisDchgCap = 0;
    }        
}

//=============================================================================================
//函数名称	:  void CurrIntegralStartSleep(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  开始睡眠，计算起始条件
//注意事项	:  
//=============================================================================================
void CurrIntegralStartSleep(void)
{
    sSlpCap.SlpStartTim = DateTime_GetSeconds(Null);    //获取起始时间

    sSlpCap.SlpInte = 0.0;
    sSlpCap.SlpCap = 0.0;
    
    sSlpCap.isSlpCap = True;
}

//=============================================================================================
//函数名称	:  void CurrIntegralEndSleep(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  结束睡眠，计算睡眠电量
//注意事项	:  
//=============================================================================================
void CurrIntegralEndSleep(void)
{
    u32 currInte;
    
    sSlpCap.SlpInte = (float)(((float)DateTime_GetSeconds(Null) - (float)sSlpCap.SlpStartTim) / 3600); //秒转换为小时
    
    //避免时间出错
    if(sSlpCap.SlpInte > 0)
    {
        sSlpCap.SlpCap = SLEEP_PWR_CURR * sSlpCap.SlpInte;   //转换为mAH
        
        currInte = (u32)sSlpCap.SlpCap * TRAN_10MA10MS_TO_MAH;   //转换为10ma10ms
        
        DchgCapIntTask(currInte);
    }
    
    sSlpCap.isSlpCap = False;
}   

/*****************************************end of CurrIntegral.c*****************************************/
