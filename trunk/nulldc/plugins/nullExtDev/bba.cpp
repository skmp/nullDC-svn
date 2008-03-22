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
u8 GAPS_regs[0x200];//0x1400~0x1600 :)
u32 bba_ram_base=0x840000;//isnt this .. configurable ?
u32 bba_dma_base;//whats this ?

#define bba_ram_addr_mask 0xFF8000
#define bba_ram_mask 0x7FFF
#ifdef NQE
u8 bba_regs[0xFF] = {0x8F,0x7F,0x55,0x41,0x32,0x11,0}; //6 first bytes is MAC
#define reg8(v) (*(u8*)&bba_regs[v])
#define reg16(v) (*(u16*)&bba_regs[v])
#define reg32(v) (*(u32*)&bba_regs[v])
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
	static u32 sk=0;
	sk++;
	if (sk&0x1f)
		return;
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
#else
#include "rtl8139c.h"
PCIDevice* pcidev;
void* opaq;
bool inttr_pending=false;
bool GAPS_INTTRON=false;
void GAPS_Inttr()
{
	if (inttr_pending && GAPS_INTTRON)
	{
		params.RaiseInterrupt(holly_EXP_PCI);
	}
	else
	{
		params.CancelInterrupt(holly_EXP_PCI);
	}
}
static void pci_update_mappings(PCIDevice *d)
{
    PCIIORegion *r;
    int cmd, i;
    uint32_t last_addr, new_addr, config_ofs;

    cmd = le16_to_cpu(*(uint16_t *)(d->config + PCI_COMMAND));
    for(i = 0; i < PCI_NUM_REGIONS; i++) {
        r = &d->io_regions[i];
        if (i == PCI_ROM_SLOT) {
            config_ofs = 0x30;
        } else {
            config_ofs = 0x10 + i * 4;
        }
        if (r->size != 0) {
            if (r->type & PCI_ADDRESS_SPACE_IO) {
                if (cmd & PCI_COMMAND_IO) {
                    new_addr = le32_to_cpu(*(uint32_t *)(d->config +
                                                         config_ofs));
                    new_addr = new_addr & ~(r->size - 1);
                    last_addr = new_addr + r->size - 1;
                    /* NOTE: we have only 64K ioports on PC */
                    if (last_addr <= new_addr || new_addr == 0 ||
                        last_addr >= 0x10000) {
                        new_addr = -1;
                    }
                } else {
                    new_addr = -1;
                }
            } else {
                if (cmd & PCI_COMMAND_MEMORY) {
                    new_addr = le32_to_cpu(*(uint32_t *)(d->config +
                                                         config_ofs));
                    /* the ROM slot has a specific enable bit */
                    if (i == PCI_ROM_SLOT && !(new_addr & 1))
                        goto no_mem_map;
                    new_addr = new_addr & ~(r->size - 1);
                    last_addr = new_addr + r->size - 1;
                    /* NOTE: we do not support wrapping */
                    /* XXX: as we cannot support really dynamic
                       mappings, we handle specific values as invalid
                       mappings. */
                    if (last_addr <= new_addr || new_addr == 0 ||
                        last_addr == -1) {
                        new_addr = -1;
                    }
                } else {
                no_mem_map:
                    new_addr = -1;
                }
            }
            /* now do the real mapping */
            if (new_addr != r->addr) {
                if (r->addr != -1) {
                    if (r->type & PCI_ADDRESS_SPACE_IO) {
                        int clss;
                        /* NOTE: specific hack for IDE in PC case:
                           only one byte must be mapped. */
                        clss = d->config[0x0a] | (d->config[0x0b] << 8);
                        if (clss == 0x0101 && r->size == 4) {
                          //  isa_unassign_ioport(r->addr + 2, 1);
                        } else {
                          //  isa_unassign_ioport(r->addr, r->size);
                        }
                    } else {
                       // cpu_register_physical_memory(pci_to_cpu_addr(r->addr),
                       //                              r->size,
                       //                              IO_MEM_UNASSIGNED);
                    }
                }
                r->addr = new_addr;
                if (r->addr != -1) {
                   // r->map_func(d, i, r->addr, r->size, r->type);
					printf("new map: Addr 0x%X, size 0x%X, type %d\n",r->addr,r->size,r->type);
                }
            }
        }
    }
}

 void pci_default_write_config(PCIDevice *d,
                              uint32_t address, uint32_t val, int len)
{
    int can_write, i;
    uint32_t end, addr;

    if (len == 4 && ((address >= 0x10 && address < 0x10 + 4 * 6) ||
                     (address >= 0x30 && address < 0x34))) {
        PCIIORegion *r;
        int reg;

        if ( address >= 0x30 ) {
            reg = PCI_ROM_SLOT;
        }else{
            reg = (address - 0x10) >> 2;
        }
        r = &d->io_regions[reg];
        if (r->size == 0)
            goto default_config;
        /* compute the stored value */
        if (reg == PCI_ROM_SLOT) {
            /* keep ROM enable bit */
            val &= (~(r->size - 1)) | 1;
        } else {
            val &= ~(r->size - 1);
            val |= r->type;
        }
        *(uint32_t *)(d->config + address) = cpu_to_le32(val);
		pci_update_mappings(d);
        printf("pci_update_mappings(0)\n");
        return;
    }
 default_config:
    /* not efficient, but simple */
    addr = address;
    for(i = 0; i < len; i++) {
        /* default read/write accesses */
        switch(d->config[0x0e]) {
        case 0x00:
        case 0x80:
            switch(addr) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x08:
            case 0x09:
            case 0x0a:
            case 0x0b:
            case 0x0e:
			//case 0x10 ... 0x27: /* base */
#define x10 0x10
			case x10:case x10 + 1:case x10 + 2:case x10 + 3:case x10 + 4:case x10 + 5:case x10 + 6:case x10 + 7:
#define x10 0x18			
			case x10:case x10 + 1:case x10 + 2:case x10 + 3:case x10 + 4:case x10 + 5:case x10 + 6:case x10 + 7:
#undef x10
#define x10 0x20
			case x10:case x10 + 1:case x10 + 2:case x10 + 3:case x10 + 4:case x10 + 5:case x10 + 6:case x10 + 7:
#undef x10
			
			case 0x30://... 0x33: /* rom */
			case 0x31:
			case 0x32:
			case 0x33:
            case 0x3d:
                can_write = 0;
                break;
            default:
                can_write = 1;
                break;
            }
            break;
        default:
        case 0x01:
            switch(addr) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x08:
            case 0x09:
            case 0x0a:
            case 0x0b:
            case 0x0e:
			case 0x38:// ... 0x3b: /* rom */
				case 0x39:case 0x3a:case 0x3b:
            case 0x3d:
                can_write = 0;
                break;
            default:
                can_write = 1;
                break;
            }
            break;
        }
        if (can_write) {
            d->config[addr] = val;
        }
        if (++addr > 0xff)
        	break;
        val >>= 8;
    }

    end = address + len;
    if (end > PCI_COMMAND && address < (PCI_COMMAND + 2)) {
        /* if the command register is modified, we must modify the mappings */
        pci_update_mappings(d);
		printf("pci_update_mappings(1)\n");
    }
}


