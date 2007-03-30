#define _WIN32_WINNT 0x0500
#include <d3dx9.h>

#include "nullRend.h"

#include "d3dRend.h"
#include "windows.h"
//#include "gl\gl.h"
#include "regs.h"
#include <vector>
//#include <xmmintrin.h>

#if REND_API == REND_D3D

#define _float_colors_
//#define _HW_INT_
//#include <D3dx9shader.h>

using namespace TASplitter;

#define DEV_CREATE_FLAGS D3DCREATE_HARDWARE_VERTEXPROCESSING
//#define DEV_CREATE_FLAGS D3DCREATE_SOFTWARE_VERTEXPROCESSING

#if DEV_CREATE_FLAGS==D3DCREATE_HARDWARE_VERTEXPROCESSING
	#define VB_CREATE_FLAGS 0
#else
	#define VB_CREATE_FLAGS D3DUSAGE_SOFTWAREPROCESSING
#endif
#define VRAM_MASK 0x7FFFFF
//Convert offset32 to offset64
u32 vramlock_ConvOffset32toOffset64(u32 offset32)
{
		//64b wide bus is archevied by interleaving the banks every 32 bits
		//so bank is Address<<3
		//bits <4 are <<1 to create space for bank num
		//bank 1 is mapped at 400000 (32b offset) and after
		u32 bank=((offset32>>22)&0x1)<<2;//bank will be used ass uper offset too
		u32 lv=offset32&0x3; //these will survive
		offset32<<=1;
		//       |inbank offset    |       bank id        | lower 2 bits (not changed)
		u32 rv=  (offset32&(VRAM_MASK-7))|bank                  | lv;
 
		return rv;
}

namespace Direct3DRenderer
{
	IDirect3D9* d3d9;
	IDirect3DDevice9* dev;
	IDirect3DVertexBuffer9* vb;
	IDirect3DVertexShader9* compiled_vs;
	IDirect3DPixelShader9* compiled_ps[128]={0};
	u32 last_ps_mode=0xFFFFFFFF;
	//CRITICAL_SECTION tex_cache_cs;
	ID3DXFont* font;
	ID3DXConstantTable* shader_consts;
	
	bool IsFullscreen=false;
	char fps_text[512];

	void SetFpsText(char* text)
	{
		strcpy(fps_text,text);
		if (!IsFullscreen)
		{
			SetWindowText((HWND)emu.WindowHandle, fps_text);
		}
	}

	const static u32 CullMode[]= 
	{
		D3DCULL_NONE,	//0	No culling	no culling
		D3DCULL_NONE,	//1	Cull if Small	Cull if	( |det| < fpu_cull_val )

		//wtf ?
		D3DCULL_CCW /*D3DCULL_CCW*/,		//2	Cull if Negative	Cull if 	( |det| < 0 ) or
						//( |det| < fpu_cull_val )
		D3DCULL_CW /*D3DCULL_CW*/,		//3	Cull if Positive	Cull if 	( |det| > 0 ) or
						//( |det| < fpu_cull_val )
	};
	const static u32 Zfunction[]=
	{
		//This bit is used in combination with the Z Write Disable bit, and 
		//supports compare processing, which is required for OpenGL and D3D 
		//versus Z buffer updates.  It is important to note that, because the
		//value of either 1/z or 1/w is referenced for the Z value, the closer
		//that the polygon is, the larger that the Z value will be.
		
		//This setting is ignored for Translucent polygons in Auto-sort 
		//mode; the comparison must be made on a "Greater or Equal" basis.  
		//This setting is also ignored for Punch Through polygons in HOLLY2; 
		//the comparison must be made on a "Less or Equal" basis.


		D3DCMP_NEVER,				//0	Never
		D3DCMP_GREATEREQUAL,		//1	Less
		D3DCMP_EQUAL,				//2	Equal
		D3DCMP_GREATER,				//3	Less Or Equal
		D3DCMP_LESSEQUAL,			//4	Greater
		D3DCMP_NOTEQUAL,			//5	Not Equal
		D3DCMP_LESS,				//6	Greater Or Equal
		D3DCMP_ALWAYS,				//7	Always

	};

	/*
	0	Zero	(0, 0, 0, 0)
	1	One	(1, 1, 1, 1)
	2	‘Other’ Color	(OR, OG, OB, OA) 
	3	Inverse ‘Other’ Color	(1-OR, 1-OG, 1-OB, 1-OA)
	4	SRC Alpha	(SA, SA, SA, SA)
	5	Inverse SRC Alpha	(1-SA, 1-SA, 1-SA, 1-SA)
	6	DST Alpha	(DA, DA, DA, DA)
	7	Inverse DST Alpha	(1-DA, 1-DA, 1-DA, 1-DA)
	*/

	const static u32 DstBlendGL[] =
	{
		D3DBLEND_ZERO,
		D3DBLEND_ONE,
		D3DBLEND_SRCCOLOR,
		D3DBLEND_INVSRCCOLOR,
		D3DBLEND_SRCALPHA,
		D3DBLEND_INVSRCALPHA,
		D3DBLEND_DESTALPHA,
		D3DBLEND_INVDESTALPHA
	};

	const static u32 SrcBlendGL[] =
	{
		D3DBLEND_ZERO,
		D3DBLEND_ONE,
		D3DBLEND_DESTCOLOR,
		D3DBLEND_INVDESTCOLOR,
		D3DBLEND_SRCALPHA,
		D3DBLEND_INVSRCALPHA,
		D3DBLEND_DESTALPHA,
		D3DBLEND_INVDESTALPHA
	};

	char texFormatName[8][30]=
	{
		"1555",
		"565",
		"4444",
		"YUV422",
		"Bump Map",
		"4 BPP Palette",
		"8 BPP Palette",
		"Reserved	, 1555"
	};

	float unkpack_bgp_to_float[256];

	f32 f16(u16 v)
	{
		u32 z=v<<16;
		return *(f32*)&z;
	}
	const u32 MipPoint[8] =
	{
		0x00006,//8
		0x00016,//16
		0x00056,//32
		0x00156,//64
		0x00556,//128
		0x01556,//256
		0x05556,//512
		0x15556//1024
	};


#define twidle_tex(format)\
						if (tcw.NO_PAL.VQ_Comp)\
					{\
						vq_codebook=(u8*)&params.vram[sa];\
						if (tcw.NO_PAL.MipMapped)\
							sa+=MipPoint[tsp.TexU];\
						##format##to8888_VQ(&pbt,(u8*)&params.vram[sa],w,h);\
					}\
					else\
					{\
						if (tcw.NO_PAL.MipMapped)\
							sa+=MipPoint[tsp.TexU]<<3;\
						##format##to8888_TW(&pbt,(u8*)&params.vram[sa],w,h);\
					}
#define norm_text(format) \
	u32 sr;\
	if (tcw.NO_PAL.StrideSel)\
					{sr=(TEXT_CONTROL&31)*32;}\
					else\
					{sr=w;}\
					format(&pbt,(u8*)&params.vram[sa],sr,h);

	typedef void fastcall texture_handler_FP(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);

	/*
	texture_handler_FP* texture_handlers[8] = 
	{
		,//0	1555 value: 1 bit; RGB values: 5 bits each
		,//1	565	 R value: 5 bits; G value: 6 bits; B value: 5 bits
		,//3	YUV422 32 bits per 2 pixels; YUYV values: 8 bits each
		,//2	4444 value: 4 bits; RGB values: 4 bits each
		,//4	Bump Map	16 bits/pixel; S value: 8 bits; R value: 8 bits
		,//5	4 BPP Palette	Palette texture with 4 bits/pixel
		,//6	8 BPP Palette	Palette texture with 8 bits/pixel
		,//7 -> undefined , handled as 0
	};

	u32 texture_format[8]
	{
		D3DFMT_A1R5G5B5,//0	1555 value: 1 bit; RGB values: 5 bits each
		D3DFMT_R5G6B5,//1	565	 R value: 5 bits; G value: 6 bits; B value: 5 bits
		D3DFMT_UYVY,//3	YUV422 32 bits per 2 pixels; YUYV values: 8 bits each
		D3DFMT_A4R4G4B4,//2	4444 value: 4 bits; RGB values: 4 bits each
		D3DFMT_UNKNOWN,//4	Bump Map	16 bits/pixel; S value: 8 bits; R value: 8 bits
		D3DFMT_A8R8G8B8,//5	4 BPP Palette	Palette texture with 4 bits/pixel
		D3DFMT_A8R8G8B8,//6	8 BPP Palette	Palette texture with 8 bits/pixel
		D3DFMT_A1R5G5B5,//7 -> undefined , handled as 0
	};
	*/
	struct TextureCacheData;
	std::vector<TextureCacheData*> lock_list;
	//Texture Cache :)
	struct TextureCacheData
	{
		u32 Start;
		IDirect3DTexture9* Texture;
		u32 Lookups;
		u32 Updates;

