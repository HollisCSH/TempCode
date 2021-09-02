//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Sample.c
//创建人  	: Handry
//创建人  	: 
//创建日期	: 
//描述	    : 采样任务源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    : 
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "Sample.h"
//#include "BSPADC.h"
//#include "BSPGPIO.h"

#include "BSP_ADC.h"
#include "BSP_GPIO.h"
#include "BatteryInfo.h"
#include "NTC.h"
#include "IOCheck.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "BSPSystemCfg.h"
#include "Storage.h"
#include "ParaCfg.h"
#include "DataDeal.h"

//=============================================================================================
//全局变量
//=============================================================================================
t_SampleData gSampleData;   //采样的数据

static u16 sADSampData[ADC_SAMP_NUM] =       //AD采样值
{
	0,		//TVS温度 AD采样值
	0,		//PCB温度 AD采样值 
	0,		//3.3V AD采样值        
	0,		//预放电短路检测 AD采样值
    0,      //预放电电阻温度 AD采样值
//	0,		//VREFH
//	0		//VREFL
};

static const u8 sADChanNum[ADC_SAMP_NUM] = 		 //各路AD采样对应的通道号
{
	TVS_TEMP_ADC_CHAN,		    //TVS温度采样通道 
	PCB_TEMP_ADC_CHAN,		    //PCB温度采样通道     
	MCU_3V3_ADC_CHAN,			//3.3V AD采样值
	PRE_DSG_SHORT_ADC_CHAN,		//预放电短路检测采样通道
    PRE_DSG_TEMP_ADC_CHAN,      
//	0x1D,
//	0x1E
};
//=============================================================================================
//静态函数声明
//=============================================================================================

//=============================================================================================
//接口函数定义
//=============================================================================================

//=============================================================================================
//函数名称	: void SampleInit(void)
//输入参数	: void  
//输出参数	: void
//静态变量	: void
//功	能	: 采样任务初始化 
//注	意	:  
//=============================================================================================
void SampleInit(void)
{
	gSampleData.ConnTemp = 0;		//连接器温度
	gSampleData.PCBTemp = 0;		//PCB板温度
	gSampleData.TVSTemp = 0;		//TVS管温度
	gSampleData.PreVolt = 0;		//预放电管短路电压

	gSampleData.MCU3v3 = 0;		//MCU 3.3V电压实际采样值
}

//=============================================================================================
//函数名称	: static u16 SampleNTCCalculation(u16 volt,u16 pow)
//输入参数	: volt：采样的ntc电压；  pow：加在ntc上的3.3v电压实际值
//输出参数	: void
//静态变量	: void
//功	能	: 根据ntc的电压，计算对应的温度
//注	意	:  
//=============================================================================================
static u16 SampleNTCCalculation(u16 volt,u16 pow)
{
    u32 res ;

    if((pow - volt) > 0)  //用分压法，求实际的电阻值
    {
        res = (u32)volt * 10000/ ( pow - volt);
    }
    else
    {
        return NTC_TEMP_OFF;
    }
    
    return NTCTempQuery(res , NTC_RESTAB[0].tab, NTC_RESTAB[0].tablen, NTC_RESTAB[0].tabofs);   //根据电阻值查表得到温度值
}

