//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: TestMode.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 测试模式源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
//=============================================================================================
//包含头文件
//=============================================================================================
#include "TestMode.h"
#include "DataDeal.h"
#include "VoltBalance.h"
#include "SH367309.h"
#include "Storage.h"
#include "BSPTimer.h"
#include "IOCheck.h"
#include "Sample.h"
#include "MAX17205.h"
#include "CommCtrl.h"
//#include "PCF85063.h"
#include "RTC.h"	//用于替代PCF85063.h
#include "DTCheck.h"
#include "ParaCfg.h"
#include "Comm.h"
#include "MBFindMap.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "ShutDownMode.h"
#include "DA213.h"
#include "NTC.h"
#include "pt.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
t_PCBATEST gPCBTest;            //PCB测试结构体
pt gPTPCBTest;					//PCB测试任务pt线程控制变量
volatile u8 gACCIntTestFlag = 0;    //ACC测试标志

//=============================================================================================
//定义接口函数
//=============================================================================================

//=============================================================================================
//函数名称	: void TestModePrepareTest(void)
//函数参数	: *d：PCB测试结构体指针
//输出参数	: void
//静态变量	: void
//功    能	: 测试前准备函数
//注    意	: 
//=============================================================================================
void TestModePrepareTest(t_PCBATEST *d)
{
    uint16_t dat;
    PT_BEGIN(d);
    d->result = 0;
    d->nextstep = 0;
    dat = 0;

    mb_ctrl_ctrl_w(512,&dat);	//关闭mosfet
    dat = 0;
    mb_ctrl_rec_w(535,&dat);	//关闭定时记录
    dat = 0;
    mb_ctrl_balctl_w(536,&dat);	//关闭均衡控制

//    BQ76940ProtDelayCtrl(FALSE); //禁止保护延时
//    afe_protdelay(false);
    
    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2) && SH367309EventFlagGet() == 0);   //等待电量计配置完毕
//    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && g_AFE_VAR.ctrl == 0);

    d->result = 0x8000;

    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && d->nextstep!=0);
    d->lc = 0;
    d->presentstep = d->nextstep;
    PT_END(d);
}

//=============================================================================================
//函数名称	: void TestModeHalloffTest(void)
//函数参数	: *d：PCB测试结构体指针
//输出参数	: void
//静态变量	: void
//功    能	: 霍尔关闭、光敏关闭测试函数
//注    意	: 
//=============================================================================================
void TestModeHalloffTest(t_PCBATEST *d)
{
    PT_BEGIN(d);
    d->result = 0;
    d->nextstep = 0;

    gStorageCTRL |= FLASH_POW_TEST_MASK;
    d->timebuf = gTimer1ms;
    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && (gTimer1ms - d->timebuf) > 1000);     //延时1S

    if((gBatteryInfo.Status.IOStatus & HALL_CHECK_SHIFT))
    {
        d->result |= 0x0001;
    }

    if((gBatteryInfo.Status.IOStatus & REMOVE_CHECK_SHIFT))
    {
        d->result |= 0x0002;
    }
    
    d->result |= 0x8000;

    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && d->nextstep!=0);

    gStorageCTRL &= ~FLASH_POW_TEST_MASK;
    d->lc = 0;
    d->presentstep = d->nextstep;
    PT_END(d);
}

