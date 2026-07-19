00a28c00  PUSH ECX
00a28c01  FLD float ptr [ESP + 0x8]
00a28c05  LEA EAX,[ESP]
00a28c08  FMUL double ptr [0x00c82db8]
00a28c0e  PUSH EAX
00a28c0f  LEA EDX,[ESP + 0xc]
00a28c13  PUSH EDX
00a28c14  FSTP float ptr [ESP + 0x10]
00a28c18  PUSH ECX
00a28c19  FLD float ptr [ESP + 0x14]
00a28c1d  MOV ECX,dword ptr [ECX + 0x230]
00a28c23  FSTP float ptr [ESP]
00a28c26  MOV dword ptr [ESP + 0xc],0x0
00a28c2e  CALL 0x00643a40
00a28c33  FLD float ptr [ESP + 0x8]
00a28c37  POP ECX
00a28c38  RET 0x4
