#include <d3dx9.h>

#include "nullRend.h"

#include "d3dRend.h"
#include "windows.h"
//#include "gl\gl.h"
#include "regs.h"
#include <vector>

//#include <D3dx9shader.h>

using namespace TASplitter;


namespace Direct3DRenderer
{
	IDirect3D9* d3d9;
	IDirect3DDevice9* dev;
	IDirect3DVertexBuffer9* vb;
	IDirect3DVertexShader9* CompiledShader;
	IDirect3DPixelShader9* CompiledPShader;
	ID3DXConstantTable* shader_consts;
	
	char Shader[] = 
"struct vertex { float4 pos : POSITION; float4 col : COLOR; float4 uv : TEXCOORD0; };"
"float W_min: register(c0);float W_max: register(c1);"
"  vertex VertexShader_Tutorial_1(in vertex vtx) {"
"vtx.pos.x=(vtx.pos.x/320)-1;"
"vtx.pos.y=-(vtx.pos.y/240)+1;"

"vtx.uv.xy*=vtx.pos.z;"
"vtx.uv.z=0;"
"vtx.uv.w=vtx.pos.z;" 

"vtx.pos.z=((1/clamp(0.0000001,10000000,vtx.pos.z))-W_min)/W_max;"
"vtx.pos.z=clamp(0, 1, vtx.pos.z);"
"vtx.pos.w=1;"
"return vtx; "
"}";

		char Pixel[] = 
			"  float4 VertexShader_Tutorial_1(in float4 zz:COLOR0 ):COLOR0 { return zz; }";

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

