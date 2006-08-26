/*
**	zGDROM.h
*/
#ifndef __ZGDROM_H__
#define __ZGDROM_H__



#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

void gdop(u32 opcode);



enum GD_Notify
{
	Notify_DiskEject,		// Params, DiskType cast to (void*)
	Notify_DiskInsert,		// Could use Eject for both, we'll see (Spec uses 1)

	Notify_CDAudioStop,		// inform the fucking authorities
	Notify_CDAudioStart,	// this is better
	Notify_CDAudioChange,	// change track
};




// GDROM i/f Control Regs	
//

#define GD_IMPEDHI0	0x00	// (R) These are all 
#define GD_IMPEDHI4	0x04	// (R) RData bus high imped
#define GD_IMPEDHI8	0x08	// (R) For DIOR- ( READ ONLY )
#define GD_IMPEDHIC	0x0C	// (R) 

#define GD_ALTSTAT	0x18	// (R) |BSY|DRDY|DF|DSC|DRQ|CORR|Reserved|CHECK|
#define GD_DEVCTRL	0x18	// (W) Device Control |R|R|R|R|1|SRST|nIEN|0|

#define GD_DATA		0x80	// (RW)	Data / Data

#define GD_ERROR	0x84	// (R) Error |SENSE(4)|MCR|ABRT|EOMF|ILI|
#define GD_FEATURES	0x84	// (W) Features |Reserved(7)|DMA| or |SetClear|FeatNum(7)|

#define GD_IREASON	0x88	// (R) Interrupt Reason |Reserved(6)|IO|CoD|
#define GD_SECTCNT	0x88	// (W) Sector Count |XFerMode(4)|ModeVal(4)|

#define GD_SECTNUM	0x8C	// (RW) Sector Number |DiscFmt(4)|Status(4)|
#define GD_BYCTLLO	0x90	// (RW) Byte Control Low / Byte Control Low
#define GD_BYCTLHI	0x94	// (RW) Byte Control High / Byte Control High

#define GD_DRVSEL	0x98	// (RW) Unused |1|R|1|0|LUN(4)|

#define GD_STATUS	0x9C	// (R) |BSY|DRDY|DF|DSC|DRQ|CORR|Reserved|CHECK|
#define GD_COMMAND	0x9C	// (W) Command
//////////////////////////////////////////


// SATA Commands

typedef enum
{
	GDC_NOP				= 0x00,
	GDC_SFT_RESET		= 0x08,
	GDC_EXEC_DIAG		= 0x90,
	GDC_SPI_PACKET		= 0xA0,
	GDC_IDENTIFY_DEV	= 0xA1,
	GDC_SET_FEATURES	= 0xEF,

} SATA_COMMANDS;



// SPI Packet Commands

typedef enum
{
	SPI_TEST_UNIT	= 0x00,
	SPI_REQ_STAT	= 0x10,
	SPI_REQ_MODE	= 0x11,
	SPI_SET_MODE	= 0x12,
	SPI_REQ_ERROR	= 0x13,
	SPI_GET_TOC		= 0x14,
	SPI_REQ_SES		= 0x15,
	SPI_CD_OPEN		= 0x16,
	SPI_CD_PLAY		= 0x20,
	SPI_CD_SEEK		= 0x21,
	SPI_CD_SCAN		= 0x22,
	SPI_CD_READ		= 0x30,
	SPI_CD_READ2	= 0x31,
	SPI_GET_SCD		= 0x40,

} SPI_PACKET_CMDCODES;

/////////////// END OF COMMANDS /////////////////


#define GDFORMAT_CDDA	0x00	// 
#define GDFORMAT_CDROM	0x10
#define GDFORMAT_XA		0x20
#define GDFORMAT_EXTRA	0x30
#define GDFORMAT_CDI	0x40
#define GDFORMAT_GDROM	0x80	// This is correct .. dont know about the rest ..

#define GDSTATE_BUSY	0x00	// State transition
#define GDSTATE_PAUSE	0x01	// Pause
#define GDSTATE_STANDBY	0x02	// Standby (drive stop)
#define GDSTATE_PLAY	0x03	// CD playback
#define GDSTATE_SEEK	0x04	// Seeking
#define GDSTATE_SCAN	0x05	// Scanning
#define GDSTATE_OPEN	0x06	// Tray is open
#define GDSTATE_NODISC	0x07	// No disc
#define GDSTATE_RETRY	0x08	// Read retry in progress (option)
#define GDSTATE_ERROR	0x09	// Reading of disc TOC failed (state does not allow access) 
///////////////////////////////////////////////////////////////////////////////////////////

