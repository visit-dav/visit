// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtVisItVTKOSPRayDevice.C                           //
// ************************************************************************* //

#include <avtVisItVTKOSPRayDevice.h>
#include <avtParallel.h>
#include <avtVisItVTKCompositor.h>

#include <DebugStream.h>
#include <TimingsManager.h>
#include <StackTimer.h>
#include <ImproperUseException.h>

#include <vtkImageData.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkMatrix4x4.h>
#include <vtkDataObject.h>
#include <vtkRectilinearGrid.h>
#include <vtkLight.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkVolume.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkPolydataMapper.h>
#include <vtkDataSetMapper.h>
#include <vtkOSPRayVolumeMapper.h>
#include <vtkVolumeProperty.h>

#include <vtkDataSetSurfaceFilter.h>
#include <vtkGeometryFilter.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>

#include <avtWorldSpaceToImageSpaceTransform.h>
#include <avtResampleFilter.h>

#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

#include <vector>

const std::string avtVisItVTKOSPRayDevice::DEVICE_TYPE_STR{"ospray"};

// ****************************************************************************
//  Method: avtVisItVTKOSPRayDevice constructor
//
//  Programmer: Kevin Griffin
//  Creation:   March 4, 2021
//
//  Modifications:
//
// ****************************************************************************

avtVisItVTKOSPRayDevice::avtVisItVTKOSPRayDevice() : avtRayTracerBase(),
    m_dataType(DataType::GEOMETRY),
    m_activeVariablePtr(nullptr),
    m_lightList(),
    m_renderingAttribs(),
    m_materialPropertiesPtr(nullptr),
    m_viewDirectionPtr(nullptr)
{}

// ****************************************************************************
//  Method: avtVisItVTKOSPRayDevice::SetMatProperties
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
//  Programmer: Kevin Griffin
//  Creation:   February 22, 2021
//
// ****************************************************************************

void
avtVisItVTKOSPRayDevice::SetMatProperties(const double props[4])
{
    if(!m_materialPropertiesPtr)
        m_materialPropertiesPtr.reset(new float[4]);

    for(int i=0; i<4; i++)
        m_materialPropertiesPtr[i] = static_cast<float>(props[i]);
}

// ****************************************************************************
//  Method: avtVisItVTKOSPRayDevice::SetViewDirection
//
//  Purpose:
//      Set the camera view direction.
//
//  Arguments:
//      direction   view direction
//
//  Programmer: Kevin Griffin
//  Creation:   February 22, 2021
//
// ****************************************************************************

void
avtVisItVTKOSPRayDevice::SetViewDirection(const double direction[3])
{
    if(!m_viewDirectionPtr)
        m_viewDirectionPtr.reset(new float[3]);

    for(int i=0; i<3; i++)
        m_viewDirectionPtr[i] = static_cast<float>(direction[i]);
}

// ****************************************************************************
//  Method: avtVisItVTKOSPRayDevice::ErrorCallback
//
//  Purpose:
//      VisItVTK device error callback function.
//
//  Arguments:
//      userData    optional user data for the callback
//      err         VisItVTK errors which can be of the following type:
//                  VisItVTK_NO_ERROR          0 - No error has been recorded
//                  VisItVTK_UNKNOWN_ERROR     1 - An unknown error has occurred
//                  VisItVTK_INVALID_ARGUMENT  2 - An invalid argument is specified
//                  VisItVTK_INVALID_OPERATION 3 - The operation is not allowed for the
//                                              specified object
//                  VisItVTK_OUT_OF_MEMORY     4 - There is not enough memory left to
//                                              execute the command
//                  VisItVTK_UNSUPPORTED_CPU   5 - The CPU is not supported as it does
//                                              not support SSE4.1
//                  VisItVTK_VERSION_MISMATCH  6 - A module could not be loaded due to
//                                              mismatching version
//      details     error message
//
//  Programmer: Kevin Griffin
//  Creation:   March 8, 2021
//
// ****************************************************************************

void
avtVisItVTKOSPRayDevice::ErrorCallback(void *userData, vtkErrorCode::ErrorIds err, const char *details)
{
    debug5 << "[VisItVTK::OSPRAY] " << avtVisItVTKDevice::GetVisItVTKErrorString(err) << ": " << details << std::endl;
}

