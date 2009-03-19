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
//                      avtOpenGLTuvokVolumeRenderer.C                   //
// ************************************************************************* //

#include "avtOpenGLTuvokVolumeRenderer.h"

#include <vtkDataArray.h>
#include <vtkRectilinearGrid.h>
#include <avtViewInfo.h>
#include <VolumeAttributes.h>
#include <DebugStream.h>
#include <tuvok/../VisItDebugOut.h>
#include <tuvok/Controller/Controller.h>
#include <tuvok/IO/CoreVolume.h>

static AbstrRenderer* CreateRenderer(const VolumeAttributes &);

// ****************************************************************************
//  Method: avtOpenGLTuvokVolumeRenderer::avtOpenGLTuvokVolumeRenderer
//
//  Purpose:
//
//  Programmer:  Josh Stratton
//  Creation:    Wed Dec 17 15:00:34 MST 2008
//
//  Modifications:
//
//    Tom Fogal, Thu Mar  5 14:31:42 MST 2009
//    Connect the appropriate type of debug output.
//    NULL out our renderer, until we know what kind of one to make.
//
// ****************************************************************************

avtOpenGLTuvokVolumeRenderer::avtOpenGLTuvokVolumeRenderer()
{
    Controller::Instance().AddDebugOut(new VisItDebugOut());
    // enable tuvok logging output -- very slow, do not leave enabled!
    Controller::Debug::Out().SetOutput(true, true, true, true);
    this->renderer = NULL;
}

// ****************************************************************************
//  Method: avtOpenGLTuvokVolumeRenderer::~avtOpenGLTuvokVolumeRenderer
//
//  Purpose:
//
//  Programmer:  Josh Stratton
//  Creation:    Wed Dec 17 15:00:34 MST 2008
//
//  Tom Fogal, Thu Mar  5 14:35:43 MST 2009
//  Add renderer instance.
//  Tidy up after our renderer.
//
// ****************************************************************************
avtOpenGLTuvokVolumeRenderer::~avtOpenGLTuvokVolumeRenderer()
{
    if(this->renderer) {
        this->renderer->Cleanup();
        Controller::Instance().ReleaseVolumerenderer(this->renderer);
        this->renderer = NULL;
    }
}

// ****************************************************************************
//  Method:  avtOpenGLTuvokVolumeRenderer::Render
//
//  Purpose: Renders the dataset using Tuvok.
//
//  Arguments:
//    grid      : the data set to render
//    data,opac : the color/opacity variables
//    view      : the viewing information
//    atts      : the current volume plot attributes
//    vmin/max/size : the min/max/range of the color variable
//    omin/max/size : the min/max/range of the opacity variable
//    gx/gy/gz      : the gradient of the opacity variable
//    gmn           : the gradient magnitude, normalized to the max grad mag
//
//  Programmer:  Josh Stratton
//  Creation:    Wed Dec 17 15:00:34 MST 2008
//
//  Modifications:
//
//   Tom Fogal, Thu Mar  5 15:57:43 MST 2009
//   Create the underlying renderer.
//
// ****************************************************************************

void
avtOpenGLTuvokVolumeRenderer::Render(vtkRectilinearGrid *grid,
                                     vtkDataArray *data,
                                     vtkDataArray *opac,
                                     const avtViewInfo &view,
                                     const VolumeAttributes &atts,
                                     float vmin, float vmax, float vsize,
                                     float omin, float omax, float osize,
                                     float *gx, float *gy, float *gz,
                                     float *gmn, bool reducedDetail)
{
    if(NULL == this->renderer) {
        this->renderer = CreateRenderer(atts);
    }
}

// ****************************************************************************
//  Function: CreateRenderer
//
//  Purpose: Gets the appropriate type of renderer considering the information
//           given in the VolumeAttributes.
//
//  Returns:    The requested renderer, or NULL if request is nonsensical.
//
//  Programmer: Tom Fogal
//  Creation:   Thu Mar  5 14:55:14 MST 2009
//
//  Modifications:
//
//    Tom Fogal, Thu Mar  5 20:15:10 MST 2009
//    Initialize the renderer before returning it.
//
//    Tom Fogal, Fri Mar  6 10:39:04 MST 2009
//    Set an empty `CoreVolume' to the dataset.
//
// ****************************************************************************
static AbstrRenderer *
CreateRenderer(const VolumeAttributes &)
{
    // hack, for now -- always return a `bad' OGL SBVR.
    const bool use_only_PoT_textures = true;
    const bool downsample = false;
    const bool disable_border = false;

    MasterController &mc = Controller::Instance();
    AbstrRenderer *ren = mc.RequestNewVolumerenderer(
                            MasterController::OPENGL_SBVR,
                            use_only_PoT_textures, downsample,
                            disable_border);
    ren->SetDataSet(new CoreVolume());
    ren->Initialize();
    return ren;
}
