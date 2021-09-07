//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Sim.c
//创建人  	: 
//创建日期	:
//描述	    : Sim模组SIM处理文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "Sim.h"
//#include "BSPSCI.h"
#include "BSP_UART.h"
#include "BmsSystem.h"
#include "Gprs.h"
#include "BSPSystemCfg.h"
#include "AtCmdTab.h"
#include "Fsm.h"
#include "ShutDownMode.h"
#include "CommCtrl.h"
#include "WIFIDeal.h"
#include "BSPTaskDef.h"

#ifdef GPRS_FW_UPGRADE
#include "GprsUpg.h"
#endif

//=============================================================================================
//定义全局变量
//=============================================================================================
SimCard __g_SimCard;
SimCard* g_pSimCard = &__g_SimCard;

static AtCmdCtrl __g_SimAtCmdCtrl;
AtCmdCtrl* g_pSimAtCmdCtrl = &__g_SimAtCmdCtrl;
static SwTimer g_SimTimer;

volatile bool g_isPowerDown = False; 
static SwTimer g_SimPowerTimer;     //SIM上电延时定时器
//static uint32 g_SendDataDelay = GPRS_SENDDATA_TIME_LONG;
uint32 g_SendDataDelay = GPRS_SENDDATA_TIME_LONG;
uint32 g_SimPowerOffDelay = POWERE_OFF_DEFAULT; //上电延时变量
char gSimTxBuff[256] = {0};
u32 gSimStartTim = 0;

//=============================================================================================
//定义接口函数
//=============================================================================================

//SIM初始化，进行g_pSimAtCmdCtrl这个结构体的注册
void Sim_Init()
{
	AtCmdCtrl_Init(g_pSimAtCmdCtrl  //需要注册的结构体地址
    , Sim_TxAtCmd                   //底层发送命令接口
    , Sim_SendCmdItem               //发送数据打包
	, g_SimReq                      //请求对应处理的队列
    , g_GprsCmdTbl                  //发送的AT命令请求及其相应处理的队列        
	, AT_OK_STR, AT_ERROR_STR);     //成功和失败的子字符串

	g_SendDataDelay = GPRS_SENDDATA_TIME_LONG;
	g_pSimAtCmdCtrl->Filter = Sim_Filter;
	Gps_Init();
	Gprs_Init();
    /*LbsInit();*/
	
#ifdef CFG_SIM_SLEEP
    //此处配置唤醒引脚中断
//	Mcu_RegGpioIsr(SIM_UART_RX_PORT, SIM_UART_RX_PIN, Sim_UartWakeUpIsr);
#endif
}

//SIM复位函数
//如果PowerReset连续5次，都没有启动成功,则必须执行一次完全关电，
//完全关电必须要Sim_PowerOff持续拉低30秒.此处拉低了60S
u8 resetreson = 0;
void Sim_PowerReset(uint8 reason)
{
	//PFL(DL_MAIN, "Sim power reset\n");
	printf("Sim power reset\n");
	
	if(g_SimPowerOffDelay++ >= (POWERE_OFF_DEFAULT+3))
	{
		g_SimPowerOffDelay = 10000;
        if(gGSMCond.IsPCBTest == True)
            BSPTaskStart(TASK_ID_WIFI_TASK, 50);
	}
	else if(g_SimPowerOffDelay >= 10000)
	{
		g_SimPowerOffDelay = POWERE_OFF_DEFAULT;
	}
	resetreson = reason;
    Sim_ShutDown();
	
	SwTimer_Start(&g_SimPowerTimer, g_SimPowerOffDelay, SIM_TIME_ID_RESET);
    
    if(g_SimPowerOffDelay >= 10000)
	{
		g_SimPowerOffDelay = POWERE_OFF_DEFAULT;
	}
}

//重新执行上电流程
void Sim_Start()
{
	Sim_PowerReset(SIM_START);
}

//SIM模组是否可以进入睡眠
Bool Sim_CanGoSleep()
{
	return !AtCmdCtrl_IsBusy(g_pSimAtCmdCtrl);
}

//SIM模组是否睡眠
Bool Sim_IsSleep()
{
	return g_pSimCard->isLowPower && g_pSimCard->isSimSleep;
}

