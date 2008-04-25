case 0x0:
	//0xF000 0x0000 -> 0xF00F
	switch(opcode&0xF)
	{
		//0x0 -> Not existing
		//0x1 -> Not existing
	case 0x2:
		//0x0002 0x0002 -> 0xF08F
		missing_op(opcode);
		/*switch(~opcode~)
		{
		default:
		invalid_op(opcode);
		} */
		break;
	case 0x3:
		//0xF00F 0x0003 -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0x0003 i0000_nnnn_0000_0011 bsrf <REG_N>
			call_opcode(i0000_nnnn_0000_0011);
			break;
			//0x1 -> Not existing
		case 0x0002:
			//0xF0FF 0x0023 i0000_nnnn_0010_0011 braf <REG_N>
			call_opcode(i0000_nnnn_0010_0011);
			break;
			//0x3 -> Not existing
			//0x4 -> Not existing
			//0x5 -> Not existing
			//0x6 -> Not existing
			//0x7 -> Not existing
		case 0x0008:
			//0xF0FF 0x0083 i0000_nnnn_1000_0011 pref @<REG_N>
			call_opcode(i0000_nnnn_1000_0011);
			break;
		case 0x0009:
			//0xF0FF 0x0093 i0000_nnnn_1001_0011 ocbi @<REG_N>
			call_opcode(i0000_nnnn_1001_0011);
			break;
		case 0x000A:
			//0xF0FF 0x00A3 i0000_nnnn_1010_0011 ocbp @<REG_N>
			call_opcode(i0000_nnnn_1010_0011);
			break;
		case 0x000B:
			//0xF0FF 0x00B3 i0000_nnnn_1011_0011 ocbwb @<REG_N>
			call_opcode(i0000_nnnn_1011_0011);
			break;
		case 0x000C:
			//0xF0FF 0x00C3 i0000_nnnn_1100_0011 movca.l R0, @<REG_N>
			call_opcode(i0000_nnnn_1100_0011);
			break;
			//0xD -> Not existing
			//0xE -> Not existing
			//0xF -> Not existing
		default:
			invalid_op(opcode);
		}
		break;
	case 0x0004:
		//0xF00F 0x0004 i0000_nnnn_mmmm_0100 mov.b <REG_M>,@(R0,<REG_N>)
		call_opcode(i0000_nnnn_mmmm_0100);
		break;
	case 0x0005:
		//0xF00F 0x0005 i0000_nnnn_mmmm_0101 mov.w <REG_M>,@(R0,<REG_N>)
		call_opcode(i0000_nnnn_mmmm_0101);
		break;
	case 0x0006:
		//0xF00F 0x0006 i0000_nnnn_mmmm_0110 mov.l <REG_M>,@(R0,<REG_N>)
		call_opcode(i0000_nnnn_mmmm_0110);
		break;
	case 0x0007:
		//0xF00F 0x0007 i0000_nnnn_mmmm_0111 mul.l <REG_M>,<REG_N>
		call_opcode(i0000_nnnn_mmmm_0111);
		break;
	case 0x8:
		//0x0008 0x0008 -> 0xFFFF
		missing_op(opcode);
		/*switch(~opcode~)
		{
		default:
		invalid_op(opcode);
		} */
		break;
	case 0x9:
		//0xF00F 0x0009 -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0:
			//0xF0FF 0x0009 -> 0xFFFF
			if (0x0009!=opcode)
				missing_op(opcode);
			/*else
			do nothing, its THE nop !!!!11
			*/
			break;
		case 0x1:
			//0xF0FF 0x0019 -> 0xFFFF
			if (0x0019==opcode)
				call_opcode(i0000_0000_0001_1001);
			else
				missing_op(opcode);
			break;
		case 0x0002:
			//0xF0FF 0x0029 i0000_nnnn_0010_1001 movt <REG_N>
			call_opcode(i0000_nnnn_0010_1001);
			break;
			//0x3 -> Not existing
			//0x4 -> Not existing
			//0x5 -> Not existing
			//0x6 -> Not existing
			//0x7 -> Not existing
			//0x8 -> Not existing
			//0x9 -> Not existing
			//0xA -> Not existing
			//0xB -> Not existing
			//0xC -> Not existing
			//0xD -> Not existing
			//0xE -> Not existing
			//0xF -> Not existing
		default:
			invalid_op(opcode);
		}
		break;
	case 0xA:
		//0xF00F 0x000A -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0x000A i0000_nnnn_0000_1010 sts MACH,<REG_N>
			call_opcode(i0000_nnnn_0000_1010);
			break;
		case 0x0001:
			//0xF0FF 0x001A i0000_nnnn_0001_1010 sts MACL,<REG_N>
			call_opcode(i0000_nnnn_0001_1010);
			break;
		case 0x0002:
			//0xF0FF 0x002A i0000_nnnn_0010_1010 sts PR,<REG_N>
			call_opcode(i0000_nnnn_0010_1010);
			break;
		case 0x0003:
			//0xF0FF 0x003A i0000_nnnn_0011_1010 stc SGR,<REG_N>
			call_opcode(i0000_nnnn_0011_1010);
			break;
			//0x4 -> Not existing
		case 0x0005:
			//0xF0FF 0x005A i0000_nnnn_0101_1010 sts FPUL,<REG_N>
			call_opcode(i0000_nnnn_0101_1010);
			break;
		case 0x0006:
			//0xF0FF 0x006A i0000_nnnn_0110_1010 sts FPSCR,<REG_N>
			call_opcode(i0000_nnnn_0110_1010);
			break;
			//0x7 -> Not existing
			//0x8 -> Not existing
			//0x9 -> Not existing
			//0xA -> Not existing
			//0xB -> Not existing
			//0xC -> Not existing
			//0xD -> Not existing
			//0xE -> Not existing
		case 0x000F:
			//0xF0FF 0x00FA i0000_nnnn_1111_1010 stc DBR,<REG_N>
			call_opcode(i0000_nnnn_1111_1010);
			break;
		default:
			invalid_op(opcode);
		}
		break;
	case 0xB:
		//0x000B 0x000B -> 0xFFFF
		//missing_op(opcode);
		if (opcode==0xb)
			call_opcode(i0000_0000_0000_1011);
		else
			invalid_op(opcode);
		
		break;
	case 0x000C:
		//0xF00F 0x000C i0000_nnnn_mmmm_1100 mov.b @(R0,<REG_M>),<REG_N>
		call_opcode(i0000_nnnn_mmmm_1100);
		break;
	case 0x000D:
		//0xF00F 0x000D i0000_nnnn_mmmm_1101 mov.w @(R0,<REG_M>),<REG_N>
		call_opcode(i0000_nnnn_mmmm_1101);
		break;
	case 0x000E:
		//0xF00F 0x000E i0000_nnnn_mmmm_1110 mov.l @(R0,<REG_M>),<REG_N>
		call_opcode(i0000_nnnn_mmmm_1110);
		break;
	case 0x000F:
		//0xF00F 0x000F i0000_nnnn_mmmm_1111 mac.l @<REG_M>+,@<REG_N>+
		call_opcode(i0000_nnnn_mmmm_1111);
		break;
	default:
		invalid_op(opcode);
	}
	break;