		TSP tsp;TCW tcw;

		u32 w,h;
		u32 size;
		bool dirty;
		u32 pal_rev;
		volatile vram_block* lock_block;

		//Called when texture entry is reused , resets any texture type info (dynamic/static)
		void Reset()
		{
			Lookups=0;
			Updates=0;
		}
		void PrintTextureName()
		{
			printf(texFormatName[tcw.NO_PAL.PixelFmt]);
	
			if (tcw.NO_PAL.VQ_Comp)
				printf(" VQ");

			if (tcw.NO_PAL.ScanOrder==0)
				printf(" TW");

			if (tcw.NO_PAL.MipMapped)
				printf(" MM");

			if (tcw.NO_PAL.StrideSel)
				printf(" Stride");

			if (tcw.NO_PAL.StrideSel)
				printf(" %d[%d]x%d @ 0x%X",(TEXT_CONTROL&31)*32,8<<tsp.TexU,8<<tsp.TexV,tcw.NO_PAL.TexAddr<<3);
			else
				printf(" %dx%d @ 0x%X",8<<tsp.TexU,8<<tsp.TexV,tcw.NO_PAL.TexAddr<<3);
			printf("\n");
		}
		void Update()
		{
//			verify(dirty);
//			verify(lock_block==0);

			Updates++;
			dirty=false;

			u32 sa=(tcw.NO_PAL.TexAddr<<3) & 0x7FFFFF;

			if (Texture==0)
			{
				/*if (tcw.NO_PAL.PixelFmt==3 && tcw.NO_PAL.ScanOrder==1)
				{
					verifyc(dev->CreateTexture(w,h,1,0,D3DFMT_UYVY,D3DPOOL_MANAGED,&Texture,0));
				}
				else
				{*/
				if (tcw.NO_PAL.MipMapped)
				{
					verifyc(dev->CreateTexture(w,h,0,D3DUSAGE_AUTOGENMIPMAP,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&Texture,0));
				}
				else
				{
					verifyc(dev->CreateTexture(w,h,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&Texture,0));
				}
				//}
			}
			D3DLOCKED_RECT rect;

			verifyc(Texture->LockRect(0,&rect,NULL,D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD));
			

			PixelBuffer pbt; 
			pbt.init(rect.pBits,rect.Pitch);
			
			switch (tcw.NO_PAL.PixelFmt)
			{
			case 0:
			case 7:
				//0	1555 value: 1 bit; RGB values: 5 bits each
				//7	Reserved	Regarded as 1555
				if (tcw.NO_PAL.ScanOrder)
				{
					//verify(tcw.NO_PAL.VQ_Comp==0);
					norm_text(argb1555to8888);
					//argb1555to8888(&pbt,(u16*)&params.vram[sa],w,h);
				}
				else
				{
					//verify(tsp.TexU==tsp.TexV);
					twidle_tex(argb1555);
				}
				break;

				//redo_argb:
				//1	565	 R value: 5 bits; G value: 6 bits; B value: 5 bits
			case 1:
				if (tcw.NO_PAL.ScanOrder)
				{
					//verify(tcw.NO_PAL.VQ_Comp==0);
					norm_text(argb565to8888);
					//(&pbt,(u16*)&params.vram[sa],w,h);
				}
				else
				{
					//verify(tsp.TexU==tsp.TexV);
					twidle_tex(argb565);
				}
				break;

				
				//2	4444 value: 4 bits; RGB values: 4 bits each
			case 2:
				if (tcw.NO_PAL.ScanOrder)
				{
					//verify(tcw.NO_PAL.VQ_Comp==0);
					//argb4444to8888(&pbt,(u16*)&params.vram[sa],w,h);
					norm_text(argb4444to8888);
				}
				else
				{
					twidle_tex(argb4444);
				}

				break;
				//3	YUV422 32 bits per 2 pixels; YUYV values: 8 bits each
			case 3:
				if (tcw.NO_PAL.ScanOrder)
				{
					norm_text(YUV422to8888);
					//norm_text(ANYtoRAW);
				}
				else
				{
					//it cant be VQ , can it ?
					//docs say that yuv can't be VQ ...
					//HW seems to support it ;p
					twidle_tex(YUV422);
				}
				break;
				//4	Bump Map	16 bits/pixel; S value: 8 bits; R value: 8 bits
			case 5:
				//5	4 BPP Palette	Palette texture with 4 bits/pixel
				verify(tcw.PAL.VQ_Comp==0);
				if (tcw.NO_PAL.MipMapped)
							sa+=MipPoint[tsp.TexU]<<1;
				palette_index = tcw.PAL.PalSelect<<4;
				pal_rev=pal_rev_16[tcw.PAL.PalSelect];
				PAL4to8888_TW(&pbt,(u8*)&params.vram[sa],w,h);
				break;
			case 6:
				//6	8 BPP Palette	Palette texture with 8 bits/pixel
				verify(tcw.PAL.VQ_Comp==0);
				if (tcw.NO_PAL.MipMapped)
							sa+=MipPoint[tsp.TexU]<<2;
				palette_index = (tcw.PAL.PalSelect<<4)&(~0xFF);
				pal_rev=pal_rev_256[tcw.PAL.PalSelect>>4];
				PAL8to8888_TW(&pbt,(u8*)&params.vram[sa],w,h);
				break;
			default:
				printf("Unhandled texture\n");
				//memset(temp_tex_buffer,0xFFEFCFAF,w*h*4);
			}


			//done , unlock texture !
			Texture->UnlockRect(0);
			//PrintTextureName();
			if (!lock_block)
				lock_list.push_back(this);
			/*
			char file[512];

			sprintf(file,"d:\\textures\\0x%x_%d_%s_VQ%d_TW%d_MM%d_.jpg",Start,Lookups,texFormatName[tcw.NO_PAL.PixelFmt]
			,tcw.NO_PAL.VQ_Comp,tcw.NO_PAL.ScanOrder,tcw.NO_PAL.MipMapped);
			D3DXSaveTextureToFileA( file,D3DXIFF_JPG,Texture,0);*/
		}
		void LockVram()
		{
			u32 sa=(tcw.NO_PAL.TexAddr<<3) & 0x7FFFFF;
			u32 ea=sa+w*h*2;
			if (ea>=(8*1024*1024))
			{
				ea=(8*1024*1024)-1;
			}
			lock_block = params.vram_lock_64(sa,ea,this);
		}
	};

	TexCacheList<TextureCacheData> TexCache;

	TextureCacheData* __fastcall GenText(TSP tsp,TCW tcw,TextureCacheData* tf)
	{
		//generate texture
		tf->Start=tcw.full;//(tcw.NO_PAL.TexAddr<<3) & 0x7FFFFF;
		
		tf->w=8<<tsp.TexU;
		tf->h=8<<tsp.TexV;
		tf->tsp=tsp;
		tf->tcw=tcw;
		tf->dirty=true;
		tf->lock_block=0;
		tf->Texture=0;
		tf->Reset();
		tf->Update();
		return tf;
	}

	TextureCacheData* __fastcall GenText(TSP tsp,TCW tcw)
	{
		//add new entry to tex cache
		TextureCacheData* tf = &TexCache.Add(0)->data;
		//Generate texture 
		return GenText(tsp,tcw,tf);
	}

	u32 RenderToTextureAddr;

