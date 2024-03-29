//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: NTC.h
//创建人  	: Handry
//创建日期	: 
//描述	    : NTC头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _NTC_H_
#define _NTC_H_

//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"

//=============================================================================
//数据结构声明
//=============================================================================

//NTC结构体声明
typedef struct
{
    const unsigned long *tab;
    unsigned short tablen;//表数据长度-1
    short tabofs; //0.1C/bit
}t_NTCTAB;

//=============================================================================================
//宏参数设定
//=============================================================================================
#define 	NTC_TEMP_OFF 		0x7FFE
#define 	NTC_TEMP_SHORT 		0x7FFD

#define 	NTC_REFV 			3300000 			//uV
#define 	NTC_REFV_LSB 		(NTC_REFV/382) 		//382:uV/LSB

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern const t_NTCTAB NTC_RESTAB[] ;	//NTC查找列表

//=============================================================================================
//声明接口函数
//=============================================================================================
//=========================================================================================================
//函数名称:	short NTCTempQuery(unsigned long volt,const unsigned long *table,unsigned short n,short offset)
//输入参数:	volt:NTCA采样数据
//		    table:NTC data-temperature 表
//			n: NTC表长度-1
//			offset:温度偏移0.1°每bit。
//输出参数: short: NTC温度值0.1°每bit。
//功能描述: 依据NTC电压值，通过折半法查询对应的温度
//注意事项:
//=========================================================================================================
short NTCTempQuery(unsigned long volt,const unsigned long *table,unsigned short n,short offset);

//=========================================================================================================
//函数名称:	unsigned long NTCFindResByTemp(s16 temp)
//输入参数:	temp：寻找的温度；0.1℃
//输出参数: 
//功能描述: 依据温度查找的对应的NTC阻值
//注意事项:
//=========================================================================================================
unsigned long NTCFindResByTemp(s16 temp);

#endif

/*****************************************end of NTC.h*****************************************/
