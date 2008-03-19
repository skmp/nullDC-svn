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
};

struct vertex_out
{ 
	float4 pos : SV_POSITION; 
};
     

vertex_out VertexShader_main(in vertex_in vin) 
{
	vertex_out vo;
	float4 res_scale=float4(0,0,320,-240);
	vo.pos.xy=vin.pos.xy+res_scale.xy;
	vo.pos.xy/=res_scale.zw;
	
	vo.pos.xy+=float2((-1.0/640)-1.0,(1.0/480)+1.0);
	
	vo.pos.z=0.5;
	
	vo.pos.w=1;

	return vo; 
}