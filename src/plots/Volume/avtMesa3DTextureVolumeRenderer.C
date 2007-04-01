// ************************************************************************* //
//                       avtMesa3DTextureVolumeRenderer.C                    //
// ************************************************************************* //

#include <avtMesa3DTextureVolumeRenderer.h>

#if !defined(_WIN32) || (defined(_WIN32) && defined(USING_MSVC7))
// Don't do this on windows yet until we figure it out.
#include <GL/gl_mangle.h>
#endif

#define avtOpenGL3DTextureVolumeRenderer avtMesa3DTextureVolumeRenderer

#include "avtOpenGL3DTextureVolumeRenderer.C"

#undef  avtOpenGL3DTextureVolumeRenderer


