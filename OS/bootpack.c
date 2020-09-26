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
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);
void makeTextbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void keywin_off(struct SHEET *key_win);
void keywin_on(struct SHEET *key_win);

struct MOUSE_COODINATE getMouseCoodinate()
{
	struct MOUSE_COODINATE temp;
	
	temp.mx = 20;
	temp.my = 20;
	
	return temp;
}

void HariMain(void)
{
	unsigned char s[50];

	struct MOUSE_DEC mdec;
	struct FIFO32 fifo;
	int fifobuf[256], keycmd_buf[3], *cons_fifo[2];
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	/* 起動時の画面情報を取得 */
	struct BOOT_INFO *binfo = (struct BOOT_INFO*)0x0ff0;
	/* シート */
	struct SHTCTL *shtctl;
	struct SHEET *sht = 0, *key_win;
	struct SHEET *sht_back, *sht_mouse, *sht_cons[2];
	struct MOUSE_COODINATE mouseCoodinate; 
	unsigned char *buf_back, buf_mouse[256], *buf_cons[2];
	int i, j ,x, y;
	int mmx = -1;
	int mmy = -1;
	int mmx2 = 0;
	struct TIMER *timer;
	int maxch;
	int key_shift = 0, key_leds = (binfo->leds >> 4) & 7;
	struct TASK *task_a, *task_cons[2], *task;
	struct MOUSE_COODINATE test;
	
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
	
	test = getMouseCoodinate();
	
	/* shtctlを保持 */
	*((int*)0x0fe4) = (int)shtctl;
	
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
	for(i = 0; i < 2; i++)
	{
		sht_cons[i] = sheet_alloc(shtctl);
		buf_cons[i] = (unsigned char*)memman_alloc_4k(memman,256*165);
		sheet_setbuf(sht_cons[i], buf_cons[i], 256, 165, -1);
		makeWindow8(buf_cons[i], 256, 165, "consols", 0);
		makeTextbox8(sht_cons[i], 8, 28, 240, 128, COL8_000000);
		task_cons[i] = task_alloc();
		task_cons[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
		task_cons[i]->tss.eip = (int)&consol_task;
		task_cons[i]->tss.es = 1 * 8;
		task_cons[i]->tss.cs = 2 * 8;
		task_cons[i]->tss.ss = 1 * 8;
		task_cons[i]->tss.ds = 1 * 8;
		task_cons[i]->tss.fs = 1 * 8;
		task_cons[i]->tss.gs = 1 * 8;
		*((int *) (task_cons[i]->tss.esp + 4)) = (int)sht_cons[i];
		*((int *) (task_cons[i]->tss.esp + 8)) = (int)memman->storage;
		task_run(task_cons[i], 2, 2);
		sht_cons[i]->task   = task_cons[i];
		sht_cons[i]->flags |= 0x20; /* カーソルあり */
		cons_fifo[i] = (int *)memman_alloc_4k(memman,128 * 4);
		fifo32_init(&task_cons[i]->fifo, 128, cons_fifo[i], task_cons[i]);
	}
	
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 8, 8, COL8_0000FF);
	
	/* マウスカーソルのデータを取得 */
	init_mouse_cursol(buf_mouse, COL8_0000FF);
	maxch = (144 - 8) / 8; /* 最大文字数 */
	
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);
	
	/* マウスカーソルを描画する */
	mouseCoodinate.mx = (binfo->screen_x - 8) / 2;
	mouseCoodinate.my = (binfo->screen_y -28 - 8) / 2;
	
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_cons[1], 56, 6);
	sheet_slide(sht_cons[0], 8, 2);
	sheet_slide(sht_mouse, mouseCoodinate.mx, mouseCoodinate.my);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_cons[1], 1);
	sheet_updown(sht_cons[0], 2);
	sheet_updown(sht_mouse, 3);
	key_win = sht_cons[0];
	keywin_on(key_win);
	
	while(1)
	{
		io_cli(); /* 割り込みを禁止する */
		if(0 == fifo32_status(&fifo))
		{
			/* FIFOが空になったので、ため込んでいる描画処理を実施する */
			//if()
			
			task_sleep(task_a); /* 他のタスクにCPUのリソースを割り振る */
			io_sti();
		}
		else
		{
			i = fifo32_get(&fifo); 
			io_sti();
			if(key_win->flags == 0)
			{
				key_win = shtctl->sheets[shtctl->top - 1];
			}
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
					/* コンソールに送信 */
					fifo32_put(&key_win->task->fifo, s[0] + 256);
				}
				if(i == 256 + 0x57 && shtctl->top > 2)
				{
					sheet_updown(shtctl->sheets[1], shtctl->top - 1);
				}
				
				/* CapsLockの状態を更新する */
				if(i == 256 + 0xba)
				{
					key_leds ^= 4;
				}
				if(i == 256 + 0x0e)
				{
					/* コンソールに送信 */
					fifo32_put(&key_win->task->fifo, 8 + 256);
				}
				if(i == 256 + 0x1c)
				{
					fifo32_put(&key_win->task->fifo, 0x1c + 256);
				}
				if(i == 256 + 0x0f)
				{
					keywin_off(key_win);
					j = key_win->height - 1;
					if(j == 0)
					{
						j = shtctl->top - 1;
					}
					key_win  = shtctl->sheets[j];
					keywin_on(key_win);
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
				if(i == 256 + 0x3b && key_shift != 0)
				{
					task = key_win->task;
					if(task != 0 && task->tss.ss0 != 0)
					{
						io_cli();
						task->tss.eax = (int)&(task->tss.esp0);
						task->tss.eip = (int)asm_end_app;
						io_sti();
					}
				}
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
						if(mmx < 0)
						{
							for(j = shtctl->top - 1; j > 0; j--)
							{
								sht = shtctl->sheets[j];
								x = mouseCoodinate.mx - sht->vx0;
								y = mouseCoodinate.my - sht->vy0;
								if(0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize)
								{
									if(sht->buf[y * sht->bxsize + x] != sht->col_inv)
									{
										sheet_updown(sht, shtctl->top - 1);
										if(sht != key_win)
										{
											keywin_off(key_win);
											key_win = sht;
											keywin_on(key_win);
										}
										/* ウインドウタイトルをクリックされているか */
										if(3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21)
										{
											mmx = mouseCoodinate.mx;
											mmy = mouseCoodinate.my;
											mmx2 = sht->vx0;
										}
										/* ×をクリックしているか */
										if(sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19)
										{
											if((sht->flags & 0x10) != 0) /* アプリが作成したウインドう */
											{
												task = sht->task;
												io_cli(); /* 強制終了中はタスク切り替えを禁止する */
												task->tss.eax = (int)&(task->tss.esp0);
												task->tss.eip = (int)asm_end_app;
												io_sti();
											}
										}
										break;
									}
								}
							}
						}
						else
						{
							x = mouseCoodinate.mx - mmx;
							y = mouseCoodinate.my - mmy;
							sheet_slide(sht, (mmx2 + x + 2) & ~3, sht->vy0 + y);
							mmy = mouseCoodinate.my;
						}
					}
					else
					{
						mmx = -1;
					}
				}
			}
		}
	}
}

void keywin_off(struct SHEET *key_win)
{
	change_wtitle8(key_win, 0);
	if((key_win->flags & 0x20) != 0)
	{
		fifo32_put(&key_win->task->fifo, 3);
	}
	
	return;
}

void keywin_on(struct SHEET *key_win)
{
	change_wtitle8(key_win, 1);
	if((key_win->flags & 0x20) != 0)
	{
		fifo32_put(&key_win->task->fifo, 2);
	}
	
	return;
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
	memman_init(man);
	man->storage = memtotal;
	
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