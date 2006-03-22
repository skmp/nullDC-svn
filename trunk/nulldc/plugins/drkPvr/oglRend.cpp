#include "oglRend.h"
#include "windows.h"


#ifndef NO_OPENGL
//if OpenGL support is not included , fallback to d3d/soft/null rendering

void EnableOpenGL(HWND hWnd, HDC& hDC, HGLRC& hRC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    // get the device context (DC)
    hDC = GetDC( hWnd );

    // set the pixel format for the DC
    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize = sizeof( pfd );
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                  PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;
    iFormat = ChoosePixelFormat( hDC, &pfd );
    SetPixelFormat( hDC, iFormat, &pfd );

    // create and enable the render context (RC)
    hRC = wglCreateContext( hDC );
    wglMakeCurrent( hDC, hRC );
}

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent( NULL, NULL );
    wglDeleteContext( hRC );
    ReleaseDC( hWnd, hDC );
}
void GetOpenGLRenderer(rend_if* rif)
{
}

#else
#include "nullRend.cpp"
void GetOpenGLRenderer(rend_if* rif)
{
	GetNullRenderer(rif);
}
#endif

