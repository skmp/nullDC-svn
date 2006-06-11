////////////////////////////////////////////////////////////////////////////////////////
/// @file  pvr.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef _pvr_h
#define _pvr_h

////////////////////////////////////////////////////////////////////////////////////////
//  PowerVR Registers
////////////////////////////////////////////////////////////////////////////////////////

struct TPVR
{
enum
{
	PVR_OPB_CFG = 0x005f8140,

	PVR_OPB_CFG_OP_SHIFT = 0x0,
	PVR_OPB_CFG_OP_MASK = (3<<PVR_OPB_CFG_OP_SHIFT),
	PVR_OPB_CFG_OM_SHIFT = 4,
	PVR_OPB_CFG_OM_MASK = (3<<PVR_OPB_CFG_OM_SHIFT),
	PVR_OPB_CFG_TP_SHIFT = 8,
	PVR_OPB_CFG_TP_MASK = (3<<PVR_OPB_CFG_TP_SHIFT),
	PVR_OPB_CFG_TM_SHIFT = 12,
	PVR_OPB_CFG_TM_MASK = (3<<PVR_OPB_CFG_TM_SHIFT),
	PVR_OPB_CFG_PT_SHIFT = 16,
	PVR_OPB_CFG_PT_MASK = (3<<PVR_OPB_CFG_PT_SHIFT),

	PVR_VPOS_IRQ		= 0x005f80cc, /* Vertical position IRQ */

	PVR_VPOS_IRQ_SCAN1_SHIFT = 0,		
	PVR_VPOS_IRQ_SCAN1_MASK = (1023<<PVR_VPOS_IRQ_SCAN1_SHIFT),		
	PVR_VPOS_IRQ_SCAN2_SHIFT = 16,		
	PVR_VPOS_IRQ_SCAN2_MASK = (1023<<PVR_VPOS_IRQ_SCAN2_SHIFT),		

	PVR_BGPLANE_Z		= 0x005f8088, /* Distance for background plane */
	PVR_RENDER_ADDR		= 0x005f8060,		/* Render output address */
	PVR_DISPLAY_ADDR1 = 0x005f8050, /* Address in VRAM for displaying odd lines */

	PVR_FB_CFG_1		= 0x005f8044,		/* Framebuffer config 1 */

	PVR_FB_CFG_1_C_SHIFT = 23,  /* Clock double enable  */
	PVR_FB_CFG_1_C_MASK = (1 << PVR_FB_CFG_1_C_SHIFT),

	PVR_FB_CFG_1_COL_RGB555 = 0,  /* Bytes per pixel 2 */
	PVR_FB_CFG_1_COL_RGB565 = 1,  /* Bytes per pixel 2 */
	PVR_FB_CFG_1_COL_RGB888 = 2,  /* Bytes per pixel 3 */
	PVR_FB_CFG_1_COL_XRGB888 = 3,  /* Bytes per pixel 4 */

	PVR_FB_CFG_1_COL_SHIFT = 2,  /* Colour mode select  */
	PVR_FB_CFG_1_COL_MASK = (3 << PVR_FB_CFG_1_COL_SHIFT),

	PVR_FB_CFG_1_SD_SHIFT = 1,  /* scan double enable  */
	PVR_FB_CFG_1_SD_MASK = (1 << PVR_FB_CFG_1_SD_SHIFT),

	PVR_FB_CFG_1_DE_SHIFT = 0,  /* display enable  */
	PVR_FB_CFG_1_DE_MASK = (1 << PVR_FB_CFG_1_DE_SHIFT),

	PVR_FB_CFG_2		= 0x005f8048,		/* Framebuffer config 2 */

	PVR_FB_CFG_2_TH_SHIFT = 16,  /* Alpha threshold   */
	PVR_FB_CFG_2_TH_MASK = (255 << PVR_FB_CFG_2_TH_SHIFT),

	PVR_FB_CFG_2_D_SHIFT = 3,  /* dither enable   */
	PVR_FB_CFG_2_D_MASK = (1 << PVR_FB_CFG_2_D_SHIFT),

