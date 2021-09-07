//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SocPointData.c
//创建人  	: Handry
//创建日期	:
//描述	    : SOC 点数据
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 不同的电芯对应不同的点数据
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SocPointData.h"

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
#if defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S) || defined(SY_PB_32000MAH_17S)
//OCV表
/*********************** < -10(不包含)℃ ******************************/
const u16 SocOCVVoltTab1[SOC_V_LEN] =      //SOC OCV表
{
	3440,			//电压表SOC0对应静置电压 [1000,5000]mV
	3470,			//电压表SOC1对应静置电压 
	3514,			//电压表SOC2对应静置电压 
	3550,			//电压表SOC3对应静置电压 
	3579,			//电压表SOC4对应静置电压 
	3600,			//电压表SOC5对应静置电压 
	3615,			//电压表SOC6对应静置电压 
	3629,			//电压表SOC7对应静置电压 
	3644,			//电压表SOC8对应静置电压 
	3663,			//电压表SOC9对应静置电压 
	3690,			//电压表SOC10对应静置电压 
	3733,			//电压表SOC11对应静置电压 
	3792,			//电压表SOC12对应静置电压 
	3844,			//电压表SOC13对应静置电压 
	3895,			//电压表SOC14对应静置电压 
	3949,			//电压表SOC15对应静置电压 
	4005,			//电压表SOC16对应静置电压 
	4065,			//电压表SOC17对应静置电压 
	4100,			//电压表SOC18对应静置电压 
};

const u8 SocOCVSOCTab1[SOC_V_LEN] =         //SOC OCV SOC表
{
	0  ,			//SOC区间表SOC0
	6  ,			//SOC区间表SOC1
	11 ,			//SOC区间表SOC2
	17 ,			//SOC区间表SOC3
	23 ,			//SOC区间表SOC4
	29 ,			//SOC区间表SOC5
	34 ,			//SOC区间表SOC6
	40 ,			//SOC区间表SOC7
	46 ,			//SOC区间表SOC8
	52 ,			//SOC区间表SOC9
	57 ,			//SOC区间表SOC10
	63 ,			//SOC区间表SOC11
	69 ,			//SOC区间表SOC12
	75 ,			//SOC区间表SOC13
	80 ,			//SOC区间表SOC14
	86 ,			//SOC区间表SOC15
	92 ,			//SOC区间表SOC16
	98 ,			//SOC区间表SOC17
	100,			//SOC区间表SOC18
};


/*********************** -10 - 10(不包含)℃ ******************************/
const u16 SocOCVVoltTab2[SOC_V_LEN] =      //SOC OCV表
{
	3440,			//电压表SOC0对应静置电压 [1000,5000]mV
	3471,			//电压表SOC1对应静置电压 
	3514,			//电压表SOC2对应静置电压 
	3550,			//电压表SOC3对应静置电压 
	3583,			//电压表SOC4对应静置电压 
	3602,			//电压表SOC5对应静置电压 
	3617,			//电压表SOC6对应静置电压 
	3631,			//电压表SOC7对应静置电压 
	3647,			//电压表SOC8对应静置电压 
	3666,			//电压表SOC9对应静置电压 
	3693,			//电压表SOC10对应静置电压 
	3738,			//电压表SOC11对应静置电压 
	3796,			//电压表SOC12对应静置电压 
	3845,			//电压表SOC13对应静置电压 
	3896,			//电压表SOC14对应静置电压 
	3950,			//电压表SOC15对应静置电压 
	4006,			//电压表SOC16对应静置电压 
	4066,			//电压表SOC17对应静置电压 
	4100,			//电压表SOC18对应静置电压 
};