case 0x0001:
	//0xF000 0x1000 i0001_nnnn_mmmm_iiii mov.l <REG_M>,@(<disp4dw>,<REG_N>)
	call_opcode(i0001_nnnn_mmmm_iiii);
	break;
case 0x2:
	//0xF000 0x2000 -> 0xF00F
	switch(opcode&0xF)
	{
	case 0x0000:
		//0xF00F 0x2000 i0010_nnnn_mmmm_0000 mov.b <REG_M>,@<REG_N>
		call_opcode(i0010_nnnn_mmmm_0000);
		break;
	case 0x0001:
		//0xF00F 0x2001 i0010_nnnn_mmmm_0001 mov.w <REG_M>,@<REG_N>
		call_opcode(i0010_nnnn_mmmm_0001);
		break;
	case 0x0002:
		//0xF00F 0x2002 i0010_nnnn_mmmm_0010 mov.l <REG_M>,@<REG_N>
		call_opcode(i0010_nnnn_mmmm_0010);
		break;
		//0x3 -> Not existing
	case 0x0004:
		//0xF00F 0x2004 i0010_nnnn_mmmm_0100 mov.b <REG_M>,@-<REG_N>
		call_opcode(i0010_nnnn_mmmm_0100);
		break;
	case 0x0005:
		//0xF00F 0x2005 i0010_nnnn_mmmm_0101 mov.w <REG_M>,@-<REG_N>
		call_opcode(i0010_nnnn_mmmm_0101);
		break;
	case 0x0006:
		//0xF00F 0x2006 i0010_nnnn_mmmm_0110 mov.l <REG_M>,@-<REG_N>
		call_opcode(i0010_nnnn_mmmm_0110);
		break;
	case 0x0007:
		//0xF00F 0x2007 i0010_nnnn_mmmm_0111 div0s <REG_M>,<REG_N>
		call_opcode(i0010_nnnn_mmmm_0111);
		break;
	case 0x0008:
		//0xF00F 0x2008 i0010_nnnn_mmmm_1000 tst <REG_M>,<REG_N>
		call_opcode(i0010_nnnn_mmmm_1000);
		break;
	case 0x0009:
		//0xF00F 0x2009 i0010_nnnn_mmmm_1001 and <REG_M>,<REG_N>
		call_opcode(i0010_nnnn_mmmm_1001);
		break;
	case 0x000A:
		//0xF00F 0x200A i0010_nnnn_mmmm_1010 xor <REG_M>,<REG_N>
		call_opcode(i0010_nnnn_mmmm_1010);
		break;
	case 0x000B:
		//0xF00F 0x200B i0010_nnnn_mmmm_1011 or <REG_M>,<REG_N>
		call_opcode(i0010_nnnn_mmmm_1011);
		break;
	case 0x000C:
		//0xF00F 0x200C i0010_nnnn_mmmm_1100 cmp/str <REG_M>,<REG_N>
		call_opcode(i0010_nnnn_mmmm_1100);
		break;
	case 0x000D:
		//0xF00F 0x200D i0010_nnnn_mmmm_1101 xtrct <REG_M>,<REG_N>
		call_opcode(i0010_nnnn_mmmm_1101);
		break;
	case 0x000E:
		//0xF00F 0x200E i0010_nnnn_mmmm_1110 mulu.w <REG_M>,<REG_N>
		call_opcode(i0010_nnnn_mmmm_1110);
		break;
	case 0x000F:
		//0xF00F 0x200F i0010_nnnn_mmmm_1111 muls.w <REG_M>,<REG_N>
		call_opcode(i0010_nnnn_mmmm_1111);
		break;
	default:
		invalid_op(opcode);
	}
	break;
