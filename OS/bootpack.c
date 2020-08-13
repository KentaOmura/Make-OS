#include "bootpack.h"

/******************************************************************
/*                          ��`                                  */
/******************************************************************
/* �������Ǘ��p�̃����� */
#define MEMMAN_ADDR 0x003c0000

/* �}�E�X�̍��W */
struct MOUSE_COODINATE
{
	int mx;
	int my;
};

/******************************************************************
/*                          �֐��錾                              */
/******************************************************************/
/* �������̎g�p�ʂ̃`�F�b�N */
unsigned int memoryUsage(struct MEMMAN *man);
/* �������Ǘ��̏����� */
void memoryInit(struct MEMMAN *man, unsigned int memtotal);
/* �n�[�h�E�F�A�̏����� */
void hardWareInit(void);
/* �}�E�X���W��臒l�𒴂��Ă���Ίۂߍ��� */
void MouseCoodinateThreshold(struct MOUSE_COODINATE *mc, unsigned int screen_max_x, unsigned int screen_max_y);

void HariMain(void)
{
	unsigned char data;
	unsigned char s[50];

	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	/* �N�����̉�ʏ����擾 */
	struct SCREEN_INFO *binfo = (struct SCREEN_INFO*)0x0ff4;
	/* �V�[�g */
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse;
	struct MOUSE_COODINATE mouseCoodinate; 
	unsigned char *buf_back, buf_mouse[256];
	
	memoryInit(memman, memoryUsage(memman));
	hardWareInit();
	enable_mouse(&mdec);
	
	init_palette();

	shtctl = shtctl_init(memman, binfo->vram, binfo->screen_x, binfo->screen_y);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	buf_back = (unsigned char*)memman_alloc_4k(memman, binfo->screen_x * binfo->screen_y);
	sheet_setbuf(sht_back, buf_back, binfo->screen_x , binfo->screen_y, -1);
	sheet_setbuf(sht_mouse, buf_mouse, 8, 8, 99);
	sheet_updown(shtctl, sht_back, 0);
	sheet_updown(shtctl, sht_mouse, 1);
	
	/* �w�i��ʂ�`�悷��i��F�j */
	boxfill8(buf_back, binfo->screen_x, COL8_0000FF, 0, 0, binfo->screen_x, binfo->screen_y);
	/* �}�E�X�J�[�\���̃f�[�^���擾 */
	init_mouse_cursol(buf_mouse, COL8_0000FF);
	sheet_slide(shtctl, sht_back, 0, 0);
	
	/* �}�E�X�J�[�\����`�悷�� */
	mouseCoodinate.mx = (binfo->screen_x - 8) / 2;
	mouseCoodinate.my = (binfo->screen_y -28 - 8) / 2;
	sheet_slide(shtctl, sht_mouse, mouseCoodinate.mx, mouseCoodinate.my);
	
	sprintf(s, "memory %dMB  free : %dKB", memman->storage / (1024 * 1024), memman_total(memman) / 1024);
	putstr8_asc(buf_back, binfo->screen_x, 0, 48, COL8_FFFFFF, s);
	sheet_refresh(shtctl, sht_back, 0, 0, binfo->screen_x, 48 + 16);
	
	while(1)
	{
		io_cli(); /* ���荞�݂��֎~���� */
		if(0 == keybord_data_num() + mouse_data_num())
		{
			io_stihlt();
		}
		else
		{
			/* �L�[�{�[�h����̃f�[�^���i�[����Ă���ꍇ */
			if(keybord_data_num())
			{
				data = get_keybord_data();
				io_sti();
				sprintf(s, "%02X", data);
				boxfill8(buf_back, binfo->screen_x, COL8_0000FF, 0, 16,binfo->screen_x - 1 , 31);
				putstr8_asc(buf_back, binfo->screen_x, 0, 16, COL8_FFFFFF, s);
				sheet_refresh(shtctl, sht_back, 0, 16, 16, 32);
			}
			/* �}�E�X����̃f�[�^���i�[����Ă���ꍇ */
			else if(mouse_data_num())
			{
				
				data = get_mouse_data();
				io_sti();
				if(1 == mouse_decode(&mdec, data))
				{
					mouseCoodinate.mx += mdec.x;
					mouseCoodinate.my += mdec.y;
					MouseCoodinateThreshold(&mouseCoodinate, binfo->screen_x, binfo->screen_y);
					
					sheet_slide(shtctl, sht_mouse, mouseCoodinate.mx, mouseCoodinate.my);
				}
			}
		}
	}
}

/* �������̍ő�g�p�ʂ̃`�F�b�N */
unsigned int memoryUsage(struct MEMMAN *man)
{
	unsigned int result;
	
	/* �g�p�ł��郁�������̃`�F�b�N */
	result = memtest(0x00400000, 0xbfffffff);
	
	return result;
}

/* �������Ǘ��̏����� */
void memoryInit(struct MEMMAN *man, unsigned int memtotal)
{
	/* �������Ǘ��҂̏����� */
	memman_init(man, memtotal);
	/* �������̎g�p�ʂ̐ݒ� */
	memman_free(man, 0x00001000, 0x0009e000);
	memman_free(man, 0x00400000, memtotal-0x00400000);
	
	return;
}

/* �n�[�h�E�F�A�̏����� */
void hardWareInit(void)
{
	init_gdtidt();
	init_pic();
	init_keybuf();
	/* IDT��PIC�̐ݒ肪���������̂ŁACPU�ւ̊��荞�݂��󂯕t���� */
	io_sti();
	/* PIC1�ƃL�[�{�[�h������(11111001) */
	io_out8(PIC0_IMR, 0xf9);
	/* �}�E�X������(11101111) */
	io_out8(PIC1_IMR, 0xef);
	init_keyboard();
}

/* �}�E�X���W��臒l�𒴂��Ă���Ίۂߍ��� */
void MouseCoodinateThreshold(struct MOUSE_COODINATE *mc, unsigned int screen_max_x, unsigned int screen_max_y)
{
	if(mc->mx < 0)
	{
		mc->mx = 0;
	}
	if(mc->my < 0)
	{
		mc->my = 0;
	}

	if(mc->mx > screen_max_x - 1)
	{
		mc->mx = screen_max_x - 1;
	}
	if(mc->my > screen_max_y - 1)
	{
		mc->my = screen_max_y - 1;
	}
}