//=============================================================================================
//函数名称	: void TestModeSelfTest(void)
//函数参数	: *d：PCB测试结构体指针
//输出参数	: void
//静态变量	: void
//功    能	: 自检测试函数
//注    意	: 
//=============================================================================================
void TestModeSelfTest(t_PCBATEST *d)
{
    uint8_t i,j;
    PT_BEGIN(d);
    d->result = 0;
    d->nextstep = 0;

    for(i = 0;i < BAT_CELL_NUM;i++)
    {
        d->maxvolt[i] = 0;
        d->minvolt[i] = 5000;
    }

    for(i=0;i<6;i++)
    {
        d->maxtemp[i] = -400;
        d->mintemp[i] = 1200;
    }

    d->tmaxvolt = 0;
    d->tminvolt = 5000;

    d->rtcsec = gRdTimeAndDate.second;

//#ifndef DISABLE_ACCELEROMETER
//    ACC_INT_TEST_FLAG = 0;
//    acc_ctrl_set(&g_ACC_DATA,ACC_TEST_INT);
//#endif
    #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
    gACCIntTestFlag = 0;
    DA213SetCtrl(DA213_TEST_INT);
    #endif    

    d->timebuf = gTimer1ms;
    while((gTimer1ms - d->timebuf)<5000)
    {
        if((gBatteryInfo.Fault.FaultInfo[0] & (~(DEV_FAULT1_RTCVAL | DEV_FAULT1_CONFIG)))  //检测故障标志，除了RTC位标定和未配置参数故障
        || (gBatteryInfo.Fault.FaultInfo[1])
        || (gBatteryInfo.Fault.FaultInfo[2])
        || (gBatteryInfo.Fault.FaultInfo[3]&(~OPT_FAULT2_ILLEGAL_CHG))) //允许存在非法充电
        {
            d->result |= 0x1;           //存在故障
        }
        
        //if(!StorageStringComp((uint8_t*)&gConfig.flag, (uint8_t*)&cConfigInit.flag, sizeof(t_CONFIG_PARA)-2))//校验参数配置
        if((gConfig.flag != cConfigInit.flag)
            || (!StorageStringComp((uint8_t*)&gConfig.cellov, (uint8_t*)&cConfigInit.cellov, T_DT_PARA_LEN * sizeof(t_DT_PARA)))//校验参数配置
            || (!StorageStringComp((uint8_t*)&gConfig.afeflt, (uint8_t*)&cConfigInit.afeflt, T_DT_BOOT_PARA_LEN * sizeof(t_DT_BOOL_PARA)))
            || (!StorageStringComp((uint8_t*)&gConfig.bala, (uint8_t*)&cConfigInit.bala,sizeof(t_BALANCE_PARA))))      
        {
            d->result |= 0x1 << 1;
        }
        
        if((gSampleData.MCU3v3 < 3200) || (gSampleData.MCU3v3 > 3400))//判断mcu3v3
        {
            d->result |= 0x1 << 2;
        }

        if((gSampleData.PreVolt > 20))   //判断预放检测零点
        {
            d->result |= 0x1 << 3;
        }

        for(i = 0;i < BAT_CELL_NUM;i++)   //判断电压,是否在正常范围内
        {
            //if(!(gConfig.basic.batvalid & (0x1<<i)))
        	if(!(SH367309_VALID_BAT & (0x1<<i)))
            {
                continue;
            }
            if((gBatteryInfo.Data.CellVolt[i] < NORMAL_TEST_LOW_VOLT) || ((gBatteryInfo.Data.CellVolt[i] > NORMAL_TEST_HIGH_VOLT)))
            {
                d->result |= 0x1 << 4;
            }
            else
            {
                //求出异常值的最大值
                if(gBatteryInfo.Data.CellVolt[i] > d->maxvolt[i])
                {
                    d->maxvolt[i] = gBatteryInfo.Data.CellVolt[i];
                }
                if(gBatteryInfo.Data.CellVolt[i] < d->minvolt[i])
                {
                    d->minvolt[i] = gBatteryInfo.Data.CellVolt[i];
                }
            }
            if(d->maxvolt[i] > d->tmaxvolt)
            {
                d->tmaxvolt = d->maxvolt[i];
            }
            if(d->minvolt[i] < d->tminvolt)
            {
                d->tminvolt = d->minvolt[i];
            }
        }
      
        for(i = 0;i < 2;i++)    //判断温度
        {
            if((gBatteryInfo.Data.CellTemp[i] < 0)||((gBatteryInfo.Data.CellTemp[i] > 400)))
            {
                d->result |= 0x1 << 5;
            }
            else
            {
                if(gBatteryInfo.Data.CellTemp[i]> d->maxtemp[i])
                {
                    d->maxtemp[i] = gBatteryInfo.Data.CellTemp[i];
                }
                if(gBatteryInfo.Data.CellTemp[i] < d->mintemp[i])
                {
                    d->mintemp[i] = gBatteryInfo.Data.CellTemp[i];
                }
            }
        }
        /*
        if((gBatteryInfo.Addit.FuelTemp < 0)||((gBatteryInfo.Addit.FuelTemp > 400)))
        {
            d->result |= 0x1 << 5;
        }
        else
        {
            if(gBatteryInfo.Addit.FuelTemp> d->maxtemp[2])
            {
                d->maxtemp[2] = gBatteryInfo.Addit.FuelTemp;
            }
            if(gBatteryInfo.Addit.FuelTemp < d->mintemp[2])
            {
                d->mintemp[2] = gBatteryInfo.Addit.FuelTemp;
            }
        }
        */
        if((gSampleData.PreTemp < 0)||((gSampleData.PreTemp > 400)))
        {
            d->result |= 0x1 << 5;
        }
        else
        {
            if(gSampleData.PreTemp> d->maxtemp[2])
            {
                d->maxtemp[2] = gSampleData.PreTemp;
            }
            if(gSampleData.PreTemp < d->mintemp[2])
            {
                d->mintemp[2] = gSampleData.PreTemp;
            }
        }        
        if((gSampleData.ConnTemp < 0)||((gSampleData.ConnTemp > 400)))
        {
            d->result |= 0x1 << 5;
        }
        else
        {
            if(gSampleData.ConnTemp> d->maxtemp[3])
            {
                d->maxtemp[3] = gSampleData.ConnTemp;
            }
            if(gSampleData.ConnTemp < d->mintemp[3])
            {
                d->mintemp[3] = gSampleData.ConnTemp;
            }
        }
        
        if((gSampleData.PCBTemp < 0)||((gSampleData.PCBTemp > 400)))
        {
            d->result |= 0x1<<5;
        }
        else
        {
            if(gSampleData.PCBTemp> d->maxtemp[4])
            {
                d->maxtemp[4] = gSampleData.PCBTemp;
            }
            if(gSampleData.PCBTemp < d->mintemp[4])
            {
                d->mintemp[4] = gSampleData.PCBTemp;
            }
        }

        if((gSampleData.TVSTemp < 0)||((gSampleData.TVSTemp > 400)))
        {
            d->result |= 0x1<<5;
        }
        else
        {
            if(gSampleData.TVSTemp > d->maxtemp[5])
            {
                d->maxtemp[5] = gSampleData.TVSTemp;
            }
            if(gSampleData.TVSTemp < d->mintemp[5])
            {
                d->mintemp[5] = gSampleData.TVSTemp;
            }
        }

        if(gBatteryInfo.Data.Curr > 1)    //afe电流零点
        {
            d->result |= 0x1 << 6;
        }
        if(gBatteryInfo.Addit.FuelAvgCurr > 10)    //电量计电流零点
        {
            d->result |= 0x1 << 7;
        }
        if(gBatteryInfo.Data.FiltSOC < 100)//判断soc
        {
            d->result |= 0x1<<8;
        }
//#ifndef DISABLE_ACCELEROMETER
//        if((g_ACC_DATA.x<-1500)||(g_ACC_DATA.x>1500)//判断加速度计
//            ||(g_ACC_DATA.y<-1500)||(g_ACC_DATA.y>1500)
//            ||(g_ACC_DATA.z<-1500)||(g_ACC_DATA.z>1500)
//        )
//        {
//            d->result |= 0x1<<9;
//        }
//#endif
        
        #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
        if((gAccVar.AccData.ax < -1500) || (gAccVar.AccData.ax > 1500)//判断加速度计
            ||(gAccVar.AccData.ay < -1500) || (gAccVar.AccData.ay > 1500)
            ||(gAccVar.AccData.az < -1500) || (gAccVar.AccData.az > 1500)
        )
        {
            d->result |= 0x1<<9;
        }
        #endif           
        
        /*
        if(((gBatteryInfo.Addit.FuelCellVolt - gBatteryInfo.Data.CellVolt[0]) > 50) 
          ||((gBatteryInfo.Addit.FuelCellVolt - gBatteryInfo.Data.CellVolt[0]) < -50))//判断电量计电压
        {
            d->result |= 0x1<<13;
        }
        */
        
        BSPTaskStart(TASK_ID_PCBTEST_TASK, 2);
        PT_NEXT(d);

    }

    i= PCF85063_BCD_to_HEC(d->rtcsec);              //判断RTC
    j= PCF85063_BCD_to_HEC(gRdTimeAndDate.second);
    if(i <= j)
    {
        i= j-i;
        if((i < 4) || (i > 6))
        {
            d->result |= 0x1 << 10;
        }
    }
    else
    {
        j= j + 60;
        if(i<=j)
        {
            i= j-i;
            if((i < 4)||(i > 6))
            {
                d->result |= 0x1<<10;
            }
        }
        else
        {
            d->result |= 0x1<<10;
        }
    }

    if(!(gBatteryInfo.Status.IOStatus & REMOVE_CHECK_SHIFT))//判断光敏有效
    {
        d->result |= 0x1<<12;
    }
    
    #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
    if(gACCIntTestFlag == 0)//判断加速度中断信号
    {
        d->result |= 0x1<<11;
    }
    #endif        

    
    for(i = 0;i < BAT_CELL_NUM;i++)                   //判断电压波动
    {
        //电压跳动更改为8mV
        if((d->maxvolt[i]-d->minvolt[i]) > 8)
        {
             d->result |= 0x1<<4;
        }
    }
    if((d->tmaxvolt - d->tminvolt)> 100)
    {
        d->result |= 0x1<<4;
    }
  
    for(i = 0;i < 6;i++)                //判断温度波动
    {
        if((d->maxtemp[i]-d->mintemp[i])>10)
        {
             d->result |= 0x1<<5;
        }
    }

    d->result |= 0x8000;


    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && d->nextstep!=0);
    d->lc = 0;
    d->presentstep = d->nextstep;
    PT_END(d);
}

