// ************************************************************************* //
//                           avtMesaVolumeRenderer.C                         //
// ************************************************************************* //

#include <avtMesaVolumeRenderer.h>

#if !defined(_WIN32)
// Don't do this on windows yet until we figure it out.
#include <GL/gl_mangle.h>
#endif

#define avtOpenGLVolumeRenderer avtMesaVolumeRenderer

#include "avtOpenGLVolumeRenderer.C"

#undef  avtOpenGLVolumeRenderer


