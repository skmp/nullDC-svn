/*
**	TA_Param.cpp - David Miller 2006 - PowerVR2 Emulation Library
*/
#include "PowerVR2.h"
using namespace PvrIf;

////////////////////
#define	TA_PARAM_CPP
#include "VertDecode.h"
#undef	TA_PARAM_CPP
////////////////////

AllocCtrl * ac;
u32 RenderPending=0;
u32 lists_complete=0;

TA_PolyMode PolyMode;




S_INLINE u32 ProcessParam(ParamBase *pb)
{
	PCW * pcw = &pb->pcw;

	lprintf("ProcessParam: %X\n", pcw->ParaType);

	switch(pcw->ParaType)
	{
	case PT_EndOfList:
		PolyMode = PM_None;

		lists_complete |= (1<<pcw->ListType);
		if(0x1F == lists_complete && RenderPending)
		{
			printf("Rendering Pending Data !\n");
			RenderPending=0;
			PvrIf::Render();
		}

		if(GlobalParams.size() > 0) {
			ASSERT_T((GlobalParams[GlobalParams.size()-1].pcw.ListType >= LT_Reserved),"<PVR> EndOfList: Reserved List Type !");
			emuIf.RaiseInterrupt(PvrInts[GlobalParams[GlobalParams.size()-1].pcw.ListType]);
		} else {
			ASSERT_T((1),"EndOfList, GlobalParamSize==0 \n");		// *FIXME* keep last param after render !
			emuIf.RaiseInterrupt(PvrInts[pb->pcw.ListType]);	// not exactly correct but it'll have to do
		}
	return 0;

	case PT_Polygon:		// Global: Polygon 
//	case PT_Modifier=4,		// Global: Modfifier Volume

		PolyMode = (pb->pcw.Volume && !pb->pcw.Shadow) ? PM_Modifier : PM_Vertex;

		if(PM_Vertex == PolyMode) {
			GlobalParams.push_back(*(GlobalParam*)pb);
		}
	return 0;

	case PT_Sprite:			// Global: Sprite
		PolyMode = PM_Sprite;
		GlobalParams.push_back(*(GlobalParam*)pb);
	return 0;

	case PT_Vertex:			// Vertex Parameter
		ASSERT_T((PM_None==PolyMode),		"<PVR> Vertex Recieved After Object List Ended !");

		if(PM_None==PolyMode) PolyMode = PM_Vertex;	// HACK 

		if(PM_Vertex == PolyMode) {
			PvrIf::AppendStrip((VertexParam*)pb);
			return 0;
		}
		else if(PM_Sprite == PolyMode) {
			//PSize = PvrIf::AppendSprite((GlobalParam*)pb);
			return 0;
		}
		else if(PM_Modifier == PolyMode) {	// Not Handled Yet *FIXME*
			return 0;
		}
		else
		{
			ASSERT_T((1), "PT_Vertex: Reached const. return value (ERROR)!");
			return 0;
		}

		//			lprintf("PT_Vertex: PolyMode: %d  PSize: %d\n", PolyMode, PSize);
	break;

	case PT_UserTileClip:	// Control: User Tile Clip	*FIXME*
	case PT_ObjectListSet:	// Control: Object List Set
	case PT_Reserved3:		// Control: Reserved Param
	case PT_Reserved6:		// Global: Reserved Param
		return 0;

	}

	ASSERT_T((1),"<PVR> ERROR Invalid ListType, Out of Context!\n");
	return 1;
}


u32 IsProcessing=0;

u32 WritePend=0;
u8 FifoBuff[96];

void TaFifo(u32 address, u32* data, u32 size)
{
	IsProcessing=true;
	s32 sz = (size<<=5);
	ParamBase * pb= NULL;

	lprintf("TaFifo(%08X, %p, %d)\n{\n", address, data, size);
	lprintf(" :: data %08X %08X %08X %08X\n",
		*(u32*)&data[0], *(u32*)&data[4], *(u32*)&data[8], *(u32*)&data[12]);

	if(WritePend) {
		memcpy(&FifoBuff[32], &data[0], 32);
		ProcessParam((ParamBase *)FifoBuff);
		WritePend = false;
		sz -= 32;
	}

	while(sz>0)
	{
		ASSERT_T((sz&31),"Illegal TaFifo sz!");
		pb = (ParamBase *)&data[((s32)size-sz)>>2];

		lprintf("-\tpb: %p :: sz: %d\n", pb, sz);

		u32 PSize = 32 ;
		if(PT_Vertex  == pb->pcw.ParaType) { if(isVert64Byte(&pb->pcw)) { PSize = 64; } }
		if(PT_Polygon == pb->pcw.ParaType) { if(isPoly64Byte(&pb->pcw)) { PSize = 64; } }
		if(PT_Sprite  == pb->pcw.ParaType) { if(isPoly64Byte(&pb->pcw)) { PSize = 64; } }

		if((64>sz) && (64==PSize))
		{
			WritePend = true;
			memcpy(FifoBuff, pb, sz);
			printf("!\nWritePend! sz:%d\n}\n",sz);
			lprintf("!\nWritePend! sz:%d\n}\n",sz);
			return;	// we're finished
		}

		ProcessParam(pb);

		sz -= PSize ;
	}
	lprintf("}\n\n");
	IsProcessing=false;
}








