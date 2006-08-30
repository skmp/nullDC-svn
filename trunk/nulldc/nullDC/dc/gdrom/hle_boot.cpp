
#include <vector>
using namespace std;

#include "gdrom_if.h"
#include "dc/mem/sb.h"
#include "dc/sh4/dmac.h"
#include "dc/sh4/sh4_if.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/rec_v1/blockmanager.h"

#ifndef BUILD_NAOMI
//HLE boot code ;)

u8 pvd_temp[2048];
u32 FindPVD(u32 ss)
{
	printf("Scanning sectors %d to %d for PVD..\n",ss,ss+90000-1);
	for (u32 s=ss;s<(ss+90000);s++)
	{
		libGDR->gdr_info.ReadSector((u8*)pvd_temp, s, 1, 2048);
		if (pvd_temp[0]==1  &&
			pvd_temp[1]==67 && 
			pvd_temp[2]==68 && 
			pvd_temp[3]==48 &&
			pvd_temp[4]==48 &&
			pvd_temp[5]==49 &&
			pvd_temp[6]==1)
		{
			printf("PVD found At sector %d\n",s);
			//1+6+1+32+32+8+8+32+4+4+4+8+4+4+4+4-> rdr (156)
			//rdr+1+1 -> LE (156+2)
			//LE+4 -> BE (what dc uses (?) ) (156+2+4) (162)
			u32 temp=*(u32*)&pvd_temp[162];
			__asm 
			{
				mov eax,temp;
				bswap eax;
				mov temp,eax;
			}

			printf("RDR found At sector %d\n",temp);
			return temp;


			/*
			1      R, the number of bytes in the record (which must be even)
			1      0 [number of sectors in extended attribute record]
			8      number of the first sector of file data or directory
			(zero for an empty file), as a both endian double word
			8      number of bytes of file data or length of directory,
			excluding the extended attribute record,
			as a both endian double word

			*/
		}
		
	//find PVD -- sector 16
	/*
	    1      1
        6      67, 68, 48, 48, 49 and 1, respectively (same as Volume
                 Descriptor Set Terminator)
        1      0
       32      system identifier
       32      volume identifier
        8      zeros
        8      total number of sectors, as a both endian double word
       32      zeros
        4      1, as a both endian word [volume set size]
        4      1, as a both endian word [volume sequence number]
        4      2048 (the sector size), as a both endian word
        8      path table length in bytes, as a both endian double word
        4      number of first sector in first little endian path table,
                 as a little endian double word
        4      number of first sector in second little endian path table,
                 as a little endian double word, or zero if there is no
                 second little endian path table
        4      number of first sector in first big endian path table,
                 as a big endian double word
        4      number of first sector in second big endian path table,
                 as a big endian double word, or zero if there is no
                 second big endian path table
       34      root directory record, as described below
      128      volume set identifier
      128      publisher identifier
      128      data preparer identifier
      128      application identifier
       37      copyright file identifier
       37      abstract file identifier
       37      bibliographical file identifier
       17      date and time of volume creation
       17      date and time of most recent modification
       17      date and time when volume expires
       17      date and time when volume is effective
        1      1
        1      0
      512      reserved for application use (usually zeros)
      653      zeros
	*/
	}
	printf("boot HLE : PVD NOT found\n");
	return 23;//a guess :p
}

bool compstr_fs(u8* buffer,char* str,u32 len)
{
	for (u32 i=0;i<len;i++)
	{
		if (buffer[i]!=str[i])
			return false;
	}

	return true;
}
static unsigned int seed;

void my_srand(unsigned int n)
{
  seed = n & 0xffff;
}

unsigned int my_rand()
{
  seed = (seed * 2109 + 9273) & 0x7fff;
  return (seed + 0xc000) & 0xffff;
}


#define MAXCHUNK (2048*1024)
void load(u8*& fh, unsigned char *ptr, unsigned long sz)
{
	memcpy(ptr,fh,sz);
	fh+=sz;
}

void load_chunk(u8*& fh, unsigned char *ptr, int sz)
{
  static int idx[MAXCHUNK/32];
  int i;

  /* Convert chunk size to number of slices */
  sz /= 32;

  /* Initialize index table with unity,
     so that each slice gets loaded exactly once */
  for(i = 0; i < sz; i++)
    idx[i] = i;

  for(i = sz-1; i >= 0; --i)
    {
      /* Select a replacement index */
      int x = (my_rand() * i) >> 16;

      /* Swap */
      int tmp = idx[i];
      idx[i] = idx[x];
      idx[x] = tmp;

      /* Load resulting slice */
      load(fh, ptr+32*idx[i], 32);
    }
}

void load_file(u8* fh, unsigned char *ptr, unsigned long filesz)
{
  unsigned long chunksz;

  my_srand(filesz);

  /* Descramble 2 meg blocks for as long as possible, then
     gradually reduce the window down to 32 bytes (1 slice) */
  for(chunksz = MAXCHUNK; chunksz >= 32; chunksz >>= 1)
    while(filesz >= chunksz)
      {
	load_chunk(fh, ptr, (int)chunksz);
	filesz -= chunksz;
	ptr += chunksz;
      }

  /* Load final incomplete slice */
  if(filesz)
    load(fh, ptr, filesz);
}

