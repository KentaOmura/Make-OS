#include"bootpack.h"

/* FIFOの初期化 */
void init_fifo(struct FIFO* fifo)
{
	fifo->data[0] = '\0';
	fifo->wcounter = 0;
	fifo->rcounter = 0;
	fifo->fsize    = FIFO_MAX;
	fifo->frees    = FIFO_MAX;
}
/* 現在の格納数を返却する */
unsigned int fifo_status(struct FIFO* fifo)
{
	return fifo->fsize - fifo->frees;
}

void fifo_put(struct FIFO* fifo,unsigned char data)
{
	/* 空きが無いので、無視する */
	if(0 == fifo->frees)
	{
		return;
	}
	
	/* データを格納する */
	fifo->data[fifo->wcounter] = data;
	fifo->wcounter++;
	fifo->frees--;

	/* 書き込みカウンタを戻す */
	if(fifo->wcounter == fifo->fsize)
	{
		fifo->wcounter = 0;
	}
	
	return;
}

unsigned char fifo_get(struct FIFO* fifo)
{
	unsigned char data;
	
	/* 格納数が0の場合 */
	if(0 == fifo_status(fifo))
	{
		return 0;
	}
	
	/* データを取得する */
	data = fifo->data[fifo->rcounter];
	fifo->rcounter++;
	fifo->frees++;
	
	/* 読み取りカウンタを戻す */
	if(fifo->rcounter == fifo->fsize)
	{
		fifo->rcounter = 0;
	}
	
	return data;
}
