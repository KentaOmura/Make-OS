[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 32]
	EXTERN	_fifo32_init
	EXTERN	_init_keyboard
	EXTERN	_enable_mouse
	EXTERN	_io_out8
	EXTERN	_timer_alloc
	EXTERN	_timer_init
	EXTERN	_timer_settime
	EXTERN	_init_palette
	EXTERN	_shtctl_init
	EXTERN	_sheet_alloc
	EXTERN	_memman_alloc_4k
	EXTERN	_sheet_setbuf
	EXTERN	_boxfill8
	EXTERN	_init_mouse_cursol
	EXTERN	_task_init
	EXTERN	_task_alloc
	EXTERN	_task_run
	EXTERN	_sheet_slide
	EXTERN	_sheet_updown
	EXTERN	_memman_total
	EXTERN	_sprintf
	EXTERN	_putstr8_asc
	EXTERN	_sheet_refresh
	EXTERN	_io_cli
	EXTERN	_fifo32_status
	EXTERN	_fifo32_get
	EXTERN	_io_sti
	EXTERN	_mouse_decode
	EXTERN	_keytable.0
	EXTERN	_task_sleep
	EXTERN	_memtest
	EXTERN	_memman_init
	EXTERN	_memman_free
	EXTERN	_init_gdtidt
	EXTERN	_init_pic
	EXTERN	_init_pit
[FILE "bootpack.c"]
[SECTION .data]
_keytable.0:
	DB	0
	DB	0
	DB	49
	DB	50
	DB	51
	DB	52
	DB	53
	DB	54
	DB	55
	DB	56
	DB	57
	DB	48
	DB	45
	DB	94
	DB	0
	DB	0
	DB	81
	DB	87
	DB	69
	DB	82
	DB	84
	DB	89
	DB	85
	DB	73
	DB	79
	DB	80
	DB	64
	DB	91
	DB	0
	DB	0
	DB	65
	DB	83
	DB	68
	DB	70
	DB	71
	DB	72
	DB	74
	DB	75
	DB	76
	DB	59
	DB	58
	DB	0
	DB	0
	DB	93
	DB	90
	DB	88
	DB	67
	DB	86
	DB	66
	DB	78
	DB	77
	DB	44
	DB	46
	DB	47
	DB	0
	DB	42
	DB	0
	DB	32
	DB	0
	DB	0
	DB	0
	DB	0
	DB	0
	DB	0
	DB	0
	DB	0
	DB	0
	DB	0
	DB	0
	DB	0
	DB	0
	DB	55
	DB	56
	DB	57
	DB	45
	DB	52
	DB	53
	DB	54
	DB	43
	DB	49
	DB	50
	DB	51
	DB	48
	DB	46
LC0:
	DB	"Window",0x00
LC1:
	DB	"memory %dMB  free : %dKB",0x00
LC5:
	DB	"3[sec]",0x00
LC4:
	DB	"10[sec]",0x00
LC2:
	DB	"%02X",0x00
LC3:
	DB	" ",0x00
[SECTION .text]
	GLOBAL	_HariMain
