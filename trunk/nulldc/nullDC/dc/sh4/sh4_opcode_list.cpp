#include "types.h"
#include "sh4_cpu.h"
#include "sh4_fpu.h"
#include "dc/mem/memutil.h"
#include "sh4_opcode_list.h"
#include "rec_v1\rec_v1_ops.h"
#include "plugins/plugin_manager.h"



OpCallFP* OpPtr[0x10000];
RecOpCallFP* RecOpPtr[0x10000];
OpcodeType OpTyp[0x10000];
sh4_opcodelistentry* OpDisasm[0x10000];

#define Mask_n_m 0xF00F
#define Mask_n_m_imm4 0xF000
#define Mask_n 0xF0FF
#define Mask_none 0xFFFF
#define Mask_imm8 0xFF00 
#define Mask_imm12 0xF000
#define Mask_n_imm8 0xF000
#define Mask_n_ml3bit 0xF08F
#define Mask_nh3bit 0xF1FF
#define Mask_nh2bit 0xF3FF

#define GetN(str) ((str>>8) & 0xf)
#define GetM(str) ((str>>4) & 0xf)
#define GetImm4(str) ((str>>0) & 0xf)
#define GetImm8(str) ((str>>0) & 0xff)
#define GetSImm8(str) ((s8)((str>>0) & 0xff))
#define GetImm12(str) ((str>>0) & 0xfff)
#define GetSImm12(str) (((s16)((GetImm12(str))<<4))>>3)

INLINE bool strcmp2(const char* &str1_o,const char * str2)
{
	u32 sz=0;
	const char* str1=str1_o;
	while (*str2!='\0')
	{
		if (*str1!=*str2)
		{
			return false;// nope , theyre diferent
		}
		sz++;
		str1++;str2++;//next char
	}

	str1_o+=sz;
	return true;
}
//custom format string
void OpDissCFS(char* text,const char* tx1,u32 pc,u16 opcode)
{
	while (*tx1!='\0')
	{
		if (*tx1=='<')
		{
			tx1++;
			if (strcmp2(tx1,"REG_N>"))
			{
				text+=sprintf(text,"R%d",GetN(opcode));
			}
			else if (strcmp2(tx1,"REG_M>") )
			{
				text+=sprintf(text,"R%d",GetM(opcode));
			}
			else if (strcmp2(tx1,"FREG_N>"))
			{
				text+=sprintf(text,"FR%d",GetN(opcode));
			}
			else if (strcmp2(tx1,"FREG_M>"))
			{
				text+=sprintf(text,"FR%d",GetM(opcode));
			}
			else if (strcmp2(tx1,"RM_BANK>"))
			{
				text+=sprintf(text,"R%d_BANK",GetM(opcode)&0x7);
			}
			else if (strcmp2(tx1,"DFREG_N>"))
			{
				text+=sprintf(text,"DR%d",GetN(opcode)>>1);
			}
			else if (strcmp2(tx1,"DFREG_M>"))
			{
				text+=sprintf(text,"DR%d",GetM(opcode)>>1);
			}
			else if (strcmp2(tx1,"XDFREG_N>"))
			{
				u32 t=GetN(opcode);
				if (t & 0x1)
					text+=sprintf(text,"XD%d",t>>1);
				else
					text+=sprintf(text,"DR%d",t>>1);
			}
			else if (strcmp2(tx1,"XDFREG_M>"))
			{
				u32 t=GetM(opcode);
				if (t & 0x1)
					text+=sprintf(text,"XD%d",t>>1);
				else
					text+=sprintf(text,"DR%d",t>>1);
			}
			else if (strcmp2(tx1,"disp4dw>"))
			{
				text+=sprintf(text,"0x%X",GetImm4(opcode)*4);
			}
			else if (strcmp2(tx1,"PCdisp8>"))
			{
				text+=sprintf(text,"0x%X[PC]",(pc&0xFFFFFFFC)+4+(GetImm8(opcode)<<2));
			}
			else if (strcmp2(tx1,"disp8b>"))
			{
				text+=sprintf(text,"0x%X",GetImm8(opcode)*1);
			}
			else if (strcmp2(tx1,"disp8w>"))
			{
				text+=sprintf(text,"0x%X",GetImm8(opcode)*2);
			}
			else if (strcmp2(tx1,"disp8dw>"))
			{
				text+=sprintf(text,"0x%X",GetImm8(opcode)*4);
			}
			else if (strcmp2(tx1,"GBRdisp8b>"))
			{
				text+=sprintf(text,"0x%X",GetImm8(opcode)*1 + sh4_cpu->GetRegister(Sh4RegType::reg_gbr));
			}
			else if (strcmp2(tx1,"GBRdisp8w>"))
			{
				text+=sprintf(text,"0x%X",GetImm8(opcode)*2 + sh4_cpu->GetRegister(Sh4RegType::reg_gbr));
			}
			else if (strcmp2(tx1,"GBRdisp8dw>"))
			{
				text+=sprintf(text,"0x%X",GetImm8(opcode)*4 + sh4_cpu->GetRegister(Sh4RegType::reg_gbr));
			}
			else if (strcmp2(tx1,"bdisp8>"))
			{
				text+=sprintf(text,"0x%X",((GetSImm8(opcode))*2 + 4 + pc));
			}
			else if (strcmp2(tx1,"bdisp12>"))
			{
				text+=sprintf(text,"0x%X",((GetSImm12(opcode))*2 + 4 + pc));
			}
			else if (strcmp2(tx1,"imm8>"))
			{
				text+=sprintf(text,"0x%X",GetImm8(opcode));
			}
			else if (strcmp2(tx1,"simm8>"))
			{
				text+=sprintf(text,"%d",GetSImm8(opcode));
			}
			else if (strcmp2(tx1,"simm8hex>"))
			{
				text+=sprintf(text,"0x%X",GetSImm8(opcode));
			}
			else
			{
				u32 ti=0;
				while (tx1[ti]!='\0')
				{
					if (tx1[ti]=='>')
						break;
					else
						ti++;
				}

//				char old=tx1[ti];
				//tx1[ti]='\0';
				printf("Sh4Dissasm : Tag not known\"%s\"\n",tx1);
				//tx1[ti]=old;
				
				*text='<';text++;
				*text=*tx1;
				tx1++;
				text++;
			}
		}
		else
		{
			*text=*tx1;
			tx1++;
			text++;
		}
	}

	*text='\0';
	tx1++;
	text++;
}










fpscr_type Get_fpscr()
{
	fpscr_type t;
	t.full=sh4_cpu->GetRegister(reg_fpscr);

	return t;
}
//fadd <FREG_M>,<FREG_N>
void d1111_nnnn_mmmm_0000(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fadd <FREG_M>,<FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fadd <DFREG_M>,<DFREG_N>",pc,opcode);
	}
}

//fsub <FREG_M>,<FREG_N>
void d1111_nnnn_mmmm_0001(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fsub <FREG_M>,<FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fsub <DFREG_M>,<DFREG_N>",pc,opcode);
	}
}

//fmul <FREG_M>,<FREG_N>
void d1111_nnnn_mmmm_0010(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fmul <FREG_M>,<FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fmul <DFREG_M>,<DFREG_N>",pc,opcode);
	}
}

//fdiv <FREG_M>,<FREG_N>
void d1111_nnnn_mmmm_0011(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fdiv <FREG_M>,<FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fdiv <DFREG_M>,<DFREG_N>",pc,opcode);
	}
}

//fcmp/eq <FREG_M>,<FREG_N>
void d1111_nnnn_mmmm_0100(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fcmp/eq <FREG_M>,<FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fcmp/eq <DFREG_M>,<DFREG_N>",pc,opcode);
	}
}

//fcmp/gt <FREG_M>,<FREG_N>
void d1111_nnnn_mmmm_0101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fcmp/gt <FREG_M>,<FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fcmp/gt <DFREG_M>,<DFREG_N>",pc,opcode);
	}
}

//fmov.s @(R0,<REG_M>),<FREG_N>
void d1111_nnnn_mmmm_0110(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().SZ==0)
	{
		OpDissCFS(text,"fmov.s @(R0,<REG_M>),<FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fmov.s @(R0,<REG_M>),<XDFREG_N>",pc,opcode);
	}
}

//fmov.s <FREG_M>,@(R0,<REG_N>)
void d1111_nnnn_mmmm_0111(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().SZ==0)
	{
		OpDissCFS(text,"fmov.s <FREG_M>,@(R0,<REG_N>)",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fmov.s <XDFREG_M>,@(R0,<REG_N>)",pc,opcode);
	}
}

//fmov.s @<REG_M>,<FREG_N>
void d1111_nnnn_mmmm_1000(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().SZ==0)
	{
		OpDissCFS(text,"fmov.s @<REG_M>,<FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fmov.s @<REG_M>,<XDFREG_N>",pc,opcode);
	}
}

//fmov.s @<REG_M>+,<FREG_N>
void d1111_nnnn_mmmm_1001(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().SZ==0)
	{
		OpDissCFS(text,"fmov.s @<REG_M>+,<FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fmov.s @<REG_M>+,<XDFREG_N>",pc,opcode);
	}
}

