#include "types.h"
#include <string.h>

#include "maple_if.h"

#include "config/config.h"

#include "dc/sh4/intc.h"
#include "dc/mem/sb.h"
#include "dc/mem/sh4_mem.h"
#include "plugins/plugin_manager.h"
#include "dc/sh4/rec_v1/blockmanager.h"
#include "dc/asic/asic.h"

eMDevInf MapleDevices_dd[4][6];
maple_device_instance MapleDevices[4];

#ifdef BUILD_NAOMI
void naomi_InitMaple(void);
void naomi_WriteMem_maple(u32 Addr,u32 data,u32 sz);
#endif
/*
	Maple ;)
	Maple IO is done on fames on both ways, in a very strict way

	Frame structure :

	input/output buffer:
	->buffer start<-
	Header
	Frame data
	Header
	Frame data
	Last Header [End bit set]
	Frame data
	....
	->buffer end<-

	Transfer info:
	Transfers are done w/ dma.
	Maple is a bidirectional bus , each device has an address

	Address format :
	7-6         |5				 |4			    |3			   |2			 |1				 |0				
	Port number |Main peripheral |Sub-periph. 5 |Sub-periph. 4 |Sub-periph. 3| Sub-periph. 2 |Sub-periph. 1 
	
	if bits 5-0 are 0 , the device is the port
	afaik , olny one or none of bits 5 to 0 can be set when a command is send
	on a resonce , the controller sets the bits of the connected subdevices to it :)

	Now , how to warp all that on a nice to use interface :
	Each dll can contain 1 maple device , and 1 maple subdevice (since they are considered diferent plugin types)
	Maple plugins should olny care about "user data" on maple frames , all else is handled by maple rooting code
*/

/*
typedef void MapleGotData(u32 header1,u32 header2,u32*data,u32 datalen);

struct MaplePluginInfo
{
	u32 InterfaceVersion;

	//UpdateCBFP* UpdateMaple;
	bool Connected;//:)
	MapleGotData* GotDataCB;
	InitFP* Init;
	TermFP* Term;
};*/

//MaplePluginInfo MaplePlugin[4][6];

void DoMapleDma();


void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen);

//realy hackish
//misses delay , and stop/start implementation
bool maple_ddt_pending_reset=false;
void maple_vblank()
{
	if (SB_MDEN &1)
	{
		if (SB_MDTSEL&1)
		{
			if (maple_ddt_pending_reset)
			{
				//printf("DDT vblank ; reset pending\n");
			}
			else
			{
				//printf("DDT vblank\n");
				DoMapleDma();
				SB_MDST = 0;
				if ((SB_MSYS>>12)&1)
				{
					maple_ddt_pending_reset=true;
				}
			}
		}
		else
		{
			maple_ddt_pending_reset=false;
		}
	}
}
void maple_SB_MSHTCL_Write(u32 data)
{
	if (data&1)
		maple_ddt_pending_reset=false;
}
void maple_SB_MDST_Write(u32 data)
{
	#ifdef BUILD_NAOMI
	naomi_WriteMem_maple(SB_MDST_addr,data,4);
	#else
	if (data & 0x1)
	{
		if (SB_MDEN &1)
		{
			//SB_MDST=1;
			DoMapleDma();
		}
	}
	SB_MDST = 0;	//No dma in progress :)
	#endif
}

