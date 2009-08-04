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

#ifndef VISIT_LINE_TOOL_H
#define VISIT_LINE_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtLineToolInterface.h>
#include <avtMatrix.h>
#include <avtQuaternion.h>
#include <avtTrackball.h>

// Forward declarations
class VisWindow;
class vtkActor;
class vtkLineSource;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkTextActor;

// ****************************************************************************
// Class: VisitLineTool
//
// Purpose:
//   This class contains an interactive line tool that can be used to define
//   a lineout curve.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 15:23:54 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Jul 23 10:23:18 PDT 2002
//   I removed the NoPlots redeclaration since it now uses the base class's
//   method implementation.
//
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//
//   Kathleen Bonnell, Wed May 28 16:14:22 PDT 2003 
//   Add method ReAddToWindow.
//
//   Kathleen Bonnell, Fri Jun  6 15:36:24 PDT 2003 
//   Add FullFrameOn/Off methods. 
//
//   Jeremy Meredith, Fri Feb  1 18:07:16 EST 2008
//   Added new value to callback used to pass the hotpoint's "data" field.
//
// ****************************************************************************

class VISWINDOW_API VisitLineTool : public VisitInteractiveTool
{
  public:
             VisitLineTool(VisWindowToolProxy &);
    virtual ~VisitLineTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void Start2DMode();
    virtual void Start3DMode();
    virtual void Stop3DMode();

    virtual void SetForegroundColor(double, double, double);

    virtual const char *  GetName() const { return "Line"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateView();
    virtual void UpdateTool();
    virtual void ReAddToWindow();

    virtual void FullFrameOn(const double, const int);
    virtual void FullFrameOff(void);

  protected:
    // Callback functions for the tool's hot points.
    static void TranslatePoint1Callback(VisitInteractiveTool *, CB_ENUM,
                                        int, int, int, int, int);
    static void TranslatePoint2Callback(VisitInteractiveTool *, CB_ENUM,
                                        int, int, int, int, int);
    static void TranslateCallback(VisitInteractiveTool *, CB_ENUM,
                                  int, int, int, int, int);

    virtual void CallCallback();
    void Translate(CB_ENUM, int, int, int, int, int);
    void InitializePoints();

    void CreateLineActor();
    void UpdateLine();

    void CreateGuide();
    void DeleteGuide();
    void AddGuide(int);
    void RemoveGuide();
    void UpdateGuide(int);
    void GetGuidePoints(int pi, avtVector *pts);

    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();

    void InitialActorSetup(int);
    void FinalActorSetup();

    double                focalDepth;
    double                translationDistance;
    avtVector            depthTranslationDistance;
    vtkLineSource       *lineSource;
    vtkActor            *lineActor;
    vtkPolyDataMapper   *lineMapper;
    vtkPolyData         *lineData;
    vtkActor            *guideActor;
    vtkPolyDataMapper   *guideMapper;
    vtkPolyData         *guideData;
    vtkTextActor        *pointTextActor[2];

    avtLineToolInterface Interface;
    bool                 addedBbox;
    bool                 window3D;
    bool                 addedGuide;
    bool                 enlongating;
    bool                 depthTranslate;
};

#endif
