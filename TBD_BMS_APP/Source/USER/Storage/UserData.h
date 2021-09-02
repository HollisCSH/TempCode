//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: UserData.h
//创建人  	: Handry
//创建日期	: 
//描述	    : 用户数据头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _USERDATA_H_
#define _USERDATA_H_
//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"
//=============================================================================================
//数据类型定义
//=============================================================================================
typedef struct
{
    u16 sta;
    u16 maxv;
    u16 maxvn;
    u16 minv;
    u16 minvn;
    u16 maxt;
    u16 maxtn;
    u16 mint;
    u16 mintn;
    u16 maxcc;
    u16 maxdc;
}t_LIFETIME;

typedef struct
{
    u16 flag;
    u16 user[16];
    t_LIFETIME lt;
    u16 crc16;
}t_USER_DATA;

//=============================================================================================
//宏参数设定
//=============================================================================================
//#define     REFRESH_USER_DATA_ONE_TIME  True

#ifdef REFRESH_USER_DATA_ONE_TIME
#define 	USER_CFG_FLAG 	0x1235
#else
#define 	USER_CFG_FLAG 	0x1237
#endif

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern t_USER_DATA gUserID;
extern const t_USER_DATA cUserIDInit;
extern t_USER_DATA gUserIDBuff;//保存配置的输入buffer

//=============================================================================================
//声明接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void UserDataInit(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 用户数据初始化函数
//注	意	:
//=============================================================================================
void UserDataInit(void);

//=============================================================================================
//函数名称	: void UserDataLifetimeUpdate(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 用户数据历史数据更新
//注    意	: 100ms任务
//=============================================================================================
void UserDataLifetimeUpdate(void);

#endif

/*****************************************end of UserData.h*****************************************/