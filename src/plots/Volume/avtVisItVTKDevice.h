// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtVisItVTKDevice.h                             //
// ************************************************************************* //

#ifndef AVT_VISIT_VTK_DEVICE_H
#define AVT_VISIT_VTK_DEVICE_H

#include <avtVisItVTKDeviceBase.h>
#include <avtRayTracerBase.h>

#include <VolumeAttributes.h>

#include <vtkCamera.h>
#include <vtkLightCollection.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <memory>

#include <LightList.h>

class vtkImageData;
class vtkVolumeMapper;

#define MAX_LIGHTS 8

using DataType = avt::visit_vtk::DataType;
using RendererType = avt::visit_vtk::RendererType;

class avtVisItVTKDevice : public avtVisItVTKDeviceBase, public avtRayTracerBase
{
public:
                            avtVisItVTKDevice();
    virtual                 ~avtVisItVTKDevice();

    void                    SetAtts(const AttributeGroup*);

    virtual const char      *GetType() override        { return "avtVisItVTKDevice"; }
    virtual const char      *GetDescription() override { return "VisItVTK Back-end Device"; }
    virtual const char      *GetDeviceType() override  { return DEVICE_TYPE_STR.c_str(); }

    // avtRayTracerBase
    virtual void          SetBackgroundMode(int mode) override {};
    virtual void          SetGradientBackgroundColors(const double [3],
                                                      const double [3]) override {};

    void SetRenderingType(const DataType dt)    { m_dataType = dt; }
    void SetLightList(const LightList& l)       { m_lightList = l; }

protected:
    virtual void                Execute(void) override;

    VolumeAttributes            m_atts;

    DataType                    m_dataType{DataType::GEOMETRY};

    // VisIt has 8 lights that can be setup
    LightList                   m_lightList;

private:
    void                 ExecuteVolume();
    void                 ExecuteSurface();
    avtImage_p           CreateFinalImage(const void *, const int, const int, const float);

    vtkCamera *          CreateCamera();
    vtkLightCollection * CreateLights();

    int                         m_numLightsEnabled{0};
    bool                        m_ambientOn{false};
    double                      m_ambientCoefficient{0.0};
    double                      m_ambientColor[3] = { 0., 0., 0.};

    int                         m_nComponents{1};
    int                         m_cellData{0};

    bool                        m_useInterpolation{true};
    bool                        m_resetColorMap{true};

    // For state changes.
    bool                        m_useColorVarMin{false};
    float                       m_colorVarMin{0.0};
    bool                        m_useColorVarMax{false};
    float                       m_colorVarMax{1.0};
    bool                        m_useOpacityVarMin{false};
    float                       m_opacityVarMin{0.0};
    bool                        m_useOpacityVarMax{false};
    float                       m_opacityVarMax{1.0};

    std::string                 m_activeVarName  {"default"};
    std::string                 m_opacityVarName {"default"};
    std::string                 m_gradientVarName{"default"};

    bool                        m_OSPRayEnabled{false};
    int                         m_OSPRayRenderType{0};

    vtkImageData*               m_imageToRender{nullptr};
    vtkVolumeMapper*            m_volumeMapper{nullptr};

    static const std::string DEVICE_TYPE_STR;
};

#endif
