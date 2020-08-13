#include "bootpack.h"

static struct FIFO gkeyboard;
static struct FIFO gmouse;

void init_pic(void)
{
	/* マスターとスレーブの割り込みを受け付けなくする */
	io_out8(PIC0_IMR, 0xff);
	io_out8(PIC1_IMR, 0xff);
	
	io_out8(PIC0_ICW1, 0x11); /* ハード基盤の設定 */
	io_out8(PIC0_ICW2, 0x20); /* IRQ0-7は、INT20-27で受けとるように設定する */
	io_out8(PIC0_ICW3, 1 << 2);  /* マスタに対してスレーブがどこに繋がっているか */
	io_out8(PIC0_ICW4, 0x01); /* ハード基盤の設定 */
	
	io_out8(PIC1_ICW1, 0x11); /* ハード基盤の設定 */
	io_out8(PIC1_ICW2, 0x28); /* IRQ8-15は、INT28-2fで受けとるように設定する */
	io_out8(PIC1_ICW3, 2);  /* スレーブがマスタの何番目に繋がっているか */
	io_out8(PIC1_ICW4, 0x01); /* ハード基盤の設定 */
	
	io_out8(PIC0_IMR, 0xfb); /* 0x11111011 PIC1以外は割り込み禁止 */
	io_out8(PIC1_IMR, 0xff); /* 0x11111111 全ての割り込みを禁止 */
	
	return;
}

/* キーボードとマウスのキューを初期化する */
void init_keybuf(void)
{
	init_fifo(&gkeyboard);
	init_fifo(&gmouse);
}

/* キーボードのデータ数 */
unsigned int keybord_data_num(void)
{
	return fifo_status(&gkeyboard);
}

/* マウスのデータ数 */
unsigned int mouse_data_num(void)
{
	return fifo_status(&gmouse);
}

unsigned char get_keybord_data(void)
{
	return fifo_get(&gkeyboard);
}

unsigned char get_mouse_data(void)
{
	return fifo_get(&gmouse);
}


/* キーボードはIRQ1に繋がっている。その為、INT 0x21が送られる */
void inthandler21(void)
{
	unsigned char data;
	data = io_in8(0x0060); /* 格納されているデータを取ってあげないと、次のデータを格納する事ができない。 */
	io_out8(PIC0_OCW2, 0x61);
	fifo_put(&gkeyboard,data);
	return;	
}

/* マウスはIRQ12に繋がっている。その為、INT 0x2cが送られる */
void inthandler2c(void)
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);/* IRQ12受付完了を通知 */
	io_out8(PIC0_OCW2, 0x62);/* IRQ02受付完了を通知 */
	data = io_in8(0x0060);
	fifo_put(&gmouse, data);
	return;
}

void inthandler27(void)
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07受付完了をPICに通知 */
	return;
}
