// ************************************************************************* //
//                              ViewerAnimation.C                            //
// ************************************************************************* //

#include <ViewerAnimation.h>

#include <iostream.h>
#include <stdio.h>

#include <ViewerEngineManager.h>
#include <ViewerFileServer.h>
#include <ViewerPlotList.h>
#include <ViewerWindowManager.h>
#include <ViewerWindow.h>
#include <ImproperUseException.h>
#include <DebugStream.h>
#include <LostConnectionException.h>
#include <ViewerQueryManager.h>
#include <avtCallback.h>
#include <avtDatabaseMetaData.h>

// ****************************************************************************
//  Method: ViewerAnimation constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
//  Modifications:
//    Eric Brugger, Fri Oct 26 12:19:44 PDT 2001
//    I added the mode member.
//
//    Eric Brugger, Wed Nov 21 10:47:42 PST 2001
//    I added the pipelineCaching mode.
//
// ****************************************************************************

ViewerAnimation::ViewerAnimation()
{
    nFrames  = 1;
    curFrame = 0;

    plotList = new ViewerPlotList(this);

    nWindows      = 0;
    nWindowsAlloc = 0;
    windows       = 0;

    mode = StopMode;

    pipelineCaching = false;
}

// ****************************************************************************
//  Method: ViewerAnimation destructor
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
// ****************************************************************************

ViewerAnimation::~ViewerAnimation()
{
    //
    // Delete any allocated class members.  We aren't deleting the windows
    // since they are being used elsewhere.
    //
    delete plotList;
    if (nWindowsAlloc > 0)
    {
        delete [] windows;
    }
}

// ****************************************************************************
// Method: ViewerAnimation::CopyFrom
//
// Purpose: 
//   Sets the attributes of this animation using the attributes of another
//   animation.
//
// Arguments:
//   anim      : The source animation.
//   copyPlots : Whether or not plots should be copied.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 12 11:00:52 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerAnimation::CopyFrom(const ViewerAnimation *anim, bool copyplots)
{
    if(anim)
    {
        if(anim->nFrames > nFrames)
            nFrames = anim->nFrames;
        if(anim->curFrame > curFrame)
            curFrame = anim->curFrame;

        mode = StopMode;
        pipelineCaching = anim->pipelineCaching;

        if(copyplots)
            plotList->CopyFrom(anim->GetPlotList());
    }
}

// ****************************************************************************
//  Method: ViewerAnimation::SetNFrames
//
//  Purpose:
//    Set the number of frames in the animation.
//
//  Arguments:
//    nFrames   The number of frames in the animation.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
// ****************************************************************************

void
ViewerAnimation::SetNFrames(const int nFrames)
{
    this->nFrames = nFrames;

    //
    // If the current frame is out of range of the new number of frames,
    // make the current frame the last frame.
    //
    if (curFrame >= nFrames)
    {
        SetFrameIndex(nFrames - 1);
    }
}

// ****************************************************************************
//  Method: ViewerAnimation::GetNFrames
//
//  Purpose:
//    Return the number of frames in the animation.
//
//  Returns:    The number of frames in the animation.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
// ****************************************************************************

int
ViewerAnimation::GetNFrames() const
{
    return nFrames;
}

// ****************************************************************************
//  Method: ViewerAnimation::SetFrameIndex
//
//  Purpose:
//    Set the current frame to the specified 0 origin frame index.
//
//  Arguments:
//    index     The 0 origin frame index.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
//  Modifications:
//    Eric Brugger, Wed Nov 21 10:47:42 PST 2001
//    I added the pipelineCaching mode.
//
//    Brad Whitlock, Tue Nov 12 11:51:31 PDT 2002
//    I made the routine return a bool indicating whether or not the frame
//    index was accepted.
//
//    Eric Brugger, Fri Nov 15 16:17:08 PST 2002
//    I added support for keyframing.
//
//    Kathleen Bonnell, Fri Feb 28 10:56:25 PST 2003  
//    Made the windows clear their pick points when the frame index changes. 
//
//    Eric Brugger, Wed Apr 23 14:08:33 PDT 2003
//    I added a call for each window associated with this animation to
//    to merge the current plot limits with the previous plot limits.
//
// ****************************************************************************

