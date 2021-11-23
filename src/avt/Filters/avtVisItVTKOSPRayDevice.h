// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtVisItVTKOSPRayDevice.h                          //
// ************************************************************************* //

#ifndef AVT_VISIT_VTK_OSPRAY_DEVICE_H
#define AVT_VISIT_VTK_OSPRAY_DEVICE_H

#include <avtVisItVTKDevice.h>
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
        namespace ospray
        {
            struct RenderingAttribs
            {
                RenderingAttribs(): OSPRayEnabled(false),

                                    resampleType{0},
                                    resampleTargetVal{static_cast<int>(1e6)},

                                    lightingEnabled(false),
                                    shadowsEnabled(false),
                                    useGridAccelerator(false),
                                    preIntegration(false),
                                    singleShade(false),
                                    oneSidedLighting(false),
                                    aoTransparencyEnabled(false),
                                    aoSamples(0),
                                    samplesPerPixel(1),
                                    aoDistance(100000.0f),
                                    minContribution(2.00f),
                                    maxContribution(0.01f),
                                    samplesPerRay(500),
                                    samplingRate(3.0f) {}

                bool    OSPRayEnabled;

                int     resampleType;
                int     resampleTargetVal;

                bool    lightingEnabled;
                // Whether to compute (hard) shadows
                bool    shadowsEnabled;
                bool    useGridAccelerator;
                bool    preIntegration;
                bool    singleShade;
                bool    oneSidedLighting;
                bool    aoTransparencyEnabled;
                // Number of rays per sample to compute ambient occlusion
                int     aoSamples;
                // Samples per pixel
                int     samplesPerPixel;
                // Maximum distance to consider for ambient occlusion
                float   aoDistance;
                // Sample contributions below/above this value will be neglected
                // to speed up rendering.
                float   minContribution;
                float   maxContribution;

                int     samplesPerRay;
                // Sampling rate for volumes
                float   samplingRate;
            };
        }   // namespace ospray
    }   // namespace visit_vtk
}   // namespace avt

using DataType = avt::visit_vtk::DataType;
using RendererType = avt::visit_vtk::RendererType;
using OSPRayRenderingAttribs = avt::visit_vtk::ospray::RenderingAttribs;

class AVTFILTERS_API avtVisItVTKOSPRayDevice : public avtVisItVTKDevice, public avtRayTracerBase
{
public:
                            avtVisItVTKOSPRayDevice();
    virtual                 ~avtVisItVTKOSPRayDevice();

    virtual const char      *GetType() override         { return "avtVisItVTKOSPRayDevice"; }
    virtual const char      *GetDescription() override  { return "VisItVTK OSPRay Back-end Device"; }
    virtual const char      *GetDeviceType() override   { return DEVICE_TYPE_STR.c_str(); }

    void SetRenderingType(const DataType dt)    { m_dataType = dt; }

     // VisIt options
    void SetLightInfo(const LightList& l)       { m_lightList = l; }

    // OSPRay Options
    void SetOSPRayEnabled(const bool b)         { m_renderingAttribs.OSPRayEnabled = b; }
    void SetResampleType(const int v)           { m_renderingAttribs.resampleType = v; }
    void SetResampleTargetVal(const int v)      { m_renderingAttribs.resampleTargetVal = v; }

    void SetLighting(const bool b)              { m_renderingAttribs.lightingEnabled = b; }
    void SetShadowsEnabled(const bool b)        { m_renderingAttribs.shadowsEnabled = b; }
    void SetUseGridAccelerator(const bool b)    { m_renderingAttribs.useGridAccelerator = b; }
    void SetPreIntegration(const bool b)        { m_renderingAttribs.preIntegration = b; }
    void SetSingleShade(const bool b)           { m_renderingAttribs.singleShade = b; }
    void SetOneSidedLighting(const bool b)      { m_renderingAttribs.oneSidedLighting = b; }
    void SetAoTransparencyEnabled(const bool b) { m_renderingAttribs.aoTransparencyEnabled = b; }
    void SetAoSamples(const int v)              { m_renderingAttribs.aoSamples = v; }
    void SetSamplesPerPixel(const int v)        { m_renderingAttribs.samplesPerPixel = v; }
    void SetAoDistance(const float v)           { m_renderingAttribs.aoDistance = v; }
    void SetMinContribution(const float v)      { m_renderingAttribs.minContribution = v; }
    void SetMaxContribution(const float v)      { m_renderingAttribs.maxContribution = v; }

    // Ray Casting Options
    void SetSamplingRate(const float v)         { m_renderingAttribs.samplingRate = v; }
    // Maximum reay recursion depth
    void SetSamplesPerRay(const int v)          { m_renderingAttribs.samplesPerRay = v; }

    // Lighting and Material Properties
    void SetMatProperties(const double[4]);
    void SetViewDirection(const double[3]);

protected:
    virtual void                Execute(void) override;

    DataType                    m_dataType;

    // VisIt has 8 lights that can be setup
    LightList                   m_lightList;

    OSPRayRenderingAttribs      m_renderingAttribs;

    std::unique_ptr<float[]>    m_materialPropertiesPtr;
    std::unique_ptr<float[]>    m_viewDirectionPtr;

    bool                        m_useInterpolation{true};
    bool                        m_resetColorMap{true};

    bool                        m_lastMapper{false};

    int                         m_nComponents{1};

    int                         m_resampleType{0};
    int                         m_resampleTargetVal{0};

 private:
    void                 ExecuteVolume();
    void                 ExecuteSurface();
    avtImage_p           CreateFinalImage(const void *, const int, const int, const float);

    vtkCamera *          CreateCamera();
    vtkLightCollection * CreateLights();

    int    numLightsEnabled{0};
    bool   ambientOn{false};
    double ambientCoefficient{0.0};
    double ambientColor[3] = { 0., 0., 0.};

    avtResampleFilter * resampleFilter{nullptr};

    vtkImageData* imageToRender{nullptr};
    vtkVolumeMapper* volumeMapper{nullptr};

    static const std::string   DEVICE_TYPE_STR;
};

#endif
