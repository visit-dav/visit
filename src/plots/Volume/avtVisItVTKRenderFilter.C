// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ***************************************************************************
//  avtVisItVTKRenderFilter.C
// ***************************************************************************

#include <avtVisItVTKRenderFilter.h>
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

#include <visit-config.h> // for HAVE_OSPRAY

#ifdef HAVE_OSPRAY
    #include <vtkOSPRayVolumeMapper.h>
    #include <vtkOSPRayRendererNode.h>
#endif

#include <vtkDataSetSurfaceFilter.h>
#include <vtkGeometryFilter.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>

#include <vector>

// #define DUMP_INTERMEDIATE_IMAGES

#ifdef DUMP_INTERMEDIATE_IMAGES
    #include <vtkWindowToImageFilter.h>
    #include <vtkPNGWriter.h>
#endif

// #ifdef PARALLEL
//   #define LOCAL_DEBUG std::cerr << __LINE__ << " [VisItVTKRenderFilter] " \
//                                 << "rank: "  << PAR_Rank() << "  "
// #else
//   #define LOCAL_DEBUG std::cerr << __LINE__ << " [VisItVTKRenderFilter] "
// #endif

#define LOCAL_DEBUG debug5 << " [VisItVTKRenderer] "

// ****************************************************************************
//  Method: avtVisItVTKRenderFilter constructor
//
//  Programmer: Allen R. Sanderson
//  Creation:  30 November 2021
//
//  Modifications:
//
// ****************************************************************************

avtVisItVTKRenderFilter::avtVisItVTKRenderFilter() : avtRayTracerBase(),
    avtVisItVTKRenderer(), m_lightList()
{
    LOCAL_DEBUG << "Creating a new filter." << std::endl;
}

// ****************************************************************************
//  Method: avtVisItVTKRenderFilter destructor
//
//  Programmer: Allen R. Sanderson
//  Creation:  30 November 2021
//
//  Modifications:
//
// ****************************************************************************

avtVisItVTKRenderFilter::~avtVisItVTKRenderFilter()
{
}


// ****************************************************************************
//  Method: avtVisItVTKRenderFilter::CreateCamera
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
avtVisItVTKRenderFilter::CreateCamera()
{
    vtkCamera *camera = vtkCamera::New();
    viewInfo.SetCameraFromView( camera );

    return camera;
}

// ****************************************************************************
//  Method: avtVisItVTKRenderFilter::CreateLights
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
avtVisItVTKRenderFilter::CreateLights()
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
//  Method: avtVisItVTKRenderFilter::CreateFinalImage
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
avtVisItVTKRenderFilter::CreateFinalImage(const void *colorBuffer,
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

#ifdef DUMP_INTERMEDIATE_IMAGES
    // For debugging the final from the compositing.
    if(PAR_Rank() == 0)
    {
      vtkPNGWriter* writer = vtkPNGWriter::New();

      writer->SetInputData(finalImageData);
      writer->SetFileName("vp_finialImage.png");
      writer->Write();
      writer->Delete();
    }
#endif

    finalImageData->Delete();

    return finalImage;
}

// ****************************************************************************
//  Method: avtVisItVTKRenderFilter::Execute
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

#define NO_DATA_VALUE -1e+37

