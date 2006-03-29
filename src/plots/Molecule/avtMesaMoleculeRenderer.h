// ************************************************************************* //
//                          avtMesaMoleculeRenderer.h                        //
// ************************************************************************* //

#ifndef AVT_MESA_MOLECULE_RENDERER_H
#define AVT_MESA_MOLECULE_RENDERER_H

#define avtOpenGLMoleculeRenderer avtMesaMoleculeRenderer
#define avtOpenGLAtomTexturer avtMesaAtomTexturer

#undef SET_IT_BACK
#ifdef AVT_OPEN_GL_MOLECULE_RENDERER_H
    #define  SET_IT_BACK
    #undef   AVT_OPEN_GL_MOLECULE_RENDERER_H
#endif

#include <avtOpenGLMoleculeRenderer.h>

#ifdef SET_IT_BACK
    #define AVT_OPEN_GL_MOLECULE_RENDERER_H
#endif

#undef avtOpenGLMoleculeRenderer
#undef avtOpenGLAtomTexturer

#endif


