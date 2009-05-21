#include "nullRend.h"

#if REND_API == REND_SW
volatile bool render_restart;
#include <windows.h>
#include <sdl.h>
#include <SDL_syswm.h>
#include <gl\gl.h>
#include "regs.h"
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

using namespace TASplitter;
#pragma comment(lib, "sdl.lib") 

//SW rendering .. yay (?)
namespace SWRenderer
{
	wchar fps_text[512];
	SDL_Surface *screen;
	HWND SdlWnd;
	struct VertexDecoder;
	FifoSplitter<VertexDecoder> TileAccel;
	
	struct Vertex
	{
		f32 x,y;
		f32 z;
		u32 EOS;
		u32 Col;
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
	__m128i _mm_load_scaled(int v,int s)
	{
		return _mm_setr_epi32(v,v+s,v+s+s,v+s+s+s);
	}
	__m128i _mm_broadcast(int v)
	{
		__m128i rv=_mm_cvtsi32_si128(v);
		return _mm_shuffle_epi32(rv,0);
	}
	typedef void ConvertBufferFP(u32* out,u32* in,u32 outstride,u32 instride);

	ConvertBufferFP* ___hahaha__[4]=
	{
		ConvertBuffer<0,2>,
		ConvertBuffer<1,2>,
		ConvertBuffer<2,1>,
		ConvertBuffer<3,1> 
	};

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

	__declspec(align(32)) u32 tempCol[640*480*2];
	void VBlank()
	{
		//present the vram to FB
		SDL_Event ev;
		while(SDL_PollEvent(&ev))
			__noop;
//return;
		//SetWindowPos(SdlWnd,0,0,640,480,0,SWP_NOZORDER);	 
		//FB_R_CTRL & 0x1000000
		u32* fba=(u32*)&params.vram[vramlock_ConvOffset32toOffset64(FB_R_SOF1 & VRAM_MASK)];

		u32 mode=FB_R_CTRL.fb_depth;
		u32 sz=(640+640*(mode>>1))*2;
		verifyf(SDL_LockSurface(screen)==0);
		//memset(screen->pixels,rand(),640*480*4);
		//___hahaha__[mode]((u32*)screen->pixels,fba,screen->pitch,sz);
		//memcpy(screen->pixels,tempCol,sizeof(tempCol));
		__m128* pdst=(__m128*)screen->pixels;
		__m128* psrc=(__m128*)tempCol;
		const int stride=640/4;
		for (int y=0;y<480;y+=4)
		{
			for (int x=0;x<640;x+=4)
			{
				__m128 a=*psrc++,b=*psrc++,c=*psrc++,d=*psrc++;
				_MM_TRANSPOSE4_PS(a,b,c,d);
				pdst[(y+0)*stride+x/4]=a;
				pdst[(y+1)*stride+x/4]=b;
				pdst[(y+2)*stride+x/4]=c;
				pdst[(y+3)*stride+x/4]=d;
			}
		}

		SDL_UnlockSurface(screen);
		SDL_UpdateRect(screen,0,0,0,0);
	}

	__forceinline int iround(float x)
	{
		return _mm_cvtt_ss2si(_mm_load_ss(&x));
	}
	int mmin(int a,int b,int c,int d)
	{
		int rv=min(a,b);
		rv=min(c,rv);
		return max(d,rv);
	}
	int mmax(int a,int b,int c,int d)
	{
		int rv=max(a,b);
		rv=max(c,rv);
		return min(d,rv);
	}

	//i think this gives false positives ...
	//yup, if ANY of the 3 tests fail the ANY tests fails.
	__forceinline void EvalHalfSpace(bool& all, bool& any,int cp,int sv,int lv)
	{
		//bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
		//bool a10 = C1 + DX12 * y0 - DY12 * x0 > qDY12;
		//bool a01 = C1 + DX12 * y0 - DY12 * x0 > -qDX12;
		//bool a11 = C1 + DX12 * y0 - DY12 * x0 > (qDY12-qDX12);

		//C1 + DX12 * y0 - DY12 * x0 > 0
		// + DX12 * y0 - DY12 * x0 > 0 - C1
		//int pd=DX * y0 - DY * x0;

		bool a = cp > sv;	//needed for ANY
		bool b = cp > lv;	//needed for ALL

		any&=a;
		all&=b;
	}

