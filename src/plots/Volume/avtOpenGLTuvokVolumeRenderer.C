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
//
// ****************************************************************************

avtOpenGLTuvokVolumeRenderer::avtOpenGLTuvokVolumeRenderer()
{
    Controller::Instance().AddDebugOut(new VisItDebugOut());
}

// ****************************************************************************
//  Method: avtOpenGLTuvokVolumeRenderer::~avtOpenGLTuvokVolumeRenderer
//
//  Purpose:
//
//  Programmer:  Josh Stratton
//  Creation:    Wed Dec 17 15:00:34 MST 2008
//
// ****************************************************************************
avtOpenGLTuvokVolumeRenderer::~avtOpenGLTuvokVolumeRenderer()
{
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
}
