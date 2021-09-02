/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"

#ifdef CONFIG_CMDLINE
#include "CmdLine.h"
#include <stdarg.h>

 //函数功能：16进制数字的字符串转换为整数，例如："0x1234" => 0x1234
 //可以用标准库函数代替：strtol 
int htoi(const char* s)
{
	int n = 0;

	if (!s) return 0;

	if (*s == '0')
	{
		s++;
		if (*s == 'x' || *s == 'X')s++;
	}

	while (*s != '\0')
	{
		if (*s <= '9' && *s >= '0')
		{
			n <<= 4;
			n |= (*s & 0xf);
		}
		else if ((*s <= 'F' && *s >= 'A') || (*s <= 'f' && *s >= 'a'))
		{
			n <<= 4;
			n |= ((*s & 0xf) + 9);
		}
		else
			break;
		s++;
	}
	return n;
}

/*
支持的命令格式如下:
Test(1,2,3,4,5) 最多支持输入5个参数
Test(1,2,0x3,4,"str")
Test 1
Test 1 "str"

支持简写输入

*/

#if 0
#define CmdLine_Strtok strtok
#else
char* CmdLine_Strtok(char* pSrc, const char* delim)
{
	static char* p = 0;
	char* pRet = Null;

	if(pSrc)
	{
		p = pSrc;
	}

	if(*p == 0)
	{
		pRet = Null;
		goto End;
	}

	pRet = p;

	//优先把字符串找出来
	if(*p == '"')	//第一个'"'
	{
		p++;
		while(*p++ != '"')
		{
			if(*p == 0)
			{
				pRet = Null;
				goto End;
			}
		}

		if(*p != 0)
		{
			*p++ = 0;
		}
		goto End;
	}

	while(*p)
	{
		const char* pByte = delim;
		for(pByte = delim; *pByte != 0; pByte++)
		{
			if(*p == *pByte)
			{
				*p++ = 0;
				goto End;
			}
		}
		p++;
	}

End:
	return pRet;
}

#endif


int CmdLine_Printf(CmdLine* pCmdLine, const char* lpszFormat, ...)
{
	int nLen = 0;
	va_list ptr;
	char g_Pfbuffer[128];
	//LOCK();

	memset(g_Pfbuffer, 0, sizeof(g_Pfbuffer));
	va_start(ptr, lpszFormat);
	nLen = vsnprintf(g_Pfbuffer, sizeof(g_Pfbuffer), lpszFormat, ptr);
	va_end(ptr);
	
	if(pCmdLine->cfg->printf) pCmdLine->cfg->printf(g_Pfbuffer);
	
	//UNLOCK();

	return nLen;
}

void CmdLine_Help(CmdLine* pCmdLine)
{
	int i = 0;
	const CmdItem* pCmdItem = pCmdLine->cfg->cmdHandlerArray;
	
	for(i = 0; i < pCmdLine->cfg->cmdHandlerCount; i++, pCmdItem++)
	{		
		CmdLine_Printf(pCmdLine, "\t %s\n", pCmdItem->Desc);
	}
}

ArgType CmdLine_GetArgType(const char* argStr)
{
	int nLen = strlen(argStr);

	if(Null == argStr) return ARGT_NONE;

	if(*argStr == '\"')
	{
		if('\"' == argStr[nLen-1])
		{
			return ARGT_STR;
		}
		else
		{
			return ARGT_ERROR;
		}
	}
	else if(argStr[0] == '0' && (argStr[1] == 'x' || argStr[1] == 'X'))
	{
		return ARGT_HEX;
	}
	else
	{
		return ARGT_DEC;
	}
}

