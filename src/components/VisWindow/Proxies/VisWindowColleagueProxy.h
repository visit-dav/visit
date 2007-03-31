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
//    Kathleen Bonnell, Wed Dec  3 16:49:14 PST 2003
//    Added 'ReAddtoolsToRenderWindow'.
//
// ****************************************************************************

class VISWINDOW_API VisWindowColleagueProxy : public VisWindowProtectionProxy
{
  public:
                        VisWindowColleagueProxy(VisWindow *vw) 
                             : VisWindowProtectionProxy(vw) {;};

    WINDOW_MODE         GetMode() { return ProxiedGetMode(); };

    void                ChangeMode(WINDOW_MODE mode)
                                  { ProxiedChangeMode(mode); };

    void                SetInteractor(VisitInteractor *i)
                                  { ProxiedSetInteractor(i); };

    void                GetForegroundColor(float *fg)
                                  { ProxiedGetForegroundColor(fg); };

    void                GetViewport(float *vport)
                                  { ProxiedGetViewport(vport); };

    vtkRenderer        *GetBackground(void)
                                  { return ProxiedGetBackground(); };

    void                GetBounds(float bounds[6])
                                  { ProxiedGetBounds(bounds); };

    vtkRenderer        *GetCanvas(void)
                                  { return ProxiedGetCanvas(); };

    vtkRenderer        *GetForeground(void)
                                  { return ProxiedGetForeground(); };

    void                HasPlots(bool b)
                                  { ProxiedHasPlots(b); };

    bool                HasPlots(void)
                                  { return ProxiedHasPlots(); };

    bool                UpdatesEnabled(void)
                                  { return ProxiedUpdatesEnabled(); };

    void                Render(void) { ProxiedRender(); };

    void                UpdatePlotList(std::vector<avtActor_p> &l)
                                  { ProxiedUpdatePlotList(l); };

    float               ComputeVectorTextScaleFactor(const float *p, const float*v = NULL) 
                                  { return ProxiedComputeVectorTextScaleFactor(p, v); };

    bool                GetAmbientOn()
                                  { return ProxiedGetAmbientOn(); };

    float               GetAmbientCoefficient()
                                  { return ProxiedGetAmbientCoefficient(); };

    bool                GetLighting()
                                  { return ProxiedGetLighting(); };

    void                UpdateLightPositions()
                                  { ProxiedUpdateLightPositions(); };

    int                 GetSurfaceRepresentation()
                                  { return ProxiedGetSurfaceRepresentation(); };

    bool                GetImmediateModeRendering()
                                  { return ProxiedGetImmediateModeRendering(); };

    bool                DisableExternalRenderRequests()
                                  { return ProxiedDisableExternalRenderRequests(); };

    bool                EnableExternalRenderRequests()
                                  { return ProxiedEnableExternalRenderRequests(); };

    void                GetScaleFactorAndType(double &s, int &t) 
                                  { ProxiedGetScaleFactorAndType(s, t); };

    void                ReAddColleaguesToRenderWindow()
                                  { ProxiedReAddColleaguesToRenderWindow(); };

    bool                GetAntialiasing()
                                  { return ProxiedGetAntialiasing(); };

    bool                GetFullFrameMode()
                                  { return ProxiedGetFullFrameMode(); };

    bool                GetSpecularFlag()
                                  { return ProxiedGetSpecularFlag(); };

    float               GetSpecularCoeff()
                                  { return ProxiedGetSpecularCoeff(); };

    float               GetSpecularPower()
                                  { return ProxiedGetSpecularPower(); };

    const ColorAttribute &GetSpecularColor()
                                  { return ProxiedGetSpecularColor(); };
    void                ReAddToolsToRenderWindow()
                                  { ProxiedReAddToolsToRenderWindow(); };
};


#endif


