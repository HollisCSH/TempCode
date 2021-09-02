//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: VoltBalance.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 电压均衡代码源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "VoltBalance.h"
#include "CommCtrl.h"
#include "DataDeal.h"
#include "SH367309.h"
#include "ParaCfg.h"
#include "DTCheck.h"

//=============================================================================================
//全局变量
//=============================================================================================
t_VoltBalance gVoltBalance; //均衡控制结构体

//=============================================================================================
//函数名称	: u8 VoltBalanceInit(void)
//输入参数	: void
//输出参数	: 初始化是否成功，TRUE:成功  FALSE:失败
//静态变量	: void
//功    能	: 电压均衡初始化函数
//注    意	:
//=============================================================================================
u8 VoltBalanceInit(void)
{
	u8 result = TRUE;

	gVoltBalance.testbalctrl = 0;	//测试均衡控制变量，每一bit代表电池开均衡
	gVoltBalance.balctrl = 0;		//均衡控制变量，每一bit代表电池开均衡
	gVoltBalance.baltimer = 0;		//均衡周期控制计时器
    
	//获取配置的值
	gVoltBalance.enable  = gConfig.bala.en;
    gVoltBalance.ballowvolt = gConfig.bala.lv;
    gVoltBalance.balupvolt = gConfig.bala.uv;    
    gVoltBalance.balthresh = gConfig.bala.deltav;
    
	return result;
}

//=============================================================================================
//函数名称	: static u8 VoltBalanceGetBalanceDebug(void)
//输入参数	: void
//输出参数	: 是否开启均衡测试模式,0:未开启；1：开启
//静态变量	: void
//功    能	: 检测是否开启均衡测试模式
//注    意	:
//=============================================================================================
static u8 VoltBalanceGetBalanceDebug(void)
{
	return CommCtrlIsBalanceTestMode();
}

//=============================================================================================
//函数名称	: static u8 VoltBalanceGetChargePlugStaus(void)
//输入参数	: void
//输出参数	: 充电枪是否插入。1：插入；0：未插入
//静态变量	: void
//功    能	: 检测充电枪是否插入函数
//注    意	:
//=============================================================================================
static u8 VoltBalanceGetChargePlugStaus(void)
{
	u8 chgplgsta = 0;	//充电器是否插入状态

	chgplgsta =  CommCtrlIsChargerPlugIn();

	return	chgplgsta;
}

//=============================================================================================
//函数名称	: u32 VoltBalanceGetBatValidMask(u8 iseven)
//输入参数	: isodd:是否是偶数节。0:偶数节；1：奇数节
//输出参数	: 输出电池为偶数节或奇数节的电池bit
//静态变量	: void
//功    能	: 检测电池为偶数节或奇数节的电池bit
//注    意	:
//=============================================================================================
u32 VoltBalanceGetBatValidMask(u8 isodd)
{
    u32 batbalance = 0;	//电池开均衡节数，偶数或奇数节
    u8 i = 0;			//循环索引
    u8 j = 0;

	for(i = 0;i < BAT_CELL_NUM;i++)
	{
		//判断有效节数
		if(((1 << i) == (SH367309_VALID_BAT & (0x1 << i))))
		{
			//奇数或偶数
			if(isodd == (j % 2))
			{
				batbalance |= 0x1<<i;	
			}
            j++;
		}
	}

    return batbalance;
}

//=============================================================================================
//函数名称	: static u16 VoltBalanceGetMinCellVolt(void)
//输入参数	: void
//输出参数	: 单体电压最小值
//静态变量	: void
//功    能	: 获取单体电压最小值
//注    意	:
//=============================================================================================
static u16 VoltBalanceGetMinCellVolt(void)
{
	return gBatteryInfo.VoltChara.MinVolt;
}

