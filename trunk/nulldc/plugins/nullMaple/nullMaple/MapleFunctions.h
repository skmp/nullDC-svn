#include "nullMaple.h"
#include "Profiles.h"

struct MapleFunction
{
	virtual u32 GetID()=0;		//returns function ID
	virtual u32 GetDesc()=0;	//returns function description info
	
	virtual bool Init()=0;
	virtual void Term()=0;
	virtual void Destroy()=0;

	//Handles a maple command
	//buffer in, in_len, out, out_len are modified to strip / add the function headers
	//before this gets called
	virtual void Dma(u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce) =0;

	MapleDevice* dev;
	static MapleFunction* Create(MapleDevice* dev,u32 function,u32 lparam,void* dparam);
	static void SetupProfile(ProfileDDI* sp,u32 function,u32 lparam,void* dparam);
};
//Varius low level structs and constants used at the packet layer
#pragma pack(push,1)
struct MapleDeviceInfo
{
	u32		func;//4
	u32		function_data[3];//3*4
	u8		area_code;//1
	u8		connector_direction;//1
	char	product_name[30];//30*1
	char	product_license[60];//60*1
	u16		standby_power;//2
	u16		max_power;//2
};

struct GetMediaInfo_data
{
	u16 total_size;
	u16 partition_number;
	u16 system_area_block;
	u16 fat_area_block;
	u16 number_fat_areas_block;
	u16 file_info_block;
	u16 number_info_blocks;
	u8 volume_icon;
	u8 reserved1;
	u16 save_area_block;
	u16 number_of_save_blocks;
	u16 reserverd0;  
};
struct BlockRead_data
{
	u8 PT;
	u8 Phase;
	u16 Block;
};
struct BlockWrite_data
{
	u8 PT;
	u8 Phase;
	u16 Block;
	//data folows, of corse :)
};
struct JoystickData
{
	u16 buttons;		// buttons bitfield
	u8 rtrig;			// right trigger
	u8 ltrig;			// left trigger
	u8 joyx;			// joystick X
	u8 joyy;			// joystick Y
	u8 joy2x;			// second joystick X
	u8 joy2y;			// second joystick Y
};

struct KeyboardData
{
	u8 shift;          // shift keys pressed (bitmask)
	u8 led;            // leds currently lit
	u8 key[6];         // normal keys pressed
};

struct MouseData
{
	u32 buttons;       // digital buttons bitfield (little endian)
	u16 axis1;         // horizontal movement (0-0x3FF) (little endian)
	u16 axis2;         // vertical movement (0-0x3FF) (little endian)
	u16 axis3;         // mouse wheel movement (0-0x3FF) (little endian)
	u16 axis4;         // ? movement (0-0x3FF) (little endian)
	u16 axis5;         // ? movement (0-0x3FF) (little endian)
	u16 axis6;         // ? movement (0-0x3FF) (little endian)
	u16 axis7;         // ? movement (0-0x3FF) (little endian)
	u16 axis8;         // ? movement (0-0x3FF) (little endian)
};
#pragma pack(pop)

template<typename T>
u32 ToBuffer(u8* dst,T& src)
{
	memcpy(dst,&src,sizeof(T));
	return sizeof(T);
}

#define wX(sz) {int _sz_evl=(sz);buffer_out_b+=(_sz_evl);buffer_out_len+=(_sz_evl);}
#define w32(data) {*(u32*)buffer_out_b=(data);wX(4);}
#define w16(data) {*(u16*)buffer_out_b=(data);wX(2);}
#define w8(data) {*(u8*)buffer_out_b=(data);wX(1);}	//buffer_out_b+=1;buffer_out_len+=1;
#define wbuff(data) {wX(ToBuffer((u8*)buffer_out_b,data));}
enum MapleFunctionID
{
	MFID_0_Input    = 0x01000000,
	MFID_1_Storage  = 0x02000000,
	MFID_2_LCD      = 0x04000000,
	MFID_3_Clock    = 0x08000000,
	MFID_4_Mic      = 0x10000000,
	MFID_5_ARGun    = 0x20000000,
	MFID_6_Keyboard = 0x40000000,
	MFID_7_LightGun = 0x80000000,
	MFID_8_PuruPuru = 0x00010000,
	MFID_9_Mouse	= 0x00020000,
	MFID_10_ExngData= 0x00040000,
	MFID_11_Camera	= 0x00080000,
};
enum MapleDeviceCommand
{
	MDC_DeviceRequest	=0x01,			//0 words.Note : Initialises device
	MDC_AllStatusReq	=0x02,			//0 words
	MDC_DeviceReset		=0x03,			//0 words
	MDC_DeviceKill		=0x04,			//0 words

	//Varius Functions
	MDCF_GetCondition=0x09,				//FT
	MDCF_GetMediaInfo=0x0A,				//FT,PT,3 pad
	MDCF_BlockRead   =0x0B,				//FT,PT,Phase,Block #
	MDCF_BlockWrite  =0x0C,				//FT,PT,Phase,Block #,data ...
	MDCF_GetLastError=0x0D,				//FT,PT,Phase,Block #
	MDCF_SetCondition=0x0E,				//FT,data ...
	MDCF_MICControl	 =0x0F,				//FT,MIC data ...
	MDCF_ARGunControl=0x10,				//FT,AR-Gun data ...
};
enum MapleDeviceRV
{
	MDRS_DeviseStatus=0x05,			//28 words
	MDRS_DeviseStatusAll=0x06,		//28 words + device depedant data
	MDRS_DeviceReply=0x07,			//0 words
	MDRS_DataTransfer=0x08,			//FT,depends on the command

	MDRE_UnkownFunction=0xFE,		//0 words
	MDRE_UnkownCmd=0xFD,			//0 words
	MDRE_TransminAgain=0xFC,		//0 words
	MDRE_FileError=0xFB,			//1 word, bitfield
	MDRE_LCDError=0xFA,				//1 word, bitfield
	MDRE_ARGunError=0xF9,			//1 word, bitfield
};