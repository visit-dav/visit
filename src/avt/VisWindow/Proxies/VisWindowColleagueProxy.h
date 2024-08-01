// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         VisWindowColleagueProxy.h                         //
// ************************************************************************* //

#ifndef VIS_WINDOW_COLLEAGUE_PROXY_H
#define VIS_WINDOW_COLLEAGUE_PROXY_H
#include <viswindow_exports.h>


#include <VisWindowProtectionProxy.h>


// ****************************************************************************
//  Class: VisWindowColleagueProxy
//
//  Purpose:
//      This is a proxy specific to VisWindow colleagues.  They may use it
//      to access the VisWindow.  This object is not a friend to VisWindow,
//      but it may access it through its base class' methods and friend
//      status.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 11:27:06 PDT 2000
//    Added methods HasPlots and UpdatesEnabled.
//
//    Hank Childs, Tue Aug  1 16:43:18 PDT 2000
//    Added method GetForeground.
//
//    Hank Childs, Wed Nov  8 14:23:16 PST 2000
//    Added GetBounds.
//
//    Hank Childs, Tue Mar 12 17:55:27 PST 2002
//    Added UpdatePlotList.
//
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002
//    Added Set/Get TypeIsCurve, GetCurveYScale, ComputeVectorTextScaleFactor.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002
//    Added GetLighting, GetAmbientOn, GetAmbientCoefficient,
//    and UpdateLightPositions.
//
//    Brad Whitlock, Mon Sep 23 16:28:18 PST 2002
//    Added methods to return the surface representation and the immediate
//    rendering mode.
//
//    Kathleen Bonnell, Thu May 15 09:46:46 PDT 2003
//    Replaced GetCurveYScale with more generic GetScaleFactorAndType.
//
//    Kathleen Bonnell, Mon May 19 09:00:54 PDT 2003
//    Removed return from GetScaleFactorAndType.
//
//    Kathleen Bonnell, Wed May 28 15:58:15 PDT 2003
//    Added methods ReAddColleaguesToRenderWindow, GetAntialiasing.
//
//    Kathleen Bonnell, Fri Jun  6 15:31:37 PDT 2003
//    Added GetFullFrameMode method.
//
//    Eric Brugger, Wed Aug 20 10:03:43 PDT 2003
//    Removed GetTypeIsCurve and SetTypeIsCurve.
//
//    Jeremy Meredith, Fri Nov 14 13:54:29 PST 2003
//    Added specular coefficients.
//
//    Brad Whitlock, Thu Nov 6 14:17:38 PST 2003
//    Added GetFrameAndState.
//
//    Kathleen Bonnell, Wed Dec  3 16:49:14 PST 2003
//    Added 'ReAddtoolsToRenderWindow'.
//
//    Hank Childs, Mon May 10 08:31:01 PDT 2004
//    Removed ImmediateMode rendering method.
//
//    Chris Wojtan, Mon Jul 26 16:21:08 PDT 2004
//    Added 4 functions for suspending and resuming opaque and
//    translucent geometry.
//
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Added RecalculateRenderOrder. Strangely, it was already implemented
//    in VisWindowColleagueProxy.C
//
//    Kathleen Bonnell, Mon Jun 27 14:54:36 PDT 2005
//    Added GetMaxPlotZShift.
//
//    Mark Blair, Mon Sep 25 11:41:09 PDT 2006
//    Added AxisAnnotationsEnabled.
//
//    Mark C. Miller, Wed Mar 28 15:56:15 PDT 2007
//    Added IsMakingExternalRenderRequests, GetAverageExternalRenderingTime
//    and DoNextExternalRenderAsVisualQueue to support the 'in-progress'
//    visual queue for SR mode.
//
//    Dave Bremer, Wed Oct 31 15:48:16 PDT 2007
//    Added flag to clear the external renderer's cached image if disabling it.
//
//    Brad Whitlock, Mon Nov 19 17:46:11 PST 2007
//    Added GetBackgroundMode.
//
//    Jeremy Meredith, Tue Apr 22 14:33:16 EDT 2008
//    Removed axis annotation disabling -- it was only added for a single
//    plot, and the functionality has been accomodated in a new window
//    modality supporting the correct style annotations.
//
//    Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//    Support 3D axis scaling (3D equivalent of full-frame mode).
//
//    Kathleen Biagas, Wed Aug 17, 2022
//    Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9.
//
//    Kevin Griffin, Thu 26 Oct 2023 09:51:22 AM PDT
//    Added ANARI support.
//
// ****************************************************************************