//fmov.s <FREG_M>,@<REG_N>
void d1111_nnnn_mmmm_1010(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().SZ==0)
	{
		OpDissCFS(text,"fmov.s <FREG_M>,@<REG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fmov.s <XDFREG_M>,@<REG_N>",pc,opcode);
	}
}

//fmov.s <FREG_M>,@-<REG_N>
void d1111_nnnn_mmmm_1011(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().SZ==0)
	{
		OpDissCFS(text,"fmov.s <FREG_M>,@-<REG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fmov.s <XDFREG_M>,@-<REG_N>",pc,opcode);
	}
}

//fmov <FREG_M>,<FREG_N>
void d1111_nnnn_mmmm_1100(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().SZ==0)
	{
		OpDissCFS(text,"fmov <FREG_M>,<FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fmov <XDFREG_M>,<XDFREG_N>",pc,opcode);
	}
}

//fabs <FREG_N>
void d1111_nnnn_0101_1101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fabs <FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fabs <DFREG_N>",pc,opcode);
	}
}
//FSCA FPUL, DRn//F0FD//1111_nnn0_1111_1101
void OpDissFSCA(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
			int n=GetN(opcode)&0xE;
			sprintf(text,"fsca FPUL,FR%d",n);
	}
	else
	{
		OpDissCFS(text,"fsca n/a [64bit]",pc,opcode);
	}


}

//fcnvds <DR_N>,FPUL
void d1111_nnnn_1011_1101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fcnvds n/a [32bit]",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fcnvds <DR_N>,FPUL",pc,opcode);
	}     
}

//fcnvsd FPUL,<DR_N>
void d1111_nnnn_1010_1101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fcnvsd n/a [32bit]",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fcnvsd FPUL,<DR_N>",pc,opcode);
	}
}

//fipr <FV_M>,<FV_N>
void OpDissfipr(char* text,const char* tx1,u32 pc,u16 opcode)
{	
	if (Get_fpscr().PR==0)
	{
		int n=GetN(opcode);
		int m=GetM(opcode);
		sprintf(text,"fipr FV%d,FV%d",m,n);
	}
	else
	{
		OpDissCFS(text,"fipr n/a [64bit]",pc,opcode);
	}


}
//fldi0 <FREG_N>
void d1111_nnnn_1000_1101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fldi0 <FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fldi0 n/a [64bit]",pc,opcode);
	}
}

//fldi1 <FREG_N>
void d1111_nnnn_1001_1101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fldi1 <FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fldi1 n/a [64bit]",pc,opcode);
	}
}

//flds <FREG_N>,FPUL
void d1111_nnnn_0001_1101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"flds <FREG_N>,FPUL",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"flds <DFREG_N>,FPUL",pc,opcode);
	}
}
//float FPUL,<FREG_N>
void d1111_nnnn_0010_1101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"float FPUL,<FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"float FPUL,<DFREG_N>",pc,opcode);
	}	      
}
//fneg <FREG_N>
void d1111_nnnn_0100_1101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fneg <FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fneg <DFREG_N>",pc,opcode);
	}
}
//frchg	- no need
//fschg	- no need

//fsqrt <FREG_N>                
void d1111_nnnn_0110_1101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fsqrt <FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fsqrt <DFREG_N>",pc,opcode);
	}
}

//ftrc <FREG_N>, FPUL
void d1111_nnnn_0011_1101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"ftrc <FREG_N>, FPUL",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"ftrc <DFREG_N>, FPUL",pc,opcode);
	}
}
//fsts FPUL,<FREG_N>
void d1111_nnnn_0000_1101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fsts FPUL,<FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fsts FPUL,<DFREG_N>",pc,opcode);
	}
}
//ftrv xmtrx,<FV_N>
void OpDissftrv(char* text,const char* tx1,u32 pc,u16 opcode)
{	
	if (Get_fpscr().PR==0)
	{
		int n=GetN(opcode)>>2;
		sprintf(text,"ftrv xmtrx,FV%d ",n);
	}
	else
	{
		OpDissCFS(text,"ftrv n/a [64 bit]",pc,opcode);
	}
}
//fmac <FREG_0>,<FREG_M>,<FREG_N> 
void OpDissfmac(char* text,const char* tx1,u32 pc,u16 opcode)
{	
	if (Get_fpscr().PR==0)
	{
		int n=GetN(opcode);
		int m=GetM(opcode);
		sprintf(text,"fmac FR0,FR%d,FR%d ",n,m);
	}
	else
	{
		OpDissCFS(text,"fmac n/a [64 bit]",pc,opcode);
	}
}

//FSRRA <FREG_N> (1111nnnn 01111101)
void d1111_nnnn_0111_1101(char* text,const char* tx1,u32 pc,u16 opcode)
{
	if (Get_fpscr().PR==0)
	{
		OpDissCFS(text,"fsrra <FREG_N>",pc,opcode);
	}
	else
	{
		OpDissCFS(text,"fsrra <DFREG_N>",pc,opcode);
	}	
}

void OpNoDiss(char* text,const char* tx1,u32 pc,u16 opcode)
{
	sprintf(text,"Missing Disassemble");
}

void dissasm_Break(char* text,const char* tx1,u32 pc,u16 opcode)
{
	DissasembleOpcode(0,pc,text);

	strcat(text,"[Break]");
}