const u8 SocOCVSOCTab2[SOC_V_LEN] =         //SOC OCV SOC表
{
	0  ,			//SOC区间表SOC0
	6  ,			//SOC区间表SOC1
	11 ,			//SOC区间表SOC2
	17 ,			//SOC区间表SOC3
	23 ,			//SOC区间表SOC4
	29 ,			//SOC区间表SOC5
	34 ,			//SOC区间表SOC6
	40 ,			//SOC区间表SOC7
	46 ,			//SOC区间表SOC8
	52 ,			//SOC区间表SOC9
	57 ,			//SOC区间表SOC10
	63 ,			//SOC区间表SOC11
	69 ,			//SOC区间表SOC12
	75 ,			//SOC区间表SOC13
	80 ,			//SOC区间表SOC14
	86 ,			//SOC区间表SOC15
	92 ,			//SOC区间表SOC16
	98 ,			//SOC区间表SOC17
	100,			//SOC区间表SOC18
};

/*********************** 10 - 45(不包含)℃ ******************************/
const u16 SocOCVVoltTab3[SOC_V_LEN] =      //SOC OCV表
{
	3399,			//电压表SOC0对应静置电压 [1000,5000]mV
	3453,			//电压表SOC1对应静置电压 
	3495,			//电压表SOC2对应静置电压 
	3535,			//电压表SOC3对应静置电压 
	3568,			//电压表SOC4对应静置电压 
	3598,			//电压表SOC5对应静置电压 
	3616,			//电压表SOC6对应静置电压 
	3631,			//电压表SOC7对应静置电压 
	3647,			//电压表SOC8对应静置电压 
	3666,			//电压表SOC9对应静置电压 
	3691,			//电压表SOC10对应静置电压 
	3732,			//电压表SOC11对应静置电压 
	3789,			//电压表SOC12对应静置电压 
	3837,			//电压表SOC13对应静置电压 
	3888,			//电压表SOC14对应静置电压 
	3942,			//电压表SOC15对应静置电压 
	3999,			//电压表SOC16对应静置电压 
	4060,			//电压表SOC17对应静置电压 
	4100,			//电压表SOC18对应静置电压
};

const u8 SocOCVSOCTab3[SOC_V_LEN] =         //SOC OCV SOC表
{
	0  ,			//SOC区间表SOC0
	6  ,			//SOC区间表SOC1
	11 ,			//SOC区间表SOC2
	17 ,			//SOC区间表SOC3
	23 ,			//SOC区间表SOC4
	29 ,			//SOC区间表SOC5
	34 ,			//SOC区间表SOC6
	40 ,			//SOC区间表SOC7
	46 ,			//SOC区间表SOC8
	52 ,			//SOC区间表SOC9
	57 ,			//SOC区间表SOC10
	63 ,			//SOC区间表SOC11
	69 ,			//SOC区间表SOC12
	75 ,			//SOC区间表SOC13
	80 ,			//SOC区间表SOC14
	86 ,			//SOC区间表SOC15
	92 ,			//SOC区间表SOC16
	98 ,			//SOC区间表SOC17
	100,			//SOC区间表SOC18
};

/*********************** > 45(不包含)℃ ******************************/
const u16 SocOCVVoltTab4[SOC_V_LEN] =      //SOC OCV表
{
	3421,			//电压表SOC0对应静置电压 [1000,5000]mV
	3458,			//电压表SOC1对应静置电压 
	3499,			//电压表SOC2对应静置电压 
	3537,			//电压表SOC3对应静置电压 
	3568,			//电压表SOC4对应静置电压 
	3600,			//电压表SOC5对应静置电压 
	3620,			//电压表SOC6对应静置电压 
	3635,			//电压表SOC7对应静置电压 
	3651,			//电压表SOC8对应静置电压 
	3670,			//电压表SOC9对应静置电压 
	3696,			//电压表SOC10对应静置电压 
	3739,			//电压表SOC11对应静置电压 
	3794,			//电压表SOC12对应静置电压 
	3843,			//电压表SOC13对应静置电压 
	3894,			//电压表SOC14对应静置电压 
	3948,			//电压表SOC15对应静置电压 
	4004,			//电压表SOC16对应静置电压 
	4063,			//电压表SOC17对应静置电压 
	4100,			//电压表SOC18对应静置电压
};

