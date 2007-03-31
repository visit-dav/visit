// ************************************************************************* //
//                         VisWindowProtectionProxy.h                        //
// ************************************************************************* //

#ifndef VIS_WINDOW_PROTECTION_PROXY_H
#define VIS_WINDOW_PROTECTION_PROXY_H
#include <viswindow_exports.h>


#include <vector>

#include <VisWindowTypes.h>

#include <avtActor.h>


class     vtkRenderer;

class     VisWindow;
class     VisitInteractor;


// ****************************************************************************
//  Class: VisWindowProtectionProxy
//
//  Purpose:
//      The VisWindow class needs to allow greater than public access to other
//      objects, but making them friends does not make sense because it would
//      allow colleagues to access other colleagues and interactors to do
//      things that only colleagues should be able to do.  The solution is to
//      provide a proxy and have that proxy allow selected access.  This class
//      is the class granted friend access to the VisWindow class.  All of its
//      methods are protected and can only be accessed by its derived types.
//      The derived types are specific proxies for interactors, colleagues,
//      and rendering.
//
//  Note:       None of the methods for VisWindowProtectionProxy can be
//              inlined because it would lead to circular include problems.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
//  Modifications:
//
//    Brad Whitlock, Mon Oct 1 14:12:42 PST 2001
//    Added ProxiedGetHotPoint() method.
//
//    Kathleen Bonnell, Wed Nov 12 10:54:55 PST 2001
//    Added ProxiedPick method.
//
//    Hank Childs, Tue Mar 12 17:55:27 PST 2002
//    Added ProxiedUpdatePlotList.  Blew away older comments.
//
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002
//    Added Proxied Set/Get TypeIsCurve, GetCurveYScale, 
//    ComputeVectorTextScaleFactor, and ProxiedLineout method.
//
//    Hank Childs, Thu Jul 11 17:45:50 PDT 2002
//    Add support for motion tracking.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002
//    Added GetLighting, GetAmbientOn, GetAmbientCoefficient,
//    and UpdateLightPositions. 
//
//    Hank Childs, Wed Sep  4 08:26:55 PDT 2002
//    Added RecalculateRenderOrder.
//
//    Kathleen Bonnell, Thu May 15 09:46:46 PDT 2003 
//    Replaced GetCurveYScale with more generic GetScaleFactorAndType.
//
//    Kathleen Bonnell, Wed May 28 16:04:50 PDT 2003  
//    Added methods ReAddColleaguesToRenderWindow, GetAntialiasing.
//
//    Kathleen Bonnell, Wed May 28 16:04:50 PDT 2003  
//    Added GetFullFrameMode. 
//
//    Eric Brugger, Wed Aug 20 10:04:13 PDT 2003
//    Removed ProxiedGetTypeIsCurve and ProxiedSetTypeIsCurve.
//
//    Jeremy Meredith, Fri Nov 14 13:53:37 PST 2003
//    Added specular properties.
//
//    Kathleen Bonnell, Wed Dec  3 16:49:14 PST 2003 
//    Added methods TransparenciesExist, ReAddToolsToRenderWindow.
//
// ****************************************************************************

class VISWINDOW_API VisWindowProtectionProxy
{
  public:
                        VisWindowProtectionProxy(VisWindow *vw) 
                                                 { viswin = vw; };

    //
    // Allow the proxy to masquerade as a VisWindow pointer whenever
    // it is convenient.
    //
    operator            VisWindow *()   { return viswin; };

  protected:
    VisWindow          *viswin;

    void                ProxiedChangeMode(WINDOW_MODE);
    vtkRenderer        *ProxiedGetBackground();
    bool                ProxiedGetBoundingBoxMode() const;
    void                ProxiedGetBounds(float [6]);
    vtkRenderer        *ProxiedGetCanvas();
    vtkRenderer        *ProxiedGetForeground();
    void                ProxiedGetForegroundColor(float [3]);
    bool                ProxiedGetHotPoint(int, int, HotPoint &) const;
    WINDOW_MODE         ProxiedGetMode();
    void                ProxiedGetViewport(float vport[4]);
    void                ProxiedHasPlots(bool);
    bool                ProxiedHasPlots();
    bool                ProxiedUpdatesEnabled();
    void                ProxiedDisableUpdates();
    void                ProxiedEnableUpdates();
    void                ProxiedSetInteractor(VisitInteractor *);
    void                ProxiedStartRender();
    void                ProxiedRender();
    void                ProxiedRecalculateRenderOrder();
    void                ProxiedStartBoundingBox();
    void                ProxiedEndBoundingBox();
    void                ProxiedSetHighlightEnabled(bool);
    void                ProxiedPick(int, int);
    void                ProxiedUpdatePlotList(std::vector<avtActor_p> &);
    void                ProxiedLineout(int, int, int, int);
    float               ProxiedComputeVectorTextScaleFactor(const float *p, 
                                                        const float *v = NULL);
    void                ProxiedMotionBegin(void);
    void                ProxiedMotionEnd(void);

    bool                ProxiedGetAmbientOn();
    float               ProxiedGetAmbientCoefficient();
    bool                ProxiedGetLighting();
    void                ProxiedUpdateLightPositions();
    int                 ProxiedGetSurfaceRepresentation();
    bool                ProxiedGetImmediateModeRendering();
    bool                ProxiedDisableExternalRenderRequests();
    bool                ProxiedEnableExternalRenderRequests();
    void                ProxiedGetScaleFactorAndType(double &s, int &t);
    void                ProxiedReAddColleaguesToRenderWindow();
    bool                ProxiedGetAntialiasing();
    bool                ProxiedGetFullFrameMode();
    bool                ProxiedGetSpecularFlag();
    float               ProxiedGetSpecularCoeff();
    float               ProxiedGetSpecularPower();
    const ColorAttribute &ProxiedGetSpecularColor();
    bool                ProxiedTransparenciesExist();
    void                ProxiedReAddToolsToRenderWindow();
};


#endif


