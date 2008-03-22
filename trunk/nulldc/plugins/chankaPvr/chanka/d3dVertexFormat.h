// ----------------------------------------------------------------------------------------
// Nombre       : vertexFormat.h
// Descripcion  : 
// ----------------------------------------------------------------------------------------

#ifndef _vertexFormat_h
#define _vertexFormat_h

#define MD3DFVF_LVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

struct TD3DLVERTEX
{
  float x,y,z;
  unsigned int uiRGBA;	  
};


#define MD3DFVF_VERTEXT1VC (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)

struct TD3DVERTEXT1VC
{
  float x,y,z;
  float nx, ny, nz;	  
  unsigned int uiRGBA;	
  float u,v;    
};


#define MD3DFVF_TLVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

struct TD3DTLVERTEX
{
  float x,y,z,oow;
  unsigned int uiRGBA;		
};


#define MD3DFVF_TLVERTEXT1 (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

struct TD3DTLVERTEXT1
{
	float x,y,z,oow;
	unsigned int uiRGBA;		
	float u,v;
};

#define MD3DFVF_TL2VERTEXT1 (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

struct TD3DTL2VERTEXT1
{
  TD3DTL2VERTEXT1(){}
	float x,y,z,oow;
	unsigned int uiRGBA;
	unsigned int uiSpecularRGBA;
	float u,v;
};


#endif