//SIM模组是否关机
Bool Sim_IsShutDown()
{
	return g_pSimCard->isShutDown;
}

//SIM进入睡眠
Bool Sim_Sleep()
{
    printf("Sim Sleep\n");
    
    gGSMCond.gsmcsq = 0xff;
    gGSMCond.gsmfixtime = 0xffff;
    gGSMCond.gpssignal = 0xff;
    gGSMCond.sateinview = 0xff;
    gGSMCond.gsmteststat = 0;
    
	SwTimer_Stop(&g_SimTimer);
	if(g_pSimCard->isLowPower)
	{
		return False;
	}
	
#ifdef CFG_SIM_SLEEP
	Gprs_SetCsclk(ENABLE_SLOW_CLK); 
#endif	
	g_pSimCard->isSimSleep = False;
	g_pSimCard->isSimSleepNew = True;
	g_pSimCard->isLowPower = True;

	
	SwTimer_Stop(&g_SimTimer);
	
	AtCmdCtrl_ResetEx(g_pSimAtCmdCtrl);

	
	Gprs_Sleep();
	Gps_Sleep();
	Gprs_WdStop();
    g_pSimCard->isSimStartOK = Sim_Wake_Fail;
    gSimStartTim = GET_TICKS();
	return True;
}

//Sim模组唤醒
//回复false为唤醒失败，true为唤醒成功
Bool Sim_Wakeup()
{
    printf("Sim Wakeup\n");    
    
    //更改为在上电后记录秒数
    //获取当前时间秒数
    //gGSMCond.gsmstartsec = BSPTaskGetSysTimer();
    gSimStartTim = GET_TICKS();   
    
	if(g_pSimCard->isLowPower == False) 
	{
		return False;
	}
	
#ifdef CFG_SIM_SLEEP
	Gprs_SetCsclk(DISABLE_SLOW_CLK);
#endif
    
	g_pSimCard->isSimSleepNew = False;
	//g_pGps->isPowerOnNew = True;
	//g_pGps->isPowerOn = False;
	SIM_WAKEUP();

	g_pSimCard->isLowPower = False;
	
	SetErrorCode(ERR_TYPE_GPRS, 0, ET_NONE);
	SetErrorCode(ERR_TYPE_GPS, 0, ET_NONE);

    //BSPSCIInit(GSM_UART_CHANNNEL,0); 
	AtCmdCtrl_ResetEx(g_pSimAtCmdCtrl);     //复位g_pSimAtCmdCtrl
	
	Gprs_Wakeup();  
	Gps_Wakeup();
	return True;
}

//唤醒调用函数
#ifdef CFG_SIM_SLEEP
void Sim_UartWakeUpIsr()
{	
	if(g_isPowerDown)
	{
	   	PostMsg(MSG_SIM_WAKEUP_IN_TIME);
		SetWakeUpType(WAKEUP_SIM);
	}
}
#endif

//SIM模块上电
void Sim_DoPowerOn()
{
	if(!Sim_Wakeup()) 
	{
		return;
	}
	
	//上电过程，POWER_KEY先拉高500ms
    //PWR KEY拉低代表为启动
    
	SIM_PWR_ON();
	//SIM_PWR_KEY_OFF();
	Sim_TimerStart(500, SIM_TIME_ID_POWERON1);
	
	g_pSimCard->isSimSleep = False;
	g_pGps->isPowerOnNew = True;
    g_pSimCard->isShutDown = False;     //关机标志清零
    g_pSimCard->isWakeFromSleep = True;
}

