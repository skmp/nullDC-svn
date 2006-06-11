

typedef void (*TPtrFillVertexFunc)(const void* pSrc, TD3DTL2VERTEXT1* pDst);
TPtrFillVertexFunc m_pFillVertexFunc = NULL;

/*
#define PVR_PACK_COLOR(a, r, g, b) ( \
  ( ((unsigned char)( a * 255.f ) ) << 24 ) | \
  ( ((unsigned char)( r * 255.f ) ) << 16 ) | \
  ( ((unsigned char)( g * 255.f ) ) << 8 ) | \
  ( ((unsigned char)( b * 255.f ) ) << 0 ) )
*/

static __forceinline  float zero_if_negative( float _f )
{
  union { float f; int i; } temp;
  temp.f = _f;
  temp.i &= ~(temp.i>>31);
  return temp.f;
}

static __forceinline  float zero_if_positive( float _f )
{
  union { float f; int i; } temp;
  temp.f = _f;
  temp.i &= (temp.i>>31);
  return temp.f;
}

static  __forceinline  float Clamp( float _x, float _min, float _max)
{
//return zero_if_positive(zero_if_negative(_x-_min)+_min-_max)+_max;
  if(_x<_min)  _x = _min;
  else
  if(_x>_max)  _x = _max;

  return _x;
}


static  __forceinline float  Clamp01( float _x)
{
//return zero_if_positive(zero_if_negative(_x)-1.0f)+1.0f;
  /**/
  if(_x<0)  _x = 0;
  else
  if(_x>1)  _x = 1;
  return _x;
  /**/
}

const __m64  mZERO  = {0};
const __m128 mmZERO = {0,0,0,0};
const __m128 mmONE  = {1,1,1,1};
const __m128 mmFF   = {255,255,255,255};
static  __forceinline unsigned FloatColor2ARGBClamp01(__m128 _x)
{
}


static  __forceinline unsigned UNAI_PACK_COLOR  (__m128 _x)
{
  __m128 r = _mm_mul_ps(mmFF,_mm_max_ps(mmZERO, _mm_min_ps(mmONE,_x)));
  unsigned c = _mm_cvtps_pi8(r).m64_u32[0];
  _mm_empty();
  return c;
}


static  __forceinline DWORD PVR_PACK_ARGB (float a, float r, float g, float b)
{
  //DWORD uRet = (*(unsigned*)(&a)) + (*(unsigned*)(&r)) + (*(unsigned*)(&g)) + (*(unsigned*)(&b));
  DWORD uRet = (unsigned(a)<<24) + (unsigned(r)<<16) + (unsigned(g)<<8) + (unsigned(b));
  return uRet;
}

inline DWORD PVRRGBA(float r, float g, float b, float a)
{
  return  PVR_PACK_ARGB( 255.0f*Clamp01(a), 255.0f*Clamp01(r), 255.0f*Clamp01(g), 255.0f*Clamp01(b) );
//return  PVR_PACK_COLOR( Clamp01(a), Clamp01(r), Clamp01(g), Clamp01(b) );
}


inline DWORD ComputeIntensityColor(float fBase)
{
	DWORD uAux = PVRRGBA(fBase*m_aBaseIntensity[0],fBase*m_aBaseIntensity[1],fBase*m_aBaseIntensity[2],fBase*m_aBaseIntensity[3]);
	return uAux;
}

inline DWORD ComputeIntensityOffset(float fBase)
{
	DWORD uAux = PVRRGBA(fBase*m_aOffsetIntensity[0],fBase*m_aOffsetIntensity[1],fBase*m_aOffsetIntensity[2],fBase*m_aOffsetIntensity[3]);
	return uAux;
}


inline static void float16Bit_To_32bit(DWORD uFloat, float* pU, float* pV)
{
	*((DWORD*)pU) = (uFloat & 0xffff0000);
	*((DWORD*)pV) = (uFloat << 16);
}



// non-textured, packed colour
struct TFormatNonTexPackedCol
{
	DWORD	flags;
	float	x, y, z;
	DWORD dummy1;
	DWORD dummy2;
	DWORD baseColour;
	DWORD dummy3;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		const TFormatNonTexPackedCol* pSrc = (const TFormatNonTexPackedCol*) _pSrc;
		pDst->x = pSrc->x;
		pDst->y = pSrc->y;
		pDst->z = pSrc->z;
  //pDst->oow = pSrc->z*g_fMaxW;
		pDst->uiRGBA = pSrc->baseColour;
	};
};


