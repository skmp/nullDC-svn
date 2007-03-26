/*
**	pvrMemory.cpp,	nullDC::nullPVR	(2007) ZeZu & drk||Raziel
*/
#include "nullPVR.h"
#include "nullRend.h"
#include "pvrMemory.h"
#include "ta_vdec.h"



u8 TA_Regs[0x200];
u8 TA_FogT[0x200];			// regs only
u8 TA_ObjT[0x1000];			// Obj.List Pointers Table
u8 TA_PalT[0x1000];			// Pallete Ram Table




u32  FASTCALL pvrReadReg(u32 addr,u32 size)
{
	addr &= 0xFFFF;
	ASSERT_F((4==size), "pvrReadReg, Len != 4");

	if(addr>=TA_REG_START && TA_REG_END>=addr)
	{
		switch(addr &0xFFF)
		{
		case ID:				return 0x17FD11DB;	// Set5.xx development box or consumer machine
		case REVISION:			return 0x00000011;	// 1.1 (FFFFFF(N/A).F(Major).F(Minor)		
		}
		//	printf("<PVR>\t pvrReadReg: @ %08X Unhandled!\n", addr);
		return *(u32*)&TA_Regs[addr-TA_REG_START];

	} else if(addr>=FOG_TABLE_START && FOG_TABLE_END>=addr)
	{
		return *(u32*)&TA_FogT[addr-FOG_TABLE_START];	

	} else if(addr>=TA_OL_POINTERS_START && TA_OL_POINTERS_END>=addr)
	{
		return *(u32*)&TA_ObjT[addr-TA_OL_POINTERS_START];

	} else if(addr>=PALETTE_RAM_START && PALETTE_RAM_END>=addr)
	{
		return *(u32*)&TA_PalT[addr-PALETTE_RAM_START];			// Pallete Ram Table
	}

	printf("#!\tERROR: pvrReadReg: @ %08X Illegal!\n", addr);
	return 0;
}


void FASTCALL pvrWriteReg(u32 addr,u32 data,u32 size)
{
	addr &= 0xFFFF;
	ASSERT_F((4==size), "pvrWriteReg, Len != 4");

	if(addr>=TA_REG_START && TA_REG_END>=addr)
	{
//		ASSERT_T(TA_State.Processing,"Write to Regs While Processing Fifo");

		switch(addr &0xFFF)
		{
		case SOFTRESET:
			printf("PVR SOFTRESET\n");
			SoftReset();
			break;

		case TA_ALLOC_CTRL:
			//combine_ac();
			break;	// useless to do anything until after write, use eol instead

		case TA_LIST_CONT:
			/*
			If list continuation processing is performed through the TA_LIST_CONT register,
			the TA initializes its internal status in the same manner as before,
			but leaves the TA_NEXT_OPB register unchanged.  As a result,
			the additional OPB for the list that is continuing to be input is stored
			after the OPB that was input last time.
			*/
			printf( ")>\tPVR2: TA_LIST_CONT Write <= %X\n", data);
			//lprintf(")>\tPVR2: TA_LIST_CONT Write <= %X\n", data);
			ListCont();
			break;	// write it

		case TA_LIST_INIT:
			/*
			If list initialization is performed through the TA_LIST_INIT register,
			the TA initializes its internal status, loads the value in the TA_NEXT_OPB_INIT register
			into the TA_NEXT_OPB register, and then allocates space in texture memory as the OPB initial area,
			from the address that is specified in the TA_OL_BASE register to the address that is specified in the
			TA_NEXT_OPB_INIT register.
			*/
			//TA_State.ListInit=1;
			printf("LIST_INIT:\n");

			//*pTA_NEXT_OPB = *pTA_NEXT_OPB_INIT ;	// Load Next OPB Init to Next OPB

			//if(TA_State.RenderPending)
			//{
			//	printf("LIST_INIT && RenderPending, Rendering !\n");
			//	TA_State.RenderPending = 0;
			//	PvrIf::Render();
			//}

			//lists_complete=0;
			//combine_ac();
			//	lprintf("\n**************** TA_LIST_INIT ****************\n\n");
			ListInit();
			break;

		case STARTRENDER:
			if(0 != data) {
				//	lprintf(")>\tSTART RENDER ! lists_complete; %X Full: %X\n", lists_complete, LT_FULL);
/*
				// *FIXME*
				if(0x1F==lists_complete)
				{
					PvrIf::Render();
					TA_State.RenderPending = 0;
				} else {
					TA_State.RenderPending = 1;
					ASSERT_T((1),"STARTRENDER && Lists Not Complete !");
				}
				*/

				nRendIf->nrRender(NULL);

				params.RaiseInterrupt(holly_RENDER_DONE);
				params.RaiseInterrupt(holly_RENDER_DONE_vd);
				params.RaiseInterrupt(holly_RENDER_DONE_isp);
				return;
			}
			//lprintf(")>\tSTART RENDER ! Write Zero: %X\n",data);
			return;
		}
		//lprintf("<PVR>\t pvrWriteReg: @ %08X <= %X Unhandled!\n", addr, data);
		*(u32*)&TA_Regs[addr-TA_REG_START] = data;
		return;

	} else if(addr>=FOG_TABLE_START && FOG_TABLE_END>=addr)
	{
		*(u32*)&TA_FogT[addr-FOG_TABLE_START] = data;
		return;

	} else if(addr>=TA_OL_POINTERS_START && TA_OL_POINTERS_END>=addr)
	{
		*(u32*)&TA_ObjT[addr-TA_OL_POINTERS_START] = data;
		return;

	} else if(addr>=PALETTE_RAM_START && PALETTE_RAM_END>=addr)
	{
		if(4==size)
			*(u32*)&TA_PalT[addr-PALETTE_RAM_START] = data;		// Pallete Ram Table
		else if(2==size)
			*(u16*)&TA_PalT[addr-PALETTE_RAM_START] = data;		// Pallete Ram Table
		else
			ASSERT_F((0),"<PVR> WRITE TO PAL RAM, SIZE NOT SUPPORTED!\n");
		return;
	}
	printf("#!\tERROR: pvrWriteReg @ %08X <= %X (%d) Illegal!\n", addr, data, size);
}





