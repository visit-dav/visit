#ifndef AVT_VOLUME_RENDERER_IMPLEMENTATION_H
#define AVT_VOLUME_RENDERER_IMPLEMENTATION_H

class vtkRectilinearGrid;
class vtkDataArray;
class VolumeAttributes;
class avtViewInfo;

// ****************************************************************************
//  Class:  avtVolumeRendererImplementation
//
//  Purpose:
//    Implements the rendering-only portion of a volume renderer in a
//    relatively stateless manner.  Meant to be instantiated at render
//    time by avtVolumeRenderer::Render, though it can be kept around
//    across renderers while the implementation itself has not changed.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  2, 2003
//
// ****************************************************************************
class avtVolumeRendererImplementation
{
  public:
                   avtVolumeRendererImplementation() {}
    virtual       ~avtVolumeRendererImplementation() {}
    virtual void   Render(vtkRectilinearGrid *grid,
                          vtkDataArray *data,
                          vtkDataArray *opac,
                          const avtViewInfo &view,
                          const VolumeAttributes&,
                          float vmin, float vmax, float vsize,
                          float omin, float omax, float osize,
                          float *gx, float *gy, float *gz, float *gmn) = 0;
};

#endif
