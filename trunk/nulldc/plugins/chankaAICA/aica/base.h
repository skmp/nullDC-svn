#pragma once
#include <windows.h>

typedef enum { RET_OK = 0, RET_FAIL = 1 } TError;

#define ARRAY_LEN(a) (sizeof(a)/sizeof(*(a)))

#define READ8LE(x) \
	*((BYTE *)x)
#define READ16LE(x) \
	*((WORD *)x)
#define READ32LE(x) \
	*((DWORD *)x)
#define WRITE8LE(x,v) \
	*((BYTE *)x) = (v)
#define WRITE16LE(x,v) \
	*((WORD *)x) = (v)
#define WRITE32LE(x,v) \
	*((DWORD *)x) = (v)


#define SWAP_LE_BE(val) ((DWORD) ( \
	(((DWORD) (val) & (DWORD) 0x000000ffU) << 24) | \
	(((DWORD) (val) & (DWORD) 0x0000ff00U) <<  8) | \
	(((DWORD) (val) & (DWORD) 0x00ff0000U) >>  8) | \
	(((DWORD) (val) & (DWORD) 0xff000000U) >> 24)))


// ----------------------------------------------------------------------------------------

/*
#ifndef _VERSION_FINAL
void __cdecl operator delete( void *pvMem, const char* pszFichero, int iLinea );
inline void* __cdecl operator new(size_t nSize, const char * lpszFileName, int nLine)
{
return ::operator new(nSize, 1, lpszFileName, nLine);
}
#define NEW(obj) new(__FILE__, __LINE__) obj
#else
#define NEW(obj) new obj

#endif // _DEBUG
*/

#define NEW(obj) new obj


#define DISPOSE(obj)	\
	do {									\
	delete (obj);				\
	(obj) = NULL;				\
	}	while(0)						

#define DISPOSE_ARRAY(obj)	\
	do {												\
	delete [](obj);						\
	(obj) = NULL;							\
	}	while(0)									

#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#define ZEROMEM(obj,size) memset((obj),0,(size))


// ----------------------------------------------------------------------------------------
// Assert
// ----------------------------------------------------------------------------------------

#if defined(_DEBUG) || defined(_RELEASE_ASSERT)

extern void LogVentanaDebug( const char *szFichero, int linea, const char *szExpr );
extern bool VentanaDebug( const char *szFichero, int linea, const char *szExpr );

#define ASSERT(expr)	                do { \
	static bool bVerAsserts = true;                                \
	if (bVerAsserts && !(expr))                                    \
{												                                      \
	LogVentanaDebug( __FILE__, __LINE__, #expr );	              \
	__asm { int 3	}                                             \
	bVerAsserts = VentanaDebug( __FILE__, __LINE__, #expr );	    \
}                                                              \
} while (0)
#else
#define ASSERT __noop
#endif


#ifndef _VERSION_FINAL
void ods( const char* pszTexto, ... );
#define ODS(expr) ods expr
#define LOG(expr) ods expr
#else
#define ODS(expr) __noop
#define LOG(expr) __noop
#endif


