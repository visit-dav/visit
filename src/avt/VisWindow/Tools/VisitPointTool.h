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

#ifndef VISIT_POINT_TOOL_H
#define VISIT_POINT_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtPointToolInterface.h>

// Forward declarations
class VisWindow;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkTextActor;

// ****************************************************************************
// Class: VisitPointTool
//
// Purpose:
//   This class contains an interactive point tool.
//
// Notes:      
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:20:16 PDT 2003
//
// Modifications:
//   Akira Haddox, Wed Jul  2 14:53:50 PDT 2003
//   Added translation along an axis.
//
//   Akira Haddox, Mon Aug  4 12:48:02 PDT 2003
//   Removed unneeded point actor.
//
//   Jeremy Meredith, Fri Feb  1 18:07:16 EST 2008
//   Added new value to callback used to pass the hotpoint's "data" field.
//
// ****************************************************************************

class VISWINDOW_API VisitPointTool : public VisitInteractiveTool
{
  public:
             VisitPointTool(VisWindowToolProxy &);
    virtual ~VisitPointTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void Start2DMode();
    virtual void Start3DMode();
    virtual void Stop3DMode();
    
    virtual void SetForegroundColor(double, double, double);
    
    virtual const char *  GetName() const { return "Point"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateView();
    virtual void UpdateTool();

  protected:
    // Callback functions for the tool's hot points.
    static void TranslateCallback(VisitInteractiveTool *, CB_ENUM,
                                  int, int, int, int, int);

    virtual void CallCallback();
    void Translate(CB_ENUM, int, int, int, int, int);

    avtVector ComputeTranslationDistance(int);

    void CreateGuide();
    void DeleteGuide();
    void AddGuide();
    void RemoveGuide();
    void UpdateGuide();
    void GetGuidePoints(avtVector *pts);

    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();

    void CreateSphere();
    void DeleteSphere();
    void UpdateSphere();

    void InitialActorSetup();
    void FinalActorSetup();

    double                focalDepth;
    avtVector            translationDistance;
    vtkActor            *guideActor;
    vtkPolyDataMapper   *guideMapper;
    vtkPolyData         *guideData;
    vtkActor            *sphereActor;
    vtkPolyDataMapper   *sphereMapper;
    vtkPolyData         *sphereData;
    vtkTextActor        *pointTextActor;

    avtPointToolInterface Interface;
    bool                 addedBbox;
    bool                 window3D;
    bool                 addedGuide;
    
    enum TranslateDirection
    {
        none = 0,
        leftAndRight,
        upAndDown,
        inAndOut
    };

    TranslateDirection   axisTranslate;
};

#endif