_HariMain:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	LEA	ESI,DWORD [-124+EBP]
	SUB	ESP,1664
	MOV	DWORD [-1664+EBP],0
	CALL	_hardWareInit
	LEA	EAX,DWORD [-1148+EBP]
	PUSH	0
	PUSH	EAX
	PUSH	256
	PUSH	ESI
	CALL	_fifo32_init
	PUSH	256
	PUSH	ESI
	CALL	_init_keyboard
	LEA	EAX,DWORD [-92+EBP]
	PUSH	EAX
	PUSH	512
	PUSH	ESI
	CALL	_enable_mouse
	ADD	ESP,36
	PUSH	248
	PUSH	33
	CALL	_io_out8
	PUSH	239
	PUSH	161
	CALL	_io_out8
	PUSH	3932160
	CALL	_memoryUsage
	MOV	DWORD [ESP],EAX
	PUSH	3932160
	CALL	_memoryInit
	CALL	_timer_alloc
	PUSH	10
	PUSH	ESI
	MOV	EBX,EAX
	PUSH	EAX
	CALL	_timer_init
	ADD	ESP,36
	PUSH	1000
	PUSH	EBX
	CALL	_timer_settime
	CALL	_timer_alloc
	PUSH	3
	PUSH	ESI
	MOV	EBX,EAX
	PUSH	EAX
	CALL	_timer_init
	PUSH	300
	PUSH	EBX
	CALL	_timer_settime
	CALL	_timer_alloc
	PUSH	1
	PUSH	ESI
	PUSH	EAX
	MOV	DWORD [-1652+EBP],EAX
	CALL	_timer_init
	ADD	ESP,40
	PUSH	50
	PUSH	DWORD [-1652+EBP]
	CALL	_timer_settime
	CALL	_init_palette
	MOVSX	EAX,WORD [4086]
	PUSH	EAX
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	PUSH	3932160
	CALL	_shtctl_init
	MOV	EBX,EAX
	PUSH	EAX
	CALL	_sheet_alloc
	PUSH	EBX
	MOV	DWORD [-1640+EBP],EAX
	CALL	_sheet_alloc
	ADD	ESP,32
	MOV	DWORD [-1644+EBP],EAX
	PUSH	EBX
	LEA	EBX,DWORD [-1404+EBP]
	CALL	_sheet_alloc
	MOV	EDI,EAX
	MOVSX	EDX,WORD [4086]
	MOVSX	EAX,WORD [4084]
	IMUL	EAX,EDX
	PUSH	EAX
	PUSH	3932160
	CALL	_memman_alloc_4k
	PUSH	10880
	PUSH	3932160
	MOV	DWORD [-1648+EBP],EAX
	CALL	_memman_alloc_4k
	PUSH	-1
	MOV	ESI,EAX
	MOVSX	EAX,WORD [4086]
	PUSH	EAX
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-1648+EBP]
	PUSH	DWORD [-1640+EBP]
	CALL	_sheet_setbuf
	ADD	ESP,40
	PUSH	3
	PUSH	8
	PUSH	8
	PUSH	EBX
	PUSH	DWORD [-1644+EBP]
	CALL	_sheet_setbuf
	PUSH	88
	PUSH	68
	PUSH	160
	PUSH	ESI
	PUSH	EDI
	CALL	_sheet_setbuf
	ADD	ESP,40
	MOVSX	EAX,WORD [4086]
	PUSH	EAX
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	0
	PUSH	0
	PUSH	3
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-1648+EBP]
	CALL	_boxfill8
	PUSH	3
	PUSH	EBX
	CALL	_init_mouse_cursol
	ADD	ESP,36
	PUSH	LC0
	PUSH	68
	PUSH	160
	PUSH	ESI
	CALL	_makeWindow8
	PUSH	7
	PUSH	16
	PUSH	144
	PUSH	28
	PUSH	8
	PUSH	EDI
	CALL	_makeTextbox8
	ADD	ESP,40
	MOV	DWORD [-1656+EBP],8
	PUSH	3932160
	MOV	DWORD [-1660+EBP],7
	CALL	_task_init
	MOV	DWORD [-1668+EBP],EAX
	MOV	DWORD [-100+EBP],EAX
	CALL	_task_alloc
	PUSH	65536
	PUSH	3932160
	MOV	EBX,EAX
	CALL	_memman_alloc_4k
	MOV	EDX,DWORD [-1640+EBP]
	ADD	EAX,65528
	MOV	DWORD [64+EBX],EAX
	MOV	DWORD [40+EBX],_task_b_main
	MOV	DWORD [80+EBX],8
	MOV	DWORD [84+EBX],16
	MOV	DWORD [88+EBX],8
	MOV	DWORD [92+EBX],8
	MOV	DWORD [96+EBX],8
	MOV	DWORD [100+EBX],8
	MOV	DWORD [4+EAX],EDX
	PUSH	EBX
	MOV	EBX,2
	CALL	_task_run
	PUSH	0
	PUSH	0
	PUSH	DWORD [-1640+EBP]
	CALL	_sheet_slide
	MOVSX	EAX,WORD [4084]
	LEA	ECX,DWORD [-8+EAX]
	MOV	EAX,ECX
	CDQ
	IDIV	EBX
	MOV	ECX,EAX
	MOV	DWORD [-1636+EBP],EAX
	MOVSX	EAX,WORD [4086]
	SUB	EAX,36
	CDQ
	IDIV	EBX
	PUSH	EAX
	MOV	DWORD [-1632+EBP],EAX
	PUSH	ECX
	LEA	EBX,DWORD [-76+EBP]
	PUSH	DWORD [-1644+EBP]
	CALL	_sheet_slide
	ADD	ESP,40
	PUSH	72
	PUSH	80
	PUSH	EDI
	CALL	_sheet_slide
	PUSH	0
	PUSH	DWORD [-1640+EBP]
	CALL	_sheet_updown
	PUSH	1
	PUSH	EDI
	CALL	_sheet_updown
	PUSH	2
	PUSH	DWORD [-1644+EBP]
	CALL	_sheet_updown
	ADD	ESP,36
	PUSH	3932160
	CALL	_memman_total
	SHR	EAX,10
	MOV	DWORD [ESP],EAX
	MOV	EAX,DWORD [3932160]
	SHR	EAX,20
	PUSH	EAX
	PUSH	LC1
	PUSH	EBX
	CALL	_sprintf
	PUSH	EBX
	PUSH	7
	PUSH	48
	PUSH	0
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-1648+EBP]
	CALL	_putstr8_asc
	ADD	ESP,40
	PUSH	64
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	0
	PUSH	0
	PUSH	DWORD [-1640+EBP]
