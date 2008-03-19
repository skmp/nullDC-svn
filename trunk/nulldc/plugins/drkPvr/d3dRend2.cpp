#define _WIN32_WINNT 0x0500

#if _DEBUG
	#define D3D_DEBUG_INFO 
#endif
#include "d3drend2.h"

#include <d3dx10.h>
#include <algorithm>
//#include "d3dRend.h"
#include "windows.h"
#include "regs.h"
#include <vector>
#include <mmintrin.h>
#include <xmmintrin.h>

//This renderer is meant for accuracy-over-speed :)
//-TA emulation, TA will generate pvr displaylists that the pvr code will handle
//-No fixed function code, sm2/3/(4?)
//-Palette lookups @ textures [allways]
//-Secondary 32F z buffer for per pixel transp. sorting & mod volumes
//-Tile-based rendering:
// ? multy-render threads ?
// Unified texture cache
// block readbacks to sysmem
// rendering by lock & fb upload


#if REND_API == REND_D3D_V2

#pragma comment(lib, "d3d10.lib") 
#if _DEBUG
	#pragma comment(lib, "d3dx10d.lib") 
#else
	#pragma comment(lib, "d3dx10.lib") 
#endif


using namespace TASplitter;

namespace Direct3DRenderer2
{
	struct TA_context;
	struct IRenderer
	{
		void (*Init)();
		void (*Term)();
		void (*Render)();
	};

	void GetIRenderer(IRenderer* d);

	IRenderer rend;

	//ID3D10Device* dev;
	RECT window_rect;

	struct VertexDecoderEx;
	
	FifoSplitterEx<VertexDecoderEx> TileAccel;

	
	u32 FrameNumber=0;
	bool IsFullscreen=false;
	wchar fps_text[512];
	
	float res_scale[4]={0,0,320,-240};
	float fb_scale[2]={1,1};
	float current_scalef[4];

	void SetFpsText(wchar* text)
	{
		wcscpy(fps_text,text);
		if (!IsFullscreen)
		{
			SetWindowText((HWND)emu.GetRenderTarget(), fps_text);
		}
	}
	void HandleEvent(u32 evid,void* p)
	{
		
	}
	void SetRenderRect(float* rect,bool do_clear)
	{
		res_scale[0]=rect[0];
		res_scale[1]=rect[1];

		res_scale[2]=rect[2]/2;
		res_scale[3]=-rect[3]/2;
	}
	void SetFBScale(float x,float y)
	{
		fb_scale[0]=x;
		fb_scale[1]=y;
	}
	
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

	struct TextureCacheData;
	std::vector<TextureCacheData*> lock_list;
	//Texture Cache :)
	struct TextureCacheData
	{
		TCW tcw;TSP tsp;
		#if d3d10tex
		IDirect3DTexture9* Texture;
		#endif
		u32 Lookups;
		u32 Updates;
		u32 LastUsed;
		u32 w,h;
		u32 size;
		bool dirty;
		u32 pal_rev;
		vram_block* lock_block;

		//Releases any resources , EXEPT the texture :)
		void Destroy()
		{
			if (lock_block)
				params.vram_unlock(lock_block);
			lock_block=0;
		}
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
#if d3d10tex
//			verify(dirty);
//			verify(lock_block==0);

			LastUsed=FrameNumber;
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

			verifyc(Texture->LockRect(0,&rect,NULL,0 /* | D3DLOCK_DISCARD*/));
			

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

				PAL4toX444_TW(&pbt,(u8*)&params.vram[sa],w,h);

				break;
			case 6:
				//6	8 BPP Palette	Palette texture with 8 bits/pixel
				verify(tcw.PAL.VQ_Comp==0);
				if (tcw.NO_PAL.MipMapped)
							sa+=MipPoint[tsp.TexU]<<2;
				palette_index = (tcw.PAL.PalSelect<<4)&(~0xFF);
				pal_rev=pal_rev_256[tcw.PAL.PalSelect>>4];

