#include "bootpack.h"

void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR*)ADR_GDT;
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR*)0x0026f800;
	int i;
	
	/* GDT�̏����� */
	for(i = 0; i < 8192; i++)
	{
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xfffffff, 0, 0x4092); /* �������݉\�B�V�X�e����p�̃Z�O�����g */
	set_segmdesc(gdt + 2, 0x0007fff,0x00280000, 0x409a); /* �V�X�e����p�̎��s�\�Z�O�����g */
	
	/* GDTR�ɐݒ肷�� */
	load_gdtr(0xffff, 0x00270000);
	
	/* IDT�̏����� */
	for(i = 0;  i < 256; i++)
	{
		set_gatedesc(idt + i, 0, 0, 0);
	}

	/* �L�[�{�[�h���荞�ݗL���������� */
	set_gatedesc(idt + 0x20, (int)asm_inthandler20, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x27, (int)asm_inthandler27, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x40, (int)asm_cons_putchar, 2 * 8, AR_INTGATE32);

	/* IDTR�ɐݒ肷�� */
	load_idtr(0x7ff, 0x0026f800);
	
	return;
}

/* �Z�O�����g��� */
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	/* �y�[�W�P�ʁi1�y�[�W 4kb�j�Ń��~�b�g������ */
	if(limit > 0xffffff)
	{
		ar |= 0x8000; /* Gbit */
		limit /= 0x1000;
	}
	
	/*
		�x�[�X�̔Ԓn��32bit�ŕ\���ł���Bshort base_low char base_mid base_high �̍��v32bit�B80286�n��CPU�Ƃ̌݊����̈�
	*/
	sd->limit_low     = limit & 0xffff;
	sd->base_low      = base  & 0xffff;
	sd->base_mid      = (base >> 16) & 0xff;
	sd->access_right  = ar & 0xff;
	sd->limit_high    = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0); /* limit_high�̏��4bit�̓Z�O�����g�����̏����L�ڂ���悤�ɂȂ��Ă���B ar��xxxx0000xxxxxxxx*/
	sd->base_high     = (base >> 24) & 0xff;
	
	return;
}

/* ���荞�ݏ�� */
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low = offset & 0xffff;
	gd->selector   = selector; /* �Z�O�����g�Z���N�^ */
	gd->reserve   = (ar >> 8) & 0xff; /* �g���ĂȂ� */
	gd->flags = ar & 0xff; /* �t���O�n */
	gd->offset_high = (offset >> 16) & 0xffff;
}