L23:
	CALL	_sheet_refresh
	ADD	ESP,20
L22:
	LEA	ESI,DWORD [-124+EBP]
	CALL	_io_cli
	PUSH	ESI
	CALL	_fifo32_status
	POP	EBX
	TEST	EAX,EAX
	JE	L26
	PUSH	ESI
	CALL	_fifo32_get
	MOV	EBX,EAX
	CALL	_io_sti
	POP	EDX
	LEA	EAX,DWORD [-256+EBX]
	CMP	EAX,255
	JBE	L27
	LEA	EAX,DWORD [-512+EBX]
	CMP	EAX,255
	JBE	L28
	CMP	EBX,10
	JE	L29
	CMP	EBX,3
	JE	L30
	TEST	EBX,EBX
	JE	L20
	PUSH	0
	PUSH	ESI
	PUSH	DWORD [-1652+EBP]
	CALL	_timer_init
	MOV	DWORD [-1660+EBP],0
L25:
	ADD	ESP,12
	PUSH	50
	PUSH	DWORD [-1652+EBP]
	CALL	_timer_settime
	MOV	EBX,DWORD [-1656+EBP]
	PUSH	44
	ADD	EBX,8
	PUSH	EBX
	PUSH	28
	PUSH	DWORD [-1656+EBP]
	PUSH	DWORD [-1660+EBP]
	PUSH	DWORD [4+EDI]
	PUSH	DWORD [EDI]
	CALL	_boxfill8
	ADD	ESP,36
	PUSH	44
	PUSH	EBX
	PUSH	28
	PUSH	DWORD [-1656+EBP]
	PUSH	EDI
	JMP	L23
L20:
	PUSH	1
	PUSH	ESI
	PUSH	DWORD [-1652+EBP]
	CALL	_timer_init
	MOV	DWORD [-1660+EBP],7
	JMP	L25
L30:
	PUSH	6
	PUSH	LC5
	PUSH	3
	PUSH	7
	PUSH	80
L24:
	PUSH	0
	PUSH	DWORD [-1640+EBP]
	CALL	_putfont8_sht
	ADD	ESP,28
	JMP	L22
