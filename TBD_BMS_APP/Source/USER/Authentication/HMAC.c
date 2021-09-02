//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: HMAC.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 加密处理文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//头文件
//=============================================================================================
//#include "S32K118.h"
//#include "PCF85063.h"
#include "ParaCfg.h"
#include "sha256.h"
#define HMAC_C
#include "HMAC.h"
//#include "stm32g0xx_hal.h"
#include "BSP_RTC.h"	
#include "BSP_FLASH.h"	

//=============================================================================================
//全局变量定义
//=============================================================================================
/* 定义支持的cmd */
const t_CERTIFI_FUN c_CERTIFI_FUN[MAX_CERTIFI_HANDLE] = 
{
    {0x01,HMACBmsCertifiStart},
    {0x02,HMACBmsCertifiDigest},
    {0x03,HMACPmsCertifiStart}
};

/* 认证控制块 */
t_CERTIFI certifi_tcb;

/* 认证密钥 */
const char *sl_authority_secret = "sb.l34&@,alwvc9^t!sUDRGUN427zosp";
//=============================================================================================
//全局函数声明
//=============================================================================================
//=============================================================================================
//函数名称: unsigned char HMACSha256Proc(const unsigned char * key, unsigned int key_len, const unsigned char * input, unsigned int ilen, unsigned char output [ 32 ])
//输	入: key：密钥；key_len：密钥长度；input：输入数据；ilen：输入长度；output：输出的摘要
//输	出: void
//静态变量:
//功	能: SHA256算法计算摘要
//=============================================================================================
unsigned char HMACSha256Proc(const unsigned char * key, unsigned int key_len, const unsigned char * input, unsigned int ilen, unsigned char output [ 32 ])
#if 1
{
    unsigned char vl_key[SHA256_BLOCKSIZE];
    unsigned char *vl_digest;
    unsigned char i;

    mbedtls_sha256_context ctx;

    vl_digest = (unsigned char *)output;

    memset((char*)vl_key, 0, SHA256_BLOCKSIZE);
    if(key_len > SHA256_BLOCKSIZE)
    {
        //mbedtls_sha256((const unsigned char * )key, key_len, vl_key, 0);

        mbedtls_sha256_init( &ctx );
        if( ( mbedtls_sha256_starts_ret( &ctx, 0 ) ) != 0 )
            return 0;

        if( ( mbedtls_sha256_update_ret( &ctx, key, key_len ) ) != 0 )
            return 0;

        if( ( mbedtls_sha256_finish_ret( &ctx, vl_key ) ) != 0 )
            return 0;
    }
    else
    {
        memcpy((char*)vl_key, key, key_len);
    }
    // return hash(o_key_pad ∥ hash(i_key_pad ∥ message))

    mbedtls_sha256_init( &ctx );
    if( ( mbedtls_sha256_starts_ret( &ctx, 0 ) ) != 0 )
        return 0;

    for(i = 0; i < SHA256_BLOCKSIZE; i++)
    {
        vl_key[i] = 0x36^vl_key[i];
        
    }
    if( ( mbedtls_sha256_update_ret( &ctx, vl_key, SHA256_BLOCKSIZE ) ) != 0 )
            return 0;
    if( ( mbedtls_sha256_update_ret( &ctx, input, ilen ) ) != 0 )
        return 0;

    if( ( mbedtls_sha256_finish_ret( &ctx, vl_digest ) ) != 0 )
        return 0;


    memset((char*)vl_key, 0, SHA256_BLOCKSIZE);
    if(key_len > SHA256_BLOCKSIZE)
    {

        mbedtls_sha256_init( &ctx );
        if( ( mbedtls_sha256_starts_ret( &ctx, 0 ) ) != 0 )
            return 0;

        if( ( mbedtls_sha256_update_ret( &ctx, key, key_len ) ) != 0 )
            return 0;

        if( ( mbedtls_sha256_finish_ret( &ctx, vl_key ) ) != 0 )
            return 0;
    }
    else
    {
        memcpy((char*)vl_key, key, key_len);
    }

    mbedtls_sha256_init( &ctx );
    if( ( mbedtls_sha256_starts_ret( &ctx, 0 ) ) != 0 )
        return 0;

    for(i = 0; i < SHA256_BLOCKSIZE; i++)
    {
        vl_key[i] = 0x5C^vl_key[i];
    }
    
    if( ( mbedtls_sha256_update_ret( &ctx, vl_key, SHA256_BLOCKSIZE ) ) != 0 )
        return 0;
    
    if( ( mbedtls_sha256_update_ret( &ctx, vl_digest, 32 ) ) != 0 )
            return 0;

    if( ( mbedtls_sha256_finish_ret( &ctx, vl_digest ) ) != 0 )
        return 0;
    

    return 1;
}

