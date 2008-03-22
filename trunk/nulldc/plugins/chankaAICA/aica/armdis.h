/************************************************************************/
/* Arm/Thumb command set disassembler                                   */
/************************************************************************/

#ifndef __ARMDIS_H__
#define __ARMDIS_H__

#define DIS_VIEW_ADDRESS 1
#define DIS_VIEW_CODE 2

int disThumb(DWORD offset, char *dest, int flags);
int disArm(DWORD offset, char *dest, int flags);

#endif // __ARMDIS_H__
