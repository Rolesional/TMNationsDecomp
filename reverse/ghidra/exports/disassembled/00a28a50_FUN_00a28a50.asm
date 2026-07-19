00a28a50  PUSH ECX
00a28a51  FLD float ptr [ESP + 0x8]
00a28a55  LEA EAX,[ESP]
00a28a58  FMUL double ptr [0x00c82db8]
00a28a5e  PUSH EAX
00a28a5f  LEA EDX,[ESP + 0xc]
00a28a63  PUSH EDX
00a28a64  FSTP float ptr [ESP + 0x10]
00a28a68  PUSH ECX
00a28a69  FLD float ptr [ESP + 0x14]
00a28a6d  MOV ECX,dword ptr [ECX + 0x34]
00a28a70  FSTP float ptr [ESP]
00a28a73  MOV dword ptr [ESP + 0xc],0x0
00a28a7b  CALL 0x00643a40
00a28a80  FLD float ptr [ESP + 0x8]
00a28a84  POP ECX
00a28a85  RET 0x4
