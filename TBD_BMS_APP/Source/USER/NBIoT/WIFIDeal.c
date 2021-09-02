//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: WIFIDeal.c
//创建人  	: 
//创建日期	:
//描述	    : WIFI模块处理文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "WIFIDeal.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "pt.h"
#include "Gprs.h"

//=============================================================================================
//全局变量
//=============================================================================================
/* WIFI处理数据结构体 */
t_WIFI_MSG gWifiDealMsg = 
{
    .RecdataBuff = {0},
};

WifiInfo wifiInfo;  /* WIFI接收的信息拆解 */
t_WIFI_Coor gWifiCoor;    /* WIFI Mac解析点 */
Queue_old gWifiCoorQueue;
pt gPTWifi;         /* WIFI任务pt线程控制变量 */

//=============================================================================================
//静态函数声明
//=============================================================================================

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称  : void WifiDealInit(void)
//输入参数  : void
//输出参数  : void
//函数功能  : WIFI处理初始化
//注意事项	:
//=============================================================================================
void WifiDealInit(void)
{
    memset(gWifiDealMsg.RecdataBuff, 0, WIFI_RX_BUF_LEN);
    SafeBuf_Init(&gWifiDealMsg.RxSafeBuff,gWifiDealMsg.RecdataBuff,WIFI_RX_BUF_LEN);
    
    gWifiDealMsg.WifiFlag.WifiFlagBit.IsUpdInfo = False;
    gWifiDealMsg.WifiFlag.WifiFlagBit.IsReset = True;
    
    //初始化队列，队列含有WIFI_TABLE_LEN个元素
	QUEUE_init(&gWifiCoorQueue, &gWifiDealMsg.WifiCoor, sizeof(t_WIFI_Coor), GET_ELEMENT_COUNT(gWifiDealMsg.WifiCoor));  
    
    PT_INIT(&gPTWifi);	//初始化通信任务pt线程控制变量
}

//=============================================================================================
//函数名称  : void WifiSendAtCmd(const u8 * AtCmdBuff,u8 len)
//输入参数  : 
//输出参数  : 
//函数功能  : WIFI发送AT命令
//注意事项	: 发送数据长度小于256
//=============================================================================================
void WifiSendAtCmd(const u8 * AtCmdBuff,u8 len)
{
    #define DataLenMax 256
    u8 pData[DataLenMax];
    memcpy(pData,AtCmdBuff,len);
    LPUART_DRV_SendData(WIFI_UART_CHANNEL, pData, len);
//    /* 更改为等待发送完毕接口 */
//    uint32_t bytesRemaining;
//	uint32_t usart_timeout = 0;
//    usart_timeout = system_ms_tick;
    
//    LPUART_DRV_SendDataPolling(WIFI_UART_CHANNEL, AtCmdBuff, len);
//    while (LPUART_DRV_GetTransmitStatus(WIFI_UART_CHANNEL, &bytesRemaining)
//            != STATUS_SUCCESS)
//    {
//		if( system_ms_tick - usart_timeout > 3 ) break;
//    }
}

//=============================================================================================
//函数名称  : void WifiRxByte(uint8 data)
//输入参数  : data:写入的数据
//输出参数  : void
//函数功能  : WIFI接收数据到接收缓存中
//注意事项	:
//=============================================================================================
void WifiRxByte(uint8 data)
{
	gWifiDealMsg.WifiFlag.WifiFlagBit.Isfull = (0 == SafeBuf_WriteByte(&gWifiDealMsg.RxSafeBuff, data));
}

//=============================================================================================
//函数名称  : int WifiDealNumOfStr(u8 *Mstr, u8 *substr)
//输入参数  : Mstr:主字符串 substr：子字符串
//输出参数  : void
//函数功能  : 寻找主字符串中子字符串的数量
//注意事项	:
//=============================================================================================
int WifiDealNumOfStr(u8 *Mstr, u8 *substr)
{
    int number = 0;
   
    u8 *p;//字符串辅助指针
    u8 *q;//字符串辅助指针
   
    while(*Mstr != '\0')
    {
        p = Mstr;
        q = substr;
       
        while((*p == *q)&&(*p!='\0')&&(*q!='\0'))
        {
            p++;
            q++; 
        } 
        if(*q=='\0')
        {
            number++; 
        }
        Mstr++;
    }
    return number;   
}

