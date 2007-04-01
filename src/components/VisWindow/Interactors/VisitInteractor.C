// ************************************************************************* //
//                              VisitInteractor.C                            //
// ************************************************************************* //

#include <VisitInteractor.h>

#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

#include <VisWindow.h>

#include <DebugStream.h>


ViewCallback   VisitInteractor::viewCallback     = NULL;


// ****************************************************************************
//  Method: VisitInteractor constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:31:30 PST 2002
//    Initialized data members about which buttons are pushed.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002   
//    Initialized new data members Center, MotionFactor and OldX/OldY. 
//    Added call to UseTimersOn().
//
// ****************************************************************************

VisitInteractor::VisitInteractor(VisWindowInteractorProxy &p) : proxy(p)
{
    UseTimersOn();
    lastX = lastY = -1;
    lastCallbackX = lastCallbackY = -1;

    leftButtonDown   = false;
    middleButtonDown = false;
    rightButtonDown  = false;

    OldX = OldY = 0.0;
    Center[0] = Center[1] = 0.0;
    MotionFactor = 10.0;
}

// ****************************************************************************
//  Method: VisitInteractor destructor
//
//  Programmer: Hank Childs
//  Creation:   September 13, 2000
//
// ****************************************************************************

VisitInteractor::~VisitInteractor()
{
    //
    // This is not reference counted, so set the count to zero so we won't
    // have any error messages.
    //
    ReferenceCount = 0;
}

// ****************************************************************************
//  Method: VisitInteractor::RegisterViewCallback
//
//  Purpose:
//    Registers a callback that will be called whenever the view changes.
//
//  Arguments:
//    vc       The new view callback.
//
//  Programmer:  Hank Childs
//  Creation:    July 17, 2001
//
//  Modifications:
//    Hank Childs, Mon Aug 20 21:46:18 PDT 2001
//    Removed argument for registering arguments.
//
// ****************************************************************************

void
VisitInteractor::RegisterViewCallback(ViewCallback vc)
{
    viewCallback     = vc;
}

// ****************************************************************************
//  Method: VisitInteractor::OnMouseMove
//
//  Purpose:
//    Called whenever the mouse moves (and consequently the view changes).
//    This will capture the view if there is a callback to receive it.
//
//  Arguments:
//    ctrl      Whether the control button has been pushed.
//    shift     Whether the shift button has been pushed.
//    X         The X coordinate on the screen.
//    Y         The Y coordinate on the screen.
//
//  Programmer: Hank Childs
//  Creation:   July 17, 2001
//
//  Modifications:
//    Hank Childs, Mon Aug 20 21:46:18 PDT 2001
//    Removed view callback args and started passing a direct pointer to the
//    visualization window.
//
//    Eric Brugger, Tue Aug 21 09:56:49 PDT 2001
//    I removed the avtViewInfo argument from viewCallback.
//
//    Kathleen Bonnell, Fri Dec 13 16:13:00 PST 2002 
//    Removed arguments to reflect vtk's new interactor api. Removed call
//    to parent class' OnMouseMove method. 
//
// ****************************************************************************

void
VisitInteractor::OnMouseMove()
{
    Interactor->GetEventPosition(lastX, lastY);
}

// ****************************************************************************
//  Method: VisitInteractor::MotionBegin
//
//  Purpose:
//      Tells outside modules that we are about to start render interaction.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2002
//
// ****************************************************************************

void
VisitInteractor::MotionBegin(void)
{
    proxy.MotionBegin();
}

// ****************************************************************************
//  Method: VisitInteractor::MotionEnd
//
//  Purpose:
//      Tells outside modules that we are about to end render interaction.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2002
//
//  Modifications:
//    Eric Brugger, Thu Nov 20 15:10:46 PST 2003
//    Remove the call to the view callback and have the derived classes do
//    it where appropriate since it was getting called too often.
//
// ****************************************************************************

void
VisitInteractor::MotionEnd(void)
{
    proxy.MotionEnd();
}

// ****************************************************************************
//  Method: VisitInteractor::IssueViewCallback
//
//  Purpose:
//      Issues a view callback when the view has changed.  This goes for the
//      granularity of when a button is pushed.  Intermediate views should
//      not be registered.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
// ****************************************************************************

