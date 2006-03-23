//gdb stub
//ported over from kos to work w/ the emu


//What is needed in order to change from kos to emu ?
//reag reads from exeption info must be mapped to  emulated regs
//trapa must be handled first had [transaparently]
//writes/reads must be done using readMem/WriteMem

//Additions
//Proper suport for Restart/Download

//--drkIIRaziel
//original header on .h
//CODE needs :wsock32.lib 

//tooo many warnings :P gota fix em


#include "types.h"
#include "dc/sh4/sh4_registers.h"
#include "dc/mem/sh4_mem.h"

#include <string.h>
#include <winsock.h>

//fixme
#define EXC_ILLEGAL_INSTR 0
#define EXC_SLOT_ILLEGAL_INSTR 1
#define EXC_DATA_ADDRESS_READ 2
#define EXC_DATA_ADDRESS_WRITE 3
#define EXC_TRAPA 4
//end fix me
/* Hitachi SH architecture instruction encoding masks */

#define COND_BR_MASK   0xff00
#define UCOND_DBR_MASK 0xe000
#define UCOND_RBR_MASK 0xf0df
#define TRAPA_MASK     0xff00

#define COND_DISP      0x00ff
#define UCOND_DISP     0x0fff
#define UCOND_REG      0x0f00

/* Hitachi SH instruction opcodes */

#define BF_INSTR       0x8b00
#define BFS_INSTR      0x8f00
#define BT_INSTR       0x8900
#define BTS_INSTR      0x8d00
#define BRA_INSTR      0xa000
#define BSR_INSTR      0xb000
#define JMP_INSTR      0x402b
#define JSR_INSTR      0x400b
#define RTS_INSTR      0x000b
#define RTE_INSTR      0x002b
#define TRAPA_INSTR    0xc300
#define SSTEP_INSTR    0xc320

/* Hitachi SH processor register masks */

//#define T_BIT_MASK     0x0001

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound
 * buffers. At least NUMREGBYTES*2 are needed for register packets.
 */
#define BUFMAX 1024

/*
 * Number of bytes for registers
 */
#define NUMREGBYTES	41*4

/*
 * Modes for packet dcload packet transmission
 */

//#define DCL_SEND       0x1
//#define DCL_RECV       0x2
//#define DCL_SENDRECV   0x3

/*
 * typedef
 */
//typedef void (*Function) ();

/*
 * Forward declarations
 */

static int hex (char);
static char *mem2hex (char *, char *, int);
static char *hex2mem (char *, char *, int);
static int hexToInt (char **, int *);
static unsigned char *getpacket (void);
static void putpacket (char *);
static int computeSignal (int exceptionVector);

static void hardBreakpoint (int, int, int, int, char*);
static void putDebugChar (char);
static char getDebugChar (void);
static void flushDebugChannel (void);

void gdb_breakpoint(void);


static int dofault;  /* Non zero, bus errors will raise exception */

/* debug > 0 prints ill-formed commands in valid packets & checksum errors */
static int remote_debug;

enum regnames
  {
    R0, R1, R2, R3, R4, R5, R6, R7,
    R8, R9, R10, R11, R12, R13, R14, R15,
    PC, PR, GBR, VBR, MACH, MACL, SR,
    FPUL, FPSCR,
    FR0, FR1, FR2, FR3, FR4, FR5, FR6, FR7,
    FR8, FR9, FR10, FR11, FR12, FR13, FR14, FR15
  };

/* map from KOS register context order to GDB sh4 order */

#define KOS_REG( r ) ( ((u32*)&r))

