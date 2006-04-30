
#include "plugins/plugin_manager.h"
#include "pvrLock.h"
 
 
#define VRAM_LOCK_HASH_SIZE (256)
#define VRAM_LOCK_HASH_MASK (VRAM_LOCK_HASH_SIZE-1)
#define VRAM_LOCK_HASH_RANGE (VRAM_SIZE/VRAM_LOCK_HASH_SIZE)
 
#define GetHash(offset_64) (((offset_64)>>15)&VRAM_LOCK_HASH_MASK)
 
 
#define VramTestMinWriteBits 5
#define VramTestMinWrite (1<<VramTestMinWriteBits)
 
 
#define VramTestMask (VRAM_MASK>>(VramTestMinWriteBits+2))//>> + 2 b/c we can store 4 addr per byte
#define VramTestSize (VramTestMask+1)
 
//to test if the address is in a block
//keep it small for possibility of l2 cache
u8 VramTestHash[VramTestSize]={0};
vram_lock_list vram_locks[VRAM_LOCK_HASH_SIZE]={0};
 
//using pre calculated tables for speed
//For all
u8 Test_Bit_All[4]={(u8)(1<<(0))|(u8)(1<<(1)),(u8)(1<<(2))|(u8)(1<<(3)),(u8)(1<<(4))|(u8)(1<<(5)),(u8)(1<<(6))|(u8)(1<<(7))};
//0's for 32b
u8 Set_Bit_32[4]={(u8)(1<<(0)),(u8)(1<<(2)),(u8)(1<<(4)),(u8)(1<<(6))};
u8 Reset_Bit_32[4]={(u8)~(1<<(0)),(u8)~(1<<(2)),(u8)~(1<<(4)),(u8)~(1<<(6))};
//1's for 64b
u8 Set_Bit_64[4]={(u8)(1<<(1)),(u8)(1<<(3)),(u8)(1<<(5)),(u8)(1<<(7))};
u8 Reset_Bit_64[4]={(u8)~(1<<(1)),(u8)~(1<<(3)),(u8)~(1<<(5)),(u8)~(1<<(7))};
 
//32/64b test
inline bool GetVramTest_All(u32 offset_64)
{
	offset_64>>=VramTestMinWriteBits;								//32B words
	return (VramTestHash[offset_64>>2]&Test_Bit_All[offset_64&0x3])!=0;
}
 
//32 bit
//NOTE: *99% bug notice* it does not accept a 32b offset but an 64b !!
inline void SetVramTest_32(u32 offset_64)
{
	offset_64>>=VramTestMinWriteBits;								//32B words
	VramTestHash[offset_64>>2]|=Set_Bit_32[offset_64&0x3];
}
 
inline void ResetVramTest_32(u32 offset_64)
{
	offset_64>>=VramTestMinWriteBits;								//32B words
	VramTestHash[offset_64>>2]&=Reset_Bit_32[offset_64&0x3];
}
 
inline bool GetVramTest_32(u32 offset_64)
{
	offset_64>>=VramTestMinWriteBits;								//32B words
	return (VramTestHash[offset_64>>2]&Set_Bit_32[offset_64&0x3])!=0;
}
 
//64 bit
//
inline void SetVramTest_64(u32 offset_64)
{
	offset_64>>=VramTestMinWriteBits;								//32B words
	VramTestHash[offset_64>>2]|=Set_Bit_64[offset_64&0x3];
}
 
inline void ResetVramTest_64(u32 offset_64)
{
	offset_64>>=VramTestMinWriteBits;								//32B words
	VramTestHash[offset_64>>2]&=Reset_Bit_64[offset_64&0x3];
}
 
inline bool GetVramTest_64(u32 offset_64)
{
	offset_64>>=VramTestMinWriteBits;								//32B words
	return (VramTestHash[offset_64>>2]&Set_Bit_64[offset_64&0x3])!=0;
}
 
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
		//bank 0 is mapped at 400000 (32b offset) and after
		u32 bank=((offset32>>22)&0x1)<<2;//bank will be used ass uper offset too
		u32 lv=offset32&0x3; //these will survive
		offset32<<=1;
		//       |inbank offset    |       bank id        | lower 2 bits (not changed)
		u32 rv=  (offset32&(VRAM_MASK-7))|bank                  | lv;
 
		return rv;
}
 
//List functions
//
bool vramlock_list_remove(vram_block* block,vram_lock_list* list)
{
	vram_block** blk=list->list;
	for (u32 i=0;i<list->len;i++)
	{
		if (blk[i])
		{
			if (blk[i]==block)
			{
				blk[i]=0;
				return true;
			}
		}
	}
	return false;
}
 
