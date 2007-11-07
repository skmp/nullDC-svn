//Pixel Shader
//Pvr emulation using a pixel shader .. bahh :p
//pp_Texture -> 1 if texture is enabled , 0 if its not
//pp_Offset -> 1 if offset is enabled , 0 if its not (only valid when texture is enabled)
//pp_ShadInstr -> 0 to 3 , see pvr docs , valid only when texture is enabled
//pp_IgnoreTexA -> 1 if on  0 if off , valid only w/ textures on
//pp_UseAlpha -> 1 if on  0 if off , works when no textures are used too ?
//
//misc #defines :
//proj2Dtex -> function to do projected lookup, either tex2D or tex2Dproj
//TextureLookup -> function to use for texture lookup.One of TextureLookup_Normal,TextureLookup_Palette,TextureLookup_Palette_Bilinear
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
sampler2D tex_pal : register(s1);

float4 current_pal: register(c0);
float4 texture_size: register(c1);

float4 PixelShader_null() : COLOR0
{
	return float4(0,0,0,0.5f); 
}

float4 TextureLookup_Normal(float4 uv)
{
		//tex2D -> same as tex2Dproj
		/*
		//no longer used, replaces with a #define
		#if ps_no_tex2D == 1
			//s.uv.xy/=s.uv.w; //cant do that on 1.1/2/3 so its useless
			float4 texcol=tex2D( samplr, s.uv); //ps 1.1 , 1.2 and 1.3 use this one.tex2Dproj is not supported on em
												//and , tex2D lookups take in acount the PROJECTED flag (witch i set)
		#else
			float4 texcol=tex2Dproj( samplr, s.uv);	//ps 1.4 and above ingore the Texture lookup flags , but they
													//have projected lookups ;)
		#endif
		*/
		return proj2Dtex( samplr, uv);
}

//utility function for pal. lookups :)
float4 PalleteLookup(float4 pos)
{
	//xyzw -> x=index , y=bank
	float4 texcol=tex2D(tex_pal,pos.bg+current_pal.xy);
	return texcol;
}

float4 TextureLookup_Palette(float4 uv)
{
	float4 pal_color=TextureLookup_Normal(uv);
	
	return PalleteLookup(pal_color);
}

float4 TextureLookup_Palette_Nproj(float2 uv)
{
	float4 pal_color=tex2D(samplr,uv);
	
	return PalleteLookup(pal_color);
}
//large .. no ? 
float4 TextureLookup_Palette_Bilinear(float4 uv)
{
	// Find point sampled texels coodinates in image space
	// -> this is correct for bilinear, wrong for anisotropic
	uv.xy/=uv.w;
	
	//store sign/texutre_size (zw is 1/xy)
	float2 sgn=sign(uv.xy)*texture_size.zw;
   
    //Calculate weight/pixel coords
	float2 TexCoordImageSpace = abs(uv.xy) * texture_size.xy;
	
	float2 weight = frac( TexCoordImageSpace );
	
	// Get Texture coodinates for all texels to sample
	// sng = sing/TextureSize
	float4 flce = float4(floor( TexCoordImageSpace ),ceil( TexCoordImageSpace))*sgn.xyxy;
	
	/*
	|(x,y)|(z,y)| -> top_0,top_1
	|(x,w)|(z,w)| -> bot_0,bot_1
	*/
	
	// Use texture coodinates to sample actual texels
	//NON projected lookups (we 'unprojected' to get in image space)
	float4 top_0 = TextureLookup_Palette_Nproj( flce.xy );
	float4 top_1 = TextureLookup_Palette_Nproj( flce.zy );
	float4 bot_0 = TextureLookup_Palette_Nproj( flce.xw );
	float4 bot_1 = TextureLookup_Palette_Nproj( flce.zw ); 
	
	//interpolate
	float4 top = lerp( top_0, top_1, weight.x );
	float4 bot = lerp( bot_0, bot_1, weight.x );
	float4 final = lerp( top, bot, weight.y );
   
    final.a=1;
    final.rb=weight;
    final.g=0;
    
	return( final );
}

//pvr only supports ARGB8888 colors, but they are pre-clamped on the vertex shader (no need to do it here)
float4 PixelShader_main(in pixel s ) : COLOR0
{ 
	float4 color=s.col;
	
	#if pp_UseAlpha==0
		color.a=1;
	#endif
	
	#if pp_Texture==1
		
		//get texture color
		float4 texcol=TextureLookup(s.uv);
		
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
			color.rgb+=s.offs.rgb;
		#endif
	#else
		//we don't realy have anything to do here -- just return the color ...
	#endif
	
	return color; 
}