static u32* kosRegMap[] =
{
  KOS_REG( r[0] ), KOS_REG( r[1] ), KOS_REG( r[2] ), KOS_REG( r[3] ),
  KOS_REG( r[4] ), KOS_REG( r[5] ), KOS_REG( r[6] ), KOS_REG( r[7] ),
  KOS_REG( r[8] ), KOS_REG( r[9] ), KOS_REG( r[10] ), KOS_REG( r[11] ),
  KOS_REG( r[12] ), KOS_REG( r[13] ), KOS_REG( r[14] ), KOS_REG( r[15] ),

  KOS_REG( pc ), KOS_REG( pr ), KOS_REG( gbr ), KOS_REG( vbr ),
  KOS_REG( mach ), KOS_REG( macl ), KOS_REG( sr ),
  KOS_REG( fpul ), KOS_REG( fpscr ),

  KOS_REG( fr[0] ), KOS_REG( fr[1] ), KOS_REG( fr[2] ), KOS_REG( fr[3] ),
  KOS_REG( fr[4] ), KOS_REG( fr[5] ), KOS_REG( fr[6] ), KOS_REG( fr[7] ),
  KOS_REG( fr[8] ), KOS_REG( fr[9] ), KOS_REG( fr[10] ), KOS_REG( fr[11] ),
  KOS_REG( fr[12] ), KOS_REG( fr[13] ), KOS_REG( fr[14] ), KOS_REG( fr[15] )
};

#undef KOS_REG

typedef struct
  {
    u16 *memAddr;
    u16 oldInstr;
  }
stepData;

//static u32 *registers;
static stepData instrBuffer;
static char stepped;
static const char hexchars[] = "0123456789abcdef";
static char remcomInBuffer[BUFMAX], remcomOutBuffer[BUFMAX];

//static char in_dcl_buf[BUFMAX], out_dcl_buf[BUFMAX];
//static int using_dcl = 0, in_dcl_pos = 0, out_dcl_pos = 0, in_dcl_size = 0;

static char highhex(int  x)
{
  return hexchars[(x >> 4) & 0xf];
}

static char lowhex(int  x)
{
  return hexchars[x & 0xf];
}

/*
 * Assembly macros
 */

//#define BREAKPOINT()   asm("trapa	#0xff"::);


/*
 * Routines to handle hex data
 */

static int
hex (char ch)
{
  if ((ch >= 'a') && (ch <= 'f'))
    return (ch - 'a' + 10);
  if ((ch >= '0') && (ch <= '9'))
    return (ch - '0');
  if ((ch >= 'A') && (ch <= 'F'))
    return (ch - 'A' + 10);
  return (-1);
}

/* convert the memory, pointed to by mem into hex, placing result in buf */
/* return a pointer to the last char put in buf (null) */
static char *
mem2hex (char *mem, char *buf, int count)
{
  int i;
  int ch;
  for (i = 0; i < count; i++)
    {
      ch = *mem++;
      *buf++ = highhex (ch);
      *buf++ = lowhex (ch);
    }
  *buf = 0;
  return (buf);
}

/* convert the hex array pointed to by buf into binary, to be placed in mem */
/* return a pointer to the character after the last byte written */

static char *
hex2mem (char *buf, char *mem, int count)
{
  int i;
  unsigned char ch;
  for (i = 0; i < count; i++)
    {
      ch = (unsigned char)hex (*buf++) << 4;
      ch = ch + (unsigned char)hex (*buf++);
      *mem++ = ch;
    }
  return (mem);
}

/**********************************************/
/* WHILE WE FIND NICE HEX CHARS, BUILD AN INT */
/* RETURN NUMBER OF CHARS PROCESSED           */
/**********************************************/
static int
hexToInt (char **ptr, int *intValue)
{
  int numChars = 0;
  int hexValue;

  *intValue = 0;

  while (**ptr)
    {
      hexValue = hex (**ptr);
      if (hexValue >= 0)
	{
	  *intValue = (*intValue << 4) | hexValue;
	  numChars++;
	}
      else
	break;

      (*ptr)++;
    }

  return (numChars);
}

/*
 * Routines to get and put packets
 */

/* scan for the sequence $<data>#<checksum>     */

