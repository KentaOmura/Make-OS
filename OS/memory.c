#include"bootpack.h"


#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;
	
	/* 386か、486以降かを確認 */
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if((eflg & EFLAGS_AC_BIT) != 0)
	{
		flg486 = 1;
	}
	eflg &= ~ EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	
	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* キャッシュ機能を禁止にする */
		store_cr0(cr0);
	}
	
	i = memtest_sub(start, end);
	
	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* キャッシュを許可する */
		store_cr0(cr0);
	}
	
	return i;
}

void memman_init(struct MEMMAN *man, unsigned int memtotal)
{
	man->storage = memtotal;
	man->frees    = 0; /* 空き情報の個数 */
	man->lostsize = 0; /* 解放に失敗した合計サイズ */
	man->losts = 0;    /* 解放に失敗した回数 */
	return;
}

unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;
	for(i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;
	for(i = 0;  i < man->frees; i++)
	{
		if(man->free[i].size >= size)
		{
			/* 空きを確保 */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if(man->free[i].size == 0)
			{
				/* freeがなくなったので前に詰める */
				man->frees--;
				for(; i < man->frees; i++)
				{
					man->free[i] = man->free[i + 1];
				}
			}
			
			return a;
		}
	}
	
	/* 空きが無い */
	return 0;
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	/* free[]がaddr順に並んでいるほうがまとめやすい為、ここで順番を決定する */
	for(i = 0; i < man->frees; i++)
	{
		if(man->free[i].addr > addr)
		{
			break;
		}
	}
	
	/* free[i -1].addr < addr < free[i].addr */
	if(i > 0)
	{
		/* 前がある */
		if(man->free[i - 1].addr + man->free[i - 1].size == addr)
		{
			/* 前の空き領域にまとめれる */
			man->free[i - 1].size += size;
			if(i < man->frees)
			{
				/* 後ろもある */
				if(addr + size == man->free[i].addr)
				{
					/* 後ろもまとめれる */
					man->free[i - 1].size += man->free[i].size;
					/* free[i]はなくなったので、free[i]からの後ろを前に詰める */
					man->frees--;
					for(;  i < man->frees; i++)
					{
						man->free[i] = man->free[i + 1]; /* 構造体の代入 */
					}
				}
			}
			
			return 0; /* 成功 */
		}
		
	}
	/* 前にまとめる事ができなかった */
	if(i < man->frees)
	{
		/* 後ろがある */
		if(addr + size == man->free[i].addr)
		{
			/* 後ろはまとめる事ができる */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; /* 成功 */
		}
	}
	/* まとめる事ができない */
	if(man->frees < MEMMAN_FREES)
	{
		/* free[i]より後ろを、後ろにずらして、隙間を作成する */
		for(j = man->frees; j > i; j--)
		{
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; /* 成功 */
	}
	/* 後ろにずらす事ができなかった */
	man->losts++;
	man->lostsize += size;
	return -1;
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}
