//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SysState.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 系统状态代码
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
//=============================================================================================
//包含头文件
//=============================================================================================
#include "SysState.h"
#include "CommCtrl.h"
//#include "BSPGPIO.h"
#include "SH367309.h"
#include "BSP_GPIO.h"

//=============================================================================================
//全局变量
//=============================================================================================
u8 gChgFlag = 0;        //充电标志，0：不充电；1：充电
u8 gDchgFlag = 0;       //放电标志，0：不放电；1：充电

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void SysStateInit(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 系统状态初始化函数
//注    意	:
//=============================================================================================
void SysStateInit(void)
{
	SysStateSetWorkSta(eWork_Init);
}

//=============================================================================================
//函数名称	: void SysStateCurrentFlagUpdate(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 充放电电流标志状态更新
//注    意	:
//=============================================================================================
void SysStateCurrentFlagUpdate(void)
{
    static u16 cntp = 0,cntn = 0,cntidle = 0;

    //有充电电流 并且充电管打开，并且充电枪插入标志置1
    if((gBatteryInfo.Data.Curr >= PCURR_LIMIT) && (gBatteryInfo.Status.MOSStatus & (1 << SH367309_CHG_MOS_SEL))
        && (True == CommCtrlIsChargerPlugIn())
    )
    {
        cntidle = 0;
        if(cntp < 60000)
        {
            cntp++;
        }
    }
    else
    {
        cntp=0;
        //空闲计数
        if(cntidle < 60000)
        {
            cntidle++;
        }
    }

    //有放电电流 并且放电管打开
    if((gBatteryInfo.Data.Curr <= NCURR_LIMIT) && (gBatteryInfo.Status.MOSStatus & (1 << SH367309_DCHG_MOS_SEL)))
    {
        cntidle = 0;
        if(cntn < 60000)
        {
            cntn++;
        }
    }
    else
    {
        cntn=0;
        //空闲计数
        if(cntidle < 60000)
        {
            cntidle++;
        }
    }

    //滤波1.5秒
    if(cntp >= CURR_DELAY)
    {
        gChgFlag = 1;
    }
    else if(gBatteryInfo.Data.Curr <= PCURR_LIMIT_RE)
    {
        gChgFlag = 0;
    }

    if(cntn >= CURR_DELAY)
    {
        gDchgFlag = 1;
    }
    else if(gBatteryInfo.Data.Curr >= NCURR_LIMIT_RE)
    {
        gDchgFlag = 0;
    }
    
    if((1 == gDchgFlag) && (0 == gChgFlag))          //放电状态
    {
        gBatteryInfo.Status.ChgDchgSta = eChgDchg_Dchg;
    }
    else if((0 == gDchgFlag) && (1 == gChgFlag))      //充电状态
    {
        gBatteryInfo.Status.ChgDchgSta = eChgDchg_Chg;
    }
    else if((0 == gDchgFlag) && (0 == gChgFlag))      //空闲状态
    {
        if(cntidle >= (CURR_DELAY * 2))
        {
            gBatteryInfo.Status.ChgDchgSta = eChgDchg_Idle;
        }
    }
}

//=============================================================================================
//函数名称	: void SysStateRunTest(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: BMS运行时跑的测试任务
//注    意	: 测试引脚输出方波，运行时调用
//=============================================================================================
void SysStateRunTest(void)
{
//    static u8 tranflag = 0;
//    
//    if(0 == tranflag)
//    {
//        tranflag = 1;
//        BSPGPIOSetPin(SYS_TEST_PORT,SYS_TEST_PIN);
//    }
//    else
//    {
//        tranflag = 0;
//        BSPGPIOClrPin(SYS_TEST_PORT,SYS_TEST_PIN);
//    }
}

//=============================================================================================
//函数名称	: e_ChgDchgStatus SysStateGetChgDchgSta(void)
//函数参数	: void
//输出参数	: 充放电状态；0：空闲；1：充电；2：放电
//静态变量	: void
//功    能	: 获取充放电标志
//注    意	:
//=============================================================================================
e_ChgDchgStatus SysStateGetChgDchgSta(void)
{
    return gBatteryInfo.Status.ChgDchgSta;
}

//=============================================================================================
//函数名称	: e_WorkStatus SysStateGetWorkSta(void)
//函数参数	: 电池工作状态；0：初始化状态；1：空闲状态；2：正在启动状态；3：运行状态；4：故障状态
//输出参数	: void
//静态变量	: void
//功    能	: 设置电池工作状态
//注    意	:
//=============================================================================================
void SysStateSetWorkSta(e_WorkStatus worksta)
{
    gBatteryInfo.Status.WorkSta = worksta;
}
    
//=============================================================================================
//函数名称	: e_WorkStatus SysStateGetWorkSta(void)
//函数参数	: void
//输出参数	: 电池工作状态；0：初始化状态；1：空闲状态；2：正在启动状态；3：运行状态；4：故障状态
//静态变量	: void
//功    能	: 获取电池工作状态
//注    意	:
//=============================================================================================
e_WorkStatus SysStateGetWorkSta(void)
{
    return gBatteryInfo.Status.WorkSta;
}

/*****************************************end of SysState.c*****************************************/