case 0x3:
	//0xF000 0x3000 -> 0xF00F
	switch(opcode&0xF)
	{
	case 0x0000:
		//0xF00F 0x3000 i0011_nnnn_mmmm_0000 cmp/eq <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_0000);
		break;
		//0x1 -> Not existing
	case 0x0002:
		//0xF00F 0x3002 i0011_nnnn_mmmm_0010 cmp/hs <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_0010);
		break;
	case 0x0003:
		//0xF00F 0x3003 i0011_nnnn_mmmm_0011 cmp/ge <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_0011);
		break;
	case 0x0004:
		//0xF00F 0x3004 i0011_nnnn_mmmm_0100 div1 <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_0100);
		break;
	case 0x0005:
		//0xF00F 0x3005 i0011_nnnn_mmmm_0101 dmulu.l <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_0101);
		break;
	case 0x0006:
		//0xF00F 0x3006 i0011_nnnn_mmmm_0110 cmp/hi <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_0110);
		break;
	case 0x0007:
		//0xF00F 0x3007 i0011_nnnn_mmmm_0111 cmp/gt <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_0111);
		break;
	case 0x0008:
		//0xF00F 0x3008 i0011_nnnn_mmmm_1000 sub <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_1000);
		break;
		//0x9 -> Not existing
	case 0x000A:
		//0xF00F 0x300A i0011_nnnn_mmmm_1010 subc <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_1010);
		break;
	case 0x000B:
		//0xF00F 0x300B i0011_nnnn_mmmm_1011 subv <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_1011);
		break;
	case 0x000C:
		//0xF00F 0x300C i0011_nnnn_mmmm_1100 add <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_1100);
		break;
	case 0x000D:
		//0xF00F 0x300D i0011_nnnn_mmmm_1101 dmuls.l <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_1101);
		break;
	case 0x000E:
		//0xF00F 0x300E i0011_nnnn_mmmm_1110 addc <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_1110);
		break;
	case 0x000F:
		//0xF00F 0x300F i0011_nnnn_mmmm_1111 addv <REG_M>,<REG_N>
		call_opcode(i0011_nnnn_mmmm_1111);
		break;
	default:
		invalid_op(opcode);
	}
	break;
