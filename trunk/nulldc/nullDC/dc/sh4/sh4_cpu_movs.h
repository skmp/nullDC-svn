//this file contains ALL register to register full moves
//

//stc SR,<REG_N>                
 sh4op(i0000_nnnn_0000_0010)//0002
{
	u32 n = GetN(op);
	r[n] = sr.full;
}


//stc GBR,<REG_N>               
 sh4op(i0000_nnnn_0001_0010)
{
	//iNimp("stc GBR,<REG_N>");
	u32 n = GetN(op);
	r[n] = gbr;
} 


//stc VBR,<REG_N>               
 sh4op(i0000_nnnn_0010_0010)
{
	//iNimp("stc VBR,<REG_N>  ");
	u32 n = GetN(op);
	r[n] = vbr;
} 


//stc SSR,<REG_N>               
 sh4op(i0000_nnnn_0011_0010)
{
	//iNimp("stc SSR,<REG_N>");
	u32 n = GetN(op);
	r[n] = ssr;
} 


//stc SPC,<REG_N>               
 sh4op(i0000_nnnn_0100_0010)
{
	//iNimp("stc SPC,<REG_N> ");
	u32 n = GetN(op);
	r[n] = spc;
} 


//stc RM_BANK,<REG_N>           
 sh4op(i0000_nnnn_1mmm_0010)
{
	//iNimp(op,"stc R0_BANK,<REG_N>\n");
	u32 n = GetN(op);
	u32 m = GetM(op) & 0x7;
	r[n] = r_bank[m];
} 

//sts FPUL,<REG_N>              
 sh4op(i0000_nnnn_0101_1010)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	r[n] = fpul;
} 

 //sts FPSCR,<REG_N>             
 sh4op(i0000_nnnn_0110_1010)
{
	//iNimp("sts FPSCR,<REG_N>");
	u32 n = GetN(op);
	r[n] = fpscr.full;
	UpdateFPSCR();
}

//stc DBR,<REG_N>             
 sh4op(i0000_nnnn_1111_1010)
{
	u32 n = GetN(op);
	r[n] = dbr;
}


//sts MACH,<REG_N>              
 sh4op(i0000_nnnn_0000_1010)
{
	//iNimp("sts MACH,<REG_N>");
	u32 n = GetN(op);

	r[n] = mach;
} 


//sts MACL,<REG_N>              
 sh4op(i0000_nnnn_0001_1010)
{//TODO : CHECK THIS
	u32 n = GetN(op);
	r[n]=macl; 
} 


//sts PR,<REG_N>                
 sh4op(i0000_nnnn_0010_1010)
{
	//iNimp("sts PR,<REG_N>");
	u32 n = GetN(op);
	r[n] = pr;
} 


 //mov.b @(R0,<REG_M>),<REG_N>   
 sh4op(i0000_nnnn_mmmm_1100)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	ReadMemBOS8(r[n],r[0],r[m]);
	//r[n]= (u32)(s8)ReadMem8(r[0]+r[m]);
} 


//mov.w @(R0,<REG_M>),<REG_N>   
 sh4op(i0000_nnnn_mmmm_1101)
{//ToDo : Check This [26/4/05]
	//iNimp("mov.w @(R0,<REG_M>),<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	ReadMemBOS16(r[n],r[0],r[m]);
	//r[n] = (u32)(s16)ReadMem16(r[0] + r[m]);
} 


//mov.l @(R0,<REG_M>),<REG_N>   
 sh4op(i0000_nnnn_mmmm_1110)
{//TODO : Check This [26/4/05]
	//iNimp("mov.l @(R0,<REG_M>),<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	//r[n] = ReadMem32(r[0] + r[m]);
	ReadMemBOU32(r[n],r[0],r[m]);
} 

 //mov.b <REG_M>,@(R0,<REG_N>)   
 sh4op(i0000_nnnn_mmmm_0100)
{
	//iNimp("mov.b <REG_M>,@(R0,<REG_N>)");
	u32 n = GetN(op);
	u32 m = GetM(op);
	//u8 valor = (u8)(R(m) & 0xFF);
	WriteMemBOU8(r[0],r[n], r[m]);
	//WriteMem8(r[0] + r[n], (u8)r[m]);
	//WriteMemoryB(R(0) + R(n), &valor);
}


