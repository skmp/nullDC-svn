#pragma once
#include "drkPvr.h"

#define RegSize (0x8000)
#define RegMask (RegSize-1)

#define PvrReg(x) (*(u32*)&regs[(x) & RegMask])

extern u8 regs[RegSize];

u32 ReadPvrRegister(u32 addr,u32 size);
void WritePvrRegister(u32 addr,u32 data,u32 size);

bool Regs_Init();
void Regs_Term();
void Regs_Reset(bool Manual);


#define ID_addr					0x00000000	//	R	Device ID	
#define REVISION_addr			0x00000004	//	R	Revision number	
#define SOFTRESET_addr			0x00000008	//	RW	CORE & TA software reset	
	
#define STARTRENDER_addr		0x00000014	//	RW	Drawing start	
#define TEST_SELECT_addr		0x00000018	//	RW	Test (writing this register is prohibited)	

#define PARAM_BASE_addr			0x00000020	//	RW	Base address for ISP parameters	

#define REGION_BASE_addr		0x0000002C	//	RW	Base address for Region Array	
#define SPAN_SORT_CFG_addr		0x00000030	//	RW	Span Sorter control	

#define VO_BORDER_COL_addr		0x00000040	//	RW	Border area color	
#define FB_R_CTRL_addr			0x00000044	//	RW	Frame buffer read control	
#define FB_W_CTRL_addr			0x00000048	//	RW	Frame buffer write control	
#define FB_W_LINESTRIDE_addr	0x0000004C	//	RW	Frame buffer line stride	
#define FB_R_SOF1_addr			0x00000050	//	RW	Read start address for field - 1/strip - 1	
#define FB_R_SOF2_addr			0x00000054	//	RW	Read start address for field - 2/strip - 2	

#define FB_R_SIZE_addr			0x0000005C	//	RW	Frame buffer XY size	
#define FB_W_SOF1_addr			0x00000060	//	RW	Write start address for field - 1/strip - 1	
#define FB_W_SOF2_addr			0x00000064	//	RW	Write start address for field - 2/strip - 2	
#define FB_X_CLIP_addr			0x00000068	//	RW	Pixel clip X coordinate	
#define FB_Y_CLIP_addr			0x0000006C	//	RW	Pixel clip Y coordinate	


#define FPU_SHAD_SCALE_addr		0x00000074	//	RW	Intensity Volume mode	
#define FPU_CULL_VAL_addr		0x00000078	//	RW	Comparison value for culling	
#define FPU_PARAM_CFG_addr		0x0000007C	//	RW	Parameter read control	
#define HALF_OFFSET_addr		0x00000080	//	RW	Pixel sampling control
#define FPU_PERP_VAL_addr		0x00000084	//	RW	Comparison value for perpendicular polygons	
#define ISP_BACKGND_D_addr		0x00000088	//	RW	Background surface depth	
#define ISP_BACKGND_T_addr		0x0000008C	//	RW	Background surface tag	

#define ISP_FEED_CFG_addr		0x00000098	//	RW	Translucent polygon sort mode	

#define SDRAM_REFRESH_addr		0x000000A0	//	RW	Texture memory refresh counter	
#define SDRAM_ARB_CFG_addr		0x000000A4	//	RW	Texture memory arbiter control	
#define SDRAM_CFG_addr			0x000000A8	//	RW	Texture memory control	

