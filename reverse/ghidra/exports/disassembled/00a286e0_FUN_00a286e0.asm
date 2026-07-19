00a286e0  SUB ESP,0x8
00a286e3  MOV EAX,dword ptr [ECX + 0x34]
00a286e6  FLD float ptr [ESP + 0xc]
00a286ea  FMUL double ptr [0x00c82db8]
00a286f0  MOV dword ptr [ESP],0x0
00a286f7  MOV dword ptr [EAX + 0x28],0x1
00a286fe  LEA EDX,[ESP]
00a28701  PUSH EDX
00a28702  FSTP float ptr [ESP + 0x10]
00a28706  FLD float ptr [ESP + 0x10]
00a2870a  LEA EAX,[ESP + 0x8]
00a2870e  PUSH EAX
00a2870f  PUSH ECX
00a28710  MOV ECX,dword ptr [ECX + 0x34]
00a28713  FSTP float ptr [ESP]
00a28716  CALL 0x00643a40
00a2871b  FLD float ptr [ESP + 0x4]
00a2871f  ADD ESP,0x8
00a28722  RET 0x4