void
VisitInteractor::IssueViewCallback(void)
{
    if (viewCallback != NULL)
    {
        if (lastX != lastCallbackX || lastY != lastCallbackY)
        {
            VisWindow *vw = proxy;
            viewCallback(vw);
            lastCallbackX = lastX;
            lastCallbackY = lastY;
        }
    }
}

// ****************************************************************************
//  Method: VisitInteractor::PrepTrackball
//
//  Purpose:
//    This should be called whenever a button is pushed or released.  It
//    prepares the trackball by updating the internal state, finding the
//    current camera, and indicating it is in preprocess mode.  All of these
//    things were modeled after a button press for the trackball.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//    Values are now accessed from the RenderWindowInteractor.
//
// ****************************************************************************

void
VisitInteractor::PrepTrackball()
{
    int x, y;
    Interactor->GetEventPosition(x, y);
    OldX = x;
    OldY = y;

    FindPokedRenderer(x, y);
    Center[0] = CurrentRenderer->GetCenter()[0];
    Center[1] = CurrentRenderer->GetCenter()[1];
}

// ****************************************************************************
//  Method: VisitInteractor::OnLeftButtonDown
//
//  Purpose:
//    Handles the left button being pushed down.  For VisitInteractor,
//    this means do nothing.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:31:30 PST 2002
//    Extended for new button press routines so that derived types can be
//    better controlled.
//
//    Hank Childs, Thu Jul 11 17:34:48 PDT 2002
//    Add better reporting of when rendering interactions start and end.
//
//    Brad Whitlock, Mon Sep 23 11:22:52 PDT 2002
//    I moved the call to MotionBegin into the hotpoint interactor so it
//    is not called twice.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::OnLeftButtonDown()
{
    PrepTrackball();
    NewButtonPress();
    leftButtonDown = true;
    StartLeftButtonAction();
}

// ****************************************************************************
//  Method: VisitInteractor::OnLeftButtonUp
//
//  Purpose:
//    Handles the left button being released.  For VisitInteractor,
//    this means do nothing.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:31:30 PST 2002
//    Extended for new button press routines so that derived types can be
//    better controlled.
//
//    Hank Childs, Thu Jul 11 17:34:48 PDT 2002
//    Add better reporting of when rendering interactions start and end.
//
//    Brad Whitlock, Mon Sep 23 11:22:52 PDT 2002
//    I moved the call to MotionEnd into the hotpoint interactor so it
//    is not called twice.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::OnLeftButtonUp()
{
    PrepTrackball();
    if (leftButtonDown)
    {
        EndLeftButtonAction();
    }
    leftButtonDown = false;
}

// ****************************************************************************
//  Method: VisitInteractor::OnMiddleButtonDown
//
//  Purpose:
//    Handles the middle button being pushed down.  For VisitInteractor, this
//    means do nothing.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:31:30 PST 2002
//    Extended for new button press routines so that derived types can be
//    better controlled.
//
//    Hank Childs, Thu Jul 11 17:34:48 PDT 2002
//    Add better reporting of when rendering interactions start and end.
//
//    Brad Whitlock, Mon Sep 23 11:22:52 PDT 2002
//    I moved the call to MotionBegin into the hotpoint interactor so it
//    is not called twice.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::OnMiddleButtonDown()
{
    PrepTrackball();
    NewButtonPress();
    middleButtonDown = true;
    StartMiddleButtonAction();
}

// ****************************************************************************
//  Method: VisitInteractor::OnMiddleButtonUp
//
//  Purpose:
//    Handles the middle button being released.  For VisitInteractor, this
//    means do nothing.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:31:30 PST 2002
//    Extended for new button press routines so that derived types can be
//    better controlled.
//
//    Hank Childs, Thu Jul 11 17:34:48 PDT 2002
//    Add better reporting of when rendering interactions start and end.
//
//    Brad Whitlock, Mon Sep 23 11:22:52 PDT 2002
//    I moved the call to MotionBegin into the hotpoint interactor so it
//    is not called twice.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::OnMiddleButtonUp()
{
    PrepTrackball();
    if (middleButtonDown)
    {
        EndMiddleButtonAction();
    }
    middleButtonDown = false;
}

