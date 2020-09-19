[FORMAT "WCOFF"]		;オブジェクトファイルを作成するモード
[INSTRSET "i486p"]		;486の命令まで使いたいという記述
[BITS 32]				;32ビットモード用の機械語を作らせる
[FILE "a_nask.nas"]		;ソールファイル名

	GLOBAL	_api_putchar
	GLOBAL	_api_end
	
[SECTION .text]

_api_putchar:
	MOV	EDX, 1
	MOV	AL, [ESP+4]
	INT	0x40
	RET
	
_api_end:
	MOV EDX, 4
	INT 0x40