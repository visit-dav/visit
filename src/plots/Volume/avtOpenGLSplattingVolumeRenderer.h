// ************************************************************************* //
//                     avtOpenGLSplattingVolumeRenderer.h                    //
// ************************************************************************* //

#ifndef AVT_OPEN_GL_SPLATTING_VOLUME_RENDERER_H
#define AVT_OPEN_GL_SPLATTING_VOLUME_RENDERER_H

#include <avtVolumeRendererImplementation.h>

// ****************************************************************************
//  Class: avtOpenGLSplattingVolumeRenderer
//
//  Purpose:
//      An implementation of a gaussian splat volume renderer that
//      uses OpenGL calls.
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
//    Jeremy Meredith, Thu Oct  2 13:36:28 PDT 2003
//    Made this class not inherit from avtVolumeRenderer.  It now 
//    gets most of its state from the arguments to Render.
//
// ****************************************************************************

class avtOpenGLSplattingVolumeRenderer : public avtVolumeRendererImplementation
{
  public:
                            avtOpenGLSplattingVolumeRenderer();
    virtual                ~avtOpenGLSplattingVolumeRenderer();

  protected:
    virtual void            Render(vtkRectilinearGrid *grid,
                                   vtkDataArray *data,
                                   vtkDataArray *opac,
                                   const avtViewInfo &view,
                                   const VolumeAttributes&,
                                   float vmin, float vmax, float vsize,
                                   float omin, float omax, float osize,
                                   float *gx, float *gy, float *gz,
                                   float *gmn);

    float                  *alphatex;
    unsigned int            alphatexId;
};


#endif


