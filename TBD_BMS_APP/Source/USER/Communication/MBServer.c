//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: MBServer.c
//创建人  	: Handry
//创建日期	: 
//描述	    : Modbus从机处理服务代码
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//头文件
//=============================================================================================
#include "MBServer.h"
#include "MBFindMap.h"
#include "CommCtrl.h"
#include "Storage.h"
#include "EventRecord.h"
#include "HMAC.h"
#include "CanComm.h"
//#include "BSPCan.h"
#include "BSP_CAN.h"
//=============================================================================================
//定义全局变量
//=============================================================================================
//CRC 循环冗余表
const u8 auchCRCHi_exp[] = {
    0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81,
	0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0,
	0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01,
	0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41,
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81,
	0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0,
	0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01,
	0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40,
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81,
	0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0,
	0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01,
	0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41,
	0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81,
	0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0,
	0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01,
	0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41,
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81,
	0x40
};

const  u8 auchCRCLo_exp[] = {
	0x00, 0xc0, 0xc1, 0x01, 0xc3, 0x03, 0x02, 0xc2, 0xc6, 0x06, 0x07, 0xc7, 0x05, 0xc5, 0xc4,
	0x04, 0xcc, 0x0c, 0x0d, 0xcd, 0x0f, 0xcf, 0xce, 0x0e, 0x0a, 0xca, 0xcb, 0x0b, 0xc9, 0x09,
	0x08, 0xc8, 0xd8, 0x18, 0x19, 0xd9, 0x1b, 0xdb, 0xda, 0x1a, 0x1e, 0xde, 0xdf, 0x1f, 0xdd,
	0x1d, 0x1c, 0xdc, 0x14, 0xd4, 0xd5, 0x15, 0xd7, 0x17, 0x16, 0xd6, 0xd2, 0x12, 0x13, 0xd3,
	0x11, 0xd1, 0xd0, 0x10, 0xf0, 0x30, 0x31, 0xf1, 0x33, 0xf3, 0xf2, 0x32, 0x36, 0xf6, 0xf7,
	0x37, 0xf5, 0x35, 0x34, 0xf4, 0x3c, 0xfc, 0xfd, 0x3d, 0xff, 0x3f, 0x3e, 0xfe, 0xfa, 0x3a,
	0x3b, 0xfb, 0x39, 0xf9, 0xf8, 0x38, 0x28, 0xe8, 0xe9, 0x29, 0xeb, 0x2b, 0x2a, 0xea, 0xee,
	0x2e, 0x2f, 0xef, 0x2d, 0xed, 0xec, 0x2c, 0xe4, 0x24, 0x25, 0xe5, 0x27, 0xe7, 0xe6, 0x26,
	0x22, 0xe2, 0xe3, 0x23, 0xe1, 0x21, 0x20, 0xe0, 0xa0, 0x60, 0x61, 0xa1, 0x63, 0xa3, 0xa2,
	0x62, 0x66, 0xa6, 0xa7, 0x67, 0xa5, 0x65, 0x64, 0xa4, 0x6c, 0xac, 0xad, 0x6d, 0xaf, 0x6f,
	0x6e, 0xae, 0xaa, 0x6a, 0x6b, 0xab, 0x69, 0xa9, 0xa8, 0x68, 0x78, 0xb8, 0xb9, 0x79, 0xbb,
	0x7b, 0x7a, 0xba, 0xbe, 0x7e, 0x7f, 0xbf, 0x7d, 0xbd, 0xbc, 0x7c, 0xb4, 0x74, 0x75, 0xb5,
	0x77, 0xb7, 0xb6, 0x76, 0x72, 0xb2, 0xb3, 0x73, 0xb1, 0x71, 0x70, 0xb0, 0x50, 0x90, 0x91,
	0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9c, 0x5c,
	0x5d, 0x9d, 0x5f, 0x9f, 0x9e, 0x5e, 0x5a, 0x9a, 0x9b, 0x5b, 0x99, 0x59, 0x58, 0x98, 0x88,
	0x48, 0x49, 0x89, 0x4b, 0x8b, 0x8a, 0x4a, 0x4e, 0x8e, 0x8f, 0x4f, 0x8d, 0x4d, 0x4c, 0x8c,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
	0x40
};

//=============================================================================================
//声明静态函数
//=============================================================================================
//===========================================================================
//函数名称	: static u8 MBServerFuncCodeHandler(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg, u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 获得响应标识
//注意事项	:
//===========================================================================
static u8 MBServerFuncCodeHandler(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg, u8 *pdulen);

