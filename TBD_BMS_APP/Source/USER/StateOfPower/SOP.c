//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SOP.c
//创建人  	: Handry
//创建日期	:
//描述	    : SOP计算源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SOP.h"
#include "SOPTab.h"
#include "NTC.h"
#include "DTTimer.h"
#include "DTCheck.h"
#include "MAX17205.h"
#include "DataDeal.h"
#include "ParaCfg.h"
#include "TestMode.h"
#include "CommCtrl.h"
#include "BSPSystemCfg.h"
#include "DateTime.h"

#ifdef BMS_USE_SOX_MODEL	
#include "SocCapCalc.h"
#include "SocSlideShow.h"
#endif

//=============================================================================================
//定义全局变量
//=============================================================================================


//=============================================================================================
//声明静态函数
//=============================================================================================
//=============================================================================================
//函数名称	:  static e_DTState SOPGetDTCState(e_DT_ID id)
//输入参数	:  id：故障id；
//输出参数	:  e_DTState：故障状态
//函数功能	:  SOP获取故障状态接口
//注意事项	:  
//=============================================================================================
static e_DTState SOPGetDTCState(e_DT_ID id);

//=============================================================================================
//函数名称	:  static s16 SOPGetBatMinTemp(void)
//输入参数	:  void
//输出参数	:  电池单体最小温度值
//函数功能	:  获取电池单体最小温度值
//注意事项	:  
//=============================================================================================
static s16 SOPGetBatMinTemp(void);

//=============================================================================================
//函数名称	:  static s16 SOPGetBatMaxTemp(void)
//输入参数	:  void
//输出参数	:  电池单体最大温度值
//函数功能	:  获取电池单体最大温度值
//注意事项	:  
//=============================================================================================
static s16 SOPGetBatMaxTemp(void);

//=============================================================================================
//函数名称	:  static s16 SOPGetBatMinVolt(void)
//输入参数	:  void
//输出参数	:  电池单体最小电压值
//函数功能	:  获取电池单体最小电压值
//注意事项	:  
//=============================================================================================
static s16 SOPGetBatMinVolt(void);

//=============================================================================================
//函数名称	:  static s16 SOPGetBatMaxVolt(void)
//输入参数	:  void
//输出参数	:  电池单体最大电压值
//函数功能	:  获取电池单体最大电压值
//注意事项	:  
//=============================================================================================
static s16 SOPGetBatMaxVolt(void);

//=============================================================================================
//函数名称	:  static u16 SOPGetBatFiltSOC(void)
//输入参数	:  void
//输出参数	:  电池SOC值
//函数功能	:  获取电池SOC值
//注意事项	:  
//=============================================================================================
static u16 SOPGetBatFiltSOC(void);

//=============================================================================================
//函数名称	:  static s16 SOPGetMaxCurr(void)
//输入参数	:  void
//输出参数	:  电量计电流最大值
//函数功能	:  获取电量计电流最大值
//注意事项	:  
//=============================================================================================
static s16 SOPGetMaxCurr(void);
	
//=============================================================================================
//函数名称	:  static s16 SOPGetBatCurr(void)
//输入参数	:  void
//输出参数	:  电池电流值
//函数功能	:  获取电池电流值
//注意事项	:  
//=============================================================================================
static s16 SOPGetBatCurr(void);

//=============================================================================================
//函数名称	:  static t_SOP_TAB* SOPGetTabByValue(s16 val, t_Val_SOPTAB_TAB const * tab)
//输入参数	:  val：输入的值，tab：值-sop表表格；
//输出参数	:  sop表地址
//函数功能	:  通过输入的值从表格查找合适的sop表地址
//注意事项	:  
//=============================================================================================
static t_SOP_TAB* SOPGetTabByValue(s16 val, t_Val_SOPTAB_TAB const * tab);

