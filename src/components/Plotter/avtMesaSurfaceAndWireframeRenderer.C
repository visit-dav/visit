// ************************************************************************* //
//                      avtMesaSurfaceAndWireframeRenderer.C                 //
// ************************************************************************* //

#include <avtMesaSurfaceAndWireframeRenderer.h>

#if !defined(_WIN32)
// Do not include mangled Mesa on windows until we figure out what to do.
#include <MangleMesaInclude/gl_mangle.h>
#endif

#define avtOpenGLSurfaceAndWireframeRenderer avtMesaSurfaceAndWireframeRenderer
#define VTK_IMPLEMENT_MESA_CXX

#include "avtOpenGLSurfaceAndWireframeRenderer.C"

#undef  avtOpenGLSurfaceAndWireframeRenderer