// ****************************************************************************
//  Method: VisitInteractor::OnRightButtonDown
//
//  Purpose:
//    Handles the right button being pushed down.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:31:30 PST 2002
//    Extended for new button press routines so that derived types can be
//    better controlled.
//
//    Hank Childs, Thu Jul 11 17:34:48 PDT 2002
//    Add better reporting of when rendering interactions start and end.
//
//    Brad Whitlock, Mon Sep 23 11:25:05 PDT 2002
//    I removed the call to MotionBegin since we use the right button for
//    the menu and having it generate a MotionBegin "event" is misleading.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::OnRightButtonDown()
{
    PrepTrackball();

    NewButtonPress();
    rightButtonDown = true;
    StartRightButtonAction();
}

// ****************************************************************************
//  Method: VisitInteractor::OnRightButtonUp
//
//  Purpose:
//    Handles the right button being released.  For VisitInteractor, this
//    means a popup window.  This is currently not implemented.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:31:30 PST 2002
//    Extended for new button press routines so that derived types can be
//    better controlled.
//
//    Hank Childs, Thu Jul 11 17:34:48 PDT 2002
//    Add better reporting of when rendering interactions start and end.
//
//    Brad Whitlock, Mon Sep 23 11:25:05 PDT 2002
//    I removed the call to MotionBegin since we use the right button for
//    the menu and having it generate a MotionBegin "event" is misleading.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::OnRightButtonUp()
{
    PrepTrackball();
    if (rightButtonDown)
    {
        EndRightButtonAction();
    }
    rightButtonDown = false;
}

// ****************************************************************************
//  Method: VisitInteractor::OnChar
//
//  Purpose:
//    Redefines the virtual function that handles characters.  This 
//    disregards all keystrokes.
//
//  Arguments:
//    <unnused>  Non-zero if the ctrl button is being held down.
//    <unnused>  Non-zero if the shift button is being held down.
//    <unnused>  The keycode.
//    <unnused>  The repeat count for that keycode.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::OnChar()
{
}

// ****************************************************************************
//  Method: VisitInteractor::NewButtonPress
//
//  Purpose:
//      A routine that performs a check to make sure that a new button press is
//      acceptable -- that is it checks to see if we are in a consistent state
//      and aborts other button presses to guarantee that if necessary.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::NewButtonPress()
{
    if (leftButtonDown)
    {
        debug1 << "Aborting left button action in response to new "
               << "button press." << endl;
        AbortLeftButtonAction();
    }

    if (middleButtonDown)
    {
        debug1 << "Aborting middle button action in response to new "
               << "button press." << endl;
        AbortMiddleButtonAction();
    }

    if (rightButtonDown)
    {
        debug1 << "Aborting right button action in response to new "
               << "button press." << endl;
        AbortRightButtonAction();
    }

    leftButtonDown   = false;
    middleButtonDown = false;
    rightButtonDown  = false;
}

// ****************************************************************************
//  Method: VisitInteractor::StartLeftButtonAction
//
//  Purpose:
//      A virtual function that is a hook for the base class, allowing it to
//      control the action if the left button is started.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::StartLeftButtonAction()
{
    ;
}

// ****************************************************************************
//  Method: VisitInteractor::AbortLeftButtonAction
//
//  Purpose:
//      A virtual function that is a hook for the base class, allowing it to
//      control the action if the left button action is aborted.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::AbortLeftButtonAction()
{
    EndLeftButtonAction();
}

// ****************************************************************************
//  Method: VisitInteractor::EndLeftButtonAction
//
//  Purpose:
//      A virtual function that is a hook for the base class, allowing it to
//      control the action if the left button action is ended.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::EndLeftButtonAction()
{
    ;
}

// ****************************************************************************
//  Method: VisitInteractor::StartMiddleButtonAction
//
//  Purpose:
//      A virtual function that is a hook for the base class, allowing it to
//      control the action if the middle button is started.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::StartMiddleButtonAction()
{
    ;
}

// ****************************************************************************
//  Method: VisitInteractor::AbortMiddleButtonAction
//
//  Purpose:
//      A virtual function that is a hook for the base class, allowing it to
//      control the action if the middle button action is aborted.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::AbortMiddleButtonAction()
{
    EndMiddleButtonAction();
}

// ****************************************************************************
//  Method: VisitInteractor::EndMiddleButtonAction
//
//  Purpose:
//      A virtual function that is a hook for the base class, allowing it to
//      control the action if the middle button action is ended.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::EndMiddleButtonAction()
{
    ;
}

