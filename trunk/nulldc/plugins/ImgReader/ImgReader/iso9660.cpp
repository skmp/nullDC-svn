#include "iso9660.h"

bool inbios=true;
FILE* f_1=0;
FILE* f_2=0;
struct tinfo
{
	FILE* file;
	u32 SFAD;
	u32 EFAD;
};
tinfo tracks[5];
void rss(u8* buff,u32 ss,FILE* file)
{
	fseek(file,ss*2352+0x10,SEEK_SET);
	fread(buff,2048,1,file);
}
void iso_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
	/*
	if (f_1==0)
	{
		f_1 = fopen("D:/thps/45000to63139.bin","rb");
		f_2 = fopen("D:/thps/69313to549150.bin","rb");
	}
	printf("GDR->Read : Sector %d , size %d , mode %d \n",StartSector,SectorCount,secsz);
	if (StartSector>=69463)
	{
		StartSector-=69463;
		for (u32 i = 0 ; i < SectorCount;i++)
		{
			rss(buff,StartSector+i,f_2);
			buff+=2048;
		}
	}
	else if (StartSector>=63438 )
	{
		StartSector-=63438 ;
		for (u32 i = 0 ; i < SectorCount;i++)
		{
			printf("Unable to read %d\n",StartSector+i); 
			buff+=2048;
		}
	}
	else if (StartSector>=45150 )
	{
		StartSector-=45150 ;
		for (u32 i = 0 ; i < SectorCount;i++)
		{
			rss(buff,StartSector+i,f_1);
			buff+=2048;
		}
	}
	else
	{
		for (u32 i = 0 ; i < SectorCount;i++)
		{
			printf("Unable to read %d\n",StartSector+i); 
			buff+=2048;
		}
	}*/
	/*
	if (StartSector>=45000)
	{
		if (inbios)
		{
			StartSector-=45000;
			if (StartSector==150+16)
				inbios=false;//bios dma'd pvd
		}
	}
	else
	{
		if (inbios)
			inbios=false;//prop not bios/bios after pvd
	}

	if (StartSector>=150)
		StartSector-=150;


	if (StartSector<16)
	{
		FILE* fip=fopen("c:\\ip.bin","rb");
		fseek(fip,StartSector*2048,SEEK_SET);
		size_t rd=fread(buff,1,SectorCount*2048,fip);
		fclose(fip);
		return;
	}*/
	/*
	StartSector-=45150;
	if (f_iso)
	{
		fseek(f_iso,StartSector*2048,SEEK_SET);
		size_t rd=fread(buff,1,SectorCount*2048,f_iso);
		if (rd!=SectorCount*2048)
		{
			printf("fread  failed ; managed to read %d sectors olny (%d bytes)\n",rd/2048,rd);
			getc(stdin);
		}
	}
	else
	{
		char fn[512]="";
		if(GetFile(fn,"FILE0003.DUP \0FILE0003.DUP\0\0")==false)
			return;
		f_iso=fopen(fn,"rb");
		
		fseek(f_iso,StartSector*2048,SEEK_SET);
		size_t rd=fread(buff,1,SectorCount*2048,f_iso);
		if (rd!=SectorCount*2048)
		{
			printf("fread  failed ; managed to read %d sectors olny (%d bytes)\n",rd/2048,rd);
			getc(stdin);
		}
	}*/

}

void iso_DriveGetTocInfo(TocInfo* toc,DiskArea area)
{
	printf("GDROM toc\n");
	memset(toc,0,sizeof(TocInfo));
/*	//Send a fake a$$ toc
	//toc->last.full		= toc->first.full	= CTOC_TRACK(1);
	toc->first.number=1;
	toc->last.number=1;
	toc->first.ControlInfo=toc->last.ControlInfo=4;
	toc->first.Addr=toc->last.Addr=0;
	toc->lba_leadout.FAD=400000;

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
//TODO : fix up
DiskType iso_DriveGetDiskType()
{
	return DiskType::GdRom;
} 


bool iso_init(char* file)
{
	return false;
}

void iso_term()
{
}