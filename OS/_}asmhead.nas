;iplによって、10シリンダ分読み込んでいるので、0x8200～0x34fff

BOOTPAK	EQU		0x00280000		; bootpackのロード先
DSKCAC	EQU		0x00100000		; ディスクのキャッシュの場所
DSKCACO	EQU		0x00008000		; ディスクのキャッシュの場所 ※インデックスを用いて、本来に読み込まれている箇所にアクセスする


; BOOT_INFO関係
CYLS	EQU		0x0ff0			; ブートセクタが設定する
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; 色数に関数情報
SCRNX	EQU		0x0ff4			; 解像度のX
SCRNY	EQU		0x0ff6			; 解像度のY
VRAM	EQU		0x0ff8			; グラフィックバッファの開始番号

		ORG		0xc200			; ロード場所を0xc200にする
		
; 画面モードの設定

; BIOSの関数の呼び出す条件に沿って、レジスタを設定する
		MOV		AL, 0x13
		MOV		AH, 0x00
		INT		0x10
		MOV		BYTE [VMODE], 8		; 画面モードを記憶しておく
		MOV		WORD [SCRNX], 320	; 出力画面のxサイズを保持
		MOV		WORD [SCRNY], 200	; 出力画面のyサイズを保持
		MOV		DWORD [VRAM], 0x000a0000
		
; キーボードのLED状態をBIOSに教えてもらう
		MOV		AH, 0x02
		INT		0x16
		MOV		[LEDS], AL
		
; PICの割り込みを禁止にする
; PICの初期化前に割り込み禁止にしておかないと、ハングするらしい。

		MOV		AL, 0xff
		OUT		0x21, AL	; （マスター側）IMRを1にする。これにより、IRRが1になっても無視できる
		NOP					; OUT命令を連続させると失敗するケースがあるため、間にNOPを挟む
		OUT		0xa1, AL	; （スレーブ側）IMRを1にする。上記理由と同様
		
		CLI					; 割り込み禁止

; CPUから1MB以上のメモリにアクセスできるように、A20GATEの設定を実施する。
; 16bitCPUとの互換性を保つために、A20GATEは常に0と判断されている。
; そのため、１M以上にアクセスする為に、A20GATEを1にする必要がある。
		CALL	waitkbdout		; アクセスできるか待ち
; キーボードコントローラーの制御を変更してA20GATEを有効にする
		MOV		AL, 0xd1
		OUT		0x64, AL		; アウトプットポートに書き込み要請
		CALL	waitkbdout
		MOV		AL, 0xdf
		OUT		0x60, AL		; A20GATEを有効化
		CALL 	waitkbdout
		
; プロテクトモード移行
[INSTRSET "i486p"]
		LGDT	[GDTR0]			; GDTの設定をGDTRにロードする(セグメントディスクリプタの数とか)
		MOV		EAX, CR0
		AND		EAX,0x7fffffff	; bit31を0にする（ページング禁止のため。セグメント方式を使用する）
		OR		EAX,0x00000001	; bit0を1にする（プロテクトモード移行のため）
		MOV		CR0,EAX
		JMP		pipelineflash	; プロテクトモードに変わると、セグメントレジスタの使用方法が変わる。
								; CPUはパイプライン方式で命令をフェッチしてデコードを次々に実施している
								; そのため、ここでパイプラインにある命令を全てフラッシュする
pipelineflash:
		MOV		AX, 1*8			; ディスクリプタの最初はNULLディスクリプタとIntelの仕様できまっている
		MOV		DS, AX
		MOV		ES, AX
		MOV		FS, AX
		MOV		GS, AX
		MOV		SS, AX
		
; bootpackの転送
		MOV		ESI, bootpack		; 転送元
		MOV		EDI, BOOTPAK		; 転送先
		MOV		ECX, 512*1024/4
		CALL	memcpy
		
; ブートセクタ
		MOV		ESI,0x7c00		; 転送元
		MOV		EDI,DSKCAC		; 転送先
		MOV		ECX,512/4
		CALL	memcpy

; 残り全部

		MOV		ESI,DSKCAC0+512	; 転送元
		MOV		EDI,DSKCAC+512	; 転送先
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; シリンダ数からバイト数/4に変換
		SUB		ECX,512/4		; IPLの分だけ差し引く
		CALL	memcpy

;起動
; この辺はなぞの処理。。。
		MOV		EBX,BOOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 転送するべきものがない
		MOV		ESI,[EBX+20]	; 転送元
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 転送先
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; スタック初期値
		JMP		DWORD 2*8:0x0000001b	;2番目のセグメントの中の1b番地目にジャンプする。2番目のGDTのベースアドレスは0x28

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; ANDの結果が0でなければwaitkbdoutへ
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 引き算した結果が0でなければmemcpyへ
		RET
; memcpyはアドレスサイズプリフィクスを入れ忘れなければ、ストリング命令でも書ける

		ALIGNB	16
GDT0:
		RESB	8				; ヌルセレクタ	1つめのディスクリプタはNULLディスクリプタである。
		DW		0xffff,0x0000,0x9200,0x00cf	; 読み書き可能セグメント32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; 実行可能セグメント32bit（bootpack用）
		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