// ****************************************************************************
//  Method: VisitInteractor::StartRightButtonAction
//
//  Purpose:
//      A virtual function that is a hook for the base class, allowing it to
//      control the action if the right button is started.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Nov 3 13:35:16 PST 2000
//    I added code to tell the VisWindow to show its menu.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::StartRightButtonAction()
{
    VisWindow *vw = proxy;
    vw->ShowMenu();
}

// ****************************************************************************
//  Method: VisitInteractor::AbortRightButtonAction
//
//  Purpose:
//      A virtual function that is a hook for the base class, allowing it to
//      control the action if the right button action is aborted.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::AbortRightButtonAction()
{
    EndRightButtonAction();
}

// ****************************************************************************
//  Method: VisitInteractor::EndRightButtonAction
//
//  Purpose:
//      A virtual function that is a hook for the base class, allowing it to
//      control the action if the right button action is ended.
//
//  Arguments:
//    ctrl      Non-zero if the ctrl button is being held down.
//    shift     Non-zero if the shift button is being held down.
//    x         The x coordinate of the mouse in display coordinates.
//    y         The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
VisitInteractor::EndRightButtonAction()
{
    VisWindow *vw = proxy;
    vw->HideMenu();
}

// ****************************************************************************
//  Method: VisitInteractor::StartBoundingBox
//
//  Purpose:
//      A convenience routine to derived types that want to start bounding
//      box mode.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2002
//
// ****************************************************************************

void
VisitInteractor::StartBoundingBox(void)
{
    //
    // If bounding box mode is enabled, then turn on the bounding box.
    //
    if (proxy.GetBoundingBoxMode())
    {
        proxy.StartBoundingBox();
    }
}

// ****************************************************************************
//  Method: VisitInteractor::EndBoundingBox
//
//  Purpose:
//      A convenience routine to derived types that want to end bounding
//      box mode.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2002
//
//  Modifications:
//    Eric Brugger, Wed Oct  8 08:44:20 PDT 2003
//    I removed a call to IssueViewCallback since MotionEnd also called it.
//
// ****************************************************************************

void
VisitInteractor::EndBoundingBox(void)
{
    //
    // We do not issue a view callback here, since MotionEnd is always
    // called and it will issue a view callback. Doing so here would cause
    // a duplicate entry to be added to the view stack messing up the
    // undo view operation.
    //

    //
    // If bounding box mode is enabled, then we turned on the bounding box,
    // before, so we should turn it off now.
    //
    if (proxy.GetBoundingBoxMode())
    {
        proxy.EndBoundingBox();
    }
}

// ****************************************************************************
//  Method: Navigate2D::ZoomCamera
//
//  Purpose:
//    Handle zooming the camera.
//
//  Programmer: Eric Brugger
//  Creation:   October 10, 2003
//
// ****************************************************************************

