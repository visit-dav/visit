// ************************************************************************* //
//                     avtExternallyRenderedImagesActor.h                    //
// ************************************************************************* //

#ifndef AVT_EXTERNALLY_RENDERED_IMAGES_ACTOR_H
#define AVT_EXTERNALLY_RENDERED_IMAGES_ACTOR_H
#include <plotter_exports.h>
#include <map>
#include <vector>

#include <avtDataObject.h>
#include <VisCallback.h>


class     vtkActor2D;
class     vtkCamera;
class     vtkImageData;
class     vtkImageMapper;
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
    void                 PrepareForRender(void);

    // used to temporarily control visibility of ERIA while its in a window
    bool                 SetVisibility(const bool mode);
    bool                 GetVisibility();

    // used to enable and disable external render requests 
    bool                 EnableExternalRenderRequests(void);
    bool                 DisableExternalRenderRequests(void);

  private:

    // used to invoke the external render request
    void                 DoExternalRender(avtDataObject_p &);

    // used to indicate if the ERIA is active or not
    bool                 makeExternalRenderRequests;

    // the actor that gets added to a renderer for a vis window that handles
    // ALL externally rendered images in that vis window. 
    vtkActor2D          *myActor;
    vtkImageMapper      *myMapper;
    vtkImageData        *dummyImage;
    vtkImageData        *lastNonDummyImage;

    VisCallbackWithDob          *extRenderCallback;
    void                        *extRenderCallbackArgs;
};


#endif
