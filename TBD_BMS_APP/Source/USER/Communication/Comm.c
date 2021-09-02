//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Comm.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 通信代码源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "Comm.h"
//#include "FM11NC08.h"
#include "MBServer.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "MBServer.h"
#include "CommCtrl.h"
#include "pt.h"

//=============================================================================================
//全局变量
//=============================================================================================
//static t_NFC_MSG gFrameRec = {NULL,0,0};	//NFC 收消息参数变量
//static t_NFC_MSG gFrameTx = {NULL,0,0};		//NFC 发消息参数变量
const u8 cPDUHead[4] = {0x02, 0x00, 0x00, 0x01};	//Modbus-NFC PDU 的附加帧头

t_MSG_PDU gMbRxBuff;				 		 //Modbus-NFC PDU 收缓存
t_MSG_PDU gMbTxBuff;				 		 //Modbus-NFC PDU 发缓存
t_COMM_POLL gCommPoll;                       //通信接收标志结构体

u16 gPduLen = 0;							 //保存回传的MODBUS PDU长度
u8 gExceptionCode = 0;						 //保存exception代码
pt gPTComm;									 //通信任务pt线程控制变量

//=============================================================================================
//静态函数声明
//=============================================================================================

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: u8 CommInit(void)
//输入参数	: void
//输出参数	: 初始化是否成功，TURE:成功  FALSE:失败
//静态变量	: void
//功    能	: 通信协议初始化函数
//注    意	:
//=============================================================================================
u8 CommInit(void)
{
	u8 result = TRUE;

	PT_INIT(&gPTComm);	//初始化通信任务pt线程控制变量

	return result;
}

//=============================================================================================
//函数名称	: void CommMainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 通信协议主任务函数
//注    意	:
//=============================================================================================
void CommMainTask(void *p)
{
//    //static pt sub_s;
//    //static u8 cnt;
//	(void)p;

//    PT_BEGIN(&gPTComm);
//    for(;;)
//    {
//        do
//        {
//            do
//            {
//            	gFrameRec.cnt = sizeof(t_MSG_PDU);
//            	gFrameRec.msg = (u8*)&gMbRxBuff;
//                FM11NC08FrameRx(&gFrameRec);
//                PT_WAIT_UNTIL(&gPTComm,(BSPTaskStart(TASK_ID_COMM_TASK, 2)) & (gFrameRec.sta > 0));
//            }while((!CommCheckCRCA((u8*)&gMbRxBuff,gFrameRec.cnt-2)));
//            break;	//跳出while(1)循环，进行接收数据的处理
//        }
//        while(1);//((FRAMESPEC.cnt<=6)||(FRAMESPEC.sta!=1)||(!CommStringComp(cPDUHead, &gMbRxBuff.pcb, 4))||(!CommCheckCRCA((u8*)&gMbRxBuff,FRAMESPEC.cnt-2)));//接收到符合的PDU

//		if(/*(gMbRxBuff.pcb == 0x00)&&*/(gMbRxBuff.cid==0x00)&&(gMbRxBuff.nad==0xA4)/*&&(gMbRxBuff.addr ==0x00)*/)//收到CPU卡文件选择指令
//		{
//			//回复6a82，模拟为CPU卡
//			gMbTxBuff.pcb = 0x02;
//			gMbTxBuff.cid = 0x6a;
//			gMbTxBuff.nad = 0x82;
//			CommCalcCRC(COMM_CRC_A , (u8*)&gMbTxBuff, 3, (u8*)&gMbTxBuff+3, (u8*)&gMbTxBuff+3+1);
//			gFrameTx.cnt = 5;
//			gFrameTx.msg = (u8*)&gMbTxBuff;
//			FM11NC08FrameTx(&gFrameTx, 100);
//	        gCommPoll.timeout = 0;
//	        gCommPoll.recflag = 1;
//			PT_WAIT_UNTIL(&gPTComm, (BSPTaskStart(TASK_ID_COMM_TASK, 2)) & (gFrameTx.sta > 0));
//		}
//		else if((gFrameRec.cnt>6)&&(gFrameRec.sta==1)&&(CommStringComp(cPDUHead, &gMbRxBuff.pcb, 4)))//符合Modbus协议帧
//		{
//			//modbus协议处理
//			gPduLen = MBServerMsgProcess(&gMbRxBuff,&gMbTxBuff);
//		}
//        else
//        {
//            gPduLen = 0;
//        }

//		if(gPduLen!= 0)
//		{
//			CommCalcCRC(COMM_CRC_A , (u8*)&gMbTxBuff, gPduLen, (u8*)&gMbTxBuff+gPduLen, (u8*)&gMbTxBuff+gPduLen+1);
//			gPduLen += 2;
//			gFrameTx.cnt = gPduLen;
//			gFrameTx.msg = (u8*)&gMbTxBuff;

//			FM11NC08FrameTx(&gFrameTx, 100);
//	        gCommPoll.timeout = 0;
//	        gCommPoll.recflag = 1;
//			PT_WAIT_UNTIL(&gPTComm, (BSPTaskStart(TASK_ID_COMM_TASK, 2)) & (gFrameTx.sta > 0));
//		}
//    }

//    PT_END(&gPTComm);
}

