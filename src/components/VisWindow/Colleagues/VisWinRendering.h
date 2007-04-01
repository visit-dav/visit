// ************************************************************************* //
//                            VisWinRendering.h                              //
// ************************************************************************* //

#ifndef VIS_WIN_RENDERING_H
#define VIS_WIN_RENDERING_H
#include <viswindow_exports.h>

#include <VisWinColleague.h>

#include <avtImage.h>
#include <ColorAttribute.h>


class vtkInteractorStyle;
class vtkRenderer;
class vtkRenderWindow;
class vtkRenderWindowInteractor;

class VisWindowColleagueProxy;


// ****************************************************************************
//  Class: VisWinRendering
//
//  Purpose:
//    This is a concrete colleague for the mediator VisWindow.  It handles 
//    all of the rendering pipeline duties for the VisWindow.  It must be
//    instantiated first in the VisWindow since many other colleagues
//    depend on it being able to return the proper renderers.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Thu Jul  6 13:39:11 PDT 2000
//    Added ability to suspend updates (rendering) as well as ability to
//    set the size and position and an explicit realize.  Changed type of iren 
//    the XRenderWindowInteractor, made data member of app context, so it could
//    be realized any time.  Separated out border portion into its own 
//    colleague.  Removed methods SetForegroundColor, Start2DMode, Stop2DMode,
//    SetUpViewport, and SetUpRenderingPipeline and axis data members.
//
//    Hank Childs, Tue Jul 11 15:23:11 PDT 2000
//    Added data member restoreToParallelProjection and methods Start2DMode and
//    Stop2DMode.
//
//    Hank Childs, Tue Aug  1 16:37:59 PDT 2000
//    Added foreground renderer.
//
//    Brad Whitlock, Tue Nov 7 10:29:17 PDT 2000
//    Made it use Qt render windows and interactors. 
//
//    Hank Childs, Mon Feb 12 19:40:13 PST 2001
//    Implemented ScreenCapture.
//
//    Brad Whitlock, Thu Apr 19 11:41:19 PDT 2001
//    Added methods to handle window iconification.
//
//    Eric Brugger, Fri Aug 17 09:01:51 PDT 2001
//    I added the method SetResizeEvent.
//
//    Brad Whitlock, Wed Aug 22 12:02:22 PDT 2001
//    Added the SetCloseCallback method.
//
//    Hank Childs, Tue Sep 18 10:41:30 PDT 2001
//    Made destructor virtual.
//
//    Brad Whitlock, Fri Nov 2 10:58:23 PDT 2001
//    Added get methods for the location and the size of the window.
//
//    Hank Childs, Tue Jan 29 10:40:42 PST 2002
//    Made this an abstract base type.
//
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002 
//    Add support for curve mode.  Added method to compute scale factor
//    for vector text (moved and modified from avtPickActor). 
//
//    Sean Ahern, Mon May 20 13:30:53 PDT 2002
//    Added the ability to raise/lower windows.
//
//    Brad Whitlock, Thu Sep 19 14:22:11 PST 2002
//    Added antialiasing, stereo, etc.
//
//    Jeremy Meredith, Tue Nov 19 17:13:04 PST 2002
//    Added stereo en/disable flag and function.
//
//    Kathleen Bonnell, Wed Dec  4 17:05:24 PST 2002  
//    Removed numAntialiasingFrames, now using GL way of anti-aliasing. 
//
//    Brad Whitlock, Wed Jan 29 14:37:43 PST 2003
//    I added a method to create a toolbar.
//
//    Brad Whitlock, Wed Mar 12 09:24:38 PDT 2003
//    I added IsVisible, SetHideCallback, and SetShowCallback.
//
//    Brad Whitlock, Wed May 21 12:03:46 PDT 2003
//    I made GetSize be virtual.
//
//    Mark C. Miller, Mon Jul  7 01:23:45 PDT 2003
//    I made GetSize non-virtual and added new virtual GetWindowSize method
//
//    Jeremy Meredith, Fri Nov 14 11:27:19 PST 2003
//    Allowed specular lighting.
//
//    Brad Whitlock, Wed Jan 7 14:42:28 PST 2004
//    I added the SetCursorForMode method.
//
//    Brad Whitlock, Tue Mar 16 09:48:53 PDT 2004
//    I added SetLargeIcons.
//
//    Mark C. Miller, Wed Mar 31 17:47:20 PST 2004
//    Added doViewportOnly bool argument to ScreenCapture method
//
//    Hank Childs, Sun May  9 16:25:27 PDT 2004
//    Added IsDirect.  Also add support for display list mode.
//
// ****************************************************************************

class VISWINDOW_API VisWinRendering : public VisWinColleague
{
  public:
                             VisWinRendering(VisWindowColleagueProxy &);
    virtual                 ~VisWinRendering();

    virtual void             SetBackgroundColor(float, float, float);
    virtual void             SetViewport(float, float, float, float);
   