//GAPS PCI emulation .. well more like checks to ensure nothing funny is writen :P
u32 reg_0x141c_write,reg_0x141c_cnt;

u32 bba_ReadMem(u32 addr,u32 sz)
{
	u32 rv=0;
	addr&=0xFFFFFF;
	if (addr>=0x1400 && addr<0x1410)
	{
		memcpy(&rv,&chip_id[addr&0xF],sz);
	}
	else if ((addr&bba_ram_addr_mask)==bba_ram_base)
	{
		memcpy(&rv,&bba_ram[addr&bba_ram_mask],sz);
	}
	else if(addr<0x1700)
	{
		printf("bba_ReadMem(0x%X,0x%X)\n",addr,sz);
		if (addr>=0x1600)
		{
			memcpy(&rv,&pcidev->config[addr&0xFF],sz);
		}
		else
		{
			memcpy(&rv,&GAPS_regs[addr&0x1ff],sz);
			if (addr==0x1418)
				rv&=0xFF;
			else if (addr==0x141c)
			{
				if (reg_0x141c_cnt==0)
					rv=0x41474553;
				else if (reg_0x141c_cnt==1)
				{
					//rv=0xdeadc0de;
					rv=reg_0x141c_write;
				}
				else if (reg_0x141c_cnt==2)
				{
					//rv=0xdeadc0de;
					rv=reg_0x141c_write;
				}
				else
				{
					rv=reg_0x141c_write;
				}
			}
		}
	}
	else if (addr<0x1800)
	{
		if (sz==1)
			return rtl8139_mmio_readb(opaq,addr-0x1700);
		else if (sz==2)
			return rtl8139_mmio_readw(opaq,addr-0x1700);
		else
			return rtl8139_mmio_readl(opaq,addr-0x1700);
	}
	else
	{
		if ((addr&0xFF8000)==0x848000)
		{
		//	printf("Dma read ?\n");
			memcpy(&rv,&bba_ram[bba_dma_base-bba_ram_base+(addr&0x7FFF)],sz);
		}
		else
			printf("WTF IS THIS SUPOSED TO DO ?\n");
	}
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
		if (addr>=0x1600)
		{
			pci_default_write_config(pcidev,addr&0xFF,data,sz);
		}
		else
		{
			memcpy(&GAPS_regs[addr&0x1ff],&data,sz);
			switch(addr)
			{
			case 0x1414:
				if (data)
					GAPS_INTTRON=true;
				else
					GAPS_INTTRON=false;

				GAPS_Inttr();
				break;
			case 0x1418:
				warn(data!=0x5a14a501 && data!=0x5a14a500);
				if (data&1)
				{
					printf("bba : Reset Released(?)\n");
					rtl8139_resete(opaq);
				}
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
				//warn((data&0xFFFFFF)!=(bba_ram_base + 32*1024));
				bba_dma_base=data&0xFFFFFF;
				printf("DMA BASE 0x%X - 0x%X\n",bba_dma_base,bba_dma_base-bba_ram_base);
				break;	/* DMA End */
			case 0x1434:
				warn(data!=0x00000001);
				break;
			case 0x141c:
				reg_0x141c_write=data;
				reg_0x141c_cnt++;
				if (reg_0x141c_write==0x41474553)
				{
					//	inttr_pending=1;
					//	GAPS_Inttr();
				}
				break;
			}
			
		}
	}	
	else if (addr<0x1800)
	{
		if (sz==1)
			rtl8139_mmio_writeb(opaq,addr-0x1700,data);
		else if (sz==2)
			rtl8139_mmio_writew(opaq,addr-0x1700,data);
		else
			rtl8139_mmio_writel(opaq,addr-0x1700,data);

	}
	else
	{
		if ((addr&0xFF8000)==0x848000)
		{
		//	printf("Dma write ?\n");
			memcpy(&bba_ram[bba_dma_base-bba_ram_base+(addr&0x7FFF)],&data,sz);
		}
		else
			printf("WTF IS THIS SUPOSED TO DO ?\n");
	}
} 
net_receive* nrc;
net_can_receive* ncrc;
void ExaminePacket(const u8* p,u32 proto)
{
	if (proto==0x0800) //IP ?
	{
		const ip_header* ip=(const ip_header*)(p+14);
		printf("SRC : %d.%d.%d.%d, DST : %d.%d.%d.%d Len : %d\n",ip->ip_src.bytes[0],ip->ip_src.bytes[1],ip->ip_src.bytes[2],ip->ip_src.bytes[3]
																,ip->ip_dst.bytes[0],ip->ip_dst.bytes[1],ip->ip_dst.bytes[2],ip->ip_dst.bytes[3]
																,ip->ip_len/256 + (u8)ip->ip_len*256);
	}
}
void bba_periodical()
{
	while(ncrc(opaq))
	{
		u8 p[1520];
		int len=pcap_io_recv(p,1520);
		if (len>0)
		{
			u32 fromaddr=nrc(opaq,p,len);
			printf("Got Packet to %02X:%02X:%02X:%02X:%02X:%02X from %02X:%02X:%02X:%02X:%02X:%02X, proto 0x%04X, to addr 0x%04X sz %d bytes\n",
				p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12]*256+p[13],fromaddr,len);
			ExaminePacket(p,p[12]*256+p[13]);
		}
		else
			break;
	}
}

