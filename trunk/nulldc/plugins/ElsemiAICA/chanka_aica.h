#pragma once


#define dbgbreak  { __asm {int 3}/* printf("Press ANY key to continue\n");getchar();*/}

#define fastcall __fastcall
#define verify(x) if((x)==false){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define verifyc(x) if(FAILED(x)){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define die(reason) { printf("Fatal error : %s\n in %s -> %s : %d \n",reason,__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define fverify verify


#define 	ReadMemArrRet(arr,addr,sz)				\
			{if (sz==1)								\
				return arr[addr];					\
			else if (sz==2)							\
				return *(u16*)&arr[addr];			\
			else if (sz==4)							\
				return *(u32*)&arr[addr];}	

#define WriteMemArrRet(arr,addr,data,sz)				\
			{if(sz==1)								\
				{arr[addr]=(u8)data;return;}				\
			else if (sz==2)							\
				{*(u16*)&arr[addr]=(u16)data;return;}		\
			else if (sz==4)							\
			{*(u32*)&arr[addr]=data;return;}}	
#define WriteMemArr(arr,addr,data,sz)				\
			{if(sz==1)								\
				{arr[addr]=(u8)data;}				\
			else if (sz==2)							\
				{*(u16*)&arr[addr]=(u16)data;}		\
			else if (sz==4)							\
			{*(u32*)&arr[addr]=data;}}	

u32 FASTCALL ReadMem_reg(u32 addr,u32 size);
void FASTCALL WriteMem_reg(u32 addr,u32 data,u32 size);

void FASTCALL UpdateSystem(u32 Cycles);
void InitARM7();
void TerminateARM7();
#define ARM7BIAS (2)
//extern u32 g_videoCableType;
//extern u32 sh4_cycles;