case 0x4:
	//0xF000 0x4000 -> 0xF00F
	switch(opcode&0xF)
	{
	case 0x0:
		//0xF00F 0x4000 -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0x4000 i0100_nnnn_0000_0000 shll <REG_N>
			call_opcode(i0100_nnnn_0000_0000);
			break;
		case 0x0001:
			//0xF0FF 0x4010 i0100_nnnn_0001_0000 dt <REG_N>
			call_opcode(i0100_nnnn_0001_0000);
			break;
		case 0x0002:
			//0xF0FF 0x4020 i0100_nnnn_0010_0000 shal <REG_N>
			call_opcode(i0100_nnnn_0010_0000);
			break;
			//0x3 -> Not existing
			//0x4 -> Not existing
			//0x5 -> Not existing
			//0x6 -> Not existing
			//0x7 -> Not existing
			//0x8 -> Not existing
			//0x9 -> Not existing
			//0xA -> Not existing
			//0xB -> Not existing
			//0xC -> Not existing
			//0xD -> Not existing
			//0xE -> Not existing
			//0xF -> Not existing
		default:
			invalid_op(opcode);
		}
		break;
	case 0x1:
		//0xF00F 0x4001 -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0x4001 i0100_nnnn_0000_0001 shlr <REG_N>
			call_opcode(i0100_nnnn_0000_0001);
			break;
		case 0x0001:
			//0xF0FF 0x4011 i0100_nnnn_0001_0001 cmp/pz <REG_N>
			call_opcode(i0100_nnnn_0001_0001);
			break;
		case 0x0002:
			//0xF0FF 0x4021 i0100_nnnn_0010_0001 shar <REG_N>
			call_opcode(i0100_nnnn_0010_0001);
			break;
			//0x3 -> Not existing
			//0x4 -> Not existing
			//0x5 -> Not existing
			//0x6 -> Not existing
			//0x7 -> Not existing
			//0x8 -> Not existing
			//0x9 -> Not existing
			//0xA -> Not existing
			//0xB -> Not existing
			//0xC -> Not existing
			//0xD -> Not existing
			//0xE -> Not existing
			//0xF -> Not existing
		default:
			invalid_op(opcode);
		}
		break;
	case 0x2:
		//0xF00F 0x4002 -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0x4002 i0100_nnnn_0000_0010 sts.l MACH,@-<REG_N>
			call_opcode(i0100_nnnn_0000_0010);
			break;
		case 0x0001:
			//0xF0FF 0x4012 i0100_nnnn_0001_0010 sts.l MACL,@-<REG_N>
			call_opcode(i0100_nnnn_0001_0010);
			break;
		case 0x0002:
			//0xF0FF 0x4022 i0100_nnnn_0010_0010 sts.l PR,@-<REG_N>
			call_opcode(i0100_nnnn_0010_0010);
			break;
		case 0x0003:
			//0xF0FF 0x4032 i0100_nnnn_0011_0010 stc.l SGR,@-<REG_N>
			call_opcode(i0100_nnnn_0011_0010);
			break;
			//0x4 -> Not existing
		case 0x0005:
			//0xF0FF 0x4052 i0100_nnnn_0101_0010 sts.l FPUL,@-<REG_N>
			call_opcode(i0100_nnnn_0101_0010);
			break;
		case 0x0006:
			//0xF0FF 0x4062 i0100_nnnn_0110_0010 sts.l FPSCR,@-<REG_N>
			call_opcode(i0100_nnnn_0110_0010);
			break;
			//0x7 -> Not existing
			//0x8 -> Not existing
			//0x9 -> Not existing
			//0xA -> Not existing
			//0xB -> Not existing
			//0xC -> Not existing
			//0xD -> Not existing
			//0xE -> Not existing
		case 0x000F:
			//0xF0FF 0x40F2 i0100_nnnn_1111_0010 stc.l DBR,@-<REG_N>
			call_opcode(i0100_nnnn_1111_0010);
			break;
		default:
			invalid_op(opcode);
		}
		break;
	case 0x3:
		//0x4003 0x0003 -> 0xF08F
		missing_op(opcode);
		/*switch(~opcode~)
		{
		default:
		invalid_op(opcode);
		} */
		break;
	case 0x4:
		//0xF00F 0x4004 -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0x4004 i0100_nnnn_0000_0100 rotl <REG_N>
			call_opcode(i0100_nnnn_0000_0100);
			break;
			//0x1 -> Not existing
		case 0x0002:
			//0xF0FF 0x4024 i0100_nnnn_0010_0100 rotcl <REG_N>
			call_opcode(i0100_nnnn_0010_0100);
			break;
			//0x3 -> Not existing
			//0x4 -> Not existing
			//0x5 -> Not existing
			//0x6 -> Not existing
			//0x7 -> Not existing
			//0x8 -> Not existing
			//0x9 -> Not existing
			//0xA -> Not existing
			//0xB -> Not existing
			//0xC -> Not existing
			//0xD -> Not existing
			//0xE -> Not existing
			//0xF -> Not existing
		default:
			invalid_op(opcode);
		}
		break;
	case 0x5:
		//0xF00F 0x4005 -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0x4005 i0100_nnnn_0000_0101 rotr <REG_N>
			call_opcode(i0100_nnnn_0000_0101);
			break;
		case 0x0001:
			//0xF0FF 0x4015 i0100_nnnn_0001_0101 cmp/pl <REG_N>
			call_opcode(i0100_nnnn_0001_0101);
			break;
		case 0x0002:
			//0xF0FF 0x4025 i0100_nnnn_0010_0101 rotcr <REG_N>
			call_opcode(i0100_nnnn_0010_0101);
			break;
			//0x3 -> Not existing
			//0x4 -> Not existing
			//0x5 -> Not existing
			//0x6 -> Not existing
			//0x7 -> Not existing
			//0x8 -> Not existing
			//0x9 -> Not existing
			//0xA -> Not existing
			//0xB -> Not existing
			//0xC -> Not existing
			//0xD -> Not existing
			//0xE -> Not existing
			//0xF -> Not existing
		default:
			invalid_op(opcode);
		}
		break;
	case 0x6:
		//0xF00F 0x4006 -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0x4006 i0100_nnnn_0000_0110 lds.l @<REG_N>+,MACH
			call_opcode(i0100_nnnn_0000_0110);
			break;
		case 0x0001:
			//0xF0FF 0x4016 i0100_nnnn_0001_0110 lds.l @<REG_N>+,MACL
			call_opcode(i0100_nnnn_0001_0110);
			break;
		case 0x0002:
			//0xF0FF 0x4026 i0100_nnnn_0010_0110 lds.l @<REG_N>+,PR
			call_opcode(i0100_nnnn_0010_0110);
			break;
		case 0x0003:
			//0xF0FF 0x4036 i0100_nnnn_0011_0110 ldc.l @<REG_N>+,SGR
			call_opcode(i0100_nnnn_0011_0110);
			break;
			//0x4 -> Not existing
		case 0x0005:
			//0xF0FF 0x4056 i0100_nnnn_0101_0110 lds.l @<REG_N>+,FPUL
			call_opcode(i0100_nnnn_0101_0110);
			break;
		case 0x0006:
			//0xF0FF 0x4066 i0100_nnnn_0110_0110 lds.l @<REG_N>+,FPSCR
			call_opcode(i0100_nnnn_0110_0110);
			break;
			//0x7 -> Not existing
			//0x8 -> Not existing
			//0x9 -> Not existing
			//0xA -> Not existing
			//0xB -> Not existing
			//0xC -> Not existing
			//0xD -> Not existing
			//0xE -> Not existing
		case 0x000F:
			//0xF0FF 0x40F6 i0100_nnnn_1111_0110 ldc.l @<REG_N>+,DBR
			call_opcode(i0100_nnnn_1111_0110);
			break;
		default:
			invalid_op(opcode);
		}
		break;
	case 0x7:
		//0x4007 0x0007 -> 0xF08F
		missing_op(opcode);
		/*switch(~opcode~)
		{
		default:
		invalid_op(opcode);
		} */
		break;
	case 0x8:
		//0xF00F 0x4008 -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0x4008 i0100_nnnn_0000_1000 shll2 <REG_N>
			call_opcode(i0100_nnnn_0000_1000);
			break;
		case 0x0001:
			//0xF0FF 0x4018 i0100_nnnn_0001_1000 shll8 <REG_N>
			call_opcode(i0100_nnnn_0001_1000);
			break;
		case 0x0002:
			//0xF0FF 0x4028 i0100_nnnn_0010_1000 shll16 <REG_N>
			call_opcode(i0100_nnnn_0010_1000);
			break;
			//0x3 -> Not existing
			//0x4 -> Not existing
			//0x5 -> Not existing
			//0x6 -> Not existing
			//0x7 -> Not existing
			//0x8 -> Not existing
			//0x9 -> Not existing
			//0xA -> Not existing
			//0xB -> Not existing
			//0xC -> Not existing
			//0xD -> Not existing
			//0xE -> Not existing
			//0xF -> Not existing
		default:
			invalid_op(opcode);
		}
		break;
	case 0x9:
		//0xF00F 0x4009 -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0x4009 i0100_nnnn_0000_1001 shlr2 <REG_N>
			call_opcode(i0100_nnnn_0000_1001);
			break;
		case 0x0001:
			//0xF0FF 0x4019 i0100_nnnn_0001_1001 shlr8 <REG_N>
			call_opcode(i0100_nnnn_0001_1001);
			break;
		case 0x0002:
			//0xF0FF 0x4029 i0100_nnnn_0010_1001 shlr16 <REG_N>
			call_opcode(i0100_nnnn_0010_1001);
			break;
			//0x3 -> Not existing
			//0x4 -> Not existing
			//0x5 -> Not existing
			//0x6 -> Not existing
			//0x7 -> Not existing
			//0x8 -> Not existing
			//0x9 -> Not existing
			//0xA -> Not existing
			//0xB -> Not existing
			//0xC -> Not existing
			//0xD -> Not existing
			//0xE -> Not existing
			//0xF -> Not existing
		default:
			invalid_op(opcode);
		}
		break;
	case 0xA:
		//0xF00F 0x400A -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0x400A i0100_nnnn_0000_1010 lds <REG_N>,MACH
			call_opcode(i0100_nnnn_0000_1010);
			break;
		case 0x0001:
			//0xF0FF 0x401A i0100_nnnn_0001_1010 lds <REG_N>,MACL
			call_opcode(i0100_nnnn_0001_1010);
			break;
		case 0x0002:
			//0xF0FF 0x402A i0100_nnnn_0010_1010 lds <REG_N>,PR
			call_opcode(i0100_nnnn_0010_1010);
			break;
		case 0x0003:
			//0xF0FF 0x403A i0100_nnnn_0011_1010 ldc <REG_N>,SGR
			call_opcode(i0100_nnnn_0011_1010);
			break;
			//0x4 -> Not existing
		case 0x0005:
			//0xF0FF 0x405A i0100_nnnn_0101_1010 lds <REG_N>,FPUL
			call_opcode(i0100_nnnn_0101_1010);
			break;
		case 0x0006:
			//0xF0FF 0x406A i0100_nnnn_0110_1010 lds <REG_N>,FPSCR
			call_opcode(i0100_nnnn_0110_1010);
			break;
			//0x7 -> Not existing
			//0x8 -> Not existing
			//0x9 -> Not existing
			//0xA -> Not existing
			//0xB -> Not existing
			//0xC -> Not existing
			//0xD -> Not existing
			//0xE -> Not existing
		case 0x000F:
			//0xF0FF 0x40FA i0100_nnnn_1111_1010 ldc <REG_N>,DBR
			call_opcode(i0100_nnnn_1111_1010);
			break;
		default:
			invalid_op(opcode);
		}
		break;
	case 0xB:
		//0xF00F 0x400B -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0x400B i0100_nnnn_0000_1011 jsr @<REG_N>
			call_opcode(i0100_nnnn_0000_1011);
			break;
		case 0x0001:
			//0xF0FF 0x401B i0100_nnnn_0001_1011 tas.b @<REG_N>
			call_opcode(i0100_nnnn_0001_1011);
			break;
		case 0x0002:
			//0xF0FF 0x402B i0100_nnnn_0010_1011 jmp @<REG_N>
			call_opcode(i0100_nnnn_0010_1011);
			break;
			//0x3 -> Not existing
			//0x4 -> Not existing
			//0x5 -> Not existing
			//0x6 -> Not existing
			//0x7 -> Not existing
			//0x8 -> Not existing
			//0x9 -> Not existing
			//0xA -> Not existing
			//0xB -> Not existing
			//0xC -> Not existing
			//0xD -> Not existing
			//0xE -> Not existing
			//0xF -> Not existing
		default:
			invalid_op(opcode);
		}
		break;
	case 0x000C:
		//0xF00F 0x400C i0100_nnnn_mmmm_1100 shad <REG_M>,<REG_N>
		call_opcode(i0100_nnnn_mmmm_1100);
		break;
	case 0x000D:
		//0xF00F 0x400D i0100_nnnn_mmmm_1101 shld <REG_M>,<REG_N>
		call_opcode(i0100_nnnn_mmmm_1101);
		break;
	case 0xE:
		//0x400E 0x000E -> 0xF08F
		missing_op(opcode);
		/*switch(~opcode~)
		{
		default:
		invalid_op(opcode);
		} */
		break;
	case 0x000F:
		//0xF00F 0x400F i0100_nnnn_mmmm_1111 mac.w @<REG_M>+,@<REG_N>+
		call_opcode(i0100_nnnn_mmmm_1111);
		break;
	default:
		invalid_op(opcode);
	}
	break;