//=============================================================================================
//函数名称	: static u16 VoltBalanceGetCellVolt(u8 cellnum)
//输入参数	: void
//输出参数	: 单体电压
//静态变量	: void
//功    能	: 获取单体电压值
//注    意	:
//=============================================================================================
static u16 VoltBalanceGetCellVolt(u8 cellnum)
{
	if(cellnum >= BAT_CELL_NUM)
	{
		return	0;	//有效性判断
	}

	return gBatteryInfo.Data.CellVolt[cellnum];
}

//=============================================================================
//函数名称: void VoltBalanceSetBalance(u32 battery)
//输入参数: battery按位表示均衡电池[0停止均衡 1开启均衡]
//输出参数: void
//功能描述: 均衡控制控制电池单体均衡启停
//注意事项: void
//=============================================================================
static void VoltBalanceSetBalance(u32 battery)
{
    if(battery != gSHAFECtrl.balctrl) //均衡与上次不一致时，重新开启均衡
    {
        gSHAFECtrl.balctrl = battery;
        SH367309EventFlagSet(SH367309_EVE_CTRL_BAL);
    }
}
#if defined(USE_B22_IM_PCBA)
//=============================================================================
//函数名称: u16 VoltBalanceCalcDiff(u16 arr[],u16 n)
//输入参数: 获取数组中最大差值
//输出参数: void
//功能描述: void
//注意事项: void
//=============================================================================
u16 VoltBalanceCalcDiff(u16 arr[],u16 n)
{
	u16 min=arr[0],max=arr[0];
	u8 count;
	
	for(count = 1;count < n;count++)
	{
		if(min>arr[count])
            min=arr[count];
		if(max<arr[count])
            max=arr[count];
	}
	return max-min;
}

