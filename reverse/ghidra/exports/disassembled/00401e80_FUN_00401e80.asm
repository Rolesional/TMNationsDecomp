00401e80  PUSH -0x1
00401e82  PUSH 0xbbcc18
00401e87  MOV EAX,FS:[0x0]
00401e8d  PUSH EAX
00401e8e  SUB ESP,0xc
00401e91  PUSH ESI
00401e92  PUSH EDI
00401e93  MOV EAX,[0x00e39150]
00401e98  XOR EAX,ESP
00401e9a  PUSH EAX
00401e9b  LEA EAX,[ESP + 0x18]
00401e9f  MOV FS:[0x0],EAX
00401ea5  MOV ESI,ECX
00401ea7  MOV dword ptr [ESP + 0xc],ESI
00401eab  CALL 0x006b57a0
00401eb0  XOR EDI,EDI
00401eb2  MOV dword ptr [ESI],0xc74d04
00401eb8  MOV dword ptr [ESI + 0x104],EDI
00401ebe  MOV EAX,[0x00c7df70]
00401ec3  MOV dword ptr [ESP + 0x20],EDI
00401ec7  MOV dword ptr [ESI + 0x108],EAX
00401ecd  MOV dword ptr [ESI + 0x10c],EDI
00401ed3  MOV ECX,dword ptr [0x00c7df70]
00401ed9  MOV dword ptr [ESI + 0x110],ECX
00401edf  MOV dword ptr [ESI + 0x114],EDI
00401ee5  MOV EDX,dword ptr [0x00c7df70]
00401eeb  MOV dword ptr [ESI + 0x118],EDX
00401ef1  MOV dword ptr [ESI + 0x11c],EDI
00401ef7  MOV EAX,[0x00c7df70]
00401efc  MOV dword ptr [ESI + 0x120],EAX
00401f02  MOV dword ptr [ESI + 0x124],EDI
00401f08  MOV ECX,dword ptr [0x00c7df70]
00401f0e  MOV dword ptr [ESI + 0x128],ECX
00401f14  MOV dword ptr [ESI + 0x12c],EDI
00401f1a  MOV EDX,dword ptr [0x00c7df70]
00401f20  MOV dword ptr [ESI + 0x130],EDX
00401f26  MOV dword ptr [ESI + 0x134],EDI
00401f2c  MOV dword ptr [ESI + 0x138],EDI
00401f32  MOV EAX,[0x00c7df6c]
00401f37  MOV dword ptr [ESI + 0x13c],EAX
00401f3d  LEA ECX,[ESP + 0x10]
00401f41  PUSH ECX
00401f42  MOV ECX,0xed66a0
00401f47  MOV byte ptr [ESP + 0x24],0x8
00401f4c  MOV dword ptr [ESI + 0x84],0x67
00401f56  MOV dword ptr [ESI + 0x88],0x6d
00401f60  MOV dword ptr [ESI + 0x8c],0x94
00401f6a  MOV dword ptr [ESI + 0x90],0x6c
00401f74  MOV dword ptr [ESI + 0x94],0x95
00401f7e  MOV dword ptr [ESI + 0x98],0x88b8
00401f88  MOV dword ptr [ESI + 0xfc],EDI
00401f8e  MOV dword ptr [ESI + 0x100],EDI
00401f94  MOV dword ptr [ESP + 0x14],0xc74cf4
00401f9c  MOV dword ptr [ESP + 0x18],0x9
00401fa4  CALL 0x00452c20
00401fa9  LEA EDX,[ESP + 0x10]
00401fad  PUSH EDX
00401fae  LEA ECX,[ESI + 0xc4]
00401fb4  MOV dword ptr [ESP + 0x14],0xc74cf4
00401fbc  MOV dword ptr [ESP + 0x18],0x9
00401fc4  CALL 0x00452c20
00401fc9  LEA EAX,[ESP + 0x10]
00401fcd  PUSH EAX
00401fce  LEA ECX,[ESI + 0x7c]
00401fd1  MOV dword ptr [ESP + 0x14],0xc74cf4
00401fd9  MOV dword ptr [ESP + 0x18],0x9
00401fe1  CALL 0x0045a9d0
00401fe6  MOV dword ptr [ESI + 0x140],EDI
00401fec  MOV EAX,ESI
00401fee  MOV ECX,dword ptr [ESP + 0x18]
00401ff2  MOV dword ptr FS:[0x0],ECX
00401ff9  POP ECX
00401ffa  POP EDI
00401ffb  POP ESI
00401ffc  ADD ESP,0x18
00401fff  RET
