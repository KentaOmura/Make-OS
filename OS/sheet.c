#include"bootpack.h"

struct SHTCTL *shtctl_init(struct MEMMAN *man, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL*)memman_alloc_4k(man, sizeof(struct SHTCTL));
	
	if(ctl == 0)
	{
		return ctl;
	}
	
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1; /* シートは1枚もない */
	for(i = 0; i < MAX_SHEETS; i++)
	{
		ctl->sheets0[i].flags = 0; /* 未使用のマーク */
	}
	
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for(i = 0; i < MAX_SHEETS; i++)
	{
		if(ctl->sheets0[i].flags == 0)
		{
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE; /* 使用中のマーク */
			sht->height = -1; /* 非表示中 */
			return sht;
		}
	}
	return 0;
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}

void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
	int h, old = sht->height;
	
	/* 設定が高すぎた場合は、修正する */
	if(height > ctl->top + 1)
	{
		height = ctl->top + 1;
	}
	
	/* 設定が低すぎる場合は、修正する */
	if(height < -1)
	{
		height = -1;
	}
	
	sht->height = height; /* 高さを設定する */
	
	if(old > height) /* 下げる時の処理 */
	{
		if(height >= 0)
		{
			/* 間を引き上げる */
			for(h = old; h > height; h--)
			{
				ctl->sheets[h] = ctl->sheets[h-1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		}
		else /* 非表示化 */
		{
			if(ctl->top > old)
			{
				/* 上になっているものを下す */
				for(h = old; h < ctl->top; h++)
				{
					ctl->sheets[h] = ctl->sheets[h+1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--;
		}
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize);
		//sheet_refresh(ctl); /* 新しい下敷き情報を用いて、画面を更新する */
	}
	else if(old < height) /* 上げる時の処理 */
	{
		if(old >= 0)
		{
			/* 間を押し下げる */
			for(h = old; h < height; h++)
			{
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		}
		else /* 非表示状態から表示状態 */
		{
			/* 上になるものを持ち上げる */
			for(h = ctl->top; h >= height; h--)
			{
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++;
		}
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize);
		//sheet_refresh(ctl);
	}
	
	return;
}


void sheet_refresh(struct SHEET *ctl, struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	/* 表示中であれば、新しい下敷きに沿って画面を書き直す*/
	if(sht->height >= 0)
	{
		sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1);
	}
	return;
}

void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	
	for(h = 0; h <= ctl->top; h++)
	{
		sht = ctl->sheets[h];
		buf = sht->buf;
		/* 範囲を絞って描画処理を実施する */
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if(bx0 < 0) {bx0 = 0;}
		if(by0 < 0) {by0 = 0;}
		if(bx1 > sht->bxsize){bx1 = sht->bxsize;}
		if(by1 > sht->bysize){by1 = sht->bysize;}
		for(by = by0; by < by1; by++)
		{
			vy = sht->vy0 + by;
			for(bx = bx0; bx < bx1; bx++)
			{
				vx = sht->vx0 + bx;
				c = buf[by * sht->bxsize + bx];
				if(c != sht->col_inv)
				{
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}
	}
	
	return;
}

void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	int old_vx0 = sht->vx0;
	int old_vy0 = sht->vy0;

	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if(sht->height >= 0) /* 表示中のレイヤーであれば */
	{
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize);
		sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize);
	}
	
	return;
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	if(sht->height > 0)
	{
		sheet_updown(ctl, sht, -1); /* 表示状態から非表示状態にする */
	}
	sht->flags = 0; /* 未使用マークにする */
	return;
}
