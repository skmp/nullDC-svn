#pragma once

#include "types.h"

extern u32 DMAC_SAR0;
extern u32 DMAC_DAR0;
extern u32 DMAC_DMATCR0;
extern u32 DMAC_CHCR0;
extern u32 DMAC_SAR1;
extern u32 DMAC_DAR1;
extern u32 DMAC_DMATCR1;
extern u32 DMAC_CHCR1;
extern u32 DMAC_SAR2;
extern u32 DMAC_DAR2;
extern u32 DMAC_DMATCR2;
extern u32 DMAC_CHCR2;
extern u32 DMAC_SAR3;
extern u32 DMAC_DAR3;
extern u32 DMAC_DMATCR3;
extern u32 DMAC_CHCR3;
extern u32 DMAC_DMAOR;

//
void DMAC_Ch2St();

//Init/Res/Term
void dmac_Init();
void dmac_Reset(bool Manual);
void dmac_Term();



#define DMAOR_MASK	0xFFFF8201