; hello-os
; TAB=4
CYLS	EQU		10				; �ǂ��܂œǂݍ��ނ�

		ORG		0x7c00			; ���̃v���O�������ǂ��ɓǂݍ��܂��̂�

; �ȉ��͕W���I��FAT12�t�H�[�}�b�g�t���b�s�[�f�B�X�N�̂��߂̋L�q

		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; �u�[�g�Z�N�^�̖��O�����R�ɏ����Ă悢�i8�o�C�g�j
		DW		512				; 1�Z�N�^�̑傫���i512�ɂ��Ȃ���΂����Ȃ��j
		DB		1				; �N���X�^�̑傫���i1�Z�N�^�ɂ��Ȃ���΂����Ȃ��j
		DW		1				; FAT���ǂ�����n�܂邩�i���ʂ�1�Z�N�^�ڂ���ɂ���j
		DB		2				; FAT�̌��i2�ɂ��Ȃ���΂����Ȃ��j
		DW		224				; ���[�g�f�B���N�g���̈�̑傫���i���ʂ�224�G���g���ɂ���j
		DW		2880			; ���̃h���C�u�̑傫���i2880�Z�N�^�ɂ��Ȃ���΂����Ȃ��j
		DB		0xf0			; ���f�B�A�̃^�C�v�i0xf0�ɂ��Ȃ���΂����Ȃ��j
		DW		9				; FAT�̈�̒����i9�Z�N�^�ɂ��Ȃ���΂����Ȃ��j
		DW		18				; 1�g���b�N�ɂ����̃Z�N�^�����邩�i18�ɂ��Ȃ���΂����Ȃ��j
		DW		2				; �w�b�h�̐��i2�ɂ��Ȃ���΂����Ȃ��j
		DD		0				; �p�[�e�B�V�������g���ĂȂ��̂ł����͕K��0
		DD		2880			; ���̃h���C�u�傫����������x����
		DB		0,0,0x29		; �悭�킩��Ȃ����ǂ��̒l�ɂ��Ă����Ƃ����炵��
		DD		0xffffffff		; ���Ԃ�{�����[���V���A���ԍ�
		DB		"HARIBOTEOS "	; �f�B�X�N�̖��O�i11�o�C�g�j
		DB		"FAT12   "		; �t�H�[�}�b�g�̖��O�i8�o�C�g�j
		RESB	18				; �Ƃ肠����18�o�C�g�����Ă���

; �v���O�����{��

entry:
		MOV		AX,0			; ���W�X�^������
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

; �f�B�X�N��ǂ�
		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; �V�����_0
		MOV		DH,0			; �w�b�h0
		MOV		CL,2			; �Z�N�^2 �{�v���O�������̂��Z�N�^1�ɏ�������B���ׁ̈A���̃Z�N�^����ǂݎ��
readloop:
		MOV		AH,0x02
		MOV		AL,1
		MOV		BX,0
		MOV		DL,0x00			; A�h���C�u
		INT		0x13
		JNC		next
next:
		MOV		AX,ES
		ADD		AX,0x0020		; 512�o�C�g��ɓǂݍ��ނ悤�ɐݒ�
		MOV		ES,AX
		ADD		CL,1			; �Z�N�^��1�i�߂�B�t���b�s�[�ł����1�Z�N�^��512�o�C�g
		CMP		CL,18			; 1�̃V�����_��18�Z�N�^���݂���B
		JBE		readloop
		MOV		CL, 1
		ADD		DH, 1			; ���̃f�B�X�N��ǂݎ���Ă���
		CMP		DH, 2			; �㉺�w�b�h��2�Ȃ̂�
		JB		readloop
		MOV		DH, 0			; �\�ɐ؂�ւ�
		ADD		CH, 1			; �ǂݍ��ރV�����_��i�߂�
		CMP		CH, CYLS
		JB		readloop		; CYLS�̒�`�l���V�����_�������Ȃ��ꍇ

; �ǂݏI������̂�haribote.sys�����s���I

		MOV		[0x0ff0],CH		; IPL���ǂ��܂œǂ񂾂̂�������
		JMP		0xc200			; SYSTEM���ǂݍ��܂��ӏ� �u�[�g�Z�N�^��0x8000�n�_��SYS�͂���0x4200���


		RESB	0x7dfe-$		; 0x7dfe�܂ł�0x00�Ŗ��߂閽��
		
		DB		0x55, 0xaa