//===========================================================================
//函数名称	: static u8 MBServerFuncCodeHandlerEx(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg, u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 获得响应标识
//注意事项	: 此处用于Can通信下调用
//===========================================================================
static u8 MBServerFuncCodeHandlerEx(t_Modbus_PDU *PmbRxMsg,t_Modbus_PDU *PmbTxMsg, u8 *pdulen);

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void MBServerMsgProcess(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；
//输出参数	: 发送缓冲的数据长度
//静态变量	: void
//功    能	: Modbus作从机的处理函数
//注    意	:
//=============================================================================================
u8 MBServerMsgProcess(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg)
{
	u8 excode = MODBUS_NO_FAULT;	//modbus 错误码
	u8 txlen = 0;

	excode = MBServerFuncCodeHandler(PmbRxMsg,PmbTxMsg,&txlen);	//根据功能码，填充发送数组,发送数组长度

    if(0 != PmbRxMsg->addr)//非全局地址返回应答
    {
		PmbTxMsg->pcb = PmbRxMsg->pcb;	//复制nfc协议头数据到发送头中
		PmbTxMsg->cid = PmbRxMsg->cid;
		PmbTxMsg->nad = PmbRxMsg->nad;
		PmbTxMsg->addr = PmbRxMsg->addr;

        if(MODBUS_NO_FAULT != excode)	//发生错误
        {
        	PmbTxMsg->funcode = PmbRxMsg->funcode + 0x80;
        	PmbTxMsg->data[0] = excode;
        	txlen = 6;
        }
        else
        {
        	PmbTxMsg->funcode = PmbRxMsg->funcode;
        	txlen += 5;
        }
    }
    else//全局地址不返回应答
    {
    	txlen = 0;
    }

    return txlen;
}

//=============================================================================================
//函数名称	: void MBServerMsgProcessEx(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；
//输出参数	: 发送缓冲的数据长度
//静态变量	: void
//功    能	: Modbus作从机的处理函数
//注    意	: 此处用于Can通信下调用
//=============================================================================================
u8 MBServerMsgProcessEx(t_Modbus_PDU *PmbRxMsg,t_Modbus_PDU *PmbTxMsg)
{
	u8 excode = MODBUS_NO_FAULT;	//modbus 错误码
	u8 txlen = 0;

	excode = MBServerFuncCodeHandlerEx(PmbRxMsg,PmbTxMsg,&txlen);	//根据功能码，填充发送数组,发送数组长度

    if(0 != PmbRxMsg->addr)//非全局地址返回应答
    {
//		PmbTxMsg->pcb = PmbRxMsg->pcb;	//复制nfc协议头数据到发送头中
//		PmbTxMsg->cid = PmbRxMsg->cid;
//		PmbTxMsg->nad = PmbRxMsg->nad;
		PmbTxMsg->addr = PmbRxMsg->addr;

        if(MODBUS_NO_FAULT != excode)	//发生错误
        {
        	PmbTxMsg->funcode = PmbRxMsg->funcode + 0x80;
        	PmbTxMsg->data[0] = excode;
        	txlen = 3;
        }
        else
        {
        	PmbTxMsg->funcode = PmbRxMsg->funcode;
        	txlen += 2;
        }
    }
    else//全局地址不返回应答
    {
    	txlen = 0;
    }

    return txlen;
}

//===========================================================================
//函数名称	: u16 MbRtuCheckCode(u8 *chkbuf, u8 len)
//输入参数	: chkbuf: 指向需要验证的ModBus消息帧的指针
//			: len   : 要效验的Modbus消息帧字节数，不包含效验码CRC
//输出参数	: UINT16: 校验码CRC	
//静态变量	: 
//功	能	: 计算并返回ModBus RTU模式下消息的CRC效验码
//注	意	:  
//===========================================================================
u16 MbRtuCheckCode(u8 *chkbuf, u8 len)
{
	u8   uchCRCHi = 0xff;   //CRC高字节初始化
	u8   uchCRCLo = 0xff;   //CRC低字节初始化
	u16  uIndex;            //查询表索引
	//u16  temp_code;

	while (len)
	{
		uIndex = (u16)(uchCRCHi ^ *chkbuf++);  
		uchCRCHi = (u8)(uchCRCLo ^ auchCRCHi_exp[uIndex]);
		uchCRCLo = auchCRCLo_exp[uIndex];
		len--;
	}
	
	//temp_code = (u16)uchCRCHi;
	//temp_code = (u16)(temp_code << 8);
	
	//return(u16)(temp_code | uchCRCLo);
    return (u16)uchCRCHi | (u16)uchCRCLo << 8;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCode03Proc(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0x03的处理函数
//注意事项	:
//===========================================================================
static u8 MBServerFuncCode03Proc(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
{
    u8 area = 0;					//操作区域
    const t_MBREG * pfunc;			//操作函数的指针
    u16 regstartaddr = 0;			//寄存器起始地址
	u16 regnum = 0;					//寄存器数目
	u16 regendaddr = 0;				//寄存器结束地址
    u16 tempdata = 0;				//填充的暂存数据
    u8 *pwirte;						//写指针
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码

	//获取查询的寄存器起始地址、结束地址和寄存器数目
	regstartaddr = ((u16)(PmbRxMsg->data[0]) << 8) | (PmbRxMsg->data[1]);
	regnum  = ((u16)(PmbRxMsg->data[2]) << 8) | (PmbRxMsg->data[3]);
	regendaddr = regstartaddr + regnum - 1;

	//判断输入数量是否超出允许范围
	if(regnum > MODBUS_ONETIME_MAX_NUM)
	{
		//超出允许范围，返回请求数量错误异常码
        *pdulen = 0;
		excode = ILLEGAL_DATA_VALUE;
		return excode;
	}

	//查找地址所在信息区域
    for(area = 0;area < MB_AREA_READ_NUM; area++)
    {
        if((cMbRegReadStart[area] <= regstartaddr) && (cMbRegReadEnd[area] >= regstartaddr))
        {
        	//找到区域后，即退出当前查找
            break;
        }
    }

    //检查地址范围,是否超出有效地址的范围
    if((area >= MB_AREA_READ_NUM)
    	||(cMbRegReadStart[area] > regendaddr)
		||(cMbRegReadEnd[area] < regendaddr))
    {
    	//地址错误
        *pdulen = 0;
    	excode = ILLEGAL_DATA_ADDRESS;
    	return excode;
    }

    PmbTxMsg->data[0] = regnum * 2;
    pwirte = &(PmbTxMsg->data[1]);		//写指针位置
    pfunc = cMBReadRegFunc[area] + regstartaddr - cMbRegReadStart[area];	//寻找操作函数的指针

	for ( ; regstartaddr <= regendaddr; regstartaddr++)
	{
        if((NULL == (*(pfunc))) || (!((*(pfunc))(regstartaddr,&tempdata))))		//执行操作函数
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }

        //填充数据
        *pwirte = (u8)(tempdata >> 8);
        *(pwirte + 1) = (u8)(tempdata & 0xff);

        pfunc++;		//下一个地址
        pwirte += 2;	//发送数组指针偏移+2
    }

    *pdulen = regnum * 2 + 1;	//发送的数据区长度，加上一字节的长度

    return excode;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCode06Proc(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0x06的处理函数
//注意事项	:
//===========================================================================
static u8 MBServerFuncCode06Proc(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
{
    u8 area = 0;					//操作区域
    const t_MBREG * pfunc;			//操作函数的指针
    u16 regstartaddr = 0;			//寄存器起始地址
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码
	u16 tempdata = 0;				//写暂存数据

	//获取查询的寄存器起始地址、结束地址和寄存器数目
	regstartaddr = ((u16)(PmbRxMsg->data[0]) << 8) | (PmbRxMsg->data[1]);

	//查找地址所在信息区域
    for(area = 0;area < MB_AREA_READ_NUM; area++)
    {
        if(cMbRegWriteStart[area] <= regstartaddr && cMbRegWriteEnd[area] >= regstartaddr)
        {
        	//找到区域后，即退出当前查找
            break;
        }
    }

    //检查地址范围,是否超出有效地址的范围
    if(area >= MB_AREA_READ_NUM)
    {
    	//地址错误
        *pdulen = 0;
    	excode = ILLEGAL_DATA_ADDRESS;
    	return excode;
    }

    //检查是否可操作配置寄存器
    if(0 == (gNVMPermitCmd & NVM_PERMIT_CMD_EN))
    {
        if((1 == area) && (MB_PARA_CFG_START != regstartaddr))
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }
    }

    //检查是否允许写用户寄存器
    if((0 == (gUserPermitCmd &gUserPermitCmd)))
    {
        if((2 == area) && ((MB_USER_DATA_DATA_START != regstartaddr)))
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }
    }

    tempdata = ((u16)(PmbRxMsg->data[2]) << 8) | (PmbRxMsg->data[3]); 		//要写入的数据
    pfunc = cMBWriteRegFunc[area] + regstartaddr - cMbRegWriteStart[area];	//寻找操作函数的指针

    if(NULL == (*(pfunc)) || !((*(pfunc))(regstartaddr,&tempdata)))			//执行操作函数
    {
    	//从设备操作故障
        *pdulen = 0;
    	excode = CHILD_DEVICE_MALFUNCTION;
    	return excode;
    }

    //填充发送数据
    PmbTxMsg->data[0] = PmbRxMsg->data[0];
    PmbTxMsg->data[1] = PmbRxMsg->data[1];
    PmbTxMsg->data[2] = PmbRxMsg->data[2];
    PmbTxMsg->data[3] = PmbRxMsg->data[3];
    *pdulen = 4;	//长度不包括功能码

    return excode;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCode16Proc(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0x10的处理函数
//注意事项	:
//===========================================================================
static u8 MBServerFuncCode16Proc(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
{
    u8 area = 0;					//操作区域
    const t_MBREG * pfunc;			//操作函数的指针
    u16 regstartaddr = 0;			//寄存器起始地址
	u16 regnum = 0;					//寄存器数目
	u16 regendaddr = 0;				//寄存器结束地址
	u8  bytecnt;					//写入寄存器数据的长度
    u16 tempdata = 0;				//填充的暂存数据
    u8 *pwirte;						//写指针
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码

	//获取查询的寄存器起始地址、结束地址和寄存器数目
	regstartaddr = ((u16)(PmbRxMsg->data[0]) << 8) | (PmbRxMsg->data[1]);
	regnum  = ((u16)(PmbRxMsg->data[2]) << 8) | (PmbRxMsg->data[3]);
	regendaddr = regstartaddr + regnum - 1;
	bytecnt = PmbRxMsg->data[4];

	//判断输入数量是否超出允许范围
	if((regnum > MODBUS_ONETIME_MAX_NUM) || (bytecnt != (regnum * 2)))
	{
		//超出允许范围，返回请求数量错误异常码
        *pdulen = 0;
		excode = ILLEGAL_DATA_VALUE;
		return excode;
	}

	//查找地址所在信息区域
    for(area = 0;area < MB_AREA_READ_NUM; area++)
    {
        if(cMbRegWriteStart[area] <= regstartaddr && cMbRegWriteEnd[area] >= regstartaddr)
        {
        	//找到区域后，即退出当前查找
            break;
        }
    }

    //检查地址范围,是否超出有效地址的范围
    if((area >= MB_AREA_READ_NUM)
    	||(cMbRegWriteStart[area] > regendaddr)
		||(cMbRegWriteEnd[area] < regendaddr))
    {
    	//地址错误
        *pdulen = 0;
    	excode = ILLEGAL_DATA_ADDRESS;
    	return excode;
    }

    //检查是否可操作配置寄存器
    if(0 == (gNVMPermitCmd & NVM_PERMIT_CMD_EN))
    {
        if((1 == area) && ((MB_PARA_CFG_START != regstartaddr) || (1 != regnum)))
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }
    }

    //检查是否允许写用户寄存器
    if((0 == (gUserPermitCmd &gUserPermitCmd)))
    {
        if((2 == area) && ((MB_USER_DATA_DATA_START != regstartaddr) || (1 != regnum)))
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }
    }

    pfunc = cMBWriteRegFunc[area] + regstartaddr - cMbRegWriteStart[area];	//寻找操作函数的指针
    pwirte = &PmbRxMsg->data[5];

    //执行写操作
	for ( ; regstartaddr <= regendaddr; regstartaddr++)
	{
        //获取数据
		tempdata = ((u16)(*pwirte) << 8) | (*(pwirte + 1));

        if(NULL == (*(pfunc)) || !((*(pfunc))(regstartaddr,&tempdata)))		//执行操作函数
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }

        pfunc++;		//下一个地址
        pwirte += 2;	//发送数组指针偏移+2
    }
    //填充发送数据
    PmbTxMsg->data[0] = PmbRxMsg->data[0];
    PmbTxMsg->data[1] = PmbRxMsg->data[1];
    PmbTxMsg->data[2] = PmbRxMsg->data[2];
    PmbTxMsg->data[3] = PmbRxMsg->data[3];
    *pdulen = 4;	//长度不包括功能码

    return excode;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCode70Proc(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0x46的处理函数
//注意事项	:
//===========================================================================
static u8 MBServerFuncCode70Proc(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
{
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码

	u32 regstartaddr = 0;			//寄存器起始地址,32位

	regstartaddr = (((u32)(PmbRxMsg->data[0]) << 24) |
					((u32)(PmbRxMsg->data[1]) << 16) |
					((u32)(PmbRxMsg->data[2]) << 8)	 |
					(PmbRxMsg->data[3]));

    if((regstartaddr < 1) && (regstartaddr > RECORD_MAX_NUM))
    {
    	//地址错误
        *pdulen = 0;
    	excode = ILLEGAL_DATA_ADDRESS;
    	return excode;
    }
    
    gStorageCTRL |= FLASH_RD_EVE_MASK;
    gPowOffTim = 0;
    
    if(0 == gFlashPowSta)
    {
        StoragePowerOn();
        gFlashPowSta = 1;
    }

    //此处读取事件记录
    gRecordRd.num = regstartaddr;
    if((gRecordRd.num >=1) &&(gRecordRd.num <=RECORD_MAX_NUM))
    {
//        MEM_RW_ADDR = (gRecordRd.num - 1)%RECORD_MAX_ITEM*RECORD_SIZE + RECORD_START_ADDR;
//        MEM_RW_CNT = sizeof(t_RECORD);
//        MEM_RW_STRUCT = STOR_BUFFER;
//        PT_SPAWN_S(&ptSTOR,&sub,mem_read);
        StorageReadFlash((gRecordRd.num - 1)%RECORD_MAX_ITEM*RECORD_SIZE + RECORD_START_ADDR,gStorageBuff,sizeof(t_RECORD));
        gRecordRd = *(t_RECORD*)gStorageBuff;
        gStorageErr &= ~(0x03<<6);
    }
    else
    {
    	gStorageErr &= ~(0x03<<6);
        gStorageErr |= (1<<6);
    }

    StorageStringCopy(&PmbTxMsg->data[0],(uint8_t*)&gRecordRd,128);
    *pdulen = 128;
    excode = 0;

    return excode;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCode104Proc(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0x68的处理函数
//注意事项	: customer CMD: certificate
//===========================================================================
static u8 MBServerFuncCode104Proc(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
{
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码
    static u8 i;    
    
    for(i = 0;i < MAX_CERTIFI_HANDLE;i++)
    {
        if(PmbRxMsg->data[0] == c_CERTIFI_FUN[i].funcode)
        break;
    }    
    
    if(i < MAX_CERTIFI_HANDLE)
    {
        c_CERTIFI_FUN[i].handle(PmbRxMsg->data,PmbTxMsg->data);
    }
    else
    {
        certifi_tcb.certifi_cmd_len = 1;    //false
        certifi_tcb.certifi_cmd_ack = 0;    //false
    }    
    
    PmbTxMsg->data[0] = PmbRxMsg->data[0];  //certifi cmd
    PmbTxMsg->data[1] = certifi_tcb.certifi_cmd_len;
    PmbTxMsg->data[2] = certifi_tcb.certifi_cmd_ack;
    *pdulen = certifi_tcb.certifi_cmd_len + 2;
    excode = 0;   
    
    return excode;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCodeHandler(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg, u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 获得响应标识
//注意事项	:
//===========================================================================
static u8 MBServerFuncCodeHandler(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg, u8 *pdulen)
{
	u8 rspSelection;//正常应答还是异常应答标识

	switch(PmbRxMsg->funcode)
	{
		//读寄存器   0x03
		case MODBUS_READ_HOLDREGISTERS:
			rspSelection = MBServerFuncCode03Proc(PmbRxMsg,PmbTxMsg,pdulen);
			break;

		//写单寄存器 0x06
		case MODBUS_WRITE_REGISTER :
			rspSelection = MBServerFuncCode06Proc(PmbRxMsg,PmbTxMsg,pdulen);
			break;

		//写多寄存器 0x10
		case MODBUS_WRITE_MULT_REG:
			rspSelection = MBServerFuncCode16Proc(PmbRxMsg,PmbTxMsg,pdulen);
			break;

		//读用户寄存器，读历史记录 0x46
		case MODBUS_CUSTOMER_REG:
			rspSelection = MBServerFuncCode70Proc(PmbRxMsg,PmbTxMsg,pdulen);
			break;
        
		//读认证寄存器，用于电池加密认证
		case MODBUS_CERTIFICATE_REG:
			rspSelection = MBServerFuncCode104Proc(PmbRxMsg,PmbTxMsg,pdulen);
			break;        

		default:
			rspSelection = ILLEGAL_FUNCTION_CODE;
			break;
	}

	return rspSelection;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCode03ProcEx(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0x03的处理函数
//注意事项	: 此处用于Can通信下调用
//===========================================================================
static u8 MBServerFuncCode03ProcEx(t_Modbus_PDU *PmbRxMsg,t_Modbus_PDU *PmbTxMsg,u8 *pdulen)
{
    u8 area = 0;					//操作区域
    const t_MBREG * pfunc;			//操作函数的指针
    u16 regstartaddr = 0;			//寄存器起始地址
	u16 regnum = 0;					//寄存器数目
	u16 regendaddr = 0;				//寄存器结束地址
    u16 tempdata = 0;				//填充的暂存数据
    u8 *pwirte;						//写指针
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码

	//获取查询的寄存器起始地址、结束地址和寄存器数目
	regstartaddr = ((u16)(PmbRxMsg->data[0]) << 8) | (PmbRxMsg->data[1]);
	regnum  = ((u16)(PmbRxMsg->data[2]) << 8) | (PmbRxMsg->data[3]);
	regendaddr = regstartaddr + regnum - 1;

	//判断输入数量是否超出允许范围
	if(regnum > MODBUS_ONETIME_MAX_NUM)
	{
		//超出允许范围，返回请求数量错误异常码
        *pdulen = 0;
		excode = ILLEGAL_DATA_VALUE;
		return excode;
	}

	//查找地址所在信息区域
    for(area = 0;area < MB_AREA_READ_NUM; area++)
    {
        if((cMbRegReadStart[area] <= regstartaddr) && (cMbRegReadEnd[area] >= regstartaddr))
        {
        	//找到区域后，即退出当前查找
            break;
        }
    }

    //检查地址范围,是否超出有效地址的范围
    if((area >= MB_AREA_READ_NUM)
    	||(cMbRegReadStart[area] > regendaddr)
		||(cMbRegReadEnd[area] < regendaddr))
    {
    	//地址错误
        *pdulen = 0;
    	excode = ILLEGAL_DATA_ADDRESS;
    	return excode;
    }

    PmbTxMsg->data[0] = regnum * 2;
    pwirte = &(PmbTxMsg->data[1]);		//写指针位置
    pfunc = cMBReadRegFunc[area] + regstartaddr - cMbRegReadStart[area];	//寻找操作函数的指针

	for ( ; regstartaddr <= regendaddr; regstartaddr++)
	{
        if((NULL == (*(pfunc))) || (!((*(pfunc))(regstartaddr,&tempdata))))		//执行操作函数
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }

        //填充数据
        *pwirte = (u8)(tempdata >> 8);
        *(pwirte + 1) = (u8)(tempdata & 0xff);

        pfunc++;		//下一个地址
        pwirte += 2;	//发送数组指针偏移+2
    }

    *pdulen = regnum * 2 + 1;	//发送的数据区长度，加上一字节的长度

    return excode;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCode06ProcEx(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0x06的处理函数
//注意事项	: 此处用于Can通信下调用
//===========================================================================
static u8 MBServerFuncCode06ProcEx(t_Modbus_PDU *PmbRxMsg,t_Modbus_PDU *PmbTxMsg,u8 *pdulen)
{
    u8 area = 0;					//操作区域
    const t_MBREG * pfunc;			//操作函数的指针
    u16 regstartaddr = 0;			//寄存器起始地址
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码
	u16 tempdata = 0;				//写暂存数据

	//获取查询的寄存器起始地址、结束地址和寄存器数目
	regstartaddr = ((u16)(PmbRxMsg->data[0]) << 8) | (PmbRxMsg->data[1]);

	//查找地址所在信息区域
    for(area = 0;area < MB_AREA_WRITE_NUM; area++)
    {
        if(cMbRegWriteStart[area] <= regstartaddr && cMbRegWriteEnd[area] >= regstartaddr)
        {
        	//找到区域后，即退出当前查找
            break;
        }
    }

    //检查地址范围,是否超出有效地址的范围
    if(area >= MB_AREA_WRITE_NUM)
    {
    	//地址错误
        *pdulen = 0;
    	excode = ILLEGAL_DATA_ADDRESS;
    	return excode;
    }
    
    //检查是否已鉴权，鉴权才可控制
    if(False == HMACReadCertifiSta())
    {
        if((0 == area))
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }
    }
    
    //检查是否可操作配置寄存器
    if(0 == (gNVMPermitCmd & NVM_PERMIT_CMD_EN))
    {
        if((1 == area) && (MB_PARA_CFG_START != regstartaddr))
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }
    }

    //检查是否允许写用户寄存器
    if((0 == (gUserPermitCmd &gUserPermitCmd)))
    {
        if((2 == area) && ((MB_USER_DATA_DATA_START != regstartaddr)))
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }
    }

    tempdata = ((u16)(PmbRxMsg->data[2]) << 8) | (PmbRxMsg->data[3]); 		//要写入的数据
    pfunc = cMBWriteRegFunc[area] + regstartaddr - cMbRegWriteStart[area];	//寻找操作函数的指针

    if(NULL == (*(pfunc)) || !((*(pfunc))(regstartaddr,&tempdata)))			//执行操作函数
    {
    	//从设备操作故障
        *pdulen = 0;
    	excode = CHILD_DEVICE_MALFUNCTION;
    	return excode;
    }

    //填充发送数据
    PmbTxMsg->data[0] = PmbRxMsg->data[0];
    PmbTxMsg->data[1] = PmbRxMsg->data[1];
    PmbTxMsg->data[2] = PmbRxMsg->data[2];
    PmbTxMsg->data[3] = PmbRxMsg->data[3];
    *pdulen = 4;	//长度不包括功能码

    return excode;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCode16ProcEx(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0x10的处理函数
//注意事项	: 此处用于Can通信下调用
//===========================================================================
static u8 MBServerFuncCode16ProcEx(t_Modbus_PDU *PmbRxMsg,t_Modbus_PDU *PmbTxMsg,u8 *pdulen)
{
    u8 area = 0;					//操作区域
    const t_MBREG * pfunc;			//操作函数的指针
    u16 regstartaddr = 0;			//寄存器起始地址
	u16 regnum = 0;					//寄存器数目
	u16 regendaddr = 0;				//寄存器结束地址
	u8  bytecnt;					//写入寄存器数据的长度
    u16 tempdata = 0;				//填充的暂存数据
    u8 *pwirte;						//写指针
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码

	//获取查询的寄存器起始地址、结束地址和寄存器数目
	regstartaddr = ((u16)(PmbRxMsg->data[0]) << 8) | (PmbRxMsg->data[1]);
	regnum  = ((u16)(PmbRxMsg->data[2]) << 8) | (PmbRxMsg->data[3]);
	regendaddr = regstartaddr + regnum - 1;
	bytecnt = PmbRxMsg->data[4];

	//判断输入数量是否超出允许范围
	if((regnum > MODBUS_ONETIME_MAX_NUM) || (bytecnt != (regnum * 2)))
	{
		//超出允许范围，返回请求数量错误异常码
        *pdulen = 0;
		excode = ILLEGAL_DATA_VALUE;
		return excode;
	}

	//查找地址所在信息区域
    for(area = 0;area < MB_AREA_WRITE_NUM; area++)
    {
        if(cMbRegWriteStart[area] <= regstartaddr && cMbRegWriteEnd[area] >= regstartaddr)
        {
        	//找到区域后，即退出当前查找
            break;
        }
    }

    //检查地址范围,是否超出有效地址的范围
    if((area >= MB_AREA_WRITE_NUM)
    	||(cMbRegWriteStart[area] > regendaddr)
		||(cMbRegWriteEnd[area] < regendaddr))
    {
    	//地址错误
        *pdulen = 0;
    	excode = ILLEGAL_DATA_ADDRESS;
    	return excode;
    }
    
    //检查是否已鉴权，鉴权才可控制
    if(False == HMACReadCertifiSta())
    {
        if((0 == area))
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }
    }
    
    //检查是否可操作配置寄存器
    if(0 == (gNVMPermitCmd & NVM_PERMIT_CMD_EN))
    {
        if((1 == area) && ((MB_PARA_CFG_START != regstartaddr) || (1 != regnum)))
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }
    }

    //检查是否允许写用户寄存器
    if((0 == (gUserPermitCmd &gUserPermitCmd)))
    {
        if((2 == area) && ((MB_USER_DATA_DATA_START != regstartaddr) || (1 != regnum)))
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }
    }

    pfunc = cMBWriteRegFunc[area] + regstartaddr - cMbRegWriteStart[area];	//寻找操作函数的指针
    pwirte = &PmbRxMsg->data[5];

    //执行写操作
	for ( ; regstartaddr <= regendaddr; regstartaddr++)
	{
        //获取数据
		tempdata = ((u16)(*pwirte) << 8) | (*(pwirte + 1));

        if(NULL == (*(pfunc)) || !((*(pfunc))(regstartaddr,&tempdata)))		//执行操作函数
        {
        	//从设备操作故障
            *pdulen = 0;
        	excode = CHILD_DEVICE_MALFUNCTION;
        	return excode;
        }

        pfunc++;		//下一个地址
        pwirte += 2;	//发送数组指针偏移+2
    }
    //填充发送数据
    PmbTxMsg->data[0] = PmbRxMsg->data[0];
    PmbTxMsg->data[1] = PmbRxMsg->data[1];
    PmbTxMsg->data[2] = PmbRxMsg->data[2];
    PmbTxMsg->data[3] = PmbRxMsg->data[3];
    *pdulen = 4;	//长度不包括功能码

    return excode;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCode70ProcEx(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0x46的处理函数
//注意事项	: 此处用于Can通信下调用
//===========================================================================
static u8 MBServerFuncCode70ProcEx(t_Modbus_PDU *PmbRxMsg,t_Modbus_PDU *PmbTxMsg,u8 *pdulen)
{
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码

	u32 regstartaddr = 0;			//寄存器起始地址,32位

	regstartaddr = (((u32)(PmbRxMsg->data[0]) << 24) |
					((u32)(PmbRxMsg->data[1]) << 16) |
					((u32)(PmbRxMsg->data[2]) << 8)	 |
					(PmbRxMsg->data[3]));

    if((regstartaddr < 1) && (regstartaddr > RECORD_MAX_NUM))
    {
    	//地址错误
        *pdulen = 0;
    	excode = ILLEGAL_DATA_ADDRESS;
    	return excode;
    }
    
    gStorageCTRL |= FLASH_RD_EVE_MASK;
    gPowOffTim = 0;
    
    if(0 == gFlashPowSta)
    {
        StoragePowerOn();
        gFlashPowSta = 1;
    }

    //此处读取事件记录
    gRecordRd.num = regstartaddr;
    if((gRecordRd.num >=1) &&(gRecordRd.num <=RECORD_MAX_NUM))
    {
//        MEM_RW_ADDR = (gRecordRd.num - 1)%RECORD_MAX_ITEM*RECORD_SIZE + RECORD_START_ADDR;
//        MEM_RW_CNT = sizeof(t_RECORD);
//        MEM_RW_STRUCT = STOR_BUFFER;
//        PT_SPAWN_S(&ptSTOR,&sub,mem_read);
        StorageReadFlash((gRecordRd.num - 1)%RECORD_MAX_ITEM*RECORD_SIZE + RECORD_START_ADDR,gStorageBuff,sizeof(t_RECORD));
        gRecordRd = *(t_RECORD*)gStorageBuff;
        gStorageErr &= ~(0x03<<6);
    }
    else
    {
    	gStorageErr &= ~(0x03<<6);
        gStorageErr |= (1<<6);
    }

    StorageStringCopy(&PmbTxMsg->data[0],(uint8_t*)&gRecordRd,128);
    *pdulen = 128;
    excode = 0;
    gStorageBuff[4] = 0;
    gStorageBuff[5] = 0;
    gStorageBuff[6] = 0;

    return excode;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCode104ProcEx(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0x68的处理函数
//注意事项	: customer CMD: certificate,此处用于Can通信下调用
//===========================================================================
static u8 MBServerFuncCode104ProcEx(t_Modbus_PDU *PmbRxMsg,t_Modbus_PDU *PmbTxMsg,u8 *pdulen)
{
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码
    static u8 i;    
    
    for(i = 0;i < MAX_CERTIFI_HANDLE;i++)
    {
        if(PmbRxMsg->data[0] == c_CERTIFI_FUN[i].funcode)
        break;
    }    
    
    if(i < MAX_CERTIFI_HANDLE)
    {
        c_CERTIFI_FUN[i].handle(PmbRxMsg->data,PmbTxMsg->data);
    }
    else
    {
        certifi_tcb.certifi_cmd_len = 1;    //false
        certifi_tcb.certifi_cmd_ack = 0;    //false
    }    
    
    PmbTxMsg->data[0] = PmbRxMsg->data[0];  //certifi cmd
    PmbTxMsg->data[1] = certifi_tcb.certifi_cmd_len;
    PmbTxMsg->data[2] = certifi_tcb.certifi_cmd_ack;
    *pdulen = certifi_tcb.certifi_cmd_len + 2;
    excode = 0;   
    
    return excode;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCode196ProcEx(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0xC4的处理函数
//注意事项	: customer CMD: certificate,此处用于Can通信下调用
//===========================================================================
static u8 MBServerFuncCode196ProcEx(t_Modbus_PDU *PmbRxMsg,t_Modbus_PDU *PmbTxMsg,u8 *pdulen)
{
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码
    
    #define     RESPOND_ACK     0X00
    #define     RESPOND_NACK    0X01   
    
    //if(memcpy(&PmbRxMsg->data,(u8 *)&(SIM->UIDH),16) && RESPOND_ACK == PmbRxMsg->data[16])  
  if(0 == memcmp(&PmbRxMsg->data,(u8 *)(UID_BASE+8U),16) && RESPOND_ACK == PmbRxMsg->data[16])//取UID 64-79 有可能重复
    {
        *pdulen = 0;
        PmbTxMsg->data[0] = 0;
        gStructCanReq.RxCanId = FlexCanChangeRx3Canid(PmbRxMsg->data[17]);
        gStructCanReq.TxCanId = SWAP16(gStructCanReq.RxCanId);        
        gStructCanReq.SendFlag.SendFlagBit.IsSendReq = False;
    }
    else
    {
        *pdulen = 0;
        excode = ILLEGAL_DATA_ADDRESS;  
    }
    PmbRxMsg->addr = 0;
    
    return excode;
}

//===========================================================================
//函数名称	: static u8 MBServerFuncCode200ProcEx(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0xC8的处理函数
//注意事项	: customer CMD: certificate,此处用于Can通信下调用
//===========================================================================
static u8 MBServerFuncCode200ProcEx(t_Modbus_PDU *PmbRxMsg,t_Modbus_PDU *PmbTxMsg,u8 *pdulen)
{
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码
    
    if(0x01 == PmbRxMsg->data[0] || 0x02 == PmbRxMsg->data[0])
    {
        gStructCanReq.SendFlag.SendFlagBit.IsSendReq = True;
        PmbTxMsg->data[0] = (u8)(gStructCanReq.RxCanId >> 8);
        *pdulen = 1;
    }
    else
    {
        excode = ILLEGAL_DATA_ADDRESS;
        *pdulen = 0;
    }
    
    return excode;
}

#ifdef BMS_ENABLE_NB_IOT
//===========================================================================
//函数名称	: static u8 MBServerFuncCode105ProcEx(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg,u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 功能码0x69的处理函数
//注意事项	: customer CMD: certificate,此处用于Can通信下调用
//===========================================================================
#include "SimCommon.h"
#include "WIFIDeal.h"
static u8 MBServerFuncCode105ProcEx(t_Modbus_PDU *PmbRxMsg,t_Modbus_PDU *PmbTxMsg,u8 *pdulen)
{
    extern void PostMsg(uint8 msgId);
    
	u8 excode = MODBUS_NO_FAULT;	//modbus错误码
    t_WIFI_Coor* pWifiCoor = Null;
    int i = 3;
    
    switch(PmbRxMsg->data[0])
    {
        //读
        case MODBUS_READ_HOLDREGISTERS:
            {
                //返回对应数据
                PmbTxMsg->data[0] = 0x01;
                PmbTxMsg->data[1] = gGSMCond.gsmcsq;
                
                PmbTxMsg->data[2] = QUEUE_getElementCount(&gWifiCoorQueue);
                
                //出队列
                for(pWifiCoor = (t_WIFI_Coor*)QUEUE_getHead(&gWifiCoorQueue); pWifiCoor; QUEUE_removeHead(&gWifiCoorQueue), pWifiCoor = (t_WIFI_Coor*)QUEUE_getHead(&gWifiCoorQueue))
                {
                    memcpy(&PmbTxMsg->data[i], pWifiCoor, sizeof(t_WIFI_Coor));
                    i+= sizeof(t_WIFI_Coor);
                }
                *pdulen = i;
            }
            break;
        
 		//写
		case MODBUS_WRITE_REGISTER:
            if(0x01 == PmbRxMsg->data[1])   //控制进入测试模式
                PostMsg(MSG_PCBA_TEST_START);
            else
                gGSMCond.IsPCBTest = False;
            
            *pdulen = 1; 
            break;
        
        default:
            excode = ILLEGAL_DATA_ADDRESS;
            *pdulen = 0;            
            break;
    }
    
    return excode;
}
#endif

//===========================================================================
//函数名称	: static u8 MBServerFuncCodeHandlerEx(t_MSG_PDU *PmbRxMsg,t_MSG_PDU *PmbTxMsg, u8 *pdulen)
//输入参数	: PmbRxMsg：指向接收缓冲区中请求报文的指针；PmbTxMsg：指向发送缓冲区的指针；pdulen：发送数据长度指针
//输出参数	: u8 返回值标识应该作出何种应答
//静态变量	:
//函数功能	: 获得响应标识
//注意事项	: 此处用于Can通信下调用
//===========================================================================
static u8 MBServerFuncCodeHandlerEx(t_Modbus_PDU *PmbRxMsg,t_Modbus_PDU *PmbTxMsg, u8 *pdulen)
{
	u8 rspSelection;//正常应答还是异常应答标识

	switch(PmbRxMsg->funcode)
	{
		//读寄存器   0x03
		case MODBUS_READ_HOLDREGISTERS:
			rspSelection = MBServerFuncCode03ProcEx(PmbRxMsg,PmbTxMsg,pdulen);
			break;

		//写单寄存器 0x06
		case MODBUS_WRITE_REGISTER :
			rspSelection = MBServerFuncCode06ProcEx(PmbRxMsg,PmbTxMsg,pdulen);
			break;

		//写多寄存器 0x10
		case MODBUS_WRITE_MULT_REG:
			rspSelection = MBServerFuncCode16ProcEx(PmbRxMsg,PmbTxMsg,pdulen);
			break;

		//读用户寄存器，读历史记录 0x46
		case MODBUS_CUSTOMER_REG:
			rspSelection = MBServerFuncCode70ProcEx(PmbRxMsg,PmbTxMsg,pdulen);
			break;
        
		//读认证寄存器，用于电池加密认证
		case MODBUS_CERTIFICATE_REG:
			rspSelection = MBServerFuncCode104ProcEx(PmbRxMsg,PmbTxMsg,pdulen);
			break;        
        
		//申请获取CANID
		case MODBUS_REQUEST_MOD_CANID:
			rspSelection = MBServerFuncCode196ProcEx(PmbRxMsg,PmbTxMsg,pdulen);
			break;        
        
		//主机强制从机重新申请地址
		case MODBUS_FORCE_MOD_CANID:
			rspSelection = MBServerFuncCode200ProcEx(PmbRxMsg,PmbTxMsg,pdulen);
			break;  

        #ifdef BMS_ENABLE_NB_IOT
		//用于NB模块测试的功能码
		case MODBUS_NB_TEST_REG:
			rspSelection = MBServerFuncCode105ProcEx(PmbRxMsg,PmbTxMsg,pdulen);
			break; 
        #endif  
        
		default:
			rspSelection = ILLEGAL_FUNCTION_CODE;
			break;
	}

	return rspSelection;
}

/*****************************************end of MBServer.c*****************************************/

