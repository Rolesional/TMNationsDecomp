00a288e0  PUSH ECX
00a288e1  FLD float ptr [ESP + 0x8]
00a288e5  LEA EAX,[ESP]
00a288e8  FMUL double ptr [0x00c82db8]
00a288ee  PUSH EAX
00a288ef  LEA EDX,[ESP + 0xc]
00a288f3  PUSH EDX
00a288f4  FSTP float ptr [ESP + 0x10]
00a288f8  PUSH ECX
00a288f9  FLD float ptr [ESP + 0x14]
00a288fd  MOV ECX,dword ptr [ECX + 0xa0]
00a28903  FSTP float ptr [ESP]
00a28906  MOV dword ptr [ESP + 0xc],0x0
00a2890e  CALL 0x00643a40
00a28913  FLD float ptr [ESP + 0x8]
00a28917  POP ECX
00a28918  RET 0x4
