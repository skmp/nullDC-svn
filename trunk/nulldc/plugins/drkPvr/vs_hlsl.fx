//Vertex Shader :)
//_HW_INT_ -> Intesity needs to be calculated , the factors are stored in z/w of texcoords
//scale_type_1 -> use old way of scaling , needs W_min/W_max to have valid values
//RES_X -> x resolution
//RES_Y -> y resolution

struct vertex 
{ 
	float4 pos : POSITION; 
	float4 col : COLOR0;
	float4 spc : COLOR1; 
	float4 uv : TEXCOORD0; 
};

float W_min: register(c0);
float W_max: register(c1);
float4 res_scale: register(c2);
vertex VertexShader_main(in vertex vtx) 
{
	vtx.pos.xy+=res_scale.xy;
	vtx.pos.xy/=res_scale.zw;
	
	vtx.pos.xy+=float2((-1.0/res_x)-1.0,(1.0/res_y)+1.0);
	
	//Saturate the colors here , no need to do it on pixel shaders
	vtx.col=saturate(vtx.col);
	vtx.spc=saturate(vtx.spc);
	
	#ifdef _HW_INT_
	vtx.col*=vtx.uv.z;
	vtx.spc*=vtx.uv.w;
	#endif

	vtx.uv.xy*=vtx.pos.z;
	vtx.uv.z=0;
	vtx.uv.w=vtx.pos.z;
	
	#ifdef scale_type_1
		vtx.pos.z=((1/clamp(vtx.pos.z,0.0000001,10000000))-W_min)/W_max;
		vtx.pos.z=clamp(vtx.pos.z,0, 1);
	//#else
		if (vtx.pos.z>1)
		{
			if (vtx.pos.z>1000)
			{
				vtx.pos.z=0.999-90/vtx.pos.z;	//max 0.09
			}
			else
			{
				vtx.pos.z=0.899-0.05/vtx.pos.z;	//max 0.05
			}
		}
		else
			vtx.pos.z=vtx.pos.z * 0.84;
	
		vtx.pos.z=clamp(1- vtx.pos.z,0.000001, 0.999999);
	#endif
	vtx.pos.z=1/(1+vtx.pos.z);
	
	vtx.pos.w=1;
	return vtx; 
}