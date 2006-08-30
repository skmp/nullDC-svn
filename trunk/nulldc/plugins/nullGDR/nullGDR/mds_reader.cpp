#include "mds_reader.h"

mds_session sessions[256];
int mds_nsessions;

int flen(FILE*f)
{
	long lpos=ftell(f);
	long epos;

	fseek(f,0,SEEK_END);
	epos=ftell(f);
	fseek(f,lpos,SEEK_SET);
	return epos;
}

#define read_binary(type,source,offset) (*(type*)((source)+(offset)))

void parse_mds(char *mds_filename,bool verbose)
{
    /*"returns the supposable bin_filename and raw entries, if something went \
    wrong it throws an exception"*/
    // read the file

	FILE*mds_file = fopen (mds_filename,"rb");
	if(!mds_file)
	{
        fprintf(stderr,"Could not open the mds-file <%s>\n",mds_filename);
		return;
	}
    
    int  mds_size = flen(mds_file);
	char *mds_content=(char*)malloc(mds_size);

    if(!mds_content)
	{
        fprintf(stderr,"Could not allocate a buffer to read <%s>\n",mds_filename);
		fclose(mds_file);
		return;
	}

	fread(mds_content,mds_size,1,mds_file);
    
    if(memcmp(mds_content,"MEDIA DESCRIPTOR",16)!=0)
	{
        fprintf(stderr,"Invalid data in <%s>. It is not an MDF/MDS file.\n",mds_filename);
		free(mds_content);
		fclose(mds_file);
		return;
	}

    // get some data from header
    int mds_header_size = 0x70;
    int mds_datablock_size = 0x50;
    int mds_extrablock_size = 0x08;
    int mds_footer_size = 0x16;
    int mds_version = read_binary(char, mds_content, 0x0010);
    int mds_revision = read_binary(char, mds_content, 0x0011);
    int mds_sessions = read_binary(unsigned char, mds_content, 0x0014);

	if(verbose)
	{
		printf("MDS/MDF version: %d.%d\n",mds_version, mds_revision);
	}

	mds_nsessions=mds_sessions;

	int mds_session_offset = 0x58; 
	int mds_extrablocks_offset=0;

	for(int mds_current_session=0;mds_current_session<mds_sessions;mds_current_session++)
	{
		sessions[mds_current_session].sectors = read_binary(int, mds_content, mds_session_offset+0x0004);
		sessions[mds_current_session].datablocks = read_binary(unsigned char, mds_content, mds_session_offset+0x000a);
		sessions[mds_current_session].leadinblocks = read_binary(unsigned char, mds_content, mds_session_offset+0x000b);
		sessions[mds_current_session].session_ = read_binary(unsigned short, mds_content, mds_session_offset+0x000c);
		sessions[mds_current_session].last_track = read_binary(unsigned short, mds_content, mds_session_offset+0x000e);
		sessions[mds_current_session].datablocks_offset = read_binary(int, mds_content, mds_session_offset+0x0014);

		mds_extrablocks_offset=sessions[mds_current_session].datablocks_offset + mds_datablock_size*sessions[mds_current_session].datablocks;

		mds_session_offset = mds_session_offset + 0x18;
	}

	int mds_extrablocks_num=0;

	for(int mds_current_session=0;mds_current_session<mds_sessions;mds_current_session++)
	{
		sessions[mds_current_session].extrablocks_offset = mds_extrablocks_offset + mds_extrablock_size * mds_extrablocks_num;

		mds_extrablocks_num+=sessions[mds_current_session].datablocks;

	}

	for(int mds_current_session=0;mds_current_session<mds_sessions;mds_current_session++)
	{

		// making sure table is empty
		sessions[mds_current_session].ntracks=0;

		// read datablocks and extrablocks
		for(int datablock=0;datablock<sessions[mds_current_session].datablocks;datablock++)
		{
			int datablock_offset = sessions[mds_current_session].datablocks_offset+mds_datablock_size*datablock;
			int extrablock_offset = sessions[mds_current_session].extrablocks_offset+mds_extrablock_size*datablock;
			int mode = read_binary(unsigned char, mds_content, datablock_offset+0x0000);
			int flags = read_binary(unsigned short, mds_content, datablock_offset+0x0002);
			int track = read_binary(unsigned char, mds_content, datablock_offset+0x0004);
			int pmin = read_binary(unsigned char, mds_content, datablock_offset+0x0009);
			int psec = read_binary(unsigned char, mds_content, datablock_offset+0x000a);
			int pfrac = read_binary(unsigned char, mds_content, datablock_offset+0x000b);
			int sectorsize = read_binary(unsigned short, mds_content, datablock_offset+0x0010);
			int sector = read_binary(int, mds_content, datablock_offset+0x0024);
			__int64 offset = read_binary(__int64, mds_content, datablock_offset+0x0028);
			int pregap = read_binary(int, mds_content, extrablock_offset+0x0000);
			int sectors = read_binary(int, mds_content, extrablock_offset+0x0004);
			if(verbose)
				printf("datablock: %3d, track: %2x, mode: %2x, flags: %x, sector size: %d, MSF: %02d:%02d.%02d, sector: %d, offset: %d, pregap: %d, sectors: %d\n" ,
					datablock, track, mode, flags, sectorsize, pmin, psec, pfrac, sector, (int)offset, pregap, sectors);
			// writing data to entries
			if (track < 0xa0)
			{
				mds_strack *t=&(sessions[mds_current_session].tracks[sessions[mds_current_session].ntracks]);
				t->track     =track;
				t->mode      =mode;
				t->flags     =flags;
				t->sectorsize=sectorsize;
				t->pregap    =pregap;
				t->sector    =sector;
				t->sectors   =sectors;
				t->offset    =offset;
				sessions[mds_current_session].ntracks++;
			}
		}
	}

	free(mds_content);
	fclose(mds_file);

    return;
}