bool b_waitpacket=true;
static unsigned char *
getpacket (void)
{

  unsigned char *buffer = (unsigned char*) &remcomInBuffer[0];
  unsigned char checksum;
  unsigned char xmitcsum;
  int count;
  char ch;

  while (b_waitpacket)
    {
      /* wait around for the start character, ignore all other characters */
      while ((ch = getDebugChar ()) != '$')
	;

retry:
      checksum = 0;
      xmitcsum =(unsigned char) -1;
      count = 0;

      /* now, read until a # or end of buffer is found */
      while (count < BUFMAX)
	{
	  ch = getDebugChar ();
          if (ch == '$')
            goto retry;
	  if (ch == '#')
	    break;
	  checksum = checksum + ch;
	  buffer[count] = ch;
	  count = count + 1;
	}
      buffer[count] = 0;

      if (ch == '#')
	{
 	  ch = getDebugChar ();
 	  xmitcsum = (unsigned char)hex (ch) << 4;
 	  ch = getDebugChar ();
 	  xmitcsum = xmitcsum+ ((unsigned char)hex (ch));

	  if (checksum != xmitcsum)
	    {
	      putDebugChar ('-');	/* failed checksum */
	    }
	  else
	    {
	      putDebugChar ('+');	/* successful transfer */

//		  printf("get_packet() -> %s\n", buffer);

	      /* if a sequence char is present, reply the sequence ID */
	      if (buffer[2] == ':')
		{
		  putDebugChar (buffer[0]);
		  putDebugChar (buffer[1]);

 		  return &buffer[3];
		}

	      return &buffer[0];
	    }
	}
    }

	return 0;//fail
}


/* send the packet in buffer. */

static void
putpacket (register char *buffer)
{

  register  int checksum;

  /*  $<packet info>#<checksum>. */
//  printf("put_packet() <- %s\n", buffer);
  do
    {
      char *src = buffer;
      putDebugChar ('$');
      checksum = 0;

      while (*src)
	{
	  int runlen;

	  /* Do run length encoding */
	  for (runlen = 0; runlen < 100; runlen ++) 
	    {
	      if (src[0] != src[runlen] || runlen == 99)
		{
		  if (runlen > 3) 
		    {
		      int encode;
		      /* Got a useful amount */
		      putDebugChar (*src);
		      checksum += *src;
		      putDebugChar ('*');
		      checksum += '*';
		      checksum += (encode = runlen + ' ' - 4);
		      putDebugChar ((unsigned char)encode);
		      src += runlen;
		    }
		  else
		    {
		      putDebugChar (*src);
		      checksum += *src;
		      src++;
		    }
		  break;
		}
	    }
	}


      putDebugChar ('#');
      putDebugChar (highhex(checksum));
      putDebugChar (lowhex(checksum));
      flushDebugChannel ();
    }
  while  (getDebugChar() != '+');

}


/*
 * this function takes the SH-1 exception number and attempts to
 * translate this number into a unix compatible signal value
 */
static int
computeSignal (int exceptionVector)
{
  int sigval;
  switch (exceptionVector)
    {
    case EXC_ILLEGAL_INSTR:
    case EXC_SLOT_ILLEGAL_INSTR:
      sigval = 4;
      break;
    case EXC_DATA_ADDRESS_READ:
    case EXC_DATA_ADDRESS_WRITE:
      sigval = 10;
      break;

    case EXC_TRAPA:
      sigval = 5;
      break;

    default:
      sigval = 7;		/* "software generated"*/
      break;
    }
  return (sigval);
}

