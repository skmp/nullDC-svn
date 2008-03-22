/*
**	TA_Regs.h - David Miller 2006 - PowerVR2 Emulation Library
*/
#ifndef __TAREGS_H__
#define __TAREGS_H__


extern u8 TA_Regs[0x200];			// regs only
extern u8 TA_FogT[0x200];			// regs only
extern u8 TA_ObjT[0x1000];			// Obj.List Pointers Table
extern u8 TA_PalT[0x1000];			// Pallete Ram Table


void InitTA_Regs(void);

	// 0x005F8000 - 0x005F9FFF			TA / PVR Core Regs
	//


# define ID					0x00000000	/*	R	Device ID	*/
# define REVISION			0x00000004	/*	R	Revision number	*/
# define SOFTRESET			0x00000008	/*	RW	CORE & TA software reset	*/
	
# define STARTRENDER		0x00000014	/*	RW	Drawing start	*/
# define TEST_SELECT		0x00000018	/*	RW	Test (writing this register is prohibited)	*/

# define PARAM_BASE			0x00000020	/*	RW	Base address for ISP parameters	*/

# define REGION_BASE		0x0000002C	/*	RW	Base address for Region Array	*/
# define SPAN_SORT_CFG		0x00000030	/*	RW	Span Sorter control	*/

# define VO_BORDER_COL		0x00000040	/*	RW	Border area color	*/
# define FB_R_CTRL			0x00000044	/*	RW	Frame buffer read control	*/
# define FB_W_CTRL			0x00000048	/*	RW	Frame buffer write control	*/
# define FB_W_LINESTRIDE	0x0000004C	/*	RW	Frame buffer line stride	*/
# define FB_R_SOF1			0x00000050	/*	RW	Read start address for field - 1/strip - 1	*/
# define FB_R_SOF2			0x00000054	/*	RW	Read start address for field - 2/strip - 2	*/

# define FB_R_SIZE			0x0000005C	/*	RW	Frame buffer XY size	*/
# define FB_W_SOF1			0x00000060	/*	RW	Write start address for field - 1/strip - 1	*/
# define FB_W_SOF2			0x00000064	/*	RW	Write start address for field - 2/strip - 2	*/
# define FB_X_CLIP			0x00000068	/*	RW	Pixel clip X coordinate	*/
# define FB_Y_CLIP			0x0000006C	/*	RW	Pixel clip Y coordinate	*/


# define FPU_SHAD_SCALE		0x00000074	/*	RW	Intensity Volume mode	*/
# define FPU_CULL_VAL		0x00000078	/*	RW	Comparison value for culling	*/
# define FPU_PARAM_CFG		0x0000007C	/*	RW	Parameter read control	*/
# define HALF_OFFSET		0x00000080	/*	RW	Pixel sampling control	*/
# define FPU_PERP_VAL		0x00000084	/*	RW	Comparison value for perpendicular polygons	*/
# define ISP_BACKGND_D		0x00000088	/*	RW	Background surface depth	*/
# define ISP_BACKGND_T		0x0000008C	/*	RW	Background surface tag	*/

# define ISP_FEED_CFG		0x00000098	/*	RW	Translucent polygon sort mode	*/

# define SDRAM_REFRESH		0x000000A0	/*	RW	Texture memory refresh counter	*/
# define SDRAM_ARB_CFG		0x000000A4	/*	RW	Texture memory arbiter control	*/
# define SDRAM_CFG			0x000000A8	/*	RW	Texture memory control	*/

