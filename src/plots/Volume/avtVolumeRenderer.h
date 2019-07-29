// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtVolumeRenderer.h                           //
// ************************************************************************* //

#ifndef AVT_VOLUME_RENDERER_H
#define AVT_VOLUME_RENDERER_H

#include <avtCustomRenderer.h>
#include <VolumeAttributes.h>

class vtkDataArray;
class avtVolumeRendererImplementation;

#define USE_HISTOGRAM 1

// ****************************************************************************
//  Class: avtVolumeRenderer
//
//  Purpose:
//      An implementation of an avtCustomRenderer for a volume plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov 19 08:07:40 PST 2001
//    Added a convenience routine to retrieve the variables from a dataset.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Wed Apr 24 09:16:51 PDT 2002 
//    Added derived types for graphics packages.
//
//    Jeremy Meredith, Tue Sep 30 11:49:42 PDT 2003
//    Added method "ReleaseGraphicsResources".  Moved alphatex to subclass.
//
//    Jeremy Meredith, Thu Oct  2 13:13:23 PDT 2003
//    Made this class be a concrete implementation of a custom renderer.
//    It will chose between actual rendering methods by instantiating an
//    avtVolumeRendererImplementation at render time.
//
//    Brad Whitlock, Mon Dec 15 14:36:29 PST 2008
//    I removed some methods.
//
//    Jeremy Meredith, Tue Jan  5 15:52:32 EST 2010
//    Added value for actual gradient magnitude maximum.
//
//    Allen Harvey, Thurs Nov 3 7:21:13 EST 2011
//    Added value for holding a compact support variable
//
//    Aliste Maguire, Thu Sep 14 13:36:16 PDT 2017
//    Added dataIs2D for early return from volume rendering.
//
// ****************************************************************************

class avtVolumeRenderer : public avtCustomRenderer
{
  public:
                            avtVolumeRenderer();
    virtual                ~avtVolumeRenderer();
    static avtVolumeRenderer *New(void);

    void                    SetAtts(const AttributeGroup*);

    virtual bool            OperatesOnScalars(void) { return true; };
    virtual void            ReleaseGraphicsResources();
    virtual void            Render(vtkDataSet *);

  protected:
    avtVolumeRendererImplementation  *rendererImplementation;
    bool                              currentRendererIsValid;

    VolumeAttributes        atts;

    void                    Initialize(vtkDataSet*);
    bool                    initialized;

    float                   vmin,vmax,vsize;
    float                   omin,omax,osize;
    float                  *hs, hs_min, hs_max, hs_size;
    
    float                  *gx, *gy, *gz, *gm, *gmn, gm_max;

    bool                    dataIs2D; // needed to tell renderers to return early

    bool                    GetScalars(vtkDataSet *ds, vtkDataArray *&d, vtkDataArray *&o);
};


typedef ref_ptr<avtVolumeRenderer> avtVolumeRenderer_p;


#endif


