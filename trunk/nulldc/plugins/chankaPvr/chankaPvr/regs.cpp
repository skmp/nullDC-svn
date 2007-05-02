#include "pvr.h"
#include "regs.h"
#include "tavideo.h"

//#include "renderer_if.h"


//seems to work allright .. ;P
//many things need to be emulated , especialy to support lle emulation but for now that's not needed
u8 regs[RegSize];

u32 FASTCALL ReadPvrRegister(u32 addr,u32 size)
{
	if (size!=4)
	{
		//error
		return 0;
	}

	return PvrReg(addr,u32);
}
void CalculateSync();
void SH4HWRegistersWriteDword(const DWORD uAddress, const DWORD uData)
{
	DWORD uAddressAux = uAddress & (~0xE0000000);
	
	DWORD addr=uAddressAux & RegMask;
	if (addr>=SPG_HBLANK_INT_addr && addr<=SPG_WIDTH_addr)
		CalculateSync();

	switch (uAddressAux)
	{
		//START_RENDER									=	0x05f8014,
	case 0x05f8014:
		if (uData != 0x0)
			TAStartRender();											
		break;
	case 0x05f8008:
		if (uData&0x1)
		{
			TAResetRegistration();
		}
		
			//TABeginRender();
		break;	

	case TPVR::PVR_DISPLAY_ADDR1:
		TANotifyDisplayAddressChange();
		break;
			
	case 0x005F8160:
    TAContinueRegistration();		
		break;
	case TPVR::PVR_TA_INIT:
		{			
			if (uData & 0x80000000)
				TAStartRegistration();

		}
		break;

	case 0x005F6820:
		if (uData&0x1)
		{
			//MessageBox(NULL,"chanka","sort dma",MB_OK);
			printf("SortDMA\n");
		}
		break;
  case TPVR::PVR_SYSTEM_RESET:
    if (uData  == 0x7611)
      printf("Reset ?!?!\n");//g_psh4_dynarec->DoReset();
    break;

	default:		
		//NaomiIOWriteDWord(uAddress,uData);
		break;
	}	

  //ODS(("WARNING_L1::HWRegisters::WriteLong %08X at %08X at %08X",uData,uAddress,g_pSH4->GetPC()));
}


void FASTCALL WritePvrRegister(u32 addr,u32 data,u32 size)
{

	if (size!=4)
	{
		//error
		return;
	}
	SH4HWRegistersWriteDword(addr,data);
	if ((addr&RegMask)==ID_addr)
		return;//read olny
	if ((addr&RegMask)==REVISION_addr)
		return;//read olny

	if ((addr&RegMask)==STARTRENDER_addr)
	{
		//start render
		//renderer->StartRender();
		//TODO : fix that mess
		//RaiseInterrupt(InterruptID::holly_RENDER_DONE);
		//RaiseInterrupt(InterruptID::holly_RENDER_DONE_isp);
		//RaiseInterrupt(InterruptID::holly_RENDER_DONE_vd);
		return;
	}

	if ((addr&RegMask)==TA_LIST_INIT_addr)
	{
		if (data>>31)
		{
//			Ta_ListInit();
			data=0;
		}
	}

	if ((addr&RegMask)==SOFTRESET_addr)
	{
		if (data!=0)
		{
			//if (data&1)
				//Ta_SoftReset();
			data=0;
		}
	}

	if ((addr&RegMask)==TA_LIST_CONT_addr)
	{
		//a write of anything works ?
	//	Ta_ListCont();
	}

	

	PvrReg(addr,u32)=data;
}


bool Regs_Init()
{
	return true;
}

void Regs_Term()
{
}

void Regs_Reset(bool Manual)
{
	ID					= 0x17FD11DB;
	REVISION			= 0x00000011;
	SOFTRESET			= 0x00000007;
	SPG_HBLANK_INT.full	= 0x031D0000;
	SPG_VBLANK_INT.full	= 0x01500104;
	FPU_PARAM_CFG		= 0x0007DF77;
	HALF_OFFSET			= 0x00000007;
	ISP_FEED_CFG		= 0x00402000;
	SDRAM_REFRESH		= 0x00000020;
	SDRAM_ARB_CFG		= 0x0000001F;
	SDRAM_CFG			= 0x15F28997;
	SPG_HBLANK.full		= 0x007E0345;
	SPG_LOAD.full		= 0x01060359;
	SPG_VBLANK.full		= 0x01500104;
	SPG_WIDTH.full		= 0x07F1933F;
	VO_CONTROL			= 0x00000108;
	VO_STARTX			= 0x0000009D;
	VO_STARTY			= 0x00000015;
	SCALER_CTL			= 0x00000400;
	FB_BURSTCTRL		= 0x00090639;
	PT_ALPHA_REF		= 0x000000FF;
}