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

float4 PixelShader_ShadeCol() : SV_Target
{
	return  float4(1,1,1,1);
}