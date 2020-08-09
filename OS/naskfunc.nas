; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; �I�u�W�F�N�g�t�@�C������郂�[�h
[BITS 32]						; 32�r�b�g���[�h�p�̋@�B�����点��
[INSTRSET "i486p"]				; 32�r�b�g���W�X�^��F�����Ă��炤�K�v������B
[FILE "naskfunc.nas"]			; �\�[�X�t�@�C�������

		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_out8, _io_in8
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL	_load_cr0, _store_cr0
		GLOBAL	_memtest_sub
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
		MOV		AL, [ESP+8] ;8bit�p������AL���g�p���Ă���
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

_load_cr0: ; int load_cr0(void)
		MOV		EAX, CR0
		RET

_store_cr0: ;void store_cr0(int cr0)
		MOV		EAX, [ESP+4]
		MOV		CR0, EAX
		RET

_load_gdtr:
		MOV		AX, [ESP+4] ; limit��16bit�ŕ\���ł���
		MOV		[ESP+6], AX ; address��32bit��ESP+8����i�[����Ă���BCPU�����g���G���f�B�A���Ȃ̂ł���ŗǂ��B
		LGDT	[ESP+6]
		RET

_load_idtr: ;GDT�Ɠ���
		MOV		AX, [ESP+4]
		MOV		[ESP+6], AX
		LIDT	[ESP+6]
		RET

_memtest_sub: ; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		ESI, 0xaa55aa55 ;pat0 = 0xaa55aa55
		MOV		EDI, 0x55aa55aa ;pat1 = 0x55aa55aa
		MOV		EAX, [ESP+12+4] ;i = start PUSH��3����{���Ă���̂ŁA�X�^�b�N�|�C���^��4 * 3��12�o�C�g�i��ł���

mts_loop:
		MOV		EBX, EAX
		ADD		EBX, 0xffc  ; p = i + 0xffc
		MOV		EDX, [EBX]  ; old = *p
		MOV		[EBX], ESI  ; *p = pat0
		XOR		DWORD [EBX], 0xffffffff ;*p ^= 0xffffffff
		CMP		EDI, [EBX]	;if(*p != pat1) goto fin
		JNE		mts_fin
		XOR		DWORD [EBX], 0xffffffff ;*p ^= 0xffffffff
		CMP		ESI, [EBX]  ;if(*p != pat0) goto fin
		JNE		mts_fin
		MOV		[EBX], EDX	; *p = old
		ADD		EAX, 0x1000	;i += 0x1000
		CMP		EAX, [ESP+12+8] ; if(i <= end) goto mts_loop
		JNE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET
		
mts_fin:
		MOV		[EBX], EDX ; *p = old
		POP		EBX
		POP		ESI
		POP		EDI
		RET
		
_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX, ESP
		PUSH	EAX
		MOV		AX, SS ; C����̊֐����R�[�����鎞�́ASS���W�X�^�Ɋi�[����Ă���l��DS��ES�����킹�Ă����K�v������B
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
		MOV		AX, SS ; C����̊֐����R�[�����鎞�́ASS���W�X�^�Ɋi�[����Ă���l��DS��ES�����킹�Ă����K�v������B
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
		MOV		AX, SS ; C����̊֐����R�[�����鎞�́ASS���W�X�^�Ɋi�[����Ă���l��DS��ES�����킹�Ă����K�v������B
		MOV		DS, AX
		MOV		ES, AX
		CALL	_inthandler27
		POP		EAX
		POPAD	
		POP		DS
		POP		ES
		IRETD