// ************************************************************************* //
//                     avtOpenGLSplattingVolumeRenderer.h                    //
// ************************************************************************* //

#ifndef AVT_OPEN_GL_SPLATTING_VOLUME_RENDERER_H
#define AVT_OPEN_GL_SPLATTING_VOLUME_RENDERER_H

#include <avtVolumeRenderer.h>


// ****************************************************************************
//  Class: avtOpenGLSplattingVolumeRenderer
//
//  Purpose:
//      An implementation of a volume renderer that uses OpenGL calls.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 30 11:52:27 PDT 2003
//    Renamed to include the word "splatting".  Moved alphatex from the base
//    class to here.  Added OpenGL texture object ID.  Added method to
//    release texture object when we are done with it.
//
// ****************************************************************************

class avtOpenGLSplattingVolumeRenderer : public avtVolumeRenderer
{
  public:
                            avtOpenGLSplattingVolumeRenderer();
    virtual                ~avtOpenGLSplattingVolumeRenderer();
    virtual void            ReleaseGraphicsResources();

  protected:
    virtual void            Render(vtkDataSet *);
    float                  *alphatex;
    unsigned int            alphatexId;
};


#endif


