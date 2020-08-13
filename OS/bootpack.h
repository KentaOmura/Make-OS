
/* rgb�e�[�u���̃C���f�b�N�X�ԍ� */
#define COL8_000000 0  /* �� */
#define COL8_FF0000 1  /* �� */
#define COL8_00FF00 2  /* �� */
#define COL8_0000FF 3  /* �� */
#define COL8_FFFF00 4  /* ���F */
#define COL8_FF00FF 5  /* �� */
#define COL8_00FFFF 6  /* ���F */
#define COL8_FFFFFF 7  /* ���F */
#define COL8_C6C6C6 8  /* �D�F */
#define COL8_840000 9  /* �Â��ԐF */
#define COL8_008400 10 /* �Â��ΐF */
#define COL8_000084 11 /* �Â��F */
#define COL8_848400 12 /* �Â����F */
#define COL8_840084 13 /* �Â����F */
#define COL8_008484 14 /* �Â����F */
#define COL8_848484 15 /* �Â��D�F */

struct SCREEN_INFO
{
	short screen_x, screen_y; /* ��ʃT�C�Y */
	char* vram; /* VRAM�̃A�h���X */
};

/* �Z�O�����g�L�q�q�p�f�[�^*/
struct SEGMENT_DESCRIPTOR 
{
	short limit_low, base_low; 
	char base_mid, access_right;
	char limit_high, base_high;
};

/* ���荞�݋L�q�q�p�f�[�^ */
struct GATE_DESCRIPTOR 
{
	short offset_low, selector;
	char reserve, flags;
	short offset_high;
};

/* �A�Z���u���R�[�h */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int address);
void load_idtr(int limit, int address);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

/* graphic.c�̒�`�֐� */
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char color, int start_x, int start_y, int end_x, int end_y);
void putfont8(char *vram, int xsize, int index_x, int index_y, char color, char* font);
void putstr8_asc(char *vram, int xsize, int index_x, int index_y, int color, unsigned char *str);
void init_mouse_cursol(char* mouse, char col);
void putstr8_asc(char *vram, int xsize, int index_x, int index_y, int color, unsigned char *str);

#define AR_INTGATE32 0x008e /* ���荞�݂��L���ł��鎖���w���l */

/* dsctbl.c�̒�` */
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

/* int.c�̒�` */
void init_pic(void);
void inthandler21(void);
void inthandler2c(void);
void inthandler27(void);
void init_keybuf(void);
unsigned int keybord_data_num(void);
unsigned int mouse_data_num(void);
unsigned char get_keybord_data(void);
unsigned char get_mouse_data(void);

/* memory.c�̒�` */
#define MEMMAN_FREES 4090 /* ��32MB */
struct FREEINFO
{
	unsigned int addr, size;
};

struct MEMMAN
{
	unsigned int storage;
	int frees,lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man, unsigned int memtotal);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);


/* mouse.c�̒�` */
/*
	�}�E�X��͍\����
*/
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

struct MOUSE_DEC
{
	unsigned char data[3]; /* �f�[�^ */
	unsigned char phase;   /* �t�F�[�Y */
	int x, y, btn;         /* �}�E�X�|�C���^��x, y�ƃ{�^���̏�� */
};

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data);
void enable_mouse(struct MOUSE_DEC *mdec);

/* keybord.c�̒�` */

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void);
void init_keyboard(void);

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1


/* fifo.c�̒�` */
#define FIFO_MAX 32

/* �L���[�\���� */
struct FIFO
{
	unsigned char data[FIFO_MAX];
	unsigned int wcounter;
	unsigned int rcounter;
	unsigned int fsize;
	unsigned int frees;
};

unsigned int fifo_status(struct FIFO* fifo);
void fifo_put(struct FIFO* fifo, unsigned char data);
void init_fifo(struct FIFO* fifo);
unsigned char fifo_get(struct FIFO* fifo);



/* sheet.c�̒�` */
struct SHEET 
{
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
};

#define MAX_SHEETS 256
#define SHEET_USE 1
struct SHTCTL
{
	unsigned char *vram;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};

struct SHTCTL *shtctl_init(struct MEMMAN *man, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1);
void sheet_refresh(struct SHEET *ctl, struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht);