//=============================================================================================
//函数名称	: void TestModeSleepTest1(void)
//函数参数	: *d：PCB测试结构体指针
//输出参数	: void
//静态变量	: void
//功    能	: 睡眠测试1函数
//注    意	: 
//=============================================================================================
void TestModeSleepTest1(t_PCBATEST *d)
{
    uint16_t dat;
    PT_BEGIN(d);
    d->result = 0;
    d->nextstep = 0;

    d->timebuf = gTimer1ms;
    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && (gTimer1ms - d->timebuf) > 1000);   //延时1S

    dat = CTRL1_SDP_MASK;
    mb_ctrl_ctrl_w(512,&dat);//休眠

    d->result |= 0x8000;

    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && d->nextstep!=0);
    d->lc = 0;
    d->presentstep = d->nextstep;
    PT_END(d);
}

//=============================================================================================
//函数名称	: void TestModeSleepTest2(void)
//函数参数	: *d：PCB测试结构体指针
//输出参数	: void
//静态变量	: void
//功    能	: 睡眠测试2函数
//注    意	: 
//=============================================================================================
void TestModeSleepTest2(t_PCBATEST *d)
{
    uint16_t dat;
    PT_BEGIN(d);
    d->result = 0;
    d->nextstep = 0;

    d->timebuf = gTimer1ms;
    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && (gTimer1ms - d->timebuf)>1000);

    dat = CTRL1_SDP_MASK;
    mb_ctrl_ctrl_w(512,&dat);//休眠
    gSHDNCond.shdnfuelflag = 1;
    //bq27541_pow(false);

    d->result |= 0x8000;

    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && d->nextstep!=0);
    d->lc = 0;
    d->presentstep = d->nextstep;
    PT_END(d);
}

