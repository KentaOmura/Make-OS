#include "bootpack.h"

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

void inthandler27(void)
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07受付完了をPICに通知 */
	return;
}
