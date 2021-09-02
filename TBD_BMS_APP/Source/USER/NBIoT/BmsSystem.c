//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: BmsSystem.c
//创建人  	: 
//创建日期	:
//描述	    : Sim模组关于电池BMS操作的文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SimCommon.h"
#include "BmsSystem.h"
#include "ParaCfg.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
//UserInfo  usersList[USER_NUM];
ScooterSettings  g_Settings;                //设置参数表
ErrorCode g_errorInfo;                      //错误信息
static uint8  g_errorList[ERR_TYPE_MAX];    //错误列表

//=============================================================================================
//定义接口函数
//=============================================================================================

//禁止放电
void SetForbidDischarge(Bool isEnable)
{
	g_Settings.IsForbidDischarge = isEnable; 
//	LOG2(isEnable ? ET_SYS_DISCHARGE_OFF : ET_SYS_DISCHARGE_ON, g_Settings.devcfg, 0);
}

//获取是否进制放电标志
Bool IsForbidDischarge()
{
    return g_Settings.IsForbidDischarge;
}

//设置激活标志
void SetActive(Bool isActive)
{
	g_Settings.IsDeactive = !isActive; 
//	LOG2(isActive ? ET_SYS_ACTIVE : ET_SYS_INACTIVE, g_Settings.devcfg, 0);
}

//获取激活标志
Bool IsActive()
{
    return !g_Settings.IsDeactive;
}

//设置错误标志
void SetErrorCode(uint8 err_type, uint32 errcode, ErrType errType)
{
    if(err_type >= ERR_TYPE_MAX)
    {
        return;
    }

    //	if(err_type == ERR_TYPE_GPRS) Printf("Err[%d][0x%x].\n", err_type, errcode);
    g_errorList[err_type] = (ET_ALARM == errType || errcode==0) ? errcode : (errcode | 0x80);
}

//获取错误码
uint8 GetErrorCode(uint8 err_type)
{
    if(err_type >= ERR_TYPE_MAX)
    {
		return 0;
    }
	
 	return g_errorList[err_type];
}

//根据错误码确定处于警戒状态
Bool ErrorCode_IsAlarm(uint8 errcode)
{
    return (errcode & 0x80) == 0;
}

//获取警戒标志
Bool IsAlarmMode()
{
	return g_Settings.IsAlarmMode;
}

/*****************************************end of BmsSystem.c*****************************************/
