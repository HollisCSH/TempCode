//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SleepMode.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 睡眠模式代码
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
//=============================================================================================
//包含头文件
//=============================================================================================
#include "SleepMode.h"
#include "BSPSystemCfg.h"
#include "BSPWatchDog.h"
#include "BSPRTC.h"
#include "BSPGPIO.h"
#include "BSPTimer.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "BSPLowPow.h"
#include "BSPCriticalZoom.h"
#include "BSPHardWareIntf.h"
#include "BSPPIT.h"
#include "BSPSystemCfg.h"
#include "BSPICS.h"
#include "Version.h"
#include "EventRecord.h"
#include "BatteryInfo.h"
#include "SH367309.h"
#include "MAX17205.h"
//#include "FM11NC08.h"
#include "MOSCtrl.h"
#include "Comm.h"
#include "IOCheck.h"
#include "DTCheck.h"
#include "DTTimer.h"
#include "BSPADC.h"
#include "Sample.h"
#include "Storage.h"
#include "MX25L16.h"
#include "CommCtrl.h"
#include "ParaCfg.h"
#include "SysState.h"
#include "DataDeal.h"
#include "HMAC.h"
#include "pt.h"

//=============================================================================================
//全局变量
//=============================================================================================
static pt gPTSleep;					     //睡眠任务pt线程控制变量
t_SLEEP_COND gSleepCond;                 //睡眠相关检测
t_SLEEP_CHECK gSleepCheckData = {0,0,0}; //睡眠检测结构体

//=============================================================================================
//定义接口函数
//=============================================================================================

//=============================================================================================
//函数名称	: void SleepModeInit(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 睡眠模式初始化函数
//注    意	:
//=============================================================================================
void SleepModeInit(void)
{
	PT_INIT(&gPTSleep);
    
    gSleepCond.sleepen = 0;
}

//=============================================================================================
//函数名称	: void SleepModeClearData(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 清除睡眠模式数据
//注    意	:
//=============================================================================================
void SleepModeClearData(void)
{
    gSleepCheckData.iicerr = 0;
    gSleepCheckData.occnt = 0;
    gSleepCheckData.v16cnt = 0;
    gSleepCheckData.res = 0;
}

//=============================================================================================
//函数名称	: void SleepModeCheck(void)
//函数参数	: void
//输出参数	: void
//静态变量	:
//功    能	: 睡眠模式判断，确认是否进入睡眠模式
//注    意	:
//=============================================================================================
void SleepModeCheck(void)
{
    //睡眠命令+睡眠过流标志无效+无运行故障
    if((CommCtrlIsSleepMode()) 
    &&(DT_N == DTTimerGetState(DT_ID_SLEEP_CURRH)
    &&(gBatteryInfo.Status.IOStatus & (HALL_CHECK_SHIFT | EXTERN_MODULE_CHECK_SHIFT))
    &&(!gBatteryInfo.Fault.FaultInfo[0])
    &&(!gBatteryInfo.Fault.FaultInfo[1])
    &&(!(gBatteryInfo.Fault.FaultInfo[2]&(~(OPT_FAULT1_CHG_OT|OPT_FAULT1_CHG_UT))))
    &&(!(gBatteryInfo.Fault.FaultInfo[3]&(~OPT_FAULT2_ILLEGAL_CHG)))))  //不允许除了非法充电的其他故障
    {
        if(CommCompareTimeout(SLEEP_COND_DELAY))//超过5秒没有通信
        {
            gSleepCond.cond |= 1;
        }
    }
	
	if(gSleepCond.cond > 0)
	{
		gSleepCond.sleepen = 1;
	}
}

//=============================================================================================
//函数名称	: void SleepModeProcess(void)
//函数参数	: 睡眠模式检测参数指针
//输出参数	: void
//静态变量	:
//功    能	: 睡眠模式处理
//注    意	:
//=============================================================================================
void SleepModeProcess(void)
{
    s16 curr;
    s16 temp[3];
    u16 volt[16] = {0};
    u8 i = 0;

    BSPRTCClrCnt();    //清零计数器
    
    if(BSPGPIOGetPin(HALL_CHECK_INT_PORT,HALL_CHECK_INT_PIN) !=0 )//G2无效退出,霍尔移除
    {
        gSleepCheckData.res |= SLPPRO_G2_MASK;
        goto EXIT ;
    }

    //读取睡眠电流
    if(SH367309DirectReadData(volt,temp,&curr))
    {
        goto IIC_ERR;
    }   
    
    if((curr < (0-BAT_SLEEP_CURR)) || (curr > (BAT_SLEEP_CURR)))//睡眠保护电流
    {
        goto OC;
    }
    
    //获取电池温度
    if(++gSleepCheckData.tempcnt >= 60)
    {
        gSleepCheckData.tempcnt = 0;
        if((temp[0] > 600)||(temp[0] < -20)||(temp[2] > 600)||(temp[2] < -20))
        {
            gSleepCheckData.res |= SLPPRO_OT_MASK;
            goto EXIT;
        }
    }   
    
    for(i = 0;i < SH367309_VOLT_NUM;i++)
    {
        if(volt[i] > gConfig.cellov.matth[DT_LV4] || volt[i] < gConfig.celluv.matth[DT_LV4])
        {
            if(gSleepCheckData.v16cnt++>=3)
            {
                gSleepCheckData.res |= SLPPRO_B16F_MASK;
            }
        }
    }
    
    gSleepCheckData.iicerr = 0;
    gSleepCheckData.occnt = 0;
    goto EXIT;    
    
IIC_ERR:
        if(gSleepCheckData.iicerr ++>=10)
            gSleepCheckData.res |= SLPPRO_IICERR_MASK;
        goto EXIT;
OC:
        if(gSleepCheckData.occnt++>=3)
            gSleepCheckData.res |= SLPPRO_OC_MASK;

EXIT:   
        return;        
}

