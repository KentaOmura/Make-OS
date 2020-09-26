[FORMAT "WCOFF"]		;オブジェクトファイルを作成するモード
[INSTRSET "i486p"]		;486の命令まで使いたいという記述
[BITS 32]				;32ビットモード用の機械語を作らせる
[FILE "a_nask.nas"]		;ソールファイル名

	GLOBAL	_api_putchar
	GLOBAL	_api_putstr0
	GLOBAL	_api_end
	GLOBAL	_api_openwin, _api_closewin
	GLOBAL _api_putstrwin, _api_boxfilwin
	GLOBAL _api_initmalloc, _api_malloc, _api_free
	GLOBAL _api_point, _api_refreshwin
	GLOBAL _api_linewin
	GLOBAL _api_getkey
	GLOBAL _api_alloctimer, _api_inittimer, _api_settimer, _api_freetimer
	GLOBAL _api_beep
	
[SECTION .text]
_api_putstr0:	; void api_putstr(char *s)
	PUSH	EBX
	MOV		EDX, 2
	MOV		EBX, [ESP + 8]	;s
	INT		0x40
	POP		EBX
	RET

_api_putchar:
	MOV		EDX, 1
	MOV		AL, [ESP+4]
	INT		0x40
	RET
	
_api_end:
	MOV		EDX, 4
	INT		0x40
	
_api_openwin: ; int api_openwin(char* buf, int xsize, int ysize, int col_inv, char *title)
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX, 5
	MOV		EBX, [ESP+16] ; buf
	MOV		ESI, [ESP+20] ; xsize
	MOV		EDI, [ESP+24] ; ysize
	MOV		EAX, [ESP+28] ; col_inv
	MOV		ECX, [ESP+32] ; title
	INT		0x40
	POP		EBX
	POP		ESI
	POP		EDI
	RET

_api_putstrwin: ; void api_putstrwin(int win, int x, int y, int col, int len, char *str)
	PUSH	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX, 6
	MOV		EBX, [ESP + 20] ;win
	MOV		ESI, [ESP + 24] ;x
	MOV		EDI, [ESP + 28] ;y
	MOV		EAX, [ESP + 32] ;col
	MOV		ECX, [ESP + 36] ;len
	MOV		EBP, [ESP + 40] ;str
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET
	
_api_boxfilwin: ; void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col)
	PUSH	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX, 7
	MOV		EBX, [ESP + 20] ;win
	MOV		ESI, [ESP + 24] ;x0
	MOV		EDI, [ESP + 28] ;y0
	MOV		EAX, [ESP + 32] ;x1
	MOV		ECX, [ESP + 36] ;y1
	MOV		EBP, [ESP + 40] ;col
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET
	
_api_initmalloc: ; void api_initmalloc(void)
	PUSH	EBX
	MOV		EDX, 8
	MOV		EBX, [CS:0x0020]	; malloc領域の番地
	MOV		EAX, EBX
	ADD		EAX, 32*1024 ;32kb足す
	MOV		ECX, [CS:0x0000]
	SUB		ECX, EAX
	INT		0x40
	POP		EBX
	RET

_api_malloc: ; char* api_malloc(int size)
	PUSH	EBX
	MOV		EDX, 9
	MOV		EBX, [CS:0x0020]	; malloc領域の番地
	MOV		ECX, [ESP+8]		; size
	INT		0x40
	POP		EBX
	RET

_api_free: ; void api_free(char *addr, int size)
	PUSH	EBX
	MOV		EDX, 10
	MOV		EBX, [CS:0x0020]	; malloc領域の番地
	MOV		EAX, [ESP + 8]
	MOV		ECX, [ESP + 12]
	INT		0x40
	POP		EBX
	RET

_api_point: ; void api_putpoint(int win, int x, int y, int col);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX, 11
	MOV		EBX, [ESP + 16] ; win
	MOV		ESI, [ESP + 20] ; x
	MOV		EDI, [ESP + 24] ; y
	MOV		EAX, [ESP + 28] ; col
	INT		0x40
	POP		EBX
	POP		ESI
	POP		EDI
	RET
	
_api_refreshwin: ; void api_refreshwin(int win, int x0, int y0, int x1, int y1);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX, 12
	MOV		EBX, [ESP + 16] ; win
	MOV		EAX, [ESP + 20] ; x0
	MOV		ECX, [ESP + 24] ; y0
	MOV		ESI, [ESP + 28] ; x1
	MOV		EDI, [ESP + 32] ; y1
	INT		0x40
	POP		EBX
	POP		ESI
	POP		EDI
	RET
	
_api_linewin: ; void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX, 13
	MOV		EBX, [ESP+20]
	MOV		EAX, [ESP+24]
	MOV		ECX, [ESP+28]
	MOV		ESI, [ESP+32]
	MOV		EDI, [ESP+36]
	MOV		EBP, [ESP+40]
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET

_api_closewin: ; void api_closewin(int win);
	PUSH	EBX
	MOV		EDX, 14
	MOV		EBX, [ESP+8]
	INT		0x40
	POP		EBX
	RET
	
_api_getkey: ; int api_getkey(int mode);
	MOV		EDX, 15
	MOV		EAX, [ESP+4];
	INT 	0x40
	RET
	
_api_alloctimer:	; int api_alloctimer(void);
	MOV		EDX,16
	INT		0x40
	RET

_api_inittimer:		; void api_inittimer(int timer, int data);
	PUSH	EBX
	MOV		EDX,17
	MOV		EBX,[ESP+ 8]		; timer
	MOV		EAX,[ESP+12]		; data
	INT		0x40
	POP		EBX
	RET

_api_settimer:		; void api_settimer(int timer, int time);
	PUSH	EBX
	MOV		EDX,18
	MOV		EBX,[ESP+ 8]		; timer
	MOV		EAX,[ESP+12]		; time
	INT		0x40
	POP		EBX
	RET

_api_freetimer:		; void api_freetimer(int timer);
	PUSH	EBX
	MOV		EDX,19
	MOV		EBX,[ESP+ 8]		; timer
	INT		0x40
	POP		EBX
	RET
		
_api_beep:			; void api_beep(int tone);
	MOV		EDX, 20
	MOV		EAX, [ESP+4]
	INT 	0x40
	RET