#else
{
    //
    unsigned char vl_key[SHA256_BLOCKSIZE];
    unsigned char o_key_pad[SHA256_BLOCKSIZE*2];
    unsigned char i_key_pad[SHA256_BLOCKSIZE*2];
    unsigned char i;

    memset((char*)vl_key, 0, SHA256_BLOCKSIZE);
    if(key_len > SHA256_BLOCKSIZE)
    {
        mbedtls_sha256((const unsigned char * )key, key_len, vl_key, 0);
    }
    else
    {
        memcpy((char*)vl_key, key, key_len);
    }
    

    for(i = 0; i < SHA256_BLOCKSIZE; i++)
    {
        o_key_pad[i] = 0x5C^vl_key[i];
        i_key_pad[i] = 0x36^vl_key[i];
    }
    // return hash(o_key_pad ∥ hash(i_key_pad ∥ message))
    memcpy(&i_key_pad[SHA256_BLOCKSIZE], input, ilen);
    mbedtls_sha256((const unsigned char * )i_key_pad, SHA256_BLOCKSIZE + ilen, vl_key, 0);
    memcpy(&o_key_pad[SHA256_BLOCKSIZE], vl_key, 32);
    mbedtls_sha256((const unsigned char * )o_key_pad, SHA256_BLOCKSIZE+32, vl_key, 0);
    memcpy(output, vl_key, 32);
    return 1;
}
#endif

//=============================================================================================
//函数名称: void HMACBmsCertifiStart(u8 * rx_buf, u8 * tx_buf)
//输	入: rx_buf:接收的modbus数据；tx_buf：要发送的modubs缓存数据
//输	出: void
//静态变量:
//功	能: BMS认证开始
//=============================================================================================
void HMACBmsCertifiStart(u8 * rx_buf, u8 * tx_buf)
{
    certifi_tcb.certifi_key_index = rx_buf[2];
    certifi_tcb.random_no[0] = gRdTimeAndDate.day;
    certifi_tcb.random_no[1] = gRdTimeAndDate.hour;
    certifi_tcb.random_no[2] = gRdTimeAndDate.minute;
    certifi_tcb.random_no[3] = gRdTimeAndDate.second;
    tx_buf[3] = certifi_tcb.random_no[0];
    tx_buf[4] = certifi_tcb.random_no[1];
    tx_buf[5] = certifi_tcb.random_no[2];
    tx_buf[6] = certifi_tcb.random_no[3];
    certifi_tcb.certifi_cmd_len = 5;//false
    certifi_tcb.certifi_cmd_ack = 1;//true
}