bool file_scan(u32 sector,char* fn,u32* ssc,u32* fsz,u32 maxsec)
{
	u32 len=(u32)strlen(fn);
	u32 max_sector=sector+maxsec;
	printf("Scanning sector %d to %d for \"%s\" file entry\n",sector,max_sector-1,fn);
	for (;sector<max_sector;sector++)
	{
		libGDR->gdr_info.ReadSector((u8*)pvd_temp, sector, 1, 2048);
		for ( u32 i=0;i<(2048-len);i+=1)
		{
			if (compstr_fs(&pvd_temp[i],fn,len))
			{
				if (pvd_temp[i-1]<len || (*(u32*)&pvd_temp[i-1-4]!=0x1000001))
				{
					printf("Found @ %d:%d , but it is not a valid ISO9660 entry[%d,0x%X]\n",sector,i,pvd_temp[i-1],*(u32*)&pvd_temp[i-1-4]);
				}
				else
				{
					printf("Found @ %d:%d , 0x%X\n",sector,i,*(u32*)&pvd_temp[i-1-4]);

					*ssc=*(u32*)&pvd_temp[i-33+2+4];
					*fsz=*(u32*)&pvd_temp[i-33+2+8+4];
					__asm
					{
						mov eax,[ssc];
						mov ecx, [fsz];

						mov edx,[eax];
						bswap edx;
						mov [eax],edx;

						mov edx,[ecx];
						bswap edx;
						mov [ecx],edx;
					};
					printf("sector : %d , size : %d\n",*ssc,*fsz);
					printf("flags : 0x%X\n",pvd_temp[i-1-4-1-1-1]);

					//1+1+8+8+1+1+1+1+1+1+1+1+1+1+4+1 (33)
					/*
					1      R, the number of bytes in the record (which must be even)
					1      0 [number of sectors in extended attribute record]
					8      number of the first sector of file data or directory
					(zero for an empty file), as a both endian double word
					8      number of bytes of file data or length of directory,
					excluding the extended attribute record,
					as a both endian double word
					1      number of years since 1900
					1      month, where 1=January, 2=February, etc.
					1      day of month, in the range from 1 to 31
					1      hour, in the range from 0 to 23
					1      minute, in the range from 0 to 59
					1      second, in the range from 0 to 59
					(for DOS this is always an even number)
					1      offset from Greenwich Mean Time, in 15-minute intervals,
					as a twos complement signed number, positive for time
					zones east of Greenwich, and negative for time zones
					west of Greenwich (DOS ignores this field)
					1      flags, with bits as follows:
						bit     value
						------  ------------------------------------------
						0 (LS)  0 for a norma1 file, 1 for a hidden file
						1       0 for a file, 1 for a directory
						2       0 [1 for an associated file]
						3       0 [1 for record format specified]
						4       0 [1 for permissions specified]
						5       0
						6       0
						7 (MS)  0 [1 if not the final record for the file]
					1      0 [file unit size for an interleaved file]
					1      0 [interleave gap size for an interleaved file]
					4      1, as a both endian word [volume sequence number]
					1      N, the identifier length
					N      identifier

					*/
					return true;
				}

			}
		}
	}
	return false;
}


void gdBootHLE(void)
{
	printf("\n~~~\tgdBootHLE()\n\n");

	u32 toc[102];
	if (libGDR->gdr_info.GetDiskType()==GdRom)
		libGDR->gdr_info.GetToc(&toc[0], DoubleDensity);
	else
		libGDR->gdr_info.GetToc(&toc[0], SingleDensity);

	int i=0;
	for(i=98; i>=0; i--)
	{
		if (toc[i]!=0xFFFFFFFF)
		{
			if(0x40 == (toc[i]&0x40))
				break;
		}
	}
	if (i==-1) i=0;
	u32 addr = ((toc[i]&0xFF00)<<8) | ((toc[i]>>8)&0xFF00) | ((toc[i]>>24)&0xFF);

	///////////////////////////
	u8 * pmem = &mem_b[0x8000];
	libGDR->gdr_info.ReadSector(pmem,addr, 16, 2048);	// addr?

	char bootfile[16] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	for(i=0; i<16; i++) {
		if(0x20 == pmem[0x60+i])
			break;
		bootfile[i] =  pmem[0x60+i];
	}

	if (strlen(bootfile)==0)
		strcpy(bootfile,"1ST_READ.BIN");

	printf("IP.BIN BootFile: %s\n", bootfile);

	u32 PVD_sec=FindPVD(addr);

	u32 file_sector,file_len;
	if (file_scan(PVD_sec+150,bootfile,&file_sector,&file_len,400)==false)
	{
		printf("File not found , scanning using data track offset\n");
		if (file_scan(PVD_sec+addr,bootfile,&file_sector,&file_len,400)==false)
		{
			printf("File not found , scanning using fixed offset(:p)\n");
			if (file_scan(addr+23,bootfile,&file_sector,&file_len,400)==false)
			{
				printf("File not found , scanning hole disk (:p)\n");
				if (file_scan(0,bootfile,&file_sector,&file_len,addr+ 0x80000)==false)
				{
					printf("File is not there after all ...\n");
					return;
				}
			}
		}
	}

	u8* memptr= (u8*) malloc(((file_len/2048) +1)*2048);// 

	libGDR->gdr_info.ReadSector(memptr,file_sector+150, (file_len/2048) +1, 2048);	// addr?

	load_file(memptr,GetMemPtr(0x8c010000,file_len),file_len);
	free(memptr);
}


#else
void gdBootHLE(void) { printf("ERROR - (GD-HLE) BOOT IN NAOMI SECTION - !\n\n"); }
#endif