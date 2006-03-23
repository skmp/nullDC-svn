#include "types.h"
#include "sh4_mem.h"
#include "sb.h"
#include "dc/pvr/pvr_if.h"
#include "dc/gdrom/gdrom_if.h"
#include "dc/aica/aica_if.h"

#include "plugins/plugin_manager.h"

//Area 0 mem map
//0x00000000- 0x001FFFFF	:MPX	System/Boot ROM
//0x00200000- 0x0021FFFF	:Flash Memory
//0x00400000- 0x005F67FF	:Unassigned
//0x005F6800- 0x005F69FF	:System Control Reg.
//0x005F6C00- 0x005F6CFF	:Maple i/f Control Reg.
//0x005F7000- 0x005F70FF	:GD-ROM
//0x005F7400- 0x005F74FF	:G1 i/f Control Reg.
//0x005F7800- 0x005F78FF	:G2 i/f Control Reg.
//0x005F7C00- 0x005F7CFF	:PVR i/f Control Reg.
//0x005F8000- 0x005F9FFF	:TA / PVR Core Reg.
//0x00600000- 0x006007FF	:MODEM
//0x00600800- 0x006FFFFF	:G2 (Reserved)
//0x00700000- 0x00707FFF	:AICA- Sound Cntr. Reg.
//0x00710000- 0x0071000B	:AICA- RTC Cntr. Reg.
//0x00800000- 0x00FFFFFF	:AICA- Wave Memory
//0x01000000- 0x01FFFFFF	:Ext. Device
//0x02000000- 0x03FFFFFF*	:Image Area*	2MB

//use unified size handler for registers
//it realy makes no sense to use different size handlers on em
u32 ReadMem_area0(u32 addr,u32 sz)
{
	addr &= 0x01FFFFFF;//to get rid of non needed bits
	if ((addr<=0x001FFFFF))//	:MPX	System/Boot ROM
	{
		/*switch (sz)
		{
		case 1:
			return [addr];
		case 2:
			return *(u16*)&bios_b[addr];
		case 4:
			return *(u32*)&bios_b[addr];
		}*/
		ReadMemArrRet(bios_b,addr,sz);
		EMUERROR3("Read from [MPX	System/Boot ROM], addr=%x , sz=%d",addr,sz);
	}
	else if ((addr>= 0x00200000) && (addr<= 0x0021FFFF))		//	:Flash Memory
	{
		//ReadMemFromPtrRet(flashrom,adr-0x00200000,sz);
		ReadMemArrRet(flash_b,addr-0x00200000,sz);
		EMUERROR3("Read from [Flash Memory], addr=%x , sz=%d",addr,sz);
	}
	else if ((addr>= 0x00400000) && (addr<= 0x005F67FF))		//	:Unassigned
	{
		EMUERROR2("Read from area0_32 not implemented [Unassigned], addr=%x",addr);
	}
	
	else if ((addr>= 0x005F6800) && (addr<= 0x005F69FF))		//	:System Control Reg.
	{
		return sb_ReadMem(addr,sz);
	}
	else if ((addr>= 0x005F6C00) && (addr<= 0x005F6CFF)) //	:Maple i/f Control Reg.
	{
		return sb_ReadMem(addr,sz);
	}
	else if ((addr>= 0x005F7400) && (addr<=0x005F74FF)) //	:G1 i/f Control Reg.
	{
		return sb_ReadMem(addr,sz);
	}
	else if ((addr>= 0x005F7800) && (addr<=0x005F78FF)) //	:G2 i/f Control Reg.
	{
		return sb_ReadMem(addr,sz);
	}
	
	else if ((addr>= 0x005F7000) && (addr<= 0x005F70FF)) //	:GD-ROM
	{
		//EMUERROR3("Read from area0_32 not implemented [GD-ROM], addr=%x,size=%d",addr,sz);
		return ReadMem_gdrom(addr,sz);
	}
	else if ((addr>= 0x005F7C00) && (addr<=0x005F7CFF)) //	/*:PVR i/f Control Reg.*/ -> ALL SB registers now
	{
		//EMUERROR2("Read from area0_32 not implemented [PVR i/f Control Reg], addr=%x",addr);
		return sb_ReadMem(addr,sz);
	}
	else if ((addr>= 0x005F8000) && (addr<=0x005F9FFF)) //	:TA / PVR Core Reg.
	{
		//EMUERROR2("Read from area0_32 not implemented [TA / PVR Core Reg], addr=%x",addr);
		return pvr_readreg_TA(addr,sz);
	}
	else if ((addr>= 0x00600000) && (addr<= 0x006007FF)) //	:MODEM
	{
		EMUERROR2("Read from area0_32 not implemented [MODEM], addr=%x",addr);
	}
	else if ((addr>= 0x00600800) && (addr<= 0x006FFFFF)) //	:G2 (Reserved)
	{
		EMUERROR2("Read from area0_32 not implemented [G2 (Reserved)], addr=%x",addr);
	}
	else if ((addr>= 0x00700000) && (addr<=0x00707FFF)) //	:AICA- Sound Cntr. Reg.
	{
		//EMUERROR2("Read from area0_32 not implemented [AICA- Sound Cntr. Reg], addr=%x",addr);
		return libAICA->aica_info.ReadMem_aica_reg(addr,sz);
	}
	else if ((addr>= 0x00710000) && (addr<= 0x0071000B)) //	:AICA- RTC Cntr. Reg.
	{
		//EMUERROR2("Read from area0_32 not implemented [AICA- RTC Cntr. Reg], addr=%x",addr);
		return ReadMem_aica_rtc(addr,sz);
	}
	else if ((addr>= 0x00800000) && (addr<=0x00FFFFFF)) //	:AICA- Wave Memory
	{
		//EMUERROR2("Read from area0_32 not implemented [AICA- Wave Memory], addr=%x",addr);
		return libAICA->aica_info.ReadMem_aica_ram(addr,sz);
	}
	else if ((addr>= 0x01000000) && (addr<= 0x01FFFFFF)) //	:Ext. Device
	{
		EMUERROR2("Read from area0_32 not implemented [Ext. Device], addr=%x",addr);
	}
	else
	{
		EMUERROR2("Read from area0_32 not mapped!!! , addr=%x",addr);
	}
	return 0;
}

