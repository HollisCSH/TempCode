//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: GprsTlv.c
//创建人  	: 
//创建日期	:
//描述	    : Sim模组Tlv电池协议处理文件
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
#include "Tlv.h"
#include "GprsTlv.h"
#include "BatteryInfo.h"
#include "DTCheck.h"
#include "Lbs.h"
#include "Gps.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
//保存已经发送成功的数据
static uint8 g_SrvTlvBuf[TAG_BUF_SIZE] = {0};
static uint8 g_SrvTlvLen = 0;

//保存正在发送的数据
static uint8 g_SendTlvLen = 0;
static uint8 g_SendTlv[sizeof(g_SrvTlvBuf)] = {0};
//static uint32 g_TlvTicks = 0;
static uint8 g_TlvBuf[15]; 

//=============================================================================================
//定义接口函数
//=============================================================================================

//GPRS TLV包初始化，将所有tlv组合成一个数组buffer：g_SrvTlvBuf
void GprsTlv_Init()
{
	g_SrvTlvLen = 0;
	static uint8 temp[42] = {0};
	Tlv* tlv = (Tlv*)&temp; //指向整个数组的指针强制转换为TLV类型的指针
	
	memset(temp, 0x00, sizeof(temp));
	
//	tlv->tag = TAG_ACC;
//	tlv->len = 1;
//	g_SrvTlvLen = Tlv_Append(g_SrvTlvBuf, g_SrvTlvLen, sizeof(g_SrvTlvBuf), tlv);
//	
//	tlv->tag = TAG_LOCK;
//	g_SrvTlvLen = Tlv_Append(g_SrvTlvBuf, g_SrvTlvLen, sizeof(g_SrvTlvBuf), tlv);
	//电池非法充电和电池过热 TLV包初始化
	tlv->tag = TAG_BATTERY1_OVP;
	tlv->len = 12;
	g_SrvTlvLen = Tlv_Append(g_SrvTlvBuf, g_SrvTlvLen, sizeof(g_SrvTlvBuf), tlv);	
	tlv->tag = TAG_BATTERY1_OTP;
	tlv->len = 12;    
	g_SrvTlvLen = Tlv_Append(g_SrvTlvBuf, g_SrvTlvLen, sizeof(g_SrvTlvBuf), tlv);
	tlv->tag = TAG_REMOVE;
	tlv->len = 12;    
	g_SrvTlvLen = Tlv_Append(g_SrvTlvBuf, g_SrvTlvLen, sizeof(g_SrvTlvBuf), tlv);    
}

//根据tag命令，输入单个字节组合成tlv，组合到pTlv数组中
int GprsTlv_AppendByteTlv(uint8* pTlv, int len, int maxLen, uint8 tag, uint8 data)
{
	Tlv* p = (Tlv*)&g_TlvBuf;
	Tlv_Init(p, tag, 1, &data);
	return GprsTlv_AppendTlv(pTlv, len, maxLen, p);
}

//根据输入的pNewTlv包，组合到pTlv数组中
int GprsTlv_AppendTlv(uint8* pTlv, int len, int maxLen, const Tlv* pNewTlv)
{
	if(pNewTlv == Null) return len;
	
	Tlv* pSrvTlv = Tlv_GetByTag(g_SrvTlvBuf, g_SrvTlvLen, pNewTlv->tag);
/*    
	if(pNewTlv->tag == TAG_ACC || pNewTlv->tag == TAG_LOCK)
	{
        //如果发生点火/熄火，开锁，上锁
		if(pSrvTlv->data[0] != pNewTlv->data[0])
        {
            //将tlv包压入GPRS_BUFF中
			return Tlv_Append(pTlv, len, maxLen, pNewTlv);
        }
		return len;
	}
*/
//    if((False == g_pLbs->isLocationUpdate) || (False == g_pGps->isLocationUpdate))
//    {
//        return len;
//    }
    
    if(pNewTlv->tag == TAG_BATTERY1_OVP || pNewTlv->tag == TAG_BATTERY1_OTP || pNewTlv->tag == TAG_REMOVE)
	{
        //第一次变化，或者间隔5分钟
        if(pNewTlv->data[0])    //存在非法失控或存在过热
        {
            if(pSrvTlv->data[0] != pNewTlv->data[0] //发生故障
            //    || SwTimer_isTimerOutEx(g_TlvTicks, 300000)
              )
            {
                return Tlv_Append(pTlv, len, maxLen, pNewTlv);
            }
        }
        else//更新SrvTlv，触发发送动作
        {
            if(pSrvTlv->data[0] != pNewTlv->data[0]) //故障消失
            {
                //memcpy(pSrvTlv, pNewTlv, pNewTlv->len + TLV_HEAD_SIZE);
                return Tlv_Append(pTlv, len, maxLen, pNewTlv);
            }            
        }
	}
	return len;
}