/////////// *TEMP* *FIXME* //////////


u8 opq[DCACHE_SIZE], trs[DCACHE_SIZE], ptu[DCACHE_SIZE];

u32 opos=0, tpos=0, ppos=0;
u32 nOpqStrips=0, nTrsStrips=0, nPtuStrips=0;

S_INLINE u8 * GetVBufferPtr(u32 which, u32 *pBytesLeft)	// which buffer, bytes left in buffer
{
	switch(which)
	{
	case LT_Opaque:
		*pBytesLeft = (DCACHE_SIZE-opos);
		return &opq[opos];

	case LT_Translucent:
		*pBytesLeft = (DCACHE_SIZE-tpos);
		return &trs[tpos];

	case LT_PunchThrough:
		*pBytesLeft = (DCACHE_SIZE-ppos);
		return &ptu[ppos];

	case LT_OpaqueMod:
	case LT_TransMod:
	case LT_Reserved:
	default: break;
	}

	*pBytesLeft=0;
	return NULL;
}

S_INLINE u32 UpdateVBuffer(u32 which, u32 written)	
{
	switch(which)
	{
	case LT_Opaque:			return DCACHE_SIZE - (opos += written) ;
	case LT_Translucent:	return DCACHE_SIZE - (tpos += written) ;
	case LT_PunchThrough:	return DCACHE_SIZE - (ppos += written) ;
	case LT_OpaqueMod:
	case LT_TransMod:
	case LT_Reserved:
	default: break;
	}
	return 0;
}












u32 idx = 0, LPType=0, ParamIdx=0;
u8 * pVBuf = NULL;



void ClearDCache()
{
	GlobalParams.clear();
	nOpqStrips	= opos	= 0;
	nTrsStrips	= tpos	= 0;
	nPtuStrips	= ppos	= 0;
	ParamIdx	= idx	= 0;
	pVBuf		= NULL;
}

u32 PvrIf::AppendStrip(VertexParam *vp)
{
	u32 BytesRemain = 0;

	if(0==idx) {
		ParamIdx = GlobalParams.size() ? (GlobalParams.size()-1) : 0 ;
		pVBuf = GetVBufferPtr(vp->pcw.ListType,&BytesRemain);
		ASSERT_T((sizeof(Vertex)>BytesRemain),		"AppendStrip() ByteRemain too Low");
	}
	ASSERT_T((NULL==pVBuf),							"AppendStrip() VBuff is NULL");
	ASSERT_F(((GlobalParams.size()-1)==ParamIdx),	"AppendStrip(), ParamIndex Changed")

	PolyParam * pp = &GlobalParams[ParamIdx];
	u32 PType = PolyType(&pp->pcw);

	ASSERT_T(((0!=idx) && (LPType!=PType)),			"AppendStrip() PType Mismatch");
	if(0==idx) { LPType = PType; }
	
	switch(PType&0xFFFF) {
	case 0x00: { DecodeStrip0(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x01: { DecodeStrip1(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x02: { DecodeStrip2(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x03: { DecodeStrip3(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x04: { DecodeStrip4(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x05: { DecodeStrip5(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x06: { DecodeStrip6(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x07: { DecodeStrip7(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x08: { DecodeStrip8(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x09: { DecodeStrip9(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x0A: { DecodeStripA(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x0B: { DecodeStripB(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x0C: { DecodeStripC(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x0D: { DecodeStripD(&((Vertex*)pVBuf)->List[idx], vp); break; }
	case 0x0E: { DecodeStripE(&((Vertex*)pVBuf)->List[idx], vp); break; }
	default:	ASSERT_T((1),"PvrIf::AppendStrip() Default Case Reached!"); return 0;
	}

	if(vp->pcw.EndOfStrip || (++idx>=MAX_VERTS))
	{
		((Vertex*)pVBuf)->Size	  = (MAX_VERTS==idx)?idx:idx+1;				// hack ?
		((Vertex*)pVBuf)->TexID	  = (u32)TCache.GetTexture(pp);
		((Vertex*)pVBuf)->ParamID = (u32)(GlobalParams.size()-1);

		switch(vp->pcw.ListType) {
		case LT_Opaque:			nOpqStrips++;	break;
		case LT_Translucent:	nTrsStrips++;	break;
		case LT_PunchThrough:	nPtuStrips++;	break;
		case LT_OpaqueMod:		__noop;			break;
		case LT_TransMod:		__noop;			break;
		default: ASSERT_F((0),"AppendStrip: ListType default !");	break;
		}
		UpdateVBuffer(vp->pcw.ListType, sizeof(Vertex));
		pVBuf = NULL;
		idx=0;
		LPType=0;
		ParamIdx=0;

		lprintf("---- EOS ----\n");
	}
	else
	{
		lprintf("Append: Idx: %X\n", idx);
	}
	return 0;
}
