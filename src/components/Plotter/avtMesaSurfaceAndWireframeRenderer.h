// ************************************************************************* //
//                    avtMesaSurfaceAndWireframeRenderer.h                   //
// ************************************************************************* //

#ifndef AVT_MESA_SURFACE_AND_WIREFRAME_RENDERER_H
#define AVT_MESA_SURFACE_AND_WIREFRAME_RENDERER_H

#define avtOpenGLSurfaceAndWireframeRenderer avtMesaSurfaceAndWireframeRenderer

#undef SET_IT_BACK
#ifdef AVT_OPEN_GL_SURFACE_AND_WIREFRAME_RENDERER_H
    #define  SET_IT_BACK
    #undef   AVT_OPEN_GL_SURFACE_AND_WIREFRAME_RENDERER_H
#endif

#include <avtOpenGLSurfaceAndWireframeRenderer.h>

#ifdef SET_IT_BACK
    #define AVT_OPEN_GL_SURFACE_AND_WIREFRAME_RENDERER_H
#endif

#undef avtOpenGLSurfaceAndWireframeRenderer

#endif


