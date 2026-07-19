00a28a90  PUSH ECX
00a28a91  FLD float ptr [ESP + 0x8]
00a28a95  PUSH ESI
00a28a96  FMUL double ptr [0x00c82db8]
00a28a9c  MOV ESI,ECX
00a28a9e  LEA EAX,[ESP + 0x4]
00a28aa2  PUSH EAX
00a28aa3  LEA ECX,[ESP + 0x10]
00a28aa7  FSTP float ptr [ESP + 0x10]
00a28aab  FLD float ptr [ESP + 0x10]
00a28aaf  PUSH ECX
00a28ab0  PUSH ECX
00a28ab1  MOV ECX,dword ptr [ESI + 0x1e0]
00a28ab7  FSTP float ptr [ESP]
00a28aba  MOV dword ptr [ESP + 0x10],0x0
00a28ac2  CALL 0x00643a40
00a28ac7  FLD float ptr [ESI + 0x1e4]
00a28acd  FMUL float ptr [ESP + 0xc]
00a28ad1  POP ESI
00a28ad2  FSTP float ptr [ESP + 0x8]
00a28ad6  FLD float ptr [ESP + 0x8]
00a28ada  POP ECX
00a28adb  RET 0x4
