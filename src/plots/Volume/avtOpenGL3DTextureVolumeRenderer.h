// ************************************************************************* //
//                     avtOpenGL3DTextureVolumeRenderer.h                    //
// ************************************************************************* //

#ifndef AVT_OPEN_GL_3D_TEXTURE_VOLUME_RENDERER_H
#define AVT_OPEN_GL_3D_TEXTURE_VOLUME_RENDERER_H

#include <avtVolumeRendererImplementation.h>

#include <VolumeAttributes.h>
#include <LightList.h>

// ****************************************************************************
//  Class: avtOpenGL3DTextureVolumeRenderer
//
//  Purpose:
//      An implementation of a volume renderer utilizing 3D texturing hardware
//      through OpenGL calls.
//
//  Programmer: Jeremy Meredith
//  Creation:   October  1, 2003
//
//  Modifications:
//
// ****************************************************************************

class avtOpenGL3DTextureVolumeRenderer : public avtVolumeRendererImplementation
{
  public:
                            avtOpenGL3DTextureVolumeRenderer();
    virtual                ~avtOpenGL3DTextureVolumeRenderer();

  protected:
    virtual void            Render(vtkRectilinearGrid *grid,
                                   vtkDataArray *data,
                                   vtkDataArray *opac,
                                   const avtViewInfo &view, 
                                   const VolumeAttributes&,
                                   float vmin, float vmax, float vsize,
                                   float omin, float omax, float osize,
                                   float *gx, float *gy, float *gz, float *gmn);

    unsigned char          *volumetex;
    unsigned int            volumetexId;
    VolumeAttributes        oldAtts;
    LightList               oldLights;
};


#endif


