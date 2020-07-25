#include"bootpack.h"

/* FIFO�̏����� */
void init_fifo(struct FIFO* fifo)
{
	fifo->data[0] = '\0';
	fifo->wcounter = 0;
	fifo->rcounter = 0;
	fifo->fsize    = FIFO_MAX;
	fifo->frees    = FIFO_MAX;
}
/* ���݂̊i�[����ԋp���� */
unsigned int fifo_status(struct FIFO* fifo)
{
	return fifo->fsize - fifo->frees;
}

void fifo_put(struct FIFO* fifo,unsigned char data)
{
	/* �󂫂������̂ŁA�������� */
	if(0 == fifo->frees)
	{
		return;
	}
	
	/* �f�[�^���i�[���� */
	fifo->data[fifo->wcounter] = data;
	fifo->wcounter++;
	fifo->frees--;

	/* �������݃J�E���^��߂� */
	if(fifo->wcounter == fifo->fsize)
	{
		fifo->wcounter = 0;
	}
	
	return;
}

unsigned char fifo_get(struct FIFO* fifo)
{
	unsigned char data;
	
	/* �i�[����0�̏ꍇ */
	if(0 == fifo_status(fifo))
	{
		return 0;
	}
	
	/* �f�[�^���擾���� */
	data = fifo->data[fifo->rcounter];
	fifo->rcounter++;
	fifo->frees++;
	
	/* �ǂݎ��J�E���^��߂� */
	if(fifo->rcounter == fifo->fsize)
	{
		fifo->rcounter = 0;
	}
	
	return data;
}
