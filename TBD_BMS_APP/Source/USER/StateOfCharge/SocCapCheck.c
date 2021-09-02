//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SocCapCheck.c
//创建人  	: Handry
//创建日期	:
//描述	    : SOC容量修正模块
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
#include "SocCapCheck.h"
#include "SocCapCalc.h"
#include "SocEepHook.h"
#include "SocLTCompensate.h"
#include "CurrIntegral.h"
#include "DataDeal.h"
#include "SysState.h"
#include "DTCheck.h"
#include "DTTimer.h"
#include "DateTime.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
u32 sNowCheckCap = 0;        //当前检测到的容量
u8 sCapDropExp = False;      //容量衰减异常标志

static t_CapCheck sVHCheckPoint;    //记录高压点信息
static t_CapCheck sVLCheckPoint;    //记录低压点信息

//=============================================================================================
//声明静态函数
//=============================================================================================
//=============================================================================================
//函数名称	: static void ClrCapCheckInfo(void) 
//输入参数	: void  
//输出参数	: void
//函数功能	: 清除容量检测记录信息
//注意事项	: 
//=============================================================================================
static void ClrCapCheckInfo(void);

//=============================================================================================
//函数名称	: static u8 CheckHisCapClr(void) 
//输入参数	: void  
//输出参数	: False:未清零 True已清零
//函数功能	: 检测记录高低压点后历史电量是否清零
//注意事项	: 
//=============================================================================================
static u8 CheckHisCapClr(void);

//=============================================================================================
//函数名称	: static void ChgTotalCapCheck(void) 
//输入参数	: void
//输出参数	: void
//函数功能	: 充电总容量检测
//注意事项	:  
//=============================================================================================
static void ChgTotalCapCheck(void);

//=============================================================================================
//函数名称	: static void DchgTotalCapCheck(void) 
//输入参数	: void
//输出参数	: void
//函数功能	: 放电总容量检测
//注意事项	:  
//=============================================================================================
static void DchgTotalCapCheck(void);

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void TotalCapCheckInit(void) 
//输入参数	: void  
//输出参数	: void
//函数功能	: 总容量检测初始化
//注意事项	:  
//=============================================================================================
void TotalCapCheckInit(void)
{
    sCapDropExp = False;
    
    //读取记录的高低压点数据
    
    //标志有效，说明记录过高压点
    if(SOC_CONFIG_FLAG == SocReadEepValueByAddr16(e_SOC_VHCapChk_Flag))
    {
        //更新到RAM中
        sVHCheckPoint.valid = SocReadEepValueByAddr16(e_SOC_VHValid_Addr);
        
        sVHCheckPoint.chgCap = SocReadEepValueByAddr32(e_SOC_VHChgCap_Addr);
        sVHCheckPoint.dchgCap = SocReadEepValueByAddr32(e_SOC_VHDchgCap_Addr);
        sVHCheckPoint.allCap = SocReadEepValueByAddr32(e_SOC_VHAllCap_Addr);
        sVHCheckPoint.timer = SocReadEepValueByAddr32(e_SOC_VHTimer_Addr);
    }   
    //尚未记录过高压点
    else
    {
        //高压点
        sVHCheckPoint.valid = 0;
        sVHCheckPoint.chgCap = 0;
        sVHCheckPoint.dchgCap = 0;
        sVHCheckPoint.allCap = 0;
        sVHCheckPoint.timer = 0;
    }        
    
     //标志有效，说明记录过高压点
    if(SOC_CONFIG_FLAG == SocReadEepValueByAddr16(e_SOC_VLCapChk_Flag))
    {
        //更新到RAM中
        sVLCheckPoint.valid = SocReadEepValueByAddr16(e_SOC_VLValid_Addr);
        
        sVLCheckPoint.chgCap = SocReadEepValueByAddr32(e_SOC_VLChgCap_Addr);
        sVLCheckPoint.dchgCap = SocReadEepValueByAddr32(e_SOC_VLDchgCap_Addr);
        sVLCheckPoint.allCap = SocReadEepValueByAddr32(e_SOC_VLAllCap_Addr);
        sVLCheckPoint.timer = SocReadEepValueByAddr32(e_SOC_VLTimer_Addr);
    }   
    //尚未记录过高压点
    else
    {
        //低压点
        sVLCheckPoint.valid = 0;
        sVLCheckPoint.chgCap = 0;
        sVLCheckPoint.dchgCap = 0;
        sVLCheckPoint.allCap = 0;
        sVLCheckPoint.timer = 0; 
    }           
}