Bool CmdLine_Parse(CmdLine* pCmdLine, char* cmdLineStr, char** pFnName, char* pArgs[], int* argCount)
{
	int maxArgCount = *argCount;
	char *token;
	char fnNameseps[]   = " (\n";
	char argSeps[]   = ", )\n";

	//Find function name
	token = CmdLine_Strtok(cmdLineStr, fnNameseps);
	if(Null == token) return False;
	*pFnName = token;

	*argCount= 0;
	token = CmdLine_Strtok( NULL, argSeps);
	while( token != NULL )
	{
		pArgs[(*argCount)++] = token;
		if((*argCount) > maxArgCount)
		{
			CmdLine_Printf(pCmdLine, "PF_ERROR: Arg count is too many\n");
			return False;
		}
		token = CmdLine_Strtok( NULL, argSeps);
	}

	return True;
}

Bool CmdLine_ArgConvert(CmdLine* pCmdLine, char* pArgs[], int argCount, uint32 arg[])
{
	int i = 0;
	ArgType at = ARGT_NONE;
	char* pChar = Null;

	for(i = 0; i < argCount; i++)
	{
		at = CmdLine_GetArgType(pArgs[i]);
		if(ARGT_DEC == at)
		{
			arg[i] = atoi(pArgs[i]);
		}
		else if(ARGT_HEX == at)
		{
			arg[i] = htoi(pArgs[i]);
		}
		else if(ARGT_STR == at)
		{
			pChar = pArgs[i];
			pChar[strlen(pChar) - 1] = 0;
			pChar++;
			arg[i] = (uint32)pChar;
		}
		else
		{
			CmdLine_Printf(pCmdLine, "\tArg[%d](%s) error. \n", i+1, pArgs[i]);
			return False;
		}
	}
	return True;
}

/*****************************************************************
函数功能：在命令表中查找函数名称，支持模糊查询。
函数参数：
	pCmdLine：命令行对象。
	pFnName：函数名称。
	pFoundEntry：查找到的第一个匹配项。
返回值：匹配项数量。
*****************************************************************/
static int CmdLine_Find(CmdLine* pCmdLine, const char* pFnName, const CmdItem** pFoundEntry)
{
	int isFind = 0;
	const CmdItem* pFnEntry = pCmdLine->cfg->cmdHandlerArray;

	for (int i = 0; i < pCmdLine->cfg->cmdHandlerCount; i++, pFnEntry++)
	{
		//和函数名部分比较
		if (strstr(pFnEntry->Desc, pFnName) == pFnEntry->Desc)
		{
			char* str;

			isFind++;

			if (Null == pFoundEntry)
			{
				*pFoundEntry = pFnEntry;
			}

			//查找函数名
			str = strchr(pFnEntry->Desc, '(');
			if (Null == str)
				str = strchr(pFnEntry->Desc, ' ');

			if (Null == str) continue;

			//和函数名完全比较
			if (memcmp(pFnEntry->Desc, pFnName, str - pFnEntry->Desc) == 0)
			{
				isFind = 1;
				*pFoundEntry = pFnEntry;
				break;
			}
		}
	}
	return isFind;
}

void CmdLine_Exe(CmdLine* pCmdLine, const char* pFnName, uint32 arg[], int argCount)
{
	Bool findCount = 0;
	const CmdItem* pFoundEntry = Null;
	
	#define FUN(n, funType, args) if(n == pFoundEntry->ex->argCount)	\
		{	\
			((funType)pFoundEntry->Proc) args;	\
			return;	\
		}

	if(strcmp(pFnName, "?") == 0)
	{
		CmdLine_Help(pCmdLine);
		return;
	}

	findCount = CmdLine_Find(pCmdLine, pFnName, &pFoundEntry);

	if(0 == findCount)
	{
		CmdLine_Printf(pCmdLine, "Unknown: %s\n", pFnName);
		return;
	}
	else if(findCount > 1)
	{
		//如果找出的函数名多于一个，则打印所有的部分比较正确的函数名
		const CmdItem* pFnEntry = pCmdLine->cfg->cmdHandlerArray;
		for(int i = 0; i < pCmdLine->cfg->cmdHandlerCount; i++, pFnEntry++)
		{
			if(strstr(pFnEntry->Desc, pFnName) == pFnEntry->Desc)
			{
				CmdLine_Printf(pCmdLine, "%s\n", pFnEntry->Desc);
			}
		}
		return;
	}

	FUN(0, FnArg0, ());
	FUN(1, FnArg01, (arg[0]));
	FUN(2, FnArg02, (arg[0], arg[1]));
	FUN(3, FnArg03, (arg[0], arg[1], arg[2]));
	FUN(4, FnArg04, (arg[0], arg[1], arg[2], arg[3]));
	FUN(5, FnArg05, (arg[0], arg[1], arg[2], arg[3], arg[4]));
}