	PVR_FB_CFG_2_COL_SHIFT = 0,  /* colour mode select   */
	PVR_FB_CFG_2_COL_MASK = (3 << PVR_FB_CFG_2_COL_SHIFT),

	PVR_DISPLAY_SIZE		= 0x005f805c,		/* display size and modulo */

	PVR_DISPLAY_SIZE_MODULO_SHIFT = 20,  /* modulo   */
	PVR_DISPLAY_SIZE_MODULO_MASK = (1023 << PVR_DISPLAY_SIZE_MODULO_SHIFT),

	PVR_DISPLAY_SIZE_LPF_SHIFT = 10,  /* Lines per field   */
	PVR_DISPLAY_SIZE_LPF_MASK = (1023 << PVR_DISPLAY_SIZE_LPF_SHIFT ),

	PVR_DISPLAY_SIZE_PDPL_SHIFT = 0,  /* pixel data per line   */
	PVR_DISPLAY_SIZE_PDPL_MASK = (1023 << PVR_DISPLAY_SIZE_PDPL_SHIFT ),

	PVR_VIDEO_CFG		= 0x005f80e8,		/* Misc video config */
	PVR_VIDEO_CFG_LR_SHIFT = 8, /* Low-res; setting this bit makes each pixel be output twice */
	PVR_VIDEO_CFG_LR_MASK = (1 << PVR_VIDEO_CFG_LR_SHIFT),

	PVR_SYNC_CFG = 0x005f80d0, 

	PVR_SYNC_CFG_I_SHIFT = 4, /* interlace mode */
	PVR_SYNC_CFG_I_MASK = (1<<PVR_SYNC_CFG_I_SHIFT),

	PVR_SYNC_CFG_E_SHIFT = 4, /* video output enable */
	PVR_SYNC_CFG_E_MASK = (1<<PVR_SYNC_CFG_E_SHIFT),

	PVR_LIST_OP_POLY		= 0,	/* opaque poly */
	PVR_LIST_OP_MOD			= 1,	/* opaque modifier */
	PVR_LIST_TR_POLY		= 2,	/* translucent poly */
	PVR_LIST_TR_MOD			= 3,	/* translucent modifier */
	PVR_LIST_PT_POLY		= 4,	/* punch-thru poly */

	PVR_CT_VGA		= 0, /* cable type */
	PVR_CT_RGB		= 2,
	PVR_CT_COMPOSITE	= 3,

	PVR_ID = 0x005f8000, 
	PVR_REVISION = 0x005f8004, 

	PVR_TA_INIT	= 0x005f8144, 		/* Initialize vertex reg. params */

	PVR_PT_ALPHA_REF = 0x005F811C,        /* alfa comp value for PT polys */

	PVR_TA_VERTBUF_START	= 0x005f8128,		/* Vertex buffer start for TA usage */
	PVR_TA_VERTBUF_POS	= 0x005f8138,		/* Top used memory location in vertbuf for TA usage */
	PVR_TA_OPB_START	= 0x005f8124,		/* Object Pointer Buffer start for TA usage */
	PVR_TA_OPB_END		= 0x005f812c,		/* OPB end for TA usage */
	PVR_TA_OPB_POS		= 0x005f8134,		/* Top used memory location in OPB for TA usage */

	PVR_BGPLANE_CFG		= 0x005f808c,		/* Background plane config */

	PVR_BGPLANE_CFG_ADDR_SHIFT = 0x3,
	PVR_BGPLANE_CFG_ADDR_MASK = (0x1FFFFF<<PVR_BGPLANE_CFG_ADDR_SHIFT),

	PVR_BGPLANE_CFG_ISP_SHIFT = 24,
	PVR_BGPLANE_CFG_ISP_MASK = (0x7<<PVR_BGPLANE_CFG_ISP_SHIFT),

	PVR_TEXTURE_MODULO = 0x005f80e4, /* Output texture width modulo */

	PVR_TEXTURE_MODULO_WIDTH_SHIFT = 0x0,
	PVR_TEXTURE_MODULO_WIDTH_MASK = (31<<PVR_TEXTURE_MODULO_WIDTH_SHIFT),