//SIM模块下电
void Sim_PowerOff()
{
    printf("Sim PowerOff\n");     
    Sim_ShutDown();
//	if(!Sim_Sleep()) 
//    {
//        return;    //已经低功耗模式则返回
//    }

//	Gprs_Sleep();
//	Gps_Sleep();
//	Gprs_WdStop();    
//	Gprs_Reset();
//	Gps_Reset();
//	Lbs_Reset();
//    /*
//	CLK_DisableModuleClock(UART1_MODULE);
//	NVIC_DisableIRQ(UART1_IRQn);
//	GPIO_SetMode(SIM_UART_RX_PORT, SIM_UART_RX_PIN, GPIO_PMD_INPUT);
//	GPIO_SetMode(SIM_UART_RX_PORT, BIT5, GPIO_PMD_INPUT);
//    */
//    BSPSCIDeInit(eSCI1);

//    //SIM模块下电
//	SIM_PWR_KEY_OFF();
//	SIM_PWR_OFF();
//    
//    g_pSimCard->isShutDown = True;     //关机标志置位
//	g_pSimCard->isSimSleep = True;
//	g_pGps->isPowerOn = False;
//	g_pSimCard->isCommOk = False;
//	g_pGprs->isSyncTime = False;
}

//SIM模块关机
void Sim_ShutDown()
{
    printf("Sim ShutDown\n");  
    gGSMCond.gsmcsq = 0xff;
    gGSMCond.gsmfixtime = 0xffff;
    gGSMCond.gpssignal = 0xff;
    gGSMCond.sateinview = 0xff;
    gGSMCond.gsmteststat = 0;
    
	SwTimer_Stop(&g_SimTimer);
	AtCmdCtrl_ResetEx(g_pSimAtCmdCtrl);

	Gprs_Sleep();
	Gps_Sleep();
	Gprs_Reset();
	Gps_Reset();
	/*Lbs_Reset();*/
    
//    BSPSCIDeInit(GSM_UART_CHANNNEL);
//    BSPSCIPinOutputCfg(GSM_UART_CHANNNEL);
//    
//    //WIFI模块也需要下电，不然电平一直拉高无法复位
//    BSPSCIDeInit(WIFI_UART_CHANNEL);
//    BSPSCIPinOutputCfg(WIFI_UART_CHANNEL);
    BSPUART_DeInit(GSM_UART_CHANNNEL);
    BSPUART_DeInit(WIFI_UART_CHANNEL);
    WifiDealInit();
    
    //SIM模块下电
	SIM_PWR_KEY_OFF();
	SIM_PWR_OFF();
    
    g_pSimCard->isLowPower = True;     //低功耗标志置位
    g_pSimCard->isShutDown = True;     //关机标志置位
	g_pSimCard->isSimSleep = True;
	g_pGps->isPowerOn = False;
	g_pSimCard->isCommOk = False;
    g_pSimCard->isAlwaysCommOk = False;
	g_pGprs->isSyncTime = False;
    g_pSimCard->isSimStartOK = Sim_Wake_Init;
}

//SIM接收放在UART中断接收函数进行接收处理
void Sim_RxByte(uint8 data)
{
	SafeBuf_WriteByte(&g_pSimAtCmdCtrl->m_SafeBuf, data);

	if(data=='\n')
	{
		if(!g_pSimCard->isCommOk)
		{
			//Printf("Sim power on\n");
		}
		g_pSimCard->isCommOk = TRUE;
        g_pSimCard->isAlwaysCommOk = TRUE;        
	}
}

void SimDebug_DumpByte(const char* pData, uint16 len, uint8 cols) 
{
    int i; 
    int counter = 0;	
	//Printf("Dump Data, addr=[0x%04x], Len=%d\n", pData, len); 
    while(counter < len) 
    { 
        //Printf("[%04x]:",counter); 
        for(i=0; i < cols; i++) 
        { 
            printf("%02X ",*pData); 
            pData++; 
            if(++counter >= len) break;	
        } 
        printf("\n"); 
    } 
}

