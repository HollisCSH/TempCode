//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SocPointData.h
//创建人  	: Handry
//创建日期	:
//描述	    : SOC 点数据头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _SOCPOINTDATA_H
#define _SOCPOINTDATA_H

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTypedef.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#if defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S) || defined(SY_PB_32000MAH_17S)
#define     SOC_T_LEN               4               //SOC OCV修正温度表长度
#define     SOC_V_LEN               19              //SOC OCV修正电压表长度

#define     SOC_POINT_CRT_NUM       1               //soc点修正点数目

#elif defined(LFP_TB_20000MAH_20S)
#define     SOC_T_LEN               4               //SOC OCV修正温度表长度
#define     SOC_V_LEN               13              //SOC OCV修正电压表长度

#define     SOC_POINT_CRT_NUM       5               //soc点修正点数目

#elif defined(LFP_HL_25000MAH_16S) || defined(LFP_PH_20000MAH_20S) || defined(LFP_GF_25000MAH_16S)
#define     SOC_T_LEN               4               //SOC OCV修正温度表长度
#define     SOC_V_LEN               21              //SOC OCV修正电压表长度

#define     SOC_POINT_CRT_NUM       5               //soc点修正点数目

#else
#endif

//=============================================================================================
//定义数据类型
//=============================================================================================
//不同温度对应的OCV-SOC表
typedef struct
{
    s16 temp;
    const u16 *OCVTab;
    const u8 *SOCTab;
}t_OCVPoint;

//SOC修正点结构定义
typedef struct												
{
    u16 VoltH;         //电压上限
    u16 VoltL;         //电压下限
    s16 CurrH;         //电流上限
    s16 CurrL;         //电流下限
    s16 TempH;         //温度上限
    s16 TempL;         //温度下限
    u16 CorSoc;        //修正点SOC
    u16 CorTime;       //SOC修正计算时间
}
t_SocPointCorr;

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern const t_OCVPoint cTempOCVTab[SOC_T_LEN]; //不同温度对应的OCV-SOC表

extern const t_SocPointCorr cSocPointTbl[SOC_POINT_CRT_NUM];    //SOC修正点

#endif

/*****************************************end of SocPointData.h*****************************************/
