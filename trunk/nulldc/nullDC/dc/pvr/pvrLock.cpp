#include "plugins/plugin_manager.h"
#include "pvrLock.h"
 
using namespace std;

vector<vram_block*> VramLocks[VRAM_SIZE/PAGE_SIZE];
//vram 32-64b
VArray vram;
 

//Address space convertion functions
void vramlock_Read32b(u32* pdst,u32 offset,u32 len)
{
	//TODO:implement .. later
}
 
//mmx version , 2x64bit at a time
//pdst needs to be 16 byte aligned
void vramlock_Read32b_mmx(u32* pdst,u32 offset,u32 len)
{
		//TODO:implement .. later
}
 
//sse version , 2x128bit at a time
//pdst needs to be 16 byte aligned
void vramlock_Read32b_sse(u32* pdst,u32 offset,u32 len)
{
		//TODO:implement .. later
}
 
 
 
//So, the bytes 0xA4000000-0xA4000003 correspond to 0xA5000000-0xA5000003, 0xA4000004-0xA4000007 to 0xA5400000-0xA5400003,
//0xA4000008-0xA400000B to 0xA5000004-0xA5000007 and so on. 
//
//
//Convert Sh4 address to vram_32 offset
u32 vramlock_ConvAddrtoOffset32(u32 Address)
{
	if (Is_64_Bit(Address))
	{
		//64b wide bus is archevied by interleavingthe banks every 32 bits
		//so bank is Address>>3
		//so >>1 to get real uper offset
		u32 t=(Address>>1)& 0x3FFFFC;	//upper bits
		u32 t2=Address&0x3;	//lower bits
		//u32 t3=t& 0x7FFFFC;		//clean upper bits
		t=((Address& (1<<2))<<20)|t|t2;//bank offset |clean upper bits|lower bits -> Ready :)!
 
		return t;
	}
	else
	{
		return  Address & 0x7FFFFF;
	}
}
 
//Convert offset64 to offset32
u32 vramlock_ConvOffset64toOffset32(u32 offset64)
{
		//64b wide bus is archevied by interleavingthe banks every 32 bits
		//so bank is Address>>3
		//so >>1 to get real uper offset
		u32 t=(offset64>>1)& (VRAM_MASK-3);	//upper bits
		u32 t2=offset64&0x3;	//lower bits
		//u32 t3=t& 0x7FFFFC;		//clean upper bits
		t=((offset64& (1<<2))<<20)|t|t2;//bank offset |clean upper bits|lower bits -> Ready :)!
 
		return t;
}
//Convert Sh4 address to vram_64 offset
u32 vramlock_ConvAddrtoOffset64(u32 Address)
{
	if (Is_64_Bit(Address))
	{
		return  Address & VRAM_MASK;//64 bit offset
	}
	else
	{
		//64b wide bus is archevied by interleaving the banks every 32 bits
		//so bank is Address<<3
		//bits <4 are <<1 to create space for bank num
		//bank 0 is mapped at 400000 (32b offset) and after
		u32 bank=((Address>>22)&0x1)<<2;//bank will be used ass uper offset too
		u32 lv=Address&0x3; //these will survive
		Address<<=1;
		//       |inbank offset    |       bank id        | lower 2 bits (not changed)
		u32 rv=  (Address&(VRAM_MASK-7))|bank                  | lv;
 
		return rv;
	}
}
//Convert offset32 to offset64
u32 vramlock_ConvOffset32toOffset64(u32 offset32)
{
		//64b wide bus is archevied by interleaving the banks every 32 bits
		//so bank is Address<<3
		//bits <4 are <<1 to create space for bank num
		//bank 1 is mapped at 400000 (32b offset) and after
		u32 bank=((offset32>>22)&0x1)<<2;//bank will be used ass uper offset too
		u32 lv=offset32&0x3; //these will survive
		offset32<<=1;
		//       |inbank offset    |       bank id        | lower 2 bits (not changed)
		u32 rv=  (offset32&(VRAM_MASK-7))|bank                  | lv;
 
		return rv;
}
 
