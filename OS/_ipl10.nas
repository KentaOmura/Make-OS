;IPLのプログラム

CYLS	EQU		10	;10シリンダ分読み込む

		ORG		0x7c00		; IPLがメインメモリにロードされる場所

; FAT12のBPBを記載。これにより、補助記憶装置のパラメータを認識できるようになる
; 参照（http://elm-chan.org/docs/fat.html）
		JMP		entry
		DB		0x90			; 対象補助記憶装置をWindowsに認識させてもらえるようにする
		DB		"DEMO  OS"		; ブートセクタの名前
		DW		512				; 1セクタの大きさ
		DB		1				; クラスタの大きさ（クラスタっていうと幾つもの集合体を指すが、ここでは１）
		DW		1				; FATがどこから始まるか
		DB		2				; FATの個数（2にしなければいけない）
		DW		224				; ルートディレクトリ領域の大きさ（普通は224エントリにする）
		DW		2880			; このドライブの大きさ（2880セクタにしなければいけない）
		DB		0xf0			; メディアのタイプ（0xf0にしなければいけない）
		DW		9				; FAT領域の長さ（9セクタにしなければいけない）
		DW		18				; 1トラックにいくつのセクタがあるか（18にしなければいけない）
		DW		2				; ヘッドの数（2にしなければいけない）
		DD		0				; パーティションを使ってないのでここは必ず0
		DD		2880			; このドライブ大きさをもう一度書く
		DB		0,0,0x29		; フロッピー0 Windowsでのフォーマット予約0 拡張ブートシグネチャ0x29
		DD		0xffffffff		; ボリュームのシリアル番号（通常はドライバで現在時刻とかに設定するらしい）
		DB		"DEMO  OS   "	; ディスクの名前（11バイト）
		DB		"FAT12   "		; フォーマットの名前（8バイト）
		

; プログラムの本体

entry:
; レジスタの初期化を実施する
		MOV		AX, 0
		MOV		SS, AX
		MOV		SP, 0x7c00 	; スタックポインタに0x7c00を保持する。
		MOV		DS, AX

; ディスクを読み込む
		MOV		AX, 0x0820	; ESに入れる演算が無い。（CPUの回路上）よってAXレジスタにいったん格納
		MOV		ES, AX 		; メモリアクセスする時にES*16が実施される。
							; これは16bitCPUの場合は、64kbまでしかアクセスできなかった。
							; ESレジスタとDSレジスタの組み合わせで一旦1MBまでアクセスできるようにしている
		MOV		CH, 0		; シリンダ0
		MOV		DH, 0		; ヘッド0
		MOV		CH, 2		; セクタ

readloop:
		MOV		SI, 0		; ロードの失敗回数

retry:
; BIOSの割り込み処理を呼び出す条件に倣って、レジスタに値を設定する
		MOV		AH, 0x02	; ディスクの読み込み
		MOV		AL, 1		; 1セクタ
		MOV		BX, 0
		MOV		DL, 0x00	; Aドライブ
		INT		0x13		; BIOSの関数呼び出し。ロード関数
		JNC		next		; キャリーフラグが0なら次のセクタの読み込み（キャリーフラグが立っていたら失敗している）
		ADD		SI, 1		; 失敗回数のカウント
		CMP		SI, 5		; 5回失敗してたら終了。もー読み込まない
		JAE		error
		MOV		AH, 0x00
		MOV		DL, 0x00
		MOV		0x13		; システムのリセット
		JMP		retry		; 再度読み込み

next:
;読み込むディスクのセクタを進める
		MOV		AX, ES		; セクタは512バイト、ESを直接ADDする事はCPUの回路的に不可能
		ADD		AX, 0x0020
		MOV		ES, AX
		ADD		CL, 1		; セクタ数のカウント
		CMP		CL, 18		; 18セクタ分読み込んだかどうか
		JBE		readloop
		MOV		CL, 1
		ADD		DH, 1
		CMP		DH, 2
		JB		readloop
		MOV		DH, 0
		ADD		CH, 1
		CMP		CH, CYLS
		JB		readloop

;読み込みしが完了したので、sysを実行。
		MOV		[0x0ff0], CH	; IPLでどこまで読み込みを実施したかを記憶する
		JMP		0xc200			; イメージファイルを作成した時に、OSのプログラム本体が読み込まれた場所。Bzバイナリエディタで参照。
		
error:
		MOV		SI,msg

putloop:
		MOV		AL, [SI]		; DSレジスタに値は0の為、SIのアドレス番地に格納されている値がそのままレジスタに格納される
		ADD		SI, 1			; SIに1を足す
		CMP		AL, 0
		JE		fin
; BIOSの割り込み処理を呼び出す条件に倣って、レジスタに値を設定する
		MOV		AH, 0x0e
		MOV		BX, 15
		INT		0x10
		JMP		putloop
fin:
;割り込みが発生するまでCPUの動作を一時停止させる。
		HLT
		JMP		fin
msg:
;ロードに失敗した時のメッセージ
		DB		0x0a, 0x0a
		DB		"load error"
		DB		0x0a
		DB		0			; 終端認識用の値
		
		RESB	0x7dfe-$	; 0x7dfeまでを0x00で埋める
		
		DB		0x55, 0xaa	; ブートセクタである事を証明する為のシグニチャ
