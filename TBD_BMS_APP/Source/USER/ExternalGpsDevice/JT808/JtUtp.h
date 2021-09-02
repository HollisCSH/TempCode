#ifndef __UTP_BASE_H_
#define __UTP_BASE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "BSPTypeDef.h"
#include "utp.h"


	typedef struct _JtUtp
	{
		uint8_t  checkSum;
		uint8_t  ver;
		uint16_t vendor;
		uint8_t  devType;
		uint8_t  cmd;
		uint8_t  len;
		uint8_t  data[1];
	}JtUtp;

	extern const UtpFrameCfg g_jtFrameCfg;
#ifdef __cplusplus
}
#endif

#endif