# define FOG_COL_RAM		0x000000B0	/*	RW	Color for Look Up table Fog	*/
# define FOG_COL_VERT		0x000000B4	/*	RW	Color for vertex Fog	*/
# define FOG_DENSITY		0x000000B8	/*	RW	Fog scale value	*/
# define FOG_CLAMP_MAX		0x000000BC	/*	RW	Color clamping maximum value	*/
# define FOG_CLAMP_MIN		0x000000C0	/*	RW	Color clamping minimum value	*/
# define SPG_TRIGGER_POS	0x000000C4	/*	RW	External trigger signal HV counter value	*/
# define SPG_HBLANK_INT		0x000000C8	/*	RW	H-blank interrupt control	*/
# define SPG_VBLANK_INT		0x000000CC	/*	RW	V-blank interrupt control	*/
# define SPG_CONTROL		0x000000D0	/*	RW	Sync pulse generator control	*/
# define SPG_HBLANK			0x000000D4	/*	RW	H-blank control	*/
# define SPG_LOAD			0x000000D8	/*	RW	HV counter load value	*/
# define SPG_VBLANK			0x000000DC	/*	RW	V-blank control	*/
# define SPG_WIDTH			0x000000E0	/*	RW	Sync width control	*/
# define TEXT_CONTROL		0x000000E4	/*	RW	Texturing control	*/
# define VO_CONTROL			0x000000E8	/*	RW	Video output control	*/
# define VO_STARTX			0x000000Ec	/*	RW	Video output start X position	*/
# define VO_STARTY			0x000000F0	/*	RW	Video output start Y position	*/
# define SCALER_CTL			0x000000F4	/*	RW	X & Y scaler control	*/

# define PAL_RAM_CTRL		0x00000108	/*	RW	Palette RAM control	*/
# define SPG_STATUS			0x0000010C	/*	R	Sync pulse generator status	*/
# define FB_BURSTCTRL		0x00000110	/*	RW	Frame buffer burst control	*/
# define FB_C_SOF			0x00000114	/*	R	Current frame buffer start address	*/
# define Y_COEFF			0x00000118	/*	RW	Y scaling coefficient	*/

# define PT_ALPHA_REF		0x0000011C	/*	RW	Alpha value for Punch Through polygon comparison	*/



/*	TA REGS
*/
# define TA_OL_BASE			0x00000124	/*	RW	Object list write start address	*/
# define TA_ISP_BASE		0x00000128	/*	RW	ISP/TSP Parameter write start address	*/
# define TA_OL_LIMIT		0x0000012C	/*	RW	Start address of next Object Pointer Block	*/
# define TA_ISP_LIMIT		0x00000130	/*	RW	Current ISP/TSP Parameter write address	*/
# define TA_NEXT_OPB		0x00000134	/*	R	Global Tile clip control	*/
# define TA_ITP_CURRENT		0x00000138	/*	R	Current ISP/TSP Parameter write address	*/
# define TA_GLOB_TILE_CLIP	0x0000013C	/*	RW	Global Tile clip control	*/
# define TA_ALLOC_CTRL		0x00000140	/*	RW	Object list control	*/
# define TA_LIST_INIT		0x00000144	/*	RW	TA initialization	*/
# define TA_YUV_TEX_BASE	0x00000148	/*	RW	YUV422 texture write start address	*/
# define TA_YUV_TEX_CTRL	0x0000014C	/*	RW	YUV converter control	*/
# define TA_YUV_TEX_CNT		0x00000150	/*	R	YUV converter macro block counter value	*/

# define TA_LIST_CONT		0x00000160	/*	RW	TA continuation processing	*/
# define TA_NEXT_OPB_INIT	0x00000164	/*	RW	Additional OPB starting address	*/


//////////////////////////////////////////

# define TA_REG_START			0x00008000
# define TA_REG_END				0x000081FF

# define FOG_TABLE_START		0x00008200	/*	RW	Look-up table Fog data	*/
# define FOG_TABLE_END			0x000083FC

# define TA_OL_POINTERS_START	0x00008600	/*	R	TA object List Pointer data	*/
# define TA_OL_POINTERS_END		0x00008F5C

# define PALETTE_RAM_START		0x00009000	/*	RW	Palette RAM	*/
# define PALETTE_RAM_END		0x00009FFC





// START TA REG MMR AREA

void InitPVR_MMRegs(void);

extern u32 * pID;
extern u32 * pREVISION;
extern u32 * pSOFTRESET;
	
extern u32 * pSTARTRENDER;
extern u32 * pTEST_SELECT;

extern u32 * pPARAM_BASE;

extern u32 * pREGION_BASE;
extern u32 * pSPAN_SORT_CFG;

extern u32 * pVO_BORDER_COL;
extern u32 * pFB_R_CTRL;
extern u32 * pFB_W_CTRL;
extern u32 * pFB_W_LINESTRIDE;
extern u32 * pFB_R_SOF1;
extern u32 * pFB_R_SOF2;