				PAL8toX444_TW(&pbt,(u8*)&params.vram[sa],w,h);
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
#endif
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
		tf->w=8<<tsp.TexU;
		tf->h=8<<tsp.TexV;
		tf->tsp=tsp;
		tf->tcw=tcw;
		tf->dirty=true;
		tf->lock_block=0;
		//tf->Texture=0;
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

#if d3d10tex
	IDirect3DTexture9* __fastcall GetTexture(TSP tsp,TCW tcw)
	{	
		u32 addr=(tcw.NO_PAL.TexAddr<<3) & 0x7FFFFF;
		if (addr==rtt_address)
		{
			rtt_FrameNumber=FrameNumber;
			return rtt_texture;
		}

		TextureCacheData* tf = TexCache.Find(tcw.full,tsp.full);
		if (tf)
		{
			tf->LastUsed=FrameNumber;
			if (tf->dirty)
			{
				tf->Update();
			}
			tf->Lookups++;

			return tf->Texture;
		}
		else
		{
			tf = GenText(tsp,tcw);
			return tf->Texture;
		}
		return 0;
	}
#endif
	void VramLockedWrite(vram_block* bl)
	{
		TextureCacheData* tcd = (TextureCacheData*)bl->userdata;
		tcd->dirty=true;
		tcd->lock_block=0;

		params.vram_unlock(bl);
	}
	extern cThread rth;

	//use that someday
	void VBlank()
	{
		FrameNumber++;
		//we need to actualy draw the image here :)
		//dev->
	}

	//Vertex storage types
	struct VertexEx
	{
		PCW pcw;

		union
		{
			u8 Raw[64-4];

			TA_Vertex0		vtx0;
			TA_Vertex1		vtx1;
			TA_Vertex2		vtx2;
			TA_Vertex3		vtx3;
			TA_Vertex4		vtx4;

			struct
			{
				TA_Vertex5A			vtx5A;
				TA_Vertex5B			vtx5B;
			};

			struct
			{
				TA_Vertex6A			vtx6A;
				TA_Vertex6B			vtx6B;
			};

			TA_Vertex7		vtx7;
			TA_Vertex8		vtx8;
			TA_Vertex9		vtx9;
			TA_Vertex10		vtx10;



			struct
			{
				TA_Vertex11A	vtx11A;
				TA_Vertex11B	vtx11B;
			};


			struct
			{
				TA_Vertex12A	vtx12A;
				TA_Vertex12B	vtx12B;
			};

			struct
			{
				TA_Vertex13A	vtx13A;
				TA_Vertex13B	vtx13B;
			};

			struct
			{
				TA_Vertex14A	vtx14A;
				TA_Vertex14B	vtx14B;
			};

			struct
			{
				TA_Sprite0A		spr0A;
				TA_Sprite0B		spr0B;
			};

			struct
			{
				TA_Sprite1A		spr1A;
				TA_Sprite1B		spr1B;
			};
		};

	};

	struct PolyParam
	{
		union
		{
		VertexEx* first;	//entry index , holds vertex/pos data
		u32		  vbindx;
		};
		u32 count;
		float zvZ;
		u32 tileclip;

		PCW pcw;
		ISP_TSP isp;
		TSP tsp;
		TCW tcw;
		//tsp2 ? tcw2 ?

		float bcol[4];
		float boff[4];
	};
	
	PolyParam* CurrentPP;

	//vertex lists
	struct TA_context
	{
		u32 Address;
		u32 LastUsed;

		List3<VertexEx> vrtx;

		List<PolyParam> global_param_op;
		List<PolyParam> global_param_pt;
		List<PolyParam> global_param_tr;

		void Init()
		{
			vrtx.Init();
			global_param_op.Init();
			global_param_pt.Init();
			global_param_tr.Init();
		}
		void Clear()
		{
			vrtx.Clear();
			global_param_op.Clear();
			global_param_pt.Clear();
			global_param_tr.Clear();
		}
		void Free()
		{
			vrtx.Free();
			global_param_op.Free();
			global_param_pt.Free();
			global_param_tr.Free();
		}
	};
	


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



