//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: DataDeal.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 电池数据处理源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	: Handry
//版本	    : 
//修改日期	: 2020/07/14
//描述	    : 
//1.BAT_ALL_TEMP_NUM替换BAT_TEMP_NUM + 1
//2.增加最近最大充放电电流寄存器，在机柜充电至80%时清零。用于统计最近车辆运行过程中的最大充放电电流。
//3.增加大电流检测寄存器。①计算放电电流超过3C时的持续时间。②计算30秒内（默认，可配置）的最大放电电流。③计算30秒内（默认，可配置）的平均放电电流。
//4.解决电池热失控误报（自耗电很快，欠压休眠，凌晨重启PMS时误报）
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "DataDeal.h"
#include "DTCheck.h"
#include "DTTimer.h"
#include "CommCtrl.h"
#include "Max17205.h"
#include "ParaCfg.h"
#include "NTC.h"
#include "BSPSystemCfg.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
s16 gVoltBuff[BAT_CELL_NUM][DATADEAL_VOLT_BUFF_LEN];    //单体电压下降速度信息缓存，用于刷新数据
s16 gTempBuff[BAT_ALL_TEMP_NUM][DATADEAL_TEMP_BUFF_LEN];//单体温度上升速度信息，加上一节电量计温度，缓存，用于刷新数据
u8 gTempRunAwayCnt[BAT_ALL_TEMP_NUM];                   //热失控、温度失控计数
u8 gDataDealFirstFlag = 0;                              //数据处理首次标志，避免因休眠唤醒导致的电压下降异常,bit0:电压，bit1：温度

//=============================================================================================
//声明静态函数
//=============================================================================================
//=============================================================================================
//函数名称	: static u16 DataDealGetCellVolt(u8 num)
//输入参数	: 电池号
//输出参数	: 电池号对应的电压值
//函数功能	: 获取单体电压值
//注意事项	:
//=============================================================================================
static u16 DataDealGetCellVolt(u8 num);

//=============================================================================================
//函数名称	: static  s16 DataDealGetCellTemp(u8 num)
//输入参数	: 电池号
//输出参数	: 电池号对应的温度值
//函数功能	: 获取单体温度值
//注意事项	:
//=============================================================================================
static s16 DataDealGetCellTemp(u8 num);


//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void DataDealDataInit(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 电池数据初始化函数
//注意事项	:
//=============================================================================================
void DataDealDataInit(void)
{
    u8 i = 0;

    for(i = 0; i < BAT_CELL_NUM; i++)
    {
        gBatteryInfo.Data.CellVolt[i] = DATADEAL_DEFULT_CELLVOLT;      //初始化单节电压数组
    }

    for(i = 0; i < BAT_TEMP_NUM; i++)
    {
        gBatteryInfo.Data.CellTemp[i] = DATADEAL_DEFULT_CELLTEMP;      //初始化单节温度数组
    }

    gBatteryInfo.VoltChara.PerVolt = DATADEAL_DEFULT_CELLVOLT;
    gBatteryInfo.TempChara.PerTemp = DATADEAL_DEFULT_CELLTEMP;

    gBatteryInfo.Status.DataReady = 0;	//数据准备标志清零
    gDataDealFirstFlag = (DATADEAL_FIRST_FLAG_VOLT | DATADEAL_FIRST_FLAG_TEMP); //数据处理首次标志置1
    
    for(i = 0; i < BAT_ALL_TEMP_NUM; i++)
    {    
        gTempRunAwayCnt[i] = 0;
    }
}

//=============================================================================================
//函数名称	: void DataDealGetBMSDataRdy(u8 mask)
//输入参数	: mask:希望关注的相应位;MASK为0；则表示不关心任何数据
//输出参数	: void
//函数功能	: 电池数据确认准备好获取
//注意事项	:
//=============================================================================================
u8 DataDealGetBMSDataRdy(u8 mask)
{
    #ifdef BMS_USE_SOX_MODEL
        #ifdef BMS_USE_SOX_MODEL_NEED_FUEL
        if(mask == (gBatteryInfo.Status.DataReady & mask))
        {
            return 1;
        }
        else
        {
            return 0;
        }    
        #else
        u8 masktemp;
        
        masktemp = mask;
        BITCLR(masktemp,1);
        
        if(masktemp == (gBatteryInfo.Status.DataReady & masktemp))
        {
            return 1;
        }
        else
        {
            return 0;
        }        
        #endif

    #else
	if(mask == (gBatteryInfo.Status.DataReady & mask))
	{
		return 1;
	}
	else
	{
		return 0;
	}
    #endif
}