	IDirect3DTexture9* __fastcall GetTexture(TSP tsp,TCW tcw)
	{	
		//u32 addr=(tcw.NO_PAL.TexAddr<<3) & 0x7FFFFF;
		/*if (addr==RenderToTextureAddr)
			return RenderToTextureTex;*/

		//EnterCriticalSection(&tex_cache_cs);
		TextureCacheData* tf = TexCache.Find(tcw.full);
		if (tf)
		{
			
			if (tf->dirty)
			{
				if (tf->tsp.full==tsp.full)
					tf->Update();
				else
				{
					if (tf->Texture)
					{
						tf->Texture->Release();
						tf->Texture=0;
					}
					GenText(tsp,tcw,tf);
				}
			}
			if (tcw.PAL.PixelFmt==5)
			{				
				if (tf->pal_rev!=pal_rev_16[tcw.PAL.PalSelect])
					tf->Update();
			}
			else if (tcw.PAL.PixelFmt==6)
			{
				if (tf->pal_rev!=pal_rev_256[tcw.PAL.PalSelect>>4])
					tf->Update();
			}

			tf->Lookups++;
			//LeaveCriticalSection(&tex_cache_cs);
			return tf->Texture;
		}
		else
		{
			tf = GenText(tsp,tcw);
			//LeaveCriticalSection(&tex_cache_cs);
			return tf->Texture;
		}
		return 0;
	}
	
	void VramLockedWrite(vram_block* bl)
	{
		//EnterCriticalSection(&tex_cache_cs);
		TextureCacheData* tcd = (TextureCacheData*)bl->userdata;
		tcd->dirty=true;
		tcd->lock_block=0;
		if (tcd->Updates==0)
		{
			tcd->Texture->Release();
			tcd->Texture=0;
		}
		params.vram_unlock(bl);
		//LeaveCriticalSection(&tex_cache_cs);
	}
	extern cThread rth;

	//use that someday
	void VBlank()
	{
		rth.Start();
		//we need to actualy draw the image here :)
		//dev->
	}

	//Vertex storage types
	//64B
	struct Vertex
	{
		//64
		float x,y,z;

#ifdef _float_colors_
		float col[4];
		float spc[4];
#else
		u32 col;
		u32 spc;
#endif
		float u,v;

		#ifdef _HW_INT_
			float base_int,offset_int;
		#endif
	};
	const D3DVERTEXELEMENT9 vertelem[] =
	{
		{0, 0,  D3DDECLTYPE_FLOAT3		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,0},
		#ifdef _float_colors_
			{0, 12, D3DDECLTYPE_FLOAT4		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},	//Base color
			{0, 28, D3DDECLTYPE_FLOAT4		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1},	//Specular(offset) color
			#ifdef _HW_INT_
				{0, 44, D3DDECLTYPE_FLOAT4		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,0},	//u,v,base intesity , offset intesity
			#else
				{0, 44, D3DDECLTYPE_FLOAT2		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,0},	//u,v,base intesity , offset intesity
			#endif
		#else
			{0, 12, D3DDECLTYPE_D3DCOLOR	, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},	//Base color
			{0, 16, D3DDECLTYPE_D3DCOLOR	, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1},	//Specular color
			{0, 20, D3DDECLTYPE_FLOAT4		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,0},	//u,v,base intesity , offset intesity
		#endif
		
		D3DDECL_END()
	};

	IDirect3DVertexDeclaration9* vdecl;

	struct PolyParam
	{
		u32 first;		//entry index , holds vertex/pos data
		u32 count;

		//lets see what more :)
		
		TSP tsp;
		TCW tcw;
		PCW pcw;
		ISP_TSP isp;
	};


	static u32 vert_reappend;

	//vertex lists
	struct TA_context
	{
		u32 Address;
		u32 LastUsed;
		f32 invW_min;
		f32 invW_max;
		List<Vertex> verts;
		List<PolyParam> global_param_op;
		List<PolyParam> global_param_pt;
		List<PolyParam> global_param_tr;

		void Init()
		{
			verts.Init();
			global_param_op.Init();
			global_param_pt.Init();
			global_param_tr.Init();
		}
		void Clear()
		{
			verts.Clear();
			global_param_op.Clear();
			global_param_pt.Clear();
			global_param_tr.Clear();
			invW_min= 1000000.0f;
			invW_max=-1000000.0f;
		}
		void Free()
		{
			verts.Free();
			global_param_op.Free();
			global_param_pt.Free();
			global_param_tr.Free();
		}
	};
	
	bool UsingAutoSort()
	{
		if (((FPU_PARAM_CFG>>21)&1) == 0)
			return ((ISP_FEED_CFG&1)==0);
		else
			return ( ((*(u32*)&params.vram[ REGION_BASE & 0x7FFFFF])>>29) & 1) == 0;
	}

	TA_context tarc;
	TA_context pvrrc;

	std::vector<TA_context> rcnt;
	u32 fastcall FindRC(u32 addr)
	{
		for (u32 i=0;i<rcnt.size();i++)
		{
			if (rcnt[i].Address==addr)
			{
				return i;
			}
		}
		return 0xFFFFFFFF;
	}
	void fastcall SetCurrentTARC(u32 addr)
	{
		//return;
		//printf("SetCurrentTARC:0x%X\n",addr);
		if (addr==tarc.Address)
			return;//nothing to do realy

		//save old context
		u32 found=FindRC(tarc.Address);
		if (found!=0xFFFFFFFF)
			memcpy(&rcnt[found],&tarc,sizeof(TA_context));

		//switch to new one
		found=FindRC(addr);
		if (found!=0xFFFFFFFF)
		{
			memcpy(&tarc,&rcnt[found],sizeof(TA_context));
		}
		else
		{
			//add one :p
			tarc.Address=addr;
			tarc.Init();
			tarc.Clear();
			rcnt.push_back(tarc);
		}
	}
	void fastcall SetCurrentPVRRC(u32 addr)
	{
		//return;
	//	printf("SetCurrentPVRRC:0x%X\n",addr);
		if (addr==tarc.Address)
		{
			memcpy(&pvrrc,&tarc,sizeof(TA_context));
			return;
		}

		u32 found=FindRC(addr);
		if (found!=0xFFFFFFFF)
		{
			memcpy(&pvrrc,&rcnt[found],sizeof(TA_context));
			return;
		}

		printf("WARNING : Unable to find a PVR rendering context\n");
		memcpy(&pvrrc,&tarc,sizeof(TA_context));
	}


	PolyParam* CurrentPP=0;
	List<PolyParam>* CurrentPPlist;
	
	template<D3DSAMPLERSTATETYPE state>
	void SetTexMode(u32 clamp,u32 mirror)
	{
		if (clamp)
			dev->SetSamplerState(0,state,D3DTADDRESS_CLAMP);
		else 
		{
			if (mirror)
				dev->SetSamplerState(0,state,D3DTADDRESS_MIRROR);
			else
				dev->SetSamplerState(0,state,D3DTADDRESS_WRAP);
		}
		
	}

	TSP cache_tsp;
	TCW cache_tcw;
	PCW cache_pcw;
	ISP_TSP cache_isp;

	//for fixed pipeline
	__forceinline
	void SetGPState_fp(PolyParam* gp)
	{
		if (gp->pcw.Texture)
		{
			dev->SetRenderState(D3DRS_SPECULARENABLE,gp->pcw.Offset );

			IDirect3DTexture9* tex=GetTexture(gp->tsp,gp->tcw);
			dev->SetTexture(0,tex);

			//if (gp->tsp.ClampV!=cache_tsp.ClampV || gp->tsp.FlipV!=cache_tsp.FlipV)
				SetTexMode<D3DSAMP_ADDRESSV>(gp->tsp.ClampV,gp->tsp.FlipV);

			//if (gp->tsp.ClampU!=cache_tsp.ClampU || gp->tsp.FlipU!=cache_tsp.FlipU)
				SetTexMode<D3DSAMP_ADDRESSU>(gp->tsp.ClampU,gp->tsp.FlipU);

			//if (gp->tsp.ShadInstr!=cache_tsp.ShadInstr ||  ( gp->tsp.UseAlpha != cache_tsp.UseAlpha) )
			{
				switch(gp->tsp.ShadInstr)	// these should be correct, except offset
				{
					//PIXRGB = TEXRGB + OFFSETRGB
					//PIXA    = TEXA
				case 0:	// Decal
					dev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
					dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

					dev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

					if (gp->tsp.IgnoreTexA)
					{
						//a=1
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
					}
					else
					{
						//a=tex.a
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					}
					break;

					//The texture color value is multiplied by the Shading Color value.  
					//The texture  value is substituted for the Shading a value.
					//PIXRGB = COLRGB x TEXRGB + OFFSETRGB
					//PIXA   = TEXA
				case 1:	// Modulate
					dev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
					dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

					dev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

					if (gp->tsp.IgnoreTexA)
					{
						//a=1
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
					}
					else
					{
						//a=tex.a
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					}
					break;
					//The texture color value is blended with the Shading Color 
					//value according to the texture a value.
					//PIXRGB = (TEXRGB x TEXA) +
					//(COLRGB x (1- TEXA) ) +
					//OFFSETRGB
					//PIXA   = COLA
				case 2:	// Decal Alpha
					if (gp->tsp.IgnoreTexA)
					{
						//Tex.a=1 , so Color = Tex
						dev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
					}
					else
					{
						dev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_BLENDTEXTUREALPHA);
					}
					dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

					dev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
					if(gp->tsp.UseAlpha)
					{
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
					}
					else
					{
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
					}
					break;

					//The texture color value is multiplied by the Shading Color value. 
					//The texture a value is multiplied by the Shading a value.
					//PIXRGB= COLRGB x  TEXRGB + OFFSETRGB
					//PIXA   = COLA  x TEXA
				case 3:	// Modulate Alpha
					dev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
					dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

					if(gp->tsp.UseAlpha)
					{
						if (gp->tsp.IgnoreTexA)
						{
							//a=Col.a
							dev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
						}
						else
						{
							//a=Text.a*Col.a
							dev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
						}
					}
					else
					{
						if (gp->tsp.IgnoreTexA)
						{
							//a= 1
							dev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTA_TFACTOR);
						}
						else
						{
							//a= Text.a*1
							dev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
						}
					}
					dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

					break;
				}
			}

