// ************************************************************************* //
//                           avtMesaVolumeRenderer.h                         //
// ************************************************************************* //

#ifndef AVT_MESA_VOLUME_RENDERER_H
#define AVT_MESA_VOLUME_RENDERER_H

#define avtOpenGLVolumeRenderer avtMesaVolumeRenderer

#undef SET_IT_BACK
#ifdef AVT_OPEN_GL_VOLUME_RENDERER_H
    #define  SET_IT_BACK
    #undef   AVT_OPEN_GL_VOLUME_RENDERER_H
#endif

#include <avtOpenGLVolumeRenderer.h>

#ifdef SET_IT_BACK
    #define AVT_OPEN_GL_VOLUME_RENDERER_H
#endif

#undef avtOpenGLVolumeRenderer

#endif


