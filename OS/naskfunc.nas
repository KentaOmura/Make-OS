; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; オブジェクトファイルを作るモード
[BITS 32]						; 32ビットモード用の機械語を作らせる
[INSTRSET "i486p"]				; 32ビットレジスタを認識してもらう必要がある。
[FILE "naskfunc.nas"]			; ソースファイル名情報

		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_out8, _io_in8
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL	_asm_inthandler21,_asm_inthandler2c,_asm_inthandler27
		EXTERN	_inthandler21, _inthandler2c, _inthandler27

[SECTION .text]

_io_hlt:	; void io_hlt(void);
		HLT
		RET

_io_cli:
		CLI
		RET

_io_sti:
		STI
		RET

_io_stihlt:
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

_io_out8:
		MOV		EDX, [ESP+4]
		MOV		AL, [ESP+8] ;8bit用だからALを使用している
		OUT		DX, AL
		RET
		
_io_load_eflags:
		PUSHFD
		POP		EAX
		RET
		
_io_store_eflags:
		MOV		EAX, [ESP+4]
		PUSH	EAX
		POPFD
		RET

_load_gdtr:
		MOV		AX, [ESP+4] ; limitは16bitで表現できる
		MOV		[ESP+6], AX ; addressは32bitでESP+8から格納されている。CPUがリトルエンディアンなのでこれで良い。
		LGDT	[ESP+6]
		RET

_load_idtr: ;GDTと同じ
		MOV		AX, [ESP+4]
		MOV		[ESP+6], AX
		LIDT	[ESP+6]
		RET

_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX, ESP
		PUSH	EAX
		MOV		AX, SS ; C言語の関数をコールする時は、SSレジスタに格納されている値にDSとESを合わせておく必要がある。
		MOV		DS, AX
		MOV		ES, AX
		CALL	_inthandler21
		POP		EAX
		POPAD	
		POP		DS
		POP		ES
		IRETD
		
_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX, ESP
		PUSH	EAX
		MOV		AX, SS ; C言語の関数をコールする時は、SSレジスタに格納されている値にDSとESを合わせておく必要がある。
		MOV		DS, AX
		MOV		ES, AX
		CALL	_inthandler2c
		POP		EAX
		POPAD	
		POP		DS
		POP		ES
		IRETD
		
_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX, ESP
		PUSH	EAX
		MOV		AX, SS ; C言語の関数をコールする時は、SSレジスタに格納されている値にDSとESを合わせておく必要がある。
		MOV		DS, AX
		MOV		ES, AX
		CALL	_inthandler27
		POP		EAX
		POPAD	
		POP		DS
		POP		ES
		IRETD