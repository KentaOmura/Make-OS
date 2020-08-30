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

void makeTextbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);

void task_b_main(struct SHEET *sht_back);

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
	struct TIMER *timer;
	int cursor_x, cursor_c;
	int maxch;
	int count = 0;
	struct TSS32 tss_a, tss_b;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)0x00270000;
	struct TASK *task_b, *task_a;
	
	static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	
	hardWareInit();
	fifo32_init(&fifo, 256, fifobuf,0);
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);

	/* PIC1�ƃL�[�{�[�h������(11111000) */
	io_out8(PIC0_IMR, 0xf8);
	/* �}�E�X������(11101111) */
	io_out8(PIC1_IMR, 0xef);
	
	memoryInit(memman, memoryUsage(memman));
	
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);
	
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
	
	makeWindow8(buf_win, 160, 68, "Window");
	makeTextbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
	maxch = (144 - 8) / 8; /* �ő啶���� */
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;

	/* taskB�Ɉ����Ƃ��Ēl��n���ׂɁA8���� */
	/* ���̎��́A�X�^�b�N�ł���task_b_esp��64KB�̃��������m�ۂ��A�Ō��64*1024�����{���鎖�ŁA�g�p�ł��郁�����̏I�[�Ԓn���w�����ɂȂ� */
	/* �Ō��8�������̂́AtaskB�Ɉ�����n���ׂł���B������ESP+4�Ɋi�[����鎖��C����̌��܂�ɂȂ��Ă���B���̌��܂�̈�-4�ł͊m�ۂ����������𒴂��鎖�ɂȂ�B�����8 */
	task_a = task_init(memman);
	fifo.task = task_a;
	task_b = task_alloc();
	task_b->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	task_b->tss.eip = (int)&task_b_main;
	task_b->tss.es = 1 * 8;
	task_b->tss.cs = 2 * 8;
	task_b->tss.ss = 1 * 8;
	task_b->tss.ds = 1 * 8;
	task_b->tss.fs = 1 * 8;
	task_b->tss.gs = 1 * 8;
	*((int *) (task_b->tss.esp + 4)) = (int)sht_back;
	task_run(task_b);
	
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
			task_sleep(task_a); /* ���̃^�X�N��CPU�̃��\�[�X������U�� */
			io_sti();
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
				if(i < 0x54 + 256)
				{
					if(keytable[i - 256] != 0 && count < maxch)
					{
						s[0] = keytable[i - 256];
						s[1] = 0;
						putfont8_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s,1);
						cursor_x += 8;
						count++;
					}
				}
				if(i == 256 + 0x0e && cursor_x > 8)
				{
					putfont8_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ",1);
					cursor_x -= 8;
					count--;
				}
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 8, 44);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
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
					if((mdec.btn & 0x01) != 0)
					{
						sheet_slide(sht_win, mouseCoodinate.mx - 80, mouseCoodinate.my - 8);
					}
				}
			}
			else
			{
				if(i != 0)
				{
					timer_init(timer, &fifo, 0);
					cursor_c = COL8_000000;
				}
				else
				{
					timer_init(timer, &fifo, 1);
					cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer, 50);
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 8, 44);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
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

void makeTextbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
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
	boxfill8(sht->buf, sht->bxsize, b, x, y,x + l * 8 ,y + 16);
	putstr8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}

/* taskB�ł�return���߂����{���Ă͂����Ȃ��B�ǂ����炩�Ă΂�Ă���킯�ł͂Ȃ��ׂł���Breturn���߂�EIP���W�X�^���Ăь��̃A�h���X�ɐݒ肷��iJMP���߁j�ׁA
�^�X�N�؂�ւ��͌Ăяo�������Ȃ����牽��EIP���W�X�^��ݒ肷��Ηǂ����킩��Ȃ��Ȃ�B�����Areturn����̂ł����ESP�̐擪�ɌĂяo�����̏ꏊ��ݒ肷��΁A���܂����͂ł��� */
void task_b_main(struct SHEET *sht_back)
{
	struct FIFO32 fifo;
	struct TIMER *timer_ls;
	int i, fifobuf[128], count = 0, count0 = 0;
	char s[11];
	
	fifo32_init(&fifo, 128, fifobuf, 0);
	
	/* �v�� */
	timer_ls = timer_alloc();
	timer_init(timer_ls, &fifo, 100);
	timer_settime(timer_ls, 100);
	
	while(1)
	{
		count++;
		io_cli();
		if(fifo32_status(&fifo) == 0)
		{
			io_sti();
		}
		else
		{
			i = fifo32_get(&fifo);
			io_sti();
			if(100 == i)
			{
				sprintf(s, "%10d", count - count0);
				putfont8_sht(sht_back, 0, 160, COL8_FFFFFF, COL8_0000FF, s, 10);
				count0 = count;
				timer_settime(timer_ls, 100);
			}
		}
	}
}