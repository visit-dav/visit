// ************************************************************************* //
//                               avtDrawer. C                                //
// ************************************************************************* //

#include <avtDrawer.h>

#include <vector>

#include <vtkCamera.h>
#include <vtkImageData.h>
//#include <vtkMesaRenderWindow.h>
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

//    vtkMesaRenderWindow *win = vtkMesaRenderWindow::New();
//    win->SetOffScreenRendering(1);
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
//  Method: avtDrawer::PerformRestriction
//
//  Purpose:
//    Calculates the domain list.  Does this by getting the spatial extents
//    and culling around the view.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

avtPipelineSpecification_p
avtDrawer::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = NULL;
    if (GetInput()->GetInfo().GetValidity().GetSpatialMetaDataPreserved())
    {
        vector<int> domains;
        avtIntervalTree *tree = GetMetaData()->GetSpatialExtents();
        avtWorldSpaceToImageSpaceTransform::GetDomainsList(viewInfo,
                                                           domains, tree);
        rv = new avtPipelineSpecification(spec);
        rv->GetDataSpecification()->GetRestriction()->RestrictDomains(domains);
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