//List functions
//
void vramlock_list_remove(vram_block* block)
{
	u32 base = block->start/PAGE_SIZE;
	u32 end = block->end/PAGE_SIZE;

	for (u32 i=base;i<=end;i++)
	{
		vector<vram_block*>* list=&VramLocks[i];
		for (int j=0;j<list->size();j++)
		{
			if ((*list)[j]==block)
			{
				(*list)[j]=0;
			}
		}
	}
}
 
void vramlock_list_add(vram_block* block)
{
	u32 base = block->start/PAGE_SIZE;
	u32 end = block->end/PAGE_SIZE;


	for (u32 i=base;i<=end;i++)
	{
		vector<vram_block*>* list=&VramLocks[i];
		for (int j=0;j<list->size();j++)
		{
			if ((*list)[j]==0)
			{
				(*list)[j]=block;
				goto added_it;
			}
		}

		list->push_back(block);
added_it:
		__noop;
	}
}
 
//simple IsInRange test
inline bool IsInRange(vram_block* block,u32 offset)
{
	return (block->start<=offset) && (block->end>=offset);
}

//returns block handle
vram_block* vramlock_Lock_32(u32 start_offset32,u32 end_offset32,void* userdata)
{
	vram_block* block=(vram_block*)malloc(sizeof(vram_block));
 
	block->end=end_offset32;
	block->start=start_offset32;
	block->len=end_offset32-start_offset32+1;
	block->userdata=userdata;
	block->type=32;
 
	//okz , new idea
	//asuming locks are 4 byte alligned (textures have to be 4 byte aligned too , dont they ? 
	//									 anyhow , fb is 32b alligned and locks are 4kb aligned 
	//									 so it doesnt matter)
	//32b to 64b maping goes 2:1
	//so we can do
	//lock(bank1,size>>1);lock(bank2,size>>1);

	/*for (u32 base=start_offset32;base<end_offset32;base+=4)
	{
		SetVramTest_32(vramlock_ConvOffset32toOffset64(base));
	}
 
	for (u32 base=block->start;base<block->end;base+=4)
	{
		vramlock_list_add(block,&vram_locks[GetHash(vramlock_ConvOffset32toOffset64(base))]);
	}*/
 
	return block;
}

vram_block* vramlock_Lock_64(u32 start_offset64,u32 end_offset64,void* userdata)
{
	vram_block* block=(vram_block* )malloc(sizeof(vram_block));
 

	if (end_offset64>(VRAM_SIZE-1))
	{
		printf("vramlock_Lock_64: end_offset64>(VRAM_SIZE-1) , Tried to lock area out of vram , possibly bug on the pvr plugin\n");
		//__asm int 3;
		end_offset64=(VRAM_SIZE-1);
	}

	if (start_offset64>end_offset64)
	{
		printf("vramlock_Lock_64: start_offset64>end_offset64 , Tried to lock negative block , possibly bug on the pvr plugin\n");
		start_offset64=0;
	}

	

	block->end=end_offset64;
	block->start=start_offset64;
	block->len=end_offset64-start_offset64+1;
	block->userdata=userdata;
	block->type=64;

	vram.LockRegion(block->start,block->len);
	vramlock_list_add(block);

	return block;
}
 
bool VramLockedWrite(u8* address)
{
	size_t offset=address-vram.data;

	if (offset<VRAM_SIZE)
	{
		u32 found=0;
		size_t addr_hash = offset/PAGE_SIZE;
		vector<vram_block*>* list=&VramLocks[addr_hash];
			
		for (size_t i=0;i<list->size();i++)
		{
			if ((*list)[i])
			{
				libPvr->pvr_info.LockedBlockWrite((*list)[i],offset);
				//found++;
				if ((*list)[i])
				{
					printf("Error , pvr is suposed to remove lock \n");
					__asm int 3;
				}
					//vramlock_list_remove((*list)[i]);
			}
		}
		list->clear();
		vram.UnLockRegion(offset&(~(PAGE_SIZE-1)),PAGE_SIZE);

		return true;
	}
	else
		return false;
}

//unlocks mem
//also frees the handle
void vramlock_Unlock_block(vram_block* block)
{
	//VRAM_SIZE/PAGE_SIZE;
	if (block->end>VRAM_SIZE)
		printf("Error : block end is after vram , skiping unlock\n");
	else
	{
		vramlock_list_remove(block);
		//more work needed
		free(block);
	}
}