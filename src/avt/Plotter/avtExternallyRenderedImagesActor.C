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
//                    avtExternallyRenderedImagesActor.C                     //
// ************************************************************************* //

#include <signal.h>

#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>
#include <avtExternallyRenderedImagesActor.h>
#include <avtImage.h>
#include <avtNullData.h>
#include <avtOriginatingSource.h>

#include <float.h>

#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkImageMapper.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderer.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>

#include <ImproperUseException.h>
#include <DebugStream.h>
#include <TimingsManager.h>

#include <map>

using std::vector;
using std::map;

// ****************************************************************************
//  Function: GetCameraLeftEye 
//
//  Purpose: Access to private data member of vtkCamera necessary to support
//           Stereo rendering in SR mode. To do this, we create a simple
//           derived class of vtkCamera that has a public method to return
//           the left eye information from vtkCamera.
//
//  Programmer: Mark C. Miller 
//  Creation:   July 21, 2006 
//
// ****************************************************************************
class vtkVisItERIACamera : vtkCamera
{
  public:
    int GetLeftEye() const { return LeftEye; };
};
static bool GetCameraLeftEye(const vtkCamera *const vtkCam)
{
    const vtkVisItERIACamera *const eriaCam = (vtkVisItERIACamera*) vtkCam;
    return eriaCam->GetLeftEye() == 1;
}

// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   December 7, 2002
//
//  Modifications:
//
//    Mark C. Miller, Wed Mar 28 15:56:15 PDT 2007
//    Added stuff to support the 'in-progress' visual queue
//
// ****************************************************************************

avtExternallyRenderedImagesActor::avtExternallyRenderedImagesActor()
{
    myMapper              = vtkImageMapper::New();
    visualQueueMapper     = vtkTextMapper::New();
    visualQueueProps      = vtkTextProperty::New();
    dummyImage            = vtkImageData::New();

    visualQueueProps->SetJustificationToCentered();
    visualQueueProps->SetVerticalJustificationToCentered();
    visualQueueProps->BoldOn();
    visualQueueProps->SetFontSize(24);
    visualQueueProps->SetColor(0.0, 0.0, 0.0);

    visualQueueMapper->SetInput("");
    visualQueueMapper->SetTextProperty(visualQueueProps);

    visualQueueActor = vtkActor2D::New();
    visualQueueActor->SetMapper(visualQueueMapper);

    lastNonDummyImage     = NULL;
    myMapper->SetInput(dummyImage);
    myMapper->SetColorWindow(255);
    myMapper->SetColorLevel(127);
    myActor               = vtkActor2D::New();
    myActor->SetMapper(myMapper);
    extRenderCallback     = NULL;
    extRenderCallbackArgs = NULL;
    makeExternalRenderRequests = false;
    for (int i = 0; i < sizeof(renderTimeHistory)/sizeof(renderTimeHistory[0]); i++)
        renderTimeHistory[i] = 0.0;
    rtIdx = 0;
    doNextExternalRenderAsVisualQueue = false;
}


// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor destructor
//
//  Programmer: Mark C. Miller 
//  Creation:   December 7, 2002
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Jan  7 15:11:25 PST 2005
//    Fix memory leak -- delete dummyImage.
//
//    Mark C. Miller, Wed Mar 28 15:56:15 PDT 2007
//    Added stuff to support the 'in-progress' visual queue
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
    if (lastNonDummyImage != NULL)
    {
        lastNonDummyImage->Delete();
        lastNonDummyImage = NULL;
    }
    if (dummyImage != NULL)
    {
        dummyImage->Delete();
        dummyImage = NULL;
    }
    if (visualQueueMapper != NULL)
    {
        visualQueueMapper->Delete();
        visualQueueMapper = NULL;
    }
    if (visualQueueProps != NULL)
    {
        visualQueueProps->Delete();
        visualQueueProps = NULL;
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
//  Modifications:
//
//    Mark C. Miller, Fri Jul 21 08:05:15 PDT 2006
//    Build a buffer of args to pass multiple args to call back
//
//    Mark C. Miller, Wed Mar 28 15:56:15 PDT 2007
//    Added timing stuff to support the 'in-progress' visual queue
// ****************************************************************************
void
avtExternallyRenderedImagesActor::DoExternalRender(avtDataObject_p &dob,
    bool leftEye)
{
    static unsigned char argsBuf[256];
    int i = 0;

    // build buffer of args to pass to Viewer
    memcpy(&argsBuf[0], &extRenderCallbackArgs, sizeof(extRenderCallbackArgs));
    i += sizeof(extRenderCallbackArgs);
    memcpy(&argsBuf[i], &leftEye, sizeof(leftEye));

    if (extRenderCallback != NULL)
    {
        double startedExternalRenderAt = TOA_THIS_LINE;
        extRenderCallback(argsBuf, dob);
        double externalRenderTime = TOA_THIS_LINE - startedExternalRenderAt;
        renderTimeHistory[rtIdx++ % 5] = externalRenderTime;
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
//  Modifications:
//    Mark C. Miller, Fri Jul 21 08:05:15 PDT 2006
//    Added vtkCamera argument to support stereo SR mode
//
//    Mark C. Miller, Wed Mar 28 15:56:15 PDT 2007
//    Added logic to set the actor to the 'in-progress' visual queue for
//    a single render
// ****************************************************************************
 
void
avtExternallyRenderedImagesActor::PrepareForRender(const vtkCamera *const cam)
{
   // return early if we're not supposed to be making external render requests
   if (!makeExternalRenderRequests)
      return;

   // return early if we're NOT actually visible
   if (!GetVisibility())
      return;

   //
   // Handle rendering of the 'in-progress' visual queue
   //
   if (doNextExternalRenderAsVisualQueue)
   {
       doNextExternalRenderAsVisualQueue = false;
       visualQueueProps->SetColor(nextForegroundColor);
       visualQueueMapper->SetInput("Waiting for parallel rendering...");
       visualQueueActor->SetPosition((float) nextWidth * 0.5, (float) nextHeight * 0.025);
       return;
   }
   else
   {
       visualQueueMapper->SetInput("");
   }

   // issue the external rendering callback
   // we use the dummyData 'value' to detect if dob was updated by the
   // DoExternalRender call
   avtNullData_p dummyData = new avtNullData(NULL);
   avtDataObject_p dummyDob;
   CopyTo(dummyDob, dummyData);

   avtDataObject_p dob = dummyDob; 
   DoExternalRender(dob, GetCameraLeftEye(cam));

   if ((*dob != *dummyDob) && (*dob != NULL))
   {

      // make sure we've recieved an image and not something else
      if (strcmp(dob->GetType(),"avtImage") != 0)
      {
          EXCEPTION1(ImproperUseException, "Expected avtImage object"); 
      }

      // put up the new image 
      avtImage_p img;
      CopyTo(img, dob);
      avtImageRepresentation& imgRep = img->GetImage();
      myMapper->SetInput(imgRep.GetImageVTK());

      // update the last non dummy image
      if (lastNonDummyImage != NULL)
      {
          lastNonDummyImage->Delete();
          lastNonDummyImage = NULL;
      }
      lastNonDummyImage = imgRep.GetImageVTK();
      lastNonDummyImage->Register(NULL);
   }
   else
   {
      // if we got back nothing, put up the dummy (blank) image
      if (*dob == NULL)
         myMapper->SetInput(dummyImage);

      // if we got 'no-change' put up the last non dummy image
      if (*dob == *dummyDob)
         myMapper->SetInput(lastNonDummyImage);
   }
}


// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::SaveVisibility
//
//  Purpose:
//     Sets the current visibility of the ERI actor to specified state but
//     also remembers previous state and associates that previous state with
//     the given object pointer. Later a call to RestoreVisibilty will return
//     it to a the previous state.
//
//  Programmer: Mark C. Miller
//  Creation:   May 5, 2005 
//
// ****************************************************************************

bool
avtExternallyRenderedImagesActor::SaveVisibility(void *objptr, const bool mode)
{
    std::map<void*,bool>::iterator it = savedVisibilityMap.find(objptr);

    if (it != savedVisibilityMap.end())
    {
          EXCEPTION1(ImproperUseException,
              "Invalid Save/Restore visibility operation."); 
    }

    savedVisibilityMap[objptr] = GetVisibility();
    return SetVisibility(mode);
}

// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::RestoreVisibility
//
//  Purpose:
//     Sets the current visibility of the ERI actor to state it was in before
//     the specified object set it.
//
//  Programmer: Mark C. Miller
//  Creation:   May 5, 2005 
//
// ****************************************************************************

bool
avtExternallyRenderedImagesActor::RestoreVisibility(void *objptr)
{
    std::map<void*,bool>::iterator it = savedVisibilityMap.find(objptr);

    if (it == savedVisibilityMap.end())
    {
          EXCEPTION1(ImproperUseException,
              "Invalid Save/Restore visibility operation."); 
    }


    bool restoreMode = it->second;
    SetVisibility(restoreMode);
    savedVisibilityMap.erase(it);
    return restoreMode;
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
//    Mark C. Miller, Wed Mar 28 15:56:15 PDT 2007
//    Added a missing const qualification for the method itself 
// ****************************************************************************

bool
avtExternallyRenderedImagesActor::GetVisibility(void) const
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
//  Modifications:
//    Mark C. Miller, Wed Nov 15 23:04:02 PST 2006
//    Removed call to SetVisibility to zero
//
//    Dave Bremer, Wed Mar 21 15:49:29 PDT 2007
//    This method is also used to disable scalable rendering.  In this use,
//    the method also needs to stop drawing an image if it was drawing one
//    before.
//
//    Dave Bremer, Wed Oct 31 15:48:16 PDT 2007
//    Modified my previous patch to optionally clear the cached image.
// ****************************************************************************

bool
avtExternallyRenderedImagesActor::DisableExternalRenderRequests(bool bClearImage)
{
   bool oldMode = makeExternalRenderRequests;
   makeExternalRenderRequests = false;
   
   if (bClearImage)
       myMapper->SetInput(dummyImage);
   
   return oldMode; 
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
//  Modifications:
//    Mark C. Miller, Wed Nov 15 23:04:02 PST 2006
//    Removed call to SetVisibility to one 
// ****************************************************************************

bool
avtExternallyRenderedImagesActor::EnableExternalRenderRequests(void)
{
   bool oldMode = makeExternalRenderRequests;
   makeExternalRenderRequests = true;
   return oldMode;
}

// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::IsMakingExternalRenderRequsts
//
//  Purpose: Query if external render requests are being made
//
//  Programmer: Mark C. Miller
//  Creation:   March 27, 2007
// ****************************************************************************

bool
avtExternallyRenderedImagesActor::IsMakingExternalRenderRequests(void) const
{
   if (!makeExternalRenderRequests)
      return false;

   if (!GetVisibility())
      return false;

   return true;
}

// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::GetAverageRenderingTime
//
//  Purpose: Return the average rendering time for last 5 most recent renders 
//
//  Programmer: Mark C. Miller
//  Creation:   March 27, 2007
// ****************************************************************************

double
avtExternallyRenderedImagesActor::GetAverageRenderingTime(void) const
{
    double sum = 0.0;
    int nvals = rtIdx < 5 ? rtIdx : 5;
    for (int i = 0; i < nvals; i++)
        sum += renderTimeHistory[i];
    return sum / nvals;
}

// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::DoNextExternalRenderAsVisualQueue
//
//  Purpose: Set info to do only the next render using the visual queue 
//
//  Programmer: Mark C. Miller
//  Creation:   March 27, 2007
// ****************************************************************************

void
avtExternallyRenderedImagesActor::DoNextExternalRenderAsVisualQueue(
    int w, int h, const double *color)
{
    nextWidth = w;
    nextHeight = h;
    for (int i = 0; i < 3; i++)
        nextForegroundColor[i] = color[i];
    doNextExternalRenderAsVisualQueue = true;
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
    ren->AddActor(visualQueueActor);
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
    ren->RemoveActor(visualQueueActor);
}


// ****************************************************************************
//  Method: avtExternallyRenderedImagesActor::UseBlankImage
//
//  Purpose:
//      Tells the renderer to stop using the last rendered image, and start
//      using a blank image instead.
//
//  Programmer: Dave Bremer
//  Creation:   Wed Oct 31 17:28:39 PDT 2007
//
// ****************************************************************************

void
avtExternallyRenderedImagesActor::UseBlankImage()
{
    myMapper->SetInput(dummyImage);
}