const u8 SocOCVSOCTab4[SOC_V_LEN] =         //SOC OCV SOC表
{
	0  ,			//SOC区间表SOC0
	6  ,			//SOC区间表SOC1
	11 ,			//SOC区间表SOC2
	17 ,			//SOC区间表SOC3
	23 ,			//SOC区间表SOC4
	29 ,			//SOC区间表SOC5
	34 ,			//SOC区间表SOC6
	40 ,			//SOC区间表SOC7
	46 ,			//SOC区间表SOC8
	52 ,			//SOC区间表SOC9
	57 ,			//SOC区间表SOC10
	63 ,			//SOC区间表SOC11
	69 ,			//SOC区间表SOC12
	75 ,			//SOC区间表SOC13
	80 ,			//SOC区间表SOC14
	86 ,			//SOC区间表SOC15
	92 ,			//SOC区间表SOC16
	98 ,			//SOC区间表SOC17
	100,			//SOC区间表SOC18
};

//不同温度对应的OCV-SOC表
const t_OCVPoint cTempOCVTab[SOC_T_LEN] = 
{
    {-100,SocOCVVoltTab1,SocOCVSOCTab1},
    {100,SocOCVVoltTab2,SocOCVSOCTab2},    
    {450,SocOCVVoltTab3,SocOCVSOCTab3},
    {600,SocOCVVoltTab4,SocOCVSOCTab4},
};

//充放电点修正
//SOC修正点
const t_SocPointCorr cSocPointTbl[SOC_POINT_CRT_NUM] =
{
	{
        4100,			//修正点电压上限[0,4500]mV
        4050,			//修正点电压下限[0,4500]mV
        900,			//修正点电流上限[-6000,4000]0.01A
        700,			//修正点电流下限[-6000,4000]0.01A
        550,			//修正点温度上限[0,170]40为0°C
        50,			    //修正点温度下限[0,170]40为0°C
        95,			    //修正点SOC值[0,100]%
        10,			    //修正点检测时间 s
    },
};

#elif defined(LFP_TB_20000MAH_20S)
//OCV表
/*********************** < -10(不包含)℃ ******************************/
const u16 SocOCVVoltTab1[SOC_V_LEN] =      //SOC OCV表
{
	3279,			//电压表SOC0对应静置电压 [1000,5000]mV
	3284,			//电压表SOC1对应静置电压  
	3289,			//电压表SOC2对应静置电压  
	3295,			//电压表SOC3对应静置电压  
	3304,			//电压表SOC4对应静置电压  
	3313,			//电压表SOC5对应静置电压  
	3318,			//电压表SOC6对应静置电压  
	3321,			//电压表SOC7对应静置电压  
	3322,			//电压表SOC8对应静置电压  
	3323,			//电压表SOC9对应静置电压  
	3324,			//电压表SOC10对应静置电压  
	3326,			//电压表SOC11对应静置电压  
	3449,			//电压表SOC12对应静置电压
};

const u8 SocOCVSOCTab1[SOC_V_LEN] =         //SOC OCV SOC表
{
	0,			//SOC区间表SOC0
	5,			//SOC区间表SOC1
	10,			//SOC区间表SOC2
	15,			//SOC区间表SOC3
	20,			//SOC区间表SOC4
	25,			//SOC区间表SOC5
	30,			//SOC区间表SOC6
	35,			//SOC区间表SOC7
	40,			//SOC区间表SOC8
	60,			//SOC区间表SOC9
	75,			//SOC区间表SOC10
	95,			//SOC区间表SOC11
	100,		//SOC区间表SOC12
};


