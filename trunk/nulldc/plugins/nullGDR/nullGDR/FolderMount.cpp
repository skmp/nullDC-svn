//1048576
#include "FolderMount.h"

#define MAX_FILES	5120
#define RES_FMH		(MAX_FILES+100)

#define MIN_FFILE	10
#define FILE_IP		0

#define MAX_SECTORS  0x100000
#define SECTOR_START 0x160BC
#define FS_START 32
struct FileInfo;

typedef void ReadSectFP(u32 Sector, u8* buffer, u32 type,FileInfo* file);
struct FileInfo
{
	FILE* handle;
	u32 StartSector;
	u32 Size;
	ReadSectFP* ReadSector;
};


FileInfo Files[MAX_FILES];

u32 SecMap[MAX_SECTORS];

u32 DiskSize=0;

u8 * PathData;
u32  PathSize;

//File Enumeration

void ReadSectorFile(u32 Sector, u8* buffer, u32 type,FileInfo* file)
{
	u32 sof=(Sector-file->StartSector);
	fseek(file->handle,2048*sof,SEEK_SET);
	u8 temp[2048];
	fread(temp,1,2048,file->handle);
	ConvertSector(&temp[0],buffer,2048,type,Sector);
}
//sector arragement
//0-15			: ip.bin data
//16-31			: PVD and SVD
//16			: PVD
//17-31			: Depends
//32-40031		: File descriptors
//40032-45149	: Reserved space
//45150-90299	: Image of 0-45149 area

//Read ISO9660 table - null sector reads
void FM_ReadSector_misc(u32 Sector, u8* buffer, u32 type)
{
	if (Sector>=45150)
		Sector-=45150;

	if (Sector<16)
	{
		printf("ip.bin %d\n",Sector);
	}
	if (Sector==16)
	{
		printf("PVD read [16]\n");
	}
	if (Sector==17)
	{
		printf("SVD read [17]?\n");
	}
	if (Sector==18)
	{
		printf("SVD read [18]?\n");
	}
	if (Sector==19)
	{
		printf("SVD read [19]?\n");
	}
	if (Sector==20)
	{
		printf("SVD read [20]?\n");
	}
	if (Sector==21)
	{
		printf("SVD read [21]?\n");
	}

	if (PathData)
	{
		if (PathSize>(Sector*2048))
		{
			ConvertSector(&PathData[Sector*2048],buffer,2048,type,Sector);
		}
	}

	printf("Sector read %d\n",Sector);
}


void FM_ReadSector(u8 * buffer,u32 Sector,u32 count ,u32 type)
{
	for (u32 i=0;i<count;i++)
	{
		u32 fh=SecMap[Sector+i];
		if (fh>=RES_FMH)
		{
			FM_ReadSector_misc(Sector+i, buffer, type);
		}
		else
			Files[fh].ReadSector(Sector+i,buffer,type,&Files[fh]);
		buffer+=type;
	}
}
struct fsi
{
	fsi* items;
	u32 count;
	u32 size;
	u32 type;

	char Name[128];
	char File[256];
};
fsi* AddFile(fsi* to,char* name,char* realname)
{
	if (to->count >= to->size)
	{
		to->size+=20;
		to->items=(fsi*)realloc(to->items,to->size*sizeof(fsi));
	}

	to->items[to->count].items=0;
	to->items[to->count].count=0;
	to->items[to->count].size=0;
	to->items[to->count].type=1;//file

	strcpy(to->items[to->count].File,realname);
	strcpy(to->items[to->count].Name,name);
	strcat(to->items[to->count].Name,";1");

	to->count++;

	return &to->items[to->count-1];
}

fsi* AddFolder(fsi* to,char* name,char* realname)
{
	fsi* rv=AddFile(to,name,realname);
	rv->type=2;//folder
	return rv;
}

void CISOFileFound(char*fullpath,char* file,void* param,u32 flags)
{
	if (flags & DIR_FLAG)
	{
		fsi*fold=AddFolder((fsi*)param,file,fullpath);
		char temp[256];
		strcpy(temp,fold->File);
		strcat(temp,"*");
		FindAllFiles(CISOFileFound ,temp,fold);
	}
	else
	{
		//file
		AddFile((fsi*)param,file,fullpath);
	}
}

u32 WritePath32(u32 value,u32 pos)
{
	*(u32*)&PathData[pos]=value;
	*(u32*)&PathData[pos+4]=(value>>24) |((value>>8)&0xFF00)|((value<<8)&0xFF0000)|(((value<<24)&0xFF000000));

	return pos+8;
}
u32 WritePath16(u16 value,u32 pos)
{
	*(u16*)&PathData[pos]=value;
	*(u16*)&PathData[pos+2]=((value<<8)&0xFF00)|(((value>>8)&0xFF));
	return pos+4;
}
u32 WritePath8(u8 value,u32 pos)
{
	PathData[pos]=value;
	return pos+1;
}