//=============================================================================================
//函数名称: static void HMACAuthorityGetDigest(unsigned int random_num, unsigned char * out_data)
//输	入: random_num:32位随机数；out_data：输出数据，buff长度32个字节
//输	出: void
//静态变量:
//功	能: 获取摘要
//=============================================================================================
static void HMACAuthorityGetDigest(unsigned int random_num, unsigned char * out_data)
{
    
    unsigned char *pl_temp1;
    unsigned char *pl_temp2;
    unsigned int vl_random_num;
    unsigned short vl_short_temp;
    unsigned char vl_input[32];	
//    u32 uidl = SIM->UIDH;    
//    u32 uidh = SIM->UIDL;
//    u32 uidl = GetMcu_IDL();					//产品唯一身份标识寄存器	r0	-	r31 
//    u32 uidh = GetMcu_IDH(); 					//产品唯一身份标识寄存器  r64	-	r95 
    u32 uidl = GetMcu_IDH();					//产品唯一身份标识寄存器	r0	-	r31 
    u32 uidh = GetMcu_IDL(); 					//产品唯一身份标识寄存器  r64	-	r95 
    memcpy((char*)vl_input, sl_authority_secret, 32);
    
    vl_random_num = random_num;
    pl_temp1 = (unsigned char*)&vl_random_num;
    pl_temp2 = (unsigned char*)&vl_short_temp;
    //vl_short_temp = Battery_get_reg_value(0, ENUM_REG_ADDR_SN12);
    vl_short_temp = gConfig.sn[0];
    
    vl_input[0] += (pl_temp1[1] + pl_temp2[0]);
    vl_input[1] += (pl_temp1[0] + pl_temp2[1]);
    //vl_short_temp = Battery_get_reg_value(0, ENUM_REG_ADDR_MCUN56);
    //vl_short_temp = *((u16*)&(SIM->UIDL)+1);
    vl_short_temp = (u16)(uidl >> 16);
    
    vl_input[2] += (pl_temp1[3] + pl_temp2[0]);
    vl_input[3] += (pl_temp1[2] + pl_temp2[1]);
    //vl_short_temp = Battery_get_reg_value(0, ENUM_REG_ADDR_MCUN12);
    //vl_short_temp = *((u16*)&(SIM->UIDL)+3);
    vl_short_temp = (u16)(uidh >> 16);
    
    vl_input[4] += (pl_temp1[1] + pl_temp2[0]);
    vl_input[5] += (pl_temp1[0] + pl_temp2[1]);
    //vl_short_temp = Battery_get_reg_value(0, ENUM_REG_ADDR_SN34);
    vl_short_temp = gConfig.sn[1];
    
    vl_input[6] += (pl_temp1[3] + pl_temp2[0]);
    vl_input[7] += (pl_temp1[2] + pl_temp2[1]);

    //vl_short_temp = Battery_get_reg_value(0, ENUM_REG_ADDR_MCUN34);
    //vl_short_temp = *((u16*)&(SIM->UIDL)+2);
    vl_short_temp = (u16)(uidh);
    
    vl_input[20] += (pl_temp1[1] + pl_temp2[0]);
    vl_input[21] += (pl_temp1[0] + pl_temp2[1]);
    //vl_short_temp = Battery_get_reg_value(0, ENUM_REG_ADDR_SN78);
    vl_short_temp = gConfig.sn[3];
    
    vl_input[22] += (pl_temp1[3] + pl_temp2[0]);
    vl_input[23] += (pl_temp1[2] + pl_temp2[1]);
    //vl_short_temp = Battery_get_reg_value(0, ENUM_REG_ADDR_SN56);
    vl_short_temp = gConfig.sn[2];
    
    vl_input[24] += (pl_temp1[1] + pl_temp2[0]);
    vl_input[25] += (pl_temp1[0] + pl_temp2[1]);
    //vl_short_temp = Battery_get_reg_value(0, ENUM_REG_ADDR_MCUN78);
    //vl_short_temp = *((u16*)&(SIM->UIDL)+0);
    vl_short_temp = (u16)(uidl);
    
    vl_input[30] += (pl_temp1[3] + pl_temp2[0]);
    vl_input[31] += (pl_temp1[2] + pl_temp2[1]);
    
    //mbedtls_sha256((const unsigned char * )vl_input, 32, out_data, 0);
    HMACSha256Proc((const unsigned char * )vl_input, 32,(const unsigned char * )sl_authority_secret, 32, out_data);

}