	PVR_YUV_ADDR		= 0x005f8148,		/* YUV conversion destination */
	PVR_YUV_CFG 		= 0x005f814c,		/* YUV configuration */
	PVR_YUV_TEX_CNT	= 0x005f8150,		/* YUV configuration 2*/

	SPG_STATUS = 0x05f810c,
	ISP_FEED_CFG = 0x005F8098,
	FPU_PARAM_CFG = 0x005F807C,

	FPU_PARAM_CFG_REGTYPE_SHIFT = 21,
	FPU_PARAM_CFG_REGTYPE_MASK = (0x1<<FPU_PARAM_CFG_REGTYPE_SHIFT),

	REGION_BASE = 0x005F802C,

	PARAM_BASE =  0x005F8020,

	PVR_SYSTEM_RESET = 0x005F6890,
};

};


////////////////////////////////////////////////////////////////////////////////////////
//  PowerVR Helper defines
////////////////////////////////////////////////////////////////////////////////////////

#define PVR_CLRFMT_ARGBPACKED		0	/* color_format */
#define PVR_CLRFMT_4FLOATS		1
#define PVR_CLRFMT_INTENSITY		2
#define PVR_CLRFMT_INTENSITY_PREV	3

#define PVR_FILTER_POINT_SAMPLED 0

#define PVR_UVFMT_32BIT			0	/* txr_uv_format */
#define PVR_UVFMT_16BIT			1

#define PVR_TA_CMD_TYPE_SHIFT		29
#define PVR_TA_CMD_TYPE_MASK		(7 << PVR_TA_CMD_TYPE_SHIFT)

#define PVR_TA_PM0_TYPE_SHIFT		24
#define PVR_TA_PM0_TYPE_MASK		(7 << PVR_TA_PM0_TYPE_SHIFT)

#define PVR_TA_PM0_STRIPLENGTH_SHIFT 18
#define PVR_TA_PM0_STRIPLENGTH_MASK (3 << PVR_TA_PM0_STRIPLENGTH_SHIFT)

#define PVR_TA_PM0_USERCLIP_SHIFT	16
#define PVR_TA_PM0_USERCLIP_MASK	(3 << PVR_TA_PM0_USERCLIP_SHIFT)

#define PVR_TA_PM0_CLRFMT_SHIFT		4
#define PVR_TA_PM0_CLRFMT_MASK		(3 << PVR_TA_PM0_CLRFMT_SHIFT)

#define PVR_TA_PM0_SHADE_SHIFT		1
#define PVR_TA_PM0_SHADE_MASK		(1 << PVR_TA_PM0_SHADE_SHIFT)

#define PVR_TA_PM0_UVFMT_SHIFT		0
#define PVR_TA_PM0_UVFMT_MASK		(1 << PVR_TA_PM0_UVFMT_SHIFT)

#define PVR_TA_PM0_MODIFIER_SHIFT	7
#define PVR_TA_PM0_MODIFIER_MASK	(1 <<  PVR_TA_PM0_MODIFIER_SHIFT)

#define PVR_TA_PM0_MODIFIERMODE_SHIFT	6
#define PVR_TA_PM0_MODIFIERMODE_MASK	(1 <<  PVR_TA_PM0_MODIFIERMODE_SHIFT)

#define PVR_TA_PM0_TXRENABLE_SHIFT	3
#define PVR_TA_PM0_TXRENABLE_MASK	(1 << PVR_TA_PM0_TXRENABLE_SHIFT)

#define PVR_TA_PM1_DEPTHCMP_SHIFT	29
#define PVR_TA_PM1_DEPTHCMP_MASK	(7 << PVR_TA_PM1_DEPTHCMP_SHIFT)

#define PVR_TA_PM1_CULLING_SHIFT	27
#define PVR_TA_PM1_CULLING_MASK		(3 << PVR_TA_PM1_CULLING_SHIFT)

#define PVR_TA_PM1_DEPTHWRITE_SHIFT	26
#define PVR_TA_PM1_DEPTHWRITE_MASK	(1 << PVR_TA_PM1_DEPTHWRITE_SHIFT)