static void
doSStep (void)
{
  u16 *instrMem;
  int displacement;
  int reg;
  u16 opcode, br_opcode;

  instrMem = (u16 *) GetMemPtr(pc,2);

  opcode = *instrMem;
  stepped = 1;

  br_opcode = opcode & COND_BR_MASK;

  if (br_opcode == BT_INSTR || br_opcode == BTS_INSTR)
    {
      if (sr.T)
	{
	  displacement = (opcode & COND_DISP) << 1;
	  if (displacement & 0x80)
	    displacement |= 0xffffff00;
	  /*
		   * Remember PC points to second instr.
		   * after PC of branch ... so add 4
		   */
	  instrMem = (u16 *) GetMemPtr(pc + displacement + 4,2);
	}
      else
      {
	/* can't put a trapa in the delay slot of a bt/s instruction */
	instrMem += ( br_opcode == BTS_INSTR ) ? 2 : 1;
      }
    }
  else if (br_opcode == BF_INSTR || br_opcode == BFS_INSTR)
    {
      if (sr.T)
    {
	/* can't put a trapa in the delay slot of a bf/s instruction */
	instrMem += ( br_opcode == BFS_INSTR ) ? 2 : 1;
    }
      else
	{
	  displacement = (opcode & COND_DISP) << 1;
	  if (displacement & 0x80)
	    displacement |= 0xffffff00;
	  /*
		   * Remember PC points to second instr.
		   * after PC of branch ... so add 4
		   */
	  instrMem = (u16 *) GetMemPtr(pc + displacement + 4,2);
	}
    }
  else if ((opcode & UCOND_DBR_MASK) == BRA_INSTR)
    {
      displacement = (opcode & UCOND_DISP) << 1;
      if (displacement & 0x0800)
	displacement |= 0xfffff000;

      /*
	   * Remember PC points to second instr.
	   * after PC of branch ... so add 4
	   */
      instrMem = (u16 *) GetMemPtr(pc + displacement + 4,2);
    }
  else if ((opcode & UCOND_RBR_MASK) == JSR_INSTR)
    {
      reg = (char) ((opcode & UCOND_REG) >> 8);

      instrMem = (u16 *) GetMemPtr(r[reg],2);
    }
  else if (opcode == RTS_INSTR)
    instrMem = (u16 *) GetMemPtr(pr,2);
  else if (opcode == RTE_INSTR)//WTF ?!?!
    instrMem = (u16 *) GetMemPtr(spc,2);//was registers[15] for some reason
  else if ((opcode & TRAPA_MASK) == TRAPA_INSTR)//WTF ?!?!
    instrMem = (u16 *) GetMemPtr(vbr+((opcode & ~TRAPA_MASK) << 2) ,2);
  else
    instrMem += 1;

  instrBuffer.memAddr = instrMem;
  instrBuffer.oldInstr = *instrMem;
  *instrMem = SSTEP_INSTR;
  //icache_flush_range((u32)instrMem, 2);
}


/* Undo the effect of a previous doSStep.  If we single stepped,
   restore the old instruction. */

static void
undoSStep (void)
{
  if (stepped)
    {  
		u16 *instrMem;
		instrMem = instrBuffer.memAddr;
		*instrMem = instrBuffer.oldInstr;
		//icache_flush_range((u32)instrMem, 2);
    }
  stepped = 0;
}

/* Handle inserting/removing a hardware breakpoint.
   Using the SH4 User Break Controller (UBC) we can have
   two breakpoints, each set for either instruction and/or operand access.
   Break channel B can match a specific data being moved, but there is
   no GDB remote protocol spec for utilizing this functionality. */

#define LREG(r, o) (*((u32*)((r)+(o))))
#define WREG(r, o) (*((u16*)((r)+(o))))
#define BREG(r, o) (*((u8*)((r)+(o))))