/*********************** -10 - 10(不包含)℃ ******************************/
const u16 SocOCVVoltTab2[SOC_V_LEN] =      //SOC OCV表
{
	3270,			//电压表SOC0对应静置电压 [1000,5000]mV
	3278,			//电压表SOC1对应静置电压   
	3279,			//电压表SOC2对应静置电压   
	3280,			//电压表SOC3对应静置电压   
	3283,			//电压表SOC4对应静置电压   
	3288,			//电压表SOC5对应静置电压   
	3299,			//电压表SOC6对应静置电压   
	3314,			//电压表SOC7对应静置电压   
	3320,			//电压表SOC8对应静置电压   
	3322,			//电压表SOC9对应静置电压   
	3323,			//电压表SOC10对应静置电压   
	3327,			//电压表SOC11对应静置电压   
	3392,			//电压表SOC12对应静置电压
};

const u8 SocOCVSOCTab2[SOC_V_LEN] =         //SOC OCV SOC表
{
	0,			//SOC区间表SOC0
	5,			//SOC区间表SOC1
	15,			//SOC区间表SOC2
	20,			//SOC区间表SOC3
	25,			//SOC区间表SOC4
	30,			//SOC区间表SOC5
	35,			//SOC区间表SOC6
	40,			//SOC区间表SOC7
	45,			//SOC区间表SOC8
	60,			//SOC区间表SOC9
	75,			//SOC区间表SOC10
	95,			//SOC区间表SOC11
	100,	    //SOC区间表SOC12
};

/***********************10 - 50℃ ******************************/
const u16 SocOCVVoltTab3[SOC_V_LEN] =      //SOC OCV表
{
	2831,			//电压表SOC0对应静置电压 [1000,5000]mV
	3198,			//电压表SOC1对应静置电压    
	3234,			//电压表SOC2对应静置电压    
	3256,			//电压表SOC3对应静置电压    
	3275,			//电压表SOC4对应静置电压    
	3285,			//电压表SOC5对应静置电压    
	3288,			//电压表SOC6对应静置电压    
	3301,			//电压表SOC7对应静置电压    
	3324,			//电压表SOC8对应静置电压    
	3326,			//电压表SOC9对应静置电压    
	3327,			//电压表SOC10对应静置电压    
	3331,			//电压表SOC11对应静置电压    
	3448,			//电压表SOC12对应静置电压
};

const u8 SocOCVSOCTab3[SOC_V_LEN] =         //SOC OCV SOC表
{
	0,			//SOC区间表SOC0
	5,			//SOC区间表SOC1
	15,			//SOC区间表SOC2
	20,			//SOC区间表SOC3
	25,			//SOC区间表SOC4
	30,			//SOC区间表SOC5
	45,			//SOC区间表SOC6
	55,			//SOC区间表SOC7
	60,			//SOC区间表SOC8
	70,			//SOC区间表SOC9
	85,			//SOC区间表SOC10
	95,			//SOC区间表SOC11
	100,		//SOC区间表SOC12
};

/*********************** > 50℃ ******************************/
const u16 SocOCVVoltTab4[SOC_V_LEN] =      //SOC OCV表
{
	2699,			//电压表SOC0对应静置电压 [1000,5000]mV
	3084,			//电压表SOC1对应静置电压    
	3201,			//电压表SOC2对应静置电压    
	3214,			//电压表SOC3对应静置电压    
	3243,			//电压表SOC4对应静置电压    
	3260,			//电压表SOC5对应静置电压    
	3279,			//电压表SOC6对应静置电压    
	3296,			//电压表SOC7对应静置电压    
	3300,			//电压表SOC8对应静置电压    
	3325,			//电压表SOC9对应静置电压    
	3329,			//电压表SOC10对应静置电压    
	3331,			//电压表SOC11对应静置电压    
	3406,			//电压表SOC12对应静置电压
};

const u8 SocOCVSOCTab4[SOC_V_LEN] =         //SOC OCV SOC表
{
	0,			//SOC区间表SOC0
	5,			//SOC区间表SOC1
	10,			//SOC区间表SOC2
	15,			//SOC区间表SOC3
	20,			//SOC区间表SOC4
	25,			//SOC区间表SOC5
	30,			//SOC区间表SOC6
	45,			//SOC区间表SOC7
	55,			//SOC区间表SOC8
	60,			//SOC区间表SOC9
	70,			//SOC区间表SOC10
	90,			//SOC区间表SOC11
	100,	    //SOC区间表SOC12
};

