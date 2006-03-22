//Declarations of registers as varaible
#include "..\nullPvr.h"
#include "pvr_mmr.h"

//PpowerVR/TA registers
//These variables will hold the information of em , all reads/writes are redirected to em
//

//Generated from pvr_mmr_list.h using :RegEx{
//Find		= "\#define {[^\t]+}{[\t]+}{[^]*$}"
//Replace	= "u32 p\1;\2//\3"
//}

//TODO : Change code to use new ,standar, method of using registers

//TODO : move to plugin
//TODO : use structs for all PowerVR/TA registers



//Core block
//  Register Name		Area0 Offset		R/W    Short Descritpion
u32 ID;				//0x005F8000		R		Device ID
u32 REVISION;			//0x005F8004		R		Revision number
u32 SOFTRESET;			//0x005F8008		RW	CORE & TA software reset
u32 STARTRENDER;		//0x005F8014		RW	Drawing start
u32 TEST_SELECT;		//0x005F8018		RW	Test (writing this register is prohibited)
u32 PARAM_BASE;		//0x005F8020		RW	Base address for ISP parameters
u32 REGION_BASE;		//0x005F802C		RW	Base address for Region Array
u32 SPAN_SORT_CFG;		//0x005F8030		RW	Span Sorter control			
u32 VO_BORDER_COL;		//0x005F8040		RW	Border area color
u32 FB_R_CTRL;			//0x005F8044		RW	Frame buffer read control
u32 FB_W_CTRL;			//0x005F8048		RW	Frame buffer write control
u32 FB_W_LINESTRIDE;	//0x005F804C		RW	Frame buffer line stride
u32 FB_R_SOF1;			//0x005F8050		RW	Read start address for field - 1/strip - 1
u32 FB_R_SOF2;			//0x005F8054		RW	Read start address for field - 2/strip - 2
u32 FB_R_SIZE;			//0x005F805C		RW	Frame buffer XY size
u32 FB_W_SOF1;			//0x005F8060		RW	Write start address for field - 1/strip - 1
u32 FB_W_SOF2;			//0x005F8064		RW	Write start address for field - 2/strip - 2
u32 FB_X_CLIP;			//0x005F8068		RW	Pixel clip X coordinate
u32 FB_Y_CLIP;			//0x005F806C		RW	Pixel clip Y coordinate
//  Register Name		Area0 Offset		R/W    Short Descritpion
u32 FPU_SHAD_SCALE;		//0x005F8074		RW	Intensity Volume mode
u32 FPU_CULL_VAL;		//0x005F8078		RW	Comparison value for culling
u32 FPU_PARAM_CFG;		//0x005F807C		RW	Parameter read control
u32 HALF_OFFSET;		//0x005F8080		RW	Pixel sampling control
u32 FPU_PERP_VAL;		//0x005F8084		RW	Comparison value for perpendicular polygons
u32 ISP_BACKGND_D;		//0x005F8088		RW	Background surface depth
u32 ISP_BACKGND_T;		//0x005F808C		RW	Background surface tag
u32 ISP_FEED_CFG;		//0x005F8098		RW	Translucent polygon sort mode
u32 SDRAM_REFRESH;		//0x005F80A0		RW	Texture memory refresh counter
u32 SDRAM_ARB_CFG;		//0x005F80A4		RW	Texture memory arbiter control
u32 SDRAM_CFG;			//0x005F80A8		RW	Texture memory control		
u32 FOG_COL_RAM;		//0x005F80B0		RW	Color for Look Up table Fog
u32 FOG_COL_VERT;		//0x005F80B4		RW	Color for vertex Fog
u32 FOG_DENSITY;		//0x005F80B8		RW	Fog scale value
u32 FOG_CLAMP_MAX;		//0x005F80BC		RW	Color clamping maximum value
u32 FOG_CLAMP_MIN;		//0x005F80C0		RW	Color clamping minimum value
u32 SPG_TRIGGER_POS;	//0x005F80C4		RW	External trigger signal HV counter value
u32 SPG_HBLANK_INT;		//0x005F80C8		RW	H-blank interrupt control
//  Register Name		Area0 Offset		R/W    Short Descritpion
u32 SPG_VBLANK_INT;		//0x005F80CC		RW	V-blank interrupt control
u32 SPG_CONTROL;		//0x005F80D0		RW	Sync pulse generator control
u32 SPG_HBLANK;			//0x005F80D4		RW	H-blank control
u32 SPG_LOAD;			//0x005F80D8		RW	HV counter load value
u32 SPG_VBLANK;			//0x005F80DC		RW	V-blank control
u32 SPG_WIDTH;			//0x005F80E0		RW	Sync width control
u32 TEXT_CONTROL;		//0x005F80E4		RW	Texturing control
u32 VO_CONTROL;			//0x005F80E8		RW	Video output control
u32 VO_STARTX;			//0x005F80Ec		RW	Video output start X position
u32 VO_STARTY;			//0x005F80F0		RW	Video output start Y position
u32 SCALER_CTL;			//0x005F80F4		RW	X & Y scaler control
u32 PAL_RAM_CTRL;		//0x005F8108		RW	Palette RAM control
u32 SPG_STATUS;			//0x005F810C		R		Sync pulse generator status
u32 FB_BURSTCTRL;		//0x005F8110		RW	Frame buffer burst control
u32 FB_C_SOF;			//0x005F8114		R		Current frame buffer start address
u32 Y_COEFF;			//0x005F8118		RW	Y scaling coefficient
u32 PT_ALPHA_REF;		//0x005F811C		RW	Alpha value for Punch Through polygon comparison
//TA block
//  Register Name		Area0 Offset		R/W    Short Descritpion
u32 TA_OL_BASE;			//0x005F8124		RW	Object list write start address
u32 TA_ISP_BASE;		//0x005F8128		RW	ISP/TSP Parameter write start address
u32 TA_OL_LIMIT;		//0x005F812C		RW	Start address of next Object Pointer Block
u32 TA_ISP_LIMIT;		//0x005F8130		RW	Current ISP/TSP Parameter write address
u32 TA_NEXT_OPB;		//0x005F8134		R		Global Tile clip control
u32 TA_ITP_CURRENT;		//0x005F8138		R		Current ISP/TSP Parameter write address
u32 TA_GLOB_TILE_CLIP;	//0x005F813C		RW	Global Tile clip control
u32 TA_ALLOC_CTRL;		//0x005F8140		RW	Object list control
u32 TA_LIST_INIT;		//0x005F8144		RW	TA initialization
u32 TA_YUV_TEX_BASE;	//0x005F8148		RW	YUV422 texture write start address
u32 TA_YUV_TEX_CTRL;	//0x005F814C		RW	YUV converter control
u32 TA_YUV_TEX_CNT;		//0x005F8150		R		YUV converter macro block counter value
u32 TA_LIST_CONT;		//0x005F8160		RW	TA continuation processing
u32 TA_NEXT_OPB_INIT;	//0x005F8164		RW	Additional OPB starting address
u32 FOG_TABLE[128];		//0x005F8200		RW	Look-up table Fog data
//u32 pFOG_TABLE_END;		//0x005F83FC		RW	Look-up table Fog data
//  Register Name		Area0 Offset		R/W    Short Descritpion
u32 TA_OL_POINTERS[600];//0x005F8600		R		TA object List Pointer data
//u32 pTA_OL_POINTERS_END;	//0x005F8F5C	R		TA object List Pointer data	
u32 PALETTE_RAM[1024];	//0x005F9000		RW	Palette RAM
//u32 pPALETTE_RAM_END;	//0x005F9FFC		RW	Palette RAM