//=============================================================================================
//函数名称	: void SleepModeResume(void)
//函数参数	:
//输出参数	: 
//静态变量	:
//功	能	: 睡眠模式恢复
//注	意	:
//=============================================================================================
void SleepModeResume(void)
{
	//BSPICSInit();		//初始化系统时钟  
    //BSPGPIOInit();
    //在NFC中进行休眠唤醒
    if(WAKE_COND_NFC_MASK != (IOCheckWakeupCondGet() & WAKE_COND_NFC_MASK))
    {
        BSPICSInit();		//初始化系统时钟 
        BSPGPIOResumeInit();
        BSPSPIInit(eSPI0,SPI0_MASTER,SPI0_BAUDRATE,SPI0_MODE);	//SPI0初始化
    }    
    
	//开启ADC
    //BSPADCEnable();
    BSPADCInit(CONN_TEMP_ADC_CHAN_MASK | PCB_TVS_TEMP_ADC_CHAN_MASK | PRE_DSG_SHORT_ADC_CHAN_MASK | CELL_VOLT_16_ADC_CHAN_MASK,
               ADC_SAMPMODE); //ADC初始化      
    //开启SPI
	//BSPSPIInit(eSPI0,SPI0_MASTER,SPI0_BAUDRATE,SPI0_MODE);	//SPI0初始化
	BSPSPIInit(eSPI1,SPI1_MASTER,SPI1_BAUDRATE,SPI1_MODE);	//SPI1初始化

	BSPGPIOSetPin(NFC_CS_PORT,NFC_CS_PIN);		            //拉高SPI0 CS，失能SPI芯片
	BSPGPIOSetPin(FLASH_CS_PORT,FLASH_CS_PIN);		        //拉高SPI1 CS，失能SPI芯片  

	//BSPWatchDogStop();                       //关闭看门狗
    
	BSPTimerStart();	                     //定时器开启
    
    gBatteryInfo.Status.DataReady = 0;	
    
    gDataDealFirstFlag = (DATADEAL_FIRST_FLAG_VOLT | DATADEAL_FIRST_FLAG_TEMP); //数据处理首次标志置1
	CommInit();
	MOSCtrlEnable();			             //充放mos管控制初始化
	CommClearTimeout();
	MX25L16Init();	                         //FLASH上电
    EventRecordInit();                       //事件记录初始化
    
    //BSPWatchDogStart();                      //看门狗启动
	BSPTaskStart(TASK_ID_TIMER10MS, 4);
	BSPTaskStart(TASK_ID_TIMER100MS, 5);
	BSPTaskStart(TASK_ID_TIMER250MS, 6);

	//NFC通信任务配置
	BSPTaskStart(TASK_ID_COMM_TASK, 7);
	BSPTaskStart(TASK_ID_NFC_TASK, 8);
	
	//MAX17205电量计任务配置
	BSPTaskStart(TASK_ID_MAX17205_TASK, 9);
	
	//BQ76940 AFE任务配置
	BSPTaskStart(TASK_ID_SH367309_TASK, 10);

	//电压/温度采样开启
	BSPTaskStart(TASK_ID_SAMPLE_TASK, 11);

	//存储任务开启
	BSPTaskStart(TASK_ID_STORAGE_TASK, 13);

	//RTC任务开启
	BSPTaskStart(TASK_ID_PCF85063_TASK, 30);
}

