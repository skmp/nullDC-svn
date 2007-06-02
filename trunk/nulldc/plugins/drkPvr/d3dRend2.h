#pragma once
#include "drkPvr.h"
#include "renderer_if.h"

#if REND_API == REND_D3D_V2
namespace Direct3DRenderer2
{
	extern bool IsFullscreen;

	bool InitRenderer();
	void TermRenderer();
	void ResetRenderer(bool Manual);
	
	bool ThreadStart();
	void ThreadEnd();
	void VBlank();
	void StartRender();
	void EndRender();

	void ListCont();
	void ListInit();
	void SoftReset();

	void SetFpsText(char* text);
	void SetRenderRect(float* rect);
	void SetFBScale(float x,float y);

	void VramLockedWrite(vram_block* bl);
};
#define rend_init         Direct3DRenderer2::InitRenderer
#define rend_term         Direct3DRenderer2::TermRenderer
#define rend_reset        Direct3DRenderer2::ResetRenderer

#define rend_thread_start Direct3DRenderer2::ThreadStart
#define rend_thread_end	  Direct3DRenderer2::ThreadEnd
#define rend_vblank       Direct3DRenderer2::VBlank
#define rend_start_render Direct3DRenderer2::StartRender
#define rend_end_render   Direct3DRenderer2::EndRender

#define rend_list_cont Direct3DRenderer2::ListCont
#define rend_list_init Direct3DRenderer2::ListInit
#define rend_list_srst Direct3DRenderer2::SoftReset

#define rend_text_invl Direct3DRenderer2::VramLockedWrite
#define rend_is_fullscreen Direct3DRenderer2::IsFullscreen
#define rend_set_fps_text Direct3DRenderer2::SetFpsText
#define rend_set_render_rect Direct3DRenderer2::SetRenderRect
#define rend_set_fb_scale Direct3DRenderer2::SetFBScale
#endif