case 0x0005:
	//0xF000 0x5000 i0101_nnnn_mmmm_iiii mov.l @(<disp4dw>,<REG_M>),<REG_N>
	call_opcode(i0101_nnnn_mmmm_iiii);
	break;
case 0x6:
	//0xF000 0x6000 -> 0xF00F
	switch(opcode&0xF)
	{
	case 0x0000:
		//0xF00F 0x6000 i0110_nnnn_mmmm_0000 mov.b @<REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_0000);
		break;
	case 0x0001:
		//0xF00F 0x6001 i0110_nnnn_mmmm_0001 mov.w @<REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_0001);
		break;
	case 0x0002:
		//0xF00F 0x6002 i0110_nnnn_mmmm_0010 mov.l @<REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_0010);
		break;
	case 0x0003:
		//0xF00F 0x6003 i0110_nnnn_mmmm_0011 mov <REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_0011);
		break;
	case 0x0004:
		//0xF00F 0x6004 i0110_nnnn_mmmm_0100 mov.b @<REG_M>+,<REG_N>
		call_opcode(i0110_nnnn_mmmm_0100);
		break;
	case 0x0005:
		//0xF00F 0x6005 i0110_nnnn_mmmm_0101 mov.w @<REG_M>+,<REG_N>
		call_opcode(i0110_nnnn_mmmm_0101);
		break;
	case 0x0006:
		//0xF00F 0x6006 i0110_nnnn_mmmm_0110 mov.l @<REG_M>+,<REG_N>
		call_opcode(i0110_nnnn_mmmm_0110);
		break;
	case 0x0007:
		//0xF00F 0x6007 i0110_nnnn_mmmm_0111 not <REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_0111);
		break;
	case 0x0008:
		//0xF00F 0x6008 i0110_nnnn_mmmm_1000 swap.b <REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_1000);
		break;
	case 0x0009:
		//0xF00F 0x6009 i0110_nnnn_mmmm_1001 swap.w <REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_1001);
		break;
	case 0x000A:
		//0xF00F 0x600A i0110_nnnn_mmmm_1010 negc <REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_1010);
		break;
	case 0x000B:
		//0xF00F 0x600B i0110_nnnn_mmmm_1011 neg <REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_1011);
		break;
	case 0x000C:
		//0xF00F 0x600C i0110_nnnn_mmmm_1100 extu.b <REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_1100);
		break;
	case 0x000D:
		//0xF00F 0x600D i0110_nnnn_mmmm_1101 extu.w <REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_1101);
		break;
	case 0x000E:
		//0xF00F 0x600E i0110_nnnn_mmmm_1110 exts.b <REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_1110);
		break;
	case 0x000F:
		//0xF00F 0x600F i0110_nnnn_mmmm_1111 exts.w <REG_M>,<REG_N>
		call_opcode(i0110_nnnn_mmmm_1111);
		break;
	default:
		invalid_op(opcode);
	}
	break;
