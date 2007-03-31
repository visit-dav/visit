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
//
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
//
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
//
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
//
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
//
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
//
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
//
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


