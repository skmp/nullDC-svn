#pragma once
#include "drkPvr.h"
#include "renderer_if.h"

#if REND_API == REND_D3D
namespace Direct3DRenderer
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
#define rend_init         Direct3DRenderer::InitRenderer
#define rend_term         Direct3DRenderer::TermRenderer
#define rend_reset        Direct3DRenderer::ResetRenderer

#define rend_thread_start Direct3DRenderer::ThreadStart
#define rend_thread_end	  Direct3DRenderer::ThreadEnd
#define rend_vblank       Direct3DRenderer::VBlank
#define rend_start_render Direct3DRenderer::StartRender
#define rend_end_render   Direct3DRenderer::EndRender

#define rend_list_cont Direct3DRenderer::ListCont
#define rend_list_init Direct3DRenderer::ListInit
#define rend_list_srst Direct3DRenderer::SoftReset

#define rend_text_invl Direct3DRenderer::VramLockedWrite
#endif