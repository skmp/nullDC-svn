#include "types.h"

struct RegInfo
{
	u32 id;
};

struct Label
{
	char* name;
	u32*  patch;
};

enum ConditionCode
{
	nz,
	z
};

struct CpuInfo
{
	//
	u32 nil;
};



#define SIB 4
#define DISP32 5

// general types
enum x86IntRegType
{
   EAX = 0,
   EBX = 3,
   ECX = 1,
   EDX = 2,
   ESI = 6,
   EDI = 7,
   EBP = 5,
   ESP = 4,

   RAX = 0,
   RBX = 3,
   RCX = 1,
   RDX = 2,
   RSI = 6,
   RDI = 7,
   RBP = 5,
   RSP = 4,
   R8  = 8, 
   R9  = 9, 
   R10 =10, 
   R11 =11,
   R12 =12,
   R13 =13,
   R14 =14, 
   R15 =15,
   GRP_Reserved=0xFFFFFFFE,
   GPR_Error=0xFFFFFFFF
} ;

typedef enum
{
   MM0 = 0,
   MM1 = 1,
   MM2 = 2,
   MM3 = 3,
   MM4 = 4,
   MM5 = 5,
   MM6 = 6,
   MM7 = 7,
   MM_Reserved=0xFFFFFFFE,
   MM_Error=0xFFFFFFFF
} x86MMXRegType;

typedef enum
{
   XMM0 = 0,
   XMM1 = 1,
   XMM2 = 2,
   XMM3 = 3,
   XMM4 = 4,
   XMM5 = 5,
   XMM6 = 6,
   XMM7 = 7,
   XMM8 = 8,
   XMM9 = 9,
   XMM10=10,
   XMM11=11,
   XMM12=12,
   XMM13=13,
   XMM14=14,
   XMM15=15,
   XMM_Reserved=0xFFFFFFFE,
   XMM_Error=0xFFFFFFFF
} x86SSERegType;


enum x86_8breg
{
	Reg_AL=0x0,
	Reg_CL,
	Reg_DL,
	Reg_BL,
	Reg_AH,
	Reg_CH,
	Reg_DH,
	Reg_BH,
};


void* EmitAlloc(u32 minsize);
void EmitAllocSet(void * ptr,u32 usedsize);
template <int DefSize=12*1024>
class emitter
{
private:
	s8* x86Ptr_base;
	s8* x86Ptr_end;

	u32 x86Ptr_size;
	GrowingList<Label> labels;

	//size is never > 4 GB 
	void Resizex86Ptr()
	{ 
		printf("\t\n WARNING *Resizex86Ptr* *Resizex86Ptr* \n *Resizex86Ptr*\n");
		u32 offset=x86Ptr-x86Ptr_base;

		//x86Ptr_base=(s8*)realloc(x86Ptr_base,x86Ptr_size*3/2);

		x86Ptr=x86Ptr_base+offset;//new base so recalc

		x86Ptr_size=x86Ptr_size*3/2;
		
		x86Ptr_end=x86Ptr_base+x86Ptr_size;
	}
	////////////////////////////////////////////////////
	void write8( u8 val ) 
	{ 
		if ((x86Ptr_end-x86Ptr)<32)
			Resizex86Ptr();
		*(u8*)x86Ptr = val; 
		x86Ptr++; 
	}

	////////////////////////////////////////////////////
	void write16( u16 val )
	{ 
		if ((x86Ptr_end-x86Ptr)<32)
			Resizex86Ptr();
		*(u16*)x86Ptr = val; 
		x86Ptr += 2; 
	}

	////////////////////////////////////////////////////
	void write32( u32 val )
	{ 
		if ((x86Ptr_end-x86Ptr)<32)
			Resizex86Ptr();

		*(u32*)x86Ptr = val; 
		x86Ptr += 4; 
	}

	////////////////////////////////////////////////////
	void write64( u64 val )
	{ 
		if ((x86Ptr_end-x86Ptr)<32)
			Resizex86Ptr();

		*(u64*)x86Ptr = val; 
		x86Ptr += 8; 
	}

	//void Rex( u8 w, u8 r, u8 x, u8 b )
	//{
	//	write8( 0x40 | (w << 3) | (r << 2) | (x << 1) | (b) );
	//}

	void Rex( u32 w, u32 r, u32 x, u32 b )
	{
		write8((u8)( 0x40 | (w << 3) | (r << 2) | (x << 1) | (b) ));
	}

	void ModRM( u8 mod, u32 rm, u32 reg )
	{
		write8( (u8)(( mod << 6 ) | ( (rm & 7) << 3 ) | ( reg & 7 ) ));
	}

	void ModRM( u8 mod, x86IntRegType rm, x86IntRegType reg )
	{
		write8( (u8)(( mod << 6 ) | ( (rm & 7) << 3 ) | ( reg & 7 )));
	}

	void ModRM( u8 mod, x86IntRegType rm, u8 reg )
	{
		write8( (u8)(( mod << 6 ) | ( (rm & 7) << 3 ) | ( reg & 7 )));
	}

	void SibSB( u8 ss, u8 rm, u8 index )
	{
		write8( ( ss << 6 ) | ( rm << 3 ) | ( index ) );
	}

	void SET8R( u8 cc, u8 to )
	{
		write8( 0x0F );
		write8( cc );
		write8( 0xC0 | ( to ) );
	}

	u8* J8Rel( u8 cc, u8 to )
	{
		write8( cc );
		write8( to );
		return (u8*)x86Ptr - 1;
	}

	u32* J32Rel( u8 cc, u32 to )
	{
		write8( 0x0F );
		write8( cc );
		write32( to );
		return (u32*)( x86Ptr - 4 );
	}


	////////////////////////////////////////////////////
	void x86Align( int bytes ) 
	{
		// fordward align
		x86Ptr+=bytes;
		x86Ptr&=~( bytes - 1 );
	}


	u32 MEMADDR(void* ptr,u8 oplen)
	{
		s8* ptr_zero=0;
		#ifdef x64
			return	(u32)((s8*)ptr - (x86Ptr + oplen));
		#else
			return	(u32)((s8*)ptr-ptr_zero);
		#endif
	}
public :
	void CMOV32RtoR( u8 cc, u8 to, u8 from )
	{
		write8( 0x0F );
		write8( cc );
		ModRM( 3, to, from );
	}

