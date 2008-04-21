/*
	this file is part of the nullDCe project
	look on nullDC.h for lience and legal information.

	main.cpp : Application entry point
*/
#include "nulldce.h"
#include "dc/dc.h"
#include "dc/sh4/sh4.h"


#if 0
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

struct sh4_opcodelistentry
{
	char* name;
	u32 mask;
	u32 key;
	char* dissasm;
};
sh4_opcodelistentry opcodes[]=
{
	
	//CPU
	{"i0000_nnnn_0000_0010",Mask_n		,0x0002	,"stc SR,<REG_N>"},//,"stc SR,<REG_N>"},//},//,Normal				,OpDissCFS,"stc SR,<REG_N>"					,2,2,CO,fix_none},	//stc SR,<REG_N>                
	{"i0000_nnnn_0001_0010",Mask_n		,0x0012	,"stc GBR,<REG_N>"},//},//,Normal				,OpDissCFS,"stc GBR,<REG_N>"				,2,2,CO,fix_none},	//stc GBR,<REG_N>               
	{"i0000_nnnn_0010_0010",Mask_n		,0x0022	,"stc VBR,<REG_N>"},//},//,Normal				,OpDissCFS,"stc VBR,<REG_N>"				,2,2,CO,fix_none},	//stc VBR,<REG_N>               
	{"i0000_nnnn_0011_0010",Mask_n		,0x0032	,"stc SSR,<REG_N>"},//},//,Normal				,OpDissCFS,"stc SSR,<REG_N>"				,2,2,CO,fix_none},	//stc SSR,<REG_N>  
	
				//STC SGR,Rn SGR > Rn 0000_nnnn_0011_1010 Privileged —(this one is 0x0f3A)
	{"i0000_nnnn_0011_1010",Mask_n		,0x003A	,"stc SGR,<REG_N>"},//},//,Normal				,OpDissCFS,"stc SGR,<REG_N>"				,3,3,CO,fix_none},	//stc SGR,<REG_N> 

	{"i0000_nnnn_0100_0010",Mask_n		,0x0042	,"stc SPC,<REG_N>"},//},//,Normal				,OpDissCFS,"stc SPC,<REG_N>"				,2,2,CO,fix_none},	//stc SPC,<REG_N>               
	{"i0000_nnnn_1mmm_0010",Mask_n_ml3bit,0x0082,"stc R0_BANK,<REG_N>"},//},//,Normal			,OpDissCFS,"stc R0_BANK,<REG_N>"			,2,2,CO,fix_none},	//stc R0_BANK,<REG_N>           
	{"i0000_nnnn_0010_0011",Mask_n		,0x0023	,"braf <REG_N>"},//},//,Branch_rel_d		,OpDissCFS,"braf <REG_N>"					,2,3,CO,fix_none},	//braf <REG_N>                  
	{"i0000_nnnn_0000_0011",Mask_n		,0x0003	,"bsrf <REG_N>"},//},//,Branch_rel_d		,OpDissCFS,"bsrf <REG_N>"					,2,3,CO,fix_none},	//bsrf <REG_N>                  
	{"i0000_nnnn_1100_0011",Mask_n		,0x00C3	,"movca.l R0, @<REG_N>"},//},//,Normal				,OpDissCFS,"movca.l R0, @<REG_N>"			,2,4,MA,fix_none},	//movca.l R0, @<REG_N>          
	{"i0000_nnnn_1001_0011",Mask_n		,0x0093	,"ocbi @<REG_N>"},//},//,Normal				,OpDissCFS,"ocbi @<REG_N>"					,1,2,MA,fix_none},	//ocbi @<REG_N>                 
	{"i0000_nnnn_1010_0011",Mask_n		,0x00A3	,"ocbp @<REG_N>"},//},//,Normal				,OpDissCFS,"ocbp @<REG_N>"					,1,2,MA,fix_none},	//ocbp @<REG_N>                 
	{"i0000_nnnn_1011_0011",Mask_n		,0x00B3	,"ocbwb @<REG_N>"},//},//,Normal				,OpDissCFS,"ocbwb @<REG_N>"					,1,2,MA,fix_none},	//ocbwb @<REG_N>                
	{"i0000_nnnn_1000_0011",Mask_n		,0x0083	,"pref @<REG_N>"},//},//,Normal				,OpDissCFS,"pref @<REG_N>"					,1,2,LS,fix_none},	//pref @<REG_N>                 
	{"i0000_nnnn_mmmm_0100",Mask_n_m	,0x0004	,"mov.b <REG_M>,@(R0,<REG_N>)"},//},//,Normal				,OpDissCFS,"mov.b <REG_M>,@(R0,<REG_N>)"	,1,1,LS,fix_none},	//mov.b <REG_M>,@(R0,<REG_N>)   
	{"i0000_nnnn_mmmm_0101",Mask_n_m	,0x0005	,"mov.w <REG_M>,@(R0,<REG_N>)"},//},//,Normal				,OpDissCFS,"mov.w <REG_M>,@(R0,<REG_N>)"	,1,1,LS,fix_none},	//mov.w <REG_M>,@(R0,<REG_N>)   
	{"i0000_nnnn_mmmm_0110",Mask_n_m	,0x0006	,"mov.l <REG_M>,@(R0,<REG_N>)"},//},//,Normal				,OpDissCFS,"mov.l <REG_M>,@(R0,<REG_N>)"	,1,1,LS,fix_none},	//mov.l <REG_M>,@(R0,<REG_N>)   
	{"i0000_nnnn_mmmm_0111",Mask_n_m	,0x0007	,"mul.l <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"mul.l <REG_M>,<REG_N>"			,2,4,CO,fix_none},	//mul.l <REG_M>,<REG_N>         
	{"i0000_0000_0010_1000",Mask_none	,0x0028	,"clrmac"},//},//,Normal				,OpDissCFS,"clrmac"							,1,3,LS,fix_none},	//clrmac                        
	{"i0000_0000_0100_1000",Mask_none	,0x0048	,"clrs"},//},//,Normal				,OpDissCFS,"clrs"							,1,1,CO,fix_none},	//clrs                          
	{"i0000_0000_0000_1000",Mask_none	,0x0008	,"clrt"},//},//,Normal				,OpDissCFS,"clrt"							,1,1,MT,fix_none},	//clrt                          
	{"i0000_0000_0011_1000",Mask_none	,0x0038	,"ldtlb"},//},//,Normal				,OpDissCFS,"ldtlb"							,1,1,CO,fix_none},//ldtlb                         
	{"i0000_0000_0101_1000",Mask_none	,0x0058	,"sets"},//},//,Normal				,OpDissCFS,"sets"							,1,1,CO,fix_none},	//sets                          
	{"i0000_0000_0001_1000",Mask_none	,0x0018	,"sett"},//},//,Normal				,OpDissCFS,"sett"							,1,1,MT,fix_none},	//sett                          
	{"i0000_0000_0001_1001",Mask_none	,0x0019	,"div0u"},//},//,Normal				,OpDissCFS,"div0u"							,1,1,EX,fix_none},//div0u                         
	{"i0000_nnnn_0010_1001",Mask_n		,0x0029	,"movt <REG_N>"},//},//,Normal				,OpDissCFS,"movt <REG_N>"					,1,1,EX,fix_none},	//movt <REG_N>                  
	{"i0000_0000_0000_1001",Mask_none	,0x0009	,"nop"},//},//,Normal				,OpDissCFS,"nop"							,1,0,MT,fix_none},	//nop                           
	{"i0000_nnnn_0101_1010",Mask_n		,0x005A	,"sts FPUL,<REG_N>"},//},//,Normal				,OpDissCFS,"sts FPUL,<REG_N>"				,1,3,LS,fix_none},	//sts FPUL,<REG_N>
	{"i0000_nnnn_0110_1010",Mask_n		,0x006A	,"sts FPSCR,<REG_N>"},//},//,Normal				,OpDissCFS,"sts FPSCR,<REG_N>"				,1,3,CO,fix_none},//sts FPSCR,<REG_N>             
	{"i0000_nnnn_1111_1010",Mask_n		,0x00FA	,"stc DBR,<REG_N>"},//},//,Normal				,OpDissCFS,"stc DBR,<REG_N>"				,1,2,CO,fix_none},	//stc DBR,<REG_N>             //guess
	{"i0000_nnnn_0000_1010",Mask_n		,0x000A	,"sts MACH,<REG_N>"},//},//,Normal				,OpDissCFS,"sts MACH,<REG_N>"				,1,3,CO,fix_none},	//sts MACH,<REG_N>              
	{"i0000_nnnn_0001_1010",Mask_n		,0x001A	,"sts MACL,<REG_N>"},//},//,Normal				,OpDissCFS,"sts MACL,<REG_N>"				,1,3,CO,fix_none},	//sts MACL,<REG_N>              
	{"i0000_nnnn_0010_1010",Mask_n		,0x002A	,"sts PR,<REG_N>"},//},//,Normal				,OpDissCFS,"sts PR,<REG_N>"					,2,2,CO,fix_none},	//sts PR,<REG_N>                
	{"i0000_0000_0010_1011",Mask_none	,0x002B	,"rte"},//},//,WritesPC			,OpDissCFS,"rte"							,5,5,CO,fix_none},	//rte                           
	{"i0000_0000_0000_1011",Mask_none	,0x000B	,"rts"},//},//,Branch_dir_d		,OpDissCFS,"rts"							,2,3,CO,fix_none},	//rts                           
	{"i0000_0000_0001_1011",Mask_none	,0x001B	,"sleep"},//},//,ReadWritePC		,OpDissCFS,"sleep"							,4,4,CO,fix_none},	//sleep                         
	{"i0000_nnnn_mmmm_1100",Mask_n_m	,0x000C	,"mov.b @(R0,<REG_M>),<REG_N>"},//},//,Normal				,OpDissCFS,"mov.b @(R0,<REG_M>),<REG_N>"	,1,2,LS,fix_none},	//mov.b @(R0,<REG_M>),<REG_N>   
	{"i0000_nnnn_mmmm_1101",Mask_n_m	,0x000D	,"mov.w @(R0,<REG_M>),<REG_N>"},//},//,Normal				,OpDissCFS,"mov.w @(R0,<REG_M>),<REG_N>"	,1,2,LS,fix_none},	//mov.w @(R0,<REG_M>),<REG_N>   
	{"i0000_nnnn_mmmm_1110",Mask_n_m	,0x000E	,"mov.l @(R0,<REG_M>),<REG_N>"},//},//,Normal				,OpDissCFS,"mov.l @(R0,<REG_M>),<REG_N>"	,1,2,LS,fix_none},	//mov.l @(R0,<REG_M>),<REG_N>   
	{"i0000_nnnn_mmmm_1111",Mask_n_m	,0x000F	,"mac.l @<REG_M>+,@<REG_N>+"},//},//,Normal				,OpDissCFS,"mac.l @<REG_M>+,@<REG_N>+"		,2,3,CO,fix_none},	//mac.l @<REG_M>+,@<REG_N>+     
	{"i0001_nnnn_mmmm_iiii",Mask_n_imm8,0x1000	,"mov.l <REG_M>,@(<disp4dw>,<REG_N>)"},//},//,Normal				,OpDissCFS,"mov.l <REG_M>,@(<disp4dw>,<REG_N>)",1,1,LS,fix_none},	//mov.l <REG_M>,@(<disp>,<REG_N>)
	{"i0010_nnnn_mmmm_0000",Mask_n_m	,0x2000	,"mov.b <REG_M>,@<REG_N>"},//},//,Normal				,OpDissCFS,"mov.b <REG_M>,@<REG_N>"			,1,1,LS,fix_none},	//mov.b <REG_M>,@<REG_N>        
	{"i0010_nnnn_mmmm_0001",Mask_n_m	,0x2001	,"mov.w <REG_M>,@<REG_N>"},//},//,Normal				,OpDissCFS,"mov.w <REG_M>,@<REG_N>"			,1,1,LS,fix_none},	// mov.w <REG_M>,@<REG_N>        
	{"i0010_nnnn_mmmm_0010",Mask_n_m	,0x2002	,"mov.l <REG_M>,@<REG_N>"},//},//,Normal				,OpDissCFS,"mov.l <REG_M>,@<REG_N>"			,1,1,LS,fix_none},	// mov.l <REG_M>,@<REG_N>        
	{"i0010_nnnn_mmmm_0100",Mask_n_m	,0x2004	,"mov.b <REG_M>,@-<REG_N>"},//},//,Normal				,OpDissCFS,"mov.b <REG_M>,@-<REG_N>"		,1,1,LS,rn_opt_1},	// mov.b <REG_M>,@-<REG_N>       
	{"i0010_nnnn_mmmm_0101",Mask_n_m	,0x2005	,"mov.w <REG_M>,@-<REG_N>"},//},//,Normal				,OpDissCFS,"mov.w <REG_M>,@-<REG_N>"		,1,1,LS,rn_opt_2},	//mov.w <REG_M>,@-<REG_N>       
	{"i0010_nnnn_mmmm_0110",Mask_n_m	,0x2006	,"mov.l <REG_M>,@-<REG_N>"},//},//,Normal				,OpDissCFS,"mov.l <REG_M>,@-<REG_N>"		,1,1,LS,rn_opt_4},	//mov.l <REG_M>,@-<REG_N>       
	{"i0010_nnnn_mmmm_0111",Mask_n_m	,0x2007	,"div0s <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"div0s <REG_M>,<REG_N>"			,1,1,EX,fix_none},	// div0s <REG_M>,<REG_N>         
	{"i0010_nnnn_mmmm_1000",Mask_n_m	,0x2008	,"tst <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"tst <REG_M>,<REG_N>"			,1,1,MT,fix_none},	// tst <REG_M>,<REG_N>           
	{"i0010_nnnn_mmmm_1001",Mask_n_m	,0x2009	,"and <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"and <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//and <REG_M>,<REG_N>           
	{"i0010_nnnn_mmmm_1010",Mask_n_m	,0x200A	,"xor <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"xor <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//xor <REG_M>,<REG_N>           
	{"i0010_nnnn_mmmm_1011",Mask_n_m	,0x200B	,"or <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"or <REG_M>,<REG_N>"				,1,1,EX,fix_none},	//or <REG_M>,<REG_N>            
	{"i0010_nnnn_mmmm_1100",Mask_n_m	,0x200C	,"cmp/str <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"cmp/str <REG_M>,<REG_N>"		,1,1,MT,fix_none},	//cmp/str <REG_M>,<REG_N>       
	{"i0010_nnnn_mmmm_1101",Mask_n_m	,0x200D	,"xtrct <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"xtrct <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//xtrct <REG_M>,<REG_N>         
	{"i0010_nnnn_mmmm_1110",Mask_n_m	,0x200E	,"mulu.w <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"mulu.w <REG_M>,<REG_N>"			,1,4,CO,fix_none},	//mulu.w <REG_M>,<REG_N>          
	{"i0010_nnnn_mmmm_1111",Mask_n_m	,0x200F	,"muls.w <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"muls.w <REG_M>,<REG_N>"			,1,4,CO,fix_none},	//muls.w <REG_M>,<REG_N>          
	{"i0011_nnnn_mmmm_0000",Mask_n_m	,0x3000	,"cmp/eq <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"cmp/eq <REG_M>,<REG_N>"			,1,1,MT,fix_none},	// cmp/eq <REG_M>,<REG_N>        
	{"i0011_nnnn_mmmm_0010",Mask_n_m	,0x3002	,"cmp/hs <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"cmp/hs <REG_M>,<REG_N>"			,1,1,MT,fix_none},	// cmp/hs <REG_M>,<REG_N>        
	{"i0011_nnnn_mmmm_0011",Mask_n_m	,0x3003	,"cmp/ge <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"cmp/ge <REG_M>,<REG_N>"			,1,1,MT,fix_none},	//cmp/ge <REG_M>,<REG_N>        
	{"i0011_nnnn_mmmm_0100",Mask_n_m	,0x3004	,"div1 <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"div1 <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//div1 <REG_M>,<REG_N>          
	{"i0011_nnnn_mmmm_0101",Mask_n_m	,0x3005	,"dmulu.l <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"dmulu.l <REG_M>,<REG_N>"		,2,4,CO,fix_none},	//dmulu.l <REG_M>,<REG_N>       
	{"i0011_nnnn_mmmm_0110",Mask_n_m	,0x3006	,"cmp/hi <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"cmp/hi <REG_M>,<REG_N>"			,1,1,MT,fix_none},	// cmp/hi <REG_M>,<REG_N>        
	{"i0011_nnnn_mmmm_0111",Mask_n_m	,0x3007	,"cmp/gt <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"cmp/gt <REG_M>,<REG_N>"			,1,1,MT,fix_none},	//cmp/gt <REG_M>,<REG_N>        
	{"i0011_nnnn_mmmm_1000",Mask_n_m	,0x3008	,"sub <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"sub <REG_M>,<REG_N>"			,1,1,EX,fix_none},	// sub <REG_M>,<REG_N>           
	{"i0011_nnnn_mmmm_1010",Mask_n_m	,0x300A	,"subc <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"subc <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//subc <REG_M>,<REG_N>          
	{"i0011_nnnn_mmmm_1011",Mask_n_m	,0x300B	,"subv <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"subv <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//subv <REG_M>,<REG_N>          
	{"i0011_nnnn_mmmm_1100",Mask_n_m	,0x300C	,"add <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"add <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//add <REG_M>,<REG_N>           
	{"i0011_nnnn_mmmm_1101",Mask_n_m	,0x300D	,"dmuls.l <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"dmuls.l <REG_M>,<REG_N>"		,1,4,CO,fix_none},	//dmuls.l <REG_M>,<REG_N>       
	{"i0011_nnnn_mmmm_1110",Mask_n_m	,0x300E	,"addc <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"addc <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//addc <REG_M>,<REG_N>          
	{"i0011_nnnn_mmmm_1111",Mask_n_m	,0x300F	,"addv <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"addv <REG_M>,<REG_N>"			,1,1,EX,fix_none},	// addv <REG_M>,<REG_N>          
	{"i0100_nnnn_0101_0010",Mask_n		,0x4052	,"sts.l FPUL,@-<REG_N>"},//},//,Normal				,OpDissCFS,"sts.l FPUL,@-<REG_N>"			,1,1,CO,rn_4	},	//sts.l FPUL,@-<REG_N>          
	{"i0100_nnnn_0110_0010",Mask_n		,0x4062	,"sts.l FPSCR,@-<REG_N>"},//},//,Normal				,OpDissCFS,"sts.l FPSCR,@-<REG_N>"			,1,2,CO,rn_4	},	//sts.l FPSCR,@-<REG_N>         
	{"i0100_nnnn_0000_0010",Mask_n		,0x4002	,"sts.l MACH,@-<REG_N>"},//},//,Normal				,OpDissCFS,"sts.l MACH,@-<REG_N>"			,1,3,CO,rn_4	},	//sts.l MACH,@-<REG_N>          
	{"i0100_nnnn_0001_0010",Mask_n		,0x4012	,"sts.l MACL,@-<REG_N>"},//},//,Normal				,OpDissCFS,"sts.l MACL,@-<REG_N>"			,1,3,CO,rn_4	},	//sts.l MACL,@-<REG_N>          
	{"i0100_nnnn_0010_0010",Mask_n		,0x4022	,"sts.l PR,@-<REG_N>"},//},//,Normal				,OpDissCFS,"sts.l PR,@-<REG_N>"				,1,1,CO,rn_4	},	//sts.l PR,@-<REG_N>            
	
					//STC.L DBR,@-Rn  0100_nnnn_1111_0010 Privileged —
	{"i0100_nnnn_1111_0010",Mask_n		,0x40F2	,"stc.l DBR,@-<REG_N>"},//},//,Normal				,OpDissCFS,"stc.l DBR,@-<REG_N>"			,2,2,CO,rn_4	},	//stc.l DBR,@-<REG_N>           

	{"i0100_nnnn_0000_0011",Mask_n		,0x4003	,"stc.l SR,@-<REG_N>"},//},//,Normal				,OpDissCFS,"stc.l SR,@-<REG_N>"				,1,1,CO,rn_4	},	//stc.l SR,@-<REG_N>            
	{"i0100_nnnn_0001_0011",Mask_n		,0x4013	,"stc.l GBR,@-<REG_N>"},//},//,Normal				,OpDissCFS,"stc.l GBR,@-<REG_N>"			,1,1,CO,rn_4	},	//stc.l GBR,@-<REG_N>           
	{"i0100_nnnn_0010_0011",Mask_n		,0x4023	,"stc.l VBR,@-<REG_N>"},//},//,Normal				,OpDissCFS,"stc.l VBR,@-<REG_N>"			,1,1,CO,rn_4	},	//stc.l VBR,@-<REG_N>           
	{"i0100_nnnn_0011_0011",Mask_n		,0x4033	,"stc.l SSR,@-<REG_N>"},//},//,Normal				,OpDissCFS,"stc.l SSR,@-<REG_N>"			,1,1,CO,rn_4	},	//stc.l SSR,@-<REG_N>           

				//STC.L SGR,@-Rn      0100_nnnn_0011_0010 Privileged —
	{"i0100_nnnn_0011_0010",Mask_n		,0x4032	,"stc.l SGR,@-<REG_N>"},//},//,Normal				,OpDissCFS,"stc.l SGR,@-<REG_N>"			,3,3,CO,rn_4	},	//stc.l SGR,@-<REG_N>           

	{"i0100_nnnn_0100_0011",Mask_n		,0x4043	,"stc.l SPC,@-<REG_N>"},//},//,Normal				,OpDissCFS,"stc.l SPC,@-<REG_N>"			,1,1,CO,rn_4	},	//stc.l SPC,@-<REG_N>           
	{"i0100_nnnn_1mmm_0011",Mask_n_ml3bit,0x4083,"stc <RM_BANK>,@-<REG_N>"},//},//,Normal			,OpDissCFS,"stc <RM_BANK>,@-<REG_N>"		,1,1,CO,rn_4	},	//stc Rm_BANK,@-<REG_N>         
	{"i0100_nnnn_0000_0110",Mask_n		,0x4006	,"lds.l @<REG_N>+,MACH"},//},//,Normal				,OpDissCFS,"lds.l @<REG_N>+,MACH"			,1,1,CO,fix_none},	//lds.l @<REG_N>+,MACH          
	{"i0100_nnnn_0001_0110",Mask_n		,0x4016	,"lds.l @<REG_N>+,MACL"},//},//,Normal				,OpDissCFS,"lds.l @<REG_N>+,MACL"			,1,1,CO,fix_none},	//lds.l @<REG_N>+,MACL          
	{"i0100_nnnn_0010_0110",Mask_n		,0x4026	,"lds.l @<REG_N>+,PR"},//},//,Normal				,OpDissCFS,"lds.l @<REG_N>+,PR"				,1,2,CO,fix_none},	//lds.l @<REG_N>+,PR            
	{"i0100_nnnn_0101_0110",Mask_n		,0x4056	,"lds.l @<REG_N>+,FPUL"},//},//,Normal				,OpDissCFS,"lds.l @<REG_N>+,FPUL"			,1,1,CO,fix_none},	//lds.l @<REG_N>+,FPUL          
	{"i0100_nnnn_0110_0110",Mask_n		,0x4066	,"lds.l @<REG_N>+,FPSCR"},//},//,WritesFPSCR		,OpDissCFS,"lds.l @<REG_N>+,FPSCR"			,1,1,CO,fix_none},	//lds.l @<REG_N>+,FPSCR         
	
				    //LDC.L @Rm+,DBR  0100_mmmm_1111_0110 Privileged —
	{"i0100_nnnn_1111_0110",Mask_n		,0x40F6	,"ldc.l @<REG_N>+,DBR"},//},//,Normal				,OpDissCFS,"ldc.l @<REG_N>+,DBR"			,1,3,CO,fix_none},	//ldc.l @<REG_N>+,DBR            

	{"i0100_nnnn_0000_0111",Mask_n		,0x4007	,"ldc.l @<REG_N>+,SR"},//},//,WritesSR			,OpDissCFS,"ldc.l @<REG_N>+,SR"				,1,1,CO,fix_none},	//ldc.l @<REG_N>+,SR            
	{"i0100_nnnn_0001_0111",Mask_n		,0x4017	,"ldc.l @<REG_N>+,GBR"},//},//,Normal				,OpDissCFS,"ldc.l @<REG_N>+,GBR"			,1,1,CO,fix_none},	//ldc.l @<REG_N>+,GBR           
	{"i0100_nnnn_0010_0111",Mask_n		,0x4027	,"ldc.l @<REG_N>+,VBR"},//},//,Normal				,OpDissCFS,"ldc.l @<REG_N>+,VBR"			,1,1,CO,fix_none},	//ldc.l @<REG_N>+,VBR           
	{"i0100_nnnn_0011_0111",Mask_n		,0x4037	,"ldc.l @<REG_N>+,SSR"},//},//,Normal				,OpDissCFS,"ldc.l @<REG_N>+,SSR"			,1,1,CO,fix_none},	//ldc.l @<REG_N>+,SSR           

			    //LDC.L @Rm+,SGR (Rm) 0100_mmmm_0011_0110 Privileged —
	{"i0100_nnnn_0011_0110",Mask_n		,0x4036	,"ldc.l @<REG_N>+,SGR"},//},//,Normal				,OpDissCFS,"ldc.l @<REG_N>+,SGR"			,3,3,CO,fix_none},	//ldc.l @<REG_N>+,SGR           

	{"i0100_nnnn_0100_0111",Mask_n		,0x4047	,"ldc.l @<REG_N>+,SPC"},//},//,Normal				,OpDissCFS,"ldc.l @<REG_N>+,SPC"			,1,1,CO,fix_none},	//ldc.l @<REG_N>+,SPC           
	{"i0100_nnnn_1mmm_0111",Mask_n_ml3bit,0x4087,"ldc.l @<REG_N>+,RM_BANK"},//},//,Normal			,OpDissCFS,"ldc.l @<REG_N>+,RM_BANK"		,1,1,CO,fix_none},	//ldc.l @<REG_N>+,RM_BANK       
	{"i0100_nnnn_0000_1010",Mask_n		,0x400A	,"lds <REG_N>,MACH"},//},//,Normal				,OpDissCFS,"lds <REG_N>,MACH"				,1,3,CO,fix_none},	//lds <REG_N>,MACH              
	{"i0100_nnnn_0001_1010",Mask_n		,0x401A	,"lds <REG_N>,MACL"},//},//,Normal				,OpDissCFS,"lds <REG_N>,MACL"				,1,3,CO,fix_none},	//lds <REG_N>,MACL              
	{"i0100_nnnn_0010_1010",Mask_n		,0x402A	,"lds <REG_N>,PR"},//},//,Normal				,OpDissCFS,"lds <REG_N>,PR"					,1,2,CO,fix_none},	//lds <REG_N>,PR                
	{"i0100_nnnn_0101_1010",Mask_n		,0x405A	,"lds <REG_N>,FPUL"},//},//,Normal				,OpDissCFS,"lds <REG_N>,FPUL"				,1,1,CO,fix_none},	//lds <REG_N>,FPUL              
	{"i0100_nnnn_0110_1010",Mask_n		,0x406A	,"lds <REG_N>,FPSCR"},//},//,WritesFPSCR		,OpDissCFS,"lds <REG_N>,FPSCR"				,1,1,CO,fix_none},	//lds <REG_N>,FPSCR             
	{"i0100_nnnn_1111_1010",Mask_n		,0x40FA	,"ldc <REG_N>,DBR"},//},//,Normal				,OpDissCFS,"ldc <REG_N>,DBR"				,1,1,CO,fix_none},	//ldc <REG_N>,DBR                
	{"i0100_nnnn_0000_1110",Mask_n		,0x400E	,"ldc <REG_N>,SR"},//},//,WritesSR			,OpDissCFS,"ldc <REG_N>,SR"					,1,1,CO,fix_none},	//ldc <REG_N>,SR                
	{"i0100_nnnn_0001_1110",Mask_n		,0x401E	,"ldc <REG_N>,GBR"},//},//,Normal				,OpDissCFS,"ldc <REG_N>,GBR"				,1,1,CO,fix_none},	//ldc <REG_N>,GBR               
	{"i0100_nnnn_0010_1110",Mask_n		,0x402E	,"ldc <REG_N>,VBR"},//},//,Normal				,OpDissCFS,"ldc <REG_N>,VBR"				,1,1,CO,fix_none},	//ldc <REG_N>,VBR               
	{"i0100_nnnn_0011_1110",Mask_n		,0x403E	,"ldc <REG_N>,SSR"},//},//,Normal				,OpDissCFS,"ldc <REG_N>,SSR"				,1,1,CO,fix_none},	//ldc <REG_N>,SSR
	
				//LDC Rm,SGR Rm > SGR 0100_mmmm_0011_1010 Privileged —
	{"i0100_nnnn_0011_1010",Mask_n		,0x403A	,"ldc <REG_N>,SGR"},//},//,Normal				,OpDissCFS,"ldc <REG_N>,SGR"				,3,3,CO,fix_none},	//ldc <REG_N>,SGR

	{"i0100_nnnn_0100_1110",Mask_n		,0x404E	,"ldc <REG_N>,SPC"},//},//,Normal				,OpDissCFS,"ldc <REG_N>,SPC"				,1,1,CO,fix_none},	//ldc <REG_N>,SPC               
	{"i0100_nnnn_1mmm_1110",Mask_n_ml3bit,0x408E,"ldc <REG_N>,<RM_BANK>"},//},//,Normal			,OpDissCFS,"ldc <REG_N>,<RM_BANK>"			,1,1,CO,fix_none},	//ldc <REG_N>,<RM_BANK>               
	{"i0100_nnnn_0000_0000",Mask_n		,0x4000	,"shll <REG_N>"},//},//,Normal				,OpDissCFS,"shll <REG_N>"					,1,1,EX,fix_none},	//shll <REG_N>                  
	{"i0100_nnnn_0001_0000",Mask_n		,0x4010	,"dt <REG_N>"},//},//,Normal				,OpDissCFS,"dt <REG_N>"						,1,1,EX,fix_none},	//dt <REG_N>                    
	{"i0100_nnnn_0010_0000",Mask_n		,0x4020	,"shal <REG_N>"},//},//,Normal				,OpDissCFS,"shal <REG_N>"					,1,1,EX,fix_none},	//shal <REG_N>                  
	{"i0100_nnnn_0000_0001",Mask_n		,0x4001	,"shlr <REG_N>"},//},//,Normal				,OpDissCFS,"shlr <REG_N>"					,1,1,EX,fix_none},	//shlr <REG_N>                  
	{"i0100_nnnn_0001_0001",Mask_n		,0x4011	,"cmp/pz <REG_N>"},//},//,Normal				,OpDissCFS,"cmp/pz <REG_N>"					,1,1,MT,fix_none},	//cmp/pz <REG_N>                
	{"i0100_nnnn_0010_0001",Mask_n		,0x4021	,"shar <REG_N>"},//},//,Normal				,OpDissCFS,"shar <REG_N>"					,1,1,EX,fix_none},	//shar <REG_N>                  
	{"i0100_nnnn_0010_0100",Mask_n		,0x4024	,"rotcl <REG_N>"},//},//,Normal				,OpDissCFS,"rotcl <REG_N>"					,1,1,EX,fix_none},	//rotcl <REG_N>                 
	{"i0100_nnnn_0000_0100",Mask_n		,0x4004	,"rotl <REG_N>"},//},//,Normal				,OpDissCFS,"rotl <REG_N>"					,1,1,EX,fix_none},	//rotl <REG_N>                  
	{"i0100_nnnn_0001_0101",Mask_n		,0x4015	,"cmp/pl <REG_N>"},//},//,Normal				,OpDissCFS,"cmp/pl <REG_N>"					,1,1,MT,fix_none},	//cmp/pl <REG_N>                
	{"i0100_nnnn_0010_0101",Mask_n		,0x4025	,"rotcr <REG_N>"},//},//,Normal				,OpDissCFS,"rotcr <REG_N>"					,1,1,EX,fix_none},	//rotcr <REG_N>                 
	{"i0100_nnnn_0000_0101",Mask_n		,0x4005	,"rotr <REG_N>"},//},//,Normal				,OpDissCFS,"rotr <REG_N>"					,1,1,EX,fix_none},	//rotr <REG_N>                  
	{"i0100_nnnn_0000_1000",Mask_n		,0x4008	,"shll2 <REG_N>"},//},//,Normal				,OpDissCFS,"shll2 <REG_N>"					,1,1,EX,fix_none},	//shll2 <REG_N>                 
	{"i0100_nnnn_0001_1000",Mask_n		,0x4018	,"shll8 <REG_N>"},//},//,Normal				,OpDissCFS,"shll8 <REG_N>"					,1,1,EX,fix_none},	//shll8 <REG_N>                 
	{"i0100_nnnn_0010_1000",Mask_n		,0x4028	,"shll16 <REG_N>"},//},//,Normal				,OpDissCFS,"shll16 <REG_N>"					,1,1,EX,fix_none},	//shll16 <REG_N>                
	{"i0100_nnnn_0000_1001",Mask_n		,0x4009	,"shlr2 <REG_N>"},//},//,Normal				,OpDissCFS,"shlr2 <REG_N>"					,1,1,EX,fix_none},	//shlr2 <REG_N>                 
	{"i0100_nnnn_0001_1001",Mask_n		,0x4019	,"shlr8 <REG_N>"},//},//,Normal				,OpDissCFS,"shlr8 <REG_N>"					,1,1,EX,fix_none},	//shlr8 <REG_N>                 
	{"i0100_nnnn_0010_1001",Mask_n		,0x4029	,"shlr16 <REG_N>"},//},//,Normal				,OpDissCFS,"shlr16 <REG_N>"					,1,1,EX,fix_none},	//shlr16 <REG_N>                
	{"i0100_nnnn_0010_1011",Mask_n		,0x402B	,"jmp @<REG_N>"},//},//,Branch_dir_d		,OpDissCFS,"jmp @<REG_N>"					,2,3,CO,fix_none},		//jmp @<REG_N>                  
	{"i0100_nnnn_0000_1011",Mask_n		,0x400B	,"jsr @<REG_N>"},//},//,Branch_dir_d		,OpDissCFS,"jsr @<REG_N>"					,2,3,CO,fix_none},	//jsr @<REG_N>                  
	{"i0100_nnnn_0001_1011",Mask_n		,0x401B	,"tas.b @<REG_N>"},//},//,Normal				,OpDissCFS,"tas.b @<REG_N>"					,5,5,CO,fix_none},	//tas.b @<REG_N>                
	{"i0100_nnnn_mmmm_1100",Mask_n_m	,0x400C	,"shad <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"shad <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//shad <REG_M>,<REG_N>          
	{"i0100_nnnn_mmmm_1101",Mask_n_m	,0x400D	,"shld <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"shld <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//shld <REG_M>,<REG_N>          
	{"i0100_nnnn_mmmm_1111",Mask_n_m	,0x400F	,"mac.w @<REG_M>+,@<REG_N>+"},//},//,Normal				,OpDissCFS,"mac.w @<REG_M>+,@<REG_N>+"		,2,3,CO,fix_none},	//mac.w @<REG_M>+,@<REG_N>+     
	{"i0101_nnnn_mmmm_iiii",Mask_n_m_imm4,0x5000,"mov.l @(<disp4dw>,<REG_M>),<REG_N>"},//},//,Normal			,OpDissCFS,"mov.l @(<disp4dw>,<REG_M>),<REG_N>",1,2,LS,fix_none},//mov.l @(<disp>,<REG_M>),<REG_N>
	{"i0110_nnnn_mmmm_0000",Mask_n_m	,0x6000	,"mov.b @<REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"mov.b @<REG_M>,<REG_N>"			,1,2,LS,fix_none},	//mov.b @<REG_M>,<REG_N>        
	{"i0110_nnnn_mmmm_0001",Mask_n_m	,0x6001	,"mov.w @<REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"mov.w @<REG_M>,<REG_N>"			,1,2,LS,fix_none},	//mov.w @<REG_M>,<REG_N>        
	{"i0110_nnnn_mmmm_0010",Mask_n_m	,0x6002	,"mov.l @<REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"mov.l @<REG_M>,<REG_N>"			,1,2,LS,fix_none},	//mov.l @<REG_M>,<REG_N>        
	{"i0110_nnnn_mmmm_0011",Mask_n_m	,0x6003	,"mov <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"mov <REG_M>,<REG_N>"			,1,0,MT,fix_none},	//mov <REG_M>,<REG_N>           
	{"i0110_nnnn_mmmm_0100",Mask_n_m	,0x6004	,"mov.b @<REG_M>+,<REG_N>"},//},//,Normal				,OpDissCFS,"mov.b @<REG_M>+,<REG_N>"		,1,1,LS,fix_none},	//mov.b @<REG_M>+,<REG_N>       
	{"i0110_nnnn_mmmm_0101",Mask_n_m	,0x6005	,"mov.w @<REG_M>+,<REG_N>"},//},//,Normal				,OpDissCFS,"mov.w @<REG_M>+,<REG_N>"		,1,1,LS,fix_none},	//mov.w @<REG_M>+,<REG_N>       
	{"i0110_nnnn_mmmm_0110",Mask_n_m	,0x6006	,"mov.l @<REG_M>+,<REG_N>"},//},//,Normal				,OpDissCFS,"mov.l @<REG_M>+,<REG_N>"		,1,1,LS,fix_none},	//mov.l @<REG_M>+,<REG_N>       
	{"i0110_nnnn_mmmm_0111",Mask_n_m	,0x6007	,"not <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"not <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//not <REG_M>,<REG_N>           
	{"i0110_nnnn_mmmm_1000",Mask_n_m	,0x6008	,"swap.b <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"swap.b <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//swap.b <REG_M>,<REG_N>        
	{"i0110_nnnn_mmmm_1001",Mask_n_m	,0x6009	,"swap.w <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"swap.w <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//swap.w <REG_M>,<REG_N>        
	{"i0110_nnnn_mmmm_1010",Mask_n_m	,0x600A	,"negc <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"negc <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//negc <REG_M>,<REG_N>          
	{"i0110_nnnn_mmmm_1011",Mask_n_m	,0x600B	,"neg <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"neg <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//neg <REG_M>,<REG_N>           
	{"i0110_nnnn_mmmm_1100",Mask_n_m	,0x600C	,"extu.b <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"extu.b <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//extu.b <REG_M>,<REG_N>        
	{"i0110_nnnn_mmmm_1101",Mask_n_m	,0x600D	,"extu.w <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"extu.w <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//extu.w <REG_M>,<REG_N>        
	{"i0110_nnnn_mmmm_1110",Mask_n_m	,0x600E	,"exts.b <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"exts.b <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//exts.b <REG_M>,<REG_N>        
	{"i0110_nnnn_mmmm_1111",Mask_n_m	,0x600F	,"exts.w <REG_M>,<REG_N>"},//},//,Normal				,OpDissCFS,"exts.w <REG_M>,<REG_N>"			,1,1,EX,fix_none},	//exts.w <REG_M>,<REG_N>        
	{"i0111_nnnn_iiii_iiii",Mask_n_imm8,0x7000	,"add #<simm8>,<REG_N>"},//},//,Normal				,OpDissCFS,"add #<simm8>,<REG_N>"			,1,1,EX,fix_none},	//add #<imm>,<REG_N>
	{"i1000_1011_iiii_iiii",Mask_imm8	,0x8B00	,"bf <bdisp8>"},//},//,Branch_rel			,OpDissCFS,"bf <bdisp8>"					,1,1,BR,fix_none},	// bf <bdisp8>                   
	{"i1000_1111_iiii_iiii",Mask_imm8	,0x8F00	,"bf.s <bdisp8>"},//},//,Branch_rel_d		,OpDissCFS,"bf.s <bdisp8>"					,1,1,BR,fix_none},	// bf.s <bdisp8>                 
	{"i1000_1001_iiii_iiii",Mask_imm8	,0x8900	,"bt <bdisp8>"},//},//,Branch_rel			,OpDissCFS,"bt <bdisp8>"					,1,1,BR,fix_none},	// bt <bdisp8>                   
	{"i1000_1101_iiii_iiii",Mask_imm8	,0x8D00	,"bt.s <bdisp8>"},//},//,Branch_rel_d		,OpDissCFS,"bt.s <bdisp8>"					,1,1,BR,fix_none},	// bt.s <bdisp8>                 
	{"i1000_1000_iiii_iiii",Mask_imm8	,0x8800	,"cmp/eq #<simm8hex>,R0"},//},//,Normal				,OpDissCFS,"cmp/eq #<simm8hex>,R0"			,1,1,MT,fix_none},	// cmp/eq #<imm>,R0              
	{"i1000_0000_mmmm_iiii",Mask_imm8	,0x8000	,"mov.b R0,@(<disp4b>,<REG_M>)"},//},//,Normal				,OpDissCFS,"mov.b R0,@(<disp4b>,<REG_M>)"	,1,1,LS,fix_none},	// mov.b R0,@(<disp>,<REG_M>)    
	{"i1000_0001_mmmm_iiii",Mask_imm8	,0x8100	,"mov.w R0,@(<disp4w>,<REG_M>)"},//},//,Normal				,OpDissCFS,"mov.w R0,@(<disp4w>,<REG_M>)"	,1,1,LS,fix_none},	// mov.w R0,@(<disp>,<REG_M>)    
	{"i1000_0100_mmmm_iiii",Mask_imm8	,0x8400	,"mov.b @(<disp4b>,<REG_M>),R0"},//},//,Normal				,OpDissCFS,"mov.b @(<disp4b>,<REG_M>),R0"	,1,2,LS,fix_none},	// mov.b @(<disp>,<REG_M>),R0    
	{"i1000_0101_mmmm_iiii",Mask_imm8	,0x8500	,"mov.w @(<disp4w>,<REG_M>),R0"},//},//,Normal				,OpDissCFS,"mov.w @(<disp4w>,<REG_M>),R0"	,1,2,LS,fix_none},	// mov.w @(<disp>,<REG_M>),R0    
	{"i1001_nnnn_iiii_iiii",Mask_n_imm8,0x9000	,"mov.w @(<PCdisp8w>),<REG_N>"},//},//,ReadsPC			,OpDissCFS,"mov.w @(<PCdisp8w>),<REG_N>"	,1,2,LS,fix_none},	//mov.w @(<disp>,PC),<REG_N>   
	{"i1010_iiii_iiii_iiii",Mask_n_imm8,0xA000	,"bra <bdisp12>"},//},//,Branch_rel_d		,OpDissCFS,"bra <bdisp12>"					,1,2,BR,fix_none},	// bra <bdisp12>
	{"i1011_iiii_iiii_iiii",Mask_n_imm8,0xB000	,"bsr <bdisp12>"},//},//,Branch_rel_d		,OpDissCFS,"bsr <bdisp12>"					,1,2,BR,fix_none},	// bsr <bdisp12>
	{"i1100_0000_iiii_iiii",Mask_imm8	,0xC000	,"mov.b R0,@(<disp8b>,GBR)"},//},//,Normal				,OpDissCFS,"mov.b R0,@(<disp8b>,GBR)"		,1,1,LS,fix_none},	// mov.b R0,@(<disp>,GBR)        
	{"i1100_0001_iiii_iiii",Mask_imm8	,0xC100	,"mov.w R0,@(<disp8w>,GBR)"},//},//,Normal				,OpDissCFS,"mov.w R0,@(<disp8w>,GBR)"		,1,1,LS,fix_none},	// mov.w R0,@(<disp>,GBR)        
	{"i1100_0010_iiii_iiii",Mask_imm8	,0xC200	,"mov.l R0,@(<disp8dw>,GBR)"},//},//,Normal				,OpDissCFS,"mov.l R0,@(<disp8dw>,GBR)"		,1,1,LS,fix_none},	// mov.l R0,@(<disp>,GBR)        
	{"i1100_0011_iiii_iiii",Mask_imm8	,0xC300	,"trapa #<imm8>"},//},//,ReadWritePC		,OpDissCFS,"trapa #<imm8>"					,7,7,CO,fix_none},	// trapa #<imm>                  
	{"i1100_0100_iiii_iiii",Mask_imm8	,0xC400	,"mov.b @(<GBRdisp8b>),R0"},//},//,Normal				,OpDissCFS,"mov.b @(<GBRdisp8b>),R0"		,1,2,LS,fix_none},	// mov.b @(<disp>,GBR),R0        
	{"i1100_0101_iiii_iiii",Mask_imm8	,0xC500	,"mov.w @(<GBRdisp8w>),R0"},//},//,Normal				,OpDissCFS,"mov.w @(<GBRdisp8w>),R0"		,1,2,LS,fix_none},	// mov.w @(<disp>,GBR),R0        
	{"i1100_0110_iiii_iiii",Mask_imm8	,0xC600	,"mov.l @(<GBRdisp8dw>),R0"},//},//,Normal				,OpDissCFS,"mov.l @(<GBRdisp8dw>),R0"		,1,2,LS,fix_none},	// mov.l @(<disp>,GBR),R0        
	{"i1100_0111_iiii_iiii",Mask_imm8	,0xC700	,"mova @(<PCdisp8d>),R0"},//},//,ReadsPC			,OpDissCFS,"mova @(<PCdisp8d>),R0"			,1,1,EX,fix_none},	// mova @(<disp>,PC),R0          
	{"i1100_1000_iiii_iiii",Mask_imm8	,0xC800	,"tst #<imm8>,R0"},//},//,Normal				,OpDissCFS,"tst #<imm8>,R0"					,1,1,MT,fix_none},	// tst #<imm>,R0                 
	{"i1100_1001_iiii_iiii",Mask_imm8	,0xC900	,"and #<imm8>,R0"},//},//,Normal				,OpDissCFS,"and #<imm8>,R0"					,1,1,EX,fix_none},	// and #<imm>,R0                 
	{"i1100_1010_iiii_iiii",Mask_imm8	,0xCA00	,"xor #<imm8>,R0"},//},//,Normal				,OpDissCFS,"xor #<imm8>,R0"					,1,1,EX,fix_none},	// xor #<imm>,R0                 
	{"i1100_1011_iiii_iiii",Mask_imm8	,0xCB00	,"or #<imm8>,R0"},//},//,Normal				,OpDissCFS,"or #<imm8>,R0"					,1,1,EX,fix_none},	// or #<imm>,R0                  
	{"i1100_1100_iiii_iiii",Mask_imm8	,0xCC00	,"tst.b #<imm8>,@(R0,GBR)"},//},//,Normal				,OpDissCFS,"tst.b #<imm8>,@(R0,GBR)"		,3,3,CO,fix_none},	// tst.b #<imm>,@(R0,GBR)        
	{"i1100_1101_iiii_iiii",Mask_imm8	,0xCD00	,"and.b #<imm8>,@(R0,GBR)"},//},//,Normal				,OpDissCFS,"and.b #<imm8>,@(R0,GBR)"		,4,4,CO,fix_none},	// and.b #<imm>,@(R0,GBR)        
	{"i1100_1110_iiii_iiii",Mask_imm8	,0xCE00	,"xor.b #<imm8>,@(R0,GBR)"},//},//,Normal				,OpDissCFS,"xor.b #<imm8>,@(R0,GBR)"		,4,4,CO,fix_none},	// xor.b #<imm>,@(R0,GBR)        
	{"i1100_1111_iiii_iiii",Mask_imm8	,0xCF00	,"or.b #<imm8>,@(R0,GBR)"},//},//,Normal				,OpDissCFS,"or.b #<imm8>,@(R0,GBR)"			,4,4,CO,fix_none},	// or.b #<imm>,@(R0,GBR)         
	{"i1101_nnnn_iiii_iiii",Mask_n_imm8,0xD000	,"mov.l @(<PCdisp8d>),<REG_N>"},//},//,ReadsPC			,OpDissCFS,"mov.l @(<PCdisp8d>),<REG_N>"	,1,2,CO,fix_none},	// mov.l @(<disp>,PC),<REG_N>    
	{"i1110_nnnn_iiii_iiii",Mask_n_imm8,0xE000	,"mov #<simm8hex>,<REG_N>"},//},//,Normal				,OpDissCFS,"mov #<simm8hex>,<REG_N>"		,1,1,EX,fix_none},	// mov #<imm>,<REG_N>
	
	//and here are the new ones :D
	{"i1111_nnnn_mmmm_0000",Mask_n_m		,0xF000,"fadd <FREG_M>,<FREG_N>"},//,Normal			,d1111_nnnn_mmmm_0000,""					,1,3,FE,fix_none},	//fadd <FREG_M>,<FREG_N>
	{"i1111_nnnn_mmmm_0001",Mask_n_m		,0xF001,"fsub <FREG_M>,<FREG_N>"},//,Normal			,d1111_nnnn_mmmm_0001,""					,1,3,FE,fix_none},	//fsub <FREG_M>,<FREG_N>   
	{"i1111_nnnn_mmmm_0010",Mask_n_m		,0xF002,"fmul <FREG_M>,<FREG_N>"},//,Normal			,d1111_nnnn_mmmm_0010,""					,1,3,FE,fix_none},	//fmul <FREG_M>,<FREG_N>   
	{"i1111_nnnn_mmmm_0011",Mask_n_m		,0xF003,"fdiv <FREG_M>,<FREG_N>"},//},//,Normal			,d1111_nnnn_mmmm_0011,""					,1,12,FE,fix_none},//fdiv <FREG_M>,<FREG_N>   
	{"i1111_nnnn_mmmm_0100",Mask_n_m		,0xF004,"fcmp/eq <FREG_M>,<FREG_N>"},//},//,Normal			,d1111_nnnn_mmmm_0100,""					,1,4,FE,fix_none},	//fcmp/eq <FREG_M>,<FREG_N>
	{"i1111_nnnn_mmmm_0101",Mask_n_m		,0xF005,"fcmp/gt <FREG_M>,<FREG_N>"},//},//,Normal			,d1111_nnnn_mmmm_0101,""					,1,4,FE,fix_none},	//fcmp/gt <FREG_M>,<FREG_N>
	{"i1111_nnnn_mmmm_0110",Mask_n_m		,0xF006,"fmov.s @(R0,<REG_M>),<FREG_N>"},//},//,Normal			,d1111_nnnn_mmmm_0110,""					,1,2,LS,fix_none},	//fmov.s @(R0,<REG_M>),<FREG_N>
	{"i1111_nnnn_mmmm_0111",Mask_n_m		,0xF007,"fmov.s <FREG_M>,@(R0,<REG_N>)"},//},//,Normal			,d1111_nnnn_mmmm_0111,""					,1,1,LS,fix_none},	//fmov.s <FREG_M>,@(R0,<REG_N>)
	{"i1111_nnnn_mmmm_1000",Mask_n_m		,0xF008,"fmov.s @<REG_M>,<FREG_N> "},//},//,Normal			,d1111_nnnn_mmmm_1000,""					,1,2,LS,fix_none},	//fmov.s @<REG_M>,<FREG_N> 
	{"i1111_nnnn_mmmm_1001",Mask_n_m		,0xF009,"fmov.s @<REG_M>+,<FREG_N>"},//},//,Normal			,d1111_nnnn_mmmm_1001,""					,1,2,LS,fix_none},	//fmov.s @<REG_M>+,<FREG_N>
	{"i1111_nnnn_mmmm_1010",Mask_n_m		,0xF00A,"fmov.s <FREG_M>,@<REG_N>"},//},//,Normal			,d1111_nnnn_mmmm_1010,""					,1,1,LS,fix_none},	//fmov.s <FREG_M>,@<REG_N>
	{"i1111_nnnn_mmmm_1011",Mask_n_m		,0xF00B,"fmov.s <FREG_M>,@-<REG_N>"},//},//,Normal			,d1111_nnnn_mmmm_1011,""					,1,1,LS,rn_fpu_4},	//fmov.s <FREG_M>,@-<REG_N>
	{"i1111_nnnn_mmmm_1100",Mask_n_m		,0xF00C,"fmov <FREG_M>,<FREG_N>"},//},//,Normal			,d1111_nnnn_mmmm_1100,""					,1,0,LS,fix_none},	//fmov <FREG_M>,<FREG_N>   
	{"i1111_nnnn_0101_1101",Mask_n			,0xF05D,"fabs <FREG_N>"},//},//,Normal			,d1111_nnnn_0101_1101,""					,1,0,LS,fix_none},	//fabs <FREG_N>            
	{"i1111_nnn0_1111_1101",Mask_nh3bit		,0xF0FD,"FSCA FPUL, DRn"},//,Normal			,OpDissFSCA			 ,""					,1,4,FE,fix_none},	//FSCA FPUL, DRn//F0FD//1111_nnnn_1111_1101
	{"i1111_nnnn_1011_1101",Mask_n			,0xF0BD,"fcnvds <DR_N>,FPUL"},//},//,Normal			,d1111_nnnn_1011_1101,""					,1,4,FE,fix_none},	//fcnvds <DR_N>,FPUL       
	{"i1111_nnnn_1010_1101",Mask_n			,0xF0AD,"fcnvsd FPUL,<DR_N>"},//},//,Normal			,d1111_nnnn_1010_1101,""					,1,4,FE,fix_none},	//fcnvsd FPUL,<DR_N>       
	{"i1111_nnmm_1110_1101",Mask_n			,0xF0ED,"fipr <FV_M>,<FV_N>"},//},//,Normal			,OpDissfipr			 ,""					,1,4,FE,fix_none},	//fipr <FV_M>,<FV_N>            
	{"i1111_nnnn_1000_1101",Mask_n			,0xF08D,"fldi0 <FREG_N>"},//},//,Normal			,d1111_nnnn_1000_1101,""					,1,0,LS,fix_none},	//fldi0 <FREG_N>           
	{"i1111_nnnn_1001_1101",Mask_n			,0xF09D,"fldi1 <FREG_N>"},//},//,Normal			,d1111_nnnn_1001_1101,""					,1,0,LS,fix_none},	//fldi1 <FREG_N>           
	{"i1111_nnnn_0001_1101",Mask_n			,0xF01D,"flds <FREG_N>,FPUL"},//},//,Normal			,d1111_nnnn_0001_1101,""					,1,0,LS,fix_none},	//flds <FREG_N>,FPUL       
	{"i1111_nnnn_0010_1101",Mask_n			,0xF02D,"float FPUL,<FREG_N>"},//},//,Normal			,d1111_nnnn_0010_1101,""					,1,3,FE,fix_none},	//float FPUL,<FREG_N>      
	{"i1111_nnnn_0100_1101",Mask_n			,0xF04D,"fneg <FREG_N>"},//},//,Normal			,d1111_nnnn_0100_1101,""					,1,0,LS,fix_none},	//fneg <FREG_N>            
	{"i1111_1011_1111_1101",Mask_none		,0xFBFD,"frchg"},//},//,WritesFPSCR		,OpDissCFS,"frchg"							,1,2,FE,fix_none},	//frchg                    
	{"i1111_0011_1111_1101",Mask_none		,0xF3FD,"fschg"},//},//,WritesFPSCR		,OpDissCFS,"fschg"							,1,2,FE,fix_none},	//fschg                    
	{"i1111_nnnn_0110_1101",Mask_n			,0xF06D,"fsqrt <FREG_N>"},//},//,Normal			,d1111_nnnn_0110_1101,""					,1,12,FE,fix_none},//fsqrt <FREG_N>                
	{"i1111_nnnn_0011_1101",Mask_n			,0xF03D,"ftrc <FREG_N>, FPUL"},//},//,Normal			,d1111_nnnn_0011_1101,""					,1,4,FE,fix_none},	//ftrc <FREG_N>, FPUL      
	{"i1111_nnnn_0000_1101",Mask_n			,0xF00D,"fsts FPUL,<FREG_N>"},//},//,Normal			,d1111_nnnn_0000_1101,""					,1,0,LS,fix_none},	//fsts FPUL,<FREG_N>       
	{"i1111_nn01_1111_1101",Mask_nh2bit		,0xF1FD,"ftrv xmtrx,<FV_N>" },//,Normal			,OpDissftrv			 ,""					,1,6,FE,fix_none},	//ftrv xmtrx,<FV_N> 
	{"i1111_nnnn_mmmm_1110",Mask_n_m		,0xF00E,"fmac <FREG_0>,<FREG_M>,<FREG_N>"},//},//,Normal			,OpDissfmac			 ,""					,1,4,FE,fix_none},	//fmac <FREG_0>,<FREG_M>,<FREG_N> 
	{"i1111_nnnn_0111_1101",Mask_n			,0xF07D,"FSRRA <FREG_N> (1111nnnn 01111101)"},//},//,Normal			,d1111_nnnn_0111_1101,""					,1,4,FE,fix_none},	//FSRRA <FREG_N> (1111nnnn 01111101)
	/*
	//HLE ops
	{0								,gdrom_hle_op			,Mask_none	,GDROM_OPCODE,ReadWritePC	,dissasm_GDROM},
	{"sh4_bpt_op",Mask_none	,BPT_OPCODE	 },//,ReadWritePC	,dissasm_Break},
*/
	//end of list
	{0,0,0}//,0,ReadWritePC}//Branch in order to stop the block and save pc ect :)
	
};
#include <stdio.h>

sh4_opcodelistentry* FindOpcode(u32 bmask,u32 bkey)
{
	u32 rv=-1;
	for(int i=0;opcodes[i].name;i++)
	{
		if ((opcodes[i].key&bmask)==bkey)
		{
			return &opcodes[i];
		}
	}
	return 0;

}
u32 FindSharedMask(u32 bmask,u32 bkey)
{
	u32 rv=-1;
	for(int i=0;opcodes[i].name;i++)
	{
		if ((opcodes[i].key&bmask)==bkey)
		{
			rv&=opcodes[i].mask;
		}
	}
	return rv;
}
u32 findpath(u32 bmask,u32 bkey,u32 id)
{
	u32 nmask=FindSharedMask(bmask,bkey);
	if (nmask==bmask)
	{
		//found !
		printf("case 0x%04X:\n//0x%04X 0x%04X %s %s\ncall_opcode(%s);\nbreak;\n",id,nmask,bkey,FindOpcode(nmask,bkey)->name,FindOpcode(nmask,bkey)->dissasm,FindOpcode(nmask,bkey)->name);
		
	}
	else
	{

	}
	return nmask;
}
void magicstuff()
{
	u32 lvl1=FindSharedMask(0,0);
	//printf("Level 2\n");
	
	for (int cntl2=0;cntl2<16;cntl2++)
	{
		u32 vl2=cntl2<<12;
		u32 lvl2=findpath(lvl1,vl2,cntl2);
		if (lvl2==lvl1)
			continue;
		if (lvl2==0xF00F)
		{
			printf("case 0x%X:\n//0x%04X 0x%04X -> 0x%04X\nswitch(opcode&0xF)\n{\n",cntl2,lvl1,vl2,lvl2);
			for (int cntl3=0;cntl3<16;cntl3++)
			{
				u32 vl3=cntl3<<0;
				u32 lvl3=findpath(lvl2,vl2 | vl3,cntl3);
				if (lvl3==lvl2)
					continue;
				if (lvl3==-1)
				{
					printf("//0x%X -> Not existing\n",cntl3);
					continue;
				}
				if (lvl3==0xF0FF)
				{
					printf("case 0x%X:\n//0x%04X 0x%04X -> 0x%04X\nswitch((opcode>>4)&0xF)\n{\n",cntl3,lvl2,vl2|vl3,lvl3);
					for (int cntl4=0;cntl4<16;cntl4++)
					{
						u32 vl4=cntl4<<4;
						u32 lvl4=findpath(lvl3,vl2 | vl3|vl4,cntl4);
						if (lvl4==lvl3)
							continue;
						if (lvl4==-1)
						{
							printf("//0x%X -> Not existing\n",cntl4);
							continue;
						}
						printf("case 0x%X:\n//0x%04X 0x%04X -> 0x%04X\nmissing_op(opcode);\n/*switch(~opcode~)\n{\n",cntl4,lvl3,vl2|vl3|vl4,lvl4);
						printf("default:\ninvalid_op(opcode);\n} */\n break;\n");
					}
					printf("default:\ninvalid_op(opcode);\n} \n break;\n");
				}
				else
				{
					printf("case 0x%X:\n//0x%04X 0x%04X -> 0x%04X\nmissing_op(opcode);\n/*switch(~opcode~)\n{\n",cntl3,vl2 | vl3,vl3,lvl3);
					printf("default:\ninvalid_op(opcode);\n} */\n break;\n");
				}
			}
			printf("default:\ninvalid_op(opcode);\n} \n break;\n");
		}
		if (lvl2==0xFF00)
		{
			printf("case 0x%X:\n//0x%04X 0x%04X -> 0x%04X\nswitch((opcode>>8)&0xF)\n{\n",cntl2,lvl1,vl2,lvl2);
			for (int cntl3=0;cntl3<16;cntl3++)
			{
				u32 vl3=cntl3<<8;
				u32 lvl3=findpath(lvl2,vl2 | vl3,cntl3);
				if (lvl3==lvl2)
					continue;
				if (lvl3==-1)
				{
					printf("//0x%X -> Not existing\n",cntl3);
					continue;
				}
				
				{
					printf("case 0x%X:\n//0x%04X 0x%04X -> 0x%04X\nmissing_op(opcode);\n/*switch(~opcode~)\n{\n",cntl3,vl2 | vl3,vl3,lvl3);
					printf("default:\ninvalid_op(opcode);\n} */\n break;\n");
				}
			}
			printf("default:\ninvalid_op(opcode);\n} \n break;\n");
		}
	}
	
/*
	for(int i=0;opcodes[i].name;i++)
	{
		printf("//%s\nopcode(%s)\n{\nmissing_op(op,\"%s\");\n}\n",opcodes[i].dissasm,opcodes[i].name,opcodes[i].dissasm);
	}
	*/
}
#endif


#if TARGET_PSP
PSP_MODULE_INFO("nullDCe/psp v0", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <pspctrl.h>
#include <pspgu.h>
#include <psprtc.h>

#include "C:\devkitPro\msys\psp\sdk\samples\gu\common/callbacks.h"
#include "C:\devkitPro\msys\psp\sdk\samples\gu\common/vram.h"
#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
static unsigned int __attribute__((aligned(16))) list[262144];

static unsigned short __attribute__((aligned(16))) pixels[BUF_WIDTH*SCR_HEIGHT];
#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
int frame_count = 0;
void* fbp0;
void* fbp1;
void* zbp;
float curr_ms = 1.0f;
int blit_method = 0;
int swizzle = 0;
SceCtrlData oldPad;
u64 last_tick;
u32 tick_frequency;

struct Vertex
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
};
void pspinit()
{
	unsigned int x,y;
	pspDebugScreenInit();
	setupCallbacks();

	// Setup GU

	 fbp0 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
	 fbp1 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
	 zbp = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_4444);

	sceGuInit();

	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_8888,fbp0,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,fbp1,BUF_WIDTH);
	sceGuDepthBuffer(zbp,BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(65535,0);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);

	
	oldPad.Buttons = 0;

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(0);

	sceRtcGetCurrentTick(&last_tick);
	tick_frequency = sceRtcGetTickResolution();
	
}
void peridical_stuff();
#endif
int main()
{
	#if TARGET_PSP
		pspinit();
	#endif
	
	sysf(EMU_FULLNAME "\n\n");

	sysf(" \"I embrace nothingness, void is my friend\n  for within my maddness solitude is painfull\"\n");
	sysf("\n");
	

	dcInit();
	
	dcReset(true);
	//for(;;)
		//peridical_stuff();
	dcResume();
	/**/
	
	run_sh4();

	dcPause();
	dcTerm();
	
	return 0;
}
#if TARGET_PSP
extern u8* sh4_vram;

void simpleBlit(int sx, int sy, int sw, int sh, int dx, int dy)
{
	// simple blit, this just copies A->B, with all the cache-misses that apply

	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));

	vertices[0].u = sx; vertices[0].v = sy;
	vertices[0].color = 0;
	vertices[0].x = dx; vertices[0].y = dy; vertices[0].z = 0;

	vertices[1].u = sx+sw; vertices[1].v = sy+sh;
	vertices[1].color = 0;
	vertices[1].x = dx+sw; vertices[1].y = dy+sh; vertices[1].z = 0;

	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_4444|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
}

void advancedBlit(int sx, int sy, int sw, int sh, int dx, int dy, int slice)
{
	int start, end;

	// blit maximizing the use of the texture-cache

	for (start = sx, end = sx+sw; start < end; start += slice, dx += slice)
	{
		struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
		int width = (start + slice) < end ? slice : end-start;

		vertices[0].u = start; vertices[0].v = sy;
		vertices[0].color = 0;
		vertices[0].x = dx; vertices[0].y = dy; vertices[0].z = 0;

		vertices[1].u = start + width; vertices[1].v = sy + sh;
		vertices[1].color = 0;
		vertices[1].x = dx + width; vertices[1].y = dy + sh; vertices[1].z = 0;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_4444|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
	}
}


#endif
void peridical_stuff()
{
	#if TARGET_PSP
	SceCtrlData pad;

		sceGuStart(GU_DIRECT,list);

		// switch methods if requested

		if(sceCtrlPeekBufferPositive(&pad, 1))
		{
			if (pad.Buttons != oldPad.Buttons)
			{
				if(pad.Buttons & PSP_CTRL_CROSS)
					blit_method ^= 1;
				if(pad.Buttons & PSP_CTRL_CIRCLE)
					swizzle ^= 1;
			}
			oldPad = pad;
		}

		sceGuTexMode(GU_PSM_5650,0,0,swizzle); // 16-bit RGBA
		sceGuTexImage(0,512,512,512,pixels); // setup texture as a 512x512 texture, even though the buffer is only 512x272 (480 visible)
		sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA); // don't get influenced by any vertex colors
		sceGuTexFilter(GU_NEAREST,GU_NEAREST); // point-filtered sampling

		if (blit_method)
			advancedBlit(0,0,SCR_WIDTH,SCR_HEIGHT,0,0,32);
		else
			simpleBlit(0,0,SCR_WIDTH,SCR_HEIGHT,0,0);

		sceGuFinish();
		sceGuSync(0,0);

		float curr_fps = 1.0f / curr_ms;

		pspDebugScreenSetOffset((int)fbp0);
		pspDebugScreenSetXY(0,0);
		pspDebugScreenPrintf("fps: %d.%03d (%dMB/s) (X = mode, O = swizzle) %s",(int)curr_fps,(int)((curr_fps-(int)curr_fps) * 1000.0f),(((int)curr_fps * SCR_WIDTH * SCR_HEIGHT * 2)/(1024*1024)),"FUCKME");

//		sceDisplayWaitVblankStart();
		fbp0 = sceGuSwapBuffers();

		// simple frame rate counter

		++frame_count;
		u64 curr_tick;
		sceRtcGetCurrentTick(&curr_tick);
		if ((curr_tick-last_tick) >= tick_frequency)
		{
			float time_span = ((int)(curr_tick-last_tick)) / (float)tick_frequency;
			curr_ms = time_span / frame_count;

			frame_count = 0;
			sceRtcGetCurrentTick(&last_tick);
		}
#endif
}