	const static u32 CullMode[]= 
	{
		D3DCULL_NONE,	//0	No culling	no culling
		D3DCULL_NONE,	//1	Cull if Small	Cull if	( |det| < fpu_cull_val )

		//wtf ?
		D3DCULL_NONE /*D3DCULL_CCW*/,		//2	Cull if Negative	Cull if 	( |det| < 0 ) or
						//( |det| < fpu_cull_val )
		D3DCULL_NONE /*D3DCULL_CW*/,		//3	Cull if Positive	Cull if 	( |det| > 0 ) or
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
		D3DCMP_GREATER,				//1	Less
		D3DCMP_EQUAL,				//2	Equal
		D3DCMP_GREATEREQUAL,		//3	Less Or Equal
		D3DCMP_LESS,				//4	Greater
		D3DCMP_NOTEQUAL,			//5	Not Equal
		D3DCMP_LESSEQUAL,			//6	Greater Or Equal
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
	u32 temp_tex_buffer[1024*1024*4];
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
		vram_block* lock_block;

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
			verify(dirty);
			verify(lock_block==0);

			Updates++;
			dirty=false;

			u32 sa=(tcw.NO_PAL.TexAddr<<3) & 0x7FFFFF;

			PixelBuffer pbt;
			pbt.p_buffer_start=pbt.p_current_line=temp_tex_buffer;
			pbt.pixels_per_line=w;

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
				}
				else
				{
					//if (tcw.NO_PAL.VQ_Comp)
						//goto redo_argb;
					//if (tcw.NO_PAL.MipMapped)
					//	sa+=MipPoint[tsp.TexU]<<3;
					//YUV422to8888_TW(&pbt,(u16*)&params.vram[sa],w,h);
					twidle_tex(YUV422);
					//verify(tcw.NO_PAL.VQ_Comp==0);
					//it cant be VQ , can it ?
				}
				break;
				//4	Bump Map	16 bits/pixel; S value: 8 bits; R value: 8 bits
			case 5:
				//5	4 BPP Palette	Palette texture with 4 bits/pixel
				verify(tcw.PAL.VQ_Comp==0);
				if (tcw.NO_PAL.MipMapped)
							sa+=MipPoint[tsp.TexU]<<2;
				//tcw.PAL.PalSelect<<4
				PAL4to8888_TW(&pbt,(u8*)&params.vram[sa],w,h);
				break;
			case 6:
				//6	8 BPP Palette	Palette texture with 8 bits/pixel
				verify(tcw.PAL.VQ_Comp==0);
				if (tcw.NO_PAL.MipMapped)
							sa+=MipPoint[tsp.TexU]<<2;
				//(tcw.PAL.PalSelect<<4)&(~0xFF)
				PAL8to8888_TW(&pbt,(u8*)&params.vram[sa],w,h);
				break;
			default:
				printf("Unhandled texture\n");
				memset(temp_tex_buffer,0xFFFFFFFF,w*h*4);
			}

			PrintTextureName();
			u32 ea=sa+w*h*2;
			if (ea>=(8*1024*1024))
			{
				ea=(8*1024*1024)-1;
			}
			//(u32 start_offset64,u32 end_offset64,void* userdata);
			lock_block = params.vram_lock_64(sa,ea,this);

			if (Texture==0)
			{
				verifyc(dev->CreateTexture(w,h,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&Texture,0));
			}
			D3DLOCKED_RECT rect;

			verifyc(Texture->LockRect(0,&rect,NULL,D3DLOCK_NOSYSLOCK));

			u32* texture_data= (u32*)rect.pBits;
			u32* source = temp_tex_buffer;

			for (u32 y=0;y<h;y++)
			{
				for (u32 x=0;x<w;x++)
				{
					texture_data[x]=*source++;
				}
				texture_data+=rect.Pitch/4;
			}
			Texture->UnlockRect(0);
			
		/*
			char file[512];

			sprintf(file,"d:\\textures\\0x%x_%d_%s_VQ%d_TW%d_MM%d_.jpg",Start,Lookups,texFormatName[tcw.NO_PAL.PixelFmt]
			,tcw.NO_PAL.VQ_Comp,tcw.NO_PAL.ScanOrder,tcw.NO_PAL.MipMapped);
			D3DXSaveTextureToFileA( file,D3DXIFF_JPG,Texture,0);*/
			
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
	
	void VramLockedWrite(vram_block* bl)
	{
		TextureCacheData* tcd = (TextureCacheData*)bl->userdata;
		tcd->dirty=true;
		tcd->lock_block=0;
		if (tcd->Updates==0)
		{
			tcd->Texture->Release();
			tcd->Texture=0;
		}
		params.vram_unlock(bl);
	}

	//use that someday
	void VBlank()
	{
		//we need to actualy draw the image here :)
		//dev->
	}

	//Vertex storage types
	//64B
	struct Vertex
	{
		/*
		//0
		float xyz[3];

		//12
		u32 nil;//padding

		//16
		float col[4];

		//32
		//tex cords if texture
		float uv[4];

		//48
		//offset color
		float col_offset[4];
*/
		//64
		float x,y,z;

		float col[4];
		float u,v;
		//unsigned int uiRGBA;
		//unsigned int uiSpecularRGBA;
		
	};
	const D3DVERTEXELEMENT9 vertelem[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,0},
		{0, 12, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		{0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,0},
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

		void Clear()
		{
			verts.Clear();
			global_param_op.Clear();
			global_param_pt.Clear();
			global_param_tr.Clear();
			invW_min= 1000000.0f;
			invW_max=-1000000.0f;
		}
	};

	
	TA_context tarc;
	TA_context pvrrc;

	std::vector<TA_context> rcnt;
	inline u32 FindRC(u32 addr)
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
	void SetCurrentTARC(u32 addr)
	{
		return;
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
			rcnt.push_back(tarc);
		}
	}
	void SetCurrentPVRRC(u32 addr)
	{
		//return;
	//	printf("SetCurrentPVRRC:0x%X\n",addr);
		//if (addr==tarc.Address)
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
	template <u32 Type>
	__forceinline
	void RendStrips(PolyParam* gp)
	{
		if (gp->count>2)
		{	
			//0 vert polys ? why does games even bother sending em  ? =P
			if (gp->pcw.Texture)
			{
				IDirect3DTexture9* tex=GetTexture(gp->tsp,gp->tcw);
				dev->SetTexture(0,tex);
				SetTexMode<D3DSAMP_ADDRESSV>(gp->tsp.ClampV,gp->tsp.FlipV);
				SetTexMode<D3DSAMP_ADDRESSU>(gp->tsp.ClampU,gp->tsp.FlipU);
				
				if (Type!=ListType_Opaque)
				{
					switch(gp->tsp.ShadInstr)	// these should be correct, except offset
					{
					case 0:	// Decal
						dev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
						dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
						dev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
						break;
					case 1:	// Modulate
						dev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
						dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
						dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
						dev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
						break;
					case 2:	// Decal Alpha
						dev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_BLENDTEXTUREALPHA);
						dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
						dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
						dev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
						break;
					case 3:	// Modulate Alpha
						dev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
						dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
						dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
						dev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
						break;
					}
				}
			}
			else
			{
				dev->SetTexture(0,NULL);
			}

			if (Type!=ListType_Opaque)
			{
				if(gp->tsp.UseAlpha)
				{
					dev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
					
					dev->SetRenderState(D3DRS_SRCBLEND, SrcBlendGL[gp->tsp.SrcInstr]);
					dev->SetRenderState(D3DRS_DESTBLEND, DstBlendGL[gp->tsp.DstInstr]);
				}
				else
					dev->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
			}


			//set cull mode !
			dev->SetRenderState(D3DRS_CULLMODE,CullMode[gp->isp.CullMode]);
			//set Z mode !
			if (Type==ListType_Opaque)
				dev->SetRenderState(D3DRS_ZFUNC,Zfunction[gp->isp.DepthMode]);
			else if (Type==ListType_Translucent)
			{
				//if (autosort) -> where ? :p
				//dev->SetRenderState(D3DRS_ZFUNC,Zfunction[6]); // : GEQ
				//else -> fix it ! someday
				dev->SetRenderState(D3DRS_ZFUNC,Zfunction[gp->isp.DepthMode]);
			}
			else
				dev->SetRenderState(D3DRS_ZFUNC,Zfunction[3]); //PT : LEQ

			dev->SetRenderState(D3DRS_ZWRITEENABLE,gp->isp.ZWriteDis==0);
			

			verifyc(dev->DrawPrimitive(D3DPT_TRIANGLESTRIP,gp->first ,
				gp->count-2));
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

	void StartRender()
	{
		SetCurrentPVRRC(PARAM_BASE);
		VertexCount+= pvrrc.verts.used;
		render_end_pending_cycles=100000;

		if (FB_W_SOF1 & 0x1000000)
			return;
		FrameCount++;

		void* ptr;
		
		u32 sz=pvrrc.verts.used*sizeof(Vertex);
		
		verifyc(vb->Lock(0,sz,&ptr,D3DLOCK_DISCARD));
		
		memcpy(ptr,pvrrc.verts.data,sz);
		
		verifyc(vb->Unlock());



		// Clear the backbuffer to a blue color
		verifyc(dev->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 ));

		// Begin the scene
		if( SUCCEEDED( dev->BeginScene() ) )
		{			
			
			verifyc(dev->SetVertexShader(CompiledShader));

#define clamp(minv,maxv,x) min(maxv,max(minv,x))
			float c0=1/clamp(0.0000001f,10000000.0f,pvrrc.invW_max);
			float c1=1/clamp(0.0000001f,10000000.0f,pvrrc.invW_min);

			verifyc(dev->SetVertexShaderConstantF(0,&c0,1));
			verifyc(dev->SetVertexShaderConstantF(1,&c1,1));
			
			//dev->SetPixelShader(CompiledPShader);

			verifyc(dev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE));
//			verifyc(dev->SetRenderState(D3DRS_,D3DCULL_NONE));
			verifyc(dev->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE));
			verifyc(dev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL));
			
			verifyc(dev->SetVertexDeclaration(vdecl));
			verifyc(dev->SetStreamSource(0,vb,0,sizeof(Vertex)));

			dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			//	D3DTTFF_DISABLE, D3DTTFF_COUNT1|2|3|4  D3DTTFF_PROJECTED 
			dev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS,
				D3DTTFF_COUNT4 | D3DTTFF_PROJECTED);

			verifyc(dev->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE));
			verifyc(dev->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE));

			RendPolyParamList<ListType_Opaque>(pvrrc.global_param_op);

			verifyc(dev->SetRenderState(D3DRS_ALPHATESTENABLE,TRUE));
			verifyc(dev->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATEREQUAL));
			
			verifyc(dev->SetRenderState(D3DRS_ALPHAREF,PT_ALPHA_REF &0xFF));

			RendPolyParamList<ListType_Punch_Through>(pvrrc.global_param_pt);

			//alpha test is not working properly btw
			//verifyc(dev->SetRenderState(D3DRS_ALPHATESTENABLE,TRUE));
			verifyc(dev->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATER));
			verifyc(dev->SetRenderState(D3DRS_ALPHAREF,0));

			RendPolyParamList<ListType_Translucent>(pvrrc.global_param_tr);

			// End the scene
			verifyc(dev->EndScene());
		}

		//pvrrc.Clear();
		
		// Present the backbuffer contents to the display
		dev->Present( NULL, NULL, NULL, NULL );
	}


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

		}
		__forceinline
		static void EndList(u32 ListType)
		{
			if (CurrentPP)
			{
				CurrentPP->count=tarc.verts.used - CurrentPP->first;
				CurrentPP=0;
				vert_reappend=0;
			}
			CurrentPPlist=0;
		}

		//Polys