//=============================================================================================
//函数名称	: void TotalCapCheckTask(void) 
//输入参数	: void  
//输出参数	: void
//函数功能	: 电池包总容量检测任务 
//注意事项	: 1s
//=============================================================================================
void TotalCapCheckTask(void)
{
    static u8 sFlag = 0;
    e_ChgDchgStatus state = eChgDchg_MAX;
    static u32 sStartLTCSec = 0;

    if(True == SocCompCrt.CompState.CompStateBit.IsNeedComp)
    {
        //有过低压补偿则清除capcheck数据
        ClrCapCheckInfo();
        sStartLTCSec = DateTime_GetSeconds(Null);   //获取当前秒数
        return;
    }
    //有过低温补偿后，至少24小时之后才进行容量修正校准
    else if((DateTime_GetSeconds(Null) - sStartLTCSec) < 86400)
    {
        ClrCapCheckInfo();
        return;
    }
    
    if((False == CAP_CHECK_ENABLE)   //总容量检测未使能
        || (True == CheckHisCapClr()))  //历史电量被清零
    //    || (0 == DataDealGetBMSDataRdy(0x01)))  //电池数据就绪 屏蔽此处，目的是为了上电不清除高低压点
    {
        if(0 == sFlag)
        {
            ClrCapCheckInfo();                  //清除记录的容量检测信息，重新检测
            sFlag = 1;
        }
    }    
    else  
    {
        sFlag = 0;
        
        state =  SysStateGetChgDchgSta();           //充放电状态
//        
//        if(eChgDchg_Chg == state)                   //正在充电
//        {
            ChgTotalCapCheck();
//        }
//        else 
        if(eChgDchg_Dchg == state)             //正在放电
        {
            DchgTotalCapCheck();
        }
    }      
}

//=============================================================================================
//定义静态函数
//=============================================================================================
//=============================================================================================
//函数名称	: static void ClrCapCheckInfo(void) 
//输入参数	: void  
//输出参数	: void
//函数功能	: 清除容量检测记录信息
//注意事项	: 
//=============================================================================================
static void ClrCapCheckInfo(void)
{
    //高压点
    sVHCheckPoint.valid = False;
    sVHCheckPoint.chgCap = 0;
    sVHCheckPoint.dchgCap = 0;
    sVHCheckPoint.allCap = 0;
    sVHCheckPoint.timer = 0;
    
    //低压点
    sVLCheckPoint.valid = False;
    sVLCheckPoint.chgCap = 0;
    sVLCheckPoint.dchgCap = 0;
    sVLCheckPoint.allCap = 0;
    sVLCheckPoint.timer = 0;
}

//=============================================================================================
//函数名称	: static u8 CheckHisCapClr(void) 
//输入参数	: void  
//输出参数	: False:未清零 True已清零
//函数功能	: 检测记录高低压点后历史电量是否清零
//注意事项	: 
//=============================================================================================
static u8 CheckHisCapClr(void)
{
    u32 hisChgCap = 0;
    u32 hisDchgCap = 0;
    
    hisChgCap = GetHisChgCap(); 
    hisDchgCap = GetHisDchgCap();
    
    if(True == sVHCheckPoint.valid)                           //已记录高压点
    {  
        if((sVHCheckPoint.chgCap > hisChgCap)               //历史充电电量比记录值变小
            || (sVHCheckPoint.dchgCap > hisDchgCap))        //历史放电电量比记录值变小
        {
            return(True);                                      //历史电量已经过清零
        }
    }
    
    if(True == sVLCheckPoint.valid)
    {
        if((sVLCheckPoint.chgCap > hisChgCap)               //历史充电电量比记录值变小
            || (sVLCheckPoint.dchgCap > hisDchgCap))        //历史放电电量比记录值变小
        {
            return(True);                                      //历史电量已经过清零
        }
    }
    
    return(False);
}

