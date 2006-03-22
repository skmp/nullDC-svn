#include "..\..\types.h"
#include "sh4_cst.h"
#include "..\mem\Elf.h"
#include "sh4_registers.h"
#include <string.h>

template <class T>
class EnStack
{
public:
	T* items ;
	int size ;  // number of elements on Stack.
	int top ;  

	EnStack()
	{
		size=10;
		items=(T*)malloc(sizeof(T)*size);
		top=0;
	}
	~EnStack()
	{ 
		free(items); 
	}

	void push(const T& val)
	{
		if (top==size)
		{
			if (size<1000)
			{

			
			size+=10;
			items=(T*)realloc(items,sizeof(T)*size);
			}else
			{
				//printf("TOO MANY CALLS\n");
			}
			//if (items==0)
				//items=0;
		}
		if (top<size)
		{
			items[top++]=val;
		}
	}
	T pop()
	{
		if (top==0)
		{
			T rv=items[0];
			return rv;
		}
		return items[--top];
	}

	int isEmpty(){ return top == 0 ; } 
	int Count(){return top;}
} ;




EnStack<cst_entry> cst;

void GetCallStackText();

void AddCall(u32 pc_callstart,u32 pc_callretadr,u32 branchaddr,u32 calltype)
{
#ifdef TRACE
	pc_callstart&=0x1FFFFFFF;
	pc_callretadr&=0x1FFFFFFF;
	branchaddr&=0x1FFFFFFF;

	//if (1)
	{
		cst_entry ct;
		ct.pc_callstart=pc_callstart;
		ct.pc_callretadr=pc_callretadr;
		ct.branchaddr=branchaddr;
		ct.calltype=calltype;
		cst.push(ct);
		//cst.push_front(ct);
	}
#endif
}

void RemoveCall(u32 retaddress,u32 rettype)
{
	#ifdef TRACE
	retaddress&=0x1FFFFFFF;
	//if (1)
	{
		if (cst.isEmpty())
			printf("Call stack unexpected ret ,pc=%x,ret=%x\n",pc,retaddress);
		else
		{
			/*cst_entry ct=*/
			cst.pop();
			/*
			if (ct.pc_callretadr!=retaddress)
			{
				printf("Call stack wrong ret ,pc=%x,%x != %x(expected)\n",pc,retaddress,ct.pc_callretadr);
			}
			if (ct.calltype!=rettype)
			{
				printf("Call stack wrong ret type ,pc=%x,%x != %x(expected)\n",pc,rettype,ct.calltype);
			}
			*/
			//cst.pop_front();
		}
	}
	#endif
}




/*extern u8 symtab[];
extern u8 strtab[];

extern u32 symindex;
extern bool bElfLoaded;
extern Elf32_Sym * pSymTab;*/


void GetCallStackText(char* buff)
{
	//char buff_[1024*10];
	//char *buff=&buff_[0];
	buff[0]='\0';

	char tmp[1024];

	cst_entry prev_cte;
	prev_cte.branchaddr=0;//init

	bool bprev_cte=false;

	if (!cst.isEmpty())
	{
		prev_cte=cst.items[cst.top-1];
		//function info : lol
		GetSymbName(prev_cte.branchaddr,tmp,true);
		buff=strcat(buff,tmp);
		sprintf(tmp,"[+0x%x",(pc&0x1fffffff)-prev_cte.branchaddr);
		buff=strcat(buff,tmp);
		buff=strcat(buff,"](current execution)\r\n");
	}
	else
	{
		buff=strcat(buff,"Dreamcast Boot Address[");
		sprintf(tmp,"+0x%x",pc);
		buff=strcat(buff,tmp);
		buff=strcat(buff,"](current execution)\r\n");
	}

	for(int indx = cst.top-1; indx>=0; indx--)
	{
		cst_entry cte=cst.items[indx];

		if (indx!=0)
		{
			prev_cte=cst.items[indx-1];
			bprev_cte=true;
		}
		else
		{
			bprev_cte=false;
		}

		if (bprev_cte)
		{
			//function info : lol
			GetSymbName(prev_cte.branchaddr,tmp,true);
			buff=strcat(buff,tmp);
			sprintf(tmp,"(%d)[+0x%x",cte.calltype,cte.pc_callretadr-prev_cte.branchaddr);
			buff=strcat(buff,tmp);
			buff=strcat(buff,"]\r\n");
		}
		else
		{
			buff=strcat(buff,"Dreamcast Boot Address[");
			sprintf(tmp,"+0x%x",cte.pc_callretadr);
			buff=strcat(buff,tmp);
			buff=strcat(buff,"]\r\n");
		}
	}

	//printf(buff);
}

