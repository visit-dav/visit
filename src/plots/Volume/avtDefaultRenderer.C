// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtDefaultRenderer.C                            //
// ************************************************************************* //

#include "avtDefaultRenderer.h"

#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRenderer.h>
#include <vtkVolumeProperty.h>
#include <vtkImageData.h>

#ifdef VISIT_OSPRAY
#include <vtkAutoInit.h>
// Ensure the object factory is correctly registered
VTK_MODULE_INIT(vtkRenderingOSPRay);
#endif

#include <VolumeAttributes.h>
#include <avtCallback.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

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
    VTKRen           = NULL;
    imageToRender    = NULL;

    volumeProp       = vtkVolumeProperty::New();
    mapper           = vtkSmartVolumeMapper::New();
    curVolume        = vtkVolume::New();
    transFunc        = vtkColorTransferFunction::New();
    opacity          = vtkPiecewiseFunction::New();

    resetColorMap    = false;
    useInterpolation = true;
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
    if (curVolume != NULL)
    {
        curVolume->Delete();
    }
    if (imageToRender != NULL)
    {
        imageToRender->Delete();
    }
    if (volumeProp != NULL)
    {
        volumeProp->Delete();
    }
    if (mapper != NULL)
    {
        mapper->Delete();
    }
    if (transFunc != NULL)
    {
        transFunc->Delete();
    }
    if (opacity != NULL)
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
//    If VTKRen is NULL, we can't render.
//
//    Alister Maguire, Wed Nov  4 07:29:17 PST 2020
//    Set the scalar opacity unit distance. This is basically setting
//    the sample distance for ray casting.
//
//    Kevin Griffin, Fri Apr 28 01::11:43 PM PST 2023
//    Exposed the render mode setting allowing it to be changed through the
//    volume plot attributes.
//
// ****************************************************************************

