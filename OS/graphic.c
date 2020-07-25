#include "bootpack.h"

/* vram�Ɏl�p�`��`�悷�� */
void boxfill8(unsigned char *vram, int xsize, unsigned char color, int start_x, int start_y, int end_x, int end_y)
{
	int x, y;
	for(y = start_y; y < end_y; y++)
	{
		for(x = start_x; x < end_x; x++)
		{
			vram[y * xsize + x] = color;
		}
	}
}

void putstr8_asc(char *vram, int xsize, int index_x, int index_y, int color, unsigned char *str)
{
	extern char hankaku[4096];
	
	while('\0' != *str)
	{
		putfont8(vram, xsize, index_x, index_y, color, hankaku + *str * 16);
		index_x += 8; /* ������1�o�C�g�ׁ̈A8bit���炵�Ď��̕�����`�悷�� */
		str++;
	}
	
	return;
}

/* �w�肵���G��vram�ɕ`�悷�� */
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char* buf)
{
	int x, y;
	
	for(y = 0; y < pysize; y++)
	{
		for(x = 0; x < pxsize; x++)
		{
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * pxsize + x];
		}
	}
}

/* �}�E�X�J�[�\�� 8 * 8 */
void init_mouse_cursol(char* mouse, char col)
{
	static char mouse_coursol[8][8] = {
		 "o......."
		,"ooo....."
		,"oooo...."
		,"oooooo.."
		,"oooooooo"
		,"...ooo.."
		,"....oo.."
		,".....oo."
	};
	
	int x, y;
	
	for(y = 0; y < 8; y++)
	{
		for(x = 0; x < 8; x++)
		{
			if(mouse_coursol[y][x] == 'o')
			{
				mouse[y * 8 + x] = COL8_FFFFFF;
			}
			if(mouse_coursol[y][x] == '.')
			{
				mouse[y * 8 + x] = col;
			}
		}
	}
	
	return;
}

/* ������\������ */
void putfont8(char *vram, int xsize, int index_x, int index_y, char color, char* font)
{
	int i;
	char *p, d;
	
	for(i = 0; i < 16; i++)
	{
		p = vram + (index_y + i) * xsize + index_x;
		d = font[i];
		
		if((d & 0x80) != 0) {p[0] = color;}
		if((d & 0x40) != 0) {p[1] = color;}
		if((d & 0x20) != 0) {p[2] = color;}
		if((d & 0x10) != 0) {p[3] = color;}
		if((d & 0x08) != 0) {p[4] = color;}
		if((d & 0x04) != 0) {p[5] = color;}
		if((d & 0x02) != 0) {p[6] = color;}
		if((d & 0x01) != 0) {p[7] = color;}
	}
	
	return;
}

void init_palette(void)
{
	/* 9�F ���т�RGB�̏� */
	static unsigned char table_rgb[16 * 3] =
	{
		0x00, 0x00, 0x00,		/* �� */
		0xff, 0x00, 0x00,		/* �� */
		0x00, 0xff, 0x00,		/* �� */
		0x00, 0x00, 0xff,		/* �� */
		0xff, 0xff, 0x00,		/* ��+�� = �� */
		0xff, 0x00, 0xff,		/* ��+�� = �� */
		0x00, 0xff, 0xff,		/* ��+�� = ���F */
		0xff, 0xff, 0xff,		/* �� */
		0xc6, 0xc6, 0xc6,		/* �D�F */
		0x84, 0x00, 0x00,		/* �Â��� */
		0x00, 0x84, 0x00,		/* �Â��� */
		0x00, 0x00, 0x84,		/* �Â��� */
		0x84, 0x84, 0x00,		/* �Â��� */
		0x84, 0x00, 0x84,		/* �Â��� */
		0x00, 0x84, 0x84,		/* �Â����F */
		0x84, 0x84, 0x84		/* �Â��D�F */
	};
	
	/* �F��ݒ肷�� */
	set_palette(0, 15, table_rgb);
	return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	
	eflags = io_load_eflags(); /* eflags�̓��e���L������ */
	io_cli();				   /* CLI�Ŋ��� */
	
	/* �p���b�g��ݒ肷�� */
	io_out8(0x03c8, start);
	for(i = start; i <= end; i++)
	{
		io_out8(0x03c9, rgb[0]);
		io_out8(0x03c9, rgb[1]);
		io_out8(0x03c9, rgb[2]);
		rgb += 3;
	}
	/* eflags�����ɖ߂� */
	io_store_eflags(eflags);
	return;
}

