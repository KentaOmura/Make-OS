#include"bootpack.h"

#define NULL 0
#define AR_CODE32_ER	0x409a

struct CONSOL_COMMAND_MEANING
{
	const char *name;
	const char *mean;
};

struct CONSOL_INFO
{
	struct SHEET *sht;
	int cur_x, cur_y, cur_c;
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

/* �R�}���h�̈Ӗ� */
struct CONSOL_COMMAND_MEANING commandMeaningTable[] =
{
	 {"mem",  "Memory Storage and using"}
	,{"dir",  "Print All File of Directory"}
	,{"cls",  "Clear Consol Display"}
	,{"type", "Contents of Specified file"}
};

void cons_newline(struct CONSOL_INFO *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	
	if(cons->cur_y < 28 + 112) /* �R���\�[���̕����`���ʂ�y���ɂ�������E����1��O */
	{
		cons->cur_y += 16;
	}
	else
	{
		/* �ŏ��̍s����Ō�̍s1��O�܂ł̃f�[�^�����炷*/
		for(y = 28; y < 28 + 112; y++)
		{
			for(x = 8; x < 8 + 240; x++)
			{
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		/* �Ō�1�s�ڂ����h�� */
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
	struct TIMER *timer;
	struct TASK *task = task_now(); /* �X���[�v�����ɕK�v�ȈׁA�������g�̃^�X�N��m��K�v������ */
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct CONSOL_INFO cons;
	unsigned char cmdline[30];
	int i, fifo_buf[128];
	int *fat = (int *)memman_alloc_4k(memman, 4 * 2880);
	
	cons.sht = sheet;
	cons.cur_x = 8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	*((int *)0x0fec) = (int)&cons;
	
	fifo32_init(&task->fifo, 128, fifo_buf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50); /* 0.5�b�Ń^�C�}�[�𒣂� */
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
					timer_init(timer, &task->fifo, 0);
					if(cons.cur_c >= 0)
					{
						cons.cur_c = COL8_FFFFFF; /* �� */
					}
				}
				else
				{
					timer_init(timer, &task->fifo, 1);
					if(cons.cur_c >= 0)
					{
						cons.cur_c = COL8_000000; /* �� */
					}
				}
				timer_settime(timer, 50);
			}
			else if(2 == i) /* �_�ł����� */
			{
				cons.cur_c = COL8_FFFFFF; /* �� */
			}
			else if(3 == i) /* �_�ł̋֎~ */
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
	char name[18], *p;
	int i;
	
	/* �R�}���h���C������t�@�C�����𐶐� */
	for(i = 0; i < 13; i++)
	{
		if(cmdline[i] <= ' ')
		{
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0; /* �t�@�C�����̌���0�ɂ��� */
	
	finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224); /* 224�̓t�@�C���̊i�[���E�� */
	
	/* �g���q�w�肪�����Ă��A�N���ł���悤�ɑΉ� */
	if(finfo == 0 && name[i - 1] != '.')
	{
		/* �g���q�����Ă�����x�T�� */
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
		set_segmdesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER);
		farcall(0,1003*8);
		memman_free_4k(memman, (int)p, finfo->size);
		cons_newline(cons);
		return 0;
	}
	
	/* �t�@�C����������Ȃ����� */
	return -1;
}

/* �R�}���h�̎��s */
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
			/* �R�}���h�łȂ��A�A�v���łȂ��A���s�ł��Ȃ� */
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
	
	/* �S�̂̃������ʂƎc�ʂ�\������ */
	sprintf(s, "total  %dMB", memman->storage / (1024 * 1024));
	putfont8_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s,30);
	cons_newline(cons);
	sprintf(s, "free  %dKB", memman_total(memman) / 1024);
	putfont8_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s,30);
	cons_newline(cons);
	cons_newline(cons);
	
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
			if((fileInfo[x].type & 0x18) == 0) /* �t�@�C���łȂ����+�f�B���N�g���łȂ��ꍇ */
			{
				sprintf(s, "fileInfo.ext    %7d", fileInfo[x].size);
				for(y = 0; y < 8; y++)
				{
					s[y] = fileInfo[x].name[y];
				}
				s[9]  = fileInfo[x].ext[0];
				s[10] = fileInfo[x].ext[1];
				s[11] = fileInfo[x].ext[2];
				putfont8_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s,30);
				cons_newline(cons);
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
	
	putfont8_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "Bad Command",30);
	cons_newline(cons);
	cons_newline(cons);
	
	return 0;
}

int typeExecute(struct CONSOL_INFO *cons, int *fat, char *cmdline)
{
	unsigned char s[15]; /* �t�@�C�����i�[�p */
	struct FILEINFO *fileInfo = file_search(cmdline + 5, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224); /* 224�̓t�@�C���̊i�[���E�� */
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	int x, y;
	int search = 0;
	char *p;
	
	if(NULL == cons || fat == NULL || cmdline == NULL)
	{
		return -1;
	}
	
	if(fileInfo != NULL)
	{
		/* �t�@�C�������������ꍇ */
		p = (char *)memman_alloc_4k(memman, fileInfo->size);
		file_loadfile(fileInfo->clustno, fileInfo->size, p, fat,(char *)(ADR_DISKIMG + 0x003e00));
		for(y = 0; y < fileInfo->size; y++)
		{
			cons_putchar(cons, p[y], 1);
		}
		memman_free_4k(memman, (int)p, fileInfo->size);
	}
	else
	{
		/* �t�@�C����������Ȃ������ꍇ */
		putfont8_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found", 15);
		cons_newline(cons);
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
		putfont8_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, commandMeaningTable[i].name, 30);
		cons_newline(cons);
		putfont8_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, commandMeaningTable[i].mean, 30);
		cons_newline(cons);
	}
	
	return 0;
}

int hltExecute(struct CONSOL_INFO *cons, int *fat)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct FILEINFO *fileInfo = file_search("HLT.HRB", (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224); /* 224�̓t�@�C���̊i�[���E�� */
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)0x00270000;
	char *p;
	
	if(NULL != fileInfo)
	{
		/* �t�@�C�������������ꍇ */
		p = (char *)memman_alloc_4k(memman, fileInfo->size);
		file_loadfile(fileInfo->clustno, fileInfo->size, p, fat,(char *)(ADR_DISKIMG + 0x003e00));
		set_segmdesc(gdt + 1003, fileInfo->size - 1, (int)p, AR_CODE32_ER);
		farcall(0,1003*8);
		memman_free_4k(memman, (int)p, fileInfo->size);
	}
	else
	{
		/* �t�@�C����������Ȃ������ꍇ */
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
	if(s[0] == 0x09) /* �^�u */
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
				break; /* 32�Ŋ���؂�� */
			}
		}
	}
	else if(s[0] == 0x0a) /* ���s */
	{
		cons_newline(cons);
	}
	else if(s[0] == 0x0d) /* ���A */
	{
		/* �������Ȃ� */
	}
	else
	{
		putfont8_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		if(move != 0)
		{
			cons->cur_x += 8;
			if(cons->cur_x == 8 + 240) /* �E�[�܂œ��B���� */
			{
				cons_newline(cons);
			}
		}
	}
}