#define glob_param_bdc \
	    if (CurrentPP)\
		{\
		CurrentPP->count=tarc.verts.used - CurrentPP->first;\
		}\
		PolyParam* d_pp =CurrentPPlist->Append(); \
		\
		CurrentPP=d_pp;\
		\
		d_pp->first=tarc.verts.used;\
		vert_reappend=0;


		__forceinline
		static void AppendPolyParam32(TA_PolyParamA* pp)
		{
			glob_param_bdc;

			d_pp->isp=pp->isp;
			d_pp->tsp=pp->tsp;
			d_pp->tcw=pp->tcw;
			d_pp->pcw=pp->pcw;
		}
		__forceinline
		static void AppendPolyParam64A(TA_PolyParamA* pp)
		{
			glob_param_bdc;

			d_pp->isp=pp->isp;
			d_pp->tsp=pp->tsp;
			d_pp->tcw=pp->tcw;
			d_pp->pcw=pp->pcw;
		}
		__forceinline
		static void AppendPolyParam64B(TA_PolyParamB* pp)
		{

		}

		//Poly Strip handling
		__forceinline
		static void StartPolyStrip()
		{
			if (vert_reappend)
			{
				Vertex* cv=tarc.verts.Append(2);
				cv[0]=cv[-1];//dup prev
				vert_reappend=tarc.verts.used-1;
			}
		}
		__forceinline
		static void EndPolyStrip()
		{
			if (vert_reappend)
			{
				Vertex* vert=&tarc.verts.data[vert_reappend];
				vert[0]=vert[1];
			}
			else
			{
				vert_reappend=1;
			}
		}

		//Poly Vertex handlers

