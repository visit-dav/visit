// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
class     vtkPolyDataMapper2D;

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
//    Brad Whitlock, Thu Nov 6 14:13:40 PST 2003
//    Added GetFrameAndState method.
//
//    Kathleen Bonnell, Wed Dec  3 16:49:14 PST 2003
//    Added methods TransparenciesExist, ReAddToolsToRenderWindow.
//
//    Hank Childs, Mon May 10 08:31:01 PDT 2004
//    Removed immediate mode rendering method.
//
//    Chris Wojtan, Mon Jul 26 16:21:08 PDT 2004
//    Added 4 functions for suspending and resuming opaque and
//    translucent geometry.
//
//    Kathleen Bonnell, Mon Jun 27 14:54:36 PDT 2005
//    Added ProxiedGetMaxPlotZShift.
//
//    Mark Blair, Wed Aug 30 14:09:00 PDT 2006
//    Added ProxiedGetPlotListIndex, ProxiedGetSize, ProxiedGetWindowSize.
//
//    Mark Blair, Tue Sep 26 11:32:36 PDT 2006
//    Added ProxiedAxisAnnotationsEnabled.
//
//    Mark Blair, Wed Oct 25 15:12:55 PDT 2006
//    Added ProxiedGetPlotInfoAtts.
//
//    Mark C. Miller, Wed Mar 28 15:56:15 PDT 2007
//    Added IsMakingExternalRenderRequests, GetAverageExternalRenderingTime
//    and DoNextExternalRenderAsVisualQueue to support the 'in-progress'
//    visual queue for SR mode.
//
//    Dave Bremer, Wed Oct 31 15:48:16 PDT 2007
//    Added flag to clear the external renderer's cached image if disabling it.
//
//    Brad Whitlock, Mon Nov 19 17:44:10 PST 2007
//    Added code to query the background mode.
//
//    Jeremy Meredith, Tue Apr 22 14:33:16 EDT 2008
//    Removed axis annotation disabling -- it was only added for a single
//    plot, and the functionality has been accomodated in a new window
//    modality supporting the correct style annotations.
//
//    Brad Whitlock, Wed Jan  7 14:49:10 PST 2009
//    I removed plot info attributes.
//
//    Jeremy Meredith, Tue Feb  2 13:45:02 EST 2010
//    Added tool update mode.
//
//    Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//    Support 3D axis scaling (3D equivalent of full-frame mode).
//
//    Brad Whitlock, Fri Oct 14 16:28:24 PDT 2011
//    Create mappers vis proxy.
//
// ****************************************************************************

class VISWINDOW_API VisWindowProtectionProxy
{
  public:
                        VisWindowProtectionProxy(VisWindow *vw)
                                                 { viswin = vw; }

    //
    // Allow the proxy to masquerade as a VisWindow pointer whenever
    // it is convenient.
    //
    operator            VisWindow *()   { return viswin; }

  protected:
    VisWindow          *viswin;

    void                ProxiedChangeMode(WINDOW_MODE);
    vtkRenderer        *ProxiedGetBackground();
    bool                ProxiedGetBoundingBoxMode() const;
    void                ProxiedGetBounds(double [6]);
    vtkRenderer        *ProxiedGetCanvas();
    vtkRenderer        *ProxiedGetForeground();
    void                ProxiedGetForegroundColor(double [3]);
    bool                ProxiedGetHotPoint(int, int, HotPoint &) const;
    WINDOW_MODE         ProxiedGetMode();
    TOOLUPDATE_MODE     ProxiedGetToolUpdateMode() const;
    int                 ProxiedGetBackgroundMode() const;
    void                ProxiedGetViewport(double vport[4]);
    void                ProxiedGetSize(int &width, int &height) const;
    void                ProxiedGetWindowSize(int &width, int &height) const;
    void                ProxiedHasPlots(bool);
    bool                ProxiedHasPlots();

    vtkPolyDataMapper2D *ProxiedCreateRubberbandMapper();
    vtkPolyDataMapper2D *ProxiedCreateXorGridMapper();

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
    double              ProxiedComputeVectorTextScaleFactor(const double *p,
                                                        const double *v = NULL);
    void                ProxiedMotionBegin(void);
    void                ProxiedMotionEnd(void);

    bool                ProxiedGetAmbientOn();
    double              ProxiedGetAmbientCoefficient();
    bool                ProxiedGetLighting();
    void                ProxiedUpdateLightPositions();
    int                 ProxiedGetSurfaceRepresentation();
    bool                ProxiedDisableExternalRenderRequests(bool bClearImage = false);
    bool                ProxiedEnableExternalRenderRequests();
    void                ProxiedGetScaleFactorAndType(double &s, int &t);
    void                ProxiedReAddColleaguesToRenderWindow();
    bool                ProxiedGetAntialiasing();
    bool                ProxiedGetFullFrameMode();
    bool                ProxiedGet3DAxisScalingFactors(double s[3]);
    bool                ProxiedGetSpecularFlag();
    double               ProxiedGetSpecularCoeff();
    double               ProxiedGetSpecularPower();
    const ColorAttribute &ProxiedGetSpecularColor();
    bool                ProxiedTransparenciesExist();
    void                ProxiedReAddToolsToRenderWindow();
    void                ProxiedGetFrameAndState(int &,int &,int &,int &,
                                                int &,int &,int &) const;

    void                ProxiedSuspendOpaqueGeometry();
    void                ProxiedSuspendTranslucentGeometry();
    void                ProxiedResumeOpaqueGeometry();
    void                ProxiedResumeTranslucentGeometry();
    double               ProxiedGetMaxPlotZShift();

    bool                ProxiedIsMakingExternalRenderRequests(void) const;
    double              ProxiedGetAverageExternalRenderingTime(void) const;
    void                ProxiedDoNextExternalRenderAsVisualQueue(
                            int w, int h, const double *c);

    void                ProxiedGetExtents(double ext[2]); // TODO: remove with VTK8.
#if defined(VISIT_OSPRAY) || defined(HAVE_OSPRAY)
    bool                ProxiedGetOsprayRendering();
    int                 ProxiedGetOspraySPP();
    int                 ProxiedGetOsprayAO();
    bool                ProxiedGetOsprayShadows();
#endif
};

#endif
