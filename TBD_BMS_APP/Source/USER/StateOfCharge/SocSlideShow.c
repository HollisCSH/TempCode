//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SocSlideShow.c
//创建人  	: Handry
//创建日期	:
//描述	    : SOC平滑显示模块
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SOCTypedef.h"
#include "SocCapCalc.h"
#include "CurrIntegral.h"
#include "SocSlideShow.h"
#include "SysState.h"
#include "DTCheck.h"
#include "DTTimer.h"
#include "SocEepHook.h"
#include "SocLTCompensate.h"
#include "DataDeal.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
static u16 sSocSlideCalc = 0xffff;       //平滑显示SOC中间计算值(千分之一)
static u8 sIsSlideShow = True;           //是否平滑显示

//=============================================================================================
//声明静态函数
//=============================================================================================
//=============================================================================================
//函数名称	:  static u16 CalcSocSlide(u16 aimSoc)
//输入参数	:  aimSoc:目标SOC
//输出参数	:  平滑后的SOC值
//函数功能	:  计算SOC平滑值
//注意事项	:  
//=============================================================================================
static u16 CalcSocSlide(u16 aimSoc);

//=============================================================================================
//函数名称	:  static void SocSlideToShow(u16 slowSoc)
//输入参数	:  slowSoc:平滑后待显示的值
//输出参数	:  void
//函数功能	:  将平滑值转化为显示值
//注意事项	: 
//=============================================================================================
static void SocSlideToShow(u16 slowSoc);

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	:  void SocSlideShowInit(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  SOC平滑显示初始化
//注意事项	:  
//=============================================================================================
void SocSlideShowInit(void)
{
    u16 soc = 0;
     
    soc = gSOCInfo.displaySOC;  //获取显示soc
    
    if(True == SocGetEepFirstFlag())    //第一次
    {
       soc = 0xffff; 
    }
    else if(soc > 1000)
    {
        soc = 1000;
    }
    
    sSocSlideCalc = soc;
}

//=============================================================================================
//函数名称	:  u8 SocGetIsSlideShow(void)
//输入参数	:  void
//输出参数	:  SOC平滑使能标志
//函数功能	:  SOC平滑使能标志获取
//注意事项	:  
//=============================================================================================
u8 SocGetIsSlideShow(void)
{
    return sIsSlideShow;
}

//=============================================================================================
//函数名称	:  void SocSetIsSlideShow(u8 IsSlideShow)
//输入参数	:  IsSlideShow：True:平滑显示；False：不平滑显示
//输出参数	:  void
//函数功能	:  SOC平滑使能设置
//注意事项	:  
//=============================================================================================
void SocSetIsSlideShow(u8 IsSlideShow)
{
    sIsSlideShow = IsSlideShow;
}

//=============================================================================================
//函数名称	:  void SocSlideShowTask(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  SOC平滑显示任务
//注意事项	:  100ms
//=============================================================================================
void SocSlideShowTask(void)
{
    u16 aimSoc = 0;
    u16 slowSoc = 0;
    
    if(0 == DataDealGetBMSDataRdy(0x35)     //AFE数据已准备好,电压，温度特征值未准备好
        || (True == SocGetEepFirstFlag())  //SOC不是第一次运行
    )    
    {
        //未更新OCV前，不进行计算
    	return;
    }  
    
    aimSoc = gSOCInfo.realSOC;
        
    if(0xffff == sSocSlideCalc)                  //平滑值未初始化
    {
        sIsSlideShow = True;
        slowSoc = aimSoc;
    }
    else if(True == sIsSlideShow)
    {
        if(ABS(sSocSlideCalc, aimSoc) <= 10)     //变化量小于1%，单位0.1%
        {
            slowSoc = aimSoc;                   //不需平滑
        }
        else
        {
            slowSoc = CalcSocSlide(aimSoc);      //计算SOC平滑值
        }   
    }
    else
    {
        sIsSlideShow = True;
        slowSoc = aimSoc;
    }
    
    SocSlideToShow(slowSoc);                     //将平滑值转化为显示值
}

//=============================================================================================
//定义静态函数
//=============================================================================================
//=============================================================================================
//函数名称	:  static u16 CalcSocSlide(u16 aimSoc)
//输入参数	:  aimSoc:目标SOC
//输出参数	:  平滑后的SOC值
//函数功能	:  计算SOC平滑值
//注意事项	:  
//=============================================================================================
static u16 CalcSocSlide(u16 aimSoc)
{
    u16 state = 0;                                  //充放电状态
    s32 inteCap = 0;                                //积分容量(1mAH)
    u32 changCap = 0;                               //容量变化量(1mAH)
    u32 totalCap = 0;                               //总容量(1mAH)
    u16 changSoc = 0;                               //变化SOC
    u16 copySoc = 0;                                //计算的SOC
    u16 stepSoc = 0;                                //SOC增长步长
    static u16 sHisState = 0;                       //历史充放电状态
    static s32 sHisCap = 0;                         //历史充放电量
    static u8 timer = 0;            

    copySoc = sSocSlideCalc;		                //备份平滑值
    totalCap = GetTotalCap();                       //总容量
    inteCap = GetChgDchgChangCap();                 //积分容量
    
    if(eChgDchg_Chg == SysStateGetChgDchgSta())
    {
        state = 1;                                  //充电状态
    }
    else if(eChgDchg_Dchg == SysStateGetChgDchgSta())
    {
        state = 2;                                  //放电状态
    }
    else
    {
        state = 0;
    }
    if(timer > 5)  //5s进来平滑一次
    {
        timer = 0;
        if((1 == state) && (1 == sHisState))            //持续充电(SOC增大)								    
        {
            if(copySoc < aimSoc)					    //平滑值低于实际值
            {
                stepSoc = (aimSoc - copySoc) / SOC_SLOW_SPEED;     //计算50s内增长到真实值步长
            
                copySoc += stepSoc;

                if(copySoc > aimSoc)				    //防止追赶过界
                {
                    copySoc = aimSoc;
                }
            }
            else if(copySoc > aimSoc)				    //平滑值高于真实值
            {
                changCap = ABS(sHisCap, inteCap);       //充电正数绝对值增大
                
                changSoc = (u16)(changCap * 1000 / totalCap);//实际变化的SOC
                
                copySoc += (changSoc / SOC_SLOW_SPEED); //平滑值放慢上升速度等待真实值
                if(copySoc > 1000)
                {
                    copySoc = 1000;
                }
            }
        }
        else if((2 == state) && (2 == sHisState))       //持续放电(SOC减小)								    
        {
            if(copySoc > aimSoc)					    //平滑值高于实际值
            {
                stepSoc = (copySoc - aimSoc) / 10;     //计算50s内下降到真实值步长
                
                if(copySoc > stepSoc)	                //防止减过界
                {
                    copySoc -= stepSoc;
                }
                else
                {
                    copySoc = 0;
                }

                if(copySoc < aimSoc)				    //平滑值不应该下降低于真实值
                {
                    copySoc = aimSoc;
                }
            }

            if(copySoc < aimSoc)					    //平滑值低于实际值
            {
                changCap = ABS(sHisCap, inteCap);       //放电正数值增大
                
                changSoc = (u16)(changCap * 1000 / totalCap);//实际变化的SOC
                
                changSoc = changSoc / SOC_SLOW_SPEED;   //平滑值放慢下降速度等待真实值

                if(copySoc > changSoc)				    //防止减过界
                {
                    copySoc -= changSoc;
                }
                else
                {
                    copySoc = 0;
                }
            }
        }
        
        sHisState = state;                              //记录历史状态
        sHisCap = inteCap;                              //记录此次积分容量
    }
    else
    {
        timer++;
    }
    return(copySoc);
}

//=============================================================================================
//函数名称	:  static void SocSlideToShow(u16 slowSoc)
//输入参数	:  slowSoc:平滑后待显示的值
//输出参数	:  void
//函数功能	:  将平滑值转化为显示值
//注意事项	: 
//=============================================================================================
static void SocSlideToShow(u16 slowSoc)
{
    static u16 sShowCopy = 0;
    
    if(DT_F == DTTimerGetState(DT_ID_VH))
    {
        sSocSlideCalc = 1000;               //高压直接显示为100%
    }
    else if(DT_F == DTTimerGetState(DT_ID_VL_LV2))
    {
        sSocSlideCalc = 10;                   //2级低压直接显示为1%
    }
//    else if(DT_F == DTTimerGetState(DT_ID_VL_LV1)
//            && (slowSoc > 20))
//    {
//        sSocSlideCalc = 20;                   //1级低压直接显示为3%
//    }
    else if((slowSoc >= 990)               //平滑值高于99%
        //&& (DT_N == DTTimerGetState(DT_ID_VH)))       //SOC高于99%但是未到高压报警
        && (gBatteryInfo.Status.ProtSta != eProt_Full)) //但是未到TAPER电流
    {
        if(sSocSlideCalc <= 990)           //显示值未高于99%
        {
            sSocSlideCalc = 990;           //保持显示99%
        }
        else if(slowSoc < sSocSlideCalc)     //平滑值变小(放电)
        {
            sSocSlideCalc = slowSoc;         //显示平滑值
        }
    }
    else if((slowSoc <= 20)                //平滑值低于2%
        && (DT_N == DTTimerGetState(DT_ID_VL_LV2)))       //SOC低于1%但是未到低压报警    
    {
        if(sSocSlideCalc >= 20)             //显示值未低于2%
        {
            sSocSlideCalc = 20;             //保持显示为2%
        }
        else if(slowSoc > sSocSlideCalc)     //平滑值变大(充电)
        {
            sSocSlideCalc = slowSoc;         //显示平滑值
        }
    }
    else
    {
        sSocSlideCalc = slowSoc;             //显示平滑值
    }
    
    gSOCInfo.displaySOC = sSocSlideCalc;
    DataDealSetBMSDataRdy(e_SoxDattaRdy);   //设置SOX数据准备好
    
    if((sShowCopy / 10) != (sSocSlideCalc / 10))       //变化超过1%
    {
        if(False == SocGetEepFirstFlag())
        {
            SocWriteEepValueByAddr16(e_SOC_DispSoc_Addr,gSOCInfo.displaySOC);  
            SocWriteEepValueByAddr16(e_SOC_Config_Flag,SOC_CONFIG_FLAG);
        }
        else
        {
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
            
            SocWriteEepValueByAddr16(e_SOC_RemCap_Addr,gSOCInfo.remaincap);            
            SocWriteEepValueByAddr16(e_SOC_DispSoc_Addr,gSOCInfo.displaySOC);
        }        
        sShowCopy =  sSocSlideCalc;
    } 
}

/*****************************************end of SocSlideShow.c*****************************************/