case 0x0007:
	//0xF000 0x7000 i0111_nnnn_iiii_iiii add #<simm8>,<REG_N>
	call_opcode(i0111_nnnn_iiii_iiii);
	break;
case 0x8:
	//0xF000 0x8000 -> 0xFF00
	switch((opcode>>8)&0xF)
	{
	case 0x0000:
		//0xFF00 0x8000 i1000_0000_mmmm_iiii mov.b R0,@(<disp4b>,<REG_M>)
		call_opcode(i1000_0000_mmmm_iiii);
		break;
	case 0x0001:
		//0xFF00 0x8100 i1000_0001_mmmm_iiii mov.w R0,@(<disp4w>,<REG_M>)
		call_opcode(i1000_0001_mmmm_iiii);
		break;
		//0x2 -> Not existing
		//0x3 -> Not existing
	case 0x0004:
		//0xFF00 0x8400 i1000_0100_mmmm_iiii mov.b @(<disp4b>,<REG_M>),R0
		call_opcode(i1000_0100_mmmm_iiii);
		break;
	case 0x0005:
		//0xFF00 0x8500 i1000_0101_mmmm_iiii mov.w @(<disp4w>,<REG_M>),R0
		call_opcode(i1000_0101_mmmm_iiii);
		break;
		//0x6 -> Not existing
		//0x7 -> Not existing
	case 0x0008:
		//0xFF00 0x8800 i1000_1000_iiii_iiii cmp/eq #<simm8hex>,R0
		call_opcode(i1000_1000_iiii_iiii);
		break;
	case 0x0009:
		//0xFF00 0x8900 i1000_1001_iiii_iiii bt <bdisp8>
		call_opcode(i1000_1001_iiii_iiii);
		break;
		//0xA -> Not existing
	case 0x000B:
		//0xFF00 0x8B00 i1000_1011_iiii_iiii bf <bdisp8>
		call_opcode(i1000_1011_iiii_iiii);
		break;
		//0xC -> Not existing
	case 0x000D:
		//0xFF00 0x8D00 i1000_1101_iiii_iiii bt.s <bdisp8>
		call_opcode(i1000_1101_iiii_iiii);
		break;
		//0xE -> Not existing
	case 0x000F:
		//0xFF00 0x8F00 i1000_1111_iiii_iiii bf.s <bdisp8>
		call_opcode(i1000_1111_iiii_iiii);
		break;
	default:
		invalid_op(opcode);
	}
	break;
case 0x0009:
	//0xF000 0x9000 i1001_nnnn_iiii_iiii mov.w @(<PCdisp8w>),<REG_N>
	call_opcode(i1001_nnnn_iiii_iiii);
	break;
case 0x000A:
	//0xF000 0xA000 i1010_iiii_iiii_iiii bra <bdisp12>
	call_opcode(i1010_iiii_iiii_iiii);
	break;
case 0x000B:
	//0xF000 0xB000 i1011_iiii_iiii_iiii bsr <bdisp12>
	call_opcode(i1011_iiii_iiii_iiii);
	break;