void dissasm_GDROM(char* text,const char* tx1,u32 pc,u16 opcode)
{
	sprintf(text,"gdrom opcode *ADD REAL DISSASM*");
}
//jump for now..
//how bout a decoded switch list ? could that _much_ faster ?
//also , it could be realy autogenerated (hint hint :P)
//
sh4_opcodelistentry opcodes[]=
{
	
	//CPU
	{rec_shil_icpu_nimp	,i0000_nnnn_0000_0010	,Mask_n		,0x0002	,Normal				,OpDissCFS,"stc SR,<REG_N>"},	//stc SR,<REG_N>                
	{rec_shil_i0000_nnnn_0001_0010	,i0000_nnnn_0001_0010	,Mask_n		,0x0012	,Normal				,OpDissCFS,"stc GBR,<REG_N>"},	//stc GBR,<REG_N>               
	{rec_shil_i0000_nnnn_0010_0010	,i0000_nnnn_0010_0010	,Mask_n		,0x0022	,Normal				,OpDissCFS,"stc VBR,<REG_N>"},	//stc VBR,<REG_N>               
	{rec_shil_i0000_nnnn_0011_0010	,i0000_nnnn_0011_0010	,Mask_n		,0x0032	,Normal				,OpDissCFS,"stc SSR,<REG_N>"},	//stc SSR,<REG_N>               
	{rec_shil_i0000_nnnn_0100_0010	,i0000_nnnn_0100_0010	,Mask_n		,0x0042	,Normal				,OpDissCFS,"stc SPC,<REG_N>"},	//stc SPC,<REG_N>               
	{rec_shil_i0000_nnnn_1mmm_0010	,i0000_nnnn_1mmm_0010	,Mask_n_ml3bit,0x0082,Normal			,OpDissCFS,"stc R0_BANK,<REG_N>"},	//stc R0_BANK,<REG_N>           
	{rec_shil_i0000_nnnn_0010_0011	,i0000_nnnn_0010_0011	,Mask_n		,0x0023	,Branch_rel_d		,OpDissCFS,"braf <REG_N>"},	//braf <REG_N>                  
	{rec_shil_i0000_nnnn_0000_0011	,i0000_nnnn_0000_0011	,Mask_n		,0x0003	,Branch_rel_d		,OpDissCFS,"bsrf <REG_N>"},	//bsrf <REG_N>                  
	{rec_shil_i0000_nnnn_1100_0011	,i0000_nnnn_1100_0011	,Mask_n		,0x00C3	,Normal				,OpDissCFS,"movca.l R0, @<REG_N>"},	//movca.l R0, @<REG_N>          
	{rec_shil_i0000_nnnn_1001_0011	,i0000_nnnn_1001_0011	,Mask_n		,0x0093	,Normal				,OpDissCFS,"ocbi @<REG_N>"},	//ocbi @<REG_N>                 
	{rec_shil_i0000_nnnn_1010_0011	,i0000_nnnn_1010_0011	,Mask_n		,0x00A3	,Normal				,OpDissCFS,"ocbp @<REG_N>"},	//ocbp @<REG_N>                 
	{rec_shil_i0000_nnnn_1011_0011	,i0000_nnnn_1011_0011	,Mask_n		,0x00B3	,Normal				,OpDissCFS,"ocbwb @<REG_N>"},	//ocbwb @<REG_N>                
	{rec_shil_i0000_nnnn_1000_0011	,i0000_nnnn_1000_0011	,Mask_n		,0x0083	,Normal				,OpDissCFS,"pref @<REG_N>"},	//pref @<REG_N>                 
	{rec_shil_i0000_nnnn_mmmm_0100	,i0000_nnnn_mmmm_0100	,Mask_n_m	,0x0004	,Normal				,OpDissCFS,"mov.b <REG_M>,@(R0,<REG_N>)"},	//mov.b <REG_M>,@(R0,<REG_N>)   
	{rec_shil_i0000_nnnn_mmmm_0101	,i0000_nnnn_mmmm_0101	,Mask_n_m	,0x0005	,Normal				,OpDissCFS,"mov.w <REG_M>,@(R0,<REG_N>)"},	//mov.w <REG_M>,@(R0,<REG_N>)   
	{rec_shil_i0000_nnnn_mmmm_0110	,i0000_nnnn_mmmm_0110	,Mask_n_m	,0x0006	,Normal				,OpDissCFS,"mov.l <REG_M>,@(R0,<REG_N>)"},	//mov.l <REG_M>,@(R0,<REG_N>)   
	{rec_shil_i0000_nnnn_mmmm_0111	,i0000_nnnn_mmmm_0111	,Mask_n_m	,0x0007	,Normal				,OpDissCFS,"mul.l <REG_M>,<REG_N>"},	//mul.l <REG_M>,<REG_N>         
	{rec_shil_i0000_0000_0010_1000	,i0000_0000_0010_1000	,Mask_none	,0x0028	,Normal				,OpDissCFS,"clrmac"},	//clrmac                        
	{rec_shil_i0000_0000_0100_1000	,i0000_0000_0100_1000	,Mask_none	,0x0048	,Normal				,OpDissCFS,"clrs"},	//clrs                          
	{rec_shil_i0000_0000_0000_1000	,i0000_0000_0000_1000	,Mask_none	,0x0008	,Normal				,OpDissCFS,"clrt"},	//clrt                          
	{rec_shil_i0000_0000_0011_1000	,i0000_0000_0011_1000	,Mask_none	,0x0038	,Normal				,OpDissCFS,"ldtlb"},//ldtlb                         
	{rec_shil_i0000_0000_0101_1000	,i0000_0000_0101_1000	,Mask_none	,0x0058	,Normal				,OpDissCFS,"sets"},	//sets                          
	{rec_shil_i0000_0000_0001_1000	,i0000_0000_0001_1000	,Mask_none	,0x0018	,Normal				,OpDissCFS,"sett"},	//sett                          
	{rec_shil_i0000_0000_0001_1001	,i0000_0000_0001_1001	,Mask_none	,0x0019	,Normal				,OpDissCFS,"div0u"},//div0u                         
	{rec_shil_i0000_nnnn_0010_1001	,i0000_nnnn_0010_1001	,Mask_n		,0x0029	,Normal				,OpDissCFS,"movt <REG_N>"},	//movt <REG_N>                  
	{rec_shil_i0000_0000_0000_1001	,i0000_0000_0000_1001	,Mask_none	,0x0009	,Normal				,OpDissCFS,"nop"},	//nop                           
	{rec_shil_i0000_nnnn_0101_1010	,i0000_nnnn_0101_1010	,Mask_n		,0x005A	,Normal				,OpDissCFS,"sts FPUL,<REG_N>"},	//sts FPUL,<REG_N>
	{rec_shil_icpu_nimp				,i0000_nnnn_0110_1010	,Mask_n		,0x006A	,Normal				,OpDissCFS,"sts FPSCR,<REG_N>"},//sts FPSCR,<REG_N>             
	{rec_shil_i0000_nnnn_1111_1010	,i0000_nnnn_1111_1010	,Mask_n		,0x00FA	,Normal				,OpDissCFS,"stc DBR,<REG_N>"},	//stc DBR,<REG_N>             
	{rec_shil_i0000_nnnn_0000_1010	,i0000_nnnn_0000_1010	,Mask_n		,0x000A	,Normal				,OpDissCFS,"sts MACH,<REG_N>"},	//sts MACH,<REG_N>              
	{rec_shil_i0000_nnnn_0001_1010	,i0000_nnnn_0001_1010	,Mask_n		,0x001A	,Normal				,OpDissCFS,"sts MACL,<REG_N>"},	//sts MACL,<REG_N>              
	{rec_shil_i0000_nnnn_0010_1010	,i0000_nnnn_0010_1010	,Mask_n		,0x002A	,Normal				,OpDissCFS,"sts PR,<REG_N>"},	//sts PR,<REG_N>                
	{rec_shil_i0000_0000_0010_1011	,i0000_0000_0010_1011	,Mask_none	,0x002B	,WritesPC			,OpDissCFS,"rte"},	//rte                           
	{rec_shil_i0000_0000_0000_1011	,i0000_0000_0000_1011	,Mask_none	,0x000B	,Branch_dir_d		,OpDissCFS,"rts"},	//rts                           
	{rec_shil_i0000_0000_0001_1011	,i0000_0000_0001_1011	,Mask_none	,0x001B	,ReadWritePC		,OpDissCFS,"sleep"},	//sleep                         
	{rec_shil_i0000_nnnn_mmmm_1100	,i0000_nnnn_mmmm_1100	,Mask_n_m	,0x000C	,Normal				,OpDissCFS,"mov.b @(R0,<REG_M>),<REG_N>"},	//mov.b @(R0,<REG_M>),<REG_N>   
	{rec_shil_i0000_nnnn_mmmm_1101	,i0000_nnnn_mmmm_1101	,Mask_n_m	,0x000D	,Normal				,OpDissCFS,"mov.w @(R0,<REG_M>),<REG_N>"},	//mov.w @(R0,<REG_M>),<REG_N>   
	{rec_shil_i0000_nnnn_mmmm_1110	,i0000_nnnn_mmmm_1110	,Mask_n_m	,0x000E	,Normal				,OpDissCFS,"mov.l @(R0,<REG_M>),<REG_N>"},	//mov.l @(R0,<REG_M>),<REG_N>   
	{rec_shil_i0000_nnnn_mmmm_1111	,i0000_nnnn_mmmm_1111	,Mask_n_m	,0x000F	,Normal				,OpDissCFS,"mac.l @<REG_M>+,@<REG_N>+"},	//mac.l @<REG_M>+,@<REG_N>+     
	{rec_shil_i0001_nnnn_mmmm_iiii	,i0001_nnnn_mmmm_iiii	,Mask_n_imm8,0x1000	,Normal				,OpDissCFS,"mov.l <REG_M>,@(<disp8dw>,<REG_N>)"},	//mov.l <REG_M>,@(<disp>,<REG_N>)
	{rec_shil_i0010_nnnn_mmmm_0000	,i0010_nnnn_mmmm_0000	,Mask_n_m	,0x2000	,Normal				,OpDissCFS,"mov.b <REG_M>,@<REG_N>"},	//mov.b <REG_M>,@<REG_N>        
	{rec_shil_i0010_nnnn_mmmm_0001	,i0010_nnnn_mmmm_0001	,Mask_n_m	,0x2001	,Normal				,OpDissCFS,"mov.w <REG_M>,@<REG_N>"},	// mov.w <REG_M>,@<REG_N>        
	{rec_shil_i0010_nnnn_mmmm_0010	,i0010_nnnn_mmmm_0010	,Mask_n_m	,0x2002	,Normal				,OpDissCFS,"mov.l <REG_M>,@<REG_N>"},	// mov.l <REG_M>,@<REG_N>        
	{rec_shil_i0010_nnnn_mmmm_0100	,i0010_nnnn_mmmm_0100	,Mask_n_m	,0x2004	,Normal				,OpDissCFS,"mov.b <REG_M>,@-<REG_N>"},	// mov.b <REG_M>,@-<REG_N>       
	{rec_shil_i0010_nnnn_mmmm_0101	,i0010_nnnn_mmmm_0101	,Mask_n_m	,0x2005	,Normal				,OpDissCFS,"mov.w <REG_M>,@-<REG_N>"},	//mov.w <REG_M>,@-<REG_N>       
	{rec_shil_i0010_nnnn_mmmm_0110	,i0010_nnnn_mmmm_0110	,Mask_n_m	,0x2006	,Normal				,OpDissCFS,"mov.l <REG_M>,@-<REG_N>"},	//mov.l <REG_M>,@-<REG_N>       
	{rec_shil_i0010_nnnn_mmmm_0111	,i0010_nnnn_mmmm_0111	,Mask_n_m	,0x2007	,Normal				,OpDissCFS,"div0s <REG_M>,<REG_N>"},	// div0s <REG_M>,<REG_N>         
	{rec_shil_i0010_nnnn_mmmm_1000	,i0010_nnnn_mmmm_1000	,Mask_n_m	,0x2008	,Normal				,OpDissCFS,"tst <REG_M>,<REG_N>"},	// tst <REG_M>,<REG_N>           
	{rec_shil_i0010_nnnn_mmmm_1001	,i0010_nnnn_mmmm_1001	,Mask_n_m	,0x2009	,Normal				,OpDissCFS,"and <REG_M>,<REG_N>"},	//and <REG_M>,<REG_N>           
	{rec_shil_i0010_nnnn_mmmm_1010	,i0010_nnnn_mmmm_1010	,Mask_n_m	,0x200A	,Normal				,OpDissCFS,"xor <REG_M>,<REG_N>"},	//xor <REG_M>,<REG_N>           
	{rec_shil_i0010_nnnn_mmmm_1011	,i0010_nnnn_mmmm_1011	,Mask_n_m	,0x200B	,Normal				,OpDissCFS,"or <REG_M>,<REG_N>"},	//or <REG_M>,<REG_N>            
	{rec_shil_i0010_nnnn_mmmm_1100	,i0010_nnnn_mmmm_1100	,Mask_n_m	,0x200C	,Normal				,OpDissCFS,"cmp/str <REG_M>,<REG_N>"},	//cmp/str <REG_M>,<REG_N>       
	{rec_shil_i0010_nnnn_mmmm_1101	,i0010_nnnn_mmmm_1101	,Mask_n_m	,0x200D	,Normal				,OpDissCFS,"xtrct <REG_M>,<REG_N>"},	//xtrct <REG_M>,<REG_N>         
	{rec_shil_i0010_nnnn_mmmm_1110	,i0010_nnnn_mmmm_1110	,Mask_n_m	,0x200E	,Normal				,OpDissCFS,"mulu.w <REG_M>,<REG_N>"},	//mulu.w <REG_M>,<REG_N>          
	{rec_shil_i0010_nnnn_mmmm_1111	,i0010_nnnn_mmmm_1111	,Mask_n_m	,0x200F	,Normal				,OpDissCFS,"muls.w <REG_M>,<REG_N>"},	//muls.w <REG_M>,<REG_N>          
	{rec_shil_i0011_nnnn_mmmm_0000	,i0011_nnnn_mmmm_0000	,Mask_n_m	,0x3000	,Normal				,OpDissCFS,"cmp/eq <REG_M>,<REG_N>"},	// cmp/eq <REG_M>,<REG_N>        
	{rec_shil_i0011_nnnn_mmmm_0010	,i0011_nnnn_mmmm_0010	,Mask_n_m	,0x3002	,Normal				,OpDissCFS,"cmp/hs <REG_M>,<REG_N>"},	// cmp/hs <REG_M>,<REG_N>        
	{rec_shil_i0011_nnnn_mmmm_0011	,i0011_nnnn_mmmm_0011	,Mask_n_m	,0x3003	,Normal				,OpDissCFS,"cmp/ge <REG_M>,<REG_N>"},	//cmp/ge <REG_M>,<REG_N>        
	{rec_shil_i0011_nnnn_mmmm_0100	,i0011_nnnn_mmmm_0100	,Mask_n_m	,0x3004	,Normal				,OpDissCFS,"div1 <REG_M>,<REG_N>"},	//div1 <REG_M>,<REG_N>          
	{rec_shil_i0011_nnnn_mmmm_0101	,i0011_nnnn_mmmm_0101	,Mask_n_m	,0x3005	,Normal				,OpDissCFS,"dmulu.l <REG_M>,<REG_N>"},	//dmulu.l <REG_M>,<REG_N>       
	{rec_shil_i0011_nnnn_mmmm_0110	,i0011_nnnn_mmmm_0110	,Mask_n_m	,0x3006	,Normal				,OpDissCFS,"cmp/hi <REG_M>,<REG_N>"},	// cmp/hi <REG_M>,<REG_N>        
	{rec_shil_i0011_nnnn_mmmm_0111	,i0011_nnnn_mmmm_0111	,Mask_n_m	,0x3007	,Normal				,OpDissCFS,"cmp/gt <REG_M>,<REG_N>"},	//cmp/gt <REG_M>,<REG_N>        
	{rec_shil_i0011_nnnn_mmmm_1000	,i0011_nnnn_mmmm_1000	,Mask_n_m	,0x3008	,Normal				,OpDissCFS,"sub <REG_M>,<REG_N>"},	// sub <REG_M>,<REG_N>           
	{rec_shil_i0011_nnnn_mmmm_1010	,i0011_nnnn_mmmm_1010	,Mask_n_m	,0x300A	,Normal				,OpDissCFS,"subc <REG_M>,<REG_N>"},	//subc <REG_M>,<REG_N>          
	{rec_shil_i0011_nnnn_mmmm_1011	,i0011_nnnn_mmmm_1011	,Mask_n_m	,0x300B	,Normal				,OpDissCFS,"subv <REG_M>,<REG_N>"},	//subv <REG_M>,<REG_N>          
	{rec_shil_i0011_nnnn_mmmm_1100	,i0011_nnnn_mmmm_1100	,Mask_n_m	,0x300C	,Normal				,OpDissCFS,"add <REG_M>,<REG_N>"},	//add <REG_M>,<REG_N>           
	{rec_shil_i0011_nnnn_mmmm_1101	,i0011_nnnn_mmmm_1101	,Mask_n_m	,0x300D	,Normal				,OpDissCFS,"dmuls.l <REG_M>,<REG_N>"},	//dmuls.l <REG_M>,<REG_N>       
	{rec_shil_i0011_nnnn_mmmm_1110	,i0011_nnnn_mmmm_1110	,Mask_n_m	,0x300E	,Normal				,OpDissCFS,"addc <REG_M>,<REG_N>"},	//addc <REG_M>,<REG_N>          
	{rec_shil_i0011_nnnn_mmmm_1111	,i0011_nnnn_mmmm_1111	,Mask_n_m	,0x300F	,Normal				,OpDissCFS,"addv <REG_M>,<REG_N>"},	// addv <REG_M>,<REG_N>          
	{rec_shil_i0100_nnnn_0101_0010	,i0100_nnnn_0101_0010	,Mask_n		,0x4052	,Normal				,OpDissCFS,"addv <REG_M>,<REG_N>"},	//sts.l FPUL,@-<REG_N>          
	{rec_shil_icpu_nimp				,i0100_nnnn_0110_0010	,Mask_n		,0x4062	,Normal				,OpDissCFS,"sts.l FPSCR,@-<REG_N>"},	//sts.l FPSCR,@-<REG_N>         
	{rec_shil_i0100_nnnn_0000_0010	,i0100_nnnn_0000_0010	,Mask_n		,0x4002	,Normal				,OpDissCFS,"sts.l MACH,@-<REG_N>"},	//sts.l MACH,@-<REG_N>          
	{rec_shil_i0100_nnnn_0001_0010	,i0100_nnnn_0001_0010	,Mask_n		,0x4012	,Normal				,OpDissCFS,"sts.l MACL,@-<REG_N> "},	//sts.l MACL,@-<REG_N>          
	{rec_shil_i0100_nnnn_0010_0010	,i0100_nnnn_0010_0010	,Mask_n		,0x4022	,Normal				,OpDissCFS,"sts.l PR,@-<REG_N> "},	//sts.l PR,@-<REG_N>            
	{rec_shil_icpu_nimp				,i0100_nnnn_0000_0011	,Mask_n		,0x4003	,Normal				,OpDissCFS,"stc.l SR,@-<REG_N> "},	//stc.l SR,@-<REG_N>            
	{rec_shil_i0100_nnnn_0001_0011	,i0100_nnnn_0001_0011	,Mask_n		,0x4013	,Normal				,OpDissCFS,"stc.l GBR,@-<REG_N> "},	//stc.l GBR,@-<REG_N>           
	{rec_shil_i0100_nnnn_0010_0011	,i0100_nnnn_0010_0011	,Mask_n		,0x4023	,Normal				,OpDissCFS,"stc.l VBR,@-<REG_N> "},	//stc.l VBR,@-<REG_N>           
	{rec_shil_i0100_nnnn_0011_0011	,i0100_nnnn_0011_0011	,Mask_n		,0x4033	,Normal				,OpDissCFS,"stc.l SSR,@-<REG_N> "},	//stc.l SSR,@-<REG_N>           
	{rec_shil_i0100_nnnn_0100_0011	,i0100_nnnn_0100_0011	,Mask_n		,0x4043	,Normal				,OpDissCFS,"stc.l SPC,@-<REG_N> "},	//stc.l SPC,@-<REG_N>           
	{rec_shil_i0100_nnnn_1mmm_0011	,i0100_nnnn_1mmm_0011	,Mask_n_ml3bit,0x4083,Normal			,OpDissCFS,"stc <RM_BANK>,@-<REG_N>"},//stc Rm_BANK,@-<REG_N>         
	{rec_shil_i0100_nnnn_0000_0110	,i0100_nnnn_0000_0110	,Mask_n		,0x4006	,Normal				,OpDissCFS,"lds.l @<REG_N>+,MACH"},	//lds.l @<REG_N>+,MACH          
	{rec_shil_i0100_nnnn_0001_0110	,i0100_nnnn_0001_0110	,Mask_n		,0x4016	,Normal				,OpDissCFS,"lds.l @<REG_N>+,MACL"},	//lds.l @<REG_N>+,MACL          
	{rec_shil_i0100_nnnn_0010_0110	,i0100_nnnn_0010_0110	,Mask_n		,0x4026	,Normal				,OpDissCFS,"lds.l @<REG_N>+,PR"},	//lds.l @<REG_N>+,PR            
	{rec_shil_i0100_nnnn_0101_0110	,i0100_nnnn_0101_0110	,Mask_n		,0x4056	,Normal				,OpDissCFS,"lds.l @<REG_N>+,FPUL"},	//lds.l @<REG_N>+,FPUL          
	{rec_shil_icpu_nimp				,i0100_nnnn_0110_0110	,Mask_n		,0x4066	,Normal				,OpDissCFS,"lds.l @<REG_N>+,FPSCR"},	//lds.l @<REG_N>+,FPSCR         
	{rec_shil_icpu_nimp				,i0100_nnnn_0000_0111	,Mask_n		,0x4007	,WritesSR			,OpDissCFS,"ldc.l @<REG_N>+,SR"},	//ldc.l @<REG_N>+,SR            
	{rec_shil_i0100_nnnn_0001_0111	,i0100_nnnn_0001_0111	,Mask_n		,0x4017	,Normal				,OpDissCFS,"ldc.l @<REG_N>+,GBR"},	//ldc.l @<REG_N>+,GBR           
	{rec_shil_i0100_nnnn_0010_0111	,i0100_nnnn_0010_0111	,Mask_n		,0x4027	,Normal				,OpDissCFS,"ldc.l @<REG_N>+,VBR"},	//ldc.l @<REG_N>+,VBR           
	{rec_shil_i0100_nnnn_0011_0111	,i0100_nnnn_0011_0111	,Mask_n		,0x4037	,Normal				,OpDissCFS,"ldc.l @<REG_N>+,SSR"},	//ldc.l @<REG_N>+,SSR           
	{rec_shil_i0100_nnnn_0100_0111	,i0100_nnnn_0100_0111	,Mask_n		,0x4047	,Normal				,OpDissCFS,"ldc.l @<REG_N>+,SPC"},	//ldc.l @<REG_N>+,SPC           
	{rec_shil_i0100_nnnn_1mmm_0111	,i0100_nnnn_1mmm_0111	,Mask_n_ml3bit,0x4087,Normal			,OpDissCFS,"ldc.l @<REG_N>+,RM_BANK"},//ldc.l @<REG_N>+,RM_BANK       
	{rec_shil_i0100_nnnn_0000_1010	,i0100_nnnn_0000_1010	,Mask_n		,0x400A	,Normal				,OpDissCFS,"lds <REG_N>,MACH"},	//lds <REG_N>,MACH              
	{rec_shil_i0100_nnnn_0001_1010	,i0100_nnnn_0001_1010	,Mask_n		,0x401A	,Normal				,OpDissCFS,"lds <REG_N>,MACL"},	//lds <REG_N>,MACL              
	{rec_shil_i0100_nnnn_0010_1010	,i0100_nnnn_0010_1010	,Mask_n		,0x402A	,Normal				,OpDissCFS,"lds <REG_N>,PR"},	//lds <REG_N>,PR                
	{rec_shil_i0100_nnnn_0101_1010	,i0100_nnnn_0101_1010	,Mask_n		,0x405A	,Normal				,OpDissCFS,"lds <REG_N>,FPUL"},	//lds <REG_N>,FPUL              
	{rec_shil_icpu_nimp				,i0100_nnnn_0110_1010	,Mask_n		,0x406A	,Normal				,OpDissCFS,"lds <REG_N>,FPSCR"},	//lds <REG_N>,FPSCR             
	{rec_shil_i0100_nnnn_1111_1010	,i0100_nnnn_1111_1010	,Mask_n		,0x40FA	,Normal				,OpDissCFS,"ldc <REG_N>,DBR"},	//ldc <REG_N>,DBR                
	{rec_shil_icpu_nimp				,i0100_nnnn_0000_1110	,Mask_n		,0x400E	,WritesSR			,OpDissCFS,"ldc <REG_N>,SR"},	//ldc <REG_N>,SR                
	{rec_shil_i0100_nnnn_0001_1110	,i0100_nnnn_0001_1110	,Mask_n		,0x401E	,Normal				,OpDissCFS,"ldc <REG_N>,GBR"},	//ldc <REG_N>,GBR               
	{rec_shil_i0100_nnnn_0010_1110	,i0100_nnnn_0010_1110	,Mask_n		,0x402E	,Normal				,OpDissCFS,"ldc <REG_N>,VBR"},	//ldc <REG_N>,VBR               
	{rec_shil_i0100_nnnn_0011_1110	,i0100_nnnn_0011_1110	,Mask_n		,0x403E	,Normal				,OpDissCFS,"ldc <REG_N>,SSR"},	//ldc <REG_N>,SSR               
	{rec_shil_i0100_nnnn_0100_1110	,i0100_nnnn_0100_1110	,Mask_n		,0x404E	,Normal				,OpDissCFS,"ldc <REG_N>,SPC"},	//ldc <REG_N>,SPC               
	{rec_shil_i0100_nnnn_1mmm_1110	,i0100_nnnn_1mmm_1110	,Mask_n_ml3bit,0x408E,Normal			,OpDissCFS,"ldc <REG_N>,<RM_BANK>"},	//ldc <REG_N>,<RM_BANK>               
	{rec_shil_i0100_nnnn_0000_0000	,i0100_nnnn_0000_0000	,Mask_n		,0x4000	,Normal				,OpDissCFS,"shll <REG_N>"},	//shll <REG_N>                  
	{rec_shil_i0100_nnnn_0001_0000	,i0100_nnnn_0001_0000	,Mask_n		,0x4010	,Normal				,OpDissCFS,"dt <REG_N>"},	//dt <REG_N>                    
	{rec_shil_i0100_nnnn_0010_0000	,i0100_nnnn_0010_0000	,Mask_n		,0x4020	,Normal				,OpDissCFS,"shal <REG_N>"},	//shal <REG_N>                  
	{rec_shil_i0100_nnnn_0000_0001	,i0100_nnnn_0000_0001	,Mask_n		,0x4001	,Normal				,OpDissCFS,"shlr <REG_N>"},	//shlr <REG_N>                  
	{rec_shil_i0100_nnnn_0001_0001	,i0100_nnnn_0001_0001	,Mask_n		,0x4011	,Normal				,OpDissCFS,"cmp/pz <REG_N>"},	//cmp/pz <REG_N>                
	{rec_shil_i0100_nnnn_0010_0001	,i0100_nnnn_0010_0001	,Mask_n		,0x4021	,Normal				,OpDissCFS,"shar <REG_N>"},	//shar <REG_N>                  
	{rec_shil_i0100_nnnn_0010_0100	,i0100_nnnn_0010_0100	,Mask_n		,0x4024	,Normal				,OpDissCFS,"rotcl <REG_N>"},	//rotcl <REG_N>                 
	{rec_shil_i0100_nnnn_0000_0100	,i0100_nnnn_0000_0100	,Mask_n		,0x4004	,Normal				,OpDissCFS,"rotl <REG_N>"},	//rotl <REG_N>                  
	{rec_shil_i0100_nnnn_0001_0101	,i0100_nnnn_0001_0101	,Mask_n		,0x4015	,Normal				,OpDissCFS,"cmp/pl <REG_N>"},	//cmp/pl <REG_N>                
	{rec_shil_i0100_nnnn_0010_0101	,i0100_nnnn_0010_0101	,Mask_n		,0x4025	,Normal				,OpDissCFS,"rotcr <REG_N>"},	//rotcr <REG_N>                 
	{rec_shil_i0100_nnnn_0000_0101	,i0100_nnnn_0000_0101	,Mask_n		,0x4005	,Normal				,OpDissCFS,"rotr <REG_N>"},	//rotr <REG_N>                  
	{rec_shil_i0100_nnnn_0000_1000	,i0100_nnnn_0000_1000	,Mask_n		,0x4008	,Normal				,OpDissCFS,"shll2 <REG_N>"},	//shll2 <REG_N>                 
	{rec_shil_i0100_nnnn_0001_1000	,i0100_nnnn_0001_1000	,Mask_n		,0x4018	,Normal				,OpDissCFS,"shll8 <REG_N>"},	//shll8 <REG_N>                 
	{rec_shil_i0100_nnnn_0010_1000	,i0100_nnnn_0010_1000	,Mask_n		,0x4028	,Normal				,OpDissCFS,"shll16 <REG_N>"},	//shll16 <REG_N>                
	{rec_shil_i0100_nnnn_0000_1001	,i0100_nnnn_0000_1001	,Mask_n		,0x4009	,Normal				,OpDissCFS,"shlr2 <REG_N>"},	//shlr2 <REG_N>                 
	{rec_shil_i0100_nnnn_0001_1001	,i0100_nnnn_0001_1001	,Mask_n		,0x4019	,Normal				,OpDissCFS,"shlr8 <REG_N>"},	//shlr8 <REG_N>                 
	{rec_shil_i0100_nnnn_0010_1001	,i0100_nnnn_0010_1001	,Mask_n		,0x4029	,Normal				,OpDissCFS,"shlr16 <REG_N>"},	//shlr16 <REG_N>                
	{rec_shil_i0100_nnnn_0010_1011	,i0100_nnnn_0010_1011	,Mask_n		,0x402B	,Branch_dir_d		,OpDissCFS,"jmp @<REG_N>"},		//jmp @<REG_N>                  
	{rec_shil_i0100_nnnn_0000_1011	,i0100_nnnn_0000_1011	,Mask_n		,0x400B	,Branch_dir_d		,OpDissCFS,"jsr @<REG_N>"},	//jsr @<REG_N>                  
	{rec_shil_i0100_nnnn_0001_1011	,i0100_nnnn_0001_1011	,Mask_n		,0x401B	,Normal				,OpDissCFS,"tas.b @<REG_N>"},	//tas.b @<REG_N>                
	{rec_shil_i0100_nnnn_mmmm_1100	,i0100_nnnn_mmmm_1100	,Mask_n_m	,0x400C	,Normal				,OpDissCFS,"shad <REG_M>,<REG_N>"},	//shad <REG_M>,<REG_N>          
	{rec_shil_i0100_nnnn_mmmm_1101	,i0100_nnnn_mmmm_1101	,Mask_n_m	,0x400D	,Normal				,OpDissCFS,"shld <REG_M>,<REG_N>"},	//shld <REG_M>,<REG_N>          
	{rec_shil_i0100_nnnn_mmmm_1111	,i0100_nnnn_mmmm_1111	,Mask_n_m	,0x400F	,Normal				,OpDissCFS,"mac.w @<REG_M>+,@<REG_N>+"},	//mac.w @<REG_M>+,@<REG_N>+     
	{rec_shil_i0101_nnnn_mmmm_iiii	,i0101_nnnn_mmmm_iiii	,Mask_n_m_imm4,0x5000,Normal			,OpDissCFS,"mov.l @(<disp4dw>,<REG_M>),<REG_N>"},//mov.l @(<disp>,<REG_M>),<REG_N>
	{rec_shil_i0110_nnnn_mmmm_0000	,i0110_nnnn_mmmm_0000	,Mask_n_m	,0x6000	,Normal				,OpDissCFS,"mov.b @<REG_M>,<REG_N>"},	//mov.b @<REG_M>,<REG_N>        
	{rec_shil_i0110_nnnn_mmmm_0001	,i0110_nnnn_mmmm_0001	,Mask_n_m	,0x6001	,Normal				,OpDissCFS,"mov.w @<REG_M>,<REG_N>"},	//mov.w @<REG_M>,<REG_N>        
	{rec_shil_i0110_nnnn_mmmm_0010	,i0110_nnnn_mmmm_0010	,Mask_n_m	,0x6002	,Normal				,OpDissCFS,"mov.l @<REG_M>,<REG_N>"},	//mov.l @<REG_M>,<REG_N>        
	{rec_shil_i0110_nnnn_mmmm_0011	,i0110_nnnn_mmmm_0011	,Mask_n_m	,0x6003	,Normal				,OpDissCFS,"mov <REG_M>,<REG_N>"},	//mov <REG_M>,<REG_N>           
	{rec_shil_i0110_nnnn_mmmm_0100	,i0110_nnnn_mmmm_0100	,Mask_n_m	,0x6004	,Normal				,OpDissCFS,"mov.b @<REG_M>+,<REG_N>"},	//mov.b @<REG_M>+,<REG_N>       
	{rec_shil_i0110_nnnn_mmmm_0101	,i0110_nnnn_mmmm_0101	,Mask_n_m	,0x6005	,Normal				,OpDissCFS,"mov.w @<REG_M>+,<REG_N>"},	//mov.w @<REG_M>+,<REG_N>       
	{rec_shil_i0110_nnnn_mmmm_0110	,i0110_nnnn_mmmm_0110	,Mask_n_m	,0x6006	,Normal				,OpDissCFS,"mov.l @<REG_M>+,<REG_N>"},	//mov.l @<REG_M>+,<REG_N>       
	{rec_shil_i0110_nnnn_mmmm_0111	,i0110_nnnn_mmmm_0111	,Mask_n_m	,0x6007	,Normal				,OpDissCFS,"not <REG_M>,<REG_N>"},	//not <REG_M>,<REG_N>           
	{rec_shil_i0110_nnnn_mmmm_1000	,i0110_nnnn_mmmm_1000	,Mask_n_m	,0x6008	,Normal				,OpDissCFS,"swap.b <REG_M>,<REG_N>"},	//swap.b <REG_M>,<REG_N>        
	{rec_shil_i0110_nnnn_mmmm_1001	,i0110_nnnn_mmmm_1001	,Mask_n_m	,0x6009	,Normal				,OpDissCFS,"swap.w <REG_M>,<REG_N>"},	//swap.w <REG_M>,<REG_N>        
	{rec_shil_i0110_nnnn_mmmm_1010	,i0110_nnnn_mmmm_1010	,Mask_n_m	,0x600A	,Normal				,OpDissCFS,"negc <REG_M>,<REG_N>"},	//negc <REG_M>,<REG_N>          
	{rec_shil_i0110_nnnn_mmmm_1011	,i0110_nnnn_mmmm_1011	,Mask_n_m	,0x600B	,Normal				,OpDissCFS,"neg <REG_M>,<REG_N>"},	//neg <REG_M>,<REG_N>           
	{rec_shil_i0110_nnnn_mmmm_1100	,i0110_nnnn_mmmm_1100	,Mask_n_m	,0x600C	,Normal				,OpDissCFS,"extu.b <REG_M>,<REG_N>"},	//extu.b <REG_M>,<REG_N>        
	{rec_shil_i0110_nnnn_mmmm_1101	,i0110_nnnn_mmmm_1101	,Mask_n_m	,0x600D	,Normal				,OpDissCFS,"extu.w <REG_M>,<REG_N>"},	//extu.w <REG_M>,<REG_N>        
	{rec_shil_i0110_nnnn_mmmm_1110	,i0110_nnnn_mmmm_1110	,Mask_n_m	,0x600E	,Normal				,OpDissCFS,"exts.b <REG_M>,<REG_N>"},	//exts.b <REG_M>,<REG_N>        
	{rec_shil_i0110_nnnn_mmmm_1111	,i0110_nnnn_mmmm_1111	,Mask_n_m	,0x600F	,Normal				,OpDissCFS,"exts.w <REG_M>,<REG_N>"},	//exts.w <REG_M>,<REG_N>        
	{rec_shil_i0111_nnnn_iiii_iiii	,i0111_nnnn_iiii_iiii	,Mask_n_imm8,0x7000	,Normal				,OpDissCFS,"add #<simm8>,<REG_N>"},	//add #<imm>,<REG_N>
	{rec_shil_i1000_1011_iiii_iiii	,i1000_1011_iiii_iiii	,Mask_imm8	,0x8B00	,Branch_rel			,OpDissCFS,"bf <bdisp8>"},	// bf <bdisp8>                   
	{rec_shil_i1000_1111_iiii_iiii	,i1000_1111_iiii_iiii	,Mask_imm8	,0x8F00	,Branch_rel_d		,OpDissCFS,"bf.s <bdisp8>"},	// bf.s <bdisp8>                 
	{rec_shil_i1000_1001_iiii_iiii	,i1000_1001_iiii_iiii	,Mask_imm8	,0x8900	,Branch_rel			,OpDissCFS,"bt <bdisp8>"},	// bt <bdisp8>                   
	{rec_shil_i1000_1101_iiii_iiii	,i1000_1101_iiii_iiii	,Mask_imm8	,0x8D00	,Branch_rel_d		,OpDissCFS,"bt.s <bdisp8>"},	// bt.s <bdisp8>                 
	{rec_shil_i1000_1000_iiii_iiii	,i1000_1000_iiii_iiii	,Mask_imm8	,0x8800	,Normal				,OpDissCFS,"cmp/eq #<simm8hex>,R0"},	// cmp/eq #<imm>,R0              
	{rec_shil_i1000_0000_mmmm_iiii	,i1000_0000_mmmm_iiii	,Mask_imm8	,0x8000	,Normal				,OpDissCFS,"mov.b R0,@(<disp8b>,<REG_M>)"},	// mov.b R0,@(<disp>,<REG_M>)    
	{rec_shil_i1000_0001_mmmm_iiii	,i1000_0001_mmmm_iiii	,Mask_imm8	,0x8100	,Normal				,OpDissCFS,"mov.w R0,@(<disp8w>,<REG_M>)"},	// mov.w R0,@(<disp>,<REG_M>)    
	{rec_shil_i1000_0100_mmmm_iiii	,i1000_0100_mmmm_iiii	,Mask_imm8	,0x8400	,Normal				,OpDissCFS,"mov.b @(<disp8b>,<REG_M>),R0"},	// mov.b @(<disp>,<REG_M>),R0    
	{rec_shil_i1000_0101_mmmm_iiii	,i1000_0101_mmmm_iiii	,Mask_imm8	,0x8500	,Normal				,OpDissCFS,"mov.w @(<disp8w>,<REG_M>),R0"},	// mov.w @(<disp>,<REG_M>),R0    
	{rec_shil_i1001_nnnn_iiii_iiii	,i1001_nnnn_iiii_iiii	,Mask_n_imm8,0x9000	,ReadsPC			,OpDissCFS,"mov.w @(<PCdisp8>),<REG_N>"},	//mov.w @(<disp>,PC),<REG_N>   
	{rec_shil_i1010_iiii_iiii_iiii	,i1010_iiii_iiii_iiii	,Mask_n_imm8,0xA000	,Branch_rel_d		,OpDissCFS,"bra <bdisp12>"},	// bra <bdisp12>
	{rec_shil_i1011_iiii_iiii_iiii	,i1011_iiii_iiii_iiii	,Mask_n_imm8,0xB000	,Branch_rel_d		,OpDissCFS,"bsr <bdisp12>"},	// bsr <bdisp12>
	{rec_shil_i1100_0000_iiii_iiii	,i1100_0000_iiii_iiii	,Mask_imm8	,0xC000	,Normal				,OpDissCFS,"mov.b R0,@(<disp8b>,GBR)"},	// mov.b R0,@(<disp>,GBR)        
	{rec_shil_i1100_0001_iiii_iiii	,i1100_0001_iiii_iiii	,Mask_imm8	,0xC100	,Normal				,OpDissCFS,"mov.w R0,@(<disp8w>,GBR)"},	// mov.w R0,@(<disp>,GBR)        
	{rec_shil_i1100_0010_iiii_iiii	,i1100_0010_iiii_iiii	,Mask_imm8	,0xC200	,Normal				,OpDissCFS,"mov.l R0,@(<disp8dw>,GBR)"},	// mov.l R0,@(<disp>,GBR)        
	{rec_shil_i1100_0011_iiii_iiii	,i1100_0011_iiii_iiii	,Mask_imm8	,0xC300	,ReadWritePC		,OpDissCFS,"trapa #<imm8>"},	// trapa #<imm>                  
	{rec_shil_i1100_0100_iiii_iiii	,i1100_0100_iiii_iiii	,Mask_imm8	,0xC400	,Normal				,OpDissCFS,"mov.b @(<GBRdisp8b>),R0"},	// mov.b @(<disp>,GBR),R0        
	{rec_shil_i1100_0101_iiii_iiii	,i1100_0101_iiii_iiii	,Mask_imm8	,0xC500	,Normal				,OpDissCFS,"mov.w @(<GBRdisp8w>),R0"},	// mov.w @(<disp>,GBR),R0        
	{rec_shil_i1100_0110_iiii_iiii	,i1100_0110_iiii_iiii	,Mask_imm8	,0xC600	,Normal				,OpDissCFS,"mov.l @(<GBRdisp8dw>),R0"},	// mov.l @(<disp>,GBR),R0        
	{rec_shil_i1100_0111_iiii_iiii	,i1100_0111_iiii_iiii	,Mask_imm8	,0xC700	,ReadsPC			,OpDissCFS,"mova @(<PCdisp8>),R0"},	// mova @(<disp>,PC),R0          
	{rec_shil_i1100_1000_iiii_iiii	,i1100_1000_iiii_iiii	,Mask_imm8	,0xC800	,Normal				,OpDissCFS,"tst #<imm8>,R0"},	// tst #<imm>,R0                 
	{rec_shil_i1100_1001_iiii_iiii	,i1100_1001_iiii_iiii	,Mask_imm8	,0xC900	,Normal				,OpDissCFS,"and #<imm8>,R0"},	// and #<imm>,R0                 
	{rec_shil_i1100_1010_iiii_iiii	,i1100_1010_iiii_iiii	,Mask_imm8	,0xCA00	,Normal				,OpDissCFS,"xor #<imm8>,R0"},	// xor #<imm>,R0                 
	{rec_shil_i1100_1011_iiii_iiii	,i1100_1011_iiii_iiii	,Mask_imm8	,0xCB00	,Normal				,OpDissCFS,"or #<imm8>,R0"},	// or #<imm>,R0                  
	{rec_shil_i1100_1100_iiii_iiii	,i1100_1100_iiii_iiii	,Mask_imm8	,0xCC00	,Normal				,OpDissCFS,"tst.b #<imm8>,@(R0,GBR)"},	// tst.b #<imm>,@(R0,GBR)        
	{rec_shil_i1100_1101_iiii_iiii	,i1100_1101_iiii_iiii	,Mask_imm8	,0xCD00	,Normal				,OpDissCFS,"and.b #<imm8>,@(R0,GBR)"},	// and.b #<imm>,@(R0,GBR)        
	{rec_shil_i1100_1110_iiii_iiii	,i1100_1110_iiii_iiii	,Mask_imm8	,0xCE00	,Normal				,OpDissCFS,"xor.b #<imm8>,@(R0,GBR)"},	// xor.b #<imm>,@(R0,GBR)        
	{rec_shil_i1100_1111_iiii_iiii	,i1100_1111_iiii_iiii	,Mask_imm8	,0xCF00	,Normal				,OpDissCFS,"or.b #<imm8>,@(R0,GBR)"},	// or.b #<imm>,@(R0,GBR)         
	{rec_shil_i1101_nnnn_iiii_iiii	,i1101_nnnn_iiii_iiii	,Mask_n_imm8,0xD000	,ReadsPC			,OpDissCFS,"mov.l @(<PCdisp8>),<REG_N>"},	// mov.l @(<disp>,PC),<REG_N>    
	{rec_shil_i1110_nnnn_iiii_iiii	,i1110_nnnn_iiii_iiii	,Mask_n_imm8,0xE000	,Normal				,OpDissCFS,"mov #<simm8hex>,<REG_N>"},	// mov #<imm>,<REG_N>
	
	//and here are the new ones :D
	{rec_shil_i1111_nnnn_mmmm_0000	,i1111_nnnn_mmmm_0000	,Mask_n_m		,0xF000,Normal			,d1111_nnnn_mmmm_0000},	//fadd <FREG_M>,<FREG_N>
	{rec_shil_i1111_nnnn_mmmm_0001	,i1111_nnnn_mmmm_0001	,Mask_n_m		,0xF001,Normal			,d1111_nnnn_mmmm_0001},	//fsub <FREG_M>,<FREG_N>   
	{rec_shil_i1111_nnnn_mmmm_0010	,i1111_nnnn_mmmm_0010	,Mask_n_m		,0xF002,Normal			,d1111_nnnn_mmmm_0010},	//fmul <FREG_M>,<FREG_N>   
	{rec_shil_i1111_nnnn_mmmm_0011	,i1111_nnnn_mmmm_0011	,Mask_n_m		,0xF003,Normal			,d1111_nnnn_mmmm_0011},	//fdiv <FREG_M>,<FREG_N>   
	{rec_shil_i1111_nnnn_mmmm_0100	,i1111_nnnn_mmmm_0100	,Mask_n_m		,0xF004,Normal			,d1111_nnnn_mmmm_0100},	//fcmp/eq <FREG_M>,<FREG_N>
	{rec_shil_i1111_nnnn_mmmm_0101	,i1111_nnnn_mmmm_0101	,Mask_n_m		,0xF005,Normal			,d1111_nnnn_mmmm_0101},	//fcmp/gt <FREG_M>,<FREG_N>
	{rec_shil_i1111_nnnn_mmmm_0110	,i1111_nnnn_mmmm_0110	,Mask_n_m		,0xF006,Normal			,d1111_nnnn_mmmm_0110},	//fmov.s @(R0,<REG_M>),<FREG_N>
	{rec_shil_i1111_nnnn_mmmm_0111	,i1111_nnnn_mmmm_0111	,Mask_n_m		,0xF007,Normal			,d1111_nnnn_mmmm_0111},	//fmov.s <FREG_M>,@(R0,<REG_N>)
	{rec_shil_i1111_nnnn_mmmm_1000	,i1111_nnnn_mmmm_1000	,Mask_n_m		,0xF008,Normal			,d1111_nnnn_mmmm_1000},	//fmov.s @<REG_M>,<FREG_N> 
	{rec_shil_i1111_nnnn_mmmm_1001	,i1111_nnnn_mmmm_1001	,Mask_n_m		,0xF009,Normal			,d1111_nnnn_mmmm_1001},	//fmov.s @<REG_M>+,<FREG_N>
	{rec_shil_i1111_nnnn_mmmm_1010	,i1111_nnnn_mmmm_1010	,Mask_n_m		,0xF00A,Normal			,d1111_nnnn_mmmm_1010},	//fmov.s <FREG_M>,@<REG_N>
	{rec_shil_i1111_nnnn_mmmm_1011	,i1111_nnnn_mmmm_1011	,Mask_n_m		,0xF00B,Normal			,d1111_nnnn_mmmm_1011},	//fmov.s <FREG_M>,@-<REG_N>
	{rec_shil_i1111_nnnn_mmmm_1100	,i1111_nnnn_mmmm_1100	,Mask_n_m		,0xF00C,Normal			,d1111_nnnn_mmmm_1100},	//fmov <FREG_M>,<FREG_N>   
	{rec_shil_i1111_nnnn_0101_1101	,i1111_nnnn_0101_1101	,Mask_n			,0xF05D,Normal			,d1111_nnnn_0101_1101},	//fabs <FREG_N>            
	{rec_shil_i1111_nnn0_1111_1101	,i1111_nnn0_1111_1101	,Mask_nh3bit	,0xF0FD,Normal			,OpDissFSCA},	//FSCA FPUL, DRn//F0FD//1111_nnnn_1111_1101
	{rec_shil_i1111_nnnn_1011_1101	,i1111_nnnn_1011_1101	,Mask_n			,0xF0BD,Normal			,d1111_nnnn_1011_1101},	//fcnvds <DR_N>,FPUL       
	{rec_shil_i1111_nnnn_1010_1101	,i1111_nnnn_1010_1101	,Mask_n			,0xF0AD,Normal			,d1111_nnnn_1010_1101},	//fcnvsd FPUL,<DR_N>       
	{rec_shil_i1111_nnmm_1110_1101	,i1111_nnmm_1110_1101	,Mask_n			,0xF0ED,Normal			,OpDissfipr},	//fipr <FV_M>,<FV_N>            
	{rec_shil_i1111_nnnn_1000_1101	,i1111_nnnn_1000_1101	,Mask_n			,0xF08D,Normal			,d1111_nnnn_1000_1101},	//fldi0 <FREG_N>           
	{rec_shil_i1111_nnnn_1001_1101	,i1111_nnnn_1001_1101	,Mask_n			,0xF09D,Normal			,d1111_nnnn_1001_1101},	//fldi1 <FREG_N>           
	{rec_shil_i1111_nnnn_0001_1101	,i1111_nnnn_0001_1101	,Mask_n			,0xF01D,Normal			,d1111_nnnn_0001_1101},	//flds <FREG_N>,FPUL       
	{rec_shil_i1111_nnnn_0010_1101	,i1111_nnnn_0010_1101	,Mask_n			,0xF02D,Normal			,d1111_nnnn_0010_1101},	//float FPUL,<FREG_N>      
	{rec_shil_i1111_nnnn_0100_1101	,i1111_nnnn_0100_1101	,Mask_n			,0xF04D,Normal			,d1111_nnnn_0100_1101},	//fneg <FREG_N>            
	{rec_shil_i1111_1011_1111_1101	,i1111_1011_1111_1101	,Mask_none		,0xFBFD,WritesFPSCR		,OpDissCFS,"frchg"},	//frchg                    
	{rec_shil_i1111_0011_1111_1101	,i1111_0011_1111_1101	,Mask_none		,0xF3FD,WritesFPSCR		,OpDissCFS,"fschg"},	//fschg                    
	{rec_shil_i1111_nnnn_0110_1101	,i1111_nnnn_0110_1101	,Mask_n			,0xF06D,Normal			,d1111_nnnn_0110_1101},	//fsqrt <FREG_N>                
	{rec_shil_i1111_nnnn_0011_1101	,i1111_nnnn_0011_1101	,Mask_n			,0xF03D,Normal			,d1111_nnnn_0011_1101},	//ftrc <FREG_N>, FPUL      
	{rec_shil_i1111_nnnn_0000_1101	,i1111_nnnn_0000_1101	,Mask_n			,0xF00D,Normal			,d1111_nnnn_0000_1101},	//fsts FPUL,<FREG_N>       
	{rec_shil_i1111_nn01_1111_1101	,i1111_nn01_1111_1101	,Mask_nh2bit	,0xF1FD,Normal			,OpDissftrv},	//ftrv xmtrx,<FV_N> 
	{rec_shil_i1111_nnnn_mmmm_1110	,i1111_nnnn_mmmm_1110	,Mask_n_m		,0xF00E,Normal			,OpDissfmac},	//fmac <FREG_0>,<FREG_M>,<FREG_N> 
	{rec_shil_i1111_nnnn_0111_1101	,i1111_nnnn_0111_1101	,Mask_n			,0xF07D,Normal			,d1111_nnnn_0111_1101},	//FSRRA <FREG_N> (1111nnnn 01111101)
	
	//HLE ops
	{0								,gdrom_hle_op			,Mask_none	,GDROM_OPCODE,ReadWritePC	,dissasm_GDROM},
	{0								,sh4_bpt_op				,Mask_none	,BPT_OPCODE	 ,ReadWritePC	,dissasm_Break},

	//end of list
	{0,0,0,0,ReadWritePC}//Branch in order to stop the block and save pc ect :)
};