void
VisitInteractor::ZoomCamera2D(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if (OldY != y)
    {
        //
        // Calculate the zoom factor.
        //
        double dyf = MotionFactor * (double)(y - OldY) /
                         (double)(Center[1]);
        double zoomFactor = pow((double)1.1, dyf);

        //
        // Calculate the new parallel scale.
        //
        VisWindow *vw = proxy;
        bool fillViewportOnZoom = vw->GetInteractorAtts()->GetFillViewportOnZoom();

        avtView2D newView2D = vw->GetView2D();

        double xDist = newView2D.window[1] - newView2D.window[0];
        double yDist = newView2D.window[3] - newView2D.window[2];
        double dX = ((1. / zoomFactor) - 1.) * (xDist / 2.);
        double dY = ((1. / zoomFactor) - 1.) * (yDist / 2.);

        //
        // If fill viewport on zoom is enabled and we are zooming and not
        // in fullframe mode then zoom so that we will fill the viewport.
        //
        if (fillViewportOnZoom && zoomFactor > 1. && !newView2D.fullFrame)
        {
            //
            // Determine the y scale factor to account for the viewport
            // and window size.
            //
            int       size[2];
            double    yScale;

            rwi->GetSize(size);

            yScale = ((newView2D.viewport[3] - newView2D.viewport[2]) /
                      (newView2D.viewport[1] - newView2D.viewport[0])) *
                     ((double) size[1] / (double) size[0]) ;

            //
            // We fill the viewport by only zooming one of the axes.  In
            // the case where we will overshoot, we zoom the second axis a
            // small amount so that we will just fill the viewport.
            //
            if ((yDist / xDist) < yScale)
            {
                //
                // Handle the case where the x direction should be zoomed.
                //
                if ((xDist + 2.0 * dX) > (yDist / yScale))
                    dY = 0.;
                else
                    dY = ((xDist + 2.0 * dX) * yScale - yDist) / 2.0;
            }
            else
            {
                //
                // Handle the case where the x direction should be zoomed.
                //
                if ((yDist + 2.0 * dY) > (xDist * yScale))
                    dX = 0.;
                else
                    dX = ((yDist + 2.0 * dY) / yScale - xDist) / 2.0;
            }
        }

        newView2D.window[0] -= dX;
        newView2D.window[1] += dX;
        newView2D.window[2] -= dY;
        newView2D.window[3] += dY;

        vw->SetView2D(newView2D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: VisitInteractor::PanImage3D
//
//  Purpose:
//    Handle panning the image in 3d.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
//  Modifications:
//    Eric Brugger, Tue Apr  2 11:28:11 PST 2002
//    I modified the routine to make the viewing changes using an avtView3D.
//
//    Eric Brugger, Wed Jun 11 08:55:07 PDT 2003
//    I changed the pan behavior so that it pans the image and doesn't
//    change the camera or focal point.
//
// ****************************************************************************

void
VisitInteractor::PanImage3D(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if ((OldX != x) || (OldY != y))
    {
        //
        // Determine the size of the window.
        //
        int       size[2];

        rwi->GetSize(size);

        //
        // Get the current view information.
        //
        VisWindow *vw = proxy;
        double    pan[2];

        avtView3D newView3D = vw->GetView3D();

        pan[0] = ((double)((x - OldX) / size[0])) / newView3D.imageZoom;
        pan[1] = ((double)((y - OldY) / size[1])) / newView3D.imageZoom;

        newView3D.imagePan[0] += pan[0];
        newView3D.imagePan[1] += pan[1];

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: VisitInteractor::PanCamera3D
//
//  Purpose:
//    Handle panning the camera in 3d.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

void
VisitInteractor::PanCamera3D(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if ((OldX != x) || (OldY != y))
    {
        //
        // Determine the size of the window.
        //
        int       size[2];
        int       width, height;

        rwi->GetSize(size);
        width = size[0];
        height = size[1];

        //
        // Get the current view information.
        //
        VisWindow *vw = proxy;

        const avtView3D &oldView3D = vw->GetView3D();

        avtVector VPN(oldView3D.normal);
        avtVector VUP(oldView3D.viewUp);
        avtVector Focus(oldView3D.focus);

        //
        // Calculate the coordinate transformation from the world coordinate
        // space to the screen coordinate space.
        //
        avtVector VUPCrossVPN;
        avtMatrix mat, matTranspose;

        VUPCrossVPN = VUP % VPN;

        mat[0][0] = VUPCrossVPN.x;
        mat[0][1] = VUPCrossVPN.y;
        mat[0][2] = VUPCrossVPN.z;
        mat[0][3] = 0.;
        mat[1][0] = VUP.x;
        mat[1][1] = VUP.y;
        mat[1][2] = VUP.z;
        mat[1][3] = 0.;
        mat[2][0] = VPN.x;
        mat[2][1] = VPN.y;
        mat[2][2] = VPN.z;
        mat[2][3] = 0.;
        mat[3][0] = 0.;
        mat[3][1] = 0.;
        mat[3][2] = 0.;
        mat[3][3] = 1.;

        matTranspose = mat;
        matTranspose.Transpose();

        //
        // Calculate the x and y pan distances in the view plane.
        //
        avtVector pan;

        pan.x = ((double)((OldX - x) / height)) *
                 2.0 * oldView3D.parallelScale;
        pan.y = ((double)((OldY - y) / height)) *
                 2.0 * oldView3D.parallelScale;
        pan.z = 0.;

        //
        // Transform the screen space pan factors to world space pan factors.
        //
        avtVector pan2;

        pan2 = matTranspose * pan;

        //
        // Set the new origin.
        //
        avtView3D newView3D = vw->GetView3D();

        newView3D.focus[0] += pan2.x;
        newView3D.focus[1] += pan2.y;
        newView3D.focus[2] += pan2.z;

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: VisitInteractor::ZoomImage3D
//
//  Purpose:
//    Handle zooming the image in 3d.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
//  Modifications:
//    Hank Childs, Tue Feb 26 10:22:43 PST 2002
//    Do not let the near plane be set to something very close to the camera,
//    because the viewing transformation gets out of whack and we lose too
//    much in depth buffering.
//
//    Eric Brugger, Tue Apr  2 11:28:11 PST 2002
//    I modified the routine to make the viewing changes using an avtView3D.
//
//    Eric Brugger, Wed Jun 11 08:55:07 PDT 2003
//    I changed the zoom behavior so that it zooms the image and doesn't
//    change the camera or focal point.
//
// ****************************************************************************

void
VisitInteractor::ZoomImage3D(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if (OldY != y)
    {
        //
        // Calculate the zoom factor.
        //
        double dyf = MotionFactor * (double)(y - OldY) /
                         (double)(Center[1]);
        double zoomFactor = pow((double)1.1, dyf);

        //
        // Calculate the new parallel scale.
        //
        VisWindow *vw = proxy;

        avtView3D newView3D = vw->GetView3D();

        newView3D.imageZoom = newView3D.imageZoom * zoomFactor;

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: VisitInteractor::DollyCameraTowardFocus3D
//
//  Purpose:
//    Handle dollying the camera toward the focus in 3d.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2004
//
// ****************************************************************************

void
VisitInteractor::DollyCameraTowardFocus3D(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if (OldY != y)
    {
        //
        // Calculate the zoom factor.
        //
        double dyf = MotionFactor * (double)(y - OldY) /
                         (double)(Center[1]);
        double zoomFactor = pow((double)1.1, dyf);

        //
        // Calculate the new parallel scale.
        //
        VisWindow *vw = proxy;

        avtView3D newView3D = vw->GetView3D();

        newView3D.parallelScale = newView3D.parallelScale / zoomFactor;

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: VisitInteractor::DollyCameraAndFocus3D
//
//  Purpose:
//    Handle dollying the camera and focus toward the focus in 3d.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

void
VisitInteractor::DollyCameraAndFocus3D(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if (OldY != y)
    {
        //
        // Calculate the zoom factor.
        //
        double dyf = -1.0 * MotionFactor * (double)(y - OldY) /
                         (double)(Center[1]);

        //
        // Calculate the new parallel scale.
        //
        VisWindow *vw = proxy;

        avtView3D newView3D = vw->GetView3D();

        newView3D.focus[0] +=
            newView3D.normal[0] * newView3D.parallelScale * dyf;
        newView3D.focus[1] +=
            newView3D.normal[1] * newView3D.parallelScale * dyf;
        newView3D.focus[2] +=
            newView3D.normal[2] * newView3D.parallelScale * dyf;

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: VisitInteractor::RotateAboutFocus3D
//
//  Purpose:
//    Handle rotating the camera about the focus in 3d.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
//  Modifications:
//    Eric Brugger, Tue Apr  2 11:28:11 PST 2002
//    I modified the routine to make the viewing changes using an avtView3D.
//
//    Hank Childs, Wed May 29 10:50:12 PDT 2002
//    Initialized data members needed for determining which direction to spin.
//
//    Eric Brugger, Tue Feb 10 08:41:08 PST 2004
//    I modified the routine to rotate about the center of rotation if one
//    is specified.
//
//    Eric Brugger, Wed Dec 29 10:54:02 PST 2004
//    I added useCenter, a flag that forces the routine to rotate about the
//    center of rotation if set.
// 
// ****************************************************************************

void
VisitInteractor::RotateAboutFocus3D(const int x, const int y,
    const bool useCenter)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if ((OldX != x) || (OldY != y))
    {
        spinOldX = OldX;
        spinOldY = OldY;
        spinNewX = x;
        spinNewY = y;

        int       size[2];
        int       width, height;
        double    unitX, unitY;
        double    unitPrevX, unitPrevY;

        rwi->GetSize(size);
        width = size[0];
        height = size[1];

        unitX = (2.0 * x) / width - 1.0;
        unitY = 1.0 - (2.0 * y) / height;

        unitPrevX = (2.0 * OldX) / width - 1.0;
        unitPrevY = 1.0 - (2.0 * OldY) / height;

        //
        // Build the quaternion and convert it to a rotation matrix.
        // The sign is flipped on the 3 component of the quaternion
        // to get the rotations about the screen z axis in the correct
        // direction.  The first component of the quaternion contains
        // x rotation information, the second component contains y
        // rotation information, and the third component contains z
        // rotation information.  The fourth component controlls the
        // magnitude of the rotation, the closer to 1 the smaller.
        // The quaternion will be of unit length.
        //
        avtMatrix rot;

        rot.MakeTrackball(unitPrevX, unitPrevY, unitX, unitY, true);
        rot.Transpose();

        //
        // Get the current view information.
        //
        VisWindow *vw = proxy;

        const avtView3D &oldView3D = vw->GetView3D();

        avtVector VPN(oldView3D.normal);
        avtVector VUP(oldView3D.viewUp);
        avtVector Focus(oldView3D.focus);

        //
        // Calculate the coordinate transformation from the world coordinate
        // space to the screen coordinate space (and its inverse).
        //
        avtVector VPNCrossVUP;
        avtMatrix mata, mataInverse;

        VPNCrossVUP = VPN % VUP;

        mata[0][0] = VPNCrossVUP.x;
        mata[0][1] = VPNCrossVUP.y;
        mata[0][2] = VPNCrossVUP.z;
        mata[0][3] = 0.;
        mata[1][0] = VUP.x;
        mata[1][1] = VUP.y;
        mata[1][2] = VUP.z;
        mata[1][3] = 0.;
        mata[2][0] = VPN.x;
        mata[2][1] = VPN.y;
        mata[2][2] = VPN.z;
        mata[2][3] = 0.;
        mata[3][0] = 0.;
        mata[3][1] = 0.;
        mata[3][2] = 0.;
        mata[3][3] = 1.;

        mataInverse = mata;
        mataInverse.Transpose();

        //
        // Calculate the translation to the center of rotation (and its
        // inverse).
        //
        avtMatrix matb, matbInverse;

        matb.MakeTranslate(-oldView3D.centerOfRotation[0],
                           -oldView3D.centerOfRotation[1],
                           -oldView3D.centerOfRotation[2]);
        matbInverse.MakeTranslate(oldView3D.centerOfRotation[0],
                                  oldView3D.centerOfRotation[1],
                                  oldView3D.centerOfRotation[2]);

        //
        // Calculate the composite transformation.
        //
        avtMatrix mat;

        mat = matbInverse * mataInverse * rot * mata * matb;

        //
        // Calculate the new view plane normal, view up vector and focus.
        //
        avtVector newVPN(mat ^ VPN);
        avtVector newVUP(mat ^ VUP);
        avtVector newFocus(mat * Focus);

        //
        // Orthogonalize the new view plane normal and view up vector.
        //
        newVPN.normalize();
        VPNCrossVUP = newVPN % newVUP;
        newVUP = VPNCrossVUP % newVPN;
        newVUP.normalize();

        //
        // Set the new view.
        //
        avtView3D newView3D = vw->GetView3D();

        newView3D.normal[0] = newVPN.x;
        newView3D.normal[1] = newVPN.y;
        newView3D.normal[2] = newVPN.z;
        newView3D.viewUp[0] = newVUP.x;
        newView3D.viewUp[1] = newVUP.y;
        newView3D.viewUp[2] = newVUP.z;
        if (oldView3D.centerOfRotationSet || useCenter)
        {
            newView3D.focus[0]  = newFocus.x;
            newView3D.focus[1]  = newFocus.y;
            newView3D.focus[2]  = newFocus.z;
        }

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: VisitInteractor::RotateAboutCamera3D
//
//  Purpose:
//    Handle rotating the focus about the camera in 3d.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

void
VisitInteractor::RotateAboutCamera3D(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if ((OldX != x) || (OldY != y))
    {
        spinOldX = OldX;
        spinOldY = OldY;
        spinNewX = x;
        spinNewY = y;

        int       size[2];
        int       width, height;
        double    unitX, unitY;
        double    unitPrevX, unitPrevY;

        rwi->GetSize(size);
        width = size[0];
        height = size[1];

        unitX = 1.0 * ((2.0 * x) / width - 1.0);
        unitY = 1.0 * (1.0 - (2.0 * y) / height);

        unitPrevX = 1.0 * ((2.0 * OldX) / width - 1.0);
        unitPrevY = 1.0 * (1.0 - (2.0 * OldY) / height);

        //
        // Build the quaternion and convert it to a rotation matrix.
        // The sign is flipped on the 3 component of the quaternion
        // to get the rotations about the screen z axis in the correct
        // direction.  The first component of the quaternion contains
        // x rotation information, the second component contains y
        // rotation information, and the third component contains z
        // rotation information.  The fourth component controlls the
        // magnitude of the rotation, the closer to 1 the smaller.
        // The quaternion will be of unit length.
        //
        avtMatrix rot;

        rot.MakeTrackball(unitPrevX, unitPrevY, unitX, unitY, true);
        rot.Transpose();

        //
        // Get the current view information.
        //
        VisWindow *vw = proxy;

        const avtView3D &oldView3D = vw->GetView3D();

        avtVector VPN(oldView3D.normal);
        avtVector VUP(oldView3D.viewUp);
        avtVector Focus(oldView3D.focus);

        //
        // Calculate the coordinate transformation from the world coordinate
        // space to the screen coordinate space (and its inverse).
        //
        avtVector VPNCrossVUP;
        avtMatrix mata, mataInverse;

        VPNCrossVUP = VPN % VUP;

        mata[0][0] = VPNCrossVUP.x;
        mata[0][1] = VPNCrossVUP.y;
        mata[0][2] = VPNCrossVUP.z;
        mata[0][3] = 0.;
        mata[1][0] = VUP.x;
        mata[1][1] = VUP.y;
        mata[1][2] = VUP.z;
        mata[1][3] = 0.;
        mata[2][0] = VPN.x;
        mata[2][1] = VPN.y;
        mata[2][2] = VPN.z;
        mata[2][3] = 0.;
        mata[3][0] = 0.;
        mata[3][1] = 0.;
        mata[3][2] = 0.;
        mata[3][3] = 1.;

        mataInverse = mata;
        mataInverse.Transpose();

        //
        // Calculate the composite transformation.
        //
        avtMatrix mat;

        mat = mataInverse * rot * mata;

        //
        // Calculate the new view plane normal, view up vector and focus.
        //
        avtVector camera;

        double distance = oldView3D.parallelScale /
                          tan(oldView3D.viewAngle * 3.1415926535 / 360.);
        camera.x = oldView3D.focus[0] + oldView3D.normal[0] * distance;
        camera.y = oldView3D.focus[1] + oldView3D.normal[1] * distance;
        camera.z = oldView3D.focus[2] + oldView3D.normal[2] * distance;

        avtVector newVPN(mat ^ VPN);
        avtVector newVUP(mat ^ VUP);
        avtVector newFocus(camera - newVPN * distance);

        //
        // Orthogonalize the new view plane normal and view up vector.
        //
        newVPN.normalize();
        VPNCrossVUP = newVPN % newVUP;
        newVUP = VPNCrossVUP % newVPN;
        newVUP.normalize();

        //
        // Set the new view.
        //
        avtView3D newView3D = vw->GetView3D();

        newView3D.normal[0] = newVPN.x;
        newView3D.normal[1] = newVPN.y;
        newView3D.normal[2] = newVPN.z;
        newView3D.viewUp[0] = newVUP.x;
        newView3D.viewUp[1] = newVUP.y;
        newView3D.viewUp[2] = newVUP.z;
        newView3D.focus[0]  = newFocus.x;
        newView3D.focus[1]  = newFocus.y;
        newView3D.focus[2]  = newFocus.z;

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: VisitInteractor::SetInteractor
//
//  Purpose:
//      Associate this 'style' with the passed render window interactor. 
//
//  Arguments:
//      rwi     The render window interactor.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   November 25, 2002 
//
// ****************************************************************************

void
VisitInteractor::SetInteractor(vtkRenderWindowInteractor *rwi)
{
    //
    //  The base class adds observers, but we simply want
    //  to set the RenderWindowInteractor.
    //
    Interactor = rwi;
}