case 0xC:
	//0xF000 0xC000 -> 0xFF00
	switch((opcode>>8)&0xF)
	{
	case 0x0000:
		//0xFF00 0xC000 i1100_0000_iiii_iiii mov.b R0,@(<disp8b>,GBR)
		call_opcode(i1100_0000_iiii_iiii);
		break;
	case 0x0001:
		//0xFF00 0xC100 i1100_0001_iiii_iiii mov.w R0,@(<disp8w>,GBR)
		call_opcode(i1100_0001_iiii_iiii);
		break;
	case 0x0002:
		//0xFF00 0xC200 i1100_0010_iiii_iiii mov.l R0,@(<disp8dw>,GBR)
		call_opcode(i1100_0010_iiii_iiii);
		break;
	case 0x0003:
		//0xFF00 0xC300 i1100_0011_iiii_iiii trapa #<imm8>
		call_opcode(i1100_0011_iiii_iiii);
		break;
	case 0x0004:
		//0xFF00 0xC400 i1100_0100_iiii_iiii mov.b @(<GBRdisp8b>),R0
		call_opcode(i1100_0100_iiii_iiii);
		break;
	case 0x0005:
		//0xFF00 0xC500 i1100_0101_iiii_iiii mov.w @(<GBRdisp8w>),R0
		call_opcode(i1100_0101_iiii_iiii);
		break;
	case 0x0006:
		//0xFF00 0xC600 i1100_0110_iiii_iiii mov.l @(<GBRdisp8dw>),R0
		call_opcode(i1100_0110_iiii_iiii);
		break;
	case 0x0007:
		//0xFF00 0xC700 i1100_0111_iiii_iiii mova @(<PCdisp8d>),R0
		call_opcode(i1100_0111_iiii_iiii);
		break;
	case 0x0008:
		//0xFF00 0xC800 i1100_1000_iiii_iiii tst #<imm8>,R0
		call_opcode(i1100_1000_iiii_iiii);
		break;
	case 0x0009:
		//0xFF00 0xC900 i1100_1001_iiii_iiii and #<imm8>,R0
		call_opcode(i1100_1001_iiii_iiii);
		break;
	case 0x000A:
		//0xFF00 0xCA00 i1100_1010_iiii_iiii xor #<imm8>,R0
		call_opcode(i1100_1010_iiii_iiii);
		break;
	case 0x000B:
		//0xFF00 0xCB00 i1100_1011_iiii_iiii or #<imm8>,R0
		call_opcode(i1100_1011_iiii_iiii);
		break;
	case 0x000C:
		//0xFF00 0xCC00 i1100_1100_iiii_iiii tst.b #<imm8>,@(R0,GBR)
		call_opcode(i1100_1100_iiii_iiii);
		break;
	case 0x000D:
		//0xFF00 0xCD00 i1100_1101_iiii_iiii and.b #<imm8>,@(R0,GBR)
		call_opcode(i1100_1101_iiii_iiii);
		break;
	case 0x000E:
		//0xFF00 0xCE00 i1100_1110_iiii_iiii xor.b #<imm8>,@(R0,GBR)
		call_opcode(i1100_1110_iiii_iiii);
		break;
	case 0x000F:
		//0xFF00 0xCF00 i1100_1111_iiii_iiii or.b #<imm8>,@(R0,GBR)
		call_opcode(i1100_1111_iiii_iiii);
		break;
	default:
		invalid_op(opcode);
	}
	break;
case 0x000D:
	//0xF000 0xD000 i1101_nnnn_iiii_iiii mov.l @(<PCdisp8d>),<REG_N>
	call_opcode(i1101_nnnn_iiii_iiii);
	break;
case 0x000E:
	//0xF000 0xE000 i1110_nnnn_iiii_iiii mov #<simm8hex>,<REG_N>
	call_opcode(i1110_nnnn_iiii_iiii);
	break;