void InitTA_MMR(void);
void InitTA_Regs(void)
{
	for(int r=0; r<0x200; r++)
		TA_Regs[r] = TA_FogT[r] = DEF_ZERO;

	InitTA_MMR();

//	ac = (AllocCtrl*)pTA_ALLOC_CTRL;

	*pID				= DEF_ID;
	*pREVISION			= DEF_REVISION;
	*pSOFTRESET			= DEF_SOFTRESET;

	*pSPG_HBLANK_INT	= DEF_HBLANK_INT;
	*pSPG_VBLANK_INT	= DEF_VBLANK_INT;

	*pFPU_PARAM_CFG		= DEF_FPU_PARAM_CFG;

	*pHALF_OFFSET		= DEF_HALF_OFFSET;

	*pISP_FEED_CFG		= DEF_ISP_FEED_CFG;

	*pSDRAM_REFRESH		= DEF_SDRAM_REFRESH;
	*pSDRAM_ARB_CFG		= DEF_SDRAM_ARB_CFG;
	*pSDRAM_CFG			= DEF_SDRAM_CFG;

	*pSPG_HBLANK		= DEF_SPG_HBLANK;
	*pSPG_LOAD			= DEF_SPG_LOAD;
	*pSPG_VBLANK		= DEF_SPG_VBLANK;
	*pSPG_WIDTH			= DEF_SPG_WIDTH;

	*pVO_CONTROL		= DEF_VO_CONTROL;
	*pVO_STARTX			= DEF_VO_STARTX;
	*pVO_STARTY			= DEF_VO_STARTY;

	*pSCALER_CTL		= DEF_SCALER_CTL;

	*pFB_BURSTCTRL		= DEF_FB_BURSTCTRL;

	*pPT_ALPHA_REF		= DEF_PT_ALPHA_REF;
}