			/*
			//i use D3DRS_SPECULARENABLE now ..
			if (gp->pcw.Offset==0)
			{
				dev->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
				dev->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
			}
			else
			{
				dev->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_ADD);
				dev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
				dev->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_SPECULAR);

				dev->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
				dev->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
			}
			*/
		}
		else
		{
			//Offset color is enabled olny if Texture is enabled ;)
			dev->SetRenderState(D3DRS_SPECULARENABLE,FALSE);
			dev->SetTexture(0,NULL);
		}
	}
	//fox pixel shaders

	//Texture -> 1 if texture is enabled , 0 if its not
	//Offset -> 1 if offset is enabled , 0 if its not (only valid when texture is enabled)
	//ShadInstr -> 0 to 3 , see pvr docs , valid only when texture is enabled
	//IgnoreTexA -> 1 if on  0 if off , valid only w/ textures on
	//UseAlpha -> 1 if on  0 if off , works when no textures are used too ?

	#define idx_pp_Texture 0
	#define idx_pp_Offset 1
	#define idx_pp_ShadInstr 2
	#define idx_pp_IgnoreTexA 3
	#define idx_pp_UseAlpha 4
	#define idx_pp_profile 5

	D3DXMACRO ps_macros[]=
	{
		{"pp_Texture",0},
		{"pp_Offset",0},
		{"pp_ShadInstr",0},
		{"pp_IgnoreTexA",0},
		{"pp_UseAlpha",0},
		{"ps_profile",0},	//Shader profile version , just defined , no value :)
		{0,0}	//end of list
	};
	char* ps_macro_numers[] =
	{
		"0",
		"1",
		"2",
		"3",
	};

	void CompilePS(u32 mode,const char* profile)
	{
		verify(mode<128);
		if (compiled_ps[mode]!=0)
			return;
		ID3DXBuffer* perr;
		ID3DXBuffer* shader;
		ID3DXConstantTable* consts;

		D3DXCompileShaderFromFileA("ps_hlsl.fx"
			,ps_macros,NULL,"PixelShader_main",profile,NULL,&shader,&perr,&consts);
		if (perr)
		{
			char* text=(char*)perr->GetBufferPointer();
			printf("%s\n",text);
		}
		verifyc(dev->CreatePixelShader((DWORD*)shader->GetBufferPointer(),&compiled_ps[mode]));
		if (perr)
			perr->Release();
		shader->Release();
		consts->Release();
	}
	void PrecompilePS()
	{
		char temp[30];
		strcpy(temp,D3DXGetPixelShaderProfile(dev));
		temp[2]='0';
		temp[4]='0';
		//printf(&temp[3]);

		ps_macros[5].Definition=&temp[3];
		const char * profile=D3DXGetPixelShaderProfile(dev);

#define forl(n,s,e) for (u32 n=s;n<=e;n++)
		forl(Texture,0,1)
		{
			ps_macros[idx_pp_Texture].Definition=ps_macro_numers[Texture];
			forl(UseAlpha,0,1)
			{
				ps_macros[idx_pp_UseAlpha].Definition=ps_macro_numers[UseAlpha];
				if (Texture)
				{
					forl(Offset,0,1)
					{
						ps_macros[idx_pp_Offset].Definition=ps_macro_numers[Offset];
						forl(ShadInstr,0,3)
						{
							ps_macros[idx_pp_ShadInstr].Definition=ps_macro_numers[ShadInstr];
							forl(IgnoreTexA,0,1)
							{
								ps_macros[idx_pp_IgnoreTexA].Definition=ps_macro_numers[IgnoreTexA];

								u32 mode=0;			
								mode|=Offset;
								mode<<=2;
								mode|=ShadInstr;
								mode<<=1;
								mode|=IgnoreTexA;
								mode<<=1;
								mode|=Texture;
								mode<<=1;
								mode|=UseAlpha;

								CompilePS(mode,profile);
							}
						}
					}
				}
				else
				{
					ps_macros[idx_pp_Offset].Definition=ps_macro_numers[0];
					ps_macros[idx_pp_ShadInstr].Definition=ps_macro_numers[0];
					ps_macros[idx_pp_IgnoreTexA].Definition=ps_macro_numers[0];

					u32 mode=0;			
					mode|=Texture;
					mode<<=1;
					mode|=UseAlpha;

					CompilePS(mode,profile);
				}
			}
		}
#undef forl
	}
	void FASTCALL SetPS(u32 mode)
	{
		if (last_ps_mode!=mode)
		{
			last_ps_mode=mode;
			dev->SetPixelShader(compiled_ps[mode]);
		}
	}
	__forceinline
	void SetGPState_ps(PolyParam* gp)
	{
		u32 mode=0;
		if (gp->pcw.Texture)
		{
			IDirect3DTexture9* tex=GetTexture(gp->tsp,gp->tcw);
			dev->SetTexture(0,tex);

			if (gp->tsp.FilterMode == 0)
			{
				dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
				dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
				dev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

			}
			else
			{
				dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
				dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
				dev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			}
			
			//if (gp->tsp.ClampV!=cache_tsp.ClampV || gp->tsp.FlipV!=cache_tsp.FlipV)
				SetTexMode<D3DSAMP_ADDRESSV>(gp->tsp.ClampV,gp->tsp.FlipV);

			//if (gp->tsp.ClampU!=cache_tsp.ClampU || gp->tsp.FlipU!=cache_tsp.FlipU)
				SetTexMode<D3DSAMP_ADDRESSU>(gp->tsp.ClampU,gp->tsp.FlipU);
			
			/*
			if (gp->tsp.FilterMode)
			{
				dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
				dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			}
			else
			{
				dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
				dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			}
			*/

			ps_macros[1].Definition=ps_macro_numers[gp->pcw.Offset];
			mode|=gp->pcw.Offset;
			

			ps_macros[2].Definition=ps_macro_numers[gp->tsp.ShadInstr];
			mode<<=2;
			mode|=gp->tsp.ShadInstr;
			

			ps_macros[3].Definition=ps_macro_numers[gp->tsp.IgnoreTexA];
			mode<<=1;
			mode|=gp->tsp.IgnoreTexA;
			
		}

		ps_macros[0].Definition=ps_macro_numers[gp->pcw.Texture];
		mode<<=1;
		mode|=gp->pcw.Texture;
		

		ps_macros[4].Definition=ps_macro_numers[gp->tsp.UseAlpha];
		mode<<=1;
		mode|=gp->tsp.UseAlpha;

		SetPS(mode);
	}


	//
	template <u32 Type>
	__forceinline
	void SetGPState(PolyParam* gp)
	{
		SetGPState_ps(gp);
		//up to here , has to be replaced by a shader
		if (Type==ListType_Translucent)
		{
			//if (gp->tsp.SrcInstr!= cache_tsp.SrcInstr)
				dev->SetRenderState(D3DRS_SRCBLEND, SrcBlendGL[gp->tsp.SrcInstr]);

			//if (gp->tsp.DstInstr!= cache_tsp.DstInstr)
				dev->SetRenderState(D3DRS_DESTBLEND, DstBlendGL[gp->tsp.DstInstr]);
		}


		//set cull mode !
		//if (gp->isp.CullMode!=cache_isp.CullMode)
			dev->SetRenderState(D3DRS_CULLMODE,CullMode[gp->isp.CullMode]);
		//set Z mode !
		if (Type==ListType_Opaque)
		{
			//if (gp->isp.DepthMode != cache_isp.DepthMode)
				dev->SetRenderState(D3DRS_ZFUNC,Zfunction[gp->isp.DepthMode]);
		}
		else if (Type==ListType_Translucent)
		{
			//if (autosort) -> where ? :p
			//dev->SetRenderState(D3DRS_ZFUNC,Zfunction[6]); // : GEQ
			//else -> fix it ! someday
			//if (gp->isp.DepthMode != cache_isp.DepthMode)
				dev->SetRenderState(D3DRS_ZFUNC,Zfunction[gp->isp.DepthMode]);
		}
		else
		{
			//gp->isp.DepthMode=6;
			//if (cache_isp.DepthMode != 6)
				dev->SetRenderState(D3DRS_ZFUNC,Zfunction[6]); //PT : LEQ //GEQ ?!?! wtf ? seems like the docs ARE wrong on this one =P
		}

		//if (gp->isp.ZWriteDis!=cache_isp.ZWriteDis)
			dev->SetRenderState(D3DRS_ZWRITEENABLE,gp->isp.ZWriteDis==0);

		//cache_tsp=gp->tsp;
		//cache_isp=gp->isp;
		//cache_pcw=gp->pcw;
		//cache_tcw=gp->tcw;
	}
	template <u32 Type>
	__forceinline
	void RendStrips(PolyParam* gp)
	{
		//0 vert polys ? why does games even bother sending em  ? =P
		if (gp->count>2)
		{		
			SetGPState<Type>(gp);
			dev->DrawPrimitive(D3DPT_TRIANGLESTRIP,gp->first ,
				gp->count-2);
		}
	}

	template <u32 Type>
	void RendPolyParamList(List<PolyParam>& gpl)
	{
		for (u32 i=0;i<gpl.used;i++)
		{		
			RendStrips<Type>(&gpl.data[i]);
		}
	}
	//
	void DrawFPS()
	{

		// Create a colour for the text - in this case blue
		D3DCOLOR fontColor = D3DCOLOR_ARGB(255,0x18,0xFF,0);  

		// Create a rectangle to indicate where on the screen it should be drawn
		RECT rct;
		rct.left=2;
		rct.right=780;
		rct.top=10;
		rct.bottom=rct.top+30;

		// Draw some text 
		font->DrawText(NULL, fps_text, -1, &rct, 0, fontColor );
	}

	//
	void DoRender()
	{
		if (FB_W_SOF1 & 0x1000000)
			return;

		void* ptr;

		u32 sz=pvrrc.verts.used*sizeof(Vertex);

		verifyc(vb->Lock(0,sz,&ptr,D3DLOCK_DISCARD));

		memcpy(ptr,pvrrc.verts.data,sz);

		verifyc(vb->Unlock());
		//memset(pvrrc.verts.data,0xFEA345FD,pvrrc.verts.size*sizeof(Vertex));



		// Clear the backbuffer to a blue color
		//All of the screen is allways filled w/ smth , no need to clear the color buffer
		//gives a nice speedup on large resolutions
		verifyc(dev->Clear( 0, NULL, /*D3DCLEAR_TARGET |*/ D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,122,199), 1.0f, 0 ));

		// Begin the scene
		if( SUCCEEDED( dev->BeginScene() ) )
		{			
			//Init stuff
			dev->SetVertexShader(compiled_vs);

#define clamp(minv,maxv,x) min(maxv,max(minv,x))
			float bg=*(float*)&ISP_BACKGND_D; 

#ifdef scale_type_1
			float c0=1/clamp(0.0000001f,10000000.0f,pvrrc.invW_max);
			float c1=1/clamp(0.0000001f,10000000.0f,pvrrc.invW_min);
			c0*=0.99f;
			c1*=1.01f;

			dev->SetVertexShaderConstantF(0,&c0,1);
			dev->SetVertexShaderConstantF(1,&c1,1);
#endif
			//dev->SetPixelShader(CompiledPShader);

			dev->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);
			//			verifyc(dev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL));

			dev->SetVertexDeclaration(vdecl);
			dev->SetStreamSource(0,vb,0,sizeof(Vertex));

			dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

			dev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS,
				D3DTTFF_COUNT4 | D3DTTFF_PROJECTED);
			
			dev->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);

			//Opaque
			dev->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
			dev->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
