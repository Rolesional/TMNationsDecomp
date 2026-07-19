009da440  SUB ESP,0x38
009da443  PUSH EBX
009da444  PUSH EBP
009da445  PUSH ESI
009da446  PUSH EDI
009da447  MOV EDI,dword ptr [ESP + 0x50]
009da44b  FLD float ptr [EDI + 0xc]
009da44e  LEA EBP,[EDI + 0xc]
009da451  FABS
009da453  MOV EBX,ECX
009da455  FSTP float ptr [ESP + 0x50]
009da459  MOV dword ptr [ESP + 0x10],EBX
009da45d  FLD float ptr [ESP + 0x50]
009da461  FSTP float ptr [ESP + 0x14]
009da465  FLD double ptr [0x00cadad8]
009da46b  CALL 0x007ea240
009da470  FSTP float ptr [ESP + 0x50]
009da474  FLD float ptr [ESP + 0x50]
009da478  MOV ECX,0x1
009da47d  FLD float ptr [ESP + 0x14]
009da481  FCOMPP
009da483  FNSTSW AX
009da485  TEST AH,0x5
009da488  JP 0x009da48e
009da48a  MOV EAX,ECX
009da48c  JMP 0x009da490
009da48e  XOR EAX,EAX
009da490  TEST EAX,EAX
009da492  MOV ESI,dword ptr [ESP + 0x4c]
009da496  MOV dword ptr [ESI + 0x124],EAX
009da49c  JNZ 0x009da4c5
009da49e  MOV dword ptr [EBX + 0x5dc],ECX
009da4a4  FLD float ptr [EDI + 0x18]
009da4a7  FSTP float ptr [ESI + 0x160]
009da4ad  FLD float ptr [EDI + 0x1c]
009da4b0  FSTP float ptr [ESI + 0x164]
009da4b6  FLD float ptr [EDI + 0x20]
009da4b9  FSTP float ptr [ESI + 0x168]
009da4bf  MOV dword ptr [ESI + 0x15c],ECX
009da4c5  CMP dword ptr [ESI + 0x124],0x0
009da4cc  JZ 0x009da50c
009da4ce  ADD dword ptr [ESI + 0x140],ECX
009da4d4  FLD float ptr [ESI + 0x144]
009da4da  FADD float ptr [EBP]
009da4dd  FSTP float ptr [ESI + 0x144]
009da4e3  FLD float ptr [EBP + 0x4]
009da4e6  FADD float ptr [ESI + 0x148]
009da4ec  FSTP float ptr [ESI + 0x148]
009da4f2  FLD float ptr [EBP + 0x8]
009da4f5  FADD float ptr [ESI + 0x14c]
009da4fb  FSTP float ptr [ESI + 0x14c]
009da501  MOV AX,word ptr [EDI + 0x48]
009da505  MOV word ptr [ESI + 0x128],AX
009da50c  MOV ECX,dword ptr [EDI + 0x40]
009da50f  TEST ECX,ECX
009da511  MOV dword ptr [EDI + 0x3c],0x0
009da518  JZ 0x009da57b
009da51a  MOV EDX,dword ptr [ECX]
009da51c  MOV EAX,dword ptr [EDX + 0x7c]
009da51f  CALL EAX
009da521  FLD float ptr [EAX + 0x8]
009da524  FSTP float ptr [ESP + 0x30]
009da528  MOV ECX,dword ptr [ESP + 0x30]
009da52c  FLD float ptr [EAX + 0x14]
009da52f  LEA EBX,[ESI + 0x130]
009da535  FSTP float ptr [ESP + 0x34]
009da539  MOV EDX,dword ptr [ESP + 0x34]
009da53d  FLD float ptr [EAX + 0x20]
009da540  MOV dword ptr [EBX],ECX
009da542  MOV dword ptr [EBX + 0x4],EDX
009da545  FSTP float ptr [ESP + 0x38]
009da549  MOV EAX,dword ptr [ESP + 0x38]
009da54d  MOV EDX,dword ptr [ESP + 0x10]
009da551  MOV dword ptr [EBX + 0x8],EAX
009da554  MOV ECX,dword ptr [EDI + 0x40]
009da557  MOV dword ptr [ESI + 0x13c],ECX
009da55d  MOV ECX,dword ptr [EDX + 0x28]
009da560  PUSH 0x0
009da562  ADD ECX,0x34
009da565  CALL 0x00476310
009da56a  MOV ECX,dword ptr [EAX]
009da56c  MOV EAX,dword ptr [ECX]
009da56e  MOV EDX,dword ptr [EAX + 0x7c]
009da571  CALL EDX
009da573  PUSH EAX
009da574  MOV ECX,EBX
009da576  CALL 0x0047f250
009da57b  FLD float ptr [EDI + 0x18]
009da57e  MOV EAX,dword ptr [ESP + 0x10]
009da582  FSTP float ptr [ESI + 0x108]
009da588  LEA EBX,[EDI + 0x18]
009da58b  FLD float ptr [EBX + 0x4]
009da58e  FSTP float ptr [ESI + 0x10c]
009da594  FLD float ptr [EBX + 0x8]
009da597  FSTP float ptr [ESI + 0x110]
009da59d  MOV EAX,dword ptr [EAX + 0x64]
009da5a0  MOV ECX,dword ptr [EAX + 0x24]
009da5a3  PUSH ECX
009da5a4  LEA ECX,[EAX + 0x14]
009da5a7  CALL 0x009bc110
009da5ac  MOV EDX,dword ptr [EAX]
009da5ae  MOV EAX,dword ptr [EDX + 0x350]
009da5b4  SUB EAX,0x2
009da5b7  JNZ 0x009daa70
009da5bd  FLD float ptr [EDI + 0x30]
009da5c0  MOV dword ptr [ESP + 0x4c],EAX
009da5c4  FLDZ
009da5c6  FMUL ST1
009da5c8  FLD float ptr [EDI + 0x34]
009da5cb  FADDP ST2,ST0
009da5cd  FMUL float ptr [EDI + 0x38]
009da5d0  FADDP
009da5d2  FSTP float ptr [ESP + 0x50]
009da5d6  FLDZ
009da5d8  FCOMP float ptr [ESP + 0x50]
009da5dc  FNSTSW AX
009da5de  TEST AH,0x5
009da5e1  JP 0x009da6bf
009da5e7  MOV EAX,dword ptr [ESP + 0x10]
009da5eb  MOV EAX,dword ptr [EAX + 0x64]
009da5ee  MOV ECX,dword ptr [EAX + 0x24]
009da5f1  PUSH ECX
009da5f2  LEA ECX,[EAX + 0x14]
009da5f5  CALL 0x009bc110
009da5fa  MOV EDX,dword ptr [EAX]
009da5fc  FLD float ptr [EDX + 0x11c]
009da602  FSTP float ptr [ESP + 0x14]
009da606  FLD float ptr [0x00c77ed8]
009da60c  FLD float ptr [ESP + 0x14]
009da610  FCOM
009da612  FNSTSW AX
009da614  FSTP ST1
009da616  TEST AH,0x5
009da619  JP 0x009da623
009da61b  FSTP ST0
009da61d  FLD float ptr [ESP + 0x50]
009da621  JMP 0x009da64c
009da623  FSUBR float ptr [ESI + 0xb4]
009da629  FSTP float ptr [ESP + 0x14]
009da62d  FLD float ptr [ESP + 0x50]
009da631  FLD float ptr [ESP + 0x14]
009da635  FCOM
009da637  FNSTSW AX
009da639  TEST AH,0x41
009da63c  JNZ 0x009da642
009da63e  FSTP ST0
009da640  JMP 0x009da64c
009da642  FSTP ST1
009da644  MOV dword ptr [ESP + 0x4c],0x1
009da64c  FSTP float ptr [ESP + 0x50]
009da650  FLD float ptr [ESI + 0xbc]
009da656  FSTP float ptr [ESP + 0x14]
009da65a  FLD float ptr [ESP + 0x14]
009da65e  FLD float ptr [ESP + 0x50]
009da662  FCOM
009da664  FNSTSW AX
009da666  TEST AH,0x41
009da669  JP 0x009da673
009da66b  FXCH
009da66d  FSTP float ptr [ESP + 0x50]
009da671  JMP 0x009da679
009da673  FSTP ST1
009da675  FST float ptr [ESP + 0x50]
009da679  FLD float ptr [ESP + 0x50]
009da67d  FSTP float ptr [ESI + 0xbc]
009da683  FLD double ptr [0x00c75240]
009da689  FMUL ST1
009da68b  FSTP float ptr [ESP + 0x50]
009da68f  FLD float ptr [ESP + 0x50]
009da693  FST float ptr [ESP + 0x30]
009da697  FSTP float ptr [ESP + 0x38]
009da69b  FSTP float ptr [ESP + 0x34]
009da69f  FLD float ptr [EDI + 0x30]
009da6a2  FSUB float ptr [ESP + 0x30]
009da6a6  FSTP float ptr [EDI + 0x30]
009da6a9  FLD float ptr [EDI + 0x34]
009da6ac  FSUB float ptr [ESP + 0x34]
009da6b0  FSTP float ptr [EDI + 0x34]
009da6b3  FLD float ptr [EDI + 0x38]
009da6b6  FSUB float ptr [ESP + 0x38]
009da6ba  FSTP float ptr [EDI + 0x38]
009da6bd  JMP 0x009da6c7
009da6bf  MOV dword ptr [ESP + 0x4c],0x1
009da6c7  CMP dword ptr [ESI + 0x124],0x0
009da6ce  JZ 0x009da942
009da6d4  MOV EAX,dword ptr [ESP + 0x10]
009da6d8  MOV EAX,dword ptr [EAX + 0x64]
009da6db  MOV ECX,dword ptr [EAX + 0x24]
009da6de  PUSH ECX
009da6df  LEA ECX,[EAX + 0x14]
009da6e2  CALL 0x009bc110
009da6e7  CMP word ptr [EDI + 0x48],0x4
009da6ec  MOV EDX,dword ptr [EAX]
009da6ee  JZ 0x009da6f8
009da6f0  FLD float ptr [EDX + 0x184]
009da6f6  JMP 0x009da6fe
009da6f8  FLD float ptr [EDX + 0x18c]
009da6fe  MOV ECX,dword ptr [EDI + 0x28]
009da701  FCHS
009da703  MOV EAX,dword ptr [EDI + 0x24]
009da706  FSTP float ptr [ESP + 0x50]
009da70a  FLD float ptr [EBP + 0x4]
009da70d  MOV EDX,dword ptr [EDI + 0x2c]
009da710  MOV dword ptr [ESP + 0x34],ECX
009da714  FMUL float ptr [ESP + 0x34]
009da718  FLD float ptr [EBP]
009da71b  MOV dword ptr [ESP + 0x30],EAX
009da71f  FMUL float ptr [ESP + 0x30]
009da723  MOV dword ptr [ESP + 0x38],EDX
009da727  FADDP
009da729  FLD float ptr [EBP + 0x8]
009da72c  FMUL float ptr [ESP + 0x38]
009da730  FADDP
009da732  FSTP float ptr [ESP + 0x14]
009da736  FLDZ
009da738  FLD float ptr [ESP + 0x14]
009da73c  FCOM
009da73e  FNSTSW AX
009da740  TEST AH,0x5
009da743  JP 0x009daa6c
009da749  CMP dword ptr [ESP + 0x4c],0x0
009da74e  FLD float ptr [EBP]
009da751  FMUL ST1
009da753  FSTP float ptr [ESP + 0x24]
009da757  FLD float ptr [EBP + 0x4]
009da75a  FMUL ST1
009da75c  FSTP float ptr [ESP + 0x28]
009da760  FMUL float ptr [EBP + 0x8]
009da763  FSTP float ptr [ESP + 0x2c]
009da767  FLD float ptr [ESP + 0x24]
009da76b  FLDZ
009da76d  FMUL ST1
009da76f  FLD float ptr [ESP + 0x28]
009da773  FADDP ST2,ST0
009da775  FLD float ptr [ESP + 0x2c]
009da779  FMUL ST1
009da77b  FADDP ST2,ST0
009da77d  FXCH
009da77f  FSTP float ptr [ESP + 0x14]
009da783  JNZ 0x009da8bb
009da789  FLD float ptr [ESP + 0x14]
009da78d  FCOM ST2
009da78f  FNSTSW AX
009da791  TEST AH,0x5
009da794  JP 0x009da8b9
009da79a  FLD ST0
009da79c  FMULP ST2
009da79e  FXCH
009da7a0  FSTP float ptr [ESP + 0x4c]
009da7a4  FLD float ptr [ESP + 0x4c]
009da7a8  FST float ptr [ESP + 0x30]
009da7ac  FSTP float ptr [ESP + 0x38]
009da7b0  FSTP float ptr [ESP + 0x34]
009da7b4  FLD float ptr [EDI + 0x24]
009da7b7  FSTP float ptr [ESP + 0x18]
009da7bb  FLD float ptr [EDI + 0x28]
009da7be  FSTP float ptr [ESP + 0x1c]
009da7c2  FLD float ptr [EDI + 0x2c]
009da7c5  FSTP float ptr [ESP + 0x20]
009da7c9  FLD float ptr [ESP + 0x18]
009da7cd  FSUB float ptr [ESP + 0x30]
009da7d1  FSTP float ptr [ESP + 0x18]
009da7d5  FLD float ptr [ESP + 0x1c]
009da7d9  FSUB float ptr [ESP + 0x34]
009da7dd  FSTP float ptr [ESP + 0x1c]
009da7e1  FLD float ptr [ESP + 0x20]
009da7e5  FSUB float ptr [ESP + 0x38]
009da7e9  FSTP float ptr [ESP + 0x20]
009da7ed  FLD float ptr [EBP + 0x4]
009da7f0  FMUL float ptr [ESP + 0x1c]
009da7f4  FLD float ptr [EBP]
009da7f7  FMUL float ptr [ESP + 0x18]
009da7fb  FADDP
009da7fd  FLD float ptr [EBP + 0x8]
009da800  FMUL float ptr [ESP + 0x20]
009da804  FADDP
009da806  FSTP float ptr [ESP + 0x4c]
009da80a  FCOMP float ptr [ESP + 0x4c]
009da80e  FNSTSW AX
009da810  TEST AH,0x41
009da813  JNZ 0x009daa70
009da819  FLD float ptr [ESI + 0x64]
009da81c  MOV EDI,dword ptr [ESP + 0x10]
009da820  MOV EAX,dword ptr [EDI + 0x28]
009da823  FSTP float ptr [ESP + 0x30]
009da827  FLD float ptr [ESI + 0x68]
009da82a  MOV EAX,dword ptr [EAX + 0x14]
009da82d  FSTP float ptr [ESP + 0x34]
009da831  ADD EAX,0x18
009da834  FLD float ptr [ESI + 0x6c]
009da837  LEA ECX,[ESP + 0x24]
009da83b  FSTP float ptr [ESP + 0x38]
009da83f  PUSH ECX
009da840  FLD float ptr [ESP + 0x34]
009da844  LEA EDX,[ESP + 0x40]
009da848  FSUB float ptr [EAX + 0x38]
009da84b  PUSH EDX
009da84c  PUSH EBP
009da84d  LEA ECX,[ESP + 0x24]
009da851  FSTP float ptr [ESP + 0x48]
009da855  PUSH ECX
009da856  FLD float ptr [ESP + 0x44]
009da85a  PUSH ECX
009da85b  FSUB float ptr [EAX + 0x3c]
009da85e  LEA EDX,[EAX + 0x4]
009da861  FSTP float ptr [ESP + 0x54]
009da865  FLD float ptr [ESP + 0x4c]
009da869  FSUB float ptr [EAX + 0x40]
009da86c  FSTP float ptr [ESP + 0x58]
009da870  FLD float ptr [ESP + 0x64]
009da874  FSTP float ptr [ESP]
009da877  PUSH EDX
009da878  FLD float ptr [EAX]
009da87a  PUSH ECX
009da87b  FSTP float ptr [ESP]
009da87e  CALL 0x009d6190
009da883  FLD float ptr [ESP + 0x40]
009da887  ADD ESP,0x1c
009da88a  FSTP float ptr [ESP + 0x24]
009da88e  LEA EAX,[ESP + 0x30]
009da892  FLD float ptr [ESP + 0x28]
009da896  PUSH EAX
009da897  FSTP float ptr [ESP + 0x2c]
009da89b  LEA ECX,[ESP + 0x28]
009da89f  FLD float ptr [ESP + 0x30]
009da8a3  PUSH ECX
009da8a4  MOV ECX,EDI
009da8a6  FSTP float ptr [ESP + 0x34]
009da8aa  CALL 0x009d7560
009da8af  POP EDI
009da8b0  POP ESI
009da8b1  POP EBP
009da8b2  POP EBX
009da8b3  ADD ESP,0x38
009da8b6  RET 0x8
009da8b9  FSTP ST0
009da8bb  MOV ESI,dword ptr [ESP + 0x10]
009da8bf  FSTP ST0
009da8c1  MOV EDX,dword ptr [ESI + 0x28]
009da8c4  FSTP ST0
009da8c6  MOV EAX,dword ptr [EDX + 0x14]
009da8c9  FLD float ptr [EBX]
009da8cb  FSUB float ptr [EAX + 0x50]
009da8ce  ADD EAX,0x18
009da8d1  LEA ECX,[ESP + 0x24]
009da8d5  PUSH ECX
009da8d6  FSTP float ptr [ESP + 0x40]
009da8da  LEA EDX,[ESP + 0x40]
009da8de  FLD float ptr [EBX + 0x4]
009da8e1  PUSH EDX
009da8e2  FSUB float ptr [EAX + 0x3c]
009da8e5  PUSH EBP
009da8e6  LEA ECX,[ESP + 0x3c]
009da8ea  PUSH ECX
009da8eb  FSTP float ptr [ESP + 0x50]
009da8ef  PUSH ECX
009da8f0  FLD float ptr [EBX + 0x8]
009da8f3  LEA EDX,[EAX + 0x4]
009da8f6  FSUB float ptr [EAX + 0x40]
009da8f9  FSTP float ptr [ESP + 0x58]
009da8fd  FLD float ptr [ESP + 0x64]
009da901  FSTP float ptr [ESP]
009da904  PUSH EDX
009da905  FLD float ptr [EAX]
009da907  PUSH ECX
009da908  FSTP float ptr [ESP]
009da90b  CALL 0x009d6190
009da910  FLD float ptr [ESP + 0x40]
009da914  ADD ESP,0x1c
009da917  FSTP float ptr [ESP + 0x24]
009da91b  PUSH EBX
009da91c  FLD float ptr [ESP + 0x2c]
009da920  LEA EAX,[ESP + 0x28]
009da924  FSTP float ptr [ESP + 0x2c]
009da928  PUSH EAX
009da929  FLD float ptr [ESP + 0x34]
009da92d  MOV ECX,ESI
009da92f  FSTP float ptr [ESP + 0x34]
009da933  CALL 0x009d7560
009da938  POP EDI
009da939  POP ESI
009da93a  POP EBP
009da93b  POP EBX
009da93c  ADD ESP,0x38
009da93f  RET 0x8
009da942  CMP word ptr [EDI + 0x48],0x4
009da947  JZ 0x009da966
009da949  MOV ESI,dword ptr [ESP + 0x10]
009da94d  MOV EAX,dword ptr [ESI + 0x64]
009da950  MOV ECX,dword ptr [EAX + 0x24]
009da953  PUSH ECX
009da954  LEA ECX,[EAX + 0x14]
009da957  CALL 0x009bc110
009da95c  MOV EDX,dword ptr [EAX]
009da95e  FLD float ptr [EDX + 0x17c]
009da964  JMP 0x009da985
009da966  MOV EAX,dword ptr [ESP + 0x10]
009da96a  MOV EAX,dword ptr [EAX + 0x64]
009da96d  MOV ECX,dword ptr [EAX + 0x24]
009da970  PUSH ECX
009da971  LEA ECX,[EAX + 0x14]
009da974  CALL 0x009bc110
009da979  MOV EDX,dword ptr [EAX]
009da97b  FLD float ptr [EDX + 0x178]
009da981  MOV ESI,dword ptr [ESP + 0x10]
009da985  FCHS
009da987  ADD EDI,0x24
009da98a  FSTP float ptr [ESP + 0x50]
009da98e  FLD float ptr [EBP + 0x4]
009da991  FMUL float ptr [EDI + 0x4]
009da994  FLD float ptr [EBP]
009da997  FMUL float ptr [EDI]
009da999  FADDP
009da99b  FLD float ptr [EBP + 0x8]
009da99e  FMUL float ptr [EDI + 0x8]
009da9a1  FADDP
009da9a3  FSTP float ptr [ESP + 0x4c]
009da9a7  FLD float ptr [ESP + 0x4c]
009da9ab  FCOMP float ptr [0x00d9e780]
009da9b1  FNSTSW AX
009da9b3  TEST AH,0x5
009da9b6  JP 0x009daa70
009da9bc  MOV EAX,dword ptr [ESI + 0x28]
009da9bf  FLD float ptr [EBX]
009da9c1  MOV EAX,dword ptr [EAX + 0x14]
009da9c4  FSTP float ptr [ESP + 0x30]
009da9c8  FLD float ptr [EBX + 0x8]
009da9cb  MOV ECX,dword ptr [EBP]
009da9ce  MOV EDX,dword ptr [EBP + 0x4]
009da9d1  FSTP float ptr [ESP + 0x38]
009da9d5  FLD float ptr [EAX + 0x54]
009da9d8  ADD EAX,0x18
009da9db  FSTP float ptr [ESP + 0x34]
009da9df  MOV dword ptr [ESP + 0x18],ECX
009da9e3  FLD float ptr [ESP + 0x30]
009da9e7  MOV ECX,dword ptr [EBP + 0x8]
009da9ea  FSUB float ptr [EAX + 0x38]
009da9ed  MOV dword ptr [ESP + 0x1c],EDX
009da9f1  MOV dword ptr [ESP + 0x20],ECX
009da9f5  LEA EDX,[ESP + 0x24]
009da9f9  FSTP float ptr [ESP + 0x3c]
009da9fd  PUSH EDX
009da9fe  FLD float ptr [ESP + 0x38]
009daa02  LEA ECX,[ESP + 0x40]
009daa06  FSUB float ptr [EAX + 0x3c]
009daa09  PUSH ECX
009daa0a  LEA EDX,[ESP + 0x20]
009daa0e  PUSH EDX
009daa0f  FSTP float ptr [ESP + 0x4c]
009daa13  PUSH EDI
009daa14  FLD float ptr [ESP + 0x48]
009daa18  PUSH ECX
009daa19  FSUB float ptr [EAX + 0x40]
009daa1c  LEA ECX,[EAX + 0x4]
009daa1f  FSTP float ptr [ESP + 0x58]
009daa23  FLD float ptr [ESP + 0x64]
009daa27  FSTP float ptr [ESP]
009daa2a  PUSH ECX
009daa2b  FLD float ptr [EAX]
009daa2d  PUSH ECX
009daa2e  FSTP float ptr [ESP]
009daa31  CALL 0x009d6190
009daa36  FLD float ptr [ESP + 0x40]
009daa3a  ADD ESP,0x1c
009daa3d  FSTP float ptr [ESP + 0x24]
009daa41  LEA EDX,[ESP + 0x30]
009daa45  FLD float ptr [ESP + 0x28]
009daa49  PUSH EDX
009daa4a  FSTP float ptr [ESP + 0x2c]
009daa4e  LEA EAX,[ESP + 0x28]
009daa52  FLD float ptr [ESP + 0x30]
009daa56  PUSH EAX
009daa57  MOV ECX,ESI
009daa59  FSTP float ptr [ESP + 0x34]
009daa5d  CALL 0x009d7560
009daa62  POP EDI
009daa63  POP ESI
009daa64  POP EBP
009daa65  POP EBX
009daa66  ADD ESP,0x38
009daa69  RET 0x8
009daa6c  FSTP ST0
009daa6e  FSTP ST0
009daa70  POP EDI
009daa71  POP ESI
009daa72  POP EBP
009daa73  POP EBX
009daa74  ADD ESP,0x38
009daa77  RET 0x8
