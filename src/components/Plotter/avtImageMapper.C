// ************************************************************************* //
//                             avtImageMapper.C                              //
// ************************************************************************* //

#include <avtImageMapper.h>

#include <vtkActor2D.h>
#include <vtkImageData.h>
#include <vtkImageMapper.h>
#include <vtkRenderer.h>

#include <avtImageDrawable.h>


// ****************************************************************************
//  Method: avtImageMapper constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

avtImageMapper::avtImageMapper()
{
    mapper = vtkImageMapper::New();
    mapper->SetColorWindow(255);
    mapper->SetColorLevel(127);
    actor  = vtkActor2D::New();
    actor->SetMapper(mapper);
    drawable = new avtImageDrawable(actor);
}


// ****************************************************************************
//  Method: avtImageMapper destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

avtImageMapper::~avtImageMapper()
{
    if (mapper != NULL)
    {
        mapper->Delete();
    }
    if (actor != NULL)
    {
        actor->Delete();
    }
}


// ****************************************************************************
//  Method: avtImageMapper::ChangedInput
//
//  Purpose:
//      This is a hook from avtImageSink to tell its derived types (us) that
//      the input has changed.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

void
avtImageMapper::ChangedInput(void)
{
    vtkImageData *img = GetImageRep().GetImageVTK();
    mapper->SetInput(img);
}


// ****************************************************************************
//  Method: avtImageMapper::Render
//
//  Purpose:
//      Renders an image to the screen.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2001
//
//  Modifications:
//      Mark C. Miller, 29Jan03
//      Due to fact that vtkActor2D now does all its relevant rendering work
//      in the RenderOverlay call, I added that call here.
//      
//  
// ****************************************************************************

void
avtImageMapper::Draw(vtkRenderer *ren)
{
    actor->RenderOverlay(ren);
    actor->RenderOpaqueGeometry(ren);
    actor->RenderTranslucentGeometry(ren);
}


