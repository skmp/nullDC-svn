//Global header for v1 dynarec
#pragma once
#include <List>
#include "types.h"
#include "dc\sh4\shil\shil.h"

//count fallbacks and average block size, average execution time , ect :)
//#define PROFILE_DYNAREC
//call asm using a second function , so we can profile it using a profiler :)
#define PROFILE_DYNAREC_CALL
void printprofile();
//disable register allocation
#define DYNAREC_REG_ALLOC (true)

typedef u32 rec_v1_BasicBlockEP();


using namespace std;   