    vtkRenderer             *GetCanvas(void);
    vtkRenderer             *GetBackground(void);
    vtkRenderer             *GetForeground(void);

    void                     Render(void);
    void                     SetInteractor(vtkInteractorStyle *);

    virtual void             Start2DMode(void);
    virtual void             Stop2DMode(void);

    virtual void             StartCurveMode(void);
    virtual void             StopCurveMode(void);

    virtual void             EnableUpdates(void);
    virtual void             MotionBegin(void);
    virtual void             MotionEnd(void);

    void                     Realize(void);
    avtImage_p               ScreenCapture(bool doViewportOnly = false,
                                           bool doCanvasZBufferToo = false);

    void                     SetSize(int, int);
    void                     GetSize(int&, int&);
    virtual void             GetWindowSize(int&, int&);
    void                     SetLocation(int, int);
    void                     GetLocation(int&, int&);
    void                     SetTitle(const char *);
    virtual void             Iconify(void) = 0;
    virtual void             DeIconify(void) = 0;
    virtual void             Show() = 0;
    virtual void             Hide() = 0;
    virtual void             Raise() = 0;
    virtual void             Lower() = 0;
    virtual bool             IsVisible() const { return true; };

    bool                     GetRealized() {return realized;};

    bool                     IsDirect(void);
    vtkRenderer             *GetFirstRenderer(void);

    virtual void             SetResizeEvent(void(*callback)(void *), void *) = 0;
    virtual void             SetCloseCallback(void(*callback)(void *), void *) = 0;
    virtual void             SetHideCallback(void(*callback)(void *), void *) {};
    virtual void             SetShowCallback(void(*callback)(void *), void *) {};
    float                    ComputeVectorTextScaleFactor(const float *pos, const float *vp = NULL);

    void                     SetRenderInfoCallback(void(*callback)(void *), void *);
    void                     SetAntialiasing(bool enabled);
    bool                     GetAntialiasing() const
                                 { return antialiasing; };
    void                     GetRenderTimes(float times[3]) const;
    void                     SetStereoRendering(bool enabled, int type);
    bool                     GetStereo() const
                                 { return stereo; };
    int                      GetStereoType() const
                                 { return stereoType; };
    void                     SetDisplayListMode(int mode);
    int                      GetDisplayListMode() const
                                 { return displayListMode; };
    virtual void             SetSurfaceRepresentation(int rep);
    int                      GetSurfaceRepresentation() const
                                 { return surfaceRepresentation; };
    virtual void             SetSpecularProperties(bool,float,float,
                                                   const ColorAttribute&);
    bool                     GetSpecularFlag() const
                                 { return specularFlag; };
    float                    GetSpecularCoeff() const
                                 { return specularCoeff; };
    float                    GetSpecularPower() const
                                 { return specularPower; };
    const ColorAttribute    &GetSpecularColor() const
                                 { return specularColor; };
    int                      GetNumTriangles() const;
    void                     SetNotifyForEachRender(bool val)
                                 { notifyForEachRender = val; };
    bool                     GetNotifyForEachRender() const
                                 { return notifyForEachRender; };

    static void              SetStereoEnabled(bool se)
                                 { stereoEnabled = se; };

    bool                     SetScalableRendering(bool mode);
    bool                     GetScalableRendering() const
                                 { return scalableRendering; };
    int                      SetScalableThreshold(int threshold);
    int                      GetScalableThreshold() const
                                 { return scalableThreshold; };

    virtual void            *CreateToolbar(const char *) { return 0; };
    virtual void             SetLargeIcons(bool) { };
    virtual void             SetCursorForMode(INTERACTION_MODE) { };

  protected:
    vtkRenderer                  *canvas;
    vtkRenderer                  *background;
    vtkRenderer                  *foreground;
    bool                          needsUpdate;
    bool                          realized;
    bool                          antialiasing;
    bool                          stereo;
    int                           stereoType;
    int                           displayListMode;
    int                           surfaceRepresentation;
    bool                          specularFlag;
    float                         specularCoeff;
    float                         specularPower;
    ColorAttribute                specularColor;
    void(*renderInfo)(void *);
    void                         *renderInfoData;
    bool                          notifyForEachRender;
    bool                          inMotion;

    // Variables to track frames per second.
    float                         minRenderTime;
    float                         maxRenderTime;
    float                         summedRenderTime;
    float                         nRenders;

    // stereo option
    static bool                   stereoEnabled;

    // scalable rendering options
    bool                          scalableRendering;
    int                           scalableThreshold;

    void                          InitializeRenderWindow(vtkRenderWindow *);
    void                          ResetCounters();

    virtual vtkRenderWindow      *GetRenderWindow(void) = 0;
    virtual vtkRenderWindowInteractor *
                                  GetRenderWindowInteractor(void) = 0;

    virtual void                  RealizeRenderWindow(void) = 0;
};


#endif


