007ab9e0  PUSH ESI
007ab9e1  PUSH EDI
007ab9e2  MOV EDI,dword ptr [ESP + 0x10]
007ab9e6  PUSH EDI
007ab9e7  MOV ESI,ECX
007ab9e9  CALL 0x007a6fd0
007ab9ee  MOV EAX,dword ptr [ESP + 0xc]
007ab9f2  TEST EAX,EAX
007ab9f4  MOV EDX,dword ptr [ESP + 0x14]
007ab9f8  MOV dword ptr [ESI + 0x78],EDX
007ab9fb  JNZ 0x007aba04
007ab9fd  POP EDI
007ab9fe  XOR EAX,EAX
007aba00  POP ESI
007aba01  RET 0x10
007aba04  MOV ECX,dword ptr [EAX + 0x8]
007aba07  TEST ECX,ECX
007aba09  MOV dword ptr [ESI + 0x50],ECX
007aba0c  JZ 0x007ab9fd
007aba0e  CMP dword ptr [ECX + 0x60],0x0
007aba12  JNZ 0x007ab9fd
007aba14  CMP EDX,0x1
007aba17  JNZ 0x007aba59
007aba19  CALL 0x007c9a70
007aba1e  MOV EDI,EAX
007aba20  MOV ECX,dword ptr [EDI + 0x6c]
007aba23  MOV EAX,dword ptr [ECX]
007aba25  MOV EDX,dword ptr [EAX]
007aba27  PUSH 0x1
007aba29  PUSH 0x2
007aba2b  PUSH EDI
007aba2c  CALL EDX
007aba2e  TEST EAX,EAX
007aba30  MOV dword ptr [ESI + 0x4],EAX
007aba33  JZ 0x007ab9fd
007aba35  PUSH EBX
007aba36  MOV ECX,ESI
007aba38  CALL 0x007a7f20
007aba3d  MOV EBX,EAX
007aba3f  MOV EAX,dword ptr [ESI + 0x4]
007aba42  PUSH EAX
007aba43  MOV ECX,EDI
007aba45  CALL 0x004c79f0
007aba4a  MOV EAX,EBX
007aba4c  POP EBX
007aba4d  POP EDI
007aba4e  MOV dword ptr [ESI + 0x4],0x0
007aba55  POP ESI
007aba56  RET 0x10
007aba59  CMP dword ptr [ESP + 0x18],0x0
007aba5e  MOV ECX,ESI
007aba60  JZ 0x007aba6e
007aba62  PUSH EDI
007aba63  PUSH EAX
007aba64  CALL 0x007ab1b0
007aba69  POP EDI
007aba6a  POP ESI
007aba6b  RET 0x10
007aba6e  PUSH EAX
007aba6f  CALL 0x007aa800
007aba74  POP EDI
007aba75  POP ESI
007aba76  RET 0x10
