[FORMAT "WCOFF"]		;�I�u�W�F�N�g�t�@�C�����쐬���郂�[�h
[INSTRSET "i486p"]		;486�̖��߂܂Ŏg�������Ƃ����L�q
[BITS 32]				;32�r�b�g���[�h�p�̋@�B�����点��
[FILE "a_nask.nas"]		;�\�[���t�@�C����

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