//mov.w <REG_M>,@(R0,<REG_N>)   
 sh4op(i0000_nnnn_mmmm_0101)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	WriteMemBOU16(r[0] , r[n], r[m]);
}


//mov.l <REG_M>,@(R0,<REG_N>)   
 sh4op(i0000_nnnn_mmmm_0110)
{//TODO : Check This [26/4/05]
	//iNimp("mov.l <REG_M>,@(R0,<REG_N>)");
	u32 n = GetN(op);
	u32 m = GetM(op);
	WriteMemBOU32(r[0], r[n], r[m]);
	//WriteMemoryL(R(0) + R(n), (DWORD*)&R(m));
}



//
// 1xxx

//mov.l <REG_M>,@(<disp>,<REG_N>)
 sh4op(i0001_nnnn_mmmm_iiii)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	u32 disp = GetImm4(op);
	WriteMemBOU32(r[n] , (disp <<2), r[m]);
}

//
//	2xxx

//mov.b <REG_M>,@<REG_N>        
 sh4op(i0010_nnnn_mmmm_0000)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	WriteMemU8(r[n],r[m] );
}

// mov.w <REG_M>,@<REG_N>        
 sh4op(i0010_nnnn_mmmm_0001)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	WriteMemU16(r[n],r[m]);
}

// mov.l <REG_M>,@<REG_N>        
 sh4op(i0010_nnnn_mmmm_0010)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	WriteMemU32(r[n],r[m]);
}

// mov.b <REG_M>,@-<REG_N>       
 sh4op(i0010_nnnn_mmmm_0100)
{
	//iNimp("mov.b <REG_M>,@-<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	//r[n]--;
	if (n==m)
	{
		iNimp(op,"Mov.b !!!m==n");
		//return;
	}
	r[n]--;
	WriteMemU8(r[n],r[m]);
	

	//WriteMemoryB(r[n], R(m) & 0xFF);

}

//mov.w <REG_M>,@-<REG_N>       
 sh4op(i0010_nnnn_mmmm_0101)
{
	//iNimp("mov.w <REG_M>,@-<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	//r[n] -= 2;
	if (n==m)
	{
		iNimp(op,"Mov.w !!!m==n");
		//return;
	}
	r[n] -= 2;
	WriteMemU16(r[n], r[m]);
}

//mov.l <REG_M>,@-<REG_N>       
 sh4op(i0010_nnnn_mmmm_0110)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	//r[n] -= 4;
	if (m==n)
	{
		iNimp(op,"Mov.l !!!m==n");
		//return;
	}
	r[n] -= 4;
	WriteMemU32(r[n],r[m]);
}

 //
// 4xxx
//sts.l FPUL,@-<REG_N>          
 sh4op(i0100_nnnn_0101_0010)
{
	//iNimp("sts.l FPUL,@-<REG_N>");
	u32 n = GetN(op);
	r[n] -= 4;
	WriteMemU32(r[n], fpul);
}


//sts.l FPSCR,@-<REG_N>         
 sh4op(i0100_nnnn_0110_0010)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	r[n] -= 4;
	WriteMemU32(r[n],fpscr.full);
}


//sts.l MACH,@-<REG_N>          
 sh4op(i0100_nnnn_0000_0010)
{
	//iNimp("sts.l MACH,@-<REG_N>");
	u32 n = GetN(op);
	r[n] -= 4;
	WriteMemU32(r[n], mach);
}


//sts.l MACL,@-<REG_N>          
 sh4op(i0100_nnnn_0001_0010)
{
	//iNimp("sts.l MACL,@-<REG_N>");
	u32 n = GetN(op);
	r[n] -= 4;
	WriteMemU32(r[n], macl);
}