#define PVR_TA_PM1_OFFSET_SHIFT	24
#define PVR_TA_PM1_OFFSET_MASK	(1 << PVR_TA_PM1_OFFSET_SHIFT)

#define PVR_TA_PM1_GORAUD_SHIFT	23
#define PVR_TA_PM1_GORAUD_MASK	(1 << PVR_TA_PM1_GORAUD_SHIFT)

#define PVR_TA_PM1_MODIFIERINST_SHIFT	29
#define PVR_TA_PM1_MODIFIERINST_MASK	(2 <<  PVR_TA_PM1_MODIFIERINST_SHIFT)

#define PVR_TA_PM2_SRCBLEND_SHIFT	29
#define PVR_TA_PM2_SRCBLEND_MASK	(7 << PVR_TA_PM2_SRCBLEND_SHIFT)

#define PVR_TA_PM2_DSTBLEND_SHIFT	26
#define PVR_TA_PM2_DSTBLEND_MASK	(7 << PVR_TA_PM2_DSTBLEND_SHIFT)

#define PVR_TA_PM2_SRCENABLE_SHIFT	25
#define PVR_TA_PM2_SRCENABLE_MASK	(1 << PVR_TA_PM2_SRCENABLE_SHIFT)

#define PVR_TA_PM2_DSTENABLE_SHIFT	24
#define PVR_TA_PM2_DSTENABLE_MASK	(1 << PVR_TA_PM2_DSTENABLE_SHIFT)

#define PVR_TA_PM2_FOG_SHIFT		22
#define PVR_TA_PM2_FOG_MASK		(3 << PVR_TA_PM2_FOG_SHIFT)

#define PVR_TA_PM2_CLAMP_SHIFT		21
#define PVR_TA_PM2_CLAMP_MASK		(1 << PVR_TA_PM2_CLAMP_SHIFT)

#define PVR_TA_PM2_ALPHA_SHIFT		20
#define PVR_TA_PM2_ALPHA_MASK		(1 << PVR_TA_PM2_ALPHA_SHIFT)

#define PVR_TA_PM2_TXRALPHA_SHIFT	19
#define PVR_TA_PM2_TXRALPHA_MASK	(1 << PVR_TA_PM2_TXRALPHA_SHIFT)

#define PVR_TA_PM2_UVFLIP_SHIFT		17
#define PVR_TA_PM2_UVFLIP_MASK		(3 << PVR_TA_PM2_UVFLIP_SHIFT)

#define PVR_TA_PM2_UVCLAMP_SHIFT	15
#define PVR_TA_PM2_UVCLAMP_MASK		(3 << PVR_TA_PM2_UVCLAMP_SHIFT)

#define PVR_TA_PM2_FILTER_SHIFT		12
#define PVR_TA_PM2_FILTER_MASK		(7 << PVR_TA_PM2_FILTER_SHIFT)

#define PVR_TA_PM2_MIPBIAS_SHIFT	8
#define PVR_TA_PM2_MIPBIAS_MASK		(15 << PVR_TA_PM2_MIPBIAS_SHIFT)

#define PVR_TA_PM2_TXRENV_SHIFT		6
#define PVR_TA_PM2_TXRENV_MASK		(3 << PVR_TA_PM2_TXRENV_SHIFT)

#define PVR_TA_PM2_USIZE_SHIFT		3
#define PVR_TA_PM2_USIZE_MASK		(7 << PVR_TA_PM2_USIZE_SHIFT)

#define PVR_TA_PM2_VSIZE_SHIFT		0
#define PVR_TA_PM2_VSIZE_MASK		(7 << PVR_TA_PM2_VSIZE_SHIFT)

#define PVR_TA_PM3_MIPMAP_SHIFT		31
#define PVR_TA_PM3_MIPMAP_MASK		(1 << PVR_TA_PM3_MIPMAP_SHIFT)

#define PVR_TA_PM3_VQ_SHIFT		30
#define PVR_TA_PM3_VQ_MASK		(1 << PVR_TA_PM3_VQ_SHIFT)

#define PVR_TA_PM3_TXRADR_SHIFT		0
#define PVR_TA_PM3_TXRADR_MASK		0x1FFFFF