case 0xF:
	//0xF000 0xF000 -> 0xF00F
	switch(opcode&0xF)
	{
	case 0x0000:
		//0xF00F 0xF000 i1111_nnnn_mmmm_0000 fadd <FREG_M>,<FREG_N>
		call_opcode(i1111_nnnn_mmmm_0000);
		break;
	case 0x0001:
		//0xF00F 0xF001 i1111_nnnn_mmmm_0001 fsub <FREG_M>,<FREG_N>
		call_opcode(i1111_nnnn_mmmm_0001);
		break;
	case 0x0002:
		//0xF00F 0xF002 i1111_nnnn_mmmm_0010 fmul <FREG_M>,<FREG_N>
		call_opcode(i1111_nnnn_mmmm_0010);
		break;
	case 0x0003:
		//0xF00F 0xF003 i1111_nnnn_mmmm_0011 fdiv <FREG_M>,<FREG_N>
		call_opcode(i1111_nnnn_mmmm_0011);
		break;
	case 0x0004:
		//0xF00F 0xF004 i1111_nnnn_mmmm_0100 fcmp/eq <FREG_M>,<FREG_N>
		call_opcode(i1111_nnnn_mmmm_0100);
		break;
	case 0x0005:
		//0xF00F 0xF005 i1111_nnnn_mmmm_0101 fcmp/gt <FREG_M>,<FREG_N>
		call_opcode(i1111_nnnn_mmmm_0101);
		break;
	case 0x0006:
		//0xF00F 0xF006 i1111_nnnn_mmmm_0110 fmov.s @(R0,<REG_M>),<FREG_N>
		call_opcode(i1111_nnnn_mmmm_0110);
		break;
	case 0x0007:
		//0xF00F 0xF007 i1111_nnnn_mmmm_0111 fmov.s <FREG_M>,@(R0,<REG_N>)
		call_opcode(i1111_nnnn_mmmm_0111);
		break;
	case 0x0008:
		//0xF00F 0xF008 i1111_nnnn_mmmm_1000 fmov.s @<REG_M>,<FREG_N>
		call_opcode(i1111_nnnn_mmmm_1000);
		break;
	case 0x0009:
		//0xF00F 0xF009 i1111_nnnn_mmmm_1001 fmov.s @<REG_M>+,<FREG_N>
		call_opcode(i1111_nnnn_mmmm_1001);
		break;
	case 0x000A:
		//0xF00F 0xF00A i1111_nnnn_mmmm_1010 fmov.s <FREG_M>,@<REG_N>
		call_opcode(i1111_nnnn_mmmm_1010);
		break;
	case 0x000B:
		//0xF00F 0xF00B i1111_nnnn_mmmm_1011 fmov.s <FREG_M>,@-<REG_N>
		call_opcode(i1111_nnnn_mmmm_1011);
		break;
	case 0x000C:
		//0xF00F 0xF00C i1111_nnnn_mmmm_1100 fmov <FREG_M>,<FREG_N>
		call_opcode(i1111_nnnn_mmmm_1100);
		break;
	case 0xD:
		//0xF00F 0xF00D -> 0xF0FF
		switch((opcode>>4)&0xF)
		{
		case 0x0000:
			//0xF0FF 0xF00D i1111_nnnn_0000_1101 fsts FPUL,<FREG_N>
			call_opcode(i1111_nnnn_0000_1101);
			break;
		case 0x0001:
			//0xF0FF 0xF01D i1111_nnnn_0001_1101 flds <FREG_N>,FPUL
			call_opcode(i1111_nnnn_0001_1101);
			break;
		case 0x0002:
			//0xF0FF 0xF02D i1111_nnnn_0010_1101 float FPUL,<FREG_N>
			call_opcode(i1111_nnnn_0010_1101);
			break;
		case 0x0003:
			//0xF0FF 0xF03D i1111_nnnn_0011_1101 ftrc <FREG_N>, FPUL
			call_opcode(i1111_nnnn_0011_1101);
			break;
		case 0x0004:
			//0xF0FF 0xF04D i1111_nnnn_0100_1101 fneg <FREG_N>
			call_opcode(i1111_nnnn_0100_1101);
			break;
		case 0x0005:
			//0xF0FF 0xF05D i1111_nnnn_0101_1101 fabs <FREG_N>
			call_opcode(i1111_nnnn_0101_1101);
			break;
		case 0x0006:
			//0xF0FF 0xF06D i1111_nnnn_0110_1101 fsqrt <FREG_N>
			call_opcode(i1111_nnnn_0110_1101);
			break;
		case 0x0007:
			//0xF0FF 0xF07D i1111_nnnn_0111_1101 FSRRA <FREG_N> (1111nnnn 01111101)
			call_opcode(i1111_nnnn_0111_1101);
			break;
		case 0x0008:
			//0xF0FF 0xF08D i1111_nnnn_1000_1101 fldi0 <FREG_N>
			call_opcode(i1111_nnnn_1000_1101);
			break;
		case 0x0009:
			//0xF0FF 0xF09D i1111_nnnn_1001_1101 fldi1 <FREG_N>
			call_opcode(i1111_nnnn_1001_1101);
			break;
		case 0x000A:
			//0xF0FF 0xF0AD i1111_nnnn_1010_1101 fcnvsd FPUL,<DR_N>
			call_opcode(i1111_nnnn_1010_1101);
			break;
		case 0x000B:
			//0xF0FF 0xF0BD i1111_nnnn_1011_1101 fcnvds <DR_N>,FPUL
			call_opcode(i1111_nnnn_1011_1101);
			break;
			//0xC -> Not existing
			//0xD -> Not existing
		case 0x000E:
			//0xF0FF 0xF0ED i1111_nnmm_1110_1101 fipr <FV_M>,<FV_N>
			call_opcode(i1111_nnmm_1110_1101);
			break;
		case 0xF:
			//0xF0FF 0xF0FD -> 0xF1FF
			//i1111_nnn0_1111_1101
			//i1111_1011_1111_1101
			//i1111_0011_1111_1101
			//i1111_nn01_1111_1101
			//missing_op(opcode);
			switch((opcode>>8)&0xf)
			{
			case 0:
			case 2:
			case 4:
			case 6:
			case 8:
			case 10:
			case 12:
			case 14:
				call_opcode(i1111_nnn0_1111_1101);
				break;
			
			case 3:
				call_opcode(i1111_0011_1111_1101);
				break;
			
			case 11:
				call_opcode(i1111_1011_1111_1101);
				break;

			case 1:
			case 5:
			case 9:
			case 13:
				call_opcode(i1111_nn01_1111_1101);
				break;

			default:
				invalid_op(opcode);
			} 
			break;
		default:
			invalid_op(opcode);
		}
		break;
	case 0x000E:
		//0xF00F 0xF00E i1111_nnnn_mmmm_1110 fmac <FREG_0>,<FREG_M>,<FREG_N>
		call_opcode(i1111_nnnn_mmmm_1110);
		break;
		//0xF -> Not existing
	default:
		invalid_op(opcode);
	}
	break;