//=============================================================================================
//函数名称	: void SampleMainTask(void *p)
//输入参数	: void  
//输出参数	: void
//静态变量	: void
//功	能	: 采样主任务
//注	意	:  
//=============================================================================================
void SampleMainTask(void *p)
{
//	u16 connresvolt = 0;
	u16 pcbresvolt = 0;
	u16 tvsresvolt = 0;
	u16 preresvolt = 0;
    
	static u8 step = 0;
	static u8 sChannel = 0;                     //采样通道
	static u16 sErrTime = 0;                    //异常次数
	static u8 sADNum = 0;						//AD数据数组索引
	static u16 sSampData[AD_FILTER_NUM] = {0};  //采样数据
    
	switch(step) 
    {
		case 0:
            
            #ifdef BMS_ENABLE_LITTLE_CURRENT_DET
			if(0 == BSPADCGetWorkState() && False == gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampIng)	//空闲
            #else
            if(0 == BSPADCGetWorkState())	//空闲
            #endif
			{
                //开启PCB TVS采样
                PCB_TVS_NTC_SAMP_ON;
                //BSPGPIOSetDir(PCB_NTC_CTR_PORT,PCB_NTC_CTR_PIN,INPUT);
                gSampleData.SampFlag.SampFlagBit.IsNTCSampIng = 1; //正在采样
                BSPTaskStart(TASK_ID_SAMPLE_TASK, 10);
                step = 1;
			}
			else
			{
                sErrTime++;
                if(sErrTime > 10)
                {
                	sErrTime = 0;
                    ;	//错误处理
                }
                BSPTaskStart(TASK_ID_SAMPLE_TASK, 10);
			}            

			break;    
            
		case 1:
            BSPADCStartSample(sADChanNum[sChannel]);
            step = 2;
            BSPTaskStart(TASK_ID_SAMPLE_TASK, 5);	//等待采样完毕
			break;    
 
		case 2:
			//AD值，获取5个ad值约300ms
			sSampData[sADNum] = ADCGetSampleData(sADChanNum[sChannel]);
			sADNum++;
            if(sADNum >= AD_FILTER_NUM)		//达到滤波数目
            {
            	//滤波
            	sADSampData[sChannel] = BSPADCDataAvgFilter(sSampData,AD_FILTER_NUM);

                //切换下一采样通道
                sADNum = 0;
                sChannel++;
                if(sChannel == 2)   //完成PCB和TVS温度采样
                {
                    step = 3;	//转到MCU 3.3
                    BSPTaskStart(TASK_ID_SAMPLE_TASK, 2);
                }
                else if(sChannel == 3)         //完成MCU3.3
                {
                    step = 4;	//预放电电流和预放电温度
                    BSPTaskStart(TASK_ID_SAMPLE_TASK, 2);                    
                }
                else if(sChannel >= ADC_SAMP_NUM)
                {
                	sChannel = 0;
                    step = 5;	//进入AD数据转换
                    BSPTaskStart(TASK_ID_SAMPLE_TASK, 2);
                }
                else //0 - 1
                {
                    step = 1;	//进入下一轮AD采样
                    BSPTaskStart(TASK_ID_SAMPLE_TASK, 2);
                }

            }
            else
            {
            	step = 1;
            	BSPTaskStart(TASK_ID_SAMPLE_TASK, 2);
            }

			break;
            
		case 3:
            //拉高
            PCB_TVS_NTC_SAMP_OFF;
			BSPTaskStart(TASK_ID_SAMPLE_TASK, 10);
			step = 1;
			break;
        
		case 4:
			PRE_DSG_NTC_SAMP_ON;		//开启预放电 NTC采样控制
			BSPTaskStart(TASK_ID_SAMPLE_TASK, 10);
			step = 1;
			break; 
        
		case 5:
			//测量出3.3V的实际电压值
			gSampleData.MCU3v3 = BSPADCCalADtoVolt(sADSampData[2]);                 
        
            //测 TVS管温度
            tvsresvolt = BSPADCCalADtoVolt(sADSampData[0]);
            gSampleData.TVSTemp = SampleNTCCalculation(tvsresvolt,gSampleData.MCU3v3);  
        
			//测量出预放电ADC电压，判断是否大于1V
			gSampleData.PreVolt = BSPADCCalADtoVolt(sADSampData[3]);
            //printf("ADC,Data[1][%d],Data[4][%d],time[%d]",sADSampData[1],sADSampData[4],HAL_GetTick()); //检测完成
            //测PCB温度
            pcbresvolt = BSPADCCalADtoVolt(sADSampData[1]);
            gSampleData.PCBTemp = SampleNTCCalculation(pcbresvolt,gSampleData.MCU3v3); 
        
            //测预放电电阻温度
            preresvolt = BSPADCCalADtoVolt(sADSampData[4]);
            gSampleData.PreTemp = SampleNTCCalculation(preresvolt,gSampleData.MCU3v3); 
        
            //BSPGPIOSetDir(PCB_NTC_CTR_PORT,PCB_NTC_CTR_PIN,OUTPUT);
            //BSPGPIOSetDir(TVS_NTC_CTR_PORT,TVS_NTC_CTR_PIN,OUTPUT);
			//PCB_NTC_SAMP_OFF;		//关闭TVS NTC采样
			//TVS_NTC_SAMP_OFF;		//关闭PCB NTC采样
            PCB_TVS_NTC_SAMP_OFF;
            PRE_DSG_NTC_SAMP_OFF;   //关闭预放电电阻温度采样
            
			gSampleData.SampFlag.SampFlagBit.IsNTCSampIng = 0; 	//不采样
            gSampleData.SampFlag.SampFlagBit.IsPreSampOk = True;    //预放电电流采样完成
			step = 0;			//循环结束，下一个循环

			DataDealSetBMSDataRdy(e_SampleRdy);
			BSPTaskStart(TASK_ID_SAMPLE_TASK, 2);//暂时开启
			break;

		default:
			step = 0;
			break;
	}    

}