//更新g_SrvTlvBuf中的数据
void GprsTlv_UpdateSentTlv()
{
	Tlv* pSrcTlv = Null;
	Tlv* pReportedTlv = (Tlv*)&g_SendTlv[0];    //发送的tlv buff缓存
	
    //逐个逐个检测g_SendTlv中的TLV包
	for(int i = 0; i < g_SendTlvLen; i += pReportedTlv->len + TLV_HEAD_SIZE)
	{
		pReportedTlv = (Tlv*)&g_SendTlv[i];
		pSrcTlv = Tlv_GetByTag(g_SrvTlvBuf, sizeof(g_SrvTlvBuf), pReportedTlv->tag);
		if(pSrcTlv) 
		{
			memcpy(pSrcTlv->data, pReportedTlv->data, pReportedTlv->len);
            
            //发生过热告警和非法充电，5分钟上报一次
            //此处关闭5分钟上报一次，2020.02.11
			//if(pReportedTlv->tag != TAG_ACC && pReportedTlv->tag != TAG_LOCK)
            //{
			//	g_TlvTicks = GET_TICKS();
            //}
		}
	}
}

//非法充电tlv数据组装
Tlv* GprsTlv_GetOvp(uint8 port)
{
	int i = 0;
	Tlv* pTlv = (Tlv*)&g_TlvBuf;
    //是否存在非法充电
    Bool isOvp = 0;
    
    pTlv->tag = (port == 0) ? TAG_BATTERY1_OVP : TAG_BATTERY2_OVP;
   
	//是否存在非法充电

//    if(OPT_FAULT2_ILLEGAL_CHG_250MAH == (gGSMCond.tlvfault[3] & OPT_FAULT2_ILLEGAL_CHG_250MAH))
//    {
//        BITSET(isOvp,1);    
//    }

    if(OPT_FAULT2_ILLEGAL_CHG_1000MAH == (gGSMCond.tlvfault[3] & OPT_FAULT2_ILLEGAL_CHG_1000MAH))
    {
        BITSET(isOvp,3);    
    }
    else if(OPT_FAULT2_ILLEGAL_CHG_500MAH == (gGSMCond.tlvfault[3] & OPT_FAULT2_ILLEGAL_CHG_500MAH))
    {
        BITSET(isOvp,2);    
    }    
    if(OPT_FAULT2_ILLEGAL_VOLT_RISE == (gGSMCond.tlvfault[3] & OPT_FAULT2_ILLEGAL_VOLT_RISE))
    {
        BITSET(isOvp,0);
    }    

//	memcpy(&pTlv->data[i], gConfig.sn, SERIAL_NUM_SIZE);
//	i += SERIAL_NUM_SIZE;        
    pTlv->data[i] = isOvp;	
	memcpy(&pTlv->data[++i], &gBatteryInfo.VoltChara.MaxVolt, 2);
	i += 2;
    
    //加入GPS定位点    
    if(True == g_pGps->isLocationUpdate)
    {
        pTlv->data[i] = GPS_LOCATE_MODE;
        memcpy(&pTlv->data[++i], &g_pGps->longitude, 8);    
        i+=8;
    }
    else if(True == g_pLbs->isLocationUpdate)
    {
        pTlv->data[i] = LBS_LOCATE_MODE;  
        memcpy(&pTlv->data[++i], &g_pLbs->longitude, 8);     
        i+=8;        
    }

	pTlv->len = i;   

	return pTlv;
}

