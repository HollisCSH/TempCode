//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: IOCheck.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 输入IO检测和保险丝检测源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
//=============================================================================================
//包含头文件
//=============================================================================================
#include "IOCheck.h"
#include "BatteryInfo.h"
#include "DataDeal.h"
#include "BSPTimer.h"

#include "BSP_TIM.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "SH367309.h"
#include "MBFindMap.h"
#include "DTCheck.h"
#include "DTTimer.h"
#include "MOSCtrl.h"
#include "TestMode.h"
#include "CommCtrl.h"
#include "version.h"
#include "CanComm.h"
#include "HMAC.h"
//#include "BSPSysTick.h"
#ifdef BMS_ENABLE_NB_IOT
#include "GSMTask.h"
#endif

//=============================================================================================
//定义全局变量
//=============================================================================================
u8 gIOWakeUpCond = 0;   //IO唤醒标志变量
u8 gFuseDetEn = 0;	    //保险丝检测使能 1：使能；0：不使能
u8 gFuseFault = 0;	    //保险丝故障 1：故障；0：正常
u8 gExModuleDetEn = 0;	//外置模块检测使能 1：使能；0：不使能
u8 gExModuleFault = 0;	//外置模块故障 1：故障；0：正常
u8 gExModuleOff = 0;    //外置模块断开标志
u8 gExModuleOffLine = 0;//外置模块完全断开标志，可以进入休眠
static u8 sExOutputFlag =False;    //外置模块配置成输出标志

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void IOCheckInit(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功	能	: IO检测初始化函数
//注	意	:
//=============================================================================================
void IOCheckInit(void)
{
	IOCheckWakeupCondReset();

	gFuseDetEn = 1;	//使能
	gFuseFault = 0;
    gExModuleOff = True;
    gExModuleOffLine = True;
}

//=============================================================================================
//函数名称	: void IOCheckHallIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测霍尔引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckHallIntCallback(void)
{
    gIOWakeUpCond |= WAKE_COND_HALL_MASK;

	//使用HALL触发对保险丝的检测
	//如果检测状态为未检测或者检测已经完成，则开始添加检测任务，否则不添加，避免任务冲突重叠
	if(0 == gFuseDetEn)
	{
		gFuseDetEn = 1;
	}
}

//=============================================================================================
//函数名称	: void IOCheckRemoveIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测拆开引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckRemoveIntCallback(void)
{
    gIOWakeUpCond |= WAKE_COND_PHOTO_MASK;
}

//=============================================================================================
//函数名称	: void IOCheckRemoveResIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测拆开后恢复引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckRemoveResIntCallback(void)
{
    gIOWakeUpCond |= WAKE_COND_RESPHOTO_MASK;
}

//=============================================================================================
//函数名称	: void IOCheckNFCIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测NFC中断引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckNFCIntCallback(void)
{
    gIOWakeUpCond |= WAKE_COND_NFC_MASK;
}

//=============================================================================================
//函数名称	: void IOCheckACCIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测ACC加速度计中断引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckACCIntCallback(void)
{
    gIOWakeUpCond |= WAKE_COND_ACC_MASK;
    gACCIntTestFlag = 1;
}

//=============================================================================================
//函数名称	: void IOCheckACCWakeIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测ACC加速度计唤醒中断引脚调用函数
//注    意	:
//=============================================================================================
void IOCheckACCWakeIntCallback(void)
{
    gIOWakeUpCond |= WAKE_COND_ACC_INT_MASK;
}

//=============================================================================================
//函数名称	: void IOCheckExModuleIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 外置模块检测中断引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckExModuleIntCallback(void)
{
    gIOWakeUpCond |= WAKE_COND_EXMODULE_MASK;
	//使用INT触发对保险丝的检测
	//如果检测状态为未检测或者检测已经完成，则开始添加检测任务，否则不添加，避免任务冲突重叠
	if(0 == gFuseDetEn)
	{
		gFuseDetEn = 1;
	}
}

//=============================================================================================
//函数名称	: void IOCheckCANIntCallback(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测CAN中断引脚中断调用函数
//注    意	:
//=============================================================================================
void IOCheckCANIntCallback(void)
{
    gIOWakeUpCond |= WAKE_COND_CAN_INT_MASK;
}

//=============================================================================================
//函数名称	: void IOCheckWakeupCondReset(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测唤醒标志位清零
//注    意	:
//=============================================================================================
void IOCheckWakeupCondReset(void)
{
    gIOWakeUpCond = 0;
}

//=============================================================================================
//函数名称	: void IOCheckWakeupCondClr(u8 Cond)
//函数参数	: Cond:要设置的唤醒标志位
//输出参数	:
//静态变量	:
//功    能	: IO检测唤醒标志位设置
//注    意	:
//=============================================================================================
void IOCheckWakeupCondClr(u8 Cond)
{
    gIOWakeUpCond &= ~Cond;
}

//=============================================================================================
//函数名称	: u8 IOCheckWakeupCondGet(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 获取IO检测唤醒标志位
//注    意	:
//=============================================================================================
u8 IOCheckWakeupCondGet(void)
{
    return gIOWakeUpCond;
}

//=============================================================================================
//函数名称	: void IOCheckMainTask(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测主任务函数
//注    意	: 50ms周期
//=============================================================================================
void IOCheckMainTask(void)
{
	u8 removestat = 0;	//拆开检测状态
    static u8 hallstat = 1;	//hall状态
    static u8 lasthallstat = 0;
    static u8 ExModuleStat = 0; //外置模块在位状态
    static u8 LastExModuleStat = 1; 
    static u8 LastIsExModule = False;  //上次状态
    static u8 LastIsCertifi = False;
    #if defined(USE_B22_IM_PCBA) || defined(USE_B20_TOPBAND_PCBA)
    #elif defined(USE_B21_IM_PCBA)
    static u16 sTimer = 0;
    #endif
    static u8 RstFilt = 0;
    static u8 RstTim = 0;  
    static u8 ExModutTimer = 0,ExModutOffTimer = 0,ExModutFirstFlg = True;
//	static u8  filttimer1 = 0;
//	static u8  filttimer2 = 0;
//	u8 prestat = 0;	//预放电管状态
    
    /* 增加霍尔触发条件 */
	//hallstat = BSPGPIOGetPin(HALL_CHECK_INT_PORT,HALL_CHECK_INT_PIN);
	//removestat = BSPGPIOGetPin(REMOVE_CHECK_INT_PORT,REMOVE_CHECK_INT_PIN);
    removestat = HAL_GPIO_ReadPin(REMOVE_INT_PORT,REMOVE_INT_PIN);
    if(False == sExOutputFlag)
    {
    //ExModuleStat = BSPGPIOGetPin(EXTERNAL_MODULE_INT_PORT,EXTERNAL_MODULE_INT_PIN);
			ExModuleStat = HAL_GPIO_ReadPin(MODULE_INT_PORT,MODULE_INT_PIN);
    }
//	prestat = BSPGPIOGetPin(PRE_DSG_CTR_PORT,PRE_DSG_CTR_PIN);

//	//霍尔接上，由高电平变为低电平，下降沿
//    if(1 == hallstat)
//    {
//    	BITCLR(gBatteryInfo.Status.IOStatus,0);
//        //在AFE一处关闭DSG EN信号
//        //BSPGPIOClrPin(DSG_ENABLE_PORT,DSG_ENABLE_PIN);
//        if(SH367309_MOS_OFF == BITGET(gBatteryInfo.Status.MOSStatus,0)
//            || (SH367309_MOS_OFF == BITGET(gBatteryInfo.Status.MOSStatus,1)))
//        {
//            sTimer++;   //延时2秒
//            if(sTimer >= (2000 / 50))
//            {
//                sTimer = 0;
//                BSPGPIOClrPin(DSG_EN_PORT,DSG_EN_PIN);
//            }
//        }            
//        
//        gMCData.IsPreOver400mA = False;
//    }
//    else	//霍尔接上
//    {
//        sTimer = 0;
//        BITSET(gBatteryInfo.Status.IOStatus,0);
//        BSPGPIOSetPin(DSG_EN_PORT,DSG_EN_PIN); //先使能DSG_EN,再控制AFE的DSG输出。
//        
//        //新增一个引脚检测预放过载
//        //gMCData.IsPreOver400mA = (False == BSPGPIOGetPin(PRE_DSG_OVER_400MA_PORT,PRE_DSG_OVER_400MA_PIN));
//    }

    //拆开检查，由高电平变为低电平，下降沿
    if(1 == removestat)
    {
    	BITCLR(gBatteryInfo.Status.IOStatus,1);
    }
    else	//拆开
    {
        BITSET(gBatteryInfo.Status.IOStatus,1);
    }
    
    //外置模块检查，由高电平变为低电平，下降沿
    if(1 == ExModuleStat)
    {
        //外置模块不在位超时
        ExModutOffTimer++;
        gExModuleOff = True;
        if(ExModutOffTimer >= EXTERN_MODULE_OFF_TIME)
        {
            //延时多一秒再关CAN
            if(ExModutOffTimer >= (EXTERN_MODULE_OFF_TIME + 20))
            {
                gExModuleOffLine = True;    //完全断开，用于判断休眠
                //MODULE_CAN_3V3_OFF();
                //MODULE_CAN_5V_OFF();
                ExModutOffTimer = EXTERN_MODULE_OFF_TIME + 20;
            }
            gStructCanFlag.DeviceFlag.DeviceFlagByte = 0;
            gStructCanReq.SendFlag.SendFlagBit.IsSendReq = True;
            BITCLR(gBatteryInfo.Status.IOStatus,3);
        }
        
        #if defined(USE_B22_IM_PCBA) || defined(USE_B20_TOPBAND_PCBA)
        #elif defined(USE_B21_IM_PCBA)
        //在AFE一处关闭DSG EN信号
        //BSPGPIOClrPin(DSG_ENABLE_PORT,DSG_ENABLE_PIN);
        if(SH367309_MOS_OFF == BITGET(gBatteryInfo.Status.MOSStatus,0)
            || (SH367309_MOS_OFF == BITGET(gBatteryInfo.Status.MOSStatus,1)))
        {
            sTimer++;   //延时2秒
            if(sTimer >= (2000 / 50))
            {
							sTimer = 0;
							//BSPGPIOClrPin(DSG_EN_PORT,DSG_EN_PIN);
							HAL_GPIO_WritePin(DSG_EN_PORT , DSG_EN_PIN , GPIO_PIN_RESET);
            }
        }        
        #endif
        
        ExModutTimer = 0;
        gExModuleDetEn = 0;
        ExModutFirstFlg = True;
    }
    else	//在位
    {
        u8 delaytime = 0;
        ExModutOffTimer = 0;
        gExModuleOff = False;
        gExModuleOffLine = False;
       // BITSET(gBatteryInfo.Status.IOStatus,3);
        #if defined(USE_B22_IM_PCBA) || defined(USE_B20_TOPBAND_PCBA)
        #elif  defined(USE_B21_IM_PCBA)
        //BSPGPIOSetPin(DSG_EN_PORT,DSG_EN_PIN); //先使能DSG_EN,再控制AFE的DSG输出。
		HAL_GPIO_WritePin(DSG_EN_PORT , DSG_EN_PIN , GPIO_PIN_SET);//先使能DSG_EN,再控制AFE的DSG输出。
		#endif
        delaytime = (True == ExModutFirstFlg)?10:100;
        
        if((True == ExModutFirstFlg) && ExModutTimer++ > delaytime)    //1/5秒周期检测一次外置短路故障
        {
            ExModutFirstFlg = False;
            ExModutTimer = 0;
            if(0 == gExModuleDetEn)
            {
                gExModuleDetEn = 1;
            }
        }        
    }

    //预放电管状态
//    (0 == prestat)?BITCLR16(gBatteryInfo.Status.IOStatus,2)  //此处置位放在MOSCtrl上
//           :BITSET(gBatteryInfo.Status.IOStatus,2);
    
    //重新插上，清除电流锁定，清除睡眠故障
    if(LastExModuleStat != gExModuleOff || LastIsExModule != gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule
        || LastIsCertifi != HMACReadCertifiSta())    //状态发送改变
    {
        LastExModuleStat = gExModuleOff;    //上次的外置模块状态
        LastIsExModule = gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule;
        LastIsCertifi = HMACReadCertifiSta();
        
        if((0 == gExModuleOff) && (gPCBTest.presentstep==0))   //外置模块重新接上
        {
						u16 tmp=0;
            mb_ctrl_ctrl_w(512,&tmp);   //清除睡眠故障
            //if(gBatteryInfo.Fault.FaultInfo[2]&(OPT_FAULT1_UV1|OPT_FAULT1_UV2))
            //{
                gMCData.predsg_err = 0;//复位故障标志
            //}              
            DTCheckOCLockStatusClr(0x03);       //清除oc lock
            if(True == gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule)
            {
                //有外置模块插入通信才开mos管
                tmp= CTRL1_CHG_MASK|CTRL1_DSG_MASK;
                mb_ctrl_ctrl_w(512,&tmp);           //开充放电MOS管
            }

			// add ...
			MODULE_CAN_3V3_ON();
			MODULE_CAN_5V_ON();     

            gMCData.mos_fault = 0;
            gMCData.chg_mos_fault_cnt=0;
            gMCData.dsg_mos_fault_cnt=0;    
        }
        else if((0 == gExModuleOff)&&(gPCBTest.presentstep==8))//测试老化模式拿开磁铁清除oclock
        {
            //鉴权后才开管子
            //u16 tmp=0;
            //mb_ctrl_ctrl_w(512,&tmp);   //清除睡眠故障
            DTCheckOCLockStatusClr(0x03);       //清除oc lock
            //if(True == HMACReadCertifiSta())
            //{
                u16 tmp= CTRL1_CHG_MASK|CTRL1_DSG_MASK|CTRL1_CPI_MASK;
                mb_ctrl_ctrl_w(512,&tmp);           //开充放电MOS管
            //}
            gMCData.predsg_err = 0;//复位故障标志
            gMCData.mos_fault = 0;
            gMCData.chg_mos_fault_cnt=0;
            gMCData.dsg_mos_fault_cnt=0;            
        }
		else if(1 == gExModuleOff)
		{
			// add ...
			//BSPGPIOClrPin(MODULE_VCC_CTR_PORT,MODULE_VCC_CTR_PIN);  //关闭外置模块电源
        HAL_GPIO_WritePin(VCC_MODULE_PORT , VCC_MODULE_PIN , GPIO_PIN_RESET);  //关闭外置模块电源    
            //关闭充放电管
            gMBCtrl.ctrl &= 0xFFF0;
		}
    }
    
    //重新插上，清除电流锁定，清除睡眠故障
    if(lasthallstat != hallstat)    //状态发送改变
    {
        lasthallstat = hallstat;    //上次的hall状态
        
        if((1 == gExModuleOff) && (0 == hallstat) && (gPCBTest.presentstep==0))   //hall重新接上 并且 外置模块不在位
        {
            u16 tmp=0;
            mb_ctrl_ctrl_w(512,&tmp);   //清除睡眠故障
            DTCheckOCLockStatusClr(0x03);       //清除oc lock
            
            if(gBatteryInfo.Fault.FaultInfo[2]&(OPT_FAULT1_UV1|OPT_FAULT1_UV2))
            {
                gMCData.predsg_err = 0;//复位故障标志
            }  
            gMCData.mos_fault = 0;
            gMCData.chg_mos_fault_cnt=0;
            gMCData.dsg_mos_fault_cnt=0;    

            RstFilt++;    
            //printf("Hall In\n");  //H暂时关闭
            if(1 == RstFilt)    //第一次先清零时间
            {
                RstTim = 0;
            }
        
        }
        else if((1 == gExModuleOff) && (0 == hallstat)&&(gPCBTest.presentstep==8))//测试老化模式拿开磁铁清除oclock
        {
            DTCheckOCLockStatusClr(0x03);//清除oc lock故障
            gMCData.mos_fault = 0;
            gMCData.chg_mos_fault_cnt=0;
            gMCData.dsg_mos_fault_cnt=0;            
        }
    }
    //10秒钟发生6次hall激活，则复位
    RstTim++;
    if(RstTim >= 200)
    {
        RstFilt = 0;
        RstTim = 0;
    }
    else
    {
        if(RstFilt >= 6)
        {
            RstFilt = 0;
            SoftReset();
        }
    }
    
    IOCheckFuseDetectTask();	//保险丝检测任务
    IOCheckModuleShortDetectTask(); //外置模块短路检测任务
    #ifdef BMS_ENABLE_NB_IOT
    IOCheckNBIoTDetectTask();       //NB模块接入检测任务
    #endif
    
    #if defined(USE_B22_IM_PCBA)
    IOCheckSCShortDetectTask();
    #endif
}

#ifdef CANBUS_MODE_JT808_ENABLE

//=============================================================================================
//函数名称	: void ModuleHWReset(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 外置模块硬件重启
//注    意	: 100ms延时
//=============================================================================================
//void ModuleHWReset(void)
//{
//	extern volatile unsigned int system_ms_tick;
//	uint32_t cur_ms = system_ms_tick;
//	MODULE_PWR_OFF();
//	while( system_ms_tick - cur_ms < 100 );
//	MODULE_PWR_ON();
//}

//=============================================================================================
//函数名称	: void ModuleIntPinSetOutput(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 外置模块检测引脚配置成输出
//注    意	: 
//=============================================================================================
void ModuleIntPinSetOutput(void)
{
    //BSPGPIOSetDir(EXTERNAL_MODULE_INT_PORT,EXTERNAL_MODULE_INT_PIN,1);
    GPIO_InitTypeDef  gpioinitstruct;
  	gpioinitstruct.Pin = MODULE_INT_PIN;
    gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
    gpioinitstruct.Pull = GPIO_NOPULL;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(MODULE_INT_PORT, &gpioinitstruct);
    HAL_GPIO_WritePin(MODULE_INT_PORT, MODULE_INT_PIN, GPIO_PIN_SET);
    sExOutputFlag = True;
}

//=============================================================================================
//函数名称	: void ModuleIntPinSetInput(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 外置模块检测引脚配置成输入
//注    意	: 
//=============================================================================================
void ModuleIntPinSetInput(void)
{
    //BSPGPIOSetDir(EXTERNAL_MODULE_INT_PORT,EXTERNAL_MODULE_INT_PIN,0);
    //HAL_GPIO_WritePin(MODULE_INT_PORT, MODULE_INT_PIN, GPIO_PIN_RESET);
    GPIO_InitTypeDef  gpioinitstruct;
  	gpioinitstruct.Pin = MODULE_INT_PIN;
    gpioinitstruct.Pull = GPIO_NOPULL;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpioinitstruct.Mode = GPIO_MODE_IT_FALLING;		//下降沿触发	
    HAL_GPIO_Init(MODULE_INT_PORT, &gpioinitstruct);
    sExOutputFlag = False;    
}

#endif


//=============================================================================================
//函数名称	: void IOCheckFuseDetectTask(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: IO检测保险丝检测任务
//注    意	: 50ms周期
//=============================================================================================
void IOCheckFuseDetectTask(void)
{
	static u8 sstep = 0;
    static u16 delaytimer = 0;
    
//不用gTimer1ms延时是为了防止在gTimer1ms溢出后，此函数会返回不执行    
//	if(0 == gFuseDetEn || gTimer1ms < 5000)	//上电5秒前，不检测
//	{
//		faulttimer = 0;
//		normaltimer = 0;
//		return;
//	}
    if((delaytimer <= 100) || (0 == gFuseDetEn) || (0 == DataDealGetBMSDataRdy(0x20)))	//上电5秒前，不检测
    {
        delaytimer++;
        if(delaytimer > 100)
        {
            delaytimer = 101;   //5秒计时已经到
        }
    	return;
    }       

	switch(sstep)
	{
		case 0:
			if(gBatteryInfo.Data.TolVolt > 3200)	//总电压大于32V才开启检测，单位10mv
			{
				//BSPGPIOSetPin(FUSE_DET_CTR_PORT,FUSE_DET_CTR_PIN);  //开启检测
				HAL_GPIO_WritePin(FUSE_DET_EN_PORT , FUSE_DET_EN_PIN , GPIO_PIN_SET);
				sstep = 1;
			}
			else
			{
                sstep = 0;
				gFuseFault = 0;	//保险丝正常，不检测
				gFuseDetEn = 0;
				//BSPGPIOClrPin(FUSE_DET_CTR_PORT,FUSE_DET_CTR_PIN);  //关闭检测
				HAL_GPIO_WritePin(FUSE_DET_EN_PORT , FUSE_DET_EN_PIN , GPIO_PIN_RESET);
				return;
			}
			break;

		case 1:
			//Fuse断开时，输出高电平;Fuse良好时，输出低电平。
			//if(1 == BSPGPIOGetPin(FUSE_DET_GET_PORT, FUSE_DET_GET_PIN))		//故障
			if(1 == HAL_GPIO_ReadPin(FUSE_DET_PORT, FUSE_DET_PIN))		//故障
			{
				gFuseFault = 1;
			}
			else if(0 == HAL_GPIO_ReadPin(FUSE_DET_PORT, FUSE_DET_PIN))	 //正常
			{
				gFuseFault = 0;
			}

			if(1 == gFuseFault)
			{
                gFuseDetEn = 1; //如果保险丝失效则继续监测
			}
            else
            {
                gFuseDetEn = 0;	//接收检测，等待下一次使能
                //BSPGPIOClrPin(FUSE_DET_CTR_PORT,FUSE_DET_CTR_PIN);  //关闭检测
								HAL_GPIO_WritePin(FUSE_DET_EN_PORT , FUSE_DET_EN_PIN , GPIO_PIN_RESET);
            }
            sstep = 0;
			break;

		default:
			sstep = 0;
            gFuseFault = 0;	//保险丝正常，不检测
			gFuseDetEn = 0;			
            break;
	}
}

//=============================================================================================
//函数名称	: void IOCheckModuleShortDetectDeal(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: IO检测外置模块短路处理任务
//注    意	: 
//=============================================================================================
void IOCheckModuleShortDetectDeal(void)
{
//    pin_settings_config_t cModuleDetConfigArr[1] = //[NUM_OF_CONFIGURED_PINS] =
//    {    
//        /* MODULE_DET_INT 外置模块短路检测引脚 */
//        {
//            .base          = MODULE_DET_GET_GPIOBASE,
//            .pinPortIdx    = MODULE_DET_GET_PN,
//            .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
//            .passiveFilter = false,
//            .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
//            .mux           = PORT_MUX_AS_GPIO,
//            .pinLock       = false,
//            .intConfig     = PORT_DMA_INT_DISABLED,
//            .clearIntFlag  = false,
//            .gpioBase      = MODULE_DET_GET_BASE,
//            .direction     = GPIO_INPUT_DIRECTION,
//            .digitalFilter = false,
//            .initValue = 0,
//        },
//    };
    
    //if(BSPGPIOGetOutPutPin(MODULE_VCC_CTR_PORT,MODULE_VCC_CTR_PIN))
    if(HAL_GPIO_ReadPin(VCC_MODULE_PORT,VCC_MODULE_PIN))  //HAL_GPIO_ReadPin() 可以读取输入和输出状态
    {
        //if(1 == BSPGPIOGetPin(MODULE_DET_GET_PORT, MODULE_DET_GET_PIN))		//短路
        if(1 == HAL_GPIO_ReadPin(MODULE_SHORT_PORT, MODULE_SHORT_PIN))		//短路
        {
            //BSPGPIOClrPin(MODULE_VCC_CTR_PORT,MODULE_VCC_CTR_PIN);  //关闭VCC
            HAL_GPIO_WritePin(VCC_MODULE_PORT , VCC_MODULE_PIN , GPIO_PIN_RESET);//关闭VCC
            gExModuleFault = 1;	//外置模块短路
            BITCLR(gBatteryInfo.Status.IOStatus,3);
            GPIO_SET_INPUT( MODULE_SHORT_PORT , MODULE_SHORT_PIN );
        }
        else if(0 == HAL_GPIO_ReadPin(MODULE_SHORT_PORT, MODULE_SHORT_PIN))	 //正常
        {
            ;
        }
    }
    //拔出    
    else
    {
        //PINS_DRV_Init(1, cModuleDetConfigArr);        
         GPIO_SET_INPUT( MODULE_SHORT_PORT , MODULE_SHORT_PIN );
    }
}

//=============================================================================================
//函数名称	: void IOCheckModuleShortDetectTask(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: IO检测外置模块短路检测任务
//注    意	: 50ms周期
//=============================================================================================
void IOCheckModuleShortDetectTask(void)
{
	static u8 faulttimer = 0;
	static u8 normaltimer = 0;
	static u8 sstep = 0;
    static u16 delaytimer = 0;
//    pin_settings_config_t cModuleDetConfigArr[1] = //[NUM_OF_CONFIGURED_PINS] =
//    {    
//        /* MODULE_DET_INT 外置模块短路检测引脚 */
//        {
//            .base          = MODULE_DET_GET_GPIOBASE,
//            .pinPortIdx    = MODULE_DET_GET_PN,
//            .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
//            .passiveFilter = false,
//            .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
//            .mux           = PORT_MUX_AS_GPIO,
//            .pinLock       = false,
//            .intConfig     = PORT_DMA_INT_DISABLED,
//            .clearIntFlag  = false,
//            .gpioBase      = MODULE_DET_GET_BASE,
//            .direction     = GPIO_INPUT_DIRECTION,
//            .digitalFilter = false,
//            .initValue = 0,
//        },
//    }; 
    
    if((delaytimer <= 50) || (0 == DataDealGetBMSDataRdy(0x20))  || (0 == gExModuleDetEn))	//上电5秒前，不检测
    {
        sstep = 0;
        delaytimer++;
        if(delaytimer > 50)
        {
            delaytimer = 51;   //5秒计时已经到
        }
        faulttimer = 0;
		normaltimer = 0;
        
        if(1 == gExModuleFault)
        {
//            cModuleDetConfigArr[0].intConfig = PORT_DMA_INT_DISABLED;
//            PINS_DRV_Init(1, cModuleDetConfigArr);
            GPIO_SET_INPUT( MODULE_SHORT_PORT , MODULE_SHORT_PIN );
        }
    	return;
    }       
    
	switch(sstep)
	{
		case 0:

            //BSPGPIOSetPin(MODULE_VCC_CTR_PORT,MODULE_VCC_CTR_PIN);            //开启检测
            GPIO_SET_INPUT(MODULE_SHORT_PORT, MODULE_SHORT_PIN);                //检测脚设置为输入
			HAL_GPIO_WritePin(VCC_MODULE_PORT , VCC_MODULE_PIN , GPIO_PIN_SET);	//开启检测
            //BSPSysDelay1ms(1);
            //HAL_Delay(1);     //对于1毫秒延迟不准确
            delay_xus(1000);
            sstep++;
            //配置为高电平触发
//            cModuleDetConfigArr[0].intConfig = PORT_INT_LOGIC_ONE;
//            PINS_DRV_Init(1, cModuleDetConfigArr);
            if(1 == HAL_GPIO_ReadPin(MODULE_SHORT_PORT, MODULE_SHORT_PIN))		//1ms后发现短路立即关闭                
			{
                IOCheckModuleShortDetectDeal();                                 //短路处理
            }
            else
            {
                GPIO_SET_RISE_EXIT(MODULE_SHORT_PORT,MODULE_SHORT_PIN);         //设置IO为高电平触发               
            }
			break;

		case 1:
            //外部模块短路，MODULE_DET_INT为高
            //外部模块正常，MODULE_DET_INT为低
			//if(1 == BSPGPIOGetPin(MODULE_DET_GET_PORT, MODULE_DET_GET_PIN))		//短路
			if(1 == HAL_GPIO_ReadPin(MODULE_SHORT_PORT, MODULE_SHORT_PIN))		//短路
			{
				faulttimer++;
				normaltimer = 0;
			}
			//else if(0 == BSPGPIOGetPin(MODULE_DET_GET_PORT, MODULE_DET_GET_PIN))	 //正常
			else if(0 == HAL_GPIO_ReadPin(MODULE_SHORT_PORT, MODULE_SHORT_PIN))	 //正常
			{
				faulttimer = 0;
				normaltimer++;
			}

			//连续出现2次，则判定状态
			if((2 <= faulttimer) && (0 == normaltimer))
			{
				gExModuleFault = 1;	//外置模块短路
                BITCLR(gBatteryInfo.Status.IOStatus,3);
				//MODULE_CAN_3V3_OFF();
				//MODULE_CAN_5V_OFF();				
			}
			else if((2 <= normaltimer) && (0 == faulttimer))
			{
				gExModuleFault = 0;	//外置模块正常
                BITSET(gBatteryInfo.Status.IOStatus,3);
			}
			else
			{
				gExModuleFault = 2;	//外置模块中间态
			}

			if(0 == gExModuleFault || 1 == gExModuleFault)
			{
				gExModuleDetEn = 0;	//接收检测，等待下一次使能
				sstep = 0;
				faulttimer = 0;
				normaltimer = 0;
			}
			break;

		default:
			;
			break;
	}
}
//=============================================================================================
//函数名称	: void HAL_GPIO_EXTI_Falling_Callback(void)
//函数参数	: 具体中断线
//输出参数	: void
//静态变量	: void
//功    能	: 下降沿中断
//注    意	: 系统函数 由 stm32g0xx_it.c 中的中断函数引入
//=============================================================================================
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == ALARM_AFE_PIN)
	{
		SH367309AlertIntCallback();		//AFE告警处理函数
	}
	else if(GPIO_Pin == REMOVE_INT_PIN)
	{
		IOCheckRemoveIntCallback();
	}
	else if(GPIO_Pin == MODULE_INT_PIN)
	{
		IOCheckExModuleIntCallback();   //外置模块检测中断引脚中断调用函数
	}
	else if(GPIO_Pin == INT_4G_PIN)
	{

	}
}

