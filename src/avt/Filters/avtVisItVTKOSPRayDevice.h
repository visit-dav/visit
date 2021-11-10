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

namespace avt
{
    namespace visit_vtk
    {
        namespace ospray
        {
            struct RenderingAttribs
            {
                RenderingAttribs(): lightingEnabled(false),
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
    virtual                 ~avtVisItVTKOSPRayDevice() = default;

    virtual const char      *GetType() override         { return "avtVisItVTKOSPRayDevice"; }
    virtual const char      *GetDescription() override  { return "VisItVTK OSPRay Back-end Device"; }
    virtual const char      *GetDeviceType() override   { return DEVICE_TYPE_STR.c_str(); }

    void SetRenderingType(const DataType dt)       { m_dataType = dt; }

     // VisIt options
    void SetActiveVariable(const char *s)       { m_activeVariablePtr = s; }
    void SetLightInfo(const LightList& l)       { m_lightList = l; }

    // OSPRay Options
    void SetLighting(const bool b)              { m_renderingAttribs.lightingEnabled = b; }
    void SetShadowsEnabled(const bool b)        { m_renderingAttribs.shadowsEnabled = b; }
    void SetUseGridAccelerator(const bool l)    { m_renderingAttribs.useGridAccelerator = l; }
    void SetPreIntegration(const bool l)        { m_renderingAttribs.preIntegration = l; }
    void SetSingleShade(const bool l)           { m_renderingAttribs.singleShade = l; }
    void SetOneSidedLighting(const bool l)      { m_renderingAttribs.oneSidedLighting = l; }
    void SetAoTransparencyEnabled(const bool l) { m_renderingAttribs.aoTransparencyEnabled = l; }
    void SetAoSamples(const int v)              { m_renderingAttribs.aoSamples = v; }
    void SetSamplesPerPixel(const int v)        { m_renderingAttribs.samplesPerPixel = v; }
    void SetAoDistance(const float v)           { m_renderingAttribs.aoDistance = v; }
    void SetMinContribution(const float v)      { m_renderingAttribs.minContribution = v; }
    void SetMaxContribution(const float v)      { m_renderingAttribs.maxContribution = v; }

    // Ray Casting Options
    void SetSamplingRate(const float v)         { m_renderingAttribs.samplingRate = v; }
    // Maximum reay recursion depth
    void SetSamplesPerRay(const int s)          { m_renderingAttribs.samplesPerRay = s; }

    // Lighting and Material Properties
    void SetMatProperties(const double[4]);
    void SetViewDirection(const double[3]);

    // VisItVTK Callback Functions
    static void                 ErrorCallback(void *, vtkErrorCode::ErrorIds, const char *);
    static void                 StatusCallback(void *, const char *);

protected:
    virtual void                Execute(void) override;

    DataType                    m_dataType;
    const char*                 m_activeVariablePtr;
    // VisIt has 8 lights that can be setup
    LightList                   m_lightList;

    OSPRayRenderingAttribs      m_renderingAttribs;

    std::unique_ptr<float[]>    m_materialPropertiesPtr;
    std::unique_ptr<float[]>    m_viewDirectionPtr;

    // avtOSPRayImageCompositor imgComm;

private:
    void                        ExecuteVolume();
    void                        ExecuteSurface();
    avtImage_p                  CreateFinalImage(const void *, const int, const int, const float);

    vtkCamera *                 CreateCamera();
    vtkLightCollection *        CreateLights();

    int numLightsEnabled{0};
    bool ambientOn{false};
    double ambientCoefficient{0.0};
    double ambientColor[3] = { 0., 0., 0.};

    /*
    VisItVTKVolume             CreateStructuredRegularVolume(vtkDataSet * const);
    VisItVTKVolumetricModel    CreateVolumetricModel(vtkDataSet * const);
    VisItVTKInstance           CreateInstance(vtkDataSet * const);
    VisItVTKWorld              CreateWorld();
    VisItVTKRenderer           CreateRenderer(const RendererType);
    VisItVTKFrame              CreateFrameBuffer(const int,
                                              int  width,
                                              int  height,
                                              bool channelColor = true,
                                              bool channelAccum = false,
                                              bool channelDepth = false,
                                              bool channelAlbedo = false);

    */
    static const std::string   DEVICE_TYPE_STR;
    /*
    VisItVTKModule             m_module;
    VisItVTKDevice             m_device;
    */
};

#endif
