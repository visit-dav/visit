/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                             ZoomInteractor.h                              //
// ************************************************************************* //

#ifndef ZOOM_INTERACTOR_H
#define ZOOM_INTERACTOR_H
#include <viswindow_exports.h>
#include <VisitInteractor.h>

class vtkActor2D;
class vtkPolyData;
class vtkPolyDataMapper2D;
class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: ZoomInteractor
//
//  Purpose:
//      An abstract type (conceptually) that has two concrete types, Zoom2D
//      and Zoom3D.  It captures the common routines for the two and defines
//      them in one spot.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Dec 21 08:35:12 PST 2001
//    Add OnTimer.
//
//    Hank Childs, Mon Mar 25 16:06:00 PST 2002
//    Remove lastX and lastY.  They were defined by the base class and being
//    erroneously re-defined here.
//
//    Eric Brugger, Tue Mar 26 14:33:43 PST 2002
//    Remove UpdateViewport and make ZoomCamera pure virtual.
//
//    Eric Brugger, Fri Apr 12 14:10:16 PDT 2002
//    Add an overloaded pure virtual ZoomCamera.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from all OnMouseMove, in order to match
//    vtk's new interactor api.
//
//    Akira Haddox, Thu Jul  3 14:15:48 PDT 2003
//    Added two flags for when control and shift are set.
//    Made Start/End/UpdateRubberBand virtual (overloaded in Zoom2D).
//
//    Kathleen Bonnell, Wed Aug 18 10:10:35 PDT 2004 
//    Added two flags for when InteractorAtts 'ClampToSquare' and 
//    'ShowGuidelines' are set. 
//
// ****************************************************************************

class VISWINDOW_API ZoomInteractor : public VisitInteractor
{
  public:
                           ZoomInteractor(VisWindowInteractorProxy &);
    virtual               ~ZoomInteractor();

    virtual void           OnMouseMove();
    virtual void           OnTimer(void);
 
  protected:
    int                    anchorX, anchorY;

    double                 canvasDeviceMinX, canvasDeviceMaxX;
    double                 canvasDeviceMinY, canvasDeviceMaxY;

    vtkPolyData           *rubberBand;
    vtkPolyDataMapper2D   *rubberBandMapper;
    vtkActor2D            *rubberBandActor;

    bool                   rubberBandMode;
    bool                   rubberBandDrawn;
        
    bool                   shiftKeyDown;
    bool                   controlKeyDown;
    bool                   shouldClampSquare;
    bool                   shouldDrawGuides;

    virtual void           StartRubberBand(int, int);
    virtual void           EndRubberBand();
    virtual void           UpdateRubberBand(int, int, int, int, int, int);
    virtual void           DrawRubberBandLine(int, int, int, int);

    void                   SetCanvasViewport(void);
    void                   ForceCoordsToViewport(int &, int &);

    virtual void           ZoomCamera(void)=0;
    virtual void           ZoomCamera(const int x, const int y)=0;

};


#endif