#define vert_cvt_base \
	Vertex* cv=tarc.verts.Append();\
	cv->x=vtx->xyz[0];\
	cv->y=vtx->xyz[1];\
	f32 invW=vtx->xyz[2];\
	cv->z=invW;\
	if (tarc.invW_min>invW)\
		tarc.invW_min=invW;\
	if (tarc.invW_max<invW)\
		tarc.invW_max=invW;

#define vert_uv_32(u_name,v_name) \
		cv->u	=	(vtx->u_name);\
		cv->v	=	(vtx->v_name);
		//cv->uv[2]	=	0; 
		//cv->uv[3]	=	invW; 

#define vert_uv_16(u_name,v_name) \
		cv->u	=	f16(vtx->u_name);\
		cv->v	=	f16(vtx->v_name);
		//cv->uv[2]	=	0; 
		//cv->uv[3]	=	invW; 

		//(Non-Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex0(TA_Vertex0* vtx)
		{
			vert_cvt_base;
			cv->col[0]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol >> 16)) ];
			cv->col[1]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol >> 8))  ];
			cv->col[2]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol >> 0))  ];
			cv->col[3]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol >> 24)) ];
		}

		//(Non-Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex1(TA_Vertex1* vtx)
		{
			vert_cvt_base;
			cv->col[0]	= vtx->BaseR;
			cv->col[1]	= vtx->BaseG;
			cv->col[2]	= vtx->BaseB;
			cv->col[3]	= vtx->BaseA;
		}

		//(Non-Textured, Intensity)
		__forceinline
		static void AppendPolyVertex2(TA_Vertex2* vtx)
		{
			vert_cvt_base;
			
			cv->col[0]	= vtx->BaseInt;
			cv->col[1]	= vtx->BaseInt;
			cv->col[2]	= vtx->BaseInt;
			cv->col[3]	= vtx->BaseInt;
		}

		//(Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex3(TA_Vertex3* vtx)
		{
			vert_cvt_base;
			
			cv->col[0]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol >> 16)) ];
			cv->col[1]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol >> 8))  ];
			cv->col[2]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol >> 0))  ];
			cv->col[3]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol >> 24)) ];

			vert_uv_32(u,v);
		}

		//(Textured, Packed Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex4(TA_Vertex4* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol >> 16)) ];
			cv->col[1]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol >> 8))  ];
			cv->col[2]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol >> 0))  ];
			cv->col[3]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol >> 24)) ];

			vert_uv_16(u,v);
		}

		//(Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex5A(TA_Vertex5A* vtx)
		{
			vert_cvt_base;
			
			cv->col[0]	= 1;//vtx->BaseR;
			cv->col[1]	= 1;//vtx->BaseG;
			cv->col[2]	= 1;//vtx->BaseB;
			cv->col[3]	= 1;//vtx->BaseA;

			vert_uv_32(u,v);
		}
		__forceinline
		static void AppendPolyVertex5B(TA_Vertex5B* vtx)
		{

		}

		//(Textured, Floating Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex6A(TA_Vertex6A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= 1;//vtx->BaseR;
			cv->col[1]	= 1;//vtx->BaseG;
			cv->col[2]	= 1;//vtx->BaseB;
			cv->col[3]	= 1;//vtx->BaseA;

			vert_uv_16(u,v);
		}
		__forceinline
		static void AppendPolyVertex6B(TA_Vertex6B* vtx)
		{

		}

		//(Textured, Intensity)
		__forceinline
		static void AppendPolyVertex7(TA_Vertex7* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= vtx->BaseInt;
			cv->col[1]	= vtx->BaseInt;
			cv->col[2]	= vtx->BaseInt;
			cv->col[3]	= vtx->BaseInt;

			vert_uv_32(u,v);
		}

		//(Textured, Intensity, 16bit UV)
		__forceinline
		static void AppendPolyVertex8(TA_Vertex8* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= 1.0f;//vtx->BaseInt;
			cv->col[1]	= 1.0f;//vtx->BaseInt;
			cv->col[2]	= 1.0f;//vtx->BaseInt;
			cv->col[3]	= 1.0f;//vtx->BaseInt;

			vert_uv_16(u,v);
		}

		//(Non-Textured, Packed Color, with Two Volumes)
		__forceinline
		static void AppendPolyVertex9(TA_Vertex9* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol0 >> 16)) ];
			cv->col[1]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol0 >> 8))  ];
			cv->col[2]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol0 >> 0))  ];
			cv->col[3]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol0 >> 24)) ];
		}

		//(Non-Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex10(TA_Vertex10* vtx)
		{
			vert_cvt_base;
			
			cv->col[0]	= 1.0f;//vtx->BaseInt0;
			cv->col[1]	= 1.0f;//vtx->BaseInt0;
			cv->col[2]	= 1.0f;//vtx->BaseInt0;
			cv->col[3]	= 1.0f;//vtx->BaseInt0;
		}

		//(Textured, Packed Color,	with Two Volumes)	
		__forceinline
		static void AppendPolyVertex11A(TA_Vertex11A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol0 >> 16)) ];
			cv->col[1]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol0 >> 8))  ];
			cv->col[2]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol0 >> 0))  ];
			cv->col[3]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol0 >> 24)) ];

			vert_uv_32(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex11B(TA_Vertex11B* vtx)
		{
			
		}

		//(Textured, Packed Color, 16bit UV, with Two Volumes)
		__forceinline
		static void AppendPolyVertex12A(TA_Vertex12A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol0 >> 16)) ];
			cv->col[1]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol0 >> 8))  ];
			cv->col[2]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol0 >> 0))  ];
			cv->col[3]	= unkpack_bgp_to_float[(255 & (vtx->BaseCol0 >> 24)) ];

			vert_uv_16(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex12B(TA_Vertex12B* vtx)
		{

		}

		//(Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex13A(TA_Vertex13A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= 1.0f;//vtx->BaseInt0;
			cv->col[1]	= 1.0f;//vtx->BaseInt0;
			cv->col[2]	= 1.0f;//vtx->BaseInt0;
			cv->col[3]	= 1.0f;//vtx->BaseInt0;

			vert_uv_32(u0,v0);			
		}
		__forceinline
		static void AppendPolyVertex13B(TA_Vertex13B* vtx)
		{

		}

		//(Textured, Intensity, 16bit UV, with Two Volumes)
		__forceinline
		static void AppendPolyVertex14A(TA_Vertex14A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= 1.0f;//vtx->BaseInt0;
			cv->col[1]	= 1.0f;//vtx->BaseInt0;
			cv->col[2]	= 1.0f;//vtx->BaseInt0;
			cv->col[3]	= 1.0f;//vtx->BaseInt0;

			vert_uv_16(u0,v0);	
		}
		__forceinline
		static void AppendPolyVertex14B(TA_Vertex14B* vtx)
		{

		}

		//Sprites
		__forceinline
		static void AppendSpriteParam(TA_SpriteParam* spr)
		{
			//printf("Sprite\n");
		}

		//Sprite Vertex Handlers
		__forceinline
		static void AppendSpriteVertex0A(TA_Sprite0A* sv)
		{

		}
		__forceinline
		static void AppendSpriteVertex0B(TA_Sprite0B* sv)
		{

		}
		__forceinline
		static void AppendSpriteVertex1A(TA_Sprite1A* sv)
		{
			
		}
		__forceinline
		static void AppendSpriteVertex1B(TA_Sprite1B* sv)
		{

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
		}
		__forceinline
		static void SoftReset()
		{
		}
	};

	FifoSplitter<VertexDecoder> TileAccel;

	bool InitRenderer(void* window)
	{
		d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

		D3DPRESENT_PARAMETERS ppar;
		memset(&ppar,0,sizeof(ppar));
/*		
		ppar.MultiSampleType = D3DMULTISAMPLE_NONE;
		ppar.BackBufferCount=3;
		ppar.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
		ppar.BackBufferFormat = D3DFMT_R8G8B8;
		
		ppar.hDeviceWindow=(HWND)Hwnd;
*/
		ppar.Windowed = TRUE;
		ppar.SwapEffect = D3DSWAPEFFECT_DISCARD;
		ppar.BackBufferFormat = D3DFMT_UNKNOWN;
		ppar.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
		ppar.EnableAutoDepthStencil=true;
		ppar.AutoDepthStencilFormat = D3DFMT_D24X8;
/*D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice 
		*/
		verifyc(d3d9->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,(HWND)params.WindowHandle,D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE,&ppar,&dev));

		//yay , 10 mb -_- =P
		verifyc(dev->CreateVertexBuffer(10*1024*1024,D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | 0,0,D3DPOOL_DEFAULT,&vb,0));
		
		verifyc(dev->CreateVertexDeclaration(vertelem,&vdecl));

		ID3DXBuffer* perr;
		ID3DXBuffer* shader;
		
		verifyc(D3DXCompileShader(Shader,sizeof(Shader),NULL,NULL,"VertexShader_Tutorial_1",D3DXGetVertexShaderProfile(dev) , 0, &shader,&perr,&shader_consts));
		if (perr)
		{
			char* text=(char*)perr->GetBufferPointer();
			printf("%s\n",text);
		}
		verifyc(dev->CreateVertexShader((DWORD*)shader->GetBufferPointer(),&CompiledShader));
		verifyc(D3DXCompileShader(Pixel,sizeof(Pixel),NULL,NULL,"VertexShader_Tutorial_1",D3DXGetPixelShaderProfile(dev) , NULL, &shader,&perr,&shader_consts));
		if (perr)
		{
			char* text=(char*)perr->GetBufferPointer();
			printf("%s\n",text);
		}
		verifyc(dev->CreatePixelShader((DWORD*)shader->GetBufferPointer(),&CompiledPShader));
		
		for (u32 i=0;i<256;i++)
		{
			unkpack_bgp_to_float[i]=i/255.0f;
		}
		return TileAccel.Init();
	}

	void TermRenderer()
	{
		TileAccel.Term();
	}

	void ResetRenderer(bool Manual)
	{
		TileAccel.Reset(Manual);
		VertexCount=0;
		FrameCount=0;
	}

	bool ThreadStart(void* window)
	{
		return true;
	}

	void ThreadEnd()
	{

	}
}

//Get null i/f
void GetDirect3DRenderer(rend_if* rif)
{
	//general init/term/reset
	rif->Init=Direct3DRenderer::InitRenderer;
	rif->Term=Direct3DRenderer::TermRenderer;
	rif->Reset=Direct3DRenderer::ResetRenderer;
	
	//thread init/term
	rif->ThreadStart=Direct3DRenderer::ThreadStart;
	rif->ThreadEnd=Direct3DRenderer::ThreadEnd;

	//drawing related functions :)
	rif->VBlank=Direct3DRenderer::VBlank;
	rif->StartRender=Direct3DRenderer::StartRender;
	
	//TA splitter i/f
	rif->Ta_ListCont=Direct3DRenderer::TileAccel.ListCont;
	rif->Ta_ListInit=Direct3DRenderer::TileAccel.ListInit;
	rif->Ta_SoftReset=Direct3DRenderer::TileAccel.SoftReset;

	//rif->VertexCount=&Direct3DRenderer::VertexCount;
	//rif->FrameCount=&Direct3DRenderer::FrameCount;
	rif->VramLockedWrite=Direct3DRenderer::VramLockedWrite;
}