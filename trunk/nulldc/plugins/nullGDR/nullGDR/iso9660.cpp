#include "iso9660.h"

bool inbios=true;
FILE* f_iso;
void iso_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
	printf("GDR->Read : Sector %d , size %d , mode %d \n",StartSector,SectorCount,secsz);
	if (StartSector<45000)
	{
		//printf("GDR->Read : Start Sector is < 45150 ; can't read sector\n");
		//return;
	}
	
	if (StartSector>=45000)
	{
		if (inbios)
		{
			StartSector-=45000;
			if (StartSector==16)
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



	//StartSector-=45150;
	if (StartSector<16)
	{
		FILE* fip=fopen("c:\\ip.bin","rb");
		fseek(fip,StartSector*2048,SEEK_SET);
		size_t rd=fread(buff,1,SectorCount*2048,fip);
		fclose(fip);
		return;
	}
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
		f_iso=fopen("F:/ADS.IXO.ISO","rb");
		
		fseek(f_iso,StartSector*2048,SEEK_SET);
		size_t rd=fread(buff,1,SectorCount*2048,f_iso);
		if (rd!=SectorCount*2048)
		{
			printf("fread  failed ; managed to read %d sectors olny (%d bytes)\n",rd/2048,rd);
			getc(stdin);
		}
	}

}

void iso_DriveGetTocInfo(TocInfo& toc,DiskArea area)
{
	/*//Send a fake a$$ toc
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


void iso_init()
{
}

void iso_term()
{
}