class VISWINDOW_API VisWindowColleagueProxy : public VisWindowProtectionProxy
{
  public:
                        VisWindowColleagueProxy(VisWindow *vw)
                             : VisWindowProtectionProxy(vw) {;}

    WINDOW_MODE         GetMode() { return ProxiedGetMode(); }

    void                ChangeMode(WINDOW_MODE mode)
                                  { ProxiedChangeMode(mode); }

    void                SetInteractor(VisitInteractor *i)
                                  { ProxiedSetInteractor(i); }

    void                GetForegroundColor(double *fg)
                                  { ProxiedGetForegroundColor(fg); }

    void                GetViewport(double *vport)
                                  { ProxiedGetViewport(vport); }

    vtkRenderer        *GetBackground(void)
                                  { return ProxiedGetBackground(); }

    int                 GetBackgroundMode() const
                                  { return ProxiedGetBackgroundMode(); }

    void                GetBounds(double bounds[6])
                                  { ProxiedGetBounds(bounds); }

    void                GetExtents(double ext[3]) // TODO: Remove with VTK8
                                  { ProxiedGetExtents(ext); }

    vtkRenderer        *GetCanvas(void)
                                  { return ProxiedGetCanvas(); }

    vtkRenderer        *GetForeground(void)
                                  { return ProxiedGetForeground(); }

    void                HasPlots(bool b)
                                  { ProxiedHasPlots(b); }

    bool                HasPlots(void)
                                  { return ProxiedHasPlots(); }

    bool                UpdatesEnabled(void)
                                  { return ProxiedUpdatesEnabled(); }

    void                Render(void) { ProxiedRender(); }

    void                UpdatePlotList(std::vector<avtActor_p> &l)
                                  { ProxiedUpdatePlotList(l); }

    double              ComputeVectorTextScaleFactor(const double *p, const double*v = NULL)
                                  { return ProxiedComputeVectorTextScaleFactor(p, v); }

    bool                GetAmbientOn()
                                  { return ProxiedGetAmbientOn(); }

    double               GetAmbientCoefficient()
                                  { return ProxiedGetAmbientCoefficient(); }

    bool                GetLighting()
                                  { return ProxiedGetLighting(); }

    void                UpdateLightPositions()
                                  { ProxiedUpdateLightPositions(); }

    int                 GetSurfaceRepresentation()
                                  { return ProxiedGetSurfaceRepresentation(); }

    bool                DisableExternalRenderRequests(bool bClearImage = false)
                                  { return ProxiedDisableExternalRenderRequests(bClearImage); }

    bool                EnableExternalRenderRequests()
                                  { return ProxiedEnableExternalRenderRequests(); }

    void                GetScaleFactorAndType(double &s, int &t)
                                  { ProxiedGetScaleFactorAndType(s, t); }

    void                ReAddColleaguesToRenderWindow()
                                  { ProxiedReAddColleaguesToRenderWindow(); }

    bool                GetAntialiasing()
                                  { return ProxiedGetAntialiasing(); }

    bool                GetFullFrameMode()
                                  { return ProxiedGetFullFrameMode(); }

    bool                Get3DAxisScalingFactors(double s[3])
                                  { return ProxiedGet3DAxisScalingFactors(s); }

    bool                GetSpecularFlag()
                                  { return ProxiedGetSpecularFlag(); }

    double              GetSpecularCoeff()
                                  { return ProxiedGetSpecularCoeff(); }

    double              GetSpecularPower()
                                  { return ProxiedGetSpecularPower(); }