int CmdLine_GetArgCount(const char* str)
{
	Bool bFlag = False;
	int nArgCount = 0;
	
	str	= strchr(str, '(');

	if(Null == str)
	{
		return 0;
	}
	
	while(*(++str) != '\0')
	{
		if(')' == *str)
		{
			break;
		}
		else if(!bFlag)
		{
			if(' ' != *str)
			{
				bFlag = True;
				nArgCount++;

				if(',' == *str)
				{
					nArgCount++;
				}
			}
		}
		else if(',' == *str)
		{
			nArgCount++;
		}
	}
	
	return *str == ')' ? nArgCount : -1;
}

void CmdLine_Reset(CmdLine* pCmdLine)
{
	if(pCmdLine->isEcho)
		CmdLine_Printf(pCmdLine, "->");
	
	memset(pCmdLine->cfg->cmdLineBuf, 0, sizeof(pCmdLine->cfg->cmdLineBuf));
	pCmdLine->cmdLineLen = 0;
}

void CmdLine_AddStr(CmdLine* pCmdLine, const char* str)
{
	CmdLine_AddStrEx(pCmdLine, str, strlen(str));
}

void CmdLine_AddStrEx(CmdLine* pCmdLine, const char* str, int len)
{
	int i = 0;
	char* pBuf = pCmdLine->cfg->cmdLineBuf;

	for(i = 0; i < len; i++, str++)
	{
		if(pCmdLine->cmdLineLen >= MAX_CMDLINE_LEN)
		{
			CmdLine_Reset(pCmdLine);
		}
		
		if(pCmdLine->isEcho)
		{
			CmdLine_Printf(pCmdLine, "%c", *str);
		}
		
		if(*str != KEY_CR && *str != KEY_LF)
		{
			pBuf[pCmdLine->cmdLineLen++] = *str;
		}
		if(KEY_CR == *str)// || ')' == *str)
		{
			char* pFnName = Null;
			char* argStr[MAX_ARG_COUNT] = {0};
			int argCount = MAX_ARG_COUNT;
			
			if(('\r' == pBuf[0] && pCmdLine->cmdLineLen == 1) || 0 == pCmdLine->cmdLineLen)
			{
				CmdLine_Reset(pCmdLine);
				return;
			}

			if(CmdLine_Parse(pCmdLine, pBuf, &pFnName, argStr, &argCount))
			{
				uint32 arg[MAX_ARG_COUNT] = {0};

				if(CmdLine_ArgConvert(pCmdLine, argStr, argCount, arg))
				{
					CmdLine_Exe(pCmdLine, pFnName, arg, argCount);
				}
			}
			CmdLine_Reset(pCmdLine);
	   	}
	}
}

void CmdLine_Init(CmdLine* cmdLine, const CmdLineCfg* cfg, Bool isEcho)
{
	int i = 0;
	
	memset(cmdLine, 0, sizeof(CmdLine));
	
	cmdLine->isEcho = isEcho;

	const CmdItem* fn = cfg->cmdHandlerArray;
	for(i = 0; i < cfg->cmdHandlerCount; i++, fn++)
	{
		int argCount = CmdLine_GetArgCount(fn->Desc);
		if(argCount < 0 || argCount > MAX_ARG_COUNT)
		{
			CmdLine_Printf(cmdLine, "[%s] error, get arg count[%d] error.\n", fn->Desc, argCount);
		}
		
		fn->ex->argCount = (int8)argCount;
	}
}

#endif

