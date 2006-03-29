// ************************************************************************* //
//                           avtMesaAtomTexturer.C                           //
// ************************************************************************* //

#include <avtMesaAtomTexturer.h>

// Mangle the GL calls to mgl.
#include <GL/gl_mangle.h>

#define avtOpenGLAtomTexturer avtMesaAtomTexturer
#define TextureModeData mesaTextureModeData
#define ShaderModeData mesaShaderModeData
#define VTK_IMPLEMENT_MESA_CXX

#include "avtOpenGLAtomTexturer.C"

#undef  avtOpenGLAtomTexturer
#undef TextureModeData
#undef ShaderModeData
