#include "bootpack.h"

/* vramに四角形を描画する */
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
		index_x += 8; /* 文字は1バイトの為、8bitずらして次の文字を描画する */
		str++;
	}
	
	return;
}

/* 指定した絵をvramに描画する */
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

/* マウスカーソル 8 * 8 */
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

/* 文字を表示する */
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
	/* 9色 並びはRGBの順 */
	static unsigned char table_rgb[16 * 3] =
	{
		0x00, 0x00, 0x00,		/* 黒 */
		0xff, 0x00, 0x00,		/* 赤 */
		0x00, 0xff, 0x00,		/* 緑 */
		0x00, 0x00, 0xff,		/* 青 */
		0xff, 0xff, 0x00,		/* 赤+緑 = 黄 */
		0xff, 0x00, 0xff,		/* 赤+青 = 紫 */
		0x00, 0xff, 0xff,		/* 緑+青 = 水色 */
		0xff, 0xff, 0xff,		/* 白 */
		0xc6, 0xc6, 0xc6,		/* 灰色 */
		0x84, 0x00, 0x00,		/* 暗い赤 */
		0x00, 0x84, 0x00,		/* 暗い緑 */
		0x00, 0x00, 0x84,		/* 暗い青 */
		0x84, 0x84, 0x00,		/* 暗い黄 */
		0x84, 0x00, 0x84,		/* 暗い紫 */
		0x00, 0x84, 0x84,		/* 暗い水色 */
		0x84, 0x84, 0x84		/* 暗い灰色 */
	};
	
	/* 色を設定する */
	set_palette(0, 15, table_rgb);
	return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	
	eflags = io_load_eflags(); /* eflagsの内容を記憶する */
	io_cli();				   /* CLIで割禁 */
	
	/* パレットを設定する */
	io_out8(0x03c8, start);
	for(i = start; i <= end; i++)
	{
		io_out8(0x03c9, rgb[0]);
		io_out8(0x03c9, rgb[1]);
		io_out8(0x03c9, rgb[2]);
		rgb += 3;
	}
	/* eflagsを元に戻す */
	io_store_eflags(eflags);
	return;
}