L29:
	PUSH	7
	PUSH	LC4
	PUSH	3
	PUSH	7
	PUSH	64
	JMP	L24
L28:
	MOVZX	EAX,BL
	PUSH	EAX
	LEA	EAX,DWORD [-92+EBP]
	PUSH	EAX
	CALL	_mouse_decode
	POP	EDX
	POP	ECX
	DEC	EAX
	JNE	L22
	MOV	EAX,DWORD [-88+EBP]
	LEA	EBX,DWORD [-1636+EBP]
	ADD	DWORD [-1636+EBP],EAX
	MOV	EAX,DWORD [-84+EBP]
	ADD	DWORD [-1632+EBP],EAX
	MOVSX	EAX,WORD [4086]
	PUSH	EAX
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	EBX
	CALL	_MouseCoodinateThreshold
	PUSH	DWORD [-1632+EBP]
	PUSH	DWORD [-1636+EBP]
	PUSH	DWORD [-1644+EBP]
	CALL	_sheet_slide
	ADD	ESP,24
	TEST	DWORD [-80+EBP],1
	JE	L22
	MOV	EAX,DWORD [-1632+EBP]
	SUB	EAX,8
	PUSH	EAX
	MOV	EAX,DWORD [-1636+EBP]
	SUB	EAX,80
	PUSH	EAX
	PUSH	EDI
	CALL	_sheet_slide
	ADD	ESP,12
	JMP	L22
L27:
	PUSH	EAX
	LEA	ESI,DWORD [-76+EBP]
	PUSH	LC2
	PUSH	ESI
	CALL	_sprintf
	PUSH	3
	PUSH	ESI
	PUSH	3
	PUSH	7
	PUSH	16
	PUSH	0
	PUSH	DWORD [-1640+EBP]
	CALL	_putfont8_sht
	ADD	ESP,40
	CMP	EBX,339
	JG	L8
	MOV	AL,BYTE [_keytable.0-256+EBX]
	TEST	AL,AL
	JE	L8
	CMP	DWORD [-1664+EBP],17
	JGE	L8
	PUSH	1
	MOV	BYTE [-76+EBP],AL
	PUSH	ESI
	PUSH	7
	PUSH	0
	PUSH	28
	PUSH	DWORD [-1656+EBP]
	PUSH	EDI
	MOV	BYTE [-75+EBP],0
	CALL	_putfont8_sht
	ADD	ESP,28
	ADD	DWORD [-1656+EBP],8
	INC	DWORD [-1664+EBP]
L8:
	CMP	EBX,270
	JE	L31
L10:
	MOV	EBX,DWORD [-1656+EBP]
	PUSH	44
	ADD	EBX,8
	PUSH	EBX
	PUSH	28
	PUSH	DWORD [-1656+EBP]
	PUSH	DWORD [-1660+EBP]
	PUSH	DWORD [4+EDI]
	PUSH	DWORD [EDI]
	CALL	_boxfill8
	PUSH	44
	PUSH	EBX
	PUSH	28
	PUSH	DWORD [-1656+EBP]
	PUSH	EDI
	CALL	_sheet_refresh
	ADD	ESP,48
	JMP	L22
L31:
	CMP	DWORD [-1656+EBP],8
	JLE	L10
	PUSH	1
	PUSH	LC3
	PUSH	7
	PUSH	0
	PUSH	28
	PUSH	DWORD [-1656+EBP]
	PUSH	EDI
	CALL	_putfont8_sht
	ADD	ESP,28
	SUB	DWORD [-1656+EBP],8
	DEC	DWORD [-1664+EBP]
	JMP	L10
L26:
	PUSH	DWORD [-1668+EBP]
	CALL	_task_sleep
	CALL	_io_sti
	POP	ECX
	JMP	L22
	GLOBAL	_memoryUsage
_memoryUsage:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	-1073741825
	PUSH	4194304
	CALL	_memtest
	LEAVE
	RET
	GLOBAL	_memoryInit