//=============================================================================================
//函数名称	:  static s32 SOPGetPercentByTab(s16 dat, t_SOP_TAB const * tab)
//输入参数	:  val：输入的值，tab：表格；
//输出参数	:  限值百分比
//函数功能	:  通过输入的值从表格查找合适的限值百分比
//注意事项	:  
//=============================================================================================
static s32 SOPGetPercentByTab(s16 val, t_SOP_TAB const * tab);

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	:  void SOPInit(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  SOP计算初始化函数
//注意事项	:  
//=============================================================================================
void SOPInit(void)
{
	//初始化限值数据数组
	gBatteryInfo.Limit.ChgCurrLim  = 0;
	gBatteryInfo.Limit.DchgCurrLim = 0;
	gBatteryInfo.Limit.ChgPowLim   = 0;
	gBatteryInfo.Limit.DchgPowLim  = 0;	
	gBatteryInfo.Limit.MaxChgVolt  = 0;	
}

//=============================================================================================
//函数名称	: u8 SOPBalanceGetChargePlugStaus(void)
//输入参数	: void
//输出参数	: 充电枪是否插入。1：插入；0：未插入
//静态变量	: void
//功    能	: 检测充电枪是否插入函数
//注    意	:
//=============================================================================================
u8 SOPGetChargePlugStaus(void)
{
	u8 chgplgsta = 0;	//充电器是否插入状态

	chgplgsta =  CommCtrlIsChargerPlugIn();

	return	chgplgsta;
}

//=============================================================================================
//函数名称	:  void SOPCalcTask(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  SOP计算主任务函数
//注意事项	:  
//=============================================================================================
void SOPCalcTask(void)
{
	s32 chgpercent = 0,dchgpercent = 0;		//充电，放电限值百分比
	s32 maxsocpercent = 0,minsocpercent = 0;//最大，最小温度下的限值百分比
	s32 mintempmaxvoltper = 0,maxtempmaxvoltper = 0;	//最小，最大温度下的最大电压下的限值百分比
	s32 mintempminvoltper = 0,maxtempminvoltper = 0;	//最小，最大温度下的最小电压下的限值百分比
	u8 dtcindex = 0;						//dtc列表索引临时值
	s16 maxchgvolt = 0;						//最大单体充电电压
	s32 chglim = 0;							//充电电流限值
	t_SOP_TAB* pvolttab = NULL;				//电压降功率表指针
	static s32 chgvoltinte = 0;				//充电电压积分
    u8 j = 0;
    #ifdef BMS_USE_SOX_MODEL
    static u8 CorrFullFlg = True;
    static u8 CorrUV1Flg = True; 
    static u8 CorrUV2Flg = True; 
    #endif
    #if defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S) || defined(SY_PB_32000MAH_17S)
    static u16 chgtime = 0;                 //持续充电电流时间
    #endif
    static u32 RtcCurrSec, RtcPreSec;
    RtcCurrSec = DateTime_GetSeconds(Null);
    
    if(0 == DataDealGetBMSDataRdy(0x37))    //AFE、电量计、NTC、特征数据已准备好
    {
    	return;
    }

	if((SOPGetBatMinTemp() != NTC_TEMP_OFF) && (SOPGetBatMinTemp() != NTC_TEMP_SHORT)	//温度采样不断线，不短路
		&& (0xffff != SOPGetBatFiltSOC()))												//soc值有效
	{
		/**********************************计算电池包最大充电电压**********************************/
		//if((SOPGetBatMinTemp() <= 0) || (SOPGetBatMaxTemp() >= 500))	//最小温度低于0℃ 或 最大温度高于50℃
        if((SOPGetDTCState(DT_ID_CHG_TH)) || (SOPGetDTCState(DT_ID_CHG_TL)))	 		//电池充电过温 或 电池充电低温
		{
			maxchgvolt = 0;                       //单体最大充电电压
			gBatteryInfo.Limit.MaxChgVolt = 0;    //包最大充电电压
		}
		else
		{
			if( SOPGetBatMaxTemp() <= 450)
			{
				maxchgvolt = MAX_CHG_VOLT;
			}
			else
			{
				maxchgvolt = MAX_CHG_VOLT;
			}
			
			gBatteryInfo.Limit.MaxChgVolt = (s32)maxchgvolt * BAT_CELL_NUM / 10;;    //包最大充电电压,0.1V
		}
		
		/**********************************计算电池包放电电流限值**********************************/	
		if((SOPGetDTCState(DT_ID_VL_LV1)) || (SOPGetDTCState(DT_ID_VL_LV2))	 		//电池欠压1级 或 电池欠压2级
		 ||(SOPGetDTCState(DT_ID_DIS_TH)) || (SOPGetDTCState(DT_ID_DIS_TL))	 		//电池放电过温 或 电池放电低温
		 ||(SOPGetDTCState(DT_ID_DIS_MOS_TH) || (SOPGetDTCState(DT_ID_CONN_TH))))	//放电MOS温度过高 或 连接器温度过高
		{
			//发生以上几种故障，放电电流限值都为0
			gBatteryInfo.Limit.DchgCurrLim = 0;
		}
		else
		{
			dchgpercent = (u32)SOPGetMaxCurr() * 9375 / 100000;			//使用电量计imax(32A)的93.75%,单位换算为10mA/lsb
			dtcindex = DTCheckFindIndexByDTID(DT_ID_DIS_CURH_LV1);		//获取放电过流故障id所对应的列表索引
			
			if((0 - *gDTConfig[dtcindex].matth) > dchgpercent)			//与一级放电过流阈值 比较,取其中较小的值
			{
				gBatteryInfo.Limit.DchgCurrLim = dchgpercent;
			}
			else
			{
				gBatteryInfo.Limit.DchgCurrLim = (0 - *gDTConfig[dtcindex].matth);
			}
		}
		
		/**********************************计算电池包充电电流限值**********************************/
        #if defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S) || defined(SY_PB_32000MAH_17S)
        //三元锂
        if(0 == SOPGetChargePlugStaus())   //充电枪拔出
        {
            chgtime = 0x0000;
        }  
        
		//充满状态判断
        //停止充电条件：
        //电池电量计的充满标志置位或持续0.5小时充电电流小于500mA，且最高电芯电压大于（单芯满充电压-10mV）。
        if((gBatteryInfo.Data.Curr <= TAPER_CURR) && (gBatteryInfo.VoltChara.MaxVolt >= (MAX_CHG_VOLT-10)))
        {
            if(chgtime < TAPER_CURR_TIME)//250ms任务
            {
                chgtime++;
            }
        }    
        
 		if(((chgtime >= TAPER_CURR_TIME) 
            &&(SOPGetBatMinVolt() >= (MAX_CHG_VOLT - 10)))) 
        {
            RtcPreSec = RtcCurrSec;
			gBatteryInfo.Status.ProtSta = eProt_Full;
            chgvoltinte = 0;
		} 
        //重启充电条件：
        //电池均衡时，最高电芯电压小于（单芯满充电压 - 100mV）；
        //电池不均衡时，最高电芯电压小于（单芯满充电压 - 80mV）且压差大于20mV。
        //已停止充电一天且最高电芯电压下降到4.05V以下，开始重启充电
        else if(((SOPGetBatMaxVolt() < (MAX_CHG_VOLT-80) && (SOPGetBatMaxVolt() - SOPGetBatMinVolt()) > 20)) || 
            (SOPGetBatMaxVolt() < (MAX_CHG_VOLT - 100)) || 
            ((SOPGetBatMaxVolt() < (MAX_CHG_VOLT-50) && ((RtcCurrSec - RtcPreSec)>=86400))) ||
            (gBatteryInfo.Fault.FaultInfo[2] & (OPT_FAULT1_UV1|OPT_FAULT1_UV2)))
        {
            gBatteryInfo.Status.ProtSta = eProt_Norm;
            chgtime = 0x0000;
        }        
        #elif defined(LFP_TB_20000MAH_20S) || defined(LFP_HL_25000MAH_16S) || defined(LFP_GF_25000MAH_16S) || defined(LFP_PH_20000MAH_20S)
        //磷酸铁锂
        //充满状态判断 最高节电压大于MAX_CHG_VOLT
        if(SOPGetBatMaxVolt() > (MAX_CHG_VOLT) || (SOPGetDTCState(DT_ID_VH)))
        {
            RtcPreSec = RtcCurrSec;
            gBatteryInfo.Status.ProtSta = eProt_Full;
        }
        //重启充电条件：
        //电池均衡时，最高电芯电压小于（单芯满充电压 - 300mV）；重启充电，小于3300mV
        //21/7/18 增加条件：超过一天不充电，就进行补一次电
        else if((SOPGetBatMaxVolt() < (MAX_CHG_VOLT - 300)) || 
            (((RtcCurrSec - RtcPreSec)>=86400)) ||
            (gBatteryInfo.Fault.FaultInfo[2] & (OPT_FAULT1_UV1|OPT_FAULT1_UV2)))
        {
            gBatteryInfo.Status.ProtSta = eProt_Norm;
            //chgtime = 0x0000;
		}
        #endif
        
        #ifdef BMS_USE_SOX_MODEL
        //充满需要修正为显示100%
        if(eProt_Full == gBatteryInfo.Status.ProtSta)
        {
            if(True == CorrFullFlg)
            {
                CorrFullFlg = False;
                CorrNowCapBySoc(1000);      //修正当前SOC为100%
                SocSetIsSlideShow(False);   //不平滑显示了，直接显示真实SOC                
            }
        }
        else
        {
            CorrFullFlg = True;
        }
        #endif
        
        //放电到低压，防止大电流拉载导致的欠压
        if(gBatteryInfo.Fault.FaultInfo[2] & OPT_FAULT1_UV1 && gSOCInfo.realSOC < 100)
        {
            if(True == CorrUV1Flg)
            {
                CorrUV1Flg = False;
                CorrNowCapBySoc(20);      //修正当前SOC为2%
                SocSetIsSlideShow(False);   //不平滑显示了，直接显示真实SOC              
            }
             
        }
        //二级欠压，直接SOC为1%
        else if(gBatteryInfo.Fault.FaultInfo[2] & OPT_FAULT1_UV2)
        {
            if(True == CorrUV2Flg)
            {
                CorrUV2Flg = False;
                CorrNowCapBySoc(10);      //修正当前SOC为1%
                SocSetIsSlideShow(False);   //不平滑显示了，直接显示真实SOC              
            }        
        }
        else
        {
            CorrUV1Flg = True; 
            CorrUV2Flg = True;
        }
        
		//充电电流限值计算
		if((SOPGetBatMinVolt() < 2000) || (0 == maxchgvolt)   						    //最小单体电压小于2000mV 或 单体最大充电电压为0
		   || (eProt_Full == gBatteryInfo.Status.ProtSta)								//电池包充放电保护状态为充满
		   || (SOPGetDTCState(DT_ID_VH)) 												//电池过压
		   || (SOPGetDTCState(DT_ID_CHG_TH)) || (SOPGetDTCState(DT_ID_CHG_TL))	 		//电池充电过温 或 电池充电低温
		   || (SOPGetDTCState(DT_ID_CHG_MOS_TH)) || (SOPGetDTCState(DT_ID_CONN_TH))		//充电MOS温度过高 或 连接器温度过高
			)
		{
			//发生以上几种情况，充电电流限值都为0
			gBatteryInfo.Limit.ChgCurrLim = 0;	
		}
		else
		{
			//求最小单体温度值的对应SOC降功率表，再根据SOC查找限值百分比
			minsocpercent = SOPGetPercentByTab(SOPGetBatFiltSOC(),SOPGetTabByValue(SOPGetBatMinTemp(),cSOPTempSOCTblTbl));
			//求最大单体温度值的对应SOC降功率表，再根据SOC查找限值百分比
			maxsocpercent = SOPGetPercentByTab(SOPGetBatFiltSOC(),SOPGetTabByValue(SOPGetBatMaxTemp(),cSOPTempSOCTblTbl));
			
			//求最小单体温度值的对应单体电压降功率表
			pvolttab = SOPGetTabByValue(SOPGetBatMinTemp(),cSOPTempVoltTblTbl);
			//求最小单体电压值的限值百分比
			mintempminvoltper = SOPGetPercentByTab(SOPGetBatMinVolt(), pvolttab);
			//求最大单体电压值的限值百分比
			mintempmaxvoltper = SOPGetPercentByTab(SOPGetBatMaxVolt(), pvolttab);			
			
			//求最大单体温度值的对应单体电压降功率表
			pvolttab = SOPGetTabByValue(SOPGetBatMaxTemp(),cSOPTempVoltTblTbl);
			//求最小单体电压值的限值百分比
			maxtempminvoltper = SOPGetPercentByTab(SOPGetBatMinVolt(), pvolttab);
			//求最大单体电压值的限值百分比
			maxtempmaxvoltper = SOPGetPercentByTab(SOPGetBatMaxVolt(), pvolttab);		

			//求最小的充电限值百分比
			chgpercent = Min(Min(minsocpercent,maxsocpercent),														//不同温度下soc查找的最小值
						 Min(Min(mintempminvoltper,mintempmaxvoltper),Min(maxtempminvoltper,maxtempmaxvoltper)));	//不同温度下单体电压查找的最小值
						 
			//根据充电限值百分比计算出充电电流限值
			gBatteryInfo.Limit.ChgCurrLim = (BAT_NORM_CAP / 10) * chgpercent / 100;
			
            #if defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S) || defined(SY_PB_32000MAH_17S)
            if(eChgDchg_Chg == gBatteryInfo.Status.ChgDchgSta)
            {
                //（最大单体充电电压 - 最大单体电压值） 积分
                if(gBatteryInfo.Limit.ChgCurrLim > 0)
                {
                    chgvoltinte += ((s32)(maxchgvolt - SOPGetBatMaxVolt())) > 4 ? 
                        ((s32)(maxchgvolt - SOPGetBatMaxVolt()) / 4):((s32)(maxchgvolt - SOPGetBatMaxVolt()));
                    
                    if(chgvoltinte >= 30)
                        chgvoltinte = 30;
                    else if(chgvoltinte <= (-BAT_NORM_CAP / 10))
                        chgvoltinte = (-BAT_NORM_CAP / 10);
                }
                
                //计算还能增加多少充电电流
                chglim = SOPGetBatCurr() + chgvoltinte + ((s32)(maxchgvolt - SOPGetBatMaxVolt()) * 100 / BAT_CSOP_RES);
                
                if((chglim <= 0))
                {
                    gBatteryInfo.Limit.ChgCurrLim = 0;
                }
                else if(chglim < gBatteryInfo.Limit.ChgCurrLim)
                {
                    gBatteryInfo.Limit.ChgCurrLim = chglim;
                }
            }
            #else
            if(eChgDchg_Chg == gBatteryInfo.Status.ChgDchgSta)
            {
                //（最大单体充电电压 - 最大单体电压值） 积分
                chgvoltinte += (s32)(maxchgvolt - SOPGetBatMaxVolt());
                if(chgvoltinte > 30)
                {
                    chgvoltinte = 30;
                }
                else if(chgvoltinte < -30)
                {
                    chgvoltinte = -30;
                }
                
                //计算还能增加多少充电电流
                chglim = SOPGetBatCurr() + chgvoltinte + ((s32)(maxchgvolt - SOPGetBatMaxVolt()) * 100 / BAT_CSOP_RES);
                
                if((chglim <= 0))
                {
                    gBatteryInfo.Limit.ChgCurrLim = 0;
                }
                else if(chglim < gBatteryInfo.Limit.ChgCurrLim)
                {
                    gBatteryInfo.Limit.ChgCurrLim = chglim;
                }
            }
            #endif
//            //LFP最大充电电流 8.5A
//            if(gBatteryInfo.Limit.ChgCurrLim >= MAX_CHG_CURR)
//            {
//                gBatteryInfo.Limit.ChgCurrLim = MAX_CHG_CURR;
//            }                
		}
	}
	else
	{
		gBatteryInfo.Limit.ChgCurrLim = 0;
	}
    
    //更新充电过流保护值， 分4个等级，告警 1级保护 2级保护 3级保护
    //修改充电过流保护门限（BMS的需求电流+电流偏置值）
    gConfigChgCurr = gConfig.ccurr;
    
    for(j = 0;j < PROTECT_MAX_LEVEL;j++)
    {
        if(8 == gPCBTest.presentstep)   //开关mos管测试模式
        {
            gConfigChgCurr.matth[j] = PROT_MAX_CHG_CURR;
        }
        else
        {
            gConfigChgCurr.matth[j] = Min((gConfig.ccurr.matth[j] + gBatteryInfo.Limit.ChgCurrLim),PROT_MAX_CHG_CURR);
        }
        
        if((gConfig.ccurr.resth[j] + gBatteryInfo.Limit.ChgCurrLim) < gConfigChgCurr.matth[j])
        {
            gConfigChgCurr.resth[j] = gConfig.ccurr.resth[j] + gBatteryInfo.Limit.ChgCurrLim;
        }
        else
        {
            //当（BMS的需求电流 + 电流恢复偏置值）大于11/12A时，恢复门限 = 最大充电电流 - 1.2 + i（i为门限级别0-3）
            gConfigChgCurr.resth[j] = gConfigChgCurr.matth[j] - gConfig.ccurr.resth[0] + j;
        }
        
//        DTTimerChangeConfig(DT_ID_CHG_CURRH_WARM,gConfigChgCurr.mattime[DT_LV1]*1000,gConfigChgCurr.restime[DT_LV1]*1000);
//        DTTimerChangeConfig(DT_ID_CHG_CURH_LV1,gConfigChgCurr.mattime[DT_LV2]*1000,gConfigChgCurr.restime[DT_LV1]*1000);
//        DTTimerChangeConfig(DT_ID_CHG_CURH_LV2,gConfigChgCurr.mattime[DT_LV3]*1000,gConfigChgCurr.restime[DT_LV2]*1000);
//        DTTimerChangeConfig(DT_ID_CHG_CURH_LV3,gConfigChgCurr.mattime[DT_LV4]*1000,gConfigChgCurr.restime[DT_LV3]*1000);
    
    }
    
}

//Sop测试函数
void SOPTestTask()
{
	s32 chgpercent = 0;						//充电，放电限值百分比
	s32 maxsocpercent = 0,minsocpercent = 0;//最大，最小温度下的限值百分比
	s32 mintempmaxvoltper = 0,maxtempmaxvoltper = 0;	//最小，最大温度下的最大电压下的限值百分比
	s32 mintempminvoltper = 0,maxtempminvoltper = 0;	//最小，最大温度下的最小电压下的限值百分比
	t_SOP_TAB* pvolttab = NULL;	
	
	//求最小单体温度值的对应SOC降功率表，再根据SOC查找限值百分比
	minsocpercent = SOPGetPercentByTab(SOPGetBatFiltSOC(),SOPGetTabByValue(SOPGetBatMinTemp() * 10,cSOPTempSOCTblTbl));
	//求最大单体温度值的对应SOC降功率表，再根据SOC查找限值百分比
	maxsocpercent = SOPGetPercentByTab(SOPGetBatFiltSOC(),SOPGetTabByValue(SOPGetBatMaxTemp() * 10,cSOPTempSOCTblTbl));
	
	//求最小单体温度值的对应单体电压降功率表
	pvolttab = SOPGetTabByValue(SOPGetBatMinTemp() * 10,cSOPTempVoltTblTbl);
	//求最小单体电压值的限值百分比
	mintempminvoltper = SOPGetPercentByTab(SOPGetBatMinVolt(), pvolttab);
	//求最大单体电压值的限值百分比
	mintempmaxvoltper = SOPGetPercentByTab(SOPGetBatMaxVolt(), pvolttab);			
	
	//求最大单体温度值的对应单体电压降功率表
	pvolttab = SOPGetTabByValue(SOPGetBatMaxTemp() * 10,cSOPTempVoltTblTbl);
	//求最小单体电压值的限值百分比
	maxtempminvoltper = SOPGetPercentByTab(SOPGetBatMinVolt(), pvolttab);
	//求最大单体电压值的限值百分比
	maxtempmaxvoltper = SOPGetPercentByTab(SOPGetBatMaxVolt(), pvolttab);		

	//求最小的充电限值百分比
	chgpercent = Min(Min(minsocpercent,maxsocpercent),														//不同温度下soc查找的最小值
				 Min(Min(mintempminvoltper,mintempmaxvoltper),Min(maxtempminvoltper,maxtempmaxvoltper)));	//不同温度下单体电压查找的最小值
				 
	//根据充电限值百分比计算出充电电流限值
	gBatteryInfo.Limit.ChgCurrLim = (BAT_NORM_CAP / 10) * chgpercent / 100;
}

//=============================================================================================
//函数名称	:  static e_DTState SOPGetDTCState(e_DT_ID id)
//输入参数	:  id：故障id；
//输出参数	:  e_DTState：故障状态
//函数功能	:  SOP获取故障状态接口
//注意事项	:  
//=============================================================================================
static e_DTState SOPGetDTCState(e_DT_ID id)
{
	return DTTimerGetState(id);
}

//=============================================================================================
//函数名称	:  static s16 SOPGetBatMinTemp(void)
//输入参数	:  void
//输出参数	:  电池单体最小温度值
//函数功能	:  获取电池单体最小温度值
//注意事项	:  
//=============================================================================================
static s16 SOPGetBatMinTemp(void)
{
	return gBatteryInfo.TempChara.MinTemp;
}

//=============================================================================================
//函数名称	:  static s16 SOPGetBatMaxTemp(void)
//输入参数	:  void
//输出参数	:  电池单体最大温度值
//函数功能	:  获取电池单体最大温度值
//注意事项	:  
//=============================================================================================
static s16 SOPGetBatMaxTemp(void)
{
	return gBatteryInfo.TempChara.MaxTemp;
}

//=============================================================================================
//函数名称	:  static s16 SOPGetBatMinVolt(void)
//输入参数	:  void
//输出参数	:  电池单体最小电压值
//函数功能	:  获取电池单体最小电压值
//注意事项	:  
//=============================================================================================
static s16 SOPGetBatMinVolt(void)
{
	return gBatteryInfo.VoltChara.MinVolt;
}

//=============================================================================================
//函数名称	:  static s16 SOPGetBatMaxVolt(void)
//输入参数	:  void
//输出参数	:  电池单体最大电压值
//函数功能	:  获取电池单体最大电压值
//注意事项	:  
//=============================================================================================
static s16 SOPGetBatMaxVolt(void)
{
	return gBatteryInfo.VoltChara.MaxVolt;
}

//=============================================================================================
//函数名称	:  static u16 SOPGetBatFiltSOC(void)
//输入参数	:  void
//输出参数	:  电池SOC值
//函数功能	:  获取电池SOC值
//注意事项	:  
//=============================================================================================
static u16 SOPGetBatFiltSOC(void)
{
	return gBatteryInfo.Data.FiltSOC;
}

//=============================================================================================
//函数名称	:  static s16 SOPGetMaxCurr(void)
//输入参数	:  void
//输出参数	:  电量计电流最大值
//函数功能	:  获取电量计电流最大值
//注意事项	:  
//=============================================================================================
static s16 SOPGetMaxCurr(void)
{
	return 0;
}

//=============================================================================================
//函数名称	:  static s16 SOPGetBatCurr(void)
//输入参数	:  void
//输出参数	:  电池电流值
//函数功能	:  获取电池电流值
//注意事项	:  
//=============================================================================================
static s16 SOPGetBatCurr(void)
{
	return gBatteryInfo.Data.Curr;
}

//=============================================================================================
//函数名称	:  static t_SOP_TAB* SOPGetTabByValue(s16 val, t_Val_SOPTAB_TAB const * tab)
//输入参数	:  val：输入的值，tab：值-sop表表格；
//输出参数	:  sop表地址
//函数功能	:  通过输入的值从表格查找合适的sop表地址
//注意事项	:  
//=============================================================================================
static t_SOP_TAB* SOPGetTabByValue(s16 val, t_Val_SOPTAB_TAB const * tab)
{
	u32 i = 0;
	
    if(tab == NULL)
    {
        return NULL;
    }
    while(0x7fff != tab[i].value)
    {
        if(val <= tab[i].value) 
        {
            return tab[i].soptab;
        }
        i++;
    }
    return tab[i].soptab;
	
}

//=============================================================================================
//函数名称	:  static s32 SOPGetPercentByTab(s16 dat, t_SOP_TAB const * tab)
//输入参数	:  val：输入的值，tab：表格；
//输出参数	:  限值百分比
//函数功能	:  通过输入的值从表格查找合适的限值百分比
//注意事项	:  
//=============================================================================================
static s32 SOPGetPercentByTab(s16 val, t_SOP_TAB const * tab)
{
    u32 i = 0;
	
    if(tab == NULL)
    {
        return 0;
    }
    while(0x7fff != tab[i].value)
    {
        if(val <= tab[i].value) 
        {
            return tab[i].percent;
        }
        i++;
    }
    return tab[i].percent;
}

/*****************************************end of SOP.c*****************************************/