//不同温度对应的OCV-SOC表
const t_OCVPoint cTempOCVTab[SOC_T_LEN] = 
{
    {-100,SocOCVVoltTab1,SocOCVSOCTab1},
    {100,SocOCVVoltTab2,SocOCVSOCTab2},    
    {500,SocOCVVoltTab3,SocOCVSOCTab3},
    {600,SocOCVVoltTab4,SocOCVSOCTab4},
};

//充放电点修正
//SOC修正点
const t_SocPointCorr cSocPointTbl[SOC_POINT_CRT_NUM] =
{
    //充电
	{
        3550,			//修正点电压上限[0,4500]mV
        3500,			//修正点电压下限[0,4500]mV
        1000,			//修正点电流上限[-6000,4000]0.01A
        200,			//修正点电流下限[-6000,4000]0.01A
        550,			//修正点温度上限[0,170]40为0°C
        50,			    //修正点温度下限[0,170]40为0°C
        97,			    //修正点SOC值[0,100]%
        3,			    //修正点检测时间 s
    },
    {
        3500,			//修正点电压上限[0,4500]mV
        3450,			//修正点电压下限[0,4500]mV
        800,			//修正点电流上限[-6000,4000]0.01A
        500,			//修正点电流下限[-6000,4000]0.01A
        550,			//修正点温度上限[0,170]40为0°C
        50,			    //修正点温度下限[0,170]40为0°C
        94,			    //修正点SOC值[0,100]%
        5,			    //修正点检测时间 s
    },
    {
        3450,			//修正点电压上限[0,4500]mV
        3430,			//修正点电压下限[0,4500]mV
        1400,			//修正点电流上限[-6000,4000]0.01A
        1100,			//修正点电流下限[-6000,4000]0.01A
        550,			//修正点温度上限[0,170]40为0°C
        50,			    //修正点温度下限[0,170]40为0°C
        90,			    //修正点SOC值[0,100]%
        5,			    //修正点检测时间 s
    },
    //放电
	{    
        3100,			//修正点电压上限[0,4500]mV
        3080,			//修正点电压下限[0,4500]mV
        -1500,			//修正点电流上限[-6000,4000]0.01A
        -2000,			//修正点电流下限[-6000,4000]0.01A
        550,			//修正点温度上限[0,170]40为0°C
        50,		    	//修正点温度下限[0,170]40为0°C
        10,			    //修正点SOC值[0,100]%
        5,			    //修正点检测时间 s
    },
	{    
        3150,			//修正点电压上限[0,4500]mV
        3130,			//修正点电压下限[0,4500]mV
        -1500,			//修正点电流上限[-6000,4000]0.01A
        -2000,			//修正点电流下限[-6000,4000]0.01A
        550,			//修正点温度上限[0,170]40为0°C
        50,		    	//修正点温度下限[0,170]40为0°C
        20,			    //修正点SOC值[0,100]%
        5,			    //修正点检测时间 s
    },
};