// ****************************************************************************
//  Method: avtVisItVTKOSPRayDevice::StatusCallback
//
//  Purpose:
//      VisItVTK device status callback function.
//
//  Arguments:
//      userData optional user data for the callback
//      details  status message
//
//  Programmer: Kevin Griffin
//  Creation:   March 8, 2021
//
// ****************************************************************************

void
avtVisItVTKOSPRayDevice::StatusCallback(void *userData, const char *details)
{
    debug5 << "[VisItVTK::OSPRAY] Status: " << details << std::endl;
}

// ****************************************************************************
//  Method: avtVisItVTKOSPRayDevice::CreateCamera
//
//  Purpose:
//      The perspective camera implements a simple thin lens camera for
//      perspective rendering, supporting optionally depth of field and stereo
//      rendering, but not motion blur.
//
//      The orthographic camera implements a simple camera with orthographic
//      projection, without support for depth of field or motion blur.
//
//  Programmer: Kevin Griffin
//  Creation:   March 8, 2021
//
// ****************************************************************************

vtkCamera *
avtVisItVTKOSPRayDevice::CreateCamera()
{
    // float aspect = 1.0f;
    // int imgWidth  = screen[0];
    // int imgHeight = screen[1];

    // if(imgHeight > 0)
    // {
    //     aspect = static_cast<float>(imgWidth) / static_cast<float>(imgHeight);
    // }

    // double cameraPosition[3] = {static_cast<float>(viewInfo.camera[0]),
    //                             static_cast<float>(viewInfo.camera[1]),
    //                             static_cast<float>(viewInfo.camera[2])};

    // double cameraUp[3] = {static_cast<float>(viewInfo.viewUp[0]),
    //                       static_cast<float>(viewInfo.viewUp[1]),
    //                       static_cast<float>(viewInfo.viewUp[2])};

    // double cameraDirection[3];

    // if(m_viewDirectionPtr)
    // {
    //     cameraDirection[0] = m_viewDirectionPtr[0];
    //     cameraDirection[1] = m_viewDirectionPtr[1];
    //     cameraDirection[2] = m_viewDirectionPtr[2];
    // }
    // else
    // {
    //     double viewDirection[3];
    //     viewDirection[0] = viewInfo.focus[0] - viewInfo.camera[0];
    //     viewDirection[1] = viewInfo.focus[1] - viewInfo.camera[1];
    //     viewDirection[2] = viewInfo.focus[2] - viewInfo.camera[2];
    //     double mag = sqrt(viewDirection[0]*viewDirection[0] +
    //                       viewDirection[1]*viewDirection[1] +
    //                       viewDirection[2]*viewDirection[2]);
    //     if (mag != 0) // only 0 if focus and camera are the same
    //     {
    //         viewDirection[0] /= mag;
    //         viewDirection[1] /= mag;
    //         viewDirection[2] /= mag;
    //     }

    //     cameraDirection[0] = static_cast<float>(viewDirection[0]);
    //     cameraDirection[1] = static_cast<float>(viewDirection[1]);
    //     cameraDirection[2] = static_cast<float>(viewDirection[2]);
    // }

    // vtkCamera *camera = vtkCamera::New();
    // camera->SetPosition  ( cameraPosition);
    // camera->SetFocalPoint(-cameraDirection);
    // camera->SetViewUp    (cameraUp);
    // camera->SetViewAngle(viewInfo.viewAngle); ///viewInfo.ImageZoom
    // camera->SetClippingRange(viewInfo.nearPlane, viewInfo.farPlane);

    // if (viewInfo.orthographic)
    //     camera->ParallelProjectionOn();
    // else
    //     camera->ParallelProjectionOff();

    // camera->SetParallelScale(viewInfo.parallelScale);

    vtkCamera *camera = vtkCamera::New();
    viewInfo.SetCameraFromView( camera );

    return camera;
}

// ****************************************************************************
//  Method: avtVisItVTKOSPRayDevice::CreateLights
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
//  Programmer: Kevin Griffin
//  Creation:   March 8, 2021
//
// ****************************************************************************

