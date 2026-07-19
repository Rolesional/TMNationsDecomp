007a7cc0  PUSH -0x1
007a7cc2  PUSH 0xc01706
007a7cc7  MOV EAX,FS:[0x0]
007a7ccd  PUSH EAX
007a7cce  PUSH ECX
007a7ccf  PUSH ESI
007a7cd0  PUSH EDI
007a7cd1  MOV EAX,[0x00e39150]
007a7cd6  XOR EAX,ESP
007a7cd8  PUSH EAX
007a7cd9  LEA EAX,[ESP + 0x10]
007a7cdd  MOV FS:[0x0],EAX
007a7ce3  MOV ESI,ECX
007a7ce5  MOV dword ptr [ESP + 0xc],ESI
007a7ce9  CALL 0x00460110
007a7cee  XOR EDI,EDI
007a7cf0  LEA ECX,[ESI + 0x20]
007a7cf3  MOV dword ptr [ESP + 0x18],EDI
007a7cf7  MOV dword ptr [ESI],0xcd53bc
007a7cfd  CALL 0x007a7470
007a7d02  LEA ECX,[ESI + 0x28]
007a7d05  CALL 0x007a74a0
007a7d0a  LEA ECX,[ESI + 0x34]
007a7d0d  CALL 0x007a74e0
007a7d12  LEA ECX,[ESI + 0x40]
007a7d15  CALL 0x007a7550
007a7d1a  LEA ECX,[ESI + 0x60]
007a7d1d  CALL 0x00502300
007a7d22  LEA ECX,[ESI + 0x6c]
007a7d25  CALL 0x00502300
007a7d2a  LEA ECX,[ESI + 0xa8]
007a7d30  CALL 0x00502300
007a7d35  LEA ECX,[ESI + 0xb4]
007a7d3b  CALL 0x00532ed0
007a7d40  LEA ECX,[ESI + 0xc0]
007a7d46  MOV byte ptr [ESP + 0x18],0x8
007a7d4b  CALL 0x007c86e0
007a7d50  MOV ECX,dword ptr [0x00ebd940]
007a7d56  PUSH 0xb
007a7d58  ADD ECX,0x20
007a7d5b  CALL 0x00401770
007a7d60  MOV EAX,dword ptr [EAX]
007a7d62  MOV dword ptr [ESI + 0x1c],EAX
007a7d65  MOV dword ptr [ESI + 0x7c],EDI
007a7d68  MOV dword ptr [ESI + 0x8c],EDI
007a7d6e  MOV dword ptr [ESI + 0x94],EDI
007a7d74  MOV dword ptr [ESI + 0x98],EDI
007a7d7a  MOV dword ptr [ESI + 0x84],EDI
007a7d80  MOV dword ptr [ESI + 0x88],EDI
007a7d86  MOV dword ptr [ESI + 0xa4],EDI
007a7d8c  MOV dword ptr [ESI + 0x58],EDI
007a7d8f  MOV dword ptr [ESI + 0x90],0x8
007a7d99  MOV dword ptr [ESI + 0x78],0x7
007a7da0  MOV dword ptr [ESI + 0x5c],EDI
007a7da3  MOV dword ptr [ESI + 0x80],EDI
007a7da9  MOV dword ptr [ESI + 0x10],0x1
007a7db0  MOV word ptr [ESI + 0x18],0x6
007a7db6  MOV EAX,ESI
007a7db8  MOV ECX,dword ptr [ESP + 0x10]
007a7dbc  MOV dword ptr FS:[0x0],ECX
007a7dc3  POP ECX
007a7dc4  POP EDI
007a7dc5  POP ESI
007a7dc6  ADD ESP,0x10
007a7dc9  RET
