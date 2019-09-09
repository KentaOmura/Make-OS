;IPL�̃v���O����

CYLS	EQU		10	;10�V�����_���ǂݍ���

		ORG		0x7c00		; IPL�����C���������Ƀ��[�h�����ꏊ

; FAT12��BPB���L�ځB����ɂ��A�⏕�L�����u�̃p�����[�^��F���ł���悤�ɂȂ�
; �Q�Ɓihttp://elm-chan.org/docs/fat.html�j
		JMP		entry
		DB		0x90			; �Ώە⏕�L�����u��Windows�ɔF�������Ă��炦��悤�ɂ���
		DB		"DEMO  OS"		; �u�[�g�Z�N�^�̖��O
		DW		512				; 1�Z�N�^�̑傫��
		DB		1				; �N���X�^�̑傫���i�N���X�^���Ă����Ɗ���̏W���̂��w�����A�����ł͂P�j
		DW		1				; FAT���ǂ�����n�܂邩
		DB		2				; FAT�̌��i2�ɂ��Ȃ���΂����Ȃ��j
		DW		224				; ���[�g�f�B���N�g���̈�̑傫���i���ʂ�224�G���g���ɂ���j
		DW		2880			; ���̃h���C�u�̑傫���i2880�Z�N�^�ɂ��Ȃ���΂����Ȃ��j
		DB		0xf0			; ���f�B�A�̃^�C�v�i0xf0�ɂ��Ȃ���΂����Ȃ��j
		DW		9				; FAT�̈�̒����i9�Z�N�^�ɂ��Ȃ���΂����Ȃ��j
		DW		18				; 1�g���b�N�ɂ����̃Z�N�^�����邩�i18�ɂ��Ȃ���΂����Ȃ��j
		DW		2				; �w�b�h�̐��i2�ɂ��Ȃ���΂����Ȃ��j
		DD		0				; �p�[�e�B�V�������g���ĂȂ��̂ł����͕K��0
		DD		2880			; ���̃h���C�u�傫����������x����
		DB		0,0,0x29		; �t���b�s�[0 Windows�ł̃t�H�[�}�b�g�\��0 �g���u�[�g�V�O�l�`��0x29
		DD		0xffffffff		; �{�����[���̃V���A���ԍ��i�ʏ�̓h���C�o�Ō��ݎ����Ƃ��ɐݒ肷��炵���j
		DB		"DEMO  OS   "	; �f�B�X�N�̖��O�i11�o�C�g�j
		DB		"FAT12   "		; �t�H�[�}�b�g�̖��O�i8�o�C�g�j
		

; �v���O�����̖{��

entry:
; ���W�X�^�̏����������{����
		MOV		AX, 0
		MOV		SS, AX
		MOV		SP, 0x7c00 	; �X�^�b�N�|�C���^��0x7c00��ێ�����B
		MOV		DS, AX

; �f�B�X�N��ǂݍ���
		MOV		AX, 0x0820	; ES�ɓ���鉉�Z�������B�iCPU�̉�H��j�����AX���W�X�^�ɂ�������i�[
		MOV		ES, AX 		; �������A�N�Z�X���鎞��ES*16�����{�����B
							; �����16bitCPU�̏ꍇ�́A64kb�܂ł����A�N�Z�X�ł��Ȃ������B
							; ES���W�X�^��DS���W�X�^�̑g�ݍ��킹�ň�U1MB�܂ŃA�N�Z�X�ł���悤�ɂ��Ă���
		MOV		CH, 0		; �V�����_0
		MOV		DH, 0		; �w�b�h0
		MOV		CH, 2		; �Z�N�^

readloop:
		MOV		SI, 0		; ���[�h�̎��s��

retry:
; BIOS�̊��荞�ݏ������Ăяo�������ɕ���āA���W�X�^�ɒl��ݒ肷��
		MOV		AH, 0x02	; �f�B�X�N�̓ǂݍ���
		MOV		AL, 1		; 1�Z�N�^
		MOV		BX, 0
		MOV		DL, 0x00	; A�h���C�u
		INT		0x13		; BIOS�̊֐��Ăяo���B���[�h�֐�
		JNC		next		; �L�����[�t���O��0�Ȃ玟�̃Z�N�^�̓ǂݍ��݁i�L�����[�t���O�������Ă����玸�s���Ă���j
		ADD		SI, 1		; ���s�񐔂̃J�E���g
		CMP		SI, 5		; 5�񎸔s���Ă���I���B���[�ǂݍ��܂Ȃ�
		JAE		error
		MOV		AH, 0x00
		MOV		DL, 0x00
		MOV		0x13		; �V�X�e���̃��Z�b�g
		JMP		retry		; �ēx�ǂݍ���

next:
;�ǂݍ��ރf�B�X�N�̃Z�N�^��i�߂�
		MOV		AX, ES		; �Z�N�^��512�o�C�g�AES�𒼐�ADD���鎖��CPU�̉�H�I�ɕs�\
		ADD		AX, 0x0020
		MOV		ES, AX
		ADD		CL, 1		; �Z�N�^���̃J�E���g
		CMP		CL, 18		; 18�Z�N�^���ǂݍ��񂾂��ǂ���
		JBE		readloop
		MOV		CL, 1
		ADD		DH, 1
		CMP		DH, 2
		JB		readloop
		MOV		DH, 0
		ADD		CH, 1
		CMP		CH, CYLS
		JB		readloop

;�ǂݍ��݂������������̂ŁAsys�����s�B
		MOV		[0x0ff0], CH	; IPL�łǂ��܂œǂݍ��݂����{���������L������
		JMP		0xc200			; �C���[�W�t�@�C�����쐬�������ɁAOS�̃v���O�����{�̂��ǂݍ��܂ꂽ�ꏊ�BBz�o�C�i���G�f�B�^�ŎQ�ƁB
		
error:
		MOV		SI,msg

putloop:
		MOV		AL, [SI]		; DS���W�X�^�ɒl��0�ׁ̈ASI�̃A�h���X�Ԓn�Ɋi�[����Ă���l�����̂܂܃��W�X�^�Ɋi�[�����
		ADD		SI, 1			; SI��1�𑫂�
		CMP		AL, 0
		JE		fin
; BIOS�̊��荞�ݏ������Ăяo�������ɕ���āA���W�X�^�ɒl��ݒ肷��
		MOV		AH, 0x0e
		MOV		BX, 15
		INT		0x10
		JMP		putloop
fin:
;���荞�݂���������܂�CPU�̓�����ꎞ��~������B
		HLT
		JMP		fin
msg:
;���[�h�Ɏ��s�������̃��b�Z�[�W
		DB		0x0a, 0x0a
		DB		"load error"
		DB		0x0a
		DB		0			; �I�[�F���p�̒l
		
		RESB	0x7dfe-$	; 0x7dfe�܂ł�0x00�Ŗ��߂�
		
		DB		0x55, 0xaa	; �u�[�g�Z�N�^�ł��鎖���ؖ�����ׂ̃V�O�j�`��
