#include "bootpack.h"

/******************************************************************
/*                          ��`                                  */
/******************************************************************
/* �������Ǘ��p�̃����� */
#define MEMMAN_ADDR 0x003c0000

/* �}�E�X�̍��W */
struct MOUSE_COODINATE
{
	int mx;
	int my;
};

/******************************************************************
/*                          �֐��錾                              */
/******************************************************************/
/* �������̎g�p�ʂ̃`�F�b�N */
unsigned int memoryUsage(struct MEMMAN *man);
/* �������Ǘ��̏����� */
void memoryInit(struct MEMMAN *man, unsigned int memtotal);
/* �n�[�h�E�F�A�̏����� */
void hardWareInit(void);
/* �}�E�X���W��臒l�𒴂��Ă���Ίۂߍ��� */
void MouseCoodinateThreshold(struct MOUSE_COODINATE *mc, unsigned int screen_max_x, unsigned int screen_max_y);
void makeWindow8(unsigned char* buf, int xsize, int ysize, char *title);

void putfont8_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);

void HariMain(void)
{
	unsigned char s[50];

	struct MOUSE_DEC mdec;
	struct FIFO32 fifo;
	int fifobuf[256];
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	/* �N�����̉�ʏ����擾 */
	struct SCREEN_INFO *binfo = (struct SCREEN_INFO*)0x0ff4;
	/* �V�[�g */
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	struct MOUSE_COODINATE mouseCoodinate; 
	unsigned char *buf_back, buf_mouse[256], *buf_win;
	int i;
	struct TIMER *timer, *timer2, *timer3;
	
	hardWareInit();
	fifo32_init(&fifo, 256, fifobuf);
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);

	/* PIC1�ƃL�[�{�[�h������(11111000) */
	io_out8(PIC0_IMR, 0xf8);
	/* �}�E�X������(11101111) */
	io_out8(PIC1_IMR, 0xef);
	
	memoryInit(memman, memoryUsage(memman));
	
	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000); /* 1�b�Ԃ�100�񊄂荞�݂���������̂ŁA1000�񊄂荞�݂�10�b�ƂȂ� */
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);
	
	init_palette();
	
	shtctl = shtctl_init(memman, binfo->vram, binfo->screen_x, binfo->screen_y);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
	buf_back = (unsigned char*)memman_alloc_4k(memman, binfo->screen_x * binfo->screen_y);
	buf_win = (unsigned char*)memman_alloc_4k(memman, 160*68);
	sheet_setbuf(sht_back, buf_back, binfo->screen_x , binfo->screen_y, -1);
	sheet_setbuf(sht_mouse, buf_mouse, 8, 8, COL8_0000FF);
	sheet_setbuf(sht_win, buf_win, 160, 68, 88);	
	/* �w�i��ʂ�`�悷��i��F�j */
	boxfill8(buf_back, binfo->screen_x, COL8_0000FF, 0, 0, binfo->screen_x, binfo->screen_y);
	/* �}�E�X�J�[�\���̃f�[�^���擾 */
	init_mouse_cursol(buf_mouse, COL8_0000FF);
	
	makeWindow8(buf_win, 160, 68, "Timer");
	
	sheet_slide(sht_back, 0, 0);
	/* �}�E�X�J�[�\����`�悷�� */
	mouseCoodinate.mx = (binfo->screen_x - 8) / 2;
	mouseCoodinate.my = (binfo->screen_y -28 - 8) / 2;
	sheet_slide(sht_mouse, mouseCoodinate.mx, mouseCoodinate.my);
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_win, 1);
	sheet_updown(sht_mouse, 2);
	
	sprintf(s, "memory %dMB  free : %dKB", memman->storage / (1024 * 1024), memman_total(memman) / 1024);
	putstr8_asc(buf_back, binfo->screen_x, 0, 48, COL8_FFFFFF, s);
	sheet_refresh(sht_back, 0, 0, binfo->screen_x, 48 + 16);
	
	while(1)
	{
		io_cli(); /* ���荞�݂��֎~���� */
		if(0 == fifo32_status(&fifo))
		{
			io_stihlt();
		}
		else
		{
			i = fifo32_get(&fifo);
			io_sti();
			/* �L�[�{�[�h����̃f�[�^���i�[����Ă���ꍇ */
			if(256 <= i && i <= 511)
			{
				sprintf(s, "%02X", i - 256);
				putfont8_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_0000FF, s, 3);
			}
			/* �}�E�X����̃f�[�^���i�[����Ă���ꍇ */
			else if(512 <= i && i <= 767)
			{
				if(1 == mouse_decode(&mdec, i - 512))
				{
					mouseCoodinate.mx += mdec.x;
					mouseCoodinate.my += mdec.y;
					MouseCoodinateThreshold(&mouseCoodinate, binfo->screen_x, binfo->screen_y);
					
					sheet_slide(sht_mouse, mouseCoodinate.mx, mouseCoodinate.my);
				}
			}
			else if(10 == i)
			{
				putfont8_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_0000FF, "10[sec]", 7);
			}
			else if(3 == i)
			{
				putfont8_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_0000FF, "3[sec]", 6);
			}
			else
			{
				if(i != 0)
				{
					timer_init(timer3, &fifo, 0);
					boxfill8(buf_back, binfo->screen_x, COL8_FFFFFF, 8, 96, 15, 111);
				}
				else
				{
					timer_init(timer3, &fifo, 1);
					boxfill8(buf_back, binfo->screen_x, COL8_0000FF, 8, 96, 15, 111);
				}
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 96, 16, 112);
			}
		}
	}
}