#define PVR_TA_PM3_TXRFMT_SHIFT		27
#define PVR_TA_PM3_TXRFMT_MASK		(7 << PVR_TA_PM3_TXRFMT_SHIFT)

#define PVR_TA_PM3_TXRCLUT4_SHIFT	 21
#define PVR_TA_PM3_TXRCLUT4_MASK	 (63 << PVR_TA_PM3_TXRCLUT4_SHIFT)

#define PVR_TA_PM3_TXRCLUT8_SHIFT	 25
#define PVR_TA_PM3_TXRCLUT8_MASK	 (3 << PVR_TA_PM3_TXRCLUT8_SHIFT)

#define PVR_TA_PM3_TWIDDLE_SHIFT	 26
#define PVR_TA_PM3_TWIDDLE_MASK	 (1 << PVR_TA_PM3_TWIDDLE_SHIFT)

#define PVR_TA_PM3_STRIDE_SHIFT	 25
#define PVR_TA_PM3_STRIDE_MASK	 (1 << PVR_TA_PM3_STRIDE_SHIFT)


#define PVR_TA_VX0_EOS_SHIFT			28
#define PVR_TA_VX0_EOS_MASK				(1<<PVR_TA_VX0_EOS_SHIFT)


#define PVR_UVFLIP_NONE		0	/* txr_uvflip */
#define PVR_UVFLIP_V			1
#define PVR_UVFLIP_U			2
#define PVR_UVFLIP_UV			3

#define PVR_UVCLAMP_NONE	0	/* txr_uvclamp */
#define PVR_UVCLAMP_V			1
#define PVR_UVCLAMP_U			2
#define PVR_UVCLAMP_UV		3


#define POLYGON_LISTTYPE(a)							(((a[0])>>24)&0x7)
#define POLYGON_STRIPLENGTH(a)					(((a[0])>>18)&0x3)
#define POLYGON_CLIPMODE(a)							(((a[0])>>16)&0x3)
#define POLYGON_MODIFIERMODE(a)					(((a[0])>>6)&0x1)

#define POLYGON_COLOURTYPE(a)						(((a[0])&PVR_TA_PM0_CLRFMT_MASK)>>PVR_TA_PM0_CLRFMT_SHIFT)
#define POLYGON_TEXTURE(a)							(((a[0])&PVR_TA_PM0_TXRENABLE_MASK)>>PVR_TA_PM0_TXRENABLE_SHIFT)
#define POLYGON_UVFORMAT(a)							(((a[0])&PVR_TA_PM0_UVFMT_MASK)>>PVR_TA_PM0_UVFMT_SHIFT)


#define POLYGON_SPECULAR(a)							(((a[0])>>2)&0x1)
#define POLYGON_SHADING(a)							(((a[0])>>1)&0x1)


#define POLYGON_USIZE(a)								(1<<((((a[2])&PVR_TA_PM2_USIZE_MASK)>>PVR_TA_PM2_USIZE_SHIFT)+3))
#define POLYGON_VSIZE(a)								(1<<((((a[2])&PVR_TA_PM2_VSIZE_MASK)>>PVR_TA_PM2_VSIZE_SHIFT)+3))

#define POLYGON_FILTER(a)               (((a[2])&PVR_TA_PM2_FILTER_MASK)>>PVR_TA_PM2_FILTER_SHIFT)


#define POLYGON_TXRFMT(a)								(((a[3])&PVR_TA_PM3_TXRFMT_MASK)>>PVR_TA_PM3_TXRFMT_SHIFT)
#define POLYGON_TXRADR(a)								(((a[3])&PVR_TA_PM3_TXRADR_MASK)>>PVR_TA_PM3_TXRADR_SHIFT)

#define POLYGON_TXRCLUT4(a)							(((a[3])&PVR_TA_PM3_TXRCLUT4_MASK)>>PVR_TA_PM3_TXRCLUT4_SHIFT)

#define POLYGON_TXRCLUT8(a)							(((a[3])&PVR_TA_PM3_TXRCLUT8_MASK)>>PVR_TA_PM3_TXRCLUT8_SHIFT)