bool
ViewerAnimation::SetFrameIndex(const int index)
{
    if (index < 0 || index > nFrames - 1)
    {
        debug1 << "ViewerAnimation::SetFrameIndex: index out of range.\n";
        return false;
    }

    if (curFrame != index)
    {
        for(int i = 0; i < nWindows; ++i)
            windows[i]->ClearPickPoints();
    }

    //
    // Clear the pipeline if pipeline caching is off.
    //
    if (!pipelineCaching)
    {
        plotList->ClearPipelines(curFrame, curFrame);
    }

    curFrame = index;

    //
    // If we are in keyframe mode, then update the plot attributes in
    // the client.
    //
    if (plotList->GetKeyframeMode())
    {
        plotList->UpdatePlotAtts();
    }
 
    //
    // Merge the view limits and update the view in any windows that are
    // in camera mode.
    //
    for(int i = 0; i < nWindows; ++i)
    {
        windows[i]->SetMergeViewLimits(true);
        windows[i]->UpdateCameraView();
    }

    //
    // Update the windows that are associated with the animation.
    //
    UpdateFrame();

    return true;
}

// ****************************************************************************
//  Method: ViewerAnimation::GetFrameIndex
//
//  Purpose:
//    Return the 0 origin index of the current frame.
//
//  Returns:    The 0 origin index of the current frame.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
// ****************************************************************************

int
ViewerAnimation::GetFrameIndex() const
{
    return curFrame;
}

// ****************************************************************************
//  Method: ViewerAnimation::NextFrame
//
//  Purpose:
//    Set the current frame to the next frame in the animation.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
//  Modifications:
//    Eric Brugger, Wed Nov 21 10:47:42 PST 2001
//    I added the pipelineCaching mode.
//
//    Eric Brugger, Fri Nov 15 16:17:08 PST 2002
//    I modified the routine to call SetFrameIndex with the new frame number.
//
// ****************************************************************************

void
ViewerAnimation::NextFrame()
{
    SetFrameIndex((curFrame + 1) % nFrames);
}

// ****************************************************************************
//  Method: ViewerAnimation::PrevFrame
//
//  Purpose:
//    Set the current frame to the previous frame in the animation.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
//  Modifications:
//    Eric Brugger, Wed Nov 21 10:47:42 PST 2001
//    I added the pipelineCaching mode.
//
//    Eric Brugger, Fri Nov 15 16:17:08 PST 2002
//    I modified the routine to call SetFrameIndex with the new frame number.
//
// ****************************************************************************

void
ViewerAnimation::PrevFrame()
{
    SetFrameIndex((curFrame + nFrames - 1) % nFrames);
}

// ****************************************************************************
// Method: ViewerAnimation::UpdateNFrames
//
// Purpose: 
//   Updates the number of frames in the animation based on the open file
//   and the existing plots' databases. This method does nothing in keyframing
//   mode since we explicitly set the number of frames in that case.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 3 10:58:11 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerAnimation::UpdateNFrames()
{
    //
    // Get the maximum number of states out of the open database and all of
    // the databases used by plots.
    //
    int nStates = plotList->GetMaximumStates();
    if(!plotList->GetKeyframeMode())
        SetNFrames(nStates);
    else if(nFrames < nStates || plotList->GetNumPlots() == 0)
        SetNFrames(nStates);
}

