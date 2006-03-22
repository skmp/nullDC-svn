#pragma once
#include "..\types.h"
#include "..\dc\sh4\sh4_if.h"

bool CreateGUI();
void DestroyGUI();
void GuiLoop();

void* GetRenderTargetHandle();