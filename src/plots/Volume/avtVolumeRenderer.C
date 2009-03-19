/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                             avtVolumeRenderer.C                           //
// ************************************************************************* //

#include "avtVolumeRenderer.h"
#include <visit-config.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <math.h>
#include <float.h>

#include <avtCallback.h>
#include <avtOpenGLSplattingVolumeRenderer.h>
#include <avtMesaSplattingVolumeRenderer.h>
#include <avtOpenGL3DTextureVolumeRenderer.h>
#include <avtMesa3DTextureVolumeRenderer.h>
#include <avtOpenGLTuvokVolumeRenderer.h>
#ifdef HAVE_LIBSLIVR
#include <avtOpenGLSLIVRVolumeRenderer.h>
#endif

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidLimitsException.h>
#include <StackTimer.h>
#include <VolumeFunctions.h>


// ****************************************************************************
//  Constructor:  avtVolumeRenderer::avtVolumeRenderer
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 30 11:44:21 PDT 2003
//    Pulled out the reference to alphatex.  It belonged in the subclass.
//
//    Brad Whitlock, Thu Jan 10 14:44:42 PST 2008
//    Added reducedDetail.
//
//    Brad Whitlock, Mon Dec 15 13:31:55 PST 2008
//    I removed histogramming code.
//
// ****************************************************************************
avtVolumeRenderer::avtVolumeRenderer()
{
    initialized = false;

    rendererImplementation = NULL;
    currentRendererIsValid = false;
    reducedDetail = false;

    gx  = NULL;
    gy  = NULL;
    gz  = NULL;
    gm  = NULL;
    gmn = NULL;
}

// ****************************************************************************
//  Destructor:  avtVolumeRenderer::~avtVolumeRenderer
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 30 11:44:21 PDT 2003
//    Pulled out the reference to alphatex.  It belonged in the subclass.
//    Added a call to ReleaseGraphicsResources.
//
// ****************************************************************************
avtVolumeRenderer::~avtVolumeRenderer()
{
    ReleaseGraphicsResources();
    delete[] gx;
    delete[] gy;
    delete[] gz;
    delete[] gm;
    delete[] gmn;
}

// ****************************************************************************
//  Method:  avtVolumeRenderer::ReleaseGraphicsResources
//
//  Purpose:
//    
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  1, 2003
//
// ****************************************************************************
void
avtVolumeRenderer::ReleaseGraphicsResources()
{
    if (rendererImplementation)
    {
        VTKRen->GetRenderWindow()->MakeCurrent();
        delete rendererImplementation;
        rendererImplementation = NULL;
    }
}

// ****************************************************************************
//  Method: avtVolumeRenderer::New
//
//  Purpose:
//      A static method that creates the correct type of renderer based on
//      whether we should use OpenGL or Mesa.
//
//  Returns:    A renderer that is of type derived from this class.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 30 11:47:16 PDT 2003
//    Renamed the renderers to contain "Splatting" in their name.
//
// ****************************************************************************
avtVolumeRenderer *
avtVolumeRenderer::New(void)
{
    return new avtVolumeRenderer;
}

// ****************************************************************************
// Method: avtVolumeRenderer::ReducedDetailModeOn
//
// Purpose: 
//   Turns on reduced detail mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 22 11:55:40 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtVolumeRenderer::ReducedDetailModeOn()
{
    reducedDetail = true;
}

// ****************************************************************************
// Method: avtVolumeRenderer::ReducedDetailModeOff
//
// Purpose: 
//   Turns off reduced detail mode.
//
// Returns:    True if we're using the SLIVR renderer; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 22 11:55:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtVolumeRenderer::ReducedDetailModeOff()
{
    reducedDetail = false;
#ifdef HAVE_LIBSLIVR
    return (atts.GetRendererType() == VolumeAttributes::SLIVR);
#else
    return false;
#endif
}

// ****************************************************************************
//  Method:  avtVolumeRenderer::Render
//
//  Purpose:
//    Set up things necessary to call the renderer implentation.  Make a new
//    implementation object if things have changed.
//
//  Arguments:
//    ds         the dataset to render
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  1, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Dec  3 20:37:07 PST 2005
//    Change test for whether or not we are doing software rendering.
//
//    Brad Whitlock, Thu Jan 10 14:42:59 PST 2008
//    Added support for SLIVR.
//
//    Brad Whitlock, Mon Dec 15 13:30:45 PST 2008
//    I removed histogramming and changed the code to call helpers.
//
//    Tom Fogal, Thu Mar  5 18:56:05 MST 2009
//    Add debug stmts detailing which VolumeRenderer we instantiate.
//
// ****************************************************************************

