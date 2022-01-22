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

#include <string>

class vtkColorTransferFunction;
class vtkImageData;
class vtkPiecewiseFunction;
class vtkVolume;
class vtkVolumeMapper;
class vtkVolumeProperty;

// ****************************************************************************
//  Class: avtVolumeRenderer
//
//  Purpose:
//      An implementation of an renderer for a volume plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
//  Modifications:
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
    virtual void            Render(vtkDataSet *);

  protected:
    VolumeAttributes           atts;
    VolumeAttributes           oldAtts;

    vtkColorTransferFunction  *transFunc    {nullptr};
    vtkPiecewiseFunction      *opacity      {nullptr};
    vtkImageData              *imageToRender{nullptr};
    vtkVolumeProperty         *volumeProp   {nullptr};
    vtkVolumeMapper           *volumeMapper {nullptr};
    vtkVolume                 *curVolume    {nullptr};

    bool                       resetColorMap{true};
    bool                       useInterpolation{true};

    bool                       OSPRayEnabled{false};

    int                        nComponents{1};
    bool                       cellData{false};
};

typedef ref_ptr<avtVolumeRenderer> avtVolumeRenderer_p;

#endif
