00a28ae0  PUSH ECX
00a28ae1  FLD float ptr [ESP + 0x8]
00a28ae5  LEA EAX,[ESP]
00a28ae8  FMUL double ptr [0x00c82db8]
00a28aee  PUSH EAX
00a28aef  LEA EDX,[ESP + 0xc]
00a28af3  PUSH EDX
00a28af4  FSTP float ptr [ESP + 0x10]
00a28af8  PUSH ECX
00a28af9  FLD float ptr [ESP + 0x14]
00a28afd  MOV ECX,dword ptr [ECX + 0x78]
00a28b00  FSTP float ptr [ESP]
00a28b03  MOV dword ptr [ESP + 0xc],0x0
00a28b0b  CALL 0x00643a40
00a28b10  FLD float ptr [ESP + 0x8]
00a28b14  POP ECX
00a28b15  RET 0x4