//SIM发送AT命令函数
Bool Sim_TxAtCmd(const char* pData, int len)
{
//    u8 i = 0 ;
	if(pData[0] == 'A' && pData[1] == 'T')
	{
		//PFL(DL_SIM, "%s", pData);
		printf("%s", pData);
	}
	else
	{
		//PFL(DL_SIM, "Data[%d]: %02X %02X %02X ", len, pData[0], pData[1], pData[2]);
		printf("Data[%d]: %02X %02X %02X ", len, pData[0], pData[1], pData[2]);
        //printf("SimTx:");
        //SimDebug_DumpByte(pData, len,60);
		//DUMP_BYTE_EX("SimTx:", pData, len);
	}
    
//    for(i = 0;i < len;i++)
//    {
//        gSimTxBuff[i] = pData[i];
//    }
    memcpy(gSimTxBuff,pData,len);
    
	LPUART_DRV_SendData(GSM_UART_CHANNNEL, (uint8_t *) gSimTxBuff, len);
//    uint32_t bytesRemaining;
//	uint32_t usart_timeout = 0;
//    usart_timeout = system_ms_tick;
//    
//    LPUART_DRV_SendDataPolling(GSM_UART_CHANNNEL, (uint8_t *) gSimTxBuff, len);
//    while (LPUART_DRV_GetTransmitStatus(GSM_UART_CHANNNEL, &bytesRemaining)
//            != STATUS_SUCCESS)
//    {
//		if( system_ms_tick - usart_timeout > 3 ) break;
//    }
	
	return True;
}

//SIM模组定时器启动
void Sim_TimerStart(uint32 timeOutMs, uint32 nId)
{
	//Printf("****Sim timer[%d] start %d.\n", nId, timeOutMs);
	SwTimer_Start(&g_SimTimer, timeOutMs, nId);
}

//判断是否接收结束
Bool Sim_Filter(const char* pRcv, uint16* len)
{
	if(!AtCmdCtrl_IsLineEnd(pRcv, len)) return False;
	
	if(IS_FOUND(pRcv, "AT+"))
	{
		//因为关闭了回显，收到"AT+"字符，说明SIM模组已经被复位
        //第二次发送相应的命令，sim868都是直接回复\r\nOK\r\n
		Gprs_Reset();
		Gps_Reset();
	}

	if(g_pGprs->isSimReady != SIMCARD_READY)
	{
		//if(IS_FOUND(pRcv, "READY\r\n"))
        if(IS_FOUND(pRcv, "\r\n+IP:"))//\r\n+IP:
		{
			printf("Sim is Ready\n");
			//启动READY定时器,如果在该时间内没有连接成功,则
			g_pGprs->isSimReady = SIMCARD_READY;
			Sim_TimerStart(120000, SIM_TIME_ID_READY);
		}
	}
	
	if(g_pGprs->isConnected)
	{
		//Printf("Stop SIM_TIME_ID_READY\n");
		SwTimer_StopEx(&g_SimTimer, SIM_TIME_ID_READY);
	}
	 
	return True;
}

//设置发送延时
void Sim_SetTxDataTime(uint32 ms)
{
	g_SendDataDelay = ms;
}

//暂不使用
int Sim_SyncSendAtCmdEx(const char* pAtCmd, const char* pAck, int waitMs, char** pRsp)
{
	*pRsp = g_pSimAtCmdCtrl->m_Rsp;
	return AtCmdCtrl_SyncSend(g_pSimAtCmdCtrl, pAtCmd, pAck, waitMs);
}

//同步发送pAtCmd命令字符串，等待接收响应
int Sim_SyncSendAtCmd(const char* pAtCmd, const char* pAck, int waitMs)
{
	return AtCmdCtrl_SyncSend(g_pSimAtCmdCtrl, pAtCmd, pAck, waitMs);
}

//异步发送命令
int Sim_AnsySendAtCmd(const char* pAtCmd, const char* pAck, int waitMs, uint32 nTag)
{
	return AtCmdCtrl_AnsySend(g_pSimAtCmdCtrl, pAtCmd, pAck, waitMs, nTag);
}

//延迟timeOutMs时间后根据命令索引发送数据
void Sim_DelaySendCmdInd(int cmdInd, uint32 timeOutMs)
{
	AtCmdCtrl_DelaySendCmdInd(g_pSimAtCmdCtrl, cmdInd, timeOutMs);
}

//根据命令索引发送数据
Bool Sim_SendCmdInd(int cmdInd)
{
	return AtCmdCtrl_SendCmdByInd(g_pSimAtCmdCtrl, cmdInd);
}

//SIM模组获取命令索引
AtCmdItem* Sim_GetCmdByInd(int cmdInd)
{
	return AtCmdCtrl_GetCmdInd(g_pSimAtCmdCtrl, cmdInd);
}