_memoryInit:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	ESI
	PUSH	EBX
	MOV	EBX,DWORD [12+EBP]
	MOV	ESI,DWORD [8+EBP]
	PUSH	EBX
	SUB	EBX,4194304
	PUSH	ESI
	CALL	_memman_init
	PUSH	647168
	PUSH	4096
	PUSH	ESI
	CALL	_memman_free
	PUSH	EBX
	PUSH	4194304
	PUSH	ESI
	CALL	_memman_free
	LEA	ESP,DWORD [-8+EBP]
	POP	EBX
	POP	ESI
	POP	EBP
	RET
	GLOBAL	_hardWareInit
_hardWareInit:
	PUSH	EBP
	MOV	EBP,ESP
	CALL	_init_gdtidt
	CALL	_init_pic
	CALL	_io_sti
	POP	EBP
	JMP	_init_pit
	GLOBAL	_MouseCoodinateThreshold
_MouseCoodinateThreshold:
	PUSH	EBP
	MOV	EBP,ESP
	MOV	EAX,DWORD [8+EBP]
	CMP	DWORD [EAX],0
	JS	L40
L36:
	CMP	DWORD [4+EAX],0
	JS	L41
L37:
	MOV	EDX,DWORD [12+EBP]
	DEC	EDX
	CMP	DWORD [EAX],EDX
	JBE	L38
	MOV	DWORD [EAX],EDX
L38:
	MOV	EDX,DWORD [16+EBP]
	DEC	EDX
	CMP	DWORD [4+EAX],EDX
	JBE	L35
	MOV	DWORD [4+EAX],EDX
L35:
	POP	EBP
	RET
L41:
	MOV	DWORD [4+EAX],0
	JMP	L37
L40:
	MOV	DWORD [EAX],0
	JMP	L36
	GLOBAL	_makeTextbox8