//sts.l PR,@-<REG_N>            
 sh4op(i0100_nnnn_0010_0010)
{//TODO : fAdd this
	u32 n = GetN(op);
	r[n] -= 4;
	WriteMemU32(r[n],pr);
}



//stc.l SR,@-<REG_N>            
 sh4op(i0100_nnnn_0000_0011)
{
	//iNimp("stc.l SR,@-<REG_N>");
	u32 n = GetN(op);
	r[n] -= 4;
	WriteMemU32(r[n], sr.full);
}


//stc.l GBR,@-<REG_N>           
 sh4op(i0100_nnnn_0001_0011)
{
	//iNimp("stc.l GBR,@-<REG_N>");
	u32 n = GetN(op);
	r[n] -= 4;
	WriteMemU32(r[n], gbr);
}


//stc.l VBR,@-<REG_N>           
 sh4op(i0100_nnnn_0010_0011)
{
	//iNimp("stc.l VBR,@-<REG_N>");
	u32 n = GetN(op);
	r[n] -= 4;
	WriteMemU32(r[n], vbr);
}


//stc.l SSR,@-<REG_N>           
 sh4op(i0100_nnnn_0011_0011)
{
	//iNimp("stc.l SSR,@-<REG_N>");
	u32 n = GetN(op);
	r[n] -= 4;
	WriteMemU32(r[n], ssr);
}


//stc.l SPC,@-<REG_N>           
 sh4op(i0100_nnnn_0100_0011)
{
	//iNimp("stc.l SPC,@-<REG_N>");
	u32 n = GetN(op);
	r[n] -= 4;
	WriteMemU32(r[n], spc);
}

//stc RM_BANK,@-<REG_N>         
 sh4op(i0100_nnnn_1mmm_0011)
{
	//iNimp("stc RM_BANK,@-<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op) & 0x07;
	r[n] -= 4;
	WriteMemU32(r[n], r_bank[m]);
}



//lds.l @<REG_N>+,MACH          
 sh4op(i0100_nnnn_0000_0110)
{
	//iNimp("lds.l @<REG_N>+,MACH");
	u32 n = GetN(op);
	ReadMemU32(mach,r[n]);
	//mach = ReadMem32(r[n]);
	r[n] += 4;
}


//lds.l @<REG_N>+,MACL          
 sh4op(i0100_nnnn_0001_0110)
{
	//iNimp("lds.l @<REG_N>+,MACL ");
	u32 n = GetN(op);
	ReadMemU32(macl,r[n]);
	//macl = ReadMem32(r[n]);
	r[n] += 4;
}


//lds.l @<REG_N>+,PR            
 sh4op(i0100_nnnn_0010_0110)
{//TODO : hADD THIS
	u32 n = GetN(op);
	ReadMemU32(pr,r[n]);
	//pr = ReadMem32(r[n]);
	r[n] += 4;
}


//lds.l @<REG_N>+,FPUL          
 sh4op(i0100_nnnn_0101_0110)
{
	//iNimp("lds.l @<REG_N>+,FPUL");
	u32 n = GetN(op);
	//fpul = ReadMem32(r[n]);
	ReadMemU32(fpul,r[n]);
	r[n] += 4;
}


//lds.l @<REG_N>+,FPSCR         
 sh4op(i0100_nnnn_0110_0110)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	//fpscr.full =ReadMem32(r[n]);
	ReadMemU32(fpscr.full,r[n]);
	UpdateFPSCR();
	r[n] += 4;
}


//ldc.l @<REG_N>+,SR            
 sh4op(i0100_nnnn_0000_0111)
{
	//iNimp("ldc.l @<REG_N>+,SR");
	u32 n = GetN(op);
	//sr.SetFull(ReadMem32(r[n])) ;
	ReadMemU32(sr.full,r[n]);
	UpdateSR();
	r[n] += 4;
}


//ldc.l @<REG_N>+,GBR           
 sh4op(i0100_nnnn_0001_0111)
{
	//iNimp("ldc.l @<REG_N>+,GBR");
	u32 n = GetN(op);
	//gbr = ReadMem32(r[n]);
	ReadMemU32(gbr,r[n]);
	r[n] += 4;
}


