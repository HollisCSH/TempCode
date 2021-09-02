
#ifndef __CRC__H_
#define __CRC__H_

#include "BSPTypeDef.h"

#ifdef __cplusplus
extern "C"{
#endif

#define CRC32_CHECK_CODE       0xa55aa55a

void 		 Crc32_Init(void);  
u32 Crc32_Calc(u32 crc,u8 *buffer, u32 size)  ;  


#ifdef __cplusplus
}
#endif

#endif //__CRC__H_
