//Vertex Shader :)
//_HW_INT_ -> Intesity needs to be calculated , the factors are stored in z/w of texcoords
//scale_type_1 -> use old way of scaling , needs W_min/W_max to have valid values
//RES_X -> x resolution
//RES_Y -> y resolution
//ZBufferMode -> z buffer mode :p
//ZBufferMode : 0 -> fp fixup
//ZBufferMode : 1 -> nothing
//ZBufferMode : 2 -> rescale
#ifndef FLT_MIN
#define FLT_MIN 1.17549435e-38f
#endif

#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38f
#endif

struct vertex_in 
{ 
	float4 pos : POSITION; 
	float4 col : COLOR0;
	float4 spc : COLOR1; 
	float2 uv : TEXCOORD0; 
};

struct vertex_out
{ 
	float4 pos : POSITION; 
	float4 col : TEXCOORD1;
	float4 spc : TEXCOORD2; 
	float4 uv : TEXCOORD0; 
};

float W_min: register(c0);
float W_max: register(c1);
float4 res_scale: register(c2);
float4 texture_size:  register(c3);

float CompressZ(float w)
{
	float e,m;
	m=frexp(w,e);
	e=clamp(e-32,-127,0);
	return ldexp(m,e);
}

vertex_out VertexShader_main(in vertex_in vin) 
{
	vertex_out vo;
	vo.pos.xy=vin.pos.xy+res_scale.xy;
	vo.pos.xy/=res_scale.zw;
	
	vo.pos.xy+=float2((-1.0/res_x)-1.0,(1.0/res_y)+1.0);
	
	/*
	//this doesnt have the desired effect.. why ? (alligns texture cord to pixel, so bilinear doesnt 'leak' from wrong pixels)
	float2 sng=sign(vtx.uv.xy);
	vtx.uv.xy=abs(vtx.uv.xy) + (texture_size.zw/2);
	vtx.uv.xy-=fmod(vtx.uv.xy,texture_size.zw);
	vtx.uv.xy*=sng;
	*/	
	vo.uv.xy=vin.uv*vin.pos.z;
	
	vo.col=saturate(vin.col)*vin.pos.z;
	vo.spc=saturate(vin.spc)*vin.pos.z;
	
	vo.uv.w=vin.pos.z;
	
	//I need to do smth about fixed function for this one
	if (! (vin.pos.z<FLT_MAX && vin.pos.z>0))
		vo.uv.z=-1;
	else
		vo.uv.z=0;
		 
	#if ZBufferMode==0
		vo.pos.z=CompressZ(vin.pos.z);
	#elif ZBufferMode==1
		vo.pos.z=0;
	#elif ZBufferMode==2
		//vo.pos.z=1-1/(1+vin.pos.z);
		if (vin.pos.z<128)
		{
			vo.pos.z=0.875*(vin.pos.z/128);
		}
		else if (vin.pos.z<8192)
		{
			vo.pos.z=0.875 + 0.0625*(vin.pos.z/8192);
		}
		else
		{
			vo.pos.z=(0.875 + 0.0625) + 0.0625*(vin.pos.z/FLT_MAX);
		}
	#endif
	
	vo.pos.w=1;

	return vo; 
}