NICInfo nd;
void bba_init()
{
	memcpy(nd.macaddr,&virtual_mac,sizeof(virtual_mac));
	opaq=0;//(((u8*)pcidev)+sizeof(PCIDevice));
	pcidev=pci_rtl8139_init(0,&nd,0);
}

void qemu_send_packet(VLANClientState*,const uint8_t* p, int sz,u32 fromaddr)
{
	pcap_io_send((void*)p,sz);
	printf("Sent Packet to %02X:%02X:%02X:%02X:%02X:%02X from %02X:%02X:%02X:%02X:%02X:%02X, proto 0x%04X, from addr 0x%04X sz %d bytes\n",
		p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12]*256+p[13],fromaddr,sz);
	ExaminePacket(p,p[12]*256+p[13]);
}

VLANClientState *qemu_new_vlan_client(void* ,net_receive* rc,net_can_receive* cr,void* opa)
{
	opaq=opa;
	nrc=rc;
	ncrc=cr;
	return 0;
}

void cpu_physical_memory_write(u32 addr,const void* buff,u32 sz)
{
	memcpy(&bba_ram[addr&bba_ram_mask],buff,sz);
	//printf("cpu_physical_memory_write 0x%X 0x%X %d\n",addr&bba_ram_mask,buff,sz);
}
void cpu_physical_memory_read(u32 addr,void* buff,u32 sz)
{
	memcpy(buff,&bba_ram[addr&bba_ram_mask],sz);
	//printf("cpu_physical_memory_read 0x%X 0x%X %d\n",addr&bba_ram_mask,buff,sz);
}
void pci_set_irq(PCIDevice* dev,u32 duno,u32 status)
{
	//printf("IRQ SET STATUS %d\n",status);
	inttr_pending=status;
	GAPS_Inttr();
}
 PCIDevice *pci_register_device(PCIBus *bus, const char *name, 
                                int instance_size, int devfn, 
                                PCIConfigReadFunc *config_read, 
                                PCIConfigWriteFunc *config_write)
 {
	 void* rvr=malloc(instance_size);
	 memset(rvr,0,instance_size);
	 PCIDevice *rv=( PCIDevice *)rvr;
	return rv;
 }

 void pci_register_io_region(PCIDevice *pci_dev, int region_num,
                            uint32_t size, int type,
                            PCIMapIORegionFunc *map_func)
{
    PCIIORegion *r;
    uint32_t addr;

    if ((unsigned int)region_num >= PCI_NUM_REGIONS)
        return;
    r = &pci_dev->io_regions[region_num];
    r->addr = -1;
    r->size = size;
    r->type = type;
    r->map_func = map_func;
    if (region_num == PCI_ROM_SLOT) {
        addr = 0x30;
    } else {
        addr = 0x10 + region_num * 4;
    }
    *(uint32_t *)(pci_dev->config + addr) = cpu_to_le32(type);
}

#endif