#include"bootpack.h"

#define NULL 0

typedef enum CONSOL_COMMAND
{
	 CONSOL_COMMAND_NON	= 0	/* �R�}���h���� */
	,CONSOL_COMMAND_MEM		/* �������\�� */
	,CONSOL_COMMAND_DIR		/* �f�B���N�g�����̃t�@�C���\�� */
	,CONSOL_COMMAND_CLS		/* �R���\�[����ʂ̃N���A */
	,CONSOL_COMMAND_TYPE	/* �w�肵���t�@�C���̒��g��\������ */
	,CONSOL_COMMAND_HELP	/* �R�}���h�̎�ʂƈӖ���\������ */
	,CONSOL_COMMAND_BAD		/* �R�}���h�s�� */
}eCONSOL_COMMAND;

struct CONSOL_COMMAND_EXE
{
	eCONSOL_COMMAND kind; /* ��� */
	int (*execute)(int **argv); /* ���s�֐� */
};

struct CONSOL_COMMAND_MEANING
{
	const char *name;
	const char *mean;
};

int memExecute(int **argv);
int dirExecute(int **argv);
int clsExecute(int **argv);
int typeExecute(int **argv);
int helpExecute(int **argv);
int badExecute(int **argv);
int execute(eCONSOL_COMMAND kind, int **argv, int cursol_y);
int cons_newline(int cursol_y, struct SHEET *sheet);

/* �R���\�[���^�X�N�̎��s�����e�[�u�� */
struct CONSOL_COMMAND_EXE commandExeTable[] =
{
	 {CONSOL_COMMAND_MEM, &memExecute}
	,{CONSOL_COMMAND_DIR, &dirExecute}
	,{CONSOL_COMMAND_CLS, &clsExecute}
	,{CONSOL_COMMAND_TYPE,&typeExecute}
	,{CONSOL_COMMAND_HELP,&helpExecute}
	,{CONSOL_COMMAND_BAD, &badExecute}
};

/* �R�}���h�̈Ӗ� */
struct CONSOL_COMMAND_MEANING commandMeaningTable[] =
{
	 {"mem",  "Memory Storage and using"}
	,{"dir",  "Print All File of Directory"}
	,{"cls",  "Clear Consol Display"}
	,{"type", "Contents of Specified file"}
};

/* �R�}���h���s */
int execute(eCONSOL_COMMAND kind, int **argv, int cursol_y)
{
	int i = 0;
	int result = cursol_y;
	
	if(CONSOL_COMMAND_NON == kind)
	{
		return result;
	}
	
	for(; i < sizeof(commandExeTable)/sizeof(struct CONSOL_COMMAND_EXE); i++)
	{
		if(commandExeTable[i].kind == kind)
		{
			result = commandExeTable[i].execute(argv);
		}
	}
	
	return result;
}

