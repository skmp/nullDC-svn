#include "chankaAICA.h"
#include "chanka_aica.h"

#include "aica\base.h"
#include "aica\arm7.h"
#include "aica\aica.h"


u8*g_pSH4SoundRAM;



#define SH4Memory_MASK ( ~0xE0000000)
#define SH4AicaMemory_START ( 0x00700000)

#define SH4SoundRAM_START ( 0x00800000)


bool g_bArm7Enable=true;

//u32 sh4_cycles;

CArm7* g_pArm7 = NULL;

HWND g_hWnd;


u32 g_videoCableType=2;

#include "aica\arm7Memory.h"

u32 FASTCALL ReadMem_reg(u32 addr,u32 size)
{
	DWORD uAddressAux = addr&SH4Memory_MASK;
	switch(size)
	{
	case 1:
		return AICAReadByte(uAddressAux-SH4AicaMemory_START);
		break;
	case 2:
		return AICAReadWord(uAddressAux-SH4AicaMemory_START);
		break;
	case 4:
		return AICAReadDword(uAddressAux-SH4AicaMemory_START);
		break;
	}

	return 0;
}
void FASTCALL WriteMem_reg(u32 addr,u32 data,u32 size)
{
	DWORD uAddressAux = addr&SH4Memory_MASK;
	switch(size)
	{
	case 1:
		WriteAicaReg<1>(uAddressAux-SH4AicaMemory_START,(BYTE)data);
		break;
	case 2:
		WriteAicaReg<2>(uAddressAux-SH4AicaMemory_START,(WORD)data);
		break;
	case 4:
		WriteAicaReg<2>(uAddressAux-SH4AicaMemory_START,data);
		break;
	}

}

/*
u32 ReadMem_ram(u32 addr,u32 size)
{
DWORD uData;
DWORD uAddress = addr;
uAddress&=SH4Memory_MASK;	

uAddress=uAddress-SH4SoundRAM_START;

uAddress&=Arm7SoundRAMMask;

switch(size)
{
case 1:
uData = *((BYTE*)(g_pSH4SoundRAM + (uAddress)));	
break;
case 2:
uData = *((WORD*)(g_pSH4SoundRAM + (uAddress)));	
break;
case 4:
uData = *((DWORD*)(g_pSH4SoundRAM + (uAddress)));	
break;
}

return uData;

}
void WriteMem_ram(u32 addr,u32 data,u32 size)
{
DWORD uAddress = addr;
uAddress&=SH4Memory_MASK;	

uAddress=uAddress-SH4SoundRAM_START;

uAddress&=Arm7SoundRAMMask;

switch(size)
{
case 1:
*((BYTE*)(g_pSH4SoundRAM + (uAddress)))=data;	
break;
case 2:
*((WORD*)(g_pSH4SoundRAM + (uAddress)))=data;	
break;
case 4:
*((DWORD*)(g_pSH4SoundRAM + (uAddress)))=data;	
break;
}
}

*/
void FASTCALL UpdateSystem(u32 Cycles)
{
	//sh4_cycles+=Cycles;
	//g_pArm7->BlockStepArm7(Cycles/(8*ARM7BIAS));
	//AICARefresh(Cycles);
	UpdateAICA(Cycles);
}

void InitArm7Memory();
void InitARM7()
{
	g_hWnd=(HWND)emu.GetRenderTarget();
	g_pArm7 = NEW(CArm7);
	InitArm7Memory();
	g_pArm7->Init();
	AICA_Init();
}
void TerminateARM7()
{
	g_pArm7->End();
	delete g_pArm7;
	AICA_Term();
}

void LogVentanaDebug( const char *szFichero, int linea, const char *szExpr )
{
	char buf[3000];

	sprintf(buf,"ASSERTION\r\nFile %s\r\nLine %d\r\nExpresion %s",szFichero,linea,szExpr);
	ODS(("%s", buf));
}

bool VentanaDebug( const char *szFichero, int linea, const char *szExpr )
{
	char buf[3000];

	sprintf(buf,"ASSERTION\r\nFile %s\r\nLine %d\r\nExpresion %s\r\n\r\n�Desea seguir viendo asserts?",szFichero,linea,szExpr);
	int rv = ::MessageBox(NULL,buf,"ASSERTION",MB_YESNO);
	return (rv == IDYES);
}

