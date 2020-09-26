#include"bootpack.h"
//#include<string.h>

#define NULL 0
#define AR_CODE32_ER	0x409a
#define AR_DATA32_RW	0x4092

struct CONSOL_COMMAND_MEANING
{
	const char *name;
	const char *mean;
};

int memExecute(struct CONSOL_INFO *cons);
int clsExecute(struct CONSOL_INFO *cons);
int dirExecute(struct CONSOL_INFO *cons);
int typeExecute(struct CONSOL_INFO *cons, int *fat, char *cmdline);
int helpExecute(struct CONSOL_INFO *cons);
int hltExecute(struct CONSOL_INFO *cons, int *fat);
int badExecute(struct CONSOL_INFO *cons);
void commandRun(struct CONSOL_INFO *cons, int *fat, char *cmdline);
void cons_newline(struct CONSOL_INFO *cons);
void cons_putchar(struct CONSOL_INFO *cons, char chr, char move);
int cmd_app(struct CONSOL_INFO *cons, int *fat, char *cmdline);
void cons_putstr1(struct CONSOL_INFO *cons, char *s, int i);
void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col);
void putfont8_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);

/* コマンドの意味 */
struct CONSOL_COMMAND_MEANING commandMeaningTable[] =
{
	 {"mem",  "Memory Storage and using"}
	,{"dir",  "Print All File of Directory"}
	,{"cls",  "Clear Consol Display"}
	,{"type", "Contents of Specified file"}
};

int *inthandler0d(int *esp)
{
	struct TASK *task = task_now();
	struct CONSOL_INFO *cons = task->cons;
	char s[30];
	cons_putstr0(cons, "\nINT 0D :\n General Proteced Exception \n");
	sprintf(s, "EIP = %08X\n", esp[11]);
	cons_putstr0(cons, s);
	return &(task->tss.esp0); /* 異常終了させる */
}

int *inthandler0c(int *esp)
{
	struct TASK *task = task_now();
	struct CONSOL_INFO *cons = task->cons;
	char s[30];
	cons_putstr0(cons, "\nINT 0C :\n Stack Exception \n");
	sprintf(s, "EIP = %08X\n", esp[11]);
	cons_putstr0(cons, s);
	return &(task->tss.esp0); /* 異常終了させる */
}

int* hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	struct TASK *task = task_now();
	struct SHTCTL *shtctl = (struct SHTCTL *)*((int*)0x0fe4);
	struct SHEET *sht;
	int *reg = &eax + 1; /* reg[0]:edi reg[1]:esi reg[2]:ebp reg[3]:esp reg[4]:ebx reg[5]:edx
							reg[6]:ecx reg[7]:eax */
	int i;
	int dsbase = task->dsbase;
	struct CONSOL_INFO *cons = task->cons;
	
	if(edx == 1)
	{
		cons_putchar(cons, eax & 0xff, 1);
	}
	else if(edx == 2)
	{
		cons_putstr0(cons, (char *)ebx + dsbase);
		/*cons_putstr0(cons, s);*/
	}
	else if(edx == 3)
	{
		cons_putstr1(cons, (char *)ebx + dsbase, ecx);
	}
	else if(edx == 4)
	{
		return &(task->tss.esp0);
	}
	else if(edx == 5)
	{
		sht = sheet_alloc(shtctl);
		sht->task = task;
		sht->flags |= 0x10;
		sheet_setbuf(sht, (char*)ebx + dsbase, esi, edi, eax);
		makeWindow8((char*)ebx + dsbase, esi,edi,(char*)ecx + dsbase, 0);
		sheet_slide(sht,((shtctl->xsize - esi) / 2) & 3, (shtctl->ysize - edi) / 2);
		sheet_updown(sht, shtctl->top);
		reg[7] = (int)sht; 
	}
	else if(edx == 6)
	{
		sht = (struct SHEET*)ebx;
		putstr8_asc(sht->buf, sht->bxsize, esi, edi, eax, (char*)ebp + dsbase);
		sheet_refresh(sht,esi,edi,esi + ecx * 8, edi + 16);
	}
	else if(edx == 7)
	{
		sht = (struct SHEET*)ebx;
		boxfill8(sht->buf, sht->bxsize, ebp, esi, edi, eax, ecx);
		sheet_refresh(sht,esi,edi,eax + 1, ecx + 1);
	}
	else if(edx == 8)
	{
		memman_init((struct MEMMAN*)(ebx + dsbase));
		ecx &= 0xfffffff0; /* 16byte単位 */
		memman_free((struct MEMMAN*)(ebx + dsbase), eax, ecx);
	}
	else if(edx == 9)
	{
		ecx = (ecx + 0x0f) & 0xfffffff0;
		reg[7] = memman_alloc((struct MEMMAN*)(ebx + dsbase), ecx); /* 確保したメモリ領域の番地をEAXレジスタに格納する */
	}
	else if(edx == 10)
	{
		ecx = (ecx + 0x0f) & 0xfffffff0;
		memman_free((struct MEMMAN*)(ebx + dsbase), eax, ecx);
	}
	else if(edx == 11)
	{
		sht = (struct SHEET *)ebx;
		sht->buf[sht->bxsize * edi + esi] = eax;
	}
	else if(edx == 12)
	{
		sht = (struct SHEET *)ebx;
		sheet_refresh(sht, eax, ecx, esi, edi);
	}
	else if(edx == 13)
	{
		sht = (struct SHEET *)ebx;
		hrb_api_linewin(sht, eax, ecx, esi, edi, ebp);
	}
	else if(edx == 14)
	{
		sheet_free((struct SHEET*)ebx);
	}
	else if(edx == 15)
	{
		while(1)
		{
			io_cli();
			if(fifo32_status(&task->fifo) == 0)
			{
				if(eax != 0)
				{
					task_sleep(task); /* FIFOが空 */
				}
				else
				{
					io_sti();
					reg[7] = -1;
					return 0;
				}
			}
			i = fifo32_get(&task->fifo);
			io_sti();
			if(i <= 1)
			{
				timer_init(cons->timer, &task->fifo,1);
				timer_settime(cons->timer, 50);
			}
			if(i == 2)
			{
				cons->cur_c = COL8_FFFFFF;
			}
			if(i == 3)
			{
				cons->cur_c = -1;
			}
			if (i >= 256) { /* キーボードデータ（タスクA経由）など */
				reg[7] = i - 256;
				return 0;
			}
		}
	}
	else if (edx == 16)
	{
		reg[7] = (int) timer_alloc();
		((struct TIMER*)reg[7])->flags2 = 1;
	}
	else if (edx == 17)
	{
		timer_init((struct TIMER *) ebx, &task->fifo, eax + 256);
	}
	else if (edx == 18)
	{
		timer_settime((struct TIMER *) ebx, eax);
	}
	else if (edx == 19)
	{
		timer_free((struct TIMER *) ebx);
	}
	else if(edx == 20)
	{
		if(eax == 0)
		{
			i = io_in8(0x61);
			io_out8(0x61, i & 0x0d);
		}
		else
		{
			i = 1193180000 / eax; /* pitのクロック数は1.19318Mhz固定 */
			io_out8(0x43, 0xb6);
			io_out8(0x42, i & 0xff);
			io_out8(0x42, i >> 8);
			i = io_in8(0x61);
			io_out8(0x61, (i | 0x03) & 0x0f);
		}
	}
	
	return 0;
}

void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col)
{
	int i, x, y, len, dx, dy;
	
	dx = x1 - x0;
	dy = y1 - y0;
	x = x0 << 10;
	y = y0 << 10;
	if(dx < 0)
	{
		dx = -dx;
	}
	if(dy < 0)
	{
		dy = -dy;
	}
	if(dx >= dy)
	{
		len = dx + 1;
		if(x0 > x1)
		{
			dx = -1024;
		}
		else
		{
			dx = 1024;
		}
		if(y0 <= y1)
		{
			dy = ((y1- y0 + 1) << 10) / len; /* 一次関数の傾き */
		}
		else
		{
			dy = ((y1- y0 - 1) << 10) / len;
		}
	}
	else
	{
		len = dy + 1;
		if(y0 > y1)
		{
			dy = -1024;
		}
		else
		{
			dy = 1024;
		}
		if(x0 <= x1)
		{
			dx = ((x1- x0 + 1) << 10) / len;
		}
		else
		{
			dx = ((x1- x0 - 1) << 10) / len;
		}
	}
	
	for(i = 0; i < len; i++)
	{
		sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
		x += dx;
		y += dy;
	}
	
	return;
}

