// ************************************************************************* //
//                       avtMesaSplattingVolumeRenderer.C                    //
// ************************************************************************* //

#include <avtMesaSplattingVolumeRenderer.h>

// Mangle the GL calls to mgl.
#include <GL/gl_mangle.h>

#define avtOpenGLSplattingVolumeRenderer avtMesaSplattingVolumeRenderer
#define VTK_IMPLEMENT_MESA_CXX

#include "avtOpenGLSplattingVolumeRenderer.C"

#undef  avtOpenGLSplattingVolumeRenderer