#define POLYGON_MIPMAP(a)							  (((a[3])&PVR_TA_PM3_MIPMAP_MASK)>>PVR_TA_PM3_MIPMAP_SHIFT)

#define POLYGON_VQ(a)							      (((a[3])&PVR_TA_PM3_VQ_MASK)>>PVR_TA_PM3_VQ_SHIFT)
#define POLYGON_NONTWIDDLE(a)					  (((a[3])&PVR_TA_PM3_TWIDDLE_MASK)>>PVR_TA_PM3_TWIDDLE_SHIFT)
#define POLYGON_STRIDE(a)							  (((a[3])&PVR_TA_PM3_STRIDE_MASK)>>PVR_TA_PM3_STRIDE_SHIFT)


#define POLYGON_DEPTHMODE(a)				    (((a[1])&PVR_TA_PM1_DEPTHCMP_MASK)>>PVR_TA_PM1_DEPTHCMP_SHIFT)
#define POLYGON_DEPTHWRITE(a)				    (((a[1])&PVR_TA_PM1_DEPTHWRITE_MASK)>>PVR_TA_PM1_DEPTHWRITE_SHIFT)
#define POLYGON_CULLING(a)					    (((a[1])&PVR_TA_PM1_CULLING_MASK)>>PVR_TA_PM1_CULLING_SHIFT)


#define POLYGON_OFFSET_ON(a)					  (((a[1])&PVR_TA_PM1_OFFSET_MASK)>>PVR_TA_PM1_OFFSET_SHIFT)
#define POLYGON_GORAUD(a)					      (((a[1])&PVR_TA_PM1_GORAUD_MASK)>>PVR_TA_PM1_GORAUD_SHIFT)

#define POLYGON_SRCBLEND(a)					    (((a[2])&PVR_TA_PM2_SRCBLEND_MASK)>>PVR_TA_PM2_SRCBLEND_SHIFT)
#define POLYGON_DSTBLEND(a)					    (((a[2])&PVR_TA_PM2_DSTBLEND_MASK)>>PVR_TA_PM2_DSTBLEND_SHIFT)

#define POLYGON_SRCENABLE(a)					  (((a[2])&PVR_TA_PM2_SRCENABLE_MASK)>>PVR_TA_PM2_SRCENABLE_SHIFT)
#define POLYGON_DSTENABLE(a)					  (((a[2])&PVR_TA_PM2_DSTENABLE_MASK)>>PVR_TA_PM2_DSTENABLE_SHIFT)

#define POLYGON_ALPHA(a)					      (((a[2])&PVR_TA_PM2_ALPHA_MASK)>>PVR_TA_PM2_ALPHA_SHIFT)
#define POLYGON_NO_TXRALPHA(a)				  (((a[2])&PVR_TA_PM2_TXRALPHA_MASK)>>PVR_TA_PM2_TXRALPHA_SHIFT)

#define POLYGON_MODIFIER(a)				      (((a[0])&PVR_TA_PM0_MODIFIER_MASK)>>PVR_TA_PM0_MODIFIER_SHIFT)


#define POLYGON_TXRSHADING(a)				    (((a[2])&PVR_TA_PM2_TXRENV_MASK)>>PVR_TA_PM2_TXRENV_SHIFT)


#define POLYGON_UVFLIP(a)				        (((a[2])&PVR_TA_PM2_UVFLIP_MASK)>>PVR_TA_PM2_UVFLIP_SHIFT)
#define POLYGON_UVCLAMP(a)				      (((a[2])&PVR_TA_PM2_UVCLAMP_MASK)>>PVR_TA_PM2_UVCLAMP_SHIFT)


#define CMD_TYPE(a)											(((a[0])&PVR_TA_CMD_TYPE_MASK)>>PVR_TA_CMD_TYPE_SHIFT)
#define VERTEX_EOS(a)										(((a[0])&PVR_TA_VX0_EOS_MASK)>>PVR_TA_VX0_EOS_SHIFT)
#define COMPUTE_ADDRESS_TXR(a)					(((POLYGON_TXRADR(a))<<3)&(g_uVideoMemorySize-1))





////////////////////////////////////////////////////////////////////////////////////////
#endif