//=============================================================================================
//函数名称	: u8 CommCompareTimeout(u16 timeout)
//输入参数	: timeout:超时时间
//输出参数	: void
//静态变量	: void
//功    能	: 通信协议检测超时时间
//注    意	:
//=============================================================================================
u8 CommCompareTimeout(u16 timeout)
{
    return gCommPoll.timeout >= (timeout / 10);
}

//=============================================================================================
//函数名称	: void CommCheckTimeout(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 通信协议断线超时任务
//注    意	:
//=============================================================================================
void CommCheckOffTimeout(void)
{
    if(gCommPoll.timeout < 60000)
    {
        gCommPoll.timeout++;
    }

    if(gCommPoll.timeout > 1600)
    {
    	gCommPoll.commoff = 1;	//无通信16s，判断为断线
    }
	else
	{
		gCommPoll.commoff = 0;	//无通信16s，判断为断线
	}
}

//=============================================================================================
//函数名称	: void CommClearTimeout(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 通信协议清零超时计数器
//注    意	:
//=============================================================================================
void CommClearTimeout(void)
{
    gCommPoll.timeout = 0;
}

//=============================================================================================
//函数名称	: void CommClearCtrl(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 通信协议清除通信控制
//注    意	:
//=============================================================================================
void CommClearCtrl(void)
{
    gMBCtrl.ctrl = 0;
    gCommPoll.recflag = 0;
}

//=============================================================================================
//函数名称	: u16 CommUpdateCRC(u8 ch, u16 *pCrc)
//输入参数	: ch:输入的单个字符   pCrc：需要放置crc的数据指针
//输出参数	: void
//函数功能	: NFC通信更新CRC
//注意事项	:
//=============================================================================================
u16 CommUpdateCRC(u8 ch, u16 *pCrc)
{
	ch = (ch ^ (u8)((*pCrc) & 0x00FF));
	ch = (ch ^ (ch << 4));

	*pCrc = (*pCrc >> 8) ^ ((u16)ch << 8) ^ ((u16)ch << 3) ^ ((u16)ch >> 4);
	return(*pCrc);
}

//=============================================================================================
//函数名称	: u16 CommCalcCRC(int CRCType,u8 *Data, u16 Length,u8 *TransmitFirst, u8 *TransmitSecond)
//输入参数	: CRCType：CRC计算类型；Data：数组指针；Length：计算的长度；TransmitFirst：CRC低字节；TransmitSecond：CRC高字节
//输出参数	: CRC:计算的CRC
//函数功能	: NFC通信计算CRC
//注意事项	:
//=============================================================================================
u16 CommCalcCRC(int CRCType,u8 *Data, u16 Length,u8 *TransmitFirst, u8 *TransmitSecond)
{
	u8 chBlock;
	u16 wCrc;

	switch(CRCType)
	{
		// ITU-V.41	crc计算初值
		case COMM_CRC_A:
			wCrc = 0x6363;
		break;

		// ISO 3309	crc计算初值
		case COMM_CRC_B:
			wCrc = 0xFFFF;
		break;

		default:
		return 0;
	}

	do
	{
		chBlock = *Data++;
		CommUpdateCRC(chBlock, &wCrc);
	} while (--Length);

	if (COMM_CRC_B == CRCType)
	{
		wCrc = ~wCrc; // ISO 3309
	}

	if(0 != TransmitFirst)
	{
		*TransmitFirst = (u8) (wCrc & 0xFF);
	}

	if(0 != TransmitSecond)
	{
		*TransmitSecond = (u8) ((wCrc >> 8) & 0xFF);
	}

	return wCrc;
}