//=============================================================================================
//函数名称	: void TestModeEvenBalTest(void)
//函数参数	: *d：PCB测试结构体指针
//输出参数	: void
//静态变量	: void
//功    能	: 偶数节均衡测试函数
//注    意	: 
//=============================================================================================
void TestModeEvenBalTest(t_PCBATEST *d)
{
    #if defined(LFP_TB_20000MAH_20S) || defined(SY_PB_32000MAH_17S)
    uint16_t dat;
    PT_BEGIN(d);
    d->result = 0;
    d->nextstep = 0;

    d->timebuf = gTimer1ms;
    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && (gTimer1ms - d->timebuf)>1000);

    dat = CTRL1_ENBLAT_MASK;
    mb_ctrl_ctrl_w(512,&dat);   //开启均衡测试

    dat = VoltBalanceGetBatValidMask(0);
    mb_ctrl_balctl_w(0,&dat);
    dat = VoltBalanceGetBatValidMask(0) >> 16;
    mb_ctrl_balctl_17_20_w(0,&dat);

    d->result |= 0x8000;

    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && d->nextstep!=0);
    dat = 0;
    mb_ctrl_ctrl_w(512,&dat);   //关闭均衡测试
    dat = 0;
    mb_ctrl_balctl_w(NULL,&dat);
    d->lc = 0;
    d->presentstep = d->nextstep;
    PT_END(d);   
    #else
    uint16_t dat;
    PT_BEGIN(d);
    d->result = 0;
    d->nextstep = 0;

    d->timebuf = gTimer1ms;
    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && (gTimer1ms - d->timebuf)>1000);

    dat = CTRL1_ENBLAT_MASK;
    mb_ctrl_ctrl_w(512,&dat);   //开启均衡测试

    dat = VoltBalanceGetBatValidMask(0);
    mb_ctrl_balctl_w(0,&dat);

    d->result |= 0x8000;

    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && d->nextstep!=0);
    dat = 0;
    mb_ctrl_ctrl_w(512,&dat);   //关闭均衡测试
    dat = 0;
    mb_ctrl_balctl_w(NULL,&dat);
    d->lc = 0;
    d->presentstep = d->nextstep;
    PT_END(d);
    #endif
}

