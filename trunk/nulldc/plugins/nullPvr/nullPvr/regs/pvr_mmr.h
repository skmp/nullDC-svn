#include "..\nullPvr.h"
#include "pvr_mmr_list.h"

//TODO : Create structs for all PowerVR/TA registers

//To create simple structs from #defines
//Generated from "pvr_mmr_list.h" using :RegEx{
//Find		= "\#define {[^\t]+}{[\t]+}{[^]*$}"
//Replace	= "struct \1_Type\2//\3\n{\nu32 data;\n};"
//}

//To create dummy unions/structs from simple structs
//RegEx2{
//Find		= "	u32 data;"
//Replace	= "union\n{\nstruct //Bit field\n{\nu32 bit_1 :1;\n};\nu32 full;//Full 32 bits\n};"
//}

//PowerVR Registers
//     Register Name		Area0 Offset		R/W    Short Descritpion
struct SB_PDSTAP_Type		//0x005F7C00		 RW		PVR-DMA PVR start address
{
	//bit 31-29	27-5							4-0
	//    000	PVR-DMA start address on PVR	Reserved

	union
	{
		struct //Bit field
		{
			u32 res_1:5;			//Reserved
			u32 address :23;		//Adress (32b units)
			u32 res_2:3;			//Reserved
		};
		u32 full;//Full 32 bits
	};
};

struct SB_PDSTAR_Type		//0x005F7C04		 RW		PVR-DMA system memory start address
{
	//bit   31-28	27-5									4-0
	//		000		PVR-DMA start address on System Memory	Reserved

	union
	{
		struct //Bit field
		{
			u32 res_1 :5;			//Reserved
			u32 address :23;		//Adress (32b units)
			u32 res_2:3;			//Reserved
		};
		u32 full;//Full 32 bits
	};
};

struct SB_PDLEN_Type		//0x005F7C08		 RW		PVR-DMA length
{
	//bit 31-24		23-5			4-0
	//    Reserved	PVR-DMA Length	Reserved

	union
	{
		struct //Bit field
		{
			u32 res_1  :5;		//Reserved
			u32 length :18;		//Length (32b units)
			u32 res_2  :3;		//Reserved
		};
		u32 full;//Full 32 bits
	};
};

enum SB_PDDIR_enum
{
	ToPvrArea=0,//From system memory to PVR area (default)
    ToSystemMem=1//From PVR area to system memory
};

struct SB_PDDIR_Type		//0x005F7C0C		 RW		PVR-DMA direction
{
	//bit 31-1		0
    //    Reserved	PVR-DMA direction

	union
	{
		struct //Bit field
		{
			SB_PDDIR_enum direction :1;	//PVR-DMA direction
			u32 res_1:30;		//Reserved
		};
		u32 full;//Full 32 bits
	};
};

enum SB_PDTSEL_enum
{
	SoftwareTrigger=0,//Software trigger  (default) The SH4 triggers PVR-DMA manually.  
					  //PVR-DMA can be triggered by writing to the SB_PDST  register.

	HardwareTrigger=1	//Hardware trigger... PVR-DMA is automatically initiated by the interrupts set 
						//in the SB_PDTNRM and SB_PDTEXT registers.  If the interrupts set in both registers 
						//are both generated, initiation is triggered by the interrupt that was generated first.
};
struct SB_PDTSEL_Type		//0x005F7C10		 RW		PVR-DMA trigger select
{
	//bit 31-1		0
	//    Reserved	PVR-DMA trigger selection
	union
	{
		struct //Bit field
		{
			SB_PDTSEL_enum trigger :1;	//PVR-DMA trigger selection
			u32 res_1:30;					//Reserved
		};
		u32 full;//Full 32 bits
	};
};


enum SB_PDEN_enum
{
   DisabePvrDma=0,	//Disabled (default) dma							 {read}
   AbortPvrDma =0,	//Abort PVR-DMA (default) [if dma is on when writen] {write}
   EnablePvrDma=1	//Enable PVR-DMA [if it was disabled]				 {write/read}
};

struct SB_PDEN_Type			//0x005F7C14	    RW	PVR-DMA enable
{
	//bit 31-1		0
	//    Reserved	PVR-DMA enable

	union
	{
		struct //Bit field
		{
			SB_PDEN_enum PvrEnabled :1;	//PVR-DMA trigger selection
			u32 res_1:30;				//Reserved
		};
		u32 full;//Full 32 bits
	};
};

enum SB_PDST_enum
{
   DmaIsNotInProgress=0,//PVR-DMA not in progress. (defualt) {read}
   DmaIsInProgress =1,	//PVR-DMA in progress				 {read}
   StartPvrDma=1		//Start DMA							 {write}
};

