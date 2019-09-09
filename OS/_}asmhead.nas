;ipl�ɂ���āA10�V�����_���ǂݍ���ł���̂ŁA0x8200�`0x34fff

BOOTPAK	EQU		0x00280000		; bootpack�̃��[�h��
DSKCAC	EQU		0x00100000		; �f�B�X�N�̃L���b�V���̏ꏊ
DSKCACO	EQU		0x00008000		; �f�B�X�N�̃L���b�V���̏ꏊ ���C���f�b�N�X��p���āA�{���ɓǂݍ��܂�Ă���ӏ��ɃA�N�Z�X����


; BOOT_INFO�֌W
CYLS	EQU		0x0ff0			; �u�[�g�Z�N�^���ݒ肷��
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; �F���Ɋ֐����
SCRNX	EQU		0x0ff4			; �𑜓x��X
SCRNY	EQU		0x0ff6			; �𑜓x��Y
VRAM	EQU		0x0ff8			; �O���t�B�b�N�o�b�t�@�̊J�n�ԍ�

		ORG		0xc200			; ���[�h�ꏊ��0xc200�ɂ���
		
; ��ʃ��[�h�̐ݒ�

; BIOS�̊֐��̌Ăяo�������ɉ����āA���W�X�^��ݒ肷��
		MOV		AL, 0x13
		MOV		AH, 0x00
		INT		0x10
		MOV		BYTE [VMODE], 8		; ��ʃ��[�h���L�����Ă���
		MOV		WORD [SCRNX], 320	; �o�͉�ʂ�x�T�C�Y��ێ�
		MOV		WORD [SCRNY], 200	; �o�͉�ʂ�y�T�C�Y��ێ�
		MOV		DWORD [VRAM], 0x000a0000
		
; �L�[�{�[�h��LED��Ԃ�BIOS�ɋ����Ă��炤
		MOV		AH, 0x02
		INT		0x16
		MOV		[LEDS], AL
		
; PIC�̊��荞�݂��֎~�ɂ���
; PIC�̏������O�Ɋ��荞�݋֎~�ɂ��Ă����Ȃ��ƁA�n���O����炵���B

		MOV		AL, 0xff
		OUT		0x21, AL	; �i�}�X�^�[���jIMR��1�ɂ���B����ɂ��AIRR��1�ɂȂ��Ă������ł���
		NOP					; OUT���߂�A��������Ǝ��s����P�[�X�����邽�߁A�Ԃ�NOP������
		OUT		0xa1, AL	; �i�X���[�u���jIMR��1�ɂ���B��L���R�Ɠ��l
		
		CLI					; ���荞�݋֎~

; CPU����1MB�ȏ�̃������ɃA�N�Z�X�ł���悤�ɁAA20GATE�̐ݒ�����{����B
; 16bitCPU�Ƃ̌݊�����ۂ��߂ɁAA20GATE�͏��0�Ɣ��f����Ă���B
; ���̂��߁A�PM�ȏ�ɃA�N�Z�X����ׂɁAA20GATE��1�ɂ���K�v������B
		CALL	waitkbdout		; �A�N�Z�X�ł��邩�҂�
; �L�[�{�[�h�R���g���[���[�̐����ύX����A20GATE��L���ɂ���
		MOV		AL, 0xd1
		OUT		0x64, AL		; �A�E�g�v�b�g�|�[�g�ɏ������ݗv��
		CALL	waitkbdout
		MOV		AL, 0xdf
		OUT		0x60, AL		; A20GATE��L����
		CALL 	waitkbdout
		
; �v���e�N�g���[�h�ڍs
[INSTRSET "i486p"]
		LGDT	[GDTR0]			; GDT�̐ݒ��GDTR�Ƀ��[�h����(�Z�O�����g�f�B�X�N���v�^�̐��Ƃ�)
		MOV		EAX, CR0
		AND		EAX,0x7fffffff	; bit31��0�ɂ���i�y�[�W���O�֎~�̂��߁B�Z�O�����g�������g�p����j
		OR		EAX,0x00000001	; bit0��1�ɂ���i�v���e�N�g���[�h�ڍs�̂��߁j
		MOV		CR0,EAX
		JMP		pipelineflash	; �v���e�N�g���[�h�ɕς��ƁA�Z�O�����g���W�X�^�̎g�p���@���ς��B
								; CPU�̓p�C�v���C�������Ŗ��߂��t�F�b�`���ăf�R�[�h�����X�Ɏ��{���Ă���
								; ���̂��߁A�����Ńp�C�v���C���ɂ��閽�߂�S�ăt���b�V������
pipelineflash:
		MOV		AX, 1*8			; �f�B�X�N���v�^�̍ŏ���NULL�f�B�X�N���v�^��Intel�̎d�l�ł��܂��Ă���
		MOV		DS, AX
		MOV		ES, AX
		MOV		FS, AX
		MOV		GS, AX
		MOV		SS, AX
		
; bootpack�̓]��
		MOV		ESI, bootpack		; �]����
		MOV		EDI, BOOTPAK		; �]����
		MOV		ECX, 512*1024/4
		CALL	memcpy
		
; �u�[�g�Z�N�^
		MOV		ESI,0x7c00		; �]����
		MOV		EDI,DSKCAC		; �]����
		MOV		ECX,512/4
		CALL	memcpy

; �c��S��

		MOV		ESI,DSKCAC0+512	; �]����
		MOV		EDI,DSKCAC+512	; �]����
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; �V�����_������o�C�g��/4�ɕϊ�
		SUB		ECX,512/4		; IPL�̕�������������
		CALL	memcpy

;�N��
; ���̕ӂ͂Ȃ��̏����B�B�B
		MOV		EBX,BOOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; �]������ׂ����̂��Ȃ�
		MOV		ESI,[EBX+20]	; �]����
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; �]����
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; �X�^�b�N�����l
		JMP		DWORD 2*8:0x0000001b	;2�Ԗڂ̃Z�O�����g�̒���1b�Ԓn�ڂɃW�����v����B2�Ԗڂ�GDT�̃x�[�X�A�h���X��0x28

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; AND�̌��ʂ�0�łȂ����waitkbdout��
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; �����Z�������ʂ�0�łȂ����memcpy��
		RET
; memcpy�̓A�h���X�T�C�Y�v���t�B�N�X�����Y��Ȃ���΁A�X�g�����O���߂ł�������

		ALIGNB	16
GDT0:
		RESB	8				; �k���Z���N�^	1�߂̃f�B�X�N���v�^��NULL�f�B�X�N���v�^�ł���B
		DW		0xffff,0x0000,0x9200,0x00cf	; �ǂݏ����\�Z�O�����g32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; ���s�\�Z�O�����g32bit�ibootpack�p�j
		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