//=============================================================================================
//函数名称	: void DataDealSetBMSDataRdy(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 设置电池数据准备标志
//注意事项	:
//=============================================================================================
void DataDealSetBMSDataRdy(e_DataRdy set)
{
	if(set < eDataRdhMax)
	{
		BITSET(gBatteryInfo.Status.DataReady,set);
	}
}

//=============================================================================================
//函数名称	: u8 DataDealRefreshVoltBuff(u8 cellnum,s16 volt)
//输入参数	: cellnum：输入的编号; volt:单体电压
//输出参数	: buff填充满标志,0:未满；1：满了
//函数功能	: 刷新电压buff
//注意事项	:
//=============================================================================================
u8 DataDealRefreshVoltBuff(u8 cellnum,s16 volt)
{
    static u8 cnt[BAT_CELL_NUM] = {0};
    u8 i = 0;
    
    if(DATADEAL_FIRST_FLAG_VOLT == (gDataDealFirstFlag & DATADEAL_FIRST_FLAG_VOLT))
    {
        for(i = 0;i < BAT_CELL_NUM;i++)
        {
            cnt[i] = 0;
        }
        gDataDealFirstFlag &= (~DATADEAL_FIRST_FLAG_VOLT);
    }
    
    if(cellnum < BAT_CELL_NUM)
    {
        if(cnt[cellnum] < DATADEAL_VOLT_BUFF_LEN)
        {
            gVoltBuff[cellnum][cnt[cellnum]] = volt; //写入buff中
            cnt[cellnum]++;
            if(cnt[cellnum] >= DATADEAL_VOLT_BUFF_LEN)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else        //超过了buff的长度，开始刷新buff
        {
            cnt[cellnum] = cnt[cellnum] - 1;
            for(i = 0;i < cnt[cellnum];i++)
            {
                gVoltBuff[cellnum][i] = gVoltBuff[cellnum][i + 1];  //全部往前移，保持最后一个buff为最新数据
            }
            gVoltBuff[cellnum][i] = volt;
            cnt[cellnum]++;
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

//=============================================================================================
//函数名称	: u8 DataDealRefreshTempBuff(u8 cellnum,s16 temp)
//输入参数	: cellnum：输入的编号;temp:单体温度
//输出参数	: buff填充满标志,0:未满；1：满了
//函数功能	: 刷新温度buff
//注意事项	:
//=============================================================================================
u8 DataDealRefreshTempBuff(u8 cellnum,s16 temp)
{
    static u8 cnt[(BAT_ALL_TEMP_NUM)] = {0};
    u8 i = 0;
    
    if(DATADEAL_FIRST_FLAG_TEMP == (gDataDealFirstFlag & DATADEAL_FIRST_FLAG_TEMP))
    {
        for(i = 0;i < (BAT_ALL_TEMP_NUM);i++)
        {
            cnt[i] = 0;
        }
        gDataDealFirstFlag &= (~DATADEAL_FIRST_FLAG_TEMP);
    }    
    
    if(cellnum < (BAT_ALL_TEMP_NUM))
    {
        if(cnt[cellnum] < DATADEAL_TEMP_BUFF_LEN)
        {
            gTempBuff[cellnum][cnt[cellnum]] = temp; //写入buff中
            cnt[cellnum]++;
            if(cnt[cellnum] >= DATADEAL_TEMP_BUFF_LEN)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else        //超过了buff的长度，开始刷新buff
        {
            cnt[cellnum] = cnt[cellnum] - 1;
            for(i = 0;i < cnt[cellnum];i++)
            {
                gTempBuff[cellnum][i] = gTempBuff[cellnum][i + 1];  //全部往前移，保持最后一个buff为最新数据
            }
            gTempBuff[cellnum][i] = temp;
            cnt[cellnum]++;
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

//=============================================================================================
//函数名称	: void DataDealVoltCharaTask(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 电压特征信息计算任务
//注意事项	: 100ms周期
//=============================================================================================
void DataDealVoltCharaTask(void)
{
    u16 num = 0;
    u16 maxNum = 0;
    u16 minNum = 0;
    s16 nowVolt = 0;
    s16 maxVolt = -10000;
    s16 minVolt = 10000;
    u32 sumVolt = 0;
    
    s16 minSpeedV = 10000;      //最小电压下降速度
    u8  minSpeedN = 0;          //最小电压下降速度电池号
    static u8 timer = 0;        //用于计算电池电压下降速度使用，10S周期
    static u8 firstflag = 0;    //用于滤掉第一个上次电压
    
    if((0 == DataDealGetBMSDataRdy(0X07)) && (DT_N == DTTimerGetState(DT_ID_AFE_FAULT)))  //检查是否数据已准备好  或者 AFE发生故障
    {
    	return;
    }
    
    if(DATADEAL_FIRST_FLAG_VOLT == (gDataDealFirstFlag & DATADEAL_FIRST_FLAG_VOLT))
    {   
        firstflag = 0;   
        for(num = 0; num < BAT_CELL_NUM; num++)
        {        
            gBatteryInfo.VoltChara.VoltDropSpeed[num] = 0;
            gBatteryInfo.VoltChara.MinDropVoltNum = 0;
            gBatteryInfo.VoltChara.MinDropVolt = 0; 
        }
    }
    
	for(num = 0; num < BAT_CELL_NUM; num++)
	{
		nowVolt = DataDealGetCellVolt(num);
		sumVolt += nowVolt;                         //计算总电压

		if(maxVolt < nowVolt)                       //求单体最大电压和电池号
		{
			maxVolt = nowVolt;
			maxNum = num;
		}

		if(minVolt > nowVolt)                       //求单体最小电压和电池号
		{
			minVolt = nowVolt;
			minNum = num;
		}
        
        if(0 == firstflag)  //首次标志结束
        {
            DataDealRefreshVoltBuff(num,nowVolt);
            if(num == (BAT_CELL_NUM - 1))
            {
                firstflag = 1;                 
            }
        }     
	}
    
    if(maxVolt == -10000)//没有有效最大值
    {
        maxVolt = 0xffff;
        maxNum = 0xff;
    }
    if(minVolt == 10000)//没有有效最小值
    {
        minVolt = 0xffff;
        minNum = 0xff;
    }

	gBatteryInfo.VoltChara.MaxVolt = maxVolt;      //最大电压
	gBatteryInfo.VoltChara.MinVolt = minVolt;      //最小电压
	gBatteryInfo.VoltChara.MaxVNum = maxNum + 1;   //电压最大电池号(从1开始)
	gBatteryInfo.VoltChara.MinVNum = minNum + 1;   //电压最小电池号(从1开始)

    gBatteryInfo.VoltChara.PerVolt = (u16)(sumVolt / BAT_CELL_NUM);	//单节电池平均电压,单位mV
    gBatteryInfo.Data.TolVolt = (u16)(sumVolt / 10);	//总电压
    
    //电芯电压下降速度
    if(DATADEAL_VOLT_REFRESH_PERIOD * 10 <= timer)     //12S周期
    {
        timer = 0;    
        for(num = 0; num < BAT_CELL_NUM; num++)
        {     
            nowVolt = DataDealGetCellVolt(num); //获取单体电压
            
            if((1000 < nowVolt) && (DT_N == DTTimerGetState(DT_ID_AFE_XREADY)))  //电压数据有效
            {
                if(DataDealRefreshVoltBuff(num,nowVolt))    //buff满了
                {
                    //获取电压下降速度120S,12S的刷新周期
                    gBatteryInfo.VoltChara.VoltDropSpeed[num] = \
                    gVoltBuff[num][DATADEAL_VOLT_BUFF_LEN - 1] - gVoltBuff[num][0];
                    
                    if(gBatteryInfo.VoltChara.VoltDropSpeed[num] < minSpeedV)
                    {
                        minSpeedV = gBatteryInfo.VoltChara.VoltDropSpeed[num];
                        minSpeedN = num;
                    } 

                    if(minSpeedV == 10000)//没有有效最小值
                    {
                        minSpeedV = 0;
                        minSpeedN = 0;
                    }
                   
                    gBatteryInfo.VoltChara.MinDropVoltNum = minSpeedN;
                    gBatteryInfo.VoltChara.MinDropVolt = minSpeedV;                    
                }
            }
        }
    }
    else
    {
        timer++;
    }
    
    DataDealSetBMSDataRdy(e_VoltDataRdy);

}

//=============================================================================================
//函数名称	: void DataDealTempCharaTask(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 温度特征信息计算任务
//注意事项	: 100ms周期
//=============================================================================================
void DataDealTempCharaTask(void)
{
    u16 num = 0;
    u16 maxNum = 0;
    u16 minNum = 0;
    s16 nowTemp = 0;
    s16 maxTemp = -10000;
    s16 minTemp = 10000;
    s16 sumTemp = 0;
    
    s16 maxSpeedT = -10000;     //最大温度上升速度
    u8  maxSpeedN = 0;          //最大温度上升速度电池号
    static u8 timer = 0;        //用于计算电池温度上升速度使用，10S周期
    static u8 firstflag = 0;    //用于滤掉第一个上次温度
    
    if((0 == DataDealGetBMSDataRdy(0X07)) && (DT_N == DTTimerGetState(DT_ID_AFE_FAULT)))  //检查是否数据已准备好  或者 AFE发生故障
    {
    	return;
    }
    
    if(DATADEAL_FIRST_FLAG_TEMP == (gDataDealFirstFlag & DATADEAL_FIRST_FLAG_TEMP))
    {   
        firstflag = 0;    
        for(num = 0;num < BAT_ALL_TEMP_NUM;num++)
        {
            gTempRunAwayCnt[num] = 0;
            gBatteryInfo.TempChara.TempRiseSpeed[num] = 0;
        }        
    }    
    
	minTemp = DataDealGetCellTemp(0);
	
    for(num = 0; num < BAT_ALL_TEMP_NUM; num++)
    {
        nowTemp = DataDealGetCellTemp(num);

        sumTemp += nowTemp;                         //温度求和

        if(maxTemp < nowTemp)                       //求单体最大温度和电池号
        {
            maxTemp = nowTemp;
            maxNum = num;
        }

        if(minTemp > nowTemp)                       //求单体最小温度和电池号
        {
            minTemp = nowTemp;
            minNum = num;
        }
        
        if(0 == firstflag)  //首次标志结束
        {
            DataDealRefreshTempBuff(num,nowTemp);
            gTempRunAwayCnt[num] = 0;
            if(num == (BAT_ALL_TEMP_NUM - 1))
            {
                firstflag = 1;    
            }
        }           
    }
        
    if(maxTemp == -10000)//没有有效最大值
    {
        maxTemp = NTC_TEMP_OFF;
        maxNum = 0xff;
    }
    if(minTemp == 10000)//没有有效最小值
    {
        minTemp = NTC_TEMP_OFF;
        minNum = 0xff;
    }    

    gBatteryInfo.TempChara.MaxTemp = maxTemp;      //组最大温度
    gBatteryInfo.TempChara.MinTemp = minTemp;      //组最小温度
    gBatteryInfo.TempChara.MaxTNum = maxNum + 1;   //组温度最大电池号(从1开始)
    gBatteryInfo.TempChara.MinTNum = minNum + 1;   //组温度最小电池号(从1开始)

    gBatteryInfo.TempChara.PerTemp = (u16)(sumTemp / (BAT_ALL_TEMP_NUM));	//单节电池平均温度
    
    //温升速度检测
    //10秒钟温度差求和，取平均    
    if(DATADEAL_TEMP_REFRESH_PERIOD * 10 <= timer)     //10S周期
    {    
        timer = 0;    
        for(num = 0; num < BAT_ALL_TEMP_NUM; num++) //加上电量计的温升速度
        {     
            nowTemp = DataDealGetCellTemp(num); //获取单体温度
            
            if((NTC_TEMP_OFF != nowTemp) && (NTC_TEMP_SHORT != nowTemp) && (DT_N == DTTimerGetState(DT_ID_AFE_XREADY)))  //温度数据有效
            {
                if(DataDealRefreshTempBuff(num,nowTemp))    //buff满了
                {
                    //获取温度上升速度10S,1S的刷新周期
                    gBatteryInfo.TempChara.TempRiseSpeed[num] = \
                    gTempBuff[num][DATADEAL_TEMP_BUFF_LEN - 1] - gTempBuff[num][0];                    

                    //增加热失控检测：近10s内温升大于或等于3度且持续8s
                    //且近120s内电压下降超过100mV
                    if(gBatteryInfo.TempChara.TempRiseSpeed[num] >= 30)
                    {
                        if(gTempRunAwayCnt[num] < 64)     //最大64S
                        {
                            gTempRunAwayCnt[num]++;
                        }
                    }
                    //恢复条件：温升速度下降到1.5度以下，等待6+故障持续时间，最大64s，解除故障
                    else if(gBatteryInfo.TempChara.TempRiseSpeed[num] < 15)
                    {
                        if(gTempRunAwayCnt[num] > 0)
                        {
                             gTempRunAwayCnt[num]--;
                        }
                    }
                
                    //求最大温度上升速度
                    if(gBatteryInfo.TempChara.TempRiseSpeed[num] > maxSpeedT)
                    {
                        maxSpeedT = gBatteryInfo.TempChara.TempRiseSpeed[num];
                        maxSpeedN = num;
                    }          
                    
                    if(maxSpeedT == -10000)//没有有效最大值
                    {
                        maxSpeedT = 0;
                        maxSpeedN = 0;
                    }
                    gBatteryInfo.TempChara.MaxRiseTempNum = maxSpeedN;
                    gBatteryInfo.TempChara.MaxRiseTemp = maxSpeedT;
                    
                }         
            }
        }  
    }
    else
    {
        timer++;
    }    
    
    DataDealSetBMSDataRdy(e_TempDataRdy);    
}

//=============================================================================================
//函数名称	: void DataDealFiltSOCTask(void)
//输入参数	: void
//输出参数	: void
//函数功能	: SOC信息处理任务
//注意事项	: 100ms周期
//=============================================================================================
void DataDealFiltSOCTask(void)
{
    gBatteryInfo.Data.FiltSOC = gBatteryInfo.Data.SOC;
    
    #ifdef BMS_USE_SOX_MODEL
    
    #else
    //充满到100% SOC条件改为：电池电量计的充满标志置位，且最低电芯电压大于单芯满充电大于（单芯满充电压-20mV）。
    //加入充满标志？
    if(gBatteryInfo.VoltChara.MinVolt < FULL_CHARGE_CELL_VOLT_LV1 || gBatteryInfo.Status.ProtSta != eProt_Full)
    {
        if(gBatteryInfo.Data.FiltSOC > 990)
        {
            gBatteryInfo.Data.FiltSOC = 990;
        }
    }
    
    //如果二级欠压后，SOC大于2%，强制把SOC置为2%。
    if(1 == gDchgUV2Flag)
    {
        if(gBatteryInfo.Data.FiltSOC > 20)
        {
            gBatteryInfo.Data.FiltSOC = 20;
        }
    } 
    #endif
    
}

//=============================================================================================
//函数名称	: void DataDealFiltCurrTask(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 电流信息处理任务
//注意事项	: 250ms周期
//=============================================================================================
void DataDealFiltCurrTask(void)
{
    static bool PreChgSta = false;
    static int16_t PreSOC = 0x0000;  
    s16 Curr10ma = 0;
	u16 AbsCurr;								/* 电流绝对值 */
	static u16 HighCurrTh = 0;					/* 大放电电流监测门限 */
	static u16 MaxHighCurrTime = 0;			/* 最大放电电流监测周期 */
	static u16 PerHighCurrTime = 0;			/* 平均放电电流监测周期 */
	static u16 LastHighCurrTh = 0xFFFF;		/* 上一次大放电电流监测门限 */
	static u16 LastMaxHighCurrTime = 0xFFFF;	/* 上一次最大放电电流监测周期 */
	static u16 LastMinHighCurrTime = 0xFFFF;	/* 上一次平均放电电流监测周期 */	
	static u16 HighCurrTimer = 0;				/* 大放电电流监测门限定时器 */
	static u16 MaxCurrTimer = 0;				/* 求一段时间内的最大电流的定时器 */
	static u16 PerCurrTimer = 0;				/* 求一段时间内的平均电流的定时器 */
	static u32 SumCurr = 0;					/* 用于求平均电流的累加电流变量 */
    
    if(False == DataDealGetBMSDataRdy(0x03))
    {
        return;
    }
    
    Curr10ma = gBatteryInfo.Data.Curr;

    /* 最近最大充放电电流，在机柜中充电至80%时清零 */
    if((((!PreChgSta) && CommCtrlIsChargerPlugIn()) && gBatteryInfo.Data.SOC >= 800)         
        ||(CommCtrlIsChargerPlugIn() && PreSOC < 800 && gBatteryInfo.Data.SOC >= 800))
    {
        gBatteryInfo.CurrChara.MaxChgCurrRecent = 0;
        gBatteryInfo.CurrChara.MaxDchgCurrRecent = 0;
    }
    else
    {
        if(Curr10ma > gBatteryInfo.CurrChara.MaxChgCurrRecent)
        {
            gBatteryInfo.CurrChara.MaxChgCurrRecent = Curr10ma;
        }
        if(Curr10ma < gBatteryInfo.CurrChara.MaxDchgCurrRecent)
        {
            gBatteryInfo.CurrChara.MaxDchgCurrRecent = Curr10ma;
        }
    }
    PreChgSta = CommCtrlIsChargerPlugIn();
    PreSOC = gBatteryInfo.Data.SOC;
    
	/* Handry Add 20-07-02
	3C电流检测 250ms周期运行任务 */    
	HighCurrTh = (gConfig.configadd.currdet.highcurrdet & 0x00FF) * 200; 			/* 单位2000mA，转换为10mA */    
	/* 获取3C电流检测参数 */
	if(LastHighCurrTh != HighCurrTh)
	{
		gBatteryInfo.CurrChara.HighCurrTime = 0;
		HighCurrTimer = 0;
		LastHighCurrTh = HighCurrTh;
	}
        
	if(Curr10ma <= 0)	/* 放电时检测 */
	{
		/* 获取电流绝对值 */
		AbsCurr = (uint16_t)(0 - Curr10ma);	

		/* 检测大放电电流持续时间，5秒为单位 */
		if(AbsCurr > HighCurrTh)
		{
			HighCurrTimer++;
			if(0 == HighCurrTimer % 20)	/* 持续大于5秒 */
			{
				HighCurrTimer %= 20;
				gBatteryInfo.CurrChara.HighCurrTime++;
				if(gBatteryInfo.CurrChara.HighCurrTime >= 0x0F) /* 0~150s */
				{
					gBatteryInfo.CurrChara.HighCurrTime = 0x0F;
				}
			}
		}
		else if(AbsCurr < (HighCurrTh - 200)) /* 放电电流小于(HighCurrTh - 200)，则清零 */
		{
			gBatteryInfo.CurrChara.HighCurrTime = 0;
			HighCurrTimer = 0;
		}
		else	/* 放电电流在(HighCurrTh - 200)与HighCurrTh之间 */
		{
			;
		}
	}
    else
    {
        AbsCurr = 0;
    }
    
    /* 检测一段时间内的最大和平均放电电流，2A为单位 */
	MaxHighCurrTime = ((gConfig.configadd.currdet.highcurrdet >> 8) & 0x000F) * 20;	/* 单位5000ms，转换为250ms */
	PerHighCurrTime = ((gConfig.configadd.currdet.highcurrdet >> 12) & 0x000F) * 20;  /* 单位5000ms，转换为250ms */	
	/* 获取最大最小电流检测周期参数 */
	if(LastMaxHighCurrTime != MaxHighCurrTime || (LastMinHighCurrTime != PerHighCurrTime))
	{
		gBatteryInfo.CurrChara.PerCurr30S = 0;
		gBatteryInfo.CurrChara.MaxCurr30S = 0;
		MaxCurrTimer = 0;		
		PerCurrTimer = 0;
		LastMaxHighCurrTime = MaxHighCurrTime;
		LastMinHighCurrTime = PerHighCurrTime;
	}

	MaxCurrTimer++; 
	PerCurrTimer++;
	
	AbsCurr /= 100;	//单位转换为1A	
	if(PerCurrTimer < PerHighCurrTime)
	{
		SumCurr += AbsCurr;
	}
	else if(PerCurrTimer == PerHighCurrTime)
	{
		/* 求PerHighCurrTime时间内的平均电流 */
        SumCurr += AbsCurr;
		gBatteryInfo.CurrChara.PerCurr30S = SumCurr / PerHighCurrTime;
		SumCurr = AbsCurr;
		PerCurrTimer = 0;
	}
	else
	{
	    gBatteryInfo.CurrChara.PerCurr30S = 0;
		SumCurr = 0;
		PerCurrTimer = 0;
	}	

	AbsCurr /= 2;	/* 单位转换为2A	*/
	if(MaxCurrTimer < MaxHighCurrTime)
	{
		if(AbsCurr > gBatteryInfo.CurrChara.MaxCurr30S)
		{
			gBatteryInfo.CurrChara.MaxCurr30S = AbsCurr;
		}
	}
	else
	{
		MaxCurrTimer = 0;
        gBatteryInfo.CurrChara.MaxCurr30S = AbsCurr;        
	}    
}

//=============================================================================================
//函数名称	: void DataDealFuelTask(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 电量计信息处理任务
//注意事项	: 100ms周期
//=============================================================================================
void DataDealFuelTask(void)
{
    if(False == CommCtrlIsChargerPlugIn())
    {
        gBatteryInfo.Data.FullChgTime = 0xffff;
    }
    else
    {
		#ifdef BMS_USE_SOX_MODEL
		if(eProt_Full == gBatteryInfo.Status.ProtSta)
		#else
        if((gBatteryInfo.Addit.FuelStaFlag & MAX1720X_FULLCHG_FLAG) != 0)
		#endif
        {
            gBatteryInfo.Data.FullChgTime = 0;
        }
        else if(gBatteryInfo.Data.Curr > 5)
        {
			//充满时间计算：待充容量 / 当前充电电流
            gBatteryInfo.Data.FullChgTime = 
                (u32)((gBatteryInfo.Data.TolCap - gBatteryInfo.Data.RemainCap) * 6 / gBatteryInfo.Data.Curr);
        }
    }
    
    gBatteryInfo.Data.RemainPower = gBatteryInfo.Data.RemainCap * gBatteryInfo.Data.TolVolt / 10000;
}

//=============================================================================================
//函数名称	: static u16 DataDealGetCellVolt(u8 num)
//输入参数	: 电池号
//输出参数	: 电池号对应的电压值
//函数功能	: 获取单体电压值
//注意事项	:
//=============================================================================================
static u16 DataDealGetCellVolt(u8 num)
{
	//参数有效性判断
	if(num > BAT_CELL_NUM)
	{
		return 0;
	}

	return gBatteryInfo.Data.CellVolt[num];
}

//=============================================================================================
//函数名称	: static  s16 DataDealGetCellTemp(u8 num)
//输入参数	: 电池号
//输出参数	: 电池号对应的温度值
//函数功能	: 获取单体温度值
//注意事项	:
//=============================================================================================
static s16 DataDealGetCellTemp(u8 num)
{
	//参数有效性判断
	if(num < BAT_TEMP_NUM)
	{
		return gBatteryInfo.Data.CellTemp[num];
	}
    else if(num == BAT_TEMP_NUM)    //电量计采样的温度
    {
        return gBatteryInfo.Addit.FuelTemp;
    }
    else
    {
        return 0;    
    }

}

/*****************************************end of DataDeal.c*****************************************/