//=============================================================================================
//函数名称	: void TestModeOddBalTest(void)
//函数参数	: *d：PCB测试结构体指针
//输出参数	: void
//静态变量	: void
//功    能	: 奇数节均衡测试函数
//注    意	: 
//=============================================================================================
void TestModeOddBalTest(t_PCBATEST *d)
{
    #if defined(LFP_TB_20000MAH_20S) || defined(SY_PB_32000MAH_17S)
    uint16_t dat;
    PT_BEGIN(d);
    d->result = 0;
    d->nextstep = 0;

    d->timebuf = gTimer1ms;
    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && (gTimer1ms - d->timebuf)>1000);

    dat = CTRL1_ENBLAT_MASK;
    mb_ctrl_ctrl_w(512,&dat);       //开启均衡测试
    dat = VoltBalanceGetBatValidMask(1);
    mb_ctrl_balctl_w(NULL,&dat);
    dat = VoltBalanceGetBatValidMask(1) >> 16;
    mb_ctrl_balctl_17_20_w(0,&dat);

    d->result |= 0x8000;

    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && d->nextstep!=0);
    dat = 0;
    mb_ctrl_ctrl_w(512,&dat);       //关闭均衡测试
    dat = 0;
    mb_ctrl_balctl_w(NULL,&dat);
    
    d->lc = 0;
    d->presentstep = d->nextstep;
    PT_END(d);    
    #else
    uint16_t dat;
    PT_BEGIN(d);
    d->result = 0;
    d->nextstep = 0;

    d->timebuf = gTimer1ms;
    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && (gTimer1ms - d->timebuf)>1000);

    dat = CTRL1_ENBLAT_MASK;
    mb_ctrl_ctrl_w(512,&dat);       //开启均衡测试
    dat = VoltBalanceGetBatValidMask(1);
    mb_ctrl_balctl_w(NULL,&dat);

    d->result |= 0x8000;

    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && d->nextstep!=0);
    dat = 0;
    mb_ctrl_ctrl_w(512,&dat);       //关闭均衡测试
    dat = 0;
    mb_ctrl_balctl_w(NULL,&dat);
    
    d->lc = 0;
    d->presentstep = d->nextstep;
    PT_END(d);
    #endif
}

