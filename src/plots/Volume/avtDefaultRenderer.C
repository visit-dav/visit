/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                           avtDefaultRenderer.C                            //
// ************************************************************************* //

#include "avtDefaultRenderer.h"

#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRenderer.h>
#include <vtkVolumeProperty.h>
#include <vtkImageData.h>
#include <vtkSmartVolumeMapper.h>

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

        curVolume->SetMapper(mapper);
        curVolume->SetProperty(volumeProp);
    }

    mapper->Render(VTKRen, curVolume);
}