void cons_putstr0(struct CONSOL_INFO *cons, char *s)
{
	for(; *s != 0; s++)
	{
		cons_putchar(cons, *s, 1);
	}
	
	return;
}

void cons_putstr1(struct CONSOL_INFO *cons, char *s, int i)
{
	int j;
	
	for(j = 0; j < i; j++)
	{
		cons_putchar(cons, s[j], 1);
	}
	
	return;
}

void cons_newline(struct CONSOL_INFO *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	
	if(cons->cur_y < 28 + 112) /* コンソールの文字描画画面のy軸における限界数の1つ手前 */
	{
		cons->cur_y += 16;
	}
	else
	{
		/* 最初の行から最後の行1つ手前までのデータをずらす*/
		for(y = 28; y < 28 + 112; y++)
		{
			for(x = 8; x < 8 + 240; x++)
			{
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		/* 最後1行目を黒塗り */
		for(y = sheet->bysize - 25; y < sheet->bysize - 9 ; y++)
		{
			for(x = 8; x < sheet->bxsize - 8; x++)
			{
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, 28, sheet->bxsize - 8, sheet->bysize - 9);
	}
	
	cons->cur_x = 8;
}

void consol_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TASK *task = task_now(); /* スリープ処理に必要な為、自分自身のタスクを知る必要がある */
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct CONSOL_INFO cons;
	unsigned char cmdline[30];
	int i;
	int *fat = (int *)memman_alloc_4k(memman, 4 * 2880);
	
	cons.sht = sheet;
	cons.cur_x = 8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	task->cons = &cons;
	
	cons.timer = timer_alloc();
	timer_init(cons.timer, &task->fifo, 1);
	timer_settime(cons.timer, 50); /* 0.5秒でタイマーを張る */
	file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));

	cons_putchar(&cons, '>', 1);
	
	while(1)
	{
		io_cli();
		if(fifo32_status(&task->fifo) == 0)
		{
			task_sleep(task);
			io_sti();
		}
		else
		{
			i = fifo32_get(&task->fifo);
			io_sti();
			if(i <= 1)
			{
				if(0 != i)
				{
					timer_init(cons.timer, &task->fifo, 0);
					if(cons.cur_c >= 0)
					{
						cons.cur_c = COL8_FFFFFF; /* 白 */
					}
				}
				else
				{
					timer_init(cons.timer, &task->fifo, 1);
					if(cons.cur_c >= 0)
					{
						cons.cur_c = COL8_000000; /* 黒 */
					}
				}
				timer_settime(cons.timer, 50);
			}
			else if(2 == i) /* 点滅を許可 */
			{
				cons.cur_c = COL8_FFFFFF; /* 白 */
			}
			else if(3 == i) /* 点滅の禁止 */
			{
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				cons.cur_c = -1;
			}
			else
			{
				if(256 <= i && i <= 511)
				{
					if(i == 256 + 8)
					{
						if(cons.cur_x > 16)
						{
							cons_putchar(&cons, ' ', 0);
							cons.cur_x -= 8;
						}
					}
					else if(i == 0x1c + 256)
					{
						cons_putchar(&cons, ' ', 0);
						cmdline[cons.cur_x / 8 - 2] = 0;
						cons_newline(&cons);
						commandRun(&cons, fat, cmdline);
						cons_putchar(&cons, '>', 1);
					}
					else
					{
						if(cons.cur_x < 240)
						{
							cmdline[cons.cur_x / 8 - 2] = i - 256;
							cons_putchar(&cons, i - 256, 1);
						}
					}
				}
			}
			if(cons.cur_c >= 0)
			{
				boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
			}
			sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
		}
	}
}

