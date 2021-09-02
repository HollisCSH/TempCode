
#include "crc32.h"

static u32 crc_table[256];  
    
/* 
**initialize the cre32 table 
*/  
void Crc32_Init(void)  
{  
    u32 c;  
    u32 i, j;  
      
    for (i = 0; i < 256; i++) 
	{  
        c = (u32)i;  
        for (j = 0; j < 8; j++) 
		{  
            if (c & 1)  
                c = 0xedb88320L ^ (c >> 1);  
            else  
                c = c >> 1;  
        }  
        crc_table[i] = c;  
    }  
}  
  
/*compute the crc32 code for buffer*/  
u32 Crc32_Calc(u32 crc,u8 *buffer, u32 size)  
{  
    u32 i;  
    for (i = 0; i < size; i++) {  
        crc = crc_table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);  
    }  
    return crc ;  
}