void
avtDefaultRenderer::Render(
    const avtVolumeRendererImplementation::RenderProperties &props,
    const avtVolumeRendererImplementation::VolumeData &volume)
{
    const char *mName = "avtDefaultRenderer::Render: ";

    if (VTKRen == NULL)
    {
        debug1 << mName << "Default Renderer: VTKRen is NULL!";
        EXCEPTION0(ImproperUseException);
    }

    //
    // 2D data has no volume, so we don't try to render this.
    //
    if (props.dataIs2D)
    {
        debug5 << mName << "Cannot perform volume rendering on "
            << "2D data... returning";
        return;
    }

    VolumeAttributes::RenderMode renderMode = props.atts.GetRenderMode();

    switch(renderMode)
    {
        case VolumeAttributes::RayCastRenderMode:
            // Use software rendering exclusively
            mapper->SetRequestedRenderModeToRayCast();
            break;
        case VolumeAttributes::GPURenderMode:
            // Use hardware accelerated rendering exclusively
            mapper->SetRequestedRenderModeToGPU();
            break;
        case VolumeAttributes::OSPRayRenderMode:
            // Use OSPRay to do software rendering
            mapper->SetRequestedRenderModeToOSPRay();
            break;
        default:
            // Best option to adapt to different data types, hardware, and
            // rendering parameters
            mapper->SetRequestedRenderModeToDefault();
    }

    if (imageToRender == NULL)
    {
        debug5 << mName << "Converting from rectilinear grid "
            << "to image data";

        //
        // Our mapper requires a vtkImageData as input. We must
        // create one.
        //
        int dims[3], extent[6];
        ((vtkRectilinearGrid *)volume.grid)->GetDimensions(dims);
        ((vtkRectilinearGrid *)volume.grid)->GetExtent(extent);

        //
        // We might be using a different scalar for opacity than
        // for color. We need to get both arrays.
        //
        vtkDataArray *dataArr = volume.data.data;
        vtkDataArray *opacArr = volume.opacity.data;

        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *)volume.grid;
        double spacingX = rgrid->GetXCoordinates()->GetTuple1(1)-
            rgrid->GetXCoordinates()->GetTuple1(0);
        double spacingY = rgrid->GetYCoordinates()->GetTuple1(1)-
            rgrid->GetYCoordinates()->GetTuple1(0);
        double spacingZ = rgrid->GetZCoordinates()->GetTuple1(1)-
            rgrid->GetZCoordinates()->GetTuple1(0);

        imageToRender = vtkImageData::New();
        imageToRender->SetDimensions(dims);
        imageToRender->SetExtent(extent);
        imageToRender->SetSpacing(spacingX, spacingY, spacingZ);
        imageToRender->AllocateScalars(VTK_FLOAT, 2);

        //
        // Set the origin to match the lower bounds of the grid
        //
        double bounds[6];
        ((vtkRectilinearGrid *)volume.grid)->GetBounds(bounds);
        imageToRender->SetOrigin(bounds[0], bounds[2], bounds[4]);

        float dataMag   = volume.data.max - volume.data.min;
        float opacMag   = volume.opacity.max - volume.opacity.min;

        //
        // We need to transfer the rgrid data over to the image data
        // and scale to the proper range.
        // VisIt populates empty space with the NO_DATA_VALUE.
        // We need to map this to a value that our mapper accepts,
        // and then clamp it out of vision.
        //
        int ptId = 0;
        for (int z = 0; z < dims[2]; ++z)
        {
            for (int y = 0; y < dims[1]; ++y)
            {
                for (int x = 0; x < dims[0]; ++x)
                {
                    //
                    // Our opacity and color data may differ. We
                    // need to add both as two separate components.
                    //
                    float dataTuple = dataArr->GetTuple1(ptId);
                    if (dataTuple <= NO_DATA_VALUE)
                    {
                        //
                        // Our color map is 0 -> 255. For no data values,
                        // assign a new value just out side of the map.
                        //
                        imageToRender->SetScalarComponentFromFloat(
                            x, y, z, 0, -1.0);
                        useInterpolation = false;
                    }
                    else
                    {
                        float numerator = 255.0 * (dataTuple - volume.data.min);
                        imageToRender->SetScalarComponentFromFloat(
                            x, y, z, 0, (numerator / dataMag));
                    }

                    float opacTuple = opacArr->GetTuple1(ptId);
                    if (opacTuple <= NO_DATA_VALUE)
                    {
                        //
                        // Our color map is 0 -> 255. For no data values,
                        // assign a new value just out side of the map.
                        //
                        imageToRender->SetScalarComponentFromFloat(
                            x, y, z, 1, -1.0);
                        useInterpolation = false;
                    }
                    else
                    {
                        float numerator = 255.0 *
                            (opacTuple - volume.opacity.min);
                        imageToRender->SetScalarComponentFromFloat(
                            x, y, z, 1, (numerator / opacMag));
                    }

                    ptId++;
                }
            }
        }

        debug5 << mName << "Adding data to the SmartVolumeMapper" << endl;

        mapper->SetInputData(imageToRender);
        mapper->SetScalarModeToUsePointData();
        mapper->SetBlendModeToComposite();
        resetColorMap = true;
    }

    if (resetColorMap || oldAtts != props.atts)
    {
        debug5 << mName << "Resetting color" << endl;

        //
        // Getting color/alpha transfer function from VisIt
        //
        unsigned char rgba[256*4];
        props.atts.GetTransferFunction(rgba);

        //
        // We don't want to see the no data values, so we turn clamping
        // off (opacity becomes 0.0)
        //
        transFunc->SetScaleToLinear();
        transFunc->SetClamping(0);
        opacity->SetClamping(0);

        float atten = props.atts.GetOpacityAttenuation() / 255.f;

        //
        // Add the color map to vtk's transfer function
        //
        for(int i = 0; i < 256; i++)
        {
            int rgbIdx  = 4 * i;
            float curOp = rgba[rgbIdx + 3] * atten;
            transFunc->AddRGBPoint(i, rgba[rgbIdx] / 255.f,
                rgba[rgbIdx + 1] / 255.f,
                rgba[rgbIdx + 2] / 255.f);
            opacity->AddPoint(i, MAX(0.0, MIN(1.0, curOp)));
        }

        //
        // For some reason, the endpoints aren't included when clamping
        // is turned off. So, let's put some padding on the ends of our
        // mapping functions.
        //
        int lastIdx = 255*4;
        transFunc->AddRGBPoint(-1, rgba[0] / 255.f,
            rgba[1] / 255.f,
            rgba[2] / 255.f);
        transFunc->AddRGBPoint(256, rgba[lastIdx] / 255.f,
            rgba[lastIdx + 1] / 255.f,
            rgba[lastIdx + 2] / 255.f);
        opacity->AddPoint(-1, rgba[3] * atten);
        opacity->AddPoint(256, rgba[lastIdx + 3] * atten);

        //
        // Set ambient, diffuse, specular, and specular power (shininess).
        //
        const double *matProp = props.atts.GetMaterialProperties();

        if (matProp != NULL)
        {
            volumeProp->SetAmbient(matProp[0]);
            volumeProp->SetDiffuse(matProp[1]);
            volumeProp->SetSpecular(matProp[2]);
            volumeProp->SetSpecularPower(matProp[3]);
        }

        //
        // Set the volume properties.
        //
        if (props.atts.GetLightingFlag())
        {
            volumeProp->SetShade(1);
        }
        else
        {
            volumeProp->SetShade(0);
        }

        volumeProp->SetScalarOpacity(opacity);
        volumeProp->SetColor(transFunc);
        volumeProp->IndependentComponentsOff();

        //
        // If our dataset contains NO_DATA_VALUEs, interpolation will
        // not work correctly on the boundaries (between a real value and
        // a no data value). Hopefully this will be addressed in the
        // future. For now, we only interpolate when our dataset contains
        // none of these values.
        //
        if (useInterpolation)
        {
            volumeProp->SetInterpolationTypeToLinear();
        }
        else
        {
            volumeProp->SetInterpolationTypeToNearest();
        }

        resetColorMap = false;
        oldAtts       = props.atts;

        //
        // We need to calculate the sample distance so that we can apply
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
        //
        double spacing[3];
        imageToRender->GetSpacing(spacing);

        double sampleDistReference = 1.0/10.0;
        double averageSpacing = (spacing[0] + spacing[1] + spacing[2]) / 3.0;
        double sampleDist     = averageSpacing / sampleDistReference;

        volumeProp->SetScalarOpacityUnitDistance(1, sampleDist);

        curVolume->SetMapper(mapper);
        curVolume->SetProperty(volumeProp);
    }

    mapper->Render(VTKRen, curVolume);
}
