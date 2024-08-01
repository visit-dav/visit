// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            VisWinRendering.h                              //
// ************************************************************************* //

#ifndef VIS_WIN_RENDERING_H
#define VIS_WIN_RENDERING_H
#include <viswindow_exports.h>

#include <VisWinColleague.h>

#include <avtCallback.h>
#include <avtImage.h>
#include <avtImageType.h>
#include <ColorAttribute.h>

#if defined(VISIT_OSPRAY) || defined(HAVE_OSPRAY)
#include <vtkOSPRayPass.h>
#endif

#ifdef VISIT_ANARI
#include <vtkAnariPass.h>
#endif

class vtkInteractorStyle;
class vtkPolyDataMapper2D;
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
//    Added the ParallelAxes window mode.
//
//    Dave Pugmire, Tue Aug 24 11:29:43 EDT 2010
//    Add compact domains options.
//
//    Gunther H. Weber, Fri Jul 15 13:51:35 PDT 2011
//    Add SetWindowSize().
//
//    Gunther H. Weber, Mon Jul 18 16:18:04 PDT 2011
//    Add ActivateWindow().
//
//    Brad Whitlock, Fri Oct 14 16:30:34 PDT 2011
//    Add mapper creation methods.
//
//    Brad Whitlock, Fri Apr  6 11:44:27 PDT 2012
//    Add GetStereoEnabled method.
//
//    Brad Whitlock, Wed Mar 13 16:08:08 PDT 2013
//    Add RenderRenderWindow.
//
//    Burlen Loring, Mon Aug 24 15:41:51 PDT 2015
//    Add support for capturing alpha channel. added
//    templated readback api so that data can be read
//    directly into the required type eliminating a
//    memcpy. added set/get methods for enabling alpha
//    channel, for configuring and enabling depth peeling
//    and configuring and enabling the alpha blending
//    compositer.
//
//    Burlen Loring, Mon Aug 31 07:51:29 PDT 2015
//    Add option to disable background in ScreenRender.
//
//    Burlen Loring, Thu Oct  8 13:33:11 PDT 2015
//    fix a couple of compiler warnings
//
//    Kathleen Biagas, Wed Aug 17, 2022
//    Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9.
//
//    Eric Brugger, Tue Jun 13 17:25:05 PDT 2023
//    Remove multi sampling related code when using VTK 9. This fixes a bug
//    where the visualization window is black when using mesagl.
//
//    Kevin Griffin, Thu 26 Oct 2023 09:51:22 AM PDT
//    Added support for ANARI.
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

    virtual void             StartParallelAxesMode(void);
    virtual void             StopParallelAxesMode(void);

    virtual void             EnableUpdates(void);
    virtual void             MotionBegin(void);
    virtual void             MotionEnd(void);

    void                     Realize(void);

    void                     GetCaptureRegion(int& r0, int& c0, int& w, int& h,
                                 bool doViewportOnly);

    void                     ScreenRender(avtImageType imgT = ColorRGBImage,
                                          bool doViewportOnly = false,
                                          bool doCanvasZBufferToo = false,
                                          bool doOpaque = true,
                                          bool doTranslucent = true,
                                          bool disableBackground = false,
                                          avtImage_p input = NULL);

    avtImage_p               ScreenReadback(bool doViewportOnly,
                                            bool readZ,
                                            bool readAlpha);

    template <typename T>
    void                     ScreenReadback(T *&r, T *&g, T *&b,
                                            T *&a, float *&z, int &w, int &h,
                                            bool doViewportOnly, bool readZ,
                                            bool readAlpha);

    avtImage_p               BackgroundReadback(bool doViewportOnly = false);

    avtImage_p               PostProcessScreenCapture(avtImage_p capturedImage,
                                 bool doViewportOnly, bool keepZBuffer);

    avtImage_p               ScreenCaptureValues(bool getZBuffer);

    virtual void             SetSize(int, int);
    virtual void             GetSize(int&, int&);
    virtual void             SetWindowSize(int, int);
    virtual void             GetWindowSize(int&, int&);
    virtual void             SetLocation(int, int);
    virtual void             GetLocation(int&, int&);
    virtual void             SetTitle(const char *);
    virtual void             Iconify(void) = 0;
    virtual void             DeIconify(void) = 0;
    virtual void             Show() = 0;
    virtual void             Hide() = 0;
    virtual void             Raise() = 0;
    virtual void             Lower() = 0;
    virtual void             ActivateWindow() = 0;
    virtual bool             IsVisible() const { return true; };

    bool                     GetRealized() {return realized;};

    bool                     IsDirect(void);
    vtkRenderer             *GetFirstRenderer(void);

    virtual void             SetResizeEvent(void(*callback)(void *), void *) = 0;
    virtual void             SetCloseCallback(void(*callback)(void *), void *) = 0;
    virtual void             SetHideCallback(void(*)(void *), void *) {};
    virtual void             SetShowCallback(void(*)(void *), void *) {};
    double                    ComputeVectorTextScaleFactor(const double *pos, const double *vp = NULL);

    void                     SetRenderInfoCallback(void(*callback)(void *), void *);
    void                     SetRenderEventCallback(void(*callback)(void *,bool), void *);
    void                     SetAntialiasing(bool enabled);
    bool                     GetAntialiasing() const
                                 { return antialiasing; };
    void                     GetRenderTimes(double times[6]) const;
    void                     SetStereoRendering(bool enabled, int type);
    bool                     GetStereo() const
                                 { return stereo; };
    int                      GetStereoType() const
                                 { return stereoType; };
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

    void                     EnableAlphaChannel();
    void                     DisableAlphaChannel();

    void                     SetOrderComposite(bool v) { orderComposite = v; }
    bool                     GetOrderComposite() const { return orderComposite; }
    void                     SetDepthCompositeThreads(size_t n) { depthCompositeThreads = n; }
    size_t                   GetDepthCompositeThreads() const { return depthCompositeThreads; }
    void                     SetAlphaCompositeThreads(size_t n) { alphaCompositeThreads = n; }
    size_t                   GetAlphaCompositeThreads() const { return alphaCompositeThreads; }
    void                     SetDepthCompositeBlocking(size_t n) { depthCompositeBlocking = n; }
    size_t                   GetDepthCompositeBlocking() const { return depthCompositeBlocking; }
    void                     SetAlphaCompositeBlocking(size_t n) { alphaCompositeBlocking = n; }
    size_t                   GetAlphaCompositeBlocking() const { return alphaCompositeBlocking; }

    void                     EnableDepthPeeling();
    void                     DisableDepthPeeling();

    void                     SetDepthPeeling(bool v) { depthPeeling = v; }
    bool                     GetDepthPeeling() const { return depthPeeling; }

    void                     SetNumberOfPeels(int n) { numberOfPeels = n; }
    int                      GetNumberOfPeels() const { return numberOfPeels; }

    void                     SetOcclusionRatio(double n) { occlusionRatio = n; }
    double                   GetOcclusionRatio() const { return occlusionRatio; }

    int                      GetNumPrimitives() const;
    void                     SetNotifyForEachRender(bool val)
                                 { notifyForEachRender = val; };
    bool                     GetNotifyForEachRender() const
                                 { return notifyForEachRender; };

    static void              SetStereoEnabled()
                                 { stereoEnabled = true; };
    static bool              GetStereoEnabled()
                                 { return stereoEnabled; }
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
    int                      GetCompactDomainsActivationMode() const
                                 { return compactDomainsActivationMode; }
    int                      SetCompactDomainsActivationMode(int mode);
    int                      GetCompactDomainsAutoThreshold() const
                                 { return compactDomainsAutoThreshold; }
    int                      SetCompactDomainsAutoThreshold(int val);
