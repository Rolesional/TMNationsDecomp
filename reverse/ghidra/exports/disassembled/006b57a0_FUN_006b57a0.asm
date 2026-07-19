006b57a0  PUSH -0x1
006b57a2  PUSH 0xbf0dbc
006b57a7  MOV EAX,FS:[0x0]
006b57ad  PUSH EAX
006b57ae  SUB ESP,0x14
006b57b1  PUSH EBP
006b57b2  PUSH ESI
006b57b3  PUSH EDI
006b57b4  MOV EAX,[0x00e39150]
006b57b9  XOR EAX,ESP
006b57bb  PUSH EAX
006b57bc  LEA EAX,[ESP + 0x24]
006b57c0  MOV FS:[0x0],EAX
006b57c6  MOV ESI,ECX
006b57c8  MOV dword ptr [ESP + 0x10],ESI
006b57cc  CALL 0x00425a80
006b57d1  XOR EDI,EDI
006b57d3  MOV dword ptr [ESP + 0x2c],EDI
006b57d7  LEA ECX,[ESI + 0x14]
006b57da  MOV dword ptr [ESI],0xcb4194
006b57e0  CALL 0x006b55c0
006b57e5  MOV byte ptr [ESP + 0x2c],0x1
006b57ea  MOV dword ptr [ESI + 0x30],EDI
006b57ed  MOV byte ptr [ESP + 0x2c],0x2
006b57f2  LEA EAX,[ESI + 0x34]
006b57f5  MOV dword ptr [EAX],EDI
006b57f7  MOV ECX,dword ptr [0x00c7df70]
006b57fd  MOV dword ptr [ESP + 0x14],EAX
006b5801  MOV dword ptr [EAX + 0x4],ECX
006b5804  MOV byte ptr [ESP + 0x2c],0x4
006b5809  LEA EAX,[ESI + 0x3c]
006b580c  MOV dword ptr [EAX],EDI
006b580e  MOV EDX,dword ptr [0x00c7df6c]
006b5814  MOV dword ptr [ESP + 0x14],EAX
006b5818  MOV dword ptr [EAX + 0x4],EDX
006b581b  MOV byte ptr [ESP + 0x2c],0x6
006b5820  LEA EAX,[ESI + 0x44]
006b5823  MOV dword ptr [EAX],EDI
006b5825  MOV ECX,dword ptr [0x00c7df70]
006b582b  MOV dword ptr [ESP + 0x14],EAX
006b582f  MOV dword ptr [EAX + 0x4],ECX
006b5832  MOV byte ptr [ESP + 0x2c],0x8
006b5837  LEA EAX,[ESI + 0x7c]
006b583a  MOV dword ptr [EAX],EDI
006b583c  MOV EDX,dword ptr [0x00c7df70]
006b5842  MOV dword ptr [ESP + 0x14],EAX
006b5846  MOV dword ptr [EAX + 0x4],EDX
006b5849  MOV byte ptr [ESP + 0x2c],0xa
006b584e  LEA EBP,[ESI + 0xbc]
006b5854  MOV EAX,EBP
006b5856  MOV dword ptr [EAX],EDI
006b5858  MOV ECX,dword ptr [0x00c7df6c]
006b585e  MOV dword ptr [ESP + 0x14],EAX
006b5862  MOV dword ptr [EAX + 0x4],ECX
006b5865  MOV byte ptr [ESP + 0x2c],0xc
006b586a  LEA EAX,[ESI + 0xc4]
006b5870  MOV dword ptr [EAX],EDI
006b5872  MOV EDX,dword ptr [0x00c7df6c]
006b5878  MOV dword ptr [ESP + 0x14],EAX
006b587c  MOV dword ptr [EAX + 0x4],EDX
006b587f  MOV byte ptr [ESP + 0x2c],0xe
006b5884  LEA EAX,[ESI + 0xcc]
006b588a  MOV dword ptr [EAX],EDI
006b588c  MOV ECX,dword ptr [0x00c7df70]
006b5892  MOV dword ptr [ESP + 0x14],EAX
006b5896  MOV dword ptr [EAX + 0x4],ECX
006b5899  MOV byte ptr [ESP + 0x2c],0x10
006b589e  LEA EAX,[ESI + 0xd4]
006b58a4  MOV dword ptr [EAX],EDI
006b58a6  MOV EDX,dword ptr [0x00c7df70]
006b58ac  MOV dword ptr [ESP + 0x14],EAX
006b58b0  MOV dword ptr [EAX + 0x4],EDX
006b58b3  MOV byte ptr [ESP + 0x2c],0x12
006b58b8  LEA EAX,[ESI + 0xdc]
006b58be  MOV dword ptr [EAX],EDI
006b58c0  MOV ECX,dword ptr [0x00c7df70]
006b58c6  MOV dword ptr [ESP + 0x14],EAX
006b58ca  MOV dword ptr [EAX + 0x4],ECX
006b58cd  MOV byte ptr [ESP + 0x2c],0x14
006b58d2  LEA EAX,[ESI + 0xe4]
006b58d8  MOV dword ptr [EAX],EDI
006b58da  MOV EDX,dword ptr [0x00c7df70]
006b58e0  MOV dword ptr [ESP + 0x14],EAX
006b58e4  MOV dword ptr [EAX + 0x4],EDX
006b58e7  MOV byte ptr [ESP + 0x2c],0x16
006b58ec  LEA ECX,[ESI + 0xf0]
006b58f2  CALL 0x006b4230
006b58f7  MOV byte ptr [ESP + 0x2c],0x17
006b58fc  FLDZ
006b58fe  FST float ptr [ESP + 0x14]
006b5902  MOV dword ptr [ESI + 0x50],EDI
006b5905  MOV EAX,dword ptr [ESP + 0x14]
006b5909  FST float ptr [ESP + 0x18]
006b590d  MOV ECX,dword ptr [ESP + 0x18]
006b5911  FSTP float ptr [ESP + 0x1c]
006b5915  FLD1
006b5917  MOV EDX,dword ptr [ESP + 0x1c]
006b591b  MOV dword ptr [ESI + 0x68],EAX
006b591e  FSTP float ptr [ESP + 0x20]
006b5922  MOV dword ptr [ESI + 0x6c],ECX
006b5925  MOV EAX,dword ptr [ESP + 0x20]
006b5929  LEA ECX,[ESP + 0x14]
006b592d  PUSH ECX
006b592e  MOV dword ptr [ESI + 0x70],EDX
006b5931  MOV ECX,EBP
006b5933  MOV dword ptr [ESI + 0x54],EDI
006b5936  MOV dword ptr [ESI + 0x5c],EDI
006b5939  MOV dword ptr [ESI + 0x58],EDI
006b593c  MOV dword ptr [ESI + 0x64],EDI
006b593f  MOV dword ptr [ESI + 0x60],EDI
006b5942  MOV dword ptr [ESI + 0x4c],EDI
006b5945  MOV dword ptr [ESI + 0x74],EAX
006b5948  MOV dword ptr [ESI + 0xa8],EDI
006b594e  MOV dword ptr [ESI + 0x84],EDI
006b5954  MOV dword ptr [ESI + 0x88],EDI
006b595a  MOV dword ptr [ESI + 0x8c],EDI
006b5960  MOV dword ptr [ESI + 0x90],EDI
006b5966  MOV dword ptr [ESI + 0x94],EDI
006b596c  MOV dword ptr [ESI + 0x9c],EDI
006b5972  MOV dword ptr [ESI + 0xa0],EDI
006b5978  MOV dword ptr [ESI + 0xa4],EDI
006b597e  MOV dword ptr [ESI + 0x98],0xffffffff
006b5988  MOV dword ptr [ESI + 0xac],EDI
006b598e  MOV dword ptr [ESI + 0xb0],0x1
006b5998  MOV dword ptr [ESI + 0xb4],0x1
006b59a2  MOV dword ptr [ESI + 0x78],0x4
006b59a9  MOV dword ptr [ESI + 0xec],EDI
006b59af  MOV dword ptr [ESI + 0xb8],EDI
006b59b5  MOV dword ptr [ESP + 0x18],0xcb4240
006b59bd  MOV dword ptr [ESP + 0x1c],0x18
006b59c5  CALL 0x00452c20
006b59ca  MOV dword ptr [ESP + 0x2c],0xffffffff
006b59d2  MOV EAX,ESI
006b59d4  MOV ECX,dword ptr [ESP + 0x24]
006b59d8  MOV dword ptr FS:[0x0],ECX
006b59df  POP ECX
006b59e0  POP EDI
006b59e1  POP ESI
006b59e2  POP EBP
006b59e3  ADD ESP,0x20
006b59e6  RET
