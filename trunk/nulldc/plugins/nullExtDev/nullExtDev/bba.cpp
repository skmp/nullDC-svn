#include "bba.h"
#include "pcap_io.h"
#pragma pack(1)

char* chip_id="GAPSPCI_BRIDGE_2";

//By SDK
//map start = 0x840000
//map end = 0x847FFC
//This ram seems to be visible on the pci address space, however it is _not_ cotnained on the chip itself.
//The chip has 2 2k fifo buffers and then dmas to that ram (? o.O ?).From the driver sources it seems like we use g2dma from that ram to system (...)
u8 bba_ram[0x8000];

u8 bba_regs[0xFF] = {0x8F,0x7F,0x55,0x41,0x32,0x11,0}; //6 first bytes is MAC
#define reg8(v) (*(u8*)&bba_regs[v])
#define reg16(v) (*(u16*)&bba_regs[v])
#define reg32(v) (*(u32*)&bba_regs[v])
u32 bba_ram_base=0x840000;//isnt this .. configurable ?

#define bba_ram_addr_mask 0xFF8000
#define bba_ram_mask 0x7FFF

//BBA is typicaly configured @ 1700, so we assume that constant for now
//0xa1001700
u32 bba_devreadmem(u32 addr,u32 sz)
{
	verify(addr<=0xFF);
	if(addr!=0x3E)
	printf("bba_devreadmem(0x%X,0x%X)\n",addr,sz);

	u32 rv=0;
	memcpy(&rv,&bba_regs[addr],sz);
	if (RT_MII_BMSR==addr)
	{
		rv=RT_MII_100_FULL|RT_MII_10_FULL|RT_MII_AN_CAPABLE|RT_MII_AN_COMPLETE|RT_MII_LINK;
	}
	else if (RT_MEDIASTATUS==addr)
	{
		rv=(reg32(addr)&(~15))|16;
	}
	else if (addr==RT_CHIPCMD)
	{
		reg8(RT_CHIPCMD)|=1;
	}
	//maby we need to handle some special cases ?
	return rv;
}
void bba_doswreset()
{

}
void bba_interrupt()
{
	if (reg16(RT_INTRSTATUS) & reg16(RT_INTRMASK))
	{
		params.RaiseInterrupt(holly_EXP_PCI);
	}
	else
	{
		params.CancelInterrupt(holly_EXP_PCI);
	}
}
void bba_stabilise_link()
{
	//reg16(RT_)
	reg16(RT_INTRSTATUS)|=RT_INT_LINK_CHANGE;
	reg32(RT_MEDIASTATUS)=128|64;
	bba_interrupt();
}
void bba_devwritemem(u32 addr,u32 data,u32 sz)
{
	verify(addr<=0xFF);
	printf("bba_devwritemem(0x%X,0x%X,0x%X)\n",addr,data,sz);
	if (addr<6)
	{
		printf("TRIED WRITE TO MAC\n");
		return;
	}
	bool dc=true;
	u32 old;
	memcpy(&old,&bba_regs[addr],sz);
	memcpy(&bba_regs[addr],&data,sz);

	switch(addr)
	{

	case RT_CHIPCMD:
		{
			verify(sz==1);
			if (RT_CMD_RESET&data)
			{
				printf("Reset !\n");
				bba_doswreset();
			}
			if (RT_CMD_RX_BUF_EMPTY&data)
			{
				printf("RT_CMD_RX_BUF_EMPTY !\n");
			}
			if (RT_CMD_RX_ENABLE&data)
			{
				printf("RT_CMD_RX_ENABLE !\n");
			}
			if (RT_CMD_TX_ENABLE&data) 
			{
				printf("RT_CMD_TX_ENABLE !\n");
			}
			old=(data&~RT_CMD_RESET)|RT_CMD_RX_BUF_EMPTY;//done !
			dc=false;
		}
		break;
	case RT_TXSTATUS0:
	case RT_TXSTATUS0+4:
	case RT_TXSTATUS0+8:
		printf("TX request, size = %d\n",data);
		pcap_io_send(&bba_ram[reg32(addr+0x10)],data);
		reg32(addr)=RT_TX_HOST_OWNS | RT_TX_STATUS_OK;
		reg16(RT_INTRSTATUS)|=RT_INT_TX_OK;
		break;
	case RT_INTRMASK +1:
	case RT_INTRSTATUS+1:
		verify(0=="RT_INTMASK/RT_INTRSTATUS unaligned access\n");
		break;

	case RT_INTRSTATUS:
		verify(sz==2);
		dc=false;
		old&=~data;
		bba_interrupt();
		break;
	
	case RT_INTRMASK:
		verify(sz==2);
		//mask was updated, recheck
		bba_interrupt();
		break;
	case RT_RXCONFIG:
		/*
		if (data&0xa)
			SetUpdateCallback(bba_stabilise_link,120);
		*/
		break;
	case RT_MII_BMCR:
		{
			if (data & (RT_MII_RESET|RT_MII_AN_ENABLE|RT_MII_AN_START))
			{
				SetUpdateCallback(bba_stabilise_link,120);
			}
		}
		break;
	default:
		printf("WAS NOT HANDLED\n");
	}

	//restore old data :p
	if (!dc)
		memcpy(&bba_regs[addr],&old,sz);
}
//GAPS PCI emulation .. well more like checks to ensure nothing funny is writen :P
u32 bba_ReadMem(u32 addr,u32 sz)
{
	u32 rv=0;
	addr&=0xFFFFFF;
	if (addr>=0x1400 && addr<0x1416)
	{
		printf("bba_ReadMem(0x%X,0x%X)\n",addr,sz);
		memcpy(&rv,&chip_id[addr&0xF],sz);
	}
	else if ((addr&bba_ram_addr_mask)==bba_ram_base)
	{
		memcpy(&rv,&bba_ram[addr&bba_ram_mask],sz);
	}
	else if(addr<0x1700)
	{
		printf("bba_ReadMem(0x%X,0x%X)\n",addr,sz);
		switch(addr)
		{
		case 0x1418:
			return 1 ;
			break;
		case 0x141c:
			//verify(0);
			//SDK/PWB3 compares it with 1 .. so lets make em happy :P
			rv=1;
			break;
		default :
			rv=0xFFFFFFFF;
		}
	}
	else
		rv=bba_devreadmem(addr-0x1700,sz);
	
	return rv;
}
void bba_WriteMem(u32 addr,u32 data,u32 sz)
{
	addr&=0xFFFFFF;
	if ((addr&bba_ram_addr_mask)==bba_ram_base)
	{
		memcpy(&bba_ram[addr&bba_ram_mask],&data,sz);
	}
	else if (addr<0x1700)
	{
		printf("bba_WriteMem(0x%X,0x%X,0x%X)\n",addr,data,sz);
		switch(addr)
		{
		case 0x1414:
			if (data&1)
				printf("BBA: Interrupt Enabled !\n");
			else
				printf("BBA: Interrupt Disabled !\n");
			break;
		case 0x1418:
			warn(data!=0x5a14a501 && data!=0x5a14a500);
			if (data&1)
				printf("bba : Reset Released(?)\n");
			else
				printf("bba : Reset Held(?)\n");
			break;
		case 0x1420:
			warn(data!=0x01000000);
			break;
		case 0x1424:
			warn(data!=0x01000000);
			break;
		case 0x1428:
			warn((data&0xFFFFFF)!=bba_ram_base);
			break;		/* DMA Base */
		case 0x142c:
			warn((data&0xFFFFFF)!=(bba_ram_base + 32*1024));
			break;	/* DMA End */
		case 0x1434:
			warn(data!=0x00000001);
			break;

		case 0x1606:
			warn(data!=0xf900);
			break;
		case 0x1630:
			warn(data!=0x00000000);
			break;
		case 0x163c:
			warn(data!=0x00);
			break;
		case 0x160d:
			warn(data!=0xf0);
			break;

		case 0x1604:
			warn(data!=0x0006); //hmm, planet web writes 0x2 .. i wonder if it has any difference ?
			break;
		case 0x1614:
			warn(data!=0x01000000);
			break;

		}
	}	
	else
		bba_devwritemem(addr-0x1700,data,sz);
} 

void bba_periodical()
{
	if (reg16(RT_INTRSTATUS)&RT_INT_RX_OK)
		return;//a packet is allready pending
	int base=reg32(RT_RXBUFHEAD);
	int len=pcap_io_recv(&bba_ram[base+4],1520);
	if (len>0)
	{
		reg16(RT_INTRSTATUS)|=RT_INT_RX_OK;
		*(u32*)&bba_ram[base]=len;
		reg8(RT_CHIPCMD)&=0;
	}
}
/*

int pcap_io_init(char *adapter);
int pcap_io_send(void* packet, int plen);
int pcap_io_recv(void* packet, int max_len);
void pcap_io_close();

int pcap_io_get_dev_num();
char* pcap_io_get_dev_desc(int num);
char* pcap_io_get_dev_name(int num);

*/

void bba_init()
{
	memcpy(bba_regs,&virtual_mac,sizeof(virtual_mac));
}