#elif defined(LFP_HL_25000MAH_16S) || defined(LFP_PH_20000MAH_20S) || defined(LFP_GF_25000MAH_16S)
//OCV表
/*********************** < -10(不包含)℃ ******************************/
const u16 SocOCVVoltTab1[SOC_V_LEN] =      //SOC OCV表
{
	2000,			//电压表SOC0对应静置电压 [1000,5000]mV
	2417,			//电压表SOC1对应静置电压  
	2666,			//电压表SOC2对应静置电压  
	2870,			//电压表SOC3对应静置电压  
	2999,			//电压表SOC4对应静置电压  
	3072,			//电压表SOC5对应静置电压  
	3110,			//电压表SOC6对应静置电压  
	3140,			//电压表SOC7对应静置电压  
	3163,			//电压表SOC8对应静置电压  
	3176,			//电压表SOC9对应静置电压  
	3186,			//电压表SOC10对应静置电压  
	3195,			//电压表SOC11对应静置电压  
	3203,			//电压表SOC12对应静置电压  
	3211,			//电压表SOC13对应静置电压  
	3219,			//电压表SOC14对应静置电压  
	3226,			//电压表SOC15对应静置电压  
	3232,			//电压表SOC16对应静置电压  
	3238,			//电压表SOC17对应静置电压  
	3241,			//电压表SOC18对应静置电压  
	3244,			//电压表SOC19对应静置电压 
	3521,			//电压表SOC20对应静置电压 
};

const u8 SocOCVSOCTab1[SOC_V_LEN] =         //SOC OCV SOC表
{
	0  ,			//SOC区间表SOC0
	5  ,			//SOC区间表SOC1
	10 ,			//SOC区间表SOC2
	15 ,			//SOC区间表SOC3
	20 ,			//SOC区间表SOC4
	25 ,			//SOC区间表SOC5
	30 ,			//SOC区间表SOC6
	35 ,			//SOC区间表SOC7
	40 ,			//SOC区间表SOC8
	45 ,			//SOC区间表SOC9
	50 ,			//SOC区间表SOC10
	55 ,			//SOC区间表SOC11
	60 ,			//SOC区间表SOC12
	65 ,			//SOC区间表SOC13
	70 ,			//SOC区间表SOC14
	75 ,			//SOC区间表SOC15
	80 ,			//SOC区间表SOC16
	85 ,			//SOC区间表SOC17
	90 ,			//SOC区间表SOC18
	95 ,			//SOC区间表SOC19
	100,			//SOC区间表SOC20
};


/*********************** 0℃ ******************************/
const u16 SocOCVVoltTab2[SOC_V_LEN] =      //SOC OCV表
{
	2000,			//电压表SOC0对应静置电压 [1000,5000]mV
	2644,			//电压表SOC1对应静置电压   
	2965,			//电压表SOC2对应静置电压   
	3103,			//电压表SOC3对应静置电压   
	3158,			//电压表SOC4对应静置电压   
	3187,			//电压表SOC5对应静置电压   
	3203,			//电压表SOC6对应静置电压   
	3214,			//电压表SOC7对应静置电压   
	3221,			//电压表SOC8对应静置电压   
	3229,			//电压表SOC9对应静置电压   
	3234,			//电压表SOC10对应静置电压   
	3238,			//电压表SOC11对应静置电压   
	3244,			//电压表SOC12对应静置电压   
	3252,			//电压表SOC13对应静置电压   
	3261,			//电压表SOC14对应静置电压   
	3269,			//电压表SOC15对应静置电压   
	3275,			//电压表SOC16对应静置电压   
	3280,			//电压表SOC17对应静置电压   
	3282,			//电压表SOC18对应静置电压   
	3284,			//电压表SOC19对应静置电压  
	3476,			//电压表SOC20对应静置电压
};

const u8 SocOCVSOCTab2[SOC_V_LEN] =         //SOC OCV SOC表
{
	0  ,			//SOC区间表SOC0
	5  ,			//SOC区间表SOC1
	10 ,			//SOC区间表SOC2
	15 ,			//SOC区间表SOC3
	20 ,			//SOC区间表SOC4
	25 ,			//SOC区间表SOC5
	30 ,			//SOC区间表SOC6
	35 ,			//SOC区间表SOC7
	40 ,			//SOC区间表SOC8
	45 ,			//SOC区间表SOC9
	50 ,			//SOC区间表SOC10
	55 ,			//SOC区间表SOC11
	60 ,			//SOC区间表SOC12
	65 ,			//SOC区间表SOC13
	70 ,			//SOC区间表SOC14
	75 ,			//SOC区间表SOC15
	80 ,			//SOC区间表SOC16
	85 ,			//SOC区间表SOC17
	90 ,			//SOC区间表SOC18
	95 ,			//SOC区间表SOC19
	100,			//SOC区间表SOC20
};