//ldc.l @<REG_N>+,VBR           
 sh4op(i0100_nnnn_0010_0111)
{
	//iNimp("ldc.l @<REG_N>+,VBR");
	u32 n = GetN(op);
	//vbr = ReadMem32(r[n]);
	ReadMemU32(vbr,r[n]);
	r[n] += 4;
}


//ldc.l @<REG_N>+,SSR           
 sh4op(i0100_nnnn_0011_0111)
{
	//iNimp("ldc.l @<REG_N>+,SSR");
	u32 n = GetN(op);
	//ssr = ReadMem32(r[n]);
	ReadMemU32(ssr,r[n]);
	r[n] += 4;
}


//ldc.l @<REG_N>+,SPC           
 sh4op(i0100_nnnn_0100_0111)
{
	//iNimp("ldc.l @<REG_N>+,SPC");
	u32 n = GetN(op);
	//spc = ReadMem32(r[n]);
	ReadMemU32(spc,r[n]);
	r[n] += 4;
}


//ldc.l @<REG_N>+,RM_BANK       
 sh4op(i0100_nnnn_1mmm_0111)
{
	//iNimp("ldc.l @<REG_N>+,R0_BANK");
	u32 n = GetN(op);
	u32 m = GetM(op) & 7;
	//r_bank[m]= ReadMem32(r[n]);
	ReadMemU32(r_bank[m],r[n]);
	r[n] += 4;
}

//lds <REG_N>,MACH              
 sh4op(i0100_nnnn_0000_1010)
{
	//iNimp("lds <REG_N>,MACH");
	u32 n = GetN(op);
	mach = r[n];
}


//lds <REG_N>,MACL              
 sh4op(i0100_nnnn_0001_1010)
{
	//iNimp("lds <REG_N>,MACL");
	u32 n = GetN(op);
	macl = r[n];
}


//lds <REG_N>,PR                
 sh4op(i0100_nnnn_0010_1010)
{//TODO : check this
	u32 n = GetN(op);
	pr = r[n];
}


//lds <REG_N>,FPUL              
 sh4op(i0100_nnnn_0101_1010)
{//TODO : CHECK THIS
	u32 n = GetN(op);
	fpul =r[n];
}


//lds <REG_N>,FPSCR             
 sh4op(i0100_nnnn_0110_1010)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	fpscr.full = r[n];
	UpdateFPSCR();
}

//ldc <REG_N>,DBR                
 sh4op(i0100_nnnn_1111_1010)
{
	u32 n = GetN(op);
	dbr = r[n];
}

//ldc <REG_N>,SR                
 sh4op(i0100_nnnn_0000_1110)
{
	//iNimp("ldc <REG_N>,SR");
	u32 n = GetN(op);
	sr.full=r[n];
	UpdateSR();
}


//ldc <REG_N>,GBR               
 sh4op(i0100_nnnn_0001_1110)
{
	u32 n = GetN(op);
	gbr = r[n];
}


//ldc <REG_N>,VBR               
 sh4op(i0100_nnnn_0010_1110)
{
	//iNimp("ldc <REG_N>,VBR");
	u32 n = GetN(op);

	vbr = r[n];
}


//ldc <REG_N>,SSR               
 sh4op(i0100_nnnn_0011_1110)
{
	//iNimp("ldc <REG_N>,SSR");
	u32 n = GetN(op);

	ssr = r[n];
}


//ldc <REG_N>,SPC               
 sh4op(i0100_nnnn_0100_1110)
{
	//iNimp("ldc <REG_N>,SPC");
	u32 n = GetN(op);

	spc = r[n];
}


//ldc <REG_N>,RM_BANK           
 sh4op(i0100_nnnn_1mmm_1110)
{
	//iNimp(op, "ldc <REG_N>,RM_BANK");
	u32 n = GetN(op);
	u32 m = GetM(op) & 7;

	r_bank[m] = r[n];
}

 //
