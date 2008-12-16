/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Added methods to Set/Get scalable activation mode and auto threshold.
//    Added data members for same.
//    Elminated call to SetScalableThreshold.
//
//    Mark C. Miller, Mon Jul 26 15:08:39 PDT 2004
//    Added GetCaptureRegion() and PostProcessScreenCapture() methods
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added args to PostProcessScreenCapture for doing viewport only and
//    keeping zbuffer
//
//    Chris Wojtan, Wed Jul 21 15:15:57 PDT 2004
//    Added bool parameters for opaque and translucent rendering in
//    ScreenCapture()
//
//    Chris Wojtan, Fri Jul 30 14:36:30 PDT 2004
//    Added an input image to ScreenCapture() and a boolean
//    indicating whether or not this pass is the first rendering pass
//
//    Jeremy Meredith, Tue Aug 31 15:42:42 PDT 2004
//    Removed the last boolean on ScreenCapture since the image being NULL
//    suffices for that test.
//
//    Mark C. Miller, Fri Mar  4 13:05:02 PST 2005
//    Changed name of GetNumTriangles to GetNumPrimitives
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added 3 most recent rendering times to set of times returned by
//    GetRenderTimes. Added supporting data members
//
//    Mark C. Miller, Wed Aug  9 19:40:30 PDT 2006
//    Removed bool arg from SetStereoEnabled since it defaults to off
//
//    Brad Whitlock, Mon Sep 18 11:06:09 PDT 2006
//    Added color texturing support.
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added new AxisArray window mode.
//
//    Tom Fogal, Thu Jul 24 17:00:44 EDT 2008
//    Add ScreenRender and ScreenReadback methods; the IceT path can avoid the
//    latter in some cases.
//    Remove ScreenCapture; it is not accessible anymore.
//
//    Eric Brugger, Tue Dec  9 14:28:15 PST 2008
//    Added the AxisParallel window mode.
//
// ****************************************************************************

class VISWINDOW_API VisWinRendering : public VisWinColleague
{
  public:
                             VisWinRendering(VisWindowColleagueProxy &);
    virtual                 ~VisWinRendering();

    virtual void             SetBackgroundColor(double, double, double);
    virtual void             SetViewport(double, double, double, double);
   
    vtkRenderer             *GetCanvas(void);
    vtkRenderer             *GetBackground(void);
    vtkRenderer             *GetForeground(void);

    void                     Render(void);
    void                     SetInteractor(vtkInteractorStyle *);

    virtual void             Start2DMode(void);
    virtual void             Stop2DMode(void);

    virtual void             StartCurveMode(void);
    virtual void             StopCurveMode(void);

    virtual void             StartAxisArrayMode(void);
    virtual void             StopAxisArrayMode(void);

    virtual void             StartAxisParallelMode(void);
    virtual void             StopAxisParallelMode(void);

    virtual void             EnableUpdates(void);
    virtual void             MotionBegin(void);
    virtual void             MotionEnd(void);

    void                     Realize(void);

    void                     GetCaptureRegion(int& r0, int& c0, int& w, int& h,
                                 bool doViewportOnly);
    void                     ScreenRender(bool doViewportOnly = false,
                                          bool doCanvasZBufferToo = false,
                                          bool doOpaque = true,
                                          bool doTranslucent = true,
                                          avtImage_p input = NULL);
    avtImage_p               ScreenReadback(bool doViewportOnly,
                                            bool doCanvasZBufferToo);
    avtImage_p               PostProcessScreenCapture(avtImage_p capturedImage,
                                 bool doViewportOnly, bool keepZBuffer);

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
    double                    ComputeVectorTextScaleFactor(const double *pos, const double *vp = NULL);

    void                     SetRenderInfoCallback(void(*callback)(void *), void *);
    void                     SetAntialiasing(bool enabled);
    bool                     GetAntialiasing() const
                                 { return antialiasing; };
    void                     GetRenderTimes(double times[6]) const;
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
    virtual void             SetSpecularProperties(bool,double,double,
                                                   const ColorAttribute&);
    bool                     GetSpecularFlag() const
                                 { return specularFlag; };
    double                   GetSpecularCoeff() const
                                 { return specularCoeff; };
    double                   GetSpecularPower() const
                                 { return specularPower; };
    const ColorAttribute    &GetSpecularColor() const
                                 { return specularColor; };

    virtual void             SetColorTexturingFlag(bool);
    bool                     GetColorTexturingFlag() const;

    int                      GetNumPrimitives() const;
    void                     SetNotifyForEachRender(bool val)
                                 { notifyForEachRender = val; };
    bool                     GetNotifyForEachRender() const
                                 { return notifyForEachRender; };

    static void              SetStereoEnabled()
                                 { stereoEnabled = true; };

    int                      GetScalableThreshold() const;
    bool                     SetScalableRendering(bool mode);
    bool                     GetScalableRendering() const
                                 { return scalableRendering; };
    int                      SetScalableActivationMode(int mode);
    int                      GetScalableActivationMode() const
                                 { return scalableActivationMode; };
    int                      SetScalableAutoThreshold(int threshold);
    int                      GetScalableAutoThreshold() const
                                 { return scalableAutoThreshold; };

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
    double                        specularCoeff;
    double                        specularPower;
    ColorAttribute                specularColor;
    bool                          colorTexturingFlag;
    void(*renderInfo)(void *);
    void                         *renderInfoData;
    bool                          notifyForEachRender;
    bool                          inMotion;

    // Variables to track frames per second.
    double                         minRenderTime;
    double                         maxRenderTime;
    double                         summedRenderTime;
    double                         nRenders;
    double                         curRenderTimes[3];

    // stereo option
    static bool                   stereoEnabled;

    // scalable rendering options
    bool                          scalableRendering;
    int                           scalableActivationMode;
    int                           scalableAutoThreshold;

    void                          InitializeRenderWindow(vtkRenderWindow *);
    void                          ResetCounters();

    virtual vtkRenderWindow      *GetRenderWindow(void) = 0;
    virtual vtkRenderWindowInteractor *
                                  GetRenderWindowInteractor(void) = 0;

    virtual void                  RealizeRenderWindow(void) = 0;
};
#endif
