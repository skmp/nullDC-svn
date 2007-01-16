#pragma once
#include "drkPvr.h"
#include "renderer_if.h"

#if REND_API == REND_SW
namespace NullRenderer
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


	void VramLockedWrite(vram_block* bl);
};
#define rend_init         NullRenderer::InitRenderer
#define rend_term         NullRenderer::TermRenderer
#define rend_reset        NullRenderer::ResetRenderer

#define rend_thread_start NullRenderer::ThreadStart
#define rend_thread_end	  NullRenderer::ThreadEnd
#define rend_vblank       NullRenderer::VBlank
#define rend_start_render NullRenderer::StartRender
#define rend_end_render   NullRenderer::EndRender

#define rend_list_cont NullRenderer::ListCont
#define rend_list_init NullRenderer::ListInit
#define rend_list_srst NullRenderer::SoftReset

#define rend_text_invl NullRenderer::VramLockedWrite
#endif