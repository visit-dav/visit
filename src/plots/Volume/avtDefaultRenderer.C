// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtDefaultRenderer.C                            //
// ************************************************************************* //

#include "avtDefaultRenderer.h"

#include <avtCallback.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRectilinearGrid.h>
#include <vtkRenderer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkOSPRayVolumeMapper.h>
#include <vtkOSPRayRendererNode.h>

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

// ****************************************************************************
//  Method: avtDefaultRenderer::avtDefaultRenderer
//
//  Purpose:
//    Initialize the memebers associated with the default renderer.
//
//  Programmer:  Alister Maguire
//  Creation:    April 3, 2017
//
//  Modifications:
//
// ****************************************************************************

avtDefaultRenderer::avtDefaultRenderer()
{
    VTKRen           = nullptr;

    curVolume        = vtkVolume::New();

    imageToRender    = nullptr;
    mapper           = nullptr;

    volumeProp       = vtkVolumeProperty::New();
    transFunc        = vtkColorTransferFunction::New();
    opacity          = vtkPiecewiseFunction::New();

    resetColorMap    = false;
}


// ****************************************************************************
//  Method: avtDefaultRenderer::~avtDefaultRenderer
//
//  Purpose:
//    Destructor.
//
//  Programmer:  Alister Maguire
//  Creation:    April 3, 2017
//
//  Modifications:
//
//    Alister Maguire, Tue Dec 11 10:18:31 PST 2018
//    Added deletions for curVolume, imageToRender, volumeProp,
//    and mapper.
//
// ****************************************************************************

avtDefaultRenderer::~avtDefaultRenderer()
{
    if (curVolume != nullptr)
    {
        curVolume->Delete();
    }

    if (imageToRender != nullptr)
    {
        imageToRender->Delete();
    }
    if (mapper != nullptr)
    {
        mapper->Delete();
    }

    if (volumeProp != nullptr)
    {
        volumeProp->Delete();
    }
    if (transFunc != nullptr)
    {
        transFunc->Delete();
    }
    if (opacity != nullptr)
    {
        opacity->Delete();
    }
}


// ****************************************************************************
//  Method:  avtDefaultRenderer::Render
//
//  Purpose:
//    Render a volume using a vtkSmartVolumeMapper
//
//  Arguments:
//    props   : the rendering properties
//    volume  : the volume to be rendered
//
//  Programmer:  Alister Maguire
//  Creation:    April 3, 2017
//
//  Modifications:
//
//    Alister Maguire, Tue Dec 11 13:02:20 PST 2018
//    Refactored to appropriately handle the NO_DATA_VALUE. Also
//    changed smart pointers to standard pointers and added memory
//    management.
//
//    Alister Maguire, Mon Mar 25 09:20:43 PDT 2019
//    Updated to use different scalars for opacity and color.
//
//    Alister Maguire, Tue Jun 11 11:08:52 PDT 2019
//    Update to use ambient, diffuse, specular, and specular power.
//
//    Alister Maguire, Tue Jun 18 11:36:44 PDT 2019
//    If VTKRen is nullptr, we can't render.
//
//    Alister Maguire, Wed Nov  4 07:29:17 PST 2020
//    Set the scalar opacity unit distance. This is basically setting
//    the sample distance for ray casting.
//
// ****************************************************************************