//SIM模组根据命令发送数据
Bool Sim_SendCmdItem(const AtCmdItem* pCmd)
{
	char* buff = (char*)g_CommonBuf;
	uint32 delay = 5000;
	
    //准备发送TCP数，发送AT+CIPSEND = len命令
	if(CMD_CIPSEND == pCmd->cmdInd)
	{
		g_SimPowerOffDelay = POWERE_OFF_DEFAULT;
		Sign_TimerReset();
		Sim_SetTxDataTime(GPRS_SENDDATA_TIME_LONG);
		return Gprs_SendData(pCmd);
	}
//    //收到>后，将需要发送的数据发送出去，因为发送长度已定，所以不用发送结束符0x1A
//	else if(CMD_CIPSENDDATA == pCmd->cmdInd)
//	{
//		g_SimPowerOffDelay = POWERE_OFF_DEFAULT;
//		Sign_TimerReset();
//		//DUMP_BYTE_EX(("Tx:"), g_pGprs->pTcpPkt, g_pGprs->tcpPktlen);
//		AtCmdCtrl_AnsySendData(g_pSimAtCmdCtrl, g_pGprs->pTcpPkt, g_pGprs->tcpPktlen, pCmd->atAck, g_SendDataDelay, (uint32)pCmd);
//		Sim_SetTxDataTime(GPRS_SENDDATA_TIME_LONG);
//		return True;
//	}

	strcpy(buff, pCmd->atCmd);
    
    //GPS打开/关闭控制
	if(CMD_CGNSPWR == pCmd->cmdInd)
	{
		sprintf(buff, pCmd->atCmd, g_pGps->isPowerOnNew);
	}
    //连接到服务器对应的端口
	else if(CMD_CIPOPEN == pCmd->cmdInd)
	{
        //将at命令的参数输入到atcmd命令中组合。
		//sprintf(buff, pCmd->atCmd, g_pDataRom->m_IpAddr, g_pDataRom->m_IpPort); 
//        sprintf(buff, pCmd->atCmd, (gUserPermitCmd & USER_TESTENVIR_CMD_EN)?IP_ADDR_TEST:IP_ADDR, 
//                                   (gUserPermitCmd & USER_TESTENVIR_CMD_EN)?IP_PORT_TEST:IP_PORT);
        sprintf(buff, pCmd->atCmd, IP_ADDR_TEST, IP_PORT_TEST);        
		delay = 20000;
	}
    //查询GPS信息
	else if(GPS_CMD_CGPSINFOCFG == pCmd->cmdInd)
	{
        //将at命令的参数输入到atcmd命令中组合。
		sprintf(buff, pCmd->atCmd, g_pGps->m_TimeOutS); 
	}
	else if(CMD_NETCLOSE == pCmd->cmdInd)
	{
		delay = 20000;
	}
    
#ifdef CFG_SIM_SLEEP	
	else if(CMD_SIM_CSCLK == pCmd->cmdInd)
	{
        //将at命令的参数输入到atcmd命令中组合。
		sprintf(buff, pCmd->atCmd, Gprs_GetCsclk()); 
		delay = 3000;
	}
#endif	
    
	else if(CMD_SMS_TEXT == pCmd->cmdInd)
	{
		delay = 10000;    //等待时间
 	}
	else if(CMD_CLBS2 == pCmd->cmdInd)
	{
        //等待基站定位
		delay = 10000;    //等待时间
 	}    
	else if(g_pSimCard->SendCmdItem)
	{
		g_pSimCard->SendCmdItem(pCmd, buff, &delay);
	}
	else if(CMD_CFUN_CLOSE == pCmd->cmdInd)
	{
		delay = 10000;    //等待时间
 	}
	else if(CMD_CFUN_OPEN == pCmd->cmdInd)
	{
        //等待基站定位
		delay = 10000;    //等待时间
 	}     
    
	return AtCmdCtrl_AnsySend(g_pSimAtCmdCtrl, buff, pCmd->atAck, delay, (uint32)pCmd);
}

