//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: MOSCtrl.c
//创建人  	: Handry
//创建日期	: 
//描述	    : MOS管控制代码
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
//=============================================================================================
//包含头文件
//=============================================================================================
#include "MOSCtrl.h"
#include "SH367309.h"
#include "BSPTimer.h"
#include "DataDeal.h"
#include "CommCtrl.h"
#include "IOCheck.h"
#include "DTCheck.h"
#include "Sample.h"
#include "SysState.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "Sample.h"
#include "pt.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
t_MC gMCData;           //mos控制结构体

//static const u16 c_FAULT_ALL_MASK[4]=  {0x067F,0x007F,0x1FFF,0x00BF};
//HNM：由硬件决定的不可屏蔽故障，出现故障立刻切断相应充放电管。
static const u16 cFaultHNMMask[4] = {0x0000,0x0000,0x0025,0x0019};

//H：高安全级别故障，出现故障标志后延时10s切断相应充放电管。
static const u16 cFaultHMask[4]   = {0x0001,0x007F,0x1FDA,0x0F80};//{0x0001,0x007F,0x1FDA,0x0080};

//M：中安全级别故障，出现故障时，如果相应充放电管处于开启状态则保持不变，如果相应充放电管处于关闭状态则禁止打开。
static const u16 cFaultMMask[4]   = {0x067E,0x0000,0x0000,0x0026};

//充电关心的故障
static const u16 cFaultChgMask[4]  = {0x067F,0x007F,0x1D49,0x0FBC};//{0x067F,0x007F,0x1D49,0x00BC};
//放电关心的故障
static const u16 cFaultDchgMask[4] = {0x067F,0x007F,0x3EB6,0x00BB};

