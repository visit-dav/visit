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

#ifndef VISIT_BOX_TOOL_H
#define VISIT_BOX_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtBoxToolInterface.h>
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
// Class: VisitBoxTool
//
// Purpose:
//   This class contains an interactive plane tool that can be used to define
//   a slice plane.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 07:13:08 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//
//   Kathleen Bonnell, Wed May 28 16:09:47 PDT 2003 
//   Add method ReAddToWindow.
//
//   Brad Whitlock, Tue Jul 13 14:12:33 PST 2004
//   Added new handlers for the new hotpoints.
//
// ****************************************************************************

class VISWINDOW_API VisitBoxTool : public VisitInteractiveTool
{
  public:
             VisitBoxTool(VisWindowToolProxy &);
    virtual ~VisitBoxTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void Start2DMode();
    virtual void Stop3DMode();
    virtual void UpdateView();

    virtual void SetForegroundColor(double, double, double);

    virtual const char *  GetName() const { return "Box"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateTool();
    virtual void ReAddToWindow();
    void SetActiveHotPoint(int v) { activeHotPoint = v; };

  protected:
    // Callback functions for the tool's hot points.
    static void TranslateCallback(VisitInteractiveTool *, CB_ENUM,
                                  int, int, int, int);
    static void ResizeCallback(VisitInteractiveTool *, CB_ENUM,
                               int, int, int, int);
    static void XMINCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int);
    static void XMAXCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int);
    static void YMINCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int);
    static void YMAXCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int);
    static void ZMINCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int);
    static void ZMAXCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int);

    virtual void CallCallback();
    void Translate(CB_ENUM, int, int, int, int);
    void Resize(CB_ENUM, int, int, int, int);

    void CreateBoxActor();

    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();
    void GetHotPointLabel(int index, char *str);

    void CreateOutline();
    void DeleteOutline();
    void AddOutline();
    void RemoveOutline();
    void UpdateOutline();
    void GetBoundingBoxOutline(int a, avtVector *verts, bool giveMin);

    void InitialActorSetup();
    void FinalActorSetup();

    void DoTransformations();

    vtkActor           *boxActor;
    vtkPolyDataMapper  *boxMapper;
    vtkPolyData        *boxData;

    vtkActor           *outlineActor[3];
    vtkPolyDataMapper  *outlineMapper[3];
    vtkPolyData        *outlineData[3];
    vtkTextActor       *outlineTextActor[4];

    vtkTextActor       *originTextActor;
    vtkTextActor       *labelTextActor[7];

    avtBoxToolInterface Interface;

    avtTrackball        trackball;
    HotPointVector      origHotPoints;
    avtMatrix           SMtx;
    avtMatrix           TMtx;

    bool                addedOutline;
    bool                addedBbox;
    bool                depthTranslate;
    int                 activeHotPoint;
    double               focalDepth;
    double               originalDistance;
    avtVector           depthTranslationDistance;

};

#endif
