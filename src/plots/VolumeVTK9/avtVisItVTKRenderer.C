// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// *************************************************************************
//   avtVisItVTKRenderer.C
// *************************************************************************

#include <avtVisItVTKRenderer.h>

#include <avtParallel.h>

#include <DebugStream.h>
#include <TimingsManager.h>
#include <StackTimer.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <avtCallback.h>
#include <avtDatasetExaminer.h>


#include <vtkImageData.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkDataObject.h>
#include <vtkRectilinearGrid.h>
#include <vtkRenderer.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <visit-config.h> // for HAVE_OSPRAY

#ifdef HAVE_OSPRAY
    #include <vtkOSPRayVolumeMapper.h>
    #include <vtkOSPRayRendererNode.h>
#endif

#include <string>
#include <vector>

// #define DUMP_IMAGE_DATA

#ifdef DUMP_IMAGE_DATA
    #include <vtkXMLImageDataWriter.h>
#endif

// #ifdef PARALLEL
//   #define LOCAL_DEBUG std::cerr << __LINE__ << " [VisItVTKRenderer] " \
//                                 << "rank: "  << PAR_Rank() << "  "
// #else
//   #define LOCAL_DEBUG std::cerr << __LINE__ << " [VisItVTKRenderer] "
// #endif

#define LOCAL_DEBUG debug5 << " [VisItVTKRenderer] "

// ****************************************************************************
//  Method: avtVisItVTKRenderer constructor
//
//  Programmer: Allen R. Sanderson
//  Creation:  30 November 2021
//
//  Modifications:
//
// ****************************************************************************

avtVisItVTKRenderer::avtVisItVTKRenderer()
{
    LOCAL_DEBUG << "Creating a new renderer." << std::endl;

    m_transFunc      = vtkColorTransferFunction::New();
    m_opacity        = vtkPiecewiseFunction::New();
    m_volumeProperty = vtkVolumeProperty::New();
    m_volume         = vtkVolume::New();
}

// ****************************************************************************
//  Method: avtVisItVTKRenderer destructor
//
//  Programmer: Allen R. Sanderson
//  Creation:  30 November 2021
//
//  Modifications:
//
// ****************************************************************************

avtVisItVTKRenderer::~avtVisItVTKRenderer()
{
    if (m_volume != nullptr)
    {
        m_volume->Delete();
    }
    if (m_volumeMapper != nullptr)
    {
        m_volumeMapper->Delete();
    }
    if (m_volumeProperty != nullptr)
    {
        m_volumeProperty->Delete();
    }

    if (m_imageToRender != nullptr)
    {
        m_imageToRender->Delete();
    }

    if (m_opacity != nullptr)
    {
        m_opacity->Delete();
    }
    if (m_transFunc != nullptr)
    {
        m_transFunc->Delete();
    }
}


// ****************************************************************************
//  Method:  avtVisItVTKRenderer::SetAtts
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
avtVisItVTKRenderer::SetAtts(const AttributeGroup *a)
{
    const VolumeAttributes *newAtts = (const VolumeAttributes*)a;

    if (*newAtts == m_atts)
        return;

    m_atts = *(const VolumeAttributes*) a;
}

// ****************************************************************************
//  Method:  avtVisItVTKRenderer::NumberOfComponents
//
//  Purpose:
//    Set the number of components based on the color and
//    opacity variable names.
//
//  Arguments:
//    activeVariable  : the active variable
//    opacityVariable : the opacity variable
//
//  Programmer: Allen R. Sanderson
//  Creation:  30 November 2021
//
//  Modifications:
//
// ****************************************************************************

int
avtVisItVTKRenderer::NumberOfComponents(const std::string activeVariable,
                                        const std::string opacityVariable)
{
    if( opacityVariable == activeVariable )
    {
        avtCallback::IssueWarning("The opacity variable is the same as "
                                  "the primary variable. Ignoring it and "
                                  "any possible min/max setting.");

        m_nComponents = 1;
    }
    else if( opacityVariable != "default" )
    {
        m_nComponents = 2;

#ifdef HAVE_OSPRAY
        if( m_atts.GetOSPRayEnabledFlag() )
        {
            avtCallback::IssueWarning("The opacity variable is not the same as "
                                      "the primary variable."
                                      "Two component rendering is not "
                                      "available with OSPRay. Ignoring it and "
                                      "any possible min/max setting.");
            m_nComponents = 1;
            m_opacityVarName = "default";
        }
#endif
    }
    else
    {
        m_nComponents = 1;
    }

    LOCAL_DEBUG << "nComponents: " << m_nComponents << "  "
                << std::endl;

    return m_nComponents;
}