// non-textured, floating colour
struct TFormatNonTexCol
{
	DWORD	flags;
	float	x, y, z;
  float	a, r, g, b;
//__m128  mmARGB;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		const TFormatNonTexCol* pSrc = (const TFormatNonTexCol*) _pSrc;
		pDst->x = pSrc->x;
		pDst->y = pSrc->y;
		pDst->z = pSrc->z;
  ////pDst->oow = pSrc->z*g_fMaxW;
  ////pDst->uiRGBA = PVR_PACK_COLOR(pSrc->a,pSrc->r,pSrc->g,pSrc->b);
    pDst->uiRGBA = PVR_PACK_ARGB(255.0f*pSrc->a,255.0f*pSrc->r,255.0f*pSrc->g,255.0f*pSrc->b);
  //pDst->uiRGBA = UNAI_PACK_COLOR(pSrc->mmARGB);
  //pDst->uiRGBA = PVRRGBA(pSrc->a,pSrc->r,pSrc->g,pSrc->b);
	};
};


// non-textured, intensity
struct TFormatNonTexIntensity
{
	DWORD flags;
	float	x, y, z;
	DWORD dummy1;
	DWORD dummy2;
	float baseIntensity;
	DWORD dummy3;


	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		const TFormatNonTexIntensity* pSrc = (const TFormatNonTexIntensity*) _pSrc;
		pDst->x = pSrc->x;
		pDst->y = pSrc->y;
		pDst->z = pSrc->z;
  //pDst->oow = pSrc->z*g_fMaxW;
		pDst->uiRGBA = ComputeIntensityColor(pSrc->baseIntensity);
	};
};
/*
static  __forceinline float zclamptype1(float z)
{
  //return Clamp01(0.01f+z*(0.98f*.5f));
	z = 0.01f+z*(0.98f*.5f);
	//clip Z
	if (z<0.01f)
		z=0.01f;
  else
	if (z>0.99f)
		z=0.99f;
	return z;
}
*/
/*
float zclamptype2(float z)
{
	z/=8.0f;
	z = 0.01f+z*0.98f;
	//clip Z
	if (z<0.01f)
		z=0.01f;
	if (z>0.99f)
		z=0.99f;
	return z;
}
float zclamptype3(float z)
{
	if (z<1.0f)
		z=z*0.25f;
	else
		z=0.25f+(sqrtf(z-1.0f)*0.05f);
	z=0.01f+0.98f*sqrtf(z);
	if (z>0.99f) z=0.99f;
	if (z<0.01f) z=0.01f;
	return z;
}
*/
// textured, packed-colour
struct TFormatTexPackedCol
{
	DWORD flags;
	float	x, y, z;
	float u,v;
	DWORD baseColour;
	DWORD offsetColour;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		const TFormatTexPackedCol* pSrc = (const TFormatTexPackedCol*) _pSrc;
		pDst->x = pSrc->x;
		pDst->y = pSrc->y;
    pDst->z = pSrc->z;
		pDst->u = pSrc->u;
		pDst->v = pSrc->v;
  //pDst->oow = pSrc->z*g_fMaxW;
		pDst->uiRGBA = pSrc->baseColour;
		pDst->uiSpecularRGBA = pSrc->offsetColour;
	};
};


// textured, packed-colour, 16bit UV
struct TFormatTexPackedCol16UV
{
	DWORD flags;
	float x,y,z;
	DWORD uv16;
	DWORD dummy;
	DWORD baseColour;
	DWORD offsetColour;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		const TFormatTexPackedCol16UV* pSrc = (const TFormatTexPackedCol16UV*) _pSrc;
		pDst->x = pSrc->x;
		pDst->y = pSrc->y;
		pDst->z = pSrc->z;

		float16Bit_To_32bit(pSrc->uv16,&pDst->u,&pDst->v);
  //pDst->oow = pSrc->z*g_fMaxW;

		pDst->uiRGBA = pSrc->baseColour;
		pDst->uiSpecularRGBA = pSrc->offsetColour;
	};
};

struct TFormatTexCol_B
{
	float a,r,g,b;
	float oa,or,og,ob;
};


// textured, floating colour
struct TFormatTexCol
{
	DWORD flags;
	float x,y,z;
	float u,v;
	DWORD dummy1,dummy2;
	float a,r,g,b;
	float oa,or,og,ob;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		if (m_uPendingDataState == E_PENDING_DATA_NONE)
		{
			const TFormatTexCol* pSrc = (const TFormatTexCol*) _pSrc;
			pDst->x = pSrc->x;
			pDst->y = pSrc->y;
			pDst->z = pSrc->z;
			pDst->u = pSrc->u;
			pDst->v = pSrc->v;
    //pDst->oow = pSrc->z*g_fMaxW;
  		m_uPendingDataState = E_PENDING_DATA_VERTEX;
		}
		else
		{
			const TFormatTexCol_B* pSrc = (const TFormatTexCol_B*) _pSrc;
			pDst->uiRGBA = PVRRGBA(pSrc->r,pSrc->g,pSrc->b,pSrc->a);
			pDst->uiSpecularRGBA = PVRRGBA(pSrc->or,pSrc->og,pSrc->ob,pSrc->oa);
		}

	}
};

