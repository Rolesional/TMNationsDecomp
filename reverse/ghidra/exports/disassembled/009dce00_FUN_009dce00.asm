009dce00  SUB ESP,0x20
009dce03  PUSH EBX
009dce04  PUSH EBP
009dce05  PUSH ESI
009dce06  PUSH EDI
009dce07  MOV ESI,ECX
009dce09  MOV ECX,dword ptr [ESI + 0x28]
009dce0c  LEA EAX,[ESP + 0x24]
009dce10  PUSH EAX
009dce11  CALL 0x0067aa70
009dce16  TEST byte ptr [ESI + 0x2f4],0x1
009dce1d  JZ 0x009dcfb6
009dce23  MOV EDI,dword ptr [ESI + 0x64]
009dce26  MOV ECX,dword ptr [EDI + 0x24]
009dce29  PUSH ECX
009dce2a  LEA ECX,[EDI + 0x14]
009dce2d  CALL 0x009bc110
009dce32  MOV EDX,dword ptr [EDI + 0x24]
009dce35  PUSH EDX
009dce36  LEA ECX,[EDI + 0x14]
009dce39  MOV EBX,EAX
009dce3b  CALL 0x009bc110
009dce40  FLD float ptr [ESP + 0x2c]
009dce44  MOV ECX,dword ptr [EBX]
009dce46  FABS
009dce48  MOV EDX,dword ptr [EAX]
009dce4a  FSTP float ptr [ESP + 0x10]
009dce4e  FLD float ptr [ESP + 0x10]
009dce52  FMUL float ptr [ECX + 0x70]
009dce55  LEA ECX,[ESI + 0x2e8]
009dce5b  FADD float ptr [EDX + 0x6c]
009dce5e  FSTP float ptr [ESP + 0x14]
009dce62  CALL 0x009d5a40
009dce67  XOR EBP,EBP
009dce69  TEST EAX,EAX
009dce6b  MOV dword ptr [ESP + 0x20],EAX
009dce6f  JBE 0x009dcfb6
009dce75  JMP 0x009dce80
009dce80  PUSH EBP
009dce81  LEA ECX,[ESI + 0x2e8]
009dce87  CALL 0x006f1300
009dce8c  MOV EDI,EAX
009dce8e  LEA EAX,[EDI + 0x10]
009dce91  LEA EBX,[EDI + 0xc0]
009dce97  PUSH EAX
009dce98  MOV ECX,EBX
009dce9a  CALL 0x0046d010
009dce9f  CMP dword ptr [EDI + 0x4],0x0
009dcea3  JZ 0x009dcf74
009dcea9  FLD float ptr [0x00d1453c]
009dceaf  FLD float ptr [ESP + 0x14]
009dceb3  FCOM
009dceb5  FNSTSW AX
009dceb7  FSTP ST1
009dceb9  TEST AH,0x5
009dcebc  JP 0x009dcec4
009dcebe  FSTP ST0
009dcec0  FLDZ
009dcec2  JMP 0x009dcece
009dcec4  FLD float ptr [ESI + 0x5e8]
009dceca  FCHS
009dcecc  FDIVRP
009dcece  FSTP float ptr [ESP + 0x18]
009dced2  PUSH ECX
009dced3  FLD float ptr [ESP + 0x1c]
009dced7  MOV ECX,EBX
009dced9  FSTP float ptr [ESP]
009dcedc  CALL 0x0046d740
009dcee1  FLD float ptr [0x00c81540]
009dcee7  MOV EBX,dword ptr [ESI + 0x64]
009dceea  FSTP float ptr [ESP + 0x10]
009dceee  MOV ECX,dword ptr [EBX + 0x24]
009dcef1  PUSH ECX
009dcef2  LEA ECX,[EBX + 0x14]
009dcef5  CALL 0x009bc110
009dcefa  MOV EDX,dword ptr [EAX]
009dcefc  CMP dword ptr [EDX + 0x378],0x0
009dcf03  JZ 0x009dcf50
009dcf05  MOV dword ptr [ESP + 0x18],0x0
009dcf0d  MOV EAX,dword ptr [EBX + 0x24]
009dcf10  PUSH EAX
009dcf11  LEA ECX,[EBX + 0x14]
009dcf14  CALL 0x009bc110
009dcf19  FLD float ptr [ESP + 0x2c]
009dcf1d  MOV EAX,dword ptr [EAX]
009dcf1f  FABS
009dcf21  FSTP float ptr [ESP + 0x1c]
009dcf25  LEA ECX,[ESP + 0x18]
009dcf29  FLD float ptr [ESP + 0x1c]
009dcf2d  PUSH ECX
009dcf2e  FMUL double ptr [0x00c82db8]
009dcf34  LEA EDX,[ESP + 0x14]
009dcf38  PUSH EDX
009dcf39  PUSH ECX
009dcf3a  MOV ECX,dword ptr [EAX + 0x378]
009dcf40  FSTP float ptr [ESP + 0x28]
009dcf44  FLD float ptr [ESP + 0x28]
009dcf48  FSTP float ptr [ESP]
009dcf4b  CALL 0x00643a40
009dcf50  FLD float ptr [ESP + 0x10]
009dcf54  FMUL double ptr [0x00c80bb8]
009dcf5a  FDIV double ptr [0x00c80bb0]
009dcf60  FSTP float ptr [ESP + 0x1c]
009dcf64  FLD float ptr [ESP + 0x1c]
009dcf68  FLD float ptr [ESI + 0x5e8]
009dcf6e  FCHS
009dcf70  FMULP
009dcf72  JMP 0x009dcf76
009dcf74  FLDZ
009dcf76  FSTP float ptr [EDI + 0x158]
009dcf7c  SUB ESP,0x8
009dcf7f  FLD float ptr [ESP + 0x3c]
009dcf83  MOV ECX,ESI
009dcf85  FSTP float ptr [ESP + 0x4]
009dcf89  FLD float ptr [ESP + 0x34]
009dcf8d  FSTP float ptr [ESP]
009dcf90  PUSH EDI
009dcf91  CALL 0x009da130
009dcf96  FLD float ptr [ESP + 0x34]
009dcf9a  PUSH ECX
009dcf9b  LEA ECX,[EDI + 0xb4]
009dcfa1  FSTP float ptr [ESP]
009dcfa4  CALL 0x009da2d0
009dcfa9  ADD EBP,0x1
009dcfac  CMP EBP,dword ptr [ESP + 0x20]
009dcfb0  JC 0x009dce80
009dcfb6  TEST byte ptr [ESI + 0x2f4],0x2
009dcfbd  JZ 0x009dcff3
009dcfbf  LEA EBX,[ESI + 0x2e8]
009dcfc5  MOV ECX,EBX
009dcfc7  CALL 0x009d5a40
009dcfcc  MOV EBP,EAX
009dcfce  XOR EDI,EDI
009dcfd0  TEST EBP,EBP
009dcfd2  JBE 0x009dcff3
009dcfd4  FLD float ptr [ESP + 0x34]
009dcfd8  PUSH ECX
009dcfd9  FSTP float ptr [ESP]
009dcfdc  PUSH EDI
009dcfdd  MOV ECX,EBX
009dcfdf  CALL 0x006f1300
009dcfe4  PUSH EAX
009dcfe5  MOV ECX,ESI
009dcfe7  CALL 0x009d65c0
009dcfec  ADD EDI,0x1
009dcfef  CMP EDI,EBP
009dcff1  JC 0x009dcfd4
009dcff3  TEST byte ptr [ESI + 0x2f4],0x4
009dcffa  JZ 0x009dd03d
009dcffc  CMP dword ptr [ESI + 0x60c],0x0
009dd003  JZ 0x009dd00f
009dd005  FLDZ
009dd007  FSTP float ptr [ESI + 0x5b4]
009dd00d  JMP 0x009dd03d
009dd00f  CMP dword ptr [ESI + 0x5c4],0x0
009dd016  JZ 0x009dd01d
009dd018  FLD float ptr [ESI + 0x54]
009dd01b  JMP 0x009dd020
009dd01d  FLD float ptr [ESI + 0x50]
009dd020  FSTP float ptr [ESP + 0x10]
009dd024  SUB ESP,0x8
009dd027  FLD float ptr [ESP + 0x3c]
009dd02b  MOV ECX,ESI
009dd02d  FSTP float ptr [ESP + 0x4]
009dd031  FLD float ptr [ESP + 0x18]
009dd035  FSTP float ptr [ESP]
009dd038  CALL 0x009d68d0
009dd03d  MOV EDI,dword ptr [ESI + 0x64]
009dd040  MOV ECX,dword ptr [EDI + 0x24]
009dd043  PUSH ECX
009dd044  LEA ECX,[EDI + 0x14]
009dd047  CALL 0x009bc110
009dd04c  FLDZ
009dd04e  MOV EDX,dword ptr [EAX]
009dd050  FCOMP float ptr [EDX + 0x94]
009dd056  FNSTSW AX
009dd058  TEST AH,0x1
009dd05b  JZ 0x009dd0d2
009dd05d  FLD float ptr [ESI + 0x5e8]
009dd063  FSUB float ptr [ESI + 0x58]
009dd066  FCOMP double ptr [0x00c75240]
009dd06c  FNSTSW AX
009dd06e  TEST AH,0x5
009dd071  JP 0x009dd077
009dd073  FLD1
009dd075  JMP 0x009dd07d
009dd077  FLD float ptr [0x00c75064]
009dd07d  MOV EAX,dword ptr [EDI + 0x24]
009dd080  FSTP float ptr [ESP + 0x10]
009dd084  PUSH EAX
009dd085  LEA ECX,[EDI + 0x14]
009dd088  CALL 0x009bc110
009dd08d  MOV ECX,dword ptr [EAX]
009dd08f  FLD float ptr [ECX + 0x94]
009dd095  FMUL float ptr [ESP + 0x10]
009dd099  FMUL float ptr [ESP + 0x34]
009dd09d  FADD float ptr [ESI + 0x5e8]
009dd0a3  FSTP float ptr [ESP + 0x34]
009dd0a7  FLD float ptr [ESI + 0x5e8]
009dd0ad  FLD float ptr [ESI + 0x58]
009dd0b0  FCOMPP
009dd0b2  FNSTSW AX
009dd0b4  FLD float ptr [ESP + 0x34]
009dd0b8  FLD float ptr [ESI + 0x58]
009dd0bb  TEST AH,0x41
009dd0be  FCOMP
009dd0c0  FNSTSW AX
009dd0c2  JNZ 0x009dd0cb
009dd0c4  TEST AH,0x5
009dd0c7  JP 0x009dd0d5
009dd0c9  JMP 0x009dd0d0
009dd0cb  TEST AH,0x41
009dd0ce  JNZ 0x009dd0d5
009dd0d0  FSTP ST0
009dd0d2  FLD float ptr [ESI + 0x58]
009dd0d5  MOV EDX,dword ptr [ESI + 0x28]
009dd0d8  FSTP float ptr [ESI + 0x5e8]
009dd0de  MOV EAX,dword ptr [EDX + 0x14]
009dd0e1  MOV ECX,dword ptr [EAX + 0x64]
009dd0e4  MOV EDX,dword ptr [ECX]
009dd0e6  MOV EAX,dword ptr [EDX + 0xc0]
009dd0ec  PUSH 0x0
009dd0ee  CALL EAX
009dd0f0  POP EDI
009dd0f1  POP ESI
009dd0f2  POP EBP
009dd0f3  POP EBX
009dd0f4  ADD ESP,0x20
009dd0f7  RET 0x4