	//return true if any is positive
	__forceinline bool EvalHalfSpaceFAny(int cp12,int cp23,int cp31)
	{
		int svt=cp12; //needed for ANY
			svt|=cp23;
			svt|=cp31;

		return svt>0;
	}

	__forceinline bool EvalHalfSpaceFAll(int cp12,int cp23,int cp31,int lv12,int lv23,int lv31)
	{
		int lvt=cp12-lv12;
			lvt|=cp23-lv23;
			lvt|=cp31-lv31;	//needed for all
		
		return lvt>0;
	}

	__forceinline void PlaneMinMax(int& MIN,int& MAX,int DX,int DY,int q)
	{
		int q_fp=(q - 1)<<4;
		int v1=0;
		int v2=q_fp*DY;
		int v3=-q_fp*DX;
		int v4=q_fp*(DY-DX);

		MIN=min(v1,min(v2,min(v3,v4)));
		MAX=max(v1,max(v2,max(v3,v4)));
	}

	struct PlaneStepper
	{
		__m128 ddx,ddy;
		__m128 c;

		void Setup(const Vertex &v1, const Vertex &v2, const Vertex &v3,int minx,int miny,int q)
		{
			float v1_z=v1.z,v2_z=v2.z,v3_z=v3.z;
			float A = ((v3_z - v1_z) * (v2.y - v1.y) - (v2_z - v1_z) * (v3.y - v1.y));
			float B = ((v3.x - v1.x) * (v2_z - v1_z) - (v2.x - v1.x) * (v3_z - v1_z));
			float C = ((v2.x - v1.x) * (v3.y - v1.y) - (v3.x - v1.x) * (v2.y - v1.y));
			float ddx_s=-A / C;
			float ddy_s=-B / C;
			ddx = _mm_load1_ps(&ddx_s);
			ddy = _mm_load1_ps(&ddy_s);

			float c_s=(v1_z - ddx_s *v1.x - ddy_s*v1.y);
			c = _mm_load1_ps(&c_s);

			//z = z1 + dzdx * (minx - v1.x) + dzdy * (minx - v1.y);
			//z = (z1 - dzdx * v1.x - v1.y*dzdy) +  dzdx*inx + dzdy *iny;
			
		}
		__m128 Ip(__m128 x,__m128 y) const
		{
			__m128 p1=_mm_mul_ps(x,ddx);
			__m128 p2=_mm_mul_ps(y,ddy);

			__m128 s1=_mm_add_ps(p1,p2);
			return _mm_add_ps(s1,c);
		}
		
	};

	struct IPs
	{
		PlaneStepper Z;
		__m128i col;

		void Setup(const Vertex &v1, const Vertex &v2, const Vertex &v3,int minx,int miny,int q)
		{
			Z.Setup(v1,v2,v3,minx,miny,q);
			//u8 cz=255*v1.z;
			u32 Col=v1.Col; // cz | cz*256 | cz*256*256 | cz*256*256*256; // to see Z values :)
			col=_mm_broadcast(Col);
			//const __m128 Green=_mm_set_ps(0.32f,0.32f,0.32f,0.32f);

		}
	};
	

	IPs __declspec(align(16)) ip;

