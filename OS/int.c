#include "bootpack.h"

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

void inthandler27(void)
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07��t������PIC�ɒʒm */
	return;
}
