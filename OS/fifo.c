#include"bootpack.h"

/* FIFO‚Ì‰Šú‰» */
void init_fifo(struct FIFO* fifo)
{
	fifo->data[0] = '\0';
	fifo->wcounter = 0;
	fifo->rcounter = 0;
	fifo->fsize    = FIFO_MAX;
	fifo->frees    = FIFO_MAX;
}
/* Œ»İ‚ÌŠi”[”‚ğ•Ô‹p‚·‚é */
unsigned int fifo_status(struct FIFO* fifo)
{
	return fifo->fsize - fifo->frees;
}

void fifo_put(struct FIFO* fifo,unsigned char data)
{
	/* ‹ó‚«‚ª–³‚¢‚Ì‚ÅA–³‹‚·‚é */
	if(0 == fifo->frees)
	{
		return;
	}
	
	/* ƒf[ƒ^‚ğŠi”[‚·‚é */
	fifo->data[fifo->wcounter] = data;
	fifo->wcounter++;
	fifo->frees--;

	/* ‘‚«‚İƒJƒEƒ“ƒ^‚ğ–ß‚· */
	if(fifo->wcounter == fifo->fsize)
	{
		fifo->wcounter = 0;
	}
	
	return;
}

unsigned char fifo_get(struct FIFO* fifo)
{
	unsigned char data;
	
	/* Ši”[”‚ª0‚Ìê‡ */
	if(0 == fifo_status(fifo))
	{
		return 0;
	}
	
	/* ƒf[ƒ^‚ğæ“¾‚·‚é */
	data = fifo->data[fifo->rcounter];
	fifo->rcounter++;
	fifo->frees++;
	
	/* “Ç‚İæ‚èƒJƒEƒ“ƒ^‚ğ–ß‚· */
	if(fifo->rcounter == fifo->fsize)
	{
		fifo->rcounter = 0;
	}
	
	return data;
}
