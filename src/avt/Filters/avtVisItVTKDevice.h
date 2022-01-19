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

#include <vtkCamera.h>
#include <vtkLightCollection.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <memory>

#include <filters_exports.h>
#include <LightList.h>

class avtResampleFilter;
class vtkImageData;
class vtkVolumeMapper;

namespace avt
{
    namespace visit_vtk
    {
        struct RenderingAttribs
        {
            RenderingAttribs(): resampleType{0},
                                resampleTargetVal{static_cast<int>(1e6)},
                                resampleFlag(true),

                                lightingEnabled(false),

                                samplesPerRay(500),
                                samplingRate(3.0f),

                                useColorVarMin(false),
                                colorVarMin(0.0),
                                useColorVarMax(false),
                                colorVarMax(1.0),
                                useOpacityVarMin(false),
                                opacityVarMin(0.0),
                                useOpacityVarMax(false),
                                opacityVarMax(1.0),

                                // OSPRay attributes.
                                OSPRayEnabled(false),
                                OSPRayRenderType(0),
                                OSPRayShadowsEnabled(false),
                                OSPRayUseGridAccelerator(false),
                                OSPRayPreIntegration(false),
                                OSPRaySingleShade(false),
                                OSPRayOneSidedLighting(false),
                                OSPRayAOTransparencyEnabled(false),
                                OSPRayAOSamples(0),
                                OSPRaySamplesPerPixel(1),
                                OSPRayAODistance(100000.0f),
                                OSPRayMinContribution(2.00f),
                                OSPRayMaxContribution(0.01f) {}

            int     resampleType;
            int     resampleTargetVal;
            bool    resampleFlag;

            bool    lightingEnabled;

            int     samplesPerRay;
            // Sampling rate for volumes
            float   samplingRate;

            bool    useColorVarMin;
            float   colorVarMin;
            bool    useColorVarMax;
            float   colorVarMax;
            bool    useOpacityVarMin;
            float   opacityVarMin;
            bool    useOpacityVarMax;
            float   opacityVarMax;

            // OSPRay attributes.
            bool    OSPRayEnabled;
            int     OSPRayRenderType;
            // Whether to compute (hard) shadows
            bool    OSPRayShadowsEnabled;
            bool    OSPRayUseGridAccelerator;
            bool    OSPRayPreIntegration;
            bool    OSPRaySingleShade;
            bool    OSPRayOneSidedLighting;
            bool    OSPRayAOTransparencyEnabled;
            // Number of rays per sample to compute ambient occlusion
            int     OSPRayAOSamples;
            // Samples per pixel
            int     OSPRaySamplesPerPixel;
            // Maximum distance to consider for ambient occlusion
            float   OSPRayAODistance;
            // Sample contributions below/above this value will be neglected
            // to speed up rendering.
            float   OSPRayMinContribution;
            float   OSPRayMaxContribution;

            };
    }   // namespace visit_vtk
}   // namespace avt

using DataType = avt::visit_vtk::DataType;
using RendererType = avt::visit_vtk::RendererType;
using RenderingAttribs = avt::visit_vtk::RenderingAttribs;

class AVTFILTERS_API avtVisItVTKDevice : public avtVisItVTKDeviceBase, public avtRayTracerBase
{
public:
                            avtVisItVTKDevice();
    virtual                 ~avtVisItVTKDevice();

    virtual const char      *GetType() override        { return "avtVisItVTKDevice"; }
    virtual const char      *GetDescription() override { return "VisItVTK Back-end Device"; }
    virtual const char      *GetDeviceType() override  { return DEVICE_TYPE_STR.c_str(); }

    // avtRayTracerBase
    virtual void          SetBackgroundMode(int mode) override {};
    virtual void          SetGradientBackgroundColors(const double [3],
                                                      const double [3]) override {};
     // VisIt options
    void SetRenderingType(const DataType dt)    { m_dataType = dt; }

    void SetLightInfo(const LightList& l)       { m_lightList = l; }

    void SetResampleType(const int v)           { m_renderingAttribs.resampleType = v; }
    void SetResampleFlag(const bool v)          { m_renderingAttribs.resampleFlag = v; }
    void SetResampleTargetVal(const int v)      { m_renderingAttribs.resampleTargetVal = v; }

    void SetLighting(const bool b)              { m_renderingAttribs.lightingEnabled = b; }

