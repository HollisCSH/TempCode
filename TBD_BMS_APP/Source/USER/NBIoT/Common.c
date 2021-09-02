//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Common.c
//创建人  	: 
//创建日期	:
//描述	    : Sim模组公共文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 常见的字符串操作，字符串转换
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SimCommon.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
uint8 g_CommonBuf[COM_BUF_SIZE];

//=============================================================================================
//定义接口函数
//=============================================================================================

// 求弧度
double radian(double d)
{
    return d * PI / 180.0;   //角度1? = π / 180
}

//计算距离单位KM
double get_distance(double lat1, double lng1, double lat2, double lng2)
{
    double radLat1 = radian(lat1);
    double radLat2 = radian(lat2);
    double a = radLat1 - radLat2;
    double b = radian(lng1) - radian(lng2);

    double dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2) )));

    dst = dst * EARTH_RADIUS;
    dst= round(dst * 10000) / 10000.0;
	
    return dst;
}

//判断当前字节是否可打印
Bool IsPrintChar(uint8 byte)
{
	return (byte >= 0x20 && byte <= 0x7E) || byte == '\r' || byte == '\n';
}

//将pDst字符串的startStr与endStr之间的字符串复制到pSrc中
//比如pDst = “abcdefg” startStr = “b” endStr = "ef" 则pSrc复制为 “cd”
char* strcpyEx(char* pSrc, const char* pDst, const char* startStr, const char* endStr)
{
	const char* p = pDst; 
	const char* p1 = Null;

    //找到startStr字符串，返回当前位置后
	if(startStr) 
	{
		p = strstrex(pDst, startStr);
		if(p == Null) 
			return Null;
	}
    
    //找到endStr字符串，则复制到endStr前
	if(endStr)
	{
		p1 = strstr(p, endStr);
		if(p1 == Null)
			return Null;
		
		memcpy(pSrc, p, p1-p);
		pSrc[p1-p] = 0;
	}
    //找不到endStr字符串，则从startStr开始，复制到末尾
	else
	{
		strcpy(pSrc, p);
	}
	
	return 	pSrc;
}

//pSrc：源字符串
//len：源字符串总长度
//pDst：寻找的目标子字符串
//根据目标子字符串寻找源字符串对应的指针位置，并返回当前的位置
//比如，pSrc = "abcdef",pDst = "bc",则返回 p = "bcdef"
uint8* bytesSearch(const uint8* pSrc, int len, const char* pDst)
{
	int i = 0;
	int dstLen = strlen(pDst);
	
	for(i = 0; i <= len - dstLen; i++, pSrc++)
	{
        //memcmp相等则返回
		if(memcmp(pSrc, pDst, dstLen) == 0) 
        {
            return (uint8*)pSrc;
        }
	}
	
	return Null;
}

//pSrc：源字符串
//len：源字符串总长度
//pDst：寻找的目标子字符串
//根据目标子字符串寻找源字符串对应的指针位置，并返回目标子字符后面的位置
//比如，pSrc = "abcdef",pDst = "bc",则返回 p = "def"
uint8* bytesSearchEx(const uint8* pSrc, int len, const char* pDst)
{
	int i = 0;
	int dstLen = strlen(pDst);
	
	for(i = 0; i <= len - dstLen; i++, pSrc++)
	{
        //memcmp相等则返回
		if(memcmp(pSrc, pDst, dstLen) == 0) 
        {
            return (uint8*)(pSrc + dstLen);
        }
	}
	
	return Null;
}

//查找子字符串pDst在pSrc的位置，如果没有找到，则返回0，如果找到，则返回pSrc的pDst位置后后面
//比如，pSrc = "abcdef",pDst = "bc",则返回 p = "def"
char* strstrex(const char* pSrc, const char* pDst)
{
	char* p = strstr(pSrc, pDst);
	
	if(p)
	{
		p += strlen(pDst);
	}

	return p;
}

//找到目标字符串之后，用空格替换它，使之下次找不到(仅处理一次)。
char* strstr_remove(char* pSrc, const char* pDst, char**pValue)
{
	char* p = strstr(pSrc, pDst);
	
	if(p)	//找到该字符串之后
	{
		*pValue =p+strlen(pDst);
		memset(p, ' ', strlen(pDst));

	}
	
	return p;
}

//功能:把一个字符串根据separator分割成为多个字符串。和strtok不同的是，每一个分隔符都会被分割成为字符串。
//参数说明:
//	strDst		: 目标字符串。
//	separator	: 分割符,可以为多个字符。
//	startInd	: 开始索引
//	strArray[]	: 输出参数，分割结果字符串数组。
//	count		: 字符串数组元素个数。
//返回值:实际分割的字符串数组个数。	
//比如 strDst = "abcdefabcdssc" separator = "a"  startInd = 0 count = 2 会被分成strArray[0] = "" strArray[1] = "bcdef"
//比如 strDst = "abcdefabcdssc" separator = "a"  startInd = 1 count = 2 会被分成strArray[0] = "bcdef" strArray[1] = "bcdssc"
int str_tok(char* strDst, const char* separator, int startInd, char* strArray[], int count)
{
	int i = 0;
	int index = 0;
	char* p = strDst;
	char* pRet = p;

	while(*p)
	{
		const char* pByte = Null;
        //separator的长度可能大于1
		for(pByte = separator; *pByte != 0; pByte++)
		{
            //比较正确
			if(*p == *pByte)
			{
				if(index++ >= startInd)
				{
					*p = 0;
					strArray[i++] = pRet;
					if(i >= count) return i;
				}
				pRet = p+1;
				break;
			}
		}
		p++;
	}

	if(strlen(pRet) && i < count && index >= startInd)
	{
		strArray[i++] = pRet;
	}

    //返回分割的字符串数组的地址
	return i;

}

