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

vertex VertexShader_main(in vertex vtx) 
{
	float x=vtx.pos.x;
	float y=vtx.pos.y;
	//vtx.pos.x=((vtx.pos.x-0.5)/319.5)-1;
	//vtx.pos.y=-((vtx.pos.y-0.5)/239.5)+1;
	
	//x-=0.5;
	x/=640;
	//y-=0.5;
	y/=480;
	
	
	vtx.pos.x=x*2 + (-1.0/res_x)-1.0;
	vtx.pos.y=-y*2+ (1.0/res_y)+1.0;
	//vtx.pos.xy+=resfix;
	

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
	#else
		if (vtx.pos.z>1)
		{
			vtx.pos.z=0.99-0.14/vtx.pos.z;
		}
		else
			vtx.pos.z=vtx.pos.z * 0.84;
	
		vtx.pos.z=clamp(1- vtx.pos.z,0.000001, 0.999999);
	#endif
	
	vtx.pos.w=1;
	return vtx; 
}