static void
hardBreakpoint (int set, int brktype, int addr, int length, char* resBuffer)
{
//  char* const ucb_base = (char*)((void*)0xff200000);
  static const int ucb_step = 0xc;
  static const char BAR = 0x0, BAMR = 0x4, BBR = 0x8, /*BASR = 0x14,*/ BRCR = 0x20;

  static const u8 bbrBrk[] = {
    0x0,  /* type 0, memory breakpoint -- unsupported */
    0x14, /* type 1, hardware breakpoint  -- unsupported */
    0x28, /* type 2, write watchpoint  -- unsupported */
    0x24, /* type 3, read watchpoint  -- unsupported */
    0x2c  /* type 4, access watchpoint  -- unsupported */
  };

  u8 bbr = 0;
//  char* ucb;
//  int i;

  if ( length <= 8 )
    do { bbr++; } while ( length >>= 1 );

  bbr |= bbrBrk[brktype];

  if ( addr == 0 ) /* GDB tries to watch 0, wasting a UCB channel */
  {
    strcpy(resBuffer, "OK");
  }
  else if ( brktype == 0 )
  {
    /* we don't support memory breakpoints -- the debugger
       will use the manual memory modification method */
    *resBuffer = '\0';
  }/*
  else if ( length > 8 )
  {
    strcpy(resBuffer, "E22");
  }
  else if (set)
  {
    WREG(ucb_base, BRCR) = 0;

    /* find a free UCB channel *//*
    for (ucb = ucb_base, i = 2; i > 0; ucb += ucb_step, i--)
      if (WREG(ucb, BBR) == 0)
        break;

    if (i)
    {
      LREG(ucb, BAR) = addr;
      BREG(ucb, BAMR) = 0x4; /* no BASR bits used, all BAR bits used *//*
      WREG(ucb, BBR) = bbr;
      strcpy(resBuffer, "OK");
    }
    else
      strcpy(resBuffer, "E12");
  }
  else
  {
    /* find matching UCB channel *//*
    for (ucb = ucb_base, i = 2; i > 0; ucb += ucb_step, i--)
      if (LREG(ucb, BAR) == addr && WREG(ucb, BBR) == bbr)
        break;

    if (i)
    {
      WREG(ucb, BBR) = 0;
      strcpy(resBuffer, "OK");
    }
    else
      strcpy(resBuffer, "E06");
  }*/
}

#undef LREG
#undef WREG

/*
This function does all exception handling.  It only does two things -
it figures out why it was called and tells gdb, and then it reacts
to gdb's requests.

When in the monitor mode we talk a human on the serial line rather than gdb.

*/

bool b_waitforgdb_command=true;
static void
gdb_handle_exception (int exceptionVector)
{
  int sigval, stepping;
  int addr, length;
  char *ptr;

  /* reply to host that an exception has occurred */
  sigval = computeSignal (exceptionVector);
  remcomOutBuffer[0] = 'S';
  remcomOutBuffer[1] = highhex(sigval);
  remcomOutBuffer[2] = lowhex (sigval);
  remcomOutBuffer[3] = 0;

  putpacket (remcomOutBuffer);

  /*
   * Do the thangs needed to undo
   * any stepping we may have done!
   */
  undoSStep ();

  stepping = 0;

  while (b_waitforgdb_command)
    {
      remcomOutBuffer[0] = 0;
      ptr = (char*)getpacket ();

      switch (*ptr++)
	{
	case '?':
	  remcomOutBuffer[0] = 'S';
	  remcomOutBuffer[1] = highhex (sigval);
	  remcomOutBuffer[2] = lowhex (sigval);
	  remcomOutBuffer[3] = 0;
	  break;
	case 'd':
	  remote_debug = !(remote_debug);	/* toggle debug flag */
	  break;

	case 'g':		/* return the value of the CPU registers */
	  {
	    int i;
	    char* outBuf = remcomOutBuffer;
	    for (i = 0; i < NUMREGBYTES/4; i++)
	      outBuf = mem2hex ((char *) (kosRegMap[i]), outBuf, 4);
	  }
	  break;

	case 'G':		/* set the value of the CPU registers - return OK */
	  {
	    int i;
	    char* inBuf = ptr;
	    for (i = 0; i < NUMREGBYTES/4; i++, inBuf += 8)
	      hex2mem (inBuf, (char *) (kosRegMap[i]), 4);
	    strcpy (remcomOutBuffer, "OK");
	  }
	  break;

	  /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
	case 'm':
	      dofault = 0;
	      /* TRY, TO READ %x,%x.  IF SUCCEED, SET PTR = 0 */
	      if (hexToInt (&ptr, &addr))
		if (*(ptr++) == ',')
		  if (hexToInt (&ptr, &length))
		    {
		      ptr = 0;
		      mem2hex ((char*)GetMemPtr(addr,1), remcomOutBuffer, length);
		    }
	      if (ptr)
		strcpy (remcomOutBuffer, "E01");

	  /* restore handler for bus error */
	  dofault = 1;
	  break;

	  /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
	case 'M':
	      dofault = 0;

	      /* TRY, TO READ '%x,%x:'.  IF SUCCEED, SET PTR = 0 */
	      if (hexToInt (&ptr, &addr))
		if (*(ptr++) == ',')
		  if (hexToInt (&ptr, &length))
		    if (*(ptr++) == ':')
		      {
			hex2mem (ptr, (char*)GetMemPtr( addr,1), length);
//			icache_flush_range(addr, length);
			ptr = 0;
			strcpy (remcomOutBuffer, "OK");
		      }
	      if (ptr)
		strcpy (remcomOutBuffer, "E02");

	  /* restore handler for bus error */
	  dofault = 1;
	  break;

	  /* cAA..AA    Continue at address AA..AA(optional) */
	  /* sAA..AA   Step one instruction from AA..AA(optional) */
	case 's':
	  stepping = 1;
	case 'c':
	  {
	    /* tRY, to read optional parameter, pc unchanged if no parm */
	    if (hexToInt (&ptr, &addr))
	      pc = addr;

	    if (stepping)
	      doSStep ();

		//RefreshDebugger(hDebugger);
	  }
	  return;
	  break;

	  /* kill the program */
	case 'k':		/* reboot */
      //arch_reboot();
		printf("GDB -> Kill app - REB00T\n");
	  break;

	  /* set or remove a breakpoint */
	case 'Z':
	case 'z':
	  {
	    int set = (*(ptr-1) == 'Z');
	    int brktype = *ptr++ - '0';
	    if (*(ptr++) == ',')
	      if (hexToInt (&ptr, &addr))
		if (*(ptr++) == ',')
		  if (hexToInt (&ptr, &length))
		    {
		      hardBreakpoint (set, brktype, addr, length, remcomOutBuffer);
		      ptr = 0;
		    }
	    if (ptr)
	      strcpy (remcomOutBuffer, "E02");
	    }
	  break;
	}			/* switch */

      /* reply to the request */
      putpacket (remcomOutBuffer);
    }
}


