#include "nullRend.h"

//null renderer is here
void nullPresentFB()
{
}

bool nullInitRenderer(void* window)
{
	return true;
}

void nullTermRenderer()
{
}

void nullResetRenderer(bool Manual)
{
}

bool nullThreadStart(void* window)
{
	return true;
}

void nullThreadEnd()
{
}

//Get null i/f
void GetNullRenderer(rend_if* rif)
{
	//general init/term/reset
	rif->Init=nullInitRenderer;
	rif->Term=nullTermRenderer;
	rif->Reset=nullResetRenderer;
	
	//thread init/term
	rif->ThreadStart=nullThreadStart;
	rif->ThreadEnd=nullThreadEnd;

	//drawing related functions :)
	rif->PresentFB=nullPresentFB;
}