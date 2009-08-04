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
//                     avtExternallyRenderedImagesActor.h                    //
// ************************************************************************* //

#ifndef AVT_EXTERNALLY_RENDERED_IMAGES_ACTOR_H
#define AVT_EXTERNALLY_RENDERED_IMAGES_ACTOR_H
#include <plotter_exports.h>

#include <map>

#include <avtDataObject.h>
#include <VisCallback.h>

using std::map;

class     vtkActor2D;
class     vtkCamera;
class     vtkImageData;
class     vtkImageMapper;
class     vtkTextMapper;
class     vtkTextProperty;
class     vtkMatrix4x4;
class     vtkRenderer;

// ****************************************************************************
//  Class: avtExternallyRenderedImagesActor
//
//  Purpose:
//
//      An avtExternallyRenderedImagesActor is an actor representing
//      imagery to be included in a VisWindow but obtained (rendered)
//      by some means outside of and unknown to the VisWindow. In theory,
//      the image(s) could be rendered by anything including a process
//      outside of VisIt such as another visualization tool. In
//      practice, the most common use case will be one or more
//      instances of engines doing scalable rendering.
//
//      ERI is an abbreviation we use for Externally Rendered Images
//      throughout all of the code having to do with the ERI actor.
//
//      The ERI actor simply sits as the last actor in the VTK rendering
//      pipeline and waits to be activated during each render by VTK
//      with a call to PrepareForRender().
//
//      To fit, conceptually, within the abstraction of a VisWindow,
//      the ERI actor is necessarily dumb, and rightly so. The only
//      things it knows about is a callback function to retrieve
//      the externally rendered image and that it can sometimes be made
//      invisible and/or its external render requests temporarily
//      disabled.
//
//      The callback function which is a VisCallbackWithDob MUST set 
//      the avtDataObject_p it returns to deal with three key returns...
//
//         a. It MUST NOT CHANGE the avtDataObject_p if nothing has
//            changed from the last request. This is to deal with
//            cases where the image data already in the ERI actor
//            is the correct data to continue displaying.
//         b. an avtDataObject_p that points to NULL if there is no
//            image data to display. This is to deal with cases where
//            there is simply nothing to render.
//         c. an avtDataObject_p that points to non-NULL and is not
//            the same avtDataObject_p as was passed into the callback.
//            This is to deal with a new image.
//
//      The client which sets up the callback must deal with ALL OTHER
//      ISSUES associated with rendering the right data for the image.
//      
//      The image concept supported by this actor also supports
//      z-buffer data.  This is so because the image(s) associated
//      with this actor may be combined in a scene where geometry is
//      coming from other actors.
//
//      It is expected that there is only ever one of these actors in
//      a scene.  Like the avtTransparencyActor, this actor will
//      handle the work of all images that are externally rendered.
//
//      An avtExternallyRenderedImagesActor does NOT inherit from avtActor
//      because, like the avtTransparencyActor, it does not have a drawable.
//
//  Modifications:
//
//    Mark C. Miller, Wed Jun  8 11:03:31 PDT 2005
//    Added alternative visibility interface that remembers state of
//    visibility flag for each object that set it so that it can be
//    easily restored to its prior state 
//
//    Mark C. Miller, Fri Jul 21 08:05:15 PDT 2006
//    To support stereo SR mode, added vtkCamera argument to PrepareForRender
//
//    Mark C. Miller, Wed Mar 28 15:56:15 PDT 2007
//    Added IsMakingExternalRenderRequests, GetAverageExternalRenderingTime
//    and DoNextExternalRenderAsVisualQueue to support the 'in-progress'
//    visual queue for SR mode.
//
//    Dave Bremer, Wed Oct 31 15:48:16 PDT 2007
//    Added flag to DisableExternalRenderRequests() to clear the cached image.
//    DisableExternalRenderRequests() has a temporary use, in which you still
//    want to draw the cached image after re-enabling, and a longer term use, 
//    in which you need to remove the cached image so it doesn't get drawn 
//    over new, non-externally-rendered images, and this flag lets you 
//    distinguish those uses.
//    I also added the UseBlankImage() call and the visualQueueActor member,
//    to allow both the last image and the "Waiting..." message to be drawn
//    simultaneously, if we stepping through time, or doing a camera fly-by.
//    In the case of a wireframe drag and then a wait for the ERI, UseBlankImage()
//    is called by VisWinPlots to clear the last image while the "Waiting..."
//    message is displayed.
// ****************************************************************************

class PLOTTER_API avtExternallyRenderedImagesActor
{
  public:
                         avtExternallyRenderedImagesActor();
    virtual              ~avtExternallyRenderedImagesActor();

    // Used to add/remove the vtkActor represented by this avtActor
    // to a vtkRenderer
    void                 AddToRenderer(vtkRenderer *);
    void                 RemoveFromRenderer(vtkRenderer *);

    // Used to register the function that should be called when this actor
    // recieves a Render request to communicate with the external object
    // and obtain the externally rendered image.
    void                 RegisterExternalRenderCallback(
                             VisCallbackWithDob *cb, void *data);

    // used to send essential information to ERIA so that it will have that
    // information when it receives a render request
    void                 PrepareForRender(const vtkCamera *const cam);

    // used to temporarily control visibility of ERIA while its in a window
    bool                 SetVisibility(const bool mode);
    bool                 GetVisibility() const;

    // like Set/Get but used to set and restore to value before set
    bool                 SaveVisibility(void *theObj, const bool mode);
    bool                 RestoreVisibility(void *theObj);

    // used to enable and disable external render requests 
    bool                 EnableExternalRenderRequests(void);
    bool                 DisableExternalRenderRequests(bool bClearImage = false);

    bool                 IsMakingExternalRenderRequests(void) const;
    double               GetAverageRenderingTime(void) const;
    void                 DoNextExternalRenderAsVisualQueue(int width,
                             int height, const double *color);
    void                 UseBlankImage();

  private:

    // used to invoke the external render request
    void                 DoExternalRender(avtDataObject_p &, bool);

    // used to indicate if the ERIA is active or not
    bool                 makeExternalRenderRequests;

    // used by Save/Restore Visibility interface
    map<void*,bool>      savedVisibilityMap;

    // the actor that gets added to a renderer for a vis window that handles
    // ALL externally rendered images in that vis window. 
    vtkActor2D          *myActor;
    vtkImageMapper      *myMapper;
    vtkActor2D          *visualQueueActor;
    vtkTextMapper       *visualQueueMapper;
    vtkTextProperty     *visualQueueProps;
    vtkImageData        *dummyImage;
    vtkImageData        *lastNonDummyImage;

    VisCallbackWithDob          *extRenderCallback;
    void                        *extRenderCallbackArgs;

    double               renderTimeHistory[5];
    int                  rtIdx;
    bool                 doNextExternalRenderAsVisualQueue;
    int                  nextWidth;
    int                  nextHeight;
    double               nextForegroundColor[3];
};


#endif
