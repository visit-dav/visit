// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtVolumeFilter.h                             //
// ************************************************************************* //

#ifndef AVT_VOLUME_FILTER_H
#define AVT_VOLUME_FILTER_H

#include <avtDatasetToDatasetFilter.h>

#include <VolumeAttributes.h>

#include <avtImage.h>
#include <avtOpacityMap.h>

class WindowAttributes;
class avtVisItVTKRenderFilter;


// ****************************************************************************
//  Class: avtVolumeFilter
//
//  Purpose:
//      Decides if a volume renderer should output a software rendered image
//      or if it should output a resampled volume plot.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 24 16:21:39 PST 2004
//    Changed inheritance hierarchy.  This filter now simply does software
//    volume rendering and is used by the volume plot.  It is the interface
//    from the volume plot to the ray tracer.  Also removed many support
//    methods that are no longer necessary since this filter doesn't switch
//    between multiple modes.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
// ****************************************************************************

class avtVolumeFilter : public avtDatasetToDatasetFilter
{
  public:
                             avtVolumeFilter();
    virtual                 ~avtVolumeFilter();

    void                     SetAttributes(const VolumeAttributes &);
    virtual const char      *GetType(void) { return "avtVolumeFilter"; };
    virtual const char      *GetDescription(void)
                                  { return "Volume rendering"; };

    avtImage_p               RenderImage(avtImage_p, const WindowAttributes &);

    int                      GetNumberOfStages(const WindowAttributes &);

  protected:
    VolumeAttributes         atts;
    char                    *primaryVariable {nullptr};

    avtVisItVTKRenderFilter *VisItVTKRenderFilter {nullptr};

    avtOpacityMap            CreateOpacityMap(double range[2]);

    int                      GetRenderVariables( int  &primIndex,
                                                 int  &opacIndex,
                                                 int  &gradIndex,
                                                 char *gradName );
    bool                     GetLogicalBounds(avtDataObject_p input,
                                              int &width,
                                              int &height,
                                              int &depth);

    avtImage_p               RenderImageVTK(avtImage_p,
                                            const WindowAttributes &);
#ifdef VISIT_SLIVR
    avtImage_p               RenderImageSLIVR(avtImage_p,
                                              const WindowAttributes &);
#endif

    virtual void             Execute(void);
    virtual avtContract_p    ModifyContract(avtContract_p);
    virtual void             VerifyInput(void);
    virtual bool             FilterUnderstandsTransformedRectMesh();
};

#endif
