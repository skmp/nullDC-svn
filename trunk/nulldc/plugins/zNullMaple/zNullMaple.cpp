/*
**	zNullMaple.cpp,	David Miller (2007)
*/
#include <stdio.h>
#include <windows.h>
#include "zNullMaple.h"
#include "MapleBus.h"




HINSTANCE hInst=NULL;

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD   ul_reason_for_call,
					  LPVOID  lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}









emu_info * ei;
maple_init_params * mip;

void FASTCALL Unload();
s32	 FASTCALL Load(emu_info*);

s32  FASTCALL Init(maple_init_params*);
void FASTCALL Term();
void FASTCALL Reset(bool);

s32 FASTCALL Create(maple_device_instance*,u8);
void FASTCALL Destroy(maple_device_instance*);


void FASTCALL SubDeviceDMA(maple_subdevice_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce);
void FASTCALL DeviceDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce);





/*
void EXPORT_CALL dcGetInterfaceInfo(plugin_interface_info* info)
{
	info->count				= 1;
	info->InterfaceVersion	= PLUGIN_I_F_VERSION;
}*/


void EXPORT_CALL dcGetInterface(u32 id, plugin_interface* info)
{/*
	maple_plugin_if		* pm	= &info->maple;
	common_info			* pci	= &info->common;
	maple_sub_plugin_if	* pms	= &info->maple_sub;

	if(0 == id)
	{
		pci->Load				= Load;
		pci->Unload				= Unload;
		pci->Type				= Plugin_Maple;
		pci->PluginVersion		= DC_MakeVersion(1,0,0,DC_VER_NORMAL);
		pci->InterfaceVersion	= MAPLE_PLUGIN_I_F_VERSION;

		pm->Init				= Init;
		pm->Reset				= Reset;
		pm->Term				= Term;
		pm->Create				= Create;
		pm->Destroy				= Destroy;

		pm->ShowConfig			= 0;
		pm->subdev_info			= 0;//MAPLE_SUBDEVICE_DISABLE_ALL;
	}*/
}














void FASTCALL Unload()				{ ei = NULL; }
s32	 FASTCALL Load(emu_info* emu)	{ ei = emu; return 0; }

s32  FASTCALL Init(maple_init_params* p)
{
	mip = p;
	return 0;
}
void FASTCALL Term()
{
	mip = NULL;
}


void FASTCALL Reset(bool Manual)
{

}

s32  FASTCALL Create(maple_device_instance* pMDI, u8 port)
{
//	pMDI->dma=ControllerDMA;
//	pMDI->data=0;
	return 0;
}

void FASTCALL Destroy(maple_device_instance* pMDI)
{

}







void FASTCALL SubDeviceDMA(maple_subdevice_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{

}
void FASTCALL DeviceDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{

}










#ifdef __USE_OLD_MAPLE_SOURCE__






void mplReset(bool,DWORD){}
void mplThreadInit(DWORD){}
void mplThreadTerm(DWORD){}
void mplConfig(DWORD,void*)
{
	printf("No Config\n");
}



void CreateInstance(MapleDev *pDev, MapleDevInst& DevInst, u8 port)
{
//	pDev->id
	DevInst.MapleDeviceDMA	=	MapleDevDMA;

}
void DestroyInstance(MapleDev*pDev, MapleDevInst& DevInst)
{

}





void MapleDevDMA(MapleDevInst* pDevInst, u32 Command,
				 u32* buffer_in,  u32 buffer_in_len,
				 u32* buffer_out, u32& buffer_out_len, u32& responce)
{

}






#ifdef BUILD_NAOMI

#include <windows.h>
#include <string.h>



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
#endif