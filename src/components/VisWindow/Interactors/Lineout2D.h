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
//                                 Lineout2D.h                               //
// ************************************************************************* //

#ifndef LINEOUT_2D_H
#define LINEOUT_2D_H

#include <VisitInteractor.h>


class vtkActor2D;
class vtkPolyData;
class vtkPolyDataMapper2D;

class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: Lineout2D
//
//  Purpose:
//      Defines what Visit's 2D Lineout interactions should look like.  
//
//  Note:  Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 16, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from all ButtonAction methods and OnMouseMove, 
//    in order to match vtk's new interactor api.
//
//    Kathleen Bonnell, Tue Feb 18 15:14:15 PST 2003     
//    Added OnTimer method.
//    
// ****************************************************************************

class VISWINDOW_API Lineout2D : public VisitInteractor
{
  public:
                        Lineout2D(VisWindowInteractorProxy &);
    virtual            ~Lineout2D();
 
    virtual void        OnMouseMove();
    virtual void        OnTimer();
    virtual void        StartLeftButtonAction();
    virtual void        EndLeftButtonAction();
    virtual void        AbortLeftButtonAction();

  protected:
    int                    anchorX, anchorY;

    double                  canvasDeviceMinX, canvasDeviceMaxX;
    double                  canvasDeviceMinY, canvasDeviceMaxY;

    vtkPolyData           *rubberBand;
    vtkPolyDataMapper2D   *rubberBandMapper;
    vtkActor2D            *rubberBandActor;

    bool                   rubberBandMode;
    bool                   doAlign;
      
    void                   StartRubberBand(int, int);
    void                   EndRubberBand();
    void                   UpdateRubberBand(int, int, int, int, int, int);
    void                   DrawRubberBandLine(int, int, int, int);

    void                   SetCanvasViewport(void);
    void                   ForceCoordsToViewport(int &, int &);
    void                   Lineout(void);
    void                   AlignToAxis(int &, int &);

};


#endif