struct TFormatTexPackedColMod_B
{
	float u1,v1;
	DWORD baseColour1;
	DWORD offsetColour1;
};

struct TFormatTexPackedColMod
{
	DWORD flags;
	float	x, y, z;
	float u0,v0;
	DWORD baseColour0;
	DWORD offsetColour0;

	float u1,v1;
	DWORD baseColour1;
	DWORD offsetColour1;
	DWORD dummy1;
	DWORD dummy2;
	DWORD dummy3;
	DWORD dummy4;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		if (m_uPendingDataState == E_PENDING_DATA_NONE)
		{
			const TFormatTexPackedColMod* pSrc = (const TFormatTexPackedColMod*) _pSrc;
			pDst->x = pSrc->x;
			pDst->y = pSrc->y;
			pDst->z = pSrc->z;
			pDst->u = pSrc->u0;
			pDst->v = pSrc->v0;
    //pDst->oow = pSrc->z*g_fMaxW;
			pDst->uiRGBA = pSrc->baseColour0;
			pDst->uiSpecularRGBA = pSrc->offsetColour0;
			m_uPendingDataState = E_PENDING_DATA_VERTEX;
		}
		else
		{
		}
	};
};


// textured, packed-colour, 16bit UV
struct TFormatTexPackedCol16UVMod
{
	DWORD flags;
	float x,y,z;
	DWORD uv16_0;
	DWORD dummy1;
	DWORD baseColour0;
	DWORD offsetColour0;

	DWORD uv16_1;
	DWORD dummy2;
	DWORD baseColour1;
	DWORD offsetColour1;
	DWORD dummy3;
	DWORD dummy4;
	DWORD dummy5;
	DWORD dummy6;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		if (m_uPendingDataState == E_PENDING_DATA_NONE)
		{
			const TFormatTexPackedCol16UVMod* pSrc = (const TFormatTexPackedCol16UVMod*) _pSrc;
			pDst->x = pSrc->x;
			pDst->y = pSrc->y;
			pDst->z = pSrc->z;

			float16Bit_To_32bit(pSrc->uv16_0,&pDst->u,&pDst->v);
    //pDst->oow = pSrc->z*g_fMaxW;

			pDst->uiRGBA = pSrc->baseColour0;
			pDst->uiSpecularRGBA = pSrc->offsetColour0;
			m_uPendingDataState = E_PENDING_DATA_VERTEX;
		}
	};
};


struct TFormatTexCol16UV_B
{
	float a,r,g,b;
	float oa,or,og,ob;
};

// textured, floating colour, 16-bit UV
struct TFormatTexCol16UV
{
	DWORD flags;
	float x,y,z;
	DWORD uv16;
	DWORD dummy1;
	DWORD dummy2;
	DWORD dummy3;
	float a,r,g,b;
	float oa,or,og,ob;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		if (m_uPendingDataState == E_PENDING_DATA_NONE)
		{
			const TFormatTexCol16UV* pSrc = (const TFormatTexCol16UV*) _pSrc;

			pDst->x = pSrc->x;
			pDst->y = pSrc->y;
			pDst->z = pSrc->z;

			float16Bit_To_32bit(pSrc->uv16,&pDst->u,&pDst->v);
    //pDst->oow = pSrc->z*g_fMaxW;

			m_uPendingDataState = E_PENDING_DATA_VERTEX;
		}
		else
		{
			const TFormatTexCol16UV_B* pSrc = (const TFormatTexCol16UV_B*) _pSrc;

			pDst->uiRGBA = PVRRGBA(pSrc->r,pSrc->g,pSrc->b,pSrc->a);
			pDst->uiSpecularRGBA = PVRRGBA(pSrc->or,pSrc->og,pSrc->ob,pSrc->oa);
		}
	}
};


// textured, intensity
struct TFormatTexIntensity
{
	DWORD flags;
	float x,y,z;
	float u,v;
	float baseIntensity;
	float offsetIntensity;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		const TFormatTexIntensity* pSrc = (const TFormatTexIntensity*) _pSrc;
		pDst->x = pSrc->x;
		pDst->y = pSrc->y;
		pDst->z = pSrc->z;
		pDst->u = pSrc->u;
		pDst->v = pSrc->v;
  //pDst->oow = pSrc->z*g_fMaxW;
		pDst->uiRGBA = ComputeIntensityColor(pSrc->baseIntensity);
		pDst->uiSpecularRGBA = ComputeIntensityOffset(pSrc->offsetIntensity);
	}
};


struct TFormatTexIntensityMod
{
	DWORD flags;
	float x,y,z;
	float u0,v0;
	float baseIntensity0;
	float offsetIntensity0;