//=============================================================================================
//函数名称  : void WifiDealWSCANPharse(void)
//输入参数  : void
//输出参数  : void
//函数功能  : 解析QWSCAN命令中的wifi信息
//注意事项	:
//=============================================================================================
void WifiDealWSCANPharse(void)
{
    #define   CUT_STRING  "CMDRSP:+WSCAN:"  /*用于拆解的子字符串*/
    u8 *wscan = gWifiDealMsg.DealdataBuff;
    char *pRsp = NULL;
    char *Msg = NULL;   
    static BssidInfo bssidinfo;

    const static int WifiInfoCnt = sizeof(WifiInfo)/4;
    const static int BssidCnt = sizeof(bssidinfo)/4;
    int bssidtemp;
    u8 i = 0;
    
    /* 可以解析出Wslen - 1个wifi信号 */
    gWifiDealMsg.Wslen = WifiDealNumOfStr(wscan,(u8 *)CUT_STRING);
    
    /* 无有效wifi信号解析 */
    if(1 >= gWifiDealMsg.Wslen)
    {
        gWifiDealMsg.WifiInfolen = 0;
        gWifiDealMsg.WifiFlag.WifiFlagBit.IsUpdInfo = False;
        return;
    }
    /* 超过WifiTab存储的长度 */
    else if(WIFI_TABLE_LEN < gWifiDealMsg.Wslen - 1)
    {
        gWifiDealMsg.Wslen = WIFI_TABLE_LEN;
    }
    else
    {
        gWifiDealMsg.Wslen -= 1;
    }
    
    gWifiDealMsg.WifiInfolen = 0;
    for(i = 0;i < gWifiDealMsg.Wslen;i++)
    {
        /* 先找出第一个与第二个之间，后找出第二个和第三个之间，以此类推 */
        pRsp = strstrex((char*)wscan, CUT_STRING);
        memset(pRsp-strlen(CUT_STRING),(int)"X",strlen(CUT_STRING));
        Msg = pRsp;
        pRsp = strstrex((char*)wscan, CUT_STRING);
        memset(gWifiDealMsg.RecdataBuff,0,WIFI_RX_BUF_LEN);    
        memcpy(gWifiDealMsg.RecdataBuff,Msg,pRsp-Msg-strlen(CUT_STRING));
        
        if(WifiInfoCnt != strtoken((char*)gWifiDealMsg.RecdataBuff, "," ,(char**)&wifiInfo, WifiInfoCnt)) 
        {
            continue;
        }
        else
        {
            sscanf(wifiInfo.RSSI, "%d", &gWifiCoor.RSSI);
            //rssi计算，rssi + 100，手机扫描出来如果是-70dbm，scan是30
            gWifiCoor.RSSI -= 100;
            if(BssidCnt != strtoken(wifiInfo.BSSID, ":" ,(char**)&bssidinfo, BssidCnt)) 
            {
                continue;
            }
            else
            {
                ++gWifiDealMsg.WifiInfolen;
                sscanf(bssidinfo.Bssid0, "%x", (int *)&bssidtemp);
                gWifiCoor.BSSID[0] = (u8)bssidtemp;    
                sscanf(bssidinfo.Bssid1, "%x", (int *)&bssidtemp);
                gWifiCoor.BSSID[1] = (u8)bssidtemp;   
                sscanf(bssidinfo.Bssid2, "%x", (int *)&bssidtemp);
                gWifiCoor.BSSID[2] = (u8)bssidtemp;   
                sscanf(bssidinfo.Bssid3, "%x", (int *)&bssidtemp);
                gWifiCoor.BSSID[3] = (u8)bssidtemp;   
                sscanf(bssidinfo.Bssid4, "%x", (int *)&bssidtemp);
                gWifiCoor.BSSID[4] = (u8)bssidtemp;   
                sscanf(bssidinfo.Bssid5, "%x", (int *)&bssidtemp);
                gWifiCoor.BSSID[5] = (u8)bssidtemp;
                QUEUE_add(&gWifiCoorQueue, &gWifiCoor, sizeof(t_WIFI_Coor));
            }
        }    
    }
    
    if(0 < gWifiDealMsg.WifiInfolen)
    {
        gWifiDealMsg.WifiFlag.WifiFlagBit.IsUpdInfo = True;
        g_pGprs->isSendWifiMac = True;
    }
}