_makeTextbox8:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	SUB	ESP,20
	MOV	EAX,DWORD [12+EBP]
	MOV	ESI,DWORD [16+EBP]
	ADD	EAX,DWORD [20+EBP]
	SUB	ESI,3
	PUSH	ESI
	MOV	DWORD [-16+EBP],EAX
	MOV	EAX,DWORD [16+EBP]
	ADD	EAX,DWORD [24+EBP]
	MOV	DWORD [-20+EBP],EAX
	MOV	EAX,DWORD [-16+EBP]
	INC	EAX
	PUSH	EAX
	MOV	DWORD [-24+EBP],EAX
	MOV	EAX,DWORD [12+EBP]
	PUSH	ESI
	SUB	EAX,2
	PUSH	EAX
	MOV	DWORD [-28+EBP],EAX
	MOV	EAX,DWORD [8+EBP]
	PUSH	15
	PUSH	DWORD [4+EAX]
	PUSH	DWORD [EAX]
	CALL	_boxfill8
	MOV	EDI,DWORD [12+EBP]
	MOV	EAX,DWORD [-20+EBP]
	SUB	EDI,3
	INC	EAX
	PUSH	EAX
	MOV	DWORD [-32+EBP],EAX
	PUSH	EDI
	MOV	EAX,DWORD [8+EBP]
	PUSH	ESI
	PUSH	EDI
	PUSH	15
	PUSH	DWORD [4+EAX]
	PUSH	DWORD [EAX]
	CALL	_boxfill8
	MOV	EAX,DWORD [8+EBP]
	ADD	ESP,56
	MOV	EBX,DWORD [-20+EBP]
	ADD	EBX,2
	PUSH	EBX
	PUSH	DWORD [-24+EBP]
	PUSH	EBX
	PUSH	EDI
	PUSH	7
	PUSH	DWORD [4+EAX]
	PUSH	DWORD [EAX]
	CALL	_boxfill8
	MOV	EAX,DWORD [-16+EBP]
	PUSH	EBX
	ADD	EAX,2
	PUSH	EAX
	PUSH	ESI
	PUSH	EAX
	MOV	EAX,DWORD [8+EBP]
	PUSH	7
	PUSH	DWORD [4+EAX]
	PUSH	DWORD [EAX]
	CALL	_boxfill8
	MOV	EAX,DWORD [8+EBP]
	ADD	ESP,56
	MOV	EBX,DWORD [16+EBP]
	SUB	EBX,2
	MOV	EDI,DWORD [12+EBP]
	PUSH	EBX
	DEC	EDI
	PUSH	DWORD [-16+EBP]
	PUSH	EBX
	PUSH	EDI
	PUSH	0
	PUSH	DWORD [4+EAX]
	PUSH	DWORD [EAX]
	CALL	_boxfill8
	MOV	EAX,DWORD [8+EBP]
	PUSH	DWORD [-20+EBP]
	PUSH	DWORD [-28+EBP]
	PUSH	EBX
	PUSH	DWORD [-28+EBP]
	PUSH	0
	PUSH	DWORD [4+EAX]
	PUSH	DWORD [EAX]
	CALL	_boxfill8
	MOV	EAX,DWORD [8+EBP]
	ADD	ESP,56
	PUSH	DWORD [-32+EBP]
	PUSH	DWORD [-16+EBP]
	PUSH	DWORD [-32+EBP]
	PUSH	DWORD [-28+EBP]
	PUSH	8
	PUSH	DWORD [4+EAX]
	PUSH	DWORD [EAX]
	CALL	_boxfill8
	MOV	EAX,DWORD [8+EBP]
	PUSH	DWORD [-32+EBP]
	PUSH	DWORD [-24+EBP]
	PUSH	EBX
	PUSH	DWORD [-24+EBP]
	PUSH	8
	PUSH	DWORD [4+EAX]
	PUSH	DWORD [EAX]
	CALL	_boxfill8
	MOV	EAX,DWORD [16+EBP]
	ADD	ESP,56
	DEC	EAX
	PUSH	DWORD [-20+EBP]
	PUSH	DWORD [-16+EBP]
	PUSH	EAX
	PUSH	EDI
	MOVZX	EAX,BYTE [28+EBP]
	PUSH	EAX
	MOV	EAX,DWORD [8+EBP]
	PUSH	DWORD [4+EAX]
	PUSH	DWORD [EAX]
	CALL	_boxfill8
	LEA	ESP,DWORD [-12+EBP]
	POP	EBX
	POP	ESI
	POP	EDI
	POP	EBP
	RET
[SECTION .data]
_closebtn.1:
	DB	"OOOOOOOOOOOOOOO@"
	DB	"OQQQQQQQQQQQQQ$@"
	DB	"OQQQQQQQQQQQQQ$@"
	DB	"OQQQ@@QQQQ@@QQ$@"
	DB	"OQQQQ@@QQ@@QQQ$@"
	DB	"OQQQQQ@@@@QQQQ$@"
	DB	"OQQQQQQ@@QQQQQ$@"
	DB	"OQQQQQ@@@@QQQQ$@"
	DB	"OQQQQ@@QQ@@QQQ$@"
	DB	"OQQQ@@QQQQ@@QQ$@"
	DB	"OQQQQQQQQQQQQQ$@"
	DB	"OQQQQQQQQQQQQQ$@"
	DB	"O$$$$$$$$$$$$$$@"
	DB	"@@@@@@@@@@@@@@@@"
[SECTION .text]
	GLOBAL	_makeWindow8