//=============================================================================================
//函数名称: static u32 HMACBigendian32Get(u8 *msg)
//输	入: msg:转换的数据起始指针
//输	出: void
//静态变量:
//功	能: 在字符串中提取大端模式的32位数据
//=============================================================================================
static u32 HMACBigendian32Get(u8 *msg)
{
    return (((u32)(*msg) << 24) |((u32)(*(msg + 1)) << 16)|((u32)(*(msg + 2)) << 8)|(*(msg + 3)));
}

//=============================================================================================
//函数名称: u8 HMACStringCompare(const u8 *msg1, const u8 *msg2, u16 cnt)
//输	入: msg1:比较的数据指针1；msg2:比较的数据指针2；cnt：长度
//输	出: void
//静态变量:
//功	能: 比较，不相等返回0，相等返回1
//=============================================================================================
u8 HMACStringCompare(const u8 *msg1, const u8 *msg2, u16 cnt)
{

    while(cnt-->0)
    {
        if(*(msg1++) != *(msg2++))
            return 0;
    }
    return 1;
}

//=============================================================================================
//函数名称: void HMACBmsCertifiDigest(u8 * rx_buf, u8 * tx_buf)
//输	入: rx_buf:接收的modbus数据；tx_buf：要发送的modubs缓存数据
//输	出: void
//静态变量:
//功	能: BMS认证过程
//=============================================================================================
void HMACBmsCertifiDigest(u8 * rx_buf, u8 * tx_buf)
{
    HMACAuthorityGetDigest(HMACBigendian32Get(certifi_tcb.random_no), certifi_tcb.degist);
    if(HMACStringCompare((u8 *)certifi_tcb.degist,&rx_buf[2],sizeof(certifi_tcb.degist)))
    {
        certifi_tcb.certifi_cmd_ack = 1;//true
        certifi_tcb.IsCertifi = 1;
    }
    else
    {
        certifi_tcb.certifi_cmd_ack = 0;//false
        certifi_tcb.IsCertifi = 0;
    }
    certifi_tcb.certifi_cmd_len = 1;//false
}

//=============================================================================================
//函数名称: void HMACPmsCertifiStart(u8 * rx_buf, u8 * tx_buf)
//输	入: rx_buf:接收的modbus数据；tx_buf：要发送的modubs缓存数据
//输	出: void
//静态变量:
//功	能: PMS认证开始
//=============================================================================================
void HMACPmsCertifiStart(u8 * rx_buf, u8 * tx_buf)
{
    u8 i=0;
    
    certifi_tcb.random_no[0]=rx_buf[2];
    certifi_tcb.random_no[1]=rx_buf[3];
    certifi_tcb.random_no[2]=rx_buf[4];
    certifi_tcb.random_no[3]=rx_buf[5];
    HMACAuthorityGetDigest(HMACBigendian32Get(certifi_tcb.random_no), certifi_tcb.degist);
    for(i = 0; i < 32; i++)
    {
        tx_buf[3 + i] = certifi_tcb.degist[i];
    }
    certifi_tcb.certifi_cmd_len = 33;//false
    certifi_tcb.certifi_cmd_ack = 1;//true
}

//=============================================================================================
//函数名称: u8 HMACClearCertifiSta(void)
//输	入: void
//输	出: void
//静态变量:
//功	能: 清除认证结果
//=============================================================================================
void HMACClearCertifiSta(void)
{
    certifi_tcb.IsCertifi = 0;
}

//=============================================================================================
//函数名称: u8 HMACReadCertifiSta(void)
//输	入: void
//输	出: void
//静态变量:
//功	能: 获取是否需要认证，认证结果
//=============================================================================================
u8 HMACReadCertifiSta(void)
{
    return certifi_tcb.IsCertifi;
}

/*****************************************end of HMAC.c*****************************************/