int cons_newline(int cursol_y, struct SHEET *sheet)
{
	int x, y;
	if(cursol_y < sheet->bysize - 32)
	{
		cursol_y += 16;
	}
	else
	{
		/* �ŏ��̍s����Ō�̍s1��O�܂ł̃f�[�^�����炷*/
		for(y = 28; y < sheet->bysize - 25; y++)
		{
			for(x = 8; x < sheet->bxsize - 8; x++)
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
		sheet_refresh(sheet, 8, 28,sheet->bxsize - 8, sheet->bysize - 9);
	}
	
	return cursol_y;
}

void consol_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct FILEINFO *fileInfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
	struct TIMER *timer;
	struct TASK *task = task_now(); /* �X���[�v�����ɕK�v�ȈׁA�������g�̃^�X�N��m��K�v������ */
	unsigned char s[30], cmdline[30];
	int i, fifo_buf[128], cursol_x = 16, cursol_y = 28, cursol_c = -1;
	int x, y;
	int *consolArg[10];
	eCONSOL_COMMAND type;
	
	fifo32_init(&task->fifo, 128, fifo_buf, task);
	
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50); /* 0.5�b�Ń^�C�}�[�𒣂� */
	
	putfont8_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);
	
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
					if(cursol_c >= 0)
					{
						cursol_c = COL8_FFFFFF; /* �� */
					}
				}
				else
				{
					timer_init(timer, &task->fifo, 1);
					if(cursol_c >= 0)
					{
						cursol_c = COL8_000000; /* �� */
					}
				}
				timer_settime(timer, 50);
			}
			else if(2 == i) /* �_�ł����� */
			{
				cursol_c = COL8_FFFFFF; /* �� */
			}
			else if(3 == i) /* �_�ł̋֎~ */
			{
				cursol_c = -1;
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursol_x, cursol_y, cursol_x + 7, cursol_y + 15);
				sheet_refresh(sheet, cursol_x, cursol_y, cursol_x + 8, cursol_y + 16);
			}
			else
			{
				if(256 <= i && i <= 511)
				{
					if(i == 256 + 8)
					{
						if(cursol_x > 16)
						{
							putfont8_sht(sheet, cursol_x, cursol_y, COL8_FFFFFF, COL8_000000, " ",1);
							cursol_x -= 8;
						}
					}
					else if(i == 0x1c + 256)
					{
						type = CONSOL_COMMAND_NON;
						putfont8_sht(sheet, cursol_x, cursol_y, COL8_FFFFFF, COL8_000000, " ",1);
						cmdline[cursol_x / 8 - 2] = 0;
						cursol_y = cons_newline(cursol_y, sheet);
						/* �R�}���h�����s���� */
						if(strcmp(cmdline, "mem") == 0)
						{
							consolArg[0] = (int *)sheet;
							consolArg[1] = (int *)memman;
							consolArg[2] = &cursol_y;
							type = CONSOL_COMMAND_MEM;
						}
						else if(strcmp(cmdline, "cls") == 0)
						{
							consolArg[0] = (int *)sheet;	
							type = CONSOL_COMMAND_CLS;
						}
						else if(strcmp(cmdline, "dir") == 0)
						{
							consolArg[0] = (int *)sheet;
							consolArg[1] = (int *)fileInfo;
							consolArg[2] = &cursol_y;
							type = CONSOL_COMMAND_DIR;
						}
						else if(strncmp(cmdline, "type ", 5) == 0)
						{
							consolArg[0] = (int *)sheet;
							consolArg[1] = (int *)fileInfo;
							consolArg[2] = (int *)cmdline;
							consolArg[3] = &cursol_y;
							consolArg[4] = (int *)memman;
							type = CONSOL_COMMAND_TYPE;
						}
						else if(strcmp(cmdline, "help") == 0)
						{
							consolArg[0] = (int *)sheet;
							consolArg[1] = &cursol_y;
							type = CONSOL_COMMAND_HELP;
						}
						else if(cmdline[0] != 0)
						{
							consolArg[0] = (int *)sheet;
							consolArg[1] = &cursol_y;
							type = CONSOL_COMMAND_BAD;
						}
						cursol_y = execute(type, consolArg, cursol_y);
						putfont8_sht(sheet, 8, cursol_y, COL8_FFFFFF, COL8_000000, ">",1);
						cursol_x = 16;
					}
					else
					{
						if(cursol_x < sheet->bxsize - 16)
						{
							s[0] = i - 256;
							s[1] = 0;
							cmdline[cursol_x / 8 - 2] = i - 256;
							putfont8_sht(sheet, cursol_x, cursol_y, COL8_FFFFFF, COL8_000000, s,1);
							cursol_x += 8;
						}
					}
				}
			}
			if(cursol_c >= 0)
			{
				boxfill8(sheet->buf, sheet->bxsize, cursol_c, cursol_x, cursol_y, cursol_x + 7, cursol_y + 15);
			}
			sheet_refresh(sheet, cursol_x, cursol_y, cursol_x + 8, cursol_y + 16);
		}
	}
}

/* argv[0] = sheet, argv[1] = memman, argv[2] = cursol_y*/
int memExecute(int **argv)
{
	char s[30];
	struct SHEET *sheet = (struct SHEET *)argv[0];
	struct MEMMAN *memman = (struct MEMMAN *)argv[1];
	int cursol_y = *argv[2];
	int result = -1;
	
	if(sheet == NULL || memman == NULL)
	{
		return result;
	}
	
	/* mem�R�}���h */
	sprintf(s, "total  %dMB", memman->storage / (1024 * 1024));
	putfont8_sht(sheet, 8, cursol_y, COL8_FFFFFF, COL8_000000, s,30);
	cursol_y = cons_newline(cursol_y, sheet);
	sprintf(s, "free  %dKB", memman_total(memman) / 1024);
	putfont8_sht(sheet, 8, cursol_y, COL8_FFFFFF, COL8_000000, s,30);
	cursol_y = cons_newline(cursol_y, sheet);
	cursol_y = cons_newline(cursol_y, sheet);

	/* �ŏI�I�ȃJ�[�\���̍s��ݒ肷�� */
	result = cursol_y;
	
	return result;
}

