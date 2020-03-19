// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIS_WIN_TOOLS_H
#define VIS_WIN_TOOLS_H
#include <viswindow_exports.h>
#include <VisWinColleague.h>
#include <VisWindowTypes.h>
#include <VisWindowToolProxy.h>
#include <avtToolInterface.h>
#include <vector>

// Forward declarations.
class VisWindowColleagueProxy;
class VisitInteractiveTool;
class VisitAxisRestrictionTool;
class VisitBoxTool;
class VisitLineTool;
class VisitPlaneTool;
class VisitPointTool;
class VisitSphereTool;
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
//   Jeremy Meredith, Fri Feb  1 12:17:28 EST 2008
//   Added the axis restriction tool.
//   Added UpdatePlotList.
//
//   Jeremy Meredith, Tue Apr 22 14:33:01 EDT 2008
//   Removed Extents tool.  (Subsumed by axis restriction tool.)
//
//   Jeremy Meredith, Wed May 19 11:00:51 EDT 2010
//   Added support for 3D axis scaling.
//
//   Burlen Loring, Mon Sep 28 16:04:23 PDT 2015
//   Added API for setting tool visibility
//
// ****************************************************************************

class VISWINDOW_API VisWinTools : public VisWinColleague
{
  public:
                     VisWinTools(VisWindowColleagueProxy &, bool createTools);
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
    virtual void     UpdatePlotList(std::vector<avtActor_p> &);

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

    virtual void     Set3DAxisScalingFactors(bool scale, const double s[3]);

    void             SetVisibility(int);

  protected:
    void                  ClearHighlight();
    void                  RegenerateHighlight();
    void                  UpdateHighlight();

    VisWindowToolProxy    toolProxy;
    vtkHighlightActor2D  *highlightActor;
    bool                  highlightAdded;
    VisitInteractiveTool *tools[10];
    int                   numTools;

    VisitBoxTool             *boxTool;
    VisitLineTool            *lineTool;
    VisitPlaneTool           *planeTool;
    VisitPointTool           *pointTool;
    VisitSphereTool          *sphereTool;
    VisitAxisRestrictionTool *axisRestrictionTool;
};

#endif