// ****************************************************************************
//  Method: ViewerAnimation::UpdateFrame
//
//  Purpose:
//    Create the plots associatied with the current frame.  This method
//    may spawn threads to create the plots associated with the frame.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
//  Modifications:
//    Eric Brugger, Mon Oct 29 08:50:10 PST 2001
//    Add the new argument to the call UpdateAnimationState.
//
//    Brad Whitlock, Thu Jul 25 16:14:39 PST 2002
//    I restructured the code to fix an animation bug.
//
//    Brad Whitlock, Fri Jan 17 10:08:38 PDT 2003
//    I passed the new animating flag to the plotlist's UpdatePlots method.
//    The animating flag tells the UpdatePlots method if it should try to
//    generate plots again if they had errors.
//
//    Eric Brugger, Fri Jan 31 13:14:17 PST 2003
//    I removed an argument from the call to UpdateAnimationState.
//
//    Brad Whitlock, Thu Feb 27 11:59:29 PDT 2003
//    I passed the mode to UpdateAnimationState to ensure that the client
//    always has the right mode.
//
// ****************************************************************************

void
ViewerAnimation::UpdateFrame()
{
    //
    // If the current frame is already valid then just update the windows.
    //
    if (plotList->ArePlotsUpToDate(curFrame))
    {
        UpdateWindows(true);         
    }
    else
    {
        //
        // The PlotList's UpdatePlots method sometimes operates in threaded
        // mode. If no additional threads were spawned, we need to update the
        // windows.
        //
        bool animating = ((mode == PlayMode) || (mode == ReversePlayMode));
        if(plotList->UpdatePlots(curFrame, animating))
            UpdateWindows(true);
    }

    //
    // Send the animation mode and time step to the client.
    //
    int amode = 2;
    if(mode == PlayMode)
        amode = 3;
    else if (mode == ReversePlayMode)
        amode = 1;
    ViewerWindowManager::Instance()->UpdateAnimationState(this, amode);
}

// ****************************************************************************
// Method: ViewerAnimation::UpdateTools
//
// Purpose: 
//   Updates the tools in all windows that contain the animation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 10:30:05 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
ViewerAnimation::UpdateTools()
{
    for(int i = 0; i < nWindows; ++i)
        windows[i]->UpdateTools();
}

// ****************************************************************************
//  Method: ViewerAnimation::GetPlotList
//
//  Purpose:
//    Return a pointer to the plot list associated with the animation.
//
//  Returns:    The pointer associated with the animation.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
// ****************************************************************************

ViewerPlotList *
ViewerAnimation::GetPlotList() const
{
    return plotList;
}

// ****************************************************************************
//  Method: ViewerAnimation::AddWindow
//
//  Purpose:
//    Associate the specified window with the animation.
//
//  Arguments:
//    window    The window to associate with the animation.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
//  Modifications:
//    Eric Brugger, Tue Feb 20 08:23:14 PST 2001
//    I replaced all references to VisWindow with ViewerWindow.
//
// ****************************************************************************

void
ViewerAnimation::AddWindow(ViewerWindow *const window)
{
    //
    // Expand the list of plots if necessary.
    //
    if (nWindows >= nWindowsAlloc)
    {
        ViewerWindow **windowsNew=0;

        nWindowsAlloc += 4;

        windowsNew = new ViewerWindow*[nWindowsAlloc];
        memcpy(windowsNew, windows, nWindows*sizeof(ViewerWindow*));

        delete [] windows;
        windows = windowsNew;
    }

    //
    // Add the window to the list.
    //
    windows[nWindows] = window;
    nWindows++;
}

// ****************************************************************************
//  Method: ViewerAnimation::DeleteWindow
//
//  Purpose:
//    Disassociate the specified window with the animation.
//
//  Arguments:
//    window    The window to disassociate with the animation.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
//  Modifications:
//    Eric Brugger, Tue Feb 20 08:23:14 PST 2001
//    I replaced all references to VisWindow with ViewerWindow.
//
// ****************************************************************************

void
ViewerAnimation::DeleteWindow(const ViewerWindow *window)
{
    //
    // Loop over the list stopping when the window is found.
    //
    int       i;

    for (i = 0; i < nWindows; i++)
    {
        if (windows[i] == window)
        {
            break;
        }
    }

    //
    // If the window was not found, flag an exception.
    //
    if (i >= nWindows)
    {
        cerr << "ViewerAnimation::DeleteWindow: window not found." << endl;
        return;
    }

    //
    // Delete the window from the list.
    //
    windows[i] = windows[nWindows-1];
    nWindows--;
}

