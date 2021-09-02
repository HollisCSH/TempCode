//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SocCapCalc.c
//创建人  	: Handry
//创建日期	:
//描述	    : SOC容量计算模块
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
#include "SocCapCalc.h"
#include "CurrIntegral.h"
#include "SysState.h"
#include "SocEepHook.h"
#include "DataDeal.h"
#include "SocLTCompensate.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
//static 
t_CAPFORM sCapForm = 
{
	CAP_ZERO_POINT + BAT_NORM_CAP_SOX / 2,          //当前容量(单位:1mAH)，初始默认值50%
	CAP_ZERO_POINT,                                 //底端容量(单位:1mAH)
	CAP_ZERO_POINT + BAT_NORM_CAP_SOX,              //顶端容量(单位:1mAH)
	BAT_NORM_CAP_SOX                                //标称容量(单位:1mAH)
};

t_SOC_INFO gSOCInfo;             //SOC信息

//=============================================================================================
//声明静态函数
//=============================================================================================
//=============================================================================================
//函数名称	:  static void CalcCurrentCap(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  计算当前容量
//注意事项	:  
//=============================================================================================
static void CalcCurrentCap(void);

//=============================================================================================
//函数名称	:  static void CalcBatterySOH(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  计算电池SOH(0.1%)
//注意事项	:  
//=============================================================================================
static void CalcBatterySOH(void);

//=============================================================================================
//函数名称	:  static void CalcBatterySOC(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  计算电池SOC值
//注意事项	:  
//=============================================================================================
static void CalcBatterySOC(void);

//=============================================================================================
//函数名称	:  static void CalcBatteryCycles(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  计算电池循环次数
//注意事项	:  
//=============================================================================================
static void CalcBatteryCycles(void);

//=============================================================================================
//函数名称	:  static void RecordCurrentCapToFLASH(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  记录当前剩余容量到FLASH
//注意事项	:  
//=============================================================================================
static void RecordCurrentCapToFLASH(void);

//=============================================================================================
//函数名称	:  static void RecordTotalCapToFLASH(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  记录电池总容量到FLASH
//注意事项	:  
//=============================================================================================
static void RecordTotalCapToFLASH(void);

