#include "bootpack.h"

#define MEMMAN_ADDR 0x003c0000

void init_keybord(void);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data);

void HariMain(void)
{
	struct SCREEN_INFO *binfo;
	struct MOUSE_DEC mdec;
	unsigned char data;
	
	unsigned char s[50];
	char mouse[64];
	int mx, my;
	unsigned int i;
	unsigned int memtotal;

	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	
	/* シート */
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, buf_mouse[256];
	
	init_gdtidt();
	init_pic();
	ini_keybuf();
	io_sti(); /* IDTとPICの設定が完了したので、CPUへの割り込みを受け付ける */
	io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */
	init_keyboard();
	enable_mouse(&mdec);

	/* 起動時の画面情報を取得 */
	binfo = (struct SCREEN_INFO*)0x0ff4;
	
	/* メモリチェック */
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal-0x00400000);
	
	/* パレットの初期化 */
	init_palette();

	shtctl = shtctl_init(memman, binfo->vram, binfo->screen_x, binfo->screen_y);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	buf_back = (unsigned char*)memman_alloc_4k(memman, binfo->screen_x * binfo->screen_y);
	sheet_setbuf(sht_back, buf_back, binfo->screen_x , binfo->screen_y, -1);
	sheet_setbuf(sht_mouse, buf_mouse, 8, 8, 99);
	
	/* 背景画面を描画する */
	boxfill8(buf_back, binfo->screen_x, COL8_0000FF, 0, 0, binfo->screen_x, binfo->screen_y);
	/* マウスカーソルのデータを取得 */
	init_mouse_cursol(buf_mouse, 99);
	sheet_slide(shtctl, sht_back, 0, 0);
	
	/* マウスカーソルを描画する */
	mx = (binfo->screen_x - 8) / 2;
	my = (binfo->screen_y -28 - 8) / 2;
	sheet_slide(shtctl, sht_mouse, mx,my);
	
	sheet_updown(shtctl, sht_back, 0);
	sheet_updown(shtctl, sht_mouse, 1);
	sprintf(s, "memory %dMB  free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putstr8_asc(buf_back, binfo->screen_x, 0, 48, COL8_FFFFFF, s);
	sheet_refresh(shtctl, sht_back, 0, 0, binfo->screen_x, 48 + 16);
	
	while(1)
	{
		io_cli(); /* 割り込みを禁止する */
		if(0 == keybord_data_num() + mouse_data_num())
		{
			io_stihlt();
		}
		else
		{
			/* キーボードからのデータが格納されている場合 */
			if(keybord_data_num())
			{
				data = get_keybord_data();
				io_sti();
				sprintf(s, "%02X", data);
				boxfill8(buf_back, binfo->screen_x, COL8_0000FF, 0, 16,binfo->screen_x - 1 , 31);
				putstr8_asc(buf_back, binfo->screen_x, 0, 16, COL8_FFFFFF, s);
				sheet_refresh(shtctl, sht_back, 0, 16, 16, 32);
			}
			/* マウスからのデータが格納されている場合 */
			else if(mouse_data_num())
			{
				
				data = get_mouse_data();
				io_sti();
				if(1 == mouse_decode(&mdec, data))
				{
					/* マウスポインタの表示処理 */
					//boxfill8(buf_back, binfo->screen_x, COL8_0000FF, mx, my, mx + 8, my + 8);
					mx += mdec.x;
					my += mdec.y;
					
					if(mx < 0)
					{
						mx = 0;
					}
					if(my < 0)
					{
						my = 0;
					}
					
					if(mx > binfo->screen_x - 8)
					{
						mx = binfo->screen_x - 8;
					}
					if(my > binfo->screen_y - 8)
					{
						my = binfo->screen_y - 8;
					}
					sprintf(s, "%2d %2d %2d", mdec.btn, mdec.x, mdec.y);
					boxfill8(buf_back, binfo->screen_x, COL8_0000FF, 0, 32, binfo->screen_x - 1 , 31+16);
					putstr8_asc(buf_back, binfo->screen_x, 0, 32, COL8_FFFFFF, s);
					sheet_refresh(shtctl, sht_back, 0,0,72,32 +16); /* 16(数字)*3 + 8(空白)*3 = 72 */
					sheet_slide(shtctl, sht_mouse, mx, my);
				}
			}
		}
	}
}