//=============================================================================================
//函数名称	: void HAL_GPIO_EXTI_Rising_Callback(void)
//函数参数	: 具体中断线
//输出参数	: void
//静态变量	: void
//功    能	: 上升沿中断
//注    意	: 系统函数 由 stm32g0xx_it.c 中的中断函数引入
//=============================================================================================
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == MODULE_SHORT_PIN)
    {
        IOCheckModuleShortDetectDeal();
    }
	#ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
	else if(GPIO_Pin == ACC_INT1_PIN)
	{
		if(0x0D == gPCBTest.presentstep || 0x03 == gPCBTest.presentstep)
		{
            if(0x0D == gPCBTest.presentstep)
            {
                gPCBTest.result |= 0x01;        //用于测试模式字输出结果
            }
            //IOCheckACCIntCallback();				//ACC INT1唤醒回调函数，active中断
		}
		else
		{
            IOCheckACCWakeIntCallback();
            //IOCheckACCIntCallback();				//ACC INT1唤醒回调函数，active中断
            DA213SetCtrl(DA213_GET_INT);
            DA213ReadIntInit();
            BSPTaskStart(TASK_ID_ACCELERO_TASK, 1);	//立刻读取中断标志
        }
	}
	else if(GPIO_Pin == ACC_INT2_PIN)
	{
		if(0x03 == gPCBTest.presentstep || 0x0D == gPCBTest.presentstep)
		{
            IOCheckACCIntCallback();				//ACC INT2唤醒回调函数，自由落体中断
		}
		else
		{
            IOCheckACCWakeIntCallback();
            //IOCheckACCIntCallback();				//ACC INT2唤醒回调函数，自由落体中断
            DA213SetCtrl(DA213_GET_INT);
            DA213ReadIntInit();
            BSPTaskStart(TASK_ID_ACCELERO_TASK, 1);	//立刻读取中断标志
		}
	}
	#endif  
}