//SIM模块上电处理过程代码
void Sim_TimerProc()
{
    //500ms时间到
	if(SwTimer_isTimerOutId(&g_SimTimer, SIM_TIME_ID_POWERON1))
	{
		//SIM_PWR_KEY_ON();
		//SIM_PWR_KEY_IO 先拉高0.5秒,再拉低并保持
		Sim_TimerStart(500, SIM_TIME_ID_POWERON2);
	}
	else if(SwTimer_isTimerOutId(&g_SimTimer, SIM_TIME_ID_POWERON2))
	{
        //BSPSCIInit(GSM_UART_CHANNNEL,0);    
        BSPUART_Init(UART_GPS,UART_GPS_BAUD);
		g_pSimCard->isCommOk = True;

		PFL(DL_MAIN, "Sim power on.\n", _FUNC_);
        
		//启动READY定时器，若15s内收不到上电Ready的URC，则重新执行上电流程
		Sim_TimerStart(15000, SIM_TIME_ID_READY);
		SIM_PWR_KEY_OFF();
	}
    //10s内若收不到上电Ready的URC，则重新执行上电流程
	else if(SwTimer_isTimerOutId(&g_SimTimer, SIM_TIME_ID_READY))
	{
		Sim_PowerReset(SIM_READY_TIMEOUT);
	}
}

//SIM模组运行任务
void Sim_Run()
{
	AtCmdCtrl_Run(g_pSimAtCmdCtrl);
    
    //关闭模组电源的延时时间到
	if(SwTimer_isTimerOut(&g_SimPowerTimer))
	{
		g_pSimCard->isLowPower = True;  //当前状态还为低功耗状态，尚未启动
		Sim_DoPowerOn();
		goto End;
	}

    //模块上电处理
	Sim_TimerProc();
	
    //收到\r\n
	if(!g_pSimCard->isCommOk)
	{
		goto End;
	}
	
	if(g_isGprsRun) 
    {
        Gprs_Run();
    }
    
	if(!AtCmdCtrl_IsIdle(g_pSimAtCmdCtrl))
	{
		goto End;
	}
    
//    //有GPS定位
//	if(((!g_pGps->isStartGetInfo) || (g_pGps->isPowerOn != g_pGps->isPowerOnNew))
//		&& (g_pGprs->isSyncTime)              //已经和服务器进行了复位通信
//        && (g_pSimCard->isSimSleep == False)
//        && (g_pSimCard->isWakeFromSleep == True))   //从睡眠中唤醒成功 
//	{
//        if(True == g_pGps->isPowerOnNew)    //开启
//        {
//            if(False == g_pLbs->isLbsToGPS)
//            {
//                g_pGps->isStartGetInfo = True;
//                Sim_SendCmdInd(GPS_CMD_CGPSINFOCFG);
//            }
//            else
//            {
//                Sim_SendCmdInd(CMD_CGNSPWR);    //开启GPS电源，从基站切换过来的          
//            }  
//        }                
//        else if(False == g_pGps->isPowerOnNew)
//        {
//            Sim_SendCmdInd(CMD_CGNSPWR);    //需要关闭GPS电源，进行基站定位了
//        }
//            
//        g_isLbsRun = False;             //暂时不需要基站定位
//		goto End;
//	}
	
#ifdef CFG_SIM_SLEEP	
	if(g_pSimCard->isSimSleep != g_pSimCard->isSimSleepNew)
	{
		//如果没有启动完毕，不能进入睡眠
		if((g_pSimCard->isSimSleepNew == False) && (!g_pGprs->isIndentify))
		{
			g_pSimCard->isSimSleep = g_pSimCard->isSimSleepNew;
		}
		else
		{
			//Sim_SendCmdInd(CMD_SIM_CSCLK);
            if(True == g_pSimCard->isSimSleepNew)   //开始休眠
            {
                //g_pGprs->isSyncTime = False;                
                g_pSimCard->isWakeFromSleep = False;

            }
            //开始唤醒
            else
            {
                ;
            }
            Sim_DelaySendCmdInd(CMD_SIM_CSCLK,3000);
            
		}
		goto End;
	}
#endif	
End:
    Fsm_SetActiveFlag(AF_SIM, !AtCmdCtrl_IsIdle(g_pSimAtCmdCtrl));
}

/*****************************************end of Sim.c*****************************************/