u32 WritePathString(char* string,u32 len,u32 pos)
{
	for (u32 i=0;i<len;i++)
		WritePath8(string[i],pos+i);

	return pos+len;
}

u32 fs_start_sec=FS_START;
u32 GetNextFSSector()
{
	return fs_start_sec++;
}

u32 data_start_sec=SECTOR_START;
u32 GetFileSector(u32 bytes)
{
	u32 fsc=(bytes/2048)+3;
	u32 rv=data_start_sec;
	data_start_sec+=fsc;

	return rv;
}
u32 hfafbase=MIN_FFILE;
u32 GetNextFileID()
{
	return hfafbase++;
}
void CreateFILE(fsi* path,u32 PosPatch,u32 SizePatch)
{
	FILE* file;
	file=fopen(path->File,"rb");
	fseek(file,0,SEEK_END);
	u32 sz=ftell(file);
	u32 fss=GetFileSector(sz);
	u32 sc= (sz/2048)+1;

	u32 nfid=GetNextFileID();

	//SizePatch -> patch list size if !=0
	if (SizePatch)
		WritePath32(sz,SizePatch);

	if (PosPatch)
		WritePath32(fss,PosPatch);

	Files[nfid].handle=file;
	Files[nfid].ReadSector=ReadSectorFile;
	Files[nfid].Size=sz;
	Files[nfid].StartSector=fss;

	for (u32 i=fss;i<(fss+sc);i++)
	{
		SecMap[i]=nfid;
	}
}