void
avtVolumeRenderer::Render(vtkDataSet *ds)
{
    StackTimer t("avtVolumeRenderer::Render");

    if (!currentRendererIsValid || !rendererImplementation)
    {
        if (rendererImplementation)
            delete rendererImplementation;

        if (avtCallback::GetSoftwareRendering())
        {
            if (atts.GetRendererType() == VolumeAttributes::Splatting)
            {
                debug5 << "Creating a (Mesa) Splatting renderer." << std::endl;
                rendererImplementation = new avtMesaSplattingVolumeRenderer;
            }
#ifdef HAVE_LIBSLIVR
            else if(atts.GetRendererType() == VolumeAttributes::SLIVR)
            {
                debug5 << "Creating a (Mesa) 3DTexture renderer." << std::endl;
                rendererImplementation = new avtMesa3DTextureVolumeRenderer;
                avtCallback::IssueWarning("SLIVR is not currently supported for "
                    "offscreen rendering. VisIt is reverting to 3D texturing.");
            }
#endif
            else if(atts.GetRendererType() == VolumeAttributes::Tuvok)
            {
                // Tuvok may work fine in Mesa but for now use previous texture slicer
                debug5 << "Creating a (Mesa) 3DTexture renderer." << std::endl;
                rendererImplementation = new avtMesa3DTextureVolumeRenderer;
                avtCallback::IssueWarning("Tuvok is not currently supported for "
                    "offscreen rendering. VisIt is reverting to 3D texturing.");
            }
            else // it == VolumeAttributes::Texture3D
            {
                debug5 << "Creating a (Mesa) 3DTexture renderer." << std::endl;
                rendererImplementation = new avtMesa3DTextureVolumeRenderer;
            }
        }
        else
        { 
            if (atts.GetRendererType() == VolumeAttributes::Splatting)
            {
                debug5 << "Creating a (HW) Splatting renderer." << std::endl;
                rendererImplementation = new avtOpenGLSplattingVolumeRenderer;
            }
#ifdef HAVE_LIBSLIVR
            else if(atts.GetRendererType() == VolumeAttributes::SLIVR)
            {
                debug5 << "Creating a (HW) SLIVR renderer." << std::endl;
                rendererImplementation = new avtOpenGLSLIVRVolumeRenderer;
            }
#endif
            else if(atts.GetRendererType() == VolumeAttributes::Tuvok)
            {
                debug5 << "Creating a (HW) Tuvok renderer." << std::endl;
                rendererImplementation = new avtOpenGLTuvokVolumeRenderer;
            }
            else // it == VolumeAttributes::Texture3D
            {
                debug5 << "Creating a (HW) 3DTexture renderer." << std::endl;
                rendererImplementation = new avtOpenGL3DTextureVolumeRenderer;
            }
        }
        currentRendererIsValid = true;
    }

    if (!initialized)
    {
        Initialize(ds);
    }

    // get data set
    vtkRectilinearGrid  *grid = (vtkRectilinearGrid*)ds;
    vtkDataArray *data = NULL;
    vtkDataArray *opac = NULL;
    bool haveScalars = VolumeGetScalars(atts, ds, data, opac);

    if (haveScalars)
    {
        StackTimer t2("Implementation Render");
        rendererImplementation->Render(grid, data, opac, view, atts,
                                       vmin, vmax, vsize,
                                       omin, omax, osize,
                                       gx, gy, gz, gmn, reducedDetail);
    }
}


