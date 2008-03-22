/************************************************************************/
/* Arm/Thumb command set disassembler                                   */
/************************************************************************/
#include "stdafx.h"
#include "base.h"
#include <stdio.h>
#include "arm7Memory.h"
#include "armdis.h"


struct Opcodes {
	DWORD mask;
	DWORD cval;
	char *mnemonic;
};

char* elfGetAddressSymbol(DWORD uAddress)
{
	return NULL;
}

#define debuggerReadMemory(addr) \
	Arm7ReadMemoryQuick(addr)

#define debuggerReadHalfWord(addr) \
	Arm7ReadHalfWordQuick(addr)

#define debuggerReadByte(addr) \
	Arm7ReadByteQuick(addr)

const char hdig[] = "0123456789abcdef";

const char *decVals[16] = {
	"0","1","2","3","4","5","6","7","8",
	"9","10","11","12","13","14","15"
};

const char *regs[16] = {
	"r0","r1","r2","r3","r4","r5","r6","r7",
	"r8","r9","r10","r11","r12","sp","lr","pc"
};

const char *conditions[16] = {
	"eq","ne","cs","cc","mi","pl","vs","vc",
	"hi","ls","ge","lt","gt","le","","nv"
};

const char *shifts[5] = {
	"lsl","lsr","asr","ror","rrx"
};

const char *armMultLoadStore[12] = {
	// non-stack
	"da","ia","db","ib",
	// stack store
	"ed","ea","fd","fa",
	// stack load
	"fa","fd","ea","ed"
};
const Opcodes armOpcodes[] = {
	// Undefined
	{0x0e000010, 0x06000010, "[ undefined ]"},
	// Branch instructions
	{0x0ff000f0, 0x01200010, "bx%c %r0"},
	{0x0f000000, 0x0a000000, "b%c %o"},
	{0x0f000000, 0x0b000000, "bl%c %o"},
	{0x0f000000, 0x0f000000, "swi%c %q"},
	// PSR transfer
	{0x0fbf0fff, 0x010f0000, "mrs%c %r3, %p"},
	{0x0db0f000, 0x0120f000, "msr%c %p, %i"},
	// Multiply instructions
	{0x0fe000f0, 0x00000090, "mul%c%s %r4, %r0, %r2"},
	{0x0fe000f0, 0x00200090, "mla%c%s %r4, %r0, %r2, %r3"},
	{0x0fa000f0, 0x00800090, "%umull%c%s %r3, %r4, %r0, %r2"},
	{0x0fa000f0, 0x00a00090, "%umlal%c%s %r3, %r4, %r0, %r2"},
	// Load/Store instructions
	{0x0fb00ff0, 0x01000090, "swp%c%b %r3, %r0, [%r4]"},
	{0x0fb000f0, 0x01000090, "[ ??? ]"},
	{0x0c100000, 0x04000000, "str%c%b%t %r3, %a"},
	{0x0c100000, 0x04100000, "ldr%c%b%t %r3, %a"},
	{0x0e100090, 0x00000090, "str%c%h %r3, %a"},
	{0x0e100090, 0x00100090, "ldr%c%h %r3, %a"},
	{0x0e100000, 0x08000000, "stm%c%m %r4%l"},
	{0x0e100000, 0x08100000, "ldm%c%m %r4%l"},
	// Data processing
	{0x0de00000, 0x00000000, "and%c%s %r3, %r4, %i"},
	{0x0de00000, 0x00200000, "eor%c%s %r3, %r4, %i"},
	{0x0de00000, 0x00400000, "sub%c%s %r3, %r4, %i"},
	{0x0de00000, 0x00600000, "rsb%c%s %r3, %r4, %i"},
	{0x0de00000, 0x00800000, "add%c%s %r3, %r4, %i"},
	{0x0de00000, 0x00a00000, "adc%c%s %r3, %r4, %i"},
	{0x0de00000, 0x00c00000, "sbc%c%s %r3, %r4, %i"},
	{0x0de00000, 0x00e00000, "rsc%c%s %r3, %r4, %i"},
	{0x0de00000, 0x01000000, "tst%c%s %r4, %i"},
	{0x0de00000, 0x01200000, "teq%c%s %r4, %i"},
	{0x0de00000, 0x01400000, "cmp%c%s %r4, %i"},
	{0x0de00000, 0x01600000, "cmn%c%s %r4, %i"},
	{0x0de00000, 0x01800000, "orr%c%s %r3, %r4, %i"},
	{0x0de00000, 0x01a00000, "mov%c%s %r3, %i"},
	{0x0de00000, 0x01c00000, "bic%c%s %r3, %r4, %i"},
	{0x0de00000, 0x01e00000, "mvn%c%s %r3, %i"},
	// Coprocessor operations
	{0x0f000010, 0x0e000000, "cdp%c %P, %N, %r3, %R4, %R0%V"},
	{0x0e100000, 0x0c000000, "stc%c%L %P, %r3, %A"},
	{0x0f100010, 0x0e000010, "mcr%c %P, %N, %r3, %R4, %R0%V"},
	{0x0f100010, 0x0e100010, "mrc%c %P, %N, %r3, %R4, %R0%V"},
	// Unknown
	{0x00000000, 0x00000000, "[ ??? ]"}
};

