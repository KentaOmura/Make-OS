#include "bootpack.h"
#include<string.h>

/******************************************************************
/*                          定義                                  */
/******************************************************************
/* マウスの座標 */
struct MOUSE_COODINATE
{
	int mx;
	int my;
};
/******************************************************************
/*                          関数宣言                              */
/******************************************************************/
/* メモリの使用量のチェック */
unsigned int memoryUsage(struct MEMMAN *man);
/* メモリ管理の初期化 */
void memoryInit(struct MEMMAN *man, unsigned int memtotal);
/* ハードウェアの初期化 */
void hardWareInit(void);
/* マウス座標が閾値を超えていれば丸め込む */
void MouseCoodinateThreshold(struct MOUSE_COODINATE *mc, unsigned int screen_max_x, unsigned int screen_max_y);
void makeWindow8(unsigned char* buf, int xsize, int ysize, char *title, int act);
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);
void putfont8_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void makeTextbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);

void HariMain(void)
{
	unsigned char s[50];

	struct MOUSE_DEC mdec;
	struct FIFO32 fifo;
	int fifobuf[256];
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	/* 起動時の画面情報を取得 */
	struct BOOT_INFO *binfo = (struct BOOT_INFO*)0x0ff0;
	/* シート */
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	struct MOUSE_COODINATE mouseCoodinate; 
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_win_b, *buf_cons;
	int i;
	struct TIMER *timer;
	int cursor_x, cursor_c;
	int maxch;
	int count = 0;
	int keyto = 0, key_shift = 0, key_leds = (binfo->leds >> 4) & 7;
	struct TSS32 tss_a, tss_b;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)0x00270000;
	struct TASK *task_a, *task_cons;
	
	static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '@', '[', 0,   0,   'a', 's',
		'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', ':', 0,   0,   ']', 'z', 'x', 'c', 'v',
		'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		 0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.',  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0x5c,0,0,0,0,0,0,0,0,0,0x5c,0,0
	};
	
	static char keytable1[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.',  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,'_',0,0,0,0,0,0,0,0,0,'|',0,0
	};
	
	hardWareInit();
	fifo32_init(&fifo, 256, fifobuf,0);
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);

	/* PIC1とキーボードを許可(11111000) */
	io_out8(PIC0_IMR, 0xf8);
	/* マウスを許可(11101111) */
	io_out8(PIC1_IMR, 0xef);
	
	memoryInit(memman, memoryUsage(memman));
	
	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->screen_x, binfo->screen_y);
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);
	
	/* sht_back */
	sht_back = sheet_alloc(shtctl);
	buf_back = (unsigned char*)memman_alloc_4k(memman, binfo->screen_x * binfo->screen_y);
	sheet_setbuf(sht_back, buf_back, binfo->screen_x , binfo->screen_y, -1);
	/* 背景画面を描画する（青一色） */
	boxfill8(buf_back, binfo->screen_x, COL8_0000FF, 0, 0, binfo->screen_x, binfo->screen_y);

	/* 引数として値を渡す為に、8引く */
	/* 下の式は、スタックであるtask_b_espに64KBのメモリを確保し、最後に64*1024を実施する事で、使用できるメモリの終端番地を指す事になる */
	/* 最後に8を引くのは、taskBに引数を渡す為である。引数はESP+4に格納される事がC言語の決まりになっている。その決まりの為-4では確保したメモリを超える事になる。よって8 */
	/* sht_cons */
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char*)memman_alloc_4k(memman,256*165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);
	makeWindow8(buf_cons, 256, 165, "consols", 0);
	makeTextbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int)&consol_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int)sht_cons;
	*((int *) (task_cons->tss.esp + 8)) = (int)memman->storage;
	task_run(task_cons, 2, 2);
	
	sht_win = sheet_alloc(shtctl);
	buf_win = (unsigned char*)memman_alloc_4k(memman, 160*68);
	sheet_setbuf(sht_win, buf_win, 160, 68, 88);	
	makeWindow8(buf_win, 160,68,"task_a", 1);
	makeTextbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);

	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 8, 8, COL8_0000FF);
	
	/* マウスカーソルのデータを取得 */
	init_mouse_cursol(buf_mouse, COL8_0000FF);
	maxch = (144 - 8) / 8; /* 最大文字数 */
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);
	
	/* マウスカーソルを描画する */
	mouseCoodinate.mx = (binfo->screen_x - 8) / 2;
	mouseCoodinate.my = (binfo->screen_y -28 - 8) / 2;
	
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_cons, 32, 120);
	sheet_slide(sht_mouse, mouseCoodinate.mx, mouseCoodinate.my);
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_cons, 1);
	sheet_updown(sht_win, 4);
	sheet_updown(sht_mouse, 5);
	
	while(1)
	{
		io_cli(); /* 割り込みを禁止する */
		if(0 == fifo32_status(&fifo))
		{
			task_sleep(task_a); /* 他のタスクにCPUのリソースを割り振る */
			io_sti();
		}
		else
		{
			i = fifo32_get(&fifo); 
			io_sti();
			
			/* キーボードからのデータが格納されている場合 */
			if(256 <= i && i <= 511)
			{
				if(i < 0x80 + 256)
				{
					if(key_shift == 0)
					{
						s[0] = keytable0[i - 256];
					}
					else
					{
						s[0] = keytable1[i - 256];
					}
				}
				else
				{
					s[0] = 0;
				}
				/* CapsLockの状態によって、逆転させる */
				if( ('a' <= s[0] && s[0] <= 'z') || ('A' <= s[0] && s[0] <= 'Z') )
				{
					if( (key_leds & 4) != 0 ) /* CapsLock状態 */
					{
						if('a' <= s[0] && s[0] <= 'z')
						{
							s[0] = keytable1[i - 256];
						}
						else
						{
							s[0] = keytable0[i - 256];
						}
					}
				}
				if(s[0] != 0)
				{
					if(keyto == 0)
					{
						if(count < maxch)
						{
							s[1] = 0;
							putfont8_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s,1);
							cursor_x += 8;
							count++;
						}
					}
					else
					{
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}
				/* CapsLockの状態を更新する */
				if(i == 256 + 0xba)
				{
					key_leds ^= 4;
				}
				if(i == 256 + 0x0e)
				{
					if(keyto == 0 && cursor_x > 8)
					{
						putfont8_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ",1);
						cursor_x -= 8;
						count--;
					}
					else
					{
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}
				if(i == 256 + 0x1c)
				{
					if(keyto == 1)
					{
						fifo32_put(&task_cons->fifo, 0x1c + 256);
					}
				}
				if(i == 256 + 0x0f)
				{
					if(keyto == 0)
					{
						keyto = 1;
						make_wtitle8(buf_win, sht_win->bxsize,"task_a", 0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "consols", 1);
						cursor_c = -1;
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 8, 44);
						fifo32_put(&task_cons->fifo, 2); /* コンソールタスクの点滅を許可 */
					}
					else
					{
						keyto = 0;
						make_wtitle8(buf_win, sht_win->bxsize,"task_a", 1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "consols", 0);
						cursor_c = COL8_000000;
						fifo32_put(&task_cons->fifo, 3); /* コンソールタスクの点滅を禁止 */
					}
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
					sheet_refresh(sht_win, 0, 0, sht_win->bxsize, 21);
				}
				if(i == 256 + 0x2a) /* 左シフトON */
				{
					key_shift |= 1;
				}
				if(i == 256 + 0x36) /* 右シフトON */
				{
					key_shift |= 2;
				}
				if(i == 256 + 0xaa) /* 左シフトOFF */
				{
					key_shift &= ~1;
				}
				if(i == 256 + 0xb6) /* 右シフトOFF */
				{
					key_shift &= ~2;
				}
				if(cursor_c >= 0)
				{
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 8, 44);
				}
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
			/* マウスからのデータが格納されている場合 */
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
					if(cursor_c >= 0)
					{
						cursor_c = COL8_000000;
					}
				}
				else
				{
					timer_init(timer, &fifo, 1);
					if(cursor_c >= 0)
					{
						cursor_c = COL8_FFFFFF;
					}
				}
				timer_settime(timer, 50);
				if(cursor_c >= 0)
				{
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 8, 44);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				}
			}
		}
	}
}

