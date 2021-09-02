//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SocPointCorr.c
//创建人  	: Handry
//创建日期	:
//描述	    : SOC点修正模块
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
#include "SocPointCorr.h"
#include "SocCapCalc.h"
#include "CurrIntegral.h"
#include "SysState.h"
#include "DataDeal.h"
#include "SocPointData.h"
#include "SocSlideShow.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
static u8 sSocCorrTime[SOC_POINT_CRT_NUM];  //SOC修正点检测次数

//=============================================================================================
//声明静态函数
//=============================================================================================
//=============================================================================================
//函数名称	: u8 CheckPointCondition(u8 index, e_SocPointType type)
//输入参数	: index:第几个点 type:0温度 1电流 2电压 
//输出参数	: False:不合法 True:合法
//函数功能	: 判断该点温度，电流，电压是否合法
//注意事项	:  
//=============================================================================================
static u8 CheckPointCondition(u8 index, e_SocPointType type);

//=============================================================================================
//函数名称	: static void SocPointCorHandle(u16 pointSoc)
//输入参数	: pointSoc:点修正目标SOC值(0.1%)
//输出参数	: void
//函数功能	: Soc点修正值处理
//注意事项	:  
//=============================================================================================
static void SocPointCorHandle(u16 pointSoc);
    
//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	:  void SocPointCorrTask(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  SOC点修正任务
//注意事项	:  1s
//=============================================================================================
void SocPointCorrTask(void)
{
    static e_ChgDchgStatus sChgstate = eChgDchg_MAX;    
    static u8 sTime = 0;
    u8 Index = 0;   
    u16 PointSocCrt = 0;
    
    if(0 == DataDealGetBMSDataRdy(0x01))    //AFE、电量计数据已准备好
    {
    	return;
    }  

    //系统状态刚刚转换完成,等待30s后再进行判断
    if(SysStateGetChgDchgSta() != sChgstate)
    {
        sTime = 0;
        sChgstate = SysStateGetChgDchgSta();
        return ;            
    }
    
    if(sTime < 30)    //等待30秒
    {
        sTime++;
        return;
    }    
    
    for(Index = 0;Index < SOC_POINT_CRT_NUM;Index++)
    {
        if((CheckPointCondition(Index, ePointTemp) == True)          //温度
            && (CheckPointCondition(Index, ePointCurr) == True)      //电流
            && (CheckPointCondition(Index, ePointVolt) == True))     //电压
        {
            sSocCorrTime[Index]++;
            
            //检测次数达到修正次数
            if(sSocCorrTime[Index] >= cSocPointTbl[Index].CorTime)  
            {
                sSocCorrTime[Index] = 0;
                PointSocCrt = cSocPointTbl[Index].CorSoc;  //目标SOC
              
                SocPointCorHandle(PointSocCrt * 10);        //修正执行
                break;
            }
        }
        else
        {
            sSocCorrTime[Index] = 0;                   //未达到修正测试检测次数为0
        }
    }

    
}

//=============================================================================================
//定义静态函数
//=============================================================================================
//=============================================================================================
//函数名称	: u8 CheckPointCondition(u8 index, e_SocPointType type)
//输入参数	: index:第几个点 type:0温度 1电流 2电压 
//输出参数	: False:不合法 True:合法
//函数功能	: 判断该点温度，电流，电压是否合法
//注意事项	:  
//=============================================================================================
static u8 CheckPointCondition(u8 index, e_SocPointType type)
{
    s16 temp = 0;       //当前值
    s16 Hpoint = 0;     //高端
    s16 Lpoint = 0;     //低端
    
    if((index >= SOC_POINT_CRT_NUM) || (type >= ePointMax)) 
    {
        return False;                   
    }
    
    switch(type)
    {
        //检测温度是否合法
        case ePointTemp:        
              temp = gBatteryInfo.TempChara.MaxTemp;
              Hpoint = cSocPointTbl[index].TempH;     //温度高端范围
              Lpoint = cSocPointTbl[index].TempL;     //温度低端范围          
            break;
        
        //检测电流是否合法
        case ePointCurr:         
              temp = gBatteryInfo.Data.Curr;	      //获取电流
              Hpoint = cSocPointTbl[index].CurrH;     //电流高端范围
              Lpoint = cSocPointTbl[index].CurrL;     //电流低端范围
            break;
            
        //检测电压是否合法     
        case ePointVolt:  
            Hpoint = cSocPointTbl[index].VoltH;       //电压高端范围
            Lpoint = cSocPointTbl[index].VoltL;       //电压低端范围
  
            if(eChgDchg_Chg == SysStateGetChgDchgSta())     //充电情况下
            {
                temp = gBatteryInfo.VoltChara.MaxVolt;    
            }
            else if(eChgDchg_Dchg == SysStateGetChgDchgSta())//放电情况下
            {
                temp = gBatteryInfo.VoltChara.MinVolt;      
            }
            else                                             //待机情况下，获取平均电压
            {
                temp = gBatteryInfo.VoltChara.PerVolt; 
            }
            break;
            
        default:
            break;
    } 
    
    if((Lpoint <= temp) && (temp <= Hpoint))
    {
        return True;
    }

    return(False);      
}

//=============================================================================================
//函数名称	: static void SocPointCorHandle(u16 pointSoc)
//输入参数	: pointSoc:点修正目标SOC值(0.1%)
//输出参数	: void
//函数功能	: Soc点修正值处理
//注意事项	:  
//=============================================================================================
static void SocPointCorHandle(u16 pointSoc)
{
    u16 nowSoc = 0;
    u16 aimSoc = 0;
    
    nowSoc = gSOCInfo.realSOC;              //当前SOC(0.1%)
    aimSoc = pointSoc;
    
    if(eChgDchg_Chg == SysStateGetChgDchgSta()) //充电情况下修正上端，不能出现充电时往下端修正
    {                               
        if(nowSoc < aimSoc)   
        {
            CorrNowCapBySoc(aimSoc);
            SocSetIsSlideShow(False);   //不平滑显示了，直接显示真实SOC
        }
    }
    else if(eChgDchg_Dchg == SysStateGetChgDchgSta())  
    {             
        if(nowSoc > aimSoc) //放电情况下修正下端，不能出现放电时往上端修正  
        {
            CorrNowCapBySoc(aimSoc);
        }
    }
    else    //空闲
    {
        CorrNowCapBySoc(aimSoc);
    }
}

/*****************************************end of SocPointCorr.c*****************************************/
