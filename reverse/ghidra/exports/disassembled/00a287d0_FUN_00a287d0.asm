00a287d0  PUSH ECX
00a287d1  FLD float ptr [ESP + 0x8]
00a287d5  LEA EAX,[ESP]
00a287d8  FMUL double ptr [0x00c82db8]
00a287de  PUSH EAX
00a287df  LEA EDX,[ESP + 0xc]
00a287e3  PUSH EDX
00a287e4  FSTP float ptr [ESP + 0x10]
00a287e8  PUSH ECX
00a287e9  FLD float ptr [ESP + 0x14]
00a287ed  MOV ECX,dword ptr [ECX + 0xac]
00a287f3  FSTP float ptr [ESP]
00a287f6  MOV dword ptr [ESP + 0xc],0x0
00a287fe  CALL 0x00643a40
00a28803  FLD float ptr [ESP + 0x8]
00a28807  POP ECX
00a28808  RET 0x4