u32 vramlock_list_add(vram_block* block,vram_lock_list* list)
{
	vram_block** blk=list->list;
	for (u32 i=0;i<list->len;i++)
	{
		if (blk[i]==0)
		{
			blk[i]=block;
			return i;
		}
		else if (block==blk[i])
			return i;
	}
	if ((list->len >= (list->alloced-1)) || (list->len==0))
	{
		
		if(blk)
		{
			blk=list->list=(vram_block**)realloc( list->list, (list->alloced+10)*sizeof(vram_block**) );
			list->alloced+=10;
		}
		else
		{
			blk=(vram_block**)malloc(10*sizeof(vram_block**));
			
			list->list=blk;
			
			list->alloced=10;
			//return 0;
		}
		//printf("list->len = %d\n" ,list->len);
		fflush(stdout);
		
		for (u32 i=list->len;i<list->alloced;i++)
		{
			//printf(" %x ,blk[%d]=0; \n" ,blk,i);
			fflush(stdout);
			blk[i]=0; 
		}
		
	
		//return 0;
	}
	//return 0;
	blk[list->len]=block;
	u32 rv=list->len++;
	return rv;
}
 
//simple IsInRange test
inline bool IsInRange(vram_block* block,u32 offset)
{
	return (block->start<=offset) && (block->end>=offset);
}
//
//32 byte size min test
//addr can be a full dc mem address , tests both 32b and 64b
//size must be 1,2,4 or 0 (0= do not care)
void vramlock_Test(u32 addr,u32 value,u32 size)
{
	addr=vramlock_ConvAddrtoOffset64(addr);
	if (GetVramTest_All(addr))
	{
		//shit
		//no code to handle it
		//lmao
		//OPT -> see if value actually changed
		vram_lock_list* hash=&vram_locks[GetHash(addr)];
		vram_block** blk_list=hash->list;
		if (blk_list==0)
			return;
		for (u32 cnt=0;cnt<hash->len;cnt++)
		{
			if (blk_list[cnt] && IsInRange(blk_list[cnt],addr))
			{
				//bah , we HIT at blk_list[cnt] :)
				//TODO:send note to the pvr plugin
				//PvrLib.lib.Update(PVRU_TCACHE_INVALIDATE, blk_list[cnt]);
				libPvr->pvr_info.LockedBlockWrite(blk_list[cnt],addr);
			}
		}
 
	}
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
 
	for (u32 base=start_offset32;base<end_offset32;base+=4)
	{
		SetVramTest_32(vramlock_ConvOffset32toOffset64(base));
	}
 
	for (u32 base=block->start;base<block->end;base+=4)
	{
		vramlock_list_add(block,&vram_locks[GetHash(vramlock_ConvOffset32toOffset64(base))]);
	}
 
	return block;
}
 
 
vram_block* vramlock_Lock_64(u32 start_offset64,u32 end_offset64,void* userdata)
{
	vram_block* block=(vram_block* )malloc(sizeof(vram_block));
 
	block->end=end_offset64;
	block->start=start_offset64;
	block->len=end_offset64-start_offset64+1;
	block->userdata=userdata;
	block->type=64;
 
	for (u32 base=start_offset64;base<end_offset64;base+=VramTestMinWrite)
	{
		SetVramTest_64(base);
	}
 
	u32 hash_end=GetHash(block->end);
	for (u32 hash_base = GetHash(block->start);hash_base<=hash_end;hash_base+=1)
	{
		vramlock_list_add(block,&vram_locks[hash_base]);
	}
 
	return block;
}
 
 
 
void vramlock_ClearBlock(vram_block* block)
{
	if (block->type==32)
	{
		for (u32 base=block->start;base<block->end;base+=4)
		{
			ResetVramTest_32(vramlock_ConvOffset32toOffset64(base));
		}
		//To be done better
		for (u32 base=block->start;base<block->end;base+=4)
		{
			vramlock_list_remove(block,&vram_locks[GetHash(vramlock_ConvOffset32toOffset64(base))]);
		}
	}
	else
	{
		for (u32 base=block->start;base<block->end;base+=VramTestMinWrite)
		{
			ResetVramTest_64(base);
		}
 
		u32 hash_end=GetHash(block->end);
		for (u32 hash_base = GetHash(block->start);hash_base<=hash_end;hash_base+=1)
		{
			vramlock_list_remove(block,&vram_locks[hash_base]);
		}
	}
 
}
 
 
//unlocks mem
//also frees the handle
void vramlock_Unlock_block(vram_block* block)
{
	vramlock_ClearBlock(block);
	//more work needed
	free(block);
}