#pragma once
#include "types.h"
#define rec_sh4op(str) void  __fastcall rec_##str (u32 op,u32 pc)

 rec_sh4op(cpu_opcode_nimp);
 rec_sh4op(fpu_opcode);

// sub <REG_M>,<REG_N>           
 rec_sh4op(i0011_nnnn_mmmm_1000);

//add <REG_M>,<REG_N>           
 rec_sh4op(i0011_nnnn_mmmm_1100);
 //
// 7xxx

//add #<imm>,<REG_N>
 rec_sh4op(i0111_nnnn_iiii_iiii);
 //Bitwise logical operations
//

//and <REG_M>,<REG_N>           
 rec_sh4op(i0010_nnnn_mmmm_1001);
//xor <REG_M>,<REG_N>           
 rec_sh4op(i0010_nnnn_mmmm_1010);
//or <REG_M>,<REG_N>            
 rec_sh4op(i0010_nnnn_mmmm_1011);
//shll2 <REG_N>                 
 rec_sh4op(i0100_nnnn_0000_1000);
//shll8 <REG_N>                 
 rec_sh4op(i0100_nnnn_0001_1000);
//shll16 <REG_N>                
 rec_sh4op(i0100_nnnn_0010_1000);
//shlr2 <REG_N>                 
 rec_sh4op(i0100_nnnn_0000_1001);
//shlr8 <REG_N>                 
 rec_sh4op(i0100_nnnn_0001_1001);
//shlr16 <REG_N>                
 rec_sh4op(i0100_nnnn_0010_1001);
// and #<imm>,R0                 
 rec_sh4op(i1100_1001_iiii_iiii);
// xor #<imm>,R0                 
 rec_sh4op(i1100_1010_iiii_iiii);
// or #<imm>,R0                  
 rec_sh4op(i1100_1011_iiii_iiii);
 //this file contains ALL register to register full moves
//

//stc SR,<REG_N>                
 rec_sh4op(i0000_nnnn_0000_0010);
//stc GBR,<REG_N>               
 rec_sh4op(i0000_nnnn_0001_0010);
//stc VBR,<REG_N>               
 rec_sh4op(i0000_nnnn_0010_0010);
//stc SSR,<REG_N>               
 rec_sh4op(i0000_nnnn_0011_0010);
//stc SPC,<REG_N>               
 rec_sh4op(i0000_nnnn_0100_0010);
//stc RM_BANK,<REG_N>           
 rec_sh4op(i0000_nnnn_1mmm_0010);
//sts FPUL,<REG_N>              
 rec_sh4op(i0000_nnnn_0101_1010);
 //sts FPSCR,<REG_N>             
 rec_sh4op(i0000_nnnn_0110_1010);
//stc DBR,<REG_N>             
 rec_sh4op(i0000_nnnn_1111_1010);
//sts MACH,<REG_N>              
 rec_sh4op(i0000_nnnn_0000_1010);
//sts MACL,<REG_N>              
 rec_sh4op(i0000_nnnn_0001_1010);
//sts PR,<REG_N>                
 rec_sh4op(i0000_nnnn_0010_1010);
 //mov.b @(R0,<REG_M>),<REG_N>   
 rec_sh4op(i0000_nnnn_mmmm_1100);
//mov.w @(R0,<REG_M>),<REG_N>   
 rec_sh4op(i0000_nnnn_mmmm_1101);
//mov.l @(R0,<REG_M>),<REG_N>   
 rec_sh4op(i0000_nnnn_mmmm_1110);
 //mov.b <REG_M>,@(R0,<REG_N>)   
 rec_sh4op(i0000_nnnn_mmmm_0100);
//mov.w <REG_M>,@(R0,<REG_N>)   
 rec_sh4op(i0000_nnnn_mmmm_0101);
//mov.l <REG_M>,@(R0,<REG_N>)   
 rec_sh4op(i0000_nnnn_mmmm_0110);
//
// 1xxx

//mov.l <REG_M>,@(<disp>,<REG_N>)
 rec_sh4op(i0001_nnnn_mmmm_iiii);
//
//	2xxx

//mov.b <REG_M>,@<REG_N>        
 rec_sh4op(i0010_nnnn_mmmm_0000);
// mov.w <REG_M>,@<REG_N>        
 rec_sh4op(i0010_nnnn_mmmm_0001);
// mov.l <REG_M>,@<REG_N>        
 rec_sh4op(i0010_nnnn_mmmm_0010);
// mov.b <REG_M>,@-<REG_N>       
 rec_sh4op(i0010_nnnn_mmmm_0100);
//mov.w <REG_M>,@-<REG_N>       
 rec_sh4op(i0010_nnnn_mmmm_0101);
//mov.l <REG_M>,@-<REG_N>       
 rec_sh4op(i0010_nnnn_mmmm_0110);
 //
// 4xxx
//sts.l FPUL,@-<REG_N>          
 rec_sh4op(i0100_nnnn_0101_0010);
//sts.l FPSCR,@-<REG_N>         
 rec_sh4op(i0100_nnnn_0110_0010);
//sts.l MACH,@-<REG_N>          
 rec_sh4op(i0100_nnnn_0000_0010);
//sts.l MACL,@-<REG_N>          
 rec_sh4op(i0100_nnnn_0001_0010);
//sts.l PR,@-<REG_N>            
 rec_sh4op(i0100_nnnn_0010_0010);
//stc.l SR,@-<REG_N>            
 rec_sh4op(i0100_nnnn_0000_0011);
//stc.l GBR,@-<REG_N>           
 rec_sh4op(i0100_nnnn_0001_0011);