#ifdef VISIT_OSPRAY
    void                     SetModePerspective(bool modePerspective);
#endif
#if defined(VISIT_OSPRAY) || defined(HAVE_OSPRAY)
    void                     SetOsprayRendering(bool enabled);
    bool                     GetOsprayRendering() const
                                 { return osprayRendering; }
    void                     SetOspraySPP(int val);
    int                      GetOspraySPP() const
                                 { return ospraySPP; }
    void                     SetOsprayAO(int val);
    int                      GetOsprayAO() const
                                 { return osprayAO; }
    void                     SetOsprayShadows(bool enabled);
    bool                     GetOsprayShadows() const
                                 { return osprayShadows; }
#endif
#ifdef HAVE_OSPRAY
    void                     Set3DView(bool enabled);
    bool                     Get3DView() const
                                 { return viewIs3D; }
#endif
#ifdef VISIT_ANARI
    void                    SetAnariRendering(const bool);
    bool                    GetAnariRendering() const { return anariRendering; }
    void                    SetAnariSPP(const int);
    int                     GetAnariSPP() const { return anariSPP; }
    void                    SetAnariAO(const int);
    int                     GetAnariAO() const { return anariAO; }
    void                    SetAnariLibraryName(const std::string);
    std::string             GetAnariLibraryName() const { return anariLibraryName; }
    void                    SetAnariLibrarySubtype(const std::string);
    std::string             GetAnariLibrarySubtype() const { return anariLibrarySubtype; }
    void                    SetAnariRendererSubtype(const std::string);
    std::string             GetAnariRendererSubtype() const { return anariRendererSubtype; }
    void                    SetUseAnariDenoiser(const bool);
    bool                    GetUseAnariDenoiser() const { return useAnariDenoiser; }
    void                    SetAnariLightFalloff(const float);
    float                   GetAnariLightFalloff() const { return anariLightFalloff; }
    void                    SetAnariAmbientIntensity(const float);
    float                   GetAnariAmbientIntensity() const { return anariAmbientIntensity; }
    void                    SetAnariMaxDepth(const int);
    int                     GetAnariMaxDepth() const { return anariMaxDepth; }
    void                    SetAnariRValue(const float);
    float                   GetAnariRValue() const { return anariRValue; }
    void                    SetAnariDebugMethod(const std::string);
    std::string             GetAnariDebugMethod() const { return anariDebugMethod; }
    void                    SetUsdDir(const std::string);
    std::string             GetUsdDir() const { return usdDir; }
    void                    SetUsdAtCommit(const bool);
    bool                    GetUsdAtCommit() const { return usdAtCommit; }
    void                    SetUsdOutputBinary(const bool);
    bool                    GetUsdOutputBinary() const { return usdOutputBinary; }
    void                    SetUsdOutputMaterial(const bool);
    bool                    GetUsdOutputMaterial() const { return usdOutputMaterial; }
    void                    SetUsdOutputPreviewSurface(const bool);
    bool                    GetUsdOutputPreviewSurface() const { return usdOutputPreviewSurface; }
    void                    SetUsdOutputMDL(const bool);
    bool                    GetUsdOutputMDL() const { return usdOutputMDL; }
    void                    SetUsdOutputMDLColors(const bool);
    bool                    GetUsdOutputMDLColors() const { return usdOutputMDLColors; }
    void                    SetUsdOutputDisplayColors(const bool);
    bool                    GetUsdOutputDisplayColors() const { return usdOutputDisplayColors; }
    void                    SetUsingUsdDevice(const bool);
    bool                    GetUsingUsdDevice() const { return usingUsdDevice; }
    vtkAnariPass            *CreateAnariPass();
