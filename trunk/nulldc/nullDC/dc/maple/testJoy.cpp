#include "types.h"
#include "maple_if.h"
#include "dc/mem/sh4_mem.h"

u16 kcode=0xFFFF;
s8 joyx=0,joyy=0;
s8 joy2x=0,joy2y=0;
u8 rt=0,lt=0;

char* testJoy_strName = "Emulated Dreamcast Controler\0";
char* testJoy_strBrand = "Faked by drkIIRaziel && ZeZu , made for nullDC\0";
u8 GetBtFromSgn(s8 val);

void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
{
 	u32 command = data[0] & 0xFF;
	u32 recadr = (data[0] >> 8) & 0xFF;
	u32 sendadr = (data[0] >> 16) & 0xFF;
	u32 ptr_out;
	if (recadr != 0x20)
	{
		WriteMem32(header2, 0xFFFFFFFF);//not conected
		return;
	}
	switch (command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			ptr_out = header2;
			//header
			WriteMem32(ptr_out,(u32)(0x05 | //response
						(((u16)sendadr << 8) & 0xFF00) |
						((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
						(((112/*size*//4) << 24) & 0xFF000000))); ptr_out += 4;
			//caps
			//4
			WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;

			//struct data
			//3*4
			WriteMem32(ptr_out, 0xFFFFFFFF); ptr_out += 4;
			WriteMem32(ptr_out, 0xFFFFFFFF); ptr_out += 4;
			WriteMem32(ptr_out, 0xFFFFFFFF); ptr_out += 4;
			//1	area code
			WriteMem8(ptr_out, 0xFF); ptr_out += 1;
			//1	direction
			WriteMem8(ptr_out, 0); ptr_out += 1;
			//30
			for (u32 i = 0; i < 30; i++)
			{
				WriteMem8(ptr_out + i, (u8)testJoy_strName[i]);
				if (!testJoy_strName[i])
					break;
			}
			ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				WriteMem8(ptr_out + i, (u8)testJoy_strBrand[i]);
				if (!testJoy_strBrand[i])
					break;
			}
			ptr_out += 60;

			//2
			WriteMem16(ptr_out, 0x04FF); ptr_out += 2;

			//2
			WriteMem16(ptr_out, 0x0069); ptr_out += 2;
			break;

		/* controller condition structure 
		typedef struct {//8 bytes
		WORD buttons;			///* buttons bitfield	/2
		u8 rtrig;			///* right trigger			/1
		u8 ltrig;			///* left trigger 			/1
		u8 joyx;			////* joystick X 			/1
		u8 joyy;			///* joystick Y				/1
		u8 joy2x;			///* second joystick X 		/1
		u8 joy2y;			///* second joystick Y 		/1
		} cont_cond_t;*/
		case 9:
			ptr_out = header2;

			//header
			WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
						(((u16)sendadr << 8) & 0xFF00) |
						((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
						(((12/*size*/ / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			//caps
			//4
			WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;

			//struct data
			//2
			WriteMem16(ptr_out, kcode); ptr_out += 2;
			
			//triger
			//1 R
			WriteMem8(ptr_out, rt); ptr_out += 1;
			//1 L
			WriteMem8(ptr_out, lt); ptr_out += 1;
			//joyx
			//1
			WriteMem8(ptr_out, GetBtFromSgn(joyx)); ptr_out += 1;
			//joyy
			//1
			WriteMem8(ptr_out, GetBtFromSgn(joyy)); ptr_out += 1;

			//1
			WriteMem8(ptr_out, GetBtFromSgn(joy2x)); ptr_out += 1;
			//1
			WriteMem8(ptr_out, GetBtFromSgn(joy2y)); ptr_out += 1;
			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;

			break;

		default:

			break;
	}
}

u8 GetBtFromSgn(s8 val)
{
	return val+128;
}