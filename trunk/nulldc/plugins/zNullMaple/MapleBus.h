/*
**	MapleBus.h
*/
#pragma once

#pragma pack(push,1)

/*	Notes
*
If no Device status request is made, the peripherals do not start to operate, but remain in standby status

One peripheral can use a maximum of three functions. Functions are accessed by specifying their respective function type.
One function can be accessed a maximum of one time during one INT, and one frame of data can be sent per access.
One port cannot be continually accessed. (If continuous access in unavoidable, use the NOP instruction.)

Transmission data (including the data format, etc.) is stored in 4 byte units.

(2) Configuration of AP bits
These are configured in one byte.
Bit		7	6	5	4	3	2	1	0
Data	PO1	PO0	D/E	LM4	LM3	LM2	LM1	LM0

When the device responds, the origin AP at the time becomes the value of the sum (all OR)
of the device's own AP and the connected expansion devices' APs.

bit			7		6		5		4		3		2		1		0
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




struct Controller_ReadFormat
{
	union {
		struct {
			unsigned char C:		1;
			unsigned char B:		1;
			unsigned char A:		1;
			unsigned char Start:	1;
			unsigned char Ua:		1;
			unsigned char Da:		1;
			unsigned char La:		1;
			unsigned char Ra:		1;

			unsigned char Z:		1;
			unsigned char Y:		1;
			unsigned char X:		1;
			unsigned char D:		1;
			unsigned char Ub:		1;
			unsigned char Db:		1;
			unsigned char Lb:		1;
			unsigned char Rb:		1;
		};

		unsigned short Buttons;
	};

	union {
		struct {
			signed char Ax1, Ax2, Ax3, Ax4, Ax5, Ax6;
		};
		signed char Av[6];
	};
};












struct DeviceID	// *FIXME*
{
	u32 FT;
	u32 FD[3];
};

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

struct FixedDevStatus
{
	DeviceID DevID;			// 16B Device ID

	u8	DestCode;			// 1B Destination Code
	u8	Direction;			// 1B Connection Direction of Expansion Socket [SD4][SD3][SD2][SD1] 2b

	u8	ProductName[30];	// Product Name 0x20 is space
	u8	License[60];		// License ...

	u16	StandbyCurrent;		// in units of 0.1 mA - 10.5 mA is designated by 00-69h.
	u16	MaximumCurrent;		// in units of 0.1 mA - 127.9 mA is designated by 04-FFh.

} ;

// items should be separeted by a ',' spaces are 0x20 


struct FreeDevStatus
{
	// first 40 bytes should conform
	// "Version 1.000,1998/05/11,315-6125-AB". 
	// "Analog Module: The 4th Edition. 05/08"
	u8 Version[14];			// 13->14 including comma
	u8 ReleaseDate[11];		// 10->11 including comma
	u8 IC_PartNo[14];		// 14->15 including comma ?

} ;

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



#pragma pack(pop)






