#define PVR_REG_READ(xx) u32 xx()
#define PVR_REG_WRITE(xx) void xx(u32 data)

//SPG_STATUS [0x005F810C] //R		Sync pulse generator status
//Read
PVR_REG_READ(rh_SPG_STATUS)
{
	SPG_STATUS^=0xFFFFFF;
	printf("SPG_STATUS : read not implemented , furthermore  it's badly hacked :D\n"); 
	return SPG_STATUS; 
}
//Write
PVR_REG_WRITE(wh_SPG_STATUS)
{
	printf("SPG_STATUS : write not possible, data=%x\n",data);
}

//Allocate any needed resources - 1 time init
void pvr_mmr_Init()
{

	//sample ^\tu32 p{[^;]*};[\t]*//{[^\t]*}[\t]*{[^\t]*}[\t]*{[^]*}$

	//from 
	//u32 pSB_PDSTAP;			//0x005F7C00		RW	PVR-DMA PVR start address
	//to
	// //\1 [\2:\3] : \4\npvr_regs[((\1_addr-PVR_BASE)>>2)].data32=&\1;\npvr_regs[((\1_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX \1 writeFunction;\npvr_regs[((\1_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX \1 readFunction\npvr_regs[((\1_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX \1 flags\n
	/*	
	//\1 [\2:\3] : \4
	pvr_regs[((SB_PDSTAP_addr-PVR_BASE)>>2)].data32=&SB_PDSTAP;
	pvr_regs[((SB_PDSTAP_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SB_PDSTAP writeFunction
	pvr_regs[((SB_PDSTAP_addr-PVR_BASE)>>2)].readFunction=0;				//TODO : FIX SB_PDSTAP readFunction
	pvr_regs[((SB_PDSTAP_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SB_PDSTAP flags
	*/

	//Core block
	//  Register Name		Area0 Offset		R/W    Short Descritpion
	//ID [0x005F8000:R] : Device ID
	pvr_regs[((ID_addr-PVR_BASE)>>2)].data32=&ID;
	pvr_regs[((ID_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX ID writeFunction;
	pvr_regs[((ID_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX ID readFunction
	pvr_regs[((ID_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX ID flags

	//REVISION [0x005F8004:R] : Revision number
	pvr_regs[((REVISION_addr-PVR_BASE)>>2)].data32=&REVISION;
	pvr_regs[((REVISION_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX REVISION writeFunction;
	pvr_regs[((REVISION_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX REVISION readFunction
	pvr_regs[((REVISION_addr-PVR_BASE)>>2)].flags=REG_READ_DATA | REG_CONST;	//TODO : FIX REVISION flags

	//SOFTRESET [0x005F8008:RW] : CORE & TA software reset
	pvr_regs[((SOFTRESET_addr-PVR_BASE)>>2)].data32=&SOFTRESET;
	pvr_regs[((SOFTRESET_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SOFTRESET writeFunction;
	pvr_regs[((SOFTRESET_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SOFTRESET readFunction
	pvr_regs[((SOFTRESET_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SOFTRESET flags

	//STARTRENDER [0x005F8014:RW] : Drawing start
	pvr_regs[((STARTRENDER_addr-PVR_BASE)>>2)].data32=&STARTRENDER;
	pvr_regs[((STARTRENDER_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX STARTRENDER writeFunction;
	pvr_regs[((STARTRENDER_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX STARTRENDER readFunction
	pvr_regs[((STARTRENDER_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX STARTRENDER flags

	//TEST_SELECT [0x005F8018:RW] : Test (writing this register is prohibited)
	pvr_regs[((TEST_SELECT_addr-PVR_BASE)>>2)].data32=&TEST_SELECT;
	pvr_regs[((TEST_SELECT_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TEST_SELECT writeFunction;
	pvr_regs[((TEST_SELECT_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TEST_SELECT readFunction
	pvr_regs[((TEST_SELECT_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TEST_SELECT flags

	//PARAM_BASE [0x005F8020:RW] : Base address for ISP parameters
	pvr_regs[((PARAM_BASE_addr-PVR_BASE)>>2)].data32=&PARAM_BASE;
	pvr_regs[((PARAM_BASE_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX PARAM_BASE writeFunction;
	pvr_regs[((PARAM_BASE_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX PARAM_BASE readFunction
	pvr_regs[((PARAM_BASE_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX PARAM_BASE flags

	//REGION_BASE [0x005F802C:RW] : Base address for Region Array
	pvr_regs[((REGION_BASE_addr-PVR_BASE)>>2)].data32=&REGION_BASE;
	pvr_regs[((REGION_BASE_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX REGION_BASE writeFunction;
	pvr_regs[((REGION_BASE_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX REGION_BASE readFunction
	pvr_regs[((REGION_BASE_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX REGION_BASE flags

	//SPAN_SORT_CFG [0x005F8030:RW] : Span Sorter control			
	pvr_regs[((SPAN_SORT_CFG_addr-PVR_BASE)>>2)].data32=&SPAN_SORT_CFG;
	pvr_regs[((SPAN_SORT_CFG_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SPAN_SORT_CFG writeFunction;
	pvr_regs[((SPAN_SORT_CFG_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SPAN_SORT_CFG readFunction
	pvr_regs[((SPAN_SORT_CFG_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SPAN_SORT_CFG flags

	//VO_BORDER_COL [0x005F8040:RW] : Border area color
	pvr_regs[((VO_BORDER_COL_addr-PVR_BASE)>>2)].data32=&VO_BORDER_COL;
	pvr_regs[((VO_BORDER_COL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX VO_BORDER_COL writeFunction;
	pvr_regs[((VO_BORDER_COL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX VO_BORDER_COL readFunction
	pvr_regs[((VO_BORDER_COL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX VO_BORDER_COL flags

	//FB_R_CTRL [0x005F8044:RW] : Frame buffer read control
	pvr_regs[((FB_R_CTRL_addr-PVR_BASE)>>2)].data32=&FB_R_CTRL;
	pvr_regs[((FB_R_CTRL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FB_R_CTRL writeFunction;
	pvr_regs[((FB_R_CTRL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FB_R_CTRL readFunction
	pvr_regs[((FB_R_CTRL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FB_R_CTRL flags

	//FB_W_CTRL [0x005F8048:RW] : Frame buffer write control
	pvr_regs[((FB_W_CTRL_addr-PVR_BASE)>>2)].data32=&FB_W_CTRL;
	pvr_regs[((FB_W_CTRL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FB_W_CTRL writeFunction;
	pvr_regs[((FB_W_CTRL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FB_W_CTRL readFunction
	pvr_regs[((FB_W_CTRL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FB_W_CTRL flags

	//FB_W_LINESTRIDE [0x005F804C:RW] : Frame buffer line stride
	pvr_regs[((FB_W_LINESTRIDE_addr-PVR_BASE)>>2)].data32=&FB_W_LINESTRIDE;
	pvr_regs[((FB_W_LINESTRIDE_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FB_W_LINESTRIDE writeFunction;
	pvr_regs[((FB_W_LINESTRIDE_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FB_W_LINESTRIDE readFunction
	pvr_regs[((FB_W_LINESTRIDE_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FB_W_LINESTRIDE flags

	//FB_R_SOF1 [0x005F8050:RW] : Read start address for field - 1/strip - 1
	pvr_regs[((FB_R_SOF1_addr-PVR_BASE)>>2)].data32=&FB_R_SOF1;
	pvr_regs[((FB_R_SOF1_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FB_R_SOF1 writeFunction;
	pvr_regs[((FB_R_SOF1_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FB_R_SOF1 readFunction
	pvr_regs[((FB_R_SOF1_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FB_R_SOF1 flags

	//FB_R_SOF2 [0x005F8054:RW] : Read start address for field - 2/strip - 2
	pvr_regs[((FB_R_SOF2_addr-PVR_BASE)>>2)].data32=&FB_R_SOF2;
	pvr_regs[((FB_R_SOF2_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FB_R_SOF2 writeFunction;
	pvr_regs[((FB_R_SOF2_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FB_R_SOF2 readFunction
	pvr_regs[((FB_R_SOF2_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FB_R_SOF2 flags

	//FB_R_SIZE [0x005F805C:RW] : Frame buffer XY size
	pvr_regs[((FB_R_SIZE_addr-PVR_BASE)>>2)].data32=&FB_R_SIZE;
	pvr_regs[((FB_R_SIZE_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FB_R_SIZE writeFunction;
	pvr_regs[((FB_R_SIZE_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FB_R_SIZE readFunction
	pvr_regs[((FB_R_SIZE_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FB_R_SIZE flags

	//FB_W_SOF1 [0x005F8060:RW] : Write start address for field - 1/strip - 1
	pvr_regs[((FB_W_SOF1_addr-PVR_BASE)>>2)].data32=&FB_W_SOF1;
	pvr_regs[((FB_W_SOF1_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FB_W_SOF1 writeFunction;
	pvr_regs[((FB_W_SOF1_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FB_W_SOF1 readFunction
	pvr_regs[((FB_W_SOF1_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FB_W_SOF1 flags

	//FB_W_SOF2 [0x005F8064:RW] : Write start address for field - 2/strip - 2
	pvr_regs[((FB_W_SOF2_addr-PVR_BASE)>>2)].data32=&FB_W_SOF2;
	pvr_regs[((FB_W_SOF2_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FB_W_SOF2 writeFunction;
	pvr_regs[((FB_W_SOF2_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FB_W_SOF2 readFunction
	pvr_regs[((FB_W_SOF2_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FB_W_SOF2 flags

	//FB_X_CLIP [0x005F8068:RW] : Pixel clip X coordinate
	pvr_regs[((FB_X_CLIP_addr-PVR_BASE)>>2)].data32=&FB_X_CLIP;
	pvr_regs[((FB_X_CLIP_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FB_X_CLIP writeFunction;
	pvr_regs[((FB_X_CLIP_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FB_X_CLIP readFunction
	pvr_regs[((FB_X_CLIP_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FB_X_CLIP flags

	//FB_Y_CLIP [0x005F806C:RW] : Pixel clip Y coordinate
	pvr_regs[((FB_Y_CLIP_addr-PVR_BASE)>>2)].data32=&FB_Y_CLIP;
	pvr_regs[((FB_Y_CLIP_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FB_Y_CLIP writeFunction;
	pvr_regs[((FB_Y_CLIP_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FB_Y_CLIP readFunction
	pvr_regs[((FB_Y_CLIP_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FB_Y_CLIP flags

	//  Register Name		Area0 Offset		R/W    Short Descritpion
	//FPU_SHAD_SCALE [0x005F8074:RW] : Intensity Volume mode
	pvr_regs[((FPU_SHAD_SCALE_addr-PVR_BASE)>>2)].data32=&FPU_SHAD_SCALE;
	pvr_regs[((FPU_SHAD_SCALE_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FPU_SHAD_SCALE writeFunction;
	pvr_regs[((FPU_SHAD_SCALE_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FPU_SHAD_SCALE readFunction
	pvr_regs[((FPU_SHAD_SCALE_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FPU_SHAD_SCALE flags

	//FPU_CULL_VAL [0x005F8078:RW] : Comparison value for culling
	pvr_regs[((FPU_CULL_VAL_addr-PVR_BASE)>>2)].data32=&FPU_CULL_VAL;
	pvr_regs[((FPU_CULL_VAL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FPU_CULL_VAL writeFunction;
	pvr_regs[((FPU_CULL_VAL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FPU_CULL_VAL readFunction
	pvr_regs[((FPU_CULL_VAL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FPU_CULL_VAL flags

	//FPU_PARAM_CFG [0x005F807C:RW] : Parameter read control
	pvr_regs[((FPU_PARAM_CFG_addr-PVR_BASE)>>2)].data32=&FPU_PARAM_CFG;
	pvr_regs[((FPU_PARAM_CFG_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FPU_PARAM_CFG writeFunction;
	pvr_regs[((FPU_PARAM_CFG_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FPU_PARAM_CFG readFunction
	pvr_regs[((FPU_PARAM_CFG_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FPU_PARAM_CFG flags

	//HALF_OFFSET [0x005F8080:RW] : Pixel sampling control
	pvr_regs[((HALF_OFFSET_addr-PVR_BASE)>>2)].data32=&HALF_OFFSET;
	pvr_regs[((HALF_OFFSET_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX HALF_OFFSET writeFunction;
	pvr_regs[((HALF_OFFSET_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX HALF_OFFSET readFunction
	pvr_regs[((HALF_OFFSET_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX HALF_OFFSET flags

	//FPU_PERP_VAL [0x005F8084:RW] : Comparison value for perpendicular polygons
	pvr_regs[((FPU_PERP_VAL_addr-PVR_BASE)>>2)].data32=&FPU_PERP_VAL;
	pvr_regs[((FPU_PERP_VAL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FPU_PERP_VAL writeFunction;
	pvr_regs[((FPU_PERP_VAL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FPU_PERP_VAL readFunction
	pvr_regs[((FPU_PERP_VAL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FPU_PERP_VAL flags

	//ISP_BACKGND_D [0x005F8088:RW] : Background surface depth
	pvr_regs[((ISP_BACKGND_D_addr-PVR_BASE)>>2)].data32=&ISP_BACKGND_D;
	pvr_regs[((ISP_BACKGND_D_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX ISP_BACKGND_D writeFunction;
	pvr_regs[((ISP_BACKGND_D_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX ISP_BACKGND_D readFunction
	pvr_regs[((ISP_BACKGND_D_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX ISP_BACKGND_D flags

	//ISP_BACKGND_T [0x005F808C:RW] : Background surface tag
	pvr_regs[((ISP_BACKGND_T_addr-PVR_BASE)>>2)].data32=&ISP_BACKGND_T;
	pvr_regs[((ISP_BACKGND_T_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX ISP_BACKGND_T writeFunction;
	pvr_regs[((ISP_BACKGND_T_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX ISP_BACKGND_T readFunction
	pvr_regs[((ISP_BACKGND_T_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX ISP_BACKGND_T flags

	//ISP_FEED_CFG [0x005F8098:RW] : Translucent polygon sort mode
	pvr_regs[((ISP_FEED_CFG_addr-PVR_BASE)>>2)].data32=&ISP_FEED_CFG;
	pvr_regs[((ISP_FEED_CFG_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX ISP_FEED_CFG writeFunction;
	pvr_regs[((ISP_FEED_CFG_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX ISP_FEED_CFG readFunction
	pvr_regs[((ISP_FEED_CFG_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX ISP_FEED_CFG flags

	//SDRAM_REFRESH [0x005F80A0:RW] : Texture memory refresh counter
	pvr_regs[((SDRAM_REFRESH_addr-PVR_BASE)>>2)].data32=&SDRAM_REFRESH;
	pvr_regs[((SDRAM_REFRESH_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SDRAM_REFRESH writeFunction;
	pvr_regs[((SDRAM_REFRESH_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SDRAM_REFRESH readFunction
	pvr_regs[((SDRAM_REFRESH_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SDRAM_REFRESH flags

	//SDRAM_ARB_CFG [0x005F80A4:RW] : Texture memory arbiter control
	pvr_regs[((SDRAM_ARB_CFG_addr-PVR_BASE)>>2)].data32=&SDRAM_ARB_CFG;
	pvr_regs[((SDRAM_ARB_CFG_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SDRAM_ARB_CFG writeFunction;
	pvr_regs[((SDRAM_ARB_CFG_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SDRAM_ARB_CFG readFunction
	pvr_regs[((SDRAM_ARB_CFG_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SDRAM_ARB_CFG flags

	//SDRAM_CFG [0x005F80A8:RW] : Texture memory control		
	pvr_regs[((SDRAM_CFG_addr-PVR_BASE)>>2)].data32=&SDRAM_CFG;
	pvr_regs[((SDRAM_CFG_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SDRAM_CFG writeFunction;
	pvr_regs[((SDRAM_CFG_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SDRAM_CFG readFunction
	pvr_regs[((SDRAM_CFG_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SDRAM_CFG flags

	//FOG_COL_RAM [0x005F80B0:RW] : Color for Look Up table Fog
	pvr_regs[((FOG_COL_RAM_addr-PVR_BASE)>>2)].data32=&FOG_COL_RAM;
	pvr_regs[((FOG_COL_RAM_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FOG_COL_RAM writeFunction;
	pvr_regs[((FOG_COL_RAM_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FOG_COL_RAM readFunction
	pvr_regs[((FOG_COL_RAM_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FOG_COL_RAM flags

	//FOG_COL_VERT [0x005F80B4:RW] : Color for vertex Fog
	pvr_regs[((FOG_COL_VERT_addr-PVR_BASE)>>2)].data32=&FOG_COL_VERT;
	pvr_regs[((FOG_COL_VERT_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FOG_COL_VERT writeFunction;
	pvr_regs[((FOG_COL_VERT_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FOG_COL_VERT readFunction
	pvr_regs[((FOG_COL_VERT_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FOG_COL_VERT flags

	//FOG_DENSITY [0x005F80B8:RW] : Fog scale value
	pvr_regs[((FOG_DENSITY_addr-PVR_BASE)>>2)].data32=&FOG_DENSITY;
	pvr_regs[((FOG_DENSITY_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FOG_DENSITY writeFunction;
	pvr_regs[((FOG_DENSITY_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FOG_DENSITY readFunction
	pvr_regs[((FOG_DENSITY_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FOG_DENSITY flags

	//FOG_CLAMP_MAX [0x005F80BC:RW] : Color clamping maximum value
	pvr_regs[((FOG_CLAMP_MAX_addr-PVR_BASE)>>2)].data32=&FOG_CLAMP_MAX;
	pvr_regs[((FOG_CLAMP_MAX_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FOG_CLAMP_MAX writeFunction;
	pvr_regs[((FOG_CLAMP_MAX_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FOG_CLAMP_MAX readFunction
	pvr_regs[((FOG_CLAMP_MAX_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FOG_CLAMP_MAX flags

	//FOG_CLAMP_MIN [0x005F80C0:RW] : Color clamping minimum value
	pvr_regs[((FOG_CLAMP_MIN_addr-PVR_BASE)>>2)].data32=&FOG_CLAMP_MIN;
	pvr_regs[((FOG_CLAMP_MIN_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FOG_CLAMP_MIN writeFunction;
	pvr_regs[((FOG_CLAMP_MIN_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FOG_CLAMP_MIN readFunction
	pvr_regs[((FOG_CLAMP_MIN_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FOG_CLAMP_MIN flags

	//SPG_TRIGGER_POS [0x005F80C4:RW] : External trigger signal HV counter value
	pvr_regs[((SPG_TRIGGER_POS_addr-PVR_BASE)>>2)].data32=&SPG_TRIGGER_POS;
	pvr_regs[((SPG_TRIGGER_POS_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SPG_TRIGGER_POS writeFunction;
	pvr_regs[((SPG_TRIGGER_POS_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SPG_TRIGGER_POS readFunction
	pvr_regs[((SPG_TRIGGER_POS_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SPG_TRIGGER_POS flags

	//SPG_HBLANK_INT [0x005F80C8:RW] : H-blank interrupt control
	pvr_regs[((SPG_HBLANK_INT_addr-PVR_BASE)>>2)].data32=&SPG_HBLANK_INT;
	pvr_regs[((SPG_HBLANK_INT_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SPG_HBLANK_INT writeFunction;
	pvr_regs[((SPG_HBLANK_INT_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SPG_HBLANK_INT readFunction
	pvr_regs[((SPG_HBLANK_INT_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SPG_HBLANK_INT flags

	//  Register Name		Area0 Offset		R/W    Short Descritpion
	//SPG_VBLANK_INT [0x005F80CC:RW] : V-blank interrupt control
	pvr_regs[((SPG_VBLANK_INT_addr-PVR_BASE)>>2)].data32=&SPG_VBLANK_INT;
	pvr_regs[((SPG_VBLANK_INT_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SPG_VBLANK_INT writeFunction;
	pvr_regs[((SPG_VBLANK_INT_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SPG_VBLANK_INT readFunction
	pvr_regs[((SPG_VBLANK_INT_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SPG_VBLANK_INT flags

	//SPG_CONTROL [0x005F80D0:RW] : Sync pulse generator control
	pvr_regs[((SPG_CONTROL_addr-PVR_BASE)>>2)].data32=&SPG_CONTROL;
	pvr_regs[((SPG_CONTROL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SPG_CONTROL writeFunction;
	pvr_regs[((SPG_CONTROL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SPG_CONTROL readFunction
	pvr_regs[((SPG_CONTROL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SPG_CONTROL flags

	//SPG_HBLANK [0x005F80D4:RW] : H-blank control
	pvr_regs[((SPG_HBLANK_addr-PVR_BASE)>>2)].data32=&SPG_HBLANK;
	pvr_regs[((SPG_HBLANK_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SPG_HBLANK writeFunction;
	pvr_regs[((SPG_HBLANK_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SPG_HBLANK readFunction
	pvr_regs[((SPG_HBLANK_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SPG_HBLANK flags

	//SPG_LOAD [0x005F80D8:RW] : HV counter load value
	pvr_regs[((SPG_LOAD_addr-PVR_BASE)>>2)].data32=&SPG_LOAD;
	pvr_regs[((SPG_LOAD_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SPG_LOAD writeFunction;
	pvr_regs[((SPG_LOAD_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SPG_LOAD readFunction
	pvr_regs[((SPG_LOAD_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SPG_LOAD flags

	//SPG_VBLANK [0x005F80DC:RW] : V-blank control
	pvr_regs[((SPG_VBLANK_addr-PVR_BASE)>>2)].data32=&SPG_VBLANK;
	pvr_regs[((SPG_VBLANK_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SPG_VBLANK writeFunction;
	pvr_regs[((SPG_VBLANK_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SPG_VBLANK readFunction
	pvr_regs[((SPG_VBLANK_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SPG_VBLANK flags

	//SPG_WIDTH [0x005F80E0:RW] : Sync width control
	pvr_regs[((SPG_WIDTH_addr-PVR_BASE)>>2)].data32=&SPG_WIDTH;
	pvr_regs[((SPG_WIDTH_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SPG_WIDTH writeFunction;
	pvr_regs[((SPG_WIDTH_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SPG_WIDTH readFunction
	pvr_regs[((SPG_WIDTH_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SPG_WIDTH flags

	//TEXT_CONTROL [0x005F80E4:RW] : Texturing control
	pvr_regs[((TEXT_CONTROL_addr-PVR_BASE)>>2)].data32=&TEXT_CONTROL;
	pvr_regs[((TEXT_CONTROL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TEXT_CONTROL writeFunction;
	pvr_regs[((TEXT_CONTROL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TEXT_CONTROL readFunction
	pvr_regs[((TEXT_CONTROL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TEXT_CONTROL flags

	//VO_CONTROL [0x005F80E8:RW] : Video output control
	pvr_regs[((VO_CONTROL_addr-PVR_BASE)>>2)].data32=&VO_CONTROL;
	pvr_regs[((VO_CONTROL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX VO_CONTROL writeFunction;
	pvr_regs[((VO_CONTROL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX VO_CONTROL readFunction
	pvr_regs[((VO_CONTROL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX VO_CONTROL flags

	//VO_STARTX [0x005F80Ec:RW] : Video output start X position
	pvr_regs[((VO_STARTX_addr-PVR_BASE)>>2)].data32=&VO_STARTX;
	pvr_regs[((VO_STARTX_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX VO_STARTX writeFunction;
	pvr_regs[((VO_STARTX_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX VO_STARTX readFunction
	pvr_regs[((VO_STARTX_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX VO_STARTX flags

	//VO_STARTY [0x005F80F0:RW] : Video output start Y position
	pvr_regs[((VO_STARTY_addr-PVR_BASE)>>2)].data32=&VO_STARTY;
	pvr_regs[((VO_STARTY_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX VO_STARTY writeFunction;
	pvr_regs[((VO_STARTY_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX VO_STARTY readFunction
	pvr_regs[((VO_STARTY_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX VO_STARTY flags

	//SCALER_CTL [0x005F80F4:RW] : X & Y scaler control
	pvr_regs[((SCALER_CTL_addr-PVR_BASE)>>2)].data32=&SCALER_CTL;
	pvr_regs[((SCALER_CTL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX SCALER_CTL writeFunction;
	pvr_regs[((SCALER_CTL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX SCALER_CTL readFunction
	pvr_regs[((SCALER_CTL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX SCALER_CTL flags

	//PAL_RAM_CTRL [0x005F8108:RW] : Palette RAM control
	pvr_regs[((PAL_RAM_CTRL_addr-PVR_BASE)>>2)].data32=&PAL_RAM_CTRL;
	pvr_regs[((PAL_RAM_CTRL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX PAL_RAM_CTRL writeFunction;
	pvr_regs[((PAL_RAM_CTRL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX PAL_RAM_CTRL readFunction
	pvr_regs[((PAL_RAM_CTRL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX PAL_RAM_CTRL flags

	//SPG_STATUS [0x005F810C:R] : Sync pulse generator status
	pvr_regs[((SPG_STATUS_addr-PVR_BASE)>>2)].data32=0;//&pSPG_STATUS;
	pvr_regs[((SPG_STATUS_addr-PVR_BASE)>>2)].writeFunction=wh_SPG_STATUS;			//TODO : FIX SPG_STATUS writeFunction;
	pvr_regs[((SPG_STATUS_addr-PVR_BASE)>>2)].readFunction=rh_SPG_STATUS;			//TODO : FIX SPG_STATUS readFunction
	pvr_regs[((SPG_STATUS_addr-PVR_BASE)>>2)].flags=0;						//TODO : FIX SPG_STATUS flags

	//FB_BURSTCTRL [0x005F8110:RW] : Frame buffer burst control
	pvr_regs[((FB_BURSTCTRL_addr-PVR_BASE)>>2)].data32=&FB_BURSTCTRL;
	pvr_regs[((FB_BURSTCTRL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FB_BURSTCTRL writeFunction;
	pvr_regs[((FB_BURSTCTRL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FB_BURSTCTRL readFunction
	pvr_regs[((FB_BURSTCTRL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FB_BURSTCTRL flags

	//FB_C_SOF [0x005F8114:R] : Current frame buffer start address
	pvr_regs[((FB_C_SOF_addr-PVR_BASE)>>2)].data32=&FB_C_SOF;
	pvr_regs[((FB_C_SOF_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FB_C_SOF writeFunction;
	pvr_regs[((FB_C_SOF_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FB_C_SOF readFunction
	pvr_regs[((FB_C_SOF_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FB_C_SOF flags

	//Y_COEFF [0x005F8118:RW] : Y scaling coefficient
	pvr_regs[((Y_COEFF_addr-PVR_BASE)>>2)].data32=&Y_COEFF;
	pvr_regs[((Y_COEFF_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX Y_COEFF writeFunction;
	pvr_regs[((Y_COEFF_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX Y_COEFF readFunction
	pvr_regs[((Y_COEFF_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX Y_COEFF flags

	//PT_ALPHA_REF [0x005F811C:RW] : Alpha value for Punch Through polygon comparison
	pvr_regs[((PT_ALPHA_REF_addr-PVR_BASE)>>2)].data32=&PT_ALPHA_REF;
	pvr_regs[((PT_ALPHA_REF_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX PT_ALPHA_REF writeFunction;
	pvr_regs[((PT_ALPHA_REF_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX PT_ALPHA_REF readFunction
	pvr_regs[((PT_ALPHA_REF_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX PT_ALPHA_REF flags

	//TA block
	//  Register Name		Area0 Offset		R/W    Short Descritpion
	//TA_OL_BASE [0x005F8124:RW] : Object list write start address
	pvr_regs[((TA_OL_BASE_addr-PVR_BASE)>>2)].data32=&TA_OL_BASE;
	pvr_regs[((TA_OL_BASE_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_OL_BASE writeFunction;
	pvr_regs[((TA_OL_BASE_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_OL_BASE readFunction
	pvr_regs[((TA_OL_BASE_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_OL_BASE flags

	//TA_ISP_BASE [0x005F8128:RW] : ISP/TSP Parameter write start address
	pvr_regs[((TA_ISP_BASE_addr-PVR_BASE)>>2)].data32=&TA_ISP_BASE;
	pvr_regs[((TA_ISP_BASE_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_ISP_BASE writeFunction;
	pvr_regs[((TA_ISP_BASE_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_ISP_BASE readFunction
	pvr_regs[((TA_ISP_BASE_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_ISP_BASE flags

	//TA_OL_LIMIT [0x005F812C:RW] : Start address of next Object Pointer Block
	pvr_regs[((TA_OL_LIMIT_addr-PVR_BASE)>>2)].data32=&TA_OL_LIMIT;
	pvr_regs[((TA_OL_LIMIT_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_OL_LIMIT writeFunction;
	pvr_regs[((TA_OL_LIMIT_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_OL_LIMIT readFunction
	pvr_regs[((TA_OL_LIMIT_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_OL_LIMIT flags

	//TA_ISP_LIMIT [0x005F8130:RW] : Current ISP/TSP Parameter write address
	pvr_regs[((TA_ISP_LIMIT_addr-PVR_BASE)>>2)].data32=&TA_ISP_LIMIT;
	pvr_regs[((TA_ISP_LIMIT_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_ISP_LIMIT writeFunction;
	pvr_regs[((TA_ISP_LIMIT_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_ISP_LIMIT readFunction
	pvr_regs[((TA_ISP_LIMIT_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_ISP_LIMIT flags

	//TA_NEXT_OPB [0x005F8134:R] : Global Tile clip control
	pvr_regs[((TA_NEXT_OPB_addr-PVR_BASE)>>2)].data32=&TA_NEXT_OPB;
	pvr_regs[((TA_NEXT_OPB_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_NEXT_OPB writeFunction;
	pvr_regs[((TA_NEXT_OPB_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_NEXT_OPB readFunction
	pvr_regs[((TA_NEXT_OPB_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_NEXT_OPB flags

	//TA_ITP_CURRENT [0x005F8138:R] : Current ISP/TSP Parameter write address
	pvr_regs[((TA_ITP_CURRENT_addr-PVR_BASE)>>2)].data32=&TA_ITP_CURRENT;
	pvr_regs[((TA_ITP_CURRENT_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_ITP_CURRENT writeFunction;
	pvr_regs[((TA_ITP_CURRENT_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_ITP_CURRENT readFunction
	pvr_regs[((TA_ITP_CURRENT_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_ITP_CURRENT flags

	//TA_GLOB_TILE_CLIP [0x005F813C:RW] : Global Tile clip control
	pvr_regs[((TA_GLOB_TILE_CLIP_addr-PVR_BASE)>>2)].data32=&TA_GLOB_TILE_CLIP;
	pvr_regs[((TA_GLOB_TILE_CLIP_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_GLOB_TILE_CLIP writeFunction;
	pvr_regs[((TA_GLOB_TILE_CLIP_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_GLOB_TILE_CLIP readFunction
	pvr_regs[((TA_GLOB_TILE_CLIP_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_GLOB_TILE_CLIP flags

	//TA_ALLOC_CTRL [0x005F8140:RW] : Object list control
	pvr_regs[((TA_ALLOC_CTRL_addr-PVR_BASE)>>2)].data32=&TA_ALLOC_CTRL;
	pvr_regs[((TA_ALLOC_CTRL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_ALLOC_CTRL writeFunction;
	pvr_regs[((TA_ALLOC_CTRL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_ALLOC_CTRL readFunction
	pvr_regs[((TA_ALLOC_CTRL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_ALLOC_CTRL flags

	//TA_LIST_INIT [0x005F8144:RW] : TA initialization
	pvr_regs[((TA_LIST_INIT_addr-PVR_BASE)>>2)].data32=&TA_LIST_INIT;
	pvr_regs[((TA_LIST_INIT_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_LIST_INIT writeFunction;
	pvr_regs[((TA_LIST_INIT_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_LIST_INIT readFunction
	pvr_regs[((TA_LIST_INIT_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_LIST_INIT flags

	//TA_YUV_TEX_BASE [0x005F8148:RW] : YUV422 texture write start address
	pvr_regs[((TA_YUV_TEX_BASE_addr-PVR_BASE)>>2)].data32=&TA_YUV_TEX_BASE;
	pvr_regs[((TA_YUV_TEX_BASE_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_YUV_TEX_BASE writeFunction;
	pvr_regs[((TA_YUV_TEX_BASE_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_YUV_TEX_BASE readFunction
	pvr_regs[((TA_YUV_TEX_BASE_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_YUV_TEX_BASE flags

	//TA_YUV_TEX_CTRL [0x005F814C:RW] : YUV converter control
	pvr_regs[((TA_YUV_TEX_CTRL_addr-PVR_BASE)>>2)].data32=&TA_YUV_TEX_CTRL;
	pvr_regs[((TA_YUV_TEX_CTRL_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_YUV_TEX_CTRL writeFunction;
	pvr_regs[((TA_YUV_TEX_CTRL_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_YUV_TEX_CTRL readFunction
	pvr_regs[((TA_YUV_TEX_CTRL_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_YUV_TEX_CTRL flags

	//TA_YUV_TEX_CNT [0x005F8150:R] : YUV converter macro block counter value
	pvr_regs[((TA_YUV_TEX_CNT_addr-PVR_BASE)>>2)].data32=&TA_YUV_TEX_CNT;
	pvr_regs[((TA_YUV_TEX_CNT_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_YUV_TEX_CNT writeFunction;
	pvr_regs[((TA_YUV_TEX_CNT_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_YUV_TEX_CNT readFunction
	pvr_regs[((TA_YUV_TEX_CNT_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_YUV_TEX_CNT flags

	//TA_LIST_CONT [0x005F8160:RW] : TA continuation processing
	pvr_regs[((TA_LIST_CONT_addr-PVR_BASE)>>2)].data32=&TA_LIST_CONT;
	pvr_regs[((TA_LIST_CONT_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_LIST_CONT writeFunction;
	pvr_regs[((TA_LIST_CONT_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_LIST_CONT readFunction
	pvr_regs[((TA_LIST_CONT_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_LIST_CONT flags

	//TA_NEXT_OPB_INIT [0x005F8164:RW] : Additional OPB starting address
	pvr_regs[((TA_NEXT_OPB_INIT_addr-PVR_BASE)>>2)].data32=&TA_NEXT_OPB_INIT;
	pvr_regs[((TA_NEXT_OPB_INIT_addr-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_NEXT_OPB_INIT writeFunction;
	pvr_regs[((TA_NEXT_OPB_INIT_addr-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_NEXT_OPB_INIT readFunction
	pvr_regs[((TA_NEXT_OPB_INIT_addr-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_NEXT_OPB_INIT flags

	//TODO : DO THESE
	for (u32 i =0;i<128;i++)
	{
		//FOG_TABLE[128] [0x005F8200:RW] : Look-up table Fog data
		pvr_regs[((FOG_TABLE_BASE_addr+i*4-PVR_BASE)>>2)].data32=&FOG_TABLE[i];
		pvr_regs[((FOG_TABLE_BASE_addr+i*4-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX FOG_TABLE[128] writeFunction;
		pvr_regs[((FOG_TABLE_BASE_addr+i*4-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX FOG_TABLE[128] readFunction
		pvr_regs[((FOG_TABLE_BASE_addr+i*4-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX FOG_TABLE[128] flags
	}
	//u32 pFOG_TABLE_END;		//0x005F83FC		RW	Look-up table Fog data

	for (u32 i =0;i<600;i++)
	{
		//  Register Name		Area0 Offset		R/W    Short Descritpion
		//TA_OL_POINTERS[600] [0x005F8600:R] : TA object List Pointer data
		pvr_regs[((TA_OL_POINTERS_BASE_addr+i*4-PVR_BASE)>>2)].data32=&TA_OL_POINTERS[i];
		pvr_regs[((TA_OL_POINTERS_BASE_addr+i*4-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX TA_OL_POINTERS[600] writeFunction;
		pvr_regs[((TA_OL_POINTERS_BASE_addr+i*4-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX TA_OL_POINTERS[600] readFunction
		pvr_regs[((TA_OL_POINTERS_BASE_addr+i*4-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX TA_OL_POINTERS[600] flags
	}
	for (u32 i =0;i<1024;i++)
	{
		//u32 pTA_OL_POINTERS_END;	//0x005F8F5C	R		TA object List Pointer data	
		//PALETTE_RAM[1024] [0x005F9000:RW] : Palette RAM
		pvr_regs[((PALETTE_RAM_BASE_addr+i*4-PVR_BASE)>>2)].data32=&PALETTE_RAM[i];
		pvr_regs[((PALETTE_RAM_BASE_addr+i*4-PVR_BASE)>>2)].writeFunction=0;			//TODO : FIX PALETTE_RAM[1024] writeFunction;
		pvr_regs[((PALETTE_RAM_BASE_addr+i*4-PVR_BASE)>>2)].readFunction=0;			//TODO : FIX PALETTE_RAM[1024] readFunction
		pvr_regs[((PALETTE_RAM_BASE_addr+i*4-PVR_BASE)>>2)].flags=REG_READ_DATA|REG_WRITE_DATA;	//TODO : FIX PALETTE_RAM[1024] flags
	}
	//u32 pPALETTE_RAM_END;	//0x005F9FFC		RW	Palette RAM

	REVISION=0x0011;
}
//Set to defualt [boot] values
void pvr_mmr_Reset()
{
	//TODO : pvr_mmr_Reset
}
//Free any allocated resources
void pvr_mmr_Term()
{
	//nothing
}