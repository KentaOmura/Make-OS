#include "bootpack.h"

struct FIFO32 *keyfifo;
int keydata0;

void wait_KBC_sendready(void)
{
	/* キーボードコントローラがデータ送信可能になるのを待つ */
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
	/* キーボードコントローラの初期化 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

/* キーボードはIRQ1に繋がっている。その為、INT 0x21が送られる */
void inthandler21(void)
{
	unsigned char data;
	data = io_in8(0x0060); /* 格納されているデータを取ってあげないと、次のデータを格納する事ができない。 */
	io_out8(PIC0_OCW2, 0x61);
	fifo32_put(keyfifo,data + keydata0);
	return;	
}

