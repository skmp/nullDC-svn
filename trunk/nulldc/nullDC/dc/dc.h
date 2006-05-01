//dc.h
//header for dc.cpp
#pragma once
#include "types.h"
#include "mem/sh4_mem.h"
#include "mem/memutil.h"
#include "sh4/sh4_if.h"

bool Init_DC();
bool Reset_DC(bool Manual);
void Term_DC();
void Start_DC();
void Stop_DC();
void LoadBiosFiles();
bool IsDCInited();
void SwitchCPU_DC();