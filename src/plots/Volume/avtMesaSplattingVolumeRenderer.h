// ************************************************************************* //
//                      avtMesaSplattingVolumeRenderer.h                     //
// ************************************************************************* //

#ifndef AVT_MESA_SPLATTING_VOLUME_RENDERER_H
#define AVT_MESA_SPLATTING_VOLUME_RENDERER_H

#define avtOpenGLSplattingVolumeRenderer avtMesaSplattingVolumeRenderer

#undef SET_IT_BACK
#ifdef AVT_OPEN_GL_SPLATTING_VOLUME_RENDERER_H
    #define  SET_IT_BACK
    #undef   AVT_OPEN_GL_SPLATTING_VOLUME_RENDERER_H
#endif

#include <avtOpenGLSplattingVolumeRenderer.h>

#ifdef SET_IT_BACK
    #define AVT_OPEN_GL_SPLATTING_VOLUME_RENDERER_H
#endif

#undef avtOpenGLSplattingVolumeRenderer

#endif


