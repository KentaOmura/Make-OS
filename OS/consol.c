#include"bootpack.h"

#define NULL 0

typedef enum CONSOL_COMMAND
{
	 CONSOL_COMMAND_NON	= 0	/* コマンド無し */
	,CONSOL_COMMAND_MEM		/* メモリ表示 */
	,CONSOL_COMMAND_DIR		/* ディレクトリ内のファイル表示 */
	,CONSOL_COMMAND_CLS		/* コンソール画面のクリア */
	,CONSOL_COMMAND_TYPE	/* 指定したファイルの中身を表示する */
	,CONSOL_COMMAND_HELP	/* コマンドの種別と意味を表示する */
	,CONSOL_COMMAND_BAD		/* コマンド不正 */
}eCONSOL_COMMAND;

struct CONSOL_COMMAND_EXE
{
	eCONSOL_COMMAND kind; /* 種別 */
	int (*execute)(int **argv); /* 実行関数 */
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

/* コンソールタスクの実行処理テーブル */
struct CONSOL_COMMAND_EXE commandExeTable[] =
{
	 {CONSOL_COMMAND_MEM, &memExecute}
	,{CONSOL_COMMAND_DIR, &dirExecute}
	,{CONSOL_COMMAND_CLS, &clsExecute}
	,{CONSOL_COMMAND_TYPE,&typeExecute}
	,{CONSOL_COMMAND_HELP,&helpExecute}
	,{CONSOL_COMMAND_BAD, &badExecute}
};

/* コマンドの意味 */
struct CONSOL_COMMAND_MEANING commandMeaningTable[] =
{
	 {"mem",  "Memory Storage and using"}
	,{"dir",  "Print All File of Directory"}
	,{"cls",  "Clear Consol Display"}
	,{"type", "Contents of Specified file"}
};

/* コマンド実行 */
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
		/* 最初の行から最後の行1つ手前までのデータをずらす*/
		for(y = 28; y < sheet->bysize - 25; y++)
		{
			for(x = 8; x < sheet->bxsize - 8; x++)
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
		sheet_refresh(sheet, 8, 28,sheet->bxsize - 8, sheet->bysize - 9);
	}
	
	return cursol_y;
}

void consol_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct FILEINFO *fileInfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
	struct TIMER *timer;
	struct TASK *task = task_now(); /* スリープ処理に必要な為、自分自身のタスクを知る必要がある */
	unsigned char s[30], cmdline[30];
	int i, fifo_buf[128], cursol_x = 16, cursol_y = 28, cursol_c = -1;
	int x, y;
	int *consolArg[10];
	eCONSOL_COMMAND type;
	
	fifo32_init(&task->fifo, 128, fifo_buf, task);
	
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50); /* 0.5秒でタイマーを張る */
	
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
						cursol_c = COL8_FFFFFF; /* 白 */
					}
				}
				else
				{
					timer_init(timer, &task->fifo, 1);
					if(cursol_c >= 0)
					{
						cursol_c = COL8_000000; /* 黒 */
					}
				}
				timer_settime(timer, 50);
			}
			else if(2 == i) /* 点滅を許可 */
			{
				cursol_c = COL8_FFFFFF; /* 白 */
			}
			else if(3 == i) /* 点滅の禁止 */
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
						/* コマンドを実行する */
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
	
	/* memコマンド */
	sprintf(s, "total  %dMB", memman->storage / (1024 * 1024));
	putfont8_sht(sheet, 8, cursol_y, COL8_FFFFFF, COL8_000000, s,30);
	cursol_y = cons_newline(cursol_y, sheet);
	sprintf(s, "free  %dKB", memman_total(memman) / 1024);
	putfont8_sht(sheet, 8, cursol_y, COL8_FFFFFF, COL8_000000, s,30);
	cursol_y = cons_newline(cursol_y, sheet);
	cursol_y = cons_newline(cursol_y, sheet);

	/* 最終的なカーソルの行を設定する */
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
			if((fileInfo[x].type & 0x18) == 0) /* ファイルでない情報+ディレクトリでない場合 */
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
	unsigned char s[15]; /* ファイル名格納用 */
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
	
	/* type 分の文字を考慮して5スタート
	   ファイル名を設定する
	*/
	for(x = 5; y < 11 && cmdline[x] != 0; x++)
	{
		/* 拡張子 */
		if(cmdline[x] == '.' && y <= 8)
		{
			y = 8;
		}
		else
		{
			s[y] = cmdline[x];
			/* 小文字は大文字に変換する */
			if('a' <= s[y] && s[y] <= 'z')
			{
				s[y] -= 0x20;
			}
			y++;
		}
	}
	/* ファイルを探す */
	for(x= 0; x < 224;x++) /* 224はファイルのデータを格納できる限界の数 */
	{
		if(finfo[x].name[0] == 0x00)
		{
			break;
		}
		/* ディレクトリまたは、ファイルでない物は対象外 */
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
		/* ファイルが見つかった場合 */
		p = (char *)memman_alloc_4k(memman, finfo[x].size);
		file_loadfile(finfo[x].clustno, finfo[x].size, p, fat,(char *)(ADR_DISKIMG + 0x003e00));
		cursol_x = 8;
		for(y = 0; y < finfo[x].size; y++)
		{
			/* 1文字ずつ出力する */
			s[0] = p[y];
			s[1] = 0;
			if(s[0] == 0x09) /* タブ */
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
						break; /* 32で割り切れる */
					}
				}
			}
			else if(s[0] == 0x0a) /* 改行 */
			{
				cursol_x = 8;
				cursol_y = cons_newline(cursol_y, sheet);
			}
			else if(s[0] == 0x0d) /* 復帰 */
			{
			}
			else
			{
				putfont8_sht(sheet, cursol_x, cursol_y, COL8_FFFFFF, COL8_000000, s, 1);
				cursol_x += 8;
				if(cursol_x == 8 + 240) /* 右端まで到達した */
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
		/* ファイルが見つからなかった場合 */
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
