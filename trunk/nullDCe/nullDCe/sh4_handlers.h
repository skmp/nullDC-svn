//stc SR,<REG_N>
opcode(i0000_nnnn_0000_0010)
{
	missing_op(op,"stc SR,<REG_N>");
}
//stc GBR,<REG_N>
opcode(i0000_nnnn_0001_0010)
{
	missing_op(op,"stc GBR,<REG_N>");
}
//stc VBR,<REG_N>
opcode(i0000_nnnn_0010_0010)
{
	missing_op(op,"stc VBR,<REG_N>");
}
//stc SSR,<REG_N>
opcode(i0000_nnnn_0011_0010)
{
	missing_op(op,"stc SSR,<REG_N>");
}
//stc SGR,<REG_N>
opcode(i0000_nnnn_0011_1010)
{
	missing_op(op,"stc SGR,<REG_N>");
}
//stc SPC,<REG_N>
opcode(i0000_nnnn_0100_0010)
{
	missing_op(op,"stc SPC,<REG_N>");
}
//stc R0_BANK,<REG_N>
opcode(i0000_nnnn_1mmm_0010)
{
	missing_op(op,"stc R0_BANK,<REG_N>");
}
//braf <REG_N>
opcode(i0000_nnnn_0010_0011)
{
	missing_op(op,"braf <REG_N>");
}
//bsrf <REG_N>
opcode(i0000_nnnn_0000_0011)
{
	missing_op(op,"bsrf <REG_N>");
}
//movca.l R0, @<REG_N>
opcode(i0000_nnnn_1100_0011)
{
	missing_op(op,"movca.l R0, @<REG_N>");
}
//ocbi @<REG_N>
opcode(i0000_nnnn_1001_0011)
{
	missing_op(op,"ocbi @<REG_N>");
}
//ocbp @<REG_N>
opcode(i0000_nnnn_1010_0011)
{
	missing_op(op,"ocbp @<REG_N>");
}
//ocbwb @<REG_N>
opcode(i0000_nnnn_1011_0011)
{
	missing_op(op,"ocbwb @<REG_N>");
}
//pref @<REG_N>
opcode(i0000_nnnn_1000_0011)
{
	missing_op(op,"pref @<REG_N>");
}
//mov.b <REG_M>,@(R0,<REG_N>)
opcode(i0000_nnnn_mmmm_0100)
{
	missing_op(op,"mov.b <REG_M>,@(R0,<REG_N>)");
}
//mov.w <REG_M>,@(R0,<REG_N>)
opcode(i0000_nnnn_mmmm_0101)
{
	//missing_op(op,"mov.w <REG_M>,@(R0,<REG_N>)");
	u32 n = GetN(op);
	u32 m = GetM(op);
	WriteMemBOU16(r[0] , r[n], r[m]);
}
//mov.l <REG_M>,@(R0,<REG_N>)
opcode(i0000_nnnn_mmmm_0110)
{
	//missing_op(op,"mov.l <REG_M>,@(R0,<REG_N>)");
	u32 n = GetN(op);
	u32 m = GetM(op);
	WriteMemBOU32(r[0], r[n], r[m]);
}
//mul.l <REG_M>,<REG_N>
opcode(i0000_nnnn_mmmm_0111)
{
	//missing_op(op,"mul.l <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	macl = (u32)((((s32)r[n]) * ((s32)r[m])));
}
//clrmac
opcode(i0000_0000_0010_1000)
{
	missing_op(op,"clrmac");
}
//clrs
opcode(i0000_0000_0100_1000)
{
	missing_op(op,"clrs");
}
//clrt
opcode(i0000_0000_0000_1000)
{
	missing_op(op,"clrt");
}
//ldtlb
opcode(i0000_0000_0011_1000)
{
	missing_op(op,"ldtlb");
}
//sets
opcode(i0000_0000_0101_1000)
{
	missing_op(op,"sets");
}
//sett
opcode(i0000_0000_0001_1000)
{
	missing_op(op,"sett");
}
//div0u
opcode(i0000_0000_0001_1001)
{
	missing_op(op,"div0u");
}
//movt <REG_N>
opcode(i0000_nnnn_0010_1001)
{
	missing_op(op,"movt <REG_N>");
}
//nop
opcode(i0000_0000_0000_1001)
{
	missing_op(op,"nop");
}
//sts FPUL,<REG_N>
opcode(i0000_nnnn_0101_1010)
{
	//missing_op(op,"sts FPUL,<REG_N>");
	u32 n = GetN(op);
	r[n] = fpul;
}
//sts FPSCR,<REG_N>
opcode(i0000_nnnn_0110_1010)
{
	missing_op(op,"sts FPSCR,<REG_N>");
}
//stc DBR,<REG_N>
opcode(i0000_nnnn_1111_1010)
{
	missing_op(op,"stc DBR,<REG_N>");
}
//sts MACH,<REG_N>
opcode(i0000_nnnn_0000_1010)
{
	missing_op(op,"sts MACH,<REG_N>");
}
//sts MACL,<REG_N>
opcode(i0000_nnnn_0001_1010)
{
	//missing_op(op,"sts MACL,<REG_N>");
	u32 n = GetN(op);
	r[n]=macl; 
}
//sts PR,<REG_N>
opcode(i0000_nnnn_0010_1010)
{
	missing_op(op,"sts PR,<REG_N>");
}
//rte
opcode(i0000_0000_0010_1011)
{
	missing_op(op,"rte");
}
//rts
opcode(i0000_0000_0000_1011)
{
	missing_op(op,"rts");
}
//sleep
opcode(i0000_0000_0001_1011)
{
	missing_op(op,"sleep");
}
//mov.b @(R0,<REG_M>),<REG_N>
opcode(i0000_nnnn_mmmm_1100)
{
	missing_op(op,"mov.b @(R0,<REG_M>),<REG_N>");
}
//mov.w @(R0,<REG_M>),<REG_N>
opcode(i0000_nnnn_mmmm_1101)
{
	missing_op(op,"mov.w @(R0,<REG_M>),<REG_N>");
}
//mov.l @(R0,<REG_M>),<REG_N>
opcode(i0000_nnnn_mmmm_1110)
{
	missing_op(op,"mov.l @(R0,<REG_M>),<REG_N>");
}
//mac.l @<REG_M>+,@<REG_N>+
opcode(i0000_nnnn_mmmm_1111)
{
	missing_op(op,"mac.l @<REG_M>+,@<REG_N>+");
}
//mov.l <REG_M>,@(<disp4dw>,<REG_N>)
opcode(i0001_nnnn_mmmm_iiii)
{
	missing_op(op,"mov.l <REG_M>,@(<disp4dw>,<REG_N>)");
}
//mov.b <REG_M>,@<REG_N>
opcode(i0010_nnnn_mmmm_0000)
{
	//missing_op(op,"mov.b <REG_M>,@<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	WriteMemU8(r[n],r[m] );
}
//mov.w <REG_M>,@<REG_N>
opcode(i0010_nnnn_mmmm_0001)
{
	missing_op(op,"mov.w <REG_M>,@<REG_N>");
}
//mov.l <REG_M>,@<REG_N>
opcode(i0010_nnnn_mmmm_0010)
{
	//missing_op(op,"mov.l <REG_M>,@<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	WriteMemU32(r[n],r[m]);
}
//mov.b <REG_M>,@-<REG_N>
opcode(i0010_nnnn_mmmm_0100)
{
	missing_op(op,"mov.b <REG_M>,@-<REG_N>");
}
//mov.w <REG_M>,@-<REG_N>
opcode(i0010_nnnn_mmmm_0101)
{
	missing_op(op,"mov.w <REG_M>,@-<REG_N>");
}
//mov.l <REG_M>,@-<REG_N>
opcode(i0010_nnnn_mmmm_0110)
{
	missing_op(op,"mov.l <REG_M>,@-<REG_N>");
}
//div0s <REG_M>,<REG_N>
opcode(i0010_nnnn_mmmm_0111)
{
	missing_op(op,"div0s <REG_M>,<REG_N>");
}
//tst <REG_M>,<REG_N>
opcode(i0010_nnnn_mmmm_1000)
{
	missing_op(op,"tst <REG_M>,<REG_N>");
}
//and <REG_M>,<REG_N>
opcode(i0010_nnnn_mmmm_1001)
{
	//missing_op(op,"and <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] &= r[m];
}
//xor <REG_M>,<REG_N>
opcode(i0010_nnnn_mmmm_1010)
{
	//missing_op(op,"xor <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] ^= r[m];
}
//or <REG_M>,<REG_N>
opcode(i0010_nnnn_mmmm_1011)
{
	//missing_op(op,"or <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);	
	r[n] |= r[m];
}
//cmp/str <REG_M>,<REG_N>
opcode(i0010_nnnn_mmmm_1100)
{
	missing_op(op,"cmp/str <REG_M>,<REG_N>");
}
//xtrct <REG_M>,<REG_N>
opcode(i0010_nnnn_mmmm_1101)
{
	missing_op(op,"xtrct <REG_M>,<REG_N>");
}
//mulu.w <REG_M>,<REG_N>
opcode(i0010_nnnn_mmmm_1110)
{
	missing_op(op,"mulu.w <REG_M>,<REG_N>");
}
//muls.w <REG_M>,<REG_N>
opcode(i0010_nnnn_mmmm_1111)
{
	missing_op(op,"muls.w <REG_M>,<REG_N>");
}
//cmp/eq <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_0000)
{
	missing_op(op,"cmp/eq <REG_M>,<REG_N>");
}
//cmp/hs <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_0010)
{
	missing_op(op,"cmp/hs <REG_M>,<REG_N>");
}
//cmp/ge <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_0011)
{
	missing_op(op,"cmp/ge <REG_M>,<REG_N>");
}
//div1 <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_0100)
{
	missing_op(op,"div1 <REG_M>,<REG_N>");
}
//dmulu.l <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_0101)
{
	missing_op(op,"dmulu.l <REG_M>,<REG_N>");
}
//cmp/hi <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_0110)
{
	missing_op(op,"cmp/hi <REG_M>,<REG_N>");
}
//cmp/gt <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_0111)
{
	missing_op(op,"cmp/gt <REG_M>,<REG_N>");
}
//sub <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_1000)
{
	missing_op(op,"sub <REG_M>,<REG_N>");
}
//subc <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_1010)
{
	missing_op(op,"subc <REG_M>,<REG_N>");
}
//subv <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_1011)
{
	missing_op(op,"subv <REG_M>,<REG_N>");
}
//add <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_1100)
{
	//missing_op(op,"add <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] +=r[m];
}
//dmuls.l <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_1101)
{
	missing_op(op,"dmuls.l <REG_M>,<REG_N>");
}
//addc <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_1110)
{
	missing_op(op,"addc <REG_M>,<REG_N>");
}
//addv <REG_M>,<REG_N>
opcode(i0011_nnnn_mmmm_1111)
{
	missing_op(op,"addv <REG_M>,<REG_N>");
}
//sts.l FPUL,@-<REG_N>
opcode(i0100_nnnn_0101_0010)
{
	missing_op(op,"sts.l FPUL,@-<REG_N>");
}
//sts.l FPSCR,@-<REG_N>
opcode(i0100_nnnn_0110_0010)
{
	missing_op(op,"sts.l FPSCR,@-<REG_N>");
}
//sts.l MACH,@-<REG_N>
opcode(i0100_nnnn_0000_0010)
{
	missing_op(op,"sts.l MACH,@-<REG_N>");
}
//sts.l MACL,@-<REG_N>
opcode(i0100_nnnn_0001_0010)
{
	missing_op(op,"sts.l MACL,@-<REG_N>");
}
//sts.l PR,@-<REG_N>
opcode(i0100_nnnn_0010_0010)
{
	missing_op(op,"sts.l PR,@-<REG_N>");
}
//stc.l DBR,@-<REG_N>
opcode(i0100_nnnn_1111_0010)
{
	missing_op(op,"stc.l DBR,@-<REG_N>");
}
//stc.l SR,@-<REG_N>
opcode(i0100_nnnn_0000_0011)
{
	missing_op(op,"stc.l SR,@-<REG_N>");
}
//stc.l GBR,@-<REG_N>
opcode(i0100_nnnn_0001_0011)
{
	missing_op(op,"stc.l GBR,@-<REG_N>");
}
//stc.l VBR,@-<REG_N>
opcode(i0100_nnnn_0010_0011)
{
	missing_op(op,"stc.l VBR,@-<REG_N>");
}
//stc.l SSR,@-<REG_N>
opcode(i0100_nnnn_0011_0011)
{
	missing_op(op,"stc.l SSR,@-<REG_N>");
}
//stc.l SGR,@-<REG_N>
opcode(i0100_nnnn_0011_0010)
{
	missing_op(op,"stc.l SGR,@-<REG_N>");
}
//stc.l SPC,@-<REG_N>
opcode(i0100_nnnn_0100_0011)
{
	missing_op(op,"stc.l SPC,@-<REG_N>");
}
//stc <RM_BANK>,@-<REG_N>
opcode(i0100_nnnn_1mmm_0011)
{
	missing_op(op,"stc <RM_BANK>,@-<REG_N>");
}
//lds.l @<REG_N>+,MACH
opcode(i0100_nnnn_0000_0110)
{
	missing_op(op,"lds.l @<REG_N>+,MACH");
}
//lds.l @<REG_N>+,MACL
opcode(i0100_nnnn_0001_0110)
{
	missing_op(op,"lds.l @<REG_N>+,MACL");
}
//lds.l @<REG_N>+,PR
opcode(i0100_nnnn_0010_0110)
{
	missing_op(op,"lds.l @<REG_N>+,PR");
}
//lds.l @<REG_N>+,FPUL
opcode(i0100_nnnn_0101_0110)
{
	missing_op(op,"lds.l @<REG_N>+,FPUL");
}
//lds.l @<REG_N>+,FPSCR
opcode(i0100_nnnn_0110_0110)
{
	missing_op(op,"lds.l @<REG_N>+,FPSCR");
}
//ldc.l @<REG_N>+,DBR
opcode(i0100_nnnn_1111_0110)
{
	missing_op(op,"ldc.l @<REG_N>+,DBR");
}
//ldc.l @<REG_N>+,SR
opcode(i0100_nnnn_0000_0111)
{
	missing_op(op,"ldc.l @<REG_N>+,SR");
}
//ldc.l @<REG_N>+,GBR
opcode(i0100_nnnn_0001_0111)
{
	missing_op(op,"ldc.l @<REG_N>+,GBR");
}
//ldc.l @<REG_N>+,VBR
opcode(i0100_nnnn_0010_0111)
{
	missing_op(op,"ldc.l @<REG_N>+,VBR");
}
//ldc.l @<REG_N>+,SSR
opcode(i0100_nnnn_0011_0111)
{
	missing_op(op,"ldc.l @<REG_N>+,SSR");
}
//ldc.l @<REG_N>+,SGR
opcode(i0100_nnnn_0011_0110)
{
	missing_op(op,"ldc.l @<REG_N>+,SGR");
}
//ldc.l @<REG_N>+,SPC
opcode(i0100_nnnn_0100_0111)
{
	missing_op(op,"ldc.l @<REG_N>+,SPC");
}
//ldc.l @<REG_N>+,RM_BANK
opcode(i0100_nnnn_1mmm_0111)
{
	missing_op(op,"ldc.l @<REG_N>+,RM_BANK");
}
//lds <REG_N>,MACH
opcode(i0100_nnnn_0000_1010)
{
	missing_op(op,"lds <REG_N>,MACH");
}
//lds <REG_N>,MACL
opcode(i0100_nnnn_0001_1010)
{
	missing_op(op,"lds <REG_N>,MACL");
}
//lds <REG_N>,PR
opcode(i0100_nnnn_0010_1010)
{
	missing_op(op,"lds <REG_N>,PR");
}
//lds <REG_N>,FPUL
opcode(i0100_nnnn_0101_1010)
{
	//missing_op(op,"lds <REG_N>,FPUL");
	u32 n = GetN(op);
	fpul =r[n];
}
//lds <REG_N>,FPSCR
opcode(i0100_nnnn_0110_1010)
{
	missing_op(op,"lds <REG_N>,FPSCR");
}
//ldc <REG_N>,DBR
opcode(i0100_nnnn_1111_1010)
{
	missing_op(op,"ldc <REG_N>,DBR");
}
//ldc <REG_N>,SR
opcode(i0100_nnnn_0000_1110)
{
	missing_op(op,"ldc <REG_N>,SR");
}
//ldc <REG_N>,GBR
opcode(i0100_nnnn_0001_1110)
{
	missing_op(op,"ldc <REG_N>,GBR");
}
//ldc <REG_N>,VBR
opcode(i0100_nnnn_0010_1110)
{
	missing_op(op,"ldc <REG_N>,VBR");
}
//ldc <REG_N>,SSR
opcode(i0100_nnnn_0011_1110)
{
	missing_op(op,"ldc <REG_N>,SSR");
}
//ldc <REG_N>,SGR
opcode(i0100_nnnn_0011_1010)
{
	missing_op(op,"ldc <REG_N>,SGR");
}
//ldc <REG_N>,SPC
opcode(i0100_nnnn_0100_1110)
{
	missing_op(op,"ldc <REG_N>,SPC");
}
//ldc <REG_N>,<RM_BANK>
opcode(i0100_nnnn_1mmm_1110)
{
	missing_op(op,"ldc <REG_N>,<RM_BANK>");
}
//shll <REG_N>
opcode(i0100_nnnn_0000_0000)
{
	missing_op(op,"shll <REG_N>");
}
//dt <REG_N>
opcode(i0100_nnnn_0001_0000)
{
	u32 n = GetN(op);
	r[n]-=1;
	if (r[n] == 0)
		sr.T=1;
	else
		sr.T=0;
}
//shal <REG_N>
opcode(i0100_nnnn_0010_0000)
{
	missing_op(op,"shal <REG_N>");
}
//shlr <REG_N>
opcode(i0100_nnnn_0000_0001)
{
	//missing_op(op,"shlr <REG_N>");
	u32 n = GetN(op);
	sr.T = r[n] & 0x1;
	r[n] >>= 1;
}
//cmp/pz <REG_N>
opcode(i0100_nnnn_0001_0001)
{
	missing_op(op,"cmp/pz <REG_N>");
}
//shar <REG_N>
opcode(i0100_nnnn_0010_0001)
{
	missing_op(op,"shar <REG_N>");
}
//rotcl <REG_N>
opcode(i0100_nnnn_0010_0100)
{
	missing_op(op,"rotcl <REG_N>");
}
//rotl <REG_N>
opcode(i0100_nnnn_0000_0100)
{
	missing_op(op,"rotl <REG_N>");
}
//cmp/pl <REG_N>
opcode(i0100_nnnn_0001_0101)
{
	missing_op(op,"cmp/pl <REG_N>");
}
//rotcr <REG_N>
opcode(i0100_nnnn_0010_0101)
{
	missing_op(op,"rotcr <REG_N>");
}
//rotr <REG_N>
opcode(i0100_nnnn_0000_0101)
{
	missing_op(op,"rotr <REG_N>");
}
//shll2 <REG_N>
opcode(i0100_nnnn_0000_1000)
{
	//missing_op(op,"shll2 <REG_N>");
	u32 n = GetN(op);
	r[n] <<= 2;
}
//shll8 <REG_N>
opcode(i0100_nnnn_0001_1000)
{
	missing_op(op,"shll8 <REG_N>");
}
//shll16 <REG_N>
opcode(i0100_nnnn_0010_1000)
{
	//missing_op(op,"shll16 <REG_N>");
	u32 n = GetN(op);
	r[n] <<= 16;
}
//shlr2 <REG_N>
opcode(i0100_nnnn_0000_1001)
{
	u32 n = GetN(op);
	r[n] >>= 2;
}
//shlr8 <REG_N>
opcode(i0100_nnnn_0001_1001)
{
	missing_op(op,"shlr8 <REG_N>");
}
//shlr16 <REG_N>
opcode(i0100_nnnn_0010_1001)
{
	//missing_op(op,"shlr16 <REG_N>");
	u32 n = GetN(op);
	r[n] >>= 16;
}
//jmp @<REG_N>
opcode(i0100_nnnn_0010_1011)
{
	missing_op(op,"jmp @<REG_N>");
}
//jsr @<REG_N>
opcode(i0100_nnnn_0000_1011)
{
	missing_op(op,"jsr @<REG_N>");
}
//tas.b @<REG_N>
opcode(i0100_nnnn_0001_1011)
{
	missing_op(op,"tas.b @<REG_N>");
}
//shad <REG_M>,<REG_N>
opcode(i0100_nnnn_mmmm_1100)
{
	missing_op(op,"shad <REG_M>,<REG_N>");
}
//shld <REG_M>,<REG_N>
opcode(i0100_nnnn_mmmm_1101)
{
	missing_op(op,"shld <REG_M>,<REG_N>");
}
//mac.w @<REG_M>+,@<REG_N>+
opcode(i0100_nnnn_mmmm_1111)
{
	missing_op(op,"mac.w @<REG_M>+,@<REG_N>+");
}
//mov.l @(<disp4dw>,<REG_M>),<REG_N>
opcode(i0101_nnnn_mmmm_iiii)
{
	missing_op(op,"mov.l @(<disp4dw>,<REG_M>),<REG_N>");
}
//mov.b @<REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_0000)
{
	//missing_op(op,"mov.b @<REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	
	ReadMemS8(r[n],r[m]);
}
//mov.w @<REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_0001)
{
	missing_op(op,"mov.w @<REG_M>,<REG_N>");
}
//mov.l @<REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_0010)
{
	missing_op(op,"mov.l @<REG_M>,<REG_N>");
}
//mov <REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_0011)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] = r[m];
}
//mov.b @<REG_M>+,<REG_N>
opcode(i0110_nnnn_mmmm_0100)
{
	missing_op(op,"mov.b @<REG_M>+,<REG_N>");
}
//mov.w @<REG_M>+,<REG_N>
opcode(i0110_nnnn_mmmm_0101)
{
	missing_op(op,"mov.w @<REG_M>+,<REG_N>");
}
//mov.l @<REG_M>+,<REG_N>
opcode(i0110_nnnn_mmmm_0110)
{
	missing_op(op,"mov.l @<REG_M>+,<REG_N>");
}
//not <REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_0111)
{
	missing_op(op,"not <REG_M>,<REG_N>");
}
//swap.b <REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_1000)
{
	missing_op(op,"swap.b <REG_M>,<REG_N>");
}
//swap.w <REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_1001)
{
	missing_op(op,"swap.w <REG_M>,<REG_N>");
}
//negc <REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_1010)
{
	missing_op(op,"negc <REG_M>,<REG_N>");
}
//neg <REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_1011)
{
	//missing_op(op,"neg <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] = -r[m];
}
//extu.b <REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_1100)
{
	missing_op(op,"extu.b <REG_M>,<REG_N>");
}
//extu.w <REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_1101)
{
	missing_op(op,"extu.w <REG_M>,<REG_N>");
}
//exts.b <REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_1110)
{
	missing_op(op,"exts.b <REG_M>,<REG_N>");
}
//exts.w <REG_M>,<REG_N>
opcode(i0110_nnnn_mmmm_1111)
{
	missing_op(op,"exts.w <REG_M>,<REG_N>");
}
//add #<simm8>,<REG_N>
opcode(i0111_nnnn_iiii_iiii)
{
	u32 n = GetN(op);
	s32 stmp1 = GetSImm8(op);
	r[n] +=stmp1;
}
//bf <bdisp8>
opcode(i1000_1011_iiii_iiii)
{
	if (sr.T==0)
	{
		//direct jump
		pc = (u32)((GetSImm8(op))*2 + 4 + pc );
		pc-=2;
	}
}
//bf.s <bdisp8>
opcode(i1000_1111_iiii_iiii)
{
	//missing_op(op,"bf.s <bdisp8>");
	if (sr.T==0)
	{
		u32 newpc=(u32) ( (GetSImm8(op)<<1) + pc+2);//pc+2 is done after
		sh4_exec_dslot();
		pc = newpc;
	}
}
//bt <bdisp8>
opcode(i1000_1001_iiii_iiii)
{
	missing_op(op,"bt <bdisp8>");
}
//bt.s <bdisp8>
opcode(i1000_1101_iiii_iiii)
{
	missing_op(op,"bt.s <bdisp8>");
}
//cmp/eq #<simm8hex>,R0
opcode(i1000_1000_iiii_iiii)
{
	missing_op(op,"cmp/eq #<simm8hex>,R0");
}
//mov.b R0,@(<disp4b>,<REG_M>)
opcode(i1000_0000_mmmm_iiii)
{
	missing_op(op,"mov.b R0,@(<disp4b>,<REG_M>)");
}
//mov.w R0,@(<disp4w>,<REG_M>)
opcode(i1000_0001_mmmm_iiii)
{
	missing_op(op,"mov.w R0,@(<disp4w>,<REG_M>)");
}
//mov.b @(<disp4b>,<REG_M>),R0
opcode(i1000_0100_mmmm_iiii)
{
	missing_op(op,"mov.b @(<disp4b>,<REG_M>),R0");
}
//mov.w @(<disp4w>,<REG_M>),R0
opcode(i1000_0101_mmmm_iiii)
{
	missing_op(op,"mov.w @(<disp4w>,<REG_M>),R0");
}
//mov.w @(<PCdisp8w>),<REG_N>
opcode(i1001_nnnn_iiii_iiii)
{
	missing_op(op,"mov.w @(<PCdisp8w>),<REG_N>");
}
//bra <bdisp12>
opcode(i1010_iiii_iiii_iiii)
{
	u32 newpc =(u32) ((  ((s16)((GetImm12(op))<<4)) >>3)  + pc + 4);
	sh4_exec_dslot();
	pc=newpc-2;
}
//bsr <bdisp12>
opcode(i1011_iiii_iiii_iiii)
{
	missing_op(op,"bsr <bdisp12>");
}
//mov.b R0,@(<disp8b>,GBR)
opcode(i1100_0000_iiii_iiii)
{
	missing_op(op,"mov.b R0,@(<disp8b>,GBR)");
}
//mov.w R0,@(<disp8w>,GBR)
opcode(i1100_0001_iiii_iiii)
{
	missing_op(op,"mov.w R0,@(<disp8w>,GBR)");
}
//mov.l R0,@(<disp8dw>,GBR)
opcode(i1100_0010_iiii_iiii)
{
	missing_op(op,"mov.l R0,@(<disp8dw>,GBR)");
}
//trapa #<imm8>
opcode(i1100_0011_iiii_iiii)
{
	missing_op(op,"trapa #<imm8>");
}
//mov.b @(<GBRdisp8b>),R0
opcode(i1100_0100_iiii_iiii)
{
	missing_op(op,"mov.b @(<GBRdisp8b>),R0");
}
//mov.w @(<GBRdisp8w>),R0
opcode(i1100_0101_iiii_iiii)
{
	missing_op(op,"mov.w @(<GBRdisp8w>),R0");
}
//mov.l @(<GBRdisp8dw>),R0
opcode(i1100_0110_iiii_iiii)
{
	missing_op(op,"mov.l @(<GBRdisp8dw>),R0");
}
//mova @(<PCdisp8d>),R0
opcode(i1100_0111_iiii_iiii)
{
	//missing_op(op,"mova @(<PCdisp8d>),R0");
	r[0] = (pc&0xFFFFFFFC)+4+(GetImm8(op)<<2);
}
//tst #<imm8>,R0
opcode(i1100_1000_iiii_iiii)
{
	missing_op(op,"tst #<imm8>,R0");
}
//and #<imm8>,R0
opcode(i1100_1001_iiii_iiii)
{
	missing_op(op,"and #<imm8>,R0");
}
//xor #<imm8>,R0
opcode(i1100_1010_iiii_iiii)
{
	missing_op(op,"xor #<imm8>,R0");
}
//or #<imm8>,R0
opcode(i1100_1011_iiii_iiii)
{
	missing_op(op,"or #<imm8>,R0");
}
//tst.b #<imm8>,@(R0,GBR)
opcode(i1100_1100_iiii_iiii)
{
	missing_op(op,"tst.b #<imm8>,@(R0,GBR)");
}
//and.b #<imm8>,@(R0,GBR)
opcode(i1100_1101_iiii_iiii)
{
	missing_op(op,"and.b #<imm8>,@(R0,GBR)");
}
//xor.b #<imm8>,@(R0,GBR)
opcode(i1100_1110_iiii_iiii)
{
	missing_op(op,"xor.b #<imm8>,@(R0,GBR)");
}
//or.b #<imm8>,@(R0,GBR)
opcode(i1100_1111_iiii_iiii)
{
	missing_op(op,"or.b #<imm8>,@(R0,GBR)");
}
//mov.l @(<PCdisp8d>),<REG_N>
opcode(i1101_nnnn_iiii_iiii)
{
	u32 n = GetN(op);
	u32 disp = (GetImm8(op));
	
	ReadMemU32(r[n],(disp<<2) + (pc & 0xFFFFFFFC) + 4);
}
//mov #<simm8hex>,<REG_N>
opcode(i1110_nnnn_iiii_iiii)
{
	u32 n = GetN(op);
	r[n] = (u32)(s32)(s8)(GetImm8(op));
}
//fadd <FREG_M>,<FREG_N>
opcode(i1111_nnnn_mmmm_0000)
{
	//missing_op(op,"fadd <FREG_M>,<FREG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	fr[n] += fr[m];
}
//fsub <FREG_M>,<FREG_N>
opcode(i1111_nnnn_mmmm_0001)
{
	missing_op(op,"fsub <FREG_M>,<FREG_N>");
}
//fmul <FREG_M>,<FREG_N>
opcode(i1111_nnnn_mmmm_0010)
{
	//missing_op(op,"fmul <FREG_M>,<FREG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	fr[n] *= fr[m];
}
//fdiv <FREG_M>,<FREG_N>
opcode(i1111_nnnn_mmmm_0011)
{
	//missing_op(op,"fdiv <FREG_M>,<FREG_N>");
	//hacked !
	u32 n = GetN(op);
	u32 m = GetM(op);

	fr[n] /= fr[m];
}
//fcmp/eq <FREG_M>,<FREG_N>
opcode(i1111_nnnn_mmmm_0100)
{
	missing_op(op,"fcmp/eq <FREG_M>,<FREG_N>");
}
//fcmp/gt <FREG_M>,<FREG_N>
opcode(i1111_nnnn_mmmm_0101)
{
	missing_op(op,"fcmp/gt <FREG_M>,<FREG_N>");
}
//fmov.s @(R0,<REG_M>),<FREG_N>
opcode(i1111_nnnn_mmmm_0110)
{
	missing_op(op,"fmov.s @(R0,<REG_M>),<FREG_N>");
}
//fmov.s <FREG_M>,@(R0,<REG_N>)
opcode(i1111_nnnn_mmmm_0111)
{
	missing_op(op,"fmov.s <FREG_M>,@(R0,<REG_N>)");
}
//fmov.s @<REG_M>,<FREG_N>
opcode(i1111_nnnn_mmmm_1000)
{
	//missing_op(op,"fmov.s @<REG_M>,<FREG_N> ");
	//hacked !
	u32 n = GetN(op);
	u32 m = GetM(op);
	ReadMemU32(fr_hex[n],r[m]);
}
//fmov.s @<REG_M>+,<FREG_N>
opcode(i1111_nnnn_mmmm_1001)
{
	missing_op(op,"fmov.s @<REG_M>+,<FREG_N>");
}
//fmov.s <FREG_M>,@<REG_N>
opcode(i1111_nnnn_mmmm_1010)
{
	missing_op(op,"fmov.s <FREG_M>,@<REG_N>");
}
//fmov.s <FREG_M>,@-<REG_N>
opcode(i1111_nnnn_mmmm_1011)
{
	missing_op(op,"fmov.s <FREG_M>,@-<REG_N>");
}
//fmov <FREG_M>,<FREG_N>
opcode(i1111_nnnn_mmmm_1100)
{
	missing_op(op,"fmov <FREG_M>,<FREG_N>");
}
//fabs <FREG_N>
opcode(i1111_nnnn_0101_1101)
{
	missing_op(op,"fabs <FREG_N>");
}