#define FOG_COL_RAM_addr		0x000000B0	//	RW	Color for Look Up table Fog	
#define FOG_COL_VERT_addr		0x000000B4	//	RW	Color for vertex Fog
#define FOG_DENSITY_addr		0x000000B8	//	RW	Fog scale value	
#define FOG_CLAMP_MAX_addr		0x000000BC	//	RW	Color clamping maximum value	
#define FOG_CLAMP_MIN_addr		0x000000C0	//	RW	Color clamping minimum value	
#define SPG_TRIGGER_POS_addr	0x000000C4	//	RW	External trigger signal HV counter value	
#define SPG_HBLANK_INT_addr		0x000000C8	//	RW	H-blank interrupt control	
#define SPG_VBLANK_INT_addr		0x000000CC	//	RW	V-blank interrupt control	
#define SPG_CONTROL_addr		0x000000D0	//	RW	Sync pulse generator control	
#define SPG_HBLANK_addr			0x000000D4	//	RW	H-blank control	
#define SPG_LOAD_addr			0x000000D8	//	RW	HV counter load value	
#define SPG_VBLANK_addr			0x000000DC	//	RW	V-blank control	
#define SPG_WIDTH_addr			0x000000E0	//	RW	Sync width control	
#define TEXT_CONTROL_addr		0x000000E4	//	RW	Texturing control	
#define VO_CONTROL_addr			0x000000E8	//	RW	Video output control	
#define VO_STARTX_addr			0x000000Ec	//	RW	Video output start X position	
#define VO_STARTY_addr			0x000000F0	//	RW	Video output start Y position	
#define SCALER_CTL_addr			0x000000F4	//	RW	X & Y scaler control	
#define PAL_RAM_CTRL_addr		0x00000108	//	RW	Palette RAM control	
#define SPG_STATUS_addr			0x0000010C	//	R	Sync pulse generator status	
#define FB_BURSTCTRL_addr		0x00000110	//	RW	Frame buffer burst control	
#define FB_C_SOF_addr			0x00000114	//	R	Current frame buffer start address	
#define Y_COEFF_addr			0x00000118	//	RW	Y scaling coefficient	

#define PT_ALPHA_REF_addr		0x0000011C	//	RW	Alpha value for Punch Through polygon comparison	



//	TA REGS
#define TA_OL_BASE_addr		0x00000124	//	RW	Object list write start address	
#define TA_ISP_BASE_addr		0x00000128	//	RW	ISP/TSP Parameter write start address	
#define TA_OL_LIMIT_addr		0x0000012C	//	RW	Start address of next Object Pointer Block	
#define TA_ISP_LIMIT_addr		0x00000130	//	RW	Current ISP/TSP Parameter write address	
#define TA_NEXT_OPB_addr		0x00000134	//	R	Global Tile clip control	
#define TA_ITP_CURRENT_addr		0x00000138	//	R	Current ISP/TSP Parameter write address	
#define TA_GLOB_TILE_CLIP_addr	0x0000013C	//	RW	Global Tile clip control
#define TA_ALLOC_CTRL_addr		0x00000140	//	RW	Object list control	
#define TA_LIST_INIT_addr		0x00000144	//	RW	TA initialization	
#define TA_YUV_TEX_BASE_addr	0x00000148	//	RW	YUV422 texture write start address	
#define TA_YUV_TEX_CTRL_addr	0x0000014C	//	RW	YUV converter control	
#define TA_YUV_TEX_CNT_addr		0x00000150	//	R	YUV converter macro block counter value	

#define TA_LIST_CONT_addr		0x00000160	//	RW	TA continuation processing	
#define TA_NEXT_OPB_INIT_addr	0x00000164	//	RW	Additional OPB starting address	



#define FOG_TABLE_START_addr		0x00000200	//	RW	Look-up table Fog data	
#define FOG_TABLE_END_addr			0x000003FC

#define TA_OL_POINTERS_START_addr	0x00000600	//	R	TA object List Pointer data	
#define TA_OL_POINTERS_END_addr		0x00000F5C

#define PALETTE_RAM_START_addr		0x00001000	//	RW	Palette RAM	
#define PALETTE_RAM_END_addr		0x00001FFC



// Regs -- Start

#define ID					PvrReg(ID_addr)	//	R	Device ID	
#define REVISION			PvrReg(REVISION_addr)	//	R	Revision number	
#define SOFTRESET			PvrReg(SOFTRESET_addr)	//	RW	CORE & TA software reset	
	
#define STARTRENDER			PvrReg(STARTRENDER_addr)	//	RW	Drawing start	
#define TEST_SELECT			PvrReg(TEST_SELECT_addr)	//	RW	Test (writing this register is prohibited)	

#define PARAM_BASE			PvrReg(PARAM_BASE_addr)	//	RW	Base address for ISP parameters	