    void SetUseColorVarMin(const bool v)        { m_renderingAttribs.useColorVarMin = v; }
    void SetColorVarMin(const float v)          { m_renderingAttribs.colorVarMin = v; }
    void SetUseColorVarMax(const bool v)        { m_renderingAttribs.useColorVarMax = v; }
    void SetColorVarMax(const float v)          { m_renderingAttribs.colorVarMax = v; }
    void SetUseOpacityVarMin(const bool v)      { m_renderingAttribs.useOpacityVarMin = v; }
    void SetOpacityVarMin(const float v)        { m_renderingAttribs.opacityVarMin = v; }
    void SetUseOpacityVarMax(const bool v)      { m_renderingAttribs.useOpacityVarMax = v; }
    void SetOpacityVarMax(const float v)        { m_renderingAttribs.opacityVarMax = v; }

    // Ray Casting Options
    void SetSamplingRate(const float v)         { m_renderingAttribs.samplingRate = v; }
    // Maximum ray recursion depth
    void SetSamplesPerRay(const int v)          { m_renderingAttribs.samplesPerRay = v; }

    // Lighting and Material Properties
    void SetMatProperties(const double[4]);
    void SetViewDirection(const double[3]);

    // OSPRay Options
    void SetOSPRayEnabled(const bool b)               { m_renderingAttribs.OSPRayEnabled = b; }
    void SetOSPRayRenderType(const int v)             { m_renderingAttribs.OSPRayRenderType = v; }
    void SetOSPRayShadowsEnabled(const bool b)        { m_renderingAttribs.OSPRayShadowsEnabled = b; }
    void SetOSPRayUseGridAccelerator(const bool b)    { m_renderingAttribs.OSPRayUseGridAccelerator = b; }
    void SetOSPRayPreIntegration(const bool b)        { m_renderingAttribs.OSPRayPreIntegration = b; }
    void SetOSPRaySingleShade(const bool b)           { m_renderingAttribs.OSPRaySingleShade = b; }
    void SetOSPRayOneSidedLighting(const bool b)      { m_renderingAttribs.OSPRayOneSidedLighting = b; }
    void SetOSPRayAOTransparencyEnabled(const bool b) { m_renderingAttribs.OSPRayAOTransparencyEnabled = b; }
    void SetOSPRayAOSamples(const int v)              { m_renderingAttribs.OSPRayAOSamples = v; }
    void SetOSPRaySamplesPerPixel(const int v)        { m_renderingAttribs.OSPRaySamplesPerPixel = v; }
    void SetOSPRayAODistance(const float v)           { m_renderingAttribs.OSPRayAODistance = v; }
    void SetOSPRayMinContribution(const float v)      { m_renderingAttribs.OSPRayMinContribution = v; }
    void SetOSPRayMaxContribution(const float v)      { m_renderingAttribs.OSPRayMaxContribution = v; }

protected:
    virtual void                Execute(void) override;

    DataType                    m_dataType{DataType::GEOMETRY};

    // VisIt has 8 lights that can be setup
    LightList                   m_lightList;

    RenderingAttribs            m_renderingAttribs;

    std::unique_ptr<float[]>    m_materialPropertiesPtr{nullptr};
    std::unique_ptr<float[]>    m_viewDirectionPtr{nullptr};

    bool                        m_useInterpolation{true};
    bool                        m_resetColorMap{true};

    bool                        m_OSPRayEnabled{false};
    int                         m_OSPRayRenderType{0};

    int                         m_nComponents{1};

    int                         m_resampleType{0};
    int                         m_resampleTargetVal{0};

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

private:
    void                 ExecuteVolume();
    void                 ExecuteSurface();
    avtImage_p           CreateFinalImage(const void *, const int, const int, const float);

    vtkCamera *          CreateCamera();
    vtkLightCollection * CreateLights();

    int    m_numLightsEnabled{0};
    bool   m_ambientOn{false};
    double m_ambientCoefficient{0.0};
    double m_ambientColor[3] = { 0., 0., 0.};

    int    m_cellData{0};

    avtResampleFilter * m_resampleFilter{nullptr};

    vtkImageData* m_imageToRender{nullptr};
    vtkVolumeMapper* m_volumeMapper{nullptr};

    static const std::string DEVICE_TYPE_STR;
};

#endif
