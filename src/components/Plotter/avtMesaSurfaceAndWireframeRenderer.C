// ************************************************************************* //
//                      avtMesaSurfaceAndWireframeRenderer.C                 //
// ************************************************************************* //

#include <avtMesaSurfaceAndWireframeRenderer.h>

#if !defined(_WIN32)
// Do not include mangled Mesa on windows until we figure out what to do.
#include <GL/gl_mangle.h>
#endif

#define avtOpenGLSurfaceAndWireframeRenderer avtMesaSurfaceAndWireframeRenderer

#include "avtOpenGLSurfaceAndWireframeRenderer.C"

#undef  avtOpenGLSurfaceAndWireframeRenderer