	float u1,v1;
	float baseIntensity1;
	float offsetIntensity1;
	DWORD dummy1;
	DWORD dummy2;
	DWORD dummy3;
	DWORD dummy4;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		if (m_uPendingDataState == E_PENDING_DATA_NONE)
		{
			const TFormatTexIntensityMod* pSrc = (const TFormatTexIntensityMod*) _pSrc;
			pDst->x = pSrc->x;
			pDst->y = pSrc->y;
			pDst->z = pSrc->z;
			pDst->u = pSrc->u0;
			pDst->v = pSrc->v0;
    //pDst->oow = pSrc->z*g_fMaxW;
			pDst->uiRGBA = ComputeIntensityColor(pSrc->baseIntensity0);
			pDst->uiSpecularRGBA = ComputeIntensityOffset(pSrc->offsetIntensity0);

			m_uPendingDataState = E_PENDING_DATA_VERTEX;
		}
		else
		{
		}
	}
};


// textured, intensity, 16-bit UV
struct TFormatTexIntensity16UV
{
	DWORD flags;
	float x,y,z;
	DWORD uv16;
	DWORD dummy;
	float baseIntensity;
	float offsetIntensity;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		const TFormatTexIntensity16UV* pSrc = (const TFormatTexIntensity16UV*) _pSrc;
		pDst->x = pSrc->x;
		pDst->y = pSrc->y;
		pDst->z = pSrc->z;

		float16Bit_To_32bit(pSrc->uv16,&pDst->u,&pDst->v);
  //pDst->oow = pSrc->z*g_fMaxW;

		pDst->uiRGBA = ComputeIntensityColor(pSrc->baseIntensity);
		pDst->uiSpecularRGBA = ComputeIntensityOffset(pSrc->offsetIntensity);
	};
};


struct TFormatTexIntensity16UVMod
{
	DWORD flags;
	float x,y,z;
	DWORD uv16_0;
	DWORD dummy1;
	float baseIntensity0;
	float offsetIntensity0;

	DWORD uv16_1;
	DWORD dummy2;
	float baseIntensity1;
	float offsetIntensity1;
	DWORD dummy3;
	DWORD dummy4;
	DWORD dummy5;
	DWORD dummy6;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		if (m_uPendingDataState == E_PENDING_DATA_NONE)
		{
			const TFormatTexIntensity16UVMod* pSrc = (const TFormatTexIntensity16UVMod*) _pSrc;
			pDst->x = pSrc->x;
			pDst->y = pSrc->y;
			pDst->z = pSrc->z;

			float16Bit_To_32bit(pSrc->uv16_0,&pDst->u,&pDst->v);
    //pDst->oow = pSrc->z*g_fMaxW;

			pDst->uiRGBA = ComputeIntensityColor(pSrc->baseIntensity0);
			pDst->uiSpecularRGBA = ComputeIntensityOffset(pSrc->offsetIntensity0);

			m_uPendingDataState = E_PENDING_DATA_VERTEX;
		}
		else
		{
		}
	};
};

struct TFormatSpriteA
{
	DWORD flags;
	float ax;
	float ay;
	float az;
	float bx;
	float by;
	float bz;
	float cx;

	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		if (m_uPendingDataState == E_PENDING_DATA_NONE)
		{
      const TFormatSpriteA* pSrc = (const TFormatSpriteA*) _pSrc;
      pDst[0].x = pSrc->ax;
      pDst[0].y = pSrc->ay;
      pDst[0].z = pSrc->az;
      pDst[1].x = pSrc->bx;
      pDst[1].y = pSrc->by;
      pDst[1].z = pSrc->bz;
      pDst[2].x = pSrc->cx;
      m_uPendingDataState = E_PENDING_DATA_VERTEX_SPRITE;			
		}
	}
};

struct TFormatSpriteB
{
	// Next 32 bytes
	float cy;
	float cz;
	float dx;
  float dy;
  float dz;
//unsigned long dummy;
	unsigned long auv;
	unsigned long buv;
	unsigned long cuv;
};


struct TFormatShadowVolume
{
	DWORD flags;
	float ax;
	float ay;
	float az;
	float bx;
	float by;
	float bz;
	float cx;
	float cy;
	float cz;
	DWORD dummy1;
	DWORD dummy2;
	DWORD dummy3;
	DWORD dummy4;
	DWORD dummy5;
	DWORD dummy6;


	static void FillVertex(const void* _pSrc, TD3DTL2VERTEXT1* pDst)
	{
		if (m_uPendingDataState == E_PENDING_DATA_NONE)
		{
			m_uPendingDataState = E_PENDING_DATA_VERTEX;
		}
		else
		{
		//m_pCurrentVertex--;
		}
	};
};