//小电流检测功能
#ifdef BMS_ENABLE_LITTLE_CURRENT_DET
//=============================================================================================
//函数名称	: void SampleLowCurrentTask(void *p)
//输入参数	: void  
//输出参数	: void
//静态变量	: void
//功	能	: 小电流采样主任务
//注	意	:  
//=============================================================================================
void SampleLowCurrentTask(void *p)
{
    #define LOW_CURR_AD_SAMP_NUM  2
    #define LOW_CURR_AD_FILTER_NUM 24    
    #define LOW_CURR_AD_P1_CHAN   0     //后续补充
    #define LOW_CURR_AD_P2_CHAN   0

	static u8 step = 0;
	static u8 sChannel = 0;                     //采样通道
	static u16 sErrTime = 0;                    //异常次数
	static u8 sADNum = 0;						//AD数据数组索引
	static u16 sSampData[LOW_CURR_AD_FILTER_NUM] = {0};  //采样数据
    static u16 sLowCurrADValue[LOW_CURR_AD_SAMP_NUM] = {0};
    
    //小电流AD采样对应的通道号
    static const u8 sADLowCurrChanNum[LOW_CURR_AD_SAMP_NUM] =   
    {
        LOW_CURR_AD_P1_CHAN,
        LOW_CURR_AD_P2_CHAN
    };
    
    if(True == gSampleData.LowCurrFlag.LowCurrFlagBit.IsInitAll)
    {
        gSampleData.LowCurrFlag.LowCurrFlagBit.IsInitAll = False;
        step = 0;
        sADNum = 0;
        sErrTime = 0;
        sChannel = 0;
        
        gSampleData.P1LowCurrVal = 0;
        gSampleData.P2LowCurrVal = 0;
        gSampleData.P1ADBaseVolt = 0;
        gSampleData.P2ADBaseVolt = 0;
        gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampOK = False; 	    //采样完成标志清除 
        gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampBaseOK = False; 	//基准采样完成标志清除 
    }

	switch(step) 
    {
		case 0:
			if(0 == BSPADCGetWorkState() && False == gSampleData.SampFlag.SampFlagBit.IsNTCSampIng)	//空闲
			{
                gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampIng = True; //正在采样
                LOW_CURRENT0_3V3_ON();
                LOW_CURRENT1_3V3_ON();
                
                if(True == gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampOK 
                    || True == gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampBaseOK) //正在持续采样
                    BSPTaskStart(TASK_ID_LOW_CURR_SAM_TASK, 5);
                else
                    BSPTaskStart(TASK_ID_LOW_CURR_SAM_TASK, 50);
                
                step = 1;
			}
			else
			{
                sErrTime++;
                if(sErrTime > 10)
                {
                	sErrTime = 0;
                    ;	//错误处理
                }
                BSPTaskStart(TASK_ID_LOW_CURR_SAM_TASK, 10);
			}
            break;
            
        case 1:
            BSPADCStartSample(sADLowCurrChanNum[sChannel]);
            step = 2;
            BSPTaskStart(TASK_ID_LOW_CURR_SAM_TASK, 2);	//等待采样完毕            
            break;
        
        case 2:
			//AD值，获取5个ad值约300ms
			sSampData[sADNum] = ADCGetSampleData(sADLowCurrChanNum[sChannel]);
			sADNum++;
            if(sADNum >= LOW_CURR_AD_FILTER_NUM)		//达到滤波数目
            {
            	//滤波
            	sLowCurrADValue[sChannel] = BSPADCDataAvgFilter(sSampData,LOW_CURR_AD_FILTER_NUM);
                //sLowCurrADValue[sChannel] += BSPADCDataMedianNumFilter(sSampData,LOW_CURR_AD_FILTER_NUM);
                //sLowCurrADValue[sChannel] = sLowCurrADValue[sChannel] / 2;
                
                //切换下一采样通道
                sADNum = 0;
                sChannel++;
                if(sChannel >= LOW_CURR_AD_SAMP_NUM)   //完成P1 P2采样
                {
                    sChannel = 0;
                    step = 3;	//开始转换采样电压值
                    BSPTaskStart(TASK_ID_LOW_CURR_SAM_TASK, 2);
                }
                else
                {
                    step = 1;	//进入下一轮AD采样
                    BSPTaskStart(TASK_ID_LOW_CURR_SAM_TASK, 2);
                }
            }
            else
            {
            	step = 1;
            	BSPTaskStart(TASK_ID_LOW_CURR_SAM_TASK, 2);
            }            
            break;
            
        case 3:
            //gSampleData.P1ADVolt = BSPADCCalADtoRealVolt(sLowCurrADValue[0],gSampleData.MCU3v3);
            //gSampleData.P2ADVolt = BSPADCCalADtoRealVolt(sLowCurrADValue[1],gSampleData.MCU3v3);  
        
			//测量出运放上实际的AD值
            if(False == gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampBaseOK)
            {
                //获取到基准值
                gSampleData.P1ADBaseVolt = BSPADCCalADtoVolt(sLowCurrADValue[0]);     //后续补充
                gSampleData.P2ADBaseVolt =  BSPADCCalADtoVolt(sLowCurrADValue[1]);     
                gSampleData.P1ADBaseVolt = 1000;
                gSampleData.P2ADBaseVolt = 1000;
                gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampBaseOK = True;
                //等待开预放电管再继续进行采样
            }
            else
            {
                //计算当前的漏电流
                gSampleData.P1LowCurrVal = BSPADCCalADtoVolt(sLowCurrADValue[0]) - gSampleData.P1ADBaseVolt;     //后续补充
                gSampleData.P2LowCurrVal = BSPADCCalADtoVolt(sLowCurrADValue[1]) - gSampleData.P2ADBaseVolt;  
                gSampleData.P1LowCurrVal = 0;
                gSampleData.P2LowCurrVal = 0;                
                gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampOK = True; 	//采样结束
                
                BSPTaskStart(TASK_ID_LOW_CURR_SAM_TASK, 50);
            }
            
			gSampleData.LowCurrFlag.LowCurrFlagBit.IsSampIng = False; 	//不采样
			step = 0;			//循环结束，下一个循环
        
            break;
        
        default:
            step = 0;
            break;
    }
}

#endif

/*****************************************end of Sample.c*****************************************/