#define GDSTATUS_CHECK	0x01
#define GDSTATUS_RSVD	0x02
#define GDSTATUS_CORR	0x04
#define GDSTATUS_DRQ	0x08
#define GDSTATUS_DSC	0x10
#define GDSTATUS_DF		0x20
#define GDSTATUS_DRDY	0x40
#define GDSTATUS_BSY	0x80

#define IRES_IO			0x02
#define IRES_COD		0x01

// SENSE - same as SCSI 
#define GSSENSE_NOSENSE		0x00
#define GDSENSE_RECOVER		0x01
#define GDSENSE_NOTREADY	0x02
#define GDSENSE_MEDIUMERR	0x03
#define GDSENSE_HW_ERROR	0x04
#define GDSENSE_ILL_REQ		0x05
#define GDSENSE_ATTENTION	0x06
#define GDSENSE_PROTECT		0x07
#define GDSENSE_RES_1		0x08
#define GDSENSE_RES_2		0x09
#define GDSENSE_RES_3		0x0A
#define GDSENSE_ABORT		0x0B
#define GDSENSE_RES_4		0x0C
#define GDSENSE_RES_5		0x0D
#define GDSENSE_RES_6		0x0E
#define GDSENSE_RES_7		0x0F
////////////////////////////////




/*	Bit 0 (CoD): 	"0" indicates data and "1" indicates a command. 
	Bit 1 (IO): 	"1" indicates transfer from device to host, and "0" from host to device.
	IO	DRQ	CoD
	0	1	1	Command packet can be received.
	1	1	1	Message can be sent from device to host.
	1	1	0	Data can be sent to host.
	0	1	0	Data can be received from host.
	1	0	1	Status register contains completion status.
*/
typedef enum
{
	GD_RESET_SR		= (0),	// host to dev, NULL Data, No Data
	GD_RECV_DATA	= (2),
	GD_RECV_CMD		= (3),
	GD_STATUS_OK	= (5),
	GD_SEND_DATA	= (6),
	GD_SEND_MSG		= (7),

	GD_STATUS_ERR	= GD_STATUS_OK | 4,		// Set CHECK
	GD_STATUS_CERR	= GD_STATUS_OK | 24,	// Set CHECK, CORR

} gdIntStatus;


typedef struct			// *FIXME* not finished
{
	union {
		struct {
			u8 CoD:	1;
			u8 DRQ: 1;
			u8 IO:	1;

			u8 CHECK:	1;
			u8 CORR:	1;
		//	u8 DRDY:	1;
		//	u8 BSY:		1;

			u8 zRs:	3;			// make sure to keep in sync w/ above if you uncomment
		};

		u8	iStatus;		// Internal Status, use GD_* from above
	};

	u8	iDevType;		// Device Type / State for SectorNum Reg

	union {
		struct {
			u8 ByteCountLO;
			u8 ByteCountHI;
		};

		u16 ByteCount;		// Internal ByteCount
	};


} gdromSR;



typedef union
{
	struct
	{
		u8 CHECK:	1;
		u8 Reserved:1;
		u8 CORR:	1;
		u8 DRQ:		1;
		u8 DSC:		1;
		u8 DF:		1;
		u8 DRDY:	1;
		u8 BSY:		1;

		u8 zPadding[3];
	};

	u32 Full;

} gdStatusReg;


typedef union 
{
	struct {
		u8 ILI:		1;
		u8 EOMF:	1;
		u8 ABRT:	1;
		u8 MCR:		1;

		u8 Sense:	4;
	};

	u8 Full;

} gdErrorReg;


typedef union
{
	struct {
		u8 LO;
		u8 HI;
		u16 pad;
	};

	u32 Full;

} gdByCnt;


typedef union
{
	struct
	{
		u8 CoD:		1;
		u8 IO:		1;
		u8 Reserved:6;

		u8 zPadding[3];
	};

	u32 Full;

} gdIReasonReg;



typedef union
{
	struct {
		u16	zResv1;
		u8	CDROM_Speed;
		u8	zResv2;
		u16	Standby_Time;
		u8	Read_Settings;
		u16	zResv3;
		u8	Read_Retry;
		u8	DriveInformation[8];
		u8	SystemVersion[8];
		u8	SystemDate[6];
	};
	u16 Words[16];

} gdReqMode;

typedef union
{
	struct {
		u8	Status:	4;
		u8	zResv1:	4;
		u8	Repeat:	4;
		u8	Format:	4;
		u8	Control:4;
		u8	Address:4;
		u8	TNO;
		u8	X;
		u8	FAD[3];
		u8	ReadRetry;
		u8	zResv2;
	};
	u16 Words[5];

} gdReqStat;


#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif //__ZGDROM_H__