void GenROOT(u32 sect,u32& PatchPos,u32& PatchSize)
{
	//1      R, the number of bytes in the record (which must be even)
		sect=WritePath8(34,sect);//a hole sector atm :P
		
		//1      0 [number of sectors in extended attribute record]
		sect=WritePath8(0,sect);

		
		//8      number of the first sector of file data or directory
		//         (zero for an empty file), as a both endian double word
		PatchPos=sect;
		sect=WritePath32(0,sect);

		//8      number of bytes of file data or length of directory,
		//         excluding the extended attribute record,
		//         as a both endian double word
		PatchSize=sect;
		sect=WritePath32(0,sect);

		//1      number of years since 1900
		sect=WritePath8(0,sect);

		//1      month, where 1=January, 2=February, etc.
		sect=WritePath8(0,sect);

		//1      day of month, in the range from 1 to 31
		sect=WritePath8(0,sect);

		//1      hour, in the range from 0 to 23
		sect=WritePath8(0,sect);

		//1      minute, in the range from 0 to 59
		sect=WritePath8(0,sect);

		//1      second, in the range from 0 to 59
		//        (for DOS this is always an even number)
		sect=WritePath8(0,sect);

		//1      offset from Greenwich Mean Time, in 15-minute intervals,
		//         as a twos complement signed number, positive for time
		//         zones east of Greenwich, and negative for time zones
		//         west of Greenwich (DOS ignores this field)
		sect=WritePath8(0,sect);

		//1      flags, with bits as follows:
		//         bit     value
		//          ------  ------------------------------------------
		//          0 (LS)  0 for a normal file, 1 for a hidden file
		//         1       0 for a file, 1 for a directory
		//         2       0 [1 for an associated file]
		//          3       0 [1 for record format specified]
		//          4       0 [1 for permissions specified]
		//          5       0
		//          6       0
		//          7 (MS)  0 [1 if not the final record for the file]
		//if (current->type==2)
			sect=WritePath8(2,sect);//folder

		// 1      0 [file unit size for an interleaved file]
		sect=WritePath8(0,sect);

		// 1      0 [interleave gap size for an interleaved file]
		sect=WritePath8(0,sect);

		// 4      1, as a both endian word [volume sequence number]
		sect=WritePath16(1,sect);

		// 1      N, the identifier length
		u8 len=1;
		sect=WritePath8(len,sect);

		// N      identifier
		sect=WritePathString("\0",len,sect);

		if (len & 1)
		{
			// P      padding byte: if N is even, P = 1 and this field contains
			//             a zero; if N is odd, P = 0 and this field is omitted
			sect=WritePath8(0,sect);
		}
}
void GenerateFromFSI(fsi* path,u32 PosPatch,u32 SizePatch)
{
	if (path->type!=2)
		return;//gah

	//SizePatch -> patch list size if !=0

	u32 patches_p[1024];
	u32 patches_s[1024];

	u32 sect,sp;
	for (u32 i=0;i<path->count;i++)
	{
		fsi* current=&path->items[i];
		
		if ((i%16)==0)
		{
			sect=GetNextFSSector()*2048;
			sp=sect/2048;
		}
		else
		{
			sect=sp*2048+(i%16)*128;
		}

		if (i==0)
		{
			if (PosPatch)
				WritePath32(sp,PosPatch);
		}

		//1      R, the number of bytes in the record (which must be even)
		sect=WritePath8(128,sect);//a hole sector atm :P
		
		//1      0 [number of sectors in extended attribute record]
		sect=WritePath8(0,sect);

		
		//8      number of the first sector of file data or directory
		//         (zero for an empty file), as a both endian double word
		patches_p[i]=sect;
		sect=WritePath32(0,sect);

		//8      number of bytes of file data or length of directory,
		//         excluding the extended attribute record,
		//         as a both endian double word
		patches_s[i]=sect;
		sect=WritePath32(0,sect);

		//1      number of years since 1900
		sect=WritePath8(0,sect);

		//1      month, where 1=January, 2=February, etc.
		sect=WritePath8(0,sect);

		//1      day of month, in the range from 1 to 31
		sect=WritePath8(0,sect);

		//1      hour, in the range from 0 to 23
		sect=WritePath8(0,sect);

		//1      minute, in the range from 0 to 59
		sect=WritePath8(0,sect);

		//1      second, in the range from 0 to 59
		//        (for DOS this is always an even number)
		sect=WritePath8(0,sect);

		//1      offset from Greenwich Mean Time, in 15-minute intervals,
		//         as a twos complement signed number, positive for time
		//         zones east of Greenwich, and negative for time zones
		//         west of Greenwich (DOS ignores this field)
		sect=WritePath8(0,sect);

		//1      flags, with bits as follows:
		//         bit     value
		//          ------  ------------------------------------------
		//          0 (LS)  0 for a normal file, 1 for a hidden file
		//         1       0 for a file, 1 for a directory
		//         2       0 [1 for an associated file]
		//          3       0 [1 for record format specified]
		//          4       0 [1 for permissions specified]
		//          5       0
		//          6       0
		//          7 (MS)  0 [1 if not the final record for the file]
		if (current->type==2)
			sect=WritePath8(2,sect);//folder
		else
			sect=WritePath8(0,sect);//file

		// 1      0 [file unit size for an interleaved file]
		sect=WritePath8(0,sect);

		// 1      0 [interleave gap size for an interleaved file]
		sect=WritePath8(0,sect);

		// 4      1, as a both endian word [volume sequence number]
		sect=WritePath16(1,sect);

		// 1      N, the identifier length
		u8 len=(u32)strlen(current->Name);
		sect=WritePath8(len,sect);

		// N      identifier
		sect=WritePathString(current->Name,len,sect);

		if (len & 1)
		{
			// P      padding byte: if N is even, P = 1 and this field contains
			//             a zero; if N is odd, P = 0 and this field is omitted
			sect=WritePath8(0,sect);
		}

		// R-33-N-P  unspecified field for system use; must contain an even
		//          number of bytes
	}

	if (SizePatch)
		WritePath32(path->count*128,SizePatch);


	for (u32 i=0;i<path->count;i++)
	{
		fsi* current=&path->items[i];
		if (current->type==2)
		{
			GenerateFromFSI(current,patches_p[i],patches_s[i]);
		}
		else if (current->type==1)
		{
			CreateFILE(current,patches_p[i],patches_s[i]);
		}
	}
}
void CreateISO(char* path)
{
	PathData=(u8*)malloc(1024*1024*10);//alloc 10 mb path storage
	PathSize=1024*1024*10;
	
	FILE* ip=fopen("c:\\ip.bin","rb");
	fread(PathData,1,16*2048,ip);
	fclose(ip);

	u32 cp=16*2048;
//	        1      1
	cp=WritePath8(1,cp);
//        6      67, 68, 48, 48, 49 and 1, respectively (same as Volume
//                 Descriptor Set Terminator)
		cp=WritePath8(67,cp);
	cp=WritePath8(68,cp);
	cp=WritePath8(48,cp);
	cp=WritePath8(48,cp);
	cp=WritePath8(49,cp);
	cp=WritePath8(1,cp);
//        1      0
	cp=WritePath8(0,cp);
//       32      system identifier
	WritePathString("ISO LAM0R\0\0\0\0",10,cp);
	cp+=32;
//       32      volume identifier
	WritePathString("ISO LAM0R\0\0\0\0",10,cp);
	cp+=32;
//        8      zeros
	for (int i=0;i<8;i++)
		cp=WritePath8(0,cp);

//        8      total number of sectors, as a both endian double word
	cp=WritePath32(MAX_SECTORS,cp);
//       32      zeros
	for (int i=0;i<32;i++)
		cp=WritePath8(0,cp);
//        4      1, as a both endian word [volume set size]
	cp=WritePath16(1,cp);
//        4      1, as a both endian word [volume sequence number]
	cp=WritePath16(1,cp);
//       4      2048 (the sector size), as a both endian word
	cp=WritePath16(2048,cp);
//       8      path table length in bytes, as a both endian double word
	cp=WritePath32(0,cp);
//        4      number of first sector in first little endian path table,
//                 as a little endian double word
	cp=WritePath16(0,cp);
//        4      number of first sector in second little endian path table,
//                 as a little endian double word, or zero if there is no
//                 second little endian path table
	cp=WritePath16(0,cp);
//        4      number of first sector in first big endian path table,
//                 as a big endian double word
	cp=WritePath16(0,cp);
//        4      number of first sector in second big endian path table,
//                 as a big endian double word, or zero if there is no
//                 second big endian path table
	cp=WritePath16(0,cp);
//       34      root directory record, as described below
		/*for (int i=0;i<34;i++)
			cp=WritePath8(0,cp);*/
	u32 pp , ps;
	GenROOT(cp,pp,ps);
	cp+=34;
//      128      volume set identifier
				for (int i=0;i<128;i++)
			cp=WritePath8(0,cp);

//      128      publisher identifier
for (int i=0;i<128;i++)
cp=WritePath8(0,cp);

//      128      data preparer identifier
for (int i=0;i<128;i++)
cp=WritePath8(0,cp);

//      128      application identifier
for (int i=0;i<128;i++)
cp=WritePath8(0,cp);

//       37      copyright file identifier
for (int i=0;i<37;i++)
cp=WritePath8(0,cp);

//       37      abstract file identifier
for (int i=0;i<37;i++)
cp=WritePath8(0,cp);

//       37      bibliographical file identifier
for (int i=0;i<37;i++)
cp=WritePath8(0,cp);

//       17      date and time of volume creation
for (int i=0;i<17;i++)
cp=WritePath8(0,cp);

//       17      date and time of most recent modification
for (int i=0;i<17;i++)
cp=WritePath8(0,cp);

//       17      date and time when volume expires
for (int i=0;i<17;i++)
cp=WritePath8(0,cp);

//       17      date and time when volume is effective
for (int i=0;i<17;i++)
cp=WritePath8(0,cp);

//        1      1
cp=WritePath8(1,cp);

//        1      0
cp=WritePath8(0,cp);


//      512      reserved for application use (usually zeros)
for (int i=0;i<512;i++)
cp=WritePath8(0,cp);

//      653      zeros
for (int i=0;i<653;i++)
cp=WritePath8(0,cp);





	fsi Root;
	Root.count=0;
	Root.size=0;
	Root.items=0;
	Root.type=2;//hey ! it's a dir !!

	FindAllFiles(CISOFileFound ,path,&Root);
	GenerateFromFSI(&Root,pp,ps);
}
void FM_init()
{
	for (int i=0;i<MAX_SECTORS;i++)
	{
		SecMap[i]=RES_FMH+1;
	}

	CreateISO("E:\\*");



	DriveNotifyEvent(DriveEvent::DiskChange,0);
}


void FM_term()
{
	if (PathData)
		free(PathData);
}


DiskType FM_DriveGetDiskType()
{
	return DiskType::GdRom;
}


void FM_DriveGetTocInfo(TocInfo& toc,DiskArea area)
{/*
	//Send a fake a$$ toc
	//toc->last.full		= toc->first.full	= CTOC_TRACK(1);
	toc->first.number=1;
	toc->last.number=1;
	toc->first.ControlInfo=toc->last.ControlInfo=4;
	toc->first.Addr=toc->last.Addr=0;
	toc->lba_leadout.FAD=DiskSize;

 	//toc->entry[0].full	= CTOC_LBA(150) | CTOC_ADR(0) | CTOC_CTRL(4);
	toc->entry[0].Addr=0;
	toc->entry[0].ControlInfo=4;
	//toc->entry[1].Addr=0;
	//toc->entry[1].ControlInfo=4;
	if (area==DoubleDensity)
	{
		toc->entry[0].FAD=150;
		//toc->entry[1].FAD=45150;
	}
	else
	{
		toc->entry[0].FAD=150;
		//toc->entry[1].FAD=45150;
	}

	for (int i=1;i<99;i++)
	{
		toc->entry[i].full=0xFFFFFFFF;
	}*/
}