#endif

    virtual void            *CreateToolbar(const char *) { return 0; };
    virtual void             SetLargeIcons(bool) { };
    virtual void             SetCursorForMode(INTERACTION_MODE) { };

    virtual vtkPolyDataMapper2D *CreateRubberbandMapper() { return 0; }
    virtual vtkPolyDataMapper2D *CreateXorGridMapper() { return 0; }


    void                     InvokeRenderCallback();

    void                    UpdateMouseActions(
                                std::string action,
                                double start_dx, double start_dy,
                                double end_dx, double end_dy,
                                bool ctrl, bool shift);

  protected:
    vtkRenderer                  *canvas {nullptr};
    vtkRenderer                  *background {nullptr};
    vtkRenderer                  *foreground {nullptr};
    bool                          needsUpdate;
    bool                          realized;
    bool                          antialiasing;
    bool                          stereo;
    int                           stereoType;
    int                           surfaceRepresentation;
    bool                          specularFlag;
    double                        specularCoeff;
    double                        specularPower;
    ColorAttribute                specularColor;
    bool                          colorTexturingFlag;
    bool                          orderComposite;
    size_t                        depthCompositeThreads;
    size_t                        depthCompositeBlocking;
    size_t                        alphaCompositeThreads;
    size_t                        alphaCompositeBlocking;
    bool                          depthPeeling;
    double                        occlusionRatio;
    int                           numberOfPeels;
#if LIB_VERSION_LE(VTK,8,2,0)
    int                           multiSamples;
#endif
#if defined(VISIT_OSPRAY) || defined(HAVE_OSPRAY)
    bool                          osprayRendering {false};
    int                           ospraySPP {1};
    int                           osprayAO {0};
    bool                          osprayShadows {false};
    vtkOSPRayPass                *osprayPass {nullptr};
#endif
#ifdef VISIT_OSPRAY
    bool                          modeIsPerspective;
#elif defined(HAVE_OSPRAY)
    bool                          viewIs3D {true};