    const ColorAttribute &GetSpecularColor()
                                  { return ProxiedGetSpecularColor(); }
    void                ReAddToolsToRenderWindow()
                                  { ProxiedReAddToolsToRenderWindow(); }

    void                GetFrameAndState(int &a, int &b, int &c, int &d,
                                         int &e, int &f, int &g) const
                                  { ProxiedGetFrameAndState(a,b,c,d,e,f,g); }

    void                SuspendOpaqueGeometry()
                                  { ProxiedSuspendOpaqueGeometry(); }

    void                SuspendTranslucentGeometry()
                                  { ProxiedSuspendTranslucentGeometry(); }

    void                ResumeOpaqueGeometry()
                                  { ProxiedResumeOpaqueGeometry(); }

    void                ResumeTranslucentGeometry()
                                  { ProxiedResumeTranslucentGeometry(); }

    void                RecalculateRenderOrder()
                                  { ProxiedRecalculateRenderOrder(); }
    double              GetMaxPlotZShift()
                                  { return ProxiedGetMaxPlotZShift(); }

    bool                IsMakingExternalRenderRequests(void) const
                             { return ProxiedIsMakingExternalRenderRequests(); }
    double              GetAverageExternalRenderingTime(void) const
                             { return ProxiedGetAverageExternalRenderingTime(); }
    void                DoNextExternalRenderAsVisualQueue(int w, int h, const double *c)
                             { ProxiedDoNextExternalRenderAsVisualQueue(w,h,c); }
#if defined(VISIT_OSPRAY) || defined(HAVE_OSPRAY)
    bool                GetOsprayRendering()
                             { return ProxiedGetOsprayRendering(); }
#endif
#ifdef HAVE_OSPRAY
    int                 GetOspraySPP()
                             { return ProxiedGetOspraySPP(); }
    int                 GetOsprayAO()
                             { return ProxiedGetOsprayAO(); }
    bool                GetOsprayShadows()
                             { return ProxiedGetOsprayShadows(); }
#endif
#ifdef VISIT_ANARI
    bool        GetAnariRendering()         { return ProxiedGetAnariRendering(); }
    int         GetAnariSPP()               { return ProxiedGetAnariSPP(); }
    int         GetAnariAO()                { return ProxiedGetAnariAO(); }
    std::string GetAnariLibraryName()       { return ProxiedGetAnariLibraryName(); }
    std::string GetAnariLibrarySubtype()    { return ProxiedGetAnariLibrarySubtype(); }
    std::string GetAnariRendererSubtype()   { return ProxiedGetAnariRendererSubtype(); }
    bool        GetUseAnariDenoiser()       { return ProxiedGetUseAnariDenoiser(); }
    float       GetAnariLightFalloff()      { return ProxiedGetAnariLightFalloff(); }
    float       GetAnariAmbientIntensity()  { return ProxiedGetAnariAmbientIntensity(); }
    int         GetAnariMaxDepth()          { return ProxiedGetAnariMaxDepth(); }
    float       GetAnariRValue()            { return ProxiedGetAnariRValue(); }
    std::string GetAnariDebugMethod()       { return ProxiedGetAnariDebugMethod(); }
    std::string GetUsdDir()                 { return ProxiedGetUsdDir(); }
    bool        GetUsdAtCommit()            { return ProxiedGetUsdAtCommit(); }
    bool        GetUsdOutputBinary()        { return ProxiedGetUsdOutputBinary(); }
    bool        GetUsdOutputMaterial()      { return ProxiedGetUsdOutputMaterial(); }
    bool        GetUsdOutputPreviewSurface()    { return ProxiedGetUsdOutputPreviewSurface(); }
    bool        GetUsdOutputMDL()               { return ProxiedGetUsdOutputMDL(); }
    bool        GetUsdOutputMDLColors()         { return ProxiedGetUsdOutputMDLColors(); }
    bool        GetUsdOutputDisplayColors()     { return ProxiedGetUsdOutputDisplayColors(); }
    bool        GetUsingUsdDevice()         { return ProxiedGetUsingUsdDevice(); }
#endif
};

#endif
