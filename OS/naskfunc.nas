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
		GLOBAL	_asm_inthandler21,_asm_inthandler2c,_asm_inthandler27,_asm_inthandler20
		GLOBAL	_load_tr, _farjmp, _farcall
		GLOBAL	_asm_cons_putchar
		GLOBAL	_asm_hrb_api,_start_app,_asm_inthandler0d, _asm_inthandler0c
		GLOBAL	_asm_end_app
		EXTERN	_inthandler21, _inthandler2c, _inthandler27,_inthandler20, _inthandler0d, _inthandler0c
		EXTERN	_cons_putchar, _hrb_api

[SECTION .text]
_load_tr:	; void load_tr(int tr)
		LTR		[ESP+4]		;tr
		RET

_farjmp:		;void farjmp(int eip, int cs)
		JMP		FAR	[ESP+4] ;eip, cs
		RET

_farcall:		;void farcall(int eip, int cs)
		CALL	FAR	[ESP+4] ;eip, cs
		RET

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
		MOV		AX, SS
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
		MOV		AX, SS
		MOV		DS, AX
		MOV		ES, AX
		CALL	_inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX, ESP
		PUSH	EAX
		MOV		AX, SS
		MOV		DS, AX
		MOV		ES, AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_cons_putchar:
		STI		;CPU�͊��荞�ݏ������[�`�������ɂȂ�̂ŁA�Ăяo���Ɠ����Ɏ�����CLI���߂����s�����B
		PUSHAD
		PUSH	1  ; move
		AND		EAX, 0xff ;AL�ɕ����R�[�h����������Ԃɂ���BAH�ȏ�̏�ʃr�b�g��0
		PUSH	EAX; chr
		PUSH	DWORD [0x0fec] ;�������̓��e��ǂݍ���ł��̂܂�PUSH����
		CALL	_cons_putchar
		ADD		ESP, 12 ;�X�^�b�N�ɐς񂾃f�[�^���̂Ă�
		POPAD
		IRETD
		
_start_app: ;void start_app(int eip, int cs, int esp, int ds, int *tss_esp);
		PUSHAD ;32�r�b�g���W�X�^��S���ێ�����(EAX�AECX�AEDX�AEBX�AESP�AEBP�AESI�AEDI)
		MOV		EAX, [ESP + 36] ;�A�v���p��EIP
		MOV		ECX, [ESP + 40] ;�A�v���p��CS
		MOV		EDX, [ESP + 44] ;�A�v���p��ESP
		MOV		EBX, [ESP + 48] ;�A�v���p��DS/SS
		MOV		EBP, [ESP + 52] ;tss.esp0�̔Ԓn
		MOV		[EBP], ESP	;OS�p��ESP
		MOV		[EBP+4], SS	;OS�p��SS��ۑ�
		MOV		ES, BX
		MOV		DS, BX
		MOV		FS, BX
		MOV		GS, BX
; �ȉ���RETF�ŃA�v���ɍs������ׂ̃X�^�b�N����
		OR		ECX, 3		;�A�v���p�̃Z�O�����g�ԍ���3��OR����
		OR		EBX, 3		;�A�v���p�̃Z�O�����g�ԍ���3��OR����
		PUSH	EBX			;�A�v����SS
		PUSH	EDX			;�A�v����ESP
		PUSH	ECX			;�A�v����CS
		PUSH	EAX			;�A�v����EIP
		RETF
;�A�v�����I�����Ă������ɂ͂��Ȃ�

_asm_hrb_api:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD		;�ۑ��ׂ̈�PUSH
		PUSHAD		;hrb_api�ɓn���ׂ�PUSH
		MOV		AX,SS
		MOV		DS,AX		; OS�p�̃Z�O�����g��DS��ES�ɂ������
		MOV		ES,AX
		CALL	_hrb_api
		CMP		EAX, 0
		JNE		end_app
		ADD		ESP, 32
		POPAD
		POP		ES
		POP		DS
		IRETD
end_app:
;EAX��tss.esp0�̔Ԓn
		MOV		ESP,[EAX]
		POPAD
		RET			;cmd_app�ɖ߂�

_asm_inthandler0d:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX, ESP
		PUSH	EAX
		MOV		AX, SS
		MOV		DS, AX
		MOV		ES, AX
		CALL	_inthandler0d
		CMP		EAX, 0
		JNE		end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP, 4		; INT 0x0d�ł͂��ꂪ�K�v
		IRETD

_asm_inthandler0c:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX, ESP
		PUSH	EAX
		MOV		AX, SS
		MOV		DS, AX
		MOV		ES, AX
		CALL	_inthandler0c
		CMP		EAX, 0
		JNE		end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP, 4		; INT 0x0c�ł͂��ꂪ�K�v
		IRETD

_asm_end_app:
		MOV		ESP,[EAX]
		MOV		DWORD [EAX + 4],0
		POPAD
		RET
