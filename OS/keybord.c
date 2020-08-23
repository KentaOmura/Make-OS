#include "bootpack.h"

struct FIFO32 *keyfifo;
int keydata0;

void wait_KBC_sendready(void)
{
	/* �L�[�{�[�h�R���g���[�����f�[�^���M�\�ɂȂ�̂�҂� */
	while(1)
	{
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(struct FIFO32 *fifo, int data0)
{
	keyfifo  = fifo;
	keydata0 = data0;
	/* �L�[�{�[�h�R���g���[���̏����� */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

/* �L�[�{�[�h��IRQ1�Ɍq�����Ă���B���ׁ̈AINT 0x21�������� */
void inthandler21(void)
{
	unsigned char data;
	data = io_in8(0x0060); /* �i�[����Ă���f�[�^������Ă����Ȃ��ƁA���̃f�[�^���i�[���鎖���ł��Ȃ��B */
	io_out8(PIC0_OCW2, 0x61);
	fifo32_put(keyfifo,data + keydata0);
	return;	
}

