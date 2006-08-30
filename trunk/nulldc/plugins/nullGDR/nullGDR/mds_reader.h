#include "nullGDR.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

struct mds_strack 
{
	int track;
	int mode;
	int flags;
	int sectorsize;
	int sector;
	int sectors;
	int pregap;
	__int64 offset;
};

struct mds_session
{
	int session_;
	int sectors;
	int datablocks;
	int leadinblocks;
	int last_track;
	int datablocks_offset;
	int extrablocks_offset;

	mds_strack tracks[256];
	int ntracks;
};

extern mds_session sessions[256];
extern int mds_nsessions;

bool parse_mds(char *mds_filename,bool verbose);
