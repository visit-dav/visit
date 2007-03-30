// ************************************************************************* //
//                          avtOpenGLVolumeRenderer.h                        //
// ************************************************************************* //

#ifndef AVT_OPEN_GL_VOLUME_RENDERER_H
#define AVT_OPEN_GL_VOLUME_RENDERER_H

#include <avtVolumeRenderer.h>


// ****************************************************************************
//  Class: avtOpenGLVolumeRenderer
//
//  Purpose:
//      An implementation of a volume renderer that uses OpenGL calls.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2002
//
// ****************************************************************************

class avtOpenGLVolumeRenderer : public avtVolumeRenderer
{
  public:
                            avtOpenGLVolumeRenderer() {;};
    virtual                ~avtOpenGLVolumeRenderer() {;};

  protected:
    virtual void            Render(vtkDataSet *);
};


#endif


