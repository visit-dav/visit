// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtVisItVTKDevice.C                                 //
// ************************************************************************* //

#include <avtVisItVTKDevice.h>
#include <avtParallel.h>
#include <avtICETCompositor.h>

#include <DebugStream.h>
#include <TimingsManager.h>
#include <StackTimer.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtLightList.h>
#include <avtResampleFilter.h>
#include <avtSourceFromAVTDataset.h>
#include <avtWorldSpaceToImageSpaceTransform.h>

#include <vtkImageData.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkDataObject.h>
#include <vtkRectilinearGrid.h>
#include <vtkLight.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkSmartVolumeMapper.h>

#ifdef HAVE_OSPRAY
    #include <vtkOSPRayVolumeMapper.h>
    #include <vtkOSPRayRendererNode.h>
#endif

#include <vtkDataSetSurfaceFilter.h>
#include <vtkGeometryFilter.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>

#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkXMLImageDataWriter.h>

#include <vector>

const std::string avtVisItVTKDevice::DEVICE_TYPE_STR{"vtk"};

//#define LOCAL_DEBUG std::cerr
#define LOCAL_DEBUG debug5


// ****************************************************************************
//  Method: avtVisItVTKDevice constructor
//
//  Programmer: Allen R. Sanderson
//  Creation:  30 November 2021
//
//  Modifications:
//
// ****************************************************************************

avtVisItVTKDevice::avtVisItVTKDevice() : avtRayTracerBase(),
    m_lightList(),
    m_renderingAttribs()
{
    LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                << "Creating a new device." << std::endl;
}

// ****************************************************************************
//  Method: avtVisItVTKDevice destructor
//
//  Programmer: Allen R. Sanderson
//  Creation:  30 November 2021
//
//  Modifications:
//
// ****************************************************************************

avtVisItVTKDevice::~avtVisItVTKDevice()
{
    if(m_imageToRender != nullptr)
        m_imageToRender->Delete();

    if(m_volumeMapper != nullptr)
        m_volumeMapper->Delete();
}


// ****************************************************************************
//  Method: avtVisItVTKDevice::SetMatProperties
//
//  Purpose:
//      Set the volume material properties.
//
//  Arguments:
//      props   material properties where:
//          props[0] = ambient
//          props[1] = diffuse
//          props[2] = specular
//          props[3] = shininess
//
//  Programmer: Allen R. Sanderson
//  Creation:   30 November 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtVisItVTKDevice::SetMatProperties(const double props[4])
{
    if(!m_materialPropertiesPtr)
        m_materialPropertiesPtr.reset(new float[4]);

    for(int i=0; i<4; i++)
        m_materialPropertiesPtr[i] = static_cast<float>(props[i]);
}

// ****************************************************************************
//  Method: avtVisItVTKDevice::SetViewDirection
//
//  Purpose:
//      Set the camera view direction.
//
//  Arguments:
//      direction   view direction
//
//  Programmer: Allen R. Sanderson
//  Creation:   30 November 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtVisItVTKDevice::SetViewDirection(const double direction[3])
{
    if(!m_viewDirectionPtr)
        m_viewDirectionPtr.reset(new float[3]);

    for(int i=0; i<3; i++)
        m_viewDirectionPtr[i] = static_cast<float>(direction[i]);
}

// ****************************************************************************
//  Method: avtVisItVTKDevice::CreateCamera
//
//  Purpose:
//      The perspective camera implements a simple thin lens camera
//      for perspective rendering, supporting optionally depth of
//      field and stereo rendering, but not motion blur.
//
//      The orthographic camera implements a simple camera with
//      orthographic projection, without support for depth of field or
//      motion blur.
//
//  Returns:
//      A vtkCamera
//
//  Programmer: Allen R. Sanderson
//  Creation:   30 November 2021
//
//  Modifications:
//
// ****************************************************************************

vtkCamera *
avtVisItVTKDevice::CreateCamera()
{
    vtkCamera *camera = vtkCamera::New();
    viewInfo.SetCameraFromView( camera );

    return camera;
}

// ****************************************************************************
//  Method: avtVisItVTKDevice::CreateLights
//
//  Purpose:
//      Create a vtkLightCollection with up to 8 vtkLight sources
//      based on the active lights configured in VisIt. VisIt supports
//      a maximum of 8 lights so the vtkLightCollection returned will
//      have a maximum of 8 light sources if all of the lights are
//      enabled.
//
//  Returns:
//      A vtkLightCollection containing up to 8 vtkLight sources
//
//  Programmer: Allen R. Sanderson
//  Creation:   30 November 2021
//
//  Modifications:
//
// ****************************************************************************