// 5xxx

//mov.l @(<disp>,<REG_M>),<REG_N>
 sh4op(i0101_nnnn_mmmm_iiii)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	u32 disp = GetImm4(op) << 2;
	//r[n]=ReadMem32(r[m]+disp);
	ReadMemBOU32(r[n],r[m],disp);
}

//
// 6xxx
//mov.b @<REG_M>,<REG_N>        
 sh4op(i0110_nnnn_mmmm_0000)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	//r[n] = (u32)(s32)(s8)ReadMem8(r[m]);
	ReadMemS8(r[n],r[m]);
} 


//mov.w @<REG_M>,<REG_N>        
 sh4op(i0110_nnnn_mmmm_0001)
{//TODO : Check This [26/4/05]
	//iNimp("mov.w @<REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	//r[n] = (u32)(s32)(s16)ReadMem16(r[m]);
	ReadMemS16(r[n] ,r[m]);
} 


//mov.l @<REG_M>,<REG_N>        
 sh4op(i0110_nnnn_mmmm_0010)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	//r[n]=ReadMem32(r[m]);
	ReadMemU32(r[n],r[m]);
} 


//mov <REG_M>,<REG_N>           
 sh4op(i0110_nnnn_mmmm_0011)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] = r[m];
}


//mov.b @<REG_M>+,<REG_N>       
 sh4op(i0110_nnnn_mmmm_0100)
{//TODO : Check This [26/4/05]
	//iNimp("mov.b @<REG_M>+,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	//r[n] = (u32)(s32)(s8)ReadMem8(r[m]);
	ReadMemS8(r[n],r[m]);
	if (n != m) 
		r[m] += 1;
} 


//mov.w @<REG_M>+,<REG_N>       
 sh4op(i0110_nnnn_mmmm_0101)
{
	//iNimp("mov.w @<REG_M>+,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	//r[n] = (u32)(s16)(u16)ReadMem16(r[m]);
	ReadMemS16(r[n],r[m]);
	if (n != m) 
		r[m] += 2;
} 


//mov.l @<REG_M>+,<REG_N>       
 sh4op(i0110_nnnn_mmmm_0110)
{//TODO : hADD THIS
	u32 n = GetN(op);
	u32 m = GetM(op);

	//r[n]=ReadMem32(r[m]);
	ReadMemU32(r[n],r[m]);
	if (n != m)
		r[m] += 4;

} 

//
//8xxx
 // mov.b R0,@(<disp>,<REG_M>)    
 sh4op(i1000_0000_mmmm_iiii)
{//TODO : Check This [26/4/05]
	//iNimp("mov.b R0,@(<disp>,<REG_M>)");
	u32 n = GetM(op);
	u32 disp = GetImm4(op);
	WriteMemBOU8(r[n],disp,r[0]);
}


// mov.w R0,@(<disp>,<REG_M>)    
 sh4op(i1000_0001_mmmm_iiii)
{//TODO : !Add this
	//iNimp("mov.w R0,@(<disp>,<REG_M>)");
	u32 disp = GetImm4(op);
	u32 m = GetM(op);
	WriteMemBOU16(r[m] , (disp << 1),r[0]);
}


// mov.b @(<disp>,<REG_M>),R0    
 sh4op(i1000_0100_mmmm_iiii)
{//TODO : Check This [26/4/05] x2
	//iNimp("mov.b @(<disp>,<REG_M>),R0");
	u32 disp = GetImm4(op);
	u32 m = GetM(op);
	//r[0] = (u32)(s8)ReadMem8(r[m] + disp);
	ReadMemBOS8(r[0] ,r[m] , disp);
}


// mov.w @(<disp>,<REG_M>),R0    
 sh4op(i1000_0101_mmmm_iiii)
{//TODO : Check This [26/4/05]
	//iNimp("mov.w @(<disp>,<REG_M>),R0");
	u32 disp = GetImm4(op);
	u32 m = GetM(op);
	//r[0] = (u32)(s16)ReadMem16(r[m] + (disp << 1));
	ReadMemBOS16(r[0],r[m] , (disp << 1));
}

 //
