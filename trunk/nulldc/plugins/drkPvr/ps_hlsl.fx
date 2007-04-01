//Pixel Shader
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
float4 PixelShader_main(in pixel s ) : COLOR0
{ 
	float4 color=saturate(s.col);
	
	#if pp_UseAlpha==0
		color.a=1;
	#endif
	
	#if pp_Texture==1
		
		//get texture color
		//s.uv.xy/=s.uv.w;
		//tex2D -> same as tex2Dproj
		#if ps_no_tex2D == 1
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