#if defined(USE_B22_IM_PCBA)

#define   MCU_CTRL_MOS_OFF  BSPGPIOSetPin(MCU_DSG_PORT,MCU_DSG_PIN)
#define   MCU_CTRL_MOS_ON   BSPGPIOClrPin(MCU_DSG_PORT,MCU_DSG_PIN)

//=============================================================================================
//函数名称	: void IOCheckSCShortDetectTask(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: IO检测电池短路检测任务
//注    意	: 50ms周期
//=============================================================================================
void IOCheckSCShortDetectTask(void)
{
    static u16 delaytimer = 0;
    static u8 sstep = 0;
    
    pin_settings_config_t cSCDetConfigArr[1] =
    {    
        //SC_DETECT 短路检测
        {
            .base          = SC_DETECT_GPIOBASE,
            .pinPortIdx    = SC_DETECT_PN,
            .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
            .passiveFilter = false,
            .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
            .mux           = PORT_MUX_AS_GPIO,
            .pinLock       = false,
            .intConfig     = PORT_INT_FALLING_EDGE, //下降沿触发
            .clearIntFlag  = false,
            .gpioBase      = SC_DETECT_BASE,
            .direction     = GPIO_INPUT_DIRECTION,
            .digitalFilter = false,
            .initValue = 0,
        }, 
    }; 
    
    if((delaytimer <= 20) || (0 == DataDealGetBMSDataRdy(0x20)))	//上电2秒前，不检测
    {
        sstep = 0;
        delaytimer++;
        if(delaytimer > 20)
        {
            delaytimer = 21;   //2秒计时已经到
        }
    	return;
    }

	switch(sstep)
	{
		case 0:
            //配置短路保护启动
            PINS_DRV_Init(1, cSCDetConfigArr);
            sstep++;
			break;
        
		case 1:
            if(DT_N == DTTimerGetState(DT_ID_DIS_SC))
            {
                MCU_CTRL_MOS_ON;
                sstep = 0;
            }
			break;

		default:
            sstep = 0;;
			break;
	}
}

