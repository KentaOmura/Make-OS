#include "bootpack.h"
struct FIFO32 *mousefifo;
int mousedata0;

void enable_mouse(struct FIFO32 *fifo, int data0,struct MOUSE_DEC *mdec)
{
	mousefifo = fifo;
	mousedata0 = data0;
	
	/* マウス有効 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase = 0;
	return; /* うまくいくとACK(0xfa)が送信されてくる */
}

/* マウスの信号の解析 */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data)
{
	int result = 0;
	
	switch(mdec->phase)
	{
	case 0:
		if(0xfa == data)
		{
			mdec->phase = 1;
		}
		break;
	case 1:
		mdec->data[0] = data;
		mdec->phase = 2;
		break;
	case 2:
		mdec->data[1] = data;
		mdec->phase = 3;
		break;
	case 3:
		mdec->data[2] = data;
		mdec->phase = 1;
		result = 1;
		
		mdec->btn = mdec->data[0] & 0x07;
		mdec->x = mdec->data[1];
		mdec->y = mdec->data[2];
		if ((mdec->data[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->data[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; /* マウスではy方向の符号が画面と反対 */
		break;
	default:
		result = -1;
	}

	return result;
}

/* マウスはIRQ12に繋がっている。その為、INT 0x2cが送られる */
void inthandler2c(void)
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);/* IRQ12受付完了を通知 */
	io_out8(PIC0_OCW2, 0x62);/* IRQ02受付完了を通知 */
	data = io_in8(0x0060);
	fifo32_put(mousefifo, data + mousedata0);
	return;
}