/* メモリの最大使用量のチェック */
unsigned int memoryUsage(struct MEMMAN *man)
{
	unsigned int result;
	
	/* 使用できるメモリ数のチェック */
	result = memtest(0x00400000, 0xbfffffff);
	
	return result;
}

/* メモリ管理の初期化 */
void memoryInit(struct MEMMAN *man, unsigned int memtotal)
{
	/* メモリ管理者の初期化 */
	memman_init(man, memtotal);
	/* メモリの使用量の設定 */
	memman_free(man, 0x00001000, 0x0009e000);
	memman_free(man, 0x00400000, memtotal-0x00400000);
	
	return;
}

/* ハードウェアの初期化 */
void hardWareInit(void)
{
	init_gdtidt();
	init_pic();
	/* IDTとPICの設定が完了したので、CPUへの割り込みを受け付ける */
	io_sti();
	init_pit();
}

/* マウス座標が閾値を超えていれば丸め込む */
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

void makeWindow8(unsigned char* buf, int xsize, int ysize, char *title, int act)
{
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	make_wtitle8(buf, xsize, title, act);
	return;
}

void make_wtitle8(unsigned char *buf, int xsize, char *title, char act)
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
	char c, tc, tbc;
	if(act != 0)
	{
		tc = COL8_FFFFFF;
		tbc = COL8_000084;
	}
	else
	{
		tc = COL8_C6C6C6;
		tbc = COL8_848484;
	}
	boxfill8(buf, xsize, tbc,         3,         3,         xsize - 4, 20       );
	putstr8_asc(buf, xsize, 24, 4, tc, title);
	
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