// ****************************************************************************
//  Method: ViewerAnimation::UpdateWindows
//
//  Purpose:
//    Update the windows associated with the animation.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
//  Modifications:
//    Kathleen Bonnell, Sat Jul 13 18:03:18 PDT 2002 
//    Allow ViewerQueryManager to udpate its scale factor.
//
//    Brad Whitlock, Thu Jul 25 16:12:41 PST 2002
//    I added a flag that is passed to the plot list's UpdateWindow method
//    that tells it if updates should be immediate or not. I also made some
//    code only compile when the viewer is multithreaded.
//
// ****************************************************************************

void
ViewerAnimation::UpdateWindows(bool immediateUpdate)
{
#ifdef VIEWER_MT
    //
    // If the plots are up-to-date then update the windows.
    //
    if (plotList->ArePlotsUpToDate(curFrame))
    {
#endif
        //
        // Update the windows associated with the animation.
        //
        for (int i = 0; i < nWindows; i++)
        {
            plotList->UpdateWindow(windows[i], curFrame, immediateUpdate);
            ViewerQueryManager::Instance()->UpdateScaleFactor(windows[i]);
        }

        //
        // Update the plot list so that the color changes on the plots.
        //
        GetPlotList()->UpdatePlotList();
#ifdef VIEWER_MT
    }
#endif
}

// ****************************************************************************
//  Method: ViewerAnimation::Play
//
//  Purpose:
//    Play the animation.
//
//  Programmer: Eric Brugger
//  Creation:   October 26, 2001
//
//  Modifications:
//    Eric Brugger, Mon Oct 29 08:50:10 PST 2001
//    Add the new argument to the call UpdateAnimationState.
//
//    Eric Brugger, Fri Jan 31 13:14:17 PST 2003
//    I removed an argument from the call to UpdateAnimationState.
//
// ****************************************************************************

void
ViewerAnimation::Play()
{
    //
    // Only enter play mode if there is at least one realized plot that is
    // up-to-date.
    //
    if (plotList->GetNumRealizedPlots() > 0 &&
        plotList->ArePlotsUpToDate(curFrame))
    {
        mode = PlayMode;
    }

    //
    // Update the animation timer.
    //
    ViewerWindowManager::Instance()->UpdateAnimationTimer();

    //
    // Update the animation controls.
    //
    ViewerWindowManager::Instance()->UpdateAnimationState(this,
        (mode == PlayMode) ? 3 : 2);
}

// ****************************************************************************
//  Method: ViewerAnimation::Stop
//
//  Purpose:
//    Stop the animation.
//
//  Arguments:
//    update : Whether or not to update the animation state.
//
//  Programmer: Eric Brugger
//  Creation:   October 26, 2001
//
//  Modifications:
//    Eric Brugger, Mon Oct 29 08:50:10 PST 2001
//    Add the new argument to the call UpdateAnimationState.
//
//    Brad Whitlock, Wed Jul 24 17:46:51 PST 2002
//    I added an option argument that tells the method whether or not to
//    update the timer and animation state.
//
//    Eric Brugger, Fri Jan 31 13:14:17 PST 2003
//    I removed an argument from the call to UpdateAnimationState.
//
// ****************************************************************************

void
ViewerAnimation::Stop(bool update)
{
    mode = StopMode;

    if(update)
    {
        //
        // Update the animation timer.
        //
        ViewerWindowManager::Instance()->UpdateAnimationTimer();

        //
        // Update the animation controls.
        //
        ViewerWindowManager::Instance()->UpdateAnimationState(this, 2);
    }
}