	List<PolyParam>* CurrentPPlist;
	
	bool UsingAutoSort()
	{
		if (((FPU_PARAM_CFG>>21)&1) == 0)
			return ((ISP_FEED_CFG&1)==0);
		else
			return 0;//( (vri(REGION_BASE)>>29) & 1) == 0;
	}


	float cur_pal_index[4]={0,0,0,1};

	//realy only uses bit0, destroys all of em atm :]
	void SetTileClip(u32 val)
	{
				
	}
	//Set the state for a PolyParam
	template <u32 Type>
	__forceinline
	void SetGPState(PolyParam* gp)
	{	/*
		if (gp->tsp.DstSelect ||
			gp->tsp.SrcSelect)
			printf("DstSelect  DstSelect\n"); */

		SetTileClip(gp->tileclip);
		//has to preserve cache_tsp/cache_isp
		//can freely use cache_tcw

		SetGPState_ps(gp);
/*
		if ((gp->tcw.full != cache_tcw.full) || (gp->tsp.full!=cache_tsp.full))
		{
			cache_tcw=gp->tcw;

			if ( gp->tsp.FilterMode == 0 ||  ( gp->tcw.PAL.PixelFmt==5|| gp->tcw.PAL.PixelFmt==6) )
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

			if (gp->tsp.full!=cache_tsp.full)
			{
				cache_tsp=gp->tsp;

				if (Type==ListType_Translucent)
				{
					dev->SetRenderState(D3DRS_SRCBLEND, SrcBlendGL[gp->tsp.SrcInstr]);
					dev->SetRenderState(D3DRS_DESTBLEND, DstBlendGL[gp->tsp.DstInstr]);
				}

				SetTexMode<D3DSAMP_ADDRESSV>(gp->tsp.ClampV,gp->tsp.FlipV);
				SetTexMode<D3DSAMP_ADDRESSU>(gp->tsp.ClampU,gp->tsp.FlipU);
			}

			if (gp->pcw.Texture)
			{
				IDirect3DTexture9* tex=GetTexture(gp->tsp,gp->tcw);
				dev->SetTexture(0,tex);
				float tsz[4];
				tsz[0]=8<<gp->tsp.TexU;
				tsz[2]=1/tsz[0];
				tsz[1]=8<<gp->tsp.TexV;
				tsz[3]=1/tsz[1];

				dev->SetPixelShaderConstantF(1,tsz,1);
				dev->SetVertexShaderConstantF(3,tsz,1);
			}
		}

		if (gp->isp.full!= cache_isp.full)
		{
			cache_isp.full=gp->isp.full;
			//set cull mode !
			dev->SetRenderState(D3DRS_CULLMODE,CullMode[gp->isp.CullMode]);
			//set Z mode !
			if (Type==ListType_Opaque)
			{
				dev->SetRenderState(D3DRS_ZFUNC,Zfunction[gp->isp.DepthMode]);
			}
			else if (Type==ListType_Translucent)
			{
				if (dosort)
					dev->SetRenderState(D3DRS_ZFUNC,Zfunction[6]); // : GEQ
				else
					dev->SetRenderState(D3DRS_ZFUNC,Zfunction[gp->isp.DepthMode]);
			}
			else
			{
				//gp->isp.DepthMode=6;
				dev->SetRenderState(D3DRS_ZFUNC,Zfunction[6]); //PT : LEQ //GEQ ?!?! wtf ? seems like the docs ARE wrong on this one =P
			}

			dev->SetRenderState(D3DRS_ZWRITEENABLE,gp->isp.ZWriteDis==0);

			
		}
		*/
	}
	//Render a PolyParam
	template <u32 Type>
	__forceinline
	void RendStrips(PolyParam* gp)
	{
			SetGPState<Type>(gp);
			if (gp->count>2)//0 vert polys ? why does games even bother sending em  ? =P
			{		
				dev->DrawPrimitive(D3DPT_TRIANGLESTRIP,gp->vbindx ,	gp->count-2);
			}
	}
	//Render a PolyParam list
	template <u32 Type>
	void RendPolyParamList(List<PolyParam>& gpl)
	{
		if (gpl.used==0)
			return;
		//we want at least 1 PParam

		for (u32 i=0;i<gpl.used;i++)
		{		
			RendStrips<Type>(&gpl.data[i]);
		}
	}
	
