//Arm7dynarec
//drk||Raziel 2007-2008
/*
	Simple dynarec, designed for compact size, quite good compiled code speed and specialised for dreamcast/aica/arm7di emulation.THIS IS NOT A GENERIC CORE.

	Features :
	 -Direct mapped code execution
	 -Direct opcode compilation
	 -Static register usage
	 -Very fast compilation, w/o any analysis, no blocks, no links.
	
	Compile unit : Opcode
	 Each opcode is indepedant from other opocodes.[There are no direct links betwen blocks?].Static register allocation.
	 Blocks are formed from stream of opcodes til an opcode that edits the stream flow.

    Block manager :
	 -The entry lookup table is 1:1 with ram.There are no special mode caches.
	 -The block pointer is at -4 offset over the head opcode entry.
*/

//Frequency defines ... bout time i wrote em ;p
#define GD_CLOCK 33868800				//XTAL i guess ;)

#define AICA_CORE_CLOCK (GD_CLOCK*2/3)	//[22579200] GD->PLL 2/3 -> AICA CORE
#define ADAC_CLOCK (AICA_CORE_CLOCK/2)	//[11289600] 44100*256, AICA CORE -> PLL 1/2 -> ADAC.I use that as a base for calculations ;)
/*
 We dont know the real clock for the arm (i have to verify)
 Some docs say 22.5.Some say 25 (?).Some say 45 -.-.
  • ~22.5 mhz is the clock provided to aica.Arm could run at this clock .. but its quite 
    low, even for audio cpu (especialy taking in acout the low power/heat of arm).Then again i think its .25 process ;\
  • ~25M[24576000] whould be aica's clock if it was doing 48000 samples/sec.
    my guess is that dc designers were thinking 48kz audio but never implemented it
  • ~45M[45056000] 11:6 pll from the CORE clock or ~45M[45158400] AICA_CORE_CLOCK*2.Many places give this value as the clock (Wikipedia, System16, more..) 
    but it doesnt appear anywhere on the official docs (they dont say anything about the arm clock oO)
*/
#define AICA_ARM_CLOCK (AICA_CORE_CLOCK)	//[22579200] AICA CORE -> ARM .Safe for now, need to check if 45 mhz

#define AICA_SDRAM_CLOCK (GD_CLOCK*2)   //[67737600] GD-> PLL 2 -> SDRAM

#define ARM_REC_CACHE_SIZE (5*1024*1024)

typedef void DynaEntryFP();
/*
struct BasicBlock
{
	u16 size;
};
*/
u8*			 DynaCodeBuffer[ARM_REC_CACHE_SIZE];
u8			 DynaCacheLookup[ARM_REC_CACHE_SIZE/16*3];
DynaEntryFP* DynaEntryPoints[AICA_MEMMAP_RAM_SIZE/4];
u32			 DynaEntryCycles[AICA_RAM_SIZE/4];

s32 RecCycles;
struct ArmContext
{
	u32 r[15];

	//more
	//irqt/pre-exit related
	DynaEntryFP**stack_base;	//0 when not inside dynarec :)
	struct 
	{
		DynaEntryFP* handler;		//0 if dynarec main hasnt been called, constant otherwise.
		DynaEntryFP* code_addr;		//address of the code that we need to unwind :)
	}unwind;
};
/*
	Static reg alloc
	eax: temp {ax;ah,al}
	ecx: temp {cx;dh,cl}
	edx: temp {dx;dh,dl}
	
	esp: stack [rly]

	ebx: flags (?) {bx;bh,bl} : {s/n,o/v}
	ebp: flags (?) {bp}		  : {c<<8 | z}
	esi: context pointer {si}
	edi: cycle count     {di}
	
*/
ArmContext armc;