#endif
#ifdef VISIT_ANARI
    bool                        anariRendering;
    int                         anariSPP;
    int                         anariAO;
    std::string                 anariLibraryName;
    std::string                 anariLibrarySubtype;
    std::string                 anariRendererSubtype;
    bool                        useAnariDenoiser;
    float                       anariLightFalloff;
    float                       anariAmbientIntensity;
    int                         anariMaxDepth;
    float                       anariRValue;
    std::string                 anariDebugMethod;
    std::string                 usdDir;
    bool                        usdAtCommit;
    bool                        usdOutputBinary;
    bool                        usdOutputMaterial;
    bool                        usdOutputPreviewSurface;
    bool                        usdOutputMDL;
    bool                        usdOutputMDLColors;
    bool                        usdOutputDisplayColors;
    bool                        usingUsdDevice;
    vtkAnariPass                *anariPass;
    bool                        anariPassValid;
#endif

    void                          (*renderInfo)(void *);
    void                         *renderInfoData {nullptr};
    void                          (*renderEvent)(void *,bool);
    void                         *renderEventData {nullptr};
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

    // Compact domains options.
    int                           compactDomainsActivationMode;
    int                           compactDomainsAutoThreshold;

    // render options
    bool                          setRenderUpdate;
    void                          InitializeRenderWindow(vtkRenderWindow *);
    void                          ResetCounters();

    virtual vtkRenderWindow      *GetRenderWindow(void) = 0;
    virtual vtkRenderWindowInteractor *
                                  GetRenderWindowInteractor(void) = 0;

    virtual void                  RealizeRenderWindow(void) = 0;
    virtual void                  RenderRenderWindow(void);

private:
    void                     SetRenderUpdate(bool _setRenderUpdate)
                             { setRenderUpdate = _setRenderUpdate; }
    bool                     GetRenderUpdate() const
                             { return setRenderUpdate; }
};

#include <cstdlib>
#if ! defined HAVE_ALIGNED_ALLOC || ! defined USE_ALIGNED_ALLOC
#define aligned_alloc(_a, _n) \
    malloc(_n)
#endif

#include <vtkRenderWindow.h>

// ****************************************************************************
//  Method: VisWinRendering::ScreenReadback
//
//  Purpose:
//      Reads back an image from our render window returning
//      the individual r,g,b and optionally a and the
//      z-buffer. Caller needs to free the r,g,b,a arrays
//      and delete [] the z-buffer. The point of this method
//      is three fold, to read back in the desired precision
//      (either uchar or float) , by splitting into rgba components
//      compositing calculations are vectoroized by the compiler,
//      and finally z-buffer read back is zero copy.
//
//  Returns:    The image on the screen.
//
//  Programmer: Burlen Loring
//  Creation:   Tue Aug 25 09:12:10 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

inline
void ReadPixels(unsigned char *&rgba, vtkRenderWindow *rwin,
    int x0, int y0, int x1, int y1)
{ rgba = rwin->GetRGBACharPixelData(x0, y0, x1, y1, 1/*front*/); }

inline
void ReadPixels(float *&rgba, vtkRenderWindow *rwin,
    int x0, int y0, int x1, int y1)
{ rgba = rwin->GetRGBAPixelData(x0, y0, x1, y1, 1/*ront*/); }

template <typename T>
T *AllocAndCopyChannel(T *rgba, size_t n, int cid)
{
    T *c = static_cast<T*>(aligned_alloc(VISIT_MEM_ALIGN, n*sizeof(T)));
    T *prgba = rgba + cid;
    for (size_t i = 0; i < n; ++i)
        c[i] = prgba[4*i];
    return c;
}

template <typename T>
void
VisWinRendering::ScreenReadback(
    T *&r, T *&g, T *&b, T *&a, float *&z, int &w, int &h,
    bool doViewportOnly, bool readZ, bool readAlpha)
{
    // Set region origin/size to be captured
    int r0, c0;
    GetCaptureRegion(r0, c0, w, h, doViewportOnly);

    // Read the pixels from the window and copy them over.
    vtkRenderWindow *renWin = GetRenderWindow();

    T *rgba = NULL;
    ReadPixels(rgba, renWin, c0, r0, c0 + w-1, r0 + h-1);

    size_t npix = w*h;

    r = AllocAndCopyChannel(rgba, npix, 0);
    g = AllocAndCopyChannel(rgba, npix, 1);
    b = AllocAndCopyChannel(rgba, npix, 2);
    if (readAlpha)
        a = AllocAndCopyChannel(rgba, npix, 3);

    delete [] rgba;

    if (readZ)
        z = renWin->GetZbufferData(c0, r0, c0 + w-1, r0 + h-1);
}

#endif