	//
	void DrawFPS()
	{
		// Create a colour for the text
		//D3DCOLOR fontColor = D3DCOLOR_ARGB(255,0x18,0xFF,0);  

		// Create a rectangle to indicate where on the screen it should be drawn
		RECT rct;
		rct.left=2;
		rct.right=780;
		rct.top=10;
		rct.bottom=rct.top+30;

		//font->
		// Draw some text
		//i need to set a new PS/FP state here .. meh ...
		//font->DrawText(NULL, fps_text, -1, &rct, 0, fontColor );
		//DrawText(
	}

	void DrawOSD()
	{
#if d3d10tex
		//dev->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);
		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		/*
		dev->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
		dev->SetRenderState( D3DRS_ALPHAREF,         0x08 );
		dev->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );
		dev->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
		dev->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
		dev->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
		dev->SetRenderState( D3DRS_CLIPPING,         TRUE );
		dev->SetRenderState( D3DRS_CLIPPLANEENABLE,  FALSE );
		dev->SetRenderState( D3DRS_VERTEXBLEND,      D3DVBF_DISABLE );
		dev->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
		dev->SetRenderState( D3DRS_FOGENABLE,        FALSE );
		dev->SetRenderState( D3DRS_COLORWRITEENABLE,
		D3DCOLORWRITEENABLE_RED  | D3DCOLORWRITEENABLE_GREEN |
		D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA );
		*/
		if (settings.OSD.ShowFPS)
		{
			DrawFPS();
		}
		if (settings.OSD.ShowStats)
		{
		}
#endif
	}
	//
	void UpdatePaletteTexure()
	{
		palette_update();
#if d3d10tex
		D3DLOCKED_RECT rect;
		//pal is organised as 16x64 texture
		pal_texture->LockRect(0,&rect,NULL,D3DLOCK_DISCARD);

		u8* tex=(u8*)rect.pBits;
		u8* src=(u8*)palette_lut;

		for (int i=0;i<64;i++)
		{
			memcpy(tex,src,16*4);
			tex+=rect.Pitch;
			src+=16*4;
		}
		pal_texture->UnlockRect(0);
#endif
	}
	//

	//
	volatile bool running=false;

	cResetEvent rs(false,true);
	cResetEvent re(false,true);

	void ListModes(void(* callback)(u32 w,u32 h,u32 rr))
	{
		
	}
	u32 THREADCALL RenderThead(void* param)
	{
		rend.Init();
		
		while(1)
		{
			rs.Wait();
			if (!running)
				break;
			SetCurrentPVRRC(PARAM_BASE);
			rend.Render();
			//render
			re.Set();
		}

		rend.Term();

		return 0;
	}

	cThread rth(RenderThead,0);
	
	void StartRender()
	{
		SetCurrentPVRRC(PARAM_BASE);
		VertexCount+= pvrrc.vrtx.GetUsedCount();
		render_end_pending_cycles= pvrrc.vrtx.GetUsedCount()*25;
		if (render_end_pending_cycles<500000)
			render_end_pending_cycles=500000;

		rs.Set();
		FrameCount++;
	}


	void EndRender()
	{
		re.Wait();
	}

	//__declspec(align(16)) static f32 FaceBaseColor[4];
	//__declspec(align(16)) static f32 FaceOffsColor[4];
	//__declspec(align(16)) static f32 SFaceBaseColor[4];
	//__declspec(align(16)) static f32 SFaceOffsColor[4];