/*********************** 20℃ ******************************/
const u16 SocOCVVoltTab3[SOC_V_LEN] =      //SOC OCV表
{
	1997,			//电压表SOC0对应静置电压 [1000,5000]mV
	3088,			//电压表SOC1对应静置电压    
	3173,			//电压表SOC2对应静置电压    
	3193,			//电压表SOC3对应静置电压    
	3218,			//电压表SOC4对应静置电压    
	3235,			//电压表SOC5对应静置电压    
	3251,			//电压表SOC6对应静置电压    
	3260,			//电压表SOC7对应静置电压    
	3265,			//电压表SOC8对应静置电压    
	3268,			//电压表SOC9对应静置电压    
	3269,			//电压表SOC10对应静置电压    
	3271,			//电压表SOC11对应静置电压    
	3275,			//电压表SOC12对应静置电压    
	3286,			//电压表SOC13对应静置电压
	3300,			//电压表SOC14对应静置电压    
	3308,			//电压表SOC15对应静置电压    
	3311,			//电压表SOC16对应静置电压    
	3313,			//电压表SOC17对应静置电压    
	3314,			//电压表SOC18对应静置电压    
	3315,			//电压表SOC19对应静置电压   
	3547,			//电压表SOC20对应静置电压
};

const u8 SocOCVSOCTab3[SOC_V_LEN] =         //SOC OCV SOC表
{
	0  ,			//SOC区间表SOC0
	5  ,			//SOC区间表SOC1
	10 ,			//SOC区间表SOC2
	15 ,			//SOC区间表SOC3
	20 ,			//SOC区间表SOC4
	25 ,			//SOC区间表SOC5
	30 ,			//SOC区间表SOC6
	35 ,			//SOC区间表SOC7
	40 ,			//SOC区间表SOC8
	45 ,			//SOC区间表SOC9
	50 ,			//SOC区间表SOC10
	55 ,			//SOC区间表SOC11
	60 ,			//SOC区间表SOC12
	65 ,			//SOC区间表SOC13
	70 ,			//SOC区间表SOC14
	75 ,			//SOC区间表SOC15
	80 ,			//SOC区间表SOC16
	85 ,			//SOC区间表SOC17
	90 ,			//SOC区间表SOC18
	95 ,			//SOC区间表SOC19
	100,			//SOC区间表SOC20
};


/*********************** 40℃ ******************************/
const u16 SocOCVVoltTab4[SOC_V_LEN] =      //SOC OCV表
{
	1997,			//电压表SOC0对应静置电压 [1000,5000]mV
	3097,			//电压表SOC1对应静置电压    
	3183,			//电压表SOC2对应静置电压    
	3202,			//电压表SOC3对应静置电压    
	3226,			//电压表SOC4对应静置电压    
	3242,			//电压表SOC5对应静置电压    
	3260,			//电压表SOC6对应静置电压    
	3274,			//电压表SOC7对应静置电压    
	3277,			//电压表SOC8对应静置电压    
	3279,			//电压表SOC9对应静置电压    
	3280,			//电压表SOC10对应静置电压    
	3283,			//电压表SOC11对应静置电压    
	3288,			//电压表SOC12对应静置电压    
	3307,			//电压表SOC13对应静置电压    
	3314,			//电压表SOC14对应静置电压    
	3316,			//电压表SOC15对应静置电压    
	3318,			//电压表SOC16对应静置电压    
	3319,			//电压表SOC17对应静置电压    
	3320,			//电压表SOC18对应静置电压    
	3322,			//电压表SOC19对应静置电压   
	3466,			//电压表SOC20对应静置电压
};