//=============================================================================================
//函数名称  : u8 WifiDealGetCoors(uint8* pBuf)
//输入参数  : void
//输出参数  : void
//函数功能  : 获取WIFI MAC地址点，出队列，放在pbuf中
//注意事项	:
//=============================================================================================
u8 WifiDealGetCoors(uint8* pBuf)
{
	int i = 4;
	t_WIFI_Coor* pWifiCoor = Null;

    //增加SOC显示
    pBuf[0] = gBatteryInfo.Data.FiltSOC & 0xff;
    pBuf[1] = (gBatteryInfo.Data.FiltSOC >> 8) & 0xff;
    
    //GPS定位
	if(g_pGprs->isSendGps)
	{
		pBuf[2] = GPS_LOCATE_MODE;
	}
    //基站定位
    else if(g_pGprs->isSendLbs)
    {
		pBuf[2] = LBS_LOCATE_MODE;
    }
    //WIFI定位
    else if(g_pGprs->isSendWifiMac)
    {
        pBuf[2] = WIFI_LOCATE_MODE;
    }
    pBuf[3] = QUEUE_getElementCount(&gWifiCoorQueue);
    
    if(gGSMCond.IsPCBTest == True)
        return i;
    
    //出队列
	for(pWifiCoor = (t_WIFI_Coor*)QUEUE_getHead(&gWifiCoorQueue); pWifiCoor; QUEUE_removeHead(&gWifiCoorQueue), pWifiCoor = (t_WIFI_Coor*)QUEUE_getHead(&gWifiCoorQueue))
	{
		memcpy(&pBuf[i], pWifiCoor, sizeof(t_WIFI_Coor));
		i+= sizeof(t_WIFI_Coor);
	}

	return i;
}

