//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: UserData.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 用户数据源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
//=============================================================================================
//包含头文件
//=============================================================================================
#include "UserData.h"
#include "Storage.h"
#include "DataDeal.h"
#include "BSPTimer.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
t_USER_DATA gUserID=
{
    USER_CFG_FLAG,
    {0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,
    0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff},
    {
        0,
        0xffff,
        0xffff,
        0xffff,
        0xffff,
        0xffff,
        0xffff,
        0xffff,
        0xffff,
        0,
        0,
    },
    0,
};//配置只读

const t_USER_DATA cUserIDInit=
{
	USER_CFG_FLAG,
    {0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,
    0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff},
    {
        0,
        0xffff,
        0xffff,
        0xffff,
        0xffff,
        0xffff,
        0xffff,
        0xffff,
        0xffff,
        0,
        0,
    },
    0,
};//配置只读
t_USER_DATA gUserIDBuff;//保存配置的输入buffer

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void UserDataInit(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 用户数据初始化函数
//注	意	:
//=============================================================================================
void UserDataInit(void)
{
	;
}

//=============================================================================================
//函数名称	: void UserDataLifetimeUpdate(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 用户数据历史数据更新
//注    意	: 100ms任务
//=============================================================================================
void UserDataLifetimeUpdate(void)
{
    u8 flag = 0;
    static u32 cnt=0xffffffff;
    static u16 delaytimer = 0;
//    if((gTimer1ms <= 5000) || (0 == DataDealGetBMSDataRdy(0x03)))	//上电5s前不执行，AFE、电量计数据已准备好才执行
//    {
//        testcnt++;
//    	return; //不用gTimer1ms延时是为了防止在gTimer1ms溢出后，此函数会返回不执行
//    }
    if((delaytimer <= 50) || (0 == DataDealGetBMSDataRdy(0x03)))	//上电5s前不执行，AFE、电量计数据已准备好才执行
    {
        delaytimer++;
        if(delaytimer > 50)
        {
            delaytimer = 51;   //5秒计时已经到
        }
    	return;
    }   
    
    if((gUserIDBuff.lt.maxv == 0xffff) || (gBatteryInfo.VoltChara.MaxVolt > gUserIDBuff.lt.maxv))
    {
        gUserIDBuff.lt.maxv = gBatteryInfo.VoltChara.MaxVolt;
        gUserIDBuff.lt.maxvn = gBatteryInfo.VoltChara.MaxVNum;
        flag= 1;
    }
    if((gUserIDBuff.lt.minv == 0xffff) || ((gBatteryInfo.VoltChara.MinVolt < gUserIDBuff.lt.minv)&&(gBatteryInfo.VoltChara.MinVolt >=1000)))
    {
        gUserIDBuff.lt.minv = gBatteryInfo.VoltChara.MinVolt;
        gUserIDBuff.lt.minvn = gBatteryInfo.VoltChara.MinVNum;
        flag= 1;
    }
    if((gUserIDBuff.lt.maxt == 0xffff) || (gBatteryInfo.TempChara.MaxTemp> ((int16_t)gUserIDBuff.lt.maxt - 400)))
    {
        gUserIDBuff.lt.maxt = gBatteryInfo.TempChara.MaxTemp + 400;
        gUserIDBuff.lt.maxtn = gBatteryInfo.TempChara.MaxTNum;
        flag= 1;
    }
    if((gUserIDBuff.lt.mint == 0xffff) || (gBatteryInfo.TempChara.MinTemp < ((int16_t)gUserIDBuff.lt.mint - 400)))
    {
        gUserIDBuff.lt.mint = gBatteryInfo.TempChara.MinTemp + 400;
        gUserIDBuff.lt.mintn = gBatteryInfo.TempChara.MinTNum;
        flag= 1;
    }
    if((gBatteryInfo.Data.Curr > gUserIDBuff.lt.maxcc))
    {
        gUserIDBuff.lt.maxcc = gBatteryInfo.Data.Curr;
        flag= 1;
    }
    if((gBatteryInfo.Data.Curr < (0-(int16_t)gUserIDBuff.lt.maxdc)))
    {
        gUserIDBuff.lt.maxdc = 0 - gBatteryInfo.Data.Curr;
        flag= 1;
    }

    if(cnt < 0xffffffff)
    {
        cnt++;
    }

    if(flag == 1)
    {
        cnt |= 0x80000000;
    }

    if(cnt >= (0x80000000 | (6000)))	//最少隔十分钟存一次,100ms任务
    {
        cnt = 0;
        gStorageCTRL |= FLASH_WR_UD_MASK;
    }
}

/*****************************************end of UserData.c*****************************************/
