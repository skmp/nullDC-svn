/*
**	DInput.h
*/
#pragma once






bool GetDInput(u32 port, Controller_ReadFormat *crf);
bool InitDInput(HINSTANCE hInst);
void TermDInput();