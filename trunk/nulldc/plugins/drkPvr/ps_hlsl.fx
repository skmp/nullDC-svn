//Pixel Shader
//Pvr emulation using a pixel shader .. bahh :p
//pp_Texture -> 1 if texture is enabled , 0 if its not
//pp_Offset -> 1 if offset is enabled , 0 if its not (only valid when texture is enabled)
//pp_ShadInstr -> 0 to 3 , see pvr docs , valid only when texture is enabled
//pp_IgnoreTexA -> 1 if on  0 if off , valid only w/ textures on
//pp_UseAlpha -> 1 if on  0 if off , works when no textures are used too ?
//
//misc #defines :
//ZBufferMode -> z buffer mode :p
//ZBufferMode : 0 -> fp fixup (nop)
//ZBufferMode : 1 -> fp Z emu (emulate fp on matnissa bits)
//ZBufferMode : 2 -> rescale  (nop)

//TextureLookup -> function to use for texture lookup.One of TextureLookup_Normal,TextureLookup_Palette,TextureLookup_Palette_Bilinear
struct pixel 
{
	float4 col : TEXCOORD1;
	
	#if pp_Texture==1
		#if pp_Offset==1
			float4 offs : TEXCOORD2;
		#endif
		//uv is now allways passed
	#endif
	
	float4 uv : TEXCOORD0;
};
 
sampler2D samplr : register(s0);
sampler2D tex_pal : register(s1);

float4 current_pal: register(c0);
float4 texture_size: register(c1);

float4 TextureLookup_Normal(float4 uv)
{
	return tex2Dproj( samplr, uv);
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
/*
	Bilinear filtering
	Screen space perspective -> Texture space linear (*W)
	Texture space quad, filtered
*/
float4 TextureLookup_Palette_Bilinear(float4 uv)
{
	float2 tcpoj=uv.xy/uv.w;			//Project texture to 2d tc space
	float2 Img=tcpoj*texture_size.xy-float2(0.5,0.5);	//to image space to get the frac/ceil
	float4 ltrb=float4(floor(Img),ceil(Img))*texture_size.zwzw;//zw=1/xy
	float2 weight=frac(Img);
	
	float4 top_left = TextureLookup_Palette_Nproj( ltrb.xy );
	float4 top_right = TextureLookup_Palette_Nproj( ltrb.zy );
	float4 bot_left = TextureLookup_Palette_Nproj( ltrb.xw );
	float4 bot_right = TextureLookup_Palette_Nproj( ltrb.zw ); 
	
	float4 top = lerp( top_left, top_right, weight.x );	//.x=0 -> left, .x=1 -> right
	float4 bot = lerp( bot_left, bot_right, weight.x );
	float4 final = lerp( top, bot, weight.y );			//.y=0 -> top , .y=1 -> bottom
	return final;
}
float4 TextureLookup_Palette_Bilinear_lol(float4 uv)
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
//compress Z to D{s6e18}S8
float CompressZ(float w)
{
	float x=floor(log2(w));
	float powx=pow(2,x);
	x=clamp(x-16,-63,0);	//s6e18, max : 2^16*(2^18-1)/2(^18) , min : 2^-47*(2^18-1)/2(^18)
	x+=62;					//bias to positive, +1 more is done by the add below.x_max =62,x_min = -1 (63;0)
	float y=(w/powx);		//mantissa bits, allways in [1..2) range as 0 is not a valid input :)
	return (x+y)/64.0f;		//Combine and save the exp + mantissa at the mantissa field.Min value is 0 (-1+1), max value is 63 +(2^18-1)/2(^18).
							//Normalised by 64 so that it falls in the [0..1) range :)
}
struct PSO
{
	float4 col:COLOR0;
	#if ZBufferMode==1
	float  z  :DEPTH;
	#endif
};
//pvr only supports ARGB8888 colors, but they are pre-clamped on the vertex shader (no need to do it here)
PSO PixelShader_main(in pixel s )
{ 
	float4 color=saturate(s.col/s.uv.w);
	clip(s.uv.z);
	
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
			color.rgb+=saturate(s.offs.rgb/s.uv.w);
		#endif
	#else
		//we don't realy have anything to do here -- just return the color ...
	#endif
	
	PSO rv;
	rv.col=color;
	#if ZBufferMode==1
	rv.z=CompressZ(s.uv.w);
	#endif
	
	return rv; 
}

PSO PixelShader_Z(float4 uv : TEXCOORD0)
{
	PSO rv;
	rv.col=float4(0,0,0,0.5f);
	
	#if ZBufferMode==1
	rv.z=CompressZ(uv.w);
	#endif
	
	return rv;
}

float4 PixelShader_ShadeCol() :COLOR0
{
	return  float4(0,0,0,0.5f);
}