007db970  PUSH ESI
007db971  MOV ESI,ECX
007db973  MOV EAX,dword ptr [ESI + 0x4]
007db976  PUSH EAX
007db977  CALL dword ptr [0x00c712d0]
007db97d  MOV ECX,dword ptr [ESI + 0x8]
007db980  PUSH ECX
007db981  MOV dword ptr [ESI + 0x4],0x0
007db988  CALL dword ptr [0x00c71264]
007db98e  MOV dword ptr [ESI + 0x8],0x0
007db995  POP ESI
007db996  RET