//make the arm core to check interrupts after the currently executing opcode
//valid ONLY when called from a callframe that ends in the dynarec code ;)
void arm_do_irqt()
{
	if (armc.stack_base)
	{
		DynaEntryFP* p=*armc.stack_base;
		*armc.stack_base=armc.unwind.handler;
		armc.unwind.code_addr=p;
	}
}
void arm_irqt()
{
	//check irqs n stuff
	/*
	if (armFiqEnable && e68k_out)
	{
		CPUFiq();
	}
	*/
}
/*
	case 0x00:
		// EQ 
      cond_res = Z_FLAG;
	  tst ebp,1
	  jnz [skip]
      break;
    case 0x01: // NE
      cond_res = !Z_FLAG;
	  tst ebp,1
	  jz [skip]
      break; 
    case 0x02: // CS
      cond_res = C_FLAG;
	  tst ebp,-2 / bt ebp,9
	  jnz [skip] / jnc [skip]
      break;
    case 0x03: // CC
      cond_res = !C_FLAG;
	  tst ebp,-2 / bt ebp,9
	  jz [skip] / jc [skip]
      break;
    case 0x04: // MI
      cond_res = N_FLAG;
	  tst bh,bh
	  jz [skip]
      break;
    case 0x05: // PL
      cond_res = !N_FLAG;
	  tst bh,bh
	  jnz [skip]
      break;
    case 0x06: // VS
      cond_res = V_FLAG;
	  tst bl,bl
	  jz [skip]
      break;
    case 0x07: // VC
      cond_res = !V_FLAG;
	  tst bl,bl
	  jnz [skip]
      break;
    case 0x08: // HI
      cond_res = C_FLAG && !Z_FLAG;
	  cmp ebp,1<<9
	  jne [skip]
      break;
    case 0x09: // LS -- NOT HI
      cond_res = !C_FLAG || Z_FLAG;
	  cmp ebp,1<<9
	  je [skip]
      break;
    case 0x0A: // GE
      cond_res = N_FLAG == V_FLAG;
	  cmp bh,bl
	  jnz [skip]
      break;
    case 0x0B: // LT -- NOT GE
      cond_res = N_FLAG != V_FLAG;
	  cmp bh,bl
	  jz [skip]
      break;
    case 0x0C: // GT
      cond_res = !Z_FLAG &&(N_FLAG == V_FLAG);
	  lea eax,[ebp*2];	//al:bit1 = Z flag
	  sub al,bh;		//this is a cheap way to expand the xor. a-b do 0 , -1 , 1 , 0.xor does 0 , 1 , 1 , 0. I add Z as a most important bit
	  add al,bl;		//so that its allways not zero if z is set.in order to be 0, z must be 0 and a-b must be zero.thats z|(a^b)=0 :D
	  jnz [skip]
      break;    
    case 0x0D: // LE -- NOT GT
      cond_res = Z_FLAG || (N_FLAG != V_FLAG);
	  lea eax,[ebp*2];	//read above, same implementation but checks on inverted result :)
	  sub al,bh;		
	  add al,bl;		
	  jz [skip]
      break; 
    case 0x0E: 
      cond_res = true; 
	  //yea well no test on generated asm ;)
      break;
    case 0x0F: //NEVER / undefined [shoudnt be used, but if used its nevarr]
    default:
      // skip opocode 
      cond_res = false;
      break;
    }
*/
void dyna_unwind_irqt()
{
	//we need to restore cycle count and pc [and possibly other stuff later]
	u32 offs=(u8*)armc.unwind.code_addr-DynaCodeBuffer;
	offs=offs/16*3;
	
	u32 addrg=(*(u32*)&DynaCacheLookup[offs]) & (0xFFFFFF);
	verify(addrg!=0xFFFFFF);//if this was called block must exist ...
	/*
	//addrg gives the 'base' for that region, we just need to scan forward ;)

	u32 currcode=(u32)armc.unwind.code_addr;
	u32* dep=(u32*)DynaEntryPoints;
	
	u32 i;
	for (i=addrg;;i++)
	{
		if (dep[i]>currcode)
			break;
	}
	*/
	//ram[addrg] belongs on the block we want
	//DynaCacheLookup
	arm_irqt();
}
//Asm part of the dynarec driver
#define ASMREG(i) [esi+i*4]
naked void _dyna_regload()
{
	__asm
	{
		mov esi,[RecCycles];
		//? flags ?
		ret;
	}
}
naked void _dyna_regflush()
{
	__asm
	{
		//Write back allocated stuff
		mov [RecCycles],esi;	//cycle count
		//? flags ?
		//and return
		ret;
	}
}
naked void FASTCALL Execute(u32 cycl)
{
	__asm
	{
		//init some values
		//this realy needs to be done only once .. ohh well
		mov armc.dyna_unwind,_dyna_unwind;

		//ecx=cycle
		//save some shit
		push edi;
		push esi;
		push ebp;
		push ebx;

		mov edi,offset armc;//this isnt handled on regload (its not an arm/related register, and doesnt have to be writen back.
		
		call _dyna_regload;	//load regs
		
		add esi,ecx;		//Update cycle count !
		//js _cleanup; //not realy needed unless we run very big blocks compared to cycle counts ...
		mov armc.stack_base,esp;	//save the stack base
		//All init/load done ! time to run some code !

_dyna_execblock:
		//get block
		mov edx,ASMREG(15);	//get r15
		and edx,AICA_MEMMAP_RAM_MASK;
		mov ecx,[DynaEntryPoints+edx];
		//add up pre-cycles
		and edx,AICA_RAM_MASK;
		add esi,[DynaEntryCycles+edx];
		//and jump to it ;)
		jmp ecx;



_dyna_unwind:
		//flow was interrupted due to interrupt !
		call _dyna_regflush;	//get registers to ram
		call dyna_unwind_irqt;	//check interrupts
		call _dyna_regload;		//reload regs to regs
		
		//do we have cycles to execute more ?
		cmp esi,0;
		jle _dyna_exit;	//if no exit
		jmp _dyna_execblock;//else execute more !
	


_dyna_exit:		//exit from dynarec ;)
		mov armc.stack_base,0;
		call _dyna_regflush;
_cleanup:
		//restore registers :)
		pop  ebx;
		pop  ebp;
		pop  esi;
		pop  edi;
	}
}