#include <math.h>
//FSCA FPUL, DRn
opcode(i1111_nnn0_1111_1101)
{
	#define sh4_pi 3.141592653589793f
	//missing_op(op,"FSCA FPUL, DRn");
	//this is hacked
	int n=GetN(op) & 0xE;
	 
	float real_pi=(((float)(s32)fpul)/65536)*(2*sh4_pi);

	fr[n | 0] = sinf(real_pi);
	fr[n | 1] = cosf(real_pi);
}

//fcnvds <DR_N>,FPUL
opcode(i1111_nnnn_1011_1101)
{
	missing_op(op,"fcnvds <DR_N>,FPUL");
}
//fcnvsd FPUL,<DR_N>
opcode(i1111_nnnn_1010_1101)
{
	missing_op(op,"fcnvsd FPUL,<DR_N>");
}
//fipr <FV_M>,<FV_N>
opcode(i1111_nnmm_1110_1101)
{
	missing_op(op,"fipr <FV_M>,<FV_N>");
}
//fldi0 <FREG_N>
opcode(i1111_nnnn_1000_1101)
{
	missing_op(op,"fldi0 <FREG_N>");
}
//fldi1 <FREG_N>
opcode(i1111_nnnn_1001_1101)
{
	missing_op(op,"fldi1 <FREG_N>");
}
//flds <FREG_N>,FPUL
opcode(i1111_nnnn_0001_1101)
{
	missing_op(op,"flds <FREG_N>,FPUL");
}
//float FPUL,<FREG_N>
opcode(i1111_nnnn_0010_1101)
{
	//missing_op(op,"float FPUL,<FREG_N>");
	//hacked ;)
	u32 n = GetN(op);
	fr[n] = (float)(int)fpul;
}
//fneg <FREG_N>
opcode(i1111_nnnn_0100_1101)
{
	missing_op(op,"fneg <FREG_N>");
}
//frchg
opcode(i1111_1011_1111_1101)
{
	missing_op(op,"frchg");
}
//fschg
opcode(i1111_0011_1111_1101)
{
	missing_op(op,"fschg");
}
//fsqrt <FREG_N>
opcode(i1111_nnnn_0110_1101)
{
	missing_op(op,"fsqrt <FREG_N>");
}
//ftrc <FREG_N>, FPUL
opcode(i1111_nnnn_0011_1101)
{
	//missing_op(op,"ftrc <FREG_N>, FPUL");
	//hacked !
	u32 n = GetN(op);
	fpul = (u32)(s32)fr[n];

	if (fpul==0x80000000)
	{
		if (fr[n]>0)
			fpul--;
	}
}
//fsts FPUL,<FREG_N>
opcode(i1111_nnnn_0000_1101)
{
	//missing_op(op,"fsts FPUL,<FREG_N>");
	u32 n = GetN(op);
	fr_hex[n] = fpul;
}
//ftrv xmtrx,<FV_N>
opcode(i1111_nn01_1111_1101)
{
	missing_op(op,"ftrv xmtrx,<FV_N>");
}
//fmac <FREG_0>,<FREG_M>,<FREG_N>
opcode(i1111_nnnn_mmmm_1110)
{
	missing_op(op,"fmac <FREG_0>,<FREG_M>,<FREG_N>");
}
//FSRRA <FREG_N> (1111nnnn 01111101)
opcode(i1111_nnnn_0111_1101)
{
	missing_op(op,"FSRRA <FREG_N> (1111nnnn 01111101)");
}
