#ifndef CODE_EMIT_H
#define CODE_EMIT_H
/*
  void ModRegRM(x86ModType, x86ModReg modReg, uint32 baseReg, x86IndexReg = x86IndexReg_none, x86ScaleVal = x86Scale_1, int32 disp = 0);
  void Group1RR(x86Reg regDest, x86Reg regSrc, uint8 groupIndex);
  void Group1RM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex);
  void Group1MR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex);
  void Group1IR(int32 imm, x86Reg regDest, uint8 groupIndex);
  void Group1IM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex);
  void Group2IR(x86Reg regDest, uint8 shiftCount, uint8 groupIndex);
  void Group2IM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex);
  void Group2RR(x86Reg regDest, uint8 groupIndex);
  void Group2RM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex);
  
  void ADDRR(x86Reg regDest, x86Reg regSrc);
  void ADDRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void ADDMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void ADDIR(int32 imm, x86Reg regDest);
  void ADDIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void PUSHES();
  void POPES();
  void ORRR(x86Reg regDest, x86Reg regSrc);
  void ORRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void ORMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void ORIR(int32 imm, x86Reg regDest);
  void ORIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void PUSHCS();
  void ADCRR(x86Reg regDest, x86Reg regSrc);
  void ADCRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void ADCMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void ADCIR(int32 imm, x86Reg regDest);
  void ADCIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void PUSHSS();
  void POPSS();
  void SBBRR(x86Reg regDest, x86Reg regSrc);
  void SBBRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SBBMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SBBIR(int32 imm, x86Reg regDest);
  void SBBIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void PUSHDS();
  void POPDS();
  void ANDRR(x86Reg regDest, x86Reg regSrc);
  void ANDRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void ANDMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void ANDIR(int32 imm, x86Reg regDest);
  void ANDIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void ES();
  void DAA();
  void SUBRR(x86Reg regDest, x86Reg regSrc);
  void SUBRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SUBMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SUBIR(int32 imm, x86Reg regDest);
  void SUBIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CS();
  void DAS();
  void XORRR(x86Reg regDest, x86Reg regSrc);
  void XORRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void XORMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void XORIR(int32 imm, x86Reg regDest);
  void XORIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SS();
  void AAA();
  void CMPRR(x86Reg regDest, x86Reg regSrc);
  void CMPRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMPMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMPIR(int32 imm, x86Reg regDest);
  void CMPIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void DS();
  void AAS();
  void INCR(x86Reg reg);
  void DECR(x86Reg reg);
  void PUSHR(x86Reg reg);
  void POPR(x86Reg reg);
  void PUSHAW();
  void PUSHAD();
  #define PUSHA PUSHAD
  void POPAW();
  void POPAD();
  #define POPA POPAD
  void FS();
  void GS();
  void OPSIZE();
  void ADSIZE();
  void PUSHID(int32 imm);
  void PUSHIW(int16 imm);
  void IMULIRR(x86Reg regDest, int32 imm, x86Reg regSrc);
  void IMULIMR(x86Reg regDest, int32 imm, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void IMULRR(x86Reg regSrc);
  void IMULMR(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void IMULRRR(x86Reg regDest, x86Reg regSrc);
  void IMULMRR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void PUSHIB(int8 imm);
  void INSB();
  void INSW();
  void INSD();
  void OUTSB();
  void OUTSW();
  void OUTSD();
  void JCC(uint8 *pTarget, int8 conditionCode);
  void JCC_Label(PatchManager *patchMgr, int8 conditionCode, uint32 labelIndex);
  void JO(uint8 *pTarget);
  void JNO(uint8 *pTarget);
  void JB(uint8 *pTarget);
  void JNB(uint8 *pTarget);
  void JZ(uint8 *pTarget);
  void JNZ(uint8 *pTarget);
  void JBE(uint8 *pTarget);
  void JNBE(uint8 *pTarget);
  void JS(uint8 *pTarget);
  void JNS(uint8 *pTarget);
  void JP(uint8 *pTarget);
  void JNP(uint8 *pTarget);
  void JL(uint8 *pTarget);
  void JNL(uint8 *pTarget);
  void JLE(uint8 *pTarget);
  void JNLE(uint8 *pTarget);
  void TESTRR(x86Reg regDest, x86Reg regSrc);
  void TESTRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void TESTIR(uint32 imm, x86Reg regSrc);
  void TESTIM(uint32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void XCHGRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void MOVRR(x86Reg regDest, x86Reg regSrc);
  void LEA(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void POPM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void NOP();
  void XCHGRR(x86Reg reg1, x86Reg reg2);
  void CBW();
  void CWDE();
  void CWD();
  void CDQ();
  void CALLI(uint32 offset, uint16 seg);
  void JMPI(uint8 *target, uint16 seg);
  void JMPI_Label(PatchManager *patchMgr, uint32 labelIndex);
  void WAIT();
  void PUSHFW();
  void PUSHFD();
  #define PUSHF PUSHFD
  void POPFW();
  void POPFD();
  #define POPF POPFD
  void SAHF();
  void LAHF();
  void MOVMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void MOVRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void MOVIR(int32 imm, x86Reg regDest);
  void MOVSB();
  void MOVSW();
  void MOVSD();
  void CMPSB();
  void CMPSW();
  void CMPSD();
  void STOSB();
  void STOSW();
  void STOSD();
  void LODSB();
  void LODSW();
  void LODSD();
  void SCASB();
  void SCASW();
  void SCASD();
  void ROLIR(x86Reg regDest, uint8 shiftCount);
  void RORIR(x86Reg regDest, uint8 shiftCount);
  void RCLIR(x86Reg regDest, uint8 shiftCount);
  void RCRIR(x86Reg regDest, uint8 shiftCount);
  void SHLIR(x86Reg regDest, uint8 shiftCount);
  void SHLDIRR(x86Reg regDest, x86Reg regSrc, uint8 shiftCount);
  void SHLDIMR(x86Reg regDest, x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SHLDRRR(x86Reg regDest, x86Reg regSrc);
  void SHLDIRM(x86Reg regSrc, x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SHLDRRM(x86Reg regSrc, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SHRDIRR(x86Reg regDest, x86Reg regSrc, uint8 shiftCount);
  void SHRDIMR(x86Reg regDest, x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp = 0);
  void SHRDRRR(x86Reg regDest, x86Reg regSrc);
  void SHRDIRM(x86Reg regSrc, x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SHRDRRM(x86Reg regSrc, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SHLDRMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SHRDRMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp);
  void SHRIR(x86Reg regDest, uint8 shiftCount);
  void SALIR(x86Reg regDest, uint8 shiftCount);
  void SARIR(x86Reg regDest, uint8 shiftCount);
  void ROLIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void RORIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void RCLIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void RCRIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SHLIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SHRIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SALIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SARIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);

  void ROLRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void RORRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void RCLRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void RCRRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SHLRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SHRRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SALRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SARRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);

  void RETN(uint16 iw = 0);
  #define RET RETN
  void MOVIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void LEAVE();
  void RETF(uint16 iw = 0);
  void INT3();
  void INT(int8 vector);
  void INTO();
  void IRET();
  void ROLRR(x86Reg regDest);
  void RORRR(x86Reg regDest);
  void RCLRR(x86Reg regDest);
  void RCRRR(x86Reg regDest);
  void SHLRR(x86Reg regDest);
  void SHRRR(x86Reg regDest);
  void SALRR(x86Reg regDest);
  void SARRR(x86Reg regDest);
  void AAM(uint8 divisor);
  void AAD(uint8 divisor);
  void XLAT();
  void ESC0();
  void ESC1();
  void ESC2();
  void ESC3();
  void ESC4();
  void ESC5();
  void ESC6();
  void ESC7();
  void LOOPNE(uint8 *pTarget);
  void LOOPNE_Label(PatchManager *patchMgr, uint32 labelIndex);
  #define LOOPNZ LOOPNE
  #define LOOPNZ_Label LOOPNE_Label
  void LOOPE(uint8 *pTarget);
  void LOOPE_Label(PatchManager *patchMgr, uint32 labelIndex);
  #define LOOPZ LOOPE
  #define LOOPZ_Label LOOPZ_Label
  void LOOP(uint8 *pTarget);
  void LOOP_Label(PatchManager *patchMgr, uint32 labelIndex);
  void JCXZ(uint8 *pTarget);
  void JCXZ_Label(PatchManager *patchMgr, uint32 labelIndex);
  void JECXZ(uint8 *pTarget);
  void JECXZ_Label(PatchManager *patchMgr, uint32 labelIndex);
  void INI(x86Reg regDest, uint8 port);
  void OUTI(x86Reg regDest, uint8 data);
  void INR(x86Reg regDest);
  void OUTR(x86Reg regDest);
  void LOCK();
  void INT1();
  void REPNE();
  void REPE();
  #define REP REPE
  void HLT();
  void CMC();
  void NOTR(x86Reg regDest);
  void NOTM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void NEGR(x86Reg regDest);
  void NEGM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CLC();
  void STC();
  void CLI();
  void STI();
  void CLD();
  void STD();
  void INCM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void DECM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CALLNM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  #define CALLM CALLNM
  void CALLR(x86Reg regSrc);
  void CALLFM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void JMPR(x86Reg regSrc);
  void JMPNM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  #define JMPM JMPNM
  void JMPFM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void PUSHM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void INVD(x86Reg reg);
  void WBINVD(x86Reg reg);
  void UD1(x86Reg reg);
  void BSWAP(x86Reg reg);
  void SETOR(x86Reg reg);
  void SETOM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETNOR(x86Reg reg);
  void SETNOM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETBR(x86Reg reg);
  void SETBM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETNBR(x86Reg reg);
  void SETNBM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETZR(x86Reg reg);
  void SETZM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETNZR(x86Reg reg);
  void SETNZM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETBER(x86Reg reg);
  void SETBEM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETNBER(x86Reg reg);
  void SETNBEM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETSR(x86Reg reg);
  void SETSM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETNSR(x86Reg reg);
  void SETNSM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETPR(x86Reg reg);
  void SETPM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETNPR(x86Reg reg);
  void SETNPM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETLR(x86Reg reg);
  void SETLM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETNLR(x86Reg reg);
  void SETNLM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETLER(x86Reg reg);
  void SETLEM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void SETNLER(x86Reg reg);
  void SETNLEM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVORR(x86Reg regDest, uint32 regSrc);
  void CMOVOMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVNORR(x86Reg regDest, uint32 regSrc);
  void CMOVNOMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVBRR(x86Reg regDest, uint32 regSrc);
  void CMOVBMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVNBRR(x86Reg regDest, uint32 regSrc);
  void CMOVNBMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVZRR(x86Reg regDest, uint32 regSrc);
  void CMOVZMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVNZRR(x86Reg regDest, uint32 regSrc);
  void CMOVNZMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVBERR(x86Reg regDest, uint32 regSrc);
  void CMOVBEMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVNBERR(x86Reg regDest, uint32 regSrc);
  void CMOVNBEMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVSRR(x86Reg regDest, uint32 regSrc);
  void CMOVSMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVNSRR(x86Reg regDest, uint32 regSrc);
  void CMOVNSMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVPRR(x86Reg regDest, uint32 regSrc);
  void CMOVPMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVNPRR(x86Reg regDest, uint32 regSrc);
  void CMOVNPMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVLRR(x86Reg regDest, uint32 regSrc);
  void CMOVLMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVNLRR(x86Reg regDest, uint32 regSrc);
  void CMOVNLMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVLERR(x86Reg regDest, uint32 regSrc);
  void CMOVLEMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void CMOVNLERR(x86Reg regDest, uint32 regSrc);
  void CMOVNLEMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);

  void MOVSXRR(x86Reg regDest, uint32 regSrc);
  void MOVZXRR(x86Reg regDest, uint32 regSrc);
  void MOVSXMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void MOVZXMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);

  void MOVQRR(x86Reg regDest, x86Reg regSrc);
  void MOVQRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
  void MOVQMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);

  void EMMS();
  void FEMMS();
  //PatchManager *patchMgr;

  //uint8 *pEmitLoc;*/

