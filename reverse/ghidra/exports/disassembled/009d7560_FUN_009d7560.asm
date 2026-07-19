009d7560  SUB ESP,0x3c
009d7563  PUSH EBX
009d7564  MOV EBX,ECX
009d7566  MOV ECX,dword ptr [EBX + 0x28]
009d7569  PUSH EBP
009d756a  PUSH 0x0
009d756c  ADD ECX,0x34
009d756f  CALL 0x00476310
009d7574  MOV EAX,dword ptr [EAX]
009d7576  MOV EBP,dword ptr [EAX + 0x58]
009d7579  TEST EBP,EBP
009d757b  JZ 0x009d7854
009d7581  MOV ECX,dword ptr [ESP + 0x48]
009d7585  PUSH ESI
009d7586  MOV ESI,dword ptr [EBP + 0x32c]
009d758c  PUSH EDI
009d758d  LEA EDI,[ESI + 0x10]
009d7590  PUSH EDI
009d7591  PUSH ECX
009d7592  LEA ECX,[ESP + 0x3c]
009d7596  CALL 0x00469ff0
009d759b  MOV EDX,dword ptr [ESP + 0x54]
009d759f  PUSH EDI
009d75a0  PUSH EDX
009d75a1  LEA ECX,[ESP + 0x48]
009d75a5  CALL 0x0046a050
009d75aa  MOV EBP,dword ptr [EBP + 0x108]
009d75b0  FLD float ptr [EBP]
009d75b3  MOV ECX,dword ptr [EBX + 0x64]
009d75b6  FLD1
009d75b8  FDIVRP
009d75ba  FSTP float ptr [ESP + 0x54]
009d75be  FLD float ptr [ESP + 0x34]
009d75c2  FLD float ptr [ESP + 0x54]
009d75c6  FLD ST0
009d75c8  FMULP ST2
009d75ca  FXCH
009d75cc  FSTP float ptr [ESP + 0x1c]
009d75d0  FLD float ptr [ESP + 0x38]
009d75d4  FMUL ST1
009d75d6  FSTP float ptr [ESP + 0x20]
009d75da  FMUL float ptr [ESP + 0x3c]
009d75de  FSTP float ptr [ESP + 0x24]
009d75e2  FLD float ptr [ESI + 0x40]
009d75e5  FADD float ptr [ESP + 0x1c]
009d75e9  FSTP float ptr [ESP + 0x28]
009d75ed  FLD float ptr [ESI + 0x44]
009d75f0  FADD float ptr [ESP + 0x20]
009d75f4  FSTP float ptr [ESP + 0x2c]
009d75f8  FLD float ptr [ESI + 0x48]
009d75fb  FADD float ptr [ESP + 0x24]
009d75ff  FSTP float ptr [ESP + 0x30]
009d7603  FLD float ptr [ESI + 0x44]
009d7606  FLD float ptr [ESI + 0x40]
009d7609  FLD float ptr [ESI + 0x48]
009d760c  FLD ST1
009d760e  FMULP ST2
009d7610  FLD ST2
009d7612  FMULP ST3
009d7614  FXCH
009d7616  FADDP ST2,ST0
009d7618  FMUL ST0
009d761a  FADDP
009d761c  FSTP float ptr [ESP + 0x10]
009d7620  FLD float ptr [ESP + 0x2c]
009d7624  FLD float ptr [ESP + 0x28]
009d7628  FLD float ptr [ESP + 0x30]
009d762c  FLD ST1
009d762e  FMULP ST2
009d7630  FLD ST2
009d7632  FMULP ST3
009d7634  FXCH
009d7636  FADDP ST2,ST0
009d7638  FMUL ST0
009d763a  FADDP
009d763c  FSTP float ptr [ESP + 0x54]
009d7640  FLD float ptr [ESP + 0x54]
009d7644  FLD float ptr [ESP + 0x10]
009d7648  FCOMPP
009d764a  FNSTSW AX
009d764c  TEST AH,0x5
009d764f  JP 0x009d7684
009d7651  MOV EAX,dword ptr [ECX + 0x24]
009d7654  PUSH EAX
009d7655  ADD ECX,0x14
009d7658  CALL 0x009bc110
009d765d  FLD float ptr [ESP + 0x54]
009d7661  MOV ECX,dword ptr [EAX]
009d7663  FSUB float ptr [ESP + 0x10]
009d7667  FLD float ptr [ECX + 0x150]
009d766d  FCOMPP
009d766f  FNSTSW AX
009d7671  TEST AH,0x5
009d7674  JP 0x009d7684
009d7676  FLDZ
009d7678  FST float ptr [ESP + 0x30]
009d767c  FST float ptr [ESP + 0x2c]
009d7680  FSTP float ptr [ESP + 0x28]
009d7684  FLD float ptr [ESP + 0x28]
009d7688  PUSH EDI
009d7689  FSTP float ptr [ESI + 0x40]
009d768c  ADD EBP,0x38
009d768f  FLD float ptr [ESP + 0x30]
009d7693  PUSH EBP
009d7694  FSTP float ptr [ESI + 0x44]
009d7697  LEA ECX,[ESP + 0x18]
009d769b  FLD float ptr [ESP + 0x38]
009d769f  FSTP float ptr [ESI + 0x48]
009d76a2  CALL 0x0046a050
009d76a7  FLD float ptr [ESP + 0x40]
009d76ab  LEA EDX,[ESI + 0x7c]
009d76ae  FSUB float ptr [ESP + 0x10]
009d76b2  PUSH EDX
009d76b3  LEA ECX,[ESP + 0x20]
009d76b7  FSTP float ptr [ESP + 0x2c]
009d76bb  FLD float ptr [ESP + 0x48]
009d76bf  FSUB float ptr [ESP + 0x18]
009d76c3  FSTP float ptr [ESP + 0x30]
009d76c7  FLD float ptr [ESP + 0x4c]
009d76cb  FSUB float ptr [ESP + 0x1c]
009d76cf  FSTP float ptr [ESP + 0x34]
009d76d3  FLD float ptr [ESP + 0x30]
009d76d7  FLD ST0
009d76d9  FLD float ptr [ESP + 0x40]
009d76dd  FLD ST0
009d76df  FMULP ST2
009d76e1  FLD float ptr [ESP + 0x34]
009d76e5  FLD ST0
009d76e7  FLD float ptr [ESP + 0x3c]
009d76eb  FLD ST0
009d76ed  FMULP ST2
009d76ef  FXCH ST4
009d76f1  FSUBRP
009d76f3  FSTP float ptr [ESP + 0x20]
009d76f7  FLD float ptr [ESP + 0x38]
009d76fb  FLD ST0
009d76fd  FMULP ST2
009d76ff  FLD float ptr [ESP + 0x2c]
009d7703  FLD ST0
009d7705  FMULP ST4
009d7707  FXCH ST2
009d7709  FSUBRP ST3,ST0
009d770b  FXCH ST2
009d770d  FSTP float ptr [ESP + 0x24]
009d7711  FMULP ST2
009d7713  FMULP ST2
009d7715  FSUBRP
009d7717  FSTP float ptr [ESP + 0x28]
009d771b  CALL 0x0046a170
009d7720  MOV EDI,dword ptr [EBX + 0x64]
009d7723  MOV EAX,dword ptr [EDI + 0x24]
009d7726  PUSH EAX
009d7727  LEA ECX,[EDI + 0x14]
009d772a  CALL 0x009bc110
009d772f  MOV ECX,dword ptr [EAX]
009d7731  FLD float ptr [ECX + 0xec]
009d7737  MOV EDX,dword ptr [EDI + 0x24]
009d773a  FSTP float ptr [ESP + 0x54]
009d773e  PUSH EDX
009d773f  FLD float ptr [ESP + 0x58]
009d7743  LEA ECX,[EDI + 0x14]
009d7746  FLD ST0
009d7748  FMUL float ptr [ESP + 0x20]
009d774c  FSTP float ptr [ESP + 0x20]
009d7750  FLD ST0
009d7752  FMUL float ptr [ESP + 0x24]
009d7756  FSTP float ptr [ESP + 0x24]
009d775a  FMUL float ptr [ESP + 0x28]
009d775e  FSTP float ptr [ESP + 0x28]
009d7762  CALL 0x009bc110
009d7767  MOV EAX,dword ptr [EAX]
009d7769  FLD float ptr [EAX + 0xe8]
009d776f  FMUL float ptr [ESP + 0x20]
009d7773  FSTP float ptr [ESP + 0x20]
009d7777  FLD float ptr [ESI + 0x58]
009d777a  FADD float ptr [ESP + 0x1c]
009d777e  FSTP float ptr [ESI + 0x58]
009d7781  FLD float ptr [ESI + 0x5c]
009d7784  FADD float ptr [ESP + 0x20]
009d7788  FSTP float ptr [ESI + 0x5c]
009d778b  FLD float ptr [ESI + 0x60]
009d778e  FADD float ptr [ESP + 0x24]
009d7792  FSTP float ptr [ESI + 0x60]
009d7795  MOV ECX,dword ptr [EDI + 0x24]
009d7798  FLD float ptr [ESI + 0x5c]
009d779b  PUSH ECX
009d779c  FLD float ptr [ESI + 0x58]
009d779f  LEA ECX,[EDI + 0x14]
009d77a2  FLD float ptr [ESI + 0x60]
009d77a5  FLD ST1
009d77a7  FMULP ST2
009d77a9  FLD ST2
009d77ab  FMULP ST3
009d77ad  FXCH
009d77af  FADDP ST2,ST0
009d77b1  FMUL ST0
009d77b3  FADDP
009d77b5  FSTP float ptr [ESP + 0x14]
009d77b9  CALL 0x009bc110
009d77be  MOV EDX,dword ptr [EAX]
009d77c0  FLD float ptr [EDX + 0x14c]
009d77c6  FSTP float ptr [ESP + 0x54]
009d77ca  FLD float ptr [ESP + 0x54]
009d77ce  FLD ST0
009d77d0  FLD float ptr [ESP + 0x10]
009d77d4  FLD ST1
009d77d6  FMULP ST2
009d77d8  FCOM
009d77da  FNSTSW AX
009d77dc  FSTP ST1
009d77de  TEST AH,0x41
009d77e1  JNZ 0x009d781e
009d77e3  FXCH
009d77e5  FSTP double ptr [ESP + 0x10]
009d77e9  CALL 0x007e8520
009d77ee  FSTP float ptr [ESP + 0x54]
009d77f2  FLD float ptr [ESP + 0x54]
009d77f6  FDIVR double ptr [ESP + 0x10]
009d77fa  FSTP float ptr [ESP + 0x54]
009d77fe  FLD float ptr [ESI + 0x58]
009d7801  FLD float ptr [ESP + 0x54]
009d7805  FLD ST0
009d7807  FMULP ST2
009d7809  FXCH
009d780b  FSTP float ptr [ESI + 0x58]
009d780e  FLD float ptr [ESI + 0x5c]
009d7811  FMUL ST1
009d7813  FSTP float ptr [ESI + 0x5c]
009d7816  FMUL float ptr [ESI + 0x60]
009d7819  FSTP float ptr [ESI + 0x60]
009d781c  JMP 0x009d7822
009d781e  FSTP ST1
009d7820  FSTP ST0
009d7822  MOV EAX,dword ptr [ESP + 0x50]
009d7826  FLD float ptr [EAX]
009d7828  POP EDI
009d7829  FADD float ptr [EBX + 0x824]
009d782f  POP ESI
009d7830  FSTP float ptr [EBX + 0x824]
009d7836  FLD float ptr [EAX + 0x4]
009d7839  FADD float ptr [EBX + 0x828]
009d783f  FSTP float ptr [EBX + 0x828]
009d7845  FLD float ptr [EAX + 0x8]
009d7848  FADD float ptr [EBX + 0x82c]
009d784e  FSTP float ptr [EBX + 0x82c]
009d7854  POP EBP
009d7855  POP EBX
009d7856  ADD ESP,0x3c
009d7859  RET 0x8
