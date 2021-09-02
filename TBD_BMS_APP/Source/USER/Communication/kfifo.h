/*
*/


#ifndef             __KFIFO__H_

#define             __KFIFO__H_

#ifdef __cplusplus
extern "C"{
#endif


typedef struct
{
    unsigned short size ;
    unsigned short in ;
    unsigned short out ;    
    unsigned char *buffer ;
}kfifo , *pkfifo ;

#define        _ADD_LOOP(x,l,s)         (x) = ( (x) + (l) ) % (s)
// s = 2的幂次方
//#define        _ADD_LOOP(x,l,s)         (x) = ( (x) + (1) )&((s) - 1)

#define             _POWER(n)              ( ( (n) & (n-1) ) == 0 )


#define        _ADD_STEP_IN(fifo)       _ADD_LOOP( (fifo)->in , 1 , (fifo)->size )
#define        _ADD_STEP_OUT(fifo)      _ADD_LOOP( (fifo)->out , 1 , (fifo)->size )

void kfifo_init( pkfifo , unsigned char *buff , unsigned short size );
void kfifo_reset( pkfifo );
void kfifo_in(pkfifo , unsigned char *in , unsigned short len);
unsigned short kfifo_out( pkfifo , unsigned char *out , unsigned short size);
unsigned short kfifo_out_keep( pkfifo , unsigned char *out , unsigned short size);
void kfifo_out_clean( pkfifo , unsigned short );
void kfifo_in_clean( pkfifo , unsigned short );
// 获取一行,以\r\n结尾
unsigned short kfifo_getLine( pkfifo , unsigned char*, unsigned short);
// 以固定头，尾 结的数据
unsigned short kfifo_getPack(pkfifo , 
	unsigned char headflag , unsigned char tailflag , unsigned char*, unsigned short );


unsigned char kfifo_get_datalen( pkfifo fifo);
#ifdef __cplusplus
}
#endif

#endif // __KFIFO__H_

