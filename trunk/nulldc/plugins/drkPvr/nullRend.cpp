#include "nullRend.h"

#if REND_API == REND_SW

#include <windows.h>
#include <sdl.h>
#include <SDL/SDL_syswm.h>
#include <gl\gl.h>
#include "regs.h"

using namespace TASplitter;
#pragma comment(lib, "sdl.lib") 

//SW rendering .. yay (?)
namespace SWRenderer
{
	char fps_text[512];
	SDL_Surface *screen;
	HWND SdlWnd;
	struct VertexDecoder;
	FifoSplitter<VertexDecoder> TileAccel;
	
	struct Vertex
	{
		f32 x,y;
		f32 z;
	};
	List<Vertex> vertlist;

	template<u32 mode,u32 pbw>
	void ConvertBuffer(u32* out,u32* in,u32 outstride,u32 instride)
	{
		#define ARGB0555( word )	(((word>>10) & 0x1F)<<27) | (((word>>5) & 0x1F)<<19) | ((word&0x1F)<<3) 

		#define ARGB565( word )		(((word>>11) & 0x1F)<<27) | (((word>>5) & 0x3F)<<18) | ((word&0x1F)<<3) 
		/*
		
		ARGB8888(0xFF,unpack_5_to_8[(word>>11) & 0x1F],	\
		unpack_6_to_8[(word>>5) & 0x3F],unpack_5_to_8[word&0x1F])
		*/
		for (u32 y=0;y<480;y++)
		{
			for (u32 x=0;x<640;x+=pbw)
			{
				if (mode==0)
				{
					//0555 , 16b
					u32 dc=in[x];
					u32 c2=dc>>16;
					u32 c1=dc & 0xFFFF;
					out[x+0]=ARGB0555(c1);
					out[x+1]=ARGB0555(c2);
				}
				else if (mode==1)
				{
					//565 , 16b
					u32 dc=in[x];
					u32 c2=dc>>16;
					u32 c1=dc & 0xFFFF;
					out[x+0]=ARGB565(c1);
					out[x+1]=ARGB565(c2);
				}
				else if (mode==3)
				{
					//0888 , 32b
					out[x]=in[x*2];
				}
			}
			out+=outstride/4;
			in+=instride*2/4;
		}
	}
	typedef void ConvertBufferFP(u32* out,u32* in,u32 outstride,u32 instride);

	ConvertBufferFP* ___hahaha__[4]=
	{
		ConvertBuffer<0,2>,
		ConvertBuffer<1,2>,
		ConvertBuffer<2,1>,
		ConvertBuffer<3,1> 
	};
#define VRAM_SIZE (0x00800000)

#define VRAM_MASK (VRAM_SIZE-1)
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
	//use that someday
	void VBlank()
	{
		//present the vram to FB
		SDL_Event ev;
		while(SDL_PollEvent(&ev))
			__noop;
return;
		//SetWindowPos(SdlWnd,0,0,640,480,0,SWP_NOZORDER);	 
		//FB_R_CTRL & 0x1000000
		u32* fba=(u32*)&params.vram[vramlock_ConvOffset32toOffset64(FB_R_SOF1 & 0x7FFFFF)];

		u32 mode=FB_R_CTRL.fb_depth;
		u32 sz=(640+640*(mode>>1))*2;
		verify(SDL_LockSurface(screen)==0)
		//memset(screen->pixels,rand(),640*480*4);
		___hahaha__[mode]((u32*)screen->pixels,fba,screen->pitch,sz);

		SDL_UnlockSurface(screen);
		SDL_UpdateRect(screen,0,0,0,0);
	}

	struct Span
	{
		struct
		{
			f32 x;
			f32 z;
		} start;
		struct
		{
			f32 x;
			f32 z;
		} end;
	};
	List<Span> spans[480];

	template<typename T>
	void swap(T& t1,T& t2)
	{
		T t3=t1;
		t1=t2;
		t2=t3;
	}