#include "basetypes.h"
#include "X86EmitTypes.h"
#include "PatchManager.h"

#define MRM(mod,reg,rm) (*pEmitLoc++ = ((mod << 6) | (reg << 3) | (rm)))
#define SIB(base, scale, index) (*pEmitLoc++ = ((scale << 6) | (index << 3) | (base)))


class CodeEmit
{
public :
	CodeEmit(int size)
	{
		patchMgr=new PatchManager();
		pEmitLoc=(u8*)malloc(size);
	}
	~CodeEmit()
	{
		delete patchMgr;
	}

	void* GetCode()
	{
		patchMgr->ApplyPatches();
		patchMgr->Reset();
		return pEmitLoc;
	}

	void SetLabelPointer(uint32 labelIndex, uint8 *ptr)
	{
		patchMgr->SetLabelPointer(labelIndex,ptr);
	}

	uint8 *GetLabelPointer(uint32 labelIndex)
	{
		return patchMgr->GetLabelPointer(labelIndex);
	}

	PatchManager *patchMgr;

	uint8 *pEmitLoc;

	void ModRegRM(x86ModType modType, x86ModReg regSpare, uint32 base, x86IndexReg index =x86IndexReg_none, x86ScaleVal scale=x86Scale_1, int32 disp = 0)
	{
		if(base > 7)
		{
			//[dword]: mod = 0, reg = regSpare, rm = 5
			MRM(x86ModType_mem,regSpare,x86BaseReg_sdword);
			*((uint32 *)pEmitLoc) = base;
			pEmitLoc += sizeof(uint32);
		}
		else
		{
			if(modType == x86ModType_reg)
			{
				//reg
				MRM(x86ModType_reg, regSpare, base);
			}
			else
			{
				if(index != x86IndexReg_none)
				{
					if(!disp)
					{
						//[sib]
						MRM(x86ModType_mem, regSpare, x86BaseReg_sib);
						SIB(base, scale, index);
					}
					else
					{
						//[sib + disp]
						if((disp < -128) || (disp > 127))
						{
							//[base + scale*index + disp32]
							MRM(x86ModType_mem_disp32, regSpare, x86BaseReg_sib);
							SIB(base, scale, index);
							*((int32 *)pEmitLoc) = disp;
							pEmitLoc += sizeof(int32);
						}
						else
						{
							//[sib + disp8]
							MRM(x86ModType_mem_disp8, regSpare, x86BaseReg_sib);
							SIB(base, scale, index);
							*pEmitLoc++ = (uint8)disp;
						}
					}
				}
				else
				{
					//[base + disp] or [base]
					if(!disp && (base != x86BaseReg_ebp))
					{
						//[base]
						MRM(x86ModType_mem, regSpare, base);
					}
					else
					{
						//[base + disp]
						if((disp < -128) || (disp > 127))
						{
							//[base + disp32]
							MRM(x86ModType_mem_disp32, regSpare, base);
							*((int32 *)pEmitLoc) = disp;
							pEmitLoc += sizeof(int32);
						}
						else
						{
							//[base + disp8]
							MRM(x86ModType_mem_disp8, regSpare, base);
							*pEmitLoc++ = (uint8)disp;
						}
					}
				}
			}
		}
	}

	//ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
	//x86IndexReg index, x86ScaleVal scale, int32 disp
	//void ModRegRM(x86ModType modType, x86ModReg regSpare, uint32 base)
	//{
	//	ModRegRM(modType,regSpare,base,x86IndexReg::x86IndexReg_none,x86ScaleVal::x86Scale_1,0);
	//}
	void Group1RR(x86Reg regDest, x86Reg regSrc, uint8 groupIndex)
	{
		//OP r8, reg8, OP r16, reg16 or OP r32, reg32

		if(regSrc < x86Reg_ax)
		{
			//m8, r8
			*pEmitLoc++ = groupIndex << 3;
			ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
		}
		else if(regSrc < x86Reg_eax)
		{
			//m16, r16
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = (groupIndex << 3) + 1;
			ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
		}
		else
		{
			//m32, r32
			*pEmitLoc++ = (groupIndex << 3) + 1;
			ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
		}
	}

