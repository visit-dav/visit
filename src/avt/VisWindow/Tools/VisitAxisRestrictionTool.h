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

#ifndef VISIT_AXIS_RESTRICTION_TOOL_H
#define VISIT_AXIS_RESTRICTION_TOOL_H
#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtAxisRestrictionToolInterface.h>
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
// Class: VisitAxisRestrictionTool
//
// Purpose:
//   This class contains an interactive tool that can be used to define
//   restrictions along arrays of parallel axes.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//    Jeremy Meredith, Thu Feb  7 17:59:55 EST 2008
//    Added support for array variables and bin-defined x positions.
//
//    Jeremy Meredith, Fri Feb 15 13:21:20 EST 2008
//    Added axis names to the axis restriction tool.
//
// ****************************************************************************

class VISWINDOW_API VisitAxisRestrictionTool : public VisitInteractiveTool
{
  public:
             VisitAxisRestrictionTool(VisWindowToolProxy &);
    virtual ~VisitAxisRestrictionTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;

    virtual void StopAxisArrayMode();

    virtual void SetForegroundColor(double, double, double);

    virtual const char *  GetName() const { return "AxisRestriction"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateView();
    virtual void UpdateTool();
    virtual void ReAddToWindow();
    virtual void UpdatePlotList(std::vector<avtActor_p> &list);
    virtual void FullFrameOn(const double, const int);
    virtual void FullFrameOff();

  protected:
    // Callback functions for the tool's hot points.
    static void MoveCallback(VisitInteractiveTool *, CB_ENUM,
                             int, int, int, int, int);

    virtual void CallCallback();
    void Move(CB_ENUM, int, int, int, int, int);

    void CreateTextActors();
    void DeleteTextActors();
    void AddText();
    void RemoveText();
    void UpdateText();

    void InitialActorSetup();
    void FinalActorSetup();

    void DoClampAndTransformations();

    static const float         radius;
    double                     focalDepth;
    std::vector<vtkTextActor*> posTextActors;
    std::vector<std::string>   axesNames;
    std::vector<double>        axesMin;
    std::vector<double>        axesMax;
    std::vector<double>        axesXPos;
    double                     color[3];

    avtAxisRestrictionToolInterface Interface;

    HotPointVector     origHotPoints;

    bool               addedBbox;
    bool               textAdded;
};

#endif
