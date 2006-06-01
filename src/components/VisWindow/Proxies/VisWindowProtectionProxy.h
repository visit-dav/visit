/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
    void                ProxiedGetBounds(double [6]);
    vtkRenderer        *ProxiedGetCanvas();
    vtkRenderer        *ProxiedGetForeground();
    void                ProxiedGetForegroundColor(double [3]);
    bool                ProxiedGetHotPoint(int, int, HotPoint &) const;
    WINDOW_MODE         ProxiedGetMode();
    void                ProxiedGetViewport(double vport[4]);
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
    double               ProxiedComputeVectorTextScaleFactor(const double *p, 
                                                        const double *v = NULL);
    void                ProxiedMotionBegin(void);
    void                ProxiedMotionEnd(void);

    bool                ProxiedGetAmbientOn();
    double               ProxiedGetAmbientCoefficient();
    bool                ProxiedGetLighting();
    void                ProxiedUpdateLightPositions();
    int                 ProxiedGetSurfaceRepresentation();
    bool                ProxiedDisableExternalRenderRequests();
    bool                ProxiedEnableExternalRenderRequests();
    void                ProxiedGetScaleFactorAndType(double &s, int &t);
    void                ProxiedReAddColleaguesToRenderWindow();
    bool                ProxiedGetAntialiasing();
    bool                ProxiedGetFullFrameMode();
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
                                  
};


#endif


