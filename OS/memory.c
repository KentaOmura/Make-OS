#include"bootpack.h"


#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;
	
	/* 386���A486�ȍ~�����m�F */
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
		cr0 |= CR0_CACHE_DISABLE; /* �L���b�V���@�\���֎~�ɂ��� */
		store_cr0(cr0);
	}
	
	i = memtest_sub(start, end);
	
	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* �L���b�V���������� */
		store_cr0(cr0);
	}
	
	return i;
}

void memman_init(struct MEMMAN *man, unsigned int memtotal)
{
	man->storage = memtotal;
	man->frees    = 0; /* �󂫏��̌� */
	man->lostsize = 0; /* ����Ɏ��s�������v�T�C�Y */
	man->losts = 0;    /* ����Ɏ��s������ */
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
			/* �󂫂��m�� */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if(man->free[i].size == 0)
			{
				/* free���Ȃ��Ȃ����̂őO�ɋl�߂� */
				man->frees--;
				for(; i < man->frees; i++)
				{
					man->free[i] = man->free[i + 1];
				}
			}
			
			return a;
		}
	}
	
	/* �󂫂����� */
	return 0;
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	/* free[]��addr���ɕ���ł���ق����܂Ƃ߂₷���ׁA�����ŏ��Ԃ����肷�� */
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
		/* �O������ */
		if(man->free[i - 1].addr + man->free[i - 1].size == addr)
		{
			/* �O�̋󂫗̈�ɂ܂Ƃ߂�� */
			man->free[i - 1].size += size;
			if(i < man->frees)
			{
				/* �������� */
				if(addr + size == man->free[i].addr)
				{
					/* �����܂Ƃ߂�� */
					man->free[i - 1].size += man->free[i].size;
					/* free[i]�͂Ȃ��Ȃ����̂ŁAfree[i]����̌���O�ɋl�߂� */
					man->frees--;
					for(;  i < man->frees; i++)
					{
						man->free[i] = man->free[i + 1]; /* �\���̂̑�� */
					}
				}
			}
			
			return 0; /* ���� */
		}
		
	}
	/* �O�ɂ܂Ƃ߂鎖���ł��Ȃ����� */
	if(i < man->frees)
	{
		/* ��낪���� */
		if(addr + size == man->free[i].addr)
		{
			/* ���͂܂Ƃ߂鎖���ł��� */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; /* ���� */
		}
	}
	/* �܂Ƃ߂鎖���ł��Ȃ� */
	if(man->frees < MEMMAN_FREES)
	{
		/* free[i]�������A���ɂ��炵�āA���Ԃ��쐬���� */
		for(j = man->frees; j > i; j--)
		{
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; /* ���� */
	}
	/* ���ɂ��炷�����ł��Ȃ����� */
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