/* This function will generate a breakpoint exception.  It is used at the
   beginning of a program to sync up with a debugger and can be used
   otherwise as a quick means to stop program execution and "break" into
   the debugger. */

void
gdb_breakpoint (void)
{
      //BREAKPOINT ();
}



void SendData(u8* data,u32 len);
void GetData(u8* data,u32 len);

static char
getDebugChar (void)
{
	#if 0
  int ch;

  if (using_dcl) {
    if (in_dcl_pos >= in_dcl_size) {
      in_dcl_size = dcload_gdbpacket(NULL, 0, in_dcl_buf, BUFMAX);
      in_dcl_pos = 0;
    }
    ch = in_dcl_buf[in_dcl_pos++];
  } else {
    /* Spin while nothing is available. */
    while((ch = scif_read()) < 0);
    ch &= 0xff;
  }

  return ch;
#endif
  u8 rv=0;
  GetData(&rv,1);
  return (char)rv;
}

static void
putDebugChar (char ch)
{
	#if 0
  if (using_dcl) {
    out_dcl_buf[out_dcl_pos++] = ch;
    if (out_dcl_pos >= BUFMAX) {
      dcload_gdbpacket(out_dcl_buf, out_dcl_pos, NULL, 0);
      out_dcl_pos = 0;
    }
  } else {
    /* write the char and flush it. */
    scif_write(ch);
    scif_flush();
  }
#endif
  SendData((u8*)&ch,1);
}
static void
flushDebugChannel ()
{
#if 0
  /* send the current complete packet and wait for a response */
  if (using_dcl) {
    if (in_dcl_pos >= in_dcl_size) {
      in_dcl_size = dcload_gdbpacket(out_dcl_buf, out_dcl_pos, in_dcl_buf, BUFMAX);
      in_dcl_pos = 0;
    } else
      dcload_gdbpacket(out_dcl_buf, out_dcl_pos, NULL, 0);
    out_dcl_pos = 0;
  }
#endif
}

#if 0
//not called for now 
static void handle_exception(u32 code) {
#if 0
	registers = (u32 *)context;
	gdb_handle_exception(code);
#endif
}

