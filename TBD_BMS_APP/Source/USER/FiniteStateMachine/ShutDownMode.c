//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: ShutDownMode.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 关机模式代码
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
//=============================================================================================
//包含头文件
//=============================================================================================
#include "ShutDownMode.h"
#include "SleepMode.h"
#include "BSPSystemCfg.h"
//#include "BSPWatchDog.h"
//#include "BSPRTC.h"
//#include "BSPGPIO.h"
#include "BSPTimer.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
//#include "BSPLowPow.h"
#include "BSPCriticalZoom.h"
#include "BSPHardWareIntf.h"
//#include "BSPPIT.h"
//#include "BSPCan.h"
#include "BSPSystemCfg.h"
//#include "BSPICS.h"
#include "Version.h"
#include "EventRecord.h"
#include "BatteryInfo.h"
#include "BatteryInfoGetSetApi.h"
#include "SH367309.h"
#include "MAX17205.h"
//#include "FM11NC08.h"
#include "CanComm.h"
#include "MOSCtrl.h"
#include "Comm.h"
#include "IOCheck.h"
#include "DTCheck.h"
#include "DTTimer.h"
//#include "BSPADC.h"
#include "Sample.h"
#include "Storage.h"
#include "MX25L16.h"
#include "CommCtrl.h"
#include "HMAC.h"
#include "DataDeal.h"
#include "TestMode.h"
#include "ParaCfg.h"
#include "DA213.h"
#include "pt.h"
#include "CurrIntegral.h"
#include "SocLTCompensate.h"
#include "GSMTask.h"
#include "Sim.h"
#include "WIFIDeal.h"

#include "BSP_IWDG.h"
#include "BSP_RTC.h"
#include "BSP_GPIO.h"
#include "BSP_PWR.h"
#include "BSP_TIM.h"
#include "BSP_CAN.h"
#include "BSP_UART.h"
#include "BSP_ADC.h"
#include "BSP_IIC.h"
#include "BSP_SYSCLK.h"