//=============================================================================================
//函数名称	: void TestModeAFEProtTest(void)
//函数参数	: *d：PCB测试结构体指针
//输出参数	: void
//静态变量	: void
//功    能	: AFE保护测试函数
//注    意	: 
//=============================================================================================
void TestModeAFEProtTest(t_PCBATEST *d)
{
    uint16_t dat;
    PT_BEGIN(d);
    d->result = 0;
    d->nextstep = 0;

    d->timebuf = gTimer1ms;
    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && (gTimer1ms - d->timebuf)>1000);

    dat = CTRL1_CHG_MASK|CTRL1_DSG_MASK|CTRL1_CPI_MASK;
    mb_ctrl_ctrl_w(512,&dat);

    d->result |= 0x8000;

    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && d->nextstep!=0);
    dat = 0;
    mb_ctrl_ctrl_w(512,&dat);//
    d->lc = 0;
    d->presentstep = d->nextstep;
    PT_END(d);
}


//=============================================================================================
//函数名称	: void TestModeTemperatureTest(void)
//函数参数	: *d：PCB测试结构体指针
//输出参数	: void
//静态变量	: void
//功    能	: 温度采样测试函数
//注    意	: 
//=============================================================================================
void TestModeTemperatureTest(t_PCBATEST *d)
{
	u8 i = 0;    
    
    PT_BEGIN(d);
    d->result = 0;
    d->nextstep = 0;

    d->timebuf = gTimer1ms;

    d->maxtemp[0] = gBatteryInfo.TempChara.MaxTemp;
    d->mintemp[0] = gBatteryInfo.TempChara.MinTemp;
    
    while((gTimer1ms - d->timebuf) < 5000)
    {
        if(gBatteryInfo.TempChara.MaxTemp > d->maxtemp[0])
        {
            d->maxtemp[0] = gBatteryInfo.TempChara.MaxTemp;
        }
        /*
        if(gBatteryInfo.Addit.FuelTemp > d->maxtemp[0])
        {
            d->maxtemp[0] = gBatteryInfo.Addit.FuelTemp;
        }
        */
        if(gSampleData.PreTemp > d->maxtemp[0])
        {
            d->maxtemp[0] = gSampleData.PreTemp;
        }        
        if(gSampleData.ConnTemp > d->maxtemp[0])
        {
            d->maxtemp[0] = gSampleData.ConnTemp;
        }
        if(gSampleData.PCBTemp > d->maxtemp[0])
        {
            d->maxtemp[0] = gSampleData.PCBTemp;
        }

        if(gSampleData.TVSTemp > d->maxtemp[0])
        {
            d->maxtemp[0] = gSampleData.TVSTemp;
        }

        if(gBatteryInfo.TempChara.MinTemp < d->mintemp[0])
        {
            d->mintemp[0] = gBatteryInfo.TempChara.MinTemp;
        }
        /*
        if(gBatteryInfo.Addit.FuelTemp < d->mintemp[0])
        {
            d->mintemp[0] = gBatteryInfo.Addit.FuelTemp;
        }
        */
        if(gSampleData.PreTemp < d->mintemp[0])
        {
            d->mintemp[0] = gSampleData.PreTemp;
        }       
        if(gSampleData.ConnTemp < d->mintemp[0])
        {
            d->mintemp[0] = gSampleData.ConnTemp;
        }
        if(gSampleData.PCBTemp < d->mintemp[0])
        {
            d->mintemp[0] = gSampleData.PCBTemp;
        }

        if(gSampleData.TVSTemp < d->mintemp[0])
        {
            d->mintemp[0] = gSampleData.TVSTemp;
        }

        if((d->maxtemp[0] - d->mintemp[0]) > 50)//差值大于5度
        {
            d->result |= 0x0001;
            goto OUT;
        }
        
		/* 出现电池温度NTC采样断线或短路，也报故障 Handry 20-07-31*/
		if((gBatteryInfo.Addit.FuelTemp == NTC_TEMP_OFF)
            ||(gBatteryInfo.Addit.FuelTemp == NTC_TEMP_SHORT))
        {
            d->result |= 0x0001;
            goto OUT;
        }
        
		for(i = 0;i < BAT_TEMP_NUM;i++)
		{
			if((gBatteryInfo.Data.CellTemp[i] == NTC_TEMP_OFF)
                ||(gBatteryInfo.Data.CellTemp[i] == NTC_TEMP_SHORT))
	        {
	            d->result |= 0x0001;
	            goto OUT;
	        }				
		}        

        BSPTaskStart(TASK_ID_PCBTEST_TASK, 2);
        PT_NEXT(d);
    }
OUT:
    d->result |= 0x8000;
    PT_WAIT_UNTIL(d,BSPTaskStart(TASK_ID_PCBTEST_TASK, 2)  && d->nextstep!=0);
    d->lc = 0;
    d->presentstep = d->nextstep;
    PT_END(d);
}