void InitTA_MMR(void)
{
	pID					= (u32*)&TA_Regs[ID];
	pREVISION			= (u32*)&TA_Regs[REVISION];
	pSOFTRESET			= (u32*)&TA_Regs[SOFTRESET];

	pSTARTRENDER		= (u32*)&TA_Regs[STARTRENDER];
	pTEST_SELECT		= (u32*)&TA_Regs[TEST_SELECT];

	pPARAM_BASE			= (u32*)&TA_Regs[PARAM_BASE];

	pREGION_BASE		= (u32*)&TA_Regs[REGION_BASE];
	pSPAN_SORT_CFG		= (u32*)&TA_Regs[SPAN_SORT_CFG];

	pVO_BORDER_COL		= (u32*)&TA_Regs[VO_BORDER_COL];
	pFB_R_CTRL			= (u32*)&TA_Regs[FB_R_CTRL];
	pFB_W_CTRL			= (u32*)&TA_Regs[FB_W_CTRL];
	pFB_W_LINESTRIDE	= (u32*)&TA_Regs[FB_W_LINESTRIDE];
	pFB_R_SOF1			= (u32*)&TA_Regs[FB_R_SOF1];
	pFB_R_SOF2			= (u32*)&TA_Regs[FB_R_SOF2];

	pFB_R_SIZE			= (u32*)&TA_Regs[FB_R_SIZE];
	pFB_W_SOF1			= (u32*)&TA_Regs[FB_W_SOF1];
	pFB_W_SOF2			= (u32*)&TA_Regs[FB_W_SOF2];
	pFB_X_CLIP			= (u32*)&TA_Regs[FB_X_CLIP];
	pFB_Y_CLIP			= (u32*)&TA_Regs[FB_Y_CLIP];

	pFPU_SHAD_SCALE		= (u32*)&TA_Regs[FPU_SHAD_SCALE];
	pFPU_CULL_VAL		= (u32*)&TA_Regs[FPU_CULL_VAL];
	pFPU_PARAM_CFG		= (u32*)&TA_Regs[FPU_PARAM_CFG];
	pHALF_OFFSET		= (u32*)&TA_Regs[HALF_OFFSET];
	pFPU_PERP_VAL		= (u32*)&TA_Regs[FPU_PERP_VAL];
	pISP_BACKGND_D		= (u32*)&TA_Regs[ISP_BACKGND_D];
	pISP_BACKGND_T		= (u32*)&TA_Regs[ISP_BACKGND_T];

	pISP_FEED_CFG		= (u32*)&TA_Regs[ISP_FEED_CFG];

	pSDRAM_REFRESH		= (u32*)&TA_Regs[SDRAM_REFRESH];
	pSDRAM_ARB_CFG		= (u32*)&TA_Regs[SDRAM_ARB_CFG];
	pSDRAM_CFG			= (u32*)&TA_Regs[SDRAM_CFG];

	pFOG_COL_RAM		= (u32*)&TA_Regs[FOG_COL_RAM];
	pFOG_COL_VERT		= (u32*)&TA_Regs[FOG_COL_VERT];
	pFOG_DENSITY		= (u32*)&TA_Regs[FOG_DENSITY];
	pFOG_CLAMP_MAX		= (u32*)&TA_Regs[FOG_CLAMP_MAX];
	pFOG_CLAMP_MIN		= (u32*)&TA_Regs[FOG_CLAMP_MIN];
	pSPG_TRIGGER_POS	= (u32*)&TA_Regs[SPG_TRIGGER_POS];
	pSPG_HBLANK_INT		= (u32*)&TA_Regs[SPG_HBLANK_INT];
	pSPG_VBLANK_INT		= (u32*)&TA_Regs[SPG_VBLANK_INT];
	pSPG_CONTROL		= (u32*)&TA_Regs[SPG_CONTROL];
	pSPG_HBLANK			= (u32*)&TA_Regs[SPG_HBLANK];
	pSPG_LOAD			= (u32*)&TA_Regs[SPG_LOAD];
	pSPG_VBLANK			= (u32*)&TA_Regs[SPG_VBLANK];
	pSPG_WIDTH			= (u32*)&TA_Regs[SPG_WIDTH];
	pTEXT_CONTROL		= (u32*)&TA_Regs[TEXT_CONTROL];
	pVO_CONTROL			= (u32*)&TA_Regs[VO_CONTROL];
	pVO_STARTX			= (u32*)&TA_Regs[VO_STARTX];
	pVO_STARTY			= (u32*)&TA_Regs[VO_STARTY];
	pSCALER_CTL			= (u32*)&TA_Regs[SCALER_CTL];

	pPAL_RAM_CTRL		= (u32*)&TA_Regs[PAL_RAM_CTRL];
	pSPG_STATUS			= (u32*)&TA_Regs[SPG_STATUS];
	pFB_BURSTCTRL		= (u32*)&TA_Regs[FB_BURSTCTRL];
	pFB_C_SOF			= (u32*)&TA_Regs[FB_C_SOF];
	pY_COEFF			= (u32*)&TA_Regs[Y_COEFF];

	pPT_ALPHA_REF		= (u32*)&TA_Regs[PT_ALPHA_REF];



	/*	TA REGS
	*/
	pTA_OL_BASE			= (u32*)&TA_Regs[TA_OL_BASE];
	pTA_ISP_BASE		= (u32*)&TA_Regs[TA_ISP_BASE];
	pTA_OL_LIMIT		= (u32*)&TA_Regs[TA_OL_LIMIT];
	pTA_ISP_LIMIT		= (u32*)&TA_Regs[TA_ISP_LIMIT];
	pTA_NEXT_OPB		= (u32*)&TA_Regs[TA_NEXT_OPB];
	pTA_ITP_CURRENT		= (u32*)&TA_Regs[TA_ITP_CURRENT];
	pTA_GLOB_TILE_CLIP	= (u32*)&TA_Regs[TA_GLOB_TILE_CLIP];
	pTA_ALLOC_CTRL		= (u32*)&TA_Regs[TA_ALLOC_CTRL];
	pTA_LIST_INIT		= (u32*)&TA_Regs[TA_LIST_INIT];
	pTA_YUV_TEX_BASE	= (u32*)&TA_Regs[TA_YUV_TEX_BASE];
	pTA_YUV_TEX_CTRL	= (u32*)&TA_Regs[TA_YUV_TEX_CTRL];
	pTA_YUV_TEX_CNT		= (u32*)&TA_Regs[TA_YUV_TEX_CNT];

	pTA_LIST_CONT		= (u32*)&TA_Regs[TA_LIST_CONT];
	pTA_NEXT_OPB_INIT	= (u32*)&TA_Regs[TA_NEXT_OPB_INIT];
}