	void ScanTrigA(Vertex* a,Vertex* b,Vertex* c)
	{
		int end=min(b->y,479);
		int start=max(a->y,0);
		
		if (b->x<c->x)
			swap(b,c);
		//draw ab-ac
		float s_x_diff=(a->x-b->x)/(b->y-a->y);
		float e_x_diff=(a->x-c->x)/(c->y-a->y);;
		float s_x,e_y;
		s_x=e_y=a->x;

		for (int y=start;y<end;y++)
		{
			Span* spn=spans[y].Append();
			spn->start.x=max(s_x,0);
			spn->start.z=0;

			spn->end.x=min(e_y,639);
			spn->end.z=0;

			s_x+=s_x_diff;
			e_y+=e_x_diff;
		}
	}
	void ScanTrigB(Vertex* a,Vertex* b,Vertex* c)
	{
	}
	void ScanTrig(Vertex* verts)
	{
		Vertex* a,*b,*c;
		if (verts[0].y>verts[1].y)		//1,0
		{
			if (verts[1].y>verts[2].y)  //2,1,0
			{
				a=&verts[2];
				b=&verts[1];
				c=&verts[0];
			}
			else		//1,2,0 or 1,0,2
			{
				if (verts[0].y>verts[2].y)	//1,2,0
				{
					a=&verts[1];
					b=&verts[2];
					c=&verts[0];
				}
				else					//1,0,2
				{
					a=&verts[1];
					b=&verts[0];
					c=&verts[2];
				}
			}
		}
		else
		{
			//0,1
			if (verts[0].y>verts[2].y)	//2,0,1
			{
				a=&verts[2];
				b=&verts[0];
				c=&verts[1];
			}
			else	//0,2,1 or 0,1,2
			{
				if (verts[1].y>verts[2].y)	//0,2,1
				{
					a=&verts[0];
					b=&verts[2];
					c=&verts[1];
				}
				else					//0,1,2
				{
					a=&verts[0];
					b=&verts[1];
					c=&verts[2];
				}
			}
		}

		ScanTrigA(a,b,c);
		ScanTrigB(a,b,c);
	}
	void StartRender()
	{
		
		render_end_pending_cycles=100000;
		if (FB_W_SOF1 & 0x1000000)
			return;
		FrameCount++;

		//Render frame
		u16* fba=(u16*)&params.vram[vramlock_ConvOffset32toOffset64(FB_R_SOF1 & 0x7FFFFF)];

		if (vertlist.used<3)
			return;
		for (u32 i=0;i<vertlist.used-2;i++)
		{
			ScanTrig(&vertlist.data[i]);
		}

		for (u32 y=0;y<480;y++)
		{/*
			u16* pline=&fba[640*y];
			for (u32 i=0;i<spans[y].used;i++)
			{
				Span* s=&spans[y].data[i];
				u32 sx=s->start.x;
				u32 ex=s->end.x;
				sx=max(min(sx,640),0);
				ex=max(min(ex,640),0);
				u16* pstart=&pline[sx];

				for (int cp=sx;cp<=ex;cp++)
				{
					pstart[cp]=0xFFFF;
				}
			}*/
			spans[y].Clear();
		}
	}
	void EndRender()
	{
	}


	//Vertex Decoding-Converting
	struct VertexDecoder
	{
		//list handling
		__forceinline
		static void StartList(u32 ListType)
		{
			
		}
		__forceinline
		static void EndList(u32 ListType)
		{
			
		}

		//Polys
#define glob_param_bdc