pt gPtMosCtrl;  //MOS控制任务pt线程控制变量

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void MOSCtrlInit(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: MOS管控制初始化函数
//注    意	:
//=============================================================================================
void MOSCtrlInit(void)
{
	//gMCData.sta = 0;
	gMCData.predsg_err = 0;
	gMCData.dsg_en = 0;
	gMCData.mos_fault = 0;
    gMCData.chg_mos_fault_cnt=0;
    gMCData.dsg_mos_fault_cnt=0;
    
    MOSCtrlEnable();
    
    PT_INIT(&gPtMosCtrl);
}

//=============================================================================================
//函数名称	: void MOSCtrlEnable(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: MOS管控制使能函数
//注    意	:
//=============================================================================================
void MOSCtrlEnable(void)
{
	//enable
    gMCData.sta = 1;
    //gMCData.predsg_flag = 0;               //复位预放电标志,放到从休眠唤醒处
    gMCData.last_dsg = 0;                    //复位放电管标志
    gMCData.predsg_inner5s = 0;
    
    gMCData.mos_fault &= (~PDS_MASK);         //清除预放电故障，不清除充放电mos管故障
    
    if(gMCData.mos_fault & CHG_MASK)          //充放电mos管故障超过3次，则锁定
    {
        if(gMCData.chg_mos_fault_cnt < 2)
        {
            gMCData.chg_mos_fault_cnt++;
        }
    }
    if(gMCData.chg_mos_fault_cnt <= 1)
    {
        gMCData.mos_fault &= (~CHG_MASK);
    }
    if(gMCData.mos_fault & DSG_MASK)
    {
        if(gMCData.dsg_mos_fault_cnt < 2)
        {
            gMCData.dsg_mos_fault_cnt++;
        }
    }
    if(gMCData.dsg_mos_fault_cnt <= 1)
    {
        gMCData.mos_fault &= (~DSG_MASK);
    }    
    
    gMCData.fault_timer1 =  gTimer1ms;
    gMCData.fault_timer2 =  gTimer1ms;

}

//=============================================================================================
//函数名称	: void MOSCtrlFirstPreInit(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: MOS管控制预放电任务初始化函数
//注    意	: 重新开始预放电
//=============================================================================================
void MOSCtrlFirstPreInit(void)
{
    if(gMCData.predsg_ctrl == 1)
    {
        gMCData.predsg_ctrl = 0;
        //task_delete(gMCData.predsg_task);
    }
	gMCData.predsg_ctrl = 1;		//初始化预放电数据
	gMCData.predsg_inner5s = 1;
	gMCData.predsg_err = 0;		//复位故障标志
	//gMCData.predsg_task = task_add(mc_predsg_task, d);	//调入预放电任务
}

//=============================================================================================
//函数名称	: void MOSCtrlPreDchgTask(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: MOS管控制预放电任务函数
//注    意	: 100ms任务
//=============================================================================================
void MOSCtrlPreDchgTask(void)
{
	static u16 errtimerLv1,errtimerLv2 = 0;		//短路判定滤波计数器
	static u16 normaltimerLv1,normaltimerLv2 = 0;	//正常判定滤波计数器
	static u16 delaytimer = 0;		//预放电管延时滤波计数器
	static u16 offpretimer = 0; 	//关闭预放电管延迟计数器
    
    //小电流检测功能
    #ifdef BMS_ENABLE_LITTLE_CURRENT_DET
	static u16 errtimer2, errtimer3= 0;		//漏电流异常判定滤波计数器
	static u16 normaltimer2,normaltimer3 = 0;	//漏电流正常判定滤波计数器    
    #define IDLE_OVER_LOW_CURRENT   10  //漏电流过流值 mA
    
    #define LOW_CURR_NORMAL_BASE_VAL_MIN    650 //漏电流检测正常值下限
    #define LOW_CURR_NORMAL_BASE_VAL_MAX    2650 //漏电流检测正常值上限
    #endif

    if(0 == DataDealGetBMSDataRdy(0x37))    //AFE、电量计、NTC、特征数据已准备好
    {
    	return;
    }

    //小电流检测功能
    #ifdef BMS_ENABLE_LITTLE_CURRENT_DET
    if(((0 == gMCData.predsg_ctrl) || (0 == BITGET(gBatteryInfo.Status.IOStatus,2)))
        || (False == gSampleData.SampFlag.SampFlagBit.IsPreSampOk))
    #else    
    if(0 == gMCData.predsg_ctrl)
    #endif
	{
//        if(0 == gSampleData.PreVolt)
//        {
//            gSampleData.PreCurr = 0;
//        }        
		errtimerLv1 = 0;
		normaltimerLv1 = 0;
		errtimerLv2 = 0;
		normaltimerLv2 = 0;
		delaytimer = 0;
		offpretimer = 0;
        //小电流检测功能
        #ifdef BMS_ENABLE_LITTLE_CURRENT_DET
        normaltimer2 = 0;
        errtimer2 = 0;
        normaltimer3 = 0;
        errtimer3 = 0;
        #endif
		return;
	}
    
    //小电流检测功能
    #ifdef BMS_ENABLE_LITTLE_CURRENT_DET
    
    //增加漏电流检测硬件电路故障检测
    if((gSampleData.P1ADBaseVolt >= LOW_CURR_NORMAL_BASE_VAL_MIN && gSampleData.P1ADBaseVolt <= LOW_CURR_NORMAL_BASE_VAL_MAX)
        && (gSampleData.P2ADBaseVolt >= LOW_CURR_NORMAL_BASE_VAL_MIN && gSampleData.P2ADBaseVolt <= LOW_CURR_NORMAL_BASE_VAL_MAX))
    {
        normaltimer3++;
        errtimer3 = 0;
    }   
    else
    {
        normaltimer3 = 0;
        errtimer3++;        
    }
    
    if(errtimer3 > 20)    //漏电流检测硬件电路异常
    {
        gBatteryInfo.Fault.FaultInfo[1] |= (DEV_LOW_CURR_CIRCUIT_FAULT);
    }

    if(normaltimer3 > 10)	//漏电流检测硬件电路正常
    {
         gBatteryInfo.Fault.FaultInfo[1] &= ~(DEV_LOW_CURR_CIRCUIT_FAULT);
    }
    
    //增加漏电流过流故障检测
    if(gBatteryInfo.Status.IOStatus & HALL_CHECK_SHIFT)
    {
        //磁铁接入，检测P2口漏电流
        if(gSampleData.P2LowCurrVal > IDLE_OVER_LOW_CURRENT)
        {
            normaltimer2 = 0;
            errtimer2++;            
        }
        else
        {
            normaltimer2++;
            errtimer2 = 0;        
        }        
    }
    else if(gBatteryInfo.Status.IOStatus & EXTERN_MODULE_CHECK_SHIFT)
    {
        //外置模块接入，检测P1口漏电流
        if(gSampleData.P1LowCurrVal > IDLE_OVER_LOW_CURRENT)
        {
            normaltimer2 = 0;
            errtimer2++;            
        }
        else
        {
            normaltimer2++;
            errtimer2 = 0;        
        }    
    }
    else
    {
        normaltimer2 = 0;
        errtimer2 = 0;
    }
    
    if(errtimer2 > (uint32_t)20)    //漏电流异常
    {
        gBatteryInfo.Fault.FaultInfo[3] |= (OPT_FAULT2_OVER_LOW_CURR);
    }

    if(normaltimer2 > 10)	//漏电流正常
    {
         gBatteryInfo.Fault.FaultInfo[3] &= ~(OPT_FAULT2_OVER_LOW_CURR);
    }
    
    #endif

	delaytimer++;
    //printf("check %d %d\n",gSampleData.PreVolt,HAL_GetTick()); //检测后对比
    if(gSampleData.PreVolt < PRE_DCHG_OVER_CURR_LV1)//预放电流
    {
    	normaltimerLv1++;
        normaltimerLv2++;
    	errtimerLv1 = 0;
        errtimerLv2 = 0;
        //gMCData.timercnt[0] = gTimer1ms;
    }
    else if(gSampleData.PreVolt > PRE_DCHG_OVER_CURR_LV1 
        && gSampleData.PreVolt < PRE_DCHG_OVER_CURR_LV2)//1级预放过流
    {
    	normaltimerLv1 = 0;
        normaltimerLv2 = 0;
    	errtimerLv1++;
        //gMCData.timercnt[0] = gTimer1ms;
    }
    else if(gSampleData.PreVolt >= PRE_DCHG_OVER_CURR_LV2)//2级预放过流
    {
    	normaltimerLv1 = 0;
        normaltimerLv2 = 0;
    	errtimerLv2++;
        //gMCData.timercnt[0] = gTimer1ms;
    }      
    else
    {
        ;
    }
    
    //预放电过温，不开预放
    if(gBatteryInfo.Fault.FaultInfo[2] & OPT_FAULT1_PRE_OTEMP)
    {
         gMCData.dsg_en = 0;
         gMCData.predsg_inner5s = 0;			//退出时将5s超时清零
         gMCData.predsg_ctrl = 0; 
         return;
    }

    //预放过载，不开预放
    if((errtimerLv1 > (uint32_t)30 || errtimerLv2 > (uint32_t)0) || (True == gMCData.IsPreOver400mA))
    {
         gMCData.dsg_en = 0;
         gMCData.predsg_err |= PREDSG_ERR_SHORT_MASK;
         gMCData.predsg_inner5s = 0;			//退出时将5s超时清零
         gMCData.predsg_ctrl = 0;
         PRE_DSG_DN();                          //检测到后立马关断
         //printf("overload %d %d\n",gSampleData.PreVolt,HAL_GetTick()); //检测后对比
         return;
    }

    if(normaltimerLv1 > 10 || normaltimerLv2 > 10)	//预放电正常
    {
         gMCData.dsg_en = 1;
    }

    if(delaytimer > (uint32_t)1800/*50*/)		//延长到3min
    {
         gMCData.predsg_inner5s = 0;
    }

    if(gMCData.mos_ctrl & DSG_MASK)//放电管闭合
    {
    	offpretimer++;
        //小电流检测功能
        #ifdef BMS_ENABLE_LITTLE_CURRENT_DET
        if((offpretimer > 30) && (gBatteryInfo.Status.MOSStatus & 0x02))//放电管闭合1s后断开预放电
        #else
        if((offpretimer > 10) && (gBatteryInfo.Status.MOSStatus & 0x02))//放电管闭合1s后断开预放电
        #endif
        {
        	offpretimer = 0;
            gMCData.predsg_inner5s = 0;
        }
    }
    else
    {
    	offpretimer = 0;
    }

    if(CommCtrlIsPreDchgEnable())//预放电有效则取消禁止休眠标志
    {
    	gMCData.predsg_inner5s = 0;
    }

    if(!CommCtrlIsPreDchgEnable() && (gMCData.predsg_inner5s == 0))
    {
        //外置模块不在位 且 无通信控制
        //之前：霍尔接上， 无预放电通信控制，且无NFC通信超过3min。
        if(!CommCtrlIsPollRec() && (!(gBatteryInfo.Status.IOStatus & EXTERN_MODULE_CHECK_SHIFT)))
        {
           gMCData.predsg_err |= PREDSG_ERR_TIMEOUT_MASK;
        }
        if(gMCData.dsg_en == 0)
        {
           gMCData.predsg_err |= PREDSG_ERR_SHORT_MASK;
        }

        gMCData.predsg_inner5s = 0;//退出时将5s超时清零
        gMCData.predsg_ctrl = 0;
        return;
    }
}

//=============================================================================================
//函数名称	: static u8 MOSCtrlFaultMaskAnd(const u16 * fault, const u16 *mask, u16 *res, u16 n)
//函数参数	: fault：实际电池故障；mask：关心的故障位；res：与 的结果；n：与的长度
//输出参数	: ret：1：关心的位有故障；0：关心的位无故障；
//静态变量	:
//功    能	: MOS管故障位检测 与
//注    意	:
//=============================================================================================
static u8 MOSCtrlFaultMaskAnd(const u16 * fault, const u16 *mask, u16 *res, u16 n)
{
    u8 ret=FALSE;
    u16 r;

    while(n--)
    {
        r = (*fault++)&(*mask++);
        if(r)
        {
            ret = TRUE;
        }
        if(res)
        {
            *(res++) = r;
        }
    }
    return ret;
}

//=============================================================================================
//函数名称	: static u8 MOSCtrlFaultMaskOr(const u16 * fault, const u16 *mask, u16 *res, u16 n)
//函数参数	: fault：实际电池故障；mask：关心的故障位；res：或 的结果；n：或的长度
//输出参数	: ret：1：关心的位有故障；0：关心的位无故障；
//静态变量	:
//功    能	: MOS管故障位检测 或
//注    意	:
//=============================================================================================
static u8 MOSCtrlFaultMaskOr(const u16 * fault, const u16 *mask, u16 *res, u16 n)
{
    u8 ret=FALSE;
    u16 r;

    while(n--)
    {
        r = (*fault++)|(*mask++);
        if(r)
        {
            ret = TRUE;
        }
        if(res)
        {
            *(res++) = r;
        }
    }
    return ret;
}

//=============================================================================================
//函数名称	: void MOSCtrlMOSFaultCheck(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: MOS管失效检测任务函数
//注    意	: 100ms任务
//=============================================================================================
void MOSCtrlMOSFaultCheck()
{
    if(0 == DataDealGetBMSDataRdy(0x03))    //AFE、电量计数据已准备好
    {
    	return;
    }

    if(gMCData.mos_ctrl & CHG_MASK)
    {
        gMCData.fault_timer1 = gTimer1ms;
    }
    else
    {
        if((gTimer1ms - gMCData.fault_timer1) >= 2000)//持续2s
        {
            if(gBatteryInfo.Data.Curr < 30)//充电电流小于300mA
            {
                gMCData.fault_timer1 = gTimer1ms - 2000;
            }
            else
            {
                if((gTimer1ms - gMCData.fault_timer1) >= 4000)//持续4-2 = 2 s
                {
                    gMCData.mos_fault |= CHG_MASK;//判定充电管故障
                }
            }
        }
    }


    if(gMCData.mos_ctrl&(DSG_MASK | PDS_MASK))
    {
        gMCData.fault_timer2 = gTimer1ms;
    }
    else
    {
        if((gTimer1ms - gMCData.fault_timer2) >= 2000)
        {
            if((gBatteryInfo.Data.Curr > -5) && (gSampleData.PreVolt < 100))//放电电流小于50mA，并且预放检测电路电压小于40mV
            {
                gMCData.fault_timer2 = gTimer1ms - 2000;
            }
            else
            {
                if((gTimer1ms - gMCData.fault_timer2) >= 4000)
                {
                    //增加实际打开预放判断
                    if((gSampleData.PreVolt >= 40) && BITGET(gBatteryInfo.Status.IOStatus,2))//预放支路预放检测电路损坏
                    {
                        gMCData.mos_fault |= PDS_MASK;//预放电路损坏
                    }
                    if(gBatteryInfo.Data.Curr <= -30)
                    {
                        gMCData.mos_fault |= DSG_MASK;//放电管损坏
                    }
                }
            }
        }
    }
}

//=============================================================================================
//函数名称	: void MOSCtrlMainTask(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: MOS管控制主任务函数
//注    意	:
//=============================================================================================
void MOSCtrlMainTask(void)
{

    u16 fault_hnm[4],fault_h[4],fault_m[4],fault_wk[4];//,fault_chg[4];
    u8 fault_flag;
    
    //小电流检测功能
    #ifdef BMS_ENABLE_LITTLE_CURRENT_DET
    static u8 sLastPreStat = False; //上一次的预放状态
    #endif    
//    if(gMCData.sta == 0)
//    {
//        return;
//    }
    
    PT_BEGIN(&gPtMosCtrl);
    
    if(0 == DataDealGetBMSDataRdy(0x37))    //AFE、电量计、NTC、特征数据已准备好
    {
    	return;
    }

    MOSCtrlFaultMaskAnd(gBatteryInfo.Fault.FaultInfo, cFaultHNMMask, fault_hnm, 4);
    MOSCtrlFaultMaskAnd(gBatteryInfo.Fault.FaultInfo, cFaultHMask, fault_h, 4);
    MOSCtrlFaultMaskAnd(gBatteryInfo.Fault.FaultInfo, cFaultMMask, fault_m, 4);
    
    //充电
    fault_flag = MOSCtrlFaultMaskAnd(fault_h, cFaultChgMask, fault_wk, 4);

    if(fault_flag)
    {
        if((gTimer1ms - gMCData.fault_chg_timer) > CHG_MC_FAULT_TIMEOUT)
        {
            MOSCtrlFaultMaskOr(fault_hnm, fault_wk, fault_hnm, 4);
            gMCData.fault_chg_timer = gTimer1ms - (CHG_MC_FAULT_TIMEOUT + 1000);
        }
    }
    else
    {
    	gMCData.fault_chg_timer = gTimer1ms;
    }

    fault_flag = MOSCtrlFaultMaskAnd(fault_hnm, cFaultChgMask, fault_wk, 4);//HNM和H级故障关于充电管相关位

    if(((fault_flag) && (gDchgFlag == 0))
       ||(!CommCtrlIsChgEnable())
   	   ||(!(gBatteryInfo.Status.IOStatus & (HALL_CHECK_SHIFT | EXTERN_MODULE_CHECK_SHIFT))))
    {
        gMCData.mos_ctrl &= ~CHG_MASK;
    }
    else
    {
        fault_flag = MOSCtrlFaultMaskAnd(gBatteryInfo.Fault.FaultInfo, cFaultChgMask, fault_wk, 4);//检查除了L级故障关于充电管位

        if((!fault_flag)||(gDchgFlag))//没有故障才能恢复
        {
            gMCData.mos_ctrl |= CHG_MASK;
        }
    }

    //放电
    MOSCtrlFaultMaskAnd(gBatteryInfo.Fault.FaultInfo, cFaultHNMMask, fault_hnm, 4);
    fault_flag = MOSCtrlFaultMaskAnd(fault_h, cFaultDchgMask, fault_wk, 4);
    if(fault_flag)
    {
        if((gTimer1ms - gMCData.fault_dsg_timer) > MC_FAULT_TIMEOUT)
        {
            MOSCtrlFaultMaskOr(fault_hnm, fault_wk, fault_hnm, 4);//超时加入放电管相关H级故障
            gMCData.fault_dsg_timer = gTimer1ms - (MC_FAULT_TIMEOUT+1000);
        }
    }
    else
    {
        gMCData.fault_dsg_timer = gTimer1ms;
    }

    fault_flag = MOSCtrlFaultMaskAnd(fault_hnm, cFaultDchgMask, fault_wk, 4);//HNM和H级故障关于放电管相关位

    if( ((fault_flag) && (gChgFlag == 0)) 
        || (!(gBatteryInfo.Status.IOStatus & (HALL_CHECK_SHIFT | EXTERN_MODULE_CHECK_SHIFT))))
    {
    	//有故障并且处于不充不放状态，移除HALL则关闭充放电管
        gMCData.mos_ctrl &= ~(DSG_MASK|PDS_MASK);
        gMCData.last_dsg = 0;
        if(gMCData.predsg_ctrl == 1)//有预放电任务则删除
        {
            gMCData.predsg_ctrl =0;
            gMCData.predsg_inner5s = 0;
            //task_delete(gMCData.predsg_task);
        }

        if((!(gBatteryInfo.Status.IOStatus & (HALL_CHECK_SHIFT | EXTERN_MODULE_CHECK_SHIFT))))//如果移除G2则复位预放标志
        {
            gMCData.predsg_flag = 0;
        }
        gMCData.dsg_en = 0;
    }
    //有HALL信号
    else
    {
        if(!(CommCtrlIsDchgEnable()))//没有CTRL 控制使能放电管
        {
            gMCData.mos_ctrl &= ~DSG_MASK;
            gMCData.last_dsg = 0;
            gMCData.dsg_en = 0;
        }

        fault_flag = MOSCtrlFaultMaskAnd(gBatteryInfo.Fault.FaultInfo, cFaultDchgMask, fault_wk, 4);//检查除了L级故障关于放电管位
        if((!fault_flag)||(gChgFlag)) //没有故障或者有充电电流才能恢复
        {
            if(gMCData.predsg_flag == 0)
            {
                gMCData.predsg_flag = 1;
                MOSCtrlFirstPreInit();	//进入预放电任务
            }

            if((CommCtrlIsDchgEnable())&&(gMCData.last_dsg == 0))//放电管由关到开
            {
                gMCData.last_dsg = 1;
                MOSCtrlFirstPreInit();	//进入预放电任务
            }

            if((gMCData.dsg_en == 0)||(!(CommCtrlIsDchgEnable())))//预放电程序不允许开启放电管
            {
                gMCData.mos_ctrl &= ~DSG_MASK;
            }
            else
            {
                gMCData.mos_ctrl |= DSG_MASK;
            }
        }

        if(gMCData.predsg_ctrl == 1)//预充程序有效或通信开启预充管，则使能预充管
        {
            gMCData.mos_ctrl |= PDS_MASK;
        }
        else
        {
            gMCData.mos_ctrl &= ~PDS_MASK;
        }
    }
    if(gMCData.mos_ctrl & PDS_MASK)	//打开预充管
    {
        
        //小电流检测功能
        #ifdef BMS_ENABLE_LITTLE_CURRENT_DET
        if(False == sLastPreStat)
        {
            BSPTaskStart(TASK_ID_LOW_CURR_SAM_TASK, 2);
            gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampBaseOK = False; 	//基准采样完成标志清除
            PT_WAIT_UNTIL(&gPtMosCtrl,True == gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampBaseOK);
            
            sLastPreStat = True;
            PRE_DSG_UP();
            BITSET(gBatteryInfo.Status.IOStatus,2);    
            
            BSPTaskStart(TASK_ID_LOW_CURR_SAM_TASK, 20);
            
            gSampleData.SampFlag.SampFlagBit.IsPreSampOk = False;
        }
        //PT_WAIT_UNTIL(&gPtMosCtrl,True == gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampOK);
        
        #else
        
        PRE_DSG_UP();
        BITSET(gBatteryInfo.Status.IOStatus,2);    
        
        #endif 
        
//        //打开预放电
//        if(False == gMCData.PreStat)
//        {
//            OZ3717EventFlagSet(OZ3717_EVE_CTRL_EN_PREDCHG);   
//            //BITSET(gBatteryInfo.Status.IOStatus,2);             
//        }
    }
    else
    {
        PRE_DSG_DN();
        //printf("ctrl %d %d\n",gSampleData.PreVolt,HAL_GetTick()); //检测后对比
        BITCLR(gBatteryInfo.Status.IOStatus,2);
        
        #ifdef BMS_ENABLE_LITTLE_CURRENT_DET
        if(True == sLastPreStat)
        {
            //关闭小电流检测功能
            gSampleData.LowCurrFlag.LowCurrFlagBit.IsInitAll = True;
            BSPTaskStop(TASK_ID_LOW_CURR_SAM_TASK);
            sLastPreStat = False;
            
            gSampleData.P1LowCurrVal = 0;
            gSampleData.P2LowCurrVal = 0;
            gSampleData.P1ADBaseVolt = 0;
            gSampleData.P2ADBaseVolt = 0;
            gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampOK = False; 	//采样完成标志清除 
            gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampBaseOK = False; 	//基准采样完成标志清除 
            gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampIng = False;
            
            LOW_CURRENT0_3V3_OFF();
            LOW_CURRENT1_3V3_OFF();
        }
        
        #endif          
        
//        //关闭预放电
//        if(True == gMCData.PreStat)
//        {
//            OZ3717EventFlagSet(OZ3717_EVE_CTRL_DIS_PREDCHG);   
//            //BITCLR(gBatteryInfo.Status.IOStatus,2);
//        }
    }

    //if((((gMCData.mos_ctrl&0x03) != SH367309MosfetGet()) ||g_PROTECT_CTRL.afe_flag.fresh_flag||timer_check(gMCData.timerid[0]))&& afe_is_mosfet_set_ok())
    if(((((gMCData.mos_ctrl&0x03) != SH367309MosfetGet()) && (gSHAFEData.MosUptFlg == True)) || (1 == gSHAFEData.protflag)) \
    		&& (0 == (SH367309EventFlagGet() & SH367309_EVE_CTRL_MOS)))
    {
        gSHAFEData.MosUptFlg = False;
        SH367309MosfetSet(gMCData.mos_ctrl);
        SH367309EventFlagSet(SH367309_EVE_CTRL_MOS);
        gSHAFEData.protflag = 0;
    }
    
    PT_INIT(&gPtMosCtrl);
    PT_END(&gPtMosCtrl);
}

/*****************************************end of MOSCtrl.c*****************************************/
