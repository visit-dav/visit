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

#ifndef VISIT_HOT_POINT_INTERACTOR_H
#define VISIT_HOT_POINT_INTERACTOR_H
#include <viswindow_exports.h>
#include <VisitInteractor.h>
#include <VisWindowTypes.h>

// Forward declarations.
struct HotPoint;
class Dolly3D;
class FlyThrough;
class Lineout2D;
class Navigate2D;
class Navigate3D;
class NavigateCurve;
class NavigateAxisArray;
class NullInteractor;
class Pick;
class Zoom2D;
class Zoom3D;
class ZoomCurve;

// ****************************************************************************
// Class: VisitHotPointInteractor
//
// Purpose:
//   This interactor is the main interactor that is installed in the VisWindow.
//   It keeps a set of other interactors around and delegates interaction
//   responsibilities to them or to tools whose hotpoints are clicked.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:57:17 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Mar 18 13:47:00 PST 2002
//   Comply with new interface from base class for better buttonpress control.
//
//   Kathleen Bonnell, Tue Apr 16 15:07:38 PDT 2002  
//   Added Lineout2D, ZoomCurve and Start/Stop CurveMode.
//   
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from all ButtonAction methods and OnMouseMove, 
//    in order to match vtk's new interactor api.  Added SetInteractor method.
//
//    Eric Brugger, Wed Oct 15 17:32:48 PDT 2003
//    Added NavigateCurve interactor.
//
//    Eric Brugger, Thu Oct 28 15:31:02 PDT 2004
//    Added FlyThrough interactor.
//
//    Eric Brugger, Tue Dec 28 07:50:45 PST 2004
//    Added Dolly3D interactor.
//
//    Kathleen Bonnell, Thu Jan 12 13:58:25 PST 2006 
//    Replaced Pick3D and Pick2D interactors with Pick.
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added new AxisArray window mode.
//
// ****************************************************************************

class VISWINDOW_API VisitHotPointInteractor : public VisitInteractor
{
public:
                      VisitHotPointInteractor(VisWindowInteractorProxy &);
    virtual          ~VisitHotPointInteractor();

    virtual void      SetInteractor(vtkRenderWindowInteractor *rwi);
 
    virtual void      StartLeftButtonAction();
    virtual void      EndLeftButtonAction();
    virtual void      AbortLeftButtonAction();
    virtual void      StartMiddleButtonAction();
    virtual void      EndMiddleButtonAction();
    virtual void      AbortMiddleButtonAction();

    virtual void      OnMouseMove();
    virtual void      OnTimer();

    virtual void      OnMouseWheelForward();
    virtual void      OnMouseWheelBackward();

    void              SetNullInteractor();

    void              Start2DMode(INTERACTION_MODE mode);
    void              Start3DMode(INTERACTION_MODE mode);
    void              StartCurveMode(INTERACTION_MODE mode);
    void              StartAxisArrayMode(INTERACTION_MODE mode);
    void              Stop2DMode();
    void              Stop3DMode();
    void              StopCurveMode();
    void              StopAxisArrayMode();
private:
    VisitInteractor  *currentInteractor;

    bool             hotPointMode;
    HotPoint         currentHotPoint;

    Dolly3D          *dolly3D;
    FlyThrough       *flyThrough;
    Lineout2D        *lineout2D;
    Navigate3D       *navigate3D;
    Navigate2D       *navigate2D;
    NavigateCurve    *navigateCurve;
    NavigateAxisArray *navigateAxisArray;
    NullInteractor   *nullInteractor;
    Pick             *pick;
    Zoom2D           *zoom2D;
    Zoom3D           *zoom3D;
    ZoomCurve        *zoomCurve;

    void              SetInteractor(VisitInteractor *);
};

#endif
