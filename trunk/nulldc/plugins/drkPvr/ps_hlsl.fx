
//Pvr emulation using a pixel shader .. bahh :p
//pp_Texture -> 1 if texture is enabled , 0 if its not
//pp_Offset -> 1 if offset is enabled , 0 if its not (only valid when texture is enabled)
//pp_ShadInstr -> 0 to 3 , see pvr docs , valid only when texture is enabled
//pp_IgnoreTexA -> 1 if on  0 if off , valid only w/ textures on
//pp_UseAlpha -> 1 if on  0 if off , works when no textures are used too ?

struct pixel 
{
	float4 col : COLOR0;
	
	#if pp_Texture==1
		#if pp_Offset==1
			float4 offs : COLOR1;
		#endif
	
		float4 uv : TEXCOORD0;
	#endif
};

sampler2D samplr : register(s0);

//Pvr only supports ARGB8888 colors , so we have to clamp em (in case they are float colors inputed directly)
float4 PixelShader(in pixel s ) : COLOR0
{ 
	float4 color=saturate(s.col);
	
	#if pp_UseAlpha==0
		color.a=1;
	#endif
	
	#if pp_Texture==1
		
		//get texture color
		//s.uv.xy/=s.uv.w;
		//tex2D -> same as tex2Dproj
		#if ps_profile <= 103
			float4 texcol=tex2D( samplr, s.uv); //ps 1.1 , 1.2 and 1.3 use this one.tex2Dproj is not supported on em
												//and , tex2D lookups take in acount the PROJECTED flag (witch i set)
		#else
			float4 texcol=tex2Dproj( samplr, s.uv);	//ps 1.4 and above ingore the Texture lookup flags , but they
													//have projected lookups ;)
		#endif
		
		//apply modifiers
		#if pp_IgnoreTexA==1
			texcol.a=1;	
		#endif
		
		//OFFSETRGB is allways added after that (if enabled)
		#if pp_ShadInstr==0
			//PIXRGB = TEXRGB + OFFSETRGB
			color.rgb=texcol.rgb;
			//PIXA    = TEXA
			color.a=texcol.a;
		#elif  pp_ShadInstr==1
			//PIXRGB = COLRGB x TEXRGB + OFFSETRGB
			color.rgb*=texcol.rgb;
			//PIXA   = TEXA
			color.a=texcol.a;
		#elif  pp_ShadInstr==2
			//PIXRGB = (TEXRGB x TEXA) + (COLRGB x (1- TEXA) ) + OFFSETRGB
			color.rgb=(texcol.rgb*texcol.a) + (color.rgb * (1-texcol.a));
			//PIXA   = COLA
			//color.a remains the same
		#elif  pp_ShadInstr==3
			//PIXRGB= COLRGB x  TEXRGB + OFFSETRGB
			color.rgb*=texcol.rgb;
			//PIXA   = COLA  x TEXA
			color.a*=texcol.a;
		#endif
	
		//if offset is enabled , add it :)
		#if pp_Offset==1
			color.rgb+=saturate(s.offs.rgb);
		#endif
	#else
		//we don't realy have anything to do here -- just return the color ...
	#endif
	
	return color; 
}


//Vertex Shader :)
//_HW_INT_ -> Intesity needs to be calculated , the factors are stored in z/w of texcoords
//scale_type_1 -> use old way of scaling , needs W_min/W_max to have valid values
struct vertex 
{ 
	float4 pos : POSITION; 
	float4 col : COLOR0;
	float4 spc : COLOR1; 
	float4 uv : TEXCOORD0; 
};

float W_min: register(c0);
float W_max: register(c1);

vertex VertexShader(in vertex vtx) 
{
	vtx.pos.x=((vtx.pos.x-0.5)/319.5)-1;
	vtx.pos.y=-((vtx.pos.y-0.5)/239.5)+1;

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