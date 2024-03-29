//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: HMAC.h
//创建人  	: Handry
//创建日期	: 
//描述	    : 加密处理头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef HMAC_H
#define HMAC_H

#ifndef HMAC_C
#define GLOBAL_HMAC extern
#else
#define GLOBAL_HMAC
#endif

//=============================================================================================
//头文件
//=============================================================================================
#include <stddef.h>
#include <stdint.h>
#include "BSPTypeDef.h"

//=============================================================================================
//宏定义
//=============================================================================================
#define     MAX_CERTIFI_HANDLE      3   /*最大处理任务接口*/
#define     SHA256_BLOCKSIZE        64

//=============================================================================================
//数据结构定义
//=============================================================================================
/* 验证函数处理结构体 */
typedef struct
{
    u8 funcode;
    void (*handle)(u8 *rx_buf, u8 *tx_buf);
}t_CERTIFI_FUN;

/* 验证信息结构体 */
typedef struct
{
    u8 IsCertifi;
    u8 certifi_cmd_len;
    u8 certifi_cmd_ack;
    
    u8 certifi_key_index;
    u8 random_no[4];
    u8 degist[32];

}t_CERTIFI;

//=============================================================================================
//变量声明，供外部使用
//=============================================================================================
extern const t_CERTIFI_FUN c_CERTIFI_FUN[MAX_CERTIFI_HANDLE];

extern t_CERTIFI certifi_tcb;

//=============================================================================================
//声明接口函数
//=============================================================================================

//=============================================================================================
//函数名称: void HMACBmsCertifiStart(u8 * rx_buf, u8 * tx_buf)
//输	入: rx_buf:接收的modbus数据；tx_buf：要发送的modubs缓存数据
//输	出: void
//静态变量:
//功	能: BMS认证开始
//=============================================================================================
void HMACBmsCertifiStart(u8 * rx_buf, u8 * tx_buf);

//=============================================================================================
//函数名称: void HMACBmsCertifiDigest(u8 * rx_buf, u8 * tx_buf)
//输	入: rx_buf:接收的modbus数据；tx_buf：要发送的modubs缓存数据
//输	出: void
//静态变量:
//功	能: BMS认证过程
//=============================================================================================
void HMACBmsCertifiDigest(u8 * rx_buf, u8 * tx_buf);

//=============================================================================================
//函数名称: void HMACPmsCertifiStart(u8 * rx_buf, u8 * tx_buf)
//输	入: rx_buf:接收的modbus数据；tx_buf：要发送的modubs缓存数据
//输	出: void
//静态变量:
//功	能: PMS认证开始
//=============================================================================================
void HMACPmsCertifiStart(u8 * rx_buf, u8 * tx_buf);

//=============================================================================================
//函数名称: u8 HMACClearCertifiSta(void)
//输	入: void
//输	出: void
//静态变量:
//功	能: 清除认证结果
//=============================================================================================
void HMACClearCertifiSta(void);

//=============================================================================================
//函数名称: u8 HMACReadCertifiSta(void)
//输	入: void
//输	出: void
//静态变量:
//功	能: 获取是否需要认证，认证结果
//=============================================================================================
u8 HMACReadCertifiSta(void);

//=============================================================================================
//函数名称: unsigned char HMACSha256Proc(const unsigned char * key, unsigned int key_len, const unsigned char * input, unsigned int ilen, unsigned char output [ 32 ])
//输	入: key：密钥；key_len：密钥长度；input：输入数据；ilen：输入长度；output：输出的摘要
//输	出: void
//静态变量:
//功	能: SHA256算法计算摘要
//=============================================================================================
unsigned char HMACSha256Proc(const unsigned char * key, unsigned int key_len, const unsigned char * input, unsigned int ilen, unsigned char output [ 32 ]);
    
#endif

/*****************************************end of HMAC.h*****************************************/