//=============================================================================================
//函数名称	: u8 CommCheckCRCA(u8 *msg, u8 cnt)
//输入参数	: msg：比较的数据指针；cnt：比较的数据的长度
//输出参数	: TRUE：CRC正确；FALSE：CRC错误
//函数功能	: NFC通信检测CRC A方式是否正确
//注意事项	:
//=============================================================================================
u8 CommCheckCRCA(u8 *msg, u8 cnt)
{
    u8 crc1,crc2;

    CommCalcCRC(COMM_CRC_A , msg, cnt, &crc1, &crc2);
    if((crc1 != msg[cnt]) || (crc2 != msg[cnt+1]))
    {
        return FALSE;
    }
    return TRUE;
}

//=============================================================================================
//函数名称	: u8 CommStringComp(const u8 *msg1, const u8 *msg2, u16 cnt)
//输入参数	: msg1：比较字符串1；msg2：比较的字符串2；cnt：比较的长度
//输出参数	: TRUE：比较正确；FALSE：比较错误
//函数功能	: 字符串比较
//注意事项	:
//=============================================================================================
u8 CommStringComp(const u8 *msg1, const u8 *msg2, u16 cnt)
{
    while(0 < cnt--)
    {
        if(*(msg1++) != *(msg2++))
        {
            return FALSE;
        }
    }
    return TRUE;
}

//=============================================================================================
//函数名称	: u8 CommStringComp(const u8 *msg1, const u8 *msg2, u16 cnt)
//输入参数	: dest：目标字符串地址； *sour：源字符串地址；cnt：复制的长度
//输出参数	:
//函数功能	: 字符串复制
//注意事项	:
//=============================================================================================
void CommStringCopy(u8 *dest, const u8 *sour, u16 cnt)
{
    while(0 < cnt--)
    {
       *(dest++) = *(sour++);
    }
}

//=============================================================================================
//函数名称	: u16 CommBigEndian16Get(u8 *msg)
//输入参数	: msg：要提取的字符串地址
//输出参数	: 提取的数据
//函数功能	: 在字符串中提取大端模式的16位数据
//注意事项	:
//=============================================================================================
u16 CommBigEndian16Get(u8 *msg)
{
    return (((u16)(*msg) << 8) | (*(msg + 1)));
}

//=============================================================================================
//函数名称	: void CommBigEndian16Put(u8 *msg, u16 data)
//输入参数	: msg：放置的地址；data：放置的数据
//输出参数	:
//函数功能	: 向字符串中输出大端模式的16位数据
//注意事项	:
//=============================================================================================
void CommBigEndian16Put(u8 *msg, u16 data)
{
    *msg = (u8)(data >> 8);
    *(msg + 1) = (u8)(data & 0xff);
}

//=============================================================================================
//函数名称	: u32 CommBigEndian32Get(u8 *msg)
//输入参数	: msg：要提取的字符串地址
//输出参数	: 提取的数据
//函数功能	: 在字符串中提取大端模式的32位数据
//注意事项	:
//=============================================================================================
u32 CommBigEndian32Get(u8 *msg)
{
    return (((u32)(*msg) << 24) |((u32)(*(msg + 1)) << 16)|((u32)(*(msg + 2)) << 8)|(*(msg + 3)));
}

//=============================================================================================
//函数名称	: void CommBigEndian32Put(u8 *msg, u32 data)
//输入参数	: msg：放置的地址；data：放置的数据
//输出参数	:
//函数功能	: 向字符串中输出大端模式的32位数据
//注意事项	:
//=============================================================================================
void CommBigEndian32Put(u8 *msg, u32 data)
{
    *msg = (u8)(data >> 24);
    *(msg + 1) = (u8)(data >> 16);
    *(msg + 2) = (u8)(data >> 8);
    *(msg + 3) = (u8)(data & 0xff);
}

/*****************************************end of Comm.c*****************************************/