_makeWindow8:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	SUB	ESP,16
	MOV	EBX,DWORD [12+EBP]
	PUSH	0
	LEA	EAX,DWORD [-1+EBX]
	LEA	EDI,DWORD [-2+EBX]
	PUSH	EAX
	MOV	DWORD [-20+EBP],EAX
	PUSH	0
	PUSH	0
	PUSH	8
	PUSH	EBX
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	PUSH	1
	PUSH	EDI
	PUSH	1
	PUSH	1
	PUSH	7
	PUSH	EBX
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	MOV	EAX,DWORD [16+EBP]
	ADD	ESP,56
	DEC	EAX
	MOV	DWORD [-24+EBP],EAX
	PUSH	EAX
	PUSH	0
	PUSH	0
	PUSH	0
	PUSH	8
	PUSH	EBX
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	MOV	ESI,DWORD [16+EBP]
	SUB	ESI,2
	PUSH	ESI
	PUSH	1
	PUSH	1
	PUSH	1
	PUSH	7
	PUSH	EBX
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	ADD	ESP,56
	PUSH	ESI
	PUSH	EDI
	PUSH	1
	PUSH	EDI
	PUSH	15
	PUSH	EBX
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	PUSH	DWORD [-24+EBP]
	PUSH	DWORD [-20+EBP]
	PUSH	0
	PUSH	DWORD [-20+EBP]
	PUSH	1
	PUSH	EBX
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	MOV	EAX,DWORD [16+EBP]
	ADD	ESP,56
	SUB	EAX,3
	PUSH	EAX
	LEA	EAX,DWORD [-3+EBX]
	PUSH	EAX
	PUSH	2
	PUSH	2
	PUSH	8
	PUSH	EBX
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	LEA	EAX,DWORD [-4+EBX]
	PUSH	20
	PUSH	EAX
	PUSH	3
	PUSH	3
	PUSH	7
	PUSH	EBX
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	ADD	ESP,56
	PUSH	ESI
	PUSH	EDI
	XOR	EDI,EDI
	PUSH	ESI
	PUSH	1
	PUSH	15
	PUSH	EBX
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	PUSH	DWORD [-24+EBP]
	PUSH	DWORD [-20+EBP]
	PUSH	DWORD [-24+EBP]
	PUSH	0
	PUSH	7
	PUSH	EBX
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	ADD	ESP,56
	PUSH	DWORD [20+EBP]
	PUSH	0
	PUSH	4
	PUSH	24
	PUSH	EBX
	PUSH	DWORD [8+EBP]
	CALL	_putstr8_asc
	ADD	ESP,24
	IMUL	ECX,EBX,5
	MOV	DWORD [-16+EBP],0
L59:
	LEA	EDX,DWORD [ECX+EBX*1]
	MOV	EAX,DWORD [8+EBP]
	XOR	ESI,ESI
	LEA	EDX,DWORD [-21+EAX+EDX*1]
L58:
	MOV	AL,BYTE [_closebtn.1+ESI+EDI*1]
	CMP	AL,64
	JE	L56
	CMP	AL,36
	JE	L64
	CMP	AL,81
	JE	L65
L56:
	MOV	AL,7
L53:
	INC	ESI
	MOV	BYTE [EDX],AL
	INC	EDX
	CMP	ESI,15
	JLE	L58
	INC	DWORD [-16+EBP]
	ADD	ECX,EBX
	ADD	EDI,16
	CMP	DWORD [-16+EBP],13
	JLE	L59
	LEA	ESP,DWORD [-12+EBP]
	POP	EBX
	POP	ESI
	POP	EDI
	POP	EBP
	RET
L65:
	MOV	AL,1
	JMP	L53
L64:
	MOV	AL,15
	JMP	L53
	GLOBAL	_putfont8_sht