void
avtDefaultRenderer::Render(
    const avtVolumeRendererImplementation::RenderProperties &props,
    const avtVolumeRendererImplementation::VolumeData &volume)
{
    const char *mName = "avtDefaultRenderer::Render: ";

    if (VTKRen == nullptr)
    {
        debug1 << mName << "Default Renderer: VTKRen is nullptr!";
        EXCEPTION0(ImproperUseException);
    }

    // 2D data has no volume, so don't try to render.
    if (props.dataIs2D)
    {
        debug5 << mName << "Cannot perform volume rendering on "
            << "2D data... returning";
        return;
    }

    if (imageToRender == nullptr)
    {
        debug5 << mName << "Converting from rectilinear grid "
            << "to image data";

        vtkRectilinearGrid* rgrid = vtkRectilinearGrid::SafeDownCast( volume.grid );

        if( rgrid->GetDataObjectType() != VTK_RECTILINEAR_GRID )
        {
            EXCEPTION1(ImproperUseException,
                       "Only vtkRectilinearGrid may be rendered.");
        }

        // Check for an implied transform.
        // avtDataAttributes &inatts = GetInput()->GetInfo().GetAttributes();
        // if (inatts.GetRectilinearGridHasTransform())
        // {
        //     EXCEPTION1(ImproperUseException,
        //                "vtkRectilinear grids with an implied transform can not be rendered.");
        // }

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

        std::cerr << __LINE__ << " [Default Renderer] dims : "
                  << dims[0] << "  " << dims[1] << "  "<< dims[2] << "  "
                  << std::endl;

        std::cerr << __LINE__ << " [Default Renderer] extent : "
                  << extent[0] << "  " << extent[1] << "  "
                  << extent[2] << "  " << extent[3] << "  "
                  << extent[4] << "  " << extent[5] << "  "
                  << std::endl;

        std::cerr << __LINE__ << " [Default Renderer] bounds : "
                  << bounds[0] << "  " << bounds[1] << "  "
                  << bounds[2] << "  " << bounds[3] << "  "
                  << bounds[4] << "  " << bounds[5] << "  "
                  << std::endl;

        std::cerr << __LINE__ << " [Default Renderer] spacing : "
                  << spacingX << "  " << spacingX << "  "<< spacingX << "  "
                  << std::endl;

        imageToRender = vtkImageData::New();
        imageToRender->SetDimensions(dims);
        imageToRender->SetExtent(extent);
        imageToRender->SetSpacing(spacingX, spacingY, spacingZ);
        imageToRender->AllocateScalars(VTK_FLOAT, 2);

        // Set the origin to match the lower bounds of the grid
        imageToRender->SetOrigin(bounds[0], bounds[2], bounds[4]);

        // There could be both a scalar and opacity data arrays. So get both.
        vtkDataArray *dataArr    = volume.data.data;
        vtkDataArray *opacityArr = volume.opacity.data;

        double dataRange[2] = {0., 0.};
        double opacityRange[2] = {0., 0.};
        dataArr->GetRange( dataRange );
        opacityArr->GetRange( opacityRange );

        std::cerr << __LINE__ << " [Default Renderer] data range : "
                  << dataRange[0] << "  " << dataRange[1] << "  "
                  << std::endl;

        std::cerr << __LINE__ << " [Default Renderer] opacity range : "
                  << opacityRange[0] << "  " << opacityRange[1] << "  "
                  << std::endl;

        double dataScale    = 255.0 / (   dataRange[1] -    dataRange[0]);
        double opacityScale = 255.0 / (opacityRange[1] - opacityRange[0]);

        // Transfer the rgrid data to the image data
        // and scale to the proper range.
        useInterpolation = true;

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
                    // The opacity and color data may differ so add
                    // both as two separate components.
                    double dataTuple = dataArr->GetTuple1(ptId);
                    if (dataTuple <= NO_DATA_VALUE)
                    {
                        // The color map is 0 -> 255. For no data values,
                        // assign a new value just out side of the map.
                        imageToRender->SetScalarComponentFromFloat(x, y, z, 0, -1.0);
                        useInterpolation = false;
                    }
                    else
                    {
                        double val = (dataTuple - dataRange[0]) * dataScale;
                        imageToRender->SetScalarComponentFromFloat(x, y, z, 0, val);
                    }

                    double opacityTuple = opacityArr->GetTuple1(ptId);
                    if (opacityTuple <= NO_DATA_VALUE)
                    {
                        // The opacity map is 0 -> 255. For no data values,
                        // assign a new value just out side of the map.
                        imageToRender->SetScalarComponentFromFloat(x, y, z, 1, -1.0);
                        useInterpolation = false;
                    }
                    else
                    {
                        double val = (opacityTuple - opacityRange[0]) * opacityScale;
                        imageToRender->SetScalarComponentFromFloat(x, y, z, 1, val);
                    }

                    ptId++;
                }
            }
        }

        std::cerr << __LINE__ << " [Default Renderer] useInterpolation: "
                  << useInterpolation << "  "
                  << std::endl;
    }

    if( mapper == nullptr || lastMapper != props.atts.GetOsprayEnabledFlag())
    {
        lastMapper = props.atts.GetOsprayEnabledFlag();

        if (mapper != nullptr)
            mapper->Delete();

        if( props.atts.GetOsprayEnabledFlag())
            mapper = vtkOSPRayVolumeMapper::New();
        else
            mapper = vtkSmartVolumeMapper::New();

        debug5 << mName << "Adding data to the SmartVolumeMapper" << endl;

        mapper->SetInputData(imageToRender);
        mapper->SetScalarModeToUsePointData();
        mapper->SetBlendModeToComposite();
        resetColorMap = true;

        double scalarRange[2];
        mapper->GetInput()->GetScalarRange(scalarRange);
        std::cerr << __LINE__ << " [Default Renderer] scalarRange: "
                  << scalarRange[0] << "  "
                  << scalarRange[1] << "  "
                  << std::endl;
    }

    if (resetColorMap || oldAtts != props.atts)
    {
        double rgbaScale = 1.0 / 255.0;

        resetColorMap = false;
        oldAtts       = props.atts;

        debug5 << mName << "Resetting color" << endl;

        // Create the transfer function and the opacity mapping.
        constexpr int tableSize = 256;
        unsigned char rgba[tableSize*4];
        props.atts.GetTransferFunction(rgba);

        // To make the  NO_DATA_VALUEs fully translucent turn clamping
        // off (opacity becomes 0.0)
        transFunc->SetScaleToLinear();
        transFunc->SetClamping(false);
        opacity->SetClamping(false);

        double atten = props.atts.GetOpacityAttenuation();

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

	// transFunc->PrintSelf( std::cerr, vtkIndent(2) );
	
        rgbIdx  = 4 * 64;
        std::cerr << __LINE__ << " [Default Renderer] RGBA: " << 64 << "  "
                  << double(rgba[rgbIdx  ]) * rgbaScale << "  "
                  << double(rgba[rgbIdx+1]) * rgbaScale << "  "
                  << double(rgba[rgbIdx+2]) * rgbaScale << "  "
                  << double(rgba[rgbIdx+3]) * rgbaScale * atten << "  "
                  << std::endl;

        rgbIdx  = 4 * 128;
        std::cerr << __LINE__ << " [Default Renderer] RGBA: " << 128 << "  "
                  << double(rgba[rgbIdx  ]) * rgbaScale << "  "
                  << double(rgba[rgbIdx+1]) * rgbaScale << "  "
                  << double(rgba[rgbIdx+2]) * rgbaScale << "  "
                  << double(rgba[rgbIdx+3]) * rgbaScale * atten << "  "
                  << std::endl;

        // Set the volume properties.
        volumeProp->SetColor(transFunc);
        volumeProp->SetScalarOpacity(opacity);
        volumeProp->IndependentComponentsOff();
        volumeProp->SetShade( props.atts.GetLightingFlag() );

        // Set ambient, diffuse, specular, and specular power (shininess).
        const double *matProp = props.atts.GetMaterialProperties();

        if (matProp != nullptr)
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

        curVolume->SetMapper(mapper);
        curVolume->SetProperty(volumeProp);
    }

    if( props.atts.GetOsprayEnabledFlag() )
    {
        vtkOSPRayRendererNode::SetRendererType("pathtracer", VTKRen);
        vtkOSPRayRendererNode::SetSamplesPerPixel(props.atts.GetOspraySpp(), VTKRen);
        vtkOSPRayRendererNode::SetAmbientSamples (props.atts.GetOsprayAoSamples(), VTKRen);
        vtkOSPRayRendererNode::SetMinContribution(props.atts.GetOsprayMinContribution(), VTKRen);
    }

    mapper->Render(VTKRen, curVolume);
}
