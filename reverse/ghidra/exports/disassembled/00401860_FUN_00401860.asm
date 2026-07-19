00401860  FLDZ
00401862  MOV EAX,dword ptr [ESP + 0x4]
00401866  FST float ptr [ECX + 0x8]
00401869  FST float ptr [ECX + 0x4]
0040186c  FSTP float ptr [ECX]
0040186e  FLD float ptr [EAX]
00401870  FSTP float ptr [ECX + 0xc]
00401873  FLD float ptr [EAX + 0x4]
00401876  FSTP float ptr [ECX + 0x10]
00401879  FLD float ptr [EAX + 0x8]
0040187c  FSTP float ptr [ECX + 0x14]
0040187f  RET 0x4