//=============================================================================================
//定义静态函数
//=============================================================================================
//=============================================================================================
//函数名称	: static void ChgTotalCapCheck(void) 
//输入参数	: void
//输出参数	: void
//函数功能	: 充电总容量检测
//注意事项	:  
//=============================================================================================
static void ChgTotalCapCheck(void)
{
    static u8 saveflg = True;
    u32 allCap = 0;
    u32 chgCap = 0;
    u32 dchgCap = 0;
    u32 calcCap = 0;
    
    sVHCheckPoint.timer++;                                                      //计算充电时长
    
    //if((DT_F == DTTimerGetState(DT_ID_VH)))                                     //充电到高压
    if(gBatteryInfo.Status.ProtSta == eProt_Full)   //充满
    {
        if(True == saveflg)
        {
            //记录高压点信息
            sVHCheckPoint.chgCap = GetHisChgCap();                                  //高压时充电电量
            sVHCheckPoint.dchgCap = GetHisDchgCap();                                //高压时放电电量
            sVHCheckPoint.valid = True;                                             //高压记录点有效  
            
            saveflg = False;
            SocEepSaveVHCheckPoint(&sVHCheckPoint);                                 //记录高压点数据
        
             if(True == sVLCheckPoint.valid)                                        //已记录低压点
             {
                if(IsInside(CAP_CHECK_MIN_TIME, sVHCheckPoint.timer, CAP_CHECK_MAX_TIME))         //充电时长有效
                {
                    allCap = GetTotalCap();                                         //获得总容量(mAH)
                    chgCap = sVHCheckPoint.chgCap - sVLCheckPoint.chgCap;           //低压到高压过程的充电电量
                    dchgCap = sVHCheckPoint.dchgCap - sVLCheckPoint.dchgCap;        //低压到高压过程的放电电量(中间可能有放电过程)
                    
                    if(chgCap > dchgCap)                                            //计算容量正确(充电量大)
                    {
                        calcCap = chgCap - dchgCap;                                 //计算的实际充电容量(mAH)
                        
                        sNowCheckCap = calcCap;                                     //记录当前检测到的容量
                        
                        //计算充电容量与放电容量误差20%以内
                        if(1 == IsInside((sVLCheckPoint.allCap * 80), (calcCap * 100), (sVLCheckPoint.allCap * 120)))
                        {
                            sCapDropExp = 0;
                            CorrTotalCap((calcCap + sVLCheckPoint.allCap) / 2);     //修正总容量
                            //为了避免剩余容量大于满充容量
                            CorrNowCapBySoc(1000);
                            SocWriteEepValueByAddr16(e_SOC_RemCap_Addr,GetRemainCap());
                        }
                        //计算充电容量与原容量误差20%以内
                        else if(1 == IsInside((allCap * 80), (calcCap * 100), (allCap * 120)))
                        {
                            sCapDropExp = 0;
                            CorrTotalCap((calcCap + allCap) / 2);                   //修正总容量
                            //为了避免剩余容量大于满充容量
                            CorrNowCapBySoc(1000);
                            SocWriteEepValueByAddr16(e_SOC_RemCap_Addr,GetRemainCap());
                        }
                        //本次计算总容量错误
                        else
                        {
                            calcCap = 0;                                            //总容量清零
                            sCapDropExp = 1;                                        //计算容量衰减异常
                        } 
                    }    
                }
                
                //记录高压点计算的总容量(无法计算或计算错误时为0)
                sVHCheckPoint.allCap = calcCap;                                     //更新计算到的充电容量
                SocWriteEepValueByAddr32(e_SOC_VHAllCap_Addr,sVHCheckPoint.allCap); //保存高压点充电容量
                
                //到高压点后则下一步等待低压，因此需清除低压点,防止再次高压时误修正
                sVLCheckPoint.valid = False;
                sVLCheckPoint.timer = 0;
                SocEepSaveVLCheckPoint(&sVLCheckPoint);     
            }  
        }         
    }
    else
    {
        saveflg = True;
    }    
}

