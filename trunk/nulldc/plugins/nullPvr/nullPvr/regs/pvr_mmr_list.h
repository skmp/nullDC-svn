//Complete list of TA / PowerVR registers
//This list includes all registers with their names , as well as a short description

#define PVR_BASE 0x005F8000

//CORE block
//      Register Name		Area0 Offset     R/W    Short Descritpion
#define ID_addr					0x005F8000		//R		Device ID
#define REVISION_addr			0x005F8004		//R		Revision number
#define SOFTRESET_addr			0x005F8008		//RW	CORE & TA software reset
#define STARTRENDER_addr			0x005F8014		//RW	Drawing start
#define TEST_SELECT_addr			0x005F8018		//RW	Test (writing this register is prohibited)
#define PARAM_BASE_addr			0x005F8020		//RW	Base address for ISP parameters
#define REGION_BASE_addr			0x005F802C		//RW	Base address for Region Array
#define SPAN_SORT_CFG_addr		0x005F8030		//RW	Span Sorter control			
#define VO_BORDER_COL_addr		0x005F8040		//RW	Border area color
#define FB_R_CTRL_addr			0x005F8044		//RW	Frame buffer read control
#define FB_W_CTRL_addr			0x005F8048		//RW	Frame buffer write control
#define FB_W_LINESTRIDE_addr		0x005F804C		//RW	Frame buffer line stride
#define FB_R_SOF1_addr			0x005F8050		//RW	Read start address for field - 1/strip - 1
#define FB_R_SOF2_addr			0x005F8054		//RW	Read start address for field - 2/strip - 2
#define FB_R_SIZE_addr			0x005F805C		//RW	Frame buffer XY size
#define FB_W_SOF1_addr			0x005F8060		//RW	Write start address for field - 1/strip - 1
#define FB_W_SOF2_addr			0x005F8064		//RW	Write start address for field - 2/strip - 2
#define FB_X_CLIP_addr			0x005F8068		//RW	Pixel clip X coordinate
#define FB_Y_CLIP_addr			0x005F806C		//RW	Pixel clip Y coordinate
//      Register Name		Area0 Offset     R/W    Short Descritpion
#define FPU_SHAD_SCALE_addr		0x005F8074		//RW	Intensity Volume mode
#define FPU_CULL_VAL_addr		0x005F8078		//RW	Comparison value for culling
#define FPU_PARAM_CFG_addr		0x005F807C		//RW	Parameter read control
#define HALF_OFFSET_addr			0x005F8080		//RW	Pixel sampling control
#define FPU_PERP_VAL_addr		0x005F8084		//RW	Comparison value for perpendicular polygons
#define ISP_BACKGND_D_addr		0x005F8088		//RW	Background surface depth
#define ISP_BACKGND_T_addr		0x005F808C		//RW	Background surface tag
#define ISP_FEED_CFG_addr		0x005F8098		//RW	Translucent polygon sort mode
#define SDRAM_REFRESH_addr		0x005F80A0		//RW	Texture memory refresh counter
#define SDRAM_ARB_CFG_addr		0x005F80A4		//RW	Texture memory arbiter control
#define SDRAM_CFG_addr			0x005F80A8		//RW	Texture memory control		
#define FOG_COL_RAM_addr			0x005F80B0		//RW	Color for Look Up table Fog
#define FOG_COL_VERT_addr		0x005F80B4		//RW	Color for vertex Fog
#define FOG_DENSITY_addr			0x005F80B8		//RW	Fog scale value
#define FOG_CLAMP_MAX_addr		0x005F80BC		//RW	Color clamping maximum value
#define FOG_CLAMP_MIN_addr		0x005F80C0		//RW	Color clamping minimum value
#define SPG_TRIGGER_POS_addr		0x005F80C4		//RW	External trigger signal HV counter value
#define SPG_HBLANK_INT_addr		0x005F80C8		//RW	H-blank interrupt control
//      Register Name		Area0 Offset     R/W    Short Descritpion
#define SPG_VBLANK_INT_addr		0x005F80CC		//RW	V-blank interrupt control
#define SPG_CONTROL_addr			0x005F80D0		//RW	Sync pulse generator control
#define SPG_HBLANK_addr			0x005F80D4		//RW	H-blank control
#define SPG_LOAD_addr			0x005F80D8		//RW	HV counter load value
#define SPG_VBLANK_addr			0x005F80DC		//RW	V-blank control
#define SPG_WIDTH_addr			0x005F80E0		//RW	Sync width control
#define TEXT_CONTROL_addr		0x005F80E4		//RW	Texturing control
#define VO_CONTROL_addr			0x005F80E8		//RW	Video output control
#define VO_STARTX_addr			0x005F80Ec		//RW	Video output start X position
#define VO_STARTY_addr			0x005F80F0		//RW	Video output start Y position
#define SCALER_CTL_addr			0x005F80F4		//RW	X & Y scaler control
#define PAL_RAM_CTRL_addr		0x005F8108		//RW	Palette RAM control
#define SPG_STATUS_addr			0x005F810C		//R		Sync pulse generator status
#define FB_BURSTCTRL_addr		0x005F8110		//RW	Frame buffer burst control
#define FB_C_SOF_addr			0x005F8114		//R		Current frame buffer start address
#define Y_COEFF_addr				0x005F8118		//RW	Y scaling coefficient
#define PT_ALPHA_REF_addr		0x005F811C		//RW	Alpha value for Punch Through polygon comparison
//TA block
//      Register Name		Area0 Offset     R/W    Short Descritpion
#define TA_OL_BASE_addr			0x005F8124		//RW	Object list write start address
#define TA_ISP_BASE_addr			0x005F8128		//RW	ISP/TSP Parameter write start address
#define TA_OL_LIMIT_addr			0x005F812C		//RW	Start address of next Object Pointer Block
#define TA_ISP_LIMIT_addr		0x005F8130		//RW	Current ISP/TSP Parameter write address
#define TA_NEXT_OPB_addr			0x005F8134		//R		Global Tile clip control
#define TA_ITP_CURRENT_addr		0x005F8138		//R		Current ISP/TSP Parameter write address
#define TA_GLOB_TILE_CLIP_addr	0x005F813C		//RW	Global Tile clip control
#define TA_ALLOC_CTRL_addr		0x005F8140		//RW	Object list control
#define TA_LIST_INIT_addr		0x005F8144		//RW	TA initialization
#define TA_YUV_TEX_BASE_addr		0x005F8148		//RW	YUV422 texture write start address
#define TA_YUV_TEX_CTRL_addr		0x005F814C		//RW	YUV converter control
#define TA_YUV_TEX_CNT_addr		0x005F8150		//R		YUV converter macro block counter value
#define TA_LIST_CONT_addr		0x005F8160		//RW	TA continuation processing
#define TA_NEXT_OPB_INIT_addr	0x005F8164		//RW	Additional OPB starting address

#define FOG_TABLE_BASE_addr		0x005F8200		//RW	Look-up table Fog data
#define FOG_TABLE_END_addr		0x005F83FC		//RW	Look-up table Fog data
//      Register Name		Area0 Offset     R/W    Short Descritpion
#define TA_OL_POINTERS_BASE_addr	0x005F8600		//R		TA object List Pointer data
#define TA_OL_POINTERS_END_addr	0x005F8F5C		//R		TA object List Pointer data	
#define PALETTE_RAM_BASE_addr	0x005F9000		//RW	Palette RAM
#define PALETTE_RAM_END_addr		0x005F9FFC		//RW	Palette RAM
			
