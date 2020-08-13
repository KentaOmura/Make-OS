#include "bootpack.h"

static struct FIFO gkeyboard;
static struct FIFO gmouse;

void init_pic(void)
{
	/* �}�X�^�[�ƃX���[�u�̊��荞�݂��󂯕t���Ȃ����� */
	io_out8(PIC0_IMR, 0xff);
	io_out8(PIC1_IMR, 0xff);
	
	io_out8(PIC0_ICW1, 0x11); /* �n�[�h��Ղ̐ݒ� */
	io_out8(PIC0_ICW2, 0x20); /* IRQ0-7�́AINT20-27�Ŏ󂯂Ƃ�悤�ɐݒ肷�� */
	io_out8(PIC0_ICW3, 1 << 2);  /* �}�X�^�ɑ΂��ăX���[�u���ǂ��Ɍq�����Ă��邩 */
	io_out8(PIC0_ICW4, 0x01); /* �n�[�h��Ղ̐ݒ� */
	
	io_out8(PIC1_ICW1, 0x11); /* �n�[�h��Ղ̐ݒ� */
	io_out8(PIC1_ICW2, 0x28); /* IRQ8-15�́AINT28-2f�Ŏ󂯂Ƃ�悤�ɐݒ肷�� */
	io_out8(PIC1_ICW3, 2);  /* �X���[�u���}�X�^�̉��ԖڂɌq�����Ă��邩 */
	io_out8(PIC1_ICW4, 0x01); /* �n�[�h��Ղ̐ݒ� */
	
	io_out8(PIC0_IMR, 0xfb); /* 0x11111011 PIC1�ȊO�͊��荞�݋֎~ */
	io_out8(PIC1_IMR, 0xff); /* 0x11111111 �S�Ă̊��荞�݂��֎~ */
	
	return;
}

/* �L�[�{�[�h�ƃ}�E�X�̃L���[������������ */
void init_keybuf(void)
{
	init_fifo(&gkeyboard);
	init_fifo(&gmouse);
}

/* �L�[�{�[�h�̃f�[�^�� */
unsigned int keybord_data_num(void)
{
	return fifo_status(&gkeyboard);
}

/* �}�E�X�̃f�[�^�� */
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


/* �L�[�{�[�h��IRQ1�Ɍq�����Ă���B���ׁ̈AINT 0x21�������� */
void inthandler21(void)
{
	unsigned char data;
	data = io_in8(0x0060); /* �i�[����Ă���f�[�^������Ă����Ȃ��ƁA���̃f�[�^���i�[���鎖���ł��Ȃ��B */
	io_out8(PIC0_OCW2, 0x61);
	fifo_put(&gkeyboard,data);
	return;	
}

/* �}�E�X��IRQ12�Ɍq�����Ă���B���ׁ̈AINT 0x2c�������� */
void inthandler2c(void)
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);/* IRQ12��t������ʒm */
	io_out8(PIC0_OCW2, 0x62);/* IRQ02��t������ʒm */
	data = io_in8(0x0060);
	fifo_put(&gmouse, data);
	return;
}

void inthandler27(void)
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07��t������PIC�ɒʒm */
	return;
}
