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

#ifndef VIS_WIN_TOOLS_H
#define VIS_WIN_TOOLS_H
#include <viswindow_exports.h>
#include <VisWinColleague.h>
#include <VisWindowTypes.h>
#include <VisWindowToolProxy.h>
#include <avtToolInterface.h>

// Forward declarations.
class VisWindowColleagueProxy;
class VisitInteractiveTool;
class VisitBoxTool;
class VisitLineTool;
class VisitPlaneTool;
class VisitPointTool;
class VisitSphereTool;
class VisitExtentsTool;
class vtkHighlightActor2D;

// ****************************************************************************
// Class: VisWinTools
//
// Purpose: 
//   This class is a colleague in the VisWindow that is in charge of the
//   interactive tools.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:55:19 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Feb 11 14:34:46 PST 2002
//   Added a methods to get a tool interface and update a tool.
//
//   Brad Whitlock, Fri May 3 11:09:48 PDT 2002
//   Added the sphere tool.
//
//   Brad Whitlock, Tue Jun 18 15:31:36 PST 2002
//   Added the line tool and NoPlots method.
//
//   Brad Whitlock, Mon Oct 28 15:31:48 PST 2002
//   Added the box tool.
//
//   Kathleen Bonnell, Tue Nov  5 08:32:59 PST 2002 
//   Added methods to turn lighting on/off. 
//
//   Kathleen Bonnell, Wed May 28 15:58:15 PDT 2003 
//   Added method ReAddToWindow. 
//
//   Kathleen Bonnell, Fri Jun  6 15:23:05 PDT 2003
//   Added FullFrameOn/Off methods. 
//
//   Akira Haddox, Thu Jun 12 15:56:03 PDT 2003
//   Added point tool.
//
//   Kathleen Bonnell, Wed Dec  3 16:49:14 PST 2003 
//   Renamed 'ReAddToWindow' to 'ReAddToolsToWindow', as tools get re-added
//   at a different time than other colleagues.
//
//   Mark Blair, Wed Aug 30 14:19:00 PDT 2006
//   Added the extents tool.
//
// ****************************************************************************

class VISWINDOW_API VisWinTools : public VisWinColleague
{
  public:
                     VisWinTools(VisWindowColleagueProxy &);
    virtual         ~VisWinTools();

    virtual void     SetBackgroundColor(double r, double g, double b);
    virtual void     SetForegroundColor(double r, double g, double b);

    virtual void     Start2DMode();
    virtual void     Stop2DMode();
    virtual void     Start3DMode();
    virtual void     Stop3DMode();
    virtual void     NoPlots();
    virtual void     UpdateView();
    virtual void     ReAddToolsToWindow();

    bool             GetHotPoint(int x, int y, HotPoint &h) const;
    int              NumToolsEnabled() const;
    void             SetHighlightEnabled(bool);

    const char      *GetToolName(int i) const;
    int              GetNumTools() const;
    void             SetToolEnabled(int i, bool val);
    bool             GetToolEnabled(int i) const;
    bool             GetToolAvailable(int i) const;
    avtToolInterface &GetToolInterface(int i) const;
    void             UpdateTool(int i);

    void             TurnLightingOn();
    void             TurnLightingOff();

    virtual void     FullFrameOn(const double, const int);
    virtual void     FullFrameOff(void);

  protected:
    void                  ClearHighlight();
    void                  RegenerateHighlight();
    void                  UpdateHighlight();

    VisWindowToolProxy    toolProxy;
    vtkHighlightActor2D  *highlightActor;
    bool                  highlightAdded;
    VisitInteractiveTool *tools[10];
    int                   numTools;

    VisitBoxTool         *boxTool;
    VisitLineTool        *lineTool;
    VisitPlaneTool       *planeTool;
    VisitPointTool       *pointTool;
    VisitSphereTool      *sphereTool;
    VisitExtentsTool     *extentsTool;
};

#endif