vtkLightCollection *
avtVisItVTKOSPRayDevice::CreateLights()
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

                //
                // This next line is a work-around for a known VTK bug
                // when switching to scene light from another light type.
                // Can be removed when we switch to new VTK dated after
                // 6Sep02.
                //
                light->SetTransformMatrix(NULL);

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
        // FIXME
        // canvas->SetAmbient(ambientColor);
    }
    else
    {
        // FIXME
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
//  Method: avtVisItVTKOSPRayDevice::CreateFinalImage
//
//  Purpose:
//      If serial, copy the VisItVTK rendered frame into an avtImage to pass back
//      to VisIt. If parallel, composite image with the other processors then
//      pass the composited image back to VisIt.
//
//  Arguments:
//      colorBuffer the VisItVTK rendered frame
//      width       frame width
//      height      frame height
//      zDepth      the average z-depth of this rendered frame
//
//  Programmer: Kevin Griffin
//  Creation:   March 8, 2021
//
// ****************************************************************************

avtImage_p
avtVisItVTKOSPRayDevice::CreateFinalImage(const void *colorBuffer,
                                          const int width,
                                          const int height,
                                          const float zDepth)
{
    const int nColorChannels = 4;
    avtImage_p finalImage = new avtImage(this);
    vtkImageData *finalImageData =
        avtImageRepresentation::NewImage(width, height, nColorChannels);

    finalImage->GetImage().SetImageVTK(finalImageData);
    // finalImage->GetImage() = imageData;

    std::unique_ptr<avtVisItVTKCompositor> compositor(new avtVisItVTKCompositor(zDepth));
    compositor->Composite(colorBuffer,
                          finalImage->GetImage().GetRGBBuffer(),
                          width,
                          height,
                          nColorChannels);

    {
      // vtkImageWriter* writer = vtkImageWriter::New();
      vtkPNGWriter* writer = vtkPNGWriter::New();

      writer->SetInputData(finalImageData);
      writer->SetFileName("finialImage.png");
      writer->Write();
      writer->Delete();
    }
    
    finalImageData->Delete();

    return finalImage;
}

// ****************************************************************************
//  Method: avtVisItVTKOSPRayDevice::Execute
//
//  Purpose:
//      Executes the OSPRay Rendering backend.
//      This means:
//      - Put the input mesh through a transform so it is in camera space.
//      - Get the sample points.
//      - Communicate the sample points (parallel only).
//      - Composite the sample points along rays.
//      - Communicate the pixels from each ray (parallel only).
//      - Output the image.
//
//  Programmer: Kevin Griffin
//  Creation:   March 8, 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtVisItVTKOSPRayDevice::Execute()
{
    StackTimer t0("VisItVTK::OSPRay Rendering");

    const char *moduleName = GetDeviceType();

    if(m_dataType == DataType::VOLUME)
        ExecuteVolume();
    else
        ; // TODO: Implement ExecuteSurface(); // Done as PseudoColotPlot
}

// ****************************************************************************
//  Method: avtVisItVTKOSPRayDevice::ExecuteVolume
//
//  Purpose:
//      Executes Volume Rendering using the OSPRay Rendering backend.
//
//
//  Programmer: Kevin Griffin
//  Creation:   March 8, 2021
//
//  Modifications:
//
// ****************************************************************************
#define NO_DATA_VALUE -1e+37

void
avtVisItVTKOSPRayDevice::ExecuteVolume()
{
    const int width  = screen[0];
    const int height = screen[1];

    auto inputTree = GetInputDataTree(); // avtDataTree_p
    int nsets = 0;
    vtkDataSet **datasetPtrs = inputTree->GetAllLeaves(nsets);
    debug5 << "[VisItVTK::OSPRAY] nsets: " << nsets << std::endl;

    std::cerr << __LINE__ << " [VisItVTK::OSPRAY] "
              << "rank: "  << PAR_Rank() << "  "
              << "nsets: " << nsets << "  "
              << std::endl;

    // There should only be one data set. If more than one it should be
    // resampled upstream in avtVolumePlot::ApplyRenderingTransformation.
    if(nsets == 1)
    {
        vtkDataSet* in_ds = datasetPtrs[ 0 ];
        vtkRectilinearGrid* rgrid = vtkRectilinearGrid::SafeDownCast( in_ds );

        if( rgrid->GetDataObjectType() != VTK_RECTILINEAR_GRID )
        {
            std::cerr << datasetPtrs[ 0 ]->GetDataObjectType() << std::endl;

            EXCEPTION1(ImproperUseException,
                       "Only vtkRectilinearGrid may be rendered.");
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

        double spacingX = (rgrid->GetXCoordinates()->GetTuple1(1)-
                           rgrid->GetXCoordinates()->GetTuple1(0));
        double spacingY = (rgrid->GetYCoordinates()->GetTuple1(1)-
                           rgrid->GetYCoordinates()->GetTuple1(0));
        double spacingZ = (rgrid->GetZCoordinates()->GetTuple1(1)-
                           rgrid->GetZCoordinates()->GetTuple1(0));

        std::cerr << __LINE__ << " [VisItVTK::OSPRAY] dims : "
                  << dims[0] << "  " << dims[1] << "  "<< dims[2] << "  "
                  << std::endl;

        std::cerr << __LINE__ << " [VisItVTK::OSPRAY] extent : "
                  << extent[0] << "  " << extent[1] << "  "
                  << extent[2] << "  " << extent[3] << "  "
                  << extent[4] << "  " << extent[5] << "  "
                  << std::endl;

        std::cerr << __LINE__ << " [VisItVTK::OSPRAY] bounds : "
                  << bounds[0] << "  " << bounds[1] << "  "
                  << bounds[2] << "  " << bounds[3] << "  "
                  << bounds[4] << "  " << bounds[5] << "  "
                  << std::endl;

        std::cerr << __LINE__ << " [VisItVTK::OSPRAY] spacing : "
                  << spacingX << "  " << spacingX << "  "<< spacingX << "  "
                  << std::endl;

        vtkImageData* imageToRender = vtkImageData::New();
        imageToRender->SetDimensions(dims);
        imageToRender->SetExtent(extent);
        imageToRender->SetSpacing(spacingX, spacingY, spacingZ);
        imageToRender->AllocateScalars(VTK_FLOAT, 2);

        // Set the origin to match the lower bounds of the grid
        imageToRender->SetOrigin(bounds[0], bounds[2], bounds[4]);

        // There could be both a scalar and opacity data arrays. So get both.
        vtkDataArray *dataArr = in_ds->GetPointData()->GetScalars();
	// FIXME - need to secondary variable.
        vtkDataArray *opacArr = in_ds->GetPointData()->GetScalars();

        double dataRange[2] = {0., 0.};
        double opacityRange[2] = {0., 0.};
        in_ds->GetScalarRange( dataRange );
        in_ds->GetScalarRange( opacityRange );

        std::cerr << __LINE__ << " [VisItVTK::OSPRAY] data range : "
                  << dataRange[0] << "  "
                  << dataRange[1] << "  "
                  << std::endl;

        std::cerr << __LINE__ << " [VisItVTK::OSPRAY] opacity range : "
                  << opacityRange[0] << "  "
                  << opacityRange[1] << "  "
                  << std::endl;

        // double dataMag = volume.data.max - volume.data.min;
        // double opacMag = volume.opacity.max - volume.opacity.min;

        double dataScale    = 255.0 / (   dataRange[1] -    dataRange[0]);
        double opacityScale = 255.0 / (opacityRange[1] - opacityRange[0]);

        // Transfer the rgrid data to the image data
        // and scale to the proper range.
        bool useInterpolation = true;

        // VisIt populates empty space with the NO_DATA_VALUE.
        // We need to map this to a value that our mapper accepts,
        // and then clamp it out of vision.
        int ptId = 0;
        for (int z = 0; z < dims[2]; ++z)
        {
            for (int y = 0; y < dims[1]; ++y)
            {
                for (int x = 0; x < dims[0]; ++x)
                {
                    // The opacity and color data may differ so add
                    // both as two separate components.
                    double dataTuple = dataArr->GetTuple1(ptId);
                    if (dataTuple <= NO_DATA_VALUE)
                    {
                        // The color map is 0 -> 255. For no data values,
                        // assign a new value just out side of the map.
                        imageToRender->SetScalarComponentFromDouble(x, y, z, 0, -1.0);
                        useInterpolation = false;
                    }
                    else
                    {
                        double val = (dataTuple - dataRange[0]) * dataScale;
                        imageToRender->SetScalarComponentFromDouble(x, y, z, 0, val);
                    }

                    double opacityTuple = opacArr->GetTuple1(ptId);
                    if (opacityTuple <= NO_DATA_VALUE)
                    {
                        // The color map is 0 -> 255. For no data values,
                        // assign a new value just out side of the map.
                        imageToRender->SetScalarComponentFromDouble(x, y, z, 1, -1.0);
                        useInterpolation = false;
                    }
                    else
                    {
                        double val = (opacityTuple - opacityRange[0]) * opacityScale;
                        imageToRender->SetScalarComponentFromDouble(x, y, z, 1, val);
                    }

                    ptId++;
                }
            }
        }

        std::cerr << __LINE__ << " [VisItVTK::OSPRAY] useInterpolation: "
                  << useInterpolation << "  "
                  << std::endl;
	
        // vtkDataSetSurfaceFilter* dssFilter = vtkDataSetSurfaceFilter::New();
        vtkGeometryFilter* dssFilter = vtkGeometryFilter::New();
        dssFilter->SetInputData( imageToRender );

        // Create the data mapper.
        vtkPolyDataMapper* dssMapper = vtkPolyDataMapper::New();
        dssMapper->SetInputData(dssFilter->GetOutput());

        // Create the actor
        vtkActor* dssActor = vtkActor::New();
        dssActor->SetMapper(dssMapper);

        // Create the volume mapper.
        // vtkOSPRayVolumeMapper* volumeMapper = vtkOSPRayVolumeMapper::New();
        vtkSmartVolumeMapper* volumeMapper = vtkSmartVolumeMapper::New();
        volumeMapper->SetInputData(imageToRender);
        volumeMapper->SetScalarModeToUsePointData();
        volumeMapper->SetBlendModeToComposite();

        double scalarRange[2];
        volumeMapper->GetInput()->GetScalarRange(scalarRange);

        std::cerr << __LINE__ << " [VisItVTK::OSPRAY] scalarRange: "
                  << scalarRange[0] << "  "
                  << scalarRange[1] << "  "
                  << std::endl;

        // Create the transfer function and the opacity mapping.
        const RGBAF *transferTable = transferFn1D->GetTableFloat();
        int tableSize = transferFn1D->GetNumberOfTableEntries();
        double range = (scalarRange[1] - scalarRange[0]) / double(tableSize-1);

        vtkColorTransferFunction* transFunc = vtkColorTransferFunction::New();
        vtkPiecewiseFunction*     opacity   = vtkPiecewiseFunction::New();

        for(int i=0; i<tableSize; i++)
        {
            // double val = scalarRange[0] + range * double(i);
            // transFunc->AddRGBPoint( val,
            transFunc->AddRGBPoint( i,
                                    transferTable[i].R,
                                    transferTable[i].G,
                                    transferTable[i].B );
            opacity->AddPoint( i, transferTable[i].A );

	    if( i%8 == 0 )
	      std::cerr << transferTable[i].A << "  ";
        }

	std::cerr << std::endl;
	
        transFunc->SetScaleToLinear();
        transFunc->SetClamping(false);
        opacity->SetClamping(false);

        // For some reason, the endpoints aren't included when
        // clamping is turned off. So, add some padding on the ends of
        // our mapping functions.
        transFunc->AddRGBPoint( -1.0,
                                transferTable[0].R,
                                transferTable[0].G,
                                transferTable[0].B);
        opacity->AddPoint( -1.0, transferTable[0].A );

        transFunc->AddRGBPoint( tableSize,
                                transferTable[tableSize-1].R,
                                transferTable[tableSize-1].G,
                                transferTable[tableSize-1].B);
        opacity->AddPoint( tableSize, transferTable[tableSize-1].A );

        std::cerr << __LINE__ << " [VisItVTK::OSPRAY] RGBA: " << tableSize << "  "
                  << transferTable[64].R << "  "
                  << transferTable[64].G << "  "
                  << transferTable[64].B << "  "
                  << transferTable[64].A << "  "
                  << std::endl;

        std::cerr << __LINE__ << " [VisItVTK::OSPRAY] RGBA: " << tableSize << "  "
                  << transferTable[128].R << "  "
                  << transferTable[128].G << "  "
                  << transferTable[128].B << "  "
                  << transferTable[128].A << "  "
                  << std::endl;

        // Set the volume properties.
        vtkVolumeProperty * volumeProperty = vtkVolumeProperty::New();
        volumeProperty->SetColor(transFunc);
        volumeProperty->SetScalarOpacity(opacity);
        volumeProperty->IndependentComponentsOn();

        volumeProperty->SetShade( m_renderingAttribs.shadowsEnabled );

        // Set ambient, diffuse, specular, and specular power (shininess).
        volumeProperty->SetAmbient      (m_materialPropertiesPtr[0]);
        volumeProperty->SetDiffuse      (m_materialPropertiesPtr[1]);
        volumeProperty->SetSpecular     (m_materialPropertiesPtr[2]);
        volumeProperty->SetSpecularPower(m_materialPropertiesPtr[3]);

        // If the dataset contains NO_DATA_VALUEs, interpolation will
        // not work correctly on the boundaries (between a real value
        // and a no data value). Hopefully this will be addressed in the
        // future. For now, only interpolate when our dataset contains
        // none of these values.
        if (useInterpolation)
        {
            volumeProperty->SetInterpolationTypeToLinear();
        }
        else
        {
            volumeProperty->SetInterpolationTypeToNearest();
        }

        vtkVolume * volume = vtkVolume::New();
        volume->SetMapper(volumeMapper);
        volume->SetProperty(volumeProperty);

        // Create camera
        vtkCamera* camera = CreateCamera();

        // Create lights
        vtkLightCollection* lights = CreateLights();

        // Create the renderer
        vtkRenderer* renderer = vtkRenderer::New();
        renderer->AddViewProp(volume);
        renderer->AddActor(dssActor);
        renderer->SetActiveCamera( camera );
        renderer->SetLightCollection( lights );

        if (ambientOn)
        {
            renderer->SetAmbient(ambientColor);
        }
        else
        {
            renderer->SetAmbient(1., 1., 1.);
        }

        vtkRenderWindow* renderWin = vtkRenderWindow::New();
        renderWin->SetSize(width, height);
        renderWin->SetMultiSamples(false);
        renderWin->AddRenderer(renderer);
        renderWin->SetOffScreenRendering(true);

        camera->Render( renderer );

        renderWin->Render();

        unsigned char * renderedFrameBuffer =
          renderWin->GetRGBACharPixelData( 0, 0, width-1, height-1, 1 );

        // Create final image
        float zVal = std::abs(bounds[4] + ((bounds[5] - bounds[4]) / 2.0f));
        avtImage_p finalImage =
            CreateFinalImage(renderedFrameBuffer, width, height, zVal);

        if(PAR_Rank() == 0)
            SetOutput(finalImage);

        dssMapper->Delete();
        dssActor->Delete();
        volumeMapper->Delete();
        transFunc->Delete();
        opacity->Delete();
        volumeProperty->Delete();
        volume->Delete();
        camera->Delete();
        lights->Delete();
        renderer->Delete();
        renderWin->Delete();
    }
    else if( nsets == 0 )
    {
        debug5 << "[VisItVTK::OSPRay] Nothing to render, no data." << std::endl;

        #ifdef PARALLEL
            // So the parallel case can still work
            avtImage_p finalImage = CreateFinalImage(nullptr, width, height, FLT_MAX);

            if(PAR_Rank() == 0)
                SetOutput(finalImage);
        #endif
    }
    else
    {
        EXCEPTION1(ImproperUseException, "Only one input dataset may be rendered.");
    }
}