vtkLightCollection *
avtVisItVTKDevice::CreateLights()
{
    double cpos[3], cfoc[3];

    vtkCamera *camera = CreateCamera();
    camera->GetPosition(cpos);
    camera->GetFocalPoint(cfoc);

    double pos[3], proj[3];
    proj[0] = cpos[0] - cfoc[0];
    proj[1] = cpos[1] - cfoc[1];
    proj[2] = cpos[2] - cfoc[2];

    double projLen = 0.0;
    for (size_t i = 0; i < 3; i++)
    {
        projLen += (proj[i] * proj[i]);
    }
    projLen = sqrt(projLen);

    int ambientCount = 0;
    m_ambientColor[0] = 0.0;
    m_ambientColor[1] = 0.0;
    m_ambientColor[2] = 0.0;

    m_numLightsEnabled = 0;
    m_ambientOn = false;
    m_ambientCoefficient = 0.0;

    vtkLight *firstNonAmbientLight = nullptr;

    vtkLightCollection* lights = vtkLightCollection::New();

    // VisIt has 8 lights
    for(int i=0; i<MAX_LIGHTS; i++)
    {
        auto lightAttributes = m_lightList.GetLight(i);

        vtkLight *light = vtkLight::New();

        // color
        light->SetColor(static_cast<double>(lightAttributes.GetColor().Red()  ) / 255.0f,
                        static_cast<double>(lightAttributes.GetColor().Green()) / 255.0f,
                        static_cast<double>(lightAttributes.GetColor().Blue() ) / 255.0f);

        // intensity
        light->SetIntensity(static_cast<double>(lightAttributes.GetBrightness()));

        light->SetFocalPoint( camera->GetFocalPoint());

        lights->AddItem(light);

        LightAttributes::LightType type = lightAttributes.GetType();

        switch (type)
        {
            case LightAttributes::Camera:
                light->SetLightTypeToCameraLight();

                // Position based on the direction
                light->SetFocalPoint(0.0, 0.0, 0.0);

                light->SetPosition(static_cast<double>(-lightAttributes.GetDirection()[0]),
                                   static_cast<double>(-lightAttributes.GetDirection()[1]),
                                   static_cast<double>(-lightAttributes.GetDirection()[2]));

                if (firstNonAmbientLight == nullptr)
                {
                    firstNonAmbientLight = light;
                }

                if (lightAttributes.GetEnabledFlag())
                {
                    light->SwitchOn();
                    m_numLightsEnabled++;
                }
                else
                {
                    light->SwitchOff();
                }
                break;

            case LightAttributes::Object:
                light->SetLightTypeToSceneLight();

                pos[0] = cfoc[0] - lightAttributes.GetDirection()[0] / projLen;
                pos[1] = cfoc[1] - lightAttributes.GetDirection()[1] / projLen;
                pos[2] = cfoc[2] - lightAttributes.GetDirection()[2] / projLen;

                light->SetPosition(pos);
                light->SetFocalPoint(cfoc);

                if (firstNonAmbientLight == nullptr)
                {
                    firstNonAmbientLight = light;
                }

                if (lightAttributes.GetEnabledFlag())
                {
                    light->SwitchOn();
                    m_numLightsEnabled++;
                }
                else
                {
                    light->SwitchOff();
                }
                break;

            case LightAttributes::Ambient:
                //
                //  Ambient light is handled by a different mechanism in VTK,
                //  so don't turn on the vtkLight, but set some flags to
                //  handle the ambient settings, if this light is enabled.
                //
                light->SwitchOff();

                if(lightAttributes.GetEnabledFlag())
                {
                    m_ambientColor[0] += lightAttributes.GetColor().Red()   * lightAttributes.GetBrightness();
                    m_ambientColor[1] += lightAttributes.GetColor().Green() * lightAttributes.GetBrightness();
                    m_ambientColor[2] += lightAttributes.GetColor().Blue()  * lightAttributes.GetBrightness();
                    m_ambientCoefficient += lightAttributes.GetBrightness();
                    m_ambientOn = true;
                    ambientCount++;
                }

                break;

            default:
                break;
        }
    }

    if (m_ambientOn)
    {
        //
        // Using an averaged ambientCoefficent for multiple ambient
        // lights yields a more pleasing visual result.
        //
        m_ambientCoefficient /= (double) ambientCount;
        // FIXME - now done in ExecuteVolume
        // canvas->SetAmbient(ambientColor);
    }
    else
    {
        // FIXME - now done in ExecuteVolume
        // canvas->SetAmbient(1., 1., 1.);
    }

    if (m_numLightsEnabled == 0 && firstNonAmbientLight != nullptr)
    {
        //
        //  Cannot leave all the lights OFF, because VTK will create a
        //  default light that is difficult to get rid of, so turn on a
        //  non-ambient light.  We don't want it to contribute anything
        //  to the scene, so set its intensity to zero.
        //
        firstNonAmbientLight->SwitchOn();
        firstNonAmbientLight->SetIntensity(0.);
    }

    return lights;
}

// ****************************************************************************
//  Method: avtVisItVTKDevice::CreateFinalImage
//
//  Purpose:
//      If serial, copy the VTK rendered frame into an avtImage to pass back
//      to VisIt. If parallel, composite image with the other processors then
//      pass the composited image back to VisIt.
//
//  Arguments:
//      colorBuffer the VTK rendered frame
//      width       frame width
//      height      frame height
//      zDepth      the average z-depth of this rendered frame
//
//  Returns:
//      An avtImage of the complete scene
//
//  Programmer: Allen R. Sanderson
//  Creation:   30 November 2021
//
//  Modifications:
//
// ****************************************************************************

avtImage_p
avtVisItVTKDevice::CreateFinalImage(const void *colorBuffer,
                                          const int width,
                                          const int height,
                                          const float zDepth)
{
    const int nColorChannels = 4;
    avtImage_p finalImage = new avtImage(this);
    vtkImageData *finalImageData =
        avtImageRepresentation::NewImage(width, height, nColorChannels);

    finalImage->GetImage().SetImageVTK(finalImageData);

    float bgColor[3];
    bgColor[0] = float(background[0]) / 255.0f;
    bgColor[1] = float(background[1]) / 255.0f;
    bgColor[2] = float(background[2]) / 255.0f;

    std::unique_ptr<avtICETCompositor>
      compositor(new avtICETCompositor(zDepth, bgColor));

    compositor->Composite(colorBuffer,
                          finalImage->GetImage().GetRGBBuffer(),
                          width,
                          height,
                          nColorChannels);

    // For debugging
    // if(PAR_Rank() == 0)
    // {
    //   vtkPNGWriter* writer = vtkPNGWriter::New();

    //   writer->SetInputData(finalImageData);
    //   writer->SetFileName("finialImage.png");
    //   writer->Write();
    //   writer->Delete();
    // }

    finalImageData->Delete();

    return finalImage;
}

// ****************************************************************************
//  Method: avtVisItVTKDevice::Execute
//
//  Purpose:
//      Executes the VTK Rendering backend.
//      This means:
//      - Put the input mesh through a transform so it is in camera space.
//      - Get the sample points.
//      - Communicate the sample points (parallel only).
//      - Composite the sample points along rays.
//      - Communicate the pixels from each ray (parallel only).
//      - Output the image.
//
//  Programmer: Allen R. Sanderson
//  Creation:   30 November 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtVisItVTKDevice::Execute()
{
    StackTimer t0("VisItVTKDevice Rendering");

    const char *moduleName = GetDeviceType();

    if(m_dataType == DataType::VOLUME)
        ExecuteVolume();
    else
        ; // TODO: Implement ExecuteSurface(); // Done as PseudoColorPlot
}