	/*
	__m128i TransposeAndCompat(__m128 a,__m128 b,__m128 c,__m128 d)
	{
		_MM_TRANSPOSE4_PS(a,b,c,d);
		
		__m128i ab=_mm_packs_epi32(_mm_cvttps_epi32(a),_mm_cvttps_epi32(b));
		__m128i cd=_mm_packs_epi32(_mm_cvttps_epi32(c),_mm_cvttps_epi32(d));
		return _mm_packus_epi16(ab,cd);
	}
	*/
	template<bool useoldmsk>
	void PixelFlush(__m128 x,__m128 y,u8* cb,__m128 oldmask)
	{
		__m128 invW=ip.Z.Ip(x,y);
		__m128* zb=(__m128*)&cb[640*480*4];

		__m128 ZMask=_mm_cmpgt_ps(invW,*zb);
		if (useoldmsk)
			ZMask=_mm_and_ps(oldmask,ZMask);
		u32 msk=_mm_movemask_ps(ZMask);//0xF
		
		if (msk==0)
			return;

		/*
		__m128 r=ip.R.Ip(x,y);
		__m128 b=ip.G.Ip(x,y);
		__m128 b=ip.B.Ip(x,y);
		__m128 a=ip.A.Ip(x,y);
		colorz=TransposeAndCompat(r,g,b,a);
		*/

		__m128i rv=ip.col;//_mm_xor_si128(_mm_cvtps_epi32(_mm_mul_ps(x,Z.c)),_mm_cvtps_epi32(y));

		if (msk!=0xF)
		{
			rv = _mm_and_si128(rv,*(__m128i*)&ZMask);
			rv = _mm_or_si128(_mm_andnot_si128(*(__m128i*)&ZMask,*(__m128i*)cb),rv);
			
			invW = _mm_and_ps(invW,ZMask);
			invW = _mm_or_ps(_mm_andnot_ps(ZMask,*zb),invW);

		}
		*zb=invW;
		*(__m128i*)cb=rv;
	}
	//u32 nok,fok;
	void Rendtriangle(const Vertex &v1, const Vertex &v2, const Vertex &v3,u32* colorBuffer)
	{
		const int stride=640*4;
		//Plane equation
		

		// 28.4 fixed-point coordinates
		const int Y1 = iround(16.0f * v1.y);
		const int Y2 = iround(16.0f * v2.y);
		const int Y3 = iround(16.0f * v3.y);

		const int X1 = iround(16.0f * v1.x);
		const int X2 = iround(16.0f * v2.x);
		const int X3 = iround(16.0f * v3.x);

		int sgn=1;

		// Deltas
		{
			//area: (X1-X3)*(Y2-Y3)-(Y1-Y3)*(X2-X3)

			if (((X1-X3)*(Y2-Y3)-(Y1-Y3)*(X2-X3))>0)
				sgn=-1;
		}

		const int DX12 = sgn*(X1 - X2);
		const int DX23 = sgn*(X2 - X3);
		const int DX31 = sgn*(X3 - X1);

		const int DY12 = sgn*(Y1 - Y2);
		const int DY23 = sgn*(Y2 - Y3);
		const int DY31 = sgn*(Y3 - Y1);

		// Fixed-point deltas
		const int FDX12 = DX12 << 4;
		const int FDX23 = DX23 << 4;
		const int FDX31 = DX31 << 4;

		const int FDY12 = DY12 << 4;
		const int FDY23 = DY23 << 4;
		const int FDY31 = DY31 << 4;

		// Block size, standard 4x4 (must be power of two)
		const int q = 4;

		// Bounding rectangle
		int minx = (mmin(X1, X2, X3,0) + 0xF) >> 4;
		int miny = (mmin(Y1, Y2, Y3,0) + 0xF) >> 4;
		
		// Start in corner of block
		minx &= ~(q - 1);
		miny &= ~(q - 1);

		int spanx = ((mmax(X1, X2, X3,640<<4) + 0xF) >> 4)-minx;
		int spany = ((mmax(Y1, Y2, Y3,480<<4) + 0xF) >> 4)-miny;

		// Half-edge constants
		int C1 = DY12 * X1 - DX12 * Y1;
		int C2 = DY23 * X2 - DX23 * Y2;
		int C3 = DY31 * X3 - DX31 * Y3;

		// Correct for fill convention
		if(DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
		if(DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
		if(DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

		int MAX_12,MAX_23,MAX_31,MIN_12,MIN_23,MIN_31;

		PlaneMinMax(MIN_12,MAX_12,DX12,DY12,q);
		PlaneMinMax(MIN_23,MAX_23,DX23,DY23,q);
		PlaneMinMax(MIN_31,MAX_31,DX31,DY31,q);

		const int FDqX12 = FDX12 * q;
		const int FDqX23 = FDX23 * q;
		const int FDqX31 = FDX31 * q;

		const int FDqY12 = FDY12 * q;
		const int FDqY23 = FDY23 * q;
		const int FDqY31 = FDY31 * q;

		const int FDX12mq = FDX12+FDY12*q;
		const int FDX23mq = FDX23+FDY23*q;
		const int FDX31mq = FDX31+FDY31*q;

		int hs12 = C1 + FDX12 * miny - FDY12 * minx + FDqY12 - MIN_12;
		int hs23 = C2 + FDX23 * miny - FDY23 * minx + FDqY23 - MIN_23;
		int hs31 = C3 + FDX31 * miny - FDY31 * minx+ FDqY31 - MIN_31;

		MAX_12-=MIN_12;
		MAX_23-=MIN_23;
		MAX_31-=MIN_31;
		
		int C1_pm = MIN_12;
		int C2_pm = MIN_23;
		int C3_pm = MIN_31;


		u8* cb_y=(u8*)colorBuffer;
		cb_y+=miny*stride + minx*(q*4);
		
		ip.Setup(v1,v2,v3,minx,miny,q);
		__m128 y_ps=_mm_cvtepi32_ps(_mm_load_scaled(miny,1));
		__m128 minx_ps=_mm_cvtepi32_ps(_mm_broadcast(minx));
		static __declspec(align(16)) float ones_ps[4]={1,1,1,1};
		static __declspec(align(16)) float q_ps[4]={q,q,q,q};

		// Loop through blocks
		for(int y = spany; y > 0; y-=q)
		{
			int Xhs12=hs12;
			int Xhs23=hs23;
			int Xhs31=hs31;
			u8* cb_x=cb_y;
			__m128 x_ps=minx_ps;
			for(int x = spanx; x > 0; x-=q)
			{
				Xhs12-=FDqY12;
				Xhs23-=FDqY23;
				Xhs31-=FDqY31;

				// Corners of block
				bool any=EvalHalfSpaceFAny(Xhs12,Xhs23,Xhs31);

				// Skip block when outside an edge
				if(!any)
				{
					cb_x+=q*q*4;
					x_ps=_mm_add_ps(x_ps,*(__m128*)q_ps);
					continue;
				}
				
				bool all=EvalHalfSpaceFAll(Xhs12,Xhs23,Xhs31,MAX_12,MAX_23,MAX_31);
				
				// Accept whole block when totally covered
				if(all)
				{
					for(int iy = q; iy > 0; iy--)
					{
						PixelFlush<false>(x_ps,y_ps,cb_x,x_ps);
						x_ps=_mm_add_ps(x_ps,*(__m128*)ones_ps);
						cb_x+=sizeof(__m128);
					}
				}
				else // Partially covered block
				{
					int CY1 = C1_pm + Xhs12;
					int CY2 = C2_pm + Xhs23;
					int CY3 = C3_pm + Xhs31;

					__m128i pfdy12=_mm_broadcast(FDY12);
					__m128i pfdy23=_mm_broadcast(FDY23);
					__m128i pfdy31=_mm_broadcast(FDY31);

					__m128i pcy1=_mm_load_scaled(CY1,FDX12);
					__m128i pcy2=_mm_load_scaled(CY2,FDX23);
					__m128i pcy3=_mm_load_scaled(CY3,FDX31);

					__m128i pzero=_mm_setzero_si128();

//bool ok=false;

					for(int iy = q; iy > 0; iy--)
					{
						__m128i a=_mm_cmpgt_epi32(_mm_or_si128(_mm_or_si128(pcy1,pcy2),pcy3),pzero);
						int msk=_mm_movemask_ps(*(__m128*)&a);
						if (msk!=0)
						{
							PixelFlush<true>(x_ps,y_ps,cb_x,*(__m128*)&a);
						}

						x_ps=_mm_add_ps(x_ps,*(__m128*)ones_ps);
						cb_x+=sizeof(__m128);

						//CY1 += FDX12mq;
						//CY2 += FDX23mq;
						//CY3 += FDX31mq;
						pcy1=_mm_sub_epi32(pcy1,pfdy12);
						pcy2=_mm_sub_epi32(pcy2,pfdy23);
						pcy3=_mm_sub_epi32(pcy3,pfdy31);
					}
					/*
					if (!ok)
					{
						nok++;
					}
					else
					{
						fok++;
					}*/
				}
			}
next_y:
			hs12+=FDqX12;
			hs23+=FDqX23;
			hs31+=FDqX31;
			cb_y+=stride*q;
			y_ps=_mm_add_ps(y_ps,*(__m128*)q_ps);
		}
	}

	void StartRender()
	{
		
		render_end_pending_cycles=100000;
		if (FB_W_SOF1 & 0x1000000)
			return;
		FrameCount++;

		//Render frame
		u16* fba=(u16*)&params.vram[vramlock_ConvOffset32toOffset64(FB_R_SOF1 & VRAM_MASK)];
		memset(tempCol,0,sizeof(tempCol));
		if (vertlist.used<3)
			return;
		for (u32 i=0;i<vertlist.used-2;i++)
		{
			//ScanTrig(&vertlist.data[i]);
			Rendtriangle(vertlist.data[i],vertlist.data[i+1],vertlist.data[i+2],(u32*)tempCol);
			if (vertlist.data[i+2].EOS)
				i+=2;
		}
		//printf("NOK:%d FOK:%d\n",nok,fok);
		//fok=nok=0;
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
			vertlist.LastPtr()->EOS=1;
		}
		__forceinline
			static void StartModVol(TA_ModVolParam* param)
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

		//Poly Vertex handlers
#define vert_cvt_base VertexCount++; Vertex* cv=vertlist.Append();cv->x=vtx->xyz[0];cv->y=vtx->xyz[1];cv->z=vtx->xyz[2];cv->EOS=0;


		//(Non-Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex0(TA_Vertex0* vtx)
		{
			vert_cvt_base;
			cv->Col=vtx->BaseCol;
		}

		//(Non-Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex1(TA_Vertex1* vtx)
		{
			vert_cvt_base;
			cv->Col=0xFFFFFFFF;
		}

		//(Non-Textured, Intensity)
		__forceinline
		static void AppendPolyVertex2(TA_Vertex2* vtx)
		{
			vert_cvt_base;
			cv->Col=0xFFFFFFFF;
		}

		//(Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex3(TA_Vertex3* vtx)
		{
			vert_cvt_base;
			cv->Col=vtx->BaseCol;
		}

		//(Textured, Packed Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex4(TA_Vertex4* vtx)
		{
			vert_cvt_base;
			cv->Col=vtx->BaseCol;
		}

		//(Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex5A(TA_Vertex5A* vtx)
		{
			vert_cvt_base;
			cv->Col=0xFFFFFFFF;
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
			cv->Col=0xFFFFFFFF;
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
			cv->Col=0xFFFFFFFF;
		}

		//(Textured, Intensity, 16bit UV)
		__forceinline
		static void AppendPolyVertex8(TA_Vertex8* vtx)
		{
			vert_cvt_base;
			cv->Col=0xFFFFFFFF;
		}

		//(Non-Textured, Packed Color, with Two Volumes)
		__forceinline
		static void AppendPolyVertex9(TA_Vertex9* vtx)
		{
			vert_cvt_base;
			cv->Col=vtx->BaseCol0;
		}

		//(Non-Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex10(TA_Vertex10* vtx)
		{
			vert_cvt_base;
			cv->Col=0xFFFFFFFF;
		}

		//(Textured, Packed Color,	with Two Volumes)	
		__forceinline
		static void AppendPolyVertex11A(TA_Vertex11A* vtx)
		{
			vert_cvt_base;
			cv->Col=vtx->BaseCol0;
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
			cv->Col=vtx->BaseCol0;
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
			cv->Col=0xFFFFFFFF;
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
			cv->Col=0xFFFFFFFF;
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
		//sprintf(tmp, "SDL_WINDOWID=%u", (unsigned long)emu.GetRenderTarget());
		//_putenv(tmp);
		
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
		 SetParent(SdlWnd,(HWND)emu.GetRenderTarget());
		 SetWindowPos(SdlWnd,0,0,0,640,480,SWP_NOZORDER | SWP_SHOWWINDOW);	 
		 EnableWindow(SdlWnd,FALSE);
	}
	void TermSDL()
	{
		SDL_Quit();
	}
	//Misc setup
	void SetFpsText(wchar* text)
	{
		wcscpy(fps_text,text);
		//if (!IsFullscreen)
		{
			SetWindowText((HWND)emu.GetRenderTarget(), fps_text);
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