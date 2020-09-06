#include"bootpack.h"


struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max)
{
	int x, y;
	char s[12];
	int search;
	
	for(y = 0; y < 11; y++)
	{
		s[y] = ' ';
	}
	y = 0;
	
	/* 指定ファイルの名前を大文字に変換する */
	for(x = 0; name[x] != 0; x++)
	{
		if(y >= 11)
		{
			return 0;
		}
		/* 拡張子 */
		if(name[x] == '.' && y <= 8)
		{
			y = 8;
		}
		else
		{
			s[y] = name[x];
			/* 小文字は大文字に変換する */
			if('a' <= s[y] && s[y] <= 'z')
			{
				s[y] -= 0x20;
			}
			y++;
		}
	}
	
	/* 指定ファイルを探す */
	for(x= 0; x < max; x++)
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
				return finfo + x;
			}
		}
	}
	
	/* 見つからなかった */
	return 0;
}

/* Fat形式の圧縮を解く */
void file_readfat(int *fat, unsigned char *img)
{
	int i,j = 0;
	for(i = 0; i < 2880; i += 2)
	{
		fat[i + 0] = (img[j + 0] | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	
	return;
}

void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
	int i;
	while(1)
	{
		if(size <= 512)
		{
			for(i = 0; i < size; i++)
			{
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		for(i = 0; i < 512; i++)
		{
			buf[i] = img[clustno * 512 + i];
		}
		
		size -= 512;
		buf += 512;
		clustno = fat[clustno];
	}
	
	return;
}