bool IsOnSh4Ram(u32 addr)
{
	if (((addr>>26)&0x7)==3)
	{
		if ((((addr>>29) &0x7)!=7))
		{
			return true;
		}
	}

	return false;
}
u32 GetMaplePort(u32 addr)
{
	for (int i=0;i<6;i++)
	{
		if ((1<<i)&addr)
			return i;
	}
	return 0;
}
u32 GetConnectedDevices(u32 Port)
{
	verify(MapleDevices[Port].connected);

	u32 rv=0;
	
	if(MapleDevices[Port].subdevices[0].connected)
		rv|=0x01;
	if(MapleDevices[Port].subdevices[1].connected)
		rv|=0x02;
	if(MapleDevices[Port].subdevices[2].connected)
		rv|=0x04;
	if(MapleDevices[Port].subdevices[3].connected)
		rv|=0x08;
	if(MapleDevices[Port].subdevices[4].connected)
		rv|=0x10;

	return rv;
}
u32 dmacount=0;
void DoMapleDma()
{
	verify(SB_MDEN &1)
#if debug_maple
	printf("Maple :DoMapleDma\n");
#endif
	u32 addr = SB_MDSTAR;
	bool last = false;
	while (last != true)
	{
		dmacount++;
		u32 header_1 = ReadMem32_nommu(addr);
		u32 header_2 = ReadMem32_nommu(addr + 4) &0x1FFFFFE0;

		last = (header_1 >> 31) == 1;//is last transfer ?
		u32 plen = (header_1 & 0xFF )+1;//transfer lenght
		u32 device = (header_1 >> 16) & 0x3;
		u32 maple_op=(header_1>>8)&7;

		if (maple_op==0)
		{
			if (!IsOnSh4Ram(header_2))
			{
				printf("MAPLE ERROR : DESTINATION NOT ON SH4 RAM 0x%X\n",header_2);
				header_2&=0xFFFFFF;
				header_2|=(3<<26);
				//goto dma_end;//a baaddd error
			}
			u32* p_out=(u32*)GetMemPtr(header_2,4);
			u32 outlen=0;

			u32* p_data =(u32*) GetMemPtr(addr + 8,(plen)*sizeof(u32));
			//Command / Response code 
			//Recipient address 
			//Sender address 
			//Number of additional words in frame 
			u32 command=p_data[0] &0xFF;
			u32 reci=(p_data[0] >> 8) & 0xFF;//0-5;
			u32 subport=GetMaplePort(reci);
			u32 wtfport=reci>>6;
			u32 send=(p_data[0] >> 16) & 0xFF;
			u32 inlen=(p_data[0]>>24) & 0xFF;
			u32 resp=0;
			inlen*=4;
			//device=wtfport;

			if (MapleDevices[device].connected && (subport==5 || MapleDevices[device].subdevices[subport].connected))
			{
				//(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce);
				if (subport==5)
				{
					MapleDevices[device].dma(
						&MapleDevices[device],
						command,
						&p_data[1],
						inlen,
						&p_out[1],
						outlen,
						resp);
				}
				else
				{
					MapleDevices[device].subdevices[subport].dma(
						&MapleDevices[device].subdevices[subport],
						command,
						&p_data[1],
						inlen,
						&p_out[1],
						outlen,
						resp);
				}


				if(reci&0x20)
					reci|=GetConnectedDevices(device);

				p_out[0]=(resp<<0)|(send<<8)|(reci<<16)|((outlen/4)<<24);
				outlen+=4;
			}
			else
			{
				outlen=4;
				p_out[0]=0xFFFFFFFF;
			}
			//NotifyMemWrite(header_2,outlen);

			//goto next command
			addr += 2 * 4 + plen * 4;
		}
		else
		{
			addr += 1 * 4;
		}
	}
//dma_end:
	asic_RaiseInterrupt(holly_MAPLE_DMA);
}

//device : 0 .. 4 -> subdevice , 5 -> main device :)
u32 GetMapleAddress(u32 port,u32 device)
{
	u32 rv=port<<6;
	/*if (device==0)
		device=5;
	else
		device-=1;*/
	rv|=1<<device;

	return rv;
}