#define REGION_BASE			PvrReg(REGION_BASE_addr)	//	RW	Base address for Region Array	
#define SPAN_SORT_CFG		PvrReg(SPAN_SORT_CFG_addr)	//	RW	Span Sorter control	

#define VO_BORDER_COL		PvrReg(VO_BORDER_COL_addr)	//	RW	Border area color	
#define FB_R_CTRL			PvrReg(FB_R_CTRL_addr)	//	RW	Frame buffer read control	
#define FB_W_CTRL			PvrReg(FB_W_CTRL_addr)	//	RW	Frame buffer write control	
#define FB_W_LINESTRIDE		PvrReg(FB_W_LINESTRIDE_addr)	//	RW	Frame buffer line stride	
#define FB_R_SOF1			PvrReg(FB_R_SOF1_addr)	//	RW	Read start address for field - 1/strip - 1	
#define FB_R_SOF2			PvrReg(FB_R_SOF2_addr)	//	RW	Read start address for field - 2/strip - 2	

#define FB_R_SIZE			PvrReg(FB_R_SIZE_addr)	//	RW	Frame buffer XY size	
#define FB_W_SOF1			PvrReg(FB_W_SOF1_addr)	//	RW	Write start address for field - 1/strip - 1	
#define FB_W_SOF2			PvrReg(FB_W_SOF2_addr)	//	RW	Write start address for field - 2/strip - 2	
#define FB_X_CLIP			PvrReg(FB_X_CLIP_addr)	//	RW	Pixel clip X coordinate	
#define FB_Y_CLIP			PvrReg(FB_Y_CLIP_addr)	//	RW	Pixel clip Y coordinate	


#define FPU_SHAD_SCALE		PvrReg(FPU_SHAD_SCALE_addr)	//	RW	Intensity Volume mode	
#define FPU_CULL_VAL		PvrReg(FPU_CULL_VAL_addr)	//	RW	Comparison value for culling	
#define FPU_PARAM_CFG		PvrReg(FPU_PARAM_CFG_addr)	//	RW	Parameter read control	
#define HALF_OFFSET			PvrReg(HALF_OFFSET_addr)	//	RW	Pixel sampling control
#define FPU_PERP_VAL		PvrReg(FPU_PERP_VAL_addr)	//	RW	Comparison value for perpendicular polygons	
#define ISP_BACKGND_D		PvrReg(ISP_BACKGND_D_addr)	//	RW	Background surface depth	
#define ISP_BACKGND_T		PvrReg(ISP_BACKGND_T_addr)	//	RW	Background surface tag	

#define ISP_FEED_CFG		PvrReg(ISP_FEED_CFG_addr)	//	RW	Translucent polygon sort mode	

#define SDRAM_REFRESH		PvrReg(SDRAM_REFRESH_addr)	//	RW	Texture memory refresh counter	
#define SDRAM_ARB_CFG		PvrReg(SDRAM_ARB_CFG_addr)	//	RW	Texture memory arbiter control	
#define SDRAM_CFG			PvrReg(SDRAM_CFG_addr)	//	RW	Texture memory control	