char* addStr(char *dest, const char *src){
	while (*src){
		*dest++ = *src++;
	}
	return dest;
}

char* addHex(char *dest, int siz, DWORD val){
	if (siz==0){
		siz = 28;
		while ( (((val>>siz)&15)==0) && (siz>=4) )
			siz -= 4;
		siz += 4;
	}
	while (siz>0){
		siz -= 4;
		*dest++ = hdig[(val>>siz)&15];
	}
	return dest;
}

int disArm(DWORD offset, char *dest, int flags){
	DWORD opcode = debuggerReadMemory(offset);

	const Opcodes *sp = armOpcodes;
	while( sp->cval != (opcode & sp->mask) )
		sp++;

	if (flags&DIS_VIEW_ADDRESS){
		dest = addHex(dest, 32, offset);
		*dest++ = ' ';
	}
	if (flags&DIS_VIEW_CODE){
		dest = addHex(dest, 32, opcode);
		*dest++ = ' ';
	}

	char *src = sp->mnemonic;
	while (*src){
		if (*src!='%')
			*dest++ = *src++;
		else{
			src++;
			switch (*src){
	  case 'c':
		  dest = addStr(dest, conditions[opcode>>28]);
		  break;
	  case 'r':
		  dest = addStr(dest, regs[(opcode>>((*(++src)-'0')*4))&15]);
		  break;
	  case 'o':
		  {
			  *dest++ = '$';
			  int off = opcode&0xffffff;
			  if (off&0x800000)
				  off |= 0xff000000;
			  off <<= 2;
			  dest = addHex(dest, 32, offset+8+off);
		  }
		  break;
	  case 'i':
		  if (opcode&(1<<25)){
			  dest = addStr(dest, "#0x");
			  int imm = opcode&0xff;
			  int rot = (opcode&0xf00)>>7;
			  int val = (imm<<(32-rot))|(imm>>rot);
			  dest = addHex(dest, 0, val);
		  } else{
			  dest = addStr(dest, regs[opcode&0x0f]);
			  int shi = (opcode>>5)&3;
			  int sdw = (opcode>>7)&0x1f;
			  if ((sdw==0)&&(shi==3))
				  shi = 4;
			  if ( (sdw) || (opcode&0x10) || (shi)) {
				  dest = addStr(dest, ", ");
				  dest = addStr(dest, shifts[shi]);
				  if (opcode&0x10){
					  *dest++ = ' ';
					  dest = addStr(dest, regs[(opcode>>8)&15]);
				  } else {
					  if (sdw==0 && ( (shi==1) || (shi==2) ))
						  sdw = 32;
					  if(shi != 4) {
						  dest = addStr(dest, " #0x");
						  dest = addHex(dest, 8, sdw);
					  }
				  }
			  }
		  }
		  break;
	  case 'p':
		  if (opcode&(1<<22))
			  dest = addStr(dest, "spsr");
		  else
			  dest = addStr(dest, "cpsr");
		  if(opcode & 0x00F00000) {
			  *dest++ = '_';
			  if(opcode & 0x00080000)
				  *dest++ = 'f';
			  if(opcode & 0x00040000)
				  *dest++ = 's';
			  if(opcode & 0x00020000)
				  *dest++ = 'x';
			  if(opcode & 0x00010000)
				  *dest++ = 'c';
		  }
		  break;
	  case 's':
		  if (opcode&(1<<20))
			  *dest++ = 's';
		  break;
	  case 'S':
		  if (opcode&(1<<22))
			  *dest++ = 's';
		  break;
	  case 'u':
		  if (opcode&(1<<22))
			  *dest++ = 's';
		  else
			  *dest++ = 'u';
		  break;
	  case 'b':
		  if (opcode&(1<<22))
			  *dest++ = 'b';
		  break;
	  case 'a':
		  if ((opcode&0x076f0000)==0x004f0000){
			  *dest++ = '[';
			  *dest++ = '$';
			  int adr = offset+8;
			  int add = (opcode&15)|((opcode>>8)&0xf0);
			  if (opcode&(1<<23))
				  adr += add;
			  else
				  adr -= add;
			  dest = addHex(dest, 32, adr);
			  *dest++ = ']';
			  dest = addStr(dest, " (=");
			  *dest++ = '$';
			  dest = addHex(dest ,32, debuggerReadMemory(adr));
			  *dest++=')';
		  }
		  if ((opcode&0x072f0000)==0x050f0000){
			  *dest++ = '[';
			  *dest++ = '$';
			  int adr = offset+8;
			  if (opcode&(1<<23))
				  adr += opcode&0xfff;
			  else
				  adr -= opcode&0xfff;
			  dest = addHex(dest, 32, adr);
			  *dest++ = ']';
			  dest = addStr(dest, " (=");
			  *dest++ = '$';
			  dest = addHex(dest ,32, debuggerReadMemory(adr));
			  *dest++=')';
		  } else {
			  int reg = (opcode>>16)&15;
			  *dest++ = '[';
			  dest = addStr(dest, regs[reg]);
			  if (!(opcode&(1<<24)))
				  *dest++ = ']';
			  if ( ((opcode&(1<<25))&&(opcode&(1<<26))) || (!(opcode&(1<<22))&&!(opcode&(1<<26))) ){
				  dest = addStr(dest, ", ");
				  if (!(opcode&(1<<23)))
					  *dest++ = '-';
				  dest = addStr(dest, regs[opcode&0x0f]);
				  int shi = (opcode>>5)&3;
				  if (opcode&(1<<26)){
					  if ( ((opcode>>7)&0x1f) || (opcode&0x10) || (shi==1) || (shi==2)){
						  dest = addStr(dest, ", ");
						  dest = addStr(dest, shifts[shi]);
						  if (opcode&0x10){
							  *dest++ = ' ';
							  dest = addStr(dest, regs[(opcode>>8)&15]);
						  } else {
							  int sdw = (opcode>>7)&0x1f;
							  if (sdw==0 && ( (shi==1) || (shi==2) ))
								  sdw = 32;
							  dest = addStr(dest, " #0x");
							  dest = addHex(dest, 8, sdw);
						  }
					  }
				  }
			  } else {
				  int off;
				  if (opcode&(1<<26))
					  off = opcode&0xfff;
				  else
					  off = (opcode&15)|((opcode>>4)&0xf0);
				  if (off){
					  dest = addStr(dest, ", ");
					  if (!(opcode&(1<<23)))
						  *dest++ = '-';
					  dest = addStr(dest, "#0x");
					  dest = addHex(dest, 0, off);
				  }
			  }
			  if (opcode&(1<<24)){
				  *dest++ = ']';
				  if (opcode&(1<<21))
					  *dest++ = '!';
			  }
		  }
		  break;
	  case 't':
		  if ((opcode&0x01200000)==0x01200000)
			  *dest++ = 't';
		  break;
	  case 'h':
		  if (opcode&(1<<6))
			  *dest++ = 's';
		  if (opcode&(1<<5))
			  *dest++ = 'h';
		  else
			  *dest++ = 'b';
		  break;
	  case 'm':
		  if (((opcode>>16)&15)==13) {
			  if(opcode & 0x00100000)
				  dest = addStr(dest, armMultLoadStore[8+((opcode>>23)&3)]);
			  else
				  dest = addStr(dest, armMultLoadStore[4+((opcode>>23)&3)]);      
		  } else
			  dest = addStr(dest, armMultLoadStore[(opcode>>23)&3]);
		  break;
	  case 'l':
		  if (opcode&(1<<21))
			  *dest++ = '!';
		  dest = addStr(dest, ", {");
		  {
			  int rlst = opcode&0xffff;
			  int msk = 0;
			  int not_first = 0;
			  while (msk<16){
				  if (rlst&(1<<msk)){
					  int fr = msk;
					  while (rlst&(1<<msk))
						  msk++;
					  int to = msk-1;
					  if (not_first)
						  //dest = addStr(dest, ", ");
						  *dest++ = ',';
					  dest = addStr(dest, regs[fr]);
					  if (fr!=to){
						  if (fr==to-1)
							  //dest = addStr(", ");
							  *dest++ = ',';
						  else
							  *dest++ = '-';
						  dest = addStr(dest, regs[to]);
					  }
					  not_first = 1;
				  } else
					  msk++;
			  }
			  *dest++ = '}';
			  if (opcode&(1<<22))
				  *dest++ = '^';
		  }
		  break;
	  case 'q':
		  *dest++ = '$';
		  dest = addHex(dest, 24, opcode&0xffffff);
		  break;
	  case 'P':
		  *dest++ = 'p';
		  dest = addStr(dest, decVals[(opcode>>8)&15]);
		  break;
	  case 'N':
		  if (opcode&0x10)
			  dest = addStr(dest, decVals[(opcode>>21)&7]);
		  else
			  dest = addStr(dest, decVals[(opcode>>20)&15]);
		  break;
	  case 'R':
		  {
			  src++;
			  int reg = 4*(*src-'0');
			  *dest++ = 'c';
			  dest = addStr(dest, decVals[(opcode>>reg)&15]);
		  }
		  break;
	  case 'V':
		  {
			  int val = (opcode>>5)&7;
			  if (val){
				  dest = addStr(dest, ", ");
				  dest = addStr(dest, decVals[val]);
			  }
		  }
		  break;
	  case 'L':
		  if (opcode&(1<<22))
			  *dest++ = 'l';
		  break;
	  case 'A':
		  if ((opcode&0x012f0000)==0x010f0000){
			  int adr = offset+8;
			  int add = (opcode&0xff)<<2;
			  if (opcode&(1<<23))
				  adr += add;
			  else
				  adr -= add;
			  *dest++ = '$';
			  addHex(dest, 32, adr);
		  } else {
			  *dest++ = '[';
			  dest = addStr(dest, regs[(opcode>>16)&15]);
			  if (!(opcode&(1<<24)))
				  *dest++ = ']';
			  int off = (opcode&0xff)<<2;
			  if (off){
				  dest = addStr(dest, ", ");
				  if (!(opcode&(1<<23)))
					  *dest++ = '-';
				  dest = addStr(dest, "#0x");
				  dest = addHex(dest, 0, off);
			  }
			  if (opcode&(1<<24)){
				  *dest++ = ']';
				  if (opcode&(1<<21))
					  *dest++ = '!';
			  }
		  }
		  break;
			}
			src++;
		}
	}
	*dest++ = 0;

	return 4;
}