extern u32 * pFB_R_SIZE;
extern u32 * pFB_W_SOF1;
extern u32 * pFB_W_SOF2;
extern u32 * pFB_X_CLIP;
extern u32 * pFB_Y_CLIP;


extern u32 * pFPU_SHAD_SCALE;
extern u32 * pFPU_CULL_VAL;
extern u32 * pFPU_PARAM_CFG;
extern u32 * pHALF_OFFSET;
extern u32 * pFPU_PERP_VAL;
extern u32 * pISP_BACKGND_D;
extern u32 * pISP_BACKGND_T;

extern u32 * pISP_FEED_CFG;

extern u32 * pSDRAM_REFRESH;
extern u32 * pSDRAM_ARB_CFG;
extern u32 * pSDRAM_CFG;

extern u32 * pFOG_COL_RAM;
extern u32 * pFOG_COL_VERT;
extern u32 * pFOG_DENSITY;
extern u32 * pFOG_CLAMP_MAX;
extern u32 * pFOG_CLAMP_MIN;
extern u32 * pSPG_TRIGGER_POS;
extern u32 * pSPG_HBLANK_INT;
extern u32 * pSPG_VBLANK_INT;
extern u32 * pSPG_CONTROL;
extern u32 * pSPG_HBLANK;
extern u32 * pSPG_LOAD;
extern u32 * pSPG_VBLANK;
extern u32 * pSPG_WIDTH;
extern u32 * pTEXT_CONTROL;
extern u32 * pVO_CONTROL;
extern u32 * pVO_STARTX;
extern u32 * pVO_STARTY;
extern u32 * pSCALER_CTL;

extern u32 * pPAL_RAM_CTRL;
extern u32 * pSPG_STATUS;
extern u32 * pFB_BURSTCTRL;
extern u32 * pFB_C_SOF;
extern u32 * pY_COEFF;

extern u32 * pPT_ALPHA_REF;



/*	TA REGS
*/
extern u32 * pTA_OL_BASE;
extern u32 * pTA_ISP_BASE;
extern u32 * pTA_OL_LIMIT;
extern u32 * pTA_ISP_LIMIT;
extern u32 * pTA_NEXT_OPB;
extern u32 * pTA_ITP_CURRENT;
extern u32 * pTA_GLOB_TILE_CLIP;
extern u32 * pTA_ALLOC_CTRL;
extern u32 * pTA_LIST_INIT;
extern u32 * pTA_YUV_TEX_BASE;
extern u32 * pTA_YUV_TEX_CTRL;
extern u32 * pTA_YUV_TEX_CNT;

extern u32 * pTA_LIST_CONT;
extern u32 * pTA_NEXT_OPB_INIT;


/* Default Values for TARegs
*/



#define DEF_ZERO			0x00000000

#define DEF_ID				0x17FD11DB
#define DEF_REVISION		0x00000011
#define DEF_SOFTRESET		0x00000007	// Softreset of SDRAM, pipeline && TA

#define DEF_FPU_PARAM_CFG	0x0007DF77

#define DEF_HALF_OFFSET		0x00000007

#define DEF_ISP_FEED_CFG	0x00402000

#define DEF_SDRAM_REFRESH	0x00000020
#define DEF_SDRAM_ARB_CFG	0x0000001F

#define DEF_SDRAM_CFG		0x15F28997

#define DEF_HBLANK_INT		0x031D0000
#define DEF_VBLANK_INT		0x01500104

#define DEF_SPG_CONTROL		0x00000040			// NTSC(1<<6)

#define DEF_SPG_HBLANK		0x007E0345
#define DEF_SPG_LOAD		0x01060359
#define DEF_SPG_VBLANK		0x01500104
#define DEF_SPG_WIDTH		0x07F1933F

#define DEF_VO_CONTROL		0x00000108
#define DEF_VO_STARTX		0x0000009D
#define DEF_VO_STARTY		0x00000015

#define DEF_SCALER_CTL		0x00000400

#define DEF_FB_BURSTCTRL	0x00090639

#define DEF_PT_ALPHA_REF	0x000000FF




#endif // __TAREGS_H__