// MMR's



u32 * pID;
u32 * pREVISION;
u32 * pSOFTRESET;

u32 * pSTARTRENDER;
u32 * pTEST_SELECT;

u32 * pPARAM_BASE;

u32 * pREGION_BASE;
u32 * pSPAN_SORT_CFG;

u32 * pVO_BORDER_COL;
u32 * pFB_R_CTRL;
u32 * pFB_W_CTRL;
u32 * pFB_W_LINESTRIDE;
u32 * pFB_R_SOF1;
u32 * pFB_R_SOF2;

u32 * pFB_R_SIZE;
u32 * pFB_W_SOF1;
u32 * pFB_W_SOF2;
u32 * pFB_X_CLIP;
u32 * pFB_Y_CLIP;


u32 * pFPU_SHAD_SCALE;
u32 * pFPU_CULL_VAL;
u32 * pFPU_PARAM_CFG;
u32 * pHALF_OFFSET;
u32 * pFPU_PERP_VAL;
u32 * pISP_BACKGND_D;
u32 * pISP_BACKGND_T;

u32 * pISP_FEED_CFG;

u32 * pSDRAM_REFRESH;
u32 * pSDRAM_ARB_CFG;
u32 * pSDRAM_CFG;

u32 * pFOG_COL_RAM;
u32 * pFOG_COL_VERT;
u32 * pFOG_DENSITY;
u32 * pFOG_CLAMP_MAX;
u32 * pFOG_CLAMP_MIN;
u32 * pSPG_TRIGGER_POS;
u32 * pSPG_HBLANK_INT;
u32 * pSPG_VBLANK_INT;
u32 * pSPG_CONTROL;
u32 * pSPG_HBLANK;
u32 * pSPG_LOAD;
u32 * pSPG_VBLANK;
u32 * pSPG_WIDTH;
u32 * pTEXT_CONTROL;
u32 * pVO_CONTROL;
u32 * pVO_STARTX;
u32 * pVO_STARTY;
u32 * pSCALER_CTL;

u32 * pPAL_RAM_CTRL;
u32 * pSPG_STATUS;
u32 * pFB_BURSTCTRL;
u32 * pFB_C_SOF;
u32 * pY_COEFF;

u32 * pPT_ALPHA_REF;



/*	TA REGS
*/
u32 * pTA_OL_BASE;
u32 * pTA_ISP_BASE;
u32 * pTA_OL_LIMIT;
u32 * pTA_ISP_LIMIT;
u32 * pTA_NEXT_OPB;
u32 * pTA_ITP_CURRENT;
u32 * pTA_GLOB_TILE_CLIP;
u32 * pTA_ALLOC_CTRL;
u32 * pTA_LIST_INIT;
u32 * pTA_YUV_TEX_BASE;
u32 * pTA_YUV_TEX_CTRL;
u32 * pTA_YUV_TEX_CNT;

u32 * pTA_LIST_CONT;
u32 * pTA_NEXT_OPB_INIT;


