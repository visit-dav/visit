// ************************************************************************* //
//                      avtMesa3DTextureVolumeRenderer.h                     //
// ************************************************************************* //

#ifndef AVT_MESA_3D_TEXTURE_VOLUME_RENDERER_H
#define AVT_MESA_3D_TEXTURE_VOLUME_RENDERER_H

#define avtOpenGL3DTextureVolumeRenderer avtMesa3DTextureVolumeRenderer

#undef SET_IT_BACK
#ifdef AVT_OPEN_GL_3D_TEXTURE_VOLUME_RENDERER_H
    #define  SET_IT_BACK
    #undef   AVT_OPEN_GL_3D_TEXTURE_VOLUME_RENDERER_H
#endif

#include <avtOpenGL3DTextureVolumeRenderer.h>

#ifdef SET_IT_BACK
    #define AVT_OPEN_GL_3D_TEXTURE_VOLUME_RENDERER_H
#endif

#undef avtOpenGL3DTextureVolumeRenderer

#endif


