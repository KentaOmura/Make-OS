#include "bootpack.h"

void change_wtitle8(struct SHEET *sht, char act)
{
	int x, y, xsize = sht->bxsize;
	char c,tc_new, tbc_new, tc_old, tbc_old, *buf = sht->buf;
	
	if(act != 0)
	{
		tc_new  = COL8_FFFFFF;
		tbc_new = COL8_000084;
		tc_old  = COL8_C6C6C6;
		tbc_old = COL8_848484;
	}
	else
	{
		tc_new  = COL8_C6C6C6;
		tbc_new = COL8_848484;
		tc_old  = COL8_FFFFFF;
		tbc_old = COL8_000084;
	}
	for(y = 3; y <= 20; y++)
	{
		for(x = 3; x <= xsize; x++)
		{
			c = buf[y * xsize + x];
			if(c == tc_old && x <= xsize - 22)
			{
				c = tc_new;
			}
			else if(c == tbc_old)
			{
				c = tbc_new;
			}
			buf[y * xsize + x] = c;
		}
	}
	sheet_refresh(sht, 3, 3, xsize, 21);
	return;
}