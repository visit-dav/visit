// ************************************************************************* //
//                       avtMesaSplattingVolumeRenderer.C                    //
// ************************************************************************* //

#include <avtMesaSplattingVolumeRenderer.h>

#if !defined(_WIN32)
// Don't do this on windows yet until we figure it out.
#include <GL/gl_mangle.h>
#endif

#define avtOpenGLSplattingVolumeRenderer avtMesaSplattingVolumeRenderer

#include "avtOpenGLSplattingVolumeRenderer.C"

#undef  avtOpenGLSplattingVolumeRenderer