//电池过热tlv数据组装
Tlv* GprsTlv_GetOtp(uint8 port)
{
	int i = 0;
	Tlv* pTlv = (Tlv*)&g_TlvBuf;
    //是否存在电池过热
    Bool isOtp = 0;
    
    pTlv->tag = (port == 0) ? TAG_BATTERY1_OTP : TAG_BATTERY2_OTP;
    
 	//是否存在电池过热或者热失控
    if((OPT_FAULT1_DCHG_OT) == (gGSMCond.tlvfault[2] & (OPT_FAULT1_DCHG_OT)))
    {
        BITSET(isOtp,0);
    }
    if(OPT_FAULT2_TEMP_RISE_FAST == (gGSMCond.tlvfault[3] & OPT_FAULT2_TEMP_RISE_FAST))
    {
        BITSET(isOtp,1);    
    }
    
//	memcpy(&pTlv->data[i], gConfig.sn, SERIAL_NUM_SIZE);
//	i += SERIAL_NUM_SIZE;    
	pTlv->data[i] = isOtp;	
	memcpy(&pTlv->data[++i], &gBatteryInfo.TempChara.MaxTemp, 2);
	i += 2;
    
    //加入GPS定位点
    if(True == g_pGps->isLocationUpdate)
    {
        pTlv->data[i] = GPS_LOCATE_MODE;
        memcpy(&pTlv->data[++i], &g_pGps->longitude, 8);    
        i+=8;
    }
    else if(True == g_pLbs->isLocationUpdate)
    {
        pTlv->data[i] = LBS_LOCATE_MODE;  
        memcpy(&pTlv->data[++i], &g_pLbs->longitude, 8);     
        i+=8;        
    }    
    
	pTlv->len = i;       
	
	return pTlv;
}

//电池拆开数据组装
Tlv* GprsTlv_GetRemove(uint8 port)
{
	int i = 0;
	Tlv* pTlv = (Tlv*)&g_TlvBuf;
    //是否存在电池过热
    Bool isRemove = gGSMCond.tlvRemoveSta;
    
    pTlv->tag = TAG_REMOVE;
    
//	memcpy(&pTlv->data[i], gConfig.sn, SERIAL_NUM_SIZE);
//	i += SERIAL_NUM_SIZE;    
	pTlv->data[i] = isRemove;	
	memcpy(&pTlv->data[++i], &gBatteryInfo.Data.SOC, 2);
	i += 2;
    
    //加入GPS定位点
    if(True == g_pGps->isLocationUpdate)
    {
        pTlv->data[i] = GPS_LOCATE_MODE;
        memcpy(&pTlv->data[++i], &g_pGps->longitude, 8);    
        i+=8;
    }
    else if(True == g_pLbs->isLocationUpdate)
    {
        pTlv->data[i] = LBS_LOCATE_MODE;  
        memcpy(&pTlv->data[++i], &g_pLbs->longitude, 8);     
        i+=8;        
    }    
    
	pTlv->len = i;       
	
	return pTlv;
}


//获取事件变化，若有变化，则打包成tlv包，返回tlv包的长度
uint8 GprsTlv_GetChangedTlv(uint8* pBuf, int size)
{
	#if 0
	return 0;
	#else
	int len = 0;
    
    //点火事件状态改变
	//len = GprsTlv_AppendByteTlv(pBuf, len, size, TAG_ACC, Pms_IsAccOn());
	#ifdef CFG_WHEEL_LOCK
	len = GprsTlv_AppendByteTlv(pBuf, len, size, TAG_LOCK, g_pPms->m_isLock);
	#endif

    //返回当前的TLV数据包的长度
    len = GprsTlv_AppendTlv(pBuf, len, size, GprsTlv_GetOvp(0));
    len = GprsTlv_AppendTlv(pBuf, len, size, GprsTlv_GetOtp(0));
    len = GprsTlv_AppendTlv(pBuf, len, size, GprsTlv_GetRemove(0));
    
	if(len)
	{
		g_SendTlvLen = len;
		memcpy(g_SendTlv, pBuf, len);

		//DUMP_BYTE(g_SendTlv, g_SendTlvLen);
	}
	return len;
	#endif
}

/*****************************************end of GprsTlv.c*****************************************/