int cmd_app(struct CONSOL_INFO *cons, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct FILEINFO *finfo;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)0x00270000;
	char name[18], *p, *q;
	struct TASK *task = task_now();
	int segsize, datasize,esp, datahrb;
	int i;
	struct SHEET *sht;
	struct SHTCTL *shtctl;

	/* コマンドラインからファイル名を生成 */
	for(i = 0; i < 13; i++)
	{
		if(cmdline[i] <= ' ')
		{
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0; /* ファイル名の後ろを0にする */
	
	finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224); /* 224はファイルの格納限界数 */
	
	/* 拡張子指定が無くても、起動できるように対応 */
	if(finfo == 0 && name[i - 1] != '.')
	{
		/* 拡張子をつけてもう一度探す */
		name[i  ] = '.';
		name[i+1] = 'H';
		name[i+2] = 'R';
		name[i+3] = 'B';
		name[i+4] = 0;
		finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	}
	
	if(finfo != 0)
	{
		p = (char *)memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat,(char *)(ADR_DISKIMG + 0x003e00));
		if(finfo->size >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00)
		{
			segsize  = *((int*)(p + 0x0000)); /* データセグメントの大きさ */
			esp      = *((int*)(p + 0x000c)); /* ESPの初期値&データ部分の転送先番地 */
			datasize = *((int*)(p + 0x0010)); /* hrbファイル内のデータ部分の大きさ */
			datahrb  = *((int*)(p + 0x0014)); /* hrbファイル内のデータ部分がどこから始まるのか */
			q = (char *)memman_alloc_4k(memman, segsize);
			task->dsbase = (int)q;
			set_segmdesc(gdt + task->sel / 8 + 1000, finfo->size - 1, (int)p, AR_CODE32_ER + 0x60);
			set_segmdesc(gdt + task->sel / 8 + 2000, segsize - 1, (int)q, AR_DATA32_RW + 0x60);
			for(i = 0;i < datasize; i++)
			{
				q[esp + i] = p[datahrb + i];
			}
			start_app(0x1b, task->sel + 1000 * 8, esp, task->sel + 2000 * 8, &(task->tss.esp0));
			shtctl = (struct SHTCTL*)*((int*)0x0fe4);
			for(i = 0; i < MAX_SHEETS; i++)
			{
				sht = &(shtctl->sheets0[i]);
				if((sht->flags & 0x11) == 0x11 && sht->task == task)
				{
					/* アプリが開きっぱなしにしたウインドを閉じる */
					sheet_free(sht);
				}
			}
			timer_cancelall(&task->fifo);
			memman_free_4k(memman, (int)q, segsize);
		}
		else
		{
			cons_putstr0(cons, ".hrb file format error.\n");
		}
		memman_free_4k(memman, (int)p, finfo->size);
		cons_newline(cons);
		return 0;
	}
	
	/* ファイルが見つからなかった */
	return -1;
}

/* コマンドの実行 */
void commandRun(struct CONSOL_INFO *cons, int *fat, char *cmdline)
{
	
	if(strcmp(cmdline, "mem") == 0)
	{
		memExecute(cons);
	}
	else if(strcmp(cmdline, "cls") == 0)
	{
		clsExecute(cons);
	}
	else if(strcmp(cmdline, "dir") == 0)
	{
		dirExecute(cons);
	}
	else if(strncmp(cmdline, "type ", 5) == 0)
	{
		typeExecute(cons, fat, cmdline);
	}
	else if(strcmp(cmdline, "help") == 0)
	{
		helpExecute(cons);
	}
	else if(cmdline[0] != 0)
	{
		if(cmd_app(cons, fat, cmdline) != 0)
		{
			/* コマンドでなく、アプリでなく、改行でもない */
			badExecute(cons);
		}
	}
	
	return;
}

int memExecute(struct CONSOL_INFO *cons)
{
	char s[30];
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	
	if(NULL == cons)
	{
		return -1;
	}
	
	/* 全体のメモリ量と残量を表示する */
	sprintf(s, "total  %dMB\nfree  %dKB\n\n", memman->storage / (1024 * 1024), memman_total(memman) / 1024);
	cons_putstr0(cons, s);
	
	return 0;
}