#define FOG_COL_RAM			PvrReg(FOG_COL_RAM_addr)	//	RW	Color for Look Up table Fog	
#define FOG_COL_VERT		PvrReg(FOG_COL_VERT_addr)	//	RW	Color for vertex Fog
#define FOG_DENSITY			PvrReg(FOG_DENSITY_addr)	//	RW	Fog scale value	
#define FOG_CLAMP_MAX		PvrReg(FOG_CLAMP_MAX_addr)	//	RW	Color clamping maximum value	
#define FOG_CLAMP_MIN		PvrReg(FOG_CLAMP_MIN_addr)	//	RW	Color clamping minimum value	
#define SPG_TRIGGER_POS		PvrReg(SPG_TRIGGER_POS_addr)	//	RW	External trigger signal HV counter value	
#define SPG_HBLANK_INT		PvrReg(SPG_HBLANK_INT_addr)	//	RW	H-blank interrupt control	
#define SPG_VBLANK_INT		PvrReg(SPG_VBLANK_INT_addr)	//	RW	V-blank interrupt control	
#define SPG_CONTROL			PvrReg(SPG_CONTROL_addr)	//	RW	Sync pulse generator control	
#define SPG_HBLANK			PvrReg(SPG_HBLANK_addr)	//	RW	H-blank control	
#define SPG_LOAD			PvrReg(SPG_LOAD_addr)	//	RW	HV counter load value	
#define SPG_VBLANK			PvrReg(SPG_VBLANK_addr)	//	RW	V-blank control	
#define SPG_WIDTH			PvrReg(SPG_WIDTH_addr)	//	RW	Sync width control	
#define TEXT_CONTROL		PvrReg(TEXT_CONTROL_addr)	//	RW	Texturing control	
#define VO_CONTROL			PvrReg(VO_CONTROL_addr)	//	RW	Video output control	
#define VO_STARTX			PvrReg(VO_STARTX_addr)	//	RW	Video output start X position	
#define VO_STARTY			PvrReg(VO_STARTY_addr)	//	RW	Video output start Y position	
#define SCALER_CTL			PvrReg(SCALER_CTL_addr)	//	RW	X & Y scaler control	
#define PAL_RAM_CTRL		PvrReg(PAL_RAM_CTRL_addr)	//	RW	Palette RAM control	
#define SPG_STATUS			PvrReg(SPG_STATUS_addr)	//	R	Sync pulse generator status	
#define FB_BURSTCTRL		PvrReg(FB_BURSTCTRL_addr)	//	RW	Frame buffer burst control	
#define FB_C_SOF			PvrReg(FB_C_SOF_addr)	//	R	Current frame buffer start address	
#define Y_COEFF				PvrReg(Y_COEFF_addr)	//	RW	Y scaling coefficient	

#define PT_ALPHA_REF		PvrReg(PT_ALPHA_REF_addr)	//	RW	Alpha value for Punch Through polygon comparison	



//	TA REGS
#define TA_OL_BASE			PvrReg(TA_OL_BASE_addr)	//	RW	Object list write start address	
#define TA_ISP_BASE			PvrReg(TA_ISP_BASE_addr)	//	RW	ISP/TSP Parameter write start address	
#define TA_OL_LIMIT			PvrReg(TA_OL_LIMIT_addr)	//	RW	Start address of next Object Pointer Block	
#define TA_ISP_LIMIT		PvrReg(TA_ISP_LIMIT_addr)	//	RW	Current ISP/TSP Parameter write address	
#define TA_NEXT_OPB			PvrReg(TA_NEXT_OPB_addr)	//	R	Global Tile clip control	
#define TA_ITP_CURRENT		PvrReg(TA_ITP_CURRENT_addr)	//	R	Current ISP/TSP Parameter write address	
#define TA_GLOB_TILE_CLIP	PvrReg(TA_GLOB_TILE_CLIP_addr)	//	RW	Global Tile clip control
#define TA_ALLOC_CTRL		PvrReg(TA_ALLOC_CTRL_addr)	//	RW	Object list control	
#define TA_LIST_INIT		PvrReg(TA_LIST_INIT_addr)	//	RW	TA initialization	
#define TA_YUV_TEX_BASE		PvrReg(TA_YUV_TEX_BASE_addr)	//	RW	YUV422 texture write start address	
#define TA_YUV_TEX_CTRL		PvrReg(TA_YUV_TEX_CTRL_addr)	//	RW	YUV converter control	
#define TA_YUV_TEX_CNT		PvrReg(TA_YUV_TEX_CNT_addr)	//	R	YUV converter macro block counter value	

#define TA_LIST_CONT		PvrReg(TA_LIST_CONT_addr)	//	RW	TA continuation processing	
#define TA_NEXT_OPB_INIT	PvrReg(TA_NEXT_OPB_INIT_addr)	//	RW	Additional OPB starting address	


#define FOG_TABLE			(&PvrReg(FOG_TABLE_START_addr))	//	RW	Look-up table Fog data	
#define TA_OL_POINTERS		(&PvrReg(TA_OL_POINTERS_START_addr))	//	R	TA object List Pointer data	
#define PALETTE_RAM			(&PvrReg(PALETTE_RAM_START_addr))	//	RW	Palette RAM	