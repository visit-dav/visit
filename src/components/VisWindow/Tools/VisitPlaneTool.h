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

#ifndef VISIT_PLANE_TOOL_H
#define VISIT_PLANE_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtPlaneToolInterface.h>
#include <avtMatrix.h>
#include <avtQuaternion.h>
#include <avtTrackball.h>

// Forward declarations
class VisWindow;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkTextActor;

// ****************************************************************************
// Class: VisitPlaneTool
//
// Purpose:
//   This class contains an interactive plane tool that can be used to define
//   a slice plane.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 10:59:06 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Feb 11 14:41:05 PST 2002
//   Added a method to return the interface. Added an upAxis text actor.
//
//   Hank Childs, Mon Apr 15 17:33:48 PDT 2002
//   Added some data members to handle error conditions better.
//
//   Kathleen Bonnell, Tue Nov  5 08:32:59 PST 2002   
//   Added methods TurnLightingOn/Off. 
//
//   Brad Whitlock, Fri Oct 18 15:15:05 PST 2002
//   Added text for the radius.
//
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//
//   Kathleen Bonnell, Wed May 28 16:14:22 PDT 2003 
//   Added method ReAddToWindow.
//
// ****************************************************************************

class VISWINDOW_API VisitPlaneTool : public VisitInteractiveTool
{
  public:
             VisitPlaneTool(VisWindowToolProxy &);
    virtual ~VisitPlaneTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void Start2DMode();
    virtual void Stop3DMode();

    virtual void SetForegroundColor(double, double, double);

    virtual const char *  GetName() const { return "Plane"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateView();
    virtual void UpdateTool();

    virtual void TurnLightingOn();
    virtual void TurnLightingOff();
    virtual void ReAddToWindow();

  protected:
    // Callback functions for the tool's hot points.
    static void TranslateCallback(VisitInteractiveTool *, CB_ENUM,
                                  int, int, int, int);
    static void TranslateNormalCallback(VisitInteractiveTool *, CB_ENUM,
                                  int, int, int, int);
    static void RotateXCallback(VisitInteractiveTool *, CB_ENUM,
                                int, int, int, int);
    static void RotateYCallback(VisitInteractiveTool *, CB_ENUM,
                                int, int, int, int);
    static void FreeRotateCallback(VisitInteractiveTool *, CB_ENUM,
                                   int, int, int, int);
    static void ResizeCallback(VisitInteractiveTool *, CB_ENUM,
                               int, int, int, int);

    avtVector Normal() const;

    virtual void CallCallback();
    void Translate(CB_ENUM, int, int, int, int);
    void TranslateNormal(CB_ENUM, int, int, int, int);
    void RotateX(CB_ENUM, int, int, int, int);
    void RotateY(CB_ENUM, int, int, int, int);
    void FreeRotate(CB_ENUM, int, int, int, int);
    void Resize(CB_ENUM, int, int, int, int);

    void CreatePlaneActor();
    void CreateVectorActor();
    void SetAwayColor(double, double, double);
    void UpdateNormalVectorColor();
    bool FacingAway() const;

    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();
    void AddRadiusText();
    void RemoveRadiusText();
    void UpdateRadiusText();

    void GetBoundingBoxOutline(avtVector *v, int &nverts) const;
    void ClipAgainstPlane(avtVector *, int &, const avtVector &,
                          const avtVector &) const;
    void CreateOutline();
    void DeleteOutline();
    void AddOutline();
    void RemoveOutline();
    void UpdateOutline();

    void InitialActorSetup();
    void FinalActorSetup();

    void       DoTransformations();
    avtMatrix  GetTrackballMatrix(const avtVector&, const avtVector&,int = -1);

    double              focalDepth;
    double              originalScale;
    double              originalDistance;
    double             translationDistance;
    bool               normalAway;
    bool               disableWhenNoPlots;
    vtkActor          *planeActor;
    vtkPolyDataMapper *planeMapper;
    vtkPolyData       *planeData;
    vtkActor          *vectorActor;
    vtkPolyDataMapper *vectorMapper;
    vtkPolyData       *vectorData;
    vtkActor          *outlineActor;
    vtkPolyDataMapper *outlineMapper;
    vtkPolyData       *outlineData;
    vtkTextActor      *normalTextActor;
    vtkTextActor      *originTextActor;
    vtkTextActor      *upAxisTextActor;
    vtkTextActor      *radiusTextActor;

    avtPlaneToolInterface Interface;

    avtTrackball       trackball;
    HotPointVector     origHotPoints;
    avtMatrix          SMtx;
    avtMatrix          RMtx;
    avtMatrix          TMtx;

    bool               addedOutline;
    bool               addedBbox;
};

#endif
