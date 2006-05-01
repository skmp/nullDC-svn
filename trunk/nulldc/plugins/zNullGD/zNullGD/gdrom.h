/*
**	gdrom.h
*/
#ifndef __GDROM_H__
#define __GDROM_H__


#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

#ifdef __cplusplus
extern "C" {
#endif




typedef struct
{
	struct
	{
		BYTE CTRL:	4;
		BYTE ADR:	4;
		BYTE FAD[3];	// MSB

	} Track[99];

	struct
	{
		BYTE CTRL:	4;
		BYTE ADR:	4;

		BYTE TrackNo;
		WORD Reserved;

	} First, Last;

	struct
	{
		BYTE CTRL:	4;
		BYTE ADR:	4;
		BYTE FAD[3];	// MSB

	} LeadOut;

} GDTOC;







#ifdef __cplusplus
}
#endif

#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif //__GDROM_H__