_putfont8_sht:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	SUB	ESP,16
	MOV	EAX,DWORD [12+EBP]
	MOV	EDI,DWORD [16+EBP]
	MOV	DWORD [-16+EBP],EAX
	MOV	ESI,DWORD [8+EBP]
	MOV	EAX,DWORD [20+EBP]
	MOV	DWORD [-20+EBP],EAX
	MOV	EAX,DWORD [28+EBP]
	MOV	DWORD [-24+EBP],EAX
	LEA	EAX,DWORD [16+EDI]
	MOV	DWORD [-28+EBP],EAX
	PUSH	EAX
	MOV	EAX,DWORD [-16+EBP]
	MOV	EBX,DWORD [32+EBP]
	LEA	EBX,DWORD [EAX+EBX*8]
	PUSH	EBX
	PUSH	EDI
	PUSH	EAX
	MOVZX	EAX,BYTE [24+EBP]
	PUSH	EAX
	PUSH	DWORD [4+ESI]
	PUSH	DWORD [ESI]
	CALL	_boxfill8
	PUSH	DWORD [-24+EBP]
	PUSH	DWORD [-20+EBP]
	PUSH	EDI
	PUSH	DWORD [-16+EBP]
	PUSH	DWORD [4+ESI]
	PUSH	DWORD [ESI]
	CALL	_putstr8_asc
	MOV	DWORD [8+EBP],ESI
	MOV	EAX,DWORD [-28+EBP]
	MOV	DWORD [20+EBP],EBX
	MOV	DWORD [24+EBP],EAX
	MOV	DWORD [16+EBP],EDI
	MOV	EAX,DWORD [-16+EBP]
	ADD	ESP,52
	MOV	DWORD [12+EBP],EAX
	LEA	ESP,DWORD [-12+EBP]
	POP	EBX
	POP	ESI
	POP	EDI
	POP	EBP
	JMP	_sheet_refresh
[SECTION .data]
LC6:
	DB	"%10d",0x00
[SECTION .text]
	GLOBAL	_task_b_main
_task_b_main:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	LEA	EAX,DWORD [-556+EBP]
	SUB	ESP,572
	LEA	EBX,DWORD [-44+EBP]
	MOV	DWORD [-580+EBP],0
	PUSH	0
	PUSH	EAX
	PUSH	128
	PUSH	EBX
	MOV	DWORD [-584+EBP],0
	CALL	_fifo32_init
	CALL	_timer_alloc
	PUSH	2
	PUSH	EBX
	PUSH	EAX
	MOV	DWORD [-576+EBP],EAX
	CALL	_timer_init
	PUSH	2
	PUSH	DWORD [-576+EBP]
	CALL	_timer_settime
	ADD	ESP,36
	CALL	_timer_alloc
	PUSH	1
	MOV	EDI,EAX
	PUSH	EBX
	PUSH	EAX
	CALL	_timer_init
	PUSH	1
	PUSH	EDI
	CALL	_timer_settime
	CALL	_timer_alloc
	PUSH	100
	PUSH	EBX
	MOV	ESI,EAX
	PUSH	EAX
	CALL	_timer_init
	ADD	ESP,32
L79:
	PUSH	100
	PUSH	ESI
L80:
	CALL	_timer_settime
	POP	ECX
	POP	EBX
L78:
	LEA	EBX,DWORD [-44+EBP]
	INC	DWORD [-580+EBP]
	CALL	_io_cli
	PUSH	EBX
	CALL	_fifo32_status
	POP	EDX
	TEST	EAX,EAX
	JE	L81
	PUSH	EBX
	CALL	_fifo32_get
	MOV	EBX,EAX
	CALL	_io_sti
	POP	EAX
	CMP	EBX,1
	JE	L82
	CMP	EBX,2
	JE	L83
	CMP	EBX,100
	JNE	L78
	MOV	EAX,DWORD [-580+EBP]
	LEA	EBX,DWORD [-572+EBP]
	SUB	EAX,DWORD [-584+EBP]
	PUSH	EAX
	PUSH	LC6
	PUSH	EBX
	CALL	_sprintf
	PUSH	10
	PUSH	EBX
	PUSH	3
	PUSH	7
	PUSH	160
	PUSH	0
	PUSH	DWORD [8+EBP]
	CALL	_putfont8_sht
	ADD	ESP,40
	MOV	EAX,DWORD [-580+EBP]
	MOV	DWORD [-584+EBP],EAX
	JMP	L79
L83:
	PUSH	2
	PUSH	DWORD [-576+EBP]
	JMP	L80
L82:
	PUSH	1
	PUSH	EDI
	JMP	L80
L81:
	CALL	_io_sti
	JMP	L78