if (!GetAsyncKeyState(VK_F1))
			RendPolyParamList<ListType_Opaque>(pvrrc.global_param_op);

			//Punch Through
			dev->SetRenderState(D3DRS_ALPHATESTENABLE,TRUE);

			dev->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATEREQUAL);

			dev->SetRenderState(D3DRS_ALPHAREF,PT_ALPHA_REF &0xFF);
			if (!GetAsyncKeyState(VK_F2))
			RendPolyParamList<ListType_Punch_Through>(pvrrc.global_param_pt);

			//translucent
			
			dev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
			dev->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATER);
			dev->SetRenderState(D3DRS_ALPHAREF,0);
if (!GetAsyncKeyState(VK_F3))
{
			//if (!UsingAutoSort())
			RendPolyParamList<ListType_Translucent>(pvrrc.global_param_tr);

}
			if (settings.ShowFPS)
			{
				DrawFPS();
			}
			// End the scene
			dev->EndScene();
		}

		// Present the backbuffer contents to the display
		dev->Present( NULL, NULL, NULL, NULL );
	}
	//
	bool running=true;
	cResetEvent rs(false,true);
	cResetEvent re(false,true);
	D3DXMACRO vs_macros[]=
	{
		{"res_x",0},
		{"res_y",0},
		{0,0}	//end of list
	};

	u32 THREADCALL RenderThead(void* param)
	{
		d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
		char temp[2][30];
		D3DPRESENT_PARAMETERS ppar;
		memset(&ppar,0,sizeof(ppar));
/*		
		ppar.MultiSampleType = D3DMULTISAMPLE_NONE;
		ppar.BackBufferCount=3;
		ppar.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
		ppar.BackBufferFormat = D3DFMT_R8G8B8;
		
		ppar.hDeviceWindow=(HWND)Hwnd;
*/
		LoadSettings();
		ppar.SwapEffect = D3DSWAPEFFECT_DISCARD;

		ppar.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;

		ppar.EnableAutoDepthStencil=TRUE;
		ppar.AutoDepthStencilFormat = D3DFMT_D24X8;

		ppar.MultiSampleType = (D3DMULTISAMPLE_TYPE)settings.Enhancements.MultiSampleCount;
		ppar.MultiSampleQuality = settings.Enhancements.MultiSampleQuality;
		
		if (settings.Fullscreen.Enabled)
		{
			ppar.Windowed =   FALSE;
			ppar.BackBufferFormat = D3DFMT_UNKNOWN;

			ppar.BackBufferWidth        = settings.Fullscreen.Res_X;
			ppar.BackBufferHeight       = settings.Fullscreen.Res_Y;
			ppar.BackBufferFormat       = D3DFMT_X8R8G8B8;
			ppar.FullScreen_RefreshRateInHz	=settings.Fullscreen.Refresh_Rate ;
			printf("drkpvr: Initialising fullscreen @%dx%d@%d",ppar.BackBufferWidth,ppar.BackBufferHeight,ppar.FullScreen_RefreshRateInHz);
			IsFullscreen=true;
		}
		else
		{
			ppar.Windowed =   TRUE;
			ppar.BackBufferFormat = D3DFMT_UNKNOWN;
			printf("drkpvr: Initialising windowed");
			IsFullscreen=false;
		}

		printf(" AA:%dx%x\n",ppar.MultiSampleType,ppar.MultiSampleQuality);

		verifyc(d3d9->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,(HWND)emu.WindowHandle,DEV_CREATE_FLAGS,&ppar,&dev));

		sprintf(temp[0],"%d",ppar.BackBufferWidth);
		sprintf(temp[1],"%d",ppar.BackBufferHeight);
		vs_macros[0].Definition=temp[0];
		vs_macros[1].Definition=temp[1];


		if (ppar.MultiSampleType!=D3DMULTISAMPLE_NONE)
			dev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, true);
		//yay , 20 mb -_- =P
		verifyc(dev->CreateVertexBuffer(20*1024*1024,D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | VB_CREATE_FLAGS,0,D3DPOOL_DEFAULT,&vb,0));
		
		verifyc(dev->CreateVertexDeclaration(vertelem,&vdecl));

		ID3DXBuffer* perr;
		ID3DXBuffer* shader;
		
		verifyc(D3DXCompileShaderFromFileA("vs_hlsl.fx",vs_macros,NULL,"VertexShader_main",D3DXGetVertexShaderProfile(dev) , 0, &shader,&perr,&shader_consts));
		if (perr)
		{
			char* text=(char*)perr->GetBufferPointer();
			printf("%s\n",text);
		}
		verifyc(dev->CreateVertexShader((DWORD*)shader->GetBufferPointer(),&compiled_vs));
		
		PrecompilePS();

		D3DXCreateFont( dev, 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, 
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &font );

		while(1)
		{
			rs.Wait();
			if (!running)
				break;
			//render
			DoRender();
			re.Set();
		}
		return 0;
	}

	cThread rth(RenderThead,0);


	union _ISP_BACKGND_T_type
	{
		struct
		{
			u32 tag_offset:3;
			u32 tag_address:21;
			u32 skip:3;
			u32 shadow:1;
			u32 cache_bypass:1;
		};
		u32 full;
	};
	union _ISP_BACKGND_D_type
	{
		u32 i;
		f32 f;
	};

	//functions to read data :p
	f32 vrf(u32 addr)
	{
		return *(f32*)&params.vram[vramlock_ConvOffset32toOffset64(addr)];
	}
	u32 vri(u32 addr)
	{
		return *(u32*)&params.vram[vramlock_ConvOffset32toOffset64(addr)];
	}
	void decode_pvr_vertex(u32 base,u32 ptr,Vertex* to);
	void StartRender()
	{
		SetCurrentPVRRC(PARAM_BASE);
		VertexCount+= pvrrc.verts.used;
		render_end_pending_cycles= pvrrc.verts.used*25;
		if (render_end_pending_cycles<500000)
			render_end_pending_cycles=500000;

		//--BG poly
		u32 param_base=PARAM_BASE & 0xF00000;
		_ISP_BACKGND_D_type bg_d; 
		_ISP_BACKGND_T_type bg_t;

		bg_d.i=ISP_BACKGND_D & ~(0xF);
		bg_t.full=ISP_BACKGND_T;
		
		PolyParam* bgpp=&pvrrc.global_param_op.data[0];
		Vertex* cv=&pvrrc.verts.data[0];

		bool PSVM=FPU_SHAD_SCALE&0x100; //double parameters for volumes

		//Get the strip base
		u32 strip_base=param_base + bg_t.tag_address*4;
		//Calculate the vertex size
		u32 strip_vs=3 + bg_t.skip;
		u32 strip_vert_num=bg_t.tag_offset;

		if (PSVM && bg_t.shadow)
		{
			strip_vs+=bg_t.skip;//2x the size needed :p
		}
		strip_vs*=4;
		//Get vertex ptr
		u32 vertex_ptr=strip_vert_num*strip_vs+strip_base +3*4;
		//now , all the info is ready :p

		bgpp->isp.full=vri(strip_base);
		bgpp->tsp.full=vri(strip_base+4);
		bgpp->tcw.full=vri(strip_base+8);
		bgpp->count=4;
		bgpp->first=0;

		//bgpp->isp.DepthMode=7;// -> this makes things AWFULLY slow .. sometime

		//Set some pcw bits .. i should realy get rid of pcw ..
		bgpp->pcw.UV_16bit=bgpp->isp.UV_16b;
		bgpp->pcw.Gouraud=bgpp->isp.Gouraud;
		bgpp->pcw.Offset=bgpp->isp.Offset;
		bgpp->pcw.Texture=bgpp->isp.Texture;

		for (int i=0;i<3;i++)
		{
			decode_pvr_vertex(strip_base,vertex_ptr,&cv[i]);
			vertex_ptr+=strip_vs;
		}
		cv[0].x=0;
		cv[0].y=480;
		cv[0].z=bg_d.f;

		cv[1].x=0;
		cv[1].y=0;
		cv[1].z=bg_d.f;

		cv[2].x=640;
		cv[2].y=480;
		cv[2].z=bg_d.f;

		cv[3]=cv[2];
		cv[3].x=640;
		cv[3].y=0;
		cv[3].z=bg_d.f;
		
		rs.Set();
		FrameCount++;
	}


	void EndRender()
	{
		re.Wait();
		for (size_t i=0;i<lock_list.size();i++)
		{
			TextureCacheData* tcd=lock_list[i];
			if (tcd->lock_block==0 && tcd->dirty==false)
				tcd->LockVram();
			
		}
		lock_list.clear();
	}

	__declspec(align(16)) static f32 FaceBaseColor[4];
	__declspec(align(16)) static f32 FaceOffsColor[4];
	__declspec(align(16)) static f32 SFaceBaseColor[4];
	__declspec(align(16)) static f32 SFaceOffsColor[4];

	struct VertexDecoder
	{

		//list handling
		__forceinline
		static void StartList(u32 ListType)
		{
			if (ListType==ListType_Opaque)
				CurrentPPlist=&tarc.global_param_op;
			else if (ListType==ListType_Punch_Through)
				CurrentPPlist=&tarc.global_param_pt;
			else if (ListType==ListType_Translucent)
				CurrentPPlist=&tarc.global_param_tr;
			
			CurrentPP=0;
			vert_reappend=0;
		}
		__forceinline
		static void EndList(u32 ListType)
		{
			vert_reappend=0;
			CurrentPP=0;
			CurrentPPlist=0;
		}

		/*
			if (CurrentPP==0 || CurrentPP->pcw.full!=pp->pcw.full || \
		CurrentPP->tcw.full!=pp->tcw.full || \
		CurrentPP->tsp.full!=pp->tsp.full || \
		CurrentPP->isp.full!=pp->isp.full	) \
		*/
		//Polys  -- update code on sprites if that gets updated too --
#define glob_param_bdc \
		{\
			PolyParam* d_pp =CurrentPPlist->Append(); \
			CurrentPP=d_pp;\
			d_pp->first=tarc.verts.used; \
			d_pp->count=0; \
			vert_reappend=0; \
			d_pp->isp=pp->isp; \
			d_pp->tsp=pp->tsp; \
			d_pp->tcw=pp->tcw; \
			d_pp->pcw=pp->pcw; \
		}

#define poly_float_color_(to,a,r,g,b) \
	to[0] = r;	\
	to[1] = g;	\
	to[2] = b;	\
	to[3] = a;

#define poly_float_color(to,src) \
	poly_float_color_(to,pp->src##A,pp->src##R,pp->src##G,pp->src##B)

	//poly param handling
	__forceinline
		static void fastcall AppendPolyParam0(TA_PolyParam0* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam1(TA_PolyParam1* pp)
		{
			glob_param_bdc;
			poly_float_color(FaceBaseColor,FaceColor);
		}
		__forceinline
		static void fastcall AppendPolyParam2A(TA_PolyParam2A* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam2B(TA_PolyParam2B* pp)
		{
			poly_float_color(FaceBaseColor,FaceColor);
			poly_float_color(FaceOffsColor,FaceOffset);
		}
		__forceinline
		static void fastcall AppendPolyParam3(TA_PolyParam3* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam4A(TA_PolyParam4A* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam4B(TA_PolyParam4B* pp)
		{
			poly_float_color(FaceBaseColor,FaceColor0);
		}

		//Poly Strip handling
		//We unite Strips together by dupplicating the [last,first].On odd sized strips
		//a second [first] vert is needed to make sure Culling works fine :)
		__forceinline
		static void StartPolyStrip()
		{
			if (vert_reappend)
			{
				if (CurrentPP->count&1)
				{
					Vertex* cv=tarc.verts.Append(3);
					cv[1].x=cv[0].x=cv[-1].x;
					cv[1].y=cv[0].y=cv[-1].y;
					cv[1].z=cv[0].z=cv[-1].z;
				}
				else
				{
					Vertex* cv=tarc.verts.Append(2);
					cv[0].x=cv[-1].x;//dup prev
					cv[0].y=cv[-1].y;//dup prev
					cv[0].z=cv[-1].z;//dup prev
				}
				vert_reappend=tarc.verts.used-1;
			}
		}
		__forceinline
		static void EndPolyStrip()
		{
			if (vert_reappend)
			{
				Vertex* vert=&tarc.verts.data[vert_reappend];
				vert[0].x=vert[1].x;
				vert[0].y=vert[1].y;
				vert[0].z=vert[1].z;
			}
			vert_reappend=1;
			CurrentPP->count=tarc.verts.used - CurrentPP->first;
		}

		//Poly Vertex handlers
#ifdef scale_type_1
#define z_update(zv) \
	if (tarc.invW_min>zv)\
		tarc.invW_min=zv;\
	if (tarc.invW_max<zv)\
		tarc.invW_max=zv;
#else
	#define z_update(zv)
#endif

	//Append vertex base
#define vert_cvt_base \
	Vertex* cv=tarc.verts.Append();\
	cv->x=vtx->xyz[0];\
	cv->y=vtx->xyz[1];\
	f32 invW=vtx->xyz[2];\
	cv->z=invW;\
	z_update(invW);

	//Resume vertex base (for B part)
#define vert_res_base \
	Vertex* cv=&tarc.verts.data[tarc.verts.used - 1];

	//uv 16/32
#define vert_uv_32(u_name,v_name) \
		cv->u	=	(vtx->u_name);\
		cv->v	=	(vtx->v_name);

#define vert_uv_16(u_name,v_name) \
		cv->u	=	f16(vtx->u_name);\
		cv->v	=	f16(vtx->v_name);

	//Color convertions
#ifdef _float_colors_
	#define vert_packed_color_(to,src) \
	{ \
	u32 t=src; \
		to[2]	= unkpack_bgp_to_float[(u8)(t)];t>>=8;\
		to[1]	= unkpack_bgp_to_float[(u8)(t)];t>>=8;\
		to[0]	= unkpack_bgp_to_float[(u8)(t)];t>>=8;\
		to[3]	= unkpack_bgp_to_float[(u8)(t)];	\
	}

	#define vert_float_color_(to,a,r,g,b) \
			to[0] = r;	\
			to[1] = g;	\
			to[2] = b;	\
			to[3] = a;
#else
	#error OLNY floating color is supported for now
#endif

	//Macros to make thins easyer ;)
#define vert_packed_color(to,src) \
	vert_packed_color_(cv->to,vtx->src);

#define vert_float_color(to,src) \
	vert_float_color_(cv->to,vtx->src##A,vtx->src##R,vtx->src##G,vtx->src##B)

	//Intesity handling
#ifdef _HW_INT_
	//Hardware intesinty handling , we just store the int value
	#define vert_int_base(base) \
		cv->base_int = vtx->base;

	#define vert_int_offs(offs) \
		cv->offset_int = vtx->offs;

	#define vert_int_no_base() \
		cv->base_int = 1;

	#define vert_int_no_offs() \
		cv->offset_int = 1;

	#define vert_face_base_color(baseint) \
		vert_float_color_(cv->col,FaceBaseColor[3],FaceBaseColor[0],FaceBaseColor[1],FaceBaseColor[2]);	 \
		vert_int_base(baseint);

	#define vert_face_offs_color(offsint) \
		vert_float_color_(cv->spc,FaceOffsColor[3],FaceOffsColor[0],FaceOffsColor[1],FaceOffsColor[2]);	 \
		vert_int_offs(offsint);
#else
	//Precaclulated intesinty (saves 8 bytes / vertex)
	#define vert_face_base_color(baseint) \
		vert_float_color_(cv->col,FaceBaseColor[3]*vtx->baseint,FaceBaseColor[0]*vtx->baseint,FaceBaseColor[1]*vtx->baseint,FaceBaseColor[2]*vtx->baseint);

	#define vert_face_offs_color(offsint) \
		vert_float_color_(cv->spc,FaceOffsColor[3]*vtx->offsint,FaceOffsColor[0]*vtx->offsint,FaceOffsColor[1]*vtx->offsint,FaceOffsColor[2]*vtx->offsint);	

	#define vert_int_no_base()
	#define vert_int_no_offs()
#endif



		//(Non-Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex0(TA_Vertex0* vtx)
		{
			vert_cvt_base;

			vert_packed_color(col,BaseCol);

			vert_int_no_base();
		}

		//(Non-Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex1(TA_Vertex1* vtx)
		{
			vert_cvt_base;

			vert_float_color(col,Base);

			vert_int_no_base();
		}

		//(Non-Textured, Intensity)
		__forceinline
		static void AppendPolyVertex2(TA_Vertex2* vtx)
		{
			vert_cvt_base;
			
			vert_face_base_color(BaseInt);
		}

		//(Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex3(TA_Vertex3* vtx)
		{
			vert_cvt_base;
			
			vert_packed_color(col,BaseCol);
			vert_packed_color(spc,OffsCol);

			vert_int_no_base();
			vert_int_no_offs();

			vert_uv_32(u,v);
		}

		//(Textured, Packed Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex4(TA_Vertex4* vtx)
		{
			vert_cvt_base;

			vert_packed_color(col,BaseCol);
			vert_packed_color(spc,OffsCol);

			vert_int_no_base();
			vert_int_no_offs();

			vert_uv_16(u,v);
		}

		//(Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex5A(TA_Vertex5A* vtx)
		{
			vert_cvt_base;

			//Colors are on B
			vert_int_no_base();
			vert_int_no_offs();

			vert_uv_32(u,v);
		}
		__forceinline
		static void AppendPolyVertex5B(TA_Vertex5B* vtx)
		{
			vert_res_base;

			vert_float_color(col,Base);
			vert_float_color(spc,Offs);
		}

		//(Textured, Floating Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex6A(TA_Vertex6A* vtx)
		{
			vert_cvt_base;

			//Colors are on B
			vert_int_no_base();
			vert_int_no_offs();

			vert_uv_16(u,v);
		}
		__forceinline
		static void AppendPolyVertex6B(TA_Vertex6B* vtx)
		{
			vert_res_base;

			vert_float_color(col,Base);
			vert_float_color(spc,Offs);
		}

		//(Textured, Intensity)
		__forceinline
		static void AppendPolyVertex7(TA_Vertex7* vtx)
		{
			vert_cvt_base;

			vert_face_base_color(BaseInt);
			vert_face_offs_color(OffsInt);

			vert_uv_32(u,v);
		}

		//(Textured, Intensity, 16bit UV)
		__forceinline
		static void AppendPolyVertex8(TA_Vertex8* vtx)
		{
			vert_cvt_base;

			vert_face_base_color(BaseInt);
			vert_face_offs_color(OffsInt);

			vert_uv_16(u,v);
			
		}

		//(Non-Textured, Packed Color, with Two Volumes)
		__forceinline
		static void AppendPolyVertex9(TA_Vertex9* vtx)
		{
			vert_cvt_base;

			vert_packed_color(col,BaseCol0);

			vert_int_no_base();
		}

		//(Non-Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex10(TA_Vertex10* vtx)
		{
			vert_cvt_base;
			
			vert_face_base_color(BaseInt0);
		}

		//(Textured, Packed Color,	with Two Volumes)	
		__forceinline
		static void AppendPolyVertex11A(TA_Vertex11A* vtx)
		{
			vert_cvt_base;

			vert_packed_color(col,BaseCol0);
			vert_packed_color(spc,OffsCol0);

			vert_int_no_base();
			vert_int_no_offs();

			vert_uv_32(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex11B(TA_Vertex11B* vtx)
		{
			vert_res_base;

		}

		//(Textured, Packed Color, 16bit UV, with Two Volumes)
		__forceinline
		static void AppendPolyVertex12A(TA_Vertex12A* vtx)
		{
			vert_cvt_base;

			vert_packed_color(col,BaseCol0);
			vert_packed_color(spc,OffsCol0);

			vert_int_no_base();
			vert_int_no_offs();

			vert_uv_16(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex12B(TA_Vertex12B* vtx)
		{
			vert_res_base;

		}

		//(Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex13A(TA_Vertex13A* vtx)
		{
			vert_cvt_base;

			vert_face_base_color(BaseInt0);
			vert_face_offs_color(OffsInt0);

			vert_uv_32(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex13B(TA_Vertex13B* vtx)
		{
			vert_res_base;

		}

		//(Textured, Intensity, 16bit UV, with Two Volumes)
		__forceinline
		static void AppendPolyVertex14A(TA_Vertex14A* vtx)
		{
			vert_cvt_base;

			vert_face_base_color(BaseInt0);
			vert_face_offs_color(OffsInt0);

			vert_uv_16(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex14B(TA_Vertex14B* vtx)
		{
			vert_res_base;

		}

		//Sprites
		__forceinline
		static void AppendSpriteParam(TA_SpriteParam* spr)
		{
			//printf("Sprite\n");
			PolyParam* d_pp =CurrentPPlist->Append(); 
			CurrentPP=d_pp;
			d_pp->first=tarc.verts.used; 
			d_pp->count=0; 
			vert_reappend=0; 
			d_pp->isp=spr->isp; 
			d_pp->tsp=spr->tsp; 
			d_pp->tcw=spr->tcw; 
			d_pp->pcw=spr->pcw; 

			vert_packed_color_(SFaceBaseColor,spr->BaseCol);
			vert_packed_color_(SFaceOffsColor,spr->OffsCol);
		}

#define append_sprite(indx) \
	vert_float_color_(cv[indx].col,SFaceBaseColor[3],SFaceBaseColor[0],SFaceBaseColor[1],SFaceBaseColor[2])\
	vert_float_color_(cv[indx].spc,SFaceOffsColor[3],SFaceOffsColor[0],SFaceOffsColor[1],SFaceOffsColor[2])
	//cv[indx].base_int=1;\
	//cv[indx].offset_int=1;

#define append_sprite_yz(indx,set,st2) \
	cv[indx].y=sv->y##set; \
	cv[indx].z=sv->z##st2; \
	z_update(sv->z##st2);

#define sprite_uv(indx,u_name,v_name) \
		cv[indx].u	=	f16(sv->u_name);\
		cv[indx].v	=	f16(sv->v_name);
		//Sprite Vertex Handlers
		__forceinline
		static void AppendSpriteVertexA(TA_Sprite1A* sv)
		{
			if (CurrentPP->count)
			{
				tarc.verts.Append(2);
			}
			Vertex* cv = tarc.verts.Append(4);
			
			//Fill static stuff
			append_sprite(0);
			append_sprite(1);
			append_sprite(2);
			append_sprite(3);

			cv[2].x=sv->x0;
			cv[2].y=sv->y0;
			cv[2].z=sv->z0;
			z_update(sv->z0);

			cv[3].x=sv->x1;
			cv[3].y=sv->y1;
			cv[3].z=sv->z1;
			z_update(sv->z1);

			cv[1].x=sv->x2;
		}
		__forceinline
		static void AppendSpriteVertexB(TA_Sprite1B* sv)
		{
			vert_res_base;
			cv-=3;

			cv[1].y=sv->y2;
			cv[1].z=sv->z2;
			z_update(sv->z2);

			cv[0].x=sv->x3;
			cv[0].y=sv->y3;
			cv[0].z=sv->z2; //temp , gota calc. 4th Z properly :p


			sprite_uv(2, u0,v0);
			sprite_uv(3, u1,v1);
			sprite_uv(1, u2,v2);
			sprite_uv(0, u0,v2);//or sprite_uv(u2,v0); ?

			if (CurrentPP->count)
			{
				//p1-p2-p3	-> visible , prev strip
				cv[-2].x=cv[-3].x;	//p2-p3-ip3	-> not visible
				cv[-2].y=cv[-3].y;
				cv[-2].z=cv[-3].z;
				
				cv[-1].x=cv[0].x;	//p3-ip3-i0 -> not visible
				cv[-1].y=cv[0].y;
				cv[-1].z=cv[0].z;

				//ip3-i0-0 -> not visible
				//i0-0-1    -> not visible
				//0-1-2
				//1-2-3
				CurrentPP->count+=2;
			}

			CurrentPP->count+=4;
		}

		//ModVolumes
		__forceinline
		static void AppendModVolParam(TA_ModVolParam* modv)
		{

		}

		//ModVol Strip handling
		__forceinline
		static void ModVolStripStart()
		{

		}
		__forceinline
		static void ModVolStripEnd()
		{

		}

		//Mod Volume Vertex handlers
		__forceinline
		static void AppendModVolVertexA(TA_ModVolA* mvv)
		{

		}
		__forceinline
		static void AppendModVolVertexB(TA_ModVolB* mvv)
		{

		}

		//Misc
		__forceinline
		static void ListCont()
		{
			//printf("LC : TA OL base = 0x%X\n",TA_OL_BASE);
			SetCurrentTARC(TA_ISP_BASE);
		}
		__forceinline
		static void ListInit()
		{
			//printf("LI : TA OL base = 0x%X\n",TA_OL_BASE);
			SetCurrentTARC(TA_ISP_BASE);
			tarc.Clear();

			//allocate storage for BG poly
			tarc.global_param_op.Append();
			tarc.verts.Append(4);
		}
		__forceinline
		static void SoftReset()
		{
		}
	};

	//decode a vertex in the native pvr format
	//used for bg poly
	void decode_pvr_vertex(u32 base,u32 ptr,Vertex* cv)
	{
		//ISP
		//TSP
		//TCW
		ISP_TSP isp;
		TSP tsp;
		TCW tcw;

		isp.full=vri(base);
		tsp.full=vri(base+4);
		tcw.full=vri(base+8);

		//XYZ
		//UV
		//Base Col
		//Offset Col

		//XYZ are _allways_ there :)
		cv->x=vrf(ptr);ptr+=4;
		cv->y=vrf(ptr);ptr+=4;
		cv->z=vrf(ptr);ptr+=4;

		if (isp.Texture)
		{	//Do texture , if any
			if (isp.UV_16b)
			{
				u32 uv=vri(ptr);
				cv->u	=	f16((u16)uv);
				cv->v	=	f16((u16)(uv>>16));
				ptr+=4;
			}
			else
			{
				cv->u=vrf(ptr);ptr+=4;
				cv->v=vrf(ptr);ptr+=4;
			}
		}

		//Color
		u32 col=vri(ptr);ptr+=4;
		vert_packed_color_(cv->col,col);
		if (isp.Offset)
		{
			//Intesity color (can be missing too ;p)
			u32 col=vri(ptr);ptr+=4;
			vert_packed_color_(cv->spc,col);
		}
	}



	FifoSplitter<VertexDecoder> TileAccel;

	bool InitRenderer()
	{
		//InitializeCriticalSectionAndSpinCount(&tex_cache_cs,8000);
		for (u32 i=0;i<256;i++)
		{
			unkpack_bgp_to_float[i]=i/255.0f;
		}
		for (u32 i=0;i<rcnt.size();i++)
		{
			rcnt[i].Free();
		}
		rcnt.clear();
		tarc.Address=0xFFFFFFFF;
		tarc.Init();
		//pvrrc needs no init , it is ALLWAYS copied from a valid tarc :)
		return TileAccel.Init();
	}

	void TermRenderer()
	{
		//DeleteCriticalSection(&tex_cache_cs); 
		for (u32 i=0;i<rcnt.size();i++)
		{
			rcnt[i].Free();
		}
		rcnt.clear();

		TileAccel.Term();
	}

	void ResetRenderer(bool Manual)
	{
		TileAccel.Reset(Manual);
		VertexCount=0;
		FrameCount=0;
	}

	bool ThreadStart()
	{
		//rth.Start();
		return true;
	}

	void ThreadEnd()
	{
		printf("ThreadEnd\n");
		running=false;
		rth.Start();
		rs.Set();
		rth.WaitToEnd(0xFFFFFFFF);
	}

	void ListCont()
	{
		TileAccel.ListCont();
	}
	void ListInit()
	{
		TileAccel.ListInit();
	}
	void SoftReset()
	{
		TileAccel.SoftReset();
	}
}
#endif