/* �������̍ő�g�p�ʂ̃`�F�b�N */
unsigned int memoryUsage(struct MEMMAN *man)
{
	unsigned int result;
	
	/* �g�p�ł��郁�������̃`�F�b�N */
	result = memtest(0x00400000, 0xbfffffff);
	
	return result;
}

/* �������Ǘ��̏����� */
void memoryInit(struct MEMMAN *man, unsigned int memtotal)
{
	/* �������Ǘ��҂̏����� */
	memman_init(man, memtotal);
	/* �������̎g�p�ʂ̐ݒ� */
	memman_free(man, 0x00001000, 0x0009e000);
	memman_free(man, 0x00400000, memtotal-0x00400000);
	
	return;
}

/* �n�[�h�E�F�A�̏����� */
void hardWareInit(void)
{
	init_gdtidt();
	init_pic();
	/* IDT��PIC�̐ݒ肪���������̂ŁACPU�ւ̊��荞�݂��󂯕t���� */
	io_sti();
	init_pit();
}

/* �}�E�X���W��臒l�𒴂��Ă���Ίۂߍ��� */
void MouseCoodinateThreshold(struct MOUSE_COODINATE *mc, unsigned int screen_max_x, unsigned int screen_max_y)
{
	if(mc->mx < 0)
	{
		mc->mx = 0;
	}
	if(mc->my < 0)
	{
		mc->my = 0;
	}

	if(mc->mx > screen_max_x - 1)
	{
		mc->mx = screen_max_x - 1;
	}
	if(mc->my > screen_max_y - 1)
	{
		mc->my = screen_max_y - 1;
	}
}

void makeWindow8(unsigned char* buf, int xsize, int ysize, char *title)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_FF0000, xsize - 1, 0,         xsize - 1, ysize - 1);
	
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_FFFFFF, 3,         3,         xsize - 4, 20       );
	
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_FFFFFF, 0,         ysize - 1, xsize - 1, ysize - 1);
	putstr8_asc(buf, xsize, 24, 4, COL8_000000, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_FFFFFF;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_FF0000;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}

void putfont8_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y,x + l * 8 - l ,y + 15);
	putstr8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}