		__forceinline
		static void fastcall AppendPolyParam0(TA_PolyParam0* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam1(TA_PolyParam1* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam2A(TA_PolyParam2A* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam2B(TA_PolyParam2B* pp)
		{
			
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
		
		}

		//Poly Strip handling
		__forceinline
		static void StartPolyStrip()
		{
			
		}
		__forceinline
		static void EndPolyStrip()
		{
			
		}

		//Poly Vertex handlers
#define vert_cvt_base VertexCount++; Vertex* cv=vertlist.Append();cv->x=vtx->xyz[0];cv->y=vtx->xyz[1];cv->z=vtx->xyz[2];


		//(Non-Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex0(TA_Vertex0* vtx)
		{
			vert_cvt_base;
		}

		//(Non-Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex1(TA_Vertex1* vtx)
		{
			vert_cvt_base;
		}

		//(Non-Textured, Intensity)
		__forceinline
		static void AppendPolyVertex2(TA_Vertex2* vtx)
		{
			vert_cvt_base;
		}

		//(Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex3(TA_Vertex3* vtx)
		{
			vert_cvt_base;
		}

		//(Textured, Packed Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex4(TA_Vertex4* vtx)
		{
			vert_cvt_base;
		}

		//(Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex5A(TA_Vertex5A* vtx)
		{
			vert_cvt_base;
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
		}

		//(Textured, Intensity, 16bit UV)
		__forceinline
		static void AppendPolyVertex8(TA_Vertex8* vtx)
		{
			vert_cvt_base;
		}

		//(Non-Textured, Packed Color, with Two Volumes)
		__forceinline
		static void AppendPolyVertex9(TA_Vertex9* vtx)
		{
			vert_cvt_base;
		}

		//(Non-Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex10(TA_Vertex10* vtx)
		{
			vert_cvt_base;
		}

		//(Textured, Packed Color,	with Two Volumes)	
		__forceinline
		static void AppendPolyVertex11A(TA_Vertex11A* vtx)
		{
			vert_cvt_base;
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
		}
		__forceinline
		static void AppendPolyVertex14B(TA_Vertex14B* vtx)
		{

		}

		//Sprites
		__forceinline
		static void AppendSpriteParam(TA_SpriteParam* spr)
		{

		}

		//Sprite Vertex Handlers
		/*
		__forceinline
		static void AppendSpriteVertex0A(TA_Sprite0A* sv)
		{

		}
		__forceinline
		static void AppendSpriteVertex0B(TA_Sprite0B* sv)
		{

		}
		*/
		__forceinline
		static void AppendSpriteVertexA(TA_Sprite1A* sv)
		{

		}
		__forceinline
		static void AppendSpriteVertexB(TA_Sprite1B* sv)
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
		}
		__forceinline
		static void ListInit()
		{
			vertlist.Clear();
		}
		__forceinline
		static void SoftReset()
		{
			vertlist.Clear();
		}
	};
	//Setup related

	bool InitSDL()
	{
		
		char tmp[512];
		sprintf(tmp, "SDL_WINDOWID=%u", (unsigned long)emu.WindowHandle);
		_putenv(tmp);
		
		 if ( SDL_Init(SDL_INIT_NOPARACHUTE|SDL_INIT_VIDEO) < 0 )
		 {
			 //msgboxf("SDL init failed");
			 return false;
		 }

		 screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
		 if ( screen == NULL ) {
			 return false;
			 //die("Unable to set 640x480 video: %s\n", SDL_GetError());
		 }

		 
		 SDL_SysWMinfo wmInfo;
		 SDL_VERSION(&wmInfo.version);
		 SDL_GetWMInfo(&wmInfo);
		 SdlWnd = wmInfo.window;

		 SetWindowPos(SdlWnd,0,0,0,640,480,SWP_NOZORDER | SWP_HIDEWINDOW);
		 SetWindowLong(SdlWnd, GWL_STYLE, WS_POPUP);
		// SetWindowLongPtr(SdlWnd, GWL_WNDPROC, GetWindowLongPtr((HWND)emu.WindowHandle,GWL_WNDPROC));
		 SetParent(SdlWnd,(HWND)emu.WindowHandle);
		 SetWindowPos(SdlWnd,0,0,0,640,480,SWP_NOZORDER | SWP_SHOWWINDOW);	 
		 EnableWindow(SdlWnd,FALSE);
	}
	void TermSDL()
	{
		SDL_Quit();
	}
	//Misc setup
	void SetFpsText(char* text)
	{
		strcpy(fps_text,text);
		//if (!IsFullscreen)
		{
			SetWindowText((HWND)emu.WindowHandle, fps_text);
		}
	}
	bool InitRenderer()
	{
		InitSDL();
		return TileAccel.Init();
	}

	void TermRenderer()
	{
		TileAccel.Term();
		TermSDL();
	}

	void ResetRenderer(bool Manual)
	{
		TileAccel.Reset(Manual);
		VertexCount=0;
		FrameCount=0;
	}

	bool ThreadStart()
	{
		return true;
	}

	void ThreadEnd()
	{

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
	
	void VramLockedWrite(vram_block* bl)
	{
		
	}
}

#endif
#if REND_API == REND_NONE

#include <windows.h>

#include "regs.h"

using namespace TASplitter;


//no rendering .. yay (?)
namespace NORenderer
{
	char fps_text[512];

	struct VertexDecoder;
	FifoSplitter<VertexDecoder> TileAccel;
	

	//use that someday
	void VBlank()
	{
		
	}

	void StartRender()
	{
		
		render_end_pending_cycles=100000;
		if (FB_W_SOF1 & 0x1000000)
			return;
		FrameCount++;
	}
	void EndRender()
	{
	}


	//Vertex Decoding-Converting
	struct VertexDecoder
	{
		//list handling
		__forceinline
		static void StartList(u32 ListType)
		{
			
		}
		__forceinline
		static void EndList(u32 ListType)
		{
			
		}

		//Polys
#define glob_param_bdc

		__forceinline
		static void fastcall AppendPolyParam0(TA_PolyParam0* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam1(TA_PolyParam1* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam2A(TA_PolyParam2A* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam2B(TA_PolyParam2B* pp)
		{
			
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
		
		}

		//Poly Strip handling
		__forceinline
		static void StartPolyStrip()
		{
			
		}
		__forceinline
		static void EndPolyStrip()
		{
			
		}

		//Poly Vertex handlers
#define vert_cvt_base VertexCount++;


		//(Non-Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex0(TA_Vertex0* vtx)
		{
			vert_cvt_base;
		}

		//(Non-Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex1(TA_Vertex1* vtx)
		{
			vert_cvt_base;
		}

		//(Non-Textured, Intensity)
		__forceinline
		static void AppendPolyVertex2(TA_Vertex2* vtx)
		{
			vert_cvt_base;
		}

		//(Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex3(TA_Vertex3* vtx)
		{
			vert_cvt_base;
		}

		//(Textured, Packed Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex4(TA_Vertex4* vtx)
		{
			vert_cvt_base;
		}

		//(Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex5A(TA_Vertex5A* vtx)
		{
			vert_cvt_base;
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
		}

		//(Textured, Intensity, 16bit UV)
		__forceinline
		static void AppendPolyVertex8(TA_Vertex8* vtx)
		{
			vert_cvt_base;
		}

		//(Non-Textured, Packed Color, with Two Volumes)
		__forceinline
		static void AppendPolyVertex9(TA_Vertex9* vtx)
		{
			vert_cvt_base;
		}

		//(Non-Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex10(TA_Vertex10* vtx)
		{
			vert_cvt_base;
		}

		//(Textured, Packed Color,	with Two Volumes)	
		__forceinline
		static void AppendPolyVertex11A(TA_Vertex11A* vtx)
		{
			vert_cvt_base;
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
		}
		__forceinline
		static void AppendPolyVertex14B(TA_Vertex14B* vtx)
		{

		}

		//Sprites
		__forceinline
		static void AppendSpriteParam(TA_SpriteParam* spr)
		{

		}

		//Sprite Vertex Handlers
		/*
		__forceinline
		static void AppendSpriteVertex0A(TA_Sprite0A* sv)
		{

		}
		__forceinline
		static void AppendSpriteVertex0B(TA_Sprite0B* sv)
		{

		}
		*/
		__forceinline
		static void AppendSpriteVertexA(TA_Sprite1A* sv)
		{

		}
		__forceinline
		static void AppendSpriteVertexB(TA_Sprite1B* sv)
		{

		}

		//ModVolumes
		__forceinline
		static void AppendModVolParam(TA_ModVolParam* modv)
		{

		}

		//ModVol Strip handling
		__forceinline
		static void StartModVol(TA_ModVolParam* param)
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
		__forceinline
		static void SetTileClip(u32 xmin,u32 ymin,u32 xmax,u32 ymax)
		{
		}
		__forceinline
		static void TileClipMode(u32 mode)
		{
			
		}
		//Misc
		__forceinline
		static void ListCont()
		{
		}
		__forceinline
		static void ListInit()
		{
			
		}
		__forceinline
		static void SoftReset()
		{
			
		}
	};
	//Setup related

	//Misc setup
	void SetFpsText(char* text)
	{
		strcpy(fps_text,text);
		//if (!IsFullscreen)
		{
			SetWindowText((HWND)emu.GetRenderTarget(), fps_text);
		}
	}
	bool InitRenderer()
	{
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

	bool ThreadStart()
	{
		return true;
	}

	void ThreadEnd()
	{

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
	
	void VramLockedWrite(vram_block* bl)
	{
		
	}
}

#endif