//=============================================================================================
//函数名称	: void WifiDealMainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: WIFI处理主任务函数
//注    意	:
//=============================================================================================
void WifiDealMainTask(void *p)
{
    #define     AT_CMD_RESET    "AT\r\n"//"AT+QRST\r\n"
    #define     AT_CMD_QWSCAN   "AT+QWSCAN\r\n"
    
    static u8 sstep = 0;
    static u8 sRetryCnt = 0;    //尝试次数
    u8 byte;
    
    /*
    PT_BEGIN(&gPTWifi);
    
    for(;;)
    {
        WifiSendAtCmd((u8 *)AT_CMD_1,strlen(AT_CMD_1));
        //PT_WAIT_UNTIL(&gPTWifi,(BSPTaskStart(TASK_ID_WIFI_TASK, 2)));
        
    }
    
    PT_END(&gPTWifi);
    */
    
    if(gWifiDealMsg.WifiFlag.WifiFlagBit.IsReset == True)
    {
        //BSPSCIInit(WIFI_UART_CHANNEL,0);
        BSPUART_Init(UART_WIFI_DEBUG,UART_WIFI_DEBUG_BAUD);
        gWifiDealMsg.WifiFlag.WifiFlagBit.IsReset = False;
        sstep = 0;
    }
    
    switch(sstep)
    {
        case 0:
            WifiSendAtCmd((u8 *)AT_CMD_RESET,strlen(AT_CMD_RESET));
            gWifiDealMsg.RecLen = 0;
            sstep++;
            gWifiDealMsg.StartTimer = GET_TICKS();
            BSPTaskStart(TASK_ID_WIFI_TASK, 2);
            break;
        
        case 1:
            if(SafeBuf_Read(&gWifiDealMsg.RxSafeBuff, &byte, 1))
            {
                //存到dealbuff中
                gWifiDealMsg.DealdataBuff[gWifiDealMsg.RecLen++] = byte;
                
                //复位响应成功
                if(IS_FOUND(gWifiDealMsg.DealdataBuff,"CMDRSP:"))
                {
                    gWifiDealMsg.StartTimer = GET_TICKS();
                    sstep++;
                    BSPTaskStart(TASK_ID_WIFI_TASK, 1000); 
                }
                else
                {
                    BSPTaskStart(TASK_ID_WIFI_TASK, 2); 
                }
            }
            else
            {
                sstep = 1;
                if(GET_TICKS() - gWifiDealMsg.StartTimer > WIFI_WAIT_TIMEOUT)
                {
                    sstep = 0;
                }
                BSPTaskStart(TASK_ID_WIFI_TASK, 2); 
            }
            
            break;
            
        case 2:
            //将数据全部读上来
            while(SafeBuf_Read(&gWifiDealMsg.RxSafeBuff, &byte, 1))
            {
                gWifiDealMsg.DealdataBuff[gWifiDealMsg.RecLen++] = byte;
            }
            
            if(IS_FOUND(gWifiDealMsg.DealdataBuff,"OK\r\n"))
            {
                sstep++;    //等待NB模块登陆服务器后启动查询
                if(gGSMCond.IsPCBTest == True)
                    BSPTaskStart(TASK_ID_WIFI_TASK, 50);
            }
            else
            {
                BSPTaskStart(TASK_ID_WIFI_TASK, 2);   
            }                
            
            //超时，重发
            if(GET_TICKS() - gWifiDealMsg.StartTimer > WIFI_WAIT_TIMEOUT)
            {
                sstep = 0;
                BSPTaskStart(TASK_ID_WIFI_TASK, 2); 
            }
            
            break;
            
        case 3:
            WifiSendAtCmd((u8 *)AT_CMD_QWSCAN,strlen(AT_CMD_QWSCAN));
            gWifiDealMsg.RecLen = 0;
            sstep++;
            gWifiDealMsg.StartTimer = GET_TICKS();
            BSPTaskStart(TASK_ID_WIFI_TASK, 2000);
            break;        
        
        case 4:
            //buff满，则立刻进行处理
            //或者等待时间超过4秒
            if((gWifiDealMsg.WifiFlag.WifiFlagBit.Isfull == True) || (GET_TICKS() - gWifiDealMsg.StartTimer > WIFI_WAIT_TIMEOUT))
            {
                gWifiDealMsg.WifiFlag.WifiFlagBit.Isfull = 0;
                while(SafeBuf_Read(&gWifiDealMsg.RxSafeBuff, &byte, 1))
                {
                    gWifiDealMsg.DealdataBuff[gWifiDealMsg.RecLen++] = byte;
                }
                WifiDealWSCANPharse();
                SafeBuf_Reset(&gWifiDealMsg.RxSafeBuff);
                sstep = 3;
                //没解析出有效wifi信号
                if(gWifiDealMsg.WifiFlag.WifiFlagBit.IsUpdInfo == False)
                {
                    //再重新尝试一次，单次尝试WIFI_ONETIME_RETRY
                    sRetryCnt++;
                    if(sRetryCnt >= WIFI_ONETIME_RETRY)
                    {
                        sRetryCnt = 0;
                    }
                    else
                    {
                        BSPTaskStart(TASK_ID_WIFI_TASK, 1000);
                    }
                }
                else
                {
                    sRetryCnt = 0;
                    /* 周期发送 */
                    //BSPTaskStart(TASK_ID_WIFI_TASK, 15000);
                }
            }
            else
            {
                BSPTaskStart(TASK_ID_WIFI_TASK, 20);  
            }
            
            break; 
        
        //清空
        case 0xaa:
            SafeBuf_Reset(&gWifiDealMsg.RxSafeBuff);
            memset(gWifiDealMsg.DealdataBuff, 0, WIFI_RX_BUF_LEN);
            gWifiDealMsg.RecLen = 0;
            sstep = 0;  //复位步骤

            BSPTaskStart(TASK_ID_WIFI_TASK, 2);           
            break;
        
        default:
            ;
            break;
    
    }
    
}

/*****************************************end of WIFIDeal.c*****************************************/
