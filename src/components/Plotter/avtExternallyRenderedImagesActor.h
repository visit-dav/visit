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
//      one or more images (plots) in a VisWindow that are rendered by
//      some means outside of and unknown to the VisWindow. In theory,
//      the image(s) could be rendered by anything including a process
//      outside of VisIt such as another visualization tool. In
//      practice, the most common use case will be one or more
//      instances of engines doing scalable rendering.
//
//      ERI is an abbreviation we use for Externally Rendered Images
//      throughout all of the code having to do with the ERI actor.
//      After implementation began, it became apparent that a slightly
//      better name for this class would be
//      avtExternallyRenderedPlotsActor as the abstraction is somewhat
//      specific to visit's notion of a plot. For example, it is the
//      information about a plot that the ERI actor must pass in the
//      external render call back.
//
//      Ultimately, the ERI actor will live in harmony with all other
//      plot actors and with multiple engines. This means engines may
//      serve up Z-buffers so that multiple engine's images may be
//      composited together or so that a mixture of real goemetry from
//      some engines can be combined in the viewer's window with
//      externally rendered images from elsewhere.
//
//      However, in the initial implementation, the ERI actor will
//      behave in sort of an all or nothing way. When it is in the
//      scene, it will manage ALL plots via external rendering. Later,
//      this restriction will be relaxed as more of an understanding
//      of the proper behavior is determined.
//
//      The ERI actor will be used in such a way that it will be
//      notified of the beginning of a Render in the
//      VisWindow. VisWinPlots notifies the ERI actor of this in its
//      UpdateView method. When the ERI actor recieves the
//      PrepareForRender request, it will issue the external render
//      callback it was assigned by the VisWindow (which in the
//      typical use case ultimately comes from the Viewer) causing the
//      engine to render the plots, sending the image data back to the
//      viewer and then populate the ERI actor's vtk actor with the
//      rendered image. This will all happen at the START of
//      rendering. Finally, when the vtkRenderer passes through all
//      its actors to do all the rendering, the image data for the
//      externally rendered images will already be in the vtk actor
//      associated with the ERI actor.
//
//      Each plot under the management of the ERI actor has an id within
//      the context of the ERI actor. That id is really just an index into
//      a list of booleans used to indicate the visibility of the associated
//      plot. The AddInput method is used to add a plot to the list of
//      plots managed by the ERI actor.
//
//      The ERI actor is intended to be ready to handle external rendering
//      whenever the need arises. For these reasons, it typically always has
//      to be aware of other plots, etc. but doesn't always make external
//      render requests on behalf of the plots it knows about. Consequently,
//      the external rendering requests that the ERI actor emits can be
//      enabled and disabled. Regardless, whether external rendering requests
//      have been enabled or disabled, the ERI actor behaves the same in all
//      other respects.
//      
//      The image concept supported by this actor also supports
//      z-buffer data.  This is so because the image(s) associated
//      with this actor may be combined in a scene where geometry is
//      coming from other actors. In addition, multiple images may
//      come from multiple sources and have to be z-buffer
//      composited. That work should be handled by this actor.
//      Presently, it is not implemented.
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
    void                 DoExternalRender(avtDataObject_p &);

    // Used to add a particular plot's actor to the list of plots' actors
    // under the management of the ERI actor
    int                  AddInput(void);

    // used to send essential information to ERIA so that it will have that
    // information when it receives a render request
    void                 PrepareForRender(vtkCamera *);

    // used to control the visibility of one of many plots' actors that are 
    // under the management of the ERIA.
    bool                 SetInputsVisibility(const int inputId, const bool mode);
    bool                 GetInputsVisibility(const int inputId);

    // used to temporarily control visibility of ERIA while its in a window
    bool                 SetVisibility(const void *who);
    bool                 SetVisibility(const bool mode, const void *who);
    bool                 SetVisibility(const bool mode);
    bool                 GetVisibility();

    // used to enable and disable external render requests 
    bool                 EnableExternalRenderRequests(void);
    bool                 DisableExternalRenderRequests(void);

  private:

    // used to indicate if the ERIA is active or not
    bool                 makeExternalRenderRequests;

    // used to record each input's visibility
    std::vector<bool>    inputVisible;

    // used to manage caller's conditional visibility settings
    std::map<const void*,bool> visibilityWhenCalledBy;

    // used to indicate if the list of inputs has changed
    bool                 inputModified;

    // the actor that gets added to a renderer for a vis window that handles
    // ALL externally rendered images in that vis window. 
    vtkActor2D          *myActor;
    vtkImageMapper      *myMapper;
    vtkMatrix4x4        *lastMat;
    vtkImageData        *dummyImage;

    VisCallbackWithDob          *extRenderCallback;
    void                        *extRenderCallbackArgs;
};


#endif
