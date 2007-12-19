#include "MapleFunctions.h"

struct StorageFunctionClass : MapleFunction
{
	u32 StorageGeometry;

	StorageFunctionClass()
	{
		StorageGeometry=0;
	}
	
	virtual u32 GetID() { return MFID_1_Storage; }
	virtual u32 GetDesc() { return StorageGeometry; }
	
	virtual bool Init()
	{
		return true;
	}
	virtual void Term()
	{
	}
	virtual void Destroy()
	{
		delete this;
	}

	virtual void Dma(u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
	{
		u8*buffer_out_b=(u8*)buffer_out;

		switch (Command)
		{
		case MDCF_GetMediaInfo:
			{
				w32(MFID_1_Storage);
				GetMediaInfo_data vmui =
				{
					0xFF,		//Total Size
					0,			//Partition ##
					0xFF,		//System Area block
					0xFE,		//Fat Area Block
					1,			//Number of Fat Area blocks
					0xFD,		//File Info block
					0x0D,		//Number of file info blocks
					0,			//Volume icon
					0,			//Reserved
					0xF0,		//Save Area Block
					0x0,		//Number of save blocks
					0,			//Reserved
				};

				wbuff(vmui);

				responce=MDRS_DataTransfer;
			}
			break;

		default:
			printf("nullMaple::StorageFunctionClass : Unknown command %d\n",Command);
			responce=MDRE_UnkownCmd;
			break;
		}
	}
};
MapleFunction* CreateFunction1(MapleDevice* dev,u32 lparam,void* dparam)
{
	StorageFunctionClass* rv= new StorageFunctionClass();
	rv->StorageGeometry=lparam;
	return rv;
}