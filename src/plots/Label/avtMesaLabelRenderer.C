#include <avtMesaLabelRenderer.h>

// Mangle the GL calls to mgl.
#include <GL/gl_mangle.h>

//
// Define avtOpenGLLabelRenderer as avtMesaLabelRenderer so we can include
// the source for the OpenGL Label renderer to get a Mesa label renderer.
//
#define avtOpenGLLabelRenderer avtMesaLabelRenderer

// Include the source
#include <avtOpenGLLabelRenderer.C>

#undef avtOpenGLLabelRenderer