//=============================================================================================
//函数名称	: void IOCheckSCShortDetectDeal(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: IO检测电池短路处理任务
//注    意	: 
//=============================================================================================
void IOCheckSCShortDetectDeal(void)
{
    const pin_settings_config_t cSCDetConfigArr[1] =
    {    
        //SC_DETECT 短路检测
        {
            .base          = SC_DETECT_GPIOBASE,
            .pinPortIdx    = SC_DETECT_PN,
            .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
            .passiveFilter = false,
            .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
            .mux           = PORT_MUX_AS_GPIO,
            .pinLock       = false,
            .intConfig     = PORT_DMA_INT_DISABLED,
            .clearIntFlag  = false,
            .gpioBase      = SC_DETECT_BASE,
            .direction     = GPIO_INPUT_DIRECTION,
            .digitalFilter = false,
            .initValue = 0,
        }, 
    }; 
    
//    if(0 == BSPGPIOGetPin(SC_DETECT_PORT, SC_DETECT_PIN))		//短路
//    {
        MCU_CTRL_MOS_OFF;   //控制关闭MOS管
        DTTimerSetState(DT_ID_DIS_SC,DT_F);	//短路
        PINS_DRV_Init(1, cSCDetConfigArr);
//    }
}

#endif

/*****************************************end of IOCheck.c*****************************************/
