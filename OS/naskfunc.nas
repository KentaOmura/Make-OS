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

_load_cr0: ; int load_cr0(void)
		MOV		EAX, CR0
		RET

_store_cr0: ;void store_cr0(int cr0)
		MOV		EAX, [ESP+4]
		MOV		CR0, EAX
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

_memtest_sub: ; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		ESI, 0xaa55aa55 ;pat0 = 0xaa55aa55
		MOV		EDI, 0x55aa55aa ;pat1 = 0x55aa55aa
		MOV		EAX, [ESP+12+4] ;i = start PUSHを3回実施しているので、スタックポインタは4 * 3の12バイト進んでいる

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
		STI		;CPUは割り込み処理ルーチン扱いになるので、呼び出しと同時に自動でCLI命令が実行される。
		PUSHAD
		PUSH	1  ; move
		AND		EAX, 0xff ;ALに文字コードが入った状態にする。AH以上の上位ビットは0
		PUSH	EAX; chr
		PUSH	DWORD [0x0fec] ;メモリの内容を読み込んでそのままPUSHする
		CALL	_cons_putchar
		ADD		ESP, 12 ;スタックに積んだデータを捨てる
		POPAD
		IRETD
		
_start_app: ;void start_app(int eip, int cs, int esp, int ds, int *tss_esp);
		PUSHAD ;32ビットレジスタを全部保持する(EAX、ECX、EDX、EBX、ESP、EBP、ESI、EDI)
		MOV		EAX, [ESP + 36] ;アプリ用のEIP
		MOV		ECX, [ESP + 40] ;アプリ用のCS
		MOV		EDX, [ESP + 44] ;アプリ用のESP
		MOV		EBX, [ESP + 48] ;アプリ用のDS/SS
		MOV		EBP, [ESP + 52] ;tss.esp0の番地
		MOV		[EBP], ESP	;OS用のESP
		MOV		[EBP+4], SS	;OS用のSSを保存
		MOV		ES, BX
		MOV		DS, BX
		MOV		FS, BX
		MOV		GS, BX
; 以下はRETFでアプリに行かせる為のスタック調整
		OR		ECX, 3		;アプリ用のセグメント番号に3をORする
		OR		EBX, 3		;アプリ用のセグメント番号に3をORする
		PUSH	EBX			;アプリのSS
		PUSH	EDX			;アプリのESP
		PUSH	ECX			;アプリのCS
		PUSH	EAX			;アプリのEIP
		RETF
;アプリが終了してもここにはこない

_asm_hrb_api:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD		;保存の為のPUSH
		PUSHAD		;hrb_apiに渡す為のPUSH
		MOV		AX,SS
		MOV		DS,AX		; OS用のセグメントをDSとESにも入れる
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
;EAXはtss.esp0の番地
		MOV		ESP,[EAX]
		POPAD
		RET			;cmd_appに戻る

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
		ADD		ESP, 4		; INT 0x0dではこれが必要
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
		ADD		ESP, 4		; INT 0x0cではこれが必要
		IRETD

_asm_end_app:
		MOV		ESP,[EAX]
		MOV		DWORD [EAX + 4],0
		POPAD
		RET