//=============================================================================================
//函数名称	: void SleepModeTask(void)
//函数参数	: void
//输出参数	: void
//静态变量	:
//功    能	: 睡眠模式任务函数
//注    意	:
//=============================================================================================
void SleepModeTask(void)
{
	if(0 == gSleepCond.sleepen)
	{
		return;
	}
	
    PT_BEGIN(&gPTSleep);
    
    EventRecordDisable();				                //关闭事件记录
    
    BSPFeedWDogTask();	                               //喂狗任务
    
	CommClearCtrl();
    CommCtrlDisable();                                 //清除通信控制    
	BSPTaskStop(TASK_ID_TIMER10MS);				       //停止定时器任务，关闭数据处理
	BSPTaskStop(TASK_ID_TIMER100MS);
	BSPTaskStop(TASK_ID_TIMER250MS);
    HMACClearCertifiSta();      //清除认证
    BSPFeedWDogTask();	                               //喂狗任务    
    
    PT_WAIT_UNTIL(&gPTSleep,gSampleData.SampFlag.SampFlagBit.IsNTCSampIng == 0);         //等待ntc采样事件结束，关闭TVS和PCB温度采样管
	BSPTaskStop(TASK_ID_SAMPLE_TASK);                   //关闭采样任务
	BSPGPIOClrPin(TVS_NTC_CTR_PORT,TVS_NTC_CTR_PIN);	//关闭TVS NTC管
	BSPGPIOClrPin(PCB_NTC_CTR_PORT,PCB_NTC_CTR_PIN);	//关闭PCB NTC管

    #ifdef    BMS_USE_MAX172XX
	BSPTaskStop(TASK_ID_MAX17205_TASK);
    #endif

	BSPTaskStop(TASK_ID_PCF85063_TASK);                 //关闭RTC模块
    BSPFeedWDogTask();	                                //喂狗任务
    
    gStorageCTRL |= FLASH_POW_OFF_MASK;	                //关闭flash电源
	PT_WAIT_UNTIL(&gPTSleep,gStorageCTRL == 0);	        //等待flash关闭结束
	BSPTaskStop(TASK_ID_STORAGE_TASK);
    BSPFeedWDogTask();	                                //喂狗任务
    
	BSPTaskStop(TASK_ID_COMM_TASK);
	FM11NC08Disable();                                  //关闭通信模块
	PT_WAIT_UNTIL(&gPTSleep,FM11NC08DisableComplete());	//等待NFC关闭结束
	BSPTaskStop(TASK_ID_NFC_TASK);
     
    BSPTaskStop(TASK_ID_SH367309_TASK);                  //停止AFE任务
    
	BSPGPIOClrPin(FUSE_DET_CTR_PORT,FUSE_DET_CTR_PIN);  //关闭保险丝检测
	BSPGPIOClrPin(PRE_DSG_EN_PORT,PRE_DSG_EN_PIN);	//关闭预放电管

	BSPTimerStop();	                                    //关闭定时器

	BSPADCDisable();                                    //关闭ADC
	
    SleepModeClearData();
	IOCheckWakeupCondReset();
    
    BSPFeedWDogTask();	                                //喂狗任务
  
	//BSPWatchDogStop();                  //关闭看门狗
    BSPGPIOSleepDeInit();               //Sleep模式下的低功耗引脚配置
 
    //霍尔变为上升沿触发
    port_interrupt_config_t hallconfig;
    hallconfig = PORT_DMA_RISING_EDGE;
    PINS_DRV_SetPinIntSel(HALL_CHECK_INT_GPIOBASE,HALL_CHECK_INT_PN,hallconfig);  
    
    BSPRTCClrTimeUp();
    BSPTaskClrSysTimer();  
    BSPRTCSetAlarmAfterSeconds(SLEEP_PERIOD_WAKEUP);    //设置下次唤醒时间，10S     
    
    SysStateSetWorkSta(eWork_Sleep);
        
	while(1)
	{
		BSPEnterPowMode(eVLPSMode);	//进入深度睡眠模式

		if(IOCheckWakeupCondGet())
		{	
            gSleepCond.cond = 0;
			//恢复所有任务
			SleepModeResume();
			
			PT_INIT(&gPTSleep);
			
            SysStateSetWorkSta(eWork_Normal);
            gSleepCond.sleepen = 0;	//使能关闭
			return;
		}
        
        if(BSPRTCGetTimeUp())
        {
            BSPRTCClrTimeUp();
            BSPRTCSetAlarmAfterSeconds(SLEEP_PERIOD_WAKEUP);    //设置下次唤醒时间，10S    
            gSleepCond.cond = 0;
            
            SleepModeProcess();
            
            //恢复所有任务
            if(gSleepCheckData.res)
            {
                SleepModeResume();
                
                PT_INIT(&gPTSleep);
                
                SysStateSetWorkSta(eWork_Normal);
                gSleepCond.sleepen = 0;	//使能关闭
                return;
            }
        }

		PT_NEXT(&gPTSleep);
	}

	PT_END(&gPTSleep);

}

/*****************************************end of SleepMode.c*****************************************/
