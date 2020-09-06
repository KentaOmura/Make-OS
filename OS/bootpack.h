
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

#define ADR_DISKIMG 0x00100000
/* �������Ǘ��p�̃����� */
#define MEMMAN_ADDR 0x003c0000

struct BOOT_INFO
{
	char cyls; /* �u�[�g�Z�N�^�͂ǂ��܂Ńf�B�X�N��ǂ񂾂̂� */
	char leds; /* �u�[�g���̃L�[�{�[�h��LED�̏�� */
	char vmode; /* �r�f�I���[�h  ���r�b�g�J���[�� */
	char reserve;
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

#define AR_TSS32 0x0089
#define ADR_GDT  0x00270000


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
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
void asm_cons_putchar(void);

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


/* fifo.c�̒�` */
#define FLAGS_OVERRUN -1

/* �L���[�\���� */
struct FIFO32 
{
	int *buf;
	int p, q, size, free, flags;
	struct TASK *task;
};
unsigned int fifo32_status(struct FIFO32* fifo);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_put(struct FIFO32 *fifo, int data);
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);

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
void enable_mouse(struct FIFO32 *fifo, int data0,struct MOUSE_DEC *mdec);

/* keybord.c�̒�` */

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void);
void init_keyboard(struct FIFO32 *fifo, int data0);

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

/* sheet.c�̒�` */
struct SHEET 
{
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	struct SHTCTL *ctl;
};

#define MAX_SHEETS 256
#define SHEET_USE 1
struct SHTCTL
{
	unsigned char *vram, *map;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};

struct SHTCTL *shtctl_init(struct MEMMAN *man, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);



/* timer,c�̒�` */
#define MAX_TIMER 500

struct TIMER
{
	struct TIMER *next;
	unsigned int timeout, flags;
	struct FIFO32 *fifo;
	int data;
};
struct TIMERCTL
{
	unsigned int count, next;
	struct TIMER *t0;
	struct TIMER timers0[MAX_TIMER];
};

extern struct TIMERCTL timerctl;
void timer_reset(void);
void init_pit(void);
void timer_settime(struct TIMER *timer, unsigned int timeout);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, unsigned char data);

/* mtask.c */
#define MAX_TASKS      1000
#define TASK_GDT0	   3 /* TSS��GDT�̉��ԖڂɊ��蓖�Ă邩 */
#define MAX_TASKS_LV   100
#define MAX_TASKLEVELS 10

extern struct TIMER* task_timer;
struct TSS32
{
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

struct TASK
{
	int sel, flags; /* sel��GDT�̔ԍ��̎� */
	int priority, level;
	struct FIFO32 fifo;
	struct TSS32 tss;
};

struct TASKLEVEL
{
	int running;
	int now;
	struct TASK *tasks[MAX_TASKS_LV];
};

struct TASKCTL
{
	int now_lv; /* ���ݓ��쒆�̃��x�� */
	char lv_change; /* �^�X�N�X�C�b�`���Ƀ��x����ύX���邩�ǂ��� */
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};

struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run(struct TASK *task,int level, int priority);
void task_switch(void);
struct TASK *task_now(void);
void task_add(struct TASK *task);
void task_remove(struct TASK *task);
void task_switchsub(void);


struct FILEINFO
{
	unsigned char name[8], ext[3], type; /* �t�@�C���̖��O�A�g���q�A�����i�ǂݎ���p�A�B���t�@�C���Ȃǁj */
	char reserve[10]; /* �}�C�N���\�t�g�����߂��K�i�B�\��Ƃ���10�o�C�g���݂��� */
	unsigned short time, date, clustno; /* �����A���t�A�N���X�^�ԍ��i�Z�N�^�Ɠ����Ӗ��j���N���X�^�̓}�C�N���\�t�g�̌��t */
	unsigned int size; /* �t�@�C���T�C�Y */
};

/* consol.c */
void consol_task(struct SHEET *sheet, unsigned int memtotal);


/* file.c */
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);
void file_readfat(int *fat, unsigned char *img);
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);