	void Group1RM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex)
	{
		//OP m8, reg8, OP m16, reg16 or OP m32, reg32

		if(regSrc < x86Reg_ax)
		{
			//m8, r8
			*pEmitLoc++ = (groupIndex << 3);
			ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
		}
		else if(regSrc < x86Reg_eax)
		{
			//m16, r16
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = (groupIndex << 3) + 1;
			ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
		}
		else
		{
			//m32, r32
			*pEmitLoc++ = (groupIndex << 3) + 1;
			ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
		}
	}

	void Group1MR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex)
	{
		//OP reg8, m8, OP reg16, m16 or OP reg32, m32
		if(regDest < x86Reg_ax)
		{
			//r8, m8
			*pEmitLoc++ = (groupIndex << 3) + 0x02;
			ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
		}
		else if(regDest < x86Reg_eax)
		{
			//r16, m16
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = (groupIndex << 3) + 0x03;
			ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
		}
		else
		{
			//r32, m32
			*pEmitLoc++ = (groupIndex << 3) + 0x03;
			ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
		}
	}

	void Group1IR(int32 imm, x86Reg regDest, uint8 groupIndex)
	{
		if(regDest < x86Reg_ax)
		{
			//r8,imm8
			if(regDest == x86Reg_al)
			{
				//al, imm8
				*pEmitLoc++ = (groupIndex << 3) + 0x04;
				*pEmitLoc++ = (int8)imm;
			}
			else
			{
				//r8, imm8
				*pEmitLoc++ = 0x80;
				ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
				*pEmitLoc++ = (int8)imm;
			}
		}
		else if(regDest < x86Reg_eax)
		{
			//r16, imm
			if((imm <= 127) && (imm >= -128))
			{
				*pEmitLoc++ = 0x66;
				*pEmitLoc++ = 0x83;
				ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
				*pEmitLoc++ = (int8)imm;
			}
			else if(regDest == x86Reg_ax)
			{
				*pEmitLoc++ = 0x66;
				*pEmitLoc++ = (groupIndex << 3) + 0x05;
				*((int16 *)pEmitLoc) = (int16)imm;
				pEmitLoc += sizeof(int16);
			}
			else
			{
				*pEmitLoc++ = 0x66;
				*pEmitLoc++ = 0x81;
				ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
				*((int16 *)pEmitLoc) = (int16)imm;
				pEmitLoc += sizeof(int16);
			}
		}
		else
		{
			//r32, imm
			if((imm <= 127) && (imm >= -128))
			{
				*pEmitLoc++ = 0x83;
				ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
				*pEmitLoc++ = (int8)imm;
			}
			else if(regDest == x86Reg_eax)
			{
				*pEmitLoc++ = (groupIndex << 3) + 0x05;
				*((int32 *)pEmitLoc) = (int32)imm;
				pEmitLoc += sizeof(int32);

			}
			else
			{
				*pEmitLoc++ = 0x81;
				ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
				*((int32 *)pEmitLoc) = (int32)imm;
				pEmitLoc += sizeof(int32);
			}
		}
	}

	void Group1IM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex)
	{
		if(ptrType == x86MemPtr_byte)
		{
			//mem8,imm8
			*pEmitLoc++ = 0x80;
			ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
			*pEmitLoc++ = (int8)imm;
		}
		else if(ptrType == x86MemPtr_dword)
		{
			//mem32, imm
			if((imm <= 127) && (imm >= -128))
			{
				//mem32, imm8
				*pEmitLoc++ = 0x83;
				ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
				*pEmitLoc++ = (int8)imm;
			}
			else
			{
				//mem32, imm32
				*pEmitLoc++ = 0x81;
				ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
				*((int32 *)pEmitLoc) = (int32)imm;
				pEmitLoc += sizeof(int32);
			}
		}
		else
		{
			//mem16, imm
			if((imm <= 127) && (imm >= -128))
			{
				//mem16, imm8
				*pEmitLoc++ = 0x66;
				*pEmitLoc++ = 0x83;
				ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
				*pEmitLoc++ = imm;
			}
			else
			{
				//mem16, imm16
				*pEmitLoc++ = 0x66;
				*pEmitLoc++ = 0x81;
				//ModRegRM(modtype, regspare, base, index, scale, disp)
				ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
				*((int16 *)pEmitLoc) = (int16)imm;
				pEmitLoc += sizeof(int16);

			}
		}
	}

	void Group2IR(x86Reg regDest, uint8 shiftCount, uint8 groupIndex)
	{
		//SHIFTOP r8, CL, SHIFTOP r16, CL, or SHIFTOP r32, CL

		if(regDest < x86Reg_ax)
		{
			if(shiftCount == 1)
			{
				*pEmitLoc++ = 0xD0;
				ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
			}
			else
			{
				*pEmitLoc++ = 0xC0;
				ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
				*pEmitLoc++ = shiftCount;
			}
		}
		else if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;

			if(shiftCount == 1)
			{
				*pEmitLoc++ = 0xD1;
				ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
			}
			else
			{
				*pEmitLoc++ = 0xC1;
				ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
				*pEmitLoc++ = shiftCount;
			}
		}
		else
		{
			if(shiftCount == 1)
			{
				*pEmitLoc++ = 0xD1;
				ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
			}
			else
			{
				*pEmitLoc++ = 0xC1;
				ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
				*pEmitLoc++ = shiftCount;
			}
		}
	}

	void Group2IM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex)
	{
		if(ptrType == x86MemPtr_byte)
		{
			if(shiftCount == 1)
			{
				*pEmitLoc++ = 0xD0;
				ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
			}
			else
			{
				*pEmitLoc++ = 0xC0;
				ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
				*pEmitLoc++ = shiftCount;
			}
		}
		else if(ptrType == x86MemPtr_word)
		{
			*pEmitLoc++ = 0x66;

			if(shiftCount == 1)
			{
				*pEmitLoc++ = 0xD1;
				ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
			}
			else
			{
				*pEmitLoc++ = 0xC1;
				ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
				*pEmitLoc++ = shiftCount;
			}
		}
		else
		{
			if(shiftCount == 1)
			{
				*pEmitLoc++ = 0xD1;
				ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
			}
			else
			{
				*pEmitLoc++ = 0xC1;
				ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
				*pEmitLoc++ = shiftCount;
			}
		}
	}

	void Group2RR(x86Reg regDest, uint8 groupIndex)
	{
		//SHIFTOP r8, CL, SHIFTOP r16, CL, or SHIFTOP r32, CL

		if(regDest < x86Reg_ax)
		{
			//m8, r8
			*pEmitLoc++ = 0xD2;
			ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
		}
		else if(regDest < x86Reg_eax)
		{
			//m16, r16
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xD3;
			ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
		}
		else
		{
			//m32, r32
			*pEmitLoc++ = 0xD3;
			ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
		}
	}

	void Group2RM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex)
	{
		//OP m8, reg8, OP m16, reg16 or OP m32, reg32

		if(ptrType == x86MemPtr_byte)
		{
			*pEmitLoc++ = 0xD2;
			ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
		}
		else if(ptrType == x86MemPtr_word)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xD3;
			ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
		}
		else
		{
			*pEmitLoc++ = 0xD3;
			ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
		}
	}

	void ADDRR(x86Reg regDest, x86Reg regSrc)
	{
		Group1RR(regDest, regSrc, 0);
	}

	void ADDRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1RM(regSrc, base, index, scale, disp, 0);
	}

	void ADDMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1MR(regDest, base, index, scale, disp, 0);
	}

	void ADDIR(int32 imm, x86Reg regDest)
	{
		Group1IR(imm, regDest, 0);
	}

	void ADDIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1IM(imm, ptrType, base, index, scale, disp, 0);
	}

	void PUSHES()
	{
		*pEmitLoc++ = 0x06;
	}

	void POPES()
	{
		*pEmitLoc++ = 0x07;
	}

	void ORRR(x86Reg regDest, x86Reg regSrc)
	{
		Group1RR(regDest, regSrc, 1);
	}

	void ORRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1RM(regSrc, base, index, scale, disp, 1);
	}

	void ORMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1MR(regDest, base, index, scale, disp, 1);
	}

	void ORIR(int32 imm, x86Reg regDest)
	{
		Group1IR(imm, regDest, 1);
	}

	void ORIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1IM(imm, ptrType, base, index, scale, disp, 1);
	}

	void PUSHCS()
	{
		*pEmitLoc++ = 0x0E;
	}

	void ADCRR(x86Reg regDest, x86Reg regSrc)
	{
		Group1RR(regDest, regSrc, 2);
	}

	void ADCRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1RM(regSrc, base, index, scale, disp, 2);
	}

	void ADCMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1MR(regDest, base, index, scale, disp, 2);
	}

	void ADCIR(int32 imm, x86Reg regDest)
	{
		Group1IR(imm, regDest, 2);
	}

	void ADCIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1IM(imm, ptrType, base, index, scale, disp, 2);
	}

	void PUSHSS()
	{
		*pEmitLoc++ = 0x16;
	}

	void POPSS()
	{
		*pEmitLoc++ = 0x17;
	}

	void SBBRR(x86Reg regDest, x86Reg regSrc)
	{
		Group1RR(regDest, regSrc, 3);
	}

	void SBBRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1RM(regSrc, base, index, scale, disp, 3);
	}

	void SBBMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1MR(regDest, base, index, scale, disp, 3);
	}

	void SBBIR(int32 imm, x86Reg regDest)
	{
		Group1IR(imm, regDest, 3);
	}

	void SBBIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1IM(imm, ptrType, base, index, scale, disp, 3);
	}

	void PUSHDS()
	{
		*pEmitLoc++ = 0x1E;
	}

	void POPDS()
	{
		*pEmitLoc++ = 0x1F;
	}

	void ANDRR(x86Reg regDest, x86Reg regSrc)
	{
		Group1RR(regDest, regSrc, 4);
	}

	void ANDRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1RM(regSrc, base, index, scale, disp, 4);
	}

	void ANDMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1MR(regDest, base, index, scale, disp, 4);
	}

	void ANDIR(int32 imm, x86Reg regDest)
	{
		Group1IR(imm, regDest, 4);
	}

	void ANDIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1IM(imm, ptrType, base, index, scale, disp, 4);
	}

	void ES()
	{
		*pEmitLoc++ = 0x26;
	}

	void DAA()
	{
		*pEmitLoc++ = 0x27;
	}

	void SUBRR(x86Reg regDest, x86Reg regSrc)
	{
		Group1RR(regDest, regSrc, 5);
	}

	void SUBRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1RM(regSrc, base, index, scale, disp, 5);
	}

	void SUBMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1MR(regDest, base, index, scale, disp, 5);
	}

	void SUBIR(int32 imm, x86Reg regDest)
	{
		Group1IR(imm, regDest, 5);
	}

	void SUBIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1IM(imm, ptrType, base, index, scale, disp, 5);
	}

	void CS()
	{
		*pEmitLoc++ = 0x2E;
	}

	void DAS()
	{
		*pEmitLoc++ = 0x2F;
	}

	void XORRR(x86Reg regDest, x86Reg regSrc)
	{
		Group1RR(regDest, regSrc, 6);
	}

	void XORRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1RM(regSrc, base, index, scale, disp, 6);
	}

	void XORMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1MR(regDest, base, index, scale, disp, 6);
	}

	void XORIR(int32 imm, x86Reg regDest)
	{
		Group1IR(imm, regDest, 6);
	}

	void XORIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1IM(imm, ptrType, base, index, scale, disp, 6);
	}

	void SS()
	{
		*pEmitLoc++ = 0x36;
	}

	void AAA()
	{
		*pEmitLoc++ = 0x37;
	}

	void CMPRR(x86Reg regDest, x86Reg regSrc)
	{
		Group1RR(regDest, regSrc, 7);
	}

	void CMPRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1RM(regSrc, base, index, scale, disp, 7);
	}

	void CMPMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1MR(regDest, base, index, scale, disp, 7);
	}

	void CMPIR(int32 imm, x86Reg regDest)
	{
		Group1IR(imm, regDest, 7);
	}

	void CMPIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group1IM(imm, ptrType, base, index, scale, disp, 7);
	}

	void DS()
	{
		*pEmitLoc++ = 0x3E;
	}

	void AAS()
	{
		*pEmitLoc++ = 0x3F;
	}

	void INCR(x86Reg reg)
	{
		if(reg < x86Reg_ax)
		{
			*pEmitLoc++ = 0xFE;
			ModRegRM(x86ModType_reg,(x86ModReg)0,(reg & 0x07));
		}
		else if(reg < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0x40 + (reg & 0x07);  
		}
		else
		{
			*pEmitLoc++ = 0x40 + (reg & 0x07);  
		}
	}

	void DECR(x86Reg reg)
	{
		if(reg < x86Reg_ax)
		{
			*pEmitLoc++ = 0xFE;
			ModRegRM(x86ModType_reg,(x86ModReg)1,(reg & 0x07));
		}
		else if(reg < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0x48 + (reg & 0x07);  
		}
		else
		{
			*pEmitLoc++ = 0x48 + (reg & 0x07);  
		}
	}

	void PUSHR(x86Reg reg)
	{
		if(reg < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x50 + (reg & 0x07);  
	}

	void POPR(x86Reg reg)
	{
		if(reg < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x58 + (reg & 0x07);  
	}

	void PUSHAW()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0x60;
	}

	void PUSHAD()
	{
		*pEmitLoc++ = 0x60;
	}

	void POPAW()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0x61;
	}

	void POPAD()
	{
		*pEmitLoc++ = 0x61;
	}

	void FS()
	{
		*pEmitLoc++ = 0x64;
	}

	void GS()
	{
		*pEmitLoc++ = 0x65;
	}

	void OPSIZE()
	{
		*pEmitLoc++ = 0x66;
	}

	void ADSIZE()
	{
		*pEmitLoc++ = 0x67;
	}

	void PUSHID(int32 imm)
	{
		*pEmitLoc++ = 0x68;
		*((int32 *)pEmitLoc++) = imm;
	}

	void PUSHIW(int16 imm)
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0x68;
		*((int16 *)pEmitLoc++) = imm;
	}

	void IMULMRR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//imul reg16, r16
			*pEmitLoc++ = 0x66;
		}
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0xAF;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
	}

	void IMULRRR(x86Reg regDest, x86Reg regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//imul reg16, r16
			*pEmitLoc++ = 0x66;
		}
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0xAF;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void IMULIRR(x86Reg regDest, int32 imm, x86Reg regSrc)
	{
		if(regSrc < x86Reg_eax)
		{
			//reg16, r8, imm8 or reg16, r16, imm16
			if((imm >= -128) && (imm <= 127))
			{
				*pEmitLoc++ = 0x66;
				*pEmitLoc++ = 0x6B;
				ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
				*pEmitLoc++ = (int8)imm;
			}
			else
			{
				*pEmitLoc++ = 0x66;
				*pEmitLoc++ = 0x69;
				ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
				*((int16 *)pEmitLoc) = (int16)imm;
				pEmitLoc += sizeof(int16);
			}
		}
		else
		{
			//reg32, r32, imm8 or reg32, r32, imm32
			if((imm >= -128) && (imm <= 127))
			{
				*pEmitLoc++ = 0x6B;
				ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
				*pEmitLoc++ = (int8)imm;
			}
			else
			{
				*pEmitLoc++ = 0x69;
				ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
				*((int32 *)pEmitLoc) = (int32)imm;
				pEmitLoc += sizeof(int32);
			}
		}
	}

	void IMULIMR(x86Reg regDest, int32 imm, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//reg16, m8, imm8 or reg16, m16, imm16
			if((imm >= -128) && (imm <= 127))
			{
				*pEmitLoc++ = 0x66;
				*pEmitLoc++ = 0x6B;
				ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
				*pEmitLoc++ = (int8)imm;
			}
			else
			{
				*pEmitLoc++ = 0x66;
				*pEmitLoc++ = 0x69;
				ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
				*((int16 *)pEmitLoc) = (int16)imm;
				pEmitLoc += sizeof(int16);
			}
		}
		else
		{
			//reg32, m32, imm8 or reg32, m32, imm32
			if((imm >= -128) && (imm <= 127))
			{
				*pEmitLoc++ = 0x6B;
				ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
				*pEmitLoc++ = (int8)imm;
			}
			else
			{
				*pEmitLoc++ = 0x69;
				ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
				*((int32 *)pEmitLoc) = (int32)imm;
				pEmitLoc += sizeof(int32);
			}
		}
	}

	void PUSHIB(int8 imm)
	{
		*pEmitLoc++ = 0x6A;
		*pEmitLoc++ = imm;
	}

	void INSB()
	{
		*pEmitLoc++ = 0x6C;
	}

	void INSW()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0x6C;
	}

	void INSD()
	{
		*pEmitLoc++ = 0x6C;
	}

	void OUTSB()
	{
		*pEmitLoc++ = 0x6E;
	}

	void OUTSW()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0x6E;
	}

	void OUTSD()
	{
		*pEmitLoc++ = 0x6E;
	}

	void FEMMS()
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x0E;
	}

	void EMMS()
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x77;
	}

	void JCC(uint8 *pTarget, int8 conditionCode)
	{
		int32 pOffset = pTarget - (pEmitLoc + 2);
		int32 pOffsetNear = pTarget - (pEmitLoc + 6);

		if((pOffset >= -128) && (pOffset <= 127))
		{
			*pEmitLoc++ = 0x70 + conditionCode;
			*pEmitLoc++ = (int8)pOffset;
		}
		else
		{
			*pEmitLoc++ = 0x0F;
			*pEmitLoc++ = 0x80 + conditionCode;
			*((int32 *)pEmitLoc) = pOffsetNear;
			pEmitLoc += sizeof(int32);
		}
	}

	void JCC_Label(PatchManager *patchMgr, int8 conditionCode, uint32 labelIndex)
	{
		if(labelIndex >= patchMgr->numLabels)
		{
			patchMgr->AddPatch(pEmitLoc + 2, PatchType_Rel32, pEmitLoc + 6, labelIndex);
			*pEmitLoc++ = 0x0F;
			*pEmitLoc++ = 0x80 + conditionCode;
			*((int32 *)pEmitLoc) = 0;
			pEmitLoc += sizeof(int32);
		}
		else
		{
			JCC(patchMgr->GetLabelPointer(labelIndex),conditionCode);
		}
	}

	void JO(uint8 *pTarget)
	{
		JCC(pTarget,0);
	}

	void JNO(uint8 *pTarget)
	{
		JCC(pTarget,1);
	}

	void JB(uint8 *pTarget)
	{
		JCC(pTarget,2);
	}

	void JNB(uint8 *pTarget)
	{
		JCC(pTarget,3);
	}

	void JZ(uint8 *pTarget)
	{
		JCC(pTarget,4);
	}

	void JNZ(uint8 *pTarget)
	{
		JCC(pTarget,5);
	}

	void JBE(uint8 *pTarget)
	{
		JCC(pTarget,6);
	}

	void JNBE(uint8 *pTarget)
	{
		JCC(pTarget,7);
	}

	void JS(uint8 *pTarget)
	{
		JCC(pTarget,8);
	}

	void JNS(uint8 *pTarget)
	{
		JCC(pTarget,9);
	}

	void JP(uint8 *pTarget)
	{
		JCC(pTarget,10);
	}

	void JNP(uint8 *pTarget)
	{
		JCC(pTarget,11);
	}

	void JL(uint8 *pTarget)
	{
		JCC(pTarget,12);
	}

	void JNL(uint8 *pTarget)
	{
		JCC(pTarget,13);
	}

	void JLE(uint8 *pTarget)
	{
		JCC(pTarget,14);
	}

	void JNLE(uint8 *pTarget)
	{
		JCC(pTarget,15);
	}

	void TESTRR(x86Reg regDest, x86Reg regSrc)
	{
		//OP r8, reg8, OP r16, reg16 or OP r32, reg32
		if(regSrc < x86Reg_ax)
		{
			//r8, r8
			*pEmitLoc++ = 0x84;
			ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
		}
		else if(regSrc < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0x85;
			ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
		}
		else
		{
			//r32, r32
			*pEmitLoc++ = 0x85;
			ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
		}
	}

	void TESTRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		//OP m8, reg8, OP m16, reg16 or OP m32, reg32

		if(regSrc < x86Reg_ax)
		{
			//m8, r8
			*pEmitLoc++ = 0x84;
			ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
		}
		else if(regSrc < x86Reg_eax)
		{
			//m16, r16
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0x85;
			ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
		}
		else
		{
			//m32, r32
			*pEmitLoc++ = 0x85;
			ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
		}
	}

	void XCHGRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		//OP m8, reg8, OP m16, reg16 or OP m32, reg32

		if(regSrc < x86Reg_ax)
		{
			//m8, r8
			*pEmitLoc++ = 0x86;
			ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
		}
		else if(regSrc < x86Reg_eax)
		{
			//m16, r16
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0x87;
			ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
		}
		else
		{
			//m32, r32
			*pEmitLoc++ = 0x87;
			ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
		}
	}

	void MOVRR(x86Reg regDest, x86Reg regSrc)
	{
		Group1RR(regDest,regSrc,17);
	}

	void LEA(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x8D;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
	}

	void POPM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(ptrType == x86MemPtr_word)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x8F;
		ModRegRM(x86ModType_mem,(x86ModReg)0,base,index,scale,disp);
	}

	void NOP()
	{
		//Alias for XCHG rAX, rAX
		*pEmitLoc++ = 0x90;
	}

	void XCHGRR(x86Reg reg1, x86Reg reg2)
	{
		if((reg2 == x86Reg_eax) || (reg2 == x86Reg_ax))
		{
			//r32, eax or r16, ax
			if(reg2 == x86Reg_ax)
			{
				*pEmitLoc++ = 0x66;
			}
			*pEmitLoc++ = 0x90 + (reg1 & 0x07);
		}
		else if((reg1 == x86Reg_eax) || (reg1 == x86Reg_ax))
		{
			//eax, r32 or ax, r16
			if(reg1 == x86Reg_ax)
			{
				*pEmitLoc++ = 0x66;
			}
			*pEmitLoc++ = 0x90 + (reg2 & 0x07);
		}
		else
		{
			if(reg1 < x86Reg_ax)
			{
				//r8, r8
				*pEmitLoc++ = 86;
				ModRegRM(x86ModType_reg,(x86ModReg)(reg2 & 0x07),(reg1 & 0x07));
			}
			else if(reg1 < x86Reg_eax)
			{
				//r16, r16
				*pEmitLoc++ = 0x66;
				*pEmitLoc++ = 87;
				ModRegRM(x86ModType_reg,(x86ModReg)(reg2 & 0x07),(reg1 & 0x07));
			}
			else
			{
				//r32, r32
				*pEmitLoc++ = 87;
				ModRegRM(x86ModType_reg,(x86ModReg)(reg2 & 0x07),(reg1 & 0x07));
			}

		}
	}

	void CBW()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0x98;
	}

	void CWDE()
	{
		*pEmitLoc++ = 0x98;
	}

	void CWD()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0x99;
	}

	void CDQ()
	{
		*pEmitLoc++ = 0x99;
	}

	void CALLI(uint32 offset, uint16 seg)
	{
		if(seg == 0)
		{
			//CALL imm32
			*pEmitLoc++ = 0xE8;
			*((uint32 *)pEmitLoc) = offset;
			pEmitLoc += sizeof(int32);

		}
		else
		{
			//CALL imm:imm32
			*pEmitLoc++ = 0x9A;
			*((uint32 *)pEmitLoc) = offset;
			pEmitLoc += sizeof(int32);
			*((uint16 *)pEmitLoc) = seg;
			pEmitLoc += sizeof(int16);
		}
	}

	void JMPI(uint8 *target, uint16 seg)
	{
		int32 offset = target - (pEmitLoc + 2);
		int32 offsetNear = target - (pEmitLoc + 5);

		if(seg == 0)
		{
			if((offset >= -128) && (offset < 128))
			{
				*pEmitLoc++ = 0xEB;
				*pEmitLoc++ = (int8)offset;
			}
			else
			{
				*pEmitLoc++ = 0xE9;
				*((int32 *)pEmitLoc) = offsetNear;
				pEmitLoc += sizeof(int32);
			}
		}
		else
		{
			//JMP imm:imm32
			*pEmitLoc++ = 0xEA;
			*((uint32 *)pEmitLoc) = offset;
			pEmitLoc += sizeof(uint32);
			*((uint16 *)pEmitLoc) = seg;
			pEmitLoc += sizeof(uint16);
		}
	}

	void JMPI_Label(PatchManager *patchMgr, uint32 labelIndex)
	{
		if(labelIndex >= patchMgr->numPatches)
		{
			patchMgr->AddPatch(pEmitLoc + 1, PatchType_Rel32, pEmitLoc + 5, labelIndex);
			*pEmitLoc++ = 0xE9;
			*((int32 *)pEmitLoc) = 0;
			pEmitLoc += sizeof(int32);
		}
		else
		{
			JMPI(patchMgr->GetLabelPointer(labelIndex),0);
		}
	}

	void WAIT()
	{
		*pEmitLoc++ = 0x9B;
	}

	void PUSHFW()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0x9C;
	}

	void PUSHFD()
	{
		*pEmitLoc++ = 0x9C;
	}

	void POPFW()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0x9D;
	}

	void POPFD()
	{
		*pEmitLoc++ = 0x9D;
	}

	void SAHF()
	{
		*pEmitLoc++ = 0x9E;
	}

	void LAHF()
	{
		*pEmitLoc++ = 0x9F;
	}

	void MOVMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if((base > 7) && ((regDest == x86Reg_eax) || (regDest == x86Reg_ax) || (regDest == x86Reg_al)))
		{
			if(regDest == x86Reg_al)
			{
				*pEmitLoc++ = 0xA0;
			}
			else
			{
				if(regDest == x86Reg_ax)
				{
					*pEmitLoc++ = 0x66;
				}
				*pEmitLoc++ = 0xA1;
			}
			*((uint32 *)pEmitLoc) = base;
			pEmitLoc += sizeof(int32);

		}
		else
		{
			Group1MR(regDest,base,index,scale,disp,17);
		}
	}

	void MOVRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if((base > 7) && ((regSrc == x86Reg_eax) || (regSrc == x86Reg_ax) || (regSrc == x86Reg_al)))
		{
			if(regSrc == x86Reg_al)
			{
				*pEmitLoc++ = 0xA2;
			}
			else
			{
				if(regSrc == x86Reg_ax)
				{
					*pEmitLoc++ = 0x66;
				}
				*pEmitLoc++ = 0xA3;
			}
			*((uint32 *)pEmitLoc) = base;
			pEmitLoc += sizeof(int32);
		}
		else
		{
			Group1RM(regSrc,base,index,scale,disp,17);
		}
	}

	void MOVIR(int32 imm, x86Reg regDest)
	{
		if(regDest < x86Reg_ax)
		{
			//r8,imm8
			*pEmitLoc++ = 0xB0 + (regDest & 0x07);
			*pEmitLoc++ = (int8)imm;
		}
		else if(regDest < x86Reg_eax)
		{
			//r16, imm16
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xB8 + (regDest & 0x07);
			*((int16 *)pEmitLoc) = (int16)imm;
			pEmitLoc += sizeof(int16);

		}
		else
		{
			//r32, imm32
			*pEmitLoc++ = 0xB8 + (regDest & 0x07);
			*((int32 *)pEmitLoc) = imm;
			pEmitLoc += sizeof(int32);
		}
	}

	void MOVSB()
	{
		*pEmitLoc++ = 0xA4;
	}

	void MOVSW()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0xA5;
	}

	void MOVSD()
	{
		*pEmitLoc++ = 0xA5;
	}

	void CMPSB()
	{
		*pEmitLoc++ = 0xA6;
	}

	void CMPSW()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0xA7;
	}

	void CMPSD()
	{
		*pEmitLoc++ = 0xA7;
	}

	void STOSB()
	{
		*pEmitLoc++ = 0xAA;
	}

	void STOSW()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0xAB;
	}

	void STOSD()
	{
		*pEmitLoc++ = 0xAB;
	}

	void LODSB()
	{
		*pEmitLoc++ = 0xAC;
	}

	void LODSW()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0xAD;
	}

	void LODSD()
	{
		*pEmitLoc++ = 0xAD;
	}

	void SCASB()
	{
		*pEmitLoc++ = 0xAD;
	}

	void SCASW()
	{
		*pEmitLoc++ = 0x66;
		*pEmitLoc++ = 0xAF;
	}

	void SCASD()
	{
		*pEmitLoc++ = 0xAF;
	}

	void ROLIR(x86Reg regDest, uint8 shiftCount)
	{
		Group2IR(regDest, shiftCount, 0);
	}

	void RORIR(x86Reg regDest, uint8 shiftCount)
	{
		Group2IR(regDest, shiftCount, 1);
	}

	void RCLIR(x86Reg regDest, uint8 shiftCount)
	{
		Group2IR(regDest, shiftCount, 2);
	}

	void RCRIR(x86Reg regDest, uint8 shiftCount)
	{
		Group2IR(regDest, shiftCount, 3);
	}

	void SHLIR(x86Reg regDest, uint8 shiftCount)
	{
		Group2IR(regDest, shiftCount, 4);
	}

	void SHRIR(x86Reg regDest, uint8 shiftCount)
	{
		Group2IR(regDest, shiftCount, 5);
	}

	void SALIR(x86Reg regDest, uint8 shiftCount)
	{
		Group2IR(regDest, shiftCount, 6);
	}

	void SARIR(x86Reg regDest, uint8 shiftCount)
	{
		Group2IR(regDest, shiftCount, 7);
	}

	void ROLIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2IM(ptrType, shiftCount, base, index, scale, disp, 0);
	}

	void RORIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2IM(ptrType, shiftCount, base, index, scale, disp, 1);
	}

	void RCLIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2IM(ptrType, shiftCount, base, index, scale, disp, 2);
	}

	void RCRIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2IM(ptrType, shiftCount, base, index, scale, disp, 3);
	}

	void SHLIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2IM(ptrType, shiftCount, base, index, scale, disp, 4);
	}

	void SHRIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2IM(ptrType, shiftCount, base, index, scale, disp, 5);
	}

	void SALIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2IM(ptrType, shiftCount, base, index, scale, disp, 6);
	}

	void SARIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2IM(ptrType, shiftCount, base, index, scale, disp, 7);
	}

	void ROLRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2RM(ptrType, base, index, scale, disp, 0);
	}

	void RORRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2RM(ptrType, base, index, scale, disp, 1);
	}

	void RCLRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2RM(ptrType, base, index, scale, disp, 2);
	}

	void RCRRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2RM(ptrType, base, index, scale, disp, 3);
	}

	void SHLRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2RM(ptrType, base, index, scale, disp, 4);
	}

	void SHRRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2RM(ptrType, base, index, scale, disp, 5);
	}

	void SALRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2RM(ptrType, base, index, scale, disp, 6);
	}

	void SARRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		Group2RM(ptrType, base, index, scale, disp, 7);
	}

	void RETN(uint16 iw)
	{
		if(iw == 0)
		{
			*pEmitLoc++ = 0xC3;
		}
		else
		{
			*pEmitLoc++ = 0xC2;
			*((uint16 *)pEmitLoc) = iw;
			pEmitLoc += sizeof(uint16);
		}
	}

	void MOVIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(ptrType == x86MemPtr_byte)
		{
			//m8,imm8
			*pEmitLoc++ = 0xC6;
			ModRegRM(x86ModType_mem,(x86ModReg)0,base,index,scale,disp);
			*pEmitLoc++ = (int8)imm;
		}
		else if(ptrType == x86MemPtr_word)
		{
			//m16, imm16
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xC7;
			ModRegRM(x86ModType_mem,(x86ModReg)0,base,index,scale,disp);
			*((int16 *)pEmitLoc) = (int16)imm;
			pEmitLoc += sizeof(int16);

		}
		else
		{
			//m32, imm32
			*pEmitLoc++ = 0xC7;
			ModRegRM(x86ModType_mem,(x86ModReg)0,base,index,scale,disp);
			*((int32 *)pEmitLoc) = imm;
			pEmitLoc += sizeof(int32);
		}
	}

	void LEAVE()
	{
		*pEmitLoc++ = 0xC9;
	}

	void RETF(uint16 iw)
	{
		if(iw == 0)
		{
			*pEmitLoc++ = 0xCB;
		}
		else
		{
			*pEmitLoc++ = 0xCA;
			*((uint16 *)pEmitLoc) = iw;
			pEmitLoc += sizeof(uint16);
		}
	}
	void INT3()
	{
		*pEmitLoc++ = 0xCC;
	}

	void INT(int8 vector)
	{
		*pEmitLoc++ = 0xCD;
		*pEmitLoc++ = vector;
	}

	void INTO()
	{
		*pEmitLoc++ = 0xCE;
	}

	void IRET()
	{
		*pEmitLoc++ = 0xCF;
	}

	void ROLRR(x86Reg regDest)
	{
		Group2RR(regDest, 0);
	}

	void RORRR(x86Reg regDest)
	{
		Group2RR(regDest, 1);
	}

	void RCLRR(x86Reg regDest)
	{
		Group2RR(regDest, 2);
	}

	void RCRRR(x86Reg regDest)
	{
		Group2RR(regDest, 3);
	}

	void SHLRR(x86Reg regDest)
	{
		Group2RR(regDest, 4);
	}

	void SHRRR(x86Reg regDest)
	{
		Group2RR(regDest, 5);
	}

	void SALRR(x86Reg regDest)
	{
		Group2RR(regDest, 6);
	}

	void SARRR(x86Reg regDest)
	{
		Group2RR(regDest, 7);
	}

	void AAM(uint8 divisor)
	{
		*pEmitLoc++ = 0xD4;
		if(divisor != 10)
		{
			*pEmitLoc++ = divisor;
		}
	}

	void AAD(uint8 divisor)
	{
		*pEmitLoc++ = 0xD5;
		if(divisor != 10)
		{
			*pEmitLoc++ = divisor;
		}
	}

	void XLAT()
	{
		*pEmitLoc++ = 0xD7;
	}

	void ESC0()
	{
		*pEmitLoc++ = 0xD8;
	}

	void ESC1()
	{
		*pEmitLoc++ = 0xD9;
	}

	void ESC2()
	{
		*pEmitLoc++ = 0xDA;
	}

	void ESC3()
	{
		*pEmitLoc++ = 0xDB;
	}

	void ESC4()
	{
		*pEmitLoc++ = 0xDC;
	}

	void ESC5()
	{
		*pEmitLoc++ = 0xDD;
	}

	void ESC6()
	{
		*pEmitLoc++ = 0xDE;
	}

	void ESC7()
	{
		*pEmitLoc++ = 0xDF;
	}

	void LOOPNE(uint8 *pTarget)
	{
		int32 pOffset = pTarget - (pEmitLoc + 2);

		*pEmitLoc++ = 0xE0;
		*pEmitLoc++ = (int8)pOffset;
	}

	void LOOPNE_Label(PatchManager *patchMgr, uint32 labelIndex)
	{
		if(labelIndex >= patchMgr->numLabels)
		{
			patchMgr->AddPatch(pEmitLoc + 1, PatchType_Rel8, pEmitLoc + 2, labelIndex);
			*pEmitLoc++ = 0xE2;
			*pEmitLoc++ = 0;
		}
		else
		{
			LOOP(patchMgr->GetLabelPointer(labelIndex));
		}
	}

	void LOOPE(uint8 *pTarget)
	{
		int32 pOffset = pTarget - (pEmitLoc + 2);

		*pEmitLoc++ = 0xE0;
		*pEmitLoc++ = (int8)pOffset;
	}

	void LOOPE_Label(PatchManager *patchMgr, uint32 labelIndex)
	{
		if(labelIndex >= patchMgr->numLabels)
		{
			patchMgr->AddPatch(pEmitLoc + 1, PatchType_Rel8, pEmitLoc + 2, labelIndex);
			*pEmitLoc++ = 0xE0;
			*pEmitLoc++ = 0;
		}
		else
		{
			LOOPE(patchMgr->GetLabelPointer(labelIndex));
		}
	}

	void LOOP(uint8 *pTarget)
	{
		int32 pOffset = pTarget - (pEmitLoc + 2);

		*pEmitLoc++ = 0xE2;
		*pEmitLoc++ = (int8)pOffset;
	}

	void LOOP_Label(PatchManager *patchMgr, uint32 labelIndex)
	{
		if(labelIndex >= patchMgr->numLabels)
		{
			patchMgr->AddPatch(pEmitLoc + 1, PatchType_Rel8, pEmitLoc + 2, labelIndex);
			*pEmitLoc++ = 0xE2;
			*pEmitLoc++ = 0;
		}
		else
		{
			LOOP(patchMgr->GetLabelPointer(labelIndex));
		}
	}

	void JCXZ(uint8 *pTarget)
	{
		int32 pOffset = pTarget - (pEmitLoc + 3);

		//ADDRESS Prefix
		*pEmitLoc++ = 0x67;
		*pEmitLoc++ = 0xE3;
		*pEmitLoc++ = (int8)pOffset;
	}

	void JCXZ_Label(PatchManager *patchMgr, uint32 labelIndex)
	{
		if(labelIndex >= patchMgr->numLabels)
		{
			patchMgr->AddPatch(pEmitLoc + 2, PatchType_Rel8, pEmitLoc + 3, labelIndex);
			*pEmitLoc++ = 0x67;
			*pEmitLoc++ = 0xE3;
			*pEmitLoc++ = 0;
		}
		else
		{
			LOOP(patchMgr->GetLabelPointer(labelIndex));
		}
	}

	void JECXZ(uint8 *pTarget)
	{
		int32 pOffset = pTarget - (pEmitLoc + 2);

		*pEmitLoc++ = 0xE3;
		*pEmitLoc++ = (int8)pOffset;
	}

	void JECXZ_Label(PatchManager *patchMgr, uint32 labelIndex)
	{
		if(labelIndex >= patchMgr->numLabels)
		{
			patchMgr->AddPatch(pEmitLoc + 1, PatchType_Rel8, pEmitLoc + 2, labelIndex);
			*pEmitLoc++ = 0xE3;
			*pEmitLoc++ = 0;
		}
		else
		{
			JECXZ(patchMgr->GetLabelPointer(labelIndex));
		}
	}

	void INI(x86Reg regDest, uint8 port)
	{
		if(regDest < x86Reg_ax)
		{
			*pEmitLoc++ = 0xE4;
			*pEmitLoc++ = port;
		}
		else if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xE5;
			*pEmitLoc++ = port;
		}
		else
		{
			*pEmitLoc++ = 0xE5;
			*pEmitLoc++ = port;
		}
	}

	void OUTI(x86Reg regDest, uint8 data)
	{
		if(regDest < x86Reg_ax)
		{
			*pEmitLoc++ = 0xE6;
			*pEmitLoc++ = data;
		}
		else if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xE7;
			*pEmitLoc++ = data;
		}
		else
		{
			*pEmitLoc++ = 0xE7;
			*pEmitLoc++ = data;
		}
	}

	void INR(x86Reg regDest)
	{
		if(regDest < x86Reg_ax)
		{
			*pEmitLoc++ = 0xEC;
		}
		else if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xED;
		}
		else
		{
			*pEmitLoc++ = 0xED;
		}
	}

	void OUTR(x86Reg regDest)
	{
		if(regDest < x86Reg_ax)
		{
			*pEmitLoc++ = 0xEE;
		}
		else if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xEF;
		}
		else
		{
			*pEmitLoc++ = 0xEF;
		}
	}

	void LOCK()
	{
		*pEmitLoc++ = 0xF0;
	}

	void INT1()
	{
		*pEmitLoc++ = 0xF1;
	}

	void REPNE()
	{
		*pEmitLoc++ = 0xF2;
	}

	void REPE()
	{
		*pEmitLoc++ = 0xF3;
	}

	void HLT()
	{
		*pEmitLoc++ = 0xF4;
	}

	void CMC()
	{
		*pEmitLoc++ = 0xF5;
	}

	void NOTR(x86Reg regDest)
	{
		if(regDest < x86Reg_ax)
		{
			*pEmitLoc++ = 0xF6;
		}
		else if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xF7;
		}
		else
		{
			*pEmitLoc++ = 0xF7;
		}
		ModRegRM(x86ModType_reg,(x86ModReg)2,regDest & 0x07);
	}

	void NOTM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(ptrType == x86MemPtr_byte)
		{
			*pEmitLoc++ = 0xF6;
		}
		else if(ptrType == x86MemPtr_word)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xF7;
		}
		else
		{
			*pEmitLoc++ = 0xF7;
		}
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void IMULRR(x86Reg regSrc)
	{
		if(regSrc < x86Reg_ax)
		{
			*pEmitLoc++ = 0xF6;
		}
		else
		{
			if(regSrc < x86Reg_eax)
			{
				*pEmitLoc++ = 0x66;
			}
			*pEmitLoc++ = 0xF7;
		}
		ModRegRM(x86ModType_reg,(x86ModReg)5,regSrc & 0x07);
	}

	void IMULMR(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(ptrType == x86MemPtr_byte)
		{
			*pEmitLoc++ = 0xF6;
		}
		else
		{
			if(ptrType == x86MemPtr_word)
			{
				*pEmitLoc++ = 0x66;
			}
			*pEmitLoc++ = 0xF7;
		}

		ModRegRM(x86ModType_mem,(x86ModReg)5,base,index,scale,disp);
	}

	void NEGR(x86Reg regDest)
	{
		if(regDest < x86Reg_ax)
		{
			*pEmitLoc++ = 0xF6;
		}
		else if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xF7;
		}
		else
		{
			*pEmitLoc++ = 0xF7;
		}
		ModRegRM(x86ModType_reg,(x86ModReg)3,regDest & 0x07);
	}

	void TESTIR(uint32 imm, x86Reg regSrc)
	{
		if(regSrc < x86Reg_ax)
		{
			if(regSrc == x86Reg_al)
			{
				*pEmitLoc++ = 0xA8;
				*pEmitLoc++ = (uint8)imm;
			}
			else
			{
				*pEmitLoc++ = 0xF6;
				ModRegRM(x86ModType_reg,(x86ModReg)0,regSrc & 0x07);
				*pEmitLoc++ = (uint8)imm;
			}
		}
		else if(regSrc < x86Reg_eax)
		{
			if(regSrc == x86Reg_ax)
			{
				*pEmitLoc++ = 0x66;
				*pEmitLoc++ = 0xA9;
				*((uint16 *)pEmitLoc) = (uint16)imm;
				pEmitLoc += sizeof(uint16);
			}
			else
			{
				*pEmitLoc++ = 0x66;
				*pEmitLoc++ = 0xF7;
				ModRegRM(x86ModType_reg,(x86ModReg)0,regSrc & 0x07);
				*((uint16 *)pEmitLoc) = (uint16)imm;
				pEmitLoc += sizeof(uint16);
			}
		}
		else
		{
			if(regSrc == x86Reg_eax)
			{
				*pEmitLoc++ = 0xA9;
				*((uint32 *)pEmitLoc) = (uint32)imm;
				pEmitLoc += sizeof(uint32);
			}
			else
			{
				*pEmitLoc++ = 0xF7;
				ModRegRM(x86ModType_reg,(x86ModReg)0,regSrc & 0x07);
				*((uint32 *)pEmitLoc) = imm;
				pEmitLoc += sizeof(uint32);
			}
		}
	}

	void TESTIM(uint32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(ptrType == x86MemPtr_byte)
		{
			*pEmitLoc++ = 0xF6;
			ModRegRM(x86ModType_reg,(x86ModReg)0,base,index,scale,disp);
			*pEmitLoc++ = (uint8)imm;
		}
		else if(ptrType == x86MemPtr_word)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xF7;
			ModRegRM(x86ModType_reg,(x86ModReg)0,base,index,scale,disp);
			*((uint16 *)pEmitLoc) = (uint16)imm;
			pEmitLoc += sizeof(uint16);
		}
		else
		{
			*pEmitLoc++ = 0xF7;
			ModRegRM(x86ModType_reg,(x86ModReg)0,base,index,scale,disp);
			*((uint32 *)pEmitLoc) = imm;
			pEmitLoc += sizeof(uint32);
		}

	}


	void NEGM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(ptrType == x86MemPtr_byte)
		{
			*pEmitLoc++ = 0xF6;
		}
		else if(ptrType == x86MemPtr_word)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xF7;
		}
		else
		{
			*pEmitLoc++ = 0xF7;
		}
		ModRegRM(x86ModType_mem,(x86ModReg)3,base,index,scale,disp);
	}

	void CLC()
	{
		*pEmitLoc++ = 0xF8;
	}

	void STC()
	{
		*pEmitLoc++ = 0xF9;
	}

	void CLI()
	{
		*pEmitLoc++ = 0xFA;
	}

	void STI()
	{
		*pEmitLoc++ = 0xFB;
	}

	void CLD()
	{
		*pEmitLoc++ = 0xFC;
	}

	void STD()
	{
		*pEmitLoc++ = 0xFD;
	}

	void INCM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(ptrType == x86MemPtr_byte)
		{
			*pEmitLoc++ = 0xFE;
		}
		else if(ptrType == x86MemPtr_word)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xFF;
		}
		else
		{
			*pEmitLoc++ = 0xFF;
		}
		ModRegRM(x86ModType_mem,(x86ModReg)0,base,index,scale,disp);
	}

	void DECM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(ptrType == x86MemPtr_byte)
		{
			*pEmitLoc++ = 0xFE;
		}
		else if(ptrType == x86MemPtr_word)
		{
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xFF;
		}
		else
		{
			*pEmitLoc++ = 0xFF;
		}
		ModRegRM(x86ModType_mem,(x86ModReg)1,base,index,scale,disp);
	}

	void CALLNM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0xFF;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void CALLR(x86Reg regSrc)
	{
		if(regSrc < x86Reg_eax)
		{
			//CALL r16
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xFF;
			ModRegRM(x86ModType_reg,(x86ModReg)2,(regSrc & 0x07));
		}
		else
		{
			//CALL r32
			*pEmitLoc++ = 0xFF;
		}
		ModRegRM(x86ModType_reg,(x86ModReg)2,(regSrc & 0x07));
	}

	void CALLFM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0xFF;
		ModRegRM(x86ModType_mem,(x86ModReg)3,base,index,scale,disp);
	}

	void JMPR(x86Reg regSrc)
	{
		if(regSrc < x86Reg_eax)
		{
			//JMP r16
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0xFF;
			ModRegRM(x86ModType_reg,(x86ModReg)4,(regSrc & 0x07));
		}
		else
		{
			//JMP r32
			*pEmitLoc++ = 0xFF;
			ModRegRM(x86ModType_reg,(x86ModReg)4,(regSrc & 0x07));
		}
	}

	void JMPNM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0xFF;
		ModRegRM(x86ModType_mem,(x86ModReg)4,base,index,scale,disp);
	}

	void JMPFM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0xFF;
		ModRegRM(x86ModType_mem,(x86ModReg)5,base,index,scale,disp);
	}

	void PUSHM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(ptrType == x86MemPtr_word)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0xFF;
		ModRegRM(x86ModType_mem,(x86ModReg)6,base,index,scale,disp);
	}

	void INVD(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x08;
	}

	void WBINVD(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x09;
	}

	void UD1(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x0B;
	}

	void BSWAP(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0xC8 + (reg & 0x07);
	}

	void SETOR(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x90;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETOM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x90;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETNOR(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x91;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETNOM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x91;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETBR(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x92;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETBM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x92;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETNBR(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x93;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETNBM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x93;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETZR(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x94;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETZM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x94;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETNZR(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x95;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETNZM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x95;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETBER(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x96;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETBEM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x96;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETNBER(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x97;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETNBEM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x97;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETSR(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x98;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETSM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x98;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETNSR(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x99;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETNSM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x99;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETPR(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x9A;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETPM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x9A;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETNPR(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x9B;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETNPM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x9B;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETLR(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x9C;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETLM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x9C;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETNLR(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x9D;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETNLM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x9D;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETLER(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x9E;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETLEM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x9E;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void SETNLER(x86Reg reg)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x9F;
		ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
	}

	void SETNLEM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x9F;
		ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
	}

	void CMOVORR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x40;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVOMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x40;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVNORR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x41;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVNOMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x41;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVBRR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x42;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVBMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x42;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVNBRR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x43;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVNBMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x43;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVZRR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x44;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVZMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x44;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVNZRR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x45;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVNZMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x45;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVBERR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x46;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVBEMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x46;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVNBERR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x47;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVNBEMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x47;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	//

	void CMOVSRR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x48;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVSMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x48;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVNSRR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x49;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVNSMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x49;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVPRR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x4A;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVPMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x4A;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVNPRR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x4B;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVNPMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x4B;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVLRR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x4C;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVLMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x4C;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVNLRR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x4D;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVNLMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x4D;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVLERR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x4E;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVLEMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x4E;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void CMOVNLERR(x86Reg regDest, uint32 regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x4F;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void CMOVNLEMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			//r16, r16
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0x4F;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
	}

	void MOVQRR(x86Reg regDest, x86Reg regSrc)
	{
		if(regDest < x86Reg_xmm0)
		{
			//MMX
			*pEmitLoc++ = 0x0F;
			*pEmitLoc++ = 0x6F;
		}
		else
		{
			//SSE2
			*pEmitLoc++ = 0xF3;
			*pEmitLoc++ = 0x0F;
			*pEmitLoc++ = 0x7E;
		}
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void MOVQMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_xmm0)
		{
			//MMX
			*pEmitLoc++ = 0x0F;
			*pEmitLoc++ = 0x6F;
		}
		else
		{
			//SSE2
			*pEmitLoc++ = 0xF3;
			*pEmitLoc++ = 0x0F;
			*pEmitLoc++ = 0x7E;
		}
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),(base & 0x07),index, scale, disp);
	}

	void MOVQRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regSrc < x86Reg_xmm0)
		{
			//MMX
			*pEmitLoc++ = 0x0F;
			*pEmitLoc++ = 0x7F;
		}
		else
		{
			//SSE2
			*pEmitLoc++ = 0x66;
			*pEmitLoc++ = 0x0F;
			*pEmitLoc++ = 0xD6;
		}
		ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),(base & 0x07),index, scale, disp);
	}

	void SHLDIRR(x86Reg regDest, x86Reg regSrc, uint8 shiftCount)
	{
		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0xA4;
		ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
		*pEmitLoc++ = shiftCount;

	}

	void SHLDRRR(x86Reg regDest, x86Reg regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0xA5;
		ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
	}

	void SHLDIMR(x86Reg regDest, x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0xA4;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),(base & 0x07),index, scale, disp);
		*pEmitLoc++ = shiftCount;

	}

	void SHLDRMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0xA5;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),(base & 0x07),index, scale, disp);
	}

	void SHRDIRR(x86Reg regDest, x86Reg regSrc, uint8 shiftCount)
	{
		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0xAC;
		ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
		*pEmitLoc++ = shiftCount;

	}

	void SHRDRRR(x86Reg regDest, x86Reg regSrc)
	{
		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0xAD;
		ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
	}

	void SHRDIMR(x86Reg regDest, x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0xAC;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),(base & 0x07),index, scale, disp);
		*pEmitLoc++ = shiftCount;

	}

	void SHRDRMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}

		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = 0xAD;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),(base & 0x07),index, scale, disp);
	}

	void MOVZXRR(x86Reg regDest, uint32 regSrc)
	{
		uint8 opcode = 0xB6;

		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}
		else if(regSrc >= x86Reg_ax)
		{
			opcode = 0xB7;
		}
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = opcode;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void MOVZXMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		uint8 opcode = 0xB6;

		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}
		else if(ptrType == x86MemPtr_word)
		{
			opcode = 0xB7;
		}
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = opcode;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base, index, scale, disp);
	}

	void MOVSXRR(x86Reg regDest, uint32 regSrc)
	{
		uint8 opcode = 0xBE;

		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}
		else if(regSrc >= x86Reg_ax)
		{
			opcode = 0xBF;
		}
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = opcode;
		ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
	}

	void MOVSXMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
	{
		uint8 opcode = 0xBE;

		if(regDest < x86Reg_eax)
		{
			*pEmitLoc++ = 0x66;
		}
		else if(ptrType == x86MemPtr_dword)
		{
			opcode = 0xBF;
		}
		*pEmitLoc++ = 0x0F;
		*pEmitLoc++ = opcode;
		ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base ,index, scale, disp);
	}

};
#endif

