009d6190  SUB ESP,0x18
009d6193  PUSH ESI
009d6194  MOV ESI,dword ptr [ESP + 0x30]
009d6198  FLD float ptr [ESI + 0x8]
009d619b  PUSH EDI
009d619c  MOV EDI,dword ptr [ESP + 0x38]
009d61a0  FMUL float ptr [EDI + 0x4]
009d61a3  MOV EAX,dword ptr [ESP + 0x28]
009d61a7  FLD float ptr [ESI + 0x4]
009d61aa  PUSH EAX
009d61ab  FMUL float ptr [EDI + 0x8]
009d61ae  LEA ECX,[ESP + 0xc]
009d61b2  FSUBP
009d61b4  FSTP float ptr [ESP + 0xc]
009d61b8  FLD float ptr [ESI]
009d61ba  FMUL float ptr [EDI + 0x8]
009d61bd  FLD float ptr [EDI]
009d61bf  FMUL float ptr [ESI + 0x8]
009d61c2  FSUBP
009d61c4  FSTP float ptr [ESP + 0x10]
009d61c8  FLD float ptr [ESI + 0x4]
009d61cb  FMUL float ptr [EDI]
009d61cd  FLD float ptr [ESI]
009d61cf  FMUL float ptr [EDI + 0x4]
009d61d2  FSUBP
009d61d4  FSTP float ptr [ESP + 0x14]
009d61d8  CALL 0x0046a170
009d61dd  FLD float ptr [EDI + 0x8]
009d61e0  MOV ECX,dword ptr [ESP + 0x8]
009d61e4  FMUL float ptr [ESP + 0xc]
009d61e8  MOV EAX,dword ptr [ESP + 0x10]
009d61ec  FLD float ptr [ESP + 0x10]
009d61f0  MOV EDX,dword ptr [ESP + 0xc]
009d61f4  FMUL float ptr [EDI + 0x4]
009d61f7  MOV dword ptr [ESP + 0x1c],EAX
009d61fb  MOV EAX,dword ptr [ESP + 0x30]
009d61ff  MOV dword ptr [ESP + 0x14],ECX
009d6203  FSUBP
009d6205  MOV dword ptr [ESP + 0x18],EDX
009d6209  FSTP float ptr [ESP + 0x8]
009d620d  FLD float ptr [EDI]
009d620f  FMUL float ptr [ESP + 0x1c]
009d6213  FLD float ptr [EDI + 0x8]
009d6216  FLD float ptr [ESP + 0x14]
009d621a  FLD ST0
009d621c  FMULP ST2
009d621e  FXCH ST2
009d6220  FSUBRP
009d6222  FSTP float ptr [ESP + 0xc]
009d6226  FMUL float ptr [EDI + 0x4]
009d6229  FLD float ptr [EDI]
009d622b  FMUL float ptr [ESP + 0x18]
009d622f  FSUBP
009d6231  FSTP float ptr [ESP + 0x10]
009d6235  FLD float ptr [EAX + 0x4]
009d6238  FMUL float ptr [ESI + 0x4]
009d623b  FLD float ptr [EAX]
009d623d  FMUL float ptr [ESI]
009d623f  FADDP
009d6241  FLD float ptr [EAX + 0x8]
009d6244  FMUL float ptr [ESI + 0x8]
009d6247  FADDP
009d6249  FSTP float ptr [ESP + 0x34]
009d624d  FLD float ptr [ESP + 0x2c]
009d6251  FLD1
009d6253  FSUB ST1,ST0
009d6255  FXCH
009d6257  FMUL float ptr [ESP + 0x34]
009d625b  FSTP float ptr [ESP + 0x38]
009d625f  FLD float ptr [ESI]
009d6261  FMUL float ptr [ESP + 0x8]
009d6265  FLD float ptr [ESI + 0x4]
009d6268  FMUL float ptr [ESP + 0xc]
009d626c  FADDP
009d626e  FLD float ptr [ESP + 0x10]
009d6272  FMUL float ptr [ESI + 0x8]
009d6275  FADDP
009d6277  FSTP float ptr [ESP + 0x34]
009d627b  FLD float ptr [ESP + 0x24]
009d627f  FDIVP
009d6281  FADD float ptr [ESP + 0x34]
009d6285  FSTP float ptr [ESP + 0x34]
009d6289  FLD float ptr [ESP + 0x34]
009d628d  FLD ST0
009d628f  FABS
009d6291  FSTP float ptr [ESP + 0x34]
009d6295  FLD float ptr [ESP + 0x34]
009d6299  FCOMP float ptr [0x00d1453c]
009d629f  FNSTSW AX
009d62a1  TEST AH,0x5
009d62a4  MOV EAX,dword ptr [ESP + 0x3c]
009d62a8  JP 0x009d62b4
009d62aa  FLDZ
009d62ac  FST float ptr [EAX + 0x8]
009d62af  FST float ptr [EAX + 0x4]
009d62b2  FSTP float ptr [EAX]
009d62b4  FDIVR float ptr [ESP + 0x38]
009d62b8  POP EDI
009d62b9  FSTP float ptr [ESP + 0x30]
009d62bd  FLD float ptr [ESI]
009d62bf  FLD float ptr [ESP + 0x30]
009d62c3  FLD ST0
009d62c5  FMULP ST2
009d62c7  FXCH
009d62c9  FSTP float ptr [EAX]
009d62cb  FLD float ptr [ESI + 0x4]
009d62ce  FMUL ST1
009d62d0  FSTP float ptr [EAX + 0x4]
009d62d3  FMUL float ptr [ESI + 0x8]
009d62d6  POP ESI
009d62d7  FSTP float ptr [EAX + 0x8]
009d62da  ADD ESP,0x18
009d62dd  RET