// ****************************************************************************
//  Method: ViewerAnimation::ReversePlay
//
//  Purpose:
//    Play the animation in reverse.
//
//  Programmer: Eric Brugger
//  Creation:   October 26, 2001
//
//  Modifications:
//    Eric Brugger, Mon Oct 29 08:50:10 PST 2001
//    Add the new argument to the call UpdateAnimationState.
//
//    Eric Brugger, Fri Jan 31 13:14:17 PST 2003
//    I removed an argument from the call to UpdateAnimationState.
//
// ****************************************************************************

void
ViewerAnimation::ReversePlay()
{
    //
    // Only enter reverse play mode if there is at least one realized plot
    // that is up-to-date.
    //
    if (plotList->GetNumRealizedPlots() > 0 &&
        plotList->ArePlotsUpToDate(curFrame))
    {
        mode = ReversePlayMode;
    }

    //
    // Update the animation timer.
    //
    ViewerWindowManager::Instance()->UpdateAnimationTimer();

    //
    // Update the animation controls.
    //
    ViewerWindowManager::Instance()->UpdateAnimationState(this,
        (mode == ReversePlayMode) ? 1 : 2);
}

// ****************************************************************************
//  Method: ViewerAnimation::GetMode
//
//  Purpose:
//    Get the mode of the animation.
//
//  Returns:    The mode of the animation (Play, Stop, ReversePlay).
//
//  Programmer: Eric Brugger
//  Creation:   October 26, 2001
//
// ****************************************************************************

ViewerAnimation::AnimationMode
ViewerAnimation::GetMode() const
{
    return mode;
}

// ****************************************************************************
//  Method: ViewerAnimation::SetPipelineCaching
//
//  Purpose:
//    Set the pipeline caching mode of the animation.
//
//  Programmer: Eric Brugger
//  Creation:   November 21, 2001
//
// ****************************************************************************

void
ViewerAnimation::SetPipelineCaching(const bool mode)
{
    //
    // If we are turning off the pipeline caching then flush the caches.
    //
    if (mode == false && nFrames > 1)
    {
        if (curFrame > 0)
            plotList->ClearPipelines(0, curFrame - 1);
        if (curFrame < nFrames - 1)
            plotList->ClearPipelines(curFrame + 1, nFrames - 1);
    }

    pipelineCaching = mode;
    if (pipelineCaching && avtCallback::GetNowinMode() == true)
    {
        debug1 << "Overriding request to do pipeline caching, since we are in "
               << "no-win mode." << endl;
        pipelineCaching = false;
    }
}

// ****************************************************************************
//  Method: ViewerAnimation::GetPipelineCaching
//
//  Purpose:
//    Get the pipeline caching mode of the animation.
//
//  Returns:    The pipeline caching mode of the animation.
//
//  Programmer: Eric Brugger
//  Creation:   November 21, 2001
//
// ****************************************************************************

bool
ViewerAnimation::GetPipelineCaching() const
{
    return pipelineCaching;
}

// ****************************************************************************
//  Method:  ViewerPlotList::SetWindowAtts
//
//  Purpose:
//    Send the window attributes to the appropriate engine.
//
//  Arguments:
//    hostName : The host on which the engine is running.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  8, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Dec 5 11:50:08 PDT 2001
//    Moved code to get the window attributes into ViewerWindow.
//
//    Brad Whitlock, Fri Feb 22 14:47:45 PST 2002
//    Made the engine RPC go through the engine manager.
//
// ****************************************************************************

bool
ViewerAnimation::SetWindowAtts(const char *hostName)
{
    //
    // Check for errors
    //
    if (nWindows < 1)
        return false;
    else if (nWindows > 1)
    {
        debug1 << "Animation::SetWindowAtts -- "
               << "More than one window for an animation is not allowed\n";
        EXCEPTION0(ImproperUseException);
    }

    //
    // Get the window attributes and send them to the engine.
    //
    ViewerWindow *w = windows[0];
    WindowAttributes winAtts = w->GetWindowAttributes();
    return ViewerEngineManager::Instance()->SetWindowAtts(hostName, &winAtts);
}