const u8 SocOCVSOCTab4[SOC_V_LEN] =         //SOC OCV SOC表
{
	0  ,			//SOC区间表SOC0
	5  ,			//SOC区间表SOC1
	10 ,			//SOC区间表SOC2
	15 ,			//SOC区间表SOC3
	20 ,			//SOC区间表SOC4
	25 ,			//SOC区间表SOC5
	30 ,			//SOC区间表SOC6
	35 ,			//SOC区间表SOC7
	40 ,			//SOC区间表SOC8
	45 ,			//SOC区间表SOC9
	50 ,			//SOC区间表SOC10
	55 ,			//SOC区间表SOC11
	60 ,			//SOC区间表SOC12
	65 ,			//SOC区间表SOC13
	70 ,			//SOC区间表SOC14
	75 ,			//SOC区间表SOC15
	80 ,			//SOC区间表SOC16
	85 ,			//SOC区间表SOC17
	90 ,			//SOC区间表SOC18
	95 ,			//SOC区间表SOC19
	100,			//SOC区间表SOC20
};

//不同温度对应的OCV-SOC表
const t_OCVPoint cTempOCVTab[SOC_T_LEN] = 
{
    {-100,SocOCVVoltTab1,SocOCVSOCTab1},
    {100,SocOCVVoltTab2,SocOCVSOCTab2},    
    {300,SocOCVVoltTab3,SocOCVSOCTab3},
    {450,SocOCVVoltTab4,SocOCVSOCTab4},
};

//充放电点修正
//SOC修正点
const t_SocPointCorr cSocPointTbl[SOC_POINT_CRT_NUM] =
{
    //充电
	{
        3550,			//修正点电压上限[0,4500]mV
        3500,			//修正点电压下限[0,4500]mV
        1000,			//修正点电流上限[-6000,4000]0.01A
        200,			//修正点电流下限[-6000,4000]0.01A
        550,			//修正点温度上限[0,170]40为0°C
        50,			    //修正点温度下限[0,170]40为0°C
        98,			    //修正点SOC值[0,100]%
        3,			    //修正点检测时间 s
    },
    {
        3430,			//修正点电压上限[0,4500]mV
        3400,			//修正点电压下限[0,4500]mV
        800,			//修正点电流上限[-6000,4000]0.01A
        600,			//修正点电流下限[-6000,4000]0.01A
        550,			//修正点温度上限[0,170]40为0°C
        50,			    //修正点温度下限[0,170]40为0°C
        95,			    //修正点SOC值[0,100]%
        5,			    //修正点检测时间 s
    },
    {
        3430,			//修正点电压上限[0,4500]mV
        3400,			//修正点电压下限[0,4500]mV
        1400,			//修正点电流上限[-6000,4000]0.01A
        1100,			//修正点电流下限[-6000,4000]0.01A
        550,			//修正点温度上限[0,170]40为0°C
        50,			    //修正点温度下限[0,170]40为0°C
        90,			    //修正点SOC值[0,100]%
        5,			    //修正点检测时间 s
    },
    //放电
	{    
        3100,			//修正点电压上限[0,4500]mV
        3080,			//修正点电压下限[0,4500]mV
        -1500,			//修正点电流上限[-6000,4000]0.01A
        -2000,			//修正点电流下限[-6000,4000]0.01A
        550,			//修正点温度上限[0,170]40为0°C
        50,		    	//修正点温度下限[0,170]40为0°C
        5,			    //修正点SOC值[0,100]%
        5,			    //修正点检测时间 s
    },
	{    
        3150,			//修正点电压上限[0,4500]mV
        3130,			//修正点电压下限[0,4500]mV
        -1500,			//修正点电流上限[-6000,4000]0.01A
        -2000,			//修正点电流下限[-6000,4000]0.01A
        550,			//修正点温度上限[0,170]40为0°C
        50,		    	//修正点温度下限[0,170]40为0°C
        16,			    //修正点SOC值[0,100]%
        5,			    //修正点检测时间 s
    },
};

#else

#endif

/*****************************************end of SocPointData.c*****************************************/

