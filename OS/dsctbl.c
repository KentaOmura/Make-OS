#include "bootpack.h"

void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR*)ADR_GDT;
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR*)0x0026f800;
	int i;
	
	/* GDTの初期化 */
	for(i = 0; i < 8192; i++)
	{
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xfffffff, 0, 0x4092); /* 書き込み可能。システム専用のセグメント */
	set_segmdesc(gdt + 2, 0x0007fff,0x00280000, 0x409a); /* システム専用の実行可能セグメント */
	
	/* GDTRに設定する */
	load_gdtr(0xffff, 0x00270000);
	
	/* IDTの初期化 */
	for(i = 0;  i < 256; i++)
	{
		set_gatedesc(idt + i, 0, 0, 0);
	}

	/* キーボード割り込み有効化させる */
	set_gatedesc(idt + 0x20, (int)asm_inthandler20, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x27, (int)asm_inthandler27, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x40, (int)asm_cons_putchar, 2 * 8, AR_INTGATE32);

	/* IDTRに設定する */
	load_idtr(0x7ff, 0x0026f800);
	
	return;
}

/* セグメント情報 */
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	/* ページ単位（1ページ 4kb）でリミットを扱う */
	if(limit > 0xffffff)
	{
		ar |= 0x8000; /* Gbit */
		limit /= 0x1000;
	}
	
	/*
		ベースの番地は32bitで表現できる。short base_low char base_mid base_high の合計32bit。80286系のCPUとの互換性の為
	*/
	sd->limit_low     = limit & 0xffff;
	sd->base_low      = base  & 0xffff;
	sd->base_mid      = (base >> 16) & 0xff;
	sd->access_right  = ar & 0xff;
	sd->limit_high    = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0); /* limit_highの上位4bitはセグメント属性の情報を記載するようになっている。 arはxxxx0000xxxxxxxx*/
	sd->base_high     = (base >> 24) & 0xff;
	
	return;
}

/* 割り込み情報 */
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low = offset & 0xffff;
	gd->selector   = selector; /* セグメントセレクタ */
	gd->reserve   = (ar >> 8) & 0xff; /* 使ってない */
	gd->flags = ar & 0xff; /* フラグ系 */
	gd->offset_high = (offset >> 16) & 0xffff;
}
