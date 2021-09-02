/*
*/
#include "kfifo.h"


void kfifo_init( pkfifo fifo , unsigned char *buff , unsigned short size )
{
    if( !fifo || !buff ) return ;
    fifo->buffer = buff ;
    fifo->size = size ;
    fifo->in = 0 ;
    fifo->out = 0 ;
}

void kfifo_reset( pkfifo fifo)
{
    if( !fifo ) return ;
    fifo->in = 0 ;
    fifo->out = 0;
}

void kfifo_in(pkfifo fifo, unsigned char *in , unsigned short len)
{
    unsigned short i = 0 ;
    if( !fifo || !in ) return ;
    for( i = 0 ; i < len ; i++ )
    {
        fifo->buffer[fifo->in] = in[i];
        //_ADD_LOOP( fifo->in , 1 , fifo->size );
        _ADD_STEP_IN( fifo );
        if( fifo->in == fifo->out ) _ADD_LOOP( fifo->out , 1 , fifo->size );
    }
}

unsigned short kfifo_out( pkfifo fifo, unsigned char *out , unsigned short size)
{
    unsigned short len = 0 ;
    if( !fifo || !out || !size ) return 0;
    while( fifo->in != fifo->out && len < size )
    {
        out[len++] = fifo->buffer[fifo->out];
        //_ADD_LOOP( fifo->out , 1 , fifo->size );
        _ADD_STEP_OUT( fifo );
    }
    return len ;
}

unsigned short kfifo_out_keep( pkfifo fifo , unsigned char *out , unsigned short size)
{
    unsigned short len = 0 ;
    unsigned short pos = 0 ;
    if( !fifo || !out || !size ) return 0;
    pos = fifo->out;
    while( fifo->in != pos && len < size )
    {
        out[len++] = fifo->buffer[pos];
        _ADD_LOOP( pos ,1 , fifo->size );
    }
    return len ;
}

void kfifo_out_clean( pkfifo fifo, unsigned short len )
{
	if( !fifo || !len ) return ;
	_ADD_LOOP( fifo->out , len , fifo->size );
}

void kfifo_in_clean( pkfifo fifo, unsigned short len )
{
	if( !fifo || !len ) return ;
	_ADD_LOOP( fifo->in , len , fifo->size );
}

unsigned short kfifo_getLine( pkfifo fifo, unsigned char* out, unsigned short size)
{
	unsigned short len = 0 ;
	unsigned short pos = 0 ;
	if( !fifo || !out || !size ) return 0;

_KFIFO_RCODE_LINE:
	pos = fifo->in ;
	len = 0 ;
	while( pos != fifo->out && len < size )
	{
		out[len++] = fifo->buffer[pos];
		_ADD_LOOP( pos ,1 , fifo->size );
		if( out[len-1] == '\r' || out[len-1] == '\n' )
		{
			fifo->in = pos ;
			return len ;
		}
	}
	// 比缓冲区大
	if( len >= size )
	{
		fifo->in = pos ;
		goto _KFIFO_RCODE_LINE;
	}
	return 0 ;
}


unsigned short kfifo_getPack(pkfifo fifo, 
	unsigned char headflag , unsigned char tailflag , unsigned char* out, unsigned short size )
{
	unsigned short len = 0 ;
	unsigned short pos = 0 ;
	if( !fifo || !out || !size ) return 0;
	//
_KFIFO_RCODE_PACK:
	while( fifo->in != fifo->out )
	{
		if( fifo->buffer[fifo->in] == headflag ) break ;
		_ADD_STEP_IN( fifo );
	}
	len = 0 ;
	pos = fifo->in ;
	while( pos != fifo->in && len < size )
	{
		out[len++] = fifo->buffer[pos];
		_ADD_LOOP( pos ,1 , fifo->size );
		if( out[len-1] == tailflag && len > 1)
		{
			fifo->in = pos ;
			return len ;
		}
	}
	if( len >= size )
	{
		fifo->in = pos ;
		goto _KFIFO_RCODE_PACK;
	}
	return 0 ;
}

/////////////////////////////////////////////////////////////////////////////
unsigned char kfifo_get_datalen( pkfifo fifo)
{
	return fifo->out > fifo->in ? (fifo->out-fifo->in) : ( fifo->in-fifo->out );
}