// ****************************************************************************
//  Method:  avtVisItVTKRenderer::NeedImage
//
//  Purpose:
//    Checks the color min/max and if needed the opacity min/max to
//    determine if an image is needed.
//
//    NOTE: m_nComponents must be set before being called.
//
//  Arguments:
//    none
//
//  Programmer: Allen R. Sanderson
//  Creation:  30 November 2021
//
//  Modifications:
//    Kathleen Biagas, Tue Feb 6 2024
//    Check if current input matches previous (only doing pointer comparison).
//    This will allow multi-domain problems that were not resampled to still
//    have all their domains rendered (as long as they are rect grids).
//
// ****************************************************************************

bool
avtVisItVTKRenderer::NeedImage(vtkDataSet *input)
{
    // if the input is different than previous we are probably
    // dealing with a multi-domain that wasn't resampled.
    // Set m_firstPass to true so m_needImage will be re-evaluated
    // and the new dataset/domain can be rendered.
    if(input != previousInput)
    {
        m_firstPass = true;
        previousInput = input;
    }

    if( m_firstPass == true ||

        // Color variable change or min/max change. The active var
        // name triggers needing an image on the first pass.
        m_useColorVarMin != m_atts.GetUseColorVarMin() ||
        (m_atts.GetUseColorVarMin() &&
         m_colorVarMin != m_atts.GetColorVarMin()) ||
        m_useColorVarMax != m_atts.GetUseColorVarMax() ||
        (m_atts.GetUseColorVarMax() &&
         m_colorVarMax != m_atts.GetColorVarMax()) ||

        // Opacity variable change or min/max change.
        (m_nComponents == 2 &&
         (m_opacityVarName != m_atts.GetOpacityVariable() ||
          m_useOpacityVarMin != m_atts.GetUseOpacityVarMin() ||
          (m_atts.GetUseOpacityVarMin() &&
           m_opacityVarMin != m_atts.GetOpacityVarMin()) ||
          m_useOpacityVarMax != m_atts.GetUseOpacityVarMax() ||
          (m_atts.GetUseOpacityVarMax() &&
           m_opacityVarMax != m_atts.GetOpacityVarMax()))) )
    {
        if( m_nComponents == 0 )
        {
            EXCEPTION1(ImproperUseException,
                       "NeedImage is being called without the number of components beging set. This error is a developer error");
        }

        m_firstPass = false;

        // Store the color variable values so to check for a state change.
        m_useColorVarMin = m_atts.GetUseColorVarMin();
        m_colorVarMin    = m_atts.GetColorVarMin();
        m_useColorVarMax = m_atts.GetUseColorVarMax();
        m_colorVarMax    = m_atts.GetColorVarMax();

        // Store the opacity variable values so to check for a state change.
        if( m_nComponents == 2 )
        {
            m_opacityVarName   = m_atts.GetOpacityVariable();
            m_useOpacityVarMin = m_atts.GetUseOpacityVarMin();
            m_opacityVarMin    = m_atts.GetOpacityVarMin();
            m_useOpacityVarMax = m_atts.GetUseOpacityVarMax();
            m_opacityVarMax    = m_atts.GetOpacityVarMax();
        }

        m_needImage = true;
    }
    else
    {
        m_needImage = false;
    }

    LOCAL_DEBUG << "needImage: " << m_needImage << "  "
                << std::endl;

    return m_needImage;
}

// ****************************************************************************
//  Method: avtVisItVTKRenderer::UpdateRenderingState
//
//  Purpose:
//    Updates the VTK Rendering.
//
//    NOTE: NeedImage must be called at least once before being called.
//
//  Programmer: Allen R. Sanderson
//  Creation:   30 November 2021
//
//  Modifications:
//
// ****************************************************************************

#ifndef NO_DATA_VALUE
  #define NO_DATA_VALUE -1e+37
#endif