//=============================================================================================
//函数名称	: static void DchgTotalCapCheck(void) 
//输入参数	: void
//输出参数	: void
//函数功能	: 放电总容量检测
//注意事项	:  
//=============================================================================================
static void DchgTotalCapCheck(void)
{
    static u8 saveflg = True;
    u32 allCap = 0;
    u32 chgCap = 0;
    u32 dchgCap = 0;
    u32 calcCap = 0;
    
    sVLCheckPoint.timer++;                                                      //计算放电时长
    
    //if((DT_F == DTTimerGetState(DT_ID_VL_LV1)) && gSOCInfo.realSOC < 100)  //放电到低压，防止大电流拉载导致的欠压
    if((gBatteryInfo.Fault.FaultInfo[2] & OPT_FAULT1_UV1 || gBatteryInfo.Fault.FaultInfo[2] & OPT_FAULT1_UV2) 
        && gSOCInfo.realSOC < 100)  //放电到低压，防止大电流拉载导致的欠压
    {
        //记录低压点
        sVLCheckPoint.chgCap = GetHisChgCap();                                  //低压时充电电量
        sVLCheckPoint.dchgCap = GetHisDchgCap();                                //低压时放电电量
        sVLCheckPoint.valid = True;                                             //低压记录点有效
        SocEepSaveVLCheckPoint(&sVLCheckPoint);                                 //保存低压点信息
        
        if(True == saveflg)
        {   
            saveflg = False;
            SocEepSaveVLCheckPoint(&sVLCheckPoint);                             //保存低压点信息
        
            if(True == sVHCheckPoint.valid)                                        //已记录高压点
            {
                if(IsInside(CAP_CHECK_MIN_TIME, sVLCheckPoint.timer, CAP_CHECK_MAX_TIME))  //放电时长有效
                {
                    allCap = GetTotalCap();                                         //获得总容量(mAH)
                    dchgCap = sVLCheckPoint.dchgCap - sVHCheckPoint.dchgCap;        //高压到低压过程的放电电量
                    chgCap = sVLCheckPoint.chgCap - sVHCheckPoint.chgCap;           //高压到低压过程的充电电量(中间可能有充电过程)
                    
                    if(dchgCap > chgCap)                                            //计算容量正确(放电量大)
                    {
                        calcCap = dchgCap - chgCap;                                 //计算的实际放电容量(mAH)

                        sNowCheckCap = calcCap;                                     //记录当前检测到的容量
                        
                        //计算放电容量与充电容量误差20%以内
                        if(1 == IsInside((sVHCheckPoint.allCap * 80), (calcCap * 100), (sVHCheckPoint.allCap * 120)))
                        {
                            sCapDropExp = 0;
                            CorrTotalCap((calcCap + sVHCheckPoint.allCap) / 2);     //修正总容量
                            CorrNowCapBySoc(gSOCInfo.displaySOC);
                            SocWriteEepValueByAddr16(e_SOC_RemCap_Addr,GetRemainCap());                            
                        }
                        //计算放电容量与原容量误差20%以内
                        else if(1 == IsInside((allCap * 80), (calcCap * 100), (allCap * 120)))
                        {
                            sCapDropExp = 0;
                            CorrTotalCap((calcCap + allCap) / 2);                   //修正总容量
                            CorrNowCapBySoc(gSOCInfo.displaySOC);
                            SocWriteEepValueByAddr16(e_SOC_RemCap_Addr,GetRemainCap());                            
                        }
                        //本次计算总容量错误
                        else
                        {
                            calcCap = 0;
                            sCapDropExp = 1;                                        //计算容量衰减异常
                        } 
                    }
                }
            
                //记录低压点计算的总容量(无法计算或计算错误时为0)
                sVLCheckPoint.allCap = calcCap;                                     //更新计算到的充电容量
                SocWriteEepValueByAddr32(e_SOC_VLAllCap_Addr,sVLCheckPoint.allCap); //保存低压点充电容量
            
                
                //到低压点后则下一步等待高压，因此需清除高压点,防止再次低压时误修正
                sVHCheckPoint.valid = False;
                sVHCheckPoint.timer = 0;
                SocEepSaveVHCheckPoint(&sVHCheckPoint);                   //保存高压点信息 
            }
        }
    }
    else
    {
        saveflg = True;
    }
}

//=============================================================================================
//函数名称	: t_CapCheck *TotalCapCheckGetPVHPoint(void)
//输入参数	: void
//输出参数	: t_CapCheck *
//函数功能	: 容量检测高压点指针获取
//注意事项	:  
//=============================================================================================
t_CapCheck *TotalCapCheckGetPVHPoint(void)
{
    return &sVHCheckPoint;
}

//=============================================================================================
//函数名称	: t_CapCheck *TotalCapCheckGetPVLPoint(void)
//输入参数	: void
//输出参数	: t_CapCheck *
//函数功能	: 容量检测低压点指针获取
//注意事项	:  
//=============================================================================================
t_CapCheck *TotalCapCheckGetPVLPoint(void)
{
    return &sVLCheckPoint;
}
/*****************************************end of SocCapCheck.c*****************************************/
