// ************************************************************************* //
//                       avtMesaSplattingVolumeRenderer.C                    //
// ************************************************************************* //

#include <avtMesaSplattingVolumeRenderer.h>

// Mangle the GL calls to mgl.
#include <GL/gl_mangle.h>

#define avtOpenGLSplattingVolumeRenderer avtMesaSplattingVolumeRenderer

#include "avtOpenGLSplattingVolumeRenderer.C"

#undef  avtOpenGLSplattingVolumeRenderer