void WriteMem_area0(u32 addr,u32 data,u32 sz)
{
	addr &= 0x01FFFFFF;//to get rid of non needed bits
	if ((addr<=0x001FFFFF))//	:MPX	System/Boot ROM
	{
		//WriteMemFromPtrRet(bootfile,adr,sz);
		EMUERROR4("Write to  [MPX	System/Boot ROM] is not possible, addr=%x,data=%x,size=%d",addr,data,sz);
	}
	else if ((addr>= 0x00200000) && (addr<= 0x0021FFFF))		//	:Flash Memory
	{
		WriteMemArrRet(flash_b,addr-0x00200000,data,sz);
		EMUERROR4("Write to [Flash Memory] , sz?!, addr=%x,data=%x,size=%d",addr,data,sz);
	}
	else if ((addr>= 0x00400000) && (addr<= 0x005F67FF))		//	:Unassigned
	{
		EMUERROR4("Write to area0_32 not implemented [Unassigned], addr=%x,data=%x,size=%d",addr,data,sz);
	}
	else if ((addr>= 0x005F6800) && (addr<= 0x005F69FF))		//	:System Control Reg.
	{
		//EMUERROR4("Write to area0_32 not implemented [System Control Reg], addr=%x,data=%x,size=%d",addr,data,sz);
		sb_WriteMem(addr,data,sz);
		return;
	}
	else if ((addr>= 0x005F6C00) && (addr<= 0x005F6CFF)) //	:Maple i/f Control Reg.
	{
		//EMUERROR4("Write to area0_32 not implemented [Maple i/f Control Reg], addr=%x,data=%x,size=%d",addr,data,sz);
		sb_WriteMem(addr,data,sz);
	}
	else if ((addr>= 0x005F7400) && (addr<=0x005F74FF)) //	:G1 i/f Control Reg.
	{
		//EMUERROR4("Write to area0_32 not implemented [G1 i/f Control Reg], addr=%x,data=%x,size=%d",addr,data,sz);
		sb_WriteMem(addr,data,sz);
	}
	else if ((addr>= 0x005F7800) && (addr<=0x005F78FF)) //	:G2 i/f Control Reg.
	{
		//EMUERROR4("Write to area0_32 not implemented [G2 i/f Control Reg], addr=%x,data=%x,size=%d",addr,data,sz);
		sb_WriteMem(addr,data,sz);
	}
	else if ((addr>= 0x005F7000) && (addr<= 0x005F70FF)) //	:GD-ROM
	{
		//EMUERROR4("Write to area0_32 not implemented [GD-ROM], addr=%x,data=%x,size=%d",addr,data,sz);
		WriteMem_gdrom(addr,data,sz);
	}
	else if ((addr>= 0x005F7C00) && (addr<=0x005F7CFF)) //	/*:PVR i/f Control Reg.*/ -> ALL SB registers
	{
		//EMUERROR4("Write to area0_32 not implemented [PVR i/f Control Reg], addr=%x,data=%x,size=%d",addr,data,sz);
		sb_WriteMem(addr,data,sz);
	}
	else if ((addr>= 0x005F8000) && (addr<=0x005F9FFF)) //	:TA / PVR Core Reg.
	{
		//EMUERROR4("Write to area0_32 not implemented [TA / PVR Core Reg], addr=%x,data=%x,size=%d",addr,data,sz);
		pvr_writereg_TA(addr,data,sz);
		return;
	}
	else if ((addr>= 0x00600000) && (addr<= 0x006007FF)) //	:MODEM
	{
		EMUERROR4("Write to area0_32 not implemented [MODEM], addr=%x,data=%x,size=%d",addr,data,sz);
	}
	else if ((addr>= 0x00600800) && (addr<= 0x006FFFFF)) //	:G2 (Reserved)
	{
		EMUERROR4("Write to area0_32 not implemented [G2 (Reserved)], addr=%x,data=%x,size=%d",addr,data,sz);
	}
	else if ((addr>= 0x00700000) && (addr<=0x00707FFF)) //	:AICA- Sound Cntr. Reg.
	{
		//EMUERROR4("Write to area0_32 not implemented [AICA- Sound Cntr. Reg], addr=%x,data=%x,size=%d",addr,data,sz);
		//aica_writereg(addr,data,sz);
		libAICA->aica_info.WriteMem_aica_reg(addr,data,sz);
		return;
	}
	else if ((addr>= 0x00710000) && (addr<= 0x0071000B)) //	:AICA- RTC Cntr. Reg.
	{
		//EMUERROR4("Write to area0_32 not implemented [AICA- RTC Cntr. Reg], addr=%x,data=%x,size=%d",addr,data,sz);
		WriteMem_aica_rtc(addr,data,sz);
		return;
	}
	else if ((addr>= 0x00800000) && (addr<=0x00FFFFFF)) //	:AICA- Wave Memory
	{
		//EMUERROR4("Write to area0_32 not implemented [AICA- Wave Memory], addr=%x,data=%x,size=%d",addr,data,sz);
		//aica_writeram(addr,data,sz);
		libAICA->aica_info.WriteMem_aica_ram(addr,data,sz);
		return;
	}
	else if ((addr>= 0x01000000) && (addr<= 0x01FFFFFF)) //	:Ext. Device
	{
		EMUERROR4("Write to area0_32 not implemented [Ext. Device], addr=%x,data=%x,size=%d",addr,data,sz);
	}
	else
	{
		EMUERROR4("Write to area0_32 not mapped!!! , addr=%x,data=%x,size=%d",addr,data,sz);
	}
	return;
}

//Init/Res/Term
void sh4_area0_Init()
{
	sb_Init();
}

void sh4_area0_Reset(bool Manual)
{
	sb_Reset(Manual);
}

void sh4_area0_Term()
{
	sb_Term();
}