	struct VertexDecoderEx
	{

		__forceinline
		static void SetTileClip(u32 xmin,u32 ymin,u32 xmax,u32 ymax)
		{
	
		}
		__forceinline
		static void TileClipMode(u32 mode)
		{
			//tileclip_val=(tileclip_val&(~0xF0000000)) | (mode<<28);
		}
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
		}
		__forceinline
		static void EndList(u32 ListType)
		{
			CurrentPPlist=0;
		}

		//Polys  -- update code on sprites if that gets updated too --
#define glob_param_bdc \
		{ \
			PolyParam* d_pp=CurrentPP =CurrentPPlist->Append(); \
			d_pp->first=tarc.vrtx.Ptr(); \
			d_pp->count=0; \
			d_pp->pcw=pp->pcw; \
			d_pp->isp=pp->isp; \
			d_pp->tsp=pp->tsp; \
			d_pp->tcw=pp->tcw; \
		}


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
			//poly_float_color(FaceBaseColor,FaceColor);
		}
		__forceinline
		static void fastcall AppendPolyParam2A(TA_PolyParam2A* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam2B(TA_PolyParam2B* pp)
		{
			//poly_float_color(FaceBaseColor,FaceColor);
			//poly_float_color(FaceOffsColor,FaceOffset);
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
			//poly_float_color(FaceBaseColor,FaceColor0);
		}

		//Poly Vertex handlers
		//allocate a new vertex and copy 64b to it
		__forceinline
			static void AppendPolyVertexData(void* data)
		{
			__m128* src=(__m128*)data;
			__m128* dst=(__m128*)tarc.vrtx.Append();

			dst[0]=src[0];
			dst[1]=src[1];

			dst[2]=src[2];
			dst[3]=src[3];
		}
		//allocate a new vertex and copy 32b to it
		__forceinline
			static void AppendPolyVertexData32(void* data)
		{
			__m128* src=(__m128*)data;
			__m128* dst=(__m128*)tarc.vrtx.Append();

			dst[0]=src[0];
			dst[1]=src[1];
		}
		//copy 32 bytes to the second half of the prev. allocated vertex.May be called after AppendPolyVertexData32
		__forceinline
			static void AppendPolyVertexData32Part(void* data)
		{
			__m128* src=(__m128*)data;
			__m128* dst=(__m128*)tarc.vrtx.ptr;

			dst[2]=src[0];
			dst[3]=src[1];
		}

#ifdef I_ADDED_THE_SPRITES_TO_EX
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
			d_pp->tileclip=tileclip_val;

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
				Vertex* old=((Vertex*)tarc.verts.ptr);
				Vertex* cv=tarc.verts.Guarantee(6,2);
				cv[0].x=old[-1].x;//dup prev
				cv[0].y=old[-1].y;//dup prev
				cv[0].z=old[-1].z;//dup prev
				vert_reappend=(Vertex*)tarc.verts.ptr;
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
				Vertex* vert=vert_reappend;
				vert[-1].x=vert[0].x;
				vert[-1].y=vert[0].y;
				vert[-1].z=vert[0].z;
				CurrentPP->count+=2;
			}
			
			CurrentPP->count+=4;
		}

		//ModVolumes

		//Mod Volume Vertex handlers
		static void StartModVol(TA_ModVolParam* param)
		{
			if (TileAccel.CurrentList!=ListType_Opaque_Modifier_Volume)
				return;
			if (param->pcw.Volume || param->isp.DepthMode)
			{
				//if (lmr_count)
				//{
					*tarc.modsz.Append()=lmr_count+1;
					lmr_count=-1;
				//}
			}
		}
		__forceinline
		static void AppendModVolVertexA(TA_ModVolA* mvv)
		{
			if (TileAccel.CurrentList!=ListType_Opaque_Modifier_Volume)
				return;
			lmr=tarc.modtrig.Append();

			lmr->x0=mvv->x0;
			lmr->y0=mvv->y0;
			lmr->z0=mvv->z0;
			lmr->x1=mvv->x1;
			lmr->y1=mvv->y1;
			lmr->z1=mvv->z1;
			lmr->x2=mvv->x2;

			lmr_count++;
		}
		__forceinline
		static void AppendModVolVertexB(TA_ModVolB* mvv)
		{
			if (TileAccel.CurrentList!=ListType_Opaque_Modifier_Volume)
				return;
			lmr->y2=mvv->y2;
			lmr->z2=mvv->z2;
		}