struct SB_PDST_Type			//0x005F7C18	    RW	PVR-DMA start
{
	union
	{
		struct //Bit field
		{
			SB_PDST_enum DmaInProgress :1;	//PVR-DMA trigger selection
			u32 res_1:30;					//Reserved
		};
		u32 full;//Full 32 bits
	};
};

#define SB_PDAPRO_SecurityCode (0x6702)
struct SB_PDAPRO_Type		//0x005F7C80		W		PVR-DMA address range
{
	//bit 31-16					15	14-8		7	6-0
	//    security code 0x6702	R	Top address	R	Bottom address

	union
	{
		struct //Bit field
		{
			u32 Bottom :7;			//Bottom Address
			u32 res_1:1;			//Reserved
			u32 Top :7;				//Top Address
			u32 res_2:1;			//Reserved
			u32 SecurityCode:16;	//security code 0x6702 , must be set to 0x6702 in order for the rest values to change
		};
		u32 full;//Full 32 bits
	};
};

//What values ? 0 init ?
typedef u32 SB_PDSTAPD_Type;		//0x005F7CF0		R		PVR-DMA address counter (on Ext)


typedef u32 SB_PDSTARD_Type;	//0x005F7CF4		R		PVR-DMA address counter (on root bus)


typedef u32 SB_PDLEND_Type;	//0x005F7CF8		R		PVR-DMA transfer counter

//CORE block
//     Register Name		Area0 Offset		R/W    Short Descritpion
#define ID_Vendor_ID (0x11DB)
#define ID_Device_ID (0x17FD)
struct ID_Type				//0x005F8000		R		Device ID
{

	//bit 31-16					15-0
	//    Device ID (0x17FD)	Vendor ID (0x11DB)

	union
	{
		struct //Bit field
		{
			u32 VendorID :16;		//Allways 0x11DB
			u32 DeviceID :16;		//Allways 0x17FD
		};
		u32 full;//Full 32 bits
	};
};

//TODO: Fix  REVISION macro
#define REVISION_Chip_Revision(maj,min) (0) 
struct REVISION_Type		//0x005F8004		R		Revision number
{
	//bit 31-16		15-0
	//	  Reserved	Chip Revision

	union
	{
		struct //Bit field
		{
			u32 Chip_Revision :16;
			u32 Reserved:16;
		};
		u32 full;//Full 32 bits
	};
};

struct SOFTRESET_Type		//0x005F8008		 RW			CORE & TA software reset
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct STARTRENDER_Type		//0x005F8014		 RW			Drawing start
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TEST_SELECT_Type		//0x005F8018		 RW			Test (writing this register is prohibited)
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct PARAM_BASE_Type		//0x005F8020		 RW			Base address for ISP parameters
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct REGION_BASE_Type		//0x005F802C		 RW			Base address for Region Array
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SPAN_SORT_CFG_Type	//0x005F8030		 RW			Span Sorter control			
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct VO_BORDER_COL_Type	//0x005F8040		 RW		Border area color
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FB_R_CTRL_Type		//0x005F8044		 RW		Frame buffer read control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FB_W_CTRL_Type		//0x005F8048		 RW		Frame buffer write control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FB_W_LINESTRIDE_Type	//0x005F804C		 RW		Frame buffer line stride
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FB_R_SOF1_Type		//0x005F8050		 RW		Read start address for field - 1/strip - 1
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FB_R_SOF2_Type		//0x005F8054		 RW		Read start address for field - 2/strip - 2
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FB_R_SIZE_Type		//0x005F805C		 RW		Frame buffer XY size
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FB_W_SOF1_Type		//0x005F8060		 RW		Write start address for field - 1/strip - 1
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FB_W_SOF2_Type		//0x005F8064		 RW		Write start address for field - 2/strip - 2
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FB_X_CLIP_Type		//0x005F8068		 RW		Pixel clip X coordinate
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FB_Y_CLIP_Type		//0x005F806C		 RW		Pixel clip Y coordinate
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

//     Register Name		Area0 Offset		R/W    Short Descritpion
struct FPU_SHAD_SCALE_Type	//0x005F8074		 RW		Intensity Volume mode
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FPU_CULL_VAL_Type	//0x005F8078		 RW		Comparison value for culling
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FPU_PARAM_CFG_Type	//0x005F807C		 RW		Parameter read control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct HALF_OFFSET_Type		//0x005F8080		 RW		Pixel sampling control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FPU_PERP_VAL_Type	//0x005F8084		 RW		Comparison value for perpendicular polygons
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct ISP_BACKGND_D_Type	//0x005F8088		 RW		Background surface depth
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct ISP_BACKGND_T_Type	//0x005F808C		 RW		Background surface tag
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct ISP_FEED_CFG_Type	//0x005F8098		 RW		Translucent polygon sort mode
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SDRAM_REFRESH_Type	//0x005F80A0		 RW		Texture memory refresh counter
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SDRAM_ARB_CFG_Type	//0x005F80A4		 RW		Texture memory arbiter control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SDRAM_CFG_Type		//0x005F80A8		 RW		Texture memory control		
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FOG_COL_RAM_Type		//0x005F80B0		 RW		Color for Look Up table Fog
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FOG_COL_VERT_Type	//0x005F80B4		 RW	Color for vertex Fog
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FOG_DENSITY_Type		//0x005F80B8		 RW	Fog scale value
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FOG_CLAMP_MAX_Type	//0x005F80BC		 RW	Color clamping maximum value
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FOG_CLAMP_MIN_Type	//0x005F80C0		 RW	Color clamping minimum value
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SPG_TRIGGER_POS_Type	//0x005F80C4		 RW	External trigger signal HV counter value
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SPG_HBLANK_INT_Type	//0x005F80C8		 RW	H-blank interrupt control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