void
avtVisItVTKRenderFilter::Execute()
{
    StackTimer t0("VisItVTKRenderer Rendering");

    const int width  = screen[0];
    const int height = screen[1];

    // Get the input data.
    auto dob = GetInput();
    auto inputTree = GetInputDataTree(); // avtDataTree_p
    int nsets = 0;
    vtkDataSet **datasetPtrs = inputTree->GetAllLeaves(nsets);

    // There should be only one data set.
    if( nsets > 1 )
    {
        LOCAL_DEBUG << "Too many datasets to render." << std::endl;

        EXCEPTION1(ImproperUseException, "Only one input dataset may be rendered. This exception can be fixed by resampling the data on to a common rectilinear mesh");
    }

    // Before calling NeedImage the number of components needs to be
    // known which is done by comparing the data and opacity variable
    // names.
    NumberOfComponents(activeVarName, opacityVarName);

    // The data and opacity ranges must be known before calling
    // UpdateRenderingState.
    if( NeedImage() )
    {
        // GetDataExtents is a parallel call so do them regardless if
        // there is data or not. Otherwise MPI crashes.
        GetDataExtents(m_dataRange, activeVarName.c_str());

        LOCAL_DEBUG << " data range : "
                    << m_dataRange[0] << "  " << m_dataRange[1] << "  "
                    << std::endl;

        if( m_nComponents == 2 )
        {
            GetDataExtents(m_opacityRange, opacityVarName.c_str());

            LOCAL_DEBUG << " opacity range : "
                        << m_opacityRange[0] << "  " << m_opacityRange[1] << "  "
                        << std::endl;
        }
    }

    LOCAL_DEBUG << "nsets: " << nsets << "  "
                << "nComponents: " << m_nComponents << "  "
                << "needImage: " << m_needImage << "  "
                << std::endl;

    // If no data then no image will be generated on this rank but
    // there is parallel compositing so account for it.
    unsigned char * renderedFrameBuffer = nullptr;
    float zDepth = 0;

    // There should be only be a rectilinear data set.
    if( nsets == 1 )
    {
        // Create camera
        vtkCamera* camera = CreateCamera();

        // Create lights
        vtkLightCollection* lights = CreateLights();

        // Create the renderer
        vtkRenderer* renderer = vtkRenderer::New();
        renderer->SetBackground(0.0f, 0.0f, 0.0f);
        renderer->SetBackgroundAlpha(0.0f);
        renderer->AddViewProp(m_volume);
        renderer->SetActiveCamera( camera );
        renderer->SetLightCollection( lights );

        if (m_ambientOn)
        {
            // LOCAL_DEBUG << "ambientOn: " << m_ambientOn << "  "
            //             << m_ambientColor[0] << "  "
            //             << m_ambientColor[1] << "  "
            //             << m_ambientColor[2] << "  "
            //             << std::endl;

            renderer->SetAmbient(m_ambientColor);
        }
        else
        {
            // LOCAL_DEBUG << "ambientOn: " << m_ambientOn << " 1, 1, 1 "
            //             << std::endl;

            renderer->SetAmbient(1., 1., 1.);
        }

        vtkDataSet* in_ds = datasetPtrs[ 0 ];

        UpdateRenderingState(in_ds, renderer);

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

#ifdef DUMP_INTERMEDIATE_IMAGES
        // For debugging the inidivually rendered images before the
        // compositing.
        {
          vtkWindowToImageFilter* im = vtkWindowToImageFilter::New();
          vtkPNGWriter* writer = vtkPNGWriter::New();

          std::stringstream name;
	  name << "vp_renderWindow";

          if( PAR_Size() > 1 )
              name << "_rank_" << PAR_Rank();
	  name << ".png";

          im->SetInput(renderWin);
          im->Update();
          writer->SetInputConnection(im->GetOutputPort());
          writer->SetFileName(name.str().c_str());
          writer->Write();
          writer->Delete();
          im->Delete();
        }
#endif
        // Get the resulting image - must have alpha values for ICET.
        renderedFrameBuffer =
            renderWin->GetRGBACharPixelData( 0, 0, width-1, height-1, 1 );

        camera->Delete();
        lights->Delete();
        renderer->Delete();
        renderWin->Delete();

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
                LOCAL_DEBUG << "bad alpha composting "
                            << cc << "/" << aa << " are bad." << std::endl;
            }
        }

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

        zDepth = transPt[2];

        LOCAL_DEBUG << "Average z depth: "
                    << transPt[2] << "  " << std::endl;
    }
    else // if( nsets == 0 )
    {
        LOCAL_DEBUG << "Nothing to render, no data." << std::endl;

        if( m_imageToRender != nullptr )
        {
            m_imageToRender->Delete();
            m_imageToRender = nullptr;
        }
    }

    // Create final image using ICET
    avtImage_p finalImage =
        CreateFinalImage(renderedFrameBuffer, width, height, zDepth);

    if(PAR_Rank() == 0)
        SetOutput(finalImage);

    // Clean up.
    if( renderedFrameBuffer )
        delete [] renderedFrameBuffer;
}
