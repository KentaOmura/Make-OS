[INSTRSET "i486p"]
[BITS 32
	MOV EDX, 2
	MOV	EBX,msg
	INT 0x40
	MOV EDX, 4
	INT 0x40
	RETF ;farJump�ŌĂяo�����̂ŁARETF���g�p����
msg:
	DB	"hello",0
	