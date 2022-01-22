// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtVolumeRenderer.C                           //
// ************************************************************************* //

#include "avtVolumeRenderer.h"

#include <visit-config.h>

#include <StackTimer.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <avtCallback.h>

#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkPiecewiseFunction.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkRenderer.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#ifdef HAVE_OSPRAY
    #include <vtkOSPRayVolumeMapper.h>
    #include <vtkOSPRayRendererNode.h>
#endif

#include <vtkGeometryFilter.h>
#include <vtkPolyDataMapper.h>

#ifndef NO_DATA_VALUE
#define NO_DATA_VALUE -1e+37
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define LOCAL_DEBUG std::cerr
// #define LOCAL_DEBUG debug5

// ****************************************************************************
//  Constructor:  avtVolumeRenderer::avtVolumeRenderer
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//
// ****************************************************************************
avtVolumeRenderer::avtVolumeRenderer()
{
    transFunc  = vtkColorTransferFunction::New();
    opacity    = vtkPiecewiseFunction::New();
    volumeProp = vtkVolumeProperty::New();
    curVolume  = vtkVolume::New();
}

// ****************************************************************************
//  Destructor:  avtVolumeRenderer::~avtVolumeRenderer
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//
// ****************************************************************************
avtVolumeRenderer::~avtVolumeRenderer()
{
    if (curVolume != nullptr)
    {
        curVolume->Delete();
    }
    if (volumeMapper != nullptr)
    {
        volumeMapper->Delete();
    }
    if (volumeProp != nullptr)
    {
        volumeProp->Delete();
    }

    if (imageToRender != nullptr)
    {
        imageToRender->Delete();
    }

    if (opacity != nullptr)
    {
        opacity->Delete();
    }
    if (transFunc != nullptr)
    {
        transFunc->Delete();
    }
}

// ****************************************************************************
//  Method: avtVolumeRenderer::New
//
//  Purpose:
//      A static method that creates the correct type of renderer.
//
//  Returns:    A renderer that is of type derived from this class.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2002
//
//  Modifications:
//
// ****************************************************************************
avtVolumeRenderer *
avtVolumeRenderer::New(void)
{
    return new avtVolumeRenderer;
}

// ****************************************************************************
//  Method:  avtVolumeRenderer::SetAtts
//
//  Purpose:
//    Set the attributes
//
//  Arguments:
//    a       : the new attributes
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//
// ****************************************************************************

void
avtVolumeRenderer::SetAtts(const AttributeGroup *a)
{
    const VolumeAttributes *newAtts = (const VolumeAttributes*)a;

    if (*newAtts == atts)
        return;

    atts = *(const VolumeAttributes*)a;
}

// ****************************************************************************
//  Method:  avtVolumeRenderer::Render
//
//  Purpose:
//    Set up things necessary to call the renderer implentation.  Make a new
//    implementation object if things have changed.
//
//  Arguments:
//    ds         The dataset to render
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  1, 2003
//
//  Modifications:
//
// ****************************************************************************

