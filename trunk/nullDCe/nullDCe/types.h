//psp uses the same naming
//damn bad luck ;)
#ifdef HOST_PSP
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#else
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
#endif

typedef u32 uptr;
typedef s32 sptr;

typedef float f32;
typedef double f64;