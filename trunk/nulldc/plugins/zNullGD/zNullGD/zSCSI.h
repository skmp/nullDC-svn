/*
**	zSCSI.h - ZeZu (2006)
*/
#ifndef __ZSCSI_H__
#define __ZSCSI_H__





void scsiInit(void);			// exists to initialize pDev
extern mvSCSI_Dev * pDev;


void scsiReadTOC(mvSCSI_Dev * pDev, TOC *pTOC, BYTE fmt, BYTE track);
void scsiReadSector(mvSCSI_Dev * pDev, BYTE* pSecBuf, DWORD dwSecAddr);


#endif //__ZSCSI_H__