void
avtVolumeRenderer::Render(vtkDataSet *in_ds)
{
    StackTimer t("avtVolumeRenderer::Render");

#ifndef HAVE_OSPRAY
    if( atts.GetOsprayEnabledFlag() )
    {
        avtCallback::IssueWarning("Trying to use OSPRay when VTK was not built with OSPRay support. Default VTK renderering will be used.");
    }
#endif

    const char *mName = "avtDefaultRenderer::Render: ";

    if (VTKRen == nullptr)
    {
        debug1 << mName << "Default Renderer: VTKRen is nullptr!";
        EXCEPTION0(ImproperUseException);
    }

    if (imageToRender == nullptr)
    {
        LOCAL_DEBUG << mName << "Converting from rectilinear grid "
                    << "to image data" << std::endl;

        // Check for an implied transform - can not be done with the
        // current paradigm!!!!!!
        // avtDataAttributes &inatts = GetInput()->GetInfo().GetAttributes();
        // if (inatts.GetRectilinearGridHasTransform())
        // {
        //     EXCEPTION1(ImproperUseException,
        //                "vtkRectilinear grids with an implied transform can not be rendered.");
        // }

        // Now to the business
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
            cellData = false;
        }
        else
        {
            dataArr = in_ds->GetCellData()->GetScalars();

            if( dataArr == nullptr )
            {
	        EXCEPTION1(InvalidVariableException, "");
            }

            cellData = true;
        }

	std::string activeVarName = dataArr->GetName();
        std::string opacityVarName = atts.GetOpacityVariable();
        vtkDataArray *opacityArr = nullptr;

        LOCAL_DEBUG << __LINE__ << " [avtVolumeRenderer] "
		    << " vars : "
		    << activeVarName << "  " << opacityVarName << "  "
		    << std::endl;

        // There could be a spearate opacity scalar data array.
        if( opacityVarName != "default" && opacityVarName != activeVarName )
        {
            nComponents = 2;

            opacityArr = in_ds->GetPointData()->GetScalars( opacityVarName.c_str() );
            if( cellData && opacityArr )
            {
                EXCEPTION1(ImproperUseException, "The opacity data does not have the same centering (point) as the primary data (cell). This expection can be fixed by resampling the data on to a common mesh.");
            }

            if( opacityArr == nullptr )
            {
                opacityArr = in_ds->GetCellData()->GetScalars( opacityVarName.c_str() );

                if( !cellData && opacityArr )
                {
                    EXCEPTION1(ImproperUseException, "The opacity data does not have the same centering (cell) as the primary data (point). This expection can be fixed by resampling the data on to a common mesh.");
                }

                if( opacityArr == nullptr )
                {
                    EXCEPTION1(InvalidVariableException, opacityVarName);
                }
            }
        }
        else
        {
            nComponents = 1;
        }

        double dataRange[2] = {0., 1.};
        double opacityRange[2] = {0., 1.};

        dataArr->GetRange( dataRange );

        if( atts.GetUseColorVarMin() )
            dataRange[0] = atts.GetColorVarMin();
        if( atts.GetUseColorVarMax() )
            dataRange[1] = atts.GetColorVarMax();

        if( nComponents == 2 )
        {
            opacityArr->GetRange( opacityRange );

            if( atts.GetUseOpacityVarMin() )
                opacityRange[0] = atts.GetOpacityVarMin();
            if( atts.GetUseOpacityVarMax() )
                opacityRange[1] = atts.GetOpacityVarMax();
        }

        LOCAL_DEBUG << __LINE__ << " [avtVolumeRenderer] "
                  << " dims : "
                  << dims[0] << "  " << dims[1] << "  "<< dims[2] << "  "
                  << std::endl;

        LOCAL_DEBUG << __LINE__ << " [avtVolumeRenderer] "
                  << " extent : "
                  << extent[0] << "  " << extent[1] << "  "
                  << extent[2] << "  " << extent[3] << "  "
                  << extent[4] << "  " << extent[5] << "  "
                  << std::endl;

        LOCAL_DEBUG << __LINE__ << " [avtVolumeRenderer] "
                  << " bounds : "
                  << bounds[0] << "  " << bounds[1] << "  "
                  << bounds[2] << "  " << bounds[3] << "  "
                  << bounds[4] << "  " << bounds[5] << "  "
                  << std::endl;

        LOCAL_DEBUG << __LINE__ << " [avtVolumeRenderer] "
                  << " spacing : "
                  << spacingX << "  " << spacingY << "  " << spacingZ << "  "
                  << std::endl;

        LOCAL_DEBUG << __LINE__ << " [avtVolumeRenderer] "
                  << " data range : "
                  << dataRange[0] << "  " << dataRange[1] << "  "
                  << std::endl;

        if( nComponents == 2 )
            LOCAL_DEBUG << __LINE__ << " [avtVolumeRenderer] "
                        << " opacity range : "
                        << opacityRange[0] << "  " << opacityRange[1] << "  "
                        << std::endl;

        LOCAL_DEBUG << __LINE__ << " [avtVolumeRenderer] "
                    << " expecting "
                    << (cellData ? "cell " : "point ") << "data "
                    << std::endl;

        // The volume mapper requires a vtkImageData as input.
        imageToRender = vtkImageData::New();
        imageToRender->SetDimensions(dims);
        imageToRender->SetExtent(extent);
        imageToRender->SetSpacing(spacingX, spacingY, spacingZ);
        imageToRender->SetOrigin(bounds[0], bounds[2], bounds[4]);

        // Data size for the image data as either point or cell data.
        vtkIdType dataSize =
            (dims[0]-cellData) * (dims[1]-cellData) * (dims[2]-cellData);

        // Allocate the new scalars
        vtkDataArray* scalars = vtkDataArray::CreateDataArray(VTK_FLOAT);
        scalars->SetName("ImageScalars");
        scalars->SetNumberOfComponents(nComponents);
        scalars->SetNumberOfTuples(dataSize);

        if( cellData )
            imageToRender->GetCellData()->SetScalars(scalars);
        else
            imageToRender->GetPointData()->SetScalars(scalars);

        scalars->Delete();

        // The values on the image must be scale to between 0 and 255.
        double dataScale    = 255.0 / (   dataRange[1] -    dataRange[0]);
        double opacityScale = 255.0 / (opacityRange[1] - opacityRange[0]);

	if( dataRange[1] <= dataRange[0])
	    dataScale = 0;

	if( opacityRange[1] <= opacityRange[0])
	    opacityScale = 0;

        // Transfer the rgrid data to the image data
        // and scale to the proper range.
        useInterpolation = true;

        // VisIt populates empty space with the NO_DATA_VALUE.
        // This needs to map this to a value that the mapper accepts,
        // and then clamp it so to be fully translucent.
        int ptId = 0;
        for (int z = 0; z < dims[2]-cellData; ++z)
        {
            for (int y = 0; y < dims[1]-cellData; ++y)
            {
                for (int x = 0; x < dims[0]-cellData; ++x)
                {
                    // The opacity and color data may differ so add
                    // both as two separate components.
                    double dataTuple = dataArr->GetTuple1(ptId);
                    if (dataTuple <= NO_DATA_VALUE)
                    {
                        // The color map is 0 -> 255. For no data values,
                        // assign a new value just out side of the map.
                        scalars->SetComponent(ptId, 0, -1.0);
                        useInterpolation = false;
                    }
                    else
                    {
                        double val = (dataTuple - dataRange[0]) * dataScale;

                        if( val < 0   ) val = 0;
                        if( val > 255 ) val = 255;

                        scalars->SetComponent(ptId, 0, val);
                    }

                    if( nComponents == 2 )
                    {
                        double opacityTuple = opacityArr->GetTuple1(ptId);
                        if (opacityTuple <= NO_DATA_VALUE)
                        {
                            // The opacity map is 0 -> 255. For no data values,
                            // assign a new value just out side of the map.
                            scalars->SetComponent(ptId, 1, -1.0);
                            useInterpolation = false;
                        }
                        else
                        {
                            double val = (opacityTuple - opacityRange[0]) * opacityScale;

                            if( val < 0   ) val = 0;
                            if( val > 255 ) val = 255;

                            scalars->SetComponent(ptId, 1, val);
                        }
                    }

                    ptId++;
                }
            }
        }

        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                    << " useInterpolation: "
                    << useInterpolation << "  "
                    << std::endl;
    }

    if( volumeMapper == nullptr ||
        OSPRayEnabled != atts.GetOsprayEnabledFlag())
    {
        OSPRayEnabled = atts.GetOsprayEnabledFlag();

        if (volumeMapper != nullptr)
            volumeMapper->Delete();

#ifdef HAVE_OSPRAY
        if( atts.GetOsprayEnabledFlag())
        {
            volumeMapper = vtkOSPRayVolumeMapper::New();

            LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                        << " OSPRay Volume Mapper "
                        << std::endl;
        }
        else
#endif
        {
            volumeMapper = vtkGPUVolumeRayCastMapper::New();

            LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                        << " GPU Volume Ray Cast Mapper "
                        << std::endl;
        }

        volumeMapper->SetBlendModeToComposite();
        resetColorMap = true;
    }

    if( cellData )
        volumeMapper->SetScalarModeToUseCellData();
    else
        volumeMapper->SetScalarModeToUsePointData();

    volumeMapper->SetInputData(imageToRender);

    if (resetColorMap || oldAtts != atts)
    {
        constexpr double rgbaScale = 1.0 / 255.0;

        resetColorMap = false;
        oldAtts       = atts;

        LOCAL_DEBUG << mName << "Resetting color" << endl;

        // Create the transfer function and the opacity mapping.
        constexpr int tableSize = 256;
        unsigned char rgba[tableSize*4];
        atts.GetTransferFunction(rgba);

        // To make the NO_DATA_VALUEs fully translucent turn clamping
        // off (opacity becomes 0.0)
        transFunc->RemoveAllPoints();
        transFunc->SetScaleToLinear();
        transFunc->SetClamping(false);

        opacity->RemoveAllPoints();
        opacity->SetClamping(false);

        double atten = atts.GetOpacityAttenuation();

        // Add the color map to vtk's transfer function
        for(int i = 0; i < tableSize; i++)
        {
            int rgbIdx  = 4 * i;
            transFunc->AddRGBPoint(i,
                                   double(rgba[rgbIdx    ]) * rgbaScale,
                                   double(rgba[rgbIdx + 1]) * rgbaScale,
                                   double(rgba[rgbIdx + 2]) * rgbaScale);
            opacity->AddPoint(i, double(rgba[rgbIdx + 3]) * rgbaScale * atten);
        }

        // For some reason, the endpoints aren't included when
        // clamping is turned off. So add some padding on the ends of
        // the mapping functions.
        transFunc->AddRGBPoint(-1,
                               double(rgba[0]) * rgbaScale,
                               double(rgba[1]) * rgbaScale,
                               double(rgba[2]) * rgbaScale);
        opacity->AddPoint(-1, double(rgba[3]) * rgbaScale * atten);

        int rgbIdx = (tableSize-1) * 4;
        transFunc->AddRGBPoint(tableSize,
                               double(rgba[rgbIdx    ]) * rgbaScale,
                               double(rgba[rgbIdx + 1]) * rgbaScale,
                               double(rgba[rgbIdx + 2]) * rgbaScale);
        opacity->AddPoint(tableSize, double(rgba[rgbIdx + 3]) * rgbaScale * atten);

        // Set the volume properties.
        volumeProp->SetColor(transFunc);
        volumeProp->SetScalarOpacity(opacity);
        volumeProp->SetIndependentComponents( nComponents == 1 );
        volumeProp->SetShade( atts.GetLightingFlag() );

        // Set ambient, diffuse, specular, and specular power (shininess).
        const double *matProp = atts.GetMaterialProperties();

        if (atts.GetLightingFlag() && matProp != nullptr)
        {
            volumeProp->SetAmbient(matProp[0]);
            volumeProp->SetDiffuse(matProp[1]);
            volumeProp->SetSpecular(matProp[2]);
            volumeProp->SetSpecularPower(matProp[3]);
        }

        // If the dataset contains NO_DATA_VALUEs, interpolation will
        // not work correctly on the boundaries (between a real value
        // and a no data value). Hopefully this will be addressed in the
        // future. For now, only interpolate when the dataset contains
        // none of these values.
        if (useInterpolation)
        {
            volumeProp->SetInterpolationTypeToLinear();
        }
        else
        {
            volumeProp->SetInterpolationTypeToNearest();
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

        double sampleDistReference = 1.0/10.0;
        double averageSpacing = (spacing[0] + spacing[1] + spacing[2]) / 3.0;
        double sampleDist     = averageSpacing / sampleDistReference;

        volumeProp->SetScalarOpacityUnitDistance(1, sampleDist);

        curVolume->SetMapper(volumeMapper);
        curVolume->SetProperty(volumeProp);
    }

#ifdef HAVE_OSPRAY
    if( atts.GetOsprayEnabledFlag() )
    {
        LOCAL_DEBUG << __LINE__ << " [VisItVTKDevice] "
                    << " RenderType: "
                    << (atts.GetOsprayRenderType() ? "PathTracer" : "SciVis") << "  "
                    << std::endl;

        if( atts.GetOsprayRenderType() == 1 )
            vtkOSPRayRendererNode::SetRendererType("pathtracer", VTKRen);
        else
            vtkOSPRayRendererNode::SetRendererType("scivis", VTKRen);

        vtkOSPRayRendererNode::SetSamplesPerPixel(atts.GetOspraySPP(), VTKRen);
        vtkOSPRayRendererNode::SetAmbientSamples (atts.GetOsprayAOSamples(), VTKRen);
        vtkOSPRayRendererNode::SetMinContribution(atts.GetOsprayMinContribution(), VTKRen);
        vtkOSPRayRendererNode::SetMaxContribution(atts.GetOsprayMaxContribution(), VTKRen);
    }
#endif

    volumeMapper->Render(VTKRen, curVolume);
}