//not called for now 
static void handle_user_trapa(u32 code) {
#if 0
	registers = (u32 *)context;
	gdb_handle_exception(EXC_TRAPA);
#endif
}

//not called for now 
static void handle_gdb_trapa(u32 code) {
	/*
  	* trapa 0x20 indicates a software trap inserted in
  	* place of code ... so back up PC by one
  	* instruction, since this instruction will
  	* later be replaced by its original one!
  	*/
#if 0
	registers = (u32 *)context;
	registers[PC] -= 2;
	gdb_handle_exception(EXC_TRAPA);
#endif
}

#endif
void gdb_init()
{
#if 0
  if (dcload_gdbpacket(NULL, 0, NULL, 0) == 0)
    using_dcl = 1;
  else
    scif_set_parameters(57600, 1);

  irq_set_handler(EXC_ILLEGAL_INSTR, handle_exception);
  irq_set_handler(EXC_SLOT_ILLEGAL_INSTR, handle_exception);
  irq_set_handler(EXC_DATA_ADDRESS_READ, handle_exception);
  irq_set_handler(EXC_DATA_ADDRESS_WRITE, handle_exception);
  irq_set_handler(EXC_USER_BREAK_PRE, handle_exception);

  trapa_set_handler(32, handle_gdb_trapa);
  trapa_set_handler(255, handle_user_trapa);

  BREAKPOINT();
#endif
}


//added code
bool bgdb_active=false;
SOCKET m_socket;

bool gdb_stub_exept_filter(int info,int trp)
{
	if (!bgdb_active)
		return false;

	//edit pc if needed
	//call gdb_handle_exception
	if (info==0x160)
	{
		switch (trp)
		{
		case  (0x20<<2):
			//pc-=2 -> no need
			gdb_handle_exception(EXC_TRAPA);
//			pc_ptr=(u16*)GetMemPtr(pc);
			break;
		case  (0xff<<2):
			pc+=2;//add 2 to the pc
			gdb_handle_exception(EXC_TRAPA);
			//pc_ptr=(u16*)GetMemPtr(pc);
			break;
		}
	}
	return true;
}

//start windows speciacific code
void SendData(u8* data,u32 len)
{
	send(m_socket,(const char*)data,len,0);
}

void GetData(u8* data,u32 len)
{
	recv(m_socket,(char*)data,len,0);
}


bool b_waitforgdb_connection=true;
void StartGDBSession()
{
	bgdb_active=true;
    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
    if ( iResult != NO_ERROR )
        printf("Error at WSAStartup()\n");

    // Create a socket.

    m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( m_socket == INVALID_SOCKET ) {
        printf( "Error at socket(): %ld\n", WSAGetLastError() );
        WSACleanup();
        return;
    }

    // Bind the socket.
    sockaddr_in service;

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    service.sin_port = htons( 2158 );

    if ( bind( m_socket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR ) {
        printf( "bind() failed.\n" );
        closesocket(m_socket);
        return;
    }
    
    // Listen on the socket.
    if ( listen( m_socket, 1 ) == SOCKET_ERROR )
        printf( "Error listening on socket.\n");

    // Accept connections.
    SOCKET AcceptSocket;

    printf( "Waiting for a client to connect...\n" );
    while (b_waitforgdb_connection) {
        AcceptSocket = (SOCKET)SOCKET_ERROR;
        while ( AcceptSocket == (SOCKET)SOCKET_ERROR ) {
            AcceptSocket = accept( m_socket, NULL, NULL );
        }
        printf( "Client Connected.\n");
        m_socket = AcceptSocket; 
        break;
    }

	gdb_stub_exept_filter(0x160,0x20<<2);

} 

void EndGDBSession()
{
	//needs to be implemented
}
//end windoze speciacific code
//called when pc is on boot area [0x8c001000]
void OnProgramBoot()
{
	if (!bgdb_active)
	{	
		printf("\n**Press y to enable gdb**\n");
		if (getc(stdin)=='y')
			StartGDBSession();
		else
			bgdb_active=false;
	}
	else
		gdb_stub_exept_filter(0x160,0x20<<2);
}