//测试函数集合
t_TESTFUNC cTestFuncTab[9] =
{
    TestModePrepareTest,
    TestModeHalloffTest,
    TestModeSelfTest,
    TestModeSleepTest1,
    TestModeSleepTest2,
    TestModeEvenBalTest,
    TestModeOddBalTest,
    TestModeAFEProtTest,
    TestModeTemperatureTest,
};

//=============================================================================================
//函数名称	: void TestModeInit(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 测试模式初始化函数
//注    意	:
//=============================================================================================
void TestModeInit(void)
{
	gPCBTest.en = 0;
}

//=============================================================================================
//函数名称	: void TestModePCBTestTask(void *p)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 测试模式主函数
//注    意	:
//=============================================================================================
void TestModePCBTestTask(void *p)
{
	(void)p;
	if(0 == gPCBTest.en)
	{
		return;
	}

    PT_BEGIN(&gPTPCBTest);
    gPCBTest.nextstep = 0;
    gPCBTest.presentstep = 1;
    gPCBTest.result =0;
    gPCBTest.sta = 1;
    gPCBTest.lc = 0;

    for(;;)
    {
        if((gPCBTest.presentstep > 0)&&(gPCBTest.presentstep <= 9))
        {
            cTestFuncTab[gPCBTest.presentstep - 1](&gPCBTest);
        }
        else
        {
            //BQ76940ProtDelayCtrl(TRUE); //开启保护延时
        	//afe_protdelay(true);
            
             gPCBTest.en = 0;	//关闭任务
            //BSPTaskStop(TASK_ID_PCBTEST_TASK);
            //task_delete(d);
            
            gPCBTest.sta = 0;
            gPCBTest.result =0;
            gPCBTest.presentstep =0;
            gPCBTest.nextstep = 0;
            PT_INIT(&gPTPCBTest);
            return;
        }
        
        //兼容老化，测试模式无命令不退出
        //if((g_TIME1MS - d->testtimeout)>600000)//10min超时
        //{
        //    d->nextstep = 0xff;//超时退出测试程序
        //}

        CommClearTimeout();

        PT_NEXT(&gPTPCBTest);
    }
    PT_END(&gPTPCBTest);

}

/*****************************************end of TestMode.c*****************************************/
