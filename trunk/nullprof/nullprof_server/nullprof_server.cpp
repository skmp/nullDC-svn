#include "Socket.h"
#include <process.h>
#include <windows.h>
#include <string>

#define EXPORT extern "C" __declspec(dllexport)

struct dpa
{
	int len;
	char* data;
};
HANDLE thr_han;
void GetPacket(Socket* soc,dpa& packet)
{
	int len;
	if (soc->ReceiveBytes2((char*)&len,4)==false)
	{
		packet.len=0;
		return;
	}
	printf("Got packet header; size=%d\n",len);
	packet.len=len;
		
	packet.data=(char*)malloc(len+2);
	soc->ReceiveBytes2(packet.data,len);
}


void SendPacket(Socket* soc,dpa& packet)
{
	soc->SendBytes2((char*)&packet.len,4);
	soc->SendBytes2(packet.data,packet.len);
}
void SendString(Socket* soc,char* str)
{
	dpa packet;
	packet.len=strlen(str);
	packet.data=str;
	SendPacket(soc,packet);
}
void SendData(Socket* soc,void* data,int size)
{
	dpa packet;
	packet.len=size;
	packet.data=(char*)data;
	SendPacket(soc,packet);
}
//"ver"    -> get ndc version string
//"blocks [all|pcall|ptime|pslow] {##}" : get block list , all gets all blocks (## is ignored)
// pcall/time/slow -> get most called/most time consuming/slower (sh4/x86 cycles)
//"pclear" -> clear profiler data
//"blockinfo [id|addr] ##" -> get block info for block id=## or addr=##

//basic types
typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;
#pragma pack(push,1)
struct nullprof_block_info
{
	u32 addr;
	void* sh4_code;
	void* x86_code;
	u32 sh4_bytes;
	u32 x86_bytes;
	u32 sh4_cycles;
	u64 time;
	u32 calls;
};

struct nullprof_blocklist
{
	nullprof_block_info* blocks;
	u32 count;
};
#pragma pack(pop)
typedef void nullprof_GetBlockFP(nullprof_block_info* to,u32 type,u32 address);
#define ALL_BLOCKS 0
#define PCALL_BLOCKS 1
#define PTIME_BLOCKS 2
#define PSLOW_BLOCKS 3

typedef void nullprof_GetBlocksFP(nullprof_blocklist* to, u32 type,u32 count);
typedef void nullprof_ClearBlockPdataFP();

struct nullprof_prof_pointers
{
	char ndc_ver[512];
	nullprof_GetBlockFP* GetBlockInfo;
	nullprof_GetBlocksFP* GetBlocks;
	nullprof_ClearBlockPdataFP* ClearPdata;
};


typedef void InitProfillerFP(nullprof_prof_pointers* pif);
nullprof_prof_pointers p_if;
unsigned __stdcall Answer(void* a);
EXPORT void InitProfiller(nullprof_prof_pointers* pif)
{
	//thread_id=GetCurrentThreadId();
	thr_han=OpenThread(THREAD_SUSPEND_RESUME | THREAD_SET_INFORMATION | THREAD_QUERY_INFORMATION,false,GetCurrentThreadId());
	printf("Setting Thread anffinity to 1...");
	DWORD_PTR arv= SetThreadAffinityMask(thr_han,1);
	printf("rv=%d\n",(u32)arv);
	
	memcpy(&p_if,pif,sizeof(p_if));
	  SocketServer in(1337,1);

	  printf("Teh l33t profiler inited , waiting for connection ...\n");
  while (1) {
    Socket* s=in.Accept();

    unsigned ret;
    _beginthreadex(0,0,Answer,(void*) s,0,&ret);
	break;
  }
	printf("Teh l33t profiler connected ...\n");
}

//"ver"    -> get ndc version string
//"blocks [all|pcall|ptime|pslow] {##}" : get block list , all gets all blocks (## is ignored)
// pcall/time/slow -> get most called/most time consuming/slower (sh4/x86 cycles)
//"pclear" -> clear profiler data
//"blockinfo [id|addr] ##" -> get block info for block id=## or addr=##
//"memget ##" , ## is on HEX format
void ProcPacket(Socket* soc,dpa& packet)
{
	char t[80];
	packet.data[packet.len]=0;
	printf("Command %s\n",packet.data);
	
	SuspendThread(thr_han);
	sscanf(packet.data,"%s",t);
	if (strcmp(t,"ver")==0)
	{
		SendString(soc,p_if.ndc_ver);
	}
	else if (strcmp(t,"blocks")==0)
	{
		int type=1 , count=40, mode =1;
		sscanf(packet.data,"%s %d %d %d",t,&type,&count,&mode);
		nullprof_blocklist blkl;
		//(nullprof_blocklist* to, u32 type,u32 count);
		p_if.GetBlocks(&blkl,type,count);
		if (mode==0)
		{
			SendData(soc,blkl.blocks,blkl.count*sizeof(nullprof_block_info));
		}
		else
		{
			char temp[1024];
			for (int i=0;i<blkl.count;i++)
			{
				double cpb=(double)blkl.blocks[i].time/(double)blkl.blocks[i].calls;
				sprintf(temp,"Block 0x%X , size %d[%d cycles] , %f cycles/call , %f cycles/emucycle,%f consumed Mhrz",
					blkl.blocks[i].addr,
					(blkl.blocks[i].sh4_bytes)>>1,
					blkl.blocks[i].sh4_cycles,
					cpb,
					cpb/blkl.blocks[i].sh4_cycles,
					blkl.blocks[i].time/(1000.0f*1000.0f));
				SendString(soc,temp);
			}
		}
	}
	else if (strcmp(t,"pclear")==0)
	{
		p_if.ClearPdata();
		SendString(soc,"Cleared profiler data :)");
	}
	else if (strcmp(t,"blockinfo")==0)
	{
		SendString(soc,"err:nso");
	}
	else if (strcmp(t,"memget")==0)
	{
		u32 address,len;
		sscanf(packet.data,"%s %x %d",t,&address,&len);
		SendData(soc,(void*)address,len);
	}
	else SendString(soc,"err:nso");
	ResumeThread(thr_han);
}
unsigned __stdcall Answer(void* a) {
  Socket* s = (Socket*) a;
	printf("Starting listener\n");
  dpa tp;
  while (1) 
  {
    //std::string r = s->ReceiveLine();
	  GetPacket(s,tp);
	  if (tp.len==0) 
		  break;
	  ProcPacket(s,tp);
	  free(tp.data);
  }
printf("Exiting listener\n");
  delete s;

  return 0;
}

int main(int argc, char* argv[]) {
  SocketServer in(1337,1);

  while (1) {
    Socket* s=in.Accept();

    unsigned ret;
    _beginthreadex(0,0,Answer,(void*) s,0,&ret);
  }
 
  return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	return TRUE;
}