// 9xxx

//mov.w @(<disp>,PC),<REG_N>   
 sh4op(i1001_nnnn_iiii_iiii)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 disp = (GetImm8(op));
	//r[n]=(u32)(s32)(s16)ReadMem16((disp<<1) + pc + 4);
	ReadMemS16(r[n],(disp<<1) + pc + 4);
}


 //
// Cxxx
// mov.b R0,@(<disp>,GBR)        
 sh4op(i1100_0000_iiii_iiii)
{
//	iNimp(op, "mov.b R0,@(<disp>,GBR)");
	u32 disp = GetImm8(op); 
	WriteMemBOU8(gbr, disp, r[0]);
}


// mov.w R0,@(<disp>,GBR)        
 sh4op(i1100_0001_iiii_iiii)
{
	//iNimp("mov.w R0,@(<disp>,GBR)");
	u32 disp = GetImm8(op);
	//Write_Word(GBR+(disp<<1),R[0]);
	WriteMemBOU16(gbr , (disp << 1), r[0]);
}


// mov.l R0,@(<disp>,GBR)        
 sh4op(i1100_0010_iiii_iiii)
{
	// iNimp("mov.l R0,@(<disp>,GBR)");
	u32 disp = (GetImm8(op));
	//u32 source = (disp << 2) + gbr;

	WriteMemBOU32(gbr,(disp << 2), r[0]);
}

// mov.b @(<disp>,GBR),R0        
 sh4op(i1100_0100_iiii_iiii)
{
//	iNimp(op, "mov.b @(<disp>,GBR),R0");
	u32 disp = GetImm8(op);
	//r[0] = (u32)(s8)ReadMem8(gbr+disp);
	ReadMemBOS8(r[0],gbr,disp);
}


// mov.w @(<disp>,GBR),R0        
 sh4op(i1100_0101_iiii_iiii)
{
//	iNimp(op, "mov.w @(<disp>,GBR),R0");
	u32 disp = GetImm8(op);
	//r[0] = (u32)(s16)ReadMem16(gbr+(disp<<1) );
	ReadMemBOS16(r[0],gbr,(disp<<1));
}


// mov.l @(<disp>,GBR),R0        
 sh4op(i1100_0110_iiii_iiii)
{
//	iNimp(op, "mov.l @(<disp>,GBR),R0");
	u32 disp = GetImm8(op);
	//r[0] = ReadMem32(gbr+(disp<<2));
	ReadMemBOU32(r[0],gbr,(disp<<2));
}


// mova @(<disp>,PC),R0          
 sh4op(i1100_0111_iiii_iiii)
{//TODO : Check This [26/4/05]
	//u32 disp = (() << 2) + ((pc + 4) & 0xFFFFFFFC);
	r[0] = (pc&0xFFFFFFFC)+4+(GetImm8(op)<<2);
}

//
// Dxxx

// mov.l @(<disp>,PC),<REG_N>    
 sh4op(i1101_nnnn_iiii_iiii)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 disp = (GetImm8(op));
	
	//r[n]=ReadMem32((disp<<2) + (pc & 0xFFFFFFFC) + 4);

	ReadMemU32(r[n],(disp<<2) + (pc & 0xFFFFFFFC) + 4);
}
//
// Exxx

// mov #<imm>,<REG_N>
 sh4op(i1110_nnnn_iiii_iiii)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	r[n] = (u32)(s32)(s8)(GetImm8(op));//(u32)(s8)= signextend8 :)
}


 //movca.l R0, @<REG_N>          
sh4op(i0000_nnnn_1100_0011)
{
	u32 n = GetN(op);
	WriteMemU32(r[n],r[0]);//at r[n],r[0]
	//iWarn(op, "movca.l R0, @<REG_N>");
} 

//clrmac                        
sh4op(i0000_0000_0010_1000)
{
	//iNimp(op, "clrmac");
	macl=0;
	mach=0;
} 