	void CMOV32MtoR( u8 cc, u8 to, u32* from )
	{
		write8( 0x0F );
		write8( cc );
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) );
	}

	s8* x86Ptr;
	
	////////////////////////////////////////////////////
	void x86SetJ8( void* j8 )
	{
		u32 jump = ( x86Ptr - (s8*)j8 ) - 1;

		if ( jump > 0x7f )
		{
			printf( "j8 greater than 0x7f!!\n" );
		}
		*(u8*)j8 = (u8)jump;
	}


	////////////////////////////////////////////////////
	void x86SetJ32( void* j32 ) 
	{
		*j32 = ( x86Ptr - (s8*)j32 ) - 4;
	}

	void XCHG8RtoR(x86_8breg to, x86_8breg from)
	{
		if(to == from) return;
		write8( 0x86 );
		write8((u8) (0xC0 | (to << 3) | from) );
	}

	u32 UsedBytes()
	{
		return (u32)(x86Ptr-x86Ptr_base);
	}
	void* GetCode()
	{
		return x86Ptr_base;
	}
	void Pad(u8 bytes)
	{
		for (int i=0;i<bytes;i++)
			INT3();
	}
	emitter()
	{
		x86Ptr_size=DefSize;
		x86Ptr_base=x86Ptr=(s8*)EmitAlloc(x86Ptr_size);
		x86Ptr_end=x86Ptr_base+x86Ptr_size;
	}
	emitter(u8* existing)
	{
		x86Ptr_size=100000;
		x86Ptr_base=x86Ptr=(s8*)existing;
		x86Ptr_end=x86Ptr_base+x86Ptr_size;
	}
	void GetCpuInfo(CpuInfo& cpuinfo)
	{
	}
	void GenCode()
	{
		if (DefSize!=0)
			EmitAllocSet(x86e->x86Ptr_base,x86Ptr-x86Ptr_base+1);
	}

	/********************/
	/* IX86 intructions */
	/********************/

	void STC( void )
	{
		write8( 0xF9 );
	}

	void CLC( void )
	{
		write8( 0xF8 );
	}

	////////////////////////////////////
	// mov instructions                /
	////////////////////////////////////
	/* mov r64 to r64 */
	void MOV64RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		Rex(1, from >> 3, 0, to >> 3);
		write8( 0x89 );
		ModRM( 3, from, to );
	}

	/* mov r64 to m64 */
	void MOV64RtoM( u64* to, x86IntRegType from ) 
	{
		Rex(1, from >> 3, 0, 0);
		write8( 0x89 );
		ModRM( 0, from, DISP32 );
		write32( MEMADDR(to, 4) );
	}

	/* mov m64 to r64 */
	void MOV64MtoR( x86IntRegType to, u64* from ) 
	{
		Rex(1, to >> 3, 0, 0);
		write8( 0x8B );
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* mov imm32 to m64 */
	void MOV64ItoM(u32* to, u32 from ) 
	{
		Rex(1, 0, 0, 0);
		write8( 0xC7 );
		ModRM( 0, 0, DISP32 );
		write32( MEMADDR(to, 8) );
		write32( from ); 
	}

	/* mov imm32 to r64 */
	void MOV32Ito64R( x86IntRegType to, s32 from ) 
	{
		Rex(1, 0, 0, to >> 3);
		write8( 0xC7 ); 
		ModRM( 0, 0, to );
		write32( from );
	}

	/* mov imm64 to r64 */
	void MOV64ItoR( x86IntRegType to, u64 from ) {
		Rex(1, 0, 0, to >> 3);
		write8( 0xB8 | (to & 0x7) ); 
		write64( from );
	}

	/* mov [r64] to r64 */
	void MOV64RmtoR( x86IntRegType to, x86IntRegType from ) {
		Rex(1, from >> 3, 0, to >> 3);
		write8( 0x8B );
		ModRM( 0, to, from );
	}

	/* mov [r64][r64*scale] to r64 */
	void MOV64RmStoR( x86IntRegType to, x86IntRegType from, x86IntRegType from2, int scale) {
		Rex(1, from >> 3, 0, to >> 3);
		write8( 0x8B );
		ModRM( 0, to, 0x4 );
		SibSB(scale, from2, from );
	}

	/* mov r64 to [r64] */
	void MOV64RtoRm( x86IntRegType to, x86IntRegType from ) {
		Rex(1, from >> 3, 0, to >> 3);
		write8( 0x89 );
		ModRM( 0, from, to );
	}

	/* mov r64 to [r64][r64*scale] */
	void MOV64RtoRmS( x86IntRegType to, x86IntRegType from, x86IntRegType from2, int scale) {
		Rex(1, from >> 3, 0, to >> 3);
		write8( 0x89 );
		ModRM( 0, to, 0x4 );
		SibSB(scale, from2, from );
	}


	/* mov r32 to r32 */
	void MOV32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		if (to > 7 || from > 7) {
			Rex(0, from >> 3, 0, to >> 3);
		}
		write8( 0x89 );
		ModRM( 3, from, to );
	}

	/* mov r32 to m32 */
	void MOV32RtoM( u32* to, x86IntRegType from ) 
	{
		if (from > 7) {
			Rex(0, from >> 3, 0, 0);
		}
		write8( 0x89 );
		ModRM( 0, from, DISP32 );
		write32( MEMADDR(to, 4) );
	}

	/* mov m32 to r32 */
	void MOV32MtoR( x86IntRegType to, u32* from ) 
	{
		if (to > 7) {
			Rex(0, to >> 3, 0, 0);
		}
		write8( 0x8B );
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* mov [r32] to r32 */
	void MOV32RmtoR( x86IntRegType to, x86IntRegType from ) {
		if (from > 7 || to > 7) {
			Rex(0, to >> 3, 0, from >> 3);
		}
		if (from == ESP) {
			write8( 0x8B );
			ModRM( 0, to, 0x4 );
			SibSB( 0, 0x4, 0x4 );
		} else {
			write8( 0x8B );
			ModRM( 0, to, from );
		}
	}

	/* mov [r32][r32*scale] to r32 */
	void MOV32RmStoR( x86IntRegType to, x86IntRegType from, x86IntRegType from2, int scale) {
		write8( 0x8B );
		ModRM( 0, to, 0x4 );
		SibSB(scale, from2, from );
	}

	/* mov r32 to [r32] */
	void MOV32RtoRm( x86IntRegType to, x86IntRegType from ) {
		if (from > 7 || to > 7) {
			Rex(0, from >> 3, 0, to >> 3);
		}
		if (to == ESP) {
			write8( 0x89 );
			ModRM( 0, from, 0x4 );
			SibSB( 0, 0x4, 0x4 );
		} else {
			write8( 0x89 );
			ModRM( 0, from, to );
		}
	}

	/* mov r32 to [r32][r32*scale] */
	void MOV32RtoRmS( x86IntRegType to, x86IntRegType from, x86IntRegType from2, int scale) {
		write8( 0x89 );
		ModRM( 0, to, 0x4 );
		SibSB(scale, from2, from );
	}

	/* mov imm32 to r32 */
	void MOV32ItoR( x86IntRegType to, u32 from ) 
	{
		if (to > 7) {
			Rex(0, 0, 0, to >> 3);
		}
		write8( (u8)(0xB8 | (to & 0x7)) ); 
		write32( from );
	}

	/* mov imm32 to m32 */
	void MOV32ItoM(u32* to, u32 from ) 
	{
		write8( 0xC7 );
		ModRM( 0, 0, DISP32 );
		write32( MEMADDR(to, 8) );
		write32( from ); 
	}

	/* mov r16 to m16 */
	void MOV16RtoM(u16* to, x86IntRegType from ) 
	{
		write8( 0x66 );
		write8( 0x89 );
		ModRM( 0, from, DISP32 );
		write32( MEMADDR(to, 4) );
	}

	/* mov m16 to r16 */
	void MOV16MtoR( x86IntRegType to, u16* from ) 
	{
		write8( 0x66 );
		write8( 0x8B );
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* mov imm16 to m16 */
	void MOV16ItoM( u16* to, u16 from ) 
	{
		write8( 0x66 );
		write8( 0xC7 );
		ModRM( 0, 0, DISP32 );
		write32( MEMADDR(to, 6) );
		write16( from ); 
	}

	/* mov r16 to [r32][r32*scale] */
	void MOV16RtoRmS( x86IntRegType to, x86IntRegType from, x86IntRegType from2, int scale) {
		write8( 0x66 );
		write8( 0x89 );
		ModRM( 0, to, 0x4 );
		SibSB(scale, from2, from );
	}

	/* mov r8 to m8 */
	void MOV8RtoM( u8* to, x86IntRegType from ) 
	{
		write8( 0x88 );
		ModRM( 0, from, DISP32 );
		write32( MEMADDR(to, 4) );
	}

	/* mov m8 to r8 */
	void MOV8MtoR( x86IntRegType to, u8* from ) 
	{
		write8( 0x8A );
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* mov imm8 to m8 */
	void MOV8ItoM( u8* to, u8 from ) 
	{
		write8( 0xC6 );
		ModRM( 0, 0, DISP32 );
		write32( MEMADDR(to, 5) );
		write8( from ); 
	}

	/* movsx r8 to r32 */
	void MOVSX32R8toR( x86IntRegType to, x86IntRegType from ) 
	{
		write16( 0xBE0F ); 
		ModRM( 3, to, from ); 
	}

	/* movsx m8 to r32 */
	void MOVSX32M8toR( x86IntRegType to, u8* from ) 
	{
		write16( 0xBE0F ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) );
	}

	/* movsx r16 to r32 */
	void MOVSX32R16toR( x86IntRegType to, x86IntRegType from ) 
	{
		write16( 0xBF0F ); 
		ModRM( 3, to, from ); 
	}

	/* movsx m16 to r32 */
	void MOVSX32M16toR( x86IntRegType to, u16* from ) 
	{
		write16( 0xBF0F ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) );
	}

	/* movzx r8 to r32 */
	void MOVZX32R8toR( x86IntRegType to, x86IntRegType from ) 
	{
		write16( 0xB60F ); 
		ModRM( 3, to, from ); 
	}

	/* movzx m8 to r32 */
	void MOVZX32M8toR( x86IntRegType to, u8* from ) 
	{
		write16( 0xB60F ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) );
	}

	/* movzx r16 to r32 */
	void MOVZX32R16toR( x86IntRegType to, x86IntRegType from ) 
	{
		write16( 0xB70F ); 
		ModRM( 3, to, from ); 
	}

	/* movzx m16 to r32 */
	void MOVZX32M16toR( x86IntRegType to, u16* from ) 
	{
		write16( 0xB70F ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) );
	}

	/* cmovbe r32 to r32 */
	void CMOVBE32RtoR( x86IntRegType to, x86IntRegType from )
	{
		CMOV32RtoR( 0x46, to, from );
	}

	/* cmovbe m32 to r32*/
	void CMOVBE32MtoR( x86IntRegType to, u32 from )
	{
		CMOV32MtoR( 0x46, to, from );
	}

	/* cmovb r32 to r32 */
	void CMOVB32RtoR( x86IntRegType to, x86IntRegType from )
	{
		CMOV32RtoR( 0x42, to, from );
	}

	/* cmovb m32 to r32*/
	void CMOVB32MtoR( x86IntRegType to, u32 from )
	{
		CMOV32MtoR( 0x42, to, from );
	}

	/* cmovae r32 to r32 */
	void CMOVAE32RtoR( x86IntRegType to, x86IntRegType from )
	{
		CMOV32RtoR( 0x43, to, from );
	}

	/* cmovae m32 to r32*/
	void CMOVAE32MtoR( x86IntRegType to, u32 from )
	{
		CMOV32MtoR( 0x43, to, from );
	}

	/* cmova r32 to r32 */
	void CMOVA32RtoR( x86IntRegType to, x86IntRegType from )
	{
		CMOV32RtoR( 0x47, to, from );
	}

	/* cmova m32 to r32*/
	void CMOVA32MtoR( x86IntRegType to, u32 from )
	{
		CMOV32MtoR( 0x47, to, from );
	}

	/* cmovo r32 to r32 */
	void CMOVO32RtoR( x86IntRegType to, x86IntRegType from )
	{
		CMOV32RtoR( 0x40, to, from );
	}

	/* cmovo m32 to r32 */
	void CMOVO32MtoR( x86IntRegType to, u32 from )
	{
		CMOV32MtoR( 0x40, to, from );
	}

	/* cmovp r32 to r32 */
	void CMOVP32RtoR( x86IntRegType to, x86IntRegType from )
	{
		CMOV32RtoR( 0x4A, to, from );
	}

	/* cmovp m32 to r32 */
	void CMOVP32MtoR( x86IntRegType to, u32 from )
	{
		CMOV32MtoR( 0x4A, to, from );
	}

	/* cmovs r32 to r32 */
	void CMOVS32RtoR( x86IntRegType to, x86IntRegType from )
	{
		CMOV32RtoR( 0x48, to, from );
	}

	/* cmovs m32 to r32 */
	void CMOVS32MtoR( x86IntRegType to, u32 from )
	{
		CMOV32MtoR( 0x48, to, from );
	}

	/* cmovno r32 to r32 */
	void CMOVNO32RtoR( x86IntRegType to, x86IntRegType from )
	{
		CMOV32RtoR( 0x41, to, from );
	}

	/* cmovno m32 to r32 */
	void CMOVNO32MtoR( x86IntRegType to, u32 from )
	{
		CMOV32MtoR( 0x41, to, from );
	}

	/* cmovnp r32 to r32 */
	void CMOVNP32RtoR( x86IntRegType to, x86IntRegType from )
	{
		CMOV32RtoR( 0x4B, to, from );
	}

	/* cmovnp m32 to r32 */
	void CMOVNP32MtoR( x86IntRegType to, u32 from )
	{
		CMOV32MtoR( 0x4B, to, from );
	}

	/* cmovns r32 to r32 */
	void CMOVNS32RtoR( x86IntRegType to, x86IntRegType from )
	{
		CMOV32RtoR( 0x49, to, from );
	}

	/* cmovns m32 to r32 */
	void CMOVNS32MtoR( x86IntRegType to, u32 from )
	{
		CMOV32MtoR( 0x49, to, from );
	}

	/* cmovne r32 to r32 */
	void CMOVNE32RtoR( x86IntRegType to, x86IntRegType from )
	{
		CMOV32RtoR( 0x45, to, from );
	}

	/* cmovne m32 to r32*/
	void CMOVNE32MtoR( x86IntRegType to, u32* from ) 
	{
		CMOV32MtoR( 0x45, to, from );
	}

	/* cmove r32 to r32*/
	void CMOVE32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		CMOV32RtoR( 0x44, to, from );
	}

	/* cmove m32 to r32*/
	void CMOVE32MtoR( x86IntRegType to, u32 from ) 
	{
		CMOV32MtoR( 0x44, to, from );
	}

	/* cmovg r32 to r32*/
	void CMOVG32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		CMOV32RtoR( 0x4F, to, from );
	}

	/* cmovg m32 to r32*/
	void CMOVG32MtoR( x86IntRegType to, u32 from ) 
	{
		CMOV32MtoR( 0x4F, to, from );
	}

	/* cmovge r32 to r32*/
	void CMOVGE32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		CMOV32RtoR( 0x4D, to, from );
	}

	/* cmovge m32 to r32*/
	void CMOVGE32MtoR( x86IntRegType to, u32 from ) 
	{
		CMOV32MtoR( 0x4D, to, from );
	}

	/* cmovl r32 to r32*/
	void CMOVL32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		CMOV32RtoR( 0x4C, to, from );
	}

	/* cmovl m32 to r32*/
	void CMOVL32MtoR( x86IntRegType to, u32 from ) 
	{
		CMOV32MtoR( 0x4C, to, from );
	}

	/* cmovle r32 to r32*/
	void CMOVLE32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		CMOV32RtoR( 0x4E, to, from );
	}

	/* cmovle m32 to r32*/
	void CMOVLE32MtoR( x86IntRegType to, u32 from ) 
	{
		CMOV32MtoR( 0x4E, to, from );
	}

	////////////////////////////////////
	// arithmetic instructions         /
	////////////////////////////////////
	/* add imm32 to r64 */
	void ADD64ItoR( x86IntRegType to, u32 from ) 
	{
		Rex(1, 0, 0, to >> 3);
		if ( to == EAX) {
			write8( 0x05 ); 
		} else {
			write8( 0x81 ); 
			ModRM( 3, 0, to );
		}
		write32( from );
	}

	/* add m64 to r64 */
	void ADD64MtoR( x86IntRegType to, u32 from ) 
	{
		Rex(1, to >> 3, 0, 0);
		write8( 0x03 ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) );
	}

	/* add r64 to r64 */
	void ADD64RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		Rex(1, from >> 3, 0, to >> 3);
		write8( 0x01 ); 
		ModRM( 3, from, to );
	}

	/* add imm32 to r32 */
	void ADD32ItoR( x86IntRegType to, u32 from ) 
	{
		if (to > 7) {
			Rex(0, to >> 3, 0, 0);
		}
		if ( to == EAX) 
		{
			write8( 0x05 ); 
		}
		else 
		{
			write8( 0x81 ); 
			ModRM( 3, 0, to );
		}
		write32( from );
	}

	/* add imm32 to m32 */
	void ADD32ItoM( u32* to, u32 from ) 
	{
		write8( 0x81 ); 
		ModRM( 0, 0, DISP32 );
		write32( MEMADDR(to, 8) );
		write32( from );
	}

	/* add r32 to r32 */
	void ADD32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		if (to > 7 || from > 7) {
			Rex(0, from >> 3, 0, to >> 3);
		}
		write8( 0x01 ); 
		ModRM( 3, from, to );
	}

	/* add r32 to m32 */
	void ADD32RtoM(u32* to, x86IntRegType from ) 
	{
		write8( 0x01 ); 
		ModRM( 0, from, DISP32 );
		write32( MEMADDR(to, 4) );
	}

	/* add m32 to r32 */
	void ADD32MtoR( x86IntRegType to, u32* from ) 
	{
		write8( 0x03 ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) );
	}

	/* add imm16 to r16 */
	void ADD16ItoR( x86IntRegType to, u16 from ) 
	{
		write8( 0x66 );
		if ( to == EAX) 
		{
			write8( 0x05 ); 
		}
		else 
		{
			write8( 0x81 ); 
			ModRM( 3, 0, to );
		}
		write16( from );
	}

	/* add imm16 to m16 */
	void ADD16ItoM( x86IntRegType to, u16 from ) 
	{
		write8( 0x66 );
		write8( 0x81 ); 
		ModRM( 0, 0, DISP32 );
		write32( MEMADDR(to, 6) );
		write16( from );
	}

	/* add r16 to m16 */
	void ADD16RtoM(u32 to, x86IntRegType from ) 
	{
		write8( 0x66 );
		write8( 0x01 ); 
		ModRM( 0, from, DISP32 );
		write32( MEMADDR(to, 4) );
	}

	/* add m16 to r16 */
	void ADD16MtoR( x86IntRegType to, u32 from ) 
	{
		write8( 0x66 );
		write8( 0x03 ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) );
	}

	/* adc imm32 to r32 */
	void ADC32ItoR( x86IntRegType to, u32 from ) 
	{
		if ( to == EAX )
		{
			write8( 0x15 );
		}
		else 
		{
			write8( 0x81 );
			ModRM( 3, 2, to );
		}
		write32( from ); 
	}

	/* adc imm32 to m32 */
	void ADC32ItoM( u32* to, u32 from ) 
	{
		write8( 0x81 ); 
		ModRM( 0, 2, DISP32 );
		write32( MEMADDR(to, 8) );
		write32( from );
	}

	/* adc r32 to r32 */
	void ADC32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		write8( 0x11 ); 
		ModRM( 3, from, to );
	}

	/* adc m32 to r32 */
	void ADC32MtoR( x86IntRegType to, u32* from ) 
	{
		write8( 0x13 ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* inc r32 */
	void INC32R( x86IntRegType to ) 
	{
		write8( 0x40 + to );
	}

	/* inc m32 */
	void INC32M( u32 to ) 
	{
		write8( 0xFF );
		ModRM( 0, 0, DISP32 );
		write32( MEMADDR(to, 4) );
	}

	/* inc r16 */
	void INC16R( x86IntRegType to ) 
	{
		write8( 0x66 );
		write8( 0x40 + to );
	}

	/* inc m16 */
	void INC16M( u32 to ) 
	{
		write8( 0x66 );
		write8( 0xFF );
		ModRM( 0, 0, DISP32 );
		write32( MEMADDR(to, 4) );
	}


	/* sub imm32 to r64 */
	void SUB64ItoR( x86IntRegType to, u32 from ) 
	{
		Rex(1, 0, 0, to >> 3);
		if ( to == EAX )
		{
			write8( 0x2D ); 
		} 
		else 
		{
			write8( 0x81 ); 
			ModRM( 3, 5, to );
		}
		write32( from ); 
	}

	/* sub r64 to r64 */
	void SUB64RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		Rex(1, from >> 3, 0, to >> 3);
		write8( 0x29 ); 
		ModRM( 3, from, to );
	}

	/* sub m64 to r64 */
	void SUB64MtoR( x86IntRegType to, u32 from ) 
	{
		Rex(1, to >> 3, 0, 0);
		write8( 0x2B ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* sub imm32 to r32 */
	void SUB32ItoR( x86IntRegType to, u32 from ) 
	{
		if ( to == EAX )
		{
			write8( 0x2D ); 
		} 
		else 
		{
			write8( 0x81 ); 
			ModRM( 3, 5, to );
		}
		write32( from ); 
	}

	/* sub imm32 to m32 */
	void SUB32ItoM( u32* to, u32 from ) 
	{
		write8( 0x81 ); 
		ModRM( 0, 5, DISP32 );
		write32( MEMADDR(to, 8) );
		write32( from );
	}

	/* sub r32 to r32 */
	void SUB32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		if (to > 7 || from > 7) {
			Rex(0, from >> 3, 0, to >> 3);
		}
		write8( 0x29 ); 
		ModRM( 3, from, to );
	}

	/* sub m32 to r32 */
	void SUB32MtoR( x86IntRegType to, u32* from ) 
	{
		write8( 0x2B ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* sub imm16 to r16 */
	void SUB16ItoR( x86IntRegType to, u16 from ) {
		if ( to == EAX ) {
			write8( 0x2D ); 
		} else {
			write8( 0x81 ); 
			ModRM( 3, 5, to );
		}
		write16( from ); 
	}

	/* sub imm16 to m16 */
	void SUB16ItoM( u32 to, u16 from ) {
		write8( 0x66 ); 
		write8( 0x81 ); 
		ModRM( 0, 5, DISP32 );
		write32( MEMADDR(to, 6) );
		write16( from );
	}

	/* sub m16 to r16 */
	void SUB16MtoR( x86IntRegType to, u32 from ) {
		write8( 0x66 ); 
		write8( 0x2B ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* sbb r64 to r64 */
	void SBB64RtoR( x86IntRegType to, x86IntRegType from ) {
		Rex(1, to >> 3, 0, from >> 3);
		write8( 0x19 ); 
		ModRM( 3, from, to );
	}

	/* sbb imm32 to r32 */
	void SBB32ItoR( x86IntRegType to, u32 from ) {
		if ( to == EAX ) {
			write8( 0x1D );
		} else {
			write8( 0x81 );
			ModRM( 3, 3, to );
		}
		write32( from ); 
	}

	/* sbb imm32 to m32 */
	void SBB32ItoM( u32 to, u32 from ) {
		write8( 0x81 );
		ModRM( 0, 3, DISP32 );
		write32( MEMADDR(to, 8) );
		write32( from );
	}

	/* sbb r32 to r32 */
	void SBB32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		write8( 0x19 ); 
		ModRM( 3, from, to );
	}

	/* sbb m32 to r32 */
	void SBB32MtoR( x86IntRegType to, u32 from ) 
	{
		write8( 0x1B ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* dec r32 */
	void DEC32R( x86IntRegType to ) 
	{
		write8( 0x48 + to );
	}

	/* dec m32 */
	void DEC32M( u32 to ) 
	{
		write8( 0xFF );
		ModRM( 0, 1, DISP32 );
		write32( MEMADDR(to, 4) );
	}

	/* dec r16 */
	void DEC16R( x86IntRegType to ) 
	{
		write8( 0x66 );
		write8( 0x48 + to );
	}

	/* dec m16 */
	void DEC16M( u32 to ) 
	{
		write8( 0x66 );
		write8( 0xFF );
		ModRM( 0, 1, DISP32 );
		write32( MEMADDR(to, 4) );
	}

	/* mul eax by r32 to edx:eax */
	void MUL32R( x86IntRegType from ) 
	{
		write8( 0xF7 ); 
		ModRM( 3, 4, from );
	}

	/* imul eax by r32 to edx:eax */
	void IMUL32R( x86IntRegType from ) 
	{
		write8( 0xF7 ); 
		ModRM( 3, 5, from );
	}

	/* mul eax by m32 to edx:eax */
	void MUL32M( u32* from ) 
	{
		write8( 0xF7 ); 
		ModRM( 0, 4, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* imul eax by m32 to edx:eax */
	void IMUL32M( u32* from ) 
	{
		write8( 0xF7 ); 
		ModRM( 0, 5, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* imul r32 by r32 to r32 */
	void IMUL32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		write16( 0xAF0F ); 
		ModRM( 3, to, from );
	}

	/* div eax by r32 to edx:eax */
	void DIV32R( x86IntRegType from ) 
	{
		write8( 0xF7 ); 
		ModRM( 3, 6, from );
	}

	/* idiv eax by r32 to edx:eax */
	void IDIV32R( x86IntRegType from ) 
	{
		write8( 0xF7 ); 
		ModRM( 3, 7, from );
	}

	/* div eax by m32 to edx:eax */
	void DIV32M( u32* from ) 
	{
		write8( 0xF7 ); 
		ModRM( 0, 6, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* idiv eax by m32 to edx:eax */
	void IDIV32M( s32* from ) 
	{
		write8( 0xF7 ); 
		ModRM( 0, 7, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	////////////////////////////////////
	// shifting instructions           /
	////////////////////////////////////

	/* shl imm8 to r64 */
	void SHL64ItoR( x86IntRegType to, u8 from ) 
	{
		if ( from == 1 )
		{
			Rex(1, 0, 0, to >> 3);
			write8( 0xD1 );
			ModRM( 3, 4, to );
			return;
		}
		Rex(1, 0, 0, to >> 3);
		write8( 0xC1 ); 
		ModRM( 3, 4, to );
		write8( from ); 
	}

	/* shl cl to r64 */
	void SHL64CLtoR( x86IntRegType to ) 
	{
		Rex(1, to >> 3, 0, 0);
		write8( 0xD3 ); 
		ModRM( 3, 4, to );
	}

	/* shr imm8 to r64 */
	void SHR64ItoR( x86IntRegType to, u8 from ) 
	{
		if ( from == 1 )
		{
			Rex(1, 0, 0, to >> 3);
			write8( 0xD1 );
			ModRM( 3, 5, to );
			return;
		}
		Rex(1, 0, 0, to >> 3);
		write8( 0xC1 ); 
		ModRM( 3, 5, to );
		write8( from ); 
	}

	/* shr cl to r64 */
	void SHR64CLtoR( x86IntRegType to ) 
	{
		Rex(1, to >> 3, 0, 0);
		write8( 0xD3 ); 
		ModRM( 3, 5, to );
	}

	/* shl imm8 to r32 */
	void SHL32ItoR( x86IntRegType to, u8 from ) 
	{
		if (to > 7) {
			Rex(0, 0, 0, to >> 3);
		}
		if ( from == 1 )
		{
			write8( 0xD1 );
			write8( (u8)(0xE0 | (to & 0x7)) );
			return;
		}
		write8( 0xC1 ); 
		ModRM( 3, 4, to );
		write8( from ); 
	}

	/* shl imm8 to m32 */
	void SHL32ItoM( u32* to, u8 from ) 
	{
		if ( from == 1 )
		{
			write8( 0xD1 );
			ModRM( 0, 4, DISP32 );
			write32( MEMADDR(to, 4) );
		}
		else
		{
			write8( 0xC1 ); 
			ModRM( 0, 4, DISP32 );
			write32( MEMADDR(to, 4) );
			write8( from ); 
		}
	}

	/* shl cl to r32 */
	void SHL32CLtoR( x86IntRegType to ) 
	{
		if (to > 7) {
			Rex(0, 0, 0, to >> 3);
		}
		write8( 0xD3 ); 
		ModRM( 3, 4, to );
	}

	/* shr imm8 to r32 */
	void SHR32ItoR( x86IntRegType to, u8 from ) {
		if (to > 7) {
			Rex(0, 0, 0, to >> 3);
		}
		if ( from == 1 )
		{
			write8( 0xD1 );
			write8( (u8)(0xE8 | (to & 0x7)) );
		}
		else
		{
			write8( 0xC1 ); 
			ModRM( 3, 5, to );
			write8( from ); 
		}
	}

	/* shr imm8 to m32 */
	void SHR32ItoM( u32* to, u8 from ) 
	{
		if ( from == 1 )
		{
			write8( 0xD1 );
			ModRM( 0, 5, DISP32 );
			write32( MEMADDR(to, 4) );
		}
		else
		{
			write8( 0xC1 ); 
			ModRM( 0, 5, DISP32 );
			write32( MEMADDR(to, 4) );
			write8( from ); 
		}
	}

	/* shr cl to r32 */
	void SHR32CLtoR( x86IntRegType to ) 
	{
		if (to > 7) {
			Rex(0, 0, 0, to >> 3);
		}
		write8( 0xD3 ); 
		ModRM( 3, 5, to );
	}


	/* sar imm8 to r64 */
	void SAR64ItoR( x86IntRegType to, u8 from ) 
	{
		if ( from == 1 )
		{
			Rex(1, 0, 0, to >> 3);
			write8( 0xD1 );
			ModRM( 3, 7, to );
			return;
		}
		Rex(1, 0, 0, to >> 3);
		write8( 0xC1 ); 
		ModRM( 3, 7, to );
		write8( from ); 
	}

	/* sar cl to r64 */
	void SAR64CLtoR( x86IntRegType to ) 
	{
		Rex(1, to >> 3, 0, 0);
		write8( 0xD3 ); 
		ModRM( 3, 7, to );
	}

	/* sar imm8 to r32 */
	void SAR32ItoR( x86IntRegType to, u8 from ) 
	{
		if (to > 7) {
			Rex(0, 0, 0, to >> 3);
		}
		if ( from == 1 )
		{
			write8( 0xD1 );
			ModRM( 3, 7, to );
			return;
		}
		write8( 0xC1 ); 
		ModRM( 3, 7, to );
		write8( from ); 
	}

	/* sar imm8 to m32 */
	void SAR32ItoM( u32* to, u8 from )
	{
		write8( 0xC1 ); 
		ModRM( 0, 7, DISP32 );
		write32( MEMADDR(to, 5) );
		write8( from );
	}

	/* sar cl to r32 */
	void SAR32CLtoR( x86IntRegType to ) 
	{
		if (to > 7) {
			Rex(0, 0, 0, to >> 3);
		}
		write8( 0xD3 ); 
		ModRM( 3, 7, to );
	}

	void RCR32ItoR( x86IntRegType to, u8 from ) 
	{
		if ( from == 1 ) {
			write8( 0xd1 );
			write8( 0xd8 | to );
		} 
		else 
		{
			write8( 0xc1 );
			write8( 0xd8 | to );
			write8( from );
		}
	}
	void RCR321toR( x86IntRegType to ) 
	{
		RCR32ItoR(to,1);
	}

	void ROR321toR( x86IntRegType to ) 
	{
		write8( 0xd1 );
		write8( 0xc8 | to );
	}

	void ROR32ItoR( x86IntRegType to ,u8 from) 
	{
		write8( 0xc1 );
		write8( 0xc8 | to );
		write8( from );
	}

	void ROL321toR( x86IntRegType to ) 
	{
		write8( 0xd1 );
		write8( 0xC0 | to );
	}

	void RCL32ItoR( x86IntRegType to, u8 from ) 
	{
		if ( from == 1 ) {
			write8( 0xd1 );
			write8( 0xd0 | to );
		} 
		else 
		{
			write8( 0xc1 );
			write8( 0xd0 | to );
			write8( from );
		}
	}

	void RCL321toM(u32*to) 
	{
		//if (count!=1)
		//	printf("RCL32ItoM count!=1 die kthx\n");

		write8(0xD1);
		write8(0x15);
		write32((u32)to);		
	}
	void RCL321toR(x86IntRegType to) 
	{
		//if (count!=1)
		//	printf("RCL32ItoM count!=1 die kthx\n");

		write8(0xD1);
		write8(0xD0|to);
		//write32((u32)to);		
	}

	// shld imm8 to r32
	void SHLD32ItoR( u32 to, u32 from, u8 shift )
	{
		write8( 0x0F );
		write8( 0xA4 );
		ModRM( 3, to, from );
		write8( shift );
	}

	// shrd imm8 to r32
	void SHRD32ItoR( u32 to, u32 from, u8 shift )
	{
		write8( 0x0F );
		write8( 0xAC );
		ModRM( 3, to, from );
		write8( shift );
	}

	////////////////////////////////////
	// logical instructions            /
	////////////////////////////////////

	/* or imm32 to r32 */
	void OR64ItoR( x86IntRegType to, u32 from ) 
	{
		Rex(1, 0, 0, to >> 3);
		if ( to == EAX ) {
			write8( 0x0D ); 
		} else {
			write8( 0x81 ); 
			ModRM( 3, 1, to );
		}
		write32( from ); 
	}

	/* or m64 to r64 */
	void OR64MtoR( x86IntRegType to, u32 from ) 
	{
		Rex(1, to >> 3, 0, 0);
		write8( 0x0B ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* or r64 to r64 */
	void OR64RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		Rex(1, from >> 3, 0, to >> 3);
		write8( 0x09 ); 
		ModRM( 3, from, to );
	}


	/* or imm32 to r32 */
	void OR32ItoR( x86IntRegType to, u32 from ) 
	{
		if ( to == EAX )
		{
			write8( 0x0D ); 
		} 
		else 
		{
			write8( 0x81 ); 
			ModRM( 3, 1, to );
		}
		write32( from ); 
	}

	/* or imm32 to m32 */
	void OR32ItoM(u32* to, u32 from ) 
	{
		write8( 0x81 ); 
		ModRM( 0, 1, DISP32 );
		write32( MEMADDR(to, 8) );
		write32( from ); 
	}

	/* or r32 to r32 */
	void OR32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		write8( 0x09 ); 
		ModRM( 3, from, to );
	}

	/* or r32 to m32 */
	void OR32RtoM(u32* to, x86IntRegType from ) 
	{
		write8( 0x09 ); 
		ModRM( 0, from, DISP32 );
		write32( MEMADDR(to, 4) );
	}

	/* or m32 to r32 */
	void OR32MtoR( x86IntRegType to, u32* from ) 
	{
		write8( 0x0B ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* or m16 to r16 */
	void OR16MtoR( x86IntRegType to, u32 from ) 
	{
		write8( 0x66 );
		write8( 0x0B ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* xor imm32 to r64 */
	void XOR64ItoR( x86IntRegType to, u32 from ) 
	{
		Rex(1, 0, 0, to >> 3);
		if ( to == EAX ) {
			write8( 0x35 ); 
		} else {
			write8( 0x81 ); 
			ModRM( 3, 6, to );
		}
		write32( from ); 
	}

	/* xor r64 to r64 */
	void XOR64RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		Rex(1, from >> 3, 0, to >> 3);
		write8( 0x31 ); 
		ModRM( 3, from, to );
	}

	/* xor m64 to r64 */
	void XOR64MtoR( x86IntRegType to, u32 from ) 
	{
		Rex(1, to >> 3, 0, 0);
		write8( 0x33 ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* xor imm32 to r32 */
	void XOR32ItoR( x86IntRegType to, u32 from ) 
	{
		if ( to == EAX )
		{
			write8( 0x35 ); 
		} 
		else 
		{
			write8( 0x81 ); 
			ModRM( 3, 6, to );
		}
		write32( from ); 
	}

	/* xor imm32 to m32 */
	void XOR32ItoM( u32* to, u32 from ) 
	{
		write8( 0x81 ); 
		ModRM( 0, 6, DISP32 );
		write32( MEMADDR(to, 8) ); 
		write32( from ); 
	}

	/* xor r32 to r32 */
	void XOR32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		write8( 0x31 ); 
		ModRM( 3, from, to );
	}

	/* xor r16 to r16 */
	void XOR16RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		write8( 0x66 );
		write8( 0x31 ); 
		ModRM( 3, from, to );
	}

	/* xor r32 to m32 */
	void XOR32RtoM( u32* to, x86IntRegType from ) 
	{
		write8( 0x31 ); 
		ModRM( 0, from, DISP32 );
		write32( MEMADDR(to, 4) );
	}

	/* xor m32 to r32 */
	void XOR32MtoR( x86IntRegType to, u32* from ) 
	{
		write8( 0x33 ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* and imm32 to r64 */
	void AND64ItoR( x86IntRegType to, u32 from ) 
	{
		Rex(1, 0, 0, to >> 3);
		if ( to == EAX ) {
			write8( 0x25 ); 
		} else {
			write8( 0x81 ); 
			ModRM( 3, 0x4, to );
		}
		write32( from ); 
	}

	/* and m64 to r64 */
	void AND64MtoR( x86IntRegType to, u32 from ) 
	{
		Rex(1, to >> 3, 0, 0);
		write8( 0x23 ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* and r64 to m64 */
	void AND64RtoM( u32 to, x86IntRegType from ) 
	{
		Rex(1, 0, 0, 0);
		write8( 0x21 ); 
		ModRM( 0, from, DISP32 );
		write32( MEMADDR(to, 4) ); 
	}

	/* and r64 to r64 */
	void AND64RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		Rex(1, from >> 3, 0, to >> 3);
		write8( 0x21 ); 
		ModRM( 3, from, to );
	}

	/* and imm32 to r32 */
	void AND32ItoR( x86IntRegType to, u32 from ) 
	{
		if (to > 7) {
			Rex(0, 0, 0, to >> 3);
		}
		if ( to == EAX ) {
			write8( 0x25 ); 
		} else {
			write8( 0x81 ); 
			ModRM( 3, 0x4, to );
		}
		write32( from ); 
	}

	/* and imm32 to m32 */
	void AND32ItoM( u32* to, u32 from ) 
	{
		write8( 0x81 ); 
		ModRM( 0, 0x4, DISP32 );
		write32( MEMADDR(to, 8) );
		write32( from ); 
	}

	/* and r32 to r32 */
	void AND32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		write8( 0x21 ); 
		ModRM( 3, from, to );
	}

	/* and r32 to m32 */
	void AND32RtoM( u32* to, x86IntRegType from ) 
	{
		write8( 0x21 ); 
		ModRM( 0, from, DISP32 );
		write32( MEMADDR(to, 4) ); 
	}

	/* and m32 to r32 */
	void AND32MtoR( x86IntRegType to, u32* from ) 
	{
		write8( 0x23 ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* and r16 to m16 */
	void AND16RtoM( u32 to, x86IntRegType from ) 
	{
		write8( 0x66 );
		write8( 0x21 ); 
		ModRM( 0, from, DISP32 );
		write32( MEMADDR(to, 4) ); 
	}

	/* and m16 to r16 */
	void AND16MtoR( x86IntRegType to, u32 from ) 
	{
		write8( 0x66 );
		write8( 0x23 ); 
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4)); 
	}

	/* not r64 */
	void NOT64R( x86IntRegType from ) 
	{
		Rex(1, 0, 0, from >> 3);
		write8( 0xF7 ); 
		ModRM( 3, 2, from );
	}

	/* not r32 */
	void NOT32R( x86IntRegType from ) 
	{
		write8( 0xF7 ); 
		ModRM( 3, 2, from );
	}

	/* neg r64 */
	void NEG64R( x86IntRegType from ) 
	{
		Rex(1, from >> 3, 0, 0);
		write8( 0xF7 ); 
		ModRM( 3, 3, from );
	}

	/* neg r32 */
	void NEG32R( x86IntRegType from ) 
	{
		write8( 0xF7 ); 
		ModRM( 3, 3, from );
	}

	/* neg r16 */
	void NEG16R( x86IntRegType from ) 
	{
		write8( 0x66 ); 
		write8( 0xF7 ); 
		ModRM( 3, 3, from );
	}

	////////////////////////////////////
	// jump instructions               /
	////////////////////////////////////
	u8* JMP( void* to ) {
		u32 jump = ( x86Ptr - (s8*)to ) - 1;

		if ( jump > 0x7f ) {
			return (u8*)JMP32( (u8*)to - ( (u8*)x86Ptr + 5 )  );
		} else {
			return (u8*)JMP8( (u8*)to - ( (u8*)x86Ptr + 2 )  );
		}
	}

	/* jmp rel8 */
	u8* JMP8( u8 to ) 
	{
		write8( 0xEB ); 
		write8( to );
		return (u8*)x86Ptr - 1;
	}

	/* jmp rel32 */
	u32* JMP32( u32 to ) 
	{
		write8( 0xE9 ); 
		write32( to ); 
		return (u32*)(x86Ptr - 4 );
	}

	/* jmp r32 */
	void JMP32R( x86IntRegType to ) 
	{
		write8( 0xFF ); 
		ModRM( 3, 4, to );
	}

	/* jmp r64 */
	void JMP64R( x86IntRegType to ) {
		Rex(1, 0, 0, to >> 3);
		write8( 0xFF );
		ModRM( 3, 4, to );
	}

	/* jp rel8 */
	u8* JP8( u8 to ) {
		return J8Rel( 0x7A, to );
	}

	/* jnp rel8 */
	u8* JNP8( u8 to ) {
		return J8Rel( 0x7B, to );
	}

	/* je rel8 */
	u8* JE8( u8 to ) {
		return J8Rel( 0x74, to );
	}

	/* jz rel8 */
	u8* JZ8( u8 to ) 
	{
		return J8Rel( 0x74, to ); 
	}

	/* js rel8 */
	u8* JS8( u8 to ) 
	{ 
		return J8Rel( 0x78, to );
	}

	/* jns rel8 */
	u8* JNS8( u8 to ) 
	{ 
		return J8Rel( 0x79, to );
	}

	/* jg rel8 */
	u8* JG8( u8 to ) 
	{ 
		return J8Rel( 0x7F, to );
	}

	/* jge rel8 */
	u8* JGE8( u8 to ) 
	{ 
		return J8Rel( 0x7D, to ); 
	}

	/* jl rel8 */
	u8* JL8( u8 to ) 
	{ 
		return J8Rel( 0x7C, to ); 
	}

	/* ja rel8 */
	u8* JA8( u8 to ) 
	{ 
		return J8Rel( 0x77, to ); 
	}

	u8* JAE8( u8 to ) 
	{ 
		return J8Rel( 0x73, to ); 
	}

	/* jb rel8 */
	u8* JB8( u8 to ) 
	{ 
		return J8Rel( 0x72, to ); 
	}

	/* jbe rel8 */
	u8* JBE8( u8 to ) 
	{ 
		return J8Rel( 0x76, to ); 
	}

	/* jle rel8 */
	u8* JLE8( u8 to ) 
	{ 
		return J8Rel( 0x7E, to ); 
	}

	/* jne rel8 */
	u8* JNE8( u8 to ) 
	{ 
		return J8Rel( 0x75, to ); 
	}

	/* jnz rel8 */
	u8* JNZ8( u8 to ) 
	{ 
		return J8Rel( 0x75, to ); 
	}

	/* jng rel8 */
	u8* JNG8( u8 to ) 
	{ 
		return J8Rel( 0x7E, to ); 
	}

	/* jnge rel8 */
	u8* JNGE8( u8 to ) 
	{ 
		return J8Rel( 0x7C, to ); 
	}

	/* jnl rel8 */
	u8* JNL8( u8 to ) 
	{ 
		return J8Rel( 0x7D, to ); 
	}

	/* jnle rel8 */
	u8* JNLE8( u8 to ) 
	{ 
		return J8Rel( 0x7F, to ); 
	}

	/* jo rel8 */
	u8* JO8( u8 to ) 
	{ 
		return J8Rel( 0x70, to ); 
	}

	/* jno rel8 */
	u8* JNO8( u8 to ) 
	{ 
		return J8Rel( 0x71, to ); 
	}

	/* je rel32 */
	u32* JE32( u32 to ) 
	{
		return J32Rel( 0x84, to );
	}

	/* jz rel32 */
	u32* JZ32( u32 to ) 
	{
		return J32Rel( 0x84, to ); 
	}

	/* jg rel32 */
	u32* JG32( u32 to ) 
	{ 
		return J32Rel( 0x8F, to );
	}

	/* jge rel32 */
	u32* JGE32( u32 to ) 
	{ 
		return J32Rel( 0x8D, to ); 
	}

	/* jl rel32 */
	u32* JL32( u32 to ) 
	{ 
		return J32Rel( 0x8C, to ); 
	}

	/* jle rel32 */
	u32* JLE32( u32 to ) 
	{ 
		return J32Rel( 0x8E, to ); 
	}

	/* jne rel32 */
	u32* JNE32( void* to ) 
	{ 
		return J32Rel( 0x85,  (u8*)to - ( (u8*)x86Ptr + 6 )  ); 
	}

	//void CALLFunc( void* func ) 
	//{
	//	CALL32( (u8*)func - ( (u8*)x86Ptr + 5 ) );
	//}


	/* jnz rel32 */
	u32* JNZ32( u32 to ) 
	{ 
		return J32Rel( 0x85, to ); 
	}

	/* jng rel32 */
	u32* JNG32( u32 to ) 
	{ 
		return J32Rel( 0x8E, to ); 
	}

	/* jnge rel32 */
	u32* JNGE32( u32 to ) 
	{ 
		return J32Rel( 0x8C, to ); 
	}

	/* jnl rel32 */
	u32* JNL32( u32 to ) 
	{ 
		return J32Rel( 0x8D, to ); 
	}

	/* jnle rel32 */
	u32* JNLE32( u32 to ) 
	{ 
		return J32Rel( 0x8F, to ); 
	}

	/* jo rel32 */
	u32* JO32( u32 to ) 
	{ 
		return J32Rel( 0x80, to ); 
	}

	/* jno rel32 */
	u32* JNO32( u32 to ) 
	{ 
		return J32Rel( 0x81, to ); 
	}

	/* call func */
	void CALLFunc( void* func ) 
	{
		CALL32( (u8*)func - ( (u8*)x86Ptr + 5 ) );
	}

	/* call rel32 */
	void CALL32( u32 to )
	{
		write8( 0xE8 ); 
		write32( to ); 
	}

	/* call r32 */
	void CALL32R( x86IntRegType to ) 
	{
		write8( 0xFF );
		ModRM( 3, 2, to );
	}

	/* call r64 */
	void CALL64R( x86IntRegType to ) 
	{
		Rex(1, 0, 0, to >> 3);
		write8( 0xFF );
		ModRM( 3, 2, to );
	}

	/* call m32 */
	void CALL32M( u32 to ) 
	{
		write8( 0xFF );
		ModRM( 0, 2, DISP32 );
		write32( MEMADDR(to, 4) );
	}

	////////////////////////////////////
	// misc instructions               /
	////////////////////////////////////

	void INT3( ) 
	{
		write8( 0xCC );
	}

	/* cmp imm32 to r64 */
	void CMP64ItoR( x86IntRegType to, u32 from ) 
	{
		Rex(1, to >> 3, 0, 0);
		if ( to == EAX )
		{
			write8( 0x3D );
		} 
		else 
		{
			write8( 0x81 );
			ModRM( 3, 7, to );
		}
		write32( from ); 
	}

	/* cmp m64 to r64 */
	void CMP64MtoR( x86IntRegType to, u32 from ) 
	{
		Rex(1, to >> 3, 0, 0);
		write8( 0x3B );
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* cmp imm32 to r32 */
	void CMP32ItoR( x86IntRegType to, u32 from ) 
	{
		if ( to == EAX )
		{
			write8( 0x3D );
		} 
		else 
		{
			write8( 0x81 );
			ModRM( 3, 7, to );
		}
		write32( from ); 
	}

	/* cmp imm32 to m32 */
	void CMP32ItoM( u32* to, u32 from ) 
	{
		write8( 0x81 ); 
		ModRM( 0, 7, DISP32 );
		write32( MEMADDR(to, 8) ); 
		write32( from ); 
	}

	/* cmp r32 to r32 */
	void CMP32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		write8( 0x39 );
		ModRM( 3, from, to );
	}

	/* cmp m32 to r32 */
	void CMP32MtoR( x86IntRegType to, u32 from ) 
	{
		write8( 0x3B );
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* cmp imm16 to r16 */
	void CMP16ItoR( x86IntRegType to, u16 from ) 
	{
		write8( 0x66 ); 
		if ( to == EAX )
		{
			write8( 0x3D );
		} 
		else 
		{
			write8( 0x81 );
			ModRM( 3, 7, to );
		}
		write16( from ); 
	}

	/* cmp imm16 to m16 */
	void CMP16ItoM( u32 to, u16 from ) 
	{
		write8( 0x66 ); 
		write8( 0x81 ); 
		ModRM( 0, 7, DISP32 );
		write32( MEMADDR(to, 6) ); 
		write16( from ); 
	}

	/* cmp r16 to r16 */
	void CMP16RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		write8( 0x66 ); 
		write8( 0x39 );
		ModRM( 3, from, to );
	}

	/* cmp m16 to r16 */
	void CMP16MtoR( x86IntRegType to, u32 from ) 
	{
		write8( 0x66 ); 
		write8( 0x3B );
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* test imm32 to r32 */
	void TEST32ItoR( x86IntRegType to, u32 from ) 
	{
		if ( to == EAX )
		{
			write8( 0xA9 );
		} 
		else 
		{
			write8( 0xF7 );
			ModRM( 3, 0, to );
		}
		write32( from ); 
	}

	/* test r32 to r32 */
	void TEST32RtoR( x86IntRegType to, x86IntRegType from ) 
	{
		write8( 0x85 );
		ModRM( 3, from, to );
	}

	/* setcc r8 */
	void SETcc8R( x86IntRegType to ,u32 cc) 
	{ 
		SET8R( (u8)(0x90|cc),(u8) to ); 
	}

	/* sets r8 */
	void SETS8R( x86IntRegType to ) 
	{ 
		SET8R( 0x98, to ); 
	}

	/* setl r8 */
	void SETL8R( x86IntRegType to ) 
	{ 
		SET8R( 0x9C, to ); 
	}

	/* setb r8 */
	void SETB8R( x86IntRegType to ) 
	{ 
		SET8R( 0x92, to ); 
	}

	/* setb r8 */
	void SETNZ8R( x86IntRegType to ) 
	{ 
		SET8R( 0x95, to ); 
	}

	/* cbw */
	void CBW( void ) 
	{
		write16( 0x9866 ); 
	}

	/* cwd */
	void CWD( void ) 
	{
		write8( 0x98 );
	}

	/* cdq */
	void CDQ( void ) 
	{
		write8( 0x99 ); 
	}

	/* push r32 */
	void PUSH32R( x86IntRegType from ) 
	{
		write8( 0x50 | from ); 
	}

	/* push m32 */
	void PUSH32M( u32 from ) 
	{
		write8( 0xFF );
		ModRM( 0, 6, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* push imm32 */
	void PUSH32I( u32 from ) 
	{
		write8( 0x68 ); 
		write32( from ); 
	}

	/* pop r32 */
	void POP32R( x86IntRegType from ) 
	{
		write8( 0x58 | from ); 
	}

	/* pushad */
	void PUSHA32( void )
	{
		write8( 0x60 ); 
	}

	/* popad */
	void POPA32( void ) 
	{
		write8( 0x61 ); 
	}

	/* pushfd */
	void PUSHFD( void )
	{
		write8( 0x9C );
	}

	/* popfd */
	void POPFD( void )
	{
		write8( 0x9D ); 
	}

	/* ret */
	void RET( void ) 
	{
		/*	write8( 0xF3 );*/ write8( 0xC3 );
	}

	void BT32ItoR( x86IntRegType to, x86IntRegType from ) 
	{
		write16( 0xBA0F );
		write8( 0xE0 | to );
		write8( from );
	}

	//FPU !!

	/********************/
	/* FPU instructions */
	/********************/

	/* fild m32 to fpu reg stack */
	void FILD32( u32 from )
	{
		write8( 0xDB );
		ModRM( 0, 0x0, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* fistp m32 from fpu reg stack */
	void FISTP32( u32 from ) 
	{
		write8( 0xDB );
		ModRM( 0, 0x3, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* fld m32 to fpu reg stack */
	void FLD32( u32 from )
	{
		write8( 0xD9 );
		ModRM( 0, 0x0, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* fst m32 from fpu reg stack */
	void FST32( u32 to ) 
	{
		write8( 0xD9 );
		ModRM( 0, 0x2, DISP32 );
		write32( MEMADDR(to, 4) ); 
	}

	/* fstp m32 from fpu reg stack */
	void FSTP32( u32 to )
	{
		write8( 0xD9 );
		ModRM( 0, 0x3, DISP32 );
		write32( MEMADDR(to, 4) ); 
	}

	/* fldcw fpu control word from m16 */
	void FLDCW( u32 from )
	{
		write8( 0xD9 );
		ModRM( 0, 0x5, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* fnstcw fpu control word to m16 */
	void FNSTCW( u32 to ) 
	{
		write8( 0xD9 );
		ModRM( 0, 0x7, DISP32 );
		write32( MEMADDR(to, 4) ); 
	}

	void FNSTSWtoAX( void ) 
	{
		write16( 0xE0DF );
	}

	/* fadd ST(src) to fpu reg stack ST(0) */
	void FADD32Rto0( x86IntRegType src )
	{
		write8( 0xD8 );
		write8( 0xC0 + src );
	}

	/* fadd ST(0) to fpu reg stack ST(src) */
	void FADD320toR( x86IntRegType src )
	{
		write8( 0xDC );
		write8( 0xC0 + src );
	}

	/* fsub ST(src) to fpu reg stack ST(0) */
	void FSUB32Rto0( x86IntRegType src )
	{
		write8( 0xD8 );
		write8( 0xE0 + src );
	}

	/* fsub ST(0) to fpu reg stack ST(src) */
	void FSUB320toR( x86IntRegType src )
	{
		write8( 0xDC );
		write8( 0xE8 + src );
	}

	/* fsubp -> substract ST(0) from ST(1), store in ST(1) and POP stack */
	void FSUBP( void )
	{
		write8( 0xDE );
		write8( 0xE9 );
	}

	/* fmul ST(src) to fpu reg stack ST(0) */
	void FMUL32Rto0( x86IntRegType src )
	{
		write8( 0xD8 );
		write8( 0xC8 + src );
	}

	/* fmul ST(0) to fpu reg stack ST(src) */
	void FMUL320toR( x86IntRegType src )
	{
		write8( 0xDC );
		write8( 0xC8 + src );
	}

	/* fdiv ST(src) to fpu reg stack ST(0) */
	void FDIV32Rto0( x86IntRegType src )
	{
		write8( 0xD8 );
		write8( 0xF0 + src );
	}

	/* fdiv ST(0) to fpu reg stack ST(src) */
	void FDIV320toR( x86IntRegType src )
	{
		write8( 0xDC );
		write8( 0xF8 + src );
	}

	/* fadd m32 to fpu reg stack */
	void FADD32( u32 from ) 
	{
		write8( 0xD8 );
		ModRM( 0, 0x0, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* fsub m32 to fpu reg stack */
	void FSUB32( u32 from ) 
	{
		write8( 0xD8 );
		ModRM( 0, 0x4, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* fmul m32 to fpu reg stack */
	void FMUL32( u32 from )
	{
		write8( 0xD8 );
		ModRM( 0, 0x1, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* fdiv m32 to fpu reg stack */
	void FDIV32( u32 from ) 
	{
		write8( 0xD8 );
		ModRM( 0, 0x6, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* fabs fpu reg stack */
	void FABS( void )
	{
		write16( 0xE1D9 );
	}

	/* fsqrt fpu reg stack */
	void FSQRT( void ) 
	{
		write16( 0xFAD9 );
	}

	/* fchs fpu reg stack */
	void FCHS( void ) 
	{
		write16( 0xE0D9 );
	}

	/* fcomi st, st(i) */
	void FCOMI( x86IntRegType src )
	{
		write8( 0xDB );
		write8( 0xF0 + src ); 
	}

	/* fcomip st, st(i) */
	void FCOMIP( x86IntRegType src )
	{
		write8( 0xDF );
		write8( 0xF0 + src ); 
	}

	/* fucomi st, st(i) */
	void FUCOMI( x86IntRegType src )
	{
		write8( 0xDB );
		write8( 0xE8 + src ); 
	}

	/* fucomip st, st(i) */
	void FUCOMIP( x86IntRegType src )
	{
		write8( 0xDF );
		write8( 0xE8 + src ); 
	}

	/* fcom m32 to fpu reg stack */
	void FCOM32( u32 from ) 
	{
		write8( 0xD8 );
		ModRM( 0, 0x2, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* fcomp m32 to fpu reg stack */
	void FCOMP32( u32 from )
	{
		write8( 0xD8 );
		ModRM( 0, 0x3, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

#define FCMOV32( low, high ) \
	{ \
	write8( low ); \
	write8( high + from );  \
	}

	void FCMOVB32( x86IntRegType from )     { FCMOV32( 0xDA, 0xC0 ); }
	void FCMOVE32( x86IntRegType from )     { FCMOV32( 0xDA, 0xC8 ); }
	void FCMOVBE32( x86IntRegType from )    { FCMOV32( 0xDA, 0xD0 ); }
	void FCMOVU32( x86IntRegType from )     { FCMOV32( 0xDA, 0xD8 ); }
	void FCMOVNB32( x86IntRegType from )    { FCMOV32( 0xDB, 0xC0 ); }
	void FCMOVNE32( x86IntRegType from )    { FCMOV32( 0xDB, 0xC8 ); }
	void FCMOVNBE32( x86IntRegType from )   { FCMOV32( 0xDB, 0xD0 ); }
	void FCMOVNU32( x86IntRegType from )    { FCMOV32( 0xDB, 0xD8 ); }


	/********************/
	/* MMX instructions */
	/********************/

	// r64 = mm

	/* movq m64 to r64 */
	void MOVQMtoR( x86MMXRegType to, u32 from )
	{
		write16( 0x6F0F );
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* movq r64 to m64 */
	void MOVQRtoM( u32 to, x86MMXRegType from ) 
	{
		write16( 0x7F0F );
		ModRM( 0, from, DISP32 );
		write32(MEMADDR(to, 4)); 
	}

	/* pand r64 to r64 */
	void PANDRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xDB0F );
		ModRM( 3, to, from ); 
	}

	/* por r64 to r64 */
	void PORRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xEB0F );
		ModRM( 3, to, from ); 
	}

	/* pxor r64 to r64 */
	void PXORRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xEF0F );
		ModRM( 3, to, from ); 
	}

	/* psllq r64 to r64 */
	void PSLLQRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xF30F );
		ModRM( 3, to, from ); 
	}

	/* psllq m64 to r64 */
	void PSLLQMtoR( x86MMXRegType to, u32 from ) 
	{
		write16( 0xF30F ); 
		ModRM( 0, to, DISP32 ); 
		write32( MEMADDR(from, 4) );
	}

	/* psllq imm8 to r64 */
	void PSLLQItoR( x86MMXRegType to, u8 from ) 
	{
		write16( 0x730F ); 
		ModRM( 3, 6, to); 
		write8( from ); 
	}

	/* psrlq r64 to r64 */
	void PSRLQRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xD30F ); 
		ModRM( 3, to, from ); 
	}

	/* psrlq m64 to r64 */
	void PSRLQMtoR( x86MMXRegType to, u32 from ) 
	{
		write16( 0xD30F ); 
		ModRM( 0, to, DISP32 ); 
		write32( MEMADDR(from, 4) ); 
	}

	/* psrlq imm8 to r64 */
	void PSRLQItoR( x86MMXRegType to, u8 from ) 
	{
		write16( 0x730F );
		ModRM( 3, 2, to); 
		write8( from ); 
	}

	/* paddusb r64 to r64 */
	void PADDUSBRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xDC0F ); 
		ModRM( 3, to, from ); 
	}

	/* paddusb m64 to r64 */
	void PADDUSBMtoR( x86MMXRegType to, u32 from ) 
	{
		write16( 0xDC0F ); 
		ModRM( 0, to, DISP32 ); 
		write32( MEMADDR(from, 4) ); 
	}

	/* paddusw r64 to r64 */
	void PADDUSWRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xDD0F ); 
		ModRM( 3, to, from ); 
	}

	/* paddusw m64 to r64 */
	void PADDUSWMtoR( x86MMXRegType to, u32 from ) 
	{
		write16( 0xDD0F ); 
		ModRM( 0, to, DISP32 ); 
		write32( MEMADDR(from, 4) ); 
	}

	/* paddb r64 to r64 */
	void PADDBRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xFC0F ); 
		ModRM( 3, to, from ); 
	}

	/* paddb m64 to r64 */
	void PADDBMtoR( x86MMXRegType to, u32 from ) 
	{
		write16( 0xFC0F ); 
		ModRM( 0, to, DISP32 ); 
		write32( MEMADDR(from, 4) ); 
	}

	/* paddw r64 to r64 */
	void PADDWRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xFD0F ); 
		ModRM( 3, to, from ); 
	}

	/* paddw m64 to r64 */
	void PADDWMtoR( x86MMXRegType to, u32 from ) 
	{
		write16( 0xFD0F ); 
		ModRM( 0, to, DISP32 ); 
		write32( MEMADDR(from, 4) ); 
	}

	/* paddd r64 to r64 */
	void PADDDRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xFE0F ); 
		ModRM( 3, to, from ); 
	}

	/* paddd m64 to r64 */
	void PADDDMtoR( x86MMXRegType to, u32 from ) 
	{
		write16( 0xFE0F ); 
		ModRM( 0, to, DISP32 ); 
		write32( MEMADDR(from, 4) ); 
	}

	/* emms */
	void EMMS( void ) 
	{
		write16( 0x770F );
	}

	void PADDSBRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xEC0F ); 
		ModRM( 3, to, from ); 
	}

	void PADDSWRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xED0F );
		ModRM( 3, to, from ); 
	}



	void PSUBSBRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xE80F ); 
		ModRM( 3, to, from ); 
	}

	void PSUBSWRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xE90F );
		ModRM( 3, to, from ); 
	}


	void PSUBBRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xF80F ); 
		ModRM( 3, to, from ); 
	}

	void PSUBWRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xF90F ); 
		ModRM( 3, to, from ); 
	}

	void PSUBDRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xFA0F ); 
		ModRM( 3, to, from ); 
	}

	void PSUBUSBRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xD80F ); 
		ModRM( 3, to, from ); 
	}

	void PSUBUSWRtoR( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0xD90F ); 
		ModRM( 3, to, from ); 
	}

	void PCMPEQBRtoR( x86MMXRegType to, x86MMXRegType from )
	{
		write16( 0x740F ); 
		ModRM( 3, to, from ); 
	}

	void PCMPEQWRtoR( x86MMXRegType to, x86MMXRegType from )
	{
		write16( 0x750F ); 
		ModRM( 3, to, from ); 
	}

	void PCMPEQDRtoR( x86MMXRegType to, x86MMXRegType from )
	{
		write16( 0x760F ); 
		ModRM( 3, to, from ); 
	}

	void PCMPGTBRtoR( x86MMXRegType to, x86MMXRegType from )
	{
		write16( 0x640F ); 
		ModRM( 3, to, from ); 
	}

	void PCMPGTWRtoR( x86MMXRegType to, x86MMXRegType from )
	{
		write16( 0x650F ); 
		ModRM( 3, to, from ); 
	}

	void PCMPGTDRtoR( x86MMXRegType to, x86MMXRegType from )
	{
		write16( 0x660F ); 
		ModRM( 3, to, from ); 
	}

	void PSRLWItoR( x86MMXRegType to, u8 from )
	{
		write16( 0x710F );
		ModRM( 3, 2 , to ); 
		write8( from );
	}

	void PSRLDItoR( x86MMXRegType to, u8 from )
	{
		write16( 0x720F );
		ModRM( 3, 2 , to ); 
		write8( from );
	}

	void PSLLWItoR( x86MMXRegType to, u8 from )
	{
		write16( 0x710F );
		ModRM( 3, 6 , to ); 
		write8( from );
	}

	void PSLLDItoR( x86MMXRegType to, u8 from )
	{
		write16( 0x720F );
		ModRM( 3, 6 , to ); 
		write8( from );
	}

	void PSRAWItoR( x86MMXRegType to, u8 from )
	{
		write16( 0x710F );
		ModRM( 3, 4 , to ); 
		write8( from );
	}

	void PSRADItoR( x86MMXRegType to, u8 from )
	{
		write16( 0x720F );
		ModRM( 3, 4 , to ); 
		write8( from );
	}

	/* por m64 to r64 */
	void PORMtoR( x86MMXRegType to, u32 from ) 
	{
		write16( 0xEB0F );
		ModRM( 0, to, DISP32 ); 
		write32( MEMADDR(from, 4) );
	}

	/* pxor m64 to r64 */
	void PXORMtoR( x86MMXRegType to, u32 from ) 
	{
		write16( 0xEF0F );
		ModRM( 0, to, DISP32 ); 
		write32( MEMADDR(from, 4) );
	}

	/* pand m64 to r64 */
	void PANDMtoR( x86MMXRegType to, u32 from ) 
	{
		u64 rip = (u64)x86Ptr + 7;
		write16( 0xDB0F );
		ModRM( 0, to, DISP32 ); 
		write32( MEMADDR(from, 4) );
	}

	void PANDNMtoR( x86MMXRegType to, u32 from )
	{
		write16( 0xDF0F );
		ModRM( 0, to, DISP32 ); 
		write32( MEMADDR(from, 4) );
	}

	void PUNPCKHDQRtoR( x86MMXRegType to, x86MMXRegType from )
	{
		write16( 0x6A0F );
		ModRM( 3, to, from );
	}

	void PUNPCKLDQRtoR( x86MMXRegType to, x86MMXRegType from )
	{
		write16( 0x620F );
		ModRM( 3, to, from );
	}

	void MOVQ64ItoR( x86MMXRegType reg, u64 i ) 
	{
		MOVQMtoR( reg, ( u32 )(x86Ptr) + 2 + 7 );
		JMP8( 8 );
		write64( i );
	}

	void MOVQRtoR( x86MMXRegType to, x86MMXRegType from )
	{
		write16( 0x6F0F );
		ModRM( 3, to, from );
	}

	/* movd m32 to r64 */
	void MOVDMtoMMX( x86MMXRegType to, u32 from ) 
	{
		write16( 0x6E0F );
		ModRM( 0, to, DISP32 );
		write32( MEMADDR(from, 4) ); 
	}

	/* movq r64 to m32 */
	void MOVDMMXtoM( u32 to, x86MMXRegType from ) 
	{
		write16( 0x7E0F );
		ModRM( 0, from, DISP32 );
		write32( MEMADDR(to, 4) ); 
	}

	/* movd r32 to r64 */
	void MOVD32MMXtoMMX( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0x6E0F );
		ModRM( 3, to, from );
	}

	/* movq r64 to r32 */
	void MOVD64MMXtoMMX( x86MMXRegType to, x86MMXRegType from ) 
	{
		write16( 0x7E0F );
		ModRM( 3, from, to );
	}


	/********************/
	/* SSE instructions */
	/********************/

#define SSEMtoR( code, overb ) \
	if (to > 7) Rex(1, to >> 3, 0, 0); \
	write16( code ); \
	ModRM( 0, to, DISP32 ); \
	write32( MEMADDR(from, 4 + overb) ); \

#define SSERtoM( code, overb ) \
	if (from > 7) Rex(1, from >> 3, 0, 0); \
	write16( code ); \
	ModRM( 0, from, DISP32 ); \
	write32( MEMADDR(to, 4 + overb) ); \

#define SSE_SS_MtoR( code, overb ) \
	write8( 0xf3 ); \
	if (to > 7) Rex(1, to >> 3, 0, 0); \
	write16( code ); \
	ModRM( 0, to, DISP32 ); \
	write32( MEMADDR(from, 4 + overb) ); \

#define SSE_SS_RtoM( code, overb ) \
	write8( 0xf3 ); \
	if (from > 7) Rex(1, from >> 3, 0, 0); \
	write16( code ); \
	ModRM( 0, from, DISP32 ); \
	write32( MEMADDR(to, 4 + overb) ); \

#define SSERtoR( code ) \
	if (to > 7 || from > 7) Rex(1, to >> 3, 0, from >> 3); \
	write16( code ); \
	ModRM( 3, to, from );

#define SSEMtoR66( code ) \
	write8( 0x66 ); \
	SSEMtoR( code, 0 );

#define SSERtoM66( code ) \
	write8( 0x66 ); \
	SSERtoM( code, 0 );

#define SSERtoR66( code ) \
	write8( 0x66 ); \
	SSERtoR( code );

#define _SSERtoR66( code ) \
	write8( 0x66 ); \
	if (to > 7 || from > 7) Rex(1, from >> 3, 0, to >> 3); \
	write16( code ); \
	ModRM( 3, from, to );

#define SSE_SS_RtoR( code ) \
	write8( 0xf3 ); \
	if (to > 7 || from > 7) Rex(1, to >> 3, 0, from >> 3); \
	write16( code ); \
	ModRM( 3, to, from );

#define CMPPSMtoR( op ) \
	SSEMtoR( 0xc20f, 1 ); \
	write8( op );

#define CMPPSRtoR( op ) \
	SSERtoR( 0xc20f ); \
	write8( op );

#define CMPSSMtoR( op ) \
	SSE_SS_MtoR( 0xc20f, 1 ); \
	write8( op );

#define CMPSSRtoR( op ) \
	SSE_SS_RtoR( 0xc20f ); \
	write8( op );



	/* movups [r32][r32*scale] to xmm1 */
	void SSE_MOVUPSRmStoR( x86SSERegType to, x86IntRegType from, x86IntRegType from2, int scale )
	{
		if (to > 7) Rex(1, to >> 3, 0, 0);
		write16( 0x100f );
		ModRM( 0, to, 0x4 );
		SibSB( scale, from2, from );
	}

	/* movups xmm1 to [r32][r32*scale] */
	void SSE_MOVUPSRtoRmS( x86SSERegType to, x86IntRegType from, x86IntRegType from2, int scale )
	{
		if (to > 7) Rex(1, to >> 3, 0, 0);
		write16( 0x110f );
		ModRM( 0, to, 0x4 );
		SibSB( scale, from2, from );
	}

	/* movups [r32] to r32 */
	void SSE_MOVUPSRmtoR( x86IntRegType to, x86IntRegType from ) 
	{
		if (to > 7) Rex(1, to >> 3, 0, 0);
		write16( 0x100f );
		ModRM( 0, to, from );
	}

	/* movups r32 to [r32] */
	void SSE_MOVUPSRtoRm( x86IntRegType to, x86IntRegType from ) 
	{
		write16( 0x110f );
		ModRM( 0, from, to );
	}

	/* movaps [r32][r32*scale] to xmm1 */
	void SSE_MOVAPSRmStoR( x86SSERegType to, x86IntRegType from, x86IntRegType from2, int scale )
	{
		write16( 0x100f );
		ModRM( 0, to, 0x4 );
		SibSB( scale, from2, from );
	}

	/* movaps xmm1 to [r32][r32*scale] */
	void SSE_MOVAPSRtoRmS( x86SSERegType to, x86IntRegType from, x86IntRegType from2, int scale )
	{
		write16( 0x110f );
		ModRM( 0, to, 0x4 );
		SibSB( scale, from2, from );
	}

	/* movaps [r32] to r32 */
	void SSE_MOVAPSRmtoR( x86IntRegType to, x86IntRegType from ) 
	{
		if (to > 7) Rex(1, to >> 3, 0, 0);
		write16( 0x280f );
		ModRM( 0, to, from );
	}

	/* movaps r32 to [r32] */
	void SSE_MOVAPSRtoRm( x86IntRegType to, x86IntRegType from ) 
	{
		if (from > 7) Rex(1, from >> 3, 0, 0);
		write16( 0x290f );
		ModRM( 0, from, to );
	}
	//**********************************************************************************/
	//MOVAPS: Move aligned Packed Single Precision FP values                           *
	//**********************************************************************************
	void SSE_MOVAPS_M128_to_XMM( x86SSERegType to, u32 from )          { SSEMtoR( 0x280f, 0 ); }
	void SSE_MOVAPS_XMM_to_M128( u32 to, x86SSERegType from )          { SSERtoM( 0x290f, 0 ); }
	void SSE_MOVAPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )     { SSERtoR( 0x280f ); }
	//**********************************************************************************/
	//MOVSS: Move Scalar Single-Precision FP  value                                    *
	//**********************************************************************************
	void SSE_MOVSS_M32_to_XMM( x86SSERegType to, u32 from )           { SSE_SS_MtoR( 0x100f, 0 ); }
	void SSE_MOVSS_XMM_to_M32( u32 to, x86SSERegType from )           { SSE_SS_RtoM( 0x110f, 0 ); }
	void SSE_MOVSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )      { SSE_SS_RtoR( 0x100f ); }
	//**********************************************************************************/
	//MOVLPS: Move low Packed Single-Precision FP                                     *
	//**********************************************************************************
	void SSE_MOVLPS_M64_to_XMM( x86SSERegType to, u32 from )          { SSEMtoR( 0x120f, 0 ); }
	void SSE_MOVLPS_XMM_to_M64( u32 to, x86SSERegType from )          { SSERtoM( 0x130f, 0 ); }
	/////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//MOVHPS: Move High Packed Single-Precision FP                                     *
	//**********************************************************************************
	void SSE_MOVHPS_M64_to_XMM( x86SSERegType to, u32 from )          { SSEMtoR( 0x160f, 0 ); }
	void SSE_MOVHPS_XMM_to_M64( u32 to, x86SSERegType from )          { SSERtoM( 0x170f, 0 ); }
	/////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//MOVLHPS: Moved packed Single-Precision FP low to high                            *
	//**********************************************************************************
	void SSE_MOVLHPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )    { SSERtoR( 0x160f ); }
	//////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//MOVHLPS: Moved packed Single-Precision FP High to Low                            *
	//**********************************************************************************
	void SSE_MOVHLPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )    { SSERtoR( 0x120f ); }
	///////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//ANDPS: Logical Bit-wise  AND for Single FP                                        *
	//**********************************************************************************
	void SSE_ANDPS_M128_to_XMM( x86SSERegType to, u32 from )           { SSEMtoR( 0x540f, 0 ); }
	void SSE_ANDPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSERtoR( 0x540f ); }
	///////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//ANDNPS : Logical Bit-wise  AND NOT of Single-precision FP values                 *
	//**********************************************************************************
	void SSE_ANDNPS_M128_to_XMM( x86SSERegType to, u32 from )          { SSEMtoR( 0x550f, 0 ); }
	void SSE_ANDNPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR( 0x550f ); }
	/////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//RCPPS : Packed Single-Precision FP Reciprocal                                     *
	//**********************************************************************************
	void SSE_RCPPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSERtoR( 0x530f ); }
	void SSE_RCPPS_M128_to_XMM( x86SSERegType to, u32 from )           { SSEMtoR( 0x530f, 0 ); }
	//////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//ORPS : Bit-wise Logical OR of Single-Precision FP Data                            *
	//**********************************************************************************
	void SSE_ORPS_M128_to_XMM( x86SSERegType to, u32 from )            { SSEMtoR( 0x560f, 0 ); }
	void SSE_ORPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )  { SSERtoR( 0x560f ); }
	/////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//XORPS : Bitwise Logical XOR of Single-Precision FP Values                        *
	//**********************************************************************************
	void SSE_XORPS_M128_to_XMM( x86SSERegType to, u32 from )           { SSEMtoR( 0x570f, 0 ); }
	void SSE_XORPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSERtoR( 0x570f ); }
	///////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//ADDPS : ADD Packed Single-Precision FP Values                                    *
	//**********************************************************************************
	void SSE_ADDPS_M128_to_XMM( x86SSERegType to, u32 from )           { SSEMtoR( 0x580f, 0 ); }
	void SSE_ADDPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSERtoR( 0x580f ); }
	////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//ADDSS : ADD Scalar Single-Precision FP Values                                    *
	//**********************************************************************************
	void SSE_ADDSS_M32_to_XMM( x86SSERegType to, u32 from )           { SSE_SS_MtoR( 0x580f, 0 ); }
	void SSE_ADDSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSE_SS_RtoR( 0x580f ); }
	/////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//SUBPS: Packed Single-Precision FP Subtract                                       *
	//**********************************************************************************
	void SSE_SUBPS_M128_to_XMM( x86SSERegType to, u32 from )           { SSEMtoR( 0x5c0f, 0 ); }
	void SSE_SUBPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSERtoR( 0x5c0f ); }
	///////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//SUBSS : Scalar  Single-Precision FP Subtract                                       *
	//**********************************************************************************
	void SSE_SUBSS_M32_to_XMM( x86SSERegType to, u32 from )           { SSE_SS_MtoR( 0x5c0f, 0 ); }
	void SSE_SUBSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSE_SS_RtoR( 0x5c0f ); }
	/////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//MULPS : Packed Single-Precision FP Multiply                                      *
	//**********************************************************************************
	void SSE_MULPS_M128_to_XMM( x86SSERegType to, u32 from )           { SSEMtoR( 0x590f, 0 ); }
	void SSE_MULPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSERtoR( 0x590f ); }
	////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//MULSS : Scalar  Single-Precision FP Multiply                                       *
	//**********************************************************************************
	void SSE_MULSS_M32_to_XMM( x86SSERegType to, u32 from )           { SSE_SS_MtoR( 0x590f, 0 ); }
	void SSE_MULSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSE_SS_RtoR( 0x590f ); }
	////////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//Packed Single-Precission FP compare (CMPccPS)                                    *
	//**********************************************************************************
	//missing  SSE_CMPPS_I8_to_XMM
	//         SSE_CMPPS_M32_to_XMM
	//	       SSE_CMPPS_XMM_to_XMM
	void SSE_CMPEQPS_M128_to_XMM( x86SSERegType to, u32 from )         { CMPPSMtoR( 0 ); }
	void SSE_CMPEQPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )    { CMPPSRtoR( 0 ); }
	void SSE_CMPLTPS_M128_to_XMM( x86SSERegType to, u32 from )         { CMPPSMtoR( 1 ); }
	void SSE_CMPLTPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )    { CMPPSRtoR( 1 ); }
	void SSE_CMPLEPS_M128_to_XMM( x86SSERegType to, u32 from )         { CMPPSMtoR( 2 ); }
	void SSE_CMPLEPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )    { CMPPSRtoR( 2 ); }
	void SSE_CMPUNORDPS_M128_to_XMM( x86SSERegType to, u32 from )      { CMPPSMtoR( 3 ); }
	void SSE_CMPUNORDPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { CMPPSRtoR( 3 ); }
	void SSE_CMPNEPS_M128_to_XMM( x86SSERegType to, u32 from )         { CMPPSMtoR( 4 ); }
	void SSE_CMPNEPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )    { CMPPSRtoR( 4 ); }
	void SSE_CMPNLTPS_M128_to_XMM( x86SSERegType to, u32 from )        { CMPPSMtoR( 5 ); }
	void SSE_CMPNLTPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )   { CMPPSRtoR( 5 ); }
	void SSE_CMPNLEPS_M128_to_XMM( x86SSERegType to, u32 from )        { CMPPSMtoR( 6 ); }
	void SSE_CMPNLEPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )   { CMPPSRtoR( 6 ); }
	void SSE_CMPORDPS_M128_to_XMM( x86SSERegType to, u32 from )        { CMPPSMtoR( 7 ); }
	void SSE_CMPORDPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )   { CMPPSRtoR( 7 ); }
	///////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//Scalar Single-Precission FP compare (CMPccSS)                                    *
	//**********************************************************************************
	//missing  SSE_CMPSS_I8_to_XMM
	//         SSE_CMPSS_M32_to_XMM
	//	       SSE_CMPSS_XMM_to_XMM
	void SSE_CMPEQSS_M32_to_XMM( x86SSERegType to, u32 from )         { CMPSSMtoR( 0 ); }
	void SSE_CMPEQSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )    { CMPSSRtoR( 0 ); }
	void SSE_CMPLTSS_M32_to_XMM( x86SSERegType to, u32 from )         { CMPSSMtoR( 1 ); }
	void SSE_CMPLTSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )    { CMPSSRtoR( 1 ); }
	void SSE_CMPLESS_M32_to_XMM( x86SSERegType to, u32 from )         { CMPSSMtoR( 2 ); }
	void SSE_CMPLESS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )    { CMPSSRtoR( 2 ); }
	void SSE_CMPUNORDSS_M32_to_XMM( x86SSERegType to, u32 from )      { CMPSSMtoR( 3 ); }
	void SSE_CMPUNORDSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { CMPSSRtoR( 3 ); }
	void SSE_CMPNESS_M32_to_XMM( x86SSERegType to, u32 from )         { CMPSSMtoR( 4 ); }
	void SSE_CMPNESS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )    { CMPSSRtoR( 4 ); }
	void SSE_CMPNLTSS_M32_to_XMM( x86SSERegType to, u32 from )        { CMPSSMtoR( 5 ); }
	void SSE_CMPNLTSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )   { CMPSSRtoR( 5 ); }
	void SSE_CMPNLESS_M32_to_XMM( x86SSERegType to, u32 from )        { CMPSSMtoR( 6 ); }
	void SSE_CMPNLESS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )   { CMPSSRtoR( 6 ); }
	void SSE_CMPORDSS_M32_to_XMM( x86SSERegType to, u32 from )        { CMPSSMtoR( 7 ); }
	void SSE_CMPORDSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )   { CMPSSRtoR( 7 ); }
	//////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//RSQRTPS : Packed Single-Precision FP Square Root Reciprocal                      *
	//**********************************************************************************
	void SSE_RSQRTPS_M128_to_XMM( x86SSERegType to, u32 from )          { SSEMtoR( 0x520f, 0 ); }
	void SSE_RSQRTPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR( 0x520f ); }
	/////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//RSQRTSS : Scalar Single-Precision FP Square Root Reciprocal                      *
	//**********************************************************************************
	void SSE_RSQRTSS_M32_to_XMM( x86SSERegType to, u32 from )          { SSE_SS_MtoR( 0x520f, 0 ); }
	void SSE_RSQRTSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSE_SS_RtoR( 0x520f ); }
	////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//SQRTPS : Packed Single-Precision FP Square Root                                  *
	//**********************************************************************************
	void SSE_SQRTPS_M128_to_XMM( x86SSERegType to, u32 from )          { SSEMtoR( 0x510f, 0 ); }
	void SSE_SQRTPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR( 0x510f ); }
	//////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//SQRTSS : Scalar Single-Precision FP Square Root                                  *
	//**********************************************************************************
	void SSE_SQRTSS_M32_to_XMM( x86SSERegType to, u32 from )          { SSE_SS_MtoR( 0x510f, 0 ); }
	void SSE_SQRTSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSE_SS_RtoR( 0x510f ); }
	////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//MAXPS: Return Packed Single-Precision FP Maximum                                 *
	//**********************************************************************************
	void SSE_MAXPS_M128_to_XMM( x86SSERegType to, u32 from )           { SSEMtoR( 0x5f0f, 0 ); }
	void SSE_MAXPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSERtoR( 0x5f0f ); }
	/////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//MAXSS: Return Scalar Single-Precision FP Maximum                                 *
	//**********************************************************************************
	void SSE_MAXSS_M32_to_XMM( x86SSERegType to, u32 from )           { SSE_SS_MtoR( 0x5f0f, 0 ); }
	void SSE_MAXSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSE_SS_RtoR( 0x5f0f ); }
	/////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//CVTPI2PS: Packed Signed INT32 to Packed Single  FP Conversion                    *
	//**********************************************************************************
	void SSE_CVTPI2PS_M64_to_XMM( x86SSERegType to, u32 from )        { SSEMtoR( 0x2a0f, 0 ); }
	void SSE_CVTPI2PS_MM_to_XMM( x86SSERegType to, x86MMXRegType from )   { SSERtoR( 0x2a0f ); }
	///////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//CVTPS2PI: Packed Single FP to Packed Signed INT32 Conversion                      *
	//**********************************************************************************
	void SSE_CVTPS2PI_M64_to_MM( x86MMXRegType to, u32 from )        { SSEMtoR( 0x2d0f, 0 ); }
	void SSE_CVTPS2PI_XMM_to_MM( x86MMXRegType to, x86SSERegType from )   { SSERtoR( 0x2d0f ); }
	///////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//CVTDQ2PS: Packed Signed INT32  to Packed Single Precision FP  Conversion         *
	//**********************************************************************************
	void SSE2_CVTDQ2PS_M128_to_XMM( x86SSERegType to, u32 from )        { SSEMtoR( 0x5b0f, 0 ); }
	void SSE2_CVTDQ2PS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )   { SSERtoR( 0x5b0f ); }
	//**********************************************************************************/
	//CVTPS2DQ: Packed Single Precision FP to Packed Signed INT32 Conversion           *
	//**********************************************************************************
	void SSE2_CVTPS2DQ_M128_to_XMM( x86SSERegType to, u32 from )        { SSEMtoR66( 0x5b0f ); }
	void SSE2_CVTPS2DQ_XMM_to_XMM( x86SSERegType to, x86SSERegType from )   { SSERtoR66( 0x5b0f ); }
	/////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//MINPS: Return Packed Single-Precision FP Minimum                                 *
	//**********************************************************************************
	void SSE_MINPS_M128_to_XMM( x86SSERegType to, u32 from )           { SSEMtoR( 0x5d0f, 0 ); }
	void SSE_MINPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSERtoR( 0x5d0f ); }
	//////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//MINSS: Return Scalar Single-Precision FP Minimum                                 *
	//**********************************************************************************
	void SSE_MINSS_M32_to_XMM( x86SSERegType to, u32 from )           { SSE_SS_MtoR( 0x5d0f, 0 ); }
	void SSE_MINSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSE_SS_RtoR( 0x5d0f ); }
	///////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//PMAXSW: Packed Signed Integer Word Maximum                                        *
	//**********************************************************************************
	//missing
	//     SSE_PMAXSW_M64_to_MM
	//		SSE2_PMAXSW_M128_to_XMM
	//		SSE2_PMAXSW_XMM_to_XMM
	void SSE_PMAXSW_MM_to_MM( x86MMXRegType to, x86MMXRegType from ){ SSERtoR( 0xEE0F ); }
	///////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//PMINSW: Packed Signed Integer Word Minimum                                        *
	//**********************************************************************************
	//missing
	//     SSE_PMINSW_M64_to_MM
	//		SSE2_PMINSW_M128_to_XMM
	//		SSE2_PMINSW_XMM_to_XMM
	void SSE_PMINSW_MM_to_MM( x86MMXRegType to, x86MMXRegType from ){ SSERtoR( 0xEA0F ); }
	//////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//SHUFPS: Shuffle Packed Single-Precision FP Values                                *
	//**********************************************************************************
	void SSE_SHUFPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from, u8 imm8 )	{ SSERtoR( 0xC60F ); write8( imm8 ); }
	void SSE_SHUFPS_M128_to_XMM( x86SSERegType to, u32 from, u8 imm8 )		{ SSEMtoR( 0xC60F, 1 ); write8( imm8 ); }
	////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//PSHUFD: Shuffle Packed DoubleWords                                               *
	//**********************************************************************************
	void SSE2_PSHUFD_XMM_to_XMM( x86SSERegType to, x86SSERegType from, u8 imm8 )	{ SSERtoR66( 0x700F ); write8( imm8 ); }
	void SSE2_PSHUFD_M128_to_XMM( x86SSERegType to, u32 from, u8 imm8 )	{ SSEMtoR66( 0x700F ); write8( imm8 ); }
	///////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//UNPCKLPS: Unpack and Interleave low Packed Single-Precision FP Data              *
	//**********************************************************************************
	//missing SSE_UNPCKLPS_M128_to_XMM
	void SSE_UNPCKLPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )	{ SSERtoR( 0x140F ); }
	////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//UNPCKHPS: Unpack and Interleave High Packed Single-Precision FP Data              *
	//**********************************************************************************
	//missing SSE_UNPCKHPS_M128_to_XMM
	void SSE_UNPCKHPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from )	{ SSERtoR( 0x150F ); }
	////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//DIVPS : Packed Single-Precision FP Divide                                       *
	//**********************************************************************************
	void SSE_DIVPS_M128_to_XMM( x86SSERegType to, u32 from )           { SSEMtoR( 0x5e0F, 0 ); }
	void SSE_DIVPS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSERtoR( 0x5e0F ); }
	//////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//DIVSS : Scalar  Single-Precision FP Divide                                       *
	//**********************************************************************************
	void SSE_DIVSS_M32_to_XMM( x86SSERegType to, u32 from )           { SSE_SS_MtoR( 0x5e0F, 0 ); }
	void SSE_DIVSS_XMM_to_XMM( x86SSERegType to, x86SSERegType from ) { SSE_SS_RtoR( 0x5e0F ); }
	/////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//STMXCSR : Store Streaming SIMD Extension Control/Status                         *
	//**********************************************************************************
	void SSE_STMXCSR( u32 from ) {
		write16( 0xAE0F );
		ModRM( 0, 0x3, DISP32 );
		write32( MEMADDR(from, 4) );
	}
	/////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//LDMXCSR : Load Streaming SIMD Extension Control/Status                         *
	//**********************************************************************************
	void SSE_LDMXCSR( u32 from ) {
		write16( 0xAE0F );
		ModRM( 0, 0x2, DISP32 );
		write32( MEMADDR(from, 4) );
	}
	/////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//PADDB,PADDW,PADDD : Add Packed Integers                                          *
	//**********************************************************************************
	void SSE2_PADDB_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xFC0F ); }
	void SSE2_PADDB_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0xFC0F ); }
	void SSE2_PADDW_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xFD0F ); }
	void SSE2_PADDW_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0xFD0F ); }
	void SSE2_PADDD_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xFE0F ); }
	void SSE2_PADDD_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0xFE0F ); }
	///////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//PCMPxx: Compare Packed Integers                                                  *
	//**********************************************************************************
	void SSE2_PCMPGTB_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0x640F ); }
	void SSE2_PCMPGTB_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0x640F ); }
	void SSE2_PCMPGTW_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0x650F ); }
	void SSE2_PCMPGTW_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0x650F ); }
	void SSE2_PCMPGTD_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0x660F ); }
	void SSE2_PCMPGTD_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0x660F ); }
	void SSE2_PCMPEQB_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0x740F ); }
	void SSE2_PCMPEQB_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0x740F ); }
	void SSE2_PCMPEQW_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0x750F ); }
	void SSE2_PCMPEQW_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0x750F ); }
	void SSE2_PCMPEQD_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0x760F ); }
	void SSE2_PCMPEQD_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0x760F ); }
	////////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//PSUBx: Subtract Packed Integers                                                  *
	//**********************************************************************************
	void SSE2_PSUBB_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xF80F ); }
	void SSE2_PSUBB_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0xF80F ); }
	void SSE2_PSUBW_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xF90F ); }
	void SSE2_PSUBW_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0xF90F ); }
	void SSE2_PSUBD_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xFA0F ); }
	void SSE2_PSUBD_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0xFA0F ); }
	void SSE2_PSUBQ_XMM_to_XMM(x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xFB0F ); }
	void SSE2_PSUBQ_M128_to_XMM(x86SSERegType to, u32 from ){ SSEMtoR66( 0xFB0F ); }
	///////////////////////////////////////////////////////////////////////////////////////
	//**********************************************************************************/
	//MOVD: Move Dword(32bit) to /from XMM reg                                         *
	//**********************************************************************************
	void SSE2_MOVD_M32_to_XMM( x86SSERegType to, u32 from ) { SSEMtoR66(0x6E0F); }
	void SSE2_MOVD_32R_to_XMM( x86SSERegType to, x86IntRegType from ) { _SSERtoR66(0x6E0F); }
	void SSE2_MOVD_XMM_to_M32( u32 to, x86SSERegType from ) { SSERtoM66(0x7E0F); }
	void SSE2_MOVD_XMM_to_32R( x86IntRegType to, x86SSERegType from ) { _SSERtoR66(0x7E0F); }
	////////////////////////////////////////////////////////////////////////////////////


	//linuz add these thanks ;P
	void SSE2_MOVDQA_M128_to_XMM(x86SSERegType to, u32 from) {SSEMtoR66(0x6F0F); } //npt sure
	void SSE2_MOVDQA_XMM_to_M128( u32 to, x86SSERegType from ){SSERtoM66(0x7F0F);} //not sure
	/*
	void SSE2_PSRLW_I8_to_XMM(x86SSERegType to, u32 from) { }
	void SSE2_PSRLD_I8_to_XMM(x86SSERegType to, u32 from) { }
	void SSE2_PSRAW_I8_to_XMM(x86SSERegType to, u32 from) { }
	void SSE2_PSRAD_I8_to_XMM(x86SSERegType to, u32 from) { }
	void SSE2_PSLLW_I8_to_XMM(x86SSERegType to, u32 from) { }
	void SSE2_PSLLD_I8_to_XMM(x86SSERegType to, u32 from) { }
	*/

	// shift right logical

	void SSE2_PSRLW_XMM_to_XMM(x86SSERegType to, x86SSERegType from) { SSERtoR66(0xD10F); }
	void SSE2_PSRLW_M128_to_XMM(x86SSERegType to, u32 from) { SSEMtoR66(0xD10F); }
	void SSE2_PSRLW_I8_to_XMM(x86SSERegType to, u8 imm8)
	{
		// FIXME
		write8( 0x66 );
		write16( 0x710F );
		ModRM( 3, 2 , to );
		write8( imm8 );
	}

	void SSE2_PSRLD_XMM_to_XMM(x86SSERegType to, x86SSERegType from) { SSERtoR66(0xD20F); }
	void SSE2_PSRLD_M128_to_XMM(x86SSERegType to, u32 from) { SSEMtoR66(0xD20F); }
	void SSE2_PSRLD_I8_to_XMM(x86SSERegType to, u8 imm8)
	{
		// FIXME
		write8( 0x66 );
		write16( 0x720F );
		ModRM( 3, 2 , to ); 
		write8( imm8 );
	}

	void SSE2_PSRLQ_XMM_to_XMM(x86SSERegType to, x86SSERegType from) { SSERtoR66(0xD30F); }
	void SSE2_PSRLQ_M128_to_XMM(x86SSERegType to, u32 from) { SSEMtoR66(0xD30F); }
	void SSE2_PSRLQ_I8_to_XMM(x86SSERegType to, u8 imm8)
	{
		// FIXME
		write8( 0x66 );
		write16( 0x730F );
		ModRM( 3, 2 , to ); 
		write8( imm8 );
	}

	void SSE2_PSRLDQ_I8_to_XMM(x86SSERegType to, u8 imm8)
	{
		// FIXME
		write8( 0x66 );
		write16( 0x730F );
		ModRM( 3, 7 , to ); 
		write8( imm8 );
	}

	// shift right arithmetic

	void SSE2_PSRAW_XMM_to_XMM(x86SSERegType to, x86SSERegType from) { SSERtoR66(0xE10F); }
	void SSE2_PSRAW_M128_to_XMM(x86SSERegType to, u32 from) { SSEMtoR66(0xE10F); }
	void SSE2_PSRAW_I8_to_XMM(x86SSERegType to, u8 imm8)
	{
		// FIXME
		write8( 0x66 );
		write16( 0x710F );
		ModRM( 3, 4 , to );
		write8( imm8 );
	}

	void SSE2_PSRAD_XMM_to_XMM(x86SSERegType to, x86SSERegType from) { SSERtoR66(0xE20F); }
	void SSE2_PSRAD_M128_to_XMM(x86SSERegType to, u32 from) { SSEMtoR66(0xE20F); }
	void SSE2_PSRAD_I8_to_XMM(x86SSERegType to, u8 imm8)
	{
		// FIXME
		write8( 0x66 );
		write16( 0x720F );
		ModRM( 3, 4 , to );
		write8( imm8 );
	}

	// shift left logical

	void SSE2_PSLLW_XMM_to_XMM(x86SSERegType to, x86SSERegType from) { SSERtoR66(0xF10F); }
	void SSE2_PSLLW_M128_to_XMM(x86SSERegType to, u32 from) { SSEMtoR66(0xF10F); }
	void SSE2_PSLLW_I8_to_XMM(x86SSERegType to, u8 imm8)
	{
		// FIXME
		write8( 0x66 );
		write16( 0x710F );
		ModRM( 3, 6 , to );
		write8( imm8 );
	}

	void SSE2_PSLLD_XMM_to_XMM(x86SSERegType to, x86SSERegType from) { SSERtoR66(0xF20F); }
	void SSE2_PSLLD_M128_to_XMM(x86SSERegType to, u32 from) { SSEMtoR66(0xF20F); }
	void SSE2_PSLLD_I8_to_XMM(x86SSERegType to, u8 imm8)
	{
		// FIXME
		write8( 0x66 );
		write16( 0x720F );
		ModRM( 3, 6 , to );
		write8( imm8 );
	}

	void SSE2_PSLLQ_XMM_to_XMM(x86SSERegType to, x86SSERegType from) { SSERtoR66(0xF30F); }
	void SSE2_PSLLQ_M128_to_XMM(x86SSERegType to, u32 from) { SSEMtoR66(0xF30F); }
	void SSE2_PSLLQ_I8_to_XMM(x86SSERegType to, u8 imm8)
	{
		// FIXME
		write8( 0x66 );
		write16( 0x730F );
		ModRM( 3, 6 , to );
		write8( imm8 );
	}

	void SSE2_PSLLDQ_I8_to_XMM(x86SSERegType to, u8 imm8)
	{
		// FIXME
		write8( 0x66 );
		write16( 0x730F );
		ModRM( 3, 7 , to ); 
		write8( imm8 );
	}

	//

	void SSE2_PMAXSW_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xEE0F ); }
	void SSE2_PMAXSW_M128_to_XMM( x86SSERegType to, u32 from ){ SSEMtoR66( 0xEE0F ); }

	void SSE2_PMAXUB_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xDE0F ); }
	void SSE2_PMAXUB_M128_to_XMM( x86SSERegType to, u32 from ){ SSEMtoR66( 0xDE0F ); }

	void SSE2_PMINSW_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xEA0F ); }
	void SSE2_PMINSW_M128_to_XMM( x86SSERegType to, u32 from ){ SSEMtoR66( 0xEA0F ); }

	void SSE2_PMINUB_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xDA0F ); }
	void SSE2_PMINUB_M128_to_XMM( x86SSERegType to, u32 from ){ SSEMtoR66( 0xDA0F ); }

	//

	void SSE2_PADDSB_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xEC0F ); }
	void SSE2_PADDSB_M128_to_XMM( x86SSERegType to, u32 from ){ SSEMtoR66( 0xEC0F ); }

	void SSE2_PADDSW_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xED0F ); }
	void SSE2_PADDSW_M128_to_XMM( x86SSERegType to, u32 from ){ SSEMtoR66( 0xED0F ); }

	void SSE2_PSUBSB_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xE80F ); }
	void SSE2_PSUBSB_M128_to_XMM( x86SSERegType to, u32 from ){ SSEMtoR66( 0xE80F ); }

	void SSE2_PSUBSW_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xE90F ); }
	void SSE2_PSUBSW_M128_to_XMM( x86SSERegType to, u32 from ){ SSEMtoR66( 0xE90F ); }

	//

	void SSE2_PXOR_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xEF0F ); }
	void SSE2_PXOR_M128_to_XMM( x86SSERegType to, u32 from ){ SSEMtoR66( 0xEF0F ); }



	void SSE2_PADDUSB_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xDC0F ); }
	void SSE2_PADDUSB_M128_to_XMM( x86SSERegType to, u32 from ){ SSEMtoR66( 0xDC0F ); }
	void SSE2_PADDUSW_XMM_to_XMM( x86SSERegType to, x86SSERegType from ){ SSERtoR66( 0xDD0F ); }
	void SSE2_PADDUSW_M128_to_XMM( x86SSERegType to, u32 from ){ SSEMtoR66( 0xDD0F ); }




	//basic logical structures
	void StartIf(RegInfo r1,RegInfo r2,ConditionCode cc)
	{
	}
	void StartIf(RegInfo r1,ConditionCode cc)
	{
	}
	void StartIf(ConditionCode cc)
	{
	}

	void ElseIf(RegInfo r1,RegInfo r2,ConditionCode cc)
	{
	}
	void ElseIf(RegInfo r1,ConditionCode cc)
	{
	}
	void ElseIf(ConditionCode cc)
	{
	}
	void Else()
	{
	}

	void EndIf()
	{
	}

	//Labels
	Label* GenLabel()
	{
	}
	Label* GenLabel(char* name)
	{
	}
	void MarkLabel(Label* label)
	{
	}

	//jumps
	void Jump(Label* label)
	{
	}
	void JumpCond(RegInfo r1,RegInfo r2,ConditionCode cc,Label* label)
	{
	}
	void JumpCond(RegInfo r1,ConditionCode cc,Label* label)
	{
	}
	void JumpCond(ConditionCode cc,Label* label)
	{
	}
};