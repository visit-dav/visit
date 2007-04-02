// ************************************************************************* //
//                           avtMesaMoleculeRenderer.C                       //
// ************************************************************************* //

#include <avtMesaMoleculeRenderer.h>

// Mangle the GL calls to mgl.
#include <GL/gl_mangle.h>

#define avtOpenGLMoleculeRenderer avtMesaMoleculeRenderer
#define avtOpenGLAtomTexturer avtMesaAtomTexturer

#define VTK_IMPLEMENT_MESA_CXX

#include "avtOpenGLMoleculeRenderer.C"

#undef  avtOpenGLMoleculeRenderer
#undef  avtOpenGLAtomTexturer


