#ifndef AVT_MESA_ATOM_TEXTURER_H
#define AVT_MESA_ATOM_TEXTURER_H

#define avtOpenGLAtomTexturer avtMesaAtomTexturer

#undef SET_IT_BACK
#ifdef AVT_OPEN_GL_MOLECULE_RENDERER_H
    #define  SET_IT_BACK
    #undef   AVT_OPENGL_ATOM_TEXTURER_H
#endif

#include <avtOpenGLAtomTexturer.h>

#ifdef SET_IT_BACK
    #define AVT_OPENGL_ATOM_TEXTURER_H
#endif

#undef avtOpenGLAtomTexturer


#endif
