/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

    void                GetForegroundColor(double *fg)
                                  { ProxiedGetForegroundColor(fg); };

    void                GetViewport(double *vport)
                                  { ProxiedGetViewport(vport); };

    vtkRenderer        *GetBackground(void)
                                  { return ProxiedGetBackground(); };

    int                 GetBackgroundMode() const
                                  { return ProxiedGetBackgroundMode(); }

    void                GetBounds(double bounds[6])
                                  { ProxiedGetBounds(bounds); };

    vtkRenderer        *GetCanvas(void)
                                  { return ProxiedGetCanvas(); };

    vtkRenderer        *GetForeground(void)
                                  { return ProxiedGetForeground(); };

    void                HasPlots(bool b)
                                  { ProxiedHasPlots(b); };

    bool                HasPlots(void)
                                  { return ProxiedHasPlots(); };

    bool                AxisAnnotationsEnabled()
                                  { return ProxiedAxisAnnotationsEnabled(); };

    bool                UpdatesEnabled(void)
                                  { return ProxiedUpdatesEnabled(); };

    void                Render(void) { ProxiedRender(); };

    void                UpdatePlotList(std::vector<avtActor_p> &l)
                                  { ProxiedUpdatePlotList(l); };

    double               ComputeVectorTextScaleFactor(const double *p, const double*v = NULL) 
                                  { return ProxiedComputeVectorTextScaleFactor(p, v); };

    bool                GetAmbientOn()
                                  { return ProxiedGetAmbientOn(); };

    double               GetAmbientCoefficient()
                                  { return ProxiedGetAmbientCoefficient(); };

    bool                GetLighting()
                                  { return ProxiedGetLighting(); };

    void                UpdateLightPositions()
                                  { ProxiedUpdateLightPositions(); };

    int                 GetSurfaceRepresentation()
                                  { return ProxiedGetSurfaceRepresentation(); };

    bool                DisableExternalRenderRequests(bool bClearImage = false)
                                  { return ProxiedDisableExternalRenderRequests(bClearImage); };

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

    double               GetSpecularCoeff()
                                  { return ProxiedGetSpecularCoeff(); };

    double               GetSpecularPower()
                                  { return ProxiedGetSpecularPower(); };

    const ColorAttribute &GetSpecularColor()
                                  { return ProxiedGetSpecularColor(); };
    void                ReAddToolsToRenderWindow()
                                  { ProxiedReAddToolsToRenderWindow(); };

    void                GetFrameAndState(int &a, int &b, int &c, int &d,
                                         int &e, int &f, int &g) const
                                  { ProxiedGetFrameAndState(a,b,c,d,e,f,g); };

    void                SuspendOpaqueGeometry()
                                  { ProxiedSuspendOpaqueGeometry(); };

    void                SuspendTranslucentGeometry()
                                  { ProxiedSuspendTranslucentGeometry(); };

    void                ResumeOpaqueGeometry()
                                  { ProxiedResumeOpaqueGeometry(); };

    void                ResumeTranslucentGeometry()
                                  { ProxiedResumeTranslucentGeometry(); };

    void                RecalculateRenderOrder()
                                  { ProxiedRecalculateRenderOrder(); };
    double               GetMaxPlotZShift()
                                  { return ProxiedGetMaxPlotZShift(); };

    bool                 IsMakingExternalRenderRequests(void) const
                             { return ProxiedIsMakingExternalRenderRequests(); };
    double               GetAverageExternalRenderingTime(void) const
                             { return ProxiedGetAverageExternalRenderingTime(); };
    void                 DoNextExternalRenderAsVisualQueue(int w, int h, const double *c)
                             { ProxiedDoNextExternalRenderAsVisualQueue(w,h,c); };

};


#endif


