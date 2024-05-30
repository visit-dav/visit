// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtVisItVTKRenderFilter.h                       //
// ************************************************************************* //

#ifndef AVT_VISIT_VTK_RENDER_FILTER_H
#define AVT_VISIT_VTK_RENDER_FILTER_H

#include <avtRayTracerBase.h>
#include <avtVisItVTKRenderer.h>

#include <vtkCamera.h>
#include <vtkLightCollection.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <memory>

#include <LightList.h>

class vtkColorTransferFunction;
class vtkImageData;
class vtkPiecewiseFunction;
class vtkVolume;
class vtkVolumeMapper;
class vtkVolumeProperty;

#define MAX_LIGHTS 8

class avtVisItVTKRenderFilter : public avtRayTracerBase, public avtVisItVTKRenderer
{
public:
                          avtVisItVTKRenderFilter();
    virtual              ~avtVisItVTKRenderFilter();

    virtual void          Execute(void) override;

    // avtRayTracerBase
    virtual void          SetBackgroundMode(int mode) override {};
    virtual void          SetGradientBackgroundColors(const double [3],
                                                      const double [3]) override {};

    void                  SetLightList(const LightList& l) { m_lightList = l; }

protected:
    avtImage_p            CreateFinalImage(const void *,
                                           const int, const int,
                                           const float);

    vtkCamera *           CreateCamera();
    vtkLightCollection *  CreateLights();

    VolumeAttributes           m_atts;

    // VisIt has 8 lights that can be setup
    LightList                  m_lightList;

    int                        m_numLightsEnabled{0};
    bool                       m_ambientOn{false};
    double                     m_ambientCoefficient{0.0};
    double                     m_ambientColor[3] = { 0., 0., 0.};
};

#endif
