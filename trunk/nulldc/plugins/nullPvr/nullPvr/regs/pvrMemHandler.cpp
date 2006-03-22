#include "..\nullPvr.h"
#include "pvr_mmr.h"
//TODO : move to plugin
//TODO : move to new code , and regenerate stubs/ hand tune code afterwards to move to new idea

/*
New idea : a map for read and a map for write
-> using 1 arrays
struct RegisterStruct
{
	u32 data;					//stores data of reg variable [if used]
	RegReadFP* readFunction;	//stored pointer to reg read function
	RegWriteFP* writeFunction;	//stored pointer to reg write function
	RegChangeFP* NextCange;		//stored pointer to reg change calculation function
	u32 flags;					//flags for read/write
}

//Descrition
u32 NextChange()-> return the next change of the registers , cpu tick delta

u32 flags	-> A combination of these:

Basic :
REG_READ_DATA		we can read the data from the data member
REG_WRITE_DATA		we can write the data to the data member
REG_READ_PREDICT	we can call the predict function and know when the register will change
Extended :
REG_WRITE_NOCHANGE	we can rad and write to this register , but the write won't change the value readed 
REG_CONST			register contains constant value

To Read :

if (flags & REG_READ_DATA )
	reddata=data;
else
{
	if (readFunction)
		regdata=readFunction();
	else
		ERROR [write olny register]
}

To predict:

if (flags & REG_READ_PREDICT)
	if (NextCange)
		next_change=NextCange();
	else 
		ERROR [code logic error , NextCange should be provided]
else
	if (flags & REG_CONST)
		next_change=never;//const
	else
		next_change=0;// unkown

To Write:

if (flags & REG_WRITE_DATA)
	data=writedata
else
{
	if (flags & REG_CONST)
		ERROR [read olny register , const]
	else
	{
		if (writeFunction)
			writeFunction(regdata)
		else
			ERROR [read olny register]
	}
}


This could also be the exposed plugin interface for registers witch will give a nice way 
for easy plugin code (using olny callbacks) and for more complex plugins (using pointers and 
precalculation functions).
*/

//Function : Generate null Read/Write code
//RegEx {
//Find		= "\#define {[^\t]+}{[\t]+}{[^\t]+}{[\t]+}{[^]*$}"
//Replace	= "//\1 [\3] \5\n//Read\nPVR_REG_READ(rh_\1)\n{\n\tprintf("\1 : read not implemented\\n"); \n\treturn 0; \n}\n\//Write\nPVR_REG_WRITE(wh_\1)\n{\n\tprintf("\1 : write not implemented, data=%d\\n",data);\n}\n"
//}

//Function : Generate Switches to redirect to Read functions
//RegEx {
//Find		= "\#define {[^\t]+}{[\t]+}{[^\t]+}{[\t]+}{[^]*$}"
//Replace	= "\n\5\ncase \1_addr:\n return plugin_pvr_read_rh_\1(); \n"
//}

//Function : Generate Switches to redirect to Write functions
//RegEx {
//Find		= "\#define {[^\t]+}{[\t]+}{[^\t]+}{[\t]+}{[^]*$}"
//Replace	= "\n\5\ncase \1_addr:\n plugin_pvr_write_wh_\1(data);\n return; \n"
//}



#define PVR_REG_READ(xx) u32 plugin_pvr_read_##xx##()
#define PVR_REG_WRITE(xx) void plugin_pvr_write_##xx##(u32 data)

#define PVR_REG_READ_OFFSET(xx) u32 plugin_pvr_read_##xx##(u32 offset)
#define PVR_REG_WRITE_OFFSET(xx) void plugin_pvr_write_##xx##(u32 offset,u32 data)