//plugins are handled from plugin manager code from now :)
//Init registers :)
void maple_Init()
{
	sb_regs[(SB_MDST_addr-SB_BASE)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[(SB_MDST_addr-SB_BASE)>>2].writeFunction=maple_SB_MDST_Write;

	sb_regs[(SB_MSHTCL_addr-SB_BASE)>>2].flags=REG_32BIT_READWRITE;
	sb_regs[(SB_MSHTCL_addr-SB_BASE)>>2].writeFunction=maple_SB_MSHTCL_Write;

	#ifdef BUILD_NAOMI
	naomi_InitMaple();
	#endif
	
}

void maple_Reset(bool Manual)
{
}

void maple_Term()
{
	
}

#ifdef BUILD_NAOMI

#include <windows.h>
#include <string.h>


/*	Notes
*
If no Device status request is made, the peripherals do not start to operate, but remain in standby status

One peripheral can use a maximum of three functions. Functions are accessed by specifying their respective function type.
One function can be accessed a maximum of one time during one INT, and one frame of data can be sent per access.
One port cannot be continually accessed. (If continuous access in unavoidable, use the NOP instruction.)

Transmission data (including the data format, etc.) is stored in 4 byte units.

(2) Configuration of AP bits
These are configured in one byte.
Bit	7	6	5	4	3	2	1	0
Data	PO1	PO0	D/E	LM4	LM3	LM2	LM1	LM0

When the device responds, the origin AP at the time becomes the value of the sum (all OR)
of the device's own AP and the connected expansion devices' APs.

	bit		7		6		5		4		3		2		1		0
1st Data	FT31	FT30	FT29	FT28	FT27	FT26	FT25	FT24
2nd Data	FT23	FT22	FT21	FT20	FT19	FT18	FT17	FT16
3rd Data	FT15	FT14	FT13	FT12	FT11	FT10	FT9		FT8
4th Data	FT7		FT6		FT5		FT4		FT3		FT2		FT1		FT0
5th Data	FD131	FD130	FD129	FD128	FD127	FD126	FD125	FD124
6th Data	FD123	FD122	FD121	FD120	FD119	FD118	FD117	FD116
7th Data	FD115	FD114	FD113	FD112	FD111	FD110	FD19	FD18
8th Data	FD17	FD16	FD15	FD14	FD13	FD12	FD11	FD10
9th Data	FD231	FD230	FD229	FD228	FD227	FD226	FD225	FD224
10th Data	FD223	FD222	FD221	FD220	FD219	FD218	FD217	FD216
11th Data	FD215	FD214	FD213	FD212	FD211	FD210	FD29	FD28
12th Data	FD27	FD26	FD25	FD24	FD23	FD22	FD21	FD20
13th Data	FD3231	FD330	FD329	FD328	FD327	FD326	FD325	FD324
14th Data	FD323	FD322	FD321	FD320	FD319	FD318	FD317	FD316
15th Data	FD315	FD314	FD313	FD312	FD311	FD310	FD39	FD38
16th Data	FD37	FD36	FD35	FD34	FD33	FD32	FD31	FD30

FT		:Designates type of function that the peripheral is equipped with.
FD1		:Designates the function definition block of the first function.
FD2		:Designates the function definition block of the second function.
FD3		:Designates the function definition block of the third function.

The contents of FD1, FD2, and FD3 vary depending on the function designated by FT.


*
*/

#define AP_LM_0		0x01	// LM Bus #
#define AP_LM_1		0x02
#define AP_LM_2		0x04
#define AP_LM_3		0x08
#define AP_LM_4		0x10

#define AP_DE		0x20	// Device / Expansion

#define AP_PORT_A	0x00	// Port
#define AP_PORT_B	0x40
#define AP_PORT_C	0x80
#define AP_PORT_D	0xC0

#define AP_DEV_A	0x20	// Main Device on Port
#define AP_DEV_B	0x60
#define AP_DEV_C	0xA0
#define AP_DEV_D	0xE0

#define MAPLE_DEV(port)		((port)|AP_DE)	// shouldn't been needed use AP_DEV_*
#define MAPLE_EXP(exp,lm)	((exp)|(lm))	// 

// Start Pattern
// Cmd Code

// DestAP/OrigAP

// DataSize
#define DATASIZE(size) ((u32)((size)<<2))

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////
#define MAPLE_DEV_REQ		0x01	// Host
#define MAPLE_DEV_REQALL	0x02	// Host
#define MAPLE_DEV_RESET		0x03	// Host
#define MAPLE_DEV_KILL		0x04	// Host

#define MAPLE_DEV_STATUS	0x05	// Peripheral
#define MAPLE_DEV_STATUSALL	0x06	// Peripheral
#define MAPLE_DEV_REPLY		0x07	// Peripheral
#define MAPLE_DATA_TRANSFER	0x08	// Peripheral

#define MAPLE_GET_CONDITION	0x09	// Host
#define MAPLE_GET_MEDIAINFO	0x0A	// Host
#define MAPLE_BLOCK_READ	0x0B	// Host
#define MAPLE_BLOCK_WRITE	0x0C	// Host
#define MAPLE_GET_LAST_ERR	0x0D	// Host
#define MAPLE_SET_CONDITION	0x0E	// Host
#define MAPLE_FT_CONTROL	0x0F	// Host
#define MAPLE_AR_CONTROL	0x10	// Host

#define MAPLE_ERR_FT_UNK	0xFE	// Peripheral
#define MAPLE_ERR_CMD_UNK	0xFD	// Peripheral
#define MAPLE_TRANS_AGAIN	0xFC	// Host/Peripheral
#define MAPLE_FILE_ERROR	0xFB	// Peripheral
#define MAPLE_LCD_ERROR		0xFA	// Peripheral
#define MAPLE_AR_ERROR		0xF9	// Peripheral

///////////////////////////////////////////////////////




/*A command file is set up in system memory, containing the instructions (settings like communications port selection,
the received data storage address, and the transfer data length) for the Maple controller and the transmission data.
The command file consists of units formed by "instruction to the controller," "received data storage address,"
and "transmission data," in that order.  Each of these units are located consecutively in system memory.
*/

typedef struct sMapleInstruction
{
	u32 DataLen	: 8;	// Send data length selection bits
	u32 Pattern	: 3;	// Pattern selection bits
	u32 _Res0	: 5;
	u32 PortSel	: 2;	// Port selection bits
	u32 _Res1	:13;
	u32 EndFlag	: 1;	// Last instruction bit

} MapleInstr, MapleInstruction ;

// fix this later .. 

typedef struct sMapleFrame
{
	// Start Data Pattern
	u8 CmdCode;
	u8 DestAP;
	u8 OrigAP;
	u8 DataLen;

} MapleFrame, MapleFrame;

typedef struct sMapleEndPatttern
{
	u8 Parity;
	u8 EndPattern;

} MapleEnd, MapleEndPattern;

/*	this should be wrong ??
*/
typedef struct sMapleTransfer
{
	MapleInstr Instr;

	u32 Addr;

	MapleFrame Frame;

	u32 Data[256];

	// MapleEnd


} MapleTrans, MapleTransfer ;


////////////////////////////////////////////////////

typedef struct sDeviceID
{
	u32 FT;
	u32 FD[3];

} DeviceID;

#define FT_CONTROLLER	0x01000000
#define FT_STORAGE		0x02000000
#define FT_BW_LCD		0x04000000
#define FT_TIMER		0x08000000
#define FT_AUDIO_IN		0x10000000
#define FT_AR_GUN		0x20000000
#define FT_KEYBOARD		0x40000000
#define FT_RESERVED		0x00000000

#define FD_CONTROLLER	0x000F06FE

// Fixed Device Status 112bytes

typedef struct sFixedDevStatus
{
	DeviceID DevID;			// 16B Device ID

	u8	DestCode;			// 1B Destination Code
	u8	Direction;			// 1B Connection Direction of Expansion Socket [SD4][SD3][SD2][SD1] 2b

	u8	ProductName[30];	// Product Name 0x20 is space
	u8	License[60];		// License ...

	u16	StandbyCurrent;		// in units of 0.1 mA - 10.5 mA is designated by 00-69h.
	u16	MaximumCurrent;		// in units of 0.1 mA - 127.9 mA is designated by 04-FFh.

} FixedDevStatus, fxDevStatus ;

// items should be separeted by a ',' spaces are 0x20 


typedef struct sFreeDevStatus
{
	// first 40 bytes should conform
// "Version 1.000,1998/05/11,315-6125-AB". 
// "Analog Module: The 4th Edition. 05/08"
	u8 Version[14];			// 13->14 including comma
	u8 ReleaseDate[11];		// 10->11 including comma
	u8 IC_PartNo[14];		// 14->15 including comma ?

} FreeDevStatus, frDevStatus ;

// Destination Codes
#define DEST_NORTH_AMERICA	0x01
#define DEST_JAPAN			0x02
#define DEST_ASIA			0x04
#define DEST_EUROPE			0x08
#define DEST_WORLDWIDE		0xFF

// Exp. Port Direction Codes
#define DIR_TOP				0x00
#define DIR_BOTTOM			0x01
#define DIR_LEFT			0x02
#define DIR_RIGHT			0x03



///////////////////////////////////////////////////////////////////////////////////////



void naomi_InitMaple(void)
{ 
	SB_MDTSEL	= 0x00000000;
	SB_MDEN	= 0x00000000;
	SB_MDST	= 0x00000000;
	SB_MSYS	= 0x3A980000;
	SB_MSHTCL	= 0x00000000;
	SB_MDAPRO = 0x00007F00;
	SB_MMSEL	= 0x00000001;

	printf("(N) Maple/JVS2 Initialized !\n");
}


#define MAPLE_E_TIMEOUT	0xFFFFFFFF
#define MAPLE_E_PARITY	0xFFFFFF00


void MapleDevReq( MapleTrans * pTrans );
void MapleDevReset( MapleTrans * pTrans );

void JvsGetID(MapleTrans * pTrans);
void JvsDoShit(MapleTrans * pTrans);


void naomi_WriteMem_maple(u32 Addr,u32 data,u32 sz)
{
	u32 Offs = 0;
	MapleTransfer * pmtr;

	if(sz!=4) {	// complain (#ifdef _DEBUG ?)
		printf("\n!\tMAPLE: WriteMem sz:%X Invalid!\n\n",sz);
	}

//	SetMLReg(Addr,data);

	// ch0 DDT ONLY
	if(SB_MDST_addr == Addr) {
		if ((data & 1) && (SB_MDEN & 1))
		{
			do
			{
				pmtr = (MapleTransfer *)(mem_b.data + (SB_MDSTAR &RAM_MASK) + Offs);
				
				Offs += ((pmtr->Instr.DataLen + 1) << 2) + 8;

				if(pmtr->Instr.Pattern == 0)
				{
					switch(pmtr->Frame.CmdCode)
					{

						// JVS (JAMMA2) USB Interface
					case 0x80:					// ???
						printf("(N) Unknown JVS Frame!\n"); break;

					case 0x82:					// JVS2_GETID
						JvsGetID(pmtr);			break;

					case 0x86:					// JVS2_GETID
						JvsDoShit(pmtr);		break;



					case MAPLE_DEV_REQ:			// Host
						MapleDevReq(pmtr);		break;

					case MAPLE_DEV_RESET:		// Host 3
						MapleDevReset(pmtr);	break;
						break;

					case MAPLE_DEV_REQALL:		// Host 2
					case MAPLE_DEV_KILL:		// Host 4

					case MAPLE_DEV_STATUS:		// Peripheral
					case MAPLE_DEV_STATUSALL:	// Peripheral
					case MAPLE_DEV_REPLY:		// Peripheral
					case MAPLE_DATA_TRANSFER:	// Peripheral

					case MAPLE_GET_CONDITION:	// Host
					case MAPLE_GET_MEDIAINFO:	// Host
					case MAPLE_BLOCK_READ:		// Host
					case MAPLE_BLOCK_WRITE:		// Host
					case MAPLE_GET_LAST_ERR:	// Host
					case MAPLE_SET_CONDITION:	// Host
					case MAPLE_FT_CONTROL:		// Host
					case MAPLE_AR_CONTROL:		// Host

					case MAPLE_ERR_FT_UNK:		// Peripheral
					case MAPLE_ERR_CMD_UNK:		// Peripheral
					case MAPLE_TRANS_AGAIN:		// Host/Peripheral
					case MAPLE_FILE_ERROR:		// Peripheral
					case MAPLE_LCD_ERROR:		// Peripheral
					case MAPLE_AR_ERROR:		// Peripheral



					default: printf("Unk. (N) Maple CC: %X \n", pmtr->Frame.CmdCode); break;
					}
				}
				else
					if(pmtr->Instr.Pattern != 3)
						printf("(N) Maple Unhandled Pattern !\n");

			} while(!pmtr->Instr.EndFlag);
		}

		SB_MDST = 0;					// DMA Finished
		RaiseInterrupt(holly_MAPLE_DMA);	// ""
	}
//	else
//		printf("\n§\tMAPLE (NAOMI): WriteMem %X <= %X sz:%X \n\n",Addr,data,sz);
}



////////////////////////////////////////////////////////////////////////////////////////////


// well its not complaining afaict but dont know if this is effective ..

void JvsGetID(MapleTrans * pTrans)
{
	const char *szJvsID = "315-6149    COPYRIGHT SEGA E\x83\x00\x20\x05NTERPRISES CO,LTD.  ";	// thx chanka src

	MapleFrame * pFrame = (MapleFrame *)(mem_b.data + (pTrans->Addr &0xFFFFFF));

	pFrame->CmdCode	= MAPLE_DEV_STATUS;
	pFrame->DestAP	= pTrans->Frame.OrigAP;
	pFrame->OrigAP	= pTrans->Frame.DestAP;
	pFrame->DataLen	= 0x7;	// 0x1C << 2 = 0x70 (112B)

	memcpy(&pTrans->Data[0],szJvsID,strlen(szJvsID));
}

struct
{
	u8 Cmd;
	u8 Mode;
	u8 Node;

} State;

void JvsDoShit(MapleTrans * pTrans)
{
/*	FILE * f = fopen("JvsDev.txt","at");
	fprintf(f,
		"MapleDevReq(%x)\n{\n"
		"\tInstr: Ln:%x Pa:%x Po:%x Ef:%x \n"
		"\tFrame: CC:%x Len:%x Dst:%x Orig:%x \n}\n\n"
		, pTrans->Addr
		, pTrans->Instr.DataLen, pTrans->Instr.Pattern, pTrans->Instr.PortSel, pTrans->Instr.EndFlag
		, pTrans->Frame.CmdCode, pTrans->Frame.DataLen, pTrans->Frame.DestAP, pTrans->Frame.OrigAP	);

	fprintf(f, "JVS Data: %X %X %X %X %X \n\n\n",
		pTrans->Data[0], pTrans->Data[1], pTrans->Data[2], pTrans->Data[3], pTrans->Data[4] );
	fclose(f);*/
	///////////////////////////////////////////////////////////////////////////////////////////////////
	MapleFrame * pFrame = (MapleFrame *)(mem_b.data + (pTrans->Addr &0xFFFFFF));

	pFrame->CmdCode	= MAPLE_DEV_STATUS;
	pFrame->DestAP	= pTrans->Frame.OrigAP;
	pFrame->OrigAP	= pTrans->Frame.DestAP;
	pFrame->DataLen	= 0x00;


	u8 * InData = ((u8*)pTrans->Data);	// chanka prob. uses separate buffers for these
	u8 * OutData = ((u8*)pTrans->Data);	// so watch out for spots where Out is written before in read


	switch(((u8*)pTrans->Data)[0])
	{

	case 0x0b:		//EEprom access (Writting)
	/*	{
			int address=InData[1];
			int size=InData[2];
			memcpy(EEprom+address,InData+4,size);
			//return MAPLE_RESPONSE_OK;
		}*/
	break;

	case 0x17:	// Select subdevice
		{
			State.Mode	= 0;
			State.Cmd	= InData[8];
			State.Node	= InData[9];
		}
	break;

	case 0x27:
		{
			State.Mode	= 1;
			State.Cmd	= InData[8];
			State.Node	= InData[9];
		}
	break;
	
	case 0x31:		//IF I return all FF, then board runs in low res
		{
			pTrans->Data[0] = 0xFFFFFFFF;
			pTrans->Data[1] = 0xFFFFFFFF;
		}
	break;


	case 0x15:
		
		pTrans->Data[0] = 0xFFFFFFFF;
		pTrans->Data[1] = 0xFFFFFFFF;

		if(GetKeyState(VK_F1)&0x8000)		//Service
			pTrans->Data[0] &= ~(1<<0x1B);

		if(GetKeyState(VK_F2)&0x8000)		//Test
			pTrans->Data[0] &= ~(1<<0x1A);

		// pretend state mode = 0

		pFrame->DataLen	= 0x00;

		OutData[0x11+1]=0x8E;	//Valid data check
		OutData[0x11+2]=0x01;
		OutData[0x11+3]=0x00;
		OutData[0x11+4]=0xFF;
		OutData[0x11+5]=0xE0;
		OutData[0x11+8]=0x01;

		switch(State.Cmd)
		{
		case 0xF1:
			pFrame->DataLen=4;
		break;

		case 0x10:
			{
				static char ID1[32]="JAMMA I/O CONTROLLER";
				OutData[0x8+0x10]=(BYTE)strlen(ID1)+3;
				for(int i=0;i<0x20;++i)
				{
					OutData[0x8+0x13+i]=ID1[i];
				}
			}
		break;
		
		case 0x11:
			{
				OutData[0x8+0x13]=0x11;	//CMD Version
			}
		break;
		case 0x12:
			{
				OutData[0x8+0x13]=0x12;	//JVS Version
			}
		break;

		case 0x13:
			{
				OutData[0x8+0x13]=0x13;	//COM Version
			}
		break;

		case 0x14:
			{		//Features
				unsigned char *FeatPtr=OutData+0x8+0x13;
				OutData[0x8+0x9+0x3]=0x0;
				OutData[0x8+0x9+0x9]=0x1;
				#define ADDFEAT(Feature,Count1,Count2,Count3)	*FeatPtr++=Feature; *FeatPtr++=Count1; *FeatPtr++=Count2; *FeatPtr++=Count3;
				ADDFEAT(1,2,10,0);	//Feat 1=Digital Inputs.  2 Players. 10 bits
				ADDFEAT(2,2,0,0);	//Feat 2=Coin inputs. 2 Inputs
				ADDFEAT(3,2,0,0);	//Feat 3=Analog. 2 Chans

				ADDFEAT(0,0,0,0);	//End of list
			}
		break;
					// dont know why this was here if these are all covered
					//	else if(State.Cmd>=0x10 && State.Cmd<=0x14)
					//		memset(OutData+0x0,State.Cmd+0x20,0x200);

		default: printf("(N) UNKNOWN JVS State.Cmd: %X !\n", State.Cmd); break;
		}

	break;


	default: printf("Unknown JVS Shitte : %X\n", ((u8*)pTrans->Data)[0]); break;
	}
}

void MapleDevReq( MapleTrans * pTrans )
{
/*	FILE * f = fopen("MplDev.txt","at");
	fprintf(f,
		"MapleDevReq(%x)\n{\n"
		"\tInstr: Ln:%x Pa:%x Po:%x Ef:%x \n"
		"\tFrame: CC:%x Len:%x Dst:%x Orig:%x \n}\n\n"
		, pTrans->Addr
		, pTrans->Instr.DataLen, pTrans->Instr.Pattern, pTrans->Instr.PortSel, pTrans->Instr.EndFlag
		, pTrans->Frame.CmdCode, pTrans->Frame.DataLen, pTrans->Frame.DestAP, pTrans->Frame.OrigAP	);
	fclose(f);	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////

	MapleFrame * pFrame = (MapleFrame *)(mem_b.data + (pTrans->Addr &0xFFFFFF));
	fxDevStatus * pStatus = (fxDevStatus *)(mem_b.data + (pTrans->Addr &0xFFFFFF) + 4);

	// this is broke, not 100% why
	if( pTrans->Frame.DestAP &AP_DE )
	{
		pFrame->CmdCode	= MAPLE_DEV_STATUS;
		pFrame->DestAP	= pTrans->Frame.OrigAP;
		pFrame->OrigAP	= pTrans->Frame.DestAP;
		pFrame->DataLen	= 0x1C;	// 0x1C << 2 = 0x70 (112B)

		// This is for a controller only atm
		pStatus->DevID.FD[2]	= pStatus->DevID.FD[1] = 0;
		pStatus->DevID.FD[0]	= FD_CONTROLLER;		// *FIXME*
		pStatus->DevID.FT		= FT_CONTROLLER;		// FT_STORAGE
		pStatus->DestCode		= DEST_WORLDWIDE;

		pStatus->Direction		= DIR_TOP;
		pStatus->StandbyCurrent	= 0x01AE;	// 1AE = 43 mA |(O) 0x0069 = 10.5mA
		pStatus->MaximumCurrent	= 0x01F4;	// 1F4 = 50 mA |(O) 0x04FF = 127.9mA
		
		strcpy((char*)pStatus->ProductName,"Dreamcast Controller         ");
		strcpy((char*)pStatus->License,"Produced By or Under License From SEGA ENTERPRISES,LTD.    ");

	} else
	{
		pFrame->CmdCode	= MAPLE_DEV_STATUS;
		pFrame->DestAP	= pTrans->Frame.OrigAP;
		pFrame->OrigAP	= pTrans->Frame.DestAP;
		pFrame->DataLen	= 0;//0x1C;	// 0x1C << 2 = 0x70 (112B)

		pStatus->DevID.FD[2]	= pStatus->DevID.FD[1] = pStatus->DevID.FD[0]	= 0;		// *FIXME*
		pStatus->DevID.FT		= 0x00000000;
		pStatus->DestCode		= DEST_WORLDWIDE;

		printf("MapleDevReq() Trans DestAP:%x is NOT a Device !\n", pTrans->Frame.DestAP );
	//	CPU_Halt("MapleDevReq: NOT A DEVICE");
	}
}

void MapleDevReset( MapleTrans * pTrans )
{
	MapleFrame * pFrame = (MapleFrame *)(mem_b.data + (pTrans->Addr &0xFFFFFF));
	fxDevStatus * pStatus = (fxDevStatus *)(mem_b.data + (pTrans->Addr &0xFFFFFF) + 4);
	if( pTrans->Frame.DestAP &AP_DE )
	{
		pFrame->CmdCode	= MAPLE_DEV_STATUS;
		pFrame->DestAP	= pTrans->Frame.OrigAP;
		pFrame->OrigAP	= pTrans->Frame.DestAP;
		pFrame->DataLen	= 0x0;	// 0x1C << 2 = 0x70 (112B)

	} else
	{
		pFrame->CmdCode	= MAPLE_DEV_STATUS;
		pFrame->DestAP	= pTrans->Frame.OrigAP;
		pFrame->OrigAP	= pTrans->Frame.DestAP;
		pFrame->DataLen	= 0;//0x1C;	// 0x1C << 2 = 0x70 (112B)

		pStatus->DevID.FD[2]	= pStatus->DevID.FD[1] = pStatus->DevID.FD[0]	= 0;		// *FIXME*
		pStatus->DevID.FT		= 0x00000000;
		pStatus->DestCode		= DEST_WORLDWIDE;

		printf("MapleDevReset() Trans DestAP:%x is NOT a Device !\n", pTrans->Frame.DestAP );
	//	CPU_Halt("MapleDevReq: NOT A DEVICE");
	}
}

#endif

