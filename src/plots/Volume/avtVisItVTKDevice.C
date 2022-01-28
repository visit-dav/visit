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

#define LOCAL_DEBUG std::cerr
// #define LOCAL_DEBUG debug5


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
    m_lightList()
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
//  Method:  avtVisItVTKDevice::SetAtts
//
//  Purpose:
//    Set the attributes
//
//  Arguments:
//    a       : the new attributes
//
//  Programmer: Allen R. Sanderson
//  Creation:  30 November 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtVisItVTKDevice::SetAtts(const AttributeGroup *a)
{
    const VolumeAttributes *newAtts = (const VolumeAttributes*)a;

    if (*newAtts == m_atts)
        return;

    m_atts = *(const VolumeAttributes*) a;
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
#ifndef HAVE_OSPRAY
    if( m_atts.GetOSPRayEnabledFlag(() )
    {
        avtCallback::IssueWarning("Trying to use OSPRay when VTK was not built with OSPRay support. The default VTK renderering will be used.");
    }
#endif

    const int width  = screen[0];
    const int height = screen[1];

    // Get the input data.
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

    // There could be separate scalar and opacity components.
    if( opacityVarName == activeVarName )
    {
        if(m_atts.GetUseOpacityVarMin() ||
           m_atts.GetUseOpacityVarMax())
            avtCallback::IssueWarning("The opacity variable is the same as "
                                      "the primary variable. Ignoring any "
                                      "min/max setting.");
        m_nComponents = 1;
    }
    else if( opacityVarName != "default" )
    {
        m_nComponents = 2;
        GetDataExtents(opacityRange, opacityVarName.c_str());
        UnifyMinMax(opacityRange, 2);
    }
    else
    {
        m_nComponents = 1;
    }

    // If no data then no image will be generated on this rank but
    // there is parallel compositing so account for it.
    if( nsets == 0 )
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

    // There should be only one data set.
    if( nsets > 1 )
    {
        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                    << "rank: "  << PAR_Rank() << " "
                    << "Too many datasets to render, skipping." << std::endl;
        EXCEPTION1(ImproperUseException, "Only one input dataset may be rendered.");
    }

    // There should be only be a rectilinear data set.
    vtkDataSet* in_ds = datasetPtrs[ 0 ];

    if( in_ds->GetDataObjectType() != VTK_RECTILINEAR_GRID )
    {
        EXCEPTION1(ImproperUseException,
                   "Only a vtkRectilinearGrid can be rendered.");
    }

    avtDataAttributes &inatts = GetInput()->GetInfo().GetAttributes();
    if (inatts.GetRectilinearGridHasTransform())
    {
        EXCEPTION1(ImproperUseException,
                   "vtkRectilinear grids with an implied transform can not be rendered.");
    }

    LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
              << "rank: "  << PAR_Rank() << " in  "
              << "nsets: " << nsets << "  "
              << "nComponents: " << m_nComponents << "  "
              << std::endl;

    // Create a new image if needed.
    if( m_imageToRender == nullptr ||

        // Color variable change or min/max change. The active var
        // name triggers needing an image on the first pass.
        m_activeVarName != activeVarName ||
        m_useColorVarMin != m_atts.GetUseColorVarMin() ||
        (m_atts.GetUseColorVarMin() &&
         m_colorVarMin != m_atts.GetColorVarMin()) ||
        m_useColorVarMax != m_atts.GetUseColorVarMax() ||
        (m_atts.GetUseColorVarMax() &&
         m_colorVarMax != m_atts.GetColorVarMax()) ||

        // Opacity variable change or min/max change.
        (m_nComponents == 2 &&
         (m_opacityVarName != opacityVarName ||
          m_useOpacityVarMin != m_atts.GetUseOpacityVarMin() ||
          (m_atts.GetUseOpacityVarMin() &&
           m_opacityVarMin != m_atts.GetOpacityVarMin()) ||
          m_useOpacityVarMax != m_atts.GetUseOpacityVarMax() ||
          (m_atts.GetUseOpacityVarMax() &&
           m_opacityVarMax != m_atts.GetOpacityVarMax()))) )
    {
        // Store the color variable values so to check for a state change.
        m_activeVarName  = activeVarName;
        m_useColorVarMin = m_atts.GetUseColorVarMin();
        m_colorVarMin    = m_atts.GetColorVarMin();
        m_useColorVarMax = m_atts.GetUseColorVarMax();
        m_colorVarMax    = m_atts.GetColorVarMax();

        // Store the opacity variable values so to check for a state change.
        m_opacityVarName   = opacityVarName;
        m_useOpacityVarMin = m_atts.GetUseOpacityVarMin();
        m_opacityVarMin    = m_atts.GetOpacityVarMin();
        m_useOpacityVarMax = m_atts.GetUseOpacityVarMax();
        m_opacityVarMax    = m_atts.GetOpacityVarMax();

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                    << "rank: "  << PAR_Rank() << " "
                    << "Converting from rectilinear grid to image data."
                    << std::endl;

        // Now to the business
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

        // If needed adjust the colar var range
        if( m_atts.GetUseColorVarMin() )
            dataRange[0] = m_atts.GetColorVarMin();
        if( m_atts.GetUseColorVarMax() )
            dataRange[1] = m_atts.GetColorVarMax();

        // If needed adjust the opacity var range
        if( m_nComponents == 2 )
        {
            if( m_atts.GetUseOpacityVarMin() )
                opacityRange[0] = m_atts.GetOpacityVarMin();
            if( m_atts.GetUseOpacityVarMax() )
                opacityRange[1] = m_atts.GetOpacityVarMax();
        }

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
        m_OSPRayEnabled != m_atts.GetOSPRayEnabledFlag())
    {
        m_OSPRayEnabled = m_atts.GetOSPRayEnabledFlag();

        if (m_volumeMapper != nullptr)
            m_volumeMapper->Delete();

        // Create the volume mapper.
#ifdef HAVE_OSPRAY
        if( m_atts.GetOSPRayEnabledFlag() )
        {
            m_volumeMapper = vtkOSPRayVolumeMapper::New();

            LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                        << "rank: "  << PAR_Rank() << " OSPRay Volume Mapper "
                        << std::endl;
        }
        else
#endif
        {
            m_volumeMapper = vtkGPUVolumeRayCastMapper::New();
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

    // Upstream in avtVolumeFilter a new color and opacity map are
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
    volumeProperty->SetShade( m_atts.GetLightingFlag() );

    LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
              << "rank: "  << PAR_Rank() << " HasGradientOpacity: "
              << volumeProperty->HasGradientOpacity() << "  "
              << std::endl;

    // Set ambient, diffuse, specular, and specular power (shininess).
    const double *matProp = m_atts.GetMaterialProperties();

    if (m_atts.GetLightingFlag() && matProp != nullptr)
    {
        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                    << "rank: "  << PAR_Rank()
                    << " lightingEnabled: " << m_atts.GetLightingFlag() << "  "
                    << matProp[0] << "  "
                    << matProp[1] << "  "
                    << matProp[2] << "  "
                    << matProp[3] << "  "
                    << std::endl;

        volumeProperty->SetAmbient(matProp[0]);
        volumeProperty->SetDiffuse(matProp[1]);
        volumeProperty->SetSpecular(matProp[2]);
        volumeProperty->SetSpecularPower(matProp[3]);
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
    if( m_atts.GetOSPRayEnabledFlag() )
    {
        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                    << "rank: "  << PAR_Rank() << " RenderType: "
                    << (m_atts.GetOSPRayRenderType() ? "PathTracer" : "SciVis") << "  "
                    << std::endl;

        if( m_atts.GetOSPRayRenderType() == 1 )
            vtkOSPRayRendererNode::SetRendererType("pathtracer", renderer);
        else
            vtkOSPRayRendererNode::SetRendererType("scivis", renderer);

        vtkOSPRayRendererNode::SetSamplesPerPixel(m_atts.GetOSPRaySPP(), renderer);
        vtkOSPRayRendererNode::SetAmbientSamples (m_atts.GetOSPRayAOSamples(),       renderer);
        vtkOSPRayRendererNode::SetMinContribution(m_atts.GetOSPRayMinContribution(), renderer);
        vtkOSPRayRendererNode::SetMaxContribution(m_atts.GetOSPRayMaxContribution(), renderer);
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