//PowerVR Registers
//      Register Name		Area0 Offset     R/W    Short Descritpion
//SB_PDSTAP [0x005F7C00] //RW	PVR-DMA PVR start address
//Read
PVR_REG_READ(rh_SB_PDSTAP)
{
	printf("SB_PDSTAP : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SB_PDSTAP)
{
	printf("SB_PDSTAP : write not implemented, data=%x\n",data);
}

//SB_PDSTAR [0x005F7C04] //RW	PVR-DMA system memory start address
//Read
PVR_REG_READ(rh_SB_PDSTAR)
{
	printf("SB_PDSTAR : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SB_PDSTAR)
{
	printf("SB_PDSTAR : write not implemented, data=%x\n",data);
}

//SB_PDLEN [0x005F7C08] //RW	PVR-DMA length
//Read
PVR_REG_READ(rh_SB_PDLEN)
{
	printf("SB_PDLEN : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SB_PDLEN)
{
	printf("SB_PDLEN : write not implemented, data=%x\n",data);
}

//SB_PDDIR [0x005F7C0C] //RW	PVR-DMA direction
//Read
PVR_REG_READ(rh_SB_PDDIR)
{
	printf("SB_PDDIR : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SB_PDDIR)
{
	printf("SB_PDDIR : write not implemented, data=%x\n",data);
}

//SB_PDTSEL [0x005F7C10] //RW	PVR-DMA trigger select
//Read
PVR_REG_READ(rh_SB_PDTSEL)
{
	printf("SB_PDTSEL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SB_PDTSEL)
{
	printf("SB_PDTSEL : write not implemented, data=%x\n",data);
}

//SB_PDEN [0x005F7C14]     //RW	PVR-DMA enable
//Read
PVR_REG_READ(rh_SB_PDEN)
{
	printf("SB_PDEN : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SB_PDEN)
{
	printf("SB_PDEN : write not implemented, data=%x\n",data);
}

//SB_PDST [0x005F7C18]     //RW	PVR-DMA start
//Read
PVR_REG_READ(rh_SB_PDST)
{
	printf("SB_PDST : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SB_PDST)
{
	printf("SB_PDST : write not implemented, data=%x\n",data);
}

//SB_PDAPRO [0x005F7C80] //W		PVR-DMA address range
//Read
PVR_REG_READ(rh_SB_PDAPRO)
{
	printf("SB_PDAPRO : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SB_PDAPRO)
{
	printf("SB_PDAPRO : write not implemented, data=%x\n",data);
}

//SB_PDSTAPD [0x005F7CF0] //R		PVR-DMA address counter (on Ext)
//Read
PVR_REG_READ(rh_SB_PDSTAPD)
{
	printf("SB_PDSTAPD : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SB_PDSTAPD)
{
	printf("SB_PDSTAPD : write not implemented, data=%x\n",data);
}

//SB_PDSTARD [0x005F7CF4] //R		PVR-DMA address counter (on root bus)
//Read
PVR_REG_READ(rh_SB_PDSTARD)
{
	printf("SB_PDSTARD : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SB_PDSTARD)
{
	printf("SB_PDSTARD : write not implemented, data=%x\n",data);
}

//SB_PDLEND [0x005F7CF8] //R		PVR-DMA transfer counter
//Read
PVR_REG_READ(rh_SB_PDLEND)
{
	printf("SB_PDLEND : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SB_PDLEND)
{
	printf("SB_PDLEND : write not implemented, data=%x\n",data);
}


//CORE block
//      Register Name		Area0 Offset     R/W    Short Descritpion
//ID [0x005F8000] //R		Device ID
//Read
PVR_REG_READ(rh_ID)
{
	printf("ID : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_ID)
{
	printf("ID : write not implemented, data=%x\n",data);
}

//REVISION [0x005F8004] //R		Revision number
//Read
PVR_REG_READ(rh_REVISION)
{
	printf("REVISION : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_REVISION)
{
	printf("REVISION : write not implemented, data=%x\n",data);
}

//SOFTRESET [0x005F8008] //RW	CORE & TA software reset
//Read
PVR_REG_READ(rh_SOFTRESET)
{
	printf("SOFTRESET : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SOFTRESET)
{
	printf("SOFTRESET : write not implemented, data=%x\n",data);
}

//STARTRENDER [0x005F8014] //RW	Drawing start
//Read
PVR_REG_READ(rh_STARTRENDER)
{
	printf("STARTRENDER : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_STARTRENDER)
{
	printf("STARTRENDER : write not implemented, data=%x\n",data);
}

//TEST_SELECT [0x005F8018] //RW	Test (writing this register is prohibited)
//Read
PVR_REG_READ(rh_TEST_SELECT)
{
	printf("TEST_SELECT : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TEST_SELECT)
{
	printf("TEST_SELECT : write not implemented, data=%x\n",data);
}

//PARAM_BASE [0x005F8020] //RW	Base address for ISP parameters
//Read
PVR_REG_READ(rh_PARAM_BASE)
{
	printf("PARAM_BASE : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_PARAM_BASE)
{
	printf("PARAM_BASE : write not implemented, data=%x\n",data);
}

//REGION_BASE [0x005F802C] //RW	Base address for Region Array
//Read
PVR_REG_READ(rh_REGION_BASE)
{
	printf("REGION_BASE : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_REGION_BASE)
{
	printf("REGION_BASE : write not implemented, data=%x\n",data);
}

//SPAN_SORT_CFG [0x005F8030] //RW	Span Sorter control			
//Read
PVR_REG_READ(rh_SPAN_SORT_CFG)
{
	printf("SPAN_SORT_CFG : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SPAN_SORT_CFG)
{
	printf("SPAN_SORT_CFG : write not implemented, data=%x\n",data);
}

//VO_BORDER_COL [0x005F8040] //RW	Border area color
//Read
PVR_REG_READ(rh_VO_BORDER_COL)
{
	printf("VO_BORDER_COL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_VO_BORDER_COL)
{
	printf("VO_BORDER_COL : write not implemented, data=%x\n",data);
}

//FB_R_CTRL [0x005F8044] //RW	Frame buffer read control
//Read
PVR_REG_READ(rh_FB_R_CTRL)
{
	printf("FB_R_CTRL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FB_R_CTRL)
{
	printf("FB_R_CTRL : write not implemented, data=%x\n",data);
}

//FB_W_CTRL [0x005F8048] //RW	Frame buffer write control
//Read
PVR_REG_READ(rh_FB_W_CTRL)
{
	printf("FB_W_CTRL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FB_W_CTRL)
{
	printf("FB_W_CTRL : write not implemented, data=%x\n",data);
}

//FB_W_LINESTRIDE [0x005F804C] //RW	Frame buffer line stride
//Read
PVR_REG_READ(rh_FB_W_LINESTRIDE)
{
	printf("FB_W_LINESTRIDE : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FB_W_LINESTRIDE)
{
	printf("FB_W_LINESTRIDE : write not implemented, data=%x\n",data);
}

//FB_R_SOF1 [0x005F8050] //RW	Read start address for field - 1/strip - 1
//Read
PVR_REG_READ(rh_FB_R_SOF1)
{
	printf("FB_R_SOF1 : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FB_R_SOF1)
{
	printf("FB_R_SOF1 : write not implemented, data=%x\n",data);
}

//FB_R_SOF2 [0x005F8054] //RW	Read start address for field - 2/strip - 2
//Read
PVR_REG_READ(rh_FB_R_SOF2)
{
	printf("FB_R_SOF2 : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FB_R_SOF2)
{
	printf("FB_R_SOF2 : write not implemented, data=%x\n",data);
}

//FB_R_SIZE [0x005F805C] //RW	Frame buffer XY size
//Read
PVR_REG_READ(rh_FB_R_SIZE)
{
	printf("FB_R_SIZE : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FB_R_SIZE)
{
	printf("FB_R_SIZE : write not implemented, data=%x\n",data);
}

//FB_W_SOF1 [0x005F8060] //RW	Write start address for field - 1/strip - 1
//Read
PVR_REG_READ(rh_FB_W_SOF1)
{
	printf("FB_W_SOF1 : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FB_W_SOF1)
{
	printf("FB_W_SOF1 : write not implemented, data=%x\n",data);
}

//FB_W_SOF2 [0x005F8064] //RW	Write start address for field - 2/strip - 2
//Read
PVR_REG_READ(rh_FB_W_SOF2)
{
	printf("FB_W_SOF2 : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FB_W_SOF2)
{
	printf("FB_W_SOF2 : write not implemented, data=%x\n",data);
}

//FB_X_CLIP [0x005F8068] //RW	Pixel clip X coordinate
//Read
PVR_REG_READ(rh_FB_X_CLIP)
{
	printf("FB_X_CLIP : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FB_X_CLIP)
{
	printf("FB_X_CLIP : write not implemented, data=%x\n",data);
}

//FB_Y_CLIP [0x005F806C] //RW	Pixel clip Y coordinate
//Read
PVR_REG_READ(rh_FB_Y_CLIP)
{
	printf("FB_Y_CLIP : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FB_Y_CLIP)
{
	printf("FB_Y_CLIP : write not implemented, data=%x\n",data);
}

//      Register Name		Area0 Offset     R/W    Short Descritpion
//FPU_SHAD_SCALE [0x005F8074] //RW	Intensity Volume mode
//Read
PVR_REG_READ(rh_FPU_SHAD_SCALE)
{
	printf("FPU_SHAD_SCALE : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FPU_SHAD_SCALE)
{
	printf("FPU_SHAD_SCALE : write not implemented, data=%x\n",data);
}

//FPU_CULL_VAL [0x005F8078] //RW	Comparison value for culling
//Read
PVR_REG_READ(rh_FPU_CULL_VAL)
{
	printf("FPU_CULL_VAL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FPU_CULL_VAL)
{
	printf("FPU_CULL_VAL : write not implemented, data=%x\n",data);
}

//FPU_PARAM_CFG [0x005F807C] //RW	Parameter read control
//Read
PVR_REG_READ(rh_FPU_PARAM_CFG)
{
	printf("FPU_PARAM_CFG : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FPU_PARAM_CFG)
{
	printf("FPU_PARAM_CFG : write not implemented, data=%x\n",data);
}

//HALF_OFFSET [0x005F8080] //RW	Pixel sampling control
//Read
PVR_REG_READ(rh_HALF_OFFSET)
{
	printf("HALF_OFFSET : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_HALF_OFFSET)
{
	printf("HALF_OFFSET : write not implemented, data=%x\n",data);
}

//FPU_PERP_VAL [0x005F8084] //RW	Comparison value for perpendicular polygons
//Read
PVR_REG_READ(rh_FPU_PERP_VAL)
{
	printf("FPU_PERP_VAL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FPU_PERP_VAL)
{
	printf("FPU_PERP_VAL : write not implemented, data=%x\n",data);
}

//ISP_BACKGND_D [0x005F8088] //RW	Background surface depth
//Read
PVR_REG_READ(rh_ISP_BACKGND_D)
{
	printf("ISP_BACKGND_D : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_ISP_BACKGND_D)
{
	printf("ISP_BACKGND_D : write not implemented, data=%x\n",data);
}

//ISP_BACKGND_T [0x005F808C] //RW	Background surface tag
//Read
PVR_REG_READ(rh_ISP_BACKGND_T)
{
	printf("ISP_BACKGND_T : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_ISP_BACKGND_T)
{
	printf("ISP_BACKGND_T : write not implemented, data=%x\n",data);
}

//ISP_FEED_CFG [0x005F8098] //RW	Translucent polygon sort mode
//Read
PVR_REG_READ(rh_ISP_FEED_CFG)
{
	printf("ISP_FEED_CFG : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_ISP_FEED_CFG)
{
	printf("ISP_FEED_CFG : write not implemented, data=%x\n",data);
}

//SDRAM_REFRESH [0x005F80A0] //RW	Texture memory refresh counter
//Read
PVR_REG_READ(rh_SDRAM_REFRESH)
{
	printf("SDRAM_REFRESH : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SDRAM_REFRESH)
{
	printf("SDRAM_REFRESH : write not implemented, data=%x\n",data);
}

//SDRAM_ARB_CFG [0x005F80A4] //RW	Texture memory arbiter control
//Read
PVR_REG_READ(rh_SDRAM_ARB_CFG)
{
	printf("SDRAM_ARB_CFG : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SDRAM_ARB_CFG)
{
	printf("SDRAM_ARB_CFG : write not implemented, data=%x\n",data);
}

//SDRAM_CFG [0x005F80A8] //RW	Texture memory control		
//Read
PVR_REG_READ(rh_SDRAM_CFG)
{
	printf("SDRAM_CFG : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SDRAM_CFG)
{
	printf("SDRAM_CFG : write not implemented, data=%x\n",data);
}

//FOG_COL_RAM [0x005F80B0] //RW	Color for Look Up table Fog
//Read
PVR_REG_READ(rh_FOG_COL_RAM)
{
	printf("FOG_COL_RAM : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FOG_COL_RAM)
{
	printf("FOG_COL_RAM : write not implemented, data=%x\n",data);
}

//FOG_COL_VERT [0x005F80B4] //RW	Color for vertex Fog
//Read
PVR_REG_READ(rh_FOG_COL_VERT)
{
	printf("FOG_COL_VERT : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FOG_COL_VERT)
{
	printf("FOG_COL_VERT : write not implemented, data=%x\n",data);
}

//FOG_DENSITY [0x005F80B8] //RW	Fog scale value
//Read
PVR_REG_READ(rh_FOG_DENSITY)
{
	printf("FOG_DENSITY : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FOG_DENSITY)
{
	printf("FOG_DENSITY : write not implemented, data=%x\n",data);
}

//FOG_CLAMP_MAX [0x005F80BC] //RW	Color clamping maximum value
//Read
PVR_REG_READ(rh_FOG_CLAMP_MAX)
{
	printf("FOG_CLAMP_MAX : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FOG_CLAMP_MAX)
{
	printf("FOG_CLAMP_MAX : write not implemented, data=%x\n",data);
}

//FOG_CLAMP_MIN [0x005F80C0] //RW	Color clamping minimum value
//Read
PVR_REG_READ(rh_FOG_CLAMP_MIN)
{
	printf("FOG_CLAMP_MIN : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FOG_CLAMP_MIN)
{
	printf("FOG_CLAMP_MIN : write not implemented, data=%x\n",data);
}

//SPG_TRIGGER_POS [0x005F80C4] //RW	External trigger signal HV counter value
//Read
PVR_REG_READ(rh_SPG_TRIGGER_POS)
{
	printf("SPG_TRIGGER_POS : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SPG_TRIGGER_POS)
{
	printf("SPG_TRIGGER_POS : write not implemented, data=%x\n",data);
}

//SPG_HBLANK_INT [0x005F80C8] //RW	H-blank interrupt control
//Read
PVR_REG_READ(rh_SPG_HBLANK_INT)
{
	printf("SPG_HBLANK_INT : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SPG_HBLANK_INT)
{
	printf("SPG_HBLANK_INT : write not implemented, data=%x\n",data);
}

//      Register Name		Area0 Offset     R/W    Short Descritpion
//SPG_VBLANK_INT [0x005F80CC] //RW	V-blank interrupt control
//Read
PVR_REG_READ(rh_SPG_VBLANK_INT)
{
	printf("SPG_VBLANK_INT : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SPG_VBLANK_INT)
{
	printf("SPG_VBLANK_INT : write not implemented, data=%x\n",data);
}

//SPG_CONTROL [0x005F80D0] //RW	Sync pulse generator control
//Read
PVR_REG_READ(rh_SPG_CONTROL)
{
	printf("SPG_CONTROL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SPG_CONTROL)
{
	printf("SPG_CONTROL : write not implemented, data=%x\n",data);
}

//SPG_HBLANK [0x005F80D4] //RW	H-blank control
//Read
PVR_REG_READ(rh_SPG_HBLANK)
{
	printf("SPG_HBLANK : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SPG_HBLANK)
{
	printf("SPG_HBLANK : write not implemented, data=%x\n",data);
}

//SPG_LOAD [0x005F80D8] //RW	HV counter load value
//Read
PVR_REG_READ(rh_SPG_LOAD)
{
	printf("SPG_LOAD : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SPG_LOAD)
{
	printf("SPG_LOAD : write not implemented, data=%x\n",data);
}

//SPG_VBLANK [0x005F80DC] //RW	V-blank control
//Read
PVR_REG_READ(rh_SPG_VBLANK)
{
	printf("SPG_VBLANK : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SPG_VBLANK)
{
	printf("SPG_VBLANK : write not implemented, data=%x\n",data);
}

//SPG_WIDTH [0x005F80E0] //RW	Sync width control
//Read
PVR_REG_READ(rh_SPG_WIDTH)
{
	printf("SPG_WIDTH : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SPG_WIDTH)
{
	printf("SPG_WIDTH : write not implemented, data=%x\n",data);
}

//TEXT_CONTROL [0x005F80E4] //RW	Texturing control
//Read
PVR_REG_READ(rh_TEXT_CONTROL)
{
	printf("TEXT_CONTROL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TEXT_CONTROL)
{
	printf("TEXT_CONTROL : write not implemented, data=%x\n",data);
}

//VO_CONTROL [0x005F80E8] //RW	Video output control
//Read
PVR_REG_READ(rh_VO_CONTROL)
{
	printf("VO_CONTROL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_VO_CONTROL)
{
	printf("VO_CONTROL : write not implemented, data=%x\n",data);
}

//VO_STARTX [0x005F80Ec] //RW	Video output start X position
//Read
PVR_REG_READ(rh_VO_STARTX)
{
	printf("VO_STARTX : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_VO_STARTX)
{
	printf("VO_STARTX : write not implemented, data=%x\n",data);
}

//VO_STARTY [0x005F80F0] //RW	Video output start Y position
//Read
PVR_REG_READ(rh_VO_STARTY)
{
	printf("VO_STARTY : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_VO_STARTY)
{
	printf("VO_STARTY : write not implemented, data=%x\n",data);
}

//SCALER_CTL [0x005F80F4] //RW	X & Y scaler control
//Read
PVR_REG_READ(rh_SCALER_CTL)
{
	printf("SCALER_CTL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SCALER_CTL)
{
	printf("SCALER_CTL : write not implemented, data=%x\n",data);
}

//PAL_RAM_CTRL [0x005F8108] //RW	Palette RAM control
//Read
PVR_REG_READ(rh_PAL_RAM_CTRL)
{
	printf("PAL_RAM_CTRL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_PAL_RAM_CTRL)
{
	printf("PAL_RAM_CTRL : write not implemented, data=%x\n",data);
}

//SPG_STATUS [0x005F810C] //R		Sync pulse generator status
//Read
PVR_REG_READ(rh_SPG_STATUS)
{
	printf("SPG_STATUS : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_SPG_STATUS)
{
	printf("SPG_STATUS : write not implemented, data=%x\n",data);
}

//FB_BURSTCTRL [0x005F8110] //RW	Frame buffer burst control
//Read
PVR_REG_READ(rh_FB_BURSTCTRL)
{
	printf("FB_BURSTCTRL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FB_BURSTCTRL)
{
	printf("FB_BURSTCTRL : write not implemented, data=%x\n",data);
}

//FB_C_SOF [0x005F8114] //R		Current frame buffer start address
//Read
PVR_REG_READ(rh_FB_C_SOF)
{
	printf("FB_C_SOF : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_FB_C_SOF)
{
	printf("FB_C_SOF : write not implemented, data=%x\n",data);
}

//Y_COEFF [0x005F8118] //RW	Y scaling coefficient
//Read
PVR_REG_READ(rh_Y_COEFF)
{
	printf("Y_COEFF : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_Y_COEFF)
{
	printf("Y_COEFF : write not implemented, data=%x\n",data);
}

//PT_ALPHA_REF [0x005F811C] //RW	Alpha value for Punch Through polygon comparison
//Read
PVR_REG_READ(rh_PT_ALPHA_REF)
{
	printf("PT_ALPHA_REF : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_PT_ALPHA_REF)
{
	printf("PT_ALPHA_REF : write not implemented, data=%x\n",data);
}

//TA block
//      Register Name		Area0 Offset     R/W    Short Descritpion
//TA_OL_BASE [0x005F8124] //RW	Object list write start address
//Read
PVR_REG_READ(rh_TA_OL_BASE)
{
	printf("TA_OL_BASE : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_OL_BASE)
{
	printf("TA_OL_BASE : write not implemented, data=%x\n",data);
}

//TA_ISP_BASE [0x005F8128] //RW	ISP/TSP Parameter write start address
//Read
PVR_REG_READ(rh_TA_ISP_BASE)
{
	printf("TA_ISP_BASE : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_ISP_BASE)
{
	printf("TA_ISP_BASE : write not implemented, data=%x\n",data);
}

//TA_OL_LIMIT [0x005F812C] //RW	Start address of next Object Pointer Block
//Read
PVR_REG_READ(rh_TA_OL_LIMIT)
{
	printf("TA_OL_LIMIT : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_OL_LIMIT)
{
	printf("TA_OL_LIMIT : write not implemented, data=%x\n",data);
}

//TA_ISP_LIMIT [0x005F8130] //RW	Current ISP/TSP Parameter write address
//Read
PVR_REG_READ(rh_TA_ISP_LIMIT)
{
	printf("TA_ISP_LIMIT : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_ISP_LIMIT)
{
	printf("TA_ISP_LIMIT : write not implemented, data=%x\n",data);
}

//TA_NEXT_OPB [0x005F8134] //R		Global Tile clip control
//Read
PVR_REG_READ(rh_TA_NEXT_OPB)
{
	printf("TA_NEXT_OPB : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_NEXT_OPB)
{
	printf("TA_NEXT_OPB : write not implemented, data=%x\n",data);
}

//TA_ITP_CURRENT [0x005F8138] //R		Current ISP/TSP Parameter write address
//Read
PVR_REG_READ(rh_TA_ITP_CURRENT)
{
	printf("TA_ITP_CURRENT : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_ITP_CURRENT)
{
	printf("TA_ITP_CURRENT : write not implemented, data=%x\n",data);
}

//TA_GLOB_TILE_CLIP [0x005F813C] //RW	Global Tile clip control
//Read
PVR_REG_READ(rh_TA_GLOB_TILE_CLIP)
{
	printf("TA_GLOB_TILE_CLIP : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_GLOB_TILE_CLIP)
{
	printf("TA_GLOB_TILE_CLIP : write not implemented, data=%x\n",data);
}

//TA_ALLOC_CTRL [0x005F8140] //RW	Object list control
//Read
PVR_REG_READ(rh_TA_ALLOC_CTRL)
{
	printf("TA_ALLOC_CTRL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_ALLOC_CTRL)
{
	printf("TA_ALLOC_CTRL : write not implemented, data=%x\n",data);
}

//TA_LIST_INIT [0x005F8144] //RW	TA initialization
//Read
PVR_REG_READ(rh_TA_LIST_INIT)
{
	printf("TA_LIST_INIT : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_LIST_INIT)
{
	printf("TA_LIST_INIT : write not implemented, data=%x\n",data);
}

//TA_YUV_TEX_BASE [0x005F8148] //RW	YUV422 texture write start address
//Read
PVR_REG_READ(rh_TA_YUV_TEX_BASE)
{
	printf("TA_YUV_TEX_BASE : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_YUV_TEX_BASE)
{
	printf("TA_YUV_TEX_BASE : write not implemented, data=%x\n",data);
}

//TA_YUV_TEX_CTRL [0x005F814C] //RW	YUV converter control
//Read
PVR_REG_READ(rh_TA_YUV_TEX_CTRL)
{
	printf("TA_YUV_TEX_CTRL : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_YUV_TEX_CTRL)
{
	printf("TA_YUV_TEX_CTRL : write not implemented, data=%x\n",data);
}

//TA_YUV_TEX_CNT [0x005F8150] //R		YUV converter macro block counter value
//Read
PVR_REG_READ(rh_TA_YUV_TEX_CNT)
{
	printf("TA_YUV_TEX_CNT : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_YUV_TEX_CNT)
{
	printf("TA_YUV_TEX_CNT : write not implemented, data=%x\n",data);
}

//TA_LIST_CONT [0x005F8160] //RW	TA continuation processing
//Read
PVR_REG_READ(rh_TA_LIST_CONT)
{
	printf("TA_LIST_CONT : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_LIST_CONT)
{
	printf("TA_LIST_CONT : write not implemented, data=%x\n",data);
}

//TA_NEXT_OPB_INIT [0x005F8164] //RW	Additional OPB starting address
//Read
PVR_REG_READ(rh_TA_NEXT_OPB_INIT)
{
	printf("TA_NEXT_OPB_INIT : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE(wh_TA_NEXT_OPB_INIT)
{
	printf("TA_NEXT_OPB_INIT : write not implemented, data=%x\n",data);
}


//FOG_TABLE_BASE [0x005F8200] //RW	Look-up table Fog data
//To
//FOG_TABLE_END [0x005F83FC] //RW	Look-up table Fog data
//Read
PVR_REG_READ_OFFSET(rh_FOG_TABLE_BASE)
{
	printf("FOG_TABLE_BASE : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE_OFFSET(wh_FOG_TABLE_BASE)
{
	printf("FOG_TABLE_BASE : write not implemented, data=%x\n",data);
}


//      Register Name		Area0 Offset     R/W    Short Descritpion
//TA_OL_POINTERS_BASE [0x005F8600] //R		TA object List Pointer data
//To
//TA_OL_POINTERS_END [0x005F8F5C] //R		TA object List Pointer data	

//Read
PVR_REG_READ_OFFSET(rh_TA_OL_POINTERS_BASE)
{
	printf("TA_OL_POINTERS_BASE : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE_OFFSET(wh_TA_OL_POINTERS_BASE)
{
	printf("TA_OL_POINTERS_BASE : write not implemented , READ OLNY, data=%x\n",data);
}




//PALETTE_RAM_BASE [0x005F9000] //RW	Palette RAM
//To
//PALETTE_RAM_END [0x005F9FFC]  //RW	Palette RAM
//Read
PVR_REG_READ_OFFSET(rh_PALETTE_RAM_BASE)
{
	printf("PALETTE_RAM_BASE : read not implemented\n"); 
	return 0; 
}
//Write
PVR_REG_WRITE_OFFSET(wh_PALETTE_RAM_BASE)
{
	printf("PALETTE_RAM_BASE : write not implemented, data=%x\n",data);
}


//READ - WRITE dispatcher rootines

//(addr>= 0x005F8000) && (addr<=0x005F9FFF)
u32 plugin_pvr_readreg_TA(u32 addr,u32 sz)
{
	//"Arrays"
	//RW	Look-up table Fog data
	if ((addr>=FOG_TABLE_BASE_addr) && (addr<=FOG_TABLE_END_addr))
	{
		return plugin_pvr_read_rh_FOG_TABLE_BASE(addr-FOG_TABLE_BASE_addr);
	}

	//R	TA object List Pointer data
	if ((addr>=TA_OL_POINTERS_BASE_addr) && (addr<=TA_OL_POINTERS_END_addr))
	{
		return plugin_pvr_read_rh_TA_OL_POINTERS_BASE(addr-TA_OL_POINTERS_BASE_addr);
	}

	//RW	Palette RAM
	if ((addr>=PALETTE_RAM_BASE_addr) && (addr<=PALETTE_RAM_END_addr))
	{
		return plugin_pvr_read_rh_PALETTE_RAM_BASE(addr-PALETTE_RAM_BASE_addr);
	}
	//it's a reg !

	switch(addr)
	{
		//R		Device ID
	case ID_addr:
		return plugin_pvr_read_rh_ID(); 


		//R		Revision number
	case REVISION_addr:
		return plugin_pvr_read_rh_REVISION(); 


		//RW	CORE & TA software reset
	case SOFTRESET_addr:
		return plugin_pvr_read_rh_SOFTRESET(); 


		//RW	Drawing start
	case STARTRENDER_addr:
		return plugin_pvr_read_rh_STARTRENDER(); 


		//RW	Test (writing this register is prohibited)
	case TEST_SELECT_addr:
		return plugin_pvr_read_rh_TEST_SELECT(); 


		//RW	Base address for ISP parameters
	case PARAM_BASE_addr:
		return plugin_pvr_read_rh_PARAM_BASE(); 


		//RW	Base address for Region Array
	case REGION_BASE_addr:
		return plugin_pvr_read_rh_REGION_BASE(); 


		//RW	Span Sorter control			
	case SPAN_SORT_CFG_addr:
		return plugin_pvr_read_rh_SPAN_SORT_CFG(); 


		//RW	Border area color
	case VO_BORDER_COL_addr:
		return plugin_pvr_read_rh_VO_BORDER_COL(); 


		//RW	Frame buffer read control
	case FB_R_CTRL_addr:
		return plugin_pvr_read_rh_FB_R_CTRL(); 


		//RW	Frame buffer write control
	case FB_W_CTRL_addr:
		return plugin_pvr_read_rh_FB_W_CTRL(); 


		//RW	Frame buffer line stride
	case FB_W_LINESTRIDE_addr:
		return plugin_pvr_read_rh_FB_W_LINESTRIDE(); 


		//RW	Read start address for field - 1/strip - 1
	case FB_R_SOF1_addr:
		return plugin_pvr_read_rh_FB_R_SOF1(); 


		//RW	Read start address for field - 2/strip - 2
	case FB_R_SOF2_addr:
		return plugin_pvr_read_rh_FB_R_SOF2(); 


		//RW	Frame buffer XY size
	case FB_R_SIZE_addr:
		return plugin_pvr_read_rh_FB_R_SIZE(); 


		//RW	Write start address for field - 1/strip - 1
	case FB_W_SOF1_addr:
		return plugin_pvr_read_rh_FB_W_SOF1(); 


		//RW	Write start address for field - 2/strip - 2
	case FB_W_SOF2_addr:
		return plugin_pvr_read_rh_FB_W_SOF2(); 


		//RW	Pixel clip X coordinate
	case FB_X_CLIP_addr:
		return plugin_pvr_read_rh_FB_X_CLIP(); 


		//RW	Pixel clip Y coordinate
	case FB_Y_CLIP_addr:
		return plugin_pvr_read_rh_FB_Y_CLIP(); 

		//      Register Name		Area0 Offset     R/W    Short Descritpion

		//RW	Intensity Volume mode
	case FPU_SHAD_SCALE_addr:
		return plugin_pvr_read_rh_FPU_SHAD_SCALE(); 


		//RW	Comparison value for culling
	case FPU_CULL_VAL_addr:
		return plugin_pvr_read_rh_FPU_CULL_VAL(); 


		//RW	Parameter read control
	case FPU_PARAM_CFG_addr:
		return plugin_pvr_read_rh_FPU_PARAM_CFG(); 


		//RW	Pixel sampling control
	case HALF_OFFSET_addr:
		return plugin_pvr_read_rh_HALF_OFFSET(); 


		//RW	Comparison value for perpendicular polygons
	case FPU_PERP_VAL_addr:
		return plugin_pvr_read_rh_FPU_PERP_VAL(); 


		//RW	Background surface depth
	case ISP_BACKGND_D_addr:
		return plugin_pvr_read_rh_ISP_BACKGND_D(); 


		//RW	Background surface tag
	case ISP_BACKGND_T_addr:
		return plugin_pvr_read_rh_ISP_BACKGND_T(); 


		//RW	Translucent polygon sort mode
	case ISP_FEED_CFG_addr:
		return plugin_pvr_read_rh_ISP_FEED_CFG(); 


		//RW	Texture memory refresh counter
	case SDRAM_REFRESH_addr:
		return plugin_pvr_read_rh_SDRAM_REFRESH(); 


		//RW	Texture memory arbiter control
	case SDRAM_ARB_CFG_addr:
		return plugin_pvr_read_rh_SDRAM_ARB_CFG(); 


		//RW	Texture memory control		
	case SDRAM_CFG_addr:
		return plugin_pvr_read_rh_SDRAM_CFG(); 


		//RW	Color for Look Up table Fog
	case FOG_COL_RAM_addr:
		return plugin_pvr_read_rh_FOG_COL_RAM(); 


		//RW	Color for vertex Fog
	case FOG_COL_VERT_addr:
		return plugin_pvr_read_rh_FOG_COL_VERT(); 


		//RW	Fog scale value
	case FOG_DENSITY_addr:
		return plugin_pvr_read_rh_FOG_DENSITY(); 


		//RW	Color clamping maximum value
	case FOG_CLAMP_MAX_addr:
		return plugin_pvr_read_rh_FOG_CLAMP_MAX(); 


		//RW	Color clamping minimum value
	case FOG_CLAMP_MIN_addr:
		return plugin_pvr_read_rh_FOG_CLAMP_MIN(); 


		//RW	External trigger signal HV counter value
	case SPG_TRIGGER_POS_addr:
		return plugin_pvr_read_rh_SPG_TRIGGER_POS(); 


		//RW	H-blank interrupt control
	case SPG_HBLANK_INT_addr:
		return plugin_pvr_read_rh_SPG_HBLANK_INT(); 

		//      Register Name		Area0 Offset     R/W    Short Descritpion

		//RW	V-blank interrupt control
	case SPG_VBLANK_INT_addr:
		return plugin_pvr_read_rh_SPG_VBLANK_INT(); 


		//RW	Sync pulse generator control
	case SPG_CONTROL_addr:
		return plugin_pvr_read_rh_SPG_CONTROL(); 


		//RW	H-blank control
	case SPG_HBLANK_addr:
		return plugin_pvr_read_rh_SPG_HBLANK(); 


		//RW	HV counter load value
	case SPG_LOAD_addr:
		return plugin_pvr_read_rh_SPG_LOAD(); 


		//RW	V-blank control
	case SPG_VBLANK_addr:
		return plugin_pvr_read_rh_SPG_VBLANK(); 


		//RW	Sync width control
	case SPG_WIDTH_addr:
		return plugin_pvr_read_rh_SPG_WIDTH(); 


		//RW	Texturing control
	case TEXT_CONTROL_addr:
		return plugin_pvr_read_rh_TEXT_CONTROL(); 


		//RW	Video output control
	case VO_CONTROL_addr:
		return plugin_pvr_read_rh_VO_CONTROL(); 


		//RW	Video output start X position
	case VO_STARTX_addr:
		return plugin_pvr_read_rh_VO_STARTX(); 


		//RW	Video output start Y position
	case VO_STARTY_addr:
		return plugin_pvr_read_rh_VO_STARTY(); 


		//RW	X & Y scaler control
	case SCALER_CTL_addr:
		return plugin_pvr_read_rh_SCALER_CTL(); 


		//RW	Palette RAM control
	case PAL_RAM_CTRL_addr:
		return plugin_pvr_read_rh_PAL_RAM_CTRL(); 


		//R		Sync pulse generator status
	case SPG_STATUS_addr:
		return plugin_pvr_read_rh_SPG_STATUS(); 


		//RW	Frame buffer burst control
	case FB_BURSTCTRL_addr:
		return plugin_pvr_read_rh_FB_BURSTCTRL(); 


		//R		Current frame buffer start address
	case FB_C_SOF_addr:
		return plugin_pvr_read_rh_FB_C_SOF(); 


		//RW	Y scaling coefficient
	case Y_COEFF_addr:
		return plugin_pvr_read_rh_Y_COEFF(); 


		//RW	Alpha value for Punch Through polygon comparison
	case PT_ALPHA_REF_addr:
		return plugin_pvr_read_rh_PT_ALPHA_REF(); 

		//TA block
		//      Register Name		Area0 Offset     R/W    Short Descritpion

		//RW	Object list write start address
	case TA_OL_BASE_addr:
		return plugin_pvr_read_rh_TA_OL_BASE(); 


		//RW	ISP/TSP Parameter write start address
	case TA_ISP_BASE_addr:
		return plugin_pvr_read_rh_TA_ISP_BASE(); 


		//RW	Start address of next Object Pointer Block
	case TA_OL_LIMIT_addr:
		return plugin_pvr_read_rh_TA_OL_LIMIT(); 


		//RW	Current ISP/TSP Parameter write address
	case TA_ISP_LIMIT_addr:
		return plugin_pvr_read_rh_TA_ISP_LIMIT(); 


		//R		Global Tile clip control
	case TA_NEXT_OPB_addr:
		return plugin_pvr_read_rh_TA_NEXT_OPB(); 


		//R		Current ISP/TSP Parameter write address
	case TA_ITP_CURRENT_addr:
		return plugin_pvr_read_rh_TA_ITP_CURRENT(); 


		//RW	Global Tile clip control
	case TA_GLOB_TILE_CLIP_addr:
		return plugin_pvr_read_rh_TA_GLOB_TILE_CLIP(); 


		//RW	Object list control
	case TA_ALLOC_CTRL_addr:
		return plugin_pvr_read_rh_TA_ALLOC_CTRL(); 


		//RW	TA initialization
	case TA_LIST_INIT_addr:
		return plugin_pvr_read_rh_TA_LIST_INIT(); 


		//RW	YUV422 texture write start address
	case TA_YUV_TEX_BASE_addr:
		return plugin_pvr_read_rh_TA_YUV_TEX_BASE(); 


		//RW	YUV converter control
	case TA_YUV_TEX_CTRL_addr:
		return plugin_pvr_read_rh_TA_YUV_TEX_CTRL(); 


		//R		YUV converter macro block counter value
	case TA_YUV_TEX_CNT_addr:
		return plugin_pvr_read_rh_TA_YUV_TEX_CNT(); 


		//RW	TA continuation processing
	case TA_LIST_CONT_addr:
		return plugin_pvr_read_rh_TA_LIST_CONT(); 


		//RW	Additional OPB starting address
	case TA_NEXT_OPB_INIT_addr:
		return plugin_pvr_read_rh_TA_NEXT_OPB_INIT(); 
	}

	EMUERROR3("Not implemented TA register read , addr=%x,sz=%d",addr,sz);
	return 0;
}
//(addr>= 0x005F8000) && (addr<=0x005F9FFF)
void plugin_pvr_writereg_TA(u32 addr,u32 data,u32 sz)
{
	//RW	Look-up table Fog data
	if ((addr>=FOG_TABLE_BASE_addr) && (addr<=FOG_TABLE_END_addr))
	{
		plugin_pvr_write_wh_FOG_TABLE_BASE(addr-FOG_TABLE_BASE_addr,data);
		return;
	}

	//R	TA object List Pointer data
	if ((addr>=TA_OL_POINTERS_BASE_addr) && (addr<=TA_OL_POINTERS_END_addr))
	{
		plugin_pvr_write_wh_TA_OL_POINTERS_BASE(addr-TA_OL_POINTERS_BASE_addr,data);
		return;
	}

	//RW	Palette RAM
	if ((addr>=PALETTE_RAM_BASE_addr) && (addr<=PALETTE_RAM_END_addr))
	{
		plugin_pvr_write_wh_PALETTE_RAM_BASE(addr-PALETTE_RAM_BASE_addr,data);
		return;
	}
	//it's a reg !
	switch(addr)
	{
		//CORE block
		//      Register Name		Area0 Offset     R/W    Short Descritpion

		//R		Device ID
	case ID_addr:
		plugin_pvr_write_wh_ID(data);
		return; 


		//R		Revision number
	case REVISION_addr:
		plugin_pvr_write_wh_REVISION(data);
		return; 


		//RW	CORE & TA software reset
	case SOFTRESET_addr:
		plugin_pvr_write_wh_SOFTRESET(data);
		return; 


		//RW	Drawing start
	case STARTRENDER_addr:
		plugin_pvr_write_wh_STARTRENDER(data);
		return; 


		//RW	Test (writing this register is prohibited)
	case TEST_SELECT_addr:
		plugin_pvr_write_wh_TEST_SELECT(data);
		return; 


		//RW	Base address for ISP parameters
	case PARAM_BASE_addr:
		plugin_pvr_write_wh_PARAM_BASE(data);
		return; 


		//RW	Base address for Region Array
	case REGION_BASE_addr:
		plugin_pvr_write_wh_REGION_BASE(data);
		return; 


		//RW	Span Sorter control			
	case SPAN_SORT_CFG_addr:
		plugin_pvr_write_wh_SPAN_SORT_CFG(data);
		return; 


		//RW	Border area color
	case VO_BORDER_COL_addr:
		plugin_pvr_write_wh_VO_BORDER_COL(data);
		return; 


		//RW	Frame buffer read control
	case FB_R_CTRL_addr:
		plugin_pvr_write_wh_FB_R_CTRL(data);
		return; 


		//RW	Frame buffer write control
	case FB_W_CTRL_addr:
		plugin_pvr_write_wh_FB_W_CTRL(data);
		return; 


		//RW	Frame buffer line stride
	case FB_W_LINESTRIDE_addr:
		plugin_pvr_write_wh_FB_W_LINESTRIDE(data);
		return; 


		//RW	Read start address for field - 1/strip - 1
	case FB_R_SOF1_addr:
		plugin_pvr_write_wh_FB_R_SOF1(data);
		return; 


		//RW	Read start address for field - 2/strip - 2
	case FB_R_SOF2_addr:
		plugin_pvr_write_wh_FB_R_SOF2(data);
		return; 


		//RW	Frame buffer XY size
	case FB_R_SIZE_addr:
		plugin_pvr_write_wh_FB_R_SIZE(data);
		return; 


		//RW	Write start address for field - 1/strip - 1
	case FB_W_SOF1_addr:
		plugin_pvr_write_wh_FB_W_SOF1(data);
		return; 


		//RW	Write start address for field - 2/strip - 2
	case FB_W_SOF2_addr:
		plugin_pvr_write_wh_FB_W_SOF2(data);
		return; 


		//RW	Pixel clip X coordinate
	case FB_X_CLIP_addr:
		plugin_pvr_write_wh_FB_X_CLIP(data);
		return; 


		//RW	Pixel clip Y coordinate
	case FB_Y_CLIP_addr:
		plugin_pvr_write_wh_FB_Y_CLIP(data);
		return; 

		//      Register Name		Area0 Offset     R/W    Short Descritpion

		//RW	Intensity Volume mode
	case FPU_SHAD_SCALE_addr:
		plugin_pvr_write_wh_FPU_SHAD_SCALE(data);
		return; 


		//RW	Comparison value for culling
	case FPU_CULL_VAL_addr:
		plugin_pvr_write_wh_FPU_CULL_VAL(data);
		return; 


		//RW	Parameter read control
	case FPU_PARAM_CFG_addr:
		plugin_pvr_write_wh_FPU_PARAM_CFG(data);
		return; 


		//RW	Pixel sampling control
	case HALF_OFFSET_addr:
		plugin_pvr_write_wh_HALF_OFFSET(data);
		return; 


		//RW	Comparison value for perpendicular polygons
	case FPU_PERP_VAL_addr:
		plugin_pvr_write_wh_FPU_PERP_VAL(data);
		return; 


		//RW	Background surface depth
	case ISP_BACKGND_D_addr:
		plugin_pvr_write_wh_ISP_BACKGND_D(data);
		return; 


		//RW	Background surface tag
	case ISP_BACKGND_T_addr:
		plugin_pvr_write_wh_ISP_BACKGND_T(data);
		return; 


		//RW	Translucent polygon sort mode
	case ISP_FEED_CFG_addr:
		plugin_pvr_write_wh_ISP_FEED_CFG(data);
		return; 


		//RW	Texture memory refresh counter
	case SDRAM_REFRESH_addr:
		plugin_pvr_write_wh_SDRAM_REFRESH(data);
		return; 


		//RW	Texture memory arbiter control
	case SDRAM_ARB_CFG_addr:
		plugin_pvr_write_wh_SDRAM_ARB_CFG(data);
		return; 


		//RW	Texture memory control		
	case SDRAM_CFG_addr:
		plugin_pvr_write_wh_SDRAM_CFG(data);
		return; 


		//RW	Color for Look Up table Fog
	case FOG_COL_RAM_addr:
		plugin_pvr_write_wh_FOG_COL_RAM(data);
		return; 


		//RW	Color for vertex Fog
	case FOG_COL_VERT_addr:
		plugin_pvr_write_wh_FOG_COL_VERT(data);
		return; 


		//RW	Fog scale value
	case FOG_DENSITY_addr:
		plugin_pvr_write_wh_FOG_DENSITY(data);
		return; 


		//RW	Color clamping maximum value
	case FOG_CLAMP_MAX_addr:
		plugin_pvr_write_wh_FOG_CLAMP_MAX(data);
		return; 


		//RW	Color clamping minimum value
	case FOG_CLAMP_MIN_addr:
		plugin_pvr_write_wh_FOG_CLAMP_MIN(data);
		return; 


		//RW	External trigger signal HV counter value
	case SPG_TRIGGER_POS_addr:
		plugin_pvr_write_wh_SPG_TRIGGER_POS(data);
		return; 


		//RW	H-blank interrupt control
	case SPG_HBLANK_INT_addr:
		plugin_pvr_write_wh_SPG_HBLANK_INT(data);
		return; 

		//      Register Name		Area0 Offset     R/W    Short Descritpion

		//RW	V-blank interrupt control
	case SPG_VBLANK_INT_addr:
		plugin_pvr_write_wh_SPG_VBLANK_INT(data);
		return; 


		//RW	Sync pulse generator control
	case SPG_CONTROL_addr:
		plugin_pvr_write_wh_SPG_CONTROL(data);
		return; 


		//RW	H-blank control
	case SPG_HBLANK_addr:
		plugin_pvr_write_wh_SPG_HBLANK(data);
		return; 


		//RW	HV counter load value
	case SPG_LOAD_addr:
		plugin_pvr_write_wh_SPG_LOAD(data);
		return; 


		//RW	V-blank control
	case SPG_VBLANK_addr:
		plugin_pvr_write_wh_SPG_VBLANK(data);
		return; 


		//RW	Sync width control
	case SPG_WIDTH_addr:
		plugin_pvr_write_wh_SPG_WIDTH(data);
		return; 


		//RW	Texturing control
	case TEXT_CONTROL_addr:
		plugin_pvr_write_wh_TEXT_CONTROL(data);
		return; 


		//RW	Video output control
	case VO_CONTROL_addr:
		plugin_pvr_write_wh_VO_CONTROL(data);
		return; 


		//RW	Video output start X position
	case VO_STARTX_addr:
		plugin_pvr_write_wh_VO_STARTX(data);
		return; 


		//RW	Video output start Y position
	case VO_STARTY_addr:
		plugin_pvr_write_wh_VO_STARTY(data);
		return; 


		//RW	X & Y scaler control
	case SCALER_CTL_addr:
		plugin_pvr_write_wh_SCALER_CTL(data);
		return; 


		//RW	Palette RAM control
	case PAL_RAM_CTRL_addr:
		plugin_pvr_write_wh_PAL_RAM_CTRL(data);
		return; 


		//R		Sync pulse generator status
	case SPG_STATUS_addr:
		plugin_pvr_write_wh_SPG_STATUS(data);
		return; 


		//RW	Frame buffer burst control
	case FB_BURSTCTRL_addr:
		plugin_pvr_write_wh_FB_BURSTCTRL(data);
		return; 


		//R		Current frame buffer start address
	case FB_C_SOF_addr:
		plugin_pvr_write_wh_FB_C_SOF(data);
		return; 


		//RW	Y scaling coefficient
	case Y_COEFF_addr:
		plugin_pvr_write_wh_Y_COEFF(data);
		return; 


		//RW	Alpha value for Punch Through polygon comparison
	case PT_ALPHA_REF_addr:
		plugin_pvr_write_wh_PT_ALPHA_REF(data);
		return; 

		//TA block
		//      Register Name		Area0 Offset     R/W    Short Descritpion

		//RW	Object list write start address
	case TA_OL_BASE_addr:
		plugin_pvr_write_wh_TA_OL_BASE(data);
		return; 


		//RW	ISP/TSP Parameter write start address
	case TA_ISP_BASE_addr:
		plugin_pvr_write_wh_TA_ISP_BASE(data);
		return; 


		//RW	Start address of next Object Pointer Block
	case TA_OL_LIMIT_addr:
		plugin_pvr_write_wh_TA_OL_LIMIT(data);
		return; 


		//RW	Current ISP/TSP Parameter write address
	case TA_ISP_LIMIT_addr:
		plugin_pvr_write_wh_TA_ISP_LIMIT(data);
		return; 


		//R		Global Tile clip control
	case TA_NEXT_OPB_addr:
		plugin_pvr_write_wh_TA_NEXT_OPB(data);
		return; 


		//R		Current ISP/TSP Parameter write address
	case TA_ITP_CURRENT_addr:
		plugin_pvr_write_wh_TA_ITP_CURRENT(data);
		return; 


		//RW	Global Tile clip control
	case TA_GLOB_TILE_CLIP_addr:
		plugin_pvr_write_wh_TA_GLOB_TILE_CLIP(data);
		return; 


		//RW	Object list control
	case TA_ALLOC_CTRL_addr:
		plugin_pvr_write_wh_TA_ALLOC_CTRL(data);
		return; 


		//RW	TA initialization
	case TA_LIST_INIT_addr:
		plugin_pvr_write_wh_TA_LIST_INIT(data);
		return; 


		//RW	YUV422 texture write start address
	case TA_YUV_TEX_BASE_addr:
		plugin_pvr_write_wh_TA_YUV_TEX_BASE(data);
		return; 


		//RW	YUV converter control
	case TA_YUV_TEX_CTRL_addr:
		plugin_pvr_write_wh_TA_YUV_TEX_CTRL(data);
		return; 


		//R		YUV converter macro block counter value
	case TA_YUV_TEX_CNT_addr:
		plugin_pvr_write_wh_TA_YUV_TEX_CNT(data);
		return; 


		//RW	TA continuation processing
	case TA_LIST_CONT_addr:
		plugin_pvr_write_wh_TA_LIST_CONT(data);
		return; 


		//RW	Additional OPB starting address
	case TA_NEXT_OPB_INIT_addr:
		plugin_pvr_write_wh_TA_NEXT_OPB_INIT(data);
		return; 
	}



	EMUERROR4("Not implemented TA register write , addr=%x,data=%x,sz=%d",addr,data,sz);
}




