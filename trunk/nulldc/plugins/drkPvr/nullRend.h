#pragma once
#include "drkPvr.h"
#include "renderer_if.h"

#if REND_API == REND_SW
namespace SWRenderer
{

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

	void VramLockedWrite(vram_block* bl);
};
#define rend_init         SWRenderer::InitRenderer
#define rend_term         SWRenderer::TermRenderer
#define rend_reset        SWRenderer::ResetRenderer

#define rend_thread_start SWRenderer::ThreadStart
#define rend_thread_end	  SWRenderer::ThreadEnd
#define rend_vblank       SWRenderer::VBlank
#define rend_start_render SWRenderer::StartRender
#define rend_end_render   SWRenderer::EndRender

#define rend_list_cont SWRenderer::ListCont
#define rend_list_init SWRenderer::ListInit
#define rend_list_srst SWRenderer::SoftReset

#define rend_text_invl SWRenderer::VramLockedWrite
#define rend_set_fps_text SWRenderer::SetFpsText
#define rend_set_render_rect(rect) 
#endif