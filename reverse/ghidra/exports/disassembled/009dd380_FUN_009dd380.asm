009dd380  SUB ESP,0x130
009dd386  PUSH EBX
009dd387  PUSH EBP
009dd388  PUSH ESI
009dd389  MOV EBX,ECX
009dd38b  MOV ECX,dword ptr [EBX + 0x28]
009dd38e  PUSH EDI
009dd38f  PUSH 0x0
009dd391  ADD ECX,0x34
009dd394  CALL 0x00476310
009dd399  MOV ECX,dword ptr [EAX]
009dd39b  MOV EAX,dword ptr [ECX]
009dd39d  MOV EDX,dword ptr [EAX + 0x7c]
009dd3a0  CALL EDX
009dd3a2  LEA EDX,[EBX + 0x6a4]
009dd3a8  MOV ESI,EAX
009dd3aa  MOV EAX,dword ptr [ESP + 0x148]
009dd3b1  MOV EDI,EDX
009dd3b3  MOV ECX,0xc
009dd3b8  MOVSD.REP ES:EDI,ESI
009dd3ba  FLD float ptr [EDX + 0x10]
009dd3bd  FSTP float ptr [ESP + 0x108]
009dd3c4  PUSH EAX
009dd3c5  MOV ECX,EBX
009dd3c7  CALL 0x009dbc80
009dd3cc  LEA EBP,[EBX + 0x2e8]
009dd3d2  MOV ECX,EBP
009dd3d4  MOV dword ptr [ESP + 0x10],EAX
009dd3d8  MOV dword ptr [EBX + 0x5e4],EAX
009dd3de  MOV dword ptr [ESP + 0x24],0x1
009dd3e6  CALL 0x009d5a40
009dd3eb  MOV EDI,EAX
009dd3ed  XOR ESI,ESI
009dd3ef  TEST EDI,EDI
009dd3f1  JBE 0x009dd41d
009dd3f3  PUSH ESI
009dd3f4  MOV ECX,EBP
009dd3f6  CALL 0x006f1300
009dd3fb  CMP dword ptr [EAX + 0x124],0x0
009dd402  JZ 0x009dd40e
009dd404  CMP word ptr [EAX + 0x128],0x6
009dd40c  JZ 0x009dd416
009dd40e  MOV dword ptr [ESP + 0x24],0x0
009dd416  ADD ESI,0x1
009dd419  CMP ESI,EDI
009dd41b  JC 0x009dd3f3
009dd41d  MOV EAX,dword ptr [EBX + 0x28]
009dd420  MOV EAX,dword ptr [EAX + 0x14]
009dd423  MOV ECX,dword ptr [EBX + 0x628]
009dd429  XOR EDX,EDX
009dd42b  ADD EAX,0x50
009dd42e  CMP dword ptr [EBX + 0x69c],0x2
009dd435  MOV dword ptr [ESP + 0x38],ECX
009dd439  MOV dword ptr [ESP + 0x4c],EDX
009dd43d  MOV dword ptr [ESP + 0x60],EAX
009dd441  JNZ 0x009ddd22
009dd447  FLD float ptr [EBX + 0x50]
009dd44a  FLD double ptr [0x00ca2d98]
009dd450  FCOM
009dd452  FNSTSW AX
009dd454  FSTP ST1
009dd456  TEST AH,0x41
009dd459  JZ 0x009ddcdc
009dd45f  FLD float ptr [EBX + 0x54]
009dd462  FCOMPP
009dd464  FNSTSW AX
009dd466  TEST AH,0x5
009dd469  JNP 0x009ddcde
009dd46f  FLD float ptr [ESP + 0x15c]
009dd476  FLD ST0
009dd478  FABS
009dd47a  FSTP float ptr [ESP + 0x30]
009dd47e  FLD float ptr [ESP + 0x30]
009dd482  FCOMP float ptr [0x00d1453c]
009dd488  FNSTSW AX
009dd48a  TEST AH,0x5
009dd48d  JNP 0x009ddcdc
009dd493  FSTP float ptr [ESP + 0x30]
009dd497  MOV ECX,dword ptr [ESP + 0x30]
009dd49b  NOT ECX
009dd49d  TEST ECX,ECX
009dd49f  JNS 0x009dd4a5
009dd4a1  FLD1
009dd4a3  JMP 0x009dd4ab
009dd4a5  FLD float ptr [0x00c75064]
009dd4ab  FSTP float ptr [ESP + 0x20]
009dd4af  FLD float ptr [ESP + 0x20]
009dd4b3  FLD float ptr [EBX + 0x708]
009dd4b9  FUCOMPP
009dd4bb  FNSTSW AX
009dd4bd  TEST AH,0x44
009dd4c0  JP 0x009ddcde
009dd4c6  CMP dword ptr [EBX + 0x5dc],EDX
009dd4cc  JNZ 0x009ddcde
009dd4d2  CMP dword ptr [EBX + 0x5d8],EDX
009dd4d8  JG 0x009ddcde
009dd4de  CMP dword ptr [ESP + 0x160],EDX
009dd4e5  JZ 0x009ddcde
009dd4eb  CMP dword ptr [ESP + 0x10],EDX
009dd4ef  JNZ 0x009ddcde
009dd4f5  CMP dword ptr [EBX + 0x60c],EDX
009dd4fb  JNZ 0x009ddcde
009dd501  FLDZ
009dd503  MOV ECX,EBP
009dd505  FST float ptr [ESP + 0x48]
009dd509  FST float ptr [ESP + 0x44]
009dd50d  FSTP float ptr [ESP + 0x40]
009dd511  CALL 0x009d5a40
009dd516  MOV EDI,EAX
009dd518  XOR ESI,ESI
009dd51a  TEST EDI,EDI
009dd51c  JBE 0x009dd558
009dd51e  MOV EDI,EDI
009dd520  PUSH ESI
009dd521  MOV ECX,EBP
009dd523  CALL 0x006f1300
009dd528  FLD float ptr [EAX + 0x144]
009dd52e  ADD EAX,0x144
009dd533  FADD float ptr [ESP + 0x40]
009dd537  ADD ESI,0x1
009dd53a  CMP ESI,EDI
009dd53c  FSTP float ptr [ESP + 0x40]
009dd540  FLD float ptr [EAX + 0x4]
009dd543  FADD float ptr [ESP + 0x44]
009dd547  FSTP float ptr [ESP + 0x44]
009dd54b  FLD float ptr [EAX + 0x8]
009dd54e  FADD float ptr [ESP + 0x48]
009dd552  FSTP float ptr [ESP + 0x48]
009dd556  JC 0x009dd520
009dd558  FLD float ptr [ESP + 0x44]
009dd55c  FLD float ptr [ESP + 0x40]
009dd560  FLD float ptr [ESP + 0x48]
009dd564  FLD ST2
009dd566  FMULP ST3
009dd568  FLD ST1
009dd56a  FMULP ST2
009dd56c  FXCH ST2
009dd56e  FADDP
009dd570  FLD ST1
009dd572  FMULP ST2
009dd574  FADDP
009dd576  FSTP float ptr [ESP + 0x10]
009dd57a  FLD float ptr [ESP + 0x10]
009dd57e  FLD float ptr [0x00e89bb8]
009dd584  FCOMP
009dd586  FNSTSW AX
009dd588  TEST AH,0x5
009dd58b  JP 0x009dd5c0
009dd58d  CALL 0x007e8520
009dd592  FSTP float ptr [ESP + 0x10]
009dd596  FLD float ptr [ESP + 0x10]
009dd59a  FLD1
009dd59c  FDIVRP
009dd59e  FSTP float ptr [ESP + 0x10]
009dd5a2  FLD float ptr [ESP + 0x10]
009dd5a6  FLD ST0
009dd5a8  FMUL float ptr [ESP + 0x40]
009dd5ac  FSTP float ptr [ESP + 0x40]
009dd5b0  FLD float ptr [ESP + 0x44]
009dd5b4  FMUL ST1
009dd5b6  FSTP float ptr [ESP + 0x44]
009dd5ba  FMUL float ptr [ESP + 0x48]
009dd5be  JMP 0x009dd5ce
009dd5c0  FSTP ST0
009dd5c2  FLDZ
009dd5c4  FST float ptr [ESP + 0x40]
009dd5c8  FLD1
009dd5ca  FSTP float ptr [ESP + 0x44]
009dd5ce  LEA ESI,[EBX + 0x6a4]
009dd5d4  FSTP float ptr [ESP + 0x48]
009dd5d8  PUSH ESI
009dd5d9  LEA EDX,[ESP + 0x44]
009dd5dd  PUSH EDX
009dd5de  LEA ECX,[ESP + 0x10c]
009dd5e5  CALL 0x00469ff0
009dd5ea  LEA EAX,[EBX + 0x6fc]
009dd5f0  PUSH EAX
009dd5f1  LEA ECX,[ESP + 0x108]
009dd5f8  PUSH ECX
009dd5f9  CALL 0x00473610
009dd5fe  FABS
009dd600  MOV EAX,dword ptr [EBX + 0x64]
009dd603  FSTP float ptr [ESP + 0x18]
009dd607  MOV EDX,dword ptr [EAX + 0x24]
009dd60a  FLD float ptr [ESP + 0x18]
009dd60e  ADD ESP,0x8
009dd611  FSTP float ptr [ESP + 0x10]
009dd615  PUSH EDX
009dd616  LEA ECX,[EAX + 0x14]
009dd619  CALL 0x009bc110
009dd61e  FLD float ptr [ESP + 0x10]
009dd622  MOV EAX,dword ptr [EAX]
009dd624  FLD float ptr [EAX + 0x28c]
009dd62a  FCOMPP
009dd62c  FNSTSW AX
009dd62e  TEST AH,0x5
009dd631  JP 0x009dd642
009dd633  MOV dword ptr [EBX + 0x69c],0x0
009dd63d  JMP 0x009ddce4
009dd642  PUSH ESI
009dd643  LEA ECX,[ESP + 0x114]
009dd64a  CALL 0x0046f210
009dd64f  LEA ECX,[ESP + 0x110]
009dd656  PUSH ECX
009dd657  LEA EDX,[EBX + 0x6e0]
009dd65d  PUSH EDX
009dd65e  LEA ECX,[ESP + 0x5c]
009dd662  CALL 0x0046a050
009dd667  FLD float ptr [ESP + 0x54]
009dd66b  MOV EAX,dword ptr [ESP + 0x60]
009dd66f  FSUB float ptr [EAX]
009dd671  FSTP float ptr [ESP + 0x80]
009dd678  FLD float ptr [ESP + 0x58]
009dd67c  FSUB float ptr [EAX + 0x4]
009dd67f  FSTP float ptr [ESP + 0x84]
009dd686  FLD float ptr [ESP + 0x5c]
009dd68a  FSUB float ptr [EAX + 0x8]
009dd68d  FSTP float ptr [ESP + 0x88]
009dd694  FLD float ptr [ESP + 0x84]
009dd69b  FLD float ptr [ESP + 0x80]
009dd6a2  FLD float ptr [ESP + 0x88]
009dd6a9  FLD ST1
009dd6ab  FMULP ST2
009dd6ad  FLD ST2
009dd6af  FMULP ST3
009dd6b1  FXCH
009dd6b3  FADDP ST2,ST0
009dd6b5  FMUL ST0
009dd6b7  FADDP
009dd6b9  FSTP float ptr [ESP + 0x20]
009dd6bd  FLD float ptr [ESP + 0x20]
009dd6c1  CALL 0x007e8520
009dd6c6  FSTP float ptr [ESP + 0x10]
009dd6ca  FLD float ptr [ESP + 0x10]
009dd6ce  FSTP float ptr [ESP + 0x28]
009dd6d2  FLD float ptr [ESP + 0x80]
009dd6d9  FSTP float ptr [ESP + 0x68]
009dd6dd  FLD float ptr [ESP + 0x84]
009dd6e4  FSTP float ptr [ESP + 0x6c]
009dd6e8  FLD float ptr [ESP + 0x88]
009dd6ef  FSTP float ptr [ESP + 0x70]
009dd6f3  FLD float ptr [ESP + 0x20]
009dd6f7  FSTP float ptr [ESP + 0x10]
009dd6fb  FLD float ptr [ESP + 0x10]
009dd6ff  FLD float ptr [0x00e89bb8]
009dd705  FCOMP
009dd707  FNSTSW AX
009dd709  TEST AH,0x5
009dd70c  JP 0x009dd74e
009dd70e  CALL 0x007e8520
009dd713  FSTP float ptr [ESP + 0x10]
009dd717  FLD float ptr [ESP + 0x10]
009dd71b  FLD1
009dd71d  FDIVRP
009dd71f  FSTP float ptr [ESP + 0x10]
009dd723  FLD float ptr [ESP + 0x10]
009dd727  FLD ST0
009dd729  FMUL float ptr [ESP + 0x80]
009dd730  FSTP float ptr [ESP + 0x68]
009dd734  FLD float ptr [ESP + 0x84]
009dd73b  FMUL ST1
009dd73d  FSTP float ptr [ESP + 0x6c]
009dd741  FMUL float ptr [ESP + 0x88]
009dd748  FSTP float ptr [ESP + 0x70]
009dd74c  JMP 0x009dd750
009dd74e  FSTP ST0
009dd750  FLD float ptr [ESP + 0x70]
009dd754  MOV EDI,dword ptr [ESP + 0x154]
009dd75b  FLD ST0
009dd75d  MOV ESI,dword ptr [EBX + 0x64]
009dd760  FLD float ptr [ESP + 0x44]
009dd764  MOV EAX,dword ptr [ESI + 0x24]
009dd767  FLD ST0
009dd769  PUSH EAX
009dd76a  FMULP ST2
009dd76c  LEA ECX,[ESI + 0x14]
009dd76f  FLD float ptr [ESP + 0x70]
009dd773  FLD ST0
009dd775  FLD float ptr [ESP + 0x4c]
009dd779  FLD ST0
009dd77b  FMULP ST2
009dd77d  FXCH ST4
009dd77f  FSUBRP
009dd781  FSTP float ptr [ESP + 0x78]
009dd785  FLD float ptr [ESP + 0x6c]
009dd789  FLD ST0
009dd78b  FMULP ST4
009dd78d  FLD float ptr [ESP + 0x44]
009dd791  FLD ST0
009dd793  FMULP ST6
009dd795  FXCH ST4
009dd797  FSUBRP ST5,ST0
009dd799  FXCH ST4
009dd79b  FSTP float ptr [ESP + 0x7c]
009dd79f  FMULP ST2
009dd7a1  FMULP ST2
009dd7a3  FSUBRP
009dd7a5  FSTP float ptr [ESP + 0x80]
009dd7ac  FLD float ptr [ESP + 0x7c]
009dd7b0  FMUL float ptr [EDI + 0x4]
009dd7b3  FLD float ptr [ESP + 0x78]
009dd7b7  FMUL float ptr [EDI]
009dd7b9  FADDP
009dd7bb  FLD float ptr [ESP + 0x80]
009dd7c2  FMUL float ptr [EDI + 0x8]
009dd7c5  FADDP
009dd7c7  FSTP float ptr [ESP + 0x1c]
009dd7cb  CALL 0x009bc110
009dd7d0  FLD float ptr [ESP + 0x18]
009dd7d4  MOV ECX,dword ptr [EAX]
009dd7d6  FABS
009dd7d8  FSTP float ptr [ESP + 0x10]
009dd7dc  FLD float ptr [ESP + 0x10]
009dd7e0  FLD float ptr [ECX + 0x284]
009dd7e6  FCOMPP
009dd7e8  FNSTSW AX
009dd7ea  TEST AH,0x5
009dd7ed  JP 0x009dd7f9
009dd7ef  MOV dword ptr [EBX + 0x69c],0x0
009dd7f9  FLD float ptr [ESP + 0x28]
009dd7fd  FLD float ptr [EBX + 0x6ec]
009dd803  FCOMPP
009dd805  FNSTSW AX
009dd807  TEST AH,0x5
009dd80a  JNP 0x009dd83c
009dd80c  MOV EDX,dword ptr [ESI + 0x24]
009dd80f  PUSH EDX
009dd810  LEA ECX,[ESI + 0x14]
009dd813  CALL 0x009bc110
009dd818  FLD float ptr [ESP + 0x28]
009dd81c  MOV EAX,dword ptr [EAX]
009dd81e  FLD float ptr [EAX + 0x280]
009dd824  FCOMPP
009dd826  FNSTSW AX
009dd828  TEST AH,0x41
009dd82b  JZ 0x009dd83c
009dd82d  MOV dword ptr [EBX + 0x69c],0x0
009dd837  JMP 0x009dd901
009dd83c  MOV ECX,dword ptr [ESI + 0x24]
009dd83f  PUSH ECX
009dd840  LEA ECX,[ESI + 0x14]
009dd843  CALL 0x009bc110
009dd848  MOV EDX,dword ptr [ESI + 0x24]
009dd84b  PUSH EDX
009dd84c  LEA ECX,[ESI + 0x14]
009dd84f  MOV dword ptr [ESP + 0x34],EAX
009dd853  CALL 0x009bc110
009dd858  FLD float ptr [ESP + 0x28]
009dd85c  FSUB float ptr [EBX + 0x6f0]
009dd862  MOV ECX,dword ptr [ESP + 0x30]
009dd866  MOV EDX,dword ptr [ECX]
009dd868  MOV EAX,dword ptr [EAX]
009dd86a  FSTP float ptr [ESP + 0x10]
009dd86e  FLD float ptr [ESP + 0x10]
009dd872  FMUL float ptr [EDX + 0x278]
009dd878  FLD float ptr [ESP + 0x6c]
009dd87c  FMUL float ptr [EDI + 0x4]
009dd87f  FLD float ptr [ESP + 0x68]
009dd883  FMUL float ptr [EDI]
009dd885  FADDP
009dd887  FLD float ptr [ESP + 0x70]
009dd88b  FMUL float ptr [EDI + 0x8]
009dd88e  FADDP
009dd890  FSTP float ptr [ESP + 0x10]
009dd894  FLD float ptr [ESP + 0x10]
009dd898  FMUL float ptr [EAX + 0x27c]
009dd89e  FSUBP
009dd8a0  FSTP float ptr [ESP + 0x10]
009dd8a4  FLD float ptr [ESP + 0x18]
009dd8a8  FSTP double ptr [ESP + 0x38]
009dd8ac  FLD float ptr [ESP + 0x10]
009dd8b0  CALL 0x007eac5c
009dd8b5  FSTP float ptr [ESP + 0x10]
009dd8b9  FLD float ptr [ESP + 0x10]
009dd8bd  LEA ECX,[ESP + 0x54]
009dd8c1  FLD double ptr [ESP + 0x38]
009dd8c5  PUSH ECX
009dd8c6  FMUL ST0
009dd8c8  MOV ECX,EBX
009dd8ca  FSTP float ptr [ESP + 0x14]
009dd8ce  FLD float ptr [ESP + 0x14]
009dd8d2  FDIV float ptr [ESP + 0x2c]
009dd8d6  FMULP
009dd8d8  FSTP float ptr [ESP + 0x14]
009dd8dc  FLD float ptr [ESP + 0x14]
009dd8e0  FLD ST0
009dd8e2  FMUL float ptr [ESP + 0x6c]
009dd8e6  FSTP float ptr [ESP + 0x58]
009dd8ea  FLD ST0
009dd8ec  FMUL float ptr [ESP + 0x70]
009dd8f0  FSTP float ptr [ESP + 0x5c]
009dd8f4  FMUL float ptr [ESP + 0x74]
009dd8f8  FSTP float ptr [ESP + 0x60]
009dd8fc  CALL 0x009d74e0
009dd901  MOV ECX,EBP
009dd903  CALL 0x009d5a40
009dd908  XOR ESI,ESI
009dd90a  TEST EAX,EAX
009dd90c  MOV dword ptr [ESP + 0x10],EAX
009dd910  JBE 0x009dd93f
009dd912  PUSH ESI
009dd913  MOV ECX,EBP
009dd915  CALL 0x006f1300
009dd91a  MOV EDX,dword ptr [ESP + 0x148]
009dd921  MOV EDI,EAX
009dd923  PUSH EDX
009dd924  PUSH EDI
009dd925  MOV ECX,EBX
009dd927  CALL 0x009daa80
009dd92c  ADD ESI,0x1
009dd92f  CMP ESI,dword ptr [ESP + 0x10]
009dd933  MOV dword ptr [EDI + 0x12c],0x0
009dd93d  JC 0x009dd912
009dd93f  MOV EAX,dword ptr [EBX + 0x64]
009dd942  MOV ECX,dword ptr [EAX + 0x24]
009dd945  PUSH ECX
009dd946  LEA ECX,[EAX + 0x14]
009dd949  CALL 0x009bc110
009dd94e  FLD float ptr [ESP + 0x15c]
009dd955  MOV ESI,dword ptr [EAX]
009dd957  PUSH ECX
009dd958  FSTP float ptr [ESP]
009dd95b  CALL 0x00660550
009dd960  FSTP float ptr [ESP + 0x24]
009dd964  MOV ECX,ESI
009dd966  FLD float ptr [ESP + 0x2c]
009dd96a  FSTP float ptr [ESP]
009dd96d  CALL 0x00a28c80
009dd972  FLD float ptr [ESP + 0x20]
009dd976  MOV EAX,dword ptr [EBX + 0x64]
009dd979  MOV EDX,dword ptr [EAX + 0x24]
009dd97c  FCHS
009dd97e  FMULP
009dd980  PUSH EDX
009dd981  LEA ECX,[EAX + 0x14]
009dd984  FSUB float ptr [ESP + 0x1c]
009dd988  FSTP float ptr [ESP + 0x14]
009dd98c  CALL 0x009bc110
009dd991  MOV EAX,dword ptr [EAX]
009dd993  FLD float ptr [EAX + 0x264]
009dd999  LEA ECX,[ESP + 0x90]
009dd9a0  FMUL float ptr [ESP + 0x10]
009dd9a4  PUSH ECX
009dd9a5  MOV ECX,EBX
009dd9a7  FSTP float ptr [ESP + 0x14]
009dd9ab  FLD float ptr [ESP + 0x14]
009dd9af  FLD ST0
009dd9b1  FMUL float ptr [ESP + 0x78]
009dd9b5  FSTP float ptr [ESP + 0x94]
009dd9bc  FLD ST0
009dd9be  FMUL float ptr [ESP + 0x7c]
009dd9c2  FSTP float ptr [ESP + 0x98]
009dd9c9  FMUL float ptr [ESP + 0x80]
009dd9d0  FSTP float ptr [ESP + 0x9c]
009dd9d7  CALL 0x009d74e0
009dd9dc  FLDZ
009dd9de  LEA EDX,[ESP + 0x68]
009dd9e2  FST float ptr [ESP + 0x54]
009dd9e6  PUSH EDX
009dd9e7  FSTP float ptr [ESP + 0x5c]
009dd9eb  LEA EAX,[ESP + 0x58]
009dd9ef  FLD1
009dd9f1  PUSH EAX
009dd9f2  FSTP float ptr [ESP + 0x64]
009dd9f6  CALL 0x00473610
009dd9fb  FDIV double ptr [0x00c80bb8]
009dda01  ADD ESP,0x4
009dda04  FSTP float ptr [ESP + 0x50]
009dda08  FLD float ptr [ESP + 0x50]
009dda0c  FSTP float ptr [ESP]
009dda0f  CALL 0x008b2400
009dda14  ADD ESP,0x4
009dda17  TEST EAX,EAX
009dda19  JZ 0x009ddc00
009dda1f  FLD float ptr [ESP + 0x20]
009dda23  MOV ESI,dword ptr [EBX + 0x64]
009dda26  FMUL float ptr [ESP + 0x4c]
009dda2a  MOV ECX,dword ptr [ESI + 0x24]
009dda2d  PUSH ECX
009dda2e  LEA ECX,[ESI + 0x14]
009dda31  FMUL double ptr [0x00c80bb8]
009dda37  FSTP double ptr [ESP + 0x3c]
009dda3b  CALL 0x009bc110
009dda40  MOV EDX,dword ptr [EAX]
009dda42  FLD float ptr [EDX + 0x294]
009dda48  FCHS
009dda4a  FCOMP double ptr [ESP + 0x38]
009dda4e  FNSTSW AX
009dda50  TEST AH,0x41
009dda53  JZ 0x009ddc00
009dda59  MOV EAX,dword ptr [ESI + 0x24]
009dda5c  PUSH EAX
009dda5d  LEA ECX,[ESI + 0x14]
009dda60  CALL 0x009bc110
009dda65  MOV ECX,dword ptr [EAX]
009dda67  FLD float ptr [ECX + 0x290]
009dda6d  FCOMP double ptr [ESP + 0x38]
009dda71  FNSTSW AX
009dda73  TEST AH,0x5
009dda76  JNP 0x009ddc00
009dda7c  FLDZ
009dda7e  MOV EDI,dword ptr [ESP + 0x158]
009dda85  FLD float ptr [ESP + 0x4c]
009dda89  FCOM
009dda8b  FNSTSW AX
009dda8d  FXCH
009dda8f  FCOMP float ptr [EDI + 0x4]
009dda92  TEST AH,0x41
009dda95  FNSTSW AX
009dda97  JNZ 0x009ddaf5
009dda99  TEST AH,0x41
009dda9c  JNZ 0x009ddabe
009dda9e  MOV EDX,dword ptr [ESI + 0x24]
009ddaa1  FSTP ST0
009ddaa3  PUSH EDX
009ddaa4  LEA ECX,[ESI + 0x14]
009ddaa7  CALL 0x009bc110
009ddaac  MOV EAX,dword ptr [EAX]
009ddaae  FLD float ptr [EAX + 0x26c]
009ddab4  FCHS
009ddab6  FMUL float ptr [EDI + 0x4]
009ddab9  JMP 0x009ddb49
009ddabe  MOV ECX,dword ptr [ESI + 0x24]
009ddac1  FSUB double ptr [0x00c75250]
009ddac7  PUSH ECX
009ddac8  LEA ECX,[ESI + 0x14]
009ddacb  FSTP float ptr [ESP + 0x14]
009ddacf  CALL 0x009bc110
009ddad4  FLD float ptr [ESP + 0x10]
009ddad8  MOV EDX,dword ptr [EAX]
009ddada  FLD float ptr [EDX + 0x270]
009ddae0  FCHS
009ddae2  FMUL float ptr [EDI + 0x4]
009ddae5  FLD ST1
009ddae7  FMULP ST2
009ddae9  FXCH
009ddaeb  FSTP float ptr [ESP + 0x10]
009ddaef  FMUL float ptr [ESP + 0x10]
009ddaf3  JMP 0x009ddb49
009ddaf5  TEST AH,0x5
009ddaf8  LEA ECX,[ESI + 0x14]
009ddafb  JP 0x009ddb17
009ddafd  MOV EAX,dword ptr [ESI + 0x24]
009ddb00  FSTP ST0
009ddb02  PUSH EAX
009ddb03  CALL 0x009bc110
009ddb08  MOV ECX,dword ptr [EAX]
009ddb0a  FLD float ptr [ECX + 0x26c]
009ddb10  FCHS
009ddb12  FMUL float ptr [EDI + 0x4]
009ddb15  JMP 0x009ddb49
009ddb17  FADD double ptr [0x00c75250]
009ddb1d  MOV EDX,dword ptr [ESI + 0x24]
009ddb20  PUSH EDX
009ddb21  FSTP float ptr [ESP + 0x14]
009ddb25  CALL 0x009bc110
009ddb2a  FLD float ptr [ESP + 0x10]
009ddb2e  MOV EAX,dword ptr [EAX]
009ddb30  FLD float ptr [EAX + 0x270]
009ddb36  FCHS
009ddb38  FMUL float ptr [EDI + 0x4]
009ddb3b  FLD ST1
009ddb3d  FMULP ST2
009ddb3f  FXCH
009ddb41  FSTP float ptr [ESP + 0x10]
009ddb45  FMUL float ptr [ESP + 0x10]
009ddb49  FSTP float ptr [ESP + 0x2c]
009ddb4d  FLD float ptr [ESP + 0x18]
009ddb51  FDIV float ptr [ESP + 0x28]
009ddb55  FSTP float ptr [ESP + 0x18]
009ddb59  FLDZ
009ddb5b  FST float ptr [ESP + 0x28]
009ddb5f  FCOM float ptr [ESP + 0x4c]
009ddb63  FNSTSW AX
009ddb65  FCOMP float ptr [ESP + 0x18]
009ddb69  TEST AH,0x5
009ddb6c  FNSTSW AX
009ddb6e  JP 0x009ddb8b
009ddb70  TEST AH,0x41
009ddb73  JNZ 0x009ddbae
009ddb75  MOV ECX,dword ptr [ESI + 0x24]
009ddb78  PUSH ECX
009ddb79  LEA ECX,[ESI + 0x14]
009ddb7c  CALL 0x009bc110
009ddb81  MOV EDX,dword ptr [EAX]
009ddb83  FLD float ptr [EDX + 0x274]
009ddb89  JMP 0x009ddba4
009ddb8b  TEST AH,0x5
009ddb8e  JP 0x009ddbae
009ddb90  MOV EAX,dword ptr [ESI + 0x24]
009ddb93  PUSH EAX
009ddb94  LEA ECX,[ESI + 0x14]
009ddb97  CALL 0x009bc110
009ddb9c  MOV ECX,dword ptr [EAX]
009ddb9e  FLD float ptr [ECX + 0x274]
009ddba4  FCHS
009ddba6  FMUL float ptr [ESP + 0x18]
009ddbaa  FSTP float ptr [ESP + 0x28]
009ddbae  MOV EDX,dword ptr [ESI + 0x24]
009ddbb1  PUSH EDX
009ddbb2  LEA ECX,[ESI + 0x14]
009ddbb5  CALL 0x009bc110
009ddbba  MOV EAX,dword ptr [EAX]
009ddbbc  FLD float ptr [EAX + 0x268]
009ddbc2  LEA ECX,[ESP + 0x54]
009ddbc6  FMUL float ptr [ESP + 0x4c]
009ddbca  PUSH ECX
009ddbcb  MOV ECX,EBX
009ddbcd  FADD float ptr [ESP + 0x30]
009ddbd1  FADD float ptr [ESP + 0x2c]
009ddbd5  FSTP float ptr [ESP + 0x14]
009ddbd9  FLD float ptr [ESP + 0x14]
009ddbdd  FLD double ptr [0x00c75240]
009ddbe3  FMUL ST1
009ddbe5  FSTP float ptr [ESP + 0x14]
009ddbe9  FLD float ptr [ESP + 0x14]
009ddbed  FST float ptr [ESP + 0x58]
009ddbf1  FSTP float ptr [ESP + 0x60]
009ddbf5  FSTP float ptr [ESP + 0x5c]
009ddbf9  CALL 0x009d7530
009ddbfe  JMP 0x009ddc0a
009ddc00  MOV dword ptr [EBX + 0x69c],0x0
009ddc0a  MOV ESI,dword ptr [ESP + 0x154]
009ddc11  FLD float ptr [ESI]
009ddc13  MOV EAX,dword ptr [EBX + 0x64]
009ddc16  MOV EDX,dword ptr [EAX + 0x24]
009ddc19  FABS
009ddc1b  FSTP float ptr [ESP + 0x10]
009ddc1f  PUSH EDX
009ddc20  FLD float ptr [ESP + 0x14]
009ddc24  LEA ECX,[EAX + 0x14]
009ddc27  FSTP float ptr [ESP + 0x14]
009ddc2b  CALL 0x009bc110
009ddc30  MOV EAX,dword ptr [EAX]
009ddc32  FLD float ptr [ESP + 0x10]
009ddc36  PUSH ECX
009ddc37  MOV ECX,EAX
009ddc39  FSTP float ptr [ESP]
009ddc3c  CALL 0x00a28d00
009ddc41  FSTP float ptr [ESP + 0x10]
009ddc45  FLD float ptr [ESI]
009ddc47  PUSH ECX
009ddc48  FSTP float ptr [ESP]
009ddc4b  CALL 0x00660550
009ddc50  FCHS
009ddc52  ADD ESP,0x4
009ddc55  FMUL float ptr [ESP + 0x10]
009ddc59  LEA EAX,[ESP + 0xb0]
009ddc60  PUSH EAX
009ddc61  MOV ECX,EBX
009ddc63  FSTP float ptr [ESP + 0x14]
009ddc67  FLD float ptr [ESP + 0x14]
009ddc6b  FLD double ptr [0x00c75240]
009ddc71  FMUL ST1
009ddc73  FSTP float ptr [ESP + 0x14]
009ddc77  FLD float ptr [ESP + 0x14]
009ddc7b  FST float ptr [ESP + 0xb4]
009ddc82  FSTP float ptr [ESP + 0xb8]
009ddc89  FSTP float ptr [ESP + 0xbc]
009ddc90  CALL 0x009d7530
009ddc95  MOV EAX,dword ptr [EBX + 0x64]
009ddc98  MOV ECX,dword ptr [EAX + 0x24]
009ddc9b  PUSH ECX
009ddc9c  LEA ECX,[EAX + 0x14]
009ddc9f  CALL 0x009bc110
009ddca4  FLD float ptr [ESI + 0x8]
009ddca7  MOV EAX,dword ptr [EAX]
009ddca9  PUSH ECX
009ddcaa  MOV ECX,EAX
009ddcac  FSTP float ptr [ESP]
009ddcaf  CALL 0x00a28cc0
009ddcb4  FSTP float ptr [ESP + 0xd4]
009ddcbb  FLDZ
009ddcbd  LEA EDX,[ESP + 0xd4]
009ddcc4  FST float ptr [ESP + 0xd8]
009ddccb  PUSH EDX
009ddccc  MOV ECX,EBX
009ddcce  FSTP float ptr [ESP + 0xe0]
009ddcd5  CALL 0x009d7530
009ddcda  JMP 0x009ddce4
009ddcdc  FSTP ST0
009ddcde  MOV dword ptr [EBX + 0x69c],EDX
009ddce4  CMP dword ptr [EBX + 0x69c],0x2
009ddceb  JZ 0x009dfcf9
009ddcf1  MOV dword ptr [EBX + 0x69c],0x1
009ddcfb  MOV ECX,dword ptr [0x00ebd820]
009ddd01  MOV EAX,dword ptr [ECX + 0x14]
009ddd04  TEST EAX,EAX
009ddd06  JNZ 0x009ddd0e
009ddd08  LEA EAX,[ECX + 0xa0]
009ddd0e  MOV ECX,EAX
009ddd10  CALL 0x00428400
009ddd15  MOV EAX,dword ptr [EAX]
009ddd17  MOV dword ptr [EBX + 0x6f4],EAX
009ddd1d  JMP 0x009dfcf9
009ddd22  MOV EAX,[0x00ebd820]
009ddd27  MOV ECX,dword ptr [EAX + 0x14]
009ddd2a  CMP ECX,EDX
009ddd2c  JNZ 0x009ddd34
009ddd2e  LEA ECX,[EAX + 0xa0]
009ddd34  CALL 0x00428400
009ddd39  CMP dword ptr [EBX + 0x69c],0x1
009ddd40  MOV ESI,dword ptr [EAX]
009ddd42  MOV dword ptr [ESP + 0x2c],ESI
009ddd46  JNZ 0x009ddd89
009ddd48  MOV EDI,dword ptr [EBX + 0x6f4]
009ddd4e  CMP ESI,EDI
009ddd50  JC 0x009ddd79
009ddd52  MOV EAX,dword ptr [EBX + 0x64]
009ddd55  MOV ECX,dword ptr [EAX + 0x24]
009ddd58  PUSH ECX
009ddd59  LEA ECX,[EAX + 0x14]
009ddd5c  CALL 0x009bc110
009ddd61  MOV EAX,dword ptr [EAX]
009ddd63  MOV EDX,ESI
009ddd65  SUB EDX,EDI
009ddd67  CMP EDX,dword ptr [EAX + 0x298]
009ddd6d  JNC 0x009ddd79
009ddd6f  MOV dword ptr [ESP + 0x4c],0x1
009ddd77  JMP 0x009ddd89
009ddd79  MOV dword ptr [EBX + 0x6f8],ESI
009ddd7f  MOV dword ptr [EBX + 0x69c],0x3
009ddd89  CMP dword ptr [EBX + 0x69c],0x3
009ddd90  JNZ 0x009dddef
009ddd92  MOV EDI,dword ptr [EBX + 0x6f8]
009ddd98  CMP ESI,EDI
009ddd9a  JC 0x009ddde1
009ddd9c  MOV EAX,dword ptr [EBX + 0x64]
009ddd9f  MOV ECX,dword ptr [EAX + 0x24]
009ddda2  PUSH ECX
009ddda3  LEA ECX,[EAX + 0x14]
009ddda6  CALL 0x009bc110
009dddab  MOV EDX,dword ptr [EAX]
009dddad  SUB ESI,EDI
009dddaf  CMP ESI,dword ptr [EDX + 0x2a8]
009dddb5  JNC 0x009ddde1
009dddb7  MOV ECX,EBP
009dddb9  CALL 0x009d5a40
009dddbe  MOV EDI,EAX
009dddc0  XOR ESI,ESI
009dddc2  TEST EDI,EDI
009dddc4  JBE 0x009dddef
009dddc6  PUSH ESI
009dddc7  MOV ECX,EBP
009dddc9  CALL 0x006f1300
009dddce  ADD ESI,0x1
009dddd1  CMP ESI,EDI
009dddd3  MOV dword ptr [EAX + 0x12c],0x1
009ddddd  JC 0x009dddc6
009ddddf  JMP 0x009dddef
009ddde1  XOR EAX,EAX
009ddde3  MOV dword ptr [EBX + 0x69c],EAX
009ddde9  MOV dword ptr [EBX + 0x6a0],EAX
009dddef  MOV ECX,EBP
009dddf1  CALL 0x009d5a40
009dddf6  TEST EAX,EAX
009dddf8  MOV dword ptr [ESP + 0x14],EAX
009dddfc  MOV dword ptr [ESP + 0x50],0x0
009dde04  JBE 0x009de8ba
009dde0a  LEA EBX,[EBX]
009dde10  MOV EAX,dword ptr [ESP + 0x50]
009dde14  PUSH EAX
009dde15  LEA ECX,[EBX + 0x2e8]
009dde1b  CALL 0x006f1300
009dde20  MOV ECX,dword ptr [ESP + 0x148]
009dde27  PUSH ECX
009dde28  MOV ESI,EAX
009dde2a  PUSH ESI
009dde2b  MOV ECX,EBX
009dde2d  CALL 0x009daa80
009dde32  MOVZX EDX,word ptr [ESI + 0x128]
009dde39  PUSH EDX
009dde3a  LEA ECX,[EBX + 0x6c]
009dde3d  CALL 0x0042dbc0
009dde42  MOV EAX,dword ptr [EAX]
009dde44  MOV ECX,dword ptr [EBX + 0x68]
009dde47  PUSH EAX
009dde48  ADD ECX,0x14
009dde4b  CALL 0x00425f00
009dde50  MOV EBP,dword ptr [EAX]
009dde52  ADD EBP,0x14
009dde55  CMP dword ptr [ESI + 0x124],0x0
009dde5c  JZ 0x009de8a5
009dde62  MOV EDI,dword ptr [EBX + 0x64]
009dde65  MOV ECX,dword ptr [EDI + 0x24]
009dde68  PUSH ECX
009dde69  LEA ECX,[EDI + 0x14]
009dde6c  CALL 0x009bc110
009dde71  FLDZ
009dde73  MOV EDX,dword ptr [EAX]
009dde75  FCOM float ptr [EDX + 0xa4]
009dde7b  FNSTSW AX
009dde7d  TEST AH,0x41
009dde80  JP 0x009de8a3
009dde86  FLD float ptr [ESI + 0x108]
009dde8c  MOV EAX,dword ptr [ESP + 0x60]
009dde90  FSTP float ptr [ESP + 0xc8]
009dde97  FLD float ptr [ESI + 0x10c]
009dde9d  FSTP float ptr [ESP + 0xcc]
009ddea4  FLD float ptr [ESI + 0x110]
009ddeaa  FSTP float ptr [ESP + 0xd0]
009ddeb1  FLD float ptr [ESP + 0xc8]
009ddeb8  FSUB float ptr [EAX]
009ddeba  FSTP float ptr [ESP + 0xc8]
009ddec1  FLD float ptr [ESP + 0xcc]
009ddec8  FSUB float ptr [EAX + 0x4]
009ddecb  FSTP float ptr [ESP + 0xcc]
009dded2  FLD float ptr [ESP + 0xd0]
009dded9  FSUB float ptr [EAX + 0x8]
009ddedc  FSTP float ptr [ESP + 0xd0]
009ddee3  FLD float ptr [ESI + 0x14c]
009ddee9  FLDZ
009ddeeb  FMUL ST1
009ddeed  FLD float ptr [ESI + 0x148]
009ddef3  FSUB ST0,ST2
009ddef5  FSTP float ptr [ESP + 0x40]
009ddef9  FLD float ptr [ESI + 0x144]
009ddeff  FSUBP ST2,ST0
009ddf01  FXCH
009ddf03  FSTP float ptr [ESP + 0x44]
009ddf07  FLD float ptr [ESI + 0x144]
009ddf0d  FMUL ST1
009ddf0f  FLD float ptr [ESI + 0x148]
009ddf15  FMULP ST2
009ddf17  FSUBRP
009ddf19  FSTP float ptr [ESP + 0x48]
009ddf1d  FLD float ptr [ESP + 0x40]
009ddf21  FLD float ptr [ESP + 0x44]
009ddf25  FLD float ptr [ESP + 0x48]
009ddf29  FLD ST1
009ddf2b  FMULP ST2
009ddf2d  FLD ST2
009ddf2f  FMULP ST3
009ddf31  FXCH
009ddf33  FADDP ST2,ST0
009ddf35  FMUL ST0
009ddf37  FADDP
009ddf39  FSTP float ptr [ESP + 0x30]
009ddf3d  FLD float ptr [ESP + 0x30]
009ddf41  FLD float ptr [0x00e89bb8]
009ddf47  FCOMP
009ddf49  FNSTSW AX
009ddf4b  TEST AH,0x5
009ddf4e  JP 0x009ddf85
009ddf50  FSTP ST1
009ddf52  CALL 0x007e8520
009ddf57  FSTP float ptr [ESP + 0x30]
009ddf5b  FLD float ptr [ESP + 0x30]
009ddf5f  FLD1
009ddf61  FDIVRP
009ddf63  FSTP float ptr [ESP + 0x30]
009ddf67  FLD float ptr [ESP + 0x30]
009ddf6b  FLD ST0
009ddf6d  FMUL float ptr [ESP + 0x40]
009ddf71  FSTP float ptr [ESP + 0x40]
009ddf75  FLD ST0
009ddf77  FMUL float ptr [ESP + 0x44]
009ddf7b  FSTP float ptr [ESP + 0x44]
009ddf7f  FMUL float ptr [ESP + 0x48]
009ddf83  JMP 0x009ddf91
009ddf85  FSTP ST0
009ddf87  FLD1
009ddf89  FSTP float ptr [ESP + 0x40]
009ddf8d  FST float ptr [ESP + 0x44]
009ddf91  CMP dword ptr [ESI + 0x4],0x0
009ddf95  FSTP float ptr [ESP + 0x48]
009ddf99  JZ 0x009de059
009ddf9f  FLD float ptr [ESP + 0x15c]
009ddfa6  CALL 0x007ea110
009ddfab  FSTP float ptr [ESP + 0x30]
009ddfaf  FLD float ptr [ESP + 0x30]
009ddfb3  FSTP float ptr [ESP + 0x30]
009ddfb7  FLD float ptr [ESP + 0x30]
009ddfbb  FLD ST0
009ddfbd  FMUL float ptr [ESP + 0x40]
009ddfc1  FSTP float ptr [ESP + 0x90]
009ddfc8  FLD ST0
009ddfca  FMUL float ptr [ESP + 0x44]
009ddfce  FSTP float ptr [ESP + 0x94]
009ddfd5  FMUL float ptr [ESP + 0x48]
009ddfd9  FSTP float ptr [ESP + 0x98]
009ddfe0  FLD float ptr [ESP + 0x15c]
009ddfe7  CALL 0x007ea240
009ddfec  FSTP float ptr [ESP + 0x30]
009ddff0  FLD float ptr [ESP + 0x30]
009ddff4  FCHS
009ddff6  FSTP float ptr [ESP + 0x30]
009ddffa  FLD float ptr [ESP + 0x30]
009ddffe  FLD double ptr [0x00c75240]
009de004  FMUL ST1
009de006  FSTP float ptr [ESP + 0x30]
009de00a  FLD float ptr [ESP + 0x30]
009de00e  FST float ptr [ESP + 0xb0]
009de015  FSTP float ptr [ESP + 0xb4]
009de01c  FSTP float ptr [ESP + 0xb8]
009de023  FLD float ptr [ESP + 0x90]
009de02a  FADD float ptr [ESP + 0xb0]
009de031  FSTP float ptr [ESP + 0x40]
009de035  FLD float ptr [ESP + 0xb4]
009de03c  FADD float ptr [ESP + 0x94]
009de043  FSTP float ptr [ESP + 0x44]
009de047  FLD float ptr [ESP + 0xb8]
009de04e  FADD float ptr [ESP + 0x98]
009de055  FSTP float ptr [ESP + 0x48]
009de059  MOV EAX,dword ptr [ESP + 0x154]
009de060  FLD float ptr [EAX]
009de062  LEA ECX,[EDI + 0x14]
009de065  FSTP float ptr [ESP + 0x54]
009de069  FLD float ptr [EAX + 0x4]
009de06c  FSTP float ptr [ESP + 0x58]
009de070  FLD float ptr [EAX + 0x8]
009de073  MOV EAX,dword ptr [EDI + 0x24]
009de076  FSTP float ptr [ESP + 0x5c]
009de07a  PUSH EAX
009de07b  FLD float ptr [ESP + 0x58]
009de07f  FMUL float ptr [ESP + 0x44]
009de083  FLD float ptr [ESP + 0x5c]
009de087  FMUL float ptr [ESP + 0x48]
009de08b  FADDP
009de08d  FLD float ptr [ESP + 0x60]
009de091  FMUL float ptr [ESP + 0x4c]
009de095  FADDP
009de097  FSTP float ptr [ESP + 0x20]
009de09b  CALL 0x009bc110
009de0a0  MOV ECX,dword ptr [EAX]
009de0a2  FLD float ptr [ECX + 0x228]
009de0a8  MOV EDX,dword ptr [EDI + 0x24]
009de0ab  FCHS
009de0ad  PUSH EDX
009de0ae  FSTP float ptr [ESP + 0x34]
009de0b2  LEA ECX,[EDI + 0x14]
009de0b5  FLD float ptr [EBX + 0x6d4]
009de0bb  FLD float ptr [ESP + 0x34]
009de0bf  FLD ST0
009de0c1  FMULP ST2
009de0c3  FXCH
009de0c5  FSTP float ptr [ESP + 0xc0]
009de0cc  FLD float ptr [EBX + 0x6d8]
009de0d2  FMUL ST1
009de0d4  FSTP float ptr [ESP + 0xc4]
009de0db  FMUL float ptr [EBX + 0x6dc]
009de0e1  FSTP float ptr [ESP + 0xc8]
009de0e8  FLD float ptr [ESP + 0xc0]
009de0ef  FSTP double ptr [ESP + 0xac]
009de0f6  FLD float ptr [ESP + 0xc4]
009de0fd  FSTP double ptr [ESP + 0x6c]
009de101  FLD float ptr [ESP + 0xc8]
009de108  FSTP double ptr [ESP + 0x34]
009de10c  CALL 0x009bc110
009de111  FLD double ptr [ESP + 0x68]
009de115  MOV dword ptr [ESP + 0x8c],EAX
009de11c  FMUL ST0
009de11e  FLD double ptr [ESP + 0xa8]
009de125  FMUL ST0
009de127  FADDP
009de129  FLD double ptr [ESP + 0x30]
009de12d  FMUL ST0
009de12f  FADDP
009de131  FSTP float ptr [ESP + 0x30]
009de135  FLD float ptr [ESP + 0x30]
009de139  CALL 0x007e8520
009de13e  FSTP float ptr [ESP + 0x30]
009de142  MOV EAX,dword ptr [ESP + 0x8c]
009de149  FLD float ptr [ESP + 0x30]
009de14d  MOV ECX,dword ptr [EAX]
009de14f  FLD float ptr [ECX + 0x234]
009de155  FCOMPP
009de157  FNSTSW AX
009de159  TEST AH,0x41
009de15c  JNZ 0x009de175
009de15e  FLDZ
009de160  FST float ptr [ESP + 0xc4]
009de167  FST float ptr [ESP + 0xc0]
009de16e  FSTP float ptr [ESP + 0xbc]
009de175  FLD float ptr [ESP + 0xc4]
009de17c  LEA ECX,[EDI + 0x14]
009de17f  FLD ST0
009de181  FLD float ptr [ESP + 0xcc]
009de188  FLD ST0
009de18a  FMULP ST2
009de18c  FLD float ptr [ESP + 0xc0]
009de193  FLD ST0
009de195  FLD float ptr [ESP + 0xd0]
009de19c  FLD ST0
009de19e  FMULP ST2
009de1a0  FXCH ST4
009de1a2  FSUBRP
009de1a4  FSTP float ptr [ESP + 0x80]
009de1ab  FLD float ptr [ESP + 0xbc]
009de1b2  FLD ST0
009de1b4  FMULP ST4
009de1b6  FLD float ptr [ESP + 0xc8]
009de1bd  FLD ST0
009de1bf  FMULP ST6
009de1c1  FXCH ST4
009de1c3  FSUBRP ST5,ST0
009de1c5  FXCH ST4
009de1c7  FSTP float ptr [ESP + 0x84]
009de1ce  FMULP ST2
009de1d0  FMULP ST2
009de1d2  FSUBRP
009de1d4  FSTP float ptr [ESP + 0x88]
009de1db  FLD float ptr [ESP + 0x80]
009de1e2  FLD double ptr [0x00c80bc8]
009de1e8  FMUL ST1
009de1ea  FXCH
009de1ec  FSTP float ptr [ESP + 0x80]
009de1f3  FLD float ptr [ESP + 0x84]
009de1fa  FMUL ST1
009de1fc  FSTP float ptr [ESP + 0x84]
009de203  FMUL float ptr [ESP + 0x88]
009de20a  FSTP float ptr [ESP + 0x88]
009de211  MOV EDX,dword ptr [EDI + 0x24]
009de214  PUSH EDX
009de215  CALL 0x009bc110
009de21a  MOV EAX,dword ptr [EAX]
009de21c  FLD float ptr [EAX + 0x23c]
009de222  MOV ECX,dword ptr [EDI + 0x24]
009de225  FMUL float ptr [ESP + 0x88]
009de22c  PUSH ECX
009de22d  LEA ECX,[EDI + 0x14]
009de230  FSTP float ptr [ESP + 0x34]
009de234  CALL 0x009bc110
009de239  MOV EDX,dword ptr [EAX]
009de23b  FLD float ptr [EDX + 0x238]
009de241  LEA EAX,[ESP + 0x80]
009de248  FMUL float ptr [ESP + 0x80]
009de24f  PUSH EAX
009de250  MOV ECX,EBX
009de252  FSTP float ptr [ESP + 0x84]
009de259  FLDZ
009de25b  FSTP float ptr [ESP + 0x88]
009de262  FLD float ptr [ESP + 0x34]
009de266  FSTP float ptr [ESP + 0x8c]
009de26d  CALL 0x009d7530
009de272  CMP dword ptr [EBX + 0x69c],0x1
009de279  JNZ 0x009de2c7
009de27b  MOV EAX,dword ptr [EBX + 0x64]
009de27e  MOV ECX,dword ptr [EAX + 0x24]
009de281  PUSH ECX
009de282  LEA ECX,[EAX + 0x14]
009de285  CALL 0x009bc110
009de28a  MOV EDX,dword ptr [ESP + 0x154]
009de291  MOV EAX,dword ptr [EAX]
009de293  FLD float ptr [EDX + 0x8]
009de296  PUSH ECX
009de297  MOV ECX,EAX
009de299  FSTP float ptr [ESP]
009de29c  CALL 0x00a28cc0
009de2a1  FSTP float ptr [ESP + 0xe0]
009de2a8  FLDZ
009de2aa  LEA EAX,[ESP + 0xe0]
009de2b1  FST float ptr [ESP + 0xe4]
009de2b8  PUSH EAX
009de2b9  MOV ECX,EBX
009de2bb  FSTP float ptr [ESP + 0xec]
009de2c2  CALL 0x009d7530
009de2c7  CMP dword ptr [EBX + 0x69c],0x1
009de2ce  FLD1
009de2d0  FSTP float ptr [ESP + 0x18]
009de2d4  JNZ 0x009de368
009de2da  MOV EDI,dword ptr [EBX + 0x64]
009de2dd  MOV ECX,dword ptr [EDI + 0x24]
009de2e0  PUSH ECX
009de2e1  LEA ECX,[EDI + 0x14]
009de2e4  CALL 0x009bc110
009de2e9  MOV EDX,dword ptr [EDI + 0x24]
009de2ec  PUSH EDX
009de2ed  LEA ECX,[EDI + 0x14]
009de2f0  MOV dword ptr [ESP + 0x34],EAX
009de2f4  CALL 0x009bc110
009de2f9  MOV EDI,EAX
009de2fb  MOV EAX,dword ptr [ESP + 0x2c]
009de2ff  SUB EAX,dword ptr [EBX + 0x6f4]
009de305  TEST EAX,EAX
009de307  MOV dword ptr [ESP + 0x68],EAX
009de30b  FILD dword ptr [ESP + 0x68]
009de30f  JGE 0x009de317
009de311  FADD float ptr [0x00d9e7b8]
009de317  MOV ECX,dword ptr [ESP + 0x30]
009de31b  FMUL double ptr [0x00c80bb8]
009de321  MOV EDX,dword ptr [ECX]
009de323  MOV EAX,dword ptr [EDX + 0x298]
009de329  ADD EAX,EAX
009de32b  TEST EAX,EAX
009de32d  MOV dword ptr [ESP + 0x68],EAX
009de331  FILD dword ptr [ESP + 0x68]
009de335  JGE 0x009de33d
009de337  FADD float ptr [0x00d9e7b8]
009de33d  FDIVP
009de33f  FSTP float ptr [ESP + 0x30]
009de343  FLD float ptr [ESP + 0x30]
009de347  CALL 0x007ea110
009de34c  FSTP float ptr [ESP + 0x30]
009de350  MOV ECX,dword ptr [EDI]
009de352  FLD float ptr [ECX + 0x2a0]
009de358  FLD1
009de35a  FSUB ST1,ST0
009de35c  FXCH
009de35e  FMUL float ptr [ESP + 0x30]
009de362  FADDP
009de364  FSTP float ptr [ESP + 0x18]
009de368  MOV EAX,dword ptr [EBX + 0x64]
009de36b  MOV EDX,dword ptr [EAX + 0x24]
009de36e  PUSH EDX
009de36f  LEA ECX,[EAX + 0x14]
009de372  CALL 0x009bc110
009de377  FLD float ptr [ESI + 0xb4]
009de37d  MOV EAX,dword ptr [EAX]
009de37f  PUSH ECX
009de380  MOV ECX,EAX
009de382  FSTP float ptr [ESP]
009de385  CALL 0x00a28b90
009de38a  FSTP float ptr [ESP + 0x30]
009de38e  MOV EDI,dword ptr [ESI + 0x12c]
009de394  TEST EDI,EDI
009de396  JZ 0x009de3b1
009de398  MOV EAX,dword ptr [EBX + 0x64]
009de39b  MOV ECX,dword ptr [EAX + 0x24]
009de39e  PUSH ECX
009de39f  LEA ECX,[EAX + 0x14]
009de3a2  CALL 0x009bc110
009de3a7  MOV EDX,dword ptr [EAX]
009de3a9  FLD float ptr [EDX + 0xb0]
009de3af  JMP 0x009de3b3
009de3b1  FLD1
009de3b3  TEST EDI,EDI
009de3b5  FSTP float ptr [ESP + 0x20]
009de3b9  JZ 0x009de3e4
009de3bb  FLD float ptr [EBX + 0x54]
009de3be  FCOMP double ptr [0x00ca2d98]
009de3c4  FNSTSW AX
009de3c6  TEST AH,0x41
009de3c9  JNZ 0x009de3e4
009de3cb  MOV EAX,dword ptr [EBX + 0x64]
009de3ce  MOV ECX,dword ptr [EAX + 0x24]
009de3d1  PUSH ECX
009de3d2  LEA ECX,[EAX + 0x14]
009de3d5  CALL 0x009bc110
009de3da  MOV EDX,dword ptr [EAX]
009de3dc  FLD float ptr [EDX + 0x244]
009de3e2  JMP 0x009de3e6
009de3e4  FLD1
009de3e6  MOV EAX,dword ptr [EBX + 0x64]
009de3e9  FSTP float ptr [ESP + 0x64]
009de3ed  MOV ECX,dword ptr [EAX + 0x24]
009de3f0  PUSH ECX
009de3f1  LEA ECX,[EAX + 0x14]
009de3f4  CALL 0x009bc110
009de3f9  MOV EDX,dword ptr [ESP + 0x154]
009de400  MOV EAX,dword ptr [EAX]
009de402  FLD float ptr [EDX + 0x8]
009de405  PUSH ECX
009de406  MOV ECX,EAX
009de408  FSTP float ptr [ESP]
009de40b  CALL 0x00a287d0
009de410  FLD float ptr [EBP + 0xc]
009de413  FMUL float ptr [ESP + 0x14c]
009de41a  MOV EDI,dword ptr [EBX + 0x64]
009de41d  MOV EAX,dword ptr [EDI + 0x24]
009de420  PUSH EAX
009de421  FMULP
009de423  LEA ECX,[EDI + 0x14]
009de426  FMUL float ptr [ESP + 0x24]
009de42a  FMUL float ptr [ESP + 0x68]
009de42e  FMUL float ptr [ESP + 0x34]
009de432  FSTP float ptr [ESP + 0x2c]
009de436  CALL 0x009bc110
009de43b  MOV ECX,dword ptr [EAX]
009de43d  FLD float ptr [ECX + 0xa4]
009de443  FCHS
009de445  FMUL double ptr [0x00d9e7a8]
009de44b  FMUL float ptr [ESP + 0x1c]
009de44f  FSTP float ptr [ESP + 0x1c]
009de453  FLD float ptr [ESP + 0x1c]
009de457  FMUL float ptr [ESP + 0x18]
009de45b  FSTP float ptr [ESP + 0x18]
009de45f  FLD float ptr [ESP + 0x18]
009de463  FLD ST0
009de465  FABS
009de467  FSTP float ptr [ESP + 0x1c]
009de46b  FLD float ptr [ESP + 0x1c]
009de46f  FLD float ptr [ESP + 0x28]
009de473  FCOMPP
009de475  FNSTSW AX
009de477  TEST AH,0x5
009de47a  JP 0x009de5d5
009de480  MOV EDX,dword ptr [EDI + 0x24]
009de483  FSTP ST0
009de485  PUSH EDX
009de486  LEA ECX,[EDI + 0x14]
009de489  CALL 0x009bc110
009de48e  MOV EAX,dword ptr [EAX]
009de490  FLD float ptr [EAX + 0xb4]
009de496  FSTP float ptr [ESP + 0x1c]
009de49a  FLDZ
009de49c  FLD float ptr [ESP + 0x18]
009de4a0  FCOM
009de4a2  FNSTSW AX
009de4a4  FLD float ptr [ESP + 0x28]
009de4a8  TEST AH,0x41
009de4ab  JZ 0x009de4af
009de4ad  FCHS
009de4af  FSTP float ptr [ESP + 0x20]
009de4b3  MOV dword ptr [ESI + 0x12c],0x1
009de4bd  FLD float ptr [ESP + 0x1c]
009de4c1  FLD ST0
009de4c3  FLD1
009de4c5  FSUBRP
009de4c7  FMUL float ptr [ESP + 0x20]
009de4cb  FXCH
009de4cd  FMULP ST2
009de4cf  FADDP
009de4d1  FSTP float ptr [ESP + 0x18]
009de4d5  FLD float ptr [ESP + 0x18]
009de4d9  FXCH
009de4db  CMP dword ptr [ESI + 0x12c],0x0
009de4e2  JZ 0x009de4f1
009de4e4  MOV ECX,dword ptr [ESP + 0x168]
009de4eb  MOV dword ptr [ECX],0x1
009de4f1  CMP dword ptr [ESP + 0x24],0x0
009de4f6  FLD ST1
009de4f8  FMUL float ptr [ESP + 0x40]
009de4fc  FSTP float ptr [ESP + 0x9c]
009de503  FLD ST1
009de505  FMUL float ptr [ESP + 0x44]
009de509  FSTP float ptr [ESP + 0xa0]
009de510  FLD float ptr [ESP + 0x48]
009de514  FMULP ST2
009de516  FXCH
009de518  FSTP float ptr [ESP + 0xa4]
009de51f  JZ 0x009de890
009de525  FLD float ptr [0x00cadc20]
009de52b  MOV EBP,dword ptr [ESP + 0x154]
009de532  FCOMP float ptr [EBP + 0x8]
009de535  FNSTSW AX
009de537  TEST AH,0x5
009de53a  JP 0x009de890
009de540  FST float ptr [ESP + 0x100]
009de547  FST float ptr [ESP + 0xfc]
009de54e  FST float ptr [ESP + 0xf8]
009de555  FST float ptr [ESP + 0xdc]
009de55c  FST float ptr [ESP + 0xd8]
009de563  FSTP float ptr [ESP + 0xd4]
009de56a  FLD float ptr [EBP]
009de56d  FSTP float ptr [ESP + 0x74]
009de571  FLD float ptr [EBP + 0x4]
009de574  FSTP float ptr [ESP + 0x78]
009de578  FLD float ptr [EBP + 0x8]
009de57b  FSTP float ptr [ESP + 0x7c]
009de57f  FLD float ptr [ESP + 0x78]
009de583  FLD float ptr [ESP + 0x74]
009de587  FLD float ptr [ESP + 0x7c]
009de58b  FLD ST1
009de58d  FMULP ST2
009de58f  FLD ST2
009de591  FMULP ST3
009de593  FXCH
009de595  FADDP ST2,ST0
009de597  FMUL ST0
009de599  FADDP
009de59b  FSTP float ptr [ESP + 0x1c]
009de59f  FLD float ptr [ESP + 0x1c]
009de5a3  FLD float ptr [0x00e89bb8]
009de5a9  FCOMP
009de5ab  FNSTSW AX
009de5ad  TEST AH,0x5
009de5b0  JP 0x009de5e6
009de5b2  CALL 0x007e8520
009de5b7  FSTP float ptr [ESP + 0x1c]
009de5bb  FLD float ptr [ESP + 0x1c]
009de5bf  FLD1
009de5c1  FDIVRP
009de5c3  FSTP float ptr [ESP + 0x1c]
009de5c7  FLD float ptr [ESP + 0x1c]
009de5cb  FMUL float ptr [ESP + 0x74]
009de5cf  FSTP float ptr [ESP + 0x74]
009de5d3  JMP 0x009de5e8
009de5d5  FLDZ
009de5d7  MOV dword ptr [ESI + 0x12c],0x0
009de5e1  JMP 0x009de4db
009de5e6  FSTP ST0
009de5e8  FLD float ptr [EBX + 0x54]
009de5eb  FLD double ptr [0x00ca2d98]
009de5f1  FCOM
009de5f3  FNSTSW AX
009de5f5  FSTP ST1
009de5f7  TEST AH,0x5
009de5fa  JP 0x009de63d
009de5fc  FSTP ST0
009de5fe  LEA EDX,[ESP + 0xec]
009de605  FLD float ptr [EBP]
009de608  PUSH EDX
009de609  FLD double ptr [0x00c833c8]
009de60f  MOV ECX,EBX
009de611  FMUL ST1
009de613  FXCH
009de615  FSTP float ptr [ESP + 0xf0]
009de61c  FLD float ptr [EBP + 0x4]
009de61f  FMUL ST1
009de621  FSTP float ptr [ESP + 0xf4]
009de628  FMUL float ptr [EBP + 0x8]
009de62b  FSTP float ptr [ESP + 0xf8]
009de632  CALL 0x009d74e0
009de637  FLD double ptr [0x00ca2d98]
009de63d  CMP dword ptr [EBX + 0x60c],0x0
009de644  JNZ 0x009de7da
009de64a  FLD float ptr [EBX + 0x50]
009de64d  FCOMP
009de64f  FNSTSW AX
009de651  TEST AH,0x41
009de654  JNZ 0x009de7da
009de65a  FLD float ptr [EBX + 0x54]
009de65d  FCOMPP
009de65f  FNSTSW AX
009de661  TEST AH,0x41
009de664  JP 0x009de7dc
009de66a  MOV ESI,dword ptr [EBX + 0x64]
009de66d  MOV EAX,dword ptr [ESI + 0x24]
009de670  PUSH EAX
009de671  LEA ECX,[ESI + 0x14]
009de674  CALL 0x009bc110
009de679  FLD float ptr [ESP + 0x74]
009de67d  MOV ECX,dword ptr [ESI + 0x24]
009de680  FABS
009de682  FSTP float ptr [ESP + 0x18]
009de686  PUSH ECX
009de687  FLD float ptr [ESP + 0x1c]
009de68b  LEA ECX,[ESI + 0x14]
009de68e  FMUL double ptr [0x00c77ef0]
009de694  MOV EDI,EAX
009de696  FADD double ptr [0x00c75250]
009de69c  FSTP double ptr [ESP + 0x6c]
009de6a0  CALL 0x009bc110
009de6a5  FLD float ptr [EBP + 0x8]
009de6a8  MOV EDX,dword ptr [EDI]
009de6aa  FABS
009de6ac  MOV EAX,dword ptr [EAX]
009de6ae  FSTP float ptr [ESP + 0x1c]
009de6b2  MOV ECX,dword ptr [ESI + 0x24]
009de6b5  FLD float ptr [ESP + 0x1c]
009de6b9  PUSH ECX
009de6ba  FADD double ptr [0x00c75250]
009de6c0  LEA ECX,[ESI + 0x14]
009de6c3  FMUL ST0
009de6c5  FST double ptr [ESP + 0xac]
009de6cc  FLD float ptr [EDX + 0x348]
009de6d2  FMUL float ptr [EBX + 0x50]
009de6d5  FMUL double ptr [0x00cbd568]
009de6db  FMUL float ptr [ESP + 0x1c]
009de6df  FMUL double ptr [ESP + 0x6c]
009de6e3  FMUL float ptr [EAX + 0x344]
009de6e9  FDIVRP
009de6eb  FSTP float ptr [ESP + 0x20]
009de6ef  CALL 0x009bc110
009de6f4  MOV EDX,dword ptr [ESI + 0x24]
009de6f7  PUSH EDX
009de6f8  LEA ECX,[ESI + 0x14]
009de6fb  MOV EDI,EAX
009de6fd  CALL 0x009bc110
009de702  MOV ECX,dword ptr [EDI]
009de704  FLD float ptr [ECX + 0x33c]
009de70a  MOV EDX,dword ptr [EAX]
009de70c  FMUL float ptr [ESP + 0x74]
009de710  LEA ECX,[ESI + 0x14]
009de713  FLD float ptr [EDX + 0x340]
009de719  FMUL float ptr [EBX + 0x50]
009de71c  FADD double ptr [0x00c75250]
009de722  FDIVP
009de724  FSTP float ptr [ESP + 0xf8]
009de72b  FLDZ
009de72d  FSTP float ptr [ESP + 0xfc]
009de734  FLD float ptr [ESP + 0x1c]
009de738  FSTP float ptr [ESP + 0x100]
009de73f  MOV EAX,dword ptr [ESI + 0x24]
009de742  PUSH EAX
009de743  CALL 0x009bc110
009de748  MOV ECX,dword ptr [ESI + 0x24]
009de74b  PUSH ECX
009de74c  LEA ECX,[ESI + 0x14]
009de74f  MOV EDI,EAX
009de751  CALL 0x009bc110
009de756  MOV EDX,dword ptr [EDI]
009de758  FLD float ptr [EDX + 0x348]
009de75e  MOV EAX,dword ptr [EAX]
009de760  FMUL float ptr [EBX + 0x50]
009de763  MOV ECX,dword ptr [ESI + 0x24]
009de766  PUSH ECX
009de767  LEA ECX,[ESI + 0x14]
009de76a  FMUL float ptr [ESP + 0x1c]
009de76e  FMUL double ptr [ESP + 0x6c]
009de772  FMUL float ptr [EAX + 0x344]
009de778  FDIV double ptr [ESP + 0xac]
009de77f  FSTP float ptr [ESP + 0x20]
009de783  CALL 0x009bc110
009de788  MOV EDX,dword ptr [ESI + 0x24]
009de78b  MOV EDI,EAX
009de78d  PUSH EDX
009de78e  LEA ECX,[ESI + 0x14]
009de791  CALL 0x009bc110
009de796  MOV ECX,dword ptr [EDI]
009de798  FLD float ptr [ECX + 0x33c]
009de79e  MOV EDX,dword ptr [EAX]
009de7a0  FLD float ptr [ESP + 0x74]
009de7a4  FMUL double ptr [0x00c80bc8]
009de7aa  FMULP
009de7ac  FLD float ptr [EDX + 0x340]
009de7b2  FMUL float ptr [EBX + 0x50]
009de7b5  FADD double ptr [0x00c75250]
009de7bb  FDIVP
009de7bd  FSTP float ptr [ESP + 0xd4]
009de7c4  FLDZ
009de7c6  FSTP float ptr [ESP + 0xd8]
009de7cd  FLD float ptr [ESP + 0x1c]
009de7d1  FSTP float ptr [ESP + 0xdc]
009de7d8  JMP 0x009de7dc
009de7da  FSTP ST0
009de7dc  LEA EBP,[EBX + 0x2e8]
009de7e2  MOV ECX,EBP
009de7e4  CALL 0x009d5a40
009de7e9  XOR ESI,ESI
009de7eb  TEST EAX,EAX
009de7ed  MOV dword ptr [ESP + 0x1c],EAX
009de7f1  JBE 0x009de892
009de7f7  PUSH ESI
009de7f8  MOV ECX,EBP
009de7fa  CALL 0x006f1300
009de7ff  TEST ESI,ESI
009de801  MOV EDI,EAX
009de803  JZ 0x009de80a
009de805  CMP ESI,0x1
009de808  JNZ 0x009de839
009de80a  FLD float ptr [EBX + 0x54]
009de80d  FCOMP double ptr [0x00ca2d98]
009de813  FNSTSW AX
009de815  TEST AH,0x41
009de818  JP 0x009de839
009de81a  CMP dword ptr [EDI + 0x12c],0x0
009de821  JZ 0x009de839
009de823  LEA EAX,[EDI + 0xa8]
009de829  PUSH EAX
009de82a  LEA ECX,[ESP + 0xfc]
009de831  PUSH ECX
009de832  MOV ECX,EBX
009de834  CALL 0x009d7490
009de839  CMP ESI,0x2
009de83c  JZ 0x009de843
009de83e  CMP ESI,0x3
009de841  JNZ 0x009de872
009de843  FLD float ptr [EBX + 0x54]
009de846  FCOMP double ptr [0x00ca2d98]
009de84c  FNSTSW AX
009de84e  TEST AH,0x41
009de851  JP 0x009de872
009de853  CMP dword ptr [EDI + 0x12c],0x0
009de85a  JZ 0x009de872
009de85c  ADD EDI,0xa8
009de862  PUSH EDI
009de863  LEA EDX,[ESP + 0xd8]
009de86a  PUSH EDX
009de86b  MOV ECX,EBX
009de86d  CALL 0x009d7490
009de872  ADD ESI,0x1
009de875  CMP ESI,dword ptr [ESP + 0x1c]
009de879  JC 0x009de7f7
009de87f  LEA EAX,[ESP + 0x9c]
009de886  PUSH EAX
009de887  MOV ECX,EBX
009de889  CALL 0x009d74e0
009de88e  JMP 0x009de8a5
009de890  FSTP ST0
009de892  LEA EAX,[ESP + 0x9c]
009de899  PUSH EAX
009de89a  MOV ECX,EBX
009de89c  CALL 0x009d74e0
009de8a1  JMP 0x009de8a5
009de8a3  FSTP ST0
009de8a5  MOV EAX,dword ptr [ESP + 0x50]
009de8a9  ADD EAX,0x1
009de8ac  CMP EAX,dword ptr [ESP + 0x14]
009de8b0  MOV dword ptr [ESP + 0x50],EAX
009de8b4  JC 0x009dde10
009de8ba  XOR EBP,EBP
009de8bc  CMP dword ptr [ESP + 0x160],EBP
009de8c3  JZ 0x009dfcd2
009de8c9  MOV EDI,dword ptr [ESP + 0x154]
009de8d0  FLD float ptr [EDI + 0x4]
009de8d3  FLD float ptr [EDI]
009de8d5  FLD float ptr [EDI + 0x8]
009de8d8  FLD ST1
009de8da  FMULP ST2
009de8dc  FLD ST2
009de8de  FMULP ST3
009de8e0  FXCH
009de8e2  FADDP ST2,ST0
009de8e4  FMUL ST0
009de8e6  FADDP
009de8e8  FSTP float ptr [ESP + 0x14]
009de8ec  FLD float ptr [ESP + 0x14]
009de8f0  CALL 0x007e8520
009de8f5  FSTP float ptr [ESP + 0x14]
009de8f9  MOV ESI,dword ptr [EBX + 0x60c]
009de8ff  FLD float ptr [ESP + 0x14]
009de903  CMP ESI,EBP
009de905  FSTP float ptr [ESP + 0x8c]
009de90c  JNZ 0x009de94d
009de90e  FLD float ptr [EBX + 0x54]
009de911  FLD double ptr [0x00ca2d98]
009de917  FCOM
009de919  FNSTSW AX
009de91b  FSTP ST1
009de91d  TEST AH,0x5
009de920  JP 0x009de953
009de922  FLD float ptr [EBX + 0x50]
009de925  FCOMP
009de927  FNSTSW AX
009de929  TEST AH,0x5
009de92c  JP 0x009de953
009de92e  CMP dword ptr [EBX + 0x69c],0x1
009de935  JNZ 0x009de953
009de937  MOV ECX,dword ptr [ESP + 0x2c]
009de93b  MOV dword ptr [EBX + 0x6f8],ECX
009de941  MOV dword ptr [EBX + 0x69c],0x3
009de94b  JMP 0x009de953
009de94d  FLD double ptr [0x00ca2d98]
009de953  CMP ESI,EBP
009de955  JNZ 0x009def1e
009de95b  FLD float ptr [EBX + 0x50]
009de95e  FCOMP
009de960  FNSTSW AX
009de962  TEST AH,0x41
009de965  JNZ 0x009de9cd
009de967  FLD float ptr [EBX + 0x54]
009de96a  FCOMP
009de96c  FNSTSW AX
009de96e  TEST AH,0x41
009de971  JNZ 0x009de9cd
009de973  MOV EAX,dword ptr [EBX + 0x64]
009de976  FSTP ST0
009de978  MOV EDX,dword ptr [EAX + 0x24]
009de97b  PUSH EDX
009de97c  LEA ECX,[EAX + 0x14]
009de97f  CALL 0x009bc110
009de984  FLD float ptr [EDI + 0x8]
009de987  MOV EAX,dword ptr [EAX]
009de989  FLD float ptr [EAX + 0x258]
009de98f  FCOMPP
009de991  FNSTSW AX
009de993  TEST AH,0x41
009de996  JNZ 0x009de9c7
009de998  FLD float ptr [0x00c75060]
009de99e  FCOMP float ptr [ESP + 0x108]
009de9a5  FNSTSW AX
009de9a7  TEST AH,0x5
009de9aa  JP 0x009de9c7
009de9ac  MOV ECX,dword ptr [ESP + 0x2c]
009de9b0  MOV EAX,0x1
009de9b5  MOV dword ptr [EBX + 0x69c],EAX
009de9bb  MOV dword ptr [EBX + 0x6a0],EAX
009de9c1  MOV dword ptr [EBX + 0x6f4],ECX
009de9c7  FLD double ptr [0x00ca2d98]
009de9cd  CMP ESI,EBP
009de9cf  JNZ 0x009def1e
009de9d5  FLD float ptr [EBX + 0x50]
009de9d8  FCOMP
009de9da  FNSTSW AX
009de9dc  TEST AH,0x41
009de9df  JNZ 0x009def1e
009de9e5  FLD float ptr [EBX + 0x54]
009de9e8  FCOMP
009de9ea  FNSTSW AX
009de9ec  TEST AH,0x41
009de9ef  JNZ 0x009def1e
009de9f5  MOV ESI,dword ptr [EBX + 0x64]
009de9f8  FSTP ST0
009de9fa  MOV EDX,dword ptr [ESI + 0x24]
009de9fd  PUSH EDX
009de9fe  LEA ECX,[ESI + 0x14]
009dea01  CALL 0x009bc110
009dea06  FLD float ptr [EDI + 0x8]
009dea09  MOV EAX,dword ptr [EAX]
009dea0b  FLD float ptr [EAX + 0x254]
009dea11  FCOMPP
009dea13  FNSTSW AX
009dea15  TEST AH,0x41
009dea18  JNZ 0x009def18
009dea1e  MOV ECX,dword ptr [ESI + 0x24]
009dea21  PUSH ECX
009dea22  LEA ECX,[ESI + 0x14]
009dea25  CALL 0x009bc110
009dea2a  FLD float ptr [EDI + 0x8]
009dea2d  MOV EDX,dword ptr [EAX]
009dea2f  FLD float ptr [EDX + 0x258]
009dea35  FCOMPP
009dea37  FNSTSW AX
009dea39  TEST AH,0x5
009dea3c  JP 0x009def18
009dea42  FLD float ptr [ESP + 0x15c]
009dea49  FLD ST0
009dea4b  FABS
009dea4d  FSTP float ptr [ESP + 0x14]
009dea51  FLD float ptr [ESP + 0x14]
009dea55  FCOMP float ptr [0x00d1453c]
009dea5b  FNSTSW AX
009dea5d  TEST AH,0x5
009dea60  JNP 0x009def16
009dea66  FSTP float ptr [ESP + 0x14]
009dea6a  MOV dword ptr [EBX + 0x69c],0x2
009dea74  MOV EAX,dword ptr [ESP + 0x14]
009dea78  NOT EAX
009dea7a  TEST EAX,EAX
009dea7c  JNS 0x009dea82
009dea7e  FLD1
009dea80  JMP 0x009dea88
009dea82  FLD float ptr [0x00c75064]
009dea88  FSTP float ptr [ESP + 0x24]
009dea8c  LEA ESI,[EBX + 0x6fc]
009dea92  FLD float ptr [ESP + 0x24]
009dea96  LEA ECX,[EBX + 0x2e8]
009dea9c  FSTP float ptr [EBX + 0x708]
009deaa2  FLDZ
009deaa4  FST float ptr [ESI + 0x8]
009deaa7  FST float ptr [ESI + 0x4]
009deaaa  FSTP float ptr [ESI]
009deaac  CALL 0x009d5a40
009deab1  XOR EBP,EBP
009deab3  TEST EAX,EAX
009deab5  MOV dword ptr [ESP + 0x14],EAX
009deab9  JBE 0x009deb00
009deabb  JMP 0x009deac0
009deac0  PUSH EBP
009deac1  LEA ECX,[EBX + 0x2e8]
009deac7  CALL 0x006f1300
009deacc  CMP dword ptr [EAX + 0x124],0x0
009dead3  JZ 0x009deaf7
009dead5  FLD float ptr [EAX + 0x144]
009deadb  FADD float ptr [ESI]
009deadd  FSTP float ptr [ESI]
009deadf  FLD float ptr [EAX + 0x148]
009deae5  FADD float ptr [ESI + 0x4]
009deae8  FSTP float ptr [ESI + 0x4]
009deaeb  FLD float ptr [EAX + 0x14c]
009deaf1  FADD float ptr [ESI + 0x8]
009deaf4  FSTP float ptr [ESI + 0x8]
009deaf7  ADD EBP,0x1
009deafa  CMP EBP,dword ptr [ESP + 0x14]
009deafe  JC 0x009deac0
009deb00  FLD float ptr [ESI + 0x4]
009deb03  FLD float ptr [ESI]
009deb05  FLD float ptr [ESI + 0x8]
009deb08  FLD ST1
009deb0a  FMULP ST2
009deb0c  FLD ST2
009deb0e  FMULP ST3
009deb10  FXCH
009deb12  FADDP ST2,ST0
009deb14  FMUL ST0
009deb16  FADDP
009deb18  FSTP float ptr [ESP + 0x14]
009deb1c  FLD float ptr [ESP + 0x14]
009deb20  FLD float ptr [0x00e89bb8]
009deb26  FCOMP
009deb28  FNSTSW AX
009deb2a  TEST AH,0x5
009deb2d  JP 0x009deb5b
009deb2f  CALL 0x007e8520
009deb34  FSTP float ptr [ESP + 0x14]
009deb38  FLD float ptr [ESP + 0x14]
009deb3c  FLD1
009deb3e  FDIVRP
009deb40  FSTP float ptr [ESP + 0x14]
009deb44  FLD float ptr [ESP + 0x14]
009deb48  FLD ST0
009deb4a  FMUL float ptr [ESI]
009deb4c  FSTP float ptr [ESI]
009deb4e  FLD float ptr [ESI + 0x4]
009deb51  FMUL ST1
009deb53  FSTP float ptr [ESI + 0x4]
009deb56  FMUL float ptr [ESI + 0x8]
009deb59  JMP 0x009deb70
009deb5b  FSTP ST0
009deb5d  MOV dword ptr [EBX + 0x69c],0x0
009deb67  FLDZ
009deb69  FST float ptr [ESI]
009deb6b  FLD1
009deb6d  FSTP float ptr [ESI + 0x4]
009deb70  FSTP float ptr [ESI + 0x8]
009deb73  MOV EBP,dword ptr [ESP + 0x60]
009deb77  FLD float ptr [EBX + 0x1dc]
009deb7d  FSUB float ptr [EBP]
009deb80  FSTP float ptr [ESP + 0x90]
009deb87  FLD float ptr [EBX + 0x1e0]
009deb8d  FSUB float ptr [EBP + 0x4]
009deb90  FSTP float ptr [ESP + 0x94]
009deb97  FLD float ptr [EBX + 0x1e4]
009deb9d  FSUB float ptr [EBP + 0x8]
009deba0  FSTP float ptr [ESP + 0x98]
009deba7  FLD float ptr [EBX + 0x1ec]
009debad  FLDZ
009debaf  FMUL ST1
009debb1  FLD float ptr [EBX + 0x1e8]
009debb7  FMUL ST1
009debb9  FADDP ST2,ST0
009debbb  FLD float ptr [EBX + 0x1f0]
009debc1  FADDP ST2,ST0
009debc3  FXCH
009debc5  FSTP float ptr [ESP + 0x14]
009debc9  FLD float ptr [ESP + 0x14]
009debcd  FLD ST0
009debcf  FMULP ST2
009debd1  FXCH
009debd3  FSTP float ptr [ESP + 0x14]
009debd7  FLD float ptr [ESP + 0x14]
009debdb  FST float ptr [ESP + 0x54]
009debdf  FSTP float ptr [ESP + 0x58]
009debe3  FSTP float ptr [ESP + 0x5c]
009debe7  FLD float ptr [ESP + 0x54]
009debeb  FADD float ptr [ESP + 0x90]
009debf2  FSTP float ptr [ESP + 0xb0]
009debf9  FLD float ptr [ESP + 0x58]
009debfd  FADD float ptr [ESP + 0x94]
009dec04  FSTP float ptr [ESP + 0xb4]
009dec0b  FLD float ptr [ESP + 0x5c]
009dec0f  FADD float ptr [ESP + 0x98]
009dec16  FSTP float ptr [ESP + 0xb8]
009dec1d  FLD float ptr [ESP + 0xb4]
009dec24  FLD float ptr [ESP + 0xb0]
009dec2b  FLD float ptr [ESP + 0xb8]
009dec32  FLD ST1
009dec34  FMULP ST2
009dec36  FLD ST2
009dec38  FMULP ST3
009dec3a  FXCH
009dec3c  FADDP ST2,ST0
009dec3e  FMUL ST0
009dec40  FADDP
009dec42  FSTP float ptr [ESP + 0x14]
009dec46  FLD float ptr [ESP + 0x14]
009dec4a  CALL 0x007e8520
009dec4f  FSTP float ptr [ESP + 0x14]
009dec53  FLD float ptr [ESP + 0x14]
009dec57  FSTP float ptr [EBX + 0x6ec]
009dec5d  FLD float ptr [ESI + 0x4]
009dec60  FMUL float ptr [EDI + 0x8]
009dec63  FLD float ptr [ESI + 0x8]
009dec66  FMUL float ptr [EDI + 0x4]
009dec69  FSUBP
009dec6b  FSTP float ptr [ESP + 0x40]
009dec6f  FLD float ptr [ESI + 0x8]
009dec72  FMUL float ptr [EDI]
009dec74  FLD float ptr [ESI]
009dec76  FMUL float ptr [EDI + 0x8]
009dec79  FSUBP
009dec7b  FSTP float ptr [ESP + 0x44]
009dec7f  FLD float ptr [ESI]
009dec81  FMUL float ptr [EDI + 0x4]
009dec84  FLD float ptr [ESI + 0x4]
009dec87  FMUL float ptr [EDI]
009dec89  FSUBP
009dec8b  FSTP float ptr [ESP + 0x48]
009dec8f  FLD float ptr [ESP + 0x15c]
009dec96  FSTP float ptr [ESP + 0x14]
009dec9a  MOV ECX,dword ptr [ESP + 0x14]
009dec9e  NOT ECX
009deca0  TEST ECX,ECX
009deca2  JNS 0x009deca8
009deca4  FLD1
009deca6  JMP 0x009decae
009deca8  FLD float ptr [0x00c75064]
009decae  FSTP float ptr [ESP + 0x24]
009decb2  FLD float ptr [ESP + 0x24]
009decb6  FLD ST0
009decb8  FMUL float ptr [ESP + 0x40]
009decbc  FSTP float ptr [ESP + 0x40]
009decc0  FLD ST0
009decc2  FMUL float ptr [ESP + 0x44]
009decc6  FSTP float ptr [ESP + 0x44]
009decca  FMUL float ptr [ESP + 0x48]
009decce  FSTP float ptr [ESP + 0x48]
009decd2  FLD float ptr [ESP + 0x44]
009decd6  FLD float ptr [ESP + 0x40]
009decda  FLD float ptr [ESP + 0x48]
009decde  FLD ST2
009dece0  FMULP ST3
009dece2  FLD ST1
009dece4  FMULP ST2
009dece6  FXCH ST2
009dece8  FADDP
009decea  FLD ST1
009decec  FMULP ST2
009decee  FADDP
009decf0  FSTP float ptr [ESP + 0x14]
009decf4  FLD float ptr [ESP + 0x14]
009decf8  FLD float ptr [0x00e89bb8]
009decfe  FCOMP
009ded00  FNSTSW AX
009ded02  TEST AH,0x5
009ded05  JP 0x009ded3e
009ded07  CALL 0x007e8520
009ded0c  FSTP float ptr [ESP + 0x14]
009ded10  FLD float ptr [ESP + 0x14]
009ded14  FLD1
009ded16  FDIVRP
009ded18  FSTP float ptr [ESP + 0x14]
009ded1c  FLD float ptr [ESP + 0x14]
009ded20  FLD ST0
009ded22  FMUL float ptr [ESP + 0x40]
009ded26  FSTP float ptr [ESP + 0x40]
009ded2a  FLD ST0
009ded2c  FMUL float ptr [ESP + 0x44]
009ded30  FSTP float ptr [ESP + 0x44]
009ded34  FMUL float ptr [ESP + 0x48]
009ded38  FSTP float ptr [ESP + 0x48]
009ded3c  JMP 0x009ded40
009ded3e  FSTP ST0
009ded40  LEA EAX,[EBX + 0x6a4]
009ded46  PUSH EAX
009ded47  MOV ECX,ESI
009ded49  CALL 0x0046a170
009ded4e  FLD float ptr [0x00c75060]
009ded54  FCOMP float ptr [EBX + 0x700]
009ded5a  FNSTSW AX
009ded5c  TEST AH,0x41
009ded5f  JNZ 0x009ded7b
009ded61  FLDZ
009ded63  MOV dword ptr [EBX + 0x69c],0x0
009ded6d  FST float ptr [ESI]
009ded6f  FLD1
009ded71  FST float ptr [ESI + 0x4]
009ded74  FXCH
009ded76  FST float ptr [ESI + 0x8]
009ded79  JMP 0x009ded7f
009ded7b  FLD1
009ded7d  FLDZ
009ded7f  FLD float ptr [ESP + 0x15c]
009ded86  FSTP float ptr [ESP + 0x14]
009ded8a  MOV EDX,dword ptr [ESP + 0x14]
009ded8e  NOT EDX
009ded90  TEST EDX,EDX
009ded92  JNS 0x009ded9c
009ded94  FXCH
009ded96  FST float ptr [ESP + 0x24]
009ded9a  JMP 0x009deda8
009ded9c  FLD float ptr [0x00c75064]
009deda2  FSTP float ptr [ESP + 0x24]
009deda6  FXCH
009deda8  FXCH
009dedaa  LEA EAX,[ESP + 0x40]
009dedae  FST float ptr [ESP + 0x9c]
009dedb5  PUSH EAX
009dedb6  FSTP float ptr [ESP + 0xa4]
009dedbd  LEA ECX,[ESP + 0xa0]
009dedc4  PUSH ECX
009dedc5  FSTP float ptr [ESP + 0xac]
009dedcc  CALL 0x00473610
009dedd1  MOV ESI,dword ptr [EBX + 0x64]
009dedd4  FMUL float ptr [ESP + 0x2c]
009dedd8  MOV EDX,dword ptr [ESI + 0x24]
009deddb  ADD ESP,0x8
009dedde  PUSH EDX
009deddf  FSTP float ptr [ESP + 0x28]
009dede3  LEA ECX,[ESI + 0x14]
009dede6  CALL 0x009bc110
009dedeb  FLD float ptr [ESP + 0x24]
009dedef  MOV EAX,dword ptr [EAX]
009dedf1  FLD float ptr [EAX + 0x290]
009dedf7  FCOMPP
009dedf9  FNSTSW AX
009dedfb  TEST AH,0x5
009dedfe  JNP 0x009deef6
009dee04  MOV ECX,dword ptr [ESI + 0x24]
009dee07  PUSH ECX
009dee08  LEA ECX,[ESI + 0x14]
009dee0b  CALL 0x009bc110
009dee10  FLD float ptr [ESP + 0x24]
009dee14  MOV EDX,dword ptr [EAX]
009dee16  FLD float ptr [EDX + 0x294]
009dee1c  FCHS
009dee1e  FCOMPP
009dee20  FNSTSW AX
009dee22  TEST AH,0x41
009dee25  JZ 0x009deef6
009dee2b  LEA ESI,[EBX + 0x6a4]
009dee31  PUSH ESI
009dee32  LEA EAX,[ESP + 0x44]
009dee36  PUSH EAX
009dee37  LEA ECX,[ESP + 0xa4]
009dee3e  CALL 0x00469ff0
009dee43  PUSH ESI
009dee44  PUSH EBP
009dee45  LEA ECX,[ESP + 0xf4]
009dee4c  CALL 0x0046a050
009dee51  MOV EAX,dword ptr [EBX + 0x64]
009dee54  MOV ECX,dword ptr [EAX + 0x24]
009dee57  PUSH ECX
009dee58  LEA ECX,[EAX + 0x14]
009dee5b  CALL 0x009bc110
009dee60  FLD float ptr [EDI + 0x8]
009dee63  MOV EAX,dword ptr [EAX]
009dee65  PUSH ECX
009dee66  MOV ECX,EAX
009dee68  FSTP float ptr [ESP]
009dee6b  CALL 0x00a28c40
009dee70  FADD float ptr [EBX + 0x6ec]
009dee76  FSTP float ptr [ESP + 0x14]
009dee7a  FLD float ptr [ESP + 0x14]
009dee7e  FST float ptr [EBX + 0x6f0]
009dee84  FSTP float ptr [ESP + 0x14]
009dee88  FLD float ptr [ESP + 0x9c]
009dee8f  FLD float ptr [ESP + 0x14]
009dee93  FLD ST0
009dee95  FMULP ST2
009dee97  FXCH
009dee99  FSTP float ptr [EBX + 0x6e0]
009dee9f  FLD ST0
009deea1  FMUL float ptr [ESP + 0xa0]
009deea8  FSTP float ptr [EBX + 0x6e4]
009deeae  FMUL float ptr [ESP + 0xa4]
009deeb5  FSTP float ptr [EBX + 0x6e8]
009deebb  FLD float ptr [ESP + 0xec]
009deec2  FADD float ptr [EBX + 0x6e0]
009deec8  FSTP float ptr [EBX + 0x6e0]
009deece  FLD float ptr [EBX + 0x6e4]
009deed4  FADD float ptr [ESP + 0xf0]
009deedb  FSTP float ptr [EBX + 0x6e4]
009deee1  FLD float ptr [ESP + 0xf4]
009deee8  FADD float ptr [EBX + 0x6e8]
009deeee  FSTP float ptr [EBX + 0x6e8]
009deef4  JMP 0x009def00
009deef6  MOV dword ptr [EBX + 0x69c],0x0
009def00  XOR EDX,EDX
009def02  CMP dword ptr [EBX + 0x69c],0x2
009def09  SETZ DL
009def0c  XOR EBP,EBP
009def0e  MOV dword ptr [EBX + 0x6a0],EDX
009def14  JMP 0x009def18
009def16  FSTP ST0
009def18  FLD double ptr [0x00ca2d98]
009def1e  CMP dword ptr [EBX + 0x69c],EBP
009def24  JNZ 0x009df00c
009def2a  FLD float ptr [EBX + 0x54]
009def2d  FCOMP
009def2f  FNSTSW AX
009def31  FLD float ptr [0x00c88eec]
009def37  TEST AH,0x41
009def3a  JNZ 0x009def6d
009def3c  FLD float ptr [EDI + 0x8]
009def3f  FLD float ptr [EBX + 0x5cc]
009def45  FCOMPP
009def47  FNSTSW AX
009def49  TEST AH,0x41
009def4c  JNZ 0x009def6d
009def4e  FLD float ptr [EDI]
009def50  FABS
009def52  FSTP float ptr [ESP + 0x14]
009def56  FLD float ptr [ESP + 0x14]
009def5a  FCOMP
009def5c  FNSTSW AX
009def5e  TEST AH,0x5
009def61  JP 0x009def6d
009def63  MOV dword ptr [EBX + 0x5c4],0x1
009def6d  FLD float ptr [EBX + 0x50]
009def70  FCOMP ST2
009def72  FNSTSW AX
009def74  FLDZ
009def76  TEST AH,0x41
009def79  JNZ 0x009defa0
009def7b  FCOM float ptr [EDI + 0x8]
009def7e  FNSTSW AX
009def80  TEST AH,0x5
009def83  JNP 0x009def9a
009def85  FLD float ptr [EDI]
009def87  FABS
009def89  FSTP float ptr [ESP + 0x14]
009def8d  FLD float ptr [ESP + 0x14]
009def91  FCOMP ST2
009def93  FNSTSW AX
009def95  TEST AH,0x41
009def98  JNZ 0x009defa0
009def9a  MOV dword ptr [EBX + 0x5c4],EBP
009defa0  FLD float ptr [EBX + 0x50]
009defa3  FCOMP ST3
009defa5  FNSTSW AX
009defa7  TEST AH,0x5
009defaa  JP 0x009deff4
009defac  FLD float ptr [EBX + 0x54]
009defaf  FCOMP ST3
009defb1  FNSTSW AX
009defb3  FSTP ST2
009defb5  TEST AH,0x5
009defb8  JP 0x009deff6
009defba  FXCH
009defbc  FCOM float ptr [EDI + 0x8]
009defbf  FNSTSW AX
009defc1  TEST AH,0x41
009defc4  JNP 0x009defea
009defc6  FLD float ptr [EDI + 0x8]
009defc9  FABS
009defcb  FSTP float ptr [ESP + 0x14]
009defcf  FLD float ptr [ESP + 0x14]
009defd3  FCOMP ST2
009defd5  FNSTSW AX
009defd7  FSTP ST1
009defd9  TEST AH,0x5
009defdc  JNP 0x009defec
009defde  MOV dword ptr [EBX + 0x5c4],0x1
009defe8  JMP 0x009deff8
009defea  FSTP ST1
009defec  MOV dword ptr [EBX + 0x5c4],EBP
009deff2  JMP 0x009deff8
009deff4  FSTP ST2
009deff6  FSTP ST0
009deff8  FCOMP float ptr [EDI + 0x8]
009deffb  FNSTSW AX
009deffd  TEST AH,0x5
009df000  JP 0x009df014
009df002  CMP dword ptr [EBX + 0x600],EBP
009df008  JZ 0x009df014
009df00a  JMP 0x009df00e
009df00c  FSTP ST0
009df00e  MOV dword ptr [EBX + 0x5c4],EBP
009df014  FLD float ptr [EDI]
009df016  FSTP float ptr [ESP + 0x14]
009df01a  FLD float ptr [ESP + 0x14]
009df01e  FMUL ST0
009df020  FSTP float ptr [ESP + 0x14]
009df024  FLD float ptr [ESP + 0x14]
009df028  FLD float ptr [EDI + 0x8]
009df02b  FABS
009df02d  FSTP float ptr [ESP + 0x14]
009df031  FLD float ptr [ESP + 0x14]
009df035  FADD double ptr [0x00c75250]
009df03b  FDIVP
009df03d  FSTP float ptr [ESP + 0x1c]
009df041  FLD float ptr [EDI]
009df043  FCHS
009df045  FSTP float ptr [ESP + 0x14]
009df049  MOV EAX,dword ptr [ESP + 0x14]
009df04d  NOT EAX
009df04f  TEST EAX,EAX
009df051  JNS 0x009df057
009df053  FLD1
009df055  JMP 0x009df05d
009df057  FLD float ptr [0x00c75064]
009df05d  MOV EAX,dword ptr [EBX + 0x64]
009df060  FSTP float ptr [ESP + 0x24]
009df064  MOV ECX,dword ptr [EAX + 0x24]
009df067  PUSH ECX
009df068  LEA ECX,[EAX + 0x14]
009df06b  CALL 0x009bc110
009df070  FLD float ptr [ESP + 0x1c]
009df074  MOV EAX,dword ptr [EAX]
009df076  PUSH ECX
009df077  MOV ECX,EAX
009df079  FSTP float ptr [ESP]
009df07c  CALL 0x00a28d40
009df081  FMUL float ptr [ESP + 0x24]
009df085  LEA EDX,[ESP + 0xe0]
009df08c  PUSH EDX
009df08d  FSTP float ptr [ESP + 0x18]
009df091  MOV ECX,EBX
009df093  FLDZ
009df095  FST float ptr [ESP + 0xe4]
009df09c  FSTP float ptr [ESP + 0xe8]
009df0a3  FLD float ptr [ESP + 0x18]
009df0a7  FSTP float ptr [ESP + 0xec]
009df0ae  CALL 0x009d7530
009df0b3  FLDZ
009df0b5  FST float ptr [ESP + 0x20]
009df0b9  LEA ECX,[EBX + 0x2e8]
009df0bf  FSTP float ptr [ESP + 0x28]
009df0c3  CALL 0x009d5a40
009df0c8  CMP EAX,EBP
009df0ca  MOV dword ptr [ESP + 0x68],EAX
009df0ce  MOV dword ptr [ESP + 0x64],EBP
009df0d2  JBE 0x009df40c
009df0d8  FLD float ptr [ESP + 0x8c]
009df0df  FABS
009df0e1  FSTP float ptr [ESP + 0x14]
009df0e5  MOV EAX,dword ptr [ESP + 0x64]
009df0e9  PUSH EAX
009df0ea  LEA ECX,[EBX + 0x2e8]
009df0f0  CALL 0x006f1300
009df0f5  MOV EBP,dword ptr [EAX + 0x4]
009df0f8  TEST EBP,EBP
009df0fa  MOV dword ptr [ESP + 0xa8],EAX
009df101  JZ 0x009df10b
009df103  FLD float ptr [EBX + 0x840]
009df109  JMP 0x009df113
009df10b  FLD float ptr [EBX + 0x840]
009df111  FCHS
009df113  FMUL double ptr [0x00d9e7a8]
009df119  MOV ECX,dword ptr [ESP + 0x158]
009df120  MOV ESI,dword ptr [EBX + 0x64]
009df123  MOV EDX,dword ptr [ESI + 0x24]
009df126  FSTP float ptr [ESP + 0x30]
009df12a  PUSH EDX
009df12b  FLD float ptr [ECX + 0x4]
009df12e  LEA ECX,[ESI + 0x14]
009df131  FMUL float ptr [ESP + 0x34]
009df135  FSTP float ptr [ESP + 0x78]
009df139  FLD float ptr [ESP + 0x78]
009df13d  FADD float ptr [EDI]
009df13f  FSTP float ptr [ESP + 0x78]
009df143  FLD float ptr [EDI + 0x4]
009df146  FLDZ
009df148  FADD ST1,ST0
009df14a  FXCH
009df14c  FSTP float ptr [ESP + 0x7c]
009df150  FADD float ptr [EDI + 0x8]
009df153  FSTP float ptr [ESP + 0x80]
009df15a  CALL 0x009bc110
009df15f  MOV EAX,dword ptr [EAX]
009df161  FLD float ptr [EAX + 0x74]
009df164  FSTP float ptr [ESP + 0x1c]
009df168  FLD float ptr [0x00c94f74]
009df16e  FCOMP float ptr [ESP + 0x14]
009df172  FNSTSW AX
009df174  TEST AH,0x41
009df177  JNZ 0x009df17d
009df179  FLDZ
009df17b  JMP 0x009df1bc
009df17d  FLD float ptr [ESP + 0x8c]
009df184  FLD float ptr [ESP + 0x1c]
009df188  FCOM
009df18a  FNSTSW AX
009df18c  TEST AH,0x5
009df18f  JP 0x009df199
009df191  FSTP ST0
009df193  FSTP ST0
009df195  FLD1
009df197  JMP 0x009df1bc
009df199  FDIVP
009df19b  FMUL double ptr [0x00c80bb8]
009df1a1  FMUL double ptr [0x00d9e7a8]
009df1a7  FSTP float ptr [ESP + 0x1c]
009df1ab  FLD float ptr [ESP + 0x1c]
009df1af  CALL 0x007ea240
009df1b4  FSTP float ptr [ESP + 0x1c]
009df1b8  FLD float ptr [ESP + 0x1c]
009df1bc  MOV ECX,dword ptr [ESI + 0x24]
009df1bf  FSTP float ptr [ESP + 0x60]
009df1c3  PUSH ECX
009df1c4  LEA ECX,[ESI + 0x14]
009df1c7  CALL 0x009bc110
009df1cc  FLD float ptr [EDI + 0x8]
009df1cf  MOV EAX,dword ptr [EAX]
009df1d1  PUSH ECX
009df1d2  MOV ECX,EAX
009df1d4  FSTP float ptr [ESP]
009df1d7  CALL 0x00a287d0
009df1dc  MOV EDX,dword ptr [ESP + 0x164]
009df1e3  MOV ESI,dword ptr [EBX + 0x64]
009df1e6  FMUL float ptr [EDX + 0xc]
009df1e9  MOV EAX,dword ptr [ESI + 0x24]
009df1ec  PUSH EAX
009df1ed  LEA ECX,[ESI + 0x14]
009df1f0  FSTP float ptr [ESP + 0x28]
009df1f4  CALL 0x009bc110
009df1f9  FLD float ptr [ESP + 0x78]
009df1fd  FLDZ
009df1ff  MOV ECX,dword ptr [EAX]
009df201  FMUL ST1
009df203  FLD float ptr [ESP + 0x74]
009df207  FADDP ST2,ST0
009df209  FMUL float ptr [ESP + 0x7c]
009df20d  FADDP
009df20f  FSTP float ptr [ESP + 0x1c]
009df213  FLD float ptr [ESP + 0x1c]
009df217  FLD float ptr [ECX + 0xa4]
009df21d  FCHS
009df21f  FMUL double ptr [0x00d9e7a8]
009df225  FMULP
009df227  FSTP float ptr [ESP + 0x18]
009df22b  FLD float ptr [ESP + 0x18]
009df22f  FABS
009df231  FSTP float ptr [ESP + 0x50]
009df235  FLD float ptr [ESP + 0x50]
009df239  FLD float ptr [ESP + 0x24]
009df23d  FCOMPP
009df23f  FNSTSW AX
009df241  TEST AH,0x5
009df244  JP 0x009df2ce
009df24a  MOV EDX,dword ptr [ESI + 0x24]
009df24d  PUSH EDX
009df24e  LEA ECX,[ESI + 0x14]
009df251  CALL 0x009bc110
009df256  MOV EAX,dword ptr [EAX]
009df258  FLD float ptr [EAX + 0xe4]
009df25e  FSTP float ptr [ESP + 0x1c]
009df262  FLD float ptr [ESP + 0x1c]
009df266  FLD ST0
009df268  FLD1
009df26a  FSUBRP
009df26c  FLD float ptr [ESP + 0x24]
009df270  FLD ST0
009df272  FMULP ST2
009df274  FLD float ptr [ESP + 0x50]
009df278  FLD ST0
009df27a  FMULP ST4
009df27c  FXCH ST2
009df27e  FADDP ST3,ST0
009df280  FXCH ST2
009df282  FSTP float ptr [ESP + 0x50]
009df286  FLD float ptr [ESP + 0x28]
009df28a  FADDP ST2,ST0
009df28c  FXCH
009df28e  FSTP float ptr [ESP + 0x28]
009df292  FADD float ptr [ESP + 0x20]
009df296  FSTP float ptr [ESP + 0x20]
009df29a  FLD float ptr [ESP + 0x18]
009df29e  FSTP float ptr [ESP + 0x1c]
009df2a2  MOV ECX,dword ptr [ESP + 0x1c]
009df2a6  NOT ECX
009df2a8  TEST ECX,ECX
009df2aa  JNS 0x009df2b0
009df2ac  FLD1
009df2ae  JMP 0x009df2b6
009df2b0  FLD float ptr [0x00c75064]
009df2b6  FSTP float ptr [ESP + 0x24]
009df2ba  MOV dword ptr [ESP + 0x4c],0x1
009df2c2  FLD float ptr [ESP + 0x24]
009df2c6  FMUL float ptr [ESP + 0x50]
009df2ca  FSTP float ptr [ESP + 0x18]
009df2ce  MOV EDX,dword ptr [ESI + 0x24]
009df2d1  PUSH EDX
009df2d2  LEA ECX,[ESI + 0x14]
009df2d5  CALL 0x009bc110
009df2da  TEST EBP,EBP
009df2dc  MOV EAX,dword ptr [EAX]
009df2de  FLD float ptr [EAX + 0x98]
009df2e4  FMUL float ptr [ESP + 0x18]
009df2e8  FSTP float ptr [ESP + 0x50]
009df2ec  JZ 0x009df37b
009df2f2  CMP dword ptr [EBX + 0x5c4],0x0
009df2f9  JZ 0x009df303
009df2fb  FLD float ptr [0x00c75064]
009df301  JMP 0x009df305
009df303  FLD1
009df305  MOV ECX,dword ptr [ESP + 0xa8]
009df30c  FSTP float ptr [ESP + 0x24]
009df310  CMP dword ptr [ECX + 0x12c],0x0
009df317  JZ 0x009df32f
009df319  MOV EDX,dword ptr [ESI + 0x24]
009df31c  PUSH EDX
009df31d  LEA ECX,[ESI + 0x14]
009df320  CALL 0x009bc110
009df325  MOV EAX,dword ptr [EAX]
009df327  FLD float ptr [EAX + 0x9c]
009df32d  JMP 0x009df331
009df32f  FLD1
009df331  MOV ECX,dword ptr [ESI + 0x24]
009df334  FSTP float ptr [ESP + 0x18]
009df338  PUSH ECX
009df339  LEA ECX,[ESI + 0x14]
009df33c  CALL 0x009bc110
009df341  FLD float ptr [ESP + 0x50]
009df345  MOV EAX,dword ptr [EAX]
009df347  FSTP double ptr [ESP + 0xa8]
009df34e  FLD float ptr [EDI + 0x8]
009df351  PUSH ECX
009df352  MOV ECX,EAX
009df354  FSTP float ptr [ESP]
009df357  CALL 0x00a288e0
009df35c  FLD float ptr [ESP + 0x24]
009df360  FMUL float ptr [ESP + 0x60]
009df364  FMUL float ptr [EBX + 0x5e8]
009df36a  FMULP
009df36c  FMUL float ptr [ESP + 0x18]
009df370  FSUBR double ptr [ESP + 0xa8]
009df377  FSTP float ptr [ESP + 0x50]
009df37b  FLD float ptr [ESP + 0x50]
009df37f  LEA EDX,[ESP + 0x9c]
009df386  FST float ptr [ESP + 0x54]
009df38a  PUSH EDX
009df38b  FLDZ
009df38d  MOV ECX,EBX
009df38f  FMUL ST1
009df391  FXCH
009df393  FSTP float ptr [ESP + 0xac]
009df39a  FLD float ptr [ESP + 0xac]
009df3a1  FST float ptr [ESP + 0x5c]
009df3a5  FSTP float ptr [ESP + 0x60]
009df3a9  FLD float ptr [ESP + 0x60]
009df3ad  FMUL ST1
009df3af  FLD float ptr [ESP + 0x34]
009df3b3  FLD ST0
009df3b5  FLD float ptr [ESP + 0x5c]
009df3b9  FLD ST0
009df3bb  FMULP ST2
009df3bd  FLD ST3
009df3bf  FSUBRP ST2,ST0
009df3c1  FXCH
009df3c3  FSTP float ptr [ESP + 0xa0]
009df3ca  FLD float ptr [ESP + 0x58]
009df3ce  FLD ST0
009df3d0  FMULP ST3
009df3d2  FXCH ST2
009df3d4  FSUBRP ST3,ST0
009df3d6  FXCH ST2
009df3d8  FSTP float ptr [ESP + 0xa4]
009df3df  FXCH
009df3e1  FMUL ST2
009df3e3  FXCH
009df3e5  FMULP ST2
009df3e7  FSUBRP
009df3e9  FSTP float ptr [ESP + 0xa8]
009df3f0  CALL 0x009d7530
009df3f5  MOV EAX,dword ptr [ESP + 0x64]
009df3f9  ADD EAX,0x1
009df3fc  CMP EAX,dword ptr [ESP + 0x68]
009df400  MOV dword ptr [ESP + 0x64],EAX
009df404  JC 0x009df0e5
009df40a  XOR EBP,EBP
009df40c  CMP dword ptr [ESP + 0x4c],EBP
009df410  MOV EAX,dword ptr [ESP + 0x2c]
009df414  JZ 0x009df436
009df416  CMP dword ptr [ESP + 0x38],EBP
009df41a  MOV dword ptr [EBX + 0x62c],EAX
009df420  JNZ 0x009df428
009df422  MOV dword ptr [EBX + 0x630],EAX
009df428  MOV ECX,EAX
009df42a  SUB ECX,dword ptr [EBX + 0x630]
009df430  MOV dword ptr [EBX + 0x634],ECX
009df436  CMP EAX,dword ptr [EBX + 0x62c]
009df43c  FLD1
009df43e  FSTP float ptr [ESP + 0x30]
009df442  JNZ 0x009df4b8
009df444  FLD float ptr [0x00d1453c]
009df44a  FCOMP float ptr [ESP + 0x28]
009df44e  FNSTSW AX
009df450  TEST AH,0x5
009df453  JP 0x009df4b8
009df455  MOV EAX,dword ptr [EBX + 0x64]
009df458  MOV EDX,dword ptr [EAX + 0x24]
009df45b  PUSH EDX
009df45c  LEA ECX,[EAX + 0x14]
009df45f  CALL 0x009bc110
009df464  FLD float ptr [ESP + 0x20]
009df468  FLD float ptr [ESP + 0x28]
009df46c  MOV EAX,dword ptr [EAX]
009df46e  FLD ST0
009df470  FSUBP ST2,ST0
009df472  FDIVP
009df474  FSTP float ptr [ESP + 0x38]
009df478  FLD float ptr [ESP + 0x38]
009df47c  FDIV float ptr [EAX + 0x200]
009df482  FSTP float ptr [ESP + 0x38]
009df486  FLDZ
009df488  FLD float ptr [ESP + 0x38]
009df48c  FCOM
009df48e  FNSTSW AX
009df490  TEST AH,0x41
009df493  JNP 0x009df4a6
009df495  FSTP ST1
009df497  FLD1
009df499  FCOM
009df49b  FNSTSW AX
009df49d  TEST AH,0x41
009df4a0  JP 0x009df4a6
009df4a2  FSTP ST1
009df4a4  JMP 0x009df4a8
009df4a6  FSTP ST0
009df4a8  FSTP float ptr [ESP + 0x20]
009df4ac  FLD float ptr [ESP + 0x20]
009df4b0  FLD1
009df4b2  FSUBRP
009df4b4  FSTP float ptr [ESP + 0x30]
009df4b8  MOV EAX,dword ptr [EBX + 0x64]
009df4bb  MOV ECX,dword ptr [EAX + 0x24]
009df4be  PUSH ECX
009df4bf  LEA ECX,[EAX + 0x14]
009df4c2  CALL 0x009bc110
009df4c7  FLD float ptr [EDI + 0x8]
009df4ca  MOV EAX,dword ptr [EAX]
009df4cc  PUSH ECX
009df4cd  MOV ECX,EAX
009df4cf  FSTP float ptr [ESP]
009df4d2  CALL 0x00a28a90
009df4d7  FSTP float ptr [ESP + 0x38]
009df4db  CMP dword ptr [EBX + 0x5c4],EBP
009df4e1  MOV EAX,dword ptr [EBX + 0x64]
009df4e4  JZ 0x009df504
009df4e6  MOV EDX,dword ptr [EAX + 0x24]
009df4e9  PUSH EDX
009df4ea  LEA ECX,[EAX + 0x14]
009df4ed  CALL 0x009bc110
009df4f2  FLD float ptr [EDI + 0x8]
009df4f5  MOV EAX,dword ptr [EAX]
009df4f7  PUSH ECX
009df4f8  MOV ECX,EAX
009df4fa  FSTP float ptr [ESP]
009df4fd  CALL 0x00a28c00
009df502  JMP 0x009df520
009df504  MOV ECX,dword ptr [EAX + 0x24]
009df507  PUSH ECX
009df508  LEA ECX,[EAX + 0x14]
009df50b  CALL 0x009bc110
009df510  FLD float ptr [EDI + 0x8]
009df513  MOV EAX,dword ptr [EAX]
009df515  PUSH ECX
009df516  MOV ECX,EAX
009df518  FSTP float ptr [ESP]
009df51b  CALL 0x00a28a50
009df520  CMP dword ptr [EBX + 0x2e4],0x1
009df527  FSTP float ptr [ESP + 0x20]
009df52b  JNZ 0x009df531
009df52d  FLDZ
009df52f  JMP 0x009df547
009df531  FLD float ptr [ESP + 0x30]
009df535  FLD ST0
009df537  FLD1
009df539  FSUBRP
009df53b  FMUL float ptr [ESP + 0x38]
009df53f  FLD float ptr [ESP + 0x20]
009df543  FMULP ST2
009df545  FADDP
009df547  MOV ESI,dword ptr [EBX + 0x64]
009df54a  FSTP float ptr [ESP + 0xa8]
009df551  FLD float ptr [EBX + 0x5e8]
009df557  MOV EDX,dword ptr [ESI + 0x24]
009df55a  FABS
009df55c  PUSH EDX
009df55d  FSTP float ptr [ESP + 0x3c]
009df561  LEA ECX,[ESI + 0x14]
009df564  FLD float ptr [ESP + 0x3c]
009df568  FSTP float ptr [ESP + 0x3c]
009df56c  CALL 0x009bc110
009df571  MOV EBP,dword ptr [EAX]
009df573  MOV EAX,dword ptr [ESI + 0x24]
009df576  PUSH EAX
009df577  LEA ECX,[ESI + 0x14]
009df57a  CALL 0x009bc110
009df57f  FLD float ptr [EDI + 0x8]
009df582  MOV EAX,dword ptr [EAX]
009df584  PUSH ECX
009df585  MOV ECX,EAX
009df587  FSTP float ptr [ESP]
009df58a  CALL 0x00a28ae0
009df58f  FLD float ptr [EBP + 0x7c]
009df592  FMUL float ptr [ESP + 0x38]
009df596  MOV EBP,dword ptr [EBX + 0x69c]
009df59c  CMP EBP,0x1
009df59f  FMULP
009df5a1  FSTP float ptr [ESP + 0x14]
009df5a5  FLD1
009df5a7  FSTP float ptr [ESP + 0x64]
009df5ab  FLDZ
009df5ad  FST float ptr [ESP + 0x60]
009df5b1  JNZ 0x009df645
009df5b7  MOV ESI,dword ptr [EBX + 0x64]
009df5ba  FSTP ST0
009df5bc  MOV ECX,dword ptr [ESI + 0x24]
009df5bf  PUSH ECX
009df5c0  LEA ECX,[ESI + 0x14]
009df5c3  CALL 0x009bc110
009df5c8  MOV EDX,dword ptr [ESI + 0x24]
009df5cb  PUSH EDX
009df5cc  LEA ECX,[ESI + 0x14]
009df5cf  MOV dword ptr [ESP + 0x3c],EAX
009df5d3  CALL 0x009bc110
009df5d8  MOV ESI,EAX
009df5da  MOV EAX,dword ptr [ESP + 0x2c]
009df5de  SUB EAX,dword ptr [EBX + 0x6f4]
009df5e4  TEST EAX,EAX
009df5e6  MOV dword ptr [ESP + 0x68],EAX
009df5ea  FILD dword ptr [ESP + 0x68]
009df5ee  JGE 0x009df5f6
009df5f0  FADD float ptr [0x00d9e7b8]
009df5f6  MOV ECX,dword ptr [ESP + 0x38]
009df5fa  FMUL double ptr [0x00c80bb8]
009df600  MOV EDX,dword ptr [ECX]
009df602  MOV EAX,dword ptr [EDX + 0x298]
009df608  FILD dword ptr [EDX + 0x298]
009df60e  TEST EAX,EAX
009df610  JGE 0x009df618
009df612  FADD float ptr [0x00d9e7b8]
009df618  FDIVP
009df61a  FSTP float ptr [ESP + 0x38]
009df61e  FLD float ptr [ESP + 0x38]
009df622  CALL 0x007ea240
009df627  FSTP float ptr [ESP + 0x38]
009df62b  MOV ECX,dword ptr [ESI]
009df62d  FLD float ptr [ECX + 0x29c]
009df633  FLD1
009df635  FSUB ST1,ST0
009df637  FXCH
009df639  FMUL float ptr [ESP + 0x38]
009df63d  FADDP
009df63f  FSTP float ptr [ESP + 0x64]
009df643  FLDZ
009df645  CMP EBP,0x3
009df648  JNZ 0x009df740
009df64e  MOV ESI,dword ptr [EBX + 0x64]
009df651  FSTP ST0
009df653  MOV EDX,dword ptr [ESI + 0x24]
009df656  MOV EDI,dword ptr [ESP + 0x2c]
009df65a  SUB EDI,dword ptr [EBX + 0x6f8]
009df660  PUSH EDX
009df661  LEA ECX,[ESI + 0x14]
009df664  CALL 0x009bc110
009df669  MOV EBP,EAX
009df66b  MOV EAX,dword ptr [ESI + 0x24]
009df66e  PUSH EAX
009df66f  LEA ECX,[ESI + 0x14]
009df672  CALL 0x009bc110
009df677  MOV ECX,EDI
009df679  TEST ECX,ECX
009df67b  MOV dword ptr [ESP + 0x38],ECX
009df67f  FILD dword ptr [ESP + 0x38]
009df683  MOV dword ptr [ESP + 0x68],EAX
009df687  JGE 0x009df68f
009df689  FADD float ptr [0x00d9e7b8]
009df68f  MOV EDX,dword ptr [EBP]
009df692  FMUL double ptr [0x00c80bb8]
009df698  MOV EAX,dword ptr [EDX + 0x2a8]
009df69e  FILD dword ptr [EDX + 0x2a8]
009df6a4  TEST EAX,EAX
009df6a6  JGE 0x009df6ae
009df6a8  FADD float ptr [0x00d9e7b8]
009df6ae  FDIVP
009df6b0  FSTP float ptr [ESP + 0x38]
009df6b4  FLD float ptr [ESP + 0x38]
009df6b8  CALL 0x007ea240
009df6bd  FSTP float ptr [ESP + 0x38]
009df6c1  MOV ECX,dword ptr [ESP + 0x68]
009df6c5  MOV EDX,dword ptr [ECX]
009df6c7  FLD float ptr [EDX + 0x2ac]
009df6cd  MOV EAX,dword ptr [ESI + 0x24]
009df6d0  FLD1
009df6d2  PUSH EAX
009df6d3  FSUB ST1,ST0
009df6d5  LEA ECX,[ESI + 0x14]
009df6d8  FXCH
009df6da  FMUL float ptr [ESP + 0x3c]
009df6de  FADDP
009df6e0  FSTP float ptr [ESP + 0x68]
009df6e4  CALL 0x009bc110
009df6e9  MOV ECX,dword ptr [EAX]
009df6eb  XOR EDX,EDX
009df6ed  MOV EAX,EDI
009df6ef  DIV dword ptr [ECX + 0x2a8]
009df6f5  TEST EAX,EAX
009df6f7  MOV dword ptr [ESP + 0x38],EAX
009df6fb  FILD dword ptr [ESP + 0x38]
009df6ff  JGE 0x009df707
009df701  FADD float ptr [0x00d9e7b8]
009df707  FSUB double ptr [0x00c75250]
009df70d  MOV EDX,dword ptr [ESI + 0x24]
009df710  PUSH EDX
009df711  LEA ECX,[ESI + 0x14]
009df714  FSTP float ptr [ESP + 0x3c]
009df718  CALL 0x009bc110
009df71d  FLD float ptr [ESP + 0x38]
009df721  MOV EAX,dword ptr [EAX]
009df723  MOV EDI,dword ptr [ESP + 0x154]
009df72a  FMUL ST0
009df72c  FSTP float ptr [ESP + 0x38]
009df730  FLD float ptr [ESP + 0x38]
009df734  FMUL float ptr [EAX + 0x2b8]
009df73a  FSTP float ptr [ESP + 0x60]
009df73e  FLDZ
009df740  MOV EAX,dword ptr [EBX + 0x5c4]
009df746  TEST EAX,EAX
009df748  JZ 0x009df756
009df74a  FLD float ptr [0x00c75064]
009df750  FSTP float ptr [ESP + 0x30]
009df754  JMP 0x009df75a
009df756  FST float ptr [ESP + 0x30]
009df75a  MOV ECX,dword ptr [EBX + 0x600]
009df760  TEST ECX,ECX
009df762  JZ 0x009df773
009df764  FLD float ptr [EBX + 0x5f4]
009df76a  FSTP float ptr [ESP + 0x8c]
009df771  JMP 0x009df77a
009df773  FST float ptr [ESP + 0x8c]
009df77a  TEST EAX,EAX
009df77c  JZ 0x009df786
009df77e  FLD float ptr [0x00c75064]
009df784  JMP 0x009df788
009df786  FLD1
009df788  CMP dword ptr [ESP + 0x10],0x0
009df78d  FSTP float ptr [ESP + 0x24]
009df791  MOV EAX,dword ptr [ESP + 0x164]
009df798  FLD float ptr [ESP + 0x30]
009df79c  FMUL float ptr [EAX + 0x4]
009df79f  FMUL float ptr [EBX + 0x54]
009df7a2  FLD float ptr [EBX + 0x50]
009df7a5  FMUL float ptr [EAX + 0x4]
009df7a8  FADDP
009df7aa  FMUL float ptr [ESP + 0xa8]
009df7b1  FLD float ptr [ESP + 0x20]
009df7b5  FLD ST0
009df7b7  FMUL float ptr [ESP + 0x8c]
009df7be  FADDP ST2,ST0
009df7c0  FLD float ptr [ESP + 0x64]
009df7c4  FMULP ST2
009df7c6  FLD float ptr [ESP + 0x14]
009df7ca  FMUL float ptr [ESP + 0x24]
009df7ce  FSUBP ST2,ST0
009df7d0  FLD float ptr [ESP + 0x60]
009df7d4  FADDP ST2,ST0
009df7d6  FXCH
009df7d8  FSTP float ptr [ESP + 0x18]
009df7dc  JZ 0x009df7ec
009df7de  FLD float ptr [ESP + 0x18]
009df7e2  FMUL double ptr [0x00d9e7a8]
009df7e8  FSTP float ptr [ESP + 0x18]
009df7ec  CMP dword ptr [EBX + 0x60c],0x0
009df7f3  JZ 0x009df81f
009df7f5  TEST ECX,ECX
009df7f7  JZ 0x009df80d
009df7f9  FLD float ptr [EBX + 0x5f4]
009df7ff  FSTP float ptr [ESP + 0x10]
009df803  FMUL float ptr [ESP + 0x10]
009df807  FSTP float ptr [ESP + 0x18]
009df80b  JMP 0x009df821
009df80d  FXCH
009df80f  FST float ptr [ESP + 0x10]
009df813  FXCH
009df815  FMUL float ptr [ESP + 0x10]
009df819  FSTP float ptr [ESP + 0x18]
009df81d  JMP 0x009df821
009df81f  FSTP ST0
009df821  FST float ptr [ESP + 0x28]
009df825  FCOM float ptr [EDI + 0x8]
009df828  FNSTSW AX
009df82a  TEST AH,0x5
009df82d  JP 0x009df963
009df833  FSTP ST0
009df835  LEA EBP,[EBX + 0x2e8]
009df83b  FLD1
009df83d  MOV ECX,EBP
009df83f  FSTP float ptr [ESP + 0x20]
009df843  CALL 0x009d5a40
009df848  XOR ESI,ESI
009df84a  TEST EAX,EAX
009df84c  MOV dword ptr [ESP + 0x38],EAX
009df850  JBE 0x009df88b
009df852  PUSH ESI
009df853  MOV ECX,EBP
009df855  CALL 0x006f1300
009df85a  CMP dword ptr [EAX + 0x12c],0x0
009df861  JZ 0x009df882
009df863  MOV EAX,dword ptr [EBX + 0x64]
009df866  MOV ECX,dword ptr [EAX + 0x24]
009df869  PUSH ECX
009df86a  LEA ECX,[EAX + 0x14]
009df86d  CALL 0x009bc110
009df872  MOV EDX,dword ptr [EAX]
009df874  FLD float ptr [EDX + 0x240]
009df87a  FMUL float ptr [ESP + 0x20]
009df87e  FSTP float ptr [ESP + 0x20]
009df882  ADD ESI,0x1
009df885  CMP ESI,dword ptr [ESP + 0x38]
009df889  JC 0x009df852
009df88b  MOV ESI,dword ptr [EBX + 0x64]
009df88e  MOV EAX,dword ptr [ESI + 0x24]
009df891  PUSH EAX
009df892  LEA ECX,[ESI + 0x14]
009df895  CALL 0x009bc110
009df89a  MOV ECX,dword ptr [ESI + 0x24]
009df89d  PUSH ECX
009df89e  LEA ECX,[ESI + 0x14]
009df8a1  MOV EBP,EAX
009df8a3  CALL 0x009bc110
009df8a8  MOV EDX,dword ptr [EBP]
009df8ab  FLD float ptr [EDX + 0x44]
009df8ae  MOV EAX,dword ptr [EAX]
009df8b0  FMUL float ptr [EDI + 0x8]
009df8b3  MOV ECX,dword ptr [ESP + 0x168]
009df8ba  CMP dword ptr [ECX],0x0
009df8bd  FADD float ptr [EAX + 0x40]
009df8c0  FMUL float ptr [EBX + 0x54]
009df8c3  FMUL float ptr [ESP + 0x20]
009df8c7  FSTP float ptr [ESP + 0x28]
009df8cb  JZ 0x009df8e0
009df8cd  MOV EDX,dword ptr [ESI + 0x24]
009df8d0  PUSH EDX
009df8d1  LEA ECX,[ESI + 0x14]
009df8d4  CALL 0x009bc110
009df8d9  MOV EAX,dword ptr [EAX]
009df8db  FLD float ptr [EAX + 0x48]
009df8de  JMP 0x009df8f1
009df8e0  MOV ECX,dword ptr [ESI + 0x24]
009df8e3  PUSH ECX
009df8e4  LEA ECX,[ESI + 0x14]
009df8e7  CALL 0x009bc110
009df8ec  MOV EDX,dword ptr [EAX]
009df8ee  FLD float ptr [EDX + 0x4c]
009df8f1  MOV EAX,dword ptr [ESP + 0x164]
009df8f8  FSTP float ptr [ESP + 0x10]
009df8fc  FLD float ptr [EAX + 0x8]
009df8ff  FMUL float ptr [ESP + 0x10]
009df903  FSTP float ptr [ESP + 0x38]
009df907  FLD float ptr [ESP + 0x28]
009df90b  FLD float ptr [ESP + 0x38]
009df90f  FCOM
009df911  FNSTSW AX
009df913  FSTP ST1
009df915  TEST AH,0x5
009df918  JP 0x009df95f
009df91a  LEA ECX,[EBX + 0x2e8]
009df920  FSTP float ptr [ESP + 0x28]
009df924  CALL 0x009d5a40
009df929  MOV EBP,EAX
009df92b  XOR ESI,ESI
009df92d  TEST EBP,EBP
009df92f  JBE 0x009df961
009df931  MOV dword ptr [ESP + 0x4c],0x1
009df939  LEA ESP,[ESP]
009df940  PUSH ESI
009df941  LEA ECX,[EBX + 0x2e8]
009df947  CALL 0x006f1300
009df94c  ADD ESI,0x1
009df94f  CMP ESI,EBP
009df951  MOV dword ptr [EAX + 0x12c],0x1
009df95b  JC 0x009df940
009df95d  JMP 0x009df961
009df95f  FSTP ST0
009df961  FLDZ
009df963  FCOMP float ptr [EDI + 0x8]
009df966  FNSTSW AX
009df968  TEST AH,0x41
009df96b  JNZ 0x009dfb31
009df971  FLD float ptr [EBX + 0x50]
009df974  FCOMP double ptr [0x00ca2d98]
009df97a  FNSTSW AX
009df97c  TEST AH,0x41
009df97f  JNZ 0x009dfb31
009df985  CMP dword ptr [EBX + 0x60c],0x0
009df98c  JNZ 0x009df9ff
009df98e  MOV ESI,dword ptr [EBX + 0x64]
009df991  MOV ECX,dword ptr [ESI + 0x24]
009df994  PUSH ECX
009df995  LEA ECX,[ESI + 0x14]
009df998  CALL 0x009bc110
009df99d  MOV EDX,dword ptr [ESI + 0x24]
009df9a0  PUSH EDX
009df9a1  LEA ECX,[ESI + 0x14]
009df9a4  MOV EBP,EAX
009df9a6  CALL 0x009bc110
009df9ab  MOV ECX,dword ptr [EBP]
009df9ae  FLD float ptr [ECX + 0x228]
009df9b4  MOV EDX,dword ptr [EAX]
009df9b6  FLD float ptr [ESP + 0x18]
009df9ba  FCHS
009df9bc  FMULP
009df9be  FMUL float ptr [EDI + 0x8]
009df9c1  FLD float ptr [EDX + 0x22c]
009df9c7  FCOMPP
009df9c9  FNSTSW AX
009df9cb  TEST AH,0x5
009df9ce  JP 0x009df9ff
009df9d0  FLD float ptr [0x00c75060]
009df9d6  FCOMP float ptr [ESP + 0x108]
009df9dd  FNSTSW AX
009df9df  TEST AH,0x5
009df9e2  JP 0x009df9ff
009df9e4  MOV EAX,dword ptr [ESP + 0x2c]
009df9e8  MOV dword ptr [EBX + 0x6f4],EAX
009df9ee  MOV EAX,0x1
009df9f3  MOV dword ptr [EBX + 0x69c],EAX
009df9f9  MOV dword ptr [EBX + 0x6a0],EAX
009df9ff  FLD1
009dfa01  LEA EBP,[EBX + 0x2e8]
009dfa07  MOV ECX,EBP
009dfa09  FSTP float ptr [ESP + 0x20]
009dfa0d  CALL 0x009d5a40
009dfa12  XOR ESI,ESI
009dfa14  TEST EAX,EAX
009dfa16  MOV dword ptr [ESP + 0x38],EAX
009dfa1a  JBE 0x009dfa59
009dfa1c  LEA ESP,[ESP]
009dfa20  PUSH ESI
009dfa21  MOV ECX,EBP
009dfa23  CALL 0x006f1300
009dfa28  CMP dword ptr [EAX + 0x12c],0x0
009dfa2f  JZ 0x009dfa50
009dfa31  MOV EAX,dword ptr [EBX + 0x64]
009dfa34  MOV ECX,dword ptr [EAX + 0x24]
009dfa37  PUSH ECX
009dfa38  LEA ECX,[EAX + 0x14]
009dfa3b  CALL 0x009bc110
009dfa40  MOV EDX,dword ptr [EAX]
009dfa42  FLD float ptr [EDX + 0x240]
009dfa48  FMUL float ptr [ESP + 0x20]
009dfa4c  FSTP float ptr [ESP + 0x20]
009dfa50  ADD ESI,0x1
009dfa53  CMP ESI,dword ptr [ESP + 0x38]
009dfa57  JC 0x009dfa20
009dfa59  MOV ESI,dword ptr [EBX + 0x64]
009dfa5c  MOV EAX,dword ptr [ESI + 0x24]
009dfa5f  PUSH EAX
009dfa60  LEA ECX,[ESI + 0x14]
009dfa63  CALL 0x009bc110
009dfa68  MOV ECX,dword ptr [ESI + 0x24]
009dfa6b  PUSH ECX
009dfa6c  LEA ECX,[ESI + 0x14]
009dfa6f  MOV EBP,EAX
009dfa71  CALL 0x009bc110
009dfa76  MOV EDX,dword ptr [EBP]
009dfa79  FLD float ptr [EDX + 0x40]
009dfa7c  MOV EAX,dword ptr [EAX]
009dfa7e  FLD float ptr [EAX + 0x44]
009dfa81  MOV ECX,dword ptr [ESP + 0x168]
009dfa88  CMP dword ptr [ECX],0x0
009dfa8b  FMUL float ptr [EDI + 0x8]
009dfa8e  FSUBP
009dfa90  FMUL float ptr [EBX + 0x50]
009dfa93  FMUL float ptr [ESP + 0x20]
009dfa97  FSTP float ptr [ESP + 0x28]
009dfa9b  JZ 0x009dfab3
009dfa9d  MOV EDX,dword ptr [ESI + 0x24]
009dfaa0  PUSH EDX
009dfaa1  LEA ECX,[ESI + 0x14]
009dfaa4  CALL 0x009bc110
009dfaa9  MOV EAX,dword ptr [EAX]
009dfaab  FLD float ptr [EAX + 0x248]
009dfab1  JMP 0x009dfac7
009dfab3  MOV ECX,dword ptr [ESI + 0x24]
009dfab6  PUSH ECX
009dfab7  LEA ECX,[ESI + 0x14]
009dfaba  CALL 0x009bc110
009dfabf  MOV EDX,dword ptr [EAX]
009dfac1  FLD float ptr [EDX + 0x24c]
009dfac7  MOV EAX,dword ptr [ESP + 0x164]
009dface  FSTP float ptr [ESP + 0x10]
009dfad2  FLD float ptr [EAX + 0x8]
009dfad5  FMUL float ptr [ESP + 0x10]
009dfad9  FSTP float ptr [ESP + 0x38]
009dfadd  FLD float ptr [ESP + 0x28]
009dfae1  FLD float ptr [ESP + 0x38]
009dfae5  FCOM
009dfae7  FNSTSW AX
009dfae9  FSTP ST1
009dfaeb  TEST AH,0x5
009dfaee  JP 0x009dfb2f
009dfaf0  LEA ECX,[EBX + 0x2e8]
009dfaf6  FSTP float ptr [ESP + 0x28]
009dfafa  CALL 0x009d5a40
009dfaff  MOV EBP,EAX
009dfb01  XOR ESI,ESI
009dfb03  TEST EBP,EBP
009dfb05  JBE 0x009dfb31
009dfb07  MOV dword ptr [ESP + 0x4c],0x1
009dfb0f  NOP
009dfb10  PUSH ESI
009dfb11  LEA ECX,[EBX + 0x2e8]
009dfb17  CALL 0x006f1300
009dfb1c  ADD ESI,0x1
009dfb1f  CMP ESI,EBP
009dfb21  MOV dword ptr [EAX + 0x12c],0x1
009dfb2b  JC 0x009dfb10
009dfb2d  JMP 0x009dfb31
009dfb2f  FSTP ST0
009dfb31  FLD float ptr [ESP + 0x28]
009dfb35  MOV ECX,dword ptr [ESP + 0x16c]
009dfb3c  FST float ptr [ECX]
009dfb3e  FLD float ptr [EDI + 0x8]
009dfb41  FSTP float ptr [ESP + 0x38]
009dfb45  MOV EDX,dword ptr [ESP + 0x38]
009dfb49  NOT EDX
009dfb4b  TEST EDX,EDX
009dfb4d  JNS 0x009dfb53
009dfb4f  FLD1
009dfb51  JMP 0x009dfb59
009dfb53  FLD float ptr [0x00c75064]
009dfb59  MOV ESI,dword ptr [EBX + 0x64]
009dfb5c  FSTP float ptr [ESP + 0x10]
009dfb60  FLD float ptr [ESP + 0x10]
009dfb64  MOV EAX,dword ptr [ESI + 0x24]
009dfb67  FMULP
009dfb69  PUSH EAX
009dfb6a  LEA ECX,[ESI + 0x14]
009dfb6d  FSUBR float ptr [ESP + 0x1c]
009dfb71  FSTP float ptr [ESP + 0x30]
009dfb75  CALL 0x009bc110
009dfb7a  MOV ECX,dword ptr [EAX]
009dfb7c  FLD float ptr [ECX + 0x30]
009dfb7f  MOV EBP,dword ptr [ESP + 0x164]
009dfb86  FMUL float ptr [EBP]
009dfb89  MOV EDX,dword ptr [ESI + 0x24]
009dfb8c  PUSH EDX
009dfb8d  LEA ECX,[ESI + 0x14]
009dfb90  FSTP float ptr [ESP + 0x6c]
009dfb94  CALL 0x009bc110
009dfb99  MOV EAX,dword ptr [EAX]
009dfb9b  FLD float ptr [EDI + 0x8]
009dfb9e  FLD float ptr [EAX + 0x2c]
009dfba1  FMUL float ptr [EBP]
009dfba4  FSTP float ptr [ESP + 0x38]
009dfba8  FLD float ptr [ESP + 0x38]
009dfbac  FCOMPP
009dfbae  FNSTSW AX
009dfbb0  TEST AH,0x5
009dfbb3  JP 0x009dfbf0
009dfbb5  FLDZ
009dfbb7  FCOMP float ptr [ESP + 0x2c]
009dfbbb  FNSTSW AX
009dfbbd  TEST AH,0x41
009dfbc0  JNZ 0x009dfbd9
009dfbc2  MOV ECX,dword ptr [ESI + 0x24]
009dfbc5  PUSH ECX
009dfbc6  LEA ECX,[ESI + 0x14]
009dfbc9  CALL 0x009bc110
009dfbce  FLD float ptr [ESP + 0x2c]
009dfbd2  MOV EDX,dword ptr [EAX]
009dfbd4  FSUB float ptr [EDX + 0x60]
009dfbd7  JMP 0x009dfbec
009dfbd9  MOV EAX,dword ptr [ESI + 0x24]
009dfbdc  PUSH EAX
009dfbdd  LEA ECX,[ESI + 0x14]
009dfbe0  CALL 0x009bc110
009dfbe5  MOV ECX,dword ptr [EAX]
009dfbe7  FLD float ptr [ECX + 0x60]
009dfbea  FCHS
009dfbec  FSTP float ptr [ESP + 0x2c]
009dfbf0  FLD float ptr [EDI + 0x8]
009dfbf3  FLD float ptr [ESP + 0x68]
009dfbf7  FCHS
009dfbf9  FCOMPP
009dfbfb  FNSTSW AX
009dfbfd  TEST AH,0x41
009dfc00  JNZ 0x009dfc3b
009dfc02  FLDZ
009dfc04  FCOMP float ptr [ESP + 0x2c]
009dfc08  FNSTSW AX
009dfc0a  TEST AH,0x5
009dfc0d  JP 0x009dfc26
009dfc0f  MOV EDX,dword ptr [ESI + 0x24]
009dfc12  PUSH EDX
009dfc13  LEA ECX,[ESI + 0x14]
009dfc16  CALL 0x009bc110
009dfc1b  MOV EAX,dword ptr [EAX]
009dfc1d  FLD float ptr [EAX + 0x60]
009dfc20  FADD float ptr [ESP + 0x2c]
009dfc24  JMP 0x009dfc37
009dfc26  MOV ECX,dword ptr [ESI + 0x24]
009dfc29  PUSH ECX
009dfc2a  LEA ECX,[ESI + 0x14]
009dfc2d  CALL 0x009bc110
009dfc32  MOV EDX,dword ptr [EAX]
009dfc34  FLD float ptr [EDX + 0x60]
009dfc37  FSTP float ptr [ESP + 0x2c]
009dfc3b  FLDZ
009dfc3d  LEA EAX,[ESP + 0x9c]
009dfc44  FST float ptr [ESP + 0x9c]
009dfc4b  PUSH EAX
009dfc4c  FSTP float ptr [ESP + 0xa4]
009dfc53  MOV ECX,EBX
009dfc55  FLD float ptr [ESP + 0x30]
009dfc59  FMUL float ptr [ESP + 0x154]
009dfc60  FSTP float ptr [ESP + 0xa8]
009dfc67  CALL 0x009d74e0
009dfc6c  MOV ESI,dword ptr [EBX + 0x64]
009dfc6f  MOV ECX,dword ptr [ESI + 0x24]
009dfc72  PUSH ECX
009dfc73  LEA ECX,[ESI + 0x14]
009dfc76  CALL 0x009bc110
009dfc7b  MOV EDX,dword ptr [ESI + 0x24]
009dfc7e  PUSH EDX
009dfc7f  LEA ECX,[ESI + 0x14]
009dfc82  MOV EDI,EAX
009dfc84  CALL 0x009bc110
009dfc89  MOV ECX,dword ptr [EDI]
009dfc8b  FLD float ptr [ECX + 0x64]
009dfc8e  MOV EDX,dword ptr [ESP + 0x148]
009dfc95  MOV EAX,dword ptr [EAX]
009dfc97  FCHS
009dfc99  FMUL float ptr [EDX + 0x8]
009dfc9c  LEA ECX,[ESP + 0x90]
009dfca3  PUSH ECX
009dfca4  MOV ECX,EBX
009dfca6  FDIV float ptr [EAX + 0x160]
009dfcac  FSTP float ptr [ESP + 0x3c]
009dfcb0  FLDZ
009dfcb2  FST float ptr [ESP + 0x94]
009dfcb9  FSTP float ptr [ESP + 0x98]
009dfcc0  FLD float ptr [ESP + 0x3c]
009dfcc4  FSTP float ptr [ESP + 0x9c]
009dfccb  CALL 0x009d74e0
009dfcd0  JMP 0x009dfcef
009dfcd2  CMP dword ptr [EBX + 0x69c],0x1
009dfcd9  JNZ 0x009dfcef
009dfcdb  MOV EDX,dword ptr [ESP + 0x2c]
009dfcdf  MOV dword ptr [EBX + 0x6f8],EDX
009dfce5  MOV dword ptr [EBX + 0x69c],0x3
009dfcef  MOV EAX,dword ptr [ESP + 0x4c]
009dfcf3  MOV dword ptr [EBX + 0x628],EAX
009dfcf9  MOV EAX,dword ptr [ESP + 0x154]
009dfd00  FLD float ptr [EAX]
009dfd02  POP EDI
009dfd03  FSTP float ptr [EBX + 0x70c]
009dfd09  POP ESI
009dfd0a  FLD float ptr [EAX + 0x4]
009dfd0d  POP EBP
009dfd0e  FSTP float ptr [EBX + 0x710]
009dfd14  FLD float ptr [EAX + 0x8]
009dfd17  FSTP float ptr [EBX + 0x714]
009dfd1d  POP EBX
009dfd1e  ADD ESP,0x130
009dfd24  RET 0x2c