int dirExecute(struct CONSOL_INFO *cons)
{
	char s[30];
	struct FILEINFO *fileInfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
	int x, y;
	
	if(NULL == cons)
	{
		return -1;
	}
	
	for(x = 0; x < 224; x++)
	{
		if(fileInfo[x].name[0] == 0x00)
		{
			break;
		}
		if(fileInfo[x].name[0] != 0xe5)
		{
			if((fileInfo[x].type & 0x18) == 0) /* ファイルでない情報+ディレクトリでない場合 */
			{
				sprintf(s, "fileInfo.ext    %7d\n", fileInfo[x].size);
				for(y = 0; y < 8; y++)
				{
					s[y] = fileInfo[x].name[y];
				}
				s[9]  = fileInfo[x].ext[0];
				s[10] = fileInfo[x].ext[1];
				s[11] = fileInfo[x].ext[2];
				cons_putstr0(cons, s);
			}
		}
	}
	cons_newline(cons);
	
	return 0;
}
int clsExecute(struct CONSOL_INFO *cons)
{
	int x, y;
	struct SHEET *sheet;
	
	if(NULL == cons)
	{
		return -1;
	}
	
	sheet = cons->sht;
	
	for(y = 28; y < 128 + 28; y++)
	{
		for(x = 8; x < 240 + 8; x++)
		{
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28,240 + 8, 128 + 28);
	cons->cur_y = 28;
	
	return 0;
}

int badExecute(struct CONSOL_INFO *cons)
{
	if(NULL == cons)
	{
		return -1;
	}
	
	cons_putstr0(cons, "Bad Command\n\n");
	
	return 0;
}

int typeExecute(struct CONSOL_INFO *cons, int *fat, char *cmdline)
{
	struct FILEINFO *fileInfo = file_search(cmdline + 5, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224); /* 224はファイルの格納限界数 */
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	char *p;
	
	if(NULL == cons || fat == NULL || cmdline == NULL)
	{
		return -1;
	}
	
	if(fileInfo != NULL)
	{
		/* ファイルが見つかった場合 */
		p = (char *)memman_alloc_4k(memman, fileInfo->size);
		file_loadfile(fileInfo->clustno, fileInfo->size, p, fat,(char *)(ADR_DISKIMG + 0x003e00));
		cons_putstr1(cons, p, fileInfo->size);
		memman_free_4k(memman, (int)p, fileInfo->size);
	}
	else
	{
		/* ファイルが見つからなかった場合 */
		cons_putstr0(cons, "File not found\n");
	}
	
	cons_newline(cons);
	return 0;
}

int helpExecute(struct CONSOL_INFO *cons)
{
	int i;
	
	if(NULL == cons)
	{
		return -1;
	}
	
	for(i = 0; i < sizeof(commandMeaningTable)/sizeof(struct CONSOL_COMMAND_MEANING); i++)
	{
	//	putfont8_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, commandMeaningTable[i].name, 30);
		cons_newline(cons);
	//	putfont8_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, commandMeaningTable[i].mean, 30);
		cons_newline(cons);
	}
	
	return 0;
}

int hltExecute(struct CONSOL_INFO *cons, int *fat)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct FILEINFO *fileInfo = file_search("HLT.HRB", (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224); /* 224はファイルの格納限界数 */
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)0x00270000;
	char *p;
	
	if(NULL != fileInfo)
	{
		/* ファイルが見つかった場合 */
		p = (char *)memman_alloc_4k(memman, fileInfo->size);
		file_loadfile(fileInfo->clustno, fileInfo->size, p, fat,(char *)(ADR_DISKIMG + 0x003e00));
		set_segmdesc(gdt + 1003, fileInfo->size - 1, (int)p, AR_CODE32_ER);
		farcall(0,1003*8);
		memman_free_4k(memman, (int)p, fileInfo->size);
	}
	else
	{
		/* ファイルが見つからなかった場合 */
		putfont8_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found", 15);
		cons_newline(cons);
	}
	
	cons_newline(cons);
	return 0;
}

void cons_putchar(struct CONSOL_INFO *cons, char chr, char move)
{
	char s[2];
	
	s[0] = chr;
	s[1] = 0;
	if(s[0] == 0x09) /* タブ */
	{
		while(1)
		{
			putfont8_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			cons->cur_x += 8;
			if(cons->cur_x == 8 + 240)
			{
				cons_newline(cons);
			}
			if(((cons->cur_x - 8) & 0x1f) == 0)
			{
				break; /* 32で割り切れる */
			}
		}
	}
	else if(s[0] == 0x0a) /* 改行 */
	{
		cons_newline(cons);
	}
	else if(s[0] == 0x0d) /* 復帰 */
	{
		/* 何もしない */
	}
	else
	{
		putfont8_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		if(move != 0)
		{
			cons->cur_x += 8;
			if(cons->cur_x == 8 + 240) /* 右端まで到達した */
			{
				cons_newline(cons);
			}
		}
	}
}