#endif
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
#ifdef I_ALSO_DID_BGP
			//allocate storage for BG poly
			tarc.global_param_op.Append();
			BGPoly=tarc.verts.Append(4);
#endif
		}
		__forceinline
		static void SoftReset()
		{
		}
	};

	bool InitRenderer()
	{
		GetIRenderer(&rend);
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
		for (u32 i=0;i<rcnt.size();i++)
		{
			rcnt[i].Free();
		}
		rcnt.clear();

		TileAccel.Term();
		//free all textures
		verify(TexCache.pfirst==0);
	}

	void ResetRenderer(bool Manual)
	{
		TileAccel.Reset(Manual);
		VertexCount=0;
		FrameCount=0;
	}

	bool ThreadStart()
	{
		running=true;
		rth.Start();
		return true;
	}

	void ThreadEnd()
	{
		printf("ThreadEnd\n");
		if (running)
		{
			verify(!rth.ended);
			running=false;
			//if (!rth.ended)
			{
				rs.Set();
				rth.WaitToEnd(0xFFFFFFFF);

				if (IsFullscreen)
				{
					SetWindowPos((HWND)emu.GetRenderTarget(),HWND_NOTOPMOST,window_rect.left,window_rect.top,window_rect.right-window_rect.left
						,window_rect.bottom-window_rect.top,0);
					//printf("Restored window : %d,%d : %dx%d\n",window_rect.left,window_rect.top,window_rect.right-window_rect.left
					//	,window_rect.bottom-window_rect.top);
				}
			}
		}
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

	struct
	{
		DXGI_SWAP_CHAIN_DESC       sd;
		ID3D10Device*              dev;
		IDXGISwapChain*            SwapChain;
		ID3D10RenderTargetView*    RenderTargetView;
		ID3D10Buffer*              vb;
		ID3D10InputLayout*         d3dvtxLayout;
		ID3D10VertexShader*		   vs10;
		ID3D10PixelShader*         ps10;
		ID3D10Blob*				   pVSBuf;
		ID3D10Blob*                pPSBuf;
	} r10;

	ID3D10Device*& dev=r10.dev;
	IDXGISwapChain*& SwapChain=r10.SwapChain;
	ID3D10Buffer*& vb=r10.vb;

	// Define the input layout
    D3D10_INPUT_ELEMENT_DESC vtxLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4, D3D10_INPUT_PER_VERTEX_DATA, 0 },  
        
    };
	
	void r10_CreateShaders()
	{
		DWORD dwShaderFlags = 0;
		#if defined( DEBUG ) || defined( _DEBUG )
				// Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
				// Setting this flag improves the shader debugging experience, but still allows 
				// the shaders to be optimized and to run exactly the way they will run in 
				// the release configuration of this program.
				dwShaderFlags |= D3D10_SHADER_DEBUG;
		#endif

		ID3D10Blob* pPSErr = NULL;

		wchar* vsfile=L"vs10_hlsl.fx";
		wchar* psfile=L"ps10_hlsl.fx";

		if (FAILED(
			D3DX10CompileFromFile(vsfile, NULL, NULL, "VertexShader_main", "vs_4_0", dwShaderFlags, NULL, NULL, &r10.pVSBuf, &pPSErr, NULL )))
		{
			printf("%s\n",pPSErr->GetBufferPointer());
			return;
		}
		
		if (FAILED( dev->CreateVertexShader( (DWORD*) r10.pVSBuf->GetBufferPointer(), r10.pVSBuf->GetBufferSize(), &r10.vs10 ) ))
			return;

		// Compile the PS from the file
		ID3D10Blob* pPSBuf = NULL;
		if (FAILED(D3DX10CompileFromFile(psfile, NULL, NULL, "PixelShader_ShadeCol", "ps_4_0", dwShaderFlags, NULL, NULL, &r10.pPSBuf, &pPSErr, NULL ) ))
		{
			printf("%s\n",pPSErr->GetBufferPointer());
			return;
		}
		if (FAILED( dev->CreatePixelShader( (DWORD*) r10.pPSBuf->GetBufferPointer(), r10.pPSBuf->GetBufferSize(), &r10.ps10 ) ))
			return;
	}
	void r10_Init()
	{
		memset(&r10,0,sizeof(r10));

		DXGI_SWAP_CHAIN_DESC& sd=r10.sd;

		sd.BufferCount = 1;
		sd.BufferDesc.Width = 640;
		sd.BufferDesc.Height = 480;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = (HWND)emu.GetRenderTarget();
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		HRESULT hr = D3D10CreateDeviceAndSwapChain( NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, D3D10_CREATE_DEVICE_DEBUG , 	D3D10_SDK_VERSION, &sd, &SwapChain, &dev );

		if( FAILED(hr) )
			return ;

		// Create a render target view
		ID3D10Texture2D *pBuffer;
		hr = SwapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), (LPVOID*)&pBuffer );

		if( FAILED(hr) )
			return ;

		hr = dev->CreateRenderTargetView( pBuffer, NULL, &r10.RenderTargetView );
		pBuffer->Release();

		if( FAILED(hr) )
			return;

		dev->OMSetRenderTargets( 1, &r10.RenderTargetView, NULL );

		// Setup the viewport
		D3D10_VIEWPORT vp;
		vp.Width = 640;
		vp.Height = 480;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		dev->RSSetViewports( 1, &vp );

		r10_CreateShaders();

		// Create the input layout
		hr = dev->CreateInputLayout( vtxLayout, 1, r10.pVSBuf->GetBufferPointer(), r10.pVSBuf->GetBufferSize(), &r10.d3dvtxLayout );
		if( FAILED(hr) )
			return ;

		// Set the input layout
		dev->IASetInputLayout( r10.d3dvtxLayout );

		//Setup the vertex buffer
		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof( VertexEx ) * 1024*32;
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE ;
		bd.MiscFlags = 0;
		hr = dev->CreateBuffer( &bd, 0, &vb );
		if( FAILED(hr) )
			return;

		UINT stride = sizeof( VertexEx );
		UINT offset = 0;
		dev->IASetVertexBuffers( 0, 1, &vb, &stride, &offset );

		dev->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

		//Set the shaders
		
		dev->PSSetShader(r10.ps10);
		dev->GSSetShader(0);
		dev->VSSetShader(r10.vs10);
	}
	void r10_Term()
	{
	}
	void r10_Render()
	{
		if (pvrrc.vrtx.allocate_list_ptr->size()==0)
			return;

		float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
		dev->ClearRenderTargetView(r10.RenderTargetView,ClearColor);
		
		void* pData;
		vb->Map(D3D10_MAP_WRITE_DISCARD,0,&pData);
		memcpy(pData,pvrrc.vrtx.allocate_list_ptr[0][0],ChunkSize);
		vb->Unmap();


		//for (u32 i=0;i<pvrrc.global_param_op.used;i++)
		{
		//	if (pvrrc.global_param_op.data[i].count<3)
			//	continue;
			dev->Draw(60,0);
		}
		SwapChain->Present(0,0);
	}
	

	void GetIRenderer(IRenderer* d)
	{
		d->Init=r10_Init;
		d->Term=r10_Term;
		d->Render=r10_Render;
	}
}
#endif
