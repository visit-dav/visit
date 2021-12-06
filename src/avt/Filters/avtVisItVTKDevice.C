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
#include <avtDatasetExaminer.h>
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
    m_dataType(DataType::GEOMETRY),
    m_lightList(),
    m_renderingAttribs(),
    m_materialPropertiesPtr(nullptr),
    m_viewDirectionPtr(nullptr)
{}

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
    if(imageToRender != nullptr)
        imageToRender->Delete();

    if(volumeMapper != nullptr)
        volumeMapper->Delete();
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
    int ambientCount = 0;
    ambientColor[0] = 0.0;
    ambientColor[1] = 0.0;
    ambientColor[2] = 0.0;

    numLightsEnabled = 0;
    ambientOn = false;
    ambientCoefficient = 0.;

    vtkLight *firstNonAmbientLight = nullptr;

    vtkLightCollection* lights = vtkLightCollection::New();

    // VisIt has 8 lights
    for(int i=0; i<8; i++)
    {
        auto lightAttributes = m_lightList.GetLight(i);

        vtkLight *light = vtkLight::New();

        // color
        light->SetColor(static_cast<double>(lightAttributes.GetColor().Red()  ) / 255.0f,
                        static_cast<double>(lightAttributes.GetColor().Green()) / 255.0f,
                        static_cast<double>(lightAttributes.GetColor().Blue() ) / 255.0f);

        // intensity
        light->SetIntensity(static_cast<double>(lightAttributes.GetBrightness()));

        LightAttributes::LightType type = lightAttributes.GetType();

        switch (type)
        {
            case LightAttributes::Ambient:
                //
                //  Ambient light is handled by a different mechanism in VTK,
                //  so don't turn on the vtkLight, but set some flags to
                //  handle the ambient settings, if this light is enabled.
                //
                light->SwitchOff();

                if(lightAttributes.GetEnabledFlag())
                {
                    ambientColor[0] += lightAttributes.GetColor().Red()   * lightAttributes.GetBrightness();
                    ambientColor[1] += lightAttributes.GetColor().Green() * lightAttributes.GetBrightness();
                    ambientColor[2] += lightAttributes.GetColor().Blue()  * lightAttributes.GetBrightness();
                    ambientCoefficient += lightAttributes.GetBrightness();
                    ambientOn = true;
                    ambientCount++;
                }

                break;

            case LightAttributes::Camera:
                light->SetLightTypeToCameraLight();

                light->SetFocalPoint(0.0, 0.0, 0.0);

                // Position based on the direction
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
                    numLightsEnabled++;
                }
                else
                {
                    light->SwitchOff();
                }
                break;

            case LightAttributes::Object:
                light->SetLightTypeToSceneLight();

                // FIXME - this is correct but not sure what to do ...
                light->SetFocalPoint(0.0, 0.0, 0.0);

                // Position based on the direction
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
                    numLightsEnabled++;
                }
                else
                {
                    light->SwitchOff();
                }
                break;

            default:
                break;
        }

        lights->AddItem(light);
    }

    if (ambientOn)
    {
        //
        // Using an averaged ambientCoefficent for multiple ambient
        // lights yields a more pleasing visual result.
        //
        ambientCoefficient /= (double) ambientCount;
        // FIXME - now done in ExecuteVolume
        // canvas->SetAmbient(ambientColor);
    }
    else
    {
        // FIXME - now done in ExecuteVolume
        // canvas->SetAmbient(1., 1., 1.);
    }

    if (numLightsEnabled == 0 && firstNonAmbientLight != nullptr)
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

    // There could be separate scalar and opacity components.
    if( opacityVarName != "default" ) //&& opacityVarName != activeVarName )
    {
        m_nComponents = 2;
        GetDataExtents(opacityRange, opacityVarName.c_str());
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

    // Make some checks first to see if an image is needed. After that
    // check to see if the data first needs to be resampled.
    bool needImage = false, mustResample = false;

    if( nsets >= 1 )
    {
        // If no image or the resampling has changed then a new image
        // is needed.
        if(imageToRender == nullptr ||
           m_resampleType != m_renderingAttribs.resampleType ||
           (m_renderingAttribs.resampleType &&
            m_renderingAttribs.resampleTargetVal != m_resampleTargetVal))
        {
            needImage = true;

            // If more than one data set or if the data is not on a
            // rectilinear grid or if the resampling has changed then
            // resample on to a single rectilinear grid.
            mustResample =
              (nsets > 1 ||
               datasetPtrs[ 0 ]->GetDataObjectType() != VTK_RECTILINEAR_GRID);
        }
    }

    if(mustResample)
      LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                << "rank: "  << PAR_Rank() << "  "
                << "must resample"
                << std::endl;

    // If one data set needs to resample the all will be resampled as
    // avtResampleFilter is a parallel call so do them regardless if
    // there is data or not. Otherwise MPI crashes.
    mustResample = UnifyMaximumValue(mustResample);

    // Check for a user resampling request, the same on all ranks.
    bool userResample =
      (m_renderingAttribs.resampleType &&
       m_renderingAttribs.resampleTargetVal != m_resampleTargetVal);

    if(mustResample || userResample)
    {
        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                  << "rank: "  << PAR_Rank() << "  resampling"
                  << std::endl;

        // Create a dummy pipeline within the device so to force an
        // execute within this "Execute".  Start with the source.
        avtSourceFromAVTDataset termsrc(GetTypedInput());

        if (resampleFilter != nullptr)
        {
          delete resampleFilter;
          resampleFilter = nullptr;
        }

        // Resample the data - must be done by all ranks.
        InternalResampleAttributes resampleAtts;

        // User requested resampling. If the type is 1 then resample
        // on to a single domain. Otherwise resample in parallel.
        if( userResample )
        {
            if( m_renderingAttribs.resampleType == 2 )
              resampleAtts.SetDistributedResample(true);
            else if( m_renderingAttribs.resampleType == 3 )
              resampleAtts.SetPerRankResample(true);
        }
        // Must resample but the user selected none so do a
        // distributed resample.
        else //if( mustResample )
        {
             resampleAtts.SetDistributedResample(true);
        }

        resampleAtts.SetTargetVal(m_renderingAttribs.resampleTargetVal);
        resampleAtts.SetPrefersPowersOfTwo(true);
        resampleAtts.SetUseTargetVal(true);

        avtResampleFilter *resampleFilter =
            new avtResampleFilter(&resampleAtts);

        resampleFilter->SetInput( termsrc.GetOutput() );
        dob = resampleFilter->GetOutput();
        dob->Update(GetGeneralContract());

        // Store the target value so if resampling is turned on
        // a change will trigger the resampling.
        m_resampleTargetVal = m_renderingAttribs.resampleTargetVal;
    }
    else
    {
        // Reset the stored target value so if resampling is turned on
        // it will happen.
        m_resampleTargetVal = 0;
    }

    // Store the resample type so to check for a state change.
    m_resampleType = m_renderingAttribs.resampleType;

    // Get the data tree which may be the origina data or from the
    // resampled data.
    inputTree = ((avtDataset*) *dob)->GetDataTree(); // avtDataTree_p

    nsets = 0;
    datasetPtrs = inputTree->GetAllLeaves(nsets);

    LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
              << "rank: "  << PAR_Rank() << " out "
              << "nsets: " << nsets << "  "
              << std::endl;

    // If no data skip but account for the parallel compositing.

    // NOTE: When resampling in parallel a rectilinear grid will be
    // created but it may not contain any data. As such, skip it if
    // the needImage flag is false as it is based on the original
    // input data. This step could also be done by examining the
    // rectilienar grid but the flag is already set so use it.
    if( needImage == false || nsets == 0 )
    {
        debug5 << "[VisItVTKDevice] Nothing to render, no data." << std::endl;

#ifdef PARALLEL
        // So the parallel case can still work
        avtImage_p finalImage =
          CreateFinalImage(nullptr, width, height, FLT_MAX);

        if(PAR_Rank() == 0)
          SetOutput(finalImage);
#endif

        return;
    }

    // After resampling there should be only one rectilinear data set.
    if( nsets != 1 )
    {
        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                  << "rank: "  << PAR_Rank() << " "
                  << "Too many datasets to render, skipping." << std::endl;
        EXCEPTION1(ImproperUseException, "Only one input dataset may be rendered.");
    }

    // Create a new image if needed.
    if( needImage )
    {
        // Now to the business
        vtkDataSet* in_ds = datasetPtrs[ 0 ];
        vtkRectilinearGrid* rgrid = vtkRectilinearGrid::SafeDownCast( in_ds );

        if( rgrid->GetDataObjectType() != VTK_RECTILINEAR_GRID )
        {
            EXCEPTION1(ImproperUseException,
                       "Only a vtkRectilinearGrid can be rendered.");
        }

        // Check for an implied transform.
        avtDataAttributes &inatts = GetInput()->GetInfo().GetAttributes();
        if (inatts.GetRectilinearGridHasTransform())
        {
            EXCEPTION1(ImproperUseException,
                       "vtkRectilinear grids with an implied transform can not be rendered.");
        }

        double bounds[6];
        rgrid->GetBounds(bounds);

        // The volume mapper requires a vtkImageData as input.
        int dims[3], extent[6];
        rgrid->GetDimensions(dims);
        rgrid->GetExtent(extent);

        // There could be both a scalar and opacity data arrays.
        vtkDataArray *dataArr = in_ds->GetPointData()->GetScalars();
        vtkDataArray *opacityArr = nullptr;
	// Might be useful - not sure yet???
        // vtkDataArray *gradientArr = nullptr;

        if( m_nComponents == 2 )
        {
            opacityArr = in_ds->GetPointData()->GetScalars( opacityVarName.c_str() );
            if( opacityArr == nullptr )
            {
                EXCEPTION1(InvalidVariableException, opacityVarName);
            }
        }

        // if( gradientVarName != "default" )
        //   gradientArr = in_ds->GetPointData()->GetVectors( gradientVarName.c_str() );

        // Get the spacing from the input grid.
        double spacingX = (rgrid->GetXCoordinates()->GetTuple1(1)-
                           rgrid->GetXCoordinates()->GetTuple1(0));
        double spacingY = (rgrid->GetYCoordinates()->GetTuple1(1)-
                           rgrid->GetYCoordinates()->GetTuple1(0));
        double spacingZ = (rgrid->GetZCoordinates()->GetTuple1(1)-
                           rgrid->GetZCoordinates()->GetTuple1(0));

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

        if( imageToRender != nullptr )
            imageToRender->Delete();

        imageToRender = vtkImageData::New();
        imageToRender->SetDimensions(dims);
        imageToRender->SetExtent(extent);
        imageToRender->SetSpacing(spacingX, spacingY, spacingZ);
        // The color and opacity data may be separate components which
        // requires the IndependentComponents in the
        // vtkVolumeProperties set to 'off'
        imageToRender->AllocateScalars(VTK_UNSIGNED_CHAR, m_nComponents);

        // Set the origin to match the lower bounds of the grid
        imageToRender->SetOrigin(bounds[0], bounds[2], bounds[4]);

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                  << "rank: "  << PAR_Rank() << " data range : "
                  << dataRange[0] << "  " << dataRange[1] << "  "
                  << std::endl;

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                  << "rank: "  << PAR_Rank() << " opacity range : "
                  << opacityRange[0] << "  " << opacityRange[1] << "  "
                  << std::endl;

        // The values on the image must be scales to between 0 and 255.
        double dataScale    = 255.0f / (   dataRange[1] -    dataRange[0]);
        double opacityScale = 255.0f / (opacityRange[1] - opacityRange[0]);

        // Transfer the rgrid data to the image data
        // and scale to the proper range.
        double data_min =  1e6;
        double data_max = -1e6;

        double opacity_min =  1e6;
        double opacity_max = -1e6;

        // VisIt populates empty space with the NO_DATA_VALUE.
        // This needs to map this to a value that the mapper accepts,
        // and then clamp it so to be fully translucent.
        int ptId = 0;
        for (int z = 0; z < dims[2]; ++z)
        {
            for (int y = 0; y < dims[1]; ++y)
            {
                for (int x = 0; x < dims[0]; ++x)
                {
                    // The color and opacity data may differ so add
                    // both as two separate components.
                    double dataTuple = dataArr->GetTuple1(ptId);
                    if (dataTuple <= NO_DATA_VALUE)
                    {
                        // The color map is 0 -> 255. For no data values,
                        // assign a new value just out side of the map.
                        imageToRender->SetScalarComponentFromDouble(x, y, z, 0, -1.0);
                        m_useInterpolation = false;
                    }
                    else
                    {
                        double val = (dataTuple - dataRange[0]) * dataScale;

                        if( val < 0   ) val = 0;
                        if( val > 255 ) val = 255;

                        imageToRender->SetScalarComponentFromDouble(x, y, z, 0, val);
                        if( data_min > val ) data_min = val;
                        if( data_max < val ) data_max = val;
                    }

                    if( m_nComponents == 2 )
                    {
                        double opacityTuple = opacityArr->GetTuple1(ptId);
                        if (opacityTuple <= NO_DATA_VALUE)
                        {
                            // The opacity map is 0 -> 255. For no data values,
                            // assign a new value just out side of the map.
                            imageToRender->SetScalarComponentFromDouble(x, y, z, 1, -1.0);
                            m_useInterpolation = false;
                        }
                        else
                        {
                            double val = (opacityTuple - opacityRange[0]) * opacityScale;
                            if( val < 0   ) val = 0;
                            if( val > 255 ) val = 255;

                            imageToRender->SetScalarComponentFromDouble(x, y, z, 1, val);
                            if( opacity_min > val ) opacity_min = val;
                            if( opacity_max < val ) opacity_max = val;
                        }
                    }

                    // double gradientTuple = gradientArr->GetTuple1(ptId);
                    // if (opacityTuple <= NO_DATA_VALUE)
                    // {
                    //     // The opacity map is 0 -> 255. For no data values,
                    //     // assign a new value just out side of the map.
                    //     imageToRender->SetScalarComponentFromDouble(x, y, z, 1, -1.0);
                    //     m_useInterpolation = false;
                    // }
                    // else
                    // {
                    //     imageToRender->SetScalarComponentFromDouble(x, y, z, 1, val);
                    // }

                    ptId++;
                }
            }
        }

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                  << "rank: "  << PAR_Rank()
                  << " dataRange: "
                  << data_min << "  " << data_max << "  "
                  << " opacityRange: "
                  << opacity_min << "  " << opacity_max << "  "
                  << std::endl;

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                  << "rank: "  << PAR_Rank() << " useInterpolation: "
                  << m_useInterpolation << "  "
                  << std::endl;

        // For debugging
        // {
        //     vtkXMLImageDataWriter* writer = vtkXMLImageDataWriter::New();

        //     writer->SetInputData(imageToRender);
        //     if( m_nComponents == 2 )
        //       writer->SetFileName("Image_Large_2_Comps.vti");
        //     else
        //     writer->SetFileName("Image_Large_1_Comps.vti");

        //     writer->Write();
        //     writer->Delete();
        // }
    }

    // Create a new volume mapper if needed.
    if( volumeMapper == nullptr ||
        m_OSPRayEnabled != m_renderingAttribs.OSPRayEnabled)
    {
        m_OSPRayEnabled = m_renderingAttribs.OSPRayEnabled;

        if (volumeMapper != nullptr)
            volumeMapper->Delete();

        // Create the volume mapper.
#ifdef HAVE_OSPRAY
        if( m_renderingAttribs.OSPRayEnabled )
        {
            vtkOSPRayVolumeMapper * vm = vtkOSPRayVolumeMapper::New();
            volumeMapper = vm;
        }
        else
#endif
        {
            vtkGPUVolumeRayCastMapper * vm = vtkGPUVolumeRayCastMapper::New();
            volumeMapper = vm;
        }

        volumeMapper->SetInputData(imageToRender);
        volumeMapper->SetScalarModeToUsePointData();
        volumeMapper->SetBlendModeToComposite();
    }

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
    // gradient->SetClamping(useInterpolation==true);

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

    // Set ambient, diffuse, specular, and specular power (shininess).
    volumeProperty->SetAmbient      (m_materialPropertiesPtr[0]);
    volumeProperty->SetDiffuse      (m_materialPropertiesPtr[1]);
    volumeProperty->SetSpecular     (m_materialPropertiesPtr[2]);
    volumeProperty->SetSpecularPower(m_materialPropertiesPtr[3]);

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
    imageToRender->GetSpacing(spacing);
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
    volume->SetMapper(volumeMapper);
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
    renderer->SetLightCollection( lights );

#ifdef HAVE_OSPRAY
    if( m_renderingAttribs.OSPRayEnabled )
    {
        vtkOSPRayRendererNode::SetRendererType("pathtracer", renderer);
        vtkOSPRayRendererNode::SetSamplesPerPixel(m_renderingAttribs.samplesPerPixel, renderer);
        vtkOSPRayRendererNode::SetAmbientSamples (m_renderingAttribs.aoSamples,       renderer);
        vtkOSPRayRendererNode::SetMinContribution(m_renderingAttribs.minContribution, renderer);
        vtkOSPRayRendererNode::SetMaxContribution(m_renderingAttribs.maxContribution, renderer);
    }
#endif

    if (ambientOn)
    {
        renderer->SetAmbient(ambientColor);
    }
    else
    {
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
            debug5 << __LINE__ << " [VisItVTKDevice] "
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
    imageToRender->GetBounds(bounds);
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
    // delete [] renderedZBuffer;

    transFunc->Delete();
    opacity->Delete();
    volumeProperty->Delete();
    volume->Delete();
    camera->Delete();
    lights->Delete();
    renderer->Delete();
    renderWin->Delete();
}
