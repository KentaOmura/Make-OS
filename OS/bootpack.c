#include "bootpack.h"

/*
	マウス解析構造体
*/
struct MOUSE_DEC
{
	unsigned char data[3]; /* データ */
	unsigned char phase;   /* フェーズ */
	int x, y, btn;         /* マウスポインタのx, yとボタンの情報 */
};

void init_keybord(void);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data);

void HariMain(void)
{
	struct SCREEN_INFO *binfo;
	struct MOUSE_DEC mdec;
	unsigned char data;
	
	unsigned char s[10];
	char mouse[64];
	int mx, my;

	init_gdtidt();
	init_pic();
	ini_keybuf();
	io_sti(); /* IDTとPICの設定が完了したので、CPUへの割り込みを受け付ける */
	io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */
	init_keyboard();
	
	/* パレットの初期化 */
	init_palette();

	/* 起動時の画面情報を取得 */
	binfo = (struct SCREEN_INFO*)0x0ff4;
	
	/* 背景画面を描画する */
	boxfill8(binfo->vram, binfo->screen_x, COL8_0000FF, 0, 0, binfo->screen_x - 1, binfo->screen_y - 1);
	
	/* マウスカーソルのデータを取得 */
	init_mouse_cursol(mouse, COL8_0000FF);
	/* マウスカーソルを描画する */
	mx = binfo->screen_x / 2;
	my = binfo->screen_y / 2;
	putblock8_8(binfo->vram, binfo->screen_x, 8, 8, mx, my, mouse);
	enable_mouse(&mdec);
	
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
				boxfill8(binfo->vram, binfo->screen_x, COL8_0000FF, 0, 16,binfo->screen_x - 1 , 31);
				putstr8_asc(binfo->vram, binfo->screen_x, 0, 16, COL8_FFFFFF, s);
			}
			/* マウスからのデータが格納されている場合 */
			else if(mouse_data_num())
			{
				
				data = get_mouse_data();
				io_sti();
				if(1 == mouse_decode(&mdec, data))
				{
					/* マウスポインタの表示処理 */
					boxfill8(binfo->vram, binfo->screen_x, COL8_0000FF, mx, my, mx + 8, my + 8);
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
					boxfill8(binfo->vram, binfo->screen_x, COL8_0000FF, 0, 32, binfo->screen_x - 1 , 31+16);
					putstr8_asc(binfo->vram, binfo->screen_x, 0, 32, COL8_FFFFFF, s);
					putblock8_8(binfo->vram, binfo->screen_x, 8, 8, mx, my, mouse);
				}
			}
		}
	}
}

/* マウスの信号の解析 */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data)
{
	int result = 0;
	
	switch(mdec->phase)
	{
	case 0:
		if(0xfa == data)
		{
			mdec->phase = 1;
		}
		break;
	case 1:
		mdec->data[0] = data;
		mdec->phase = 2;
		break;
	case 2:
		mdec->data[1] = data;
		mdec->phase = 3;
		break;
	case 3:
		mdec->data[2] = data;
		mdec->phase = 1;
		result = 1;
		
		mdec->btn = mdec->data[0] & 0x07;
		mdec->x = mdec->data[1];
		mdec->y = mdec->data[2];
		if ((mdec->data[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->data[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; /* マウスではy方向の符号が画面と反対 */
		break;
	default:
		result = -1;
	}

	return result;
}

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void)
{
	/* キーボードコントローラがデータ送信可能になるのを待つ */
	while(1)
	{
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	/* キーボードコントローラの初期化 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}


#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(struct MOUSE_DEC *mdec)
{
	/* マウス有効 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase = 0;
	return; /* うまくいくとACK(0xfa)が送信されてくる */
}