//stc.l VBR,@-<REG_N>           
 rec_sh4op(i0100_nnnn_0010_0011);
//stc.l SSR,@-<REG_N>           
 rec_sh4op(i0100_nnnn_0011_0011);
//stc.l SPC,@-<REG_N>           
 rec_sh4op(i0100_nnnn_0100_0011);
//stc RM_BANK,@-<REG_N>         
 rec_sh4op(i0100_nnnn_1mmm_0011);
//lds.l @<REG_N>+,MACH          
 rec_sh4op(i0100_nnnn_0000_0110);
//lds.l @<REG_N>+,MACL          
 rec_sh4op(i0100_nnnn_0001_0110);
//lds.l @<REG_N>+,PR            
 rec_sh4op(i0100_nnnn_0010_0110);
//lds.l @<REG_N>+,FPUL          
 rec_sh4op(i0100_nnnn_0101_0110);
//lds.l @<REG_N>+,FPSCR         
 rec_sh4op(i0100_nnnn_0110_0110);
//ldc.l @<REG_N>+,SR            
 rec_sh4op(i0100_nnnn_0000_0111);
//ldc.l @<REG_N>+,GBR           
 rec_sh4op(i0100_nnnn_0001_0111);
//ldc.l @<REG_N>+,VBR           
 rec_sh4op(i0100_nnnn_0010_0111);
//ldc.l @<REG_N>+,SSR           
 rec_sh4op(i0100_nnnn_0011_0111);
//ldc.l @<REG_N>+,SPC           
 rec_sh4op(i0100_nnnn_0100_0111);
//ldc.l @<REG_N>+,RM_BANK       
 rec_sh4op(i0100_nnnn_1mmm_0111);
//lds <REG_N>,MACH              
 rec_sh4op(i0100_nnnn_0000_1010);
//lds <REG_N>,MACL              
 rec_sh4op(i0100_nnnn_0001_1010);
//lds <REG_N>,PR                
 rec_sh4op(i0100_nnnn_0010_1010);
//lds <REG_N>,FPUL              
 rec_sh4op(i0100_nnnn_0101_1010);
//lds <REG_N>,FPSCR             
 rec_sh4op(i0100_nnnn_0110_1010);
//ldc <REG_N>,DBR                
 rec_sh4op(i0100_nnnn_1111_1010);
//ldc <REG_N>,SR                
 rec_sh4op(i0100_nnnn_0000_1110);
//ldc <REG_N>,GBR               
 rec_sh4op(i0100_nnnn_0001_1110);
//ldc <REG_N>,VBR               
 rec_sh4op(i0100_nnnn_0010_1110);
//ldc <REG_N>,SSR               
 rec_sh4op(i0100_nnnn_0011_1110);
//ldc <REG_N>,SPC               
 rec_sh4op(i0100_nnnn_0100_1110);
//ldc <REG_N>,RM_BANK           
 rec_sh4op(i0100_nnnn_1mmm_1110);
 //
// 5xxx

//mov.l @(<disp>,<REG_M>),<REG_N>
 rec_sh4op(i0101_nnnn_mmmm_iiii);
//
// 6xxx
//mov.b @<REG_M>,<REG_N>        
 rec_sh4op(i0110_nnnn_mmmm_0000);
//mov.w @<REG_M>,<REG_N>        
 rec_sh4op(i0110_nnnn_mmmm_0001);
//mov.l @<REG_M>,<REG_N>        
 rec_sh4op(i0110_nnnn_mmmm_0010);
//mov <REG_M>,<REG_N>           
 rec_sh4op(i0110_nnnn_mmmm_0011);
//mov.b @<REG_M>+,<REG_N>       
 rec_sh4op(i0110_nnnn_mmmm_0100);
//mov.w @<REG_M>+,<REG_N>       
 rec_sh4op(i0110_nnnn_mmmm_0101);
//mov.l @<REG_M>+,<REG_N>       
 rec_sh4op(i0110_nnnn_mmmm_0110);
//
//8xxx
 // mov.b R0,@(<disp>,<REG_M>)    
 rec_sh4op(i1000_0000_mmmm_iiii);
// mov.w R0,@(<disp>,<REG_M>)    
 rec_sh4op(i1000_0001_mmmm_iiii);
// mov.b @(<disp>,<REG_M>),R0    
 rec_sh4op(i1000_0100_mmmm_iiii);
// mov.w @(<disp>,<REG_M>),R0    
 rec_sh4op(i1000_0101_mmmm_iiii);

 //
// 9xxx

//mov.w @(<disp>,PC),<REG_N>   
 rec_sh4op(i1001_nnnn_iiii_iiii);
 //
// Cxxx
// mov.b R0,@(<disp>,GBR)        
 rec_sh4op(i1100_0000_iiii_iiii);
// mov.w R0,@(<disp>,GBR)        
 rec_sh4op(i1100_0001_iiii_iiii);
// mov.l R0,@(<disp>,GBR)        
 rec_sh4op(i1100_0010_iiii_iiii);
// mov.b @(<disp>,GBR),R0        
 rec_sh4op(i1100_0100_iiii_iiii);
// mov.w @(<disp>,GBR),R0        
 rec_sh4op(i1100_0101_iiii_iiii);
// mov.l @(<disp>,GBR),R0        
 rec_sh4op(i1100_0110_iiii_iiii);
// mova @(<disp>,PC),R0          
 rec_sh4op(i1100_0111_iiii_iiii);
//
// Dxxx

// mov.l @(<disp>,PC),<REG_N>    
 rec_sh4op(i1101_nnnn_iiii_iiii);
//
// Exxx

// mov #<imm>,<REG_N>
 rec_sh4op(i1110_nnnn_iiii_iiii);