bool bBuilded=false;
void BuildOpcodeTables()
{
	if (bBuilded)
		return;
	u32 fpu_unh=0;
	u32 cpu_unh=0;

	bBuilded=true;
	for (int i=0;i<0x10000;i++)
	{
		OpPtr[i]=iNotImplemented;// (OpCallFP*)0;
		OpTyp[i]=Invalid;
		RecOpPtr[i]=rec_shil_icpu_nimp;
		for (int i2=0;opcodes[i2].oph;i2++)
		{
			if ((i&opcodes[i2].mask)==opcodes[i2].rez)
			{
				if (OpTyp[i]==Invalid)
				{
					OpPtr[i]=opcodes[i2].oph;
					RecOpPtr[i]=opcodes[i2].rec_oph;
					//if (((i&0xF00F)>0x4006) && ((i&0xF00F)<0x400F) )//0x9739
					//if ( ((i&0xF0FF)==0x400E)  || ((i&0xF0FF)==0x4007) )
						//RecOpPtr[i]=rec_cpu_opcode_nimp;

					OpTyp[i]=opcodes[i2].type;
					OpDisasm[i]=&opcodes[i2];
					if (OpDisasm[i]->dissasm==0)
						OpDisasm[i]->dissasm=OpNoDiss;
				}
				else
				{
					printf("OPCODE TABLE FAULT , DOUBLE DEFINED OPCODE\n");
				}
			}
		}
	}

	u32 cpu_count;
	for (cpu_count=0;opcodes[cpu_count].oph;cpu_count++)	
	{
		if (rec_shil_icpu_nimp==opcodes[cpu_count].rec_oph)
		{
			cpu_unh++;
		}
	}

	cpu_unh = (cpu_count-31)-cpu_unh;
	fpu_unh	= 31-fpu_unh;

	printf("shil generation status : %d%% cpu done[%d of %d] , %d%% fpu done[%d of %d]\n",
		cpu_unh*100/(cpu_count-31),cpu_unh,(cpu_count-31) ,fpu_unh*100/31	,fpu_unh,31);
}