//=============================================================================================
//全局变量
//=============================================================================================
static pt gPTSHDN;					//关机任务pt线程控制变量
t_SHDN_COND gSHDNCond;            //关机模式相关结构体
t_SHUTDOWN_STATE gSHDNState;      //关机状态

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void ShutDownModeInit(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 关机模式初始化函数
//注    意	:
//=============================================================================================
void ShutDownModeInit(void)
{
	PT_INIT(&gPTSHDN);
    gSHDNCond.shdnen = 0;
}

//=============================================================================================
//函数名称	: void ShutDownModeCheck(void)
//函数参数	: void
//输出参数	: void
//静态变量	:
//功    能	: 关机模式判断，确认是否进入关机模式
//注    意	:
//=============================================================================================
void ShutDownModeCheck(void)
{
	static u16 cmdtimer = 0,uvtimer = 0;
    static u32 idletimer = 0;
    u8 IdleSta = (eChgDchg_Idle == SysStateGetChgDchgSta())? True : False;
	
	//欠压2级 并且 充电电流小于250mA
	/*
	if((DT_F == DTTimerGetState(DT_ID_VL_LV2)) && (gBatteryInfo.Data.Curr < 25))
	{
		uvtimer++;
		if(uvtimer > (30000 / 50))	//延迟30S
		{
			uvtimer = 0;
			gSHDNCond.cond |= SHDN_COND_UV4_MASK;
		}
	}
	else
	{
		uvtimer = 0;
	}
	*/  
    if((((gBatteryInfo.Fault.FaultInfo[2] & (OPT_FAULT1_UV1|OPT_FAULT1_UV2)) && (gPCBTest.presentstep == 0))
    || ((gBatteryInfo.Fault.FaultInfo[2] & OPT_FAULT1_UV2) && ((gPCBTest.presentstep > 0))))
    && (gBatteryInfo.Data.Curr < 25))//欠压故障等级4,并且充电电流小于250mA关机
    {
        uvtimer++;
        if(uvtimer > (900000 / 50))	//延迟900S
        {
            uvtimer = 0;
            gSHDNCond.cond |= SHDN_COND_UV4_MASK;
            if(((gBatteryInfo.Fault.FaultInfo[2] & OPT_FAULT1_UV2) && ((gPCBTest.presentstep > 0))))
                gPCBTest.presentstep = 0xff;
        }
    }
    else
    {
        uvtimer = 0;
    }
    
	//关机命令
	if(CommCtrlIsShDnMode())	
    {
		cmdtimer++;
		if(cmdtimer > (2000 / 50))	//延迟2S
		{
			cmdtimer = 0;
			gSHDNCond.cond |= SHDN_COND_SHDNCMD_MASK;
		}
    }
	else
	{
		cmdtimer = 0;
	}
	
	if(gMCData.predsg_inner5s == 1) //预放电5S内禁止关机
    {
        CommClearTimeout();//实现预放电完毕后再等待通信超时后关机
        gSHDNCond.cond = 0;
        //REC_TYPE &= ~REC_TYPE_TIMEOUT;
    }
	
	if(gReSetCmd == 0x7AB9)
    {
        gSHDNCond.cond |= SHDN_COND_RESET;
    }
	
	if(0 == BITGET(gBatteryInfo.Status.IOStatus,3))
	{
		IdleSta = False;    //清除休眠状态，重新计时
	}

    //浅度休眠时被短信/其他中断唤醒
    if(True == BatteryInfoExGetIsAwakened())
    {
        IdleSta = False;    //清除休眠状态，重新计时
        BatteryInfoExClrAwakened();
    }
    
    //空闲状态判断
    if(True == IdleSta)
    {
        idletimer++;
        if(idletimer >= EX_ENTER_LIGHT_SLEEP_TIME)
        {
            gSHDNCond.IdleFlag.IdleFlagBit.EnterLightSleep = True;
            
            //定时唤醒周期到
            if(0 == (idletimer % EX_EXIT_LIGHT_SLEEP_PERIOD))
            {
                gSHDNCond.IdleFlag.IdleFlagBit.WakeUp = True;
            }
            
            //达到关机时间
            gSHDNCond.IdleFlag.IdleFlagBit.EnterShutDown = (idletimer >= EX_ENTER_DEEP_SLEEP_TIME) ? True : False;
        }
        else
        {
            gSHDNCond.IdleFlag.IdleFlagBit.EnterLightSleep = False;
            gSHDNCond.IdleFlag.IdleFlagBit.EnterShutDown = False;        
        }
    }
    else
    {
        idletimer = 0;
        gSHDNCond.IdleFlag.IdleFlagBit.EnterLightSleep = False;
        gSHDNCond.IdleFlag.IdleFlagBit.EnterShutDown = False;
    }
    
    //通信超时关机条件，满足其中一个条件即关机：
    //1.通信超时且无磁铁
    //2.通信超时但是有磁铁，可是MOS管无控制
    //3.EnterShutDown标志为True，已经长时间处于空闲状态了
    if(((((1 == gCommPoll.commoff) && (1 == gExModuleOffLine))
        && ((0 == (gBatteryInfo.Status.IOStatus & (HALL_CHECK_SHIFT | EXTERN_MODULE_CHECK_SHIFT))) ||
        (((gBatteryInfo.Status.IOStatus & (HALL_CHECK_SHIFT | EXTERN_MODULE_CHECK_SHIFT))) && (0 == gBatteryInfo.Status.MOSStatus))))
        //21/7/18 增加条件：更改成外置模块通信在位 且 无机柜数据通信，才进入休眠
        || (True == gSHDNCond.IdleFlag.IdleFlagBit.EnterShutDown
        && True == gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule
        && False == gStructCanFlag.DeviceFlag.DeviceFlagBit.IsCabinet))
        #if defined(BMS_ENABLE_NB_IOT)
        && (gGSMCond.NeedWifiFlag == False) /* 增加休眠条件，没发送wifi定位成功则不休眠*/
        #endif
      )
    {
        idletimer = 0;
        gSHDNCond.IdleFlag.IdleFlagBit.EnterLightSleep = True;
        gSHDNCond.IdleFlag.IdleFlagBit.EnterShutDown = False;        
        gSHDNCond.cond |= SHDN_COND_COMM_MASK;
    }

	//if(gSHDNCond.cond > 0 || 1 == gCommPoll.commoff )
    if(gSHDNCond.cond > 0)
	{
		gSHDNCond.shdnen = 1;
	}
}

//=============================================================================================
//函数名称	: void ShutDownModeClearData(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 清除关机模式数据
//注    意	:
//=============================================================================================
void ShutDownModeClearData(void)
{
    gSHDNCond.cond = 0;
    gSHDNCond.shdnfuelflag = 0;
}


//=============================================================================================
//函数名称	: void ShutDownModeResume(void)
//函数参数	:
//输出参数	: 
//静态变量	:
//功	能	: 关机模式恢复
//注	意	:
//=============================================================================================
void ShutDownModeResume(void)
{
	//开启ADC
  //BSPADCEnable();
//  BSPADCInit(CONN_TEMP_ADC_CHAN_MASK | PCB_TVS_TEMP_ADC_CHAN_MASK | PRE_DSG_SHORT_ADC_CHAN_MASK | CELL_VOLT_16_ADC_CHAN_MASK,
//               ADC_SAMPMODE); //ADC初始化  
	BSPADCInit(); //ADC初始化
	//开启I2C
//	BSPIICInit(eIIC0);	//IIC0初始化
//	BSPIICInit(eIIC1);	//IIC1初始化
	BSPI2C_Init(SH367309_IIC_CHANNEL);	//AFE IIC初始化
	BSPI2C_Init(DA213_IIC_CHANNEL);	//ACC IIC初始化
    
    //开启SPI
	//BSPSPIInit(eSPI0,SPI0_MASTER,SPI0_BAUDRATE,SPI0_MODE);	//SPI0初始化
	//BSPSPIInit(eSPI1,SPI1_MASTER,SPI1_BAUDRATE,SPI1_MODE);	//SPI1初始化
	BSPSPIInit();	//SPI1初始化
	FLASH_POWER_DISABLE();  //拉高SPI1 CS，失能SPI芯片
	//BSPGPIOSetPin(NFC_CS_PORT,NFC_CS_PIN);		            //拉高SPI0 CS，失能SPI芯片
	//BSPGPIOSetPin(FLASH_CS_PORT,FLASH_CS_PIN);		        //拉高SPI1 CS，失能SPI芯片
	CanCommInit();  //CAN通信初始化
	BSPCan_Init();
   //FlexCanInit();

    
	//BSPTimerStart();	                     //定时器开启
	BSPPITInit();												 		 //定时器开启
	BSPRTCInit();                            //RTC开启
	gBatteryInfo.Status.DataReady = 0;
    
	MX25L16Init();	                         //FLASH上电	
	gDataDealFirstFlag = (DATADEAL_FIRST_FLAG_VOLT | DATADEAL_FIRST_FLAG_TEMP); //数据处理首次标志置1
	CommInit();
	MOSCtrlEnable();			             				//充放mos管控制初始化
	CommClearTimeout(); 
	EventRecordInit();                       	//事件记录初始化
    
	SH367309Init();	                         	//AFE初始化
//    PCF85063Init();                       //RTC初始化
    
    #ifdef    BMS_USE_MAX172XX
    Max17205ResumeInit();                    //电量计初始化
    #endif

	#ifdef CANBUS_MODE_JT808_ENABLE
	{
		extern void ResetJT808(void);
		ResetJT808();			//JT808 初始化
	}
	#endif	
    
	BSPIDogStart();                      //看门狗启动
	BSPTaskStart(TASK_ID_TIMER10MS, 4);
	BSPTaskStart(TASK_ID_TIMER100MS, 5);
	BSPTaskStart(TASK_ID_TIMER250MS, 6);
	
	//MAX17205电量计任务配置
    #ifdef    BMS_USE_MAX172XX
	BSPTaskStart(TASK_ID_MAX17205_TASK, 7);
    #endif

    #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
    DA213Enable();          //DA213加速度任务开启
    #endif
	//BQ76940 AFE任务配置
	BSPTaskStart(TASK_ID_SH367309_TASK, 10);
    
    //NFC通信任务配置
//	BSPTaskStart(TASK_ID_COMM_TASK, 9);
//	BSPTaskStart(TASK_ID_NFC_TASK, 10);
    
    //CAN通信任务配置
	BSPTaskStart(TASK_ID_CAN_COMM_TASK, 8);
    
	//电压/温度采样配置
	BSPTaskStart(TASK_ID_SAMPLE_TASK, 60);

	//存储任务配置
	BSPTaskStart(TASK_ID_STORAGE_TASK, 13);

	//RTC任务配置
	BSPTaskStart(TASK_ID_PCF85063_TASK, 30);

	#ifdef CANBUS_MODE_JT808_ENABLE
	BSPTaskStart(TASK_ID_JT808_TASK, 30);
	#endif
    
    if((0x04 == gPCBTest.presentstep) || (0x05 == gPCBTest.presentstep))
    {
        BSPTaskStart(TASK_ID_PCBTEST_TASK, 2);
    }
}

//=============================================================================================
//函数名称	: void ShutDownModeTask(void)
//函数参数	: void
//输出参数	: void
//静态变量	:
//功    能	: 关机模式任务函数
//注    意	:
//=============================================================================================
void ShutDownModeTask(void)
{
	if(0 == gSHDNCond.shdnen)
	{
		return;
	}
	
    PT_BEGIN(&gPTSHDN);

	gRecType |= REC_TYPE_SHDN;	//关机事件

	gSHDNState.timer_cnt[0]= gTimer1ms;
    PT_WAIT_UNTIL(&gPTSHDN,((gTimer1ms - gSHDNState.timer_cnt[0]) > 5000)||(((gRecType & REC_TYPE_SHDN) == 0) && (gStorageCTRL==0)));//等待存储完事件
    EventRecordDisable();				               //关闭事件记录
    BSPFeedWDogTask();	                               //喂狗任务    
    
	CommClearCtrl();
    CommCtrlDisable();                                 //清除通信控制
	BSPTaskStop(TASK_ID_TIMER10MS);				       //停止定时器任务，关闭数据处理
	BSPTaskStop(TASK_ID_TIMER100MS);
	BSPTaskStop(TASK_ID_TIMER250MS);
    HMACClearCertifiSta();      //清除认证
    BSPFeedWDogTask();	                               //喂狗任务    

    PT_WAIT_UNTIL(&gPTSHDN,gSampleData.SampFlag.SampFlagBit.IsNTCSampIng == 0);     //等待ntc采样事件结束，关闭TVS和PCB温度采样管
	BSPTaskStop(TASK_ID_SAMPLE_TASK);                   //关闭采样任务
    
	#if defined(USE_B20_TOPBAND_PCBA) || defined(USE_B22_IM_PCBA)
    PCB_NTC_SAMP_OFF;
    TVS_NTC_SAMP_OFF;
	#elif defined(USE_B21_IM_PCBA)
	PCB_TVS_NTC_SAMP_OFF;
	#endif
    PRE_DSG_NTC_SAMP_OFF;
//	BSPGPIOClrPin(TVS_NTC_CTR_PORT,TVS_NTC_CTR_PIN);	//关闭TVS NTC管
//	BSPGPIOClrPin(PCB_NTC_CTR_PORT,PCB_NTC_CTR_PIN);	//关闭PCB NTC管
    
    BSPFeedWDogTask();	                                //喂狗任务
    
    #ifdef    BMS_USE_MAX172XX
    if(DT_F == DTTimerGetState(DT_ID_VL_LV2) || (1 == gSHDNCond.shdnfuelflag) || (gSHDNCond.cond & SHDN_COND_RESET))
    {
        gSHDNCond.shdnfuelflag = 0;
        gSHDNState.timer_cnt[0]= gTimer1ms;     //无法关闭则延迟5S后关闭
        gMax17205Var.RunStat = eFuel_Sta_Disable;
        PT_WAIT_UNTIL(&gPTSHDN,((gTimer1ms - gSHDNState.timer_cnt[0]) > 5000)||(gMax17205Var.RunStat == eFuel_Sta_Idle));	//等待电量计流程结束
    }

    BSPIICChannalDel(gMax17205Var.socchn);    
	BSPTaskStop(TASK_ID_MAX17205_TASK);
    #endif
    
#ifdef BMS_USE_SOX_MODEL    
	#ifdef ENABLE_CALC_STANDBY_CURR
	CurrIntegralStartSleep();
	#endif
    
 	#ifdef SOC_LT_COMPENSATE_ENABLE
	SocLTCompensateEnterShDn();
	#endif   
#endif
    
    #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
    if(((4 == gPCBTest.presentstep) || (5 == gPCBTest.presentstep)) || (gSHDNCond.cond & SHDN_COND_RESET)
        || ((gBatteryInfo.Fault.FaultInfo[2] & OPT_FAULT1_UV2) == OPT_FAULT1_UV2))
    {
        gSHDNState.timer_cnt[0]= gTimer1ms;
        DA213SetCtrl(DA213_SUSPEND_MODE);
        PT_WAIT_UNTIL(&gPTSHDN,(gAccVar.Ctrl == 0) || ((gTimer1ms - gSHDNState.timer_cnt[0]) > 1000));	  //等待ACC进入挂起模式
    }
    DA213Disable();         //DA213加速度任务关闭
    #endif
    
    BSPFeedWDogTask();	                                //喂狗任务

	BSPTaskStop(TASK_ID_PCF85063_TASK);                 //关闭RTC模块
//    BSPIICChannalDel(gRTCCond.rtcchn);      
    
    BSPTaskStop(TASK_ID_CAN_COMM_TASK);
	//BSPTaskStop(TASK_ID_COMM_TASK);
	//FM11NC08Disable();                                  //关闭通信模块
	//PT_WAIT_UNTIL(&gPTSHDN,FM11NC08DisableComplete());	//等待NFC关闭结束
	BSPTaskStop(TASK_ID_NFC_TASK);
    BSPFeedWDogTask();	                                //喂狗任务
    
	gStorageCTRL |= FLASH_POW_OFF_MASK;	                //关闭flash电源
	PT_WAIT_UNTIL(&gPTSHDN,gStorageCTRL == 0);	        //等待flash关闭结束
	BSPTaskStop(TASK_ID_STORAGE_TASK);
    BSPFeedWDogTask();	                                //喂狗任务
	
	SH367309EventFlagSet(SH367309_EVE_CTRL_SLP);          //关闭AFE
	gSHDNState.timer_cnt[0]= gTimer1ms;                 //无法关闭则延迟5S后关闭 
    PT_WAIT_UNTIL(&gPTSHDN,((gTimer1ms - gSHDNState.timer_cnt[0]) > 5000) || (0 == SH367309EventFlagGet()));//等待AFE模块关闭完毕
    BSPTaskStop(TASK_ID_SH367309_TASK);
//    BSPIICChannalDel(gSHAFEData.afechn);      
    BSPFeedWDogTask();	
    
	BSPTaskStop(TASK_ID_IICDEAL_TASK);  

#ifdef CANBUS_MODE_JT808_ENABLE
	BSPTaskStop(TASK_ID_JT808_TASK);  
#endif	

#if defined(BMS_ENABLE_NB_IOT)
    BSPTaskStop(TASK_ID_NB_IOT_TASK);
    BSPTaskStop(TASK_ID_WIFI_TASK); 
    
//    BSPSCIDeInit(GSM_UART_CHANNNEL);
//    BSPSCIPinOutputCfg(GSM_UART_CHANNNEL);
//    BSPSCIDeInit(WIFI_UART_CHANNEL);
//    BSPSCIPinOutputCfg(WIFI_UART_CHANNEL);
	BSPUART_DeInit(UART_WIFI_DEBUG);
    BSPUART_DeInit(UART_GPS);
    
    Sim_ShutDown();
    WifiDealInit();    
    if((gBatteryInfo.Fault.FaultInfo[2] & (OPT_FAULT1_UV1|OPT_FAULT1_UV2)))
        BSPRTCStop();
#endif	
	
//	BSPGPIOClrPin(FUSE_DET_CTR_PORT,FUSE_DET_CTR_PIN);  //关闭保险丝检测
//	BSPGPIOClrPin(PRE_DSG_EN_PORT,PRE_DSG_EN_PIN);	//关闭预放电管
	HAL_GPIO_WritePin(FUSE_DET_EN_PORT, FUSE_DET_EN_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PRE_DSG_EN_PORT, PRE_DSG_EN_PIN, GPIO_PIN_RESET);
	BSPFeedWDogTask();	    
    
	BSPTimerStop();	                                    //关闭定时器
//    BSPRTCStop();                                   //关闭RTC
    
	BSPADCDisable();                                    //关闭ADC
	BSPI2C_DeInit(SH367309_IIC_CHANNEL);
	BSPI2C_DeInit(DA213_IIC_CHANNEL);
//	BSPIICDisable(eIIC0);                               //关闭IIC
//	BSPIICDisable(eIIC1);    
//	BSPSPIDisable(eSPI0);                               //关闭SPI
	//BSPSPIDisable(eSPI1);

	//FlexCANDeInit();
  //FlexCANStanbyMode();
	BSPCan_DeInit();
    PWR_14V_5V_DISABLE();
	if(gSHDNCond.cond & SHDN_COND_RESET)//延时1s后重启
	{
		SoftReset();
	}
    
	ShutDownModeClearData();
	DTCheckOCLockStatusClr(0x03);       //清除oc lock
	IOCheckWakeupCondReset();
	//BSPRTCClrTimeUp();				//MCU RTC 完全不用处理	供电和获取时间
    BSPTaskClrSysTimer();
    //BSPGPIOShDnDeInit();               //关机模式下的低功耗引脚配置
	BSPGPIODeInit();
    //BSPCANStanby();
    
    SysStateSetWorkSta(eWork_ShutDown);
    
	while(1)
	{
//        {   //该段为休眠 喂狗 代码
//            MX_LPTIM1_Init();                   //使能低功耗定时器 3S
//            
//            SleepSet:
//            HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI); //进入停止模式
//            if( Get_TIM_FeedDogFlag()           //定时器唤醒
//                &&(IOCheckWakeupCondGet() == 0) //没有外部唤醒
//                #if defined(BMS_ENABLE_NB_IOT)
//                && (BSPRTCGetTimeUp() == 0)     //没有RTC闹钟
//                #endif           
//                )
//            {
//                BSPFeedWDogTask();	            //喂狗任务
//                BSP_TIM_ClearFeedDogFlag();     //清除喂狗标志
//                goto SleepSet;                  //继续睡眠
//            }
//            MX_LPTIM1_DeInit();                 //跳出睡眠关闭定时器
//        }
        {
            static FLASH_OBProgramInitTypeDef OptionsBytesStruct;
            HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);
            {
                if(OptionsBytesStruct.USERConfig & 0x00020000)  //bit17为0时停止模式下冻结看门狗
                {
                    OptionsBytesStruct.USERConfig &= 0xFFFDFFFF;//      
                    HAL_FLASH_Unlock();             
                    HAL_FLASH_OB_Unlock();                      //解锁FLASH 
                    HAL_FLASHEx_OBProgram(&OptionsBytesStruct); //flash为ff时候才能写入，所以先擦除，网上很多不成功就是没擦除                
                    HAL_FLASH_OB_Lock();                        //上锁FLASH 
                    HAL_FLASH_Lock();
                    HAL_FLASH_OB_Launch();                      //会导致重启一次  永久不会再进此处          
                }
            }  
            HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI); //进入停止模式                      
        }

        //HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
        //NFC,HALL,REMOVE
        if(IOCheckWakeupCondGet()
        #if defined(BMS_ENABLE_NB_IOT)
        || (BSPRTCGetTimeUp())
        #endif            
        )
        {	
//            //在NFC中进行休眠唤醒
//            if(WAKE_COND_NFC_MASK != (IOCheckWakeupCondGet() & WAKE_COND_NFC_MASK))
//            {
                if(eWork_ShutDown == SysStateGetWorkSta())    //休眠唤醒
                {
                    SysStateSetWorkSta(eWork_WakeUp);
                    //BSPWatchDogStop();                                  //关闭看门狗    
                    //BSPGPIOSetPin(SYS_RUN_PWR_PORT,SYS_RUN_PWR_PIN); 							
                    BSPICSInit();		//初始化系统时钟 
                    //BSPGPIOResumeInit();
					BSPGPIOInit();
					PWR_14V_5V_ENABLE();		
                }
//            }
            
//            if(BSPGPIOGetPin(HALL_CHECK_INT_PORT,HALL_CHECK_INT_PIN))   //G2无效
//            {
//                BITCLR(gBatteryInfo.Status.IOStatus,0);    //将G2信号置为无效
//                gMCData.predsg_flag = 0;//复位预放电标志
//            }
            if(IOCheckWakeupCondGet() & WAKE_COND_HALL_MASK)
            {
                gMCData.predsg_flag = 0;//复位预放电标志
            }
            if(IOCheckWakeupCondGet() & WAKE_COND_RESPHOTO_MASK)
            {
                //清除标志
                IOCheckWakeupCondClr(WAKE_COND_RESPHOTO_MASK);
            }
             if(IOCheckWakeupCondGet() & WAKE_COND_EXMODULE_MASK)
            {
                //清除标志
                IOCheckWakeupCondClr(WAKE_COND_EXMODULE_MASK);
            }           
            if(IOCheckWakeupCondGet() & WAKE_COND_CAN_INT_MASK)
            {
                //清除标志
                IOCheckWakeupCondClr(WAKE_COND_CAN_INT_MASK);
            }
			//恢复所有任务
			ShutDownModeResume();
            SysStateSetWorkSta(eWork_Normal);
			
			PT_INIT(&gPTSHDN);
			
            gSHDNCond.shdnen = 0;	//使能关闭
			return;
		}

		PT_NEXT(&gPTSHDN);
	}

	PT_END(&gPTSHDN);

}

/*****************************************end of ShutDownMode.c*****************************************/