//     Register Name		Area0 Offset		 R/W    Short Descritpion
struct SPG_VBLANK_INT_Type	//0x005F80CC		 RW	V-blank interrupt control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SPG_CONTROL_Type		//0x005F80D0		 RW	Sync pulse generator control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SPG_HBLANK_Type		//0x005F80D4		 RW	H-blank control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SPG_LOAD_Type		//0x005F80D8		 RW	HV counter load value
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SPG_VBLANK_Type		//0x005F80DC		 RW	V-blank control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SPG_WIDTH_Type		//0x005F80E0		 RW	Sync width control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TEXT_CONTROL_Type	//0x005F80E4		 RW	Texturing control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct VO_CONTROL_Type		//0x005F80E8		 RW	Video output control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct VO_STARTX_Type		//0x005F80Ec		 RW	Video output start X position
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct VO_STARTY_Type		//0x005F80F0		 RW	Video output start Y position
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SCALER_CTL_Type		//0x005F80F4		 RW	X & Y scaler control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct PAL_RAM_CTRL_Type	//0x005F8108		 RW	Palette RAM control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct SPG_STATUS_Type		//0x005F810C		 R		Sync pulse generator status
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FB_BURSTCTRL_Type	//0x005F8110		 RW	Frame buffer burst control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FB_C_SOF_Type		//0x005F8114		 R		Current frame buffer start address
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct Y_COEFF_Type			//0x005F8118		 RW	Y scaling coefficient
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct PT_ALPHA_REF_Type	//0x005F811C		 RW	Alpha value for Punch Through polygon comparison
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

//TA block
//     Register Name		Area0 Offset		 R/W    Short Descritpion
struct TA_OL_BASE_Type		//0x005F8124		 RW		Object list write start address
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_ISP_BASE_Type		//0x005F8128		 RW		ISP/TSP Parameter write start address
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_OL_LIMIT_Type		//0x005F812C		 RW		Start address of next Object Pointer Block
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_ISP_LIMIT_Type	//0x005F8130		 RW		Current ISP/TSP Parameter write address
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_NEXT_OPB_Type		//0x005F8134		 R		Global Tile clip control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_ITP_CURRENT_Type	//0x005F8138		 R		Current ISP/TSP Parameter write address
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_GLOB_TILE_CLIP_Type//0x005F813C		 RW		Global Tile clip control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_ALLOC_CTRL_Type	//0x005F8140		 RW		Object list control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_LIST_INIT_Type	//0x005F8144		 RW		TA initialization
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_YUV_TEX_BASE_Type	//0x005F8148		 RW		YUV422 texture write start address
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_YUV_TEX_CTRL_Type	//0x005F814C		 RW		YUV converter control
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_YUV_TEX_CNT_Type	//0x005F8150		 R		YUV converter macro block counter value
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_LIST_CONT_Type	//0x005F8160		 RW		TA continuation processing
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_NEXT_OPB_INIT_Type//0x005F8164		 RW		Additional OPB starting address
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

//Needs to be edited by hand
struct FOG_TABLE_BASE_Type	//0x005F8200		 RW		Look-up table Fog data
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct FOG_TABLE_END_Type	//0x005F83FC		 RW		Look-up table Fog data
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

//     Register Name			Area0 Offset     R/W    Short Descritpion
struct TA_OL_POINTERS_BASE_Type	//0x005F8600	 R		TA object List Pointer data
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct TA_OL_POINTERS_END_Type	//0x005F8F5C	 R		TA object List Pointer data	
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct PALETTE_RAM_BASE_Type	//0x005F9000	 RW		Palette RAM
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};

struct PALETTE_RAM_END_Type		//0x005F9FFC	 RW		Palette RAM
{
	union
	{
		struct //Bit field
		{
			u32 bit_1 :1;
		};
		u32 full;//Full 32 bits
	};
};


void pvr_mmr_Init();
void pvr_mmr_Reset();
void pvr_mmr_Term();