//=============================================================================
//函数名称: void VoltBalanceHighLowEndianBalance()
//输入参数: 高低端均衡控制
//输出参数: void
//功能描述: void
//注意事项: void
//=============================================================================
void VoltBalanceHighLowEndianBalance(void)
{
    #define     LOW_ENDIAN_CELL_NUM     10
    #define     HIGH_ENDIAN_CELL_NUM    7
    
    #define     ENDIAN_OPEN_BALA_VOLT_DIFF     10  //高低端之间开启均衡压差
    #define     CELL_OPEN_BALA_VOLT_DIFF       40  //判断开启高端均衡，允许的高/低端内部压差
    
    //高低端最大压差
    u16 HighVoltDiff = 0,LowVoltDiff;
    //前1~10串的平均单体电压为LVavg，后11~17串的平均单体电压为HVavg
    u16 LVavg = 0,HVavg = 0,SumV = 0;
    static union
	{
		struct
		{
			u8 HighBala  :1;	//开高端均衡
			u8 LowBala   :1;	//开低端均衡
            u8 Normal    :1;    //正常模式
			u8 RVSD      :5;    //保留
		}BalaFlagBit;
		u8 BalaFlagByte;
	}tBalaFlag;     
    u8 i = 0;
    
    HighVoltDiff = VoltBalanceCalcDiff(&gBatteryInfo.Data.CellVolt[10],HIGH_ENDIAN_CELL_NUM);
    LowVoltDiff = VoltBalanceCalcDiff(gBatteryInfo.Data.CellVolt,LOW_ENDIAN_CELL_NUM);

    SumV = 0;
    for(i = 0;i < LOW_ENDIAN_CELL_NUM;i++)
    {
        SumV += VoltBalanceGetCellVolt(i);
    }
    LVavg = SumV / LOW_ENDIAN_CELL_NUM;
    
    SumV = 0;
    for(i = 0;i < HIGH_ENDIAN_CELL_NUM;i++)
    {
        SumV += VoltBalanceGetCellVolt(LOW_ENDIAN_CELL_NUM + i);
    }
    HVavg = SumV / HIGH_ENDIAN_CELL_NUM;
    
    if((0 == (gVoltBalance.balctrl & 0x3FFFFFFF)) 
        && !(gBatteryInfo.Fault.FaultInfo[0] & DEV_FAULT1_AFE))
    {
        //高串数单体偏高，HVavg-LVavg>5mv时，触发修复，设置标志位，BL_L_EN、BL_L_EN2都设置为低
        if(HVavg - LVavg > ENDIAN_OPEN_BALA_VOLT_DIFF && False == tBalaFlag.BalaFlagBit.HighBala)
        {
            if(HighVoltDiff < CELL_OPEN_BALA_VOLT_DIFF && LowVoltDiff < CELL_OPEN_BALA_VOLT_DIFF)
                tBalaFlag.BalaFlagBit.HighBala = True;
            else
                tBalaFlag.BalaFlagBit.HighBala = False;
            tBalaFlag.BalaFlagBit.LowBala = False;
        }
        else if(True == tBalaFlag.BalaFlagBit.HighBala)
        {
            if(HVavg <= LVavg || HighVoltDiff >= CELL_OPEN_BALA_VOLT_DIFF || LowVoltDiff >= CELL_OPEN_BALA_VOLT_DIFF)
            {
                tBalaFlag.BalaFlagBit.HighBala = False;    
            }
        }
        else
        {
            tBalaFlag.BalaFlagBit.HighBala = False;
        }
        
        //低串数单体偏高，HVavg-LVavg<-5mv时，触发修复，设置标志位，BL_L_EN、BL_L_EN2都设置为高
        if(LVavg - HVavg > ENDIAN_OPEN_BALA_VOLT_DIFF && False == tBalaFlag.BalaFlagBit.LowBala)
        {
            if(LowVoltDiff < CELL_OPEN_BALA_VOLT_DIFF && HighVoltDiff < CELL_OPEN_BALA_VOLT_DIFF)
                tBalaFlag.BalaFlagBit.LowBala = True;
            else
                tBalaFlag.BalaFlagBit.LowBala = False;
            tBalaFlag.BalaFlagBit.HighBala = False;
        }
        else if(True == tBalaFlag.BalaFlagBit.LowBala)
        {
            if(LVavg <= HVavg || LowVoltDiff >= CELL_OPEN_BALA_VOLT_DIFF || HighVoltDiff >= CELL_OPEN_BALA_VOLT_DIFF)
            {
                tBalaFlag.BalaFlagBit.LowBala = False;
            }
        }
        else
        {
            tBalaFlag.BalaFlagBit.LowBala = False;
        }
        
        if(False == tBalaFlag.BalaFlagBit.LowBala && False == tBalaFlag.BalaFlagBit.HighBala)
            tBalaFlag.BalaFlagBit.Normal = True;
        else
            tBalaFlag.BalaFlagBit.Normal = False;
    }
    else
    {
        tBalaFlag.BalaFlagBit.Normal = True;
        tBalaFlag.BalaFlagBit.HighBala = False;
        tBalaFlag.BalaFlagBit.LowBala = False;
    }
    
    //高低端 均衡控制
    if(True == tBalaFlag.BalaFlagBit.Normal)
    {
        VOLT_LOW_BALA_10S_ON
		VOLT_HIGH_BALA_10S_OFF
    }
    else if(True == tBalaFlag.BalaFlagBit.HighBala)
    {
        VOLT_LOW_BALA_10S_OFF
		VOLT_HIGH_BALA_10S_OFF
    }
    else if(True == tBalaFlag.BalaFlagBit.LowBala)
    {
        VOLT_LOW_BALA_10S_ON
		VOLT_HIGH_BALA_10S_ON
    }
    else
    {
        VOLT_LOW_BALA_10S_ON
		VOLT_HIGH_BALA_10S_OFF
    }
}
#endif
//=============================================================================================
//函数名称	: void VoltBalanceTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 电压均衡函数
//注    意	:
//=============================================================================================
void VoltBalanceTask(void)
{
	static u8 isodd = 0;	//0：开启偶数节，1开启奇数节
    u8 i = 0;				//循环索引
    u8 j = 0;

    if(0 == DataDealGetBMSDataRdy(0x01))    //单体电压数据已准备好
    {
        gVoltBalance.balctrl = 0;
		#if defined(USE_B20_TOPBAND_PCBA)
        VOLT_HIGH_BALA_10S_OFF
        VOLT_LOW_BALA_10S_OFF 
		#elif defined(USE_B22_IM_PCBA)
        VOLT_LOW_BALA_10S_ON
		VOLT_HIGH_BALA_10S_OFF
		#else
		
		#endif
    	return;
    }
    
	//开启均衡调试模式
	if(1 == VoltBalanceGetBalanceDebug())
	{
		//优先开启偶数节均衡
		if(gVoltBalance.testbalctrl & VoltBalanceGetBatValidMask(0))
		{
			gVoltBalance.balctrl = gVoltBalance.testbalctrl & VoltBalanceGetBatValidMask(0);	//开启偶数节均衡
		}
		else if(gVoltBalance.testbalctrl & VoltBalanceGetBatValidMask(1))
		{
			gVoltBalance.balctrl = gVoltBalance.testbalctrl & VoltBalanceGetBatValidMask(1);	//开启奇数节均衡
		}
		else
		{
			gVoltBalance.balctrl = 0;		//关闭所有均衡
		}
        
		#if defined(USE_B20_TOPBAND_PCBA) || defined(USE_B22_IM_PCBA)
        if(BITGET32(gVoltBalance.testbalctrl,30))
            VOLT_LOW_BALA_10S_ON
        else
            VOLT_LOW_BALA_10S_OFF
        
        if(BITGET32(gVoltBalance.testbalctrl,31))
            VOLT_HIGH_BALA_10S_ON
        else
            VOLT_HIGH_BALA_10S_OFF
		#elif defined(USE_B21_IM_PCBA)
		#else
		
		#endif
        
		VoltBalanceSetBalance(gVoltBalance.balctrl);
	}
	//均衡未使能或充电枪未插入
	else if((0 == gVoltBalance.enable) || (0 == VoltBalanceGetChargePlugStaus()) )
	{
		gVoltBalance.balctrl = 0;		//关闭所有均衡
		VoltBalanceSetBalance(gVoltBalance.balctrl);
        #if defined(USE_B22_IM_PCBA)
        VOLT_LOW_BALA_10S_ON
		VOLT_HIGH_BALA_10S_OFF
        #endif
	}
	//均衡使能并且充电枪插入后，均衡检测&开启
	else if(VOLTBALANCE_PERIOD <= (gVoltBalance.baltimer++))
	{
		gVoltBalance.baltimer = 0;	//清零，重新计时
        gVoltBalance.balctrl = 0;
        
        for(i = 0;i < BAT_CELL_NUM;i++)
        {
        	//判断有效节数
            if(0 != (SH367309_VALID_BAT & (0x1 << i)))
            {
            	//判断是否符合均衡开启条件，从偶数节开始检测，下个周期进行奇数节检测
                if((isodd == j++ % 2)&&\
                    (VoltBalanceGetCellVolt(i) > gVoltBalance.ballowvolt)&&\
                    (VoltBalanceGetCellVolt(i) < gVoltBalance.balupvolt) &&\
                    (ABS(VoltBalanceGetCellVolt(i),VoltBalanceGetMinCellVolt()) > gVoltBalance.balthresh))
                {
                    gVoltBalance.balctrl |= 0x1<<i;
                }
            }

        }
        isodd = (isodd + 1) % 2;	//偶数节/奇数节切换
        VoltBalanceSetBalance(gVoltBalance.balctrl);      
        
        #if defined(USE_B22_IM_PCBA)
        VoltBalanceHighLowEndianBalance();  //高低端均衡
        #endif     
	}
	else
	{
		;
	}
}

/*****************************************end of VoltBalance.c*****************************************/