void
avtVisItVTKRenderer::UpdateRenderingState(vtkDataSet * in_ds,
                                          vtkRenderer *renderer)
{
    StackTimer t0("VisItVTKRenderer Rendering");

#ifndef HAVE_OSPRAY
    if( m_atts.GetOSPRayEnabledFlag() )
    {
        avtCallback::IssueWarning("Trying to use OSPRay when VTK was not built with OSPRay support. The default VTK renderering will be used.");
    }
#endif

    if( in_ds->GetDataObjectType() != VTK_RECTILINEAR_GRID )
    {
        LOCAL_DEBUG << "Only a vtkRectilinearGrid can be rendered."
                    << std::endl;

        EXCEPTION1(ImproperUseException,
                   "Only a vtkRectilinearGrid can be rendered. This exception can be fixed by resampling the data on to a rectilinear mesh");
    }

    // Create a new image if needed.
    if( m_imageToRender == nullptr || m_needImage  )
    {
        if( m_firstPass == true )
        {
            EXCEPTION1(ImproperUseException,
                       "UpdateRenderingState is being called before NeedImage has been called. This error is a developer error");
        }

        if((m_dataRange[0] > m_dataRange[1]) ||
           (m_nComponents == 2 && m_opacityRange[0] > m_opacityRange[1]))
        {
            EXCEPTION1(ImproperUseException,
                       "UpdateRenderingState is being called before the data ranges have been set. This error is a developer error");
        }

        LOCAL_DEBUG << "Converting from rectilinear grid to image data."
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
                EXCEPTION1(InvalidVariableException, "");
            }

            m_cellData = true;
        }

        // There could be a spearate opacity scalar data array.
        vtkDataArray *opacityArr = nullptr;

        if( m_nComponents == 2 )
        {
            std::string opacityVarName = m_atts.GetOpacityVariable();

            opacityArr = in_ds->GetPointData()->GetScalars( opacityVarName.c_str() );
            if( m_cellData && opacityArr )
            {
                EXCEPTION1(ImproperUseException, "The opacity data centering (point) does not match the primary data centering (cell). This exception can be fixed by resampling the data on to a common rectilinear mesh");
            }

            if( opacityArr == nullptr )
            {
              opacityArr = in_ds->GetCellData()->GetScalars( opacityVarName.c_str() );

                if( !m_cellData && opacityArr )
                {
                    EXCEPTION1(ImproperUseException, "The opacity data centering (cell) does not match the primary data centering (point). This exception can be fixed by resampling the data on to a common rectilinear mesh");
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
            m_dataRange[0] = m_atts.GetColorVarMin();
        if( m_atts.GetUseColorVarMax() )
            m_dataRange[1] = m_atts.GetColorVarMax();

        // If needed adjust the opacity var range
        if( m_nComponents == 2 )
        {
            if( m_atts.GetUseOpacityVarMin() )
                m_opacityRange[0] = m_atts.GetOpacityVarMin();
            if( m_atts.GetUseOpacityVarMax() )
                m_opacityRange[1] = m_atts.GetOpacityVarMax();
        }

        LOCAL_DEBUG << "extent : "
                    << extent[0] << "  " << extent[1] << "  "
                    << extent[2] << "  " << extent[3] << "  "
                    << extent[4] << "  " << extent[5] << "  "
                    << std::endl;

        LOCAL_DEBUG << "bounds : "
                    << bounds[0] << "  " << bounds[1] << "  "
                    << bounds[2] << "  " << bounds[3] << "  "
                    << bounds[4] << "  " << bounds[5] << "  "
                    << std::endl;

        LOCAL_DEBUG << "spacing : "
                    << spacingX << "  " << spacingY << "  " << spacingZ << "  "
                    << std::endl;

        LOCAL_DEBUG << "data range : "
                    << m_dataRange[0] << "  " << m_dataRange[1] << "  "
                    << std::endl;

        if( m_nComponents == 2 )
            LOCAL_DEBUG << "opacity range : "
                        << m_opacityRange[0] << "  " << m_opacityRange[1] << "  "
                        << std::endl;

        LOCAL_DEBUG << "expecting "
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
        double dataScale    = 255.0f / (   m_dataRange[1] -    m_dataRange[0]);
        double opacityScale = 255.0f / (m_opacityRange[1] - m_opacityRange[0]);

        if( m_dataRange[1] <= m_dataRange[0])
            dataScale = 0;

        if( m_opacityRange[1] <= m_opacityRange[0])
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
                        double val = (dataTuple - m_dataRange[0]) * dataScale;

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
                            double val = (opacityTuple - m_opacityRange[0]) * opacityScale;
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

        LOCAL_DEBUG << "useInterpolation: "
                    << m_useInterpolation << "  "
                    << std::endl;

        // LOCAL_DEBUG << "NumberOfTuples: "
        //             << dataArr->GetNumberOfTuples() << "  "
        //             << " NumberOfTuples: "
        //             << ptId
        //             << std::endl;

#ifdef DUMP_IMAGE_DATA
        // For debugging the data images.
        {
            std::stringstream name;
            name << "vp_data_image_" << m_nComponents << "_comps";

#ifdef PARALLEL
            if( PAR_Size() > 1 )
              name << "_rank_" << PAR_Rank();
#endif

            name << ".vti";

            vtkXMLImageDataWriter* writer = vtkXMLImageDataWriter::New();
            writer->SetInputData(m_imageToRender);
            writer->SetFileName(name.str().c_str());
            writer->Write();
            writer->Delete();
        }
#endif
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

            LOCAL_DEBUG << "OSPRay Volume Mapper "
                        << std::endl;
        }
        else
#endif
        {
            m_volumeMapper = vtkGPUVolumeRayCastMapper::New();
            LOCAL_DEBUG << "GPU Volume Ray Cast Mapper "
                        << std::endl;
        }

        m_volumeMapper->SetBlendModeToComposite();
    }

    if( m_cellData )
        m_volumeMapper->SetScalarModeToUseCellData();
    else
        m_volumeMapper->SetScalarModeToUsePointData();

    m_volumeMapper->SetInputData(m_imageToRender);

    constexpr double rgbaScale = 1.0 / 255.0;

    // Create the transfer function and the opacity mapping.
    constexpr int tableSize = 256;
    unsigned char rgba[tableSize*4];
    m_atts.GetTransferFunction(rgba);

    // To make the NO_DATA_VALUEs fully translucent turn clamping
    // off (opacity becomes 0.0)
    m_transFunc->RemoveAllPoints();
    m_transFunc->SetScaleToLinear();
    m_transFunc->SetClamping(false);

    m_opacity->RemoveAllPoints();
    m_opacity->SetClamping(false);

    double atten = m_atts.GetOpacityAttenuation();

    // Add the color map to vtk's transfer function
    for(int i = 0; i < tableSize; i++)
    {
        int rgbIdx  = 4 * i;
        m_transFunc->AddRGBPoint(i,
                                 double(rgba[rgbIdx    ]) * rgbaScale,
                                 double(rgba[rgbIdx + 1]) * rgbaScale,
                                 double(rgba[rgbIdx + 2]) * rgbaScale);
        m_opacity->AddPoint(i, double(rgba[rgbIdx + 3]) * rgbaScale * atten);
    }

    // For some reason, the endpoints aren't included when
    // clamping is turned off. So add some padding on the ends of
    // the mapping functions.
    m_transFunc->AddRGBPoint(-1,
                             double(rgba[0]) * rgbaScale,
                             double(rgba[1]) * rgbaScale,
                             double(rgba[2]) * rgbaScale);
    m_opacity->AddPoint(-1, double(rgba[3]) * rgbaScale * atten);

    int rgbIdx = (tableSize-1) * 4;
    m_transFunc->AddRGBPoint(tableSize,
                             double(rgba[rgbIdx    ]) * rgbaScale,
                             double(rgba[rgbIdx + 1]) * rgbaScale,
                             double(rgba[rgbIdx + 2]) * rgbaScale);
    m_opacity->AddPoint(tableSize, double(rgba[rgbIdx + 3]) * rgbaScale * atten);

    // Set the volume properties.
    m_volumeProperty->SetColor(m_transFunc);
    m_volumeProperty->SetScalarOpacity(m_opacity);
    // volumeProperty->SetGradientOpacity(gradient);
    m_volumeProperty->SetIndependentComponents( m_nComponents == 1 );
    m_volumeProperty->SetShade( m_atts.GetLightingFlag() );

    LOCAL_DEBUG << "HasGradientOpacity: "
                << m_volumeProperty->HasGradientOpacity() << "  "
                << std::endl;

    // Set ambient, diffuse, specular, and specular power (shininess).
    const double *matProp = m_atts.GetMaterialProperties();

    if (m_atts.GetLightingFlag() && matProp != nullptr)
    {
        LOCAL_DEBUG << "lightingEnabled: " << m_atts.GetLightingFlag() << "  "
                    << matProp[0] << "  "
                    << matProp[1] << "  "
                    << matProp[2] << "  "
                    << matProp[3] << "  "
                    << std::endl;

        m_volumeProperty->SetAmbient(matProp[0]);
        m_volumeProperty->SetDiffuse(matProp[1]);
        m_volumeProperty->SetSpecular(matProp[2]);
        m_volumeProperty->SetSpecularPower(matProp[3]);
    }

    // If the dataset contains NO_DATA_VALUEs, interpolation will
    // not work correctly on the boundaries (between a real value
    // and a no data value). Hopefully this will be addressed in the
    // future. For now, only interpolate when the dataset contains
    // none of these values.
    if (m_useInterpolation)
    {
        m_volumeProperty->SetInterpolationTypeToLinear();
    }
    else
    {
        m_volumeProperty->SetInterpolationTypeToNearest();
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

    m_volumeProperty->SetScalarOpacityUnitDistance(1, sampleDist);

    LOCAL_DEBUG << "sampleDist: " << sampleDist << "  "
                << std::endl;

    // vtkSmartVolumeMapper::SafeDownCast( volumeMapper )->SetSampleDistance( averageSpacing * 0.1);

    // vtkSmartVolumeMapper::SafeDownCast( volumeMapper )->AutoAdjustSampleDistancesOff();

    // Set up the volume
    m_volume->SetMapper(m_volumeMapper);
    m_volume->SetProperty(m_volumeProperty);

#ifdef HAVE_OSPRAY
    if( m_atts.GetOSPRayEnabledFlag() )
    {
        LOCAL_DEBUG << "RenderType: "
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
}