// ****************************************************************************
//  Method: avtVisItVTKDevice::ExecuteVolume
//
//  Purpose:
//      Executes Volume Rendering using the VTK Rendering backend.
//
//  Programmer: Allen R. Sanderson
//  Creation:   30 November 2021
//
//  Modifications:
//
// ****************************************************************************
#define NO_DATA_VALUE -1e+37

void
avtVisItVTKDevice::ExecuteVolume()
{
    const int width  = screen[0];
    const int height = screen[1];

#ifndef HAVE_OSPRAY
    if( m_renderingAttribs.OSPRayEnabled )
    {
        avtCallback::IssueWarning("Trying to use OSPRay when VTK was not built with OSPRay support. The default VTK renderering will be used.");
    }
#endif

    auto dob = GetInput();
    auto inputTree = GetInputDataTree(); // avtDataTree_p
    int nsets = 0;
    vtkDataSet **datasetPtrs = inputTree->GetAllLeaves(nsets);
    debug5 << "[VisItVTKDevice] nsets: " << nsets << std::endl;

    // GetDataExtents is a parallel call so do them regardless if
    // there is data or not. Otherwise MPI crashes.
    double    dataRange[2] = {0., 1.};
    double opacityRange[2] = {0., 1.};

    GetDataExtents(dataRange, activeVarName.c_str());
    UnifyMinMax(dataRange, 2);

    if( m_renderingAttribs.useColorVarMin )
        dataRange[0] = m_renderingAttribs.colorVarMin;
    if( m_renderingAttribs.useColorVarMax )
        dataRange[1] = m_renderingAttribs.colorVarMax;

    // There could be separate scalar and opacity components.
    if( opacityVarName != "default" && opacityVarName != activeVarName )
    {
        m_nComponents = 2;
        GetDataExtents(opacityRange, opacityVarName.c_str());
        UnifyMinMax(opacityRange, 2);

        if( m_renderingAttribs.useOpacityVarMin )
            opacityRange[0] = m_renderingAttribs.opacityVarMin;
        if( m_renderingAttribs.useOpacityVarMax )
            opacityRange[1] = m_renderingAttribs.opacityVarMax;
    }
    else
    {
        m_nComponents = 1;
    }

    LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
              << "rank: "  << PAR_Rank() << " in  "
              << "nsets: " << nsets << "  "
              << "nComponents: " << m_nComponents << "  "
              << std::endl;

    // Flags to make sure the data and opacity have the same
    // centering. Used when checking for required resampling.
    bool dataCellCentering = false, opacityCellCentering = false;

    // If there is input data get the centering and check if
    // resampling is required.
    if( nsets >= 1 )
    {
        vtkDataSet* in_ds = datasetPtrs[ 0 ];

        // Get the data centering.
        if( in_ds->GetPointData()->GetScalars() != nullptr )
            dataCellCentering = false;
        else if( in_ds->GetCellData()->GetScalars() != nullptr )
            dataCellCentering = true;
    }

    // Check for a user resampling request, it is the same on all ranks.
    bool userResample =
      (m_renderingAttribs.resampleType &&
       (m_resampleType != m_renderingAttribs.resampleType ||
        m_renderingAttribs.resampleTargetVal != m_resampleTargetVal ||
        m_renderingAttribs.resampleCentering != m_resampleCentering));

    // If there is user resampling then a new image is needed.
    bool needImage = userResample;

    // Check to see if the data must be resampled.
    int mustResample = NoResampling;

    // Only check if not doing user resampling.
    if( !userResample )
    {
        // If the resampling has changed, or if
        // the color/opacity variable/min/max has changed then a new
        // image is needed.
        if(
            // Resampling change
            m_resampleType != m_renderingAttribs.resampleType ||
            (m_renderingAttribs.resampleType &&
             (m_renderingAttribs.resampleTargetVal != m_resampleTargetVal ||
              m_renderingAttribs.resampleCentering != m_resampleCentering)) ||

            // Color variable change or min/max change. The active var
            // name triggers needing an image on the first pass.
            m_activeVarName != activeVarName ||
            m_useColorVarMin != m_renderingAttribs.useColorVarMin ||
            (m_renderingAttribs.useColorVarMin &&
             m_colorVarMin != m_renderingAttribs.colorVarMin) ||
            m_useColorVarMax != m_renderingAttribs.useColorVarMax ||
            (m_renderingAttribs.useColorVarMax &&
             m_colorVarMax != m_renderingAttribs.colorVarMax) ||

            // Opacity variable change or min/max change.
            (m_nComponents == 2 &&
             (m_opacityVarName != opacityVarName ||
              m_useOpacityVarMin != m_renderingAttribs.useOpacityVarMin ||
              (m_renderingAttribs.useOpacityVarMin &&
               m_opacityVarMin != m_renderingAttribs.opacityVarMin) ||
              m_useOpacityVarMax != m_renderingAttribs.useOpacityVarMax ||
              (m_renderingAttribs.useOpacityVarMax &&
               m_opacityVarMax != m_renderingAttribs.opacityVarMax)))
            )
        {
            needImage = true;

            // Check if resampling is required.
            if( nsets >= 1 )
            {
                vtkDataSet* in_ds = datasetPtrs[ 0 ];

                // If more than one data set then resampling is required.
                if( nsets > 1 )
                    mustResample |= MutlipleDatasets;
                // If the data is not on a rectilinear grid then
                // resampling is required.
                if( in_ds->GetDataObjectType() != VTK_RECTILINEAR_GRID )
                    mustResample |= NonRectilinearGrid;

                // Get the opacity centering.
                if( m_nComponents == 2 )
                {
                    if( in_ds->GetPointData()->GetScalars( opacityVarName.c_str() ) != nullptr )
                        opacityCellCentering = false;
                    else if( in_ds->GetCellData()->GetScalars( opacityVarName.c_str() ) != nullptr )
                        opacityCellCentering = true;
                }
                else // if( m_nComponents == 1 )
                    opacityCellCentering = dataCellCentering;

                // If the data centering does not match the opacity
                // centering then resampling is required.
                if( dataCellCentering != opacityCellCentering )
                    mustResample |= DifferentCentering;
            }
        }

        // If one data set needs to be resample then all data sets will be
        // resampled as avtResampleFilter is a parallel call. Otherwise
        // MPI crashes.
        mustResample = UnifyBitwiseOrValue(mustResample);
    }

    // Corner case when initally there are more ranks than domains. If
    // there is no data initially but after a resampling there is
    // data. Then if a new image is needed force a resampling so to
    // have data for the new image.
    if( needImage && m_needResampledData )
    {
        mustResample |= AttributesChanged;

        // dob = m_resampleFilter->GetOutput();

        // // Get the data tree which may be the original data or from the
        // // resampled data.
        // inputTree = ((avtDataset*) *dob)->GetDataTree(); // avtDataTree_p

        // nsets = 0;
        // datasetPtrs = inputTree->GetAllLeaves(nsets);
    }

    LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                << "rank: "  << PAR_Rank() << " "
                << (needImage    ? "need image  " : "")
                << (userResample ? "user resample  " : "")
                << (mustResample ? "must resample  " : "")
                << (mustResample ? mustResample : 0)
                << std::endl;

    // Store the resample type, target value, and centering so to
    // check for a state change.
    m_resampleType      = m_renderingAttribs.resampleType;
    m_resampleTargetVal = m_renderingAttribs.resampleTargetVal;
    m_resampleCentering = m_renderingAttribs.resampleCentering;

    // Store the color variable values so to check for a state change.
    m_activeVarName = activeVarName;
    m_useColorVarMin = m_renderingAttribs.useColorVarMin;
    m_colorVarMin    = m_renderingAttribs.colorVarMin;
    m_useColorVarMax = m_renderingAttribs.useColorVarMax;
    m_colorVarMax    = m_renderingAttribs.colorVarMax;

    // Store the opacity variable values so to check for a state change.
    m_opacityVarName = opacityVarName;
    m_useOpacityVarMin = m_renderingAttribs.useOpacityVarMin;
    m_opacityVarMin    = m_renderingAttribs.opacityVarMin;
    m_useOpacityVarMax = m_renderingAttribs.useOpacityVarMax;
    m_opacityVarMax    = m_renderingAttribs.opacityVarMax;

    // If the data must be resampled and user did not request
    // resampling report a warning so the user knows the data has
    // been resampled.
    if( mustResample && !userResample )
    {
        // Unify the values because it is possible that a rank may not
        // have any data.
        dataCellCentering    = UnifyMaximumValue(dataCellCentering);
        opacityCellCentering = UnifyMaximumValue(opacityCellCentering);

        std::string msg("The data must be resampled because ");

        if( mustResample & MutlipleDatasets )
        {
            msg += "each rank has more than one domain";

            if( mustResample & (NonRectilinearGrid | DifferentCentering | AttributesChanged))
                msg += ", and ";
        }

        if( mustResample & NonRectilinearGrid )
        {
            msg += "the data is not on a rectilinear grid";

            if( mustResample & (DifferentCentering | AttributesChanged))
                msg += ", and ";
        }

        if( mustResample & DifferentCentering)
        {
            msg += "the data and opacity have different centering";

            if( mustResample & AttributesChanged)
                msg += ", and ";
        }

        if( mustResample & AttributesChanged)
        {
            msg += "the data and/or opacity attributes have changed";
        }

        msg += ". The data and if needed the opacity have been resampled on to a ";

        if( PAR_Size() == 1 )
            msg += "single rectilinear grid.";
        else
        {
            if(m_renderingAttribs.resampleType == ParallelRedistribute)
                msg += "common rectilinear grid and redistributed to each rank.";
            else if(m_renderingAttribs.resampleType == ParallelPerRank )
                msg += "rectilinear grid on each rank.";
            else
                msg += "common rectilinear grid and redistributed to each rank.";
        }

        avtCallback::IssueWarning(msg.c_str());
    }

    // Resampling
    if(userResample || mustResample)
    {
        if(PAR_Size() == 1 &&
           (m_renderingAttribs.resampleType == ParallelRedistribute ||
            m_renderingAttribs.resampleType == ParallelPerRank))
        {
            avtCallback::IssueWarning("Running in serial but parallel resampling was selected. "
                                      "Single domain sampling will be performed.");
        }
        else if(PAR_Size() > 1 &&
                 m_renderingAttribs.resampleType == SingleDomain)
        {
            avtCallback::IssueWarning("Running in parallel but single domain resampling was selected. Parallel resampling should be used.");
        }

        // If the user selected a specific centering use
        // it. Otherwise use the centering from the color data.
        if( m_renderingAttribs.resampleCentering )
            dataCellCentering =
                m_renderingAttribs.resampleCentering == CellCentering;

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                    << "rank: "  << PAR_Rank() << "  "
                    << "centering requested " << m_renderingAttribs.resampleCentering << "  doing "
                    << (dataCellCentering ? "cell " :  "point ")
                    << "centered resampling"
                    << std::endl;

        // Create a dummy pipeline within the device so to force an
        // execute within this "Execute".  Start with the source.
        avtDataset_p ds;
        CopyTo(ds, dob);
        avtSourceFromAVTDataset termsrc(ds);

        // Resample the data - must be done by all ranks.
        InternalResampleAttributes resampleAtts;

        // User requested resampling. If the type is 1 then resample
        // on to a single domain. Otherwise resample in parallel
        // (ignored if running in serial).
        if( PAR_Size() > 1 )
        {
            if( userResample )
            {
                if( m_renderingAttribs.resampleType == ParallelRedistribute )
                  resampleAtts.SetDistributedResample(true);
                else if( m_renderingAttribs.resampleType == ParallelPerRank )
                  resampleAtts.SetPerRankResample(true);
            }
            // Must resample but the user selected 'only if required' so
            // do a distributed resample.
            else //if( mustResample )
            {
                resampleAtts.SetDistributedResample(true);
            }
        }

        resampleAtts.SetTargetVal(m_renderingAttribs.resampleTargetVal);
        resampleAtts.SetPrefersPowersOfTwo(true);
        resampleAtts.SetUseTargetVal(true);

        if (m_resampleFilter != nullptr)
        {
            delete m_resampleFilter;
            m_resampleFilter = nullptr;
        }

        avtResampleFilter *m_resampleFilter =
            new avtResampleFilter(&resampleAtts);

        m_resampleFilter->MakeOutputCellCentered( dataCellCentering );

        m_resampleFilter->SetInput( termsrc.GetOutput() );
        // Prevent this intermediate object from getting cleared
        // out, so it is still there when rendering.
        m_resampleFilter->GetOutput()->SetTransientStatus(false);

        dob = m_resampleFilter->GetOutput();

        // Since this is Execute, forcing an update is okay...
        dob->Update(GetGeneralContract());

        // Get the data tree which may be the original data or from the
        // resampled data.
        inputTree = ((avtDataset*) *dob)->GetDataTree(); // avtDataTree_p

        nsets = 0;
        datasetPtrs = inputTree->GetAllLeaves(nsets);

        // This flag indicates that after resmapling there is data on
        // this rank and that an image will be rendered.
        m_imageOnThisRank = (nsets == 1);

        // If an image is needed and the data came from resampling the
        // resampled data has to be regenerated as it is transient.
        m_needResampledData = true;
    }
    else if( m_renderingAttribs.resampleType == OnlyIfRequired )
    {
        if( needImage )
        {
            // This flag indicates that there is data on this rank and
            // that an image will be rendered.
            m_imageOnThisRank = (nsets == 1);
            m_needResampledData = false;
        }
    }

    LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
              << "rank: "  << PAR_Rank() << " out "
              << "nsets: " << nsets << "  "
              << "imageOnThisRank: " << m_imageOnThisRank << "  "
              << "needResampledData: " << m_needResampledData << "  "
              << std::endl;

    // If no image will be generated on this rank account for the
    // parallel compositing.
    if( m_imageOnThisRank == false )
    {
        debug5 << "[VisItVTKDevice] Nothing to render, no data." << std::endl;

#ifdef PARALLEL
        // So the parallel case can still work
        avtImage_p finalImage =
          CreateFinalImage(nullptr, width, height, FLT_MAX);

        if(PAR_Rank() == 0)
          SetOutput(finalImage);
#endif

        if( m_imageToRender != nullptr )
        {
            m_imageToRender->Delete();
            m_imageToRender = nullptr;
        }

        return;
    }

    // Create a new image if needed.
    if( needImage )
    {
        // After resampling there should be only one rectilinear data set.
        if( nsets > 1 )
        {
            LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                        << "rank: "  << PAR_Rank() << " "
                        << "Too many datasets to render, skipping." << std::endl;
            EXCEPTION1(ImproperUseException, "Only one input dataset may be rendered.");
        }

        // Check for an implied transform.
        avtDataAttributes &inatts = GetInput()->GetInfo().GetAttributes();
        if (inatts.GetRectilinearGridHasTransform())
        {
            EXCEPTION1(ImproperUseException,
                       "vtkRectilinear grids with an implied transform can not be rendered.");
        }

        // Now to the business
        vtkDataSet* in_ds = datasetPtrs[ 0 ];

        if( in_ds->GetDataObjectType() != VTK_RECTILINEAR_GRID )
        {
            EXCEPTION1(ImproperUseException,
                       "Only a vtkRectilinearGrid can be rendered.");
        }

        vtkRectilinearGrid* rgrid = vtkRectilinearGrid::SafeDownCast( in_ds );

        // Get the bounds and extents.
        double bounds[6];
        rgrid->GetBounds(bounds);

        int dims[3], extent[6];
        rgrid->GetDimensions(dims);
        rgrid->GetExtent(extent);

        // Get the spacing from the input grid.
        double spacingX = (rgrid->GetXCoordinates()->GetTuple1(1)-
                           rgrid->GetXCoordinates()->GetTuple1(0));
        double spacingY = (rgrid->GetYCoordinates()->GetTuple1(1)-
                           rgrid->GetYCoordinates()->GetTuple1(0));
        double spacingZ = (rgrid->GetZCoordinates()->GetTuple1(1)-
                           rgrid->GetZCoordinates()->GetTuple1(0));

        // Get the active variable scalar data array.
        vtkDataArray *dataArr = in_ds->GetPointData()->GetScalars();

        if( dataArr )
        {
            m_cellData = false;
        }
        else
        {
            dataArr = in_ds->GetCellData()->GetScalars();

            if( dataArr == nullptr )
            {
                EXCEPTION1(InvalidVariableException, activeVarName);
            }

            m_cellData = true;
        }

        // There could be a spearate opacity scalar data array.
        vtkDataArray *opacityArr = nullptr;
        if( m_nComponents == 2 )
        {
            opacityArr = in_ds->GetPointData()->GetScalars( opacityVarName.c_str() );
            if( m_cellData && opacityArr )
            {
                EXCEPTION1(ImproperUseException, "The opacity data does not have the same centering (point) as the primary data (cell). This expection can be fixed by resampling the data on to a common mesh.");
            }

            if( opacityArr == nullptr )
            {
                opacityArr = in_ds->GetCellData()->GetScalars( opacityVarName.c_str() );

                if( !m_cellData && opacityArr )
                {
                    EXCEPTION1(ImproperUseException, "The opacity data does not have the same centering (cell) as the primary data (point). This expection can be fixed by resampling the data on to a common mesh.");
                }

                if( opacityArr == nullptr )
                {
                    EXCEPTION1(InvalidVariableException, opacityVarName);
                }
            }
        }

        // Might be useful - not sure yet???
        // vtkDataArray *gradientArr = nullptr;
        // if( gradientVarName != "default" )
        //   gradientArr = in_ds->GetPointData()->GetVectors( gradientVarName.c_str() );


        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                  << "rank: "  << PAR_Rank() << " dims : "
                  << dims[0] << "  " << dims[1] << "  "<< dims[2] << "  "
                  << std::endl;

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                  << "rank: "  << PAR_Rank() << " extent : "
                  << extent[0] << "  " << extent[1] << "  "
                  << extent[2] << "  " << extent[3] << "  "
                  << extent[4] << "  " << extent[5] << "  "
                  << std::endl;

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                  << "rank: "  << PAR_Rank() << " bounds : "
                  << bounds[0] << "  " << bounds[1] << "  "
                  << bounds[2] << "  " << bounds[3] << "  "
                  << bounds[4] << "  " << bounds[5] << "  "
                  << std::endl;

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                  << "rank: "  << PAR_Rank() << " spacing : "
                  << spacingX << "  " << spacingY << "  " << spacingZ << "  "
                  << std::endl;

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                  << "rank: "  << PAR_Rank() << " data range : "
                  << dataRange[0] << "  " << dataRange[1] << "  "
                  << std::endl;

        if( m_nComponents == 2 )
            LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                        << "rank: "  << PAR_Rank() << " opacity range : "
                        << opacityRange[0] << "  " << opacityRange[1] << "  "
                        << std::endl;

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                    << "rank: "  << PAR_Rank() << " expecting "
                    << (m_cellData ? "cell " : "point ") << "data "
                    << std::endl;

        // The volume mapper requires a vtkImageData as input.
        if( m_imageToRender != nullptr )
            m_imageToRender->Delete();

        m_imageToRender = vtkImageData::New();
        m_imageToRender->SetDimensions(dims);
        m_imageToRender->SetExtent(extent);
        m_imageToRender->SetSpacing(spacingX, spacingY, spacingZ);
        m_imageToRender->SetOrigin(bounds[0], bounds[2], bounds[4]);

        // The color and opacity data may be separate components which
        // requires the IndependentComponents in the
        // vtkVolumeProperties set to 'off'

        // Data size for the image data as either point or cell data.
        vtkIdType dataSize =
            (dims[0]-m_cellData) * (dims[1]-m_cellData) * (dims[2]-m_cellData);

        // Allocate the new scalars
        vtkDataArray* scalars = vtkDataArray::CreateDataArray(VTK_FLOAT);
        scalars->SetName("ImageScalars");
        scalars->SetNumberOfComponents(m_nComponents);
        scalars->SetNumberOfTuples(dataSize);

        if( m_cellData )
            m_imageToRender->GetCellData()->SetScalars(scalars);
        else
            m_imageToRender->GetPointData()->SetScalars(scalars);

        scalars->Delete();

        // The values on the image must be scale to between 0 and 255.
        double dataScale    = 255.0f / (   dataRange[1] -    dataRange[0]);
        double opacityScale = 255.0f / (opacityRange[1] - opacityRange[0]);

        if( dataRange[1] <= dataRange[0])
            dataScale = 0;

        if( opacityRange[1] <= opacityRange[0])
            opacityScale = 0;

        // double data_min =  1e6;
        // double data_max = -1e6;

        // double opacity_min =  1e6;
        // double opacity_max = -1e6;

        // Transfer the rectilinear grid data to the image data and
        // scale to the proper range.
        m_useInterpolation = true;

        // VisIt populates empty space with the NO_DATA_VALUE.
        // This needs to map this to a value that the mapper accepts,
        // and then clamp it so to be fully translucent.
        int ptId = 0;
        for (int z = 0; z < dims[2]-m_cellData; ++z)
        {
            for (int y = 0; y < dims[1]-m_cellData; ++y)
            {
                for (int x = 0; x < dims[0]-m_cellData; ++x)
                {
                    // The color and opacity data may differ so add
                    // both as two separate components.
                    double dataTuple = dataArr->GetTuple1(ptId);
                    if (dataTuple <= NO_DATA_VALUE)
                    {
                        // The color map is 0 -> 255. For no data values,
                        // assign a new value just out side of the map.
                        scalars->SetComponent(ptId, 0, -1.0);
                        m_useInterpolation = false;
                    }
                    else
                    {
                        double val = (dataTuple - dataRange[0]) * dataScale;

                        if( val < 0   ) val = 0;
                        if( val > 255 ) val = 255;

                        scalars->SetComponent(ptId, 0, val);

                        // if( data_min > val ) data_min = val;
                        // if( data_max < val ) data_max = val;
                    }

                    if( m_nComponents == 2 )
                    {
                        double opacityTuple = opacityArr->GetTuple1(ptId);
                        if (opacityTuple <= NO_DATA_VALUE)
                        {
                            // The opacity map is 0 -> 255. For no data values,
                            // assign a new value just out side of the map.
                            scalars->SetComponent(ptId, 1, -1.0);
                            m_useInterpolation = false;
                        }
                        else
                        {
                            double val = (opacityTuple - opacityRange[0]) * opacityScale;
                            if( val < 0   ) val = 0;
                            if( val > 255 ) val = 255;

                            scalars->SetComponent(ptId, 1, val);

                            // if( opacity_min > val ) opacity_min = val;
                            // if( opacity_max < val ) opacity_max = val;
                        }
                    }

                    // double gradientTuple = gradientArr->GetTuple1(ptId);
                    // if (gradientTuple <= NO_DATA_VALUE)
                    // {
                    //     // The gradient map is 0 -> 255. For no data values,
                    //     // assign a new value just out side of the map.
                    //     scalars->SetComponent(ptId, 2, -1.0);
                    //     m_useInterpolation = false;
                    // }
                    // else
                    // {
                    //     scalars->SetComponent(ptId, 2, vale);
                    // }

                    ptId++;
                }
            }
        }

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                    << "rank: "  << PAR_Rank() << " useInterpolation: "
                    << m_useInterpolation << "  "
                    << std::endl;

        // LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
        //             << "rank: "  << PAR_Rank()
        //             << " NumberOfTuples: "
        //             << dataArr->GetNumberOfTuples() << "  "
        //             << " NumberOfTuples: "
        //             << ptId
        //             << std::endl;

        // LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
        //             << "rank: "  << PAR_Rank()
        //             << " dataRange: "
        //             << data_min << "  " << data_max << "  ";
        // if( m_nComponents == 2 )
        //     LOCAL_DEBUG << " opacityRange: "
        //                 << opacity_min << "  " << opacity_max << "  ";
        // LOCAL_DEBUG << std::endl;

        // For debugging
        // {
        //     vtkXMLImageDataWriter* writer = vtkXMLImageDataWriter::New();

        //     writer->SetInputData(m_imageToRender);
        //     if( m_nComponents == 2 )
        //       writer->SetFileName("Image_Large_2_Comps.vti");
        //     else
        //     writer->SetFileName("Image_Large_1_Comps.vti");

        //     writer->Write();
        //     writer->Delete();
        // }
    }

    // Create a new volume mapper if needed.
    if( m_volumeMapper == nullptr ||
        m_OSPRayEnabled != m_renderingAttribs.OSPRayEnabled)
    {
        m_OSPRayEnabled = m_renderingAttribs.OSPRayEnabled;

        if (m_volumeMapper != nullptr)
            m_volumeMapper->Delete();

        // Create the volume mapper.
#ifdef HAVE_OSPRAY
        if( m_renderingAttribs.OSPRayEnabled )
        {
            vtkOSPRayVolumeMapper * vm = vtkOSPRayVolumeMapper::New();
            m_volumeMapper = vm;

            LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                        << "rank: "  << PAR_Rank() << " OSPRay Volume Mapper "
                        << std::endl;
        }
        else
#endif
        {
            vtkGPUVolumeRayCastMapper * vm = vtkGPUVolumeRayCastMapper::New();
            m_volumeMapper = vm;

            LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                        << "rank: "  << PAR_Rank() << " GPU Volume Ray Cast Mapper "
                        << std::endl;
        }

        m_volumeMapper->SetBlendModeToComposite();
    }

    if( m_cellData )
        m_volumeMapper->SetScalarModeToUseCellData();
    else
        m_volumeMapper->SetScalarModeToUsePointData();

    m_volumeMapper->SetInputData(m_imageToRender);

    // Upstream in avtVolumeFIlter an new color and opacity map are
    // created each time so recreate them here.

    // Create the transfer function and the opacity mapping.
    const RGBAF *transferTable = transferFn1D->GetTransferFunc();
    int tableSize = transferFn1D->GetNumberOfTableEntries();

    vtkColorTransferFunction* transFunc = vtkColorTransferFunction::New();
    vtkPiecewiseFunction*     opacity   = vtkPiecewiseFunction::New();
    // vtkPiecewiseFunction*     gradient  = vtkPiecewiseFunction::New();

    // Add the color map to vtk's transfer function
    for(int i=0; i<tableSize; i++)
    {
        transFunc->AddRGBPoint( i,
                                transferTable[i].R,
                                transferTable[i].G,
                                transferTable[i].B );
        opacity->AddPoint( i, transferTable[i].A );
    }

    // For some reason, the endpoints aren't included when
    // clamping is turned off. So add some padding on the ends of
    // our mapping functions.
    if( m_useInterpolation == false )
    {
        int i = 0;
        transFunc->AddRGBPoint( -1.0,
                                transferTable[i].R,
                                transferTable[i].G,
                                transferTable[i].B);
        opacity->AddPoint( -1.0, transferTable[i].A);

        i = tableSize - 1;
        transFunc->AddRGBPoint( tableSize,
                                transferTable[i].R,
                                transferTable[i].G,
                                transferTable[i].B);
        opacity->AddPoint( tableSize, transferTable[i].A  );
    }

    // To make the NO_DATA_VALUEs fully translucent turn clamping
    // off (opacity becomes 0.0)
    transFunc->SetScaleToLinear();
    transFunc->SetClamping(m_useInterpolation);
    opacity->SetClamping(m_useInterpolation);
    // gradient->SetClamping(m_useInterpolation);

    // Set the volume properties.
    vtkVolumeProperty * volumeProperty = vtkVolumeProperty::New();
    volumeProperty->SetColor(transFunc);
    volumeProperty->SetScalarOpacity(opacity);
    // volumeProperty->SetGradientOpacity(gradient);
    volumeProperty->SetIndependentComponents( m_nComponents == 1 );
    volumeProperty->SetShade( m_renderingAttribs.lightingEnabled );

    LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
              << "rank: "  << PAR_Rank() << " HasGradientOpacity: "
              << volumeProperty->HasGradientOpacity() << "  "
              << std::endl;

    LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                << "rank: "  << PAR_Rank()
                << " lightingEnabled: " << m_renderingAttribs.lightingEnabled << "  "
                << m_materialPropertiesPtr[0] << "  "
                << m_materialPropertiesPtr[1] << "  "
                << m_materialPropertiesPtr[2] << "  "
                << m_materialPropertiesPtr[3] << "  "
                << std::endl;

    // Set ambient, diffuse, specular, and specular power (shininess).
    if( 0 && m_renderingAttribs.lightingEnabled )
    {
        volumeProperty->SetAmbient      (m_materialPropertiesPtr[0]);
        volumeProperty->SetDiffuse      (m_materialPropertiesPtr[1]);
        volumeProperty->SetSpecular     (m_materialPropertiesPtr[2]);
        volumeProperty->SetSpecularPower(m_materialPropertiesPtr[3]);
    }

    // If the dataset contains NO_DATA_VALUEs, interpolation will
    // not work correctly on the boundaries (between a real value
    // and a no data value). Hopefully this will be addressed in the
    // future. For now, only interpolate when the dataset contains
    // none of these values.
    if (m_useInterpolation)
    {
        volumeProperty->SetInterpolationTypeToLinear();
    }
    else
    {
        volumeProperty->SetInterpolationTypeToNearest();
    }

    // A sample distance needs to be calculate the so to apply an
    // opacity correction.
    //
    // NOTE 1: vtkSmartVolumeMapper->SetSampleDistance does not work, so we
    // need to rely on vtkVolumeProperty->SetScalarOpacityUnitDistance.
    //
    // NOTE 2: This magic number "sampleDistReference" is completely
    // made up. It acts as a "reference sample count" that results in
    // an opacity correction that generally "looks good". Increasing this
    // value will result in an increased opacity intensity, while decreasing
    // this value will result in a decreased opacity intensity.
    double spacing[3];
    m_imageToRender->GetSpacing(spacing);

    double sampleDistReference = 1.0 / 10.0;
    double averageSpacing = (spacing[0] + spacing[1] + spacing[2]) / 3.0;
    double sampleDist     = averageSpacing / sampleDistReference;

    volumeProperty->SetScalarOpacityUnitDistance(1, sampleDist);

    LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
              << "rank: "  << PAR_Rank() << " sampleDist: "
              << sampleDist << "  "
              << std::endl;

    // vtkSmartVolumeMapper::SafeDownCast( volumeMapper )->SetSampleDistance( averageSpacing * 0.1);

    // vtkSmartVolumeMapper::SafeDownCast( volumeMapper )->AutoAdjustSampleDistancesOff();

    // Set up the volume
    vtkVolume * volume = vtkVolume::New();
    volume->SetMapper(m_volumeMapper);
    volume->SetProperty(volumeProperty);

    // Create camera
    vtkCamera* camera = CreateCamera();

    // Create lights
    vtkLightCollection* lights = CreateLights();

    // Create the renderer
    vtkRenderer* renderer = vtkRenderer::New();
    renderer->SetBackground(0.0f, 0.0f, 0.0f);
    renderer->SetBackgroundAlpha(0.0f);
    renderer->AddViewProp(volume);
    renderer->SetActiveCamera( camera );
    // renderer->SetLightCollection( lights );

#ifdef HAVE_OSPRAY
    if( m_renderingAttribs.OSPRayEnabled )
    {
        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                    << "rank: "  << PAR_Rank() << " RenderType: "
                    << (m_renderingAttribs.OSPRayRenderType ? "PathTracer" : "SciVis") << "  "
                    << std::endl;

        if( m_renderingAttribs.OSPRayRenderType == 1 )
            vtkOSPRayRendererNode::SetRendererType("pathtracer", renderer);
        else
            vtkOSPRayRendererNode::SetRendererType("scivis", renderer);

        vtkOSPRayRendererNode::SetSamplesPerPixel(m_renderingAttribs.OSPRaySamplesPerPixel, renderer);
        vtkOSPRayRendererNode::SetAmbientSamples (m_renderingAttribs.OSPRayAOSamples,       renderer);
        vtkOSPRayRendererNode::SetMinContribution(m_renderingAttribs.OSPRayMinContribution, renderer);
        vtkOSPRayRendererNode::SetMaxContribution(m_renderingAttribs.OSPRayMaxContribution, renderer);
    }
#endif

    if (m_ambientOn)
    {
        // LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
        //             << "rank: "  << PAR_Rank()
        //             << " ambientOn: " << m_ambientOn << "  "
        //             << m_ambientColor[0] << "  "
        //             << m_ambientColor[1] << "  "
        //             << m_ambientColor[2] << "  "
        //             << std::endl;

        renderer->SetAmbient(m_ambientColor);
    }
    else
    {
        // LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
        //             << "rank: "  << PAR_Rank()
        //          << " ambientOn: " << m_ambientOn << " 1, 1, 1 "
        //             << std::endl;

        renderer->SetAmbient(1., 1., 1.);
    }

    // Create an off screen render window.
    vtkRenderWindow* renderWin = vtkRenderWindow::New();
    renderWin->SetSize(width, height);
    renderWin->SetMultiSamples(false);
    renderWin->AddRenderer(renderer);
    renderWin->SetOffScreenRendering(true);
    renderWin->SetAlphaBitPlanes(true);

    camera->Render( renderer );

    // Finally render the image.
    renderWin->Render();

    // Get the resulting image - must have alpha values for ICET.
    unsigned char * renderedFrameBuffer =
        renderWin->GetRGBACharPixelData( 0, 0, width-1, height-1, 1 );

    // Debugging check for bad alpha values
    if( DebugStream::Level5() )
    {
        int cc = 0, aa = 0;
        for( int i=0; i<width*height*4; i+=4 )
        {
            double rval = double(renderedFrameBuffer[i+0]) / 255.0f;
            double gval = double(renderedFrameBuffer[i+1]) / 255.0f;
            double bval = double(renderedFrameBuffer[i+2]) / 255.0f;
            double aval = double(renderedFrameBuffer[i+3]) / 255.0f;

            if( aval > 0 )
            {
                ++aa;
                if(rval > aval || gval > aval || bval > aval)
                {
                    ++cc;
                }
            }
        }

        if( cc > 0 )
        {
            LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                        << "rank: "  << PAR_Rank() << " bad alpha composting "
                        << cc << "/" << aa << " are bad." << std::endl;
        }
    }

    // {
    //   vtkWindowToImageFilter* im = vtkWindowToImageFilter::New();
    //   vtkPNGWriter* writer = vtkPNGWriter::New();

    //   std::stringstream name;
    //   if( PAR_Size() > 1 )
    //       name << "renderWindow_" << PAR_Rank() << ".png";
    //   else
    //       name << "renderWindow.png";

    //   im->SetInput(renderWin);
    //   im->Update();
    //   writer->SetInputConnection(im->GetOutputPort());
    //   writer->SetFileName(name.str().c_str());
    //   writer->Write();
    //   writer->Delete();
    //   im->Delete();
    // }

    // Get the z centroid value in image space of the grid. It is used
    // for the ordering of the compositing in ICET.
    double scale[3] = {1,1,1};
    float aspect = 1.0f;
    if(height > 0)
    {
        aspect = static_cast<float>(width) / static_cast<float>(height);
    }

    vtkMatrix4x4 *transform = vtkMatrix4x4::New();
    avtWorldSpaceToImageSpaceTransform::CalculateTransform(viewInfo, transform,
                                                           scale, aspect);

    double bounds[6];
    m_imageToRender->GetBounds(bounds);
    double centroidPt[4], transPt[4];
    centroidPt[0] = bounds[0] + (bounds[1] - bounds[0]) / 2.0f;
    centroidPt[1] = bounds[2] + (bounds[3] - bounds[2]) / 2.0f;
    centroidPt[2] = bounds[4] + (bounds[5] - bounds[4]) / 2.0f;
    centroidPt[3] = 1.0f;

    transform->MultiplyPoint(centroidPt, transPt);
    transform->Delete();

    LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
              << "rank: "  << PAR_Rank() << " Average z depth: "
              << transPt[2] << "  " << std::endl;

    // Create final image using ICET
    avtImage_p finalImage =
        CreateFinalImage(renderedFrameBuffer, width, height, transPt[2]);

    if(PAR_Rank() == 0)
        SetOutput(finalImage);

    // Clean up.
    delete [] renderedFrameBuffer;

    transFunc->Delete();
    opacity->Delete();
    volumeProperty->Delete();
    volume->Delete();
    camera->Delete();
    lights->Delete();
    renderer->Delete();
    renderWin->Delete();
}
