// ************************************************************************* //
//                    avtExternallyRenderedImagesActor.C                     //
// ************************************************************************* //

#include <signal.h>

#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>
#include <avtExternallyRenderedImagesActor.h>
#include <avtImage.h>
#include <avtTerminatingSource.h>

#include <float.h>

#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkImageMapper.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderer.h>
#include <vtkTIFFReader.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <TimingsManager.h>

using std::vector;
using std::map;

// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   December 7, 2002
//
// ****************************************************************************

avtExternallyRenderedImagesActor::avtExternallyRenderedImagesActor()
{
    myMapper              = vtkImageMapper::New();
    dummyImage            = vtkImageData::New();
    myMapper->SetColorWindow(255);
    myMapper->SetColorLevel(127);
    myActor               = vtkActor2D::New();
    myActor->SetMapper(myMapper);
    extRenderCallback     = NULL;
    extRenderCallbackArgs = NULL;
    makeExternalRenderRequests = false;
}


// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor destructor
//
//  Programmer: Mark C. Miller 
//  Creation:   December 7, 2002
//
// ****************************************************************************

avtExternallyRenderedImagesActor::~avtExternallyRenderedImagesActor()
{
    if (myActor != NULL)
    {
        myActor->Delete();
        myActor = NULL;
    }
    if (myMapper != NULL)
    {
        myMapper->Delete();
        myMapper = NULL;
    }
}


// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::RegisterExternalRenderCallback
//
//  Purpose:
//      Registers a callback that allows the ERIA to get a new image.
//
//  Programmer: Mark C. Miller
//  Creation:   January 13, 2003
//
// ****************************************************************************
 
void
avtExternallyRenderedImagesActor::RegisterExternalRenderCallback(
    VisCallbackWithDob *cb, void *data) 
{
   extRenderCallback     = cb;
   extRenderCallbackArgs = data;
}


// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::DoExternalRender
//
//  Purpose:
//      Make an explicit call to do external rendering 
//
//  Arguments:
//      index   The plot index.
//      dob     A place to put the data object.
//
//  Programmer: Mark C. Miller
//  Creation:   January 13, 2003
//
// ****************************************************************************
 
void
avtExternallyRenderedImagesActor::DoExternalRender(avtDataObject_p &dob)
{
    if (extRenderCallback != NULL)
    {
        extRenderCallback(extRenderCallbackArgs, dob);
    }
    else
    {
        debug1 << "Would like to have made an external render request, "
                  "but no callback was registered." << endl;
        dob = NULL;
    }
}


// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::PrepareForRender
//
//  Purpose:
//     This method is where all the communication with the engine, via the
//     external render callback, is done. When the VisWinPlots collegue of
//     the VisWindow recieves the UpdateView request, it turns around and
//     tells ERI actor to PrepareForRender.
//
//     Note: Issues having to do with skipping external render requests
//     when nothing has changed are to be handled in the external render
//     callback itself and NOT here. The ERIA simply doesn't have enough
//     information to make that determination.
//
//  Programmer: Mark C. Miller 
//  Creation:   January 9, 2003 
//
// ****************************************************************************
 
void
avtExternallyRenderedImagesActor::PrepareForRender(void)
{
   // return early if we're not supposed to be making external render requests
   if (!makeExternalRenderRequests)
      return;

   // return early if we're NOT actually visible
   if (!GetVisibility())
      return;

   // issue the external rendering callback
   // we play a trick with initialization of dob to create a useful
   // NULL-like pointer that we can distinguish from NULL itself
   int n = 2;
   avtDataObject_p dob = ref_ptr<avtDataObject>((avtDataObject*)1,&n);
   DoExternalRender(dob);

   if ((*dob != NULL) && (*dob != (avtDataObject*)1) && !strcmp(dob->GetType(),"avtImage"))
   {
      // we know this is really an avtImage object, so make one from it
      avtImage_p img;
      CopyTo(img, dob);
      avtImageRepresentation& imgRep = img->GetImage();
      myMapper->SetInput(imgRep.GetImageVTK());
   }
   else
   {
      if (*dob == NULL)
         myMapper->SetInput(dummyImage);
   }

}


// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::SetVisibility
//
//  Purpose:
//     Sets the current visibility of the ERI actor. Returns the old value.
//
//  Programmer: Mark C. Miller
//  Creation:   January 9, 2003
//
// ****************************************************************************

bool
avtExternallyRenderedImagesActor::SetVisibility(const bool mode)
{
    bool oldMode = myActor->GetVisibility()==0 ? false : true;
    if (mode)
       myActor->SetVisibility(1);
    else
       myActor->SetVisibility(0);
    return oldMode;
}


// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::GetVisibility
//
//  Purpose:
//      Returns the current visibility of the ERI actor 
//      navigate bounding box mode).
//
//  Programmer: Mark C. Miller
//  Creation:   January 9, 2003
//
// ****************************************************************************

bool
avtExternallyRenderedImagesActor::GetVisibility(void)
{
   return myActor->GetVisibility()==0 ? false : true;
}


// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::DisableExternalRenderRequests
//
//  Purpose:
//      Disables the external rendered images actor's external render requests.
//      Sometimes, the ERI actor needs to perform all of its functions EXCEPT
//      actually making the external render requests. This function can be used
//      to temporarily disable its external render requests.
//
//  Return: true if the external render request was already previously disabled 
//          false otherwise
//
//  Programmer: Mark C. Miller
//  Creation:   February 4, 2003
//
// ****************************************************************************

bool
avtExternallyRenderedImagesActor::DisableExternalRenderRequests(void)
{
   bool oldMode = makeExternalRenderRequests;
   makeExternalRenderRequests = false;
   SetVisibility(false);
   myMapper->SetInput(dummyImage);
   return oldMode ? false : true;
}

// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::EnableExternalRenderRequests(void)
//
//  Purpose:
//      Enables the external rendered images actor to make external requests.
//      Sometimes, the ERI actor needs to perform all of its functions EXCEPT
//      actually making the external render requests. This function can
//      re-enable its external render requests if it ever needs to be disabled.
//
//  Return: true if the external render request was already previously enabled
//          false otherwise
//
//  Programmer: Mark C. Miller
//  Creation:   February 4, 2003
//
// ****************************************************************************

bool
avtExternallyRenderedImagesActor::EnableExternalRenderRequests(void)
{
   bool oldMode = makeExternalRenderRequests;
   makeExternalRenderRequests = true;
   SetVisibility(true);
   return oldMode;
}

// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::AddToRenderer
//
//  Purpose:
//      Adds the VTK actor for this AVT actor to the specified renderer.
//
//  Arguments:
//      ren     The renderer to add this actor to.
//
//  Programmer: Mark C. Miller
//  Creation:   January 9, 2003
//
// ****************************************************************************

void
avtExternallyRenderedImagesActor::AddToRenderer(vtkRenderer *ren)
{
    ren->AddActor(myActor);
}


// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::RemoveFromRenderer
//
//  Purpose:
//      Removes the VTK actor for this AVT actor from the specified renderer.
//
//  Arguments:
//      ren     The renderer to remove this actor from.
//
//  Programmer: Mark C. Miller
//  Creation:   January 9, 2003
//
// ****************************************************************************

void
avtExternallyRenderedImagesActor::RemoveFromRenderer(vtkRenderer *ren)
{
    ren->RemoveActor(myActor);
}
