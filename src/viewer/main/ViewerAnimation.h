// ************************************************************************* //
//                              ViewerAnimation.h                            //
// ************************************************************************* //

#ifndef VIEWER_ANIMATION_H
#define VIEWER_ANIMATION_H
#include <viewer_exports.h>

class DataNode;
class ViewerPlotList;
class ViewerWindow;

// ****************************************************************************
//  Class: ViewerAnimation
//
//  Purpose:
//    ViewerAnimation is a class which implements time varying animations.
//    It has a number of frames and a current frame.  There are methods
//    for controlling both.  It also has a plot list associated with it,
//    which is a list of plots that each span some number of frames.
//    Last but not least an animation has windows associated with it.
//    An animation can have multiple windows associated with it.  If the
//    animation changes then the windows will get updated.
//
//  Note:
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
//  Modifications:
//    Eric Brugger, Tue Feb 20 08:21:40 PST 2001
//    I replaced all references to VisWindow with ViewerWindow.
//
//    Eric Brugger, Fri Feb 26 12:12:53 PDT 2001
//    I added the methods Play, Stop, ReversePlay and GetMode.
//
//    Jeremy Meredith, Fri Nov  9 10:13:08 PST 2001
//    Added method SetWindowAtts.
//
//    Eric Brugger, Wed Nov 21 10:46:22 PST 2001
//    I added the pipeline caching mode.
//
//    Brad Whitlock, Tue Feb 12 10:24:07 PDT 2002
//    I added a method to update tools.
//
//    Brad Whitlock, Fri Feb 22 14:47:25 PST 2002
//    I changed the prototype for SetWindowAtts.
//
//    Brad Whitlock, Wed Jul 24 17:45:39 PST 2002
//    I added an optional flag to the Stop method. I also added a flag to
//    the UpdateWindows method that tells the method whether or not window
//    updates should be immediate or delayed.
//
//    Eric Brugger, Fri Nov 15 16:15:01 PST 2002
//    I added support for keyframing.
//
//    Brad Whitlock, Mon Dec 30 15:00:09 PST 2002
//    I added some methods to set/get the database time state.
//
//    Eric Brugger, Fri Jan 31 13:13:17 PST 2003 
//    I removed the keyFraming argument from GetStateIndex.
//
//    Brad Whitlock, Thu Feb 6 15:45:29 PST 2003
//    Added a CopyFrom method.
//
//    Brad Whitlock, Thu Apr 3 09:40:32 PDT 2003
//    I removed the concept of database time state since it's really a
//    plot-oriented concept and not related to the animation.
//
//    Brad Whitlock, Wed Jul 16 13:05:27 PST 2003
//    Added CreateNode and SetFromNode.
//
//    Hank Childs, Tue Sep 16 10:19:53 PDT 2003
//    Added bogus implementations of GetTimeIndex and GetNTimes
//
// ****************************************************************************

class VIEWER_API ViewerAnimation
{
  public:
    ViewerAnimation();
    ~ViewerAnimation();

    typedef enum {PlayMode, StopMode, ReversePlayMode} AnimationMode;

    void CopyFrom(const ViewerAnimation *, bool=true);

    void UpdateNFrames();
    void SetNFrames(const int nFrames);
    int  GetNFrames() const;
    bool SetFrameIndex(const int index);
    int  GetFrameIndex() const;
    void NextFrame();
    void PrevFrame();
    void UpdateFrame();
    void UpdateTools();

    //
    // These routines do *not* work when we are doing keyframing.
    // They were put in so that other modules could reference them and
    // distinguish that they were interested in the time index.
    // They should be removed or correctly implemented when keyframing
    // is better separated from the time state in the viewer.
    //
    // The motivation for adding these routines is that there are many places
    // in the viewer where GetFrameIndex is being used where GetTimeIndex
    // is what is desired.  This is an attempt to "leave bread crumbs" for
    // whoever addresses this issue.
    //
    int  GetNTimes() const { return GetNFrames(); };
    int  GetTimeIndex() const { return GetFrameIndex(); };
    
    ViewerPlotList *GetPlotList() const;

    void AddWindow(ViewerWindow *const window);
    void DeleteWindow(const ViewerWindow *window);
    void UpdateWindows(bool immediateUpdate);

    void Play();
    void Stop(bool update = true);
    void ReversePlay();

    AnimationMode GetMode() const;

    void SetPipelineCaching(const bool mode);
    bool GetPipelineCaching() const;

    bool SetWindowAtts(const char *hostName);

    void CreateNode(DataNode *);
    bool SetFromNode(DataNode *);

  private:
    int             nFrames;
    int             curFrame;

    ViewerPlotList *plotList;

    int             nWindows;
    int             nWindowsAlloc;
    ViewerWindow  **windows;

    AnimationMode   mode;

    bool            pipelineCaching;
};
#endif