int dirExecute(int **argv)
{
	char s[30];
	struct SHEET *sheet = (struct SHEET *)argv[0];
	struct FILEINFO *fileInfo = (struct FILEINFO *)argv[1];
	int cursol_y = *argv[2];
	int x, y;
	int result = -1;
	
	
	if(sheet == NULL || fileInfo == NULL)
	{
		return result;
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
				putfont8_sht(sheet, 8, cursol_y, COL8_FFFFFF, COL8_000000, s,30);
				cursol_y = cons_newline(cursol_y, sheet);
			}
		}
	}
	cursol_y = cons_newline(cursol_y, sheet);
	
	result = cursol_y;
	
	return result;
}
int clsExecute(int **argv)
{
	struct SHEET *sheet = (struct SHEET *)argv[0];
	int result = -1;
	int x, y;
	
	if(sheet == NULL)
	{
		return result;
	}
	
	for(y = 28; y < sheet->bysize - 9; y++)
	{
		for(x = 8; x < sheet->bxsize - 8; x++)
		{
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28,sheet->bxsize - 8, sheet->bysize - 9);
	
	result = 28;
	
	return result;
}
int badExecute(int **argv)
{
	struct SHEET *sheet = (struct SHEET *)argv[0];
	int cursol_y = *argv[1];
	int result = -1;
	
	if(sheet == NULL)
	{
		return result;
	}
	
	putfont8_sht(sheet, 8, cursol_y, COL8_FFFFFF, COL8_000000, "Bad Command",30);
	cursol_y = cons_newline(cursol_y, sheet);
	cursol_y = cons_newline(cursol_y, sheet);
	result = cursol_y;
	
	return result;
}
int typeExecute(int **argv)
{
	unsigned char s[15]; /* �t�@�C�����i�[�p */
	struct SHEET *sheet = (struct SHEET *)argv[0];
	struct FILEINFO *finfo = (struct FILEINFO *)argv[1];
	unsigned char *cmdline = (unsigned char *)argv[2];
	int cursol_y = *argv[3];
	struct MEMMAN *memman = (struct MEMMAN *)argv[4];
	int cursol_x;
	int x, y;
	int search = 0;
	char *p;
	int *fat = (int *)memman_alloc_4k(memman, 4 * 2880);
	
	file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));

	for(y = 0; y < 11; y++)
	{
		s[y] = ' ';
	}
	y = 0;
	
	/* type ���̕������l������5�X�^�[�g
	   �t�@�C������ݒ肷��
	*/
	for(x = 5; y < 11 && cmdline[x] != 0; x++)
	{
		/* �g���q */
		if(cmdline[x] == '.' && y <= 8)
		{
			y = 8;
		}
		else
		{
			s[y] = cmdline[x];
			/* �������͑啶���ɕϊ����� */
			if('a' <= s[y] && s[y] <= 'z')
			{
				s[y] -= 0x20;
			}
			y++;
		}
	}
	/* �t�@�C����T�� */
	for(x= 0; x < 224;x++) /* 224�̓t�@�C���̃f�[�^���i�[�ł�����E�̐� */
	{
		if(finfo[x].name[0] == 0x00)
		{
			break;
		}
		/* �f�B���N�g���܂��́A�t�@�C���łȂ����͑ΏۊO */
		if((finfo[x].type & 0x18) == 0)
		{
			search = 1;
			for(y = 0; y < 11; y++)
			{
				if(finfo[x].name[y] != s[y])
				{
					search = -1;
					break;
				}
			}
			if(search == 1)
			{
				break;
			}
		}
	}
	if(x < 224 && finfo[x].name[0] != 0x00)
	{
		/* �t�@�C�������������ꍇ */
		p = (char *)memman_alloc_4k(memman, finfo[x].size);
		file_loadfile(finfo[x].clustno, finfo[x].size, p, fat,(char *)(ADR_DISKIMG + 0x003e00));
		cursol_x = 8;
		for(y = 0; y < finfo[x].size; y++)
		{
			/* 1�������o�͂��� */
			s[0] = p[y];
			s[1] = 0;
			if(s[0] == 0x09) /* �^�u */
			{
				while(1)
				{
					putfont8_sht(sheet, cursol_x, cursol_y, COL8_FFFFFF, COL8_000000, " ", 1);
					cursol_x += 8;
					if(cursol_x == 8 + 240)
					{
						cursol_x = 8;
						cursol_y = cons_newline(cursol_y, sheet);
					}
					if(((cursol_x - 8) & 0x1f) == 0)
					{
						break; /* 32�Ŋ���؂�� */
					}
				}
			}
			else if(s[0] == 0x0a) /* ���s */
			{
				cursol_x = 8;
				cursol_y = cons_newline(cursol_y, sheet);
			}
			else if(s[0] == 0x0d) /* ���A */
			{
			}
			else
			{
				putfont8_sht(sheet, cursol_x, cursol_y, COL8_FFFFFF, COL8_000000, s, 1);
				cursol_x += 8;
				if(cursol_x == 8 + 240) /* �E�[�܂œ��B���� */
				{
					cursol_x = 8;
					cursol_y = cons_newline(cursol_y, sheet);
				}
			}
		}
		memman_free_4k(memman, (int)p,finfo[x].size);
	}
	else
	{
		/* �t�@�C����������Ȃ������ꍇ */
		putfont8_sht(sheet, 8, cursol_y, COL8_FFFFFF, COL8_000000, "File not found", 15);
		cursol_y = cons_newline(cursol_y, sheet);
	}
	
	cursol_y = cons_newline(cursol_y, sheet);

	return cursol_y;
}

int helpExecute(int **argv)
{
	struct SHEET *sheet = (struct SHEET *)argv[0];
	int cursol_y = *argv[1];
	int i;
	
	for(i = 0; i < sizeof(commandMeaningTable)/sizeof(struct CONSOL_COMMAND_MEANING); i++)
	{
		putfont8_sht(sheet, 8, cursol_y, COL8_FFFFFF, COL8_000000, commandMeaningTable[i].name, 30);
		cursol_y = cons_newline(cursol_y, sheet);
		putfont8_sht(sheet, 8, cursol_y, COL8_FFFFFF, COL8_000000, commandMeaningTable[i].mean, 30);
		cursol_y = cons_newline(cursol_y, sheet);
	}
	
	return cursol_y;
}