void DissasembleOpcode(u16 opcode,u32 pc,char* Dissasm)
{
	if (!bBuilded)
		BuildOpcodeTables();

	if (OpDisasm[opcode]!=0)
	{	
		OpDisasm[opcode]->Dissasemble(Dissasm,pc,opcode);
	}
	else
	{
		sprintf(Dissasm,"Unkown Opcode 0x%X",opcode);
	}
}

//more complex , smaller arrays :P



//TODO : switch to partial look up table
/*
#define put_asm1(xx) (*asm_p)=(xx);asm_p++;
#define put_asm2(xx,yy) put_asm1(xx) put_asm1(yy)
#define put_asm3(xx,yy,zz) put_asm2(xx,yy) put_asm1(zz)
#define put_asm4(xx,yy,zz,ww) put_asm3(xx,yy,zz) put_asm1(ww)

#define put_asm4_dw(xx) (*(u32*)asm_p)=(xx);asm_p+=4;

u8 masm[0x1000];
u8* asm_p=masm;

OpCallFP* GenerateCallTable(u32 level,u32 found)
{
	OpCallFP* rv=(OpCallFP*) malloc(16*sizeof(OpCallFP*));
	
	if (level==4)//2nd
	{
	}
	else if (level==3)//3nd
	{
	}
	else if (level==2)//4th
	{
	}
	else if (level==1)//1st
	{
		for (int i=0;i<16;i++)
		{

		}
	}

}
void* GenerateAsm(u32 AndValue,u32 s_v,u32 Table)
{
	u8*  ps=asm_p;
	//  8b c1		   // mov	 eax, ecx //get xx bits
	put_asm2(0x8b,0xc1);
	//  25 00 f0 XX XX   // and	 eax, 61440		; 0000f000H
	put_asm1(0x25);
	put_asm4_dw(AndValue);
	//  c1 e0 SS	   // shl	 eax, 10		; 0000000aH
	put_asm3(0xc1,0xe0,s_v);
	//  0b 05 CC CC CC 00// or	 eax, DWORD PTR ?opTable@@3P6IXII@ZA ; opTable
	put_asm2(0x0b,0x05);
    put_asm4_dw(Table);
	//  8b 00		   // mov	 eax, DWORD PTR [eax]
	put_asm2(0x8b,0x00);
	//  ff e0		   // jmp	 eax
	put_asm2(0xff,0xe0);
	
	asm_p=(u8*)(((u32)asm_p&(~0xF)) +0x10);
	return ps;
}

*/