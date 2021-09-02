//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: EventRecord.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 事件记录源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
//=============================================================================================
//包含头文件
//=============================================================================================
#include "EventRecord.h"
#include "BSP_GPIO.h"
#include "ShutDownMode.h"
#include "DataDeal.h"
#include "MBFindMap.h"
#include "MAX17205.h"
#include "CommCtrl.h"
#include "Storage.h"
#include "IOCheck.h"
#include "UserData.h"
#include "SH367309.h"
#include "SysState.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
u8 gEveRecEn = 0;             //事件记录使能；0：不使能，1使能
u8 gRecType = 0;		      //记录的类型
u16 gRecFault[4] = {0};	      //记录的故障信息
u16 gRecRSTBuff=0;            //复位事件记录缓存
u8 gRecChgBuff=0;             //充电事件记录缓存
u8 gRecPhotoBuff=0;           //拆开事件记录缓存
u16 gRecCtrlBuff=0;           //控制事件记录缓存
u16 gRecStateBuff=0;          //状态事件记录缓存
u8 gRecInfoType;              //控制/动作事件记录类型
u16 grecdiv = 0;	              //定时记录事件间隔
u8 gRecIndex;                 //事件记录索引     
    
t_RECORD gRecordRd;           //读取的记录只读
t_RECORD gRecordWr;           //写入的记录
t_RECORD_INFO gRecordInfo = //事件记录信息
{
	EVE_REC_CFG_FLAG,
    0,
    0,
    {
        0
    },
    0,
};//记录概要信息只读

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void EventRecordInit(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 事件初始化函数
//注	意	: void
//=============================================================================================
void EventRecordInit(void)
{
	gRecType = 0;
    gEveRecEn = 1;
    grecdiv = 0;
}

//=============================================================================================
//函数名称	: void EventRecordInit(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 关闭事件记录
//注	意	: void
//=============================================================================================
void EventRecordDisable(void)
{
    gEveRecEn = 0;
}

//=============================================================================================
//函数名称	: void EventRecordEnable(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 开启事件记录
//注	意	: void
//=============================================================================================
void EventRecordEnable(void)
{
    gEveRecEn = 1;
}

//=============================================================================================
//函数名称	: void EventRecordGetIOStatus(void)
//函数参数	: void
//输出参数	: STATE寄存器中磁铁及MOS管状态
//静态变量	: void
//功	能	: 事件记录获取STATE寄存器中磁铁及MOS管状态
//注    意	: 
//=============================================================================================
static u16 EventRecordGetIOStatus(void)
{
    u16 ret;
    
    ret  = gBatteryInfo.Status.MOSStatus & 0x03;
    ret |= gBatteryInfo.Status.IOStatus & 0x0004;
    //ret |= GPIO_PinOutGet(PRE_DSG_PORT,PRE_DSG)<<2;
    if(gBatteryInfo.Status.IOStatus & HALL_CHECK_SHIFT)
    {
        ret |= STATE_HALL;
    }
    
    #ifdef CANBUS_MODE_JT808_ENABLE
    //外置模块插入则记录
    if(gBatteryInfo.Status.IOStatus & EXTERN_MODULE_CHECK_SHIFT)
    {
        ret |= STATE_EXMODULE;
    }    
    #endif
    
    return ret;
}

//=============================================================================================
//函数名称	: void EventRecordChargeCheck(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 事件记录检测充电器，获取充电事件标志
//注    意	: bit0--充电满,bit1--充电器接入
//=============================================================================================
static u8 EventRecordChargeCheck(void)
{
    u8 ret=0;
#ifdef BMS_USE_SOX_MODEL
 	if(eProt_Full == gBatteryInfo.Status.ProtSta)
#else    
    if(((gBatteryInfo.Addit.FuelStaFlag & MAX1720X_FULLCHG_FLAG) != 0))   //充满电
#endif	
    {
        ret |= 0x1;
    }
    if(CommCtrlIsChargerPlugIn())
    {
        ret |= 0x2;
    }
    return ret;
}

//=============================================================================================
//函数名称	: void EventRecordMainTask(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 事件记录主任务函数
//注    意	: 50ms任务
//=============================================================================================
void EventRecordMainTask(void)
{
    u8 i;
    u16 databuf;
    u8 chg_buf;
    //static u16 reccnt = 0;
    static u8 checkrstflag = 1; //每次任务只检查一次，记录一次
    static u32 RecTim = 0;

    if((0 == DataDealGetBMSDataRdy(0x3F)) || (0 == gEveRecEn))  //所有数据已准备好
    {
    	return;
    }

    //使能定时记录
    if(grecdiv > 0)
    {
        extern u32 gTimer1ms;
        if((gTimer1ms - RecTim) >= grecdiv * 245) //250ms周期,后面补偿5ms误差
        {
        	gRecType |= REC_TYPE_PERIOD;
            RecTim = gTimer1ms;
        }
        else
        {
            ;
        }
    }

    //故障记录逻辑更改为出现故障和故障恢复时都进行记录，包括设备故障和运行故障
    for(i = 0;i < 4;i++)
    {
        if(gBatteryInfo.Fault.FaultInfo[i] != gRecFault[i])
        {
        	gRecType |= REC_TYPE_FAULT;	//	记录类型，故障类型
        }
        gRecFault[i] = gBatteryInfo.Fault.FaultInfo[i];
    }

    //加速度计记录，预留
    if(IOCheckWakeupCondGet()&WAKE_COND_ACC_MASK)
    {
        IOCheckWakeupCondClr(WAKE_COND_ACC_MASK);
        if(0 == EventRecordGetIOStatus())
        {
            gRecType |= REC_TYPE_ACC;
            if((gUserID.lt.sta&0x02)==0)
            {
                gUserIDBuff.lt.sta |= 0x02;
                gStorageCTRL |= FLASH_WR_UD_MASK;
            }
        }
    }
    
    //检查复位源，记录上次复位的原因
//    if(1 == checkrstflag)
//    {
//        u32 rststat = RCM->SSRS & 0x00002FFE;  
//        if(rststat)
//        {
//            RCM->SSRS = 0x00002FFE;         //写1清除
//            gRecType |= REC_TYPE_RESET;		//记录为复位事件
//            gRecRSTBuff = rststat;          //保存复位源
//            //清除复位源寄存器
//            checkrstflag = 0;   //等待下次复位进入
//            goto RECORD;
//        }	        
//    }
//复位标志RCC_FLAG_OBLRST, RCC_FLAG_PINRST, RCC_FLAG_PWRRST,RCC_FLAG_SFTRST, RCC_FLAG_IWDGRST, RCC_FLAG_WWDGRST and RCC_FLAG_LPWRRST.
    if(1 == checkrstflag)
    {
		u32 rststat = RCC->CSR >> 16;       //判断有无复位标志	__HAL_RCC_CLEAR_RESET_FLAGS();	RMVF
        if(rststat)
        {		
			rststat = RCC->CSR>>16;
			__HAL_RCC_CLEAR_RESET_FLAGS();  //清除复位标志
            gRecType |= REC_TYPE_RESET;		//记录为复位事件
            gRecRSTBuff = rststat;          //保存复位源
            //清除复位源寄存器
            checkrstflag = 0;               //等待下次复位进入
            goto RECORD;
        }	        
    }

    chg_buf = EventRecordChargeCheck();//收集需判断的数据
    if((chg_buf^gRecChgBuff)&(chg_buf&0x2))//充电器接入或拔除，充电由不满到满
    {
    	gRecType |= REC_TYPE_CHG;
    }
    gRecChgBuff = chg_buf;

    //光敏电阻由200ms滤波后标志判断
    if((gBatteryInfo.Status.IOStatus & REMOVE_CHECK_SHIFT))
    {
        if(gRecPhotoBuff==0)
        {
            gRecPhotoBuff = 1;
            if((gUserID.lt.sta&0x01)==0)
            {
                gUserIDBuff.lt.sta |= 0x01;//记录历史数据
                gStorageCTRL |= FLASH_WR_UD_MASK;
            }
            gRecType |= REC_TYPE_PHOTO;
        }
    }
    else
    {
        gRecPhotoBuff = 0;
    }

    //动作事件记录
    if(gRecCtrlBuff != gMBCtrl.ctrl)
    {
        gRecCtrlBuff = gMBCtrl.ctrl;
        gRecInfoType = REC_INFO_TYPE_CTRL;
        gRecType |= REC_TYPE_INFO;
    }
    
    //开关状态变化
    else if(gRecStateBuff != EventRecordGetIOStatus())
    {
        gRecStateBuff = EventRecordGetIOStatus();
        gRecInfoType = REC_INFO_TYPE_STATE;
        gRecType |= REC_TYPE_INFO;
    }

    RECORD: 
    //有事件发生
    if(gRecType & 0xff && !(gStorageCTRL & FLASH_WR_EVE_MASK))
    {
        for(gRecIndex = 0;gRecIndex < 8;gRecIndex++)
        {
            if(gRecType & (1 << gRecIndex))
            {
                gRecordWr.type = (1 << gRecIndex);

                for(i = 0;i < 56;i++)
                {
                    if(cMBReadonlyDataRdFunc[i](256+i,&databuf))
                    {
                        gRecordWr.d[i << 1] = databuf >> 8;
                        gRecordWr.d[(i << 1)+1] = databuf & 0xff;
                    }
                    else
                    {
                        gRecordWr.d[i << 1] = 0xff;
                        gRecordWr.d[(i << 1)+1] = 0xff;
                    }
                }
                //保存ctrl寄存器数据
                gRecordWr.d[112] = gMBCtrl.ctrl >> 8;
                gRecordWr.d[113] = gMBCtrl.ctrl & 0xff;

                //如果记录为复位事件则将STATE寄存器替换为复位源标志
                if(gRecordWr.type == REC_TYPE_RESET)
                {
                    gRecordWr.d[8] = gRecRSTBuff >> 8;
                    gRecordWr.d[9] = gRecRSTBuff & 0xff;
                }
                //关机事件
                if(gRecordWr.type == REC_TYPE_SHDN)
                {
                    if(gSHDNCond.cond & SHDN_COND_COMM_MASK)
                    {
                        gRecordWr.d[7] = 0x01;
                    }
                    else if(gSHDNCond.cond & SHDN_COND_UV4_MASK)
                    {
                        gRecordWr.d[7] = 0x02;
                    }
                    else if(gSHDNCond.cond & SHDN_COND_SHDNCMD_MASK)
                    {
                        gRecordWr.d[7] = 0x03;
                    }
                    else if(gSHDNCond.cond & SHDN_COND_RESET)
                    {
                        gRecordWr.d[7] = 0x04;
                    }
                    else
                    {
                        gRecordWr.d[7] = 0xff;
                    }

                }
                //动作事件记录
                if(gRecordWr.type == REC_TYPE_INFO)
                {
                    gRecordWr.d[7] = gRecInfoType;
                }

                gStorageCTRL |= FLASH_WR_EVE_MASK;
                gRecType &= ~(1<<gRecIndex);
                BSPTaskStart(TASK_ID_STORAGE_TASK, 5);
                break;
            }
        }
    }
}

/*****************************************end of EventRecord.c*****************************************/
