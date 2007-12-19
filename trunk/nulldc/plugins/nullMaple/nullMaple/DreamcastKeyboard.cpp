#include "DreamcastDevices.h"

#ifdef this_stuf_works
class DreamcastKeyboardDevice : public DreamcastDevice
{
public:
	DreamcastKeyboardDevice() {}
	DreamcastKeyboardDevice(maple_device_instance* inst) {}

	virtual DreamcastDevice* Create(maple_device_instance* inst)
	{
		return new DreamcastKeyboardDevice(inst);
	}
	virtual void Init()
	{
		
	}
	virtual void Term()
	{
		
	}
	virtual void Destroy()
	{
		
	}

	virtual MapleDeviceType GetType() const	{ return MDT_Main; }
	virtual u32 GetFlags() const	{ return MDTF_Hotplug; }

	virtual const wchar* GetName() const
	{
		return "nullDC Keyboard/n";
	}
	virtual const wchar* GetGuid() const
	{
		return DreamcastKeyboardGuid;
	}

	virtual InputProvider* GetInput() const
	{
		return 0;
	}
	virtual void Dma(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
	{
		//uhh hum whatever ;p
		const char strName_kbd[64] = "Emulated Dreamcast Keyboard\0";
		const char strBrand[64] = "Produced By or Under License From SEGA ENTERPRISES,LTD.\0";

		u8*buffer_out_b=(u8*)buffer_out;

		switch (Command)
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
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			responce=5;

			//caps
			//4
			w32(1 << 30);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)strName_kbd[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)strBrand[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x04FF); 

			//2
			w16(0x0069); 
			break;

			/* controller condition structure 
			typedef struct {//8 bytes
			int8 shift          ; shift keys pressed (bitmask)	//1
			int8 led            ; leds currently lit			//1
			int8 key[6]         ; normal keys pressed			//6
			} cont_cond_t;*/
		case 9:


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			responce=0x08;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32((1 << 30));
			//struct data
			//int8 shift          ; shift keys pressed (bitmask)	//1
			w8(0);//kb_shift
			//int8 led            ; leds currently lit			//1
			w8(0);//kb_led
			//int8 key[6]         ; normal keys pressed			//6
			for (int i=0;i<6;i++)
			{
				w8(0);//kb_key[i]
			}

			break;

		default:
			printf("UNKOWN MAPLE COMMAND %d\n",Command);
			break;
		}
	}
};

DeviceList<DreamcastKeyboardDevice> djd_al;
#endif