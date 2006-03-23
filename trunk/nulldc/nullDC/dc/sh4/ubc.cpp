#include "types.h"
#include "dc/mem/sh4_internal_reg.h"
#include "ubc.h"

//UBC BARA 0xFF200000 0x1F200000 32 Undefined Held Held Held Iclk
u32 UBC_BARA;
//UBC BAMRA 0xFF200004 0x1F200004 8 Undefined Held Held Held Iclk
u8 UBC_BAMRA;
//UBC BBRA 0xFF200008 0x1F200008 16 0x0000 Held Held Held Iclk
u16 UBC_BBRA;
//UBC BARB 0xFF20000C 0x1F20000C 32 Undefined Held Held Held Iclk
u32 UBC_BARB;
//UBC BAMRB 0xFF200010 0x1F200010 8 Undefined Held Held Held Iclk
u8 UBC_BAMRB;
//UBC BBRB 0xFF200014 0x1F200014 16 0x0000 Held Held Held Iclk
u16 UBC_BBRB;
//UBC BDRB 0xFF200018 0x1F200018 32 Undefined Held Held Held Iclk
u32 UBC_BDRB;
//UBC BDMRB 0xFF20001C 0x1F20001C 32 Undefined Held Held Held Iclk
u32 UBC_BDMRB;
//UBC BRCR 0xFF200020 0x1F200020 16 0x0000 Held Held Held Iclk
u16 UBC_BRCR;

//Init term res
void ubc_Init()
{
	//UBC BARA 0xFF200000 0x1F200000 32 Undefined Held Held Held Iclk
	UBC[(UBC_BARA_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	UBC[(UBC_BARA_addr&0xFF)>>2].NextCange=0;
	UBC[(UBC_BARA_addr&0xFF)>>2].readFunction=0;
	UBC[(UBC_BARA_addr&0xFF)>>2].writeFunction=0;
	UBC[(UBC_BARA_addr&0xFF)>>2].data32=&UBC_BARA;

	//UBC BAMRA 0xFF200004 0x1F200004 8 Undefined Held Held Held Iclk
	UBC[(UBC_BAMRA_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	UBC[(UBC_BAMRA_addr&0xFF)>>2].NextCange=0;
	UBC[(UBC_BAMRA_addr&0xFF)>>2].readFunction=0;
	UBC[(UBC_BAMRA_addr&0xFF)>>2].writeFunction=0;
	UBC[(UBC_BAMRA_addr&0xFF)>>2].data8=&UBC_BAMRA;

	//UBC BBRA 0xFF200008 0x1F200008 16 0x0000 Held Held Held Iclk
	UBC[(UBC_BBRA_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	UBC[(UBC_BBRA_addr&0xFF)>>2].NextCange=0;
	UBC[(UBC_BBRA_addr&0xFF)>>2].readFunction=0;
	UBC[(UBC_BBRA_addr&0xFF)>>2].writeFunction=0;
	UBC[(UBC_BBRA_addr&0xFF)>>2].data16=&UBC_BBRA;

	//UBC BARB 0xFF20000C 0x1F20000C 32 Undefined Held Held Held Iclk
	UBC[(UBC_BARB_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	UBC[(UBC_BARB_addr&0xFF)>>2].NextCange=0;
	UBC[(UBC_BARB_addr&0xFF)>>2].readFunction=0;
	UBC[(UBC_BARB_addr&0xFF)>>2].writeFunction=0;
	UBC[(UBC_BARB_addr&0xFF)>>2].data32=&UBC_BARB;

	//UBC BAMRB 0xFF200010 0x1F200010 8 Undefined Held Held Held Iclk
	UBC[(UBC_BAMRB_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	UBC[(UBC_BAMRB_addr&0xFF)>>2].NextCange=0;
	UBC[(UBC_BAMRB_addr&0xFF)>>2].readFunction=0;
	UBC[(UBC_BAMRB_addr&0xFF)>>2].writeFunction=0;
	UBC[(UBC_BAMRB_addr&0xFF)>>2].data8=&UBC_BAMRB;

	//UBC BBRB 0xFF200014 0x1F200014 16 0x0000 Held Held Held Iclk
	UBC[(UBC_BBRB_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	UBC[(UBC_BBRB_addr&0xFF)>>2].NextCange=0;
	UBC[(UBC_BBRB_addr&0xFF)>>2].readFunction=0;
	UBC[(UBC_BBRB_addr&0xFF)>>2].writeFunction=0;
	UBC[(UBC_BBRB_addr&0xFF)>>2].data16=&UBC_BBRB;

	//UBC BDRB 0xFF200018 0x1F200018 32 Undefined Held Held Held Iclk
	UBC[(UBC_BDRB_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	UBC[(UBC_BDRB_addr&0xFF)>>2].NextCange=0;
	UBC[(UBC_BDRB_addr&0xFF)>>2].readFunction=0;
	UBC[(UBC_BDRB_addr&0xFF)>>2].writeFunction=0;
	UBC[(UBC_BDRB_addr&0xFF)>>2].data32=&UBC_BDRB;

	//UBC BDMRB 0xFF20001C 0x1F20001C 32 Undefined Held Held Held Iclk
	UBC[(UBC_BDMRB_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	UBC[(UBC_BDMRB_addr&0xFF)>>2].NextCange=0;
	UBC[(UBC_BDMRB_addr&0xFF)>>2].readFunction=0;
	UBC[(UBC_BDMRB_addr&0xFF)>>2].writeFunction=0;
	UBC[(UBC_BDMRB_addr&0xFF)>>2].data32=&UBC_BDMRB;

	//UBC BRCR 0xFF200020 0x1F200020 16 0x0000 Held Held Held Iclk
	UBC[(UBC_BRCR_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	UBC[(UBC_BRCR_addr&0xFF)>>2].NextCange=0;
	UBC[(UBC_BRCR_addr&0xFF)>>2].readFunction=0;
	UBC[(UBC_BRCR_addr&0xFF)>>2].writeFunction=0;
	UBC[(UBC_BRCR_addr&0xFF)>>2].data16=&UBC_BRCR;

}
void ubc_Reset(bool Manual)
{
}
void ubc_Term()
{
}
//ubc is disabled on dreamcast and can't be used ... but kos-debug uses it !...