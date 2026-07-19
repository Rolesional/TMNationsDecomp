009dffd0  SUB ESP,0x78
009dffd3  FLDZ
009dffd5  PUSH EBP
009dffd6  PUSH ESI
009dffd7  MOV ESI,ECX
009dffd9  MOV EAX,dword ptr [ESI + 0x824]
009dffdf  MOV ECX,dword ptr [ESI + 0x828]
009dffe5  MOV EDX,dword ptr [ESI + 0x82c]
009dffeb  MOV dword ptr [ESP + 0x64],EAX
009dffef  MOV EAX,dword ptr [ESI + 0x818]
009dfff5  MOV dword ptr [ESP + 0x68],ECX
009dfff9  MOV ECX,dword ptr [ESI + 0x81c]
009dffff  MOV dword ptr [ESP + 0x6c],EDX
009e0003  MOV EDX,dword ptr [ESI + 0x820]
009e0009  FST float ptr [ESI + 0x820]
009e000f  FST float ptr [ESI + 0x81c]
009e0015  MOV dword ptr [ESP + 0x58],EAX
009e0019  FST float ptr [ESI + 0x818]
009e001f  MOV dword ptr [ESP + 0x5c],ECX
009e0023  FST float ptr [ESI + 0x82c]
009e0029  MOV dword ptr [ESP + 0x60],EDX
009e002d  FST float ptr [ESI + 0x828]
009e0033  FST float ptr [ESI + 0x824]
009e0039  FCOM float ptr [ESI + 0x1e8]
009e003f  FNSTSW AX
009e0041  TEST AH,0x41
009e0044  JNZ 0x009e004d
009e0046  MOV EAX,0x1
009e004b  JMP 0x009e004f
009e004d  XOR EAX,EAX
009e004f  TEST byte ptr [ESI + 0x2f4],0x30
009e0056  JNZ 0x009e0f09
009e005c  XOR EBP,EBP
009e005e  CMP EAX,EBP
009e0060  JNZ 0x009e0f09
009e0066  MOV ECX,ESI
009e0068  FSTP ST0
009e006a  CALL 0x009dd100
009e006f  FLD float ptr [ESP + 0x84]
009e0076  PUSH ECX
009e0077  MOV ECX,ESI
009e0079  FSTP float ptr [ESP]
009e007c  CALL 0x009dce00
009e0081  MOV ECX,dword ptr [0x00ebd820]
009e0087  MOV EAX,dword ptr [ECX + 0x14]
009e008a  CMP EAX,EBP
009e008c  JZ 0x009e0092
009e008e  MOV ECX,EAX
009e0090  JMP 0x009e0098
009e0092  ADD ECX,0xa0
009e0098  PUSH EBX
009e0099  PUSH EDI
009e009a  CALL 0x00428400
009e009f  MOV EAX,dword ptr [EAX]
009e00a1  MOV ECX,ESI
009e00a3  MOV dword ptr [ESP + 0x34],EAX
009e00a7  CALL 0x009d6340
009e00ac  MOV ECX,dword ptr [ESI + 0x28]
009e00af  MOV EDI,dword ptr [ECX + 0x14]
009e00b2  MOV EBX,EAX
009e00b4  MOV EAX,dword ptr [ESI + 0x64]
009e00b7  ADD EDI,0x18
009e00ba  CMP EBX,EBP
009e00bc  MOV dword ptr [ESP + 0x30],EBX
009e00c0  MOV dword ptr [ESP + 0x38],EDI
009e00c4  JZ 0x009e00dc
009e00c6  MOV EDX,dword ptr [EAX + 0x24]
009e00c9  PUSH EDX
009e00ca  LEA ECX,[EAX + 0x14]
009e00cd  CALL 0x009bc110
009e00d2  MOV EAX,dword ptr [EAX]
009e00d4  FLD float ptr [EAX + 0x160]
009e00da  JMP 0x009e00f0
009e00dc  MOV ECX,dword ptr [EAX + 0x24]
009e00df  PUSH ECX
009e00e0  LEA ECX,[EAX + 0x14]
009e00e3  CALL 0x009bc110
009e00e8  MOV EDX,dword ptr [EAX]
009e00ea  FLD float ptr [EDX + 0x164]
009e00f0  CMP EBX,EBP
009e00f2  FSTP float ptr [ESP + 0x2c]
009e00f6  FLD float ptr [ESP + 0x2c]
009e00fa  FSTP float ptr [EDI + 0x34]
009e00fd  JZ 0x009e0103
009e00ff  FLDZ
009e0101  JMP 0x009e011a
009e0103  MOV EAX,dword ptr [ESI + 0x64]
009e0106  MOV ECX,dword ptr [EAX + 0x24]
009e0109  PUSH ECX
009e010a  LEA ECX,[EAX + 0x14]
009e010d  CALL 0x009bc110
009e0112  MOV EDX,dword ptr [EAX]
009e0114  FLD float ptr [EDX + 0x154]
009e011a  FSTP float ptr [ESP + 0x2c]
009e011e  FLD float ptr [ESP + 0x2c]
009e0122  FSTP float ptr [EDI + 0x28]
009e0125  TEST byte ptr [ESI + 0x2f4],0x2
009e012c  JZ 0x009e0ea3
009e0132  FLDZ
009e0134  MOV ECX,dword ptr [ESI + 0x28]
009e0137  LEA EAX,[ESP + 0x3c]
009e013b  FSTP float ptr [ESP + 0x24]
009e013f  PUSH EAX
009e0140  CALL 0x0067aa70
009e0145  TEST byte ptr [ESI + 0x2f4],0x8
009e014c  JZ 0x009e016a
009e014e  FLDZ
009e0150  LEA ECX,[ESP + 0x3c]
009e0154  PUSH ECX
009e0155  FST float ptr [ESP + 0x40]
009e0159  MOV ECX,dword ptr [ESI + 0x28]
009e015c  FSTP float ptr [ESP + 0x48]
009e0160  CALL 0x0067bf60
009e0165  JMP 0x009e09f8
009e016a  MOV ECX,dword ptr [ESI + 0x28]
009e016d  LEA EDX,[ESP + 0x54]
009e0171  PUSH EDX
009e0172  CALL 0x0067aaf0
009e0177  MOV ECX,dword ptr [ESI + 0x28]
009e017a  LEA EAX,[ESP + 0x48]
009e017e  PUSH EAX
009e017f  CALL 0x0067ab30
009e0184  FLDZ
009e0186  LEA ECX,[ESP + 0x3c]
009e018a  FSTP float ptr [ESI + 0x5b0]
009e0190  PUSH ECX
009e0191  MOV ECX,ESI
009e0193  CALL 0x009d7ee0
009e0198  FLD float ptr [ESI + 0x2e0]
009e019e  FMUL ST0
009e01a0  FSTP float ptr [ESP + 0x18]
009e01a4  FLD float ptr [ESP + 0x40]
009e01a8  FLD float ptr [ESP + 0x3c]
009e01ac  FLD float ptr [ESP + 0x44]
009e01b0  FLD ST1
009e01b2  FMULP ST2
009e01b4  FLD ST2
009e01b6  FMULP ST3
009e01b8  FXCH
009e01ba  FADDP ST2,ST0
009e01bc  FMUL ST0
009e01be  FADDP
009e01c0  FSTP float ptr [ESP + 0x2c]
009e01c4  FLD float ptr [ESP + 0x2c]
009e01c8  FLD float ptr [ESP + 0x18]
009e01cc  FCOM
009e01ce  FNSTSW AX
009e01d0  TEST AH,0x5
009e01d3  JP 0x009e0231
009e01d5  FLD float ptr [0x00e89bb8]
009e01db  FCOMPP
009e01dd  FNSTSW AX
009e01df  TEST AH,0x5
009e01e2  JP 0x009e0233
009e01e4  FLD float ptr [ESI + 0x2e0]
009e01ea  FSTP double ptr [ESP + 0x18]
009e01ee  CALL 0x007e8520
009e01f3  FSTP float ptr [ESP + 0x2c]
009e01f7  FLD float ptr [ESP + 0x2c]
009e01fb  LEA EDX,[ESP + 0x3c]
009e01ff  FDIVR double ptr [ESP + 0x18]
009e0203  PUSH EDX
009e0204  MOV ECX,ESI
009e0206  FSTP float ptr [ESP + 0x30]
009e020a  FLD float ptr [ESP + 0x30]
009e020e  FLD ST0
009e0210  FMUL float ptr [ESP + 0x40]
009e0214  FSTP float ptr [ESP + 0x40]
009e0218  FLD float ptr [ESP + 0x44]
009e021c  FMUL ST1
009e021e  FSTP float ptr [ESP + 0x44]
009e0222  FMUL float ptr [ESP + 0x48]
009e0226  FSTP float ptr [ESP + 0x48]
009e022a  CALL 0x009d7540
009e022f  JMP 0x009e0235
009e0231  FSTP ST1
009e0233  FSTP ST0
009e0235  FLD1
009e0237  LEA EAX,[ESP + 0x10]
009e023b  FST float ptr [ESP + 0x78]
009e023f  PUSH EAX
009e0240  LEA ECX,[ESP + 0x7c]
009e0244  FST float ptr [ESP + 0x80]
009e024b  PUSH ECX
009e024c  FST float ptr [ESP + 0x88]
009e0253  MOV ECX,ESI
009e0255  FSTP float ptr [ESP + 0x8c]
009e025c  MOV dword ptr [ESP + 0x18],EBP
009e0260  CALL 0x009dbb70
009e0265  FLD1
009e0267  LEA EDX,[ESP + 0x14]
009e026b  FST float ptr [ESP + 0x18]
009e026f  PUSH EDX
009e0270  FSTP float ptr [ESP + 0x18]
009e0274  LEA EAX,[ESP + 0x1c]
009e0278  PUSH EAX
009e0279  LEA ECX,[ESP + 0x50]
009e027d  PUSH ECX
009e027e  MOV ECX,ESI
009e0280  CALL 0x009d7d10
009e0285  MOV EDI,dword ptr [ESI + 0x64]
009e0288  MOV EDX,dword ptr [EDI + 0x24]
009e028b  PUSH EDX
009e028c  LEA ECX,[EDI + 0x14]
009e028f  CALL 0x009bc110
009e0294  MOV EBX,EAX
009e0296  MOV EAX,dword ptr [EDI + 0x24]
009e0299  PUSH EAX
009e029a  LEA ECX,[EDI + 0x14]
009e029d  CALL 0x009bc110
009e02a2  FLD float ptr [ESP + 0x44]
009e02a6  MOV ECX,dword ptr [EBX]
009e02a8  FABS
009e02aa  MOV EDX,dword ptr [EAX]
009e02ac  FSTP float ptr [ESP + 0x2c]
009e02b0  FLD float ptr [ESP + 0x2c]
009e02b4  FMUL float ptr [ECX + 0x70]
009e02b7  FADD float ptr [EDX + 0x6c]
009e02ba  FSTP float ptr [ESP + 0x2c]
009e02be  FLD float ptr [0x00d1453c]
009e02c4  FLD float ptr [ESP + 0x2c]
009e02c8  FCOM
009e02ca  FNSTSW AX
009e02cc  FSTP ST1
009e02ce  TEST AH,0x5
009e02d1  JP 0x009e02d9
009e02d3  FSTP ST0
009e02d5  FLDZ
009e02d7  JMP 0x009e02f1
009e02d9  FLD1
009e02db  PUSH ECX
009e02dc  FDIVRP
009e02de  FSTP float ptr [ESP + 0x30]
009e02e2  FLD float ptr [ESP + 0x30]
009e02e6  FSTP float ptr [ESP]
009e02e9  CALL 0x0061ab30
009e02ee  ADD ESP,0x4
009e02f1  MOV EAX,dword ptr [ESP + 0x3c]
009e02f5  FSTP float ptr [ESP + 0x2c]
009e02f9  FLD float ptr [ESI + 0x5e8]
009e02ff  MOV ECX,dword ptr [ESP + 0x40]
009e0303  MOV EDX,dword ptr [ESP + 0x44]
009e0307  FCHS
009e0309  FMUL float ptr [ESP + 0x2c]
009e030d  MOV dword ptr [ESI + 0x70c],EAX
009e0313  MOV dword ptr [ESI + 0x710],ECX
009e0319  MOV dword ptr [ESP + 0x28],EBP
009e031d  MOV dword ptr [ESI + 0x714],EDX
009e0323  FSTP float ptr [ESP + 0x2c]
009e0327  MOV EAX,dword ptr [EDI + 0x24]
009e032a  PUSH EAX
009e032b  LEA ECX,[EDI + 0x14]
009e032e  CALL 0x009bc110
009e0333  MOV ECX,dword ptr [EAX]
009e0335  CMP dword ptr [ECX + 0x354],0x3
009e033c  JNZ 0x009e0395
009e033e  FLD float ptr [ESP + 0x2c]
009e0342  LEA EDX,[ESP + 0x24]
009e0346  PUSH EDX
009e0347  MOV EDX,dword ptr [ESP + 0x14]
009e034b  LEA EAX,[ESP + 0x2c]
009e034f  PUSH EAX
009e0350  LEA ECX,[ESP + 0x80]
009e0357  PUSH ECX
009e0358  PUSH EDX
009e0359  PUSH ECX
009e035a  FSTP float ptr [ESP]
009e035d  LEA EAX,[ESP + 0x68]
009e0361  FLD float ptr [ESP + 0x28]
009e0365  PUSH EAX
009e0366  LEA ECX,[ESP + 0x54]
009e036a  PUSH ECX
009e036b  SUB ESP,0x8
009e036e  FSTP float ptr [ESP + 0x4]
009e0372  LEA EDX,[ESP + 0x6c]
009e0376  FLD float ptr [ESP + 0x3c]
009e037a  FSTP float ptr [ESP]
009e037d  PUSH EDX
009e037e  FLD float ptr [ESP + 0xb4]
009e0385  PUSH ECX
009e0386  MOV ECX,ESI
009e0388  FSTP float ptr [ESP]
009e038b  CALL 0x00a2ffa0
009e0390  JMP 0x009e04bc
009e0395  MOV EAX,dword ptr [EDI + 0x24]
009e0398  PUSH EAX
009e0399  LEA ECX,[EDI + 0x14]
009e039c  CALL 0x009bc110
009e03a1  MOV ECX,dword ptr [EAX]
009e03a3  CMP dword ptr [ECX + 0x354],0x4
009e03aa  JNZ 0x009e0403
009e03ac  FLD float ptr [ESP + 0x2c]
009e03b0  LEA EDX,[ESP + 0x24]
009e03b4  PUSH EDX
009e03b5  MOV EDX,dword ptr [ESP + 0x14]
009e03b9  LEA EAX,[ESP + 0x2c]
009e03bd  PUSH EAX
009e03be  LEA ECX,[ESP + 0x80]
009e03c5  PUSH ECX
009e03c6  PUSH EDX
009e03c7  PUSH ECX
009e03c8  FSTP float ptr [ESP]
009e03cb  LEA EAX,[ESP + 0x68]
009e03cf  FLD float ptr [ESP + 0x28]
009e03d3  PUSH EAX
009e03d4  LEA ECX,[ESP + 0x54]
009e03d8  PUSH ECX
009e03d9  SUB ESP,0x8
009e03dc  FSTP float ptr [ESP + 0x4]
009e03e0  LEA EDX,[ESP + 0x6c]
009e03e4  FLD float ptr [ESP + 0x3c]
009e03e8  FSTP float ptr [ESP]
009e03eb  PUSH EDX
009e03ec  FLD float ptr [ESP + 0xb4]
009e03f3  PUSH ECX
009e03f4  MOV ECX,ESI
009e03f6  FSTP float ptr [ESP]
009e03f9  CALL 0x00a30b20
009e03fe  JMP 0x009e04bc
009e0403  MOV EAX,dword ptr [EDI + 0x24]
009e0406  PUSH EAX
009e0407  LEA ECX,[EDI + 0x14]
009e040a  CALL 0x009bc110
009e040f  FLD float ptr [ESP + 0x2c]
009e0413  MOV ECX,dword ptr [EAX]
009e0415  CMP dword ptr [ECX + 0x354],0x5
009e041c  JNZ 0x009e046e
009e041e  LEA EDX,[ESP + 0x24]
009e0422  PUSH EDX
009e0423  MOV EDX,dword ptr [ESP + 0x14]
009e0427  LEA EAX,[ESP + 0x2c]
009e042b  PUSH EAX
009e042c  LEA ECX,[ESP + 0x80]
009e0433  PUSH ECX
009e0434  PUSH EDX
009e0435  PUSH ECX
009e0436  FSTP float ptr [ESP]
009e0439  LEA EAX,[ESP + 0x68]
009e043d  FLD float ptr [ESP + 0x28]
009e0441  PUSH EAX
009e0442  LEA ECX,[ESP + 0x54]
009e0446  PUSH ECX
009e0447  SUB ESP,0x8
009e044a  FSTP float ptr [ESP + 0x4]
009e044e  LEA EDX,[ESP + 0x6c]
009e0452  FLD float ptr [ESP + 0x3c]
009e0456  FSTP float ptr [ESP]
009e0459  PUSH EDX
009e045a  FLD float ptr [ESP + 0xb4]
009e0461  PUSH ECX
009e0462  MOV ECX,ESI
009e0464  FSTP float ptr [ESP]
009e0467  CALL 0x009dd380
009e046c  JMP 0x009e04bc
009e046e  LEA EAX,[ESP + 0x24]
009e0472  PUSH EAX
009e0473  MOV EAX,dword ptr [ESP + 0x14]
009e0477  LEA ECX,[ESP + 0x2c]
009e047b  PUSH ECX
009e047c  LEA EDX,[ESP + 0x80]
009e0483  PUSH EDX
009e0484  PUSH EAX
009e0485  PUSH ECX
009e0486  FSTP float ptr [ESP]
009e0489  LEA ECX,[ESP + 0x68]
009e048d  FLD float ptr [ESP + 0x28]
009e0491  PUSH ECX
009e0492  LEA EDX,[ESP + 0x54]
009e0496  PUSH EDX
009e0497  SUB ESP,0x8
009e049a  FSTP float ptr [ESP + 0x4]
009e049e  LEA EAX,[ESP + 0x6c]
009e04a2  FLD float ptr [ESP + 0x3c]
009e04a6  FSTP float ptr [ESP]
009e04a9  PUSH EAX
009e04aa  FLD float ptr [ESP + 0xb4]
009e04b1  PUSH ECX
009e04b2  MOV ECX,ESI
009e04b4  FSTP float ptr [ESP]
009e04b7  CALL 0x00a2f120
009e04bc  XOR EDI,EDI
009e04be  LEA EBX,[ESI + 0x2e8]
009e04c4  MOV ECX,EBX
009e04c6  MOV dword ptr [ESP + 0x2c],EDI
009e04ca  MOV dword ptr [ESP + 0x18],EDI
009e04ce  CALL 0x009d5a40
009e04d3  MOV EBP,EAX
009e04d5  TEST EBP,EBP
009e04d7  JBE 0x009e0579
009e04dd  LEA ECX,[ECX]
009e04e0  PUSH EDI
009e04e1  MOV ECX,EBX
009e04e3  CALL 0x006f1300
009e04e8  CMP dword ptr [EAX + 0x124],0x0
009e04ef  JZ 0x009e0503
009e04f1  CMP dword ptr [EAX],0x0
009e04f4  MOV ECX,0x1
009e04f9  MOV dword ptr [ESP + 0x18],ECX
009e04fd  JZ 0x009e0503
009e04ff  MOV dword ptr [ESP + 0x2c],ECX
009e0503  ADD EDI,0x1
009e0506  CMP EDI,EBP
009e0508  JC 0x009e04e0
009e050a  CMP dword ptr [ESP + 0x18],0x0
009e050f  JZ 0x009e053e
009e0511  MOV EAX,dword ptr [ESI + 0x64]
009e0514  MOV ECX,dword ptr [EAX + 0x24]
009e0517  PUSH ECX
009e0518  LEA ECX,[EAX + 0x14]
009e051b  CALL 0x009bc110
009e0520  FLD float ptr [ESI + 0x54]
009e0523  MOV EDX,dword ptr [EAX]
009e0525  FCHS
009e0527  FMUL float ptr [ESI + 0x5a8]
009e052d  FLD float ptr [EDX + 0x58]
009e0530  FMUL float ptr [ESI + 0x5ac]
009e0536  FSUBP
009e0538  FSTP float ptr [ESI + 0x5b0]
009e053e  CMP dword ptr [ESP + 0x2c],0x0
009e0543  JZ 0x009e0579
009e0545  MOV EAX,dword ptr [ESI + 0x64]
009e0548  MOV ECX,dword ptr [EAX + 0x24]
009e054b  PUSH ECX
009e054c  LEA ECX,[EAX + 0x14]
009e054f  CALL 0x009bc110
009e0554  FLDZ
009e0556  MOV EDX,dword ptr [EAX]
009e0558  FCOM float ptr [EDX + 0xa4]
009e055e  FNSTSW AX
009e0560  TEST AH,0x41
009e0563  JNZ 0x009e0577
009e0565  LEA EAX,[ESP + 0x3c]
009e0569  FSTP float ptr [ESP + 0x3c]
009e056d  PUSH EAX
009e056e  MOV ECX,ESI
009e0570  CALL 0x009d7540
009e0575  JMP 0x009e0579
009e0577  FSTP ST0
009e0579  MOV ECX,dword ptr [ESP + 0x2c]
009e057d  MOV EDI,dword ptr [ESP + 0x30]
009e0581  MOV EBP,dword ptr [ESP + 0x34]
009e0585  PUSH ECX
009e0586  PUSH EDI
009e0587  PUSH EBP
009e0588  LEA EDX,[ESP + 0x60]
009e058c  PUSH EDX
009e058d  MOV ECX,ESI
009e058f  CALL 0x009d83a0
009e0594  FLD float ptr [0x00d1453c]
009e059a  FCOMP float ptr [ESI + 0x5c]
009e059d  FNSTSW AX
009e059f  TEST AH,0x5
009e05a2  JP 0x009e06fe
009e05a8  MOV EAX,dword ptr [ESI + 0x74c]
009e05ae  SUB EAX,0x1
009e05b1  JZ 0x009e0610
009e05b3  SUB EAX,0x1
009e05b6  JZ 0x009e05ed
009e05b8  SUB EAX,0x1
009e05bb  JNZ 0x009e06fe
009e05c1  MOV EAX,dword ptr [ESI + 0x64]
009e05c4  MOV dword ptr [ESI + 0x600],0x1
009e05ce  MOV ECX,dword ptr [EAX + 0x24]
009e05d1  PUSH ECX
009e05d2  LEA ECX,[EAX + 0x14]
009e05d5  CALL 0x009bc110
009e05da  MOV EDX,dword ptr [EAX]
009e05dc  FLD float ptr [EDX + 0xf0]
009e05e2  FSTP float ptr [ESI + 0x5f4]
009e05e8  JMP 0x009e06fe
009e05ed  MOV EAX,dword ptr [ESI + 0x64]
009e05f0  MOV ECX,dword ptr [EAX + 0x24]
009e05f3  PUSH ECX
009e05f4  LEA ECX,[EAX + 0x14]
009e05f7  CALL 0x009bc110
009e05fc  MOV EDX,dword ptr [EAX]
009e05fe  FLD float ptr [EDX + 0x108]
009e0604  MOV EAX,dword ptr [ESP + 0x38]
009e0608  FSTP float ptr [EAX + 0x34]
009e060b  JMP 0x009e06fe
009e0610  TEST EDI,EDI
009e0612  JZ 0x009e06fe
009e0618  CMP EBP,dword ptr [ESI + 0x610]
009e061e  JBE 0x009e06fe
009e0624  FLD float ptr [ESP + 0x48]
009e0628  FCHS
009e062a  FSTP float ptr [ESP + 0x18]
009e062e  FLD float ptr [ESP + 0x4c]
009e0632  FCHS
009e0634  FSTP float ptr [ESP + 0x1c]
009e0638  FLD float ptr [ESP + 0x50]
009e063c  FCHS
009e063e  FSTP float ptr [ESP + 0x20]
009e0642  FLD float ptr [ESP + 0x18]
009e0646  FLD float ptr [ESP + 0x1c]
009e064a  FLD float ptr [ESP + 0x20]
009e064e  FLD ST1
009e0650  FMULP ST2
009e0652  FLD ST2
009e0654  FMULP ST3
009e0656  FXCH
009e0658  FADDP ST2,ST0
009e065a  FMUL ST0
009e065c  FADDP
009e065e  FSTP float ptr [ESP + 0x38]
009e0662  FLD float ptr [ESP + 0x38]
009e0666  FLD float ptr [0x00e89bb8]
009e066c  FCOMP
009e066e  FNSTSW AX
009e0670  TEST AH,0x5
009e0673  JP 0x009e06ac
009e0675  CALL 0x007e8520
009e067a  FSTP float ptr [ESP + 0x38]
009e067e  FLD float ptr [ESP + 0x38]
009e0682  FLD1
009e0684  FDIVRP
009e0686  FSTP float ptr [ESP + 0x38]
009e068a  FLD float ptr [ESP + 0x38]
009e068e  FLD ST0
009e0690  FMUL float ptr [ESP + 0x18]
009e0694  FSTP float ptr [ESP + 0x18]
009e0698  FLD float ptr [ESP + 0x1c]
009e069c  FMUL ST1
009e069e  FSTP float ptr [ESP + 0x1c]
009e06a2  FMUL float ptr [ESP + 0x20]
009e06a6  FSTP float ptr [ESP + 0x20]
009e06aa  JMP 0x009e06ae
009e06ac  FSTP ST0
009e06ae  MOV EAX,dword ptr [ESI + 0x64]
009e06b1  MOV ECX,dword ptr [EAX + 0x24]
009e06b4  PUSH ECX
009e06b5  LEA ECX,[EAX + 0x14]
009e06b8  CALL 0x009bc110
009e06bd  MOV EDX,dword ptr [EAX]
009e06bf  FLD float ptr [EDX + 0x104]
009e06c5  LEA EAX,[ESP + 0x18]
009e06c9  FSTP float ptr [ESP + 0x38]
009e06cd  PUSH EAX
009e06ce  FLD float ptr [ESP + 0x3c]
009e06d2  MOV ECX,ESI
009e06d4  FLD ST0
009e06d6  FMUL float ptr [ESP + 0x1c]
009e06da  FSTP float ptr [ESP + 0x1c]
009e06de  FLD float ptr [ESP + 0x20]
009e06e2  FMUL ST1
009e06e4  FSTP float ptr [ESP + 0x20]
009e06e8  FMUL float ptr [ESP + 0x24]
009e06ec  FSTP float ptr [ESP + 0x24]
009e06f0  CALL 0x009d7860
009e06f5  LEA ECX,[EBP + 0x64]
009e06f8  MOV dword ptr [ESI + 0x610],ECX
009e06fe  MOV EDI,dword ptr [ESI + 0x64]
009e0701  MOV EDX,dword ptr [EDI + 0x24]
009e0704  PUSH EDX
009e0705  LEA ECX,[EDI + 0x14]
009e0708  CALL 0x009bc110
009e070d  FLD float ptr [ESI + 0x670]
009e0713  MOV EAX,dword ptr [EAX]
009e0715  FLD float ptr [EAX + 0x39c]
009e071b  FCOMPP
009e071d  FNSTSW AX
009e071f  TEST AH,0x5
009e0722  JP 0x009e0769
009e0724  MOV ECX,dword ptr [EDI + 0x24]
009e0727  PUSH ECX
009e0728  LEA ECX,[EDI + 0x14]
009e072b  CALL 0x009bc110
009e0730  FLD float ptr [ESI + 0x670]
009e0736  MOV EDX,dword ptr [EAX]
009e0738  FLD float ptr [EDX + 0x398]
009e073e  FCOMPP
009e0740  FNSTSW AX
009e0742  TEST AH,0x5
009e0745  MOV EAX,dword ptr [ESI + 0x654]
009e074b  JP 0x009e0759
009e074d  CMP EAX,0x2
009e0750  JNC 0x009e0763
009e0752  MOV EAX,0x2
009e0757  JMP 0x009e0763
009e0759  CMP EAX,0x1
009e075c  JNC 0x009e0763
009e075e  MOV EAX,0x1
009e0763  MOV dword ptr [ESI + 0x654],EAX
009e0769  MOV EAX,dword ptr [EDI + 0x24]
009e076c  PUSH EAX
009e076d  LEA ECX,[EDI + 0x14]
009e0770  CALL 0x009bc110
009e0775  FLD float ptr [ESI + 0x674]
009e077b  MOV ECX,dword ptr [EAX]
009e077d  FLD float ptr [ECX + 0x39c]
009e0783  FCOMPP
009e0785  FNSTSW AX
009e0787  TEST AH,0x5
009e078a  JP 0x009e07d1
009e078c  MOV EDX,dword ptr [EDI + 0x24]
009e078f  PUSH EDX
009e0790  LEA ECX,[EDI + 0x14]
009e0793  CALL 0x009bc110
009e0798  FLD float ptr [ESI + 0x674]
009e079e  MOV EAX,dword ptr [EAX]
009e07a0  FLD float ptr [EAX + 0x398]
009e07a6  FCOMPP
009e07a8  FNSTSW AX
009e07aa  TEST AH,0x5
009e07ad  MOV EAX,dword ptr [ESI + 0x658]
009e07b3  JP 0x009e07c1
009e07b5  CMP EAX,0x2
009e07b8  JNC 0x009e07cb
009e07ba  MOV EAX,0x2
009e07bf  JMP 0x009e07cb
009e07c1  CMP EAX,0x1
009e07c4  JNC 0x009e07cb
009e07c6  MOV EAX,0x1
009e07cb  MOV dword ptr [ESI + 0x658],EAX
009e07d1  MOV ECX,dword ptr [EDI + 0x24]
009e07d4  PUSH ECX
009e07d5  LEA ECX,[EDI + 0x14]
009e07d8  CALL 0x009bc110
009e07dd  FLD float ptr [ESI + 0x678]
009e07e3  MOV EDX,dword ptr [EAX]
009e07e5  FLD float ptr [EDX + 0x3a0]
009e07eb  FCOMPP
009e07ed  FNSTSW AX
009e07ef  TEST AH,0x5
009e07f2  JP 0x009e0836
009e07f4  MOV EAX,dword ptr [EDI + 0x24]
009e07f7  PUSH EAX
009e07f8  LEA ECX,[EDI + 0x14]
009e07fb  CALL 0x009bc110
009e0800  FLD float ptr [ESI + 0x678]
009e0806  MOV ECX,dword ptr [EAX]
009e0808  FLD float ptr [ECX + 0x28]
009e080b  FCOMPP
009e080d  FNSTSW AX
009e080f  TEST AH,0x5
009e0812  MOV EAX,dword ptr [ESI + 0x1fc]
009e0818  JP 0x009e0826
009e081a  CMP EAX,0x2
009e081d  JNC 0x009e0830
009e081f  MOV EAX,0x2
009e0824  JMP 0x009e0830
009e0826  CMP EAX,0x1
009e0829  JNC 0x009e0830
009e082b  MOV EAX,0x1
009e0830  MOV dword ptr [ESI + 0x1fc],EAX
009e0836  MOV EAX,dword ptr [ESI + 0x658]
009e083c  CMP EAX,dword ptr [ESI + 0x660]
009e0842  JBE 0x009e0856
009e0844  MOV DL,byte ptr [ESI + 0x201]
009e084a  MOV dword ptr [ESI + 0x660],EAX
009e0850  MOV byte ptr [ESI + 0x66c],DL
009e0856  MOV EAX,dword ptr [ESI + 0x654]
009e085c  CMP EAX,dword ptr [ESI + 0x664]
009e0862  JBE 0x009e0876
009e0864  MOV dword ptr [ESI + 0x664],EAX
009e086a  MOV AL,byte ptr [ESI + 0x201]
009e0870  MOV byte ptr [ESI + 0x66c],AL
009e0876  MOV EAX,dword ptr [ESI + 0x1fc]
009e087c  CMP EAX,dword ptr [ESI + 0x668]
009e0882  JBE 0x009e0896
009e0884  MOV CL,byte ptr [ESI + 0x200]
009e088a  MOV dword ptr [ESI + 0x668],EAX
009e0890  MOV byte ptr [ESI + 0x66d],CL
009e0896  LEA EDX,[ESP + 0x10]
009e089a  PUSH EDX
009e089b  LEA EAX,[ESP + 0x1c]
009e089f  PUSH EAX
009e08a0  PUSH 0x7
009e08a2  MOV ECX,ESI
009e08a4  MOV dword ptr [ESI + 0x650],EBP
009e08aa  CALL 0x009d87f0
009e08af  TEST EAX,EAX
009e08b1  JZ 0x009e08fd
009e08b3  MOV ECX,dword ptr [EDI + 0x24]
009e08b6  PUSH ECX
009e08b7  LEA ECX,[EDI + 0x14]
009e08ba  CALL 0x009bc110
009e08bf  MOV EDX,dword ptr [EDI + 0x24]
009e08c2  PUSH EDX
009e08c3  LEA ECX,[EDI + 0x14]
009e08c6  MOV EBX,EAX
009e08c8  CALL 0x009bc110
009e08cd  MOV EDX,dword ptr [EBX]
009e08cf  FLD float ptr [EDX + 0xf0]
009e08d5  MOV ECX,dword ptr [ESP + 0x10]
009e08d9  FMUL float ptr [ESP + 0x20]
009e08dd  MOV EAX,dword ptr [EAX]
009e08df  PUSH ECX
009e08e0  PUSH 0x1
009e08e2  PUSH ECX
009e08e3  FSTP float ptr [ESP + 0x44]
009e08e7  MOV ECX,dword ptr [EAX + 0xf8]
009e08ed  FLD float ptr [ESP + 0x44]
009e08f1  FSTP float ptr [ESP]
009e08f4  PUSH ECX
009e08f5  PUSH EBP
009e08f6  MOV ECX,ESI
009e08f8  CALL 0x009d5f50
009e08fd  LEA EDX,[ESP + 0x10]
009e0901  PUSH EDX
009e0902  LEA EAX,[ESP + 0x1c]
009e0906  PUSH EAX
009e0907  PUSH 0x1a
009e0909  MOV ECX,ESI
009e090b  CALL 0x009d87f0
009e0910  TEST EAX,EAX
009e0912  JZ 0x009e0961
009e0914  MOV EDI,dword ptr [ESI + 0x64]
009e0917  MOV ECX,dword ptr [EDI + 0x24]
009e091a  PUSH ECX
009e091b  LEA ECX,[EDI + 0x14]
009e091e  CALL 0x009bc110
009e0923  MOV EDX,dword ptr [EDI + 0x24]
009e0926  PUSH EDX
009e0927  LEA ECX,[EDI + 0x14]
009e092a  MOV EBX,EAX
009e092c  CALL 0x009bc110
009e0931  MOV EDX,dword ptr [EBX]
009e0933  FLD float ptr [EDX + 0xf4]
009e0939  MOV ECX,dword ptr [ESP + 0x10]
009e093d  FMUL float ptr [ESP + 0x20]
009e0941  MOV EAX,dword ptr [EAX]
009e0943  PUSH ECX
009e0944  PUSH 0x1
009e0946  PUSH ECX
009e0947  FSTP float ptr [ESP + 0x44]
009e094b  MOV ECX,dword ptr [EAX + 0xfc]
009e0951  FLD float ptr [ESP + 0x44]
009e0955  FSTP float ptr [ESP]
009e0958  PUSH ECX
009e0959  PUSH EBP
009e095a  MOV ECX,ESI
009e095c  CALL 0x009d5f50
009e0961  LEA EDX,[ESP + 0x10]
009e0965  PUSH EDX
009e0966  LEA EAX,[ESP + 0x1c]
009e096a  PUSH EAX
009e096b  PUSH 0x1e
009e096d  MOV ECX,ESI
009e096f  CALL 0x009d87f0
009e0974  TEST EAX,EAX
009e0976  JZ 0x009e09c5
009e0978  MOV EDI,dword ptr [ESI + 0x64]
009e097b  MOV ECX,dword ptr [EDI + 0x24]
009e097e  PUSH ECX
009e097f  LEA ECX,[EDI + 0x14]
009e0982  CALL 0x009bc110
009e0987  MOV EDX,dword ptr [EDI + 0x24]
009e098a  PUSH EDX
009e098b  LEA ECX,[EDI + 0x14]
009e098e  MOV EBX,EAX
009e0990  CALL 0x009bc110
009e0995  MOV EDX,dword ptr [EBX]
009e0997  FLD float ptr [EDX + 0xf0]
009e099d  MOV ECX,dword ptr [ESP + 0x10]
009e09a1  FMUL float ptr [ESP + 0x20]
009e09a5  MOV EAX,dword ptr [EAX]
009e09a7  PUSH ECX
009e09a8  PUSH 0x2
009e09aa  PUSH ECX
009e09ab  FSTP float ptr [ESP + 0x44]
009e09af  MOV ECX,dword ptr [EAX + 0xf8]
009e09b5  FLD float ptr [ESP + 0x44]
009e09b9  FSTP float ptr [ESP]
009e09bc  PUSH ECX
009e09bd  PUSH EBP
009e09be  MOV ECX,ESI
009e09c0  CALL 0x009d5f50
009e09c5  LEA EDX,[ESP + 0x10]
009e09c9  PUSH EDX
009e09ca  LEA EAX,[ESP + 0x1c]
009e09ce  PUSH EAX
009e09cf  PUSH 0x1d
009e09d1  MOV ECX,ESI
009e09d3  CALL 0x009d87f0
009e09d8  TEST EAX,EAX
009e09da  JZ 0x009e09e6
009e09dc  MOV dword ptr [ESI + 0x60c],0x1
009e09e6  PUSH EBP
009e09e7  MOV ECX,ESI
009e09e9  CALL 0x009d5420
009e09ee  MOV EDX,dword ptr [ESI]
009e09f0  MOV EAX,dword ptr [EDX + 0x1a8]
009e09f6  CALL EAX
009e09f8  MOV ECX,dword ptr [ESI + 0x28]
009e09fb  LEA EDI,[ESI + 0x6d4]
009e0a01  PUSH EDI
009e0a02  CALL 0x0067ab30
009e0a07  MOV EAX,dword ptr [ESI + 0x64]
009e0a0a  MOV ECX,dword ptr [EAX + 0x24]
009e0a0d  PUSH ECX
009e0a0e  LEA ECX,[EAX + 0x14]
009e0a11  CALL 0x009bc110
009e0a16  MOV EDX,dword ptr [EAX]
009e0a18  FLD float ptr [EDX + 0x228]
009e0a1e  FLD1
009e0a20  FDIVRP
009e0a22  FSTP float ptr [ESP + 0x38]
009e0a26  FLD float ptr [EDI]
009e0a28  FLD float ptr [ESP + 0x38]
009e0a2c  FLD ST0
009e0a2e  FMULP ST2
009e0a30  FXCH
009e0a32  FSTP float ptr [EDI]
009e0a34  FLD ST0
009e0a36  FMUL float ptr [EDI + 0x4]
009e0a39  FSTP float ptr [EDI + 0x4]
009e0a3c  FMUL float ptr [EDI + 0x8]
009e0a3f  FSTP float ptr [EDI + 0x8]
009e0a42  MOV EDI,dword ptr [ESI + 0x64]
009e0a45  MOV EAX,dword ptr [EDI + 0x24]
009e0a48  PUSH EAX
009e0a49  LEA ECX,[EDI + 0x14]
009e0a4c  CALL 0x009bc110
009e0a51  MOV ECX,dword ptr [EAX]
009e0a53  FLD float ptr [ECX + 0x384]
009e0a59  MOV EDX,dword ptr [EDI + 0x24]
009e0a5c  PUSH EDX
009e0a5d  FSTP float ptr [ESP + 0x38]
009e0a61  LEA ECX,[EDI + 0x14]
009e0a64  CALL 0x009bc110
009e0a69  FLD float ptr [ESP + 0x24]
009e0a6d  MOV EAX,dword ptr [EAX]
009e0a6f  MOV ECX,dword ptr [EAX + 0x380]
009e0a75  PUSH 0x0
009e0a77  PUSH ECX
009e0a78  FSTP float ptr [ESP]
009e0a7b  CALL 0x00643a80
009e0a80  FSTP float ptr [ESP + 0x38]
009e0a84  FLD float ptr [ESP + 0x38]
009e0a88  FADD float ptr [ESP + 0x34]
009e0a8c  FLD float ptr [ESP + 0x8c]
009e0a93  FLD ST0
009e0a95  FMULP ST2
009e0a97  FLD float ptr [ESI + 0x624]
009e0a9d  FADDP ST2,ST0
009e0a9f  FXCH
009e0aa1  FSTP float ptr [ESP + 0x38]
009e0aa5  FLD float ptr [ESP + 0x38]
009e0aa9  FST float ptr [ESI + 0x624]
009e0aaf  FSTP float ptr [ESP + 0x38]
009e0ab3  FLDZ
009e0ab5  FLD float ptr [ESP + 0x38]
009e0ab9  FCOM
009e0abb  FNSTSW AX
009e0abd  TEST AH,0x41
009e0ac0  JNP 0x009e0ad3
009e0ac2  FSTP ST1
009e0ac4  FLD1
009e0ac6  FCOM
009e0ac8  FNSTSW AX
009e0aca  TEST AH,0x41
009e0acd  JP 0x009e0ad3
009e0acf  FSTP ST1
009e0ad1  JMP 0x009e0ad5
009e0ad3  FSTP ST0
009e0ad5  FSTP float ptr [ESP + 0x2c]
009e0ad9  PUSH ECX
009e0ada  FLD float ptr [ESP + 0x30]
009e0ade  LEA ECX,[ESI + 0x228]
009e0ae4  FSTP float ptr [ESI + 0x624]
009e0aea  FSTP float ptr [ESP]
009e0aed  CALL 0x00baacd0
009e0af2  FLD float ptr [ESP + 0x60]
009e0af6  FLD float ptr [ESP + 0x8c]
009e0afd  FLD ST0
009e0aff  FMULP ST2
009e0b01  FLD float ptr [ESP + 0x6c]
009e0b05  FADDP ST2,ST0
009e0b07  FXCH
009e0b09  FSTP float ptr [ESP + 0x38]
009e0b0d  FLD float ptr [ESI + 0x244]
009e0b13  FCHS
009e0b15  FSTP float ptr [ESP + 0x34]
009e0b19  FLD float ptr [ESI + 0x244]
009e0b1f  FSTP float ptr [ESP + 0x30]
009e0b23  FLD float ptr [ESP + 0x38]
009e0b27  FLD float ptr [ESP + 0x34]
009e0b2b  FCOM
009e0b2d  FNSTSW AX
009e0b2f  TEST AH,0x1
009e0b32  JNZ 0x009e0b38
009e0b34  FSTP ST1
009e0b36  JMP 0x009e0b4d
009e0b38  FSTP ST0
009e0b3a  FLD float ptr [ESP + 0x30]
009e0b3e  FCOM
009e0b40  FNSTSW AX
009e0b42  TEST AH,0x41
009e0b45  JP 0x009e0b4b
009e0b47  FSTP ST1
009e0b49  JMP 0x009e0b4d
009e0b4b  FSTP ST0
009e0b4d  FSTP float ptr [ESP + 0x2c]
009e0b51  FLD float ptr [ESP + 0x2c]
009e0b55  FDIV float ptr [ESI + 0x244]
009e0b5b  FMUL float ptr [ESI + 0x24c]
009e0b61  FSTP float ptr [ESP + 0x28]
009e0b65  FLD float ptr [ESI + 0x230]
009e0b6b  FSTP float ptr [ESP + 0x38]
009e0b6f  FLD float ptr [ESI + 0x250]
009e0b75  FCHS
009e0b77  FSTP float ptr [ESP + 0x34]
009e0b7b  FLD float ptr [ESI + 0x250]
009e0b81  FSTP float ptr [ESP + 0x30]
009e0b85  FLD float ptr [ESP + 0x38]
009e0b89  FLD float ptr [ESP + 0x34]
009e0b8d  FCOM
009e0b8f  FNSTSW AX
009e0b91  TEST AH,0x1
009e0b94  JNZ 0x009e0b9a
009e0b96  FSTP ST1
009e0b98  JMP 0x009e0baf
009e0b9a  FSTP ST0
009e0b9c  FLD float ptr [ESP + 0x30]
009e0ba0  FCOM
009e0ba2  FNSTSW AX
009e0ba4  TEST AH,0x41
009e0ba7  JP 0x009e0bad
009e0ba9  FSTP ST1
009e0bab  JMP 0x009e0baf
009e0bad  FSTP ST0
009e0baf  FSTP float ptr [ESP + 0x2c]
009e0bb3  FLD float ptr [ESP + 0x2c]
009e0bb7  FSTP float ptr [ESI + 0x230]
009e0bbd  FLD float ptr [ESI + 0x238]
009e0bc3  FADD float ptr [ESP + 0x28]
009e0bc7  FSTP float ptr [ESP + 0x38]
009e0bcb  FLD float ptr [ESI + 0x24c]
009e0bd1  FCHS
009e0bd3  FSTP float ptr [ESP + 0x34]
009e0bd7  FLD float ptr [ESI + 0x24c]
009e0bdd  FSTP float ptr [ESP + 0x30]
009e0be1  FLD float ptr [ESP + 0x38]
009e0be5  FLD float ptr [ESP + 0x34]
009e0be9  FCOM
009e0beb  FNSTSW AX
009e0bed  TEST AH,0x1
009e0bf0  JNZ 0x009e0bf6
009e0bf2  FSTP ST1
009e0bf4  JMP 0x009e0c0b
009e0bf6  FSTP ST0
009e0bf8  FLD float ptr [ESP + 0x30]
009e0bfc  FCOM
009e0bfe  FNSTSW AX
009e0c00  TEST AH,0x41
009e0c03  JP 0x009e0c09
009e0c05  FSTP ST1
009e0c07  JMP 0x009e0c0b
009e0c09  FSTP ST0
009e0c0b  FSTP float ptr [ESP + 0x2c]
009e0c0f  PUSH ECX
009e0c10  FLD float ptr [ESP + 0x30]
009e0c14  LEA ECX,[ESI + 0x214]
009e0c1a  FSTP float ptr [ESI + 0x238]
009e0c20  FSTP float ptr [ESP]
009e0c23  CALL 0x00baacd0
009e0c28  FLD float ptr [ESP + 0x68]
009e0c2c  FCHS
009e0c2e  FMUL float ptr [ESP + 0x8c]
009e0c35  FSUB float ptr [ESP + 0x74]
009e0c39  FSTP float ptr [ESP + 0x38]
009e0c3d  FLD float ptr [ESI + 0x244]
009e0c43  FCHS
009e0c45  FSTP float ptr [ESP + 0x34]
009e0c49  FLD float ptr [ESI + 0x244]
009e0c4f  FSTP float ptr [ESP + 0x30]
009e0c53  FLD float ptr [ESP + 0x38]
009e0c57  FLD float ptr [ESP + 0x34]
009e0c5b  FCOM
009e0c5d  FNSTSW AX
009e0c5f  TEST AH,0x1
009e0c62  JNZ 0x009e0c68
009e0c64  FSTP ST1
009e0c66  JMP 0x009e0c7d
009e0c68  FSTP ST0
009e0c6a  FLD float ptr [ESP + 0x30]
009e0c6e  FCOM
009e0c70  FNSTSW AX
009e0c72  TEST AH,0x41
009e0c75  JP 0x009e0c7b
009e0c77  FSTP ST1
009e0c79  JMP 0x009e0c7d
009e0c7b  FSTP ST0
009e0c7d  FSTP float ptr [ESP + 0x2c]
009e0c81  FLD float ptr [ESP + 0x2c]
009e0c85  FDIV float ptr [ESI + 0x244]
009e0c8b  FMUL float ptr [ESI + 0x24c]
009e0c91  FSTP float ptr [ESP + 0x28]
009e0c95  FLD float ptr [ESI + 0x21c]
009e0c9b  FSTP float ptr [ESP + 0x38]
009e0c9f  FLD float ptr [ESI + 0x250]
009e0ca5  FCHS
009e0ca7  FSTP float ptr [ESP + 0x34]
009e0cab  FLD float ptr [ESI + 0x250]
009e0cb1  FSTP float ptr [ESP + 0x30]
009e0cb5  FLD float ptr [ESP + 0x38]
009e0cb9  FLD float ptr [ESP + 0x34]
009e0cbd  FCOM
009e0cbf  FNSTSW AX
009e0cc1  TEST AH,0x1
009e0cc4  JNZ 0x009e0cca
009e0cc6  FSTP ST1
009e0cc8  JMP 0x009e0cdf
009e0cca  FSTP ST0
009e0ccc  FLD float ptr [ESP + 0x30]
009e0cd0  FCOM
009e0cd2  FNSTSW AX
009e0cd4  TEST AH,0x41
009e0cd7  JP 0x009e0cdd
009e0cd9  FSTP ST1
009e0cdb  JMP 0x009e0cdf
009e0cdd  FSTP ST0
009e0cdf  FSTP float ptr [ESP + 0x2c]
009e0ce3  FLD float ptr [ESP + 0x2c]
009e0ce7  FSTP float ptr [ESI + 0x21c]
009e0ced  FLD float ptr [ESI + 0x224]
009e0cf3  FADD float ptr [ESP + 0x28]
009e0cf7  FSTP float ptr [ESP + 0x38]
009e0cfb  FLD float ptr [ESI + 0x24c]
009e0d01  FCHS
009e0d03  FSTP float ptr [ESP + 0x34]
009e0d07  FLD float ptr [ESI + 0x24c]
009e0d0d  FSTP float ptr [ESP + 0x30]
009e0d11  FLD float ptr [ESP + 0x38]
009e0d15  FLD float ptr [ESP + 0x34]
009e0d19  FCOM
009e0d1b  FNSTSW AX
009e0d1d  TEST AH,0x1
009e0d20  JNZ 0x009e0d26
009e0d22  FSTP ST1
009e0d24  JMP 0x009e0d3b
009e0d26  FSTP ST0
009e0d28  FLD float ptr [ESP + 0x30]
009e0d2c  FCOM
009e0d2e  FNSTSW AX
009e0d30  TEST AH,0x41
009e0d33  JP 0x009e0d39
009e0d35  FSTP ST1
009e0d37  JMP 0x009e0d3b
009e0d39  FSTP ST0
009e0d3b  FSTP float ptr [ESP + 0x2c]
009e0d3f  PUSH 0x6
009e0d41  FLD float ptr [ESP + 0x30]
009e0d45  MOV ECX,ESI
009e0d47  FSTP float ptr [ESI + 0x224]
009e0d4d  CALL 0x009d8790
009e0d52  TEST EAX,EAX
009e0d54  JZ 0x009e0d5a
009e0d56  FLD1
009e0d58  JMP 0x009e0d60
009e0d5a  FLD float ptr [0x00c75064]
009e0d60  MOV ECX,dword ptr [ESI + 0x60]
009e0d63  FSTP float ptr [ESP + 0x28]
009e0d67  FLD float ptr [ESP + 0x44]
009e0d6b  MOV ECX,dword ptr [ECX + 0x48]
009e0d6e  FMUL double ptr [0x00c82db8]
009e0d74  PUSH 0x0
009e0d76  PUSH ECX
009e0d77  FSTP float ptr [ESP + 0x40]
009e0d7b  FLD float ptr [ESP + 0x40]
009e0d7f  FABS
009e0d81  FSTP float ptr [ESP + 0x40]
009e0d85  FLD float ptr [ESP + 0x40]
009e0d89  FSTP float ptr [ESP + 0x40]
009e0d8d  FLD float ptr [ESP + 0x40]
009e0d91  FSTP float ptr [ESP]
009e0d94  CALL 0x00643a80
009e0d99  FSTP float ptr [ESP + 0x38]
009e0d9d  FLD float ptr [ESP + 0x38]
009e0da1  FMUL float ptr [ESP + 0x8c]
009e0da8  FMUL float ptr [ESP + 0x28]
009e0dac  FADD float ptr [ESI + 0x23c]
009e0db2  FSTP float ptr [ESP + 0x38]
009e0db6  FLDZ
009e0db8  FLD float ptr [ESP + 0x38]
009e0dbc  FCOM
009e0dbe  FNSTSW AX
009e0dc0  TEST AH,0x41
009e0dc3  JNP 0x009e0dd6
009e0dc5  FSTP ST1
009e0dc7  FLD1
009e0dc9  FCOM
009e0dcb  FNSTSW AX
009e0dcd  TEST AH,0x41
009e0dd0  JP 0x009e0dd6
009e0dd2  FSTP ST1
009e0dd4  JMP 0x009e0dd8
009e0dd6  FSTP ST0
009e0dd8  MOV EDX,dword ptr [ESI + 0x60]
009e0ddb  FSTP float ptr [ESP + 0x2c]
009e0ddf  FLD float ptr [ESP + 0x2c]
009e0de3  PUSH 0x0
009e0de5  FSTP float ptr [ESI + 0x23c]
009e0deb  MOV ECX,dword ptr [EDX + 0x44]
009e0dee  FLD float ptr [ESP + 0x48]
009e0df2  PUSH ECX
009e0df3  FMUL double ptr [0x00c82db8]
009e0df9  FSTP float ptr [ESP + 0x40]
009e0dfd  FLD float ptr [ESP + 0x40]
009e0e01  FABS
009e0e03  FSTP float ptr [ESP + 0x40]
009e0e07  FLD float ptr [ESP + 0x40]
009e0e0b  FSTP float ptr [ESP + 0x40]
009e0e0f  FLD float ptr [ESP + 0x40]
009e0e13  FSTP float ptr [ESP]
009e0e16  CALL 0x00643a80
009e0e1b  FSTP float ptr [ESP + 0x38]
009e0e1f  FLD float ptr [ESP + 0x38]
009e0e23  FMUL float ptr [ESP + 0x8c]
009e0e2a  FMUL float ptr [ESP + 0x28]
009e0e2e  FADD float ptr [ESI + 0x240]
009e0e34  FSTP float ptr [ESP + 0x38]
009e0e38  FLDZ
009e0e3a  FLD float ptr [ESP + 0x38]
009e0e3e  FCOM
009e0e40  FNSTSW AX
009e0e42  TEST AH,0x41
009e0e45  JNP 0x009e0e58
009e0e47  FSTP ST1
009e0e49  FLD1
009e0e4b  FCOM
009e0e4d  FNSTSW AX
009e0e4f  TEST AH,0x41
009e0e52  JP 0x009e0e58
009e0e54  FSTP ST1
009e0e56  JMP 0x009e0e5a
009e0e58  FSTP ST0
009e0e5a  FSTP float ptr [ESP + 0x2c]
009e0e5e  LEA EBX,[ESI + 0x2e8]
009e0e64  FLD float ptr [ESP + 0x2c]
009e0e68  MOV ECX,EBX
009e0e6a  FSTP float ptr [ESI + 0x240]
009e0e70  CALL 0x009d5a40
009e0e75  MOV EBP,EAX
009e0e77  XOR EDI,EDI
009e0e79  CMP EBP,EDI
009e0e7b  JA 0x009e0eb2
009e0e7d  FLDZ
009e0e7f  FST float ptr [ESI + 0x670]
009e0e85  MOV dword ptr [ESI + 0x5d8],EDI
009e0e8b  FST float ptr [ESI + 0x674]
009e0e91  MOV dword ptr [ESI + 0x5d4],EDI
009e0e97  FSTP float ptr [ESI + 0x678]
009e0e9d  MOV dword ptr [ESI + 0x5dc],EDI
009e0ea3  POP EDI
009e0ea4  POP EBX
009e0ea5  POP ESI
009e0ea6  POP EBP
009e0ea7  ADD ESP,0x78
009e0eaa  RET 0x4
009e0eb0  FSTP ST0
009e0eb2  PUSH EDI
009e0eb3  MOV ECX,EBX
009e0eb5  CALL 0x006f1300
009e0eba  FLDZ
009e0ebc  XOR ECX,ECX
009e0ebe  MOV dword ptr [EAX + 0x124],ECX
009e0ec4  MOV dword ptr [EAX + 0x140],ECX
009e0eca  FST float ptr [EAX + 0x110]
009e0ed0  FST float ptr [EAX + 0x10c]
009e0ed6  ADD EDI,0x1
009e0ed9  CMP EDI,EBP
009e0edb  FST float ptr [EAX + 0x108]
009e0ee1  FST float ptr [EAX + 0x14c]
009e0ee7  FST float ptr [EAX + 0x148]
009e0eed  FST float ptr [EAX + 0x144]
009e0ef3  MOV word ptr [EAX + 0x128],CX
009e0efa  MOV dword ptr [EAX + 0x15c],ECX
009e0f00  JC 0x009e0eb0
009e0f02  XOR EDI,EDI
009e0f04  JMP 0x009e0e7f
009e0f09  MOV ECX,dword ptr [ESI + 0x28]
009e0f0c  FST float ptr [ESP + 0x64]
009e0f10  LEA EAX,[ESP + 0x64]
009e0f14  FST float ptr [ESP + 0x68]
009e0f18  PUSH EAX
009e0f19  FSTP float ptr [ESP + 0x70]
009e0f1d  CALL 0x0067bf60
009e0f22  FLDZ
009e0f24  LEA ECX,[ESP + 0x64]
009e0f28  FST float ptr [ESP + 0x64]
009e0f2c  PUSH ECX
009e0f2d  FST float ptr [ESP + 0x6c]
009e0f31  MOV ECX,dword ptr [ESI + 0x28]
009e0f34  FSTP float ptr [ESP + 0x70]
009e0f38  CALL 0x0067bfb0
009e0f3d  FLDZ
009e0f3f  MOV ECX,dword ptr [ESI + 0x28]
009e0f42  FST float ptr [ESP + 0x64]
009e0f46  LEA EDX,[ESP + 0x64]
009e0f4a  FST float ptr [ESP + 0x68]
009e0f4e  PUSH EDX
009e0f4f  FSTP float ptr [ESP + 0x70]
009e0f53  CALL 0x0067c0f0
009e0f58  FLDZ
009e0f5a  MOV ECX,dword ptr [ESI + 0x28]
009e0f5d  FST float ptr [ESP + 0x64]
009e0f61  LEA EAX,[ESP + 0x64]
009e0f65  FST float ptr [ESP + 0x68]
009e0f69  PUSH EAX
009e0f6a  FSTP float ptr [ESP + 0x70]
009e0f6e  CALL 0x0067c140
009e0f73  POP ESI
009e0f74  POP EBP
009e0f75  ADD ESP,0x78
009e0f78  RET 0x4