//=============================================================================================
//函数名称	:  static void RefreshSOCInfoToBatteryInfo(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  更新容量信息到输出端口
//注意事项	:  
//=============================================================================================
static void RefreshSOCInfoToBatteryInfo(void);

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	:  void SocCalcInit(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  SOC计算模块初始化
//注意事项	:  
//=============================================================================================
void SocCalcInit(void)
{
    sCapForm.nowCap = CAP_ZERO_POINT + gSOCInfo.remaincap;
    sCapForm.topCap = CAP_ZERO_POINT + gSOCInfo.tolcap;
    sCapForm.baseCap = CAP_ZERO_POINT;
    sCapForm.standCap = BAT_NORM_CAP_SOX;
    
    //参数异常,或者之前未存储参数时使用默认值，默认S0C 50%
    if((sCapForm.nowCap > sCapForm.topCap) || (sCapForm.nowCap < sCapForm.baseCap))
    {
        sCapForm.nowCap = CAP_ZERO_POINT + BAT_NORM_CAP_SOX / 2;
        sCapForm.topCap = CAP_ZERO_POINT + BAT_NORM_CAP_SOX;
        sCapForm.baseCap = CAP_ZERO_POINT;
        sCapForm.standCap = BAT_NORM_CAP_SOX;
    }
    
    //总容量参数异常
    if(sCapForm.topCap == CAP_ZERO_POINT)
    {
        gSOCInfo.tolcap = BAT_NORM_CAP_SOX;
        sCapForm.topCap = CAP_ZERO_POINT + gSOCInfo.tolcap;
    }
}

//=============================================================================================
//函数名称	:  void SocCalcTask(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  SOC计算任务
//注意事项	:  10ms调用
//=============================================================================================
void SocCalcTask(void)
{
    static u8 sState = 0;
    
    if(0 == DataDealGetBMSDataRdy(0x35)     //AFE数据已准备好,电压，温度特征值未准备好
        || (True == SocGetEepFirstFlag())  //SOC不是第一次运行
    )    
    {
        //未更新OCV前，不进行计算
    	return;
    }  
    
    switch(sState)
    {
        case 0:
            CalcCurrentCap();       //计算剩余容量
            CalcBatterySOC();       //计算SOC          
            CalcBatterySOH();       //计算SOH
            CalcBatteryCycles();    //计算循环次数
            sState = 1;
            break;
        
        case 1:
            RecordCurrentCapToFLASH();      //写当前剩余容量到FLASH
            RefreshSOCInfoToBatteryInfo();  //更新容量信息到电池信息
            sState = 0;
            break;
        
        default:
            sState = 0;
            break;
    }        
}

//=============================================================================================
//函数名称	:  void CorrNowCapBySoc(u16 soc)
//输入参数	:  soc:目标SOC(0.1%)
//输出参数	:  void
//函数功能	:  根据SOC修正当前容量
//注意事项	:  
//=============================================================================================
void CorrNowCapBySoc(u16 soc)
{
    u32 nowCap = 0;
    u32 lowCap = 0;
    u32 allCap = 0;
    
    lowCap = sCapForm.baseCap;
    allCap = sCapForm.topCap;
    
    if(allCap <= lowCap)
    {
        return;
    }
    
    nowCap = (allCap - lowCap) * soc / 1000 + CAP_ZERO_POINT;
    
    if(nowCap > allCap)
    {
        nowCap = allCap;
    }
    else if(nowCap < lowCap)
    {
        nowCap = lowCap;
    }
    
    sCapForm.nowCap = nowCap;
    gSOCInfo.realSOC = soc;
    
    RecordCurrentCapToFLASH();      //写当前剩余容量到FLASH
    RefreshSOCInfoToBatteryInfo();  //更新容量信息到电池信息
}

//=============================================================================================
//函数名称	:  void CorrTotalCap(u32 cap)
//输入参数	:  cap:总容量(单位:1mAH)
//输出参数	:  void
//函数功能	:  修正总容量
//注意事项	:  
//=============================================================================================
void CorrTotalCap(u32 cap)
{
    u32 soc = 0;
    u32 nowCap = 0;
    u32 lowCap = 0;
    u32 allCap = 0;
    
    nowCap = sCapForm.nowCap;
    lowCap = sCapForm.baseCap;
    allCap = sCapForm.topCap;
    
    if(allCap <= lowCap)                //变量异常
    {
        cap = 0;
        nowCap = CAP_ZERO_POINT;
        allCap = CAP_ZERO_POINT;
    }
    else
    {
        if(nowCap >= allCap)            //变量异常
        {
            soc = 10000;
        }
        else if(nowCap <= lowCap)       //变量异常
        {
            soc = 0;
        }
        else
        {
            soc = (nowCap - lowCap) * 10000 / (allCap - lowCap);
        }
        
        nowCap = cap * soc / 10000 + CAP_ZERO_POINT;
        allCap = cap + CAP_ZERO_POINT;
        
        if(nowCap > allCap)             //计算当前容量超过总容量
        {
            nowCap = allCap;
        }
    }
    
    sCapForm.nowCap = nowCap;
    sCapForm.topCap = allCap;
    sCapForm.baseCap = CAP_ZERO_POINT;
    
    /* 新增记录容量校准时的循环次数 */
    gSOCInfo.Calibcycles = gSOCInfo.cycles;
    SocWriteEepValueByAddr16(e_SOC_CalibCyc_Addr,gSOCInfo.Calibcycles);
    SocWriteEepValueByAddr16(e_SOC_CalibCyc_Flag,SOC_CONFIG_FLAG);
    
    SocWriteEepValueByAddr16(e_SOC_TolCap_Addr,cap);
    RecordTotalCapToFLASH();              //写电池总容量到FLASH
    RecordCurrentCapToFLASH();            //写当前剩余容量到FLASH
    RefreshSOCInfoToBatteryInfo();        //更新容量信息到电池信息
}

//=============================================================================================
//函数名称	:  void LTCompensateCorrTotalCap(u32 cap)
//输入参数	:  cap:总容量(单位:1mAH)
//输出参数	:  void
//函数功能	:  低温补偿修正总容量
//注意事项	:  
//=============================================================================================
void LTCompensateCorrTotalCap(u32 cap)
{
    u32 soc = 0;
    u32 nowCap = 0;
    u32 lowCap = 0;
    u32 allCap = 0;
    
    nowCap = sCapForm.nowCap;
    lowCap = sCapForm.baseCap;
    allCap = sCapForm.topCap;
    
    if(allCap <= lowCap)                //变量异常
    {
        cap = 0;
        nowCap = CAP_ZERO_POINT;
        allCap = CAP_ZERO_POINT;
    }
    else
    {
        if(nowCap >= allCap)            //变量异常
        {
            soc = 10000;
        }
        else if(nowCap <= lowCap)       //变量异常
        {
            soc = 0;
        }
        else
        {
            soc = (nowCap - lowCap) * 10000 / (allCap - lowCap);
        }
        
        nowCap = cap * soc / 10000 + CAP_ZERO_POINT;
        allCap = cap + CAP_ZERO_POINT;
        
        if(nowCap > allCap)             //计算当前容量超过总容量
        {
            nowCap = allCap;
        }
    }
    
    sCapForm.nowCap = nowCap;
    sCapForm.topCap = allCap;
    sCapForm.baseCap = CAP_ZERO_POINT;
    
    //RecordTotalCapToFLASH();              //写电池总容量到FLASH
    //RecordCurrentCapToFLASH();            //写当前剩余容量到FLASH
    //RefreshSOCInfoToBatteryInfo();        //更新容量信息到电池信息
}

//=============================================================================================
//函数名称	:  u32 GetRemainCap(void)
//输入参数	:  void
//输出参数	:  剩余电量,即当前容量(单位:1mAH)
//函数功能	:  获取剩余电量
//注意事项	:  
//=============================================================================================
u32 GetRemainCap(void)
{
    return(sCapForm.nowCap - CAP_ZERO_POINT);
}

//=============================================================================================
//函数名称	:  u32 GetTotalCap(void)
//输入参数	:  void
//输出参数	:  总容量(单位:1mAH)
//函数功能	:  获取总容量
//注意事项	:  
//=============================================================================================
u32 GetTotalCap(void)
{
    return(sCapForm.topCap - CAP_ZERO_POINT);
}

//=============================================================================================
//函数名称	:  u32 GetStandCap(void)
//输入参数	:  void
//输出参数	:  标称容量(单位:1mAH)
//函数功能	:  获取标称容量
//注意事项	:  
//=============================================================================================
u32 GetStandCap(void)
{
    return(sCapForm.standCap);
}

//=============================================================================================
//定义静态函数
//=============================================================================================
//=============================================================================================
//函数名称	:  static void CalcCurrentCap(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  计算当前容量
//注意事项	:  
//=============================================================================================
static void CalcCurrentCap(void)
{
    s32 nowChangCap = 0;            //当前充放电电量变化值(单位:1mAH,正数为充电量,负数为放电量)
    s32 realChangCap = 0;           //实际充放电电量变化值(单位:1mAH,正数为充电量,负数为放电量)   
    static s32 sHisChangCap = 0;    //历史充放电电量变化值(单位:1mAH,正数为充电量,负数为放电量)
    
    nowChangCap = GetChgDchgChangCap();
    
    realChangCap = nowChangCap - sHisChangCap;                  //求间隔时间实际变化电量
    
    if((realChangCap > (s32)sCapForm.standCap)
        || (realChangCap < ((s32)0 - (s32)sCapForm.standCap)))  //计算异常(变化量超过标称容量)
    {
        realChangCap = 0;                                       //抛弃本次计算值
    }
        
    sCapForm.nowCap += realChangCap;                            //计算当前容量(即剩余容量,充电为正数容量增加，放电为负数容量减小)
    
    if(sCapForm.nowCap > sCapForm.topCap)                       //当前容量高端越界了
    {
        sCapForm.nowCap = sCapForm.topCap;
    }
    
    if(sCapForm.nowCap < sCapForm.baseCap)                      //当前容量低端越界了
    {
        sCapForm.nowCap = sCapForm.baseCap;
    }
    
    sHisChangCap = nowChangCap;
}

//=============================================================================================
//函数名称	:  static void CalcBatterySOC(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  计算电池SOC值
//注意事项	:  
//=============================================================================================
static void CalcBatterySOC(void)
{
    u32 soc = 0;
    u32 nowCap = 0;
    u32 lowCap = 0;
    u32 allCap = 0;
    
    nowCap = sCapForm.nowCap;
    lowCap = sCapForm.baseCap;
    allCap = sCapForm.topCap;
    
    if(nowCap >= allCap)
    {
        soc = 1000;
    }
    else if((nowCap <= lowCap) || (allCap <= lowCap))
    {
        soc = 0;
    }
    else
    {
        soc = (nowCap - lowCap) * 1000 / (allCap - lowCap);
        
        if(soc > 1000)
        {
            soc = 1000;
        }
    }
    
    //更新SOC
    gSOCInfo.realSOC = (u16)soc;
}

//=============================================================================================
//函数名称	:  static void CalcBatterySOH(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  计算电池SOH(0.1%)
//注意事项	:  
//=============================================================================================
static void CalcBatterySOH(void)
{
    u16 soh = 0;
    
    if(GetStandCap() > 1)
    {
        #ifdef SOC_LT_COMPENSATE_ENABLE
        //需要低温补偿则不更新SOH
        if(True == SocCompCrt.CompState.CompStateBit.IsNeedComp)
        {
            soh = gSOCInfo.soh;
        }
        else
        {
            soh = (u16)(GetTotalCap() * 1000 / GetStandCap());                //使用标称容量(mAH)
        }
        #else
        soh = (u16)(GetTotalCap() * 1000 / GetStandCap());                //使用标称容量(mAH)
        #endif

        if(soh > 1000)
        {
            soh = 1000;
        }
    }
    else
    {
        soh = 0;
    }
    
    //更新SOH
    gSOCInfo.soh = soh;
}

//=============================================================================================
//函数名称	:  static void CalcBatteryCycles(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  计算电池循环次数
//注意事项	:  
//=============================================================================================
static void CalcBatteryCycles(void)
{
    u16 cycle = 0;
    /*
    #ifdef SOC_LT_COMPENSATE_ENABLE
    if(True == SocCompCrt.CompState.CompStateBit.IsNeedComp)
        cycle = GetHisDchgCap() / SocCompCrt.LtcAllCap;
    else
        cycle = GetHisDchgCap() / gSOCInfo.tolcap;

    #else
    cycle = GetHisDchgCap() / gSOCInfo.tolcap;
    #endif
    */
    cycle = GetHisDchgCap() / GetStandCap();
    //更新循环次数
    gSOCInfo.cycles = cycle;
}

//=============================================================================================
//函数名称	:  static void RecordCurrentCapToFLASH(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  记录当前剩余容量到FLASH
//注意事项	:  
//=============================================================================================
static void RecordCurrentCapToFLASH(void)
{
    u32 nowCap = 0;
    u16 changeSoc = 0;
    static u16 sHisSOC = 0;     
    static u32 sHisCap = 0;                             //历史电量(单位:1mAH)
    
    nowCap = GetRemainCap();
    changeSoc = (u16)(ABS(gSOCInfo.realSOC,sHisSOC));
    
    if((changeSoc >= SOC_CHANGE_TO_WRITE_FLASH)            //达到写FLASH变化量
        || (ABS(nowCap, sHisCap) >= HISCAP_CHANG_TO_WRITE_EEP)) //与EEP值不一致
      //|| (0 == nowCap) || (GetTotalCap() == nowCap))     //充满或放空
    {
        sHisSOC = gSOCInfo.realSOC;
        sHisCap = nowCap;        
        SocWriteEepValueByAddr16(e_SOC_RemCap_Addr,nowCap);
    }
}

//=============================================================================================
//函数名称	:  static void RecordTotalCapToFLASH(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  记录电池总容量到FLASH
//注意事项	:  
//=============================================================================================
static void RecordTotalCapToFLASH(void)
{
    u32 allCap = 0;
    static u32 sHisCap = 0;                         //历史电量(单位:1mAH)
    
    allCap = sCapForm.topCap - sCapForm.baseCap;
    
    if(allCap != sHisCap)                           //总容量变化
    {
        SocWriteEepValueByAddr16(e_SOC_TolCap_Addr,allCap); //写EEP
        sHisCap = allCap;
    }
}

//=============================================================================================
//函数名称	:  static void RefreshSOCInfoToBatteryInfo(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  更新容量信息到输出端口
//注意事项	:  
//=============================================================================================
static void RefreshSOCInfoToBatteryInfo(void)
{
    gSOCInfo.tolcap = (u16)GetTotalCap();         //总容量
    gSOCInfo.remaincap = (u16)GetRemainCap();     //当前剩余容量
/*
    gBatteryInfo.Data.TolCap = (u16)GetTotalCap();         //总容量
    gBatteryInfo.Data.RemainCap = (u16)GetRemainCap();     //当前剩余容量

    gBatteryInfo.Data.RemainPower = (u16)(GetRemainCap() * gBatteryInfo.Data.TolVolt / 10000); //单位，0.1WH，1mAH * 10mV = 0.00001 WH
    
    gBatteryInfo.Data.SOC = gSOCInfo.displaySOC;    //显示SOC
    gBatteryInfo.Data.SOH = gSOCInfo.soh;           //SOH
    
    gBatteryInfo.Data.ChgDchgTimes = gSOCInfo.cycles;   //循环次数
*/
    gBatteryInfo.Data.TolCap = (u16)GetTotalCap();         //总容量
    gBatteryInfo.Data.RemainCap = (u16)GetRemainCap();     //当前剩余容量

    gBatteryInfo.Data.RemainPower = (u16)(GetRemainCap() * gBatteryInfo.Data.TolVolt / 10000); //单位，0.1WH，1mAH * 10mV = 0.00001 WH
    
    gBatteryInfo.Data.SOC = gSOCInfo.displaySOC;    //显示SOC
    gBatteryInfo.Data.SOH = gSOCInfo.soh;           //SOH
    
    gBatteryInfo.Data.ChgDchgTimes = gSOCInfo.cycles;   //循环次数    
    
}

/*****************************************end of SocCapCalc.c*****************************************/