// ****************************************************************************
//  Method:  avtVolumeRenderer::Initialize
//
//  Purpose:
//    Calculate some one-time stuff with respect to the data set
//
//  Arguments:
//    ds      : the data set
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Nov 13 11:31:57 PST 2001
//    Made it use the Sobel gradient operator by default since it looks so 
//    much better.  Fixed the Sobel operator to work with ghost values.
//
//    Hank Childs, Mon Nov 19 08:49:55 PST 2001
//    Used the opacity variable to calculate gradients.
//
//    Hank Childs, Mon Nov 19 15:34:44 PST 2001
//    Use the original variable extents when coloring.
//
//    Hank Childs, Wed Dec 12 10:54:58 PST 2001
//    Allow for variable extents to be artificially set.
//
//    Kathleen Bonnell, Mon Nov 19 16:05:37 PST 2001 
//    VTK 4.0 API changes require use of vtkDataArray in place of
//    vtkScalars for rgrid coordinates. 
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Jeremy Meredith, Tue Sep 30 11:47:41 PDT 2003
//    Only calculate the gradient if lighting is enabled *and* if it does
//    not already exist.  The SetAtts method will delete it if it is invalid.
//
//    Jeremy Meredith, Thu Oct  2 13:15:27 PDT 2003
//    Choose the gradient method based on settings from the user.  Changed
//    the ghost method to avoid ghosts entirely.
//
//    Jeremy Meredith, Wed Dec 29 10:10:59 PST 2004
//    I was missing some checks in the non-ghost centered diff gradient
//    calculation to make sure we didn't walk off the edge of the mesh.
//
//    Kathleen Bonnell, Fri Mar  4 13:55:09 PST 2005 
//    Account for Log scaling when determining vmin and vmax. 
//
//    Hank Childs, Tue Feb  6 15:39:01 PST 2007
//    Treat the values for "min" and "max" as min and max, not as log(min),
//    log(max).  That is 6 was being treated as 10^6.  No longer.
//
//    Brad Whitlock, Mon Dec 15 13:24:12 PST 2008
//    I moved most of the code to helper functions.
//
// ****************************************************************************

void
avtVolumeRenderer::Initialize(vtkDataSet *ds)
{
    StackTimer t("avtVolumeRenderer::Initialize");

    // get data set
    vtkRectilinearGrid  *grid = (vtkRectilinearGrid*)ds;
    vtkDataArray *data = 0, *opac = 0;
    if(!VolumeGetScalars(atts, ds, data, opac))
        return;

    // Get the volume variable's extents.
    VolumeGetVariableExtents(atts, data,
        this->varmin, this->varmax, 
        this->vmin, this->vmax, this->vsize);

    // Get the opacity variable's extents.
    VolumeGetOpacityExtents(atts, opac,
        this->omin, this->omax, this->osize);

    // calculate gradient
    if (atts.GetLightingFlag() &&
        gx == 0) // make sure the gradient was invalidated first
    { 
        int dims[3];
        grid->GetDimensions(dims);

        int nels = dims[0] * dims[1] * dims[2];
        gx  = new float[nels];
        gy  = new float[nels];
        gz  = new float[nels];
        gm  = new float[nels];
        gmn = new float[nels];
        float ghostval = omax+osize;
        VolumeCalculateGradient(atts, grid, opac, gx, gy, gz, gm, gmn, ghostval);
    }

    data->Delete();
    opac->Delete();
    initialized = true;
} 

bool
avtVolumeRenderer::GetScalars(vtkDataSet *ds, vtkDataArray *&d, vtkDataArray *&o)
{
    return VolumeGetScalars(atts, ds, d, o);
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
//    Hank Childs, Wed Dec 12 11:15:56 PST 2001
//    Re-initialize if the atts change, so new extents can be set.
//
//    Jeremy Meredith, Tue Sep 30 11:48:46 PDT 2003
//    Make sure we need to invalidate the gradient before doing so.
//
//    Jeremy Meredith, Thu Oct  2 13:16:15 PDT 2003
//    Added support for multiple renderer types.
//
//    Hank Childs, Mon Nov 14 09:54:54 PST 2005
//    If the attributes are the same as what we had before, do nothing.
//
// ****************************************************************************


void
avtVolumeRenderer::SetAtts(const AttributeGroup *a)
{
    const VolumeAttributes *newAtts = (const VolumeAttributes*)a;
    if (*newAtts == atts)
        return;
    currentRendererIsValid = (atts.GetRendererType() == newAtts->GetRendererType());

#if 0
    bool invalidateGradient = !(atts.GradientWontChange(*newAtts));
#else
    bool invalidateGradient = atts.ChangesRequireRecalculation(*newAtts);
#endif
    // Clean up memory.
    if (invalidateGradient)
    {
        if (gx != NULL)
        {
            delete[] gx;
            gx = NULL;
        }
        if (gy != NULL)
        {
            delete[] gy;
            gy = NULL;
        }
        if (gz != NULL)
        {
            delete[] gz;
            gz = NULL;
        }
        if (gm != NULL)
        {
            delete[] gm;
            gm = NULL;
        }
        if (gmn != NULL)
        {
            delete[] gmn;
            gmn = NULL;
        }
    }

    atts = *(const VolumeAttributes*)a;

    initialized = false;
}
