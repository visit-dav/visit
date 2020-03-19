// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_VOLUME_RENDERER_IMPLEMENTATION_H
#define AVT_VOLUME_RENDERER_IMPLEMENTATION_H
#include <avtViewInfo.h>
#include <VolumeAttributes.h>
#include <vtkPointData.h>
#include <vtkRenderer.h>

class vtkDataSet;
class vtkDataArray;

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
//  Modifications:
//    Brad Whitlock, Thu Jan 10 14:46:11 PST 2008
//    Added reducedDetail flag.
//
//    Brad Whitlock, Tue Apr 21 11:32:05 PDT 2009
//    I gathered properties into structs that we can pass to Render. I
//    also added the window size to the struct.
//
//    Jeremy Meredith, Tue Jan  5 15:53:30 EST 2010
//    Added storage of unnormalize gradient magnitude and its maximum value.
//
//    Allen Harvey, Thurs Nov 3 7:21:13 EST 2011
//    Added data items to support volume rendering without resampling
//
//    Alister Maguire, Wed Mar  8 10:40:09 PST 2017
//    I added a vtkRenderer class member so that the volume renderers
//    (avtDefaultRenderer, in particular) have access to 
//    VisIt's render window. I added a getter and setter along 
//    with this member. 
//
//    Alister Maguire, Thu Sep 14 13:36:16 PDT 2017
//    Added dataIs2D. 
//
// ****************************************************************************

class avtVolumeRendererImplementation
{
  public:
    struct RenderProperties
    {
        RenderProperties() : view(), atts()
        {
            backgroundColor[0] = backgroundColor[1] = backgroundColor[2] = 0.;
            windowSize[0] = windowSize[1] = 0;
        }

        avtViewInfo      view;
        VolumeAttributes atts;
        float            backgroundColor[3];
        int              windowSize[2];
        bool             dataIs2D; // mostly needed so renderers can do early return
    };

    struct VariableData
    {
        VariableData()
        {
            data = NULL;
            min = max = size = 0.f;
        }

        vtkDataArray *data;
        float         min;
        float         max;
        float         size;
    };

    struct VolumeData
    {
        VolumeData() : data(), opacity()
        {
            grid = NULL;
            gx = gy = gz = gm = gmn = hs = NULL;
            gm_max = 0;
        }

        VariableData        data;
        VariableData        opacity;
        vtkDataSet         *grid;
        float              *gx;
        float              *gy;
        float              *gz;
        float              *gm;
        float              *gmn;
        float               gm_max;
        float              *hs;
        float               hs_min;
    };

                         avtVolumeRendererImplementation() { }
    virtual             ~avtVolumeRendererImplementation() { }
    virtual void         Render(const RenderProperties &props, const VolumeData &volume) = 0;
    virtual vtkRenderer *GetVTKRenderer() { return VTKRen; }
    virtual void         SetVTKRenderer(vtkRenderer *ren) { VTKRen = ren; }

  protected:

    vtkRenderer        *VTKRen;

};

#endif