//功能:把一个字符串根据separator分割成为多个字符串。和strtok不同的是，每一个分隔符都会被分割成为字符串。
//参数说明:
//	strDst		: 目标字符串。
//	separator	: 分割符,可以为多个字符。
//	startInd	: 开始索引
//	strArray[]	: 输出参数，分割结果字符串数组。
//	count		: 字符串数组元素个数。
//返回值:实际分割的字符串数组个数。	
//比如 strDst = "abcdefabcdssc" separator = "a"  startInd = 0 count = 2 会被分成strArray[0] = "" strArray[1] = "bcdef"
//比如 strDst = "abcdefabcdssc" separator = "a"  startInd = 1 count = 2 会被分成strArray[0] = "bcdef" strArray[1] = "bcdssc"
int str_tokex(char* strDst, const char* separator, int startInd, char* strArray[], int count)
{
	int i = 0;
	int index = 0;
	char* p = strDst;
	char* pRet = p;
	const char* pByte = Null;
    pByte = separator;
    
	while(*p)
	{
        //separator的长度可能大于1
//		for(pByte = separator; *pByte < strlen(separator); pByte++)
//		{
        //比较正确
            if(*p == *pByte)
            {
                if(index++ >= startInd)
                {
                    *p = 0;
                    strArray[i++] = pRet;
                    if(i >= count) 
                    {
                        return i;
                    }
                }
                pRet = p+strlen(separator);
//			    break;
            }
//		}
		p++;
	}

	if(strlen(pRet) && i < count && index >= startInd)
	{
		strArray[i++] = pRet;
	}

    //返回分割的字符串数组的地址
	return i;
}

//根据分隔符查询字符串后的索引值
int strtokenValueByIndex(char* strDst, const char* separator, int index)
{
	int value = 0;
	char* p = Null;
	
	str_tok(strDst, separator, index, &p, 1);

	if(p && strlen(p))
	{
        //从字符串读取格式化输入。给到value
		sscanf(p, "%d",&value);
	}
	
	return value;
}

//字节转化为二进制的字符串
char* ByteToBinStr(uint8 byte, char* str)
{
	#if 1
	for(int i = 0; i < 4; i++)
	{
		str[i] = (byte >> i & 0x01) ? '1' : '0';
	}
	str[4] = ' ';
	for(int i = 4; i < 8; i++)
	{
		str[i+1] = (byte >> i & 0x01) ? '1' : '0';
	}
	str[9] = 0;
	#else
	for(int i = 7; i >= 4; i--)
	{
		str[7-i] = (byte >> i & 0x01) ? '1' : '0';
	}
	str[4] = ' ';
	for(int i = 3; i >= 0; i--)
	{
		str[7-i+1] = (byte >> i & 0x01) ? '1' : '0';
	}
	str[9] = 0;
	#endif
	
	return str;
}

//多个字节转化为二进制的字符串，中间用空格隔开
char* ToBinStr(void* pData, int len, char* str)
{
	int ind = 0;
	uint8* pByte = (uint8*)pData;
	for(int i = 0; i < len; i++)
	{
		ByteToBinStr(pByte[i], &str[ind]);
		ind = strlen(str);
		if(i < len - 1)
		{
			str[ind++] = ' ';
		}
	}

	return str;
}

//将16进制字符转为16进制数，例如"0x12"转为数字18
int str_htoi(const char *s)
{
	int n = 0;

	if( !s ) return 0;

	if( *s == '0' )
	{
		s++;
		if( *s == 'x' || *s == 'X' )s++;
	}

	while(*s != '\0')
	{
		if( *s <= '9' && *s >= '0')
		{
			n <<= 4;
			n |= ( *s & 0xf );
		}
		else if(( *s <= 'F' && *s >= 'A') || ( *s <= 'f' && *s >= 'a'))
		{
			n <<= 4;
			n |= ( (*s & 0xf) + 9 );
		}
		else 
			break;
		s++;
	}
	return n;
}

//"A1"->0xA1
Bool hexToByte(const char* s, uint8* val)
{
	uint8 by = 0;
	for (int i = 0; i < 2; i++, s++)
	{
		if (*s <= '9' && *s >= '0')
		{
			by |= *s - '0';
		}
		else if (*s <= 'F' && *s >= 'A')
		{
			by |= *s - 'A' + 10;
		}
		else if (*s <= 'f' && *s >= 'a')
		{
			by |= *s - 'a' + 10;
		}
		else
		{
			return False;
		}

		if (i == 0)
		{
			by <<= 4;
		}

	}
	*val = by;
	return True;
}

//"1234A2FF" =>0x1234A2FF,并放在buff中
Bool hexStrToByteArray(const char* s, int len, uint8* pBuf)
{
	for (int i = 0; i < len; i+=2, s+=2, pBuf++)
	{
		if (!hexToByte(s, pBuf)) 
			return False;
	}
	return True;
}

// -----------------------------------------------------------------
// 函数功能: 字节数组转字符串类型
// 函数名称: hexbyte_to_hex_str
// 参数1   : src 源字符串
// 参数2   : obj 目标字节数组
// 说明    : {0x01,0x03,0x03,x04} --> "01020304"
// 返回值  : 1 成功
// -----------------------------------------------------------------
int hexbyte_to_hex_str (char *src, char *obj, uint16_t length)
{
	for (int i = 0; i < length; i++)
	{
		sprintf (obj, "%02X", src[i]);
		obj += 2;
	}
	return length*2;
}


/*****************************************end of Common.c*****************************************/
