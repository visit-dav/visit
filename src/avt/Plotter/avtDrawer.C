/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                               avtDrawer. C                                //
// ************************************************************************* //

#include <avtDrawer.h>

#include <vector>

#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>

#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtWorldSpaceToImageSpaceTransform.h>

#include <NoInputException.h>


using     std::vector;


// ***************************************************************************
//  Method: avtDrawer constructor
//
//  Arguments:
//    map       The mapper this drawer should use.
//
//  Programmer: Kathleen Bonnell
//  Creation:   January 04, 2001
//
//  Modifications:
//    Hank Childs, Fri Jan  5 17:10:59 PST 2001
//    Modified arguments.
//
// ***************************************************************************

avtDrawer::avtDrawer(avtMapper *map)
{
    mapper = map;
    modified = true;
    windowSize[0] = windowSize[1] = 300;
    viewInfo.SetToDefault();
}


// ***************************************************************************
//  Method: avtDrawer destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   January 04, 2001 
//
// ***************************************************************************

avtDrawer::~avtDrawer()
{
    mapper = NULL;
}


// ****************************************************************************
//  Method: avtDrawer::SetWindowSize
//
//  Purpose:
//    Sets the size of the window. 
//
//  Inputs:
//    size    the size for the virtual render window.
//
//  Programmer: Kathleen Bonnell
//  Creation:   January 04, 2001 
//
// ****************************************************************************

void
avtDrawer::SetWindowSize(const int size [2] )
{
    SetWindowSize(size[0], size[1]);
}

  
// ****************************************************************************
//  Method: avtDrawer::SetWindowSize
//
//  Purpose:
//    Sets the size of the window. 
//
//  Inputs:
//    x         the width of the virtual render window.
//    y         the height of the virtual render window.
//
//  Programmer: Kathleen Bonnell
//  Creation:   January 04, 2001
//
// ****************************************************************************

void
avtDrawer::SetWindowSize(const int x, const int y )
{
    if (windowSize[0] != x || windowSize[1] != y)
    {
        windowSize[0] = x;
        windowSize[1] = y;
        modified = true;
    }
}


// ****************************************************************************
//  Method: avtDrawer::Execute
//
//  Purpose:  
//    Execute for this filter.  This means draw the picture and dump it into
//    an image.
//
//  Arguments:
//    dl        Unused. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   January 04, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Jan 12 09:38:17 PST 2001
//    Renamed from CreateImage.
//
// ****************************************************************************

void
avtDrawer::Execute(void)
{
    //
    // We need to set up a renderer and render window in order
    // to use vtkWindowToImageFilter with our drawables.
    //

    vtkRenderer *ren = vtkRenderer::New();
    mapper->SetInput(GetInput());
    avtDrawable_p drawable = mapper->GetDrawable();
    drawable->Add(ren);

    vtkCamera *camera = ren->GetActiveCamera();
    viewInfo.SetCameraFromView(camera);

    vtkRenderWindow *win = vtkRenderWindow::New();
    win->SetSize(windowSize);
    win->AddRenderer(ren);
    win->Render();

    vtkWindowToImageFilter *w2if = vtkWindowToImageFilter::New(); 
    w2if->SetInput(win);
    w2if->Update();

    //
    // Remove the drawable from our temporary render or it will stay forever
    //
    drawable->Remove(ren);

    SetOutputImage(w2if->GetOutput());

    //
    //  Delete temporaries.
    //
    w2if->Delete();
    win->Delete();
    ren->Delete();
}


// ****************************************************************************
//  Method: avtDrawer::SetViewInfo
//
//  Purpose:
//    Sets the view info to reflect the argument and updates the internal
//    information.
//
//  Arguments:
//    vI        The new view information.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    January 04, 2001 
//
//  Modifications:
//
// ****************************************************************************

void
avtDrawer::SetViewInfo(const avtViewInfo &vI)
{
    viewInfo = vI;
    modified = true;
}


// ****************************************************************************
//  Method: avtDrawer::SetViewInfo
//
//  Purpose:
//    Sets the view info from the vtkCamera argument and updates the internal
//    information.
//
//  Arguments:
//    vtkcam  The camera from which to set the view information.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    January 04, 2001 
//
//  Modifications:
//
// ****************************************************************************

void
avtDrawer::SetViewInfo(vtkCamera *vtkcam)
{
    viewInfo.SetViewFromCamera(vtkcam);
    modified = true;
}


// ****************************************************************************
//  Method: avtDrawer::ModifyContract
//
//  Purpose:
//    Calculates the domain list.  Does this by getting the spatial extents
//    and culling around the view.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
//  Modifications:
//
//    Hank Childs, Thu May 29 10:00:27 PDT 2008
//    Calculate aspect ratio and send in as an argument to domain culling
//    subroutine.
//
// ****************************************************************************

avtContract_p
avtDrawer::ModifyContract(avtContract_p spec)
{
    avtContract_p rv = NULL;
    if (GetInput()->GetInfo().GetValidity().GetSpatialMetaDataPreserved())
    {
        vector<int> domains;
        avtIntervalTree *tree = GetMetaData()->GetSpatialExtents();
        double aspect = 1.;
        if (windowSize[1] > 0)
        {
            aspect = (double)windowSize[0] / (double)windowSize[1];
        }
        avtWorldSpaceToImageSpaceTransform::GetDomainsList(viewInfo,
                                                        domains, tree, aspect);
        rv = new avtContract(spec);
        rv->GetDataRequest()->GetRestriction()->RestrictDomains(domains);
    }
    else
    {
        //
        // Our spatial extents tree is shot, so we can't narrow the